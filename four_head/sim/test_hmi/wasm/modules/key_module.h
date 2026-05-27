/* key_module.h — 按键映射模块
 *
 * 拥有: 无持久状态 (纯翻译模块)
 * 输入: Key_Inject (由 main.c 调用)
 * 输出(__weak):  HmiState_OnKey → state_module
 *
 * 职责:
 *   1. 物理按键码 → 语义索引映射
 *   2. 按键事件透明转发到 state_module
 */
#ifndef KEY_MODULE_H
#define KEY_MODULE_H

#include <stdint.h>
#include "modules/weak_macro.h"

void Key_Init(void);
void Key_Inject(uint8_t key, uint8_t evt);  /* 主入口 */

/* __weak 输出 → state_module
 * param 打包: 低8位=key, 高8位=evt (兼容原 on_key_event 格式) */
WEAK void HmiState_OnKey(uint16_t param, uint16_t unused, void *data_ptr);

/* 公用映射函数 */
int8_t Key_HeadKeyToIndex(uint8_t key);
uint8_t Key_HeadIndexToKey(uint8_t idx);
int8_t Key_DigitKeyToLevel(uint8_t key);
uint8_t Key_IsHeadKey(uint8_t key);

#endif /* KEY_MODULE_H */
