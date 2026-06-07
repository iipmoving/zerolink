/**
 * @file    data_switcher.c
 * @brief   Data Switcher — 统一调度 + PULL 显式路由 (v2.2)
 * @layer   core
 *
 * **迁移状态**: v2.2 PULL 路由。模块通过 MODULE_EXPORT 注册 GetIO,
 * Switcher 在 DoWork 调用之间显式路由数据。
 *
 * 路由机制 (v2.2 PULL):
 *   Switcher_Run → Producer.DoWork → 检查 Producer 的 g_output
 *   → _route_xxx() 显式调用 Consumer 回调 → Consumer memcpy + ST_NEW
 *   → Consumer.DoWork 消费
 *
 * @OUTPUT_CALLBACK 例外:
 *   需要即时回调的模块 (如蜂鸣器) 设 ST_OUT 触发 _onOutput,
 *   须有 /* @OUTPUT_CALLBACK: <reason> — user confirmed */ 标记,
 *   并在 interface_map.h 白名单注册。
 */

#include "core/std_module.h"
#include "data_switcher.h"

/* 模块 GetIO 声明 (由 MODULE_EXPORT 生成) */
void Adc_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
void Power_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));

/* Consumer 回调声明 (PULL: Switcher 显式调用, 模块实现后取消注释)
void Power_OnAdcData(Para_Grp_t *pOut);
*/

#define MAX_MODULES  16

typedef struct {
    void       (*pDoWork)(void);
    Para_Grp_t *pOut;   /* 模块的 g_output 指针 */
} ModuleSlot_t;

static ModuleSlot_t s_slots[MAX_MODULES];
static uint8_t      s_count = 0;

void Switcher_Register(void (*pDoWork)(void), Para_Grp_t *pOut)
{
    if (s_count < MAX_MODULES) {
        s_slots[s_count].pDoWork = pDoWork;
        s_slots[s_count].pOut    = pOut;
        s_count++;
    }
}

/* === PULL 路由函数 ================================================
 * 每个 producer 一个 _route_xxx().
 * 检查 producer 的 g_output.para 是否有新数据 → 显式调 consumer 回调.
 * Producer 只写 g_output.para, Switcher 负责路由.
 * ==================================================================== */

static void _route_adc(Para_Grp_t *pOut)
{
    if (!pOut || !pOut->para) return;
    /* Adc 输出 → Power 消费 (Power 模块实现后取消注释)
    Power_OnAdcData(pOut);
    */
    (void)pOut;
}

/* === 初始化 ======================================================= */
void Switcher_Init(void)
{
    Para_Grp_t *pIn, *pOut;
    void       (*pWork)(void);

    Adc_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork, pOut);

    Power_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork, pOut);
}

/* === Slot1 调度入口 (约每 10ms) ================================== */
void Switcher_Run_Slot1(void)
{
    /* Producer DoWork → 显式路由 → 下一个 DoWork */
    for (uint8_t i = 0; i < s_count; i++) {
        if (!s_slots[i].pDoWork) continue;
        s_slots[i].pDoWork();
        /* 每个 producer DoWork 之后检查是否需要路由 */
        if (s_slots[i].pOut && s_slots[i].pOut->para) {
            /* TODO: 根据模块索引派发路由 */
            switch (i) {
            case 0: _route_adc(s_slots[i].pOut); break;
            /* case N: _route_xxx(s_slots[N].pOut); break; */
            default: break;
            }
        }
    }
}
