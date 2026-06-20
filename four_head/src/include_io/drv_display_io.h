/**
 * @file    drv_display_io.h
 * @brief   DrvDisplay Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   drv
 *
 * 显示驱动 — 从 Hmi 接收显示数据，驱动段码/LED
 *
 * 输入源:
 *   AppHmi → DrvDisplay  (显示数据 → DrvDisplay (段码/LED))
 *   AppSegAlign → DrvDisplay  (段码控制 → DrvDisplay (替代 __weak DrvSegAlign_WriteCom/BlockHmi, route区分: 0=write 1=block))
 */

#ifndef DRVDISPLAY_IO_H
#define DRVDISPLAY_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ========== OUTPUT (无) — 本模块没有输出 ========== */

/* DrvDisplay_Output — 无输出 */
typedef struct {
    uint8_t  res[4];
} MODULE_OUTPUT(DrvDisplay);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppHmi → DrvDisplay  输入参数  (显示数据 → DrvDisplay (段码/LED))
 * ------------------------------------------------------------------ */
typedef struct {
    int8_t hot_head_idx;     /* 热点炉头索引 (-1=无) */
    uint8_t seg_chars[8];     /* 8位段码字符 */
    uint8_t seg_blink[4];     /* 炉头闪烁控制 */
    uint8_t seg_mode;     /* 显示模式 */
    uint8_t leds_power;     /* 电源 LED */
    uint8_t leds_timer;     /* 定时 LED */
    uint8_t leds_pause;     /* 暂停 LED */
    uint8_t leds_child_lock;     /* 童锁 LED */
    uint8_t leds_head_select[4];     /* 炉头选择 LED */
    uint8_t leds_power_level[10];     /* 功率档位 LED */
} MODULE_INPUT_PARAMS(AppHmi, DrvDisplay);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppHmi, DrvDisplay) *params;
} MODULE_INPUT_LINK(AppHmi, DrvDisplay);

/* ------------------------------------------------------------------
 * AppSegAlign → DrvDisplay  输入参数  (段码控制 → DrvDisplay (替代 __weak DrvSegAlign_WriteCom/BlockHmi, route区分: 0=write 1=block))
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t com_mask[11];     /* 11个COM位的段码掩码(route=0时有效) */
    uint8_t dirty;     /* 1=段码有更新需要刷新(route=0) */
    uint8_t block;     /* 1=阻塞HMI刷新 0=恢复(route=1) */
} MODULE_INPUT_PARAMS(AppSegAlign, DrvDisplay);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppSegAlign, DrvDisplay) *params;
} MODULE_INPUT_LINK(AppSegAlign, DrvDisplay);

/* DrvDisplay_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppHmi, DrvDisplay) *AppHmi_params;  /* 指向 AppHmi 输出的 LINK 列 */
    MODULE_INPUT_LINK(AppSegAlign, DrvDisplay) *AppSegAlign_params;  /* 指向 AppSegAlign 输出的 LINK 列 */
} MODULE_INPUT(DrvDisplay);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(DrvDisplay);

#endif /* DRVDISPLAY_IO_H */
