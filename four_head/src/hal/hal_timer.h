/**
 * hal_timer.h —— TIM0 1ms时基接口
 *
 * 依赖: 无（仅 stdint.h）
 * 层级: HAL —— 纯硬件操作
 *
 * ISR只做: 清标志 + tick++ + 置1ms标志
 * 所有消息调度/模块处理在主循环中完成
 */
#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include <stdint.h>

void HAL_Timer_Init(void);
uint32_t HAL_Timer_GetTick(void);
uint8_t HAL_Timer_1msElapsed(void);   /* 查询并清除1ms标志 */

#endif /* HAL_TIMER_H */
