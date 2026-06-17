/**
 * @file    isrwork.c
 * @brief   ISRWORK 模块 — ISR 实时处理入口 (v2.3)
 * @layer   app
 *
 * 输入:
 *   - A_Data_t: 来自 ModuleA
 *   - B_Data_t: 来自 ModuleB
 *   - C_Data_t: 来自 ModuleC
 *   - isr_source: ISR 来源标识
 *
 * 输出:
 *   - ISRWORK_OutData_t: 处理结果
 *
 * 两条通路:
 *   1. 主循环: Switcher_Run_Slot1() → ISRWORK_DoWork()
 *   2. PendSV: PendSV_Handler() → Switcher_Run_ISR_Slot() → ISRWORK_DoWork()
 *
 * 使用:
 *   1. #include "core/std_module_v2.3.h"
 *   2. MODULE_SKELETON(ISRWORK)
 *   3. 实现 Init() + ProcessInput()
 *   4. MODULE_EXPORT(ISRWORK)
 *   5. 在 Switcher_PendSV_Init() 后调用 Switcher_RegisterModule("ISRWORK", ...)
 */

#include "core/std_module_v2.3.h"
#include "core/pendsv_switcher.h"
#include <string.h>

/* ========== 模块依赖: 声明外部 g_output ========== */
DECLARE_MODULE_OUTPUT(ModuleA);
DECLARE_MODULE_OUTPUT(ModuleB);
DECLARE_MODULE_OUTPUT(ModuleC);

/* ========== 1. 输入数据结构 ========== */
#pragma pack(4)
typedef struct {
    uint8_t  a_valid;
    uint8_t  b_valid;
    uint8_t  c_valid;
    uint8_t  isr_source;    /* ISR 来源标识 */
    uint8_t  res[4];        /* 对齐填充 */
    
    uint16_t a_value;       /* ModuleA 数据 */
    uint16_t b_value;       /* ModuleB 数据 */
    uint16_t c_value;       /* ModuleC 数据 */
} ISRWORK_InData_t;
#pragma pack()

/* ========== 2. 输出数据结构 ========== */
#pragma pack(4)
typedef struct {
    uint8_t  result_valid;
    uint8_t  res[3];
    uint16_t fused_value;   /* 融合结果 */
    uint16_t fault_code;    /* 故障码 */
} ISRWORK_OutData_t;
#pragma pack()

static ISRWORK_InData_t  s_in;
static ISRWORK_OutData_t s_out;

/* ========== 3. 骨架宏 ========== */
MODULE_SKELETON(ISRWORK);

/* ========== 4. 辅助: 收集输入数据 ========== */
static void CollectInput(void)
{
    ISRWORK_InData_t *in = (ISRWORK_InData_t *)g_input.para;
    
    /* 从 ModuleA 收集数据 */
    if (g_ModuleA_output.para != NULL) {
        uint16_t *pA = (uint16_t *)g_ModuleA_output.para;
        in->a_value = *pA;
        in->a_valid = 1;
    }
    
    /* 从 ModuleB 收集数据 */
    if (g_ModuleB_output.para != NULL) {
        uint16_t *pB = (uint16_t *)g_ModuleB_output.para;
        in->b_value = *pB;
        in->b_valid = 1;
    }
    
    /* 从 ModuleC 收集数据 */
    if (g_ModuleC_output.para != NULL) {
        uint16_t *pC = (uint16_t *)g_ModuleC_output.para;
        in->c_value = *pC;
        in->c_valid = 1;
    }
    
    /* 记录 ISR 来源 */
    in->isr_source = g_isr_source;
    
    g_input.info.status |= ST_NEW;
}

/* ========== 5. ProcessInput: 三段式处理 ========== */
static void ProcessInput(void)
{
    ISRWORK_InData_t  *in  = (ISRWORK_InData_t *)g_input.para;
    ISRWORK_OutData_t *out = (ISRWORK_OutData_t *)g_output.para;
    
    /* ① 输入段: 收集数据 */
    CollectInput();
    
    if (!(g_input.info.status & ST_NEW)) {
        return;  /* 无新数据，直接返回 */
    }
    
    /* ② 计算段: 根据 ISR 来源选择处理策略 */
    switch (in->isr_source) {
        case ISR_SOURCE_ADC:
            /* ADC ISR: 融合 A 和 B 数据 */
            if (in->a_valid && in->b_valid) {
                out->fused_value = (in->a_value + in->b_value) / 2;
            }
            break;
            
        case ISR_SOURCE_TIMER:
            /* Timer ISR: 完整融合 A+B+C */
            if (in->a_valid && in->b_valid && in->c_valid) {
                out->fused_value = (in->a_value + in->b_value + in->c_value) / 3;
            }
            break;
            
        case ISR_SOURCE_FAULT:
            /* Fault ISR: 故障处理 */
            out->fault_code = 0x1234;
            break;
            
        default:
            /* 通用处理 */
            break;
    }
    
    /* ③ 输出段: 置结果有效标志 */
    out->result_valid = 1;
    g_output.info.status |= ST_OUT;
    
    /* 清除输入标志 */
    g_input.info.status &= ~ST_NEW;
}

/* ========== 6. Init: 初始化 ========== */
static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

/* ========== 7. 导出模块 ========== */
MODULE_EXPORT(ISRWORK);