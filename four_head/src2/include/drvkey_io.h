/**
 * @file    drvkey_io.h
 * @brief   DrvKey Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   drv
 *
 * 按键扫描与事件分发 — 输出按键事件到 Hmi/Cooking/SegAlign
 *
 * 输出目标:
 *   DrvKey → AppHmi  (按键事件 → HMI)
 *   DrvKey → AppCooking  (按键事件 → Cooking)
 *   DrvKey → AppSegAlign  (按键事件 → SegAlign)
 */

#ifndef DRVKEY_IO_H
#define DRVKEY_IO_H

#include <stdint.h>
#include "../core/std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * DrvKey → AppHmi  输出参数  (按键事件 → HMI)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t key_code;     /* 按键码 KeyCode_t */
    uint8_t key_state;     /* 按键状态 KEY_STATE_* */
    uint8_t head_index;     /* 关联炉头索引 */
} MODULE_OUTPUT_PARAMS(DrvKey, AppHmi);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(DrvKey, AppHmi) *params;
} MODULE_OUTPUT_LINK(DrvKey, AppHmi);

/* ------------------------------------------------------------------
 * DrvKey → AppCooking  输出参数  (按键事件 → Cooking)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t key_code;     /* 按键码 KeyCode_t */
    uint8_t key_state;     /* 按键状态 KEY_STATE_* */
    uint8_t head_index;     /* 关联炉头索引 */
} MODULE_OUTPUT_PARAMS(DrvKey, AppCooking);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(DrvKey, AppCooking) *params;
} MODULE_OUTPUT_LINK(DrvKey, AppCooking);

/* ------------------------------------------------------------------
 * DrvKey → AppSegAlign  输出参数  (按键事件 → SegAlign)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t key_code;     /* 按键码 KeyCode_t */
    uint8_t key_state;     /* 按键状态 KEY_STATE_* */
    uint8_t head_index;     /* 关联炉头索引 */
} MODULE_OUTPUT_PARAMS(DrvKey, AppSegAlign);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(DrvKey, AppSegAlign) *params;
} MODULE_OUTPUT_LINK(DrvKey, AppSegAlign);

/* DrvKey_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(DrvKey, AppHmi)  AppHmi_params;  /* → AppHmi */
    MODULE_OUTPUT_LINK(DrvKey, AppCooking)  AppCooking_params;  /* → AppCooking */
    MODULE_OUTPUT_LINK(DrvKey, AppSegAlign)  AppSegAlign_params;  /* → AppSegAlign */
} MODULE_OUTPUT(DrvKey);

/* ========== INPUT (无) — 本模块没有输入 ========== */

/* DrvKey_Input — 无输入 */
typedef struct {
    uint8_t  res[4];
} MODULE_INPUT(DrvKey);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(DrvKey);

#endif /* DRVKEY_IO_H */
