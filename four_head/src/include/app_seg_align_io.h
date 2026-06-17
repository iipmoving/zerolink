/**
 * app_seg_align_io.h —— AppSegAlign 输入接口定义 (v2.3)
 *
 * 数据流:
 *   INPUT: DrvKey → AppSegAlign (按键事件)
 *
 * AppSegAlign 只有 INPUT，没有 OUTPUT
 *
 * Include 权限:
 *   - 仅 app_seg_align.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/app_seg_align_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef APP_SEG_ALIGN_IO_H
#define APP_SEG_ALIGN_IO_H

#include "../core/std_module.h"
#include <stdint.h>

/* ========== OUTPUT (无) ========== */
/* AppSegAlign 没有输出 */

/* ========== INPUT (AppSegAlign 从别的模块得到的数据) ========== */

/* Key_to_SegAlign_Params — 输入数据参数 (布局与 DrvKey OUTPUT 一致) */
typedef struct {
    uint8_t  key_code;        /* KeyCode_t: 按键码 */
    uint8_t  key_state;       /* KEY_STATE_*: 按键状态 */
    uint8_t  head_index;      /* 炉头索引 */
    uint8_t  res[1];
} MODULE_INPUT_PARAMS(Key, SegAlign);

/* Key_to_SegAlign_Input_Link — 输入管道 (与 DrvKey OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(Key, SegAlign) *params;
} MODULE_INPUT_LINK(Key, SegAlign);

/* AppSegAlign_Input — 输入聚合 */
typedef struct {
    MODULE_INPUT_LINK(Key, SegAlign) *key;  /* 从 DrvKey 得到 */
} MODULE_INPUT(AppSegAlign);

#endif /* APP_SEG_ALIGN_IO_H */