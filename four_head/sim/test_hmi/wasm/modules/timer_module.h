/* timer_module.h — 简易定时器模块
 *
 * 职责: 管理选中超时、定时设置自动确认、强火超时、倒计时
 *
 * 输入(weak): Timer_OnStateChange (state_module 通过 weak 回调传入)
 * 输出(weak): HmiState_OnSelectTimeout, HmiState_OnTimerExpired,
 *             HmiState_OnBoostTimeout
 *
 * 层级: CORE — 无业务依赖，纯计数 + 超时判定
 */
#ifndef TIMER_MODULE_H
#define TIMER_MODULE_H

#include "app/app_hmi.h"
#include "modules/weak_macro.h"

/* === 公用入口 (由 main.c 调用) === */
void Timer_Init(void);
void Timer_OnTick(void);        /* 每100ms */
void Timer_OnSecondTick(void);  /* 每1s */

/* === __weak 输出 (由 state_module 强实现) === */
WEAK void HmiState_OnSelectTimeout(uint8_t zone);
WEAK void HmiState_OnTimerExpired(uint8_t zone);
WEAK void HmiState_OnBoostTimeout(uint8_t zone);

/* === 公用查询 === */
uint32_t Timer_GetTick(void);   /* 供 main.c 读取当前tick传给display */

#endif /* TIMER_MODULE_H */
