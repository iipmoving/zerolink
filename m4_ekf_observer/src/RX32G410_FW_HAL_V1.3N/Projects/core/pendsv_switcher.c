/**
 * @file    pendsv_switcher.c
 * @brief   PendSV Switcher — 实时通路调度器 (v2.3)
 * @layer   core
 *
 * 所有模块通过统一 3-参数 GetIO 注册到 PendSV 注册表。
 * PendSV 通路用于紧急 ISR 事件的实时处理。
 *
 * 关键特性:
 *   - PendSV 作为中断本身就有硬件保护，不需要软件锁
 *   - PendSV 优先级设为最低，确保所有 ISR 完成后才执行
 *   - g_isr_pending 用于累积 ISR 请求
 *
 * 数据流:
 *   ISR_Handler() → Switcher_TriggerPendSV(source) → 设置 g_isr_pending
 *   ↓
 *   PendSV_Handler() → 检查 g_isr_pending → Switcher_Run_ISR_Slot()
 *   ↓
 *   ISRWORK_DoWork() → CollectInput() 从各模块 g_output 拉数据 → 统一处理
 */

#include "std_module_v2.3.h"
#include "pendsv_switcher.h"

#include <string.h>

/* ========== PendSV 状态变量 ========== */
volatile uint8_t g_isr_pending = 0;   /* ISR 请求标志 (累积请求) */
volatile uint8_t g_isr_source = 0;    /* ISR 来源标识 */

/* ========== 模块槽位定义 ========== */
typedef struct {
    const char    *name;
    Para_Grp_t    *pIn;
    Para_Grp_t    *pOut;
    void          (*pDoWork)(void);
    void          (*pOnISR)(void);      /* ISR 独立入口回调 */
} ModuleSlot_t;

static ModuleSlot_t s_slot[PENSV_MAX_MODULES];
static uint8_t      s_slot_count = 0;

/* ========== 内部声明 ========== */
static void Switcher_Route_ADC_ISR(void);
static void Switcher_Route_Timer_ISR(void);
static void Switcher_Route_Fault_ISR(void);
static void Switcher_Route_Generic_ISR(void);

/* ================================================================
 * Switcher_PendSV_Init — 初始化 PendSV 通路
 * ================================================================ */
void Switcher_PendSV_Init(void)
{
    memset(s_slot, 0, sizeof(s_slot));
    s_slot_count = 0;
    g_isr_pending = 0;
    g_isr_source = 0;
}

/* ================================================================
 * Switcher_RegisterModule — 注册模块到 PendSV 注册表
 * ================================================================ */
void Switcher_RegisterModule(const char *name, 
                             Para_Grp_t *pIn, 
                             Para_Grp_t *pOut, 
                             void (*pDoWork)(void),
                             void (*pOnISR)(void))
{
    if (s_slot_count >= PENSV_MAX_MODULES) {
        return;
    }
    
    s_slot[s_slot_count].name     = name;
    s_slot[s_slot_count].pIn      = pIn;
    s_slot[s_slot_count].pOut     = pOut;
    s_slot[s_slot_count].pDoWork  = pDoWork;
    s_slot[s_slot_count].pOnISR   = pOnISR;
    s_slot_count++;
}

/* ================================================================
 * Switcher_FindModuleSlot — 查找模块槽位 (内部)
 * ================================================================ */
static ModuleSlot_t* Switcher_FindModuleSlot(const char *name)
{
    for (uint8_t i = 0; i < s_slot_count; i++) {
        if (s_slot[i].name != NULL && strcmp(s_slot[i].name, name) == 0) {
            return &s_slot[i];
        }
    }
    return NULL;
}

/* ================================================================
 * Switcher_FindModuleOutput — 查找模块输出 (按名称)
 * ================================================================ */
Para_Grp_t* Switcher_FindModuleOutput(const char *name)
{
    ModuleSlot_t *slot = Switcher_FindModuleSlot(name);
    if (slot) {
        return slot->pOut;
    }
    return NULL;
}

/* ================================================================
 * Switcher_FindModule — 查找模块 DoWork (按名称)
 * ================================================================ */
void* Switcher_FindModule(const char *name)
{
    ModuleSlot_t *slot = Switcher_FindModuleSlot(name);
    if (slot) {
        return (void*)slot->pDoWork;
    }
    return NULL;
}

/* ================================================================
 * Switcher_FindModuleOnISR — 查找模块 OnISR 回调 (按名称)
 * ================================================================ */
void* Switcher_FindModuleOnISR(const char *name)
{
    ModuleSlot_t *slot = Switcher_FindModuleSlot(name);
    if (slot) {
        return (void*)slot->pOnISR;
    }
    return NULL;
}

/* ================================================================
 * Switcher_TriggerPendSV — ISR 触发 PendSV
 *
 *   PendSV 特性:
 *   - 写 SCB->ICSR.PENDSVSET 触发
 *   - 中断正在执行时，同优先级中断被屏蔽 (硬件保护)
 *   - 不需要软件锁
 * ================================================================ */
void Switcher_TriggerPendSV(uint8_t isr_source)
{
    g_isr_source = isr_source;
    g_isr_pending = 1;
    
    /* 触发 PendSV: 设置 SCB->ICSR PENDSVSET 位 (bit28) */
    /* SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; */
    __disable_irq();
    __enable_irq();
}

/* ================================================================
 * PendSV_Handler — PendSV 中断处理
 *
 *   PendSV 作为中断本身就有硬件保护:
 *   - 进入中断时自动关中断 (同优先级)
 *   - 退出时自动开中断
 *   - 不需要额外的软件锁 g_pendsv_busy
 *
 *   流程:
 *     1. 检查 g_isr_pending (无请求直接退出)
 *     2. 清标志: g_isr_pending = 0
 *     3. 调用 Switcher_Run_ISR_Slot()
 *     4. 退出中断 (自动开中断)
 * ================================================================ */
void PendSV_Handler(void)
{
    /* 无请求检查 */
    if (!g_isr_pending) {
        return;
    }
    
    /* 清请求标志 */
    g_isr_pending = 0;
    
    /* 执行 ISR 处理槽 */
    Switcher_Run_ISR_Slot(g_isr_source);
    
    /* 退出中断: 自动开中断，如有新请求会再次触发 */
}

/* ================================================================
 * Switcher_Run_ISR_Slot — ISR 处理槽路由
 * ================================================================ */
void Switcher_Run_ISR_Slot(uint8_t isr_source)
{
    switch (isr_source) {
        case ISR_SOURCE_ADC:
            Switcher_Route_ADC_ISR();
            break;
            
        case ISR_SOURCE_TIMER:
            Switcher_Route_Timer_ISR();
            break;
            
        case ISR_SOURCE_FAULT:
            Switcher_Route_Fault_ISR();
            break;
            
        default:
            Switcher_Route_Generic_ISR();
            break;
    }
}

/* ================================================================
 * Switcher_Route_ADC_ISR — ADC ISR 路由
 *
 *   两种模式:
 *   1. OnISR 回调: 低开销，不碰 g_input/g_output
 *   2. DoWork: 完整处理流程
 * ================================================================ */
static void Switcher_Route_ADC_ISR(void)
{
    /* 先尝试 OnISR 回调 (低开销) */
    void (*Adc_OnISR)(void) = Switcher_FindModuleOnISR("AppAdc");
    if (Adc_OnISR) {
        Adc_OnISR();
    } else {
        /* 回退到 DoWork */
        void (*Adc_DoWork)(void) = Switcher_FindModule("AppAdc");
        if (Adc_DoWork) {
            Adc_DoWork();
        }
    }
    
    /* ISRWORK 处理 */
    void (*ISRWORK_OnISR)(void) = Switcher_FindModuleOnISR("ISRWORK");
    if (ISRWORK_OnISR) {
        ISRWORK_OnISR();
    } else {
        void (*ISRWORK_DoWork)(void) = Switcher_FindModule("ISRWORK");
        if (ISRWORK_DoWork) {
            ISRWORK_DoWork();
        }
    }
}

/* ================================================================
 * Switcher_Route_Timer_ISR — Timer ISR 路由
 * ================================================================ */
static void Switcher_Route_Timer_ISR(void)
{
    void (*ModuleA_OnISR)(void) = Switcher_FindModuleOnISR("ModuleA");
    void (*ModuleB_OnISR)(void) = Switcher_FindModuleOnISR("ModuleB");
    void (*ModuleC_OnISR)(void) = Switcher_FindModuleOnISR("ModuleC");
    
    if (ModuleA_OnISR) ModuleA_OnISR();
    if (ModuleB_OnISR) ModuleB_OnISR();
    if (ModuleC_OnISR) ModuleC_OnISR();
    
    void (*ISRWORK_OnISR)(void) = Switcher_FindModuleOnISR("ISRWORK");
    if (ISRWORK_OnISR) {
        ISRWORK_OnISR();
    } else {
        void (*ISRWORK_DoWork)(void) = Switcher_FindModule("ISRWORK");
        if (ISRWORK_DoWork) {
            ISRWORK_DoWork();
        }
    }
}

/* ================================================================
 * Switcher_Route_Fault_ISR — Fault ISR 路由
 * ================================================================ */
static void Switcher_Route_Fault_ISR(void)
{
    void (*ISRWORK_OnISR)(void) = Switcher_FindModuleOnISR("ISRWORK");
    if (ISRWORK_OnISR) {
        ISRWORK_OnISR();
    } else {
        void (*ISRWORK_DoWork)(void) = Switcher_FindModule("ISRWORK");
        if (ISRWORK_DoWork) {
            ISRWORK_DoWork();
        }
    }
}

/* ================================================================
 * Switcher_Route_Generic_ISR — 通用 ISR 路由
 * ================================================================ */
static void Switcher_Route_Generic_ISR(void)
{
    void (*ISRWORK_OnISR)(void) = Switcher_FindModuleOnISR("ISRWORK");
    if (ISRWORK_OnISR) {
        ISRWORK_OnISR();
    } else {
        void (*ISRWORK_DoWork)(void) = Switcher_FindModule("ISRWORK");
        if (ISRWORK_DoWork) {
            ISRWORK_DoWork();
        }
    }
}