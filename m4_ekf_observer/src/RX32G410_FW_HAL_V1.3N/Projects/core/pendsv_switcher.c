/**
 * @file    pendsv_switcher.c
 * @brief   PendSV Switcher — 实时通路调度器 (v2.3)
 * @layer   core
 *
 * 所有 ISR 模块通过统一的 Switcher_RegisterISRModule() 注册。
 * 注册时只需提供 ISR_DoWork 函数指针 (从 GetISR_IO 获取)。
 * PendSV_Handler 遍历所有已注册模块，按序调用 ISR_DoWork。
 *
 * 数据流:
 *   ISR_Handler() → Switcher_TriggerPendSV(source)
 *   ↓
 *   PendSV_Handler() → 遍历 s_isr_workers[] → 依次调用 ISR_DoWork
 *   ↓
 *   各 ISR_DoWork 内: ISR_InputCallback → ISR_ProcessInput → ISR_OutputCallback
 *
 * PendSV 特性:
 *   - 优先级最低，在所有 ISR 完成后执行
 *   - 同级中断不嵌套，无软件锁需求
 *   - 使用 SCB->ICSR.PENDSVSET (bit28) 触发
 */

#include "pendsv_switcher.h"
#include <string.h>

/* ========== PendSV 状态变量 ========== */
volatile uint8_t g_isr_pending = 0;
volatile uint8_t g_isr_source  = 0;

/* ========== ISR 模块注册表 ========== */
static void (*s_isr_workers[PENSV_MAX_MODULES])(void);
static uint8_t  s_isr_count = 0;

/* ========== PendSV 触发: SCB->ICSR bit28 ========== */
#define SCB_ICSR_ADDR      0xE000ED04
#define SCB_ICSR_PENDSVSET (1UL << 28)

/* ================================================================
 * Switcher_PendSV_Init — 初始化 PendSV 通路
 * ================================================================ */
void Switcher_PendSV_Init(void)
{
    memset(s_isr_workers, 0, sizeof(s_isr_workers));
    s_isr_count   = 0;
    g_isr_pending = 0;
    g_isr_source  = 0;
}

/* ================================================================
 * Switcher_RegisterISRModule — 注册 ISR 模块
 *
 *   pISR_DoWork: 从模块 GetISR_IO 获取的 ISR_DoWork 函数指针
 *   注意: 无需传 pIn/pOut，ISR_DoWork 内部使用模块静态变量
 * ================================================================ */
void Switcher_RegisterISRModule(void (*pISR_DoWork)(void))
{
    if (s_isr_count >= PENSV_MAX_MODULES) {
        return;
    }
    if (pISR_DoWork == NULL) {
        return;
    }
    s_isr_workers[s_isr_count++] = pISR_DoWork;
}

/* ================================================================
 * Switcher_TriggerPendSV — ISR 中触发 PendSV
 *
 *   1. 保存 ISR 来源
 *   2. 置 g_isr_pending 标志
 *   3. 写 SCB->ICSR.PENDSVSET 触发 PendSV 异常
 *
 *   在 ISR 中调用此函数: PendSV 会在当前 ISR 退出后立即执行
 *   多次触发: PendSV 在 pending 状态累积，退出前只执行一次
 * ================================================================ */
void Switcher_TriggerPendSV(uint8_t source)
{
    g_isr_source  = source;
    g_isr_pending = 1;

    /* 触发 PendSV: SCB->ICSR |= PENDSVSET (bit28) */
    *(volatile uint32_t *)SCB_ICSR_ADDR |= SCB_ICSR_PENDSVSET;
}

/* ================================================================
 * PendSV_Handler — PendSV 中断服务
 *
 *   1. 检查 g_isr_pending (无请求直接退出)
 *   2. 清 pending 标志
 *   3. 遍历 s_isr_workers 调用所有已注册 ISR 模块
 *   4. PendSV 优先级最低，不会打断其他中断
 *
 *   注意: 如果在处理过程中又有新 PendSV 请求，
 *         g_isr_pending 会在下次进入时再次处理
 * ================================================================ */
void PendSV_Handler(void)
{
    if (!g_isr_pending) {
        return;
    }

    g_isr_pending = 0;

    for (uint8_t i = 0; i < s_isr_count; i++) {
        if (s_isr_workers[i]) {
            s_isr_workers[i]();
        }
    }
}
