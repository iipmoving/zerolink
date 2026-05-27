/* main.c — WASM 测试引擎接线层 (v2.0 __weak 零依赖架构)
 *
 * 职责:
 *   1. 按序初始化 4 个独立模块
 *   2. 实现 engine_* 导出函数 (WASM ↔ JS 接口)
 *   3. 提供 DRV 桩 (WASM 环境无硬件)
 *   4. 驱动 tick → 各模块分发
 *
 * 模块接线 (__weak 链接器自动解析):
 *   main ──Key_Inject──→ key_module ──HmiState_OnKey──→ state_module
 *   main ──Timer_OnTick──→ timer_module ──HmiState_On*Timeout──→ state_module
 *   main ──Display_OnTickPhase──→ display_module ──DrvDisplay_OnRefresh──→ main(DRV桩)
 *   state_module ──Display_OnStateChange──→ display_module
 *   state_module ──Timer_OnStateChange──→ timer_module
 */

#include "app/app_hmi.h"
#include "modules/key_module.h"
#include "modules/state_module.h"
#include "modules/timer_module.h"
#include "modules/display_module.h"
#include <stddef.h>

/* ===== DRV 桩 (WASM 环境无硬件) ===== */
void Drv_Display_Commit(const void *frame)  { (void)frame; }
void Drv_Display_SetRawLEDs(uint8_t io8, uint8_t io9, uint8_t io10)
{ (void)io8; (void)io9; (void)io10; }
void Drv_Display_ShowRawSMG(const char *upper, const char *lower)
{ (void)upper; (void)lower; }

/* 强实现 display_module 的 __weak 输出 */
void DrvDisplay_OnRefresh(const HmiDisplayCache_t *cache)
{
    (void)cache;
    /* WASM 环境: 无需真实刷新, 查询函数直接读 display_module 缓存 */
}

/* ===== WASM 导出: 引擎接口 ===== */

void engine_init(void)
{
    State_Init();
    Timer_Init();
    Display_Init();
    Key_Init();
}

void engine_post_key(uint8_t key, uint8_t evt)
{
    Key_Inject(key, evt);
}

void engine_tick_100ms(void)
{
    Timer_OnTick();
    State_OnTick();
    Display_OnTickPhase(Timer_GetTick());
}

void engine_tick_1s(void)
{
    Timer_OnSecondTick();
}

/* ===== WASM 导出: 状态查询 (委托 state_module) ===== */

uint8_t engine_get_global_mode(void)   { return State_GetGlobalMode(); }
uint8_t engine_is_child_lock(void)     { return State_IsChildLock(); }
uint8_t engine_is_paused(void)         { return State_IsPaused(); }
int8_t  engine_get_hot_head(void)      { return State_GetHotHead(); }
uint8_t engine_get_stack_depth(void)   { return State_GetStackDepth(); }

int8_t engine_get_stack_at(uint8_t pos)
{
    return State_GetStackAt(pos);
}

uint8_t engine_get_zone_node(uint8_t idx)
{
    return State_GetZoneNode(idx);
}

uint8_t engine_get_zone_power(uint8_t idx)
{
    return State_GetZonePower(idx);
}

uint8_t engine_get_zone_boost(uint8_t idx)
{
    return State_GetZoneBoost(idx);
}

uint8_t engine_get_zone_timer_setting(uint8_t idx)
{
    return State_GetZoneTimerSetting(idx);
}

uint8_t engine_get_zone_timer_active(uint8_t idx)
{
    return State_GetZoneTimerActive(idx);
}

uint16_t engine_get_zone_timer_value(uint8_t idx)
{
    return State_GetZoneTimerValue(idx);
}

/* ===== WASM 导出: 显示查询 (委托 state_module, 原装显示逻辑) ===== */

uint8_t engine_get_seg_char(uint8_t pos)
{
    return State_GetSegChar(pos);
}

uint8_t engine_get_seg_blink(uint8_t zone)
{
    return State_GetSegBlink(zone);
}

uint8_t engine_get_seg_mode(void)
{
    return State_GetSegMode();
}

uint8_t engine_get_led_power(void)
{
    return State_GetLedPower();
}

uint8_t engine_get_led_timer(void)
{
    return State_GetLedTimer();
}

uint8_t engine_get_led_pause(void)
{
    return State_GetLedPause();
}

uint8_t engine_get_led_child_lock(void)
{
    return State_GetLedChildLock();
}

uint8_t engine_get_led_head_select(uint8_t idx)
{
    return State_GetLedHeadSelect(idx);
}

uint8_t engine_get_led_power_level(uint8_t idx)
{
    return State_GetLedPowerLevel(idx);
}

/* ===== WASM 导出: 强制操作 ===== */

void engine_force_select_confirm(uint8_t idx)
{
    State_ForceSelectConfirm(idx);
}

void engine_force_boost_exit(uint8_t idx)
{
    State_ForceBoostExit(idx);
}

void engine_force_timer_expire(uint8_t idx)
{
    State_ForceTimerExpire(idx);
}
