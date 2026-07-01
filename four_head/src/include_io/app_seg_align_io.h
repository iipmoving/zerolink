// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
/**
 * @file    app_seg_align_io.h
 * @brief   AppSegAlign Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * 段码对齐处理 — 从 Key 输入，无输出
 *
 * 输入源:
 *   DrvKey → AppSegAlign  (按键事件 → SegAlign 调校模式)
 *
 * 输出目标:
 *   AppSegAlign → DrvDisplay  (段码控制 → DrvDisplay (替代 __weak DrvSegAlign_WriteCom/BlockHmi, route区分: 0=write 1=block))
 */

#ifndef APPSEGALIGN_IO_H
#define APPSEGALIGN_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppSegAlign → DrvDisplay  输出参数  (段码控制 → DrvDisplay (替代 __weak DrvSegAlign_WriteCom/BlockHmi, route区分: 0=write 1=block))
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t com_mask[11];     /* 11个COM位的段码掩码(route=0时有效) */
    uint8_t dirty;     /* 1=段码有更新需要刷新(route=0) */
    uint8_t block;     /* 1=阻塞HMI刷新 0=恢复(route=1) */
} MODULE_OUTPUT_PARAMS(AppSegAlign, DrvDisplay);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppSegAlign, DrvDisplay) *params;
} MODULE_OUTPUT_LINK(AppSegAlign, DrvDisplay);

/* AppSegAlign_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppSegAlign, DrvDisplay) *DrvDisplay_params;  /* → DrvDisplay */
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
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
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

// ===== [END AI GENERATED] =====

