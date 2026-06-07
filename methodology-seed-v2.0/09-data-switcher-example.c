/* ===================================================================
 * 09 — 数据交换机完整范式：两模块 + 中间层 (v2.1)
 *
 * 核心:
 *   - std_module.h: MODULE_SKELETON() + MODULE_EXPORT() 消除样板
 *   - Para_Grp_t: 统一参数组 { Info_Header info; void *para; }
 *   - __weak 路由: producer 调 _onOutput → __weak chain → consumer STRONG memcpy
 *   - Switcher: 只注册 DoWork 函数指针 + 顺序调用, 不搬运数据, 不碰 status
 *
 * 本文件演示:
 *   Module ADC     — 纯生产者: 采数据 → 写 g_output.para → 置 ST_OUT
 *   Module Power   — 纯消费者: STRONG Adc_OnOutput memcpy → ST_NEW → ProcessInput 消费
 *   Switcher       — 调度员: 声明 __weak 背板 + 注册 DoWork + 顺序调用
 *
 * 实际项目中分布在独立文件里，这里合并便于阅读。
 * ================================================================= */

#include <stdint.h>
#include <string.h>

/* ===================================================================
 * §0 std_module.h (core/std_module.h) — 每个模块 #include 一次
 * ================================================================= */

enum {
    ST_INIT  = 0x01,   /* bit0: 已初始化 */
    ST_NEW   = 0x02,   /* bit1: 新输入到达 */
    ST_OUT   = 0x04,   /* bit2: 输出就绪 */
};

typedef struct {
    uint8_t status;
    uint8_t res1;
    uint8_t route;
    uint8_t res2;
} Info_Header;

typedef struct {
    Info_Header info;
    void       *para;
} Para_Grp_t;

#define MODULE_SKELETON()                                                \
    static Para_Grp_t g_input;                                          \
    static Para_Grp_t g_output;                                         \
    static uint8_t    g_init_done = 0;                                  \
    static void       (*_onOutput)(Para_Grp_t *) = NULL;                \
                                                                         \
    static void Init(void);                                              \
    static void ProcessInput(void);                                      \
                                                                         \
    static void Constructor(void) {                                      \
        memset(&g_input,  0, sizeof(g_input));                           \
        memset(&g_output, 0, sizeof(g_output));                         \
        g_init_done = 0;                                                \
        Init();                                                         \
        g_output.info.status |= ST_INIT;                                 \
    }                                                                    \
                                                                         \
    static void DoWork(void) {                                           \
        if (!g_init_done) { Constructor(); g_init_done = 1; }           \
        ProcessInput();                                                  \
        if ((g_output.info.status & ST_OUT) && _onOutput) {             \
            _onOutput(&g_output);                                       \
            g_output.info.status &= ~ST_OUT;                             \
        }                                                                \
    }

#define MODULE_EXPORT(module_name)                                       \
    void module_name##_GetIO(Para_Grp_t **ppIn,                          \
                             Para_Grp_t **ppOut,                         \
                             void      (**ppDoWork)(void)) {             \
        if (!g_init_done) { Constructor(); g_init_done = 1; }           \
        *ppIn     = &g_input;                                            \
        *ppOut    = &g_output;                                           \
        *ppDoWork = DoWork;                                              \
    }                                                                    \
                                                                         \
    __attribute__((weak)) void module_name##_OnOutput(Para_Grp_t *pOut)  \
    { (void)pOut; }                                                      \
    static void _Reg__##module_name(void) __attribute__((constructor));  \
    static void _Reg__##module_name(void) {                              \
        _onOutput = module_name##_OnOutput;                              \
    }


/* ===================================================================
 * §1 模块对外接口 (_io.h)
 *
 * _io.h 只放:
 *   - 本模块的 InData_t / OutData_t (挂在 Para_Grp_t.para 后面)
 *   - 无 GetIO / DoWork 声明 — 由 MODULE_EXPORT 宏生成
 *
 * 规则:
 *   - _io.h 保留 #define guard (公开接口)
 *   - _io.h 不加入编译器 -I path → 全路径 include
 *   - 模块自身 .h (如有) 注释 //#define (私有, L0 阻断)
 * ================================================================= */

/* --------------------- include/adc_io.h ----------------------------- */
#ifndef ADC_IO_H
#define ADC_IO_H       /* ← 保留: _io.h 是公开接口 */

#include <stdint.h>

/* ADC 内部数据结构 — 挂在 Para_Grp_t.para 后面 */
/* ADC 是纯生产者 — 只有输出数据, 没有输入数据 */
typedef struct {
    uint16_t voltage;       /* mV */
    uint16_t current;       /* mA */
} AdcOutData_t;

#endif /* ADC_IO_H */


/* --------------------- include/power_io.h --------------------------- */
#ifndef POWER_IO_H
#define POWER_IO_H       /* ← 保留: _io.h 是公开接口 */

#include <stdint.h>

/* Power 内部数据结构 — 挂在 Para_Grp_t.para 后面 */
typedef struct {
    uint16_t voltage;       /* mV — 来自 ADC */
    uint16_t current;       /* mA — 来自 ADC */
    uint8_t  op_mode;       /* 运行模式 */
    uint8_t  res;           /* 32位对齐 */
} PowerInData_t;

typedef struct {
    uint16_t power;         /* mW */
    uint8_t  overcurrent;
    uint8_t  mode_active;
} PowerOutData_t;

#endif /* POWER_IO_H */


/* ===================================================================
 * §2 模块实现 — MODULE_SKELETON() + MODULE_EXPORT()
 *
 * 每个模块:
 *   1. 定义 InData_t / OutData_t + static s_in / s_out
 *   2. MODULE_SKELETON() — 展开 g_input/g_output/Constructor/DoWork
 *   3. Init() — 绑定 g_input.para / g_output.para
 *   4. ProcessInput() — 三段式
 *   5. MODULE_EXPORT(Module) — 展开 GetIO + __weak OnOutput
 *   6. STRONG {Producer}_OnOutput — 每个数据源一个, memcpy + 置 ST_NEW
 *
 * 关键: 模块不知道 Switcher 存在, 不知道谁在写 g_input.para.
 * ================================================================= */

/* ======================== adc.c ================================== */
#include "../include/adc_io.h"       /* 全路径 — 自己的 _io.h */
#include "core/std_module.h"

static AdcOutData_t s_out;
static uint8_t      _tick = 0;       /* 模拟 100ms 采一次 */

MODULE_SKELETON();

static void Init(void)
{
    memset(&s_out, 0, sizeof(s_out));
    g_output.para = &s_out;
    /* ADC 无输入 — g_input.para 保持 NULL */
}

static void ProcessInput(void)
{
    AdcOutData_t *out = (AdcOutData_t *)g_output.para;

    /* ★ 每帧先清输出标志 */
    g_output.info.status &= ~ST_OUT;

    /* ====== 计算段 ====== */
    _tick++;
    if (_tick < 10) return;          /* 没到 100ms, 本帧无产出 */
    _tick = 0;

    out->voltage = 12000;            /* 模拟采样: 12V */
    out->current = 2500;             /* 模拟采样: 2.5A */

    /* ====== 输出段 ====== */
    g_output.info.status |= ST_OUT;  /* DoWork 检测到后自动调 _onOutput */
}

MODULE_EXPORT(Adc);
/* 生成:
 *   Adc_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void))
 *   __attribute__((weak)) void Adc_OnOutput(Para_Grp_t *pOut) {}
 *   并自动注册 _onOutput = Adc_OnOutput
 */


/* ======================== power.c ================================ */
#include "../include/power_io.h"      /* 全路径 — 自己的 _io.h */
#include "core/std_module.h"
/* 不 include adc_io.h — 不知道 ADC 的存在 */

static PowerInData_t  s_in;
static PowerOutData_t s_out;

MODULE_SKELETON();

static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

static void ProcessInput(void)
{
    PowerInData_t  *in  = (PowerInData_t *)g_input.para;
    PowerOutData_t *out = (PowerOutData_t *)g_output.para;

    /* ★ 每帧先清输出标志 */
    g_output.info.status &= ~ST_OUT;

    /* ====== 输入段 ====== */
    if (!(g_input.info.status & ST_NEW)) return;
    /* ST_NEW 由 Adc_OnOutput STRONG 回调置位 (见下方) */

    /* ====== 计算段: 消费 g_input.para ====== */
    out->power       = (uint32_t)in->voltage * in->current / 1000;
    out->overcurrent = (in->current > 5000) ? 1 : 0;
    out->mode_active = in->op_mode;

    /* 消费完毕 */
    g_input.info.status &= ~ST_NEW;

    /* ====== 输出段 ====== */
    g_output.info.status |= ST_OUT;
}

MODULE_EXPORT(Power);

/* STRONG: 接收 ADC 的输出 — linker 解析 Adc_OnOutput 到这个实现 */
void Adc_OnOutput(Para_Grp_t *pOut)
{
    /* consumer 自己 memcpy — Switcher 不搬运数据 */
    memcpy(g_input.para, pOut->para, sizeof(PowerInData_t));
    g_input.info.status |= ST_NEW;
}


/* ===================================================================
 * §3 中间层 Switcher
 *
 * 全项目唯一有权 include 所有 _io.h 的文件.
 * 职责:
 *   - 声明 __weak stubs 作为路由背板
 *   - Init: 调各模块 GetIO → Switcher_Register(pDoWork)
 *   - Run:  顺序调各模块 DoWork
 *
 * Switcher 不搬运数据, 不碰任何模块的 status 字节.
 * 数据通过 __weak 回调自动路由.
 *
 * 时序:
 *   Adc_DoWork() → 写 g_output.para → ST_OUT → DoWork 自动调 _onOutput
 *   → __weak Adc_OnOutput → linker 解析到 Power 的 STRONG Adc_OnOutput
 *   → memcpy(g_input.para, pOut->para, ...) → ST_NEW
 *   → Power_DoWork() → ProcessInput 检查 ST_NEW → 消费
 * ================================================================= */

/* ==================== data_switcher.c ============================ */
#include "core/std_module.h"
#include "data_switcher.h"
#include "../include/adc_io.h"        /* 特权: 全路径 include 所有 _io.h */
#include "../include/power_io.h"

/* 模块 GetIO 声明 (由各模块的 MODULE_EXPORT 生成) */
void Adc_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
void Power_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));

#define MAX_MODULES  16

typedef struct {
    void (*pDoWork)(void);
} ModuleSlot_t;

static ModuleSlot_t s_slots[MAX_MODULES];
static uint8_t      s_count = 0;

void Switcher_Register(void (*pDoWork)(void))
{
    if (s_count < MAX_MODULES)
        s_slots[s_count++].pDoWork = pDoWork;
}

/* === __weak 路由背板 ============================================
 * Switcher 声明跨模块 __weak stub — linker 自动接线.
 * Producer 的 MODULE_EXPORT 生成 __weak {Producer}_OnOutput.
 * Consumer 提供 STRONG {Producer}_OnOutput(pOut) → memcpy.
 * ================================================================ */
__attribute__((weak)) void Adc_OnOutput(Para_Grp_t *pOut)
{ (void)pOut; }

void Switcher_Init(void)
{
    Para_Grp_t *pIn, *pOut;
    void       (*pWork)(void);

    Adc_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork);

    Power_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork);
}

void Switcher_Run(void)
{
    for (uint8_t i = 0; i < s_count; i++)
        if (s_slots[i].pDoWork)
            s_slots[i].pDoWork();
}


/* ===================================================================
 * §4 调度层
 *
 * Task 只通过 __weak 调 Switcher, 不知道任何模块的存在.
 * ================================================================= */

/* ==================== app_task.c ================================== */

__attribute__((weak)) void Switcher_Run_Slot2(void) {}

void Task_TimeChip2(void)
{
    Switcher_Run_Slot2();   /* __weak — linker 接线 */
}

/* ==================== data_switcher.c (续) ======================= */

/* STRONG — 覆盖 task 的 __weak */
void Switcher_Run_Slot2(void)
{
    Switcher_Run();
}


/* ===================================================================
 * §5 完整时序
 *
 *   帧 N (ADC 100ms 到期):
 *     Adc_DoWork:
 *       ProcessInput: 写 s_out.voltage=12000, s_out.current=2500
 *       → g_output.info.status |= ST_OUT
 *       DoWork 检测 ST_OUT → 调 _onOutput(&g_output)
 *       → __weak Adc_OnOutput → linker 解析到 Power 的 STRONG
 *     Power: Adc_OnOutput(pOut):
 *       memcpy(g_input.para, pOut->para, sizeof(PowerInData_t))
 *       → g_input.info.status |= ST_NEW
 *     Power_DoWork:
 *       ProcessInput: 检测 ST_NEW → 消费 → power=30000mW
 *       → g_output.info.status |= ST_OUT
 *
 *   帧 N+1..N+9 (ADC 无产出):
 *     Adc_DoWork: ProcessInput → ST_OUT 不置位 → _onOutput 不调
 *     Power_DoWork: ProcessInput → ST_NEW=0 → return
 *
 * 时序保证: producer 和 consumer 在同一帧内执行, 数据当场到达.
 * ================================================================= */


/* ===================================================================
 * §6 与旧模式的对比
 *
 * __weak 模式 (v1.0):
 *   - consumer 需 _LINK_t 副本 (双向维护)
 *   - void* 解耦
 *
 * 数据交换机 v2.0 (已废弃):
 *   - Switcher 搬运 Output_t 字段 → Input_t 字段
 *   - Switcher 管理所有 status 字节
 *   - 模块手动写 GetIO / DoWork / Constructor
 *
 * 数据交换机 v2.1 (本文件):
 *   - MODULE_SKELETON() + MODULE_EXPORT() 消除样板
 *   - Para_Grp_t 统一参数组
 *   - __weak 回调链自动路由 — consumer 自己 memcpy
 *   - Switcher 只注册 DoWork + 顺序调用
 *   - consumer 不需要 _LINK_t 副本
 * ================================================================= */


int main(void)
{
    Switcher_Init();

    while (1) {
        Task_TimeChip2();
    }
    return 0;
}
