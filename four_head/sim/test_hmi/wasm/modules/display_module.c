/* display_module.c — 显示派生模块实现
 *
 * 职责:
 *   1. 接收 state_module 的 OnStateChange 通知, 缓存 node (仅 blinking 判定需要)
 *   2. 在 tick 相位更新 seg_blink
 *   3. 通过 DrvDisplay_OnRefresh __weak 回调输出给 DRV 层
 *
 * 注: seg_chars / seg_mode / LED 的完整派生由 state_module 拥有.
 *     WASM 查询全部走 State_Get*, 本模块不导出 Display_Get*.
 *     本模块仅负责 blink 相位 (display_module 独有职责) + DRV 刷新回调.
 */
#include "modules/display_module.h"
#include "cfg/hmi_data.h"
#include <stddef.h>

/* ================================================================
 * Module-static state
 * ================================================================ */
static uint32_t s_display_tick;
static uint8_t  s_head_nodes[4];   /* 仅缓存 node 字段, 用于 blink 判定 */
static HmiDisplayCache_t s_display; /* 仅 seg_blink[4] 被填充 */

/* ================================================================
 * Strong implementation: state_module __weak → display_module strong
 * ================================================================ */
void Display_OnStateChange(uint8_t zone, const HmiHead_t *head,
    const HmiGlobalState_t *global)
{
    (void)global;
    if (zone < 4u && head != NULL) {
        s_head_nodes[zone] = head->node;
    }
}

/* ================================================================
 * __weak stub: overridden by main.c (DRV 桩 in WASM 环境)
 * ================================================================ */
__attribute__((weak)) void DrvDisplay_OnRefresh(const HmiDisplayCache_t *cache)
{
    (void)cache;
}

/* ================================================================
 * Public API
 * ================================================================ */

void Display_Init(void)
{
    uint8_t i;
    s_display_tick = 0u;
    for (i = 0u; i < 4u; i++) {
        s_head_nodes[i]       = (uint8_t)HMI_ZONE_IDLE;
        s_display.seg_blink[i] = 0u;
    }
}

void Display_OnTickPhase(uint32_t tick)
{
    uint8_t i;

    s_display_tick = tick;

    /* 500ms 周期闪烁 (tick=100ms, /5 = 每500ms翻转) */
    for (i = 0u; i < 4u; i++) {
        if (s_head_nodes[i] == (uint8_t)HMI_ZONE_SELECTING) {
            s_display.seg_blink[i] = ((tick / 5u) & 1u) ? 1u : 0u;
        } else {
            s_display.seg_blink[i] = 0u;
        }
    }

    DrvDisplay_OnRefresh(&s_display);
}
