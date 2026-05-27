/* state_module.h — HMI 状态机核心模块
 *
 * 拥有: s_heads[4], s_global, s_hot_head, s_select_stack
 * 输入(强符号): HmiState_OnKey, HmiState_OnSelectTimeout,
 *                HmiState_OnTimerExpired, HmiState_OnBoostTimeout
 * 输出(__weak):  Display_OnStateChange, Timer_OnStateChange
 */
#ifndef STATE_MODULE_H
#define STATE_MODULE_H

#include "app/app_hmi.h"
#include "modules/weak_macro.h"

/* === 公用入口 === */
void State_Init(void);
void State_OnTick(void);  /* 每100ms调用, 驱动上电序列等 */

/* === __weak 输出 (由其他模块强实现) === */
WEAK void Display_OnStateChange(uint8_t zone, const HmiHead_t *head,
    const HmiGlobalState_t *global);
WEAK void Timer_OnStateChange(uint8_t zone, const HmiHead_t *head);

/* === 公用查询 (WASM engine_get_* 委托) === */
uint8_t State_GetGlobalMode(void);
uint8_t State_IsChildLock(void);
uint8_t State_IsPaused(void);
int8_t  State_GetHotHead(void);
uint8_t State_GetStackDepth(void);
int8_t  State_GetStackAt(uint8_t pos);
uint8_t State_GetZoneNode(uint8_t idx);
uint8_t State_GetZonePower(uint8_t idx);
uint8_t State_GetZoneBoost(uint8_t idx);
uint8_t State_GetZoneTimerSetting(uint8_t idx);
uint8_t State_GetZoneTimerActive(uint8_t idx);
uint16_t State_GetZoneTimerValue(uint8_t idx);

/* === 公用查询: 显示缓存 (WASM engine_get_* 委托) === */
uint8_t State_GetSegChar(uint8_t pos);
uint8_t State_GetSegBlink(uint8_t zone);
uint8_t State_GetSegMode(void);
uint8_t State_GetLedPower(void);
uint8_t State_GetLedTimer(void);
uint8_t State_GetLedPause(void);
uint8_t State_GetLedChildLock(void);
uint8_t State_GetLedHeadSelect(uint8_t idx);
uint8_t State_GetLedPowerLevel(uint8_t idx);

/* === 强制操作 (测试用) === */
void State_ForceSelectConfirm(uint8_t idx);
void State_ForceBoostExit(uint8_t idx);
void State_ForceTimerExpire(uint8_t idx);

#endif
