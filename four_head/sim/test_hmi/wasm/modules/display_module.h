/* display_module.h — 显示派生模块
 *
 * 拥有: s_head_nodes[4] (仅 blinking 判定需要的 node 字段)
 * 输入(强符号): Display_OnStateChange (由 state_module __weak 调用)
 * 输出(__weak):  DrvDisplay_OnRefresh → main.c DRV 桩
 * 输入(显示引擎): Display_OnTickPhase (由 main.c tick 循环调用)
 *
 * 注: seg_chars / seg_mode / LED 的完整派生由 state_module 拥有,
 *     WASM engine_get_* 显示查询全部委托 State_Get*.
 *     本模块仅负责 blink 相位 + DrvDisplay_OnRefresh 回调.
 */
#ifndef DISPLAY_MODULE_H
#define DISPLAY_MODULE_H

#include "app/app_hmi.h"
#include "modules/weak_macro.h"

void Display_Init(void);
void Display_OnTickPhase(uint32_t tick);  /* 更新 blink 相位并触发刷新 */

/* __weak 输出 → main.c DRV 桩 */
WEAK void DrvDisplay_OnRefresh(const HmiDisplayCache_t *cache);

#endif /* DISPLAY_MODULE_H */
