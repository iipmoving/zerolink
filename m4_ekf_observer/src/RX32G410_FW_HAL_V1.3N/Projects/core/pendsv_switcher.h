/**
 * @file    pendsv_switcher.h
 * @brief   PendSV Switcher — 实时通路调度器 (v2.3)
 * @layer   core
 *
 * PendSV 通路用于紧急 ISR 事件的实时处理。
 * ISR 只调用 Switcher_TriggerPendSV() 触发 PendSV，
 * 在 PendSV_Handler 中遍历所有已注册 ISR 模块顺序处理。
 *
 * 设计要点:
 *   - PendSV 优先级最低，在所有 ISR 完成后执行
 *   - 无软件锁，利用 PendSV 硬件不嵌套特性
 *   - 无运行时字符串查找，纯函数指针数组
 *   - 每个 ISR 模块内部通过 g_isr_source 决定是否处理
 */

#ifndef PENSV_SWITCHER_H
#define PENSV_SWITCHER_H

#include <stdint.h>

/* 最大 ISR 模块数 */
#define PENSV_MAX_MODULES  16

/* ========== ISR 来源标识 ========== */
#define ISR_SOURCE_ADC     0x01
#define ISR_SOURCE_TIMER   0x02
#define ISR_SOURCE_FAULT   0x03

/* ========== PendSV 状态变量 ========== */
extern volatile uint8_t g_isr_pending;   /* PendSV 请求标志 */
extern volatile uint8_t g_isr_source;    /* ISR 来源 (最后一次触发) */

/* ========== API ========== */

/** @brief 初始化 PendSV 通路 */
void Switcher_PendSV_Init(void);

/** @brief 注册 ISR 模块 (使用 GetISR_IO 获取 pISR_DoWork) */
void Switcher_RegisterISRModule(void (*pISR_DoWork)(void));

/** @brief 从 ISR 中触发 PendSV (设置标志 + 触发 PendSV 异常) */
void Switcher_TriggerPendSV(uint8_t source);

/** @brief PendSV Handler — 在启动文件中声明为 PendSV 向量入口 */
void PendSV_Handler(void);

#endif /* PENSV_SWITCHER_H */
