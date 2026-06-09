/**
 * @file    example_switcher.c
 * @brief   Switcher — 数据交换机中间层 (范例)
 *
 * ★ 全项目唯一有权全路径 include 所有 _io.h 的文件
 *
 * 数据流:
 *   Module A (Producer) ──→ Switcher 搬运 ──→ Module B (Consumer)
 *     Output_t.counter         Input_t.counter
 *     Output_t.mode            Input_t.mode
 *
 * 时序 (每帧):
 *   1. Switcher 检测 Producer 的 bit1 → 搬运字段 → 清 Producer bit1 → 设 Consumer bit1
 *   2. Switcher 调 Producer.DoWork()   → 清 bit1 → 计算 → 有产出则置 bit1
 *   3. Switcher 调 Consumer.DoWork()   → 检查 bit1 → 消费 → 清 bit1
 */

/* ★ Switcher.c 是全项目唯一有权 include _io.h 的文件 */
#include "../include/example_a_io.h"
#include "../include/example_b_io.h"
#include "example_switcher.h"

/* ====== IO 指针缓存（Switcher_Init 时绑定） ====== */
static ExampleA_Output_t *pA_Out = NULL;
static ExampleB_Input_t  *pB_In  = NULL;

/* ====== 初始化 ====== */

void ExampleSwitcher_Init(void)
{
    void *pDummy = NULL;

    /* 绑定 A 的输出槽指针 */
    ExampleA_GetIO(&pDummy, &pA_Out);

    /* 绑定 B 的输入槽指针 */
    ExampleB_GetIO(&pB_In, &pDummy);

    /* 清零所有输入槽 status → 模块首次 DoWork 自检 bit0=0 → 调 _Constructor() */
    pB_In->status = 0;
}

/* ====== 调试快照 ====== */
static SwitcherDebug_t s_dbg;

void ExampleSwitcher_GetDebug(SwitcherDebug_t *pDbg)
{
    if (pDbg) *pDbg = s_dbg;
}

/* ====== 运行时路由 ====== */

void ExampleSwitcher_Run(void)
{
    /* ====== 1. 数据路由：A → B ====== */
    if (pA_Out != NULL && (pA_Out->status & 0x02)) {
        /* A 有产出 → 搬运到 B 的输入槽 */
        pB_In->counter = pA_Out->counter;
        pB_In->mode    = pA_Out->mode;

        /* 清 A 的输出标志（数据已被取走） */
        pA_Out->status &= ~0x02;

        /* 设 B 的输入标志（新数据到达） */
        pB_In->status  |= 0x02;
    }

    /* ====== 2. 执行模块主逻辑 ====== */
    ExampleA_DoWork();       /* Producer — 产出本帧数据 */
    ExampleB_DoWork();       /* Consumer — 消费上帧路由来的数据（一帧延迟）*/

    /* ====== 3. 更新调试快照 ====== */
    s_dbg.a_counter  = (pA_Out ? pA_Out->counter : 0);
    s_dbg.a_status   = (pA_Out ? pA_Out->status  : 0);
    /* 注: Switcher 只报告 raw 路由值，不认知模块内部处理逻辑 */
    s_dbg.b_consumed = (pB_In  ? pB_In->counter : 0);  // 简化: 假设 B 已消费
}
