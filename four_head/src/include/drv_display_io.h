/**
 * drv_display_io.h —— DrvDisplay 输入接口定义 (v2.3)
 *
 * 数据流:
 *   INPUT: AppHmi → DrvDisplay (显示数据)
 *
 * DrvDisplay 只有 INPUT，没有 OUTPUT
 *
 * Include 权限:
 *   - 仅 drv_display.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/drv_display_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef DRV_DISPLAY_IO_H
#define DRV_DISPLAY_IO_H

#include "../core/std_module.h"
#include <stdint.h>

/* ========== OUTPUT (无) ========== */
/* DrvDisplay 是底层驱动，没有输出 */

/* ========== INPUT (DrvDisplay 从别的模块得到的数据) ========== */

/* Hmi_to_Display_Params — 输入数据参数 (布局与 AppHmi OUTPUT 一致) */
typedef struct {
    uint8_t  head_index;      /* 炉头索引 0-3 */
    char     seg_chars[8];    /* 段码显示字符 */
    uint8_t  seg_mode;        /* SEG_MODE_*: 显示模式 */
    uint8_t  led_bits;        /* LED 位图 */
    uint8_t  res[1];
} MODULE_INPUT_PARAMS(Hmi, Display);

/* Hmi_to_Display_Input_Link — 输入管道 (与 AppHmi OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(Hmi, Display) *params;
} MODULE_INPUT_LINK(Hmi, Display);

/* DrvDisplay_Input — 输入聚合 */
typedef struct {
    MODULE_INPUT_LINK(Hmi, Display) *hmi;  /* 从 AppHmi 得到 */
} MODULE_INPUT(DrvDisplay);

#endif /* DRV_DISPLAY_IO_H */