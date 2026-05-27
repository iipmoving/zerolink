/* key_module.c — 按键映射模块实现
 *
 * 职责:
 *   1. 接收 main.c 注入的按键事件
 *   2. 通过 __weak 回调 HmiState_OnKey 转发到 state_module
 *   3. 提供物理按键码 ↔ 语义索引的转换函数
 *
 * 映射关系 (来自 drv_key.h 的 KeyCode_t):
 *   KEY_LEFT_P_SET_UP  (15) → Head 0 (Z1)
 *   KEY_RIGHT_P_SET_UP (14) → Head 1 (Z2)
 *   KEY_LEFT_P_SET     (13) → Head 2 (Z3)
 *   KEY_RIGHT_P_SET    (12) → Head 3 (Z4)
 *   KEY_POWER_0~9 (22~31) → level 0~9
 */
#include "modules/key_module.h"
#include "drv/drv_key.h"
#include <stddef.h>

/* ================================================================
 * __weak stub: overridden by state_module.c (强符号)
 * ================================================================ */
__attribute__((weak)) void HmiState_OnKey(uint16_t param, uint16_t unused, void *data_ptr)
{
    (void)param;
    (void)unused;
    (void)data_ptr;
}

/* ================================================================
 * Public API
 * ================================================================ */

void Key_Init(void)
{
    /* 无状态初始化 */
}

void Key_Inject(uint8_t key, uint8_t evt)
{
    uint16_t param = ((uint16_t)evt << 8) | key;
    HmiState_OnKey(param, 0, NULL);
}

/* ================================================================
 * Mapping functions
 * ================================================================ */

/* Key_HeadKeyToIndex: 炉头选择键 → 炉头索引 (0-3), -1=非炉头键 */
int8_t Key_HeadKeyToIndex(uint8_t key)
{
    switch (key) {
    case KEY_LEFT_P_SET_UP:   return 0;
    case KEY_RIGHT_P_SET_UP:  return 1;
    case KEY_LEFT_P_SET:      return 2;
    case KEY_RIGHT_P_SET:     return 3;
    default:                  return -1;
    }
}

/* Key_HeadIndexToKey: 炉头索引 (0-3) → 物理按键码 */
uint8_t Key_HeadIndexToKey(uint8_t idx)
{
    switch (idx) {
    case 0: return KEY_LEFT_P_SET_UP;
    case 1: return KEY_RIGHT_P_SET_UP;
    case 2: return KEY_LEFT_P_SET;
    case 3: return KEY_RIGHT_P_SET;
    default: return KEY_NONE;
    }
}

/* Key_DigitKeyToLevel: 数字键 (KEY_POWER_0~9) → 档位 (0-9), -1=非法 */
int8_t Key_DigitKeyToLevel(uint8_t key)
{
    if (key >= KEY_POWER_0 && key <= KEY_POWER_9) {
        return (int8_t)(key - KEY_POWER_0);
    }
    return -1;
}

/* Key_IsHeadKey: 判断是否为炉头选择键 */
uint8_t Key_IsHeadKey(uint8_t key)
{
    return (key == KEY_LEFT_P_SET || key == KEY_RIGHT_P_SET
         || key == KEY_LEFT_P_SET_UP || key == KEY_RIGHT_P_SET_UP) ? 1u : 0u;
}
