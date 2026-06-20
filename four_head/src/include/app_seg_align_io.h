/**
 * @file    app_seg_align_io.h
 * @brief   AppSegAlign Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * 段码对齐处理 — 从 Key 输入，无输出
 *
 * 输入源:
 *   DrvKey → AppSegAlign  (按键事件 → SegAlign 调校模式)
 */

#ifndef APPSEGALIGN_IO_H
#define APPSEGALIGN_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ========== OUTPUT (无) — 本模块没有输出 ========== */

/* AppSegAlign_Output — 无输出 */
typedef struct {
    uint8_t  res[4];
} MODULE_OUTPUT(AppSegAlign);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * DrvKey → AppSegAlign  输入参数  (按键事件 → SegAlign 调校模式)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t key_code;     /* 按键码 KeyCode_t */
    uint8_t key_state;     /* 按键状态 KEY_STATE_* */
    uint8_t head_index;     /* 关联炉头索引 */
} MODULE_INPUT_PARAMS(DrvKey, AppSegAlign);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(DrvKey, AppSegAlign) *params;
} MODULE_INPUT_LINK(DrvKey, AppSegAlign);

/* AppSegAlign_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(DrvKey, AppSegAlign) *DrvKey_params;  /* 指向 DrvKey 输出的 LINK 列 */
} MODULE_INPUT(AppSegAlign);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(AppSegAlign);

#endif /* APPSEGALIGN_IO_H */
