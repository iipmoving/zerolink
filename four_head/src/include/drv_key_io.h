/**
 * drv_key_io.h —— DrvKey 输出接口定义 (v2.3)
 *
 * 数据流: DrvKey → AppHmi / AppCooking / AppSegAlign
 *
 * DrvKey 只有 OUTPUT (按键事件)，没有 INPUT
 *
 * Include 权限:
 *   - 仅 drv_key.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/drv_key_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef DRV_KEY_IO_H
#define DRV_KEY_IO_H

#include "../core/std_module.h"
#include <stdint.h>

/* ========== OUTPUT (DrvKey 给别的模块提供的数据) ========== */

/* DrvKey_to_AppHmi_Params — 输出数据参数 (v2.3 完整模块名) */
typedef struct {
    uint8_t  key_code;      /* KeyCode_t: 按键码 */
    uint8_t  key_state;     /* KEY_STATE_*: 按键状态 */
    uint8_t  head_index;    /* 炉头索引 */
    uint8_t  res[1];
} MODULE_OUTPUT_PARAMS(DrvKey, AppHmi);

/* DrvKey_to_AppHmi_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大炉头数 = 4 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(DrvKey, AppHmi) *params;  /* 指向自己的输出数据 */
} MODULE_OUTPUT_LINK(DrvKey, AppHmi);

/* DrvKey_to_AppCooking_Params */
typedef struct {
    uint8_t  key_code;
    uint8_t  key_state;
    uint8_t  head_index;
    uint8_t  res[1];
} MODULE_OUTPUT_PARAMS(DrvKey, AppCooking);

/* DrvKey_to_AppCooking_Output_Link */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(DrvKey, AppCooking) *params;
} MODULE_OUTPUT_LINK(DrvKey, AppCooking);

/* DrvKey_to_AppSegAlign_Params */
typedef struct {
    uint8_t  key_code;
    uint8_t  key_state;
    uint8_t  head_index;
    uint8_t  res[1];
} MODULE_OUTPUT_PARAMS(DrvKey, AppSegAlign);

/* DrvKey_to_AppSegAlign_Output_Link */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(DrvKey, AppSegAlign) *params;
} MODULE_OUTPUT_LINK(DrvKey, AppSegAlign);

/* DrvKey_Output — 输出聚合 (v2.3 对称命名: 成员名 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(DrvKey, AppHmi)       *AppHmi_params;       /* → AppHmi */
    MODULE_OUTPUT_LINK(DrvKey, AppCooking)   *AppCooking_params;   /* → AppCooking */
    MODULE_OUTPUT_LINK(DrvKey, AppSegAlign)  *AppSegAlign_params;  /* → AppSegAlign */
} MODULE_OUTPUT(DrvKey);

/* ========== INPUT (无) ========== */
/* DrvKey 是底层驱动，没有输入 */

/* ========== Consumer INPUT_LINK 定义在 Consumer 的 io.h ========== */
/* AppHmi_INPUT_LINK(DrvKey, AppHmi) 定义在 app_hmi_io.h */
/* AppCooking_INPUT_LINK(DrvKey, AppCooking) 定义在 app_cooking_io.h */
/* AppSegAlign_INPUT_LINK(DrvKey, AppSegAlign) 定义在 app_seg_align_io.h */

#endif /* DRV_KEY_IO_H */