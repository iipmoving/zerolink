// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
/**
 * @file    drv_comm_mgr_io.h
 * @brief   DrvCommMgr Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   drv
 *
 * 通信驱动管理 — 从 AppCommMgr 接收发送请求，发送 Modbus
 *
 * 输入源:
 *   AppCommMgr → DrvCommMgr  (TX发送请求 → DrvCommMgr (替代 __weak DrvCommMgr_OnSendReq))
 *
 * 输出目标:
 *   DrvCommMgr → AppCommMgr  (TX完成/RX数据事件 → AppCommMgr (替代上行回调 OnTxDone/OnDataUpdate))
 */

#ifndef DRVCOMMMGR_IO_H
#define DRVCOMMMGR_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * DrvCommMgr → AppCommMgr  输出参数  (TX完成/RX数据事件 → AppCommMgr (替代上行回调 OnTxDone/OnDataUpdate))
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t event;     /* 1=TX_DONE 2=RX_DATA */
    uint8_t head_idx;     /* 炉头索引0-3 */
    uint8_t res[2];     /* 对齐 */
    uint8_t rx_data[256];     /* RX_DATA: 接收帧数据 */
    uint16_t rx_len;     /* RX_DATA: 接收帧长度 */
} MODULE_OUTPUT_PARAMS(DrvCommMgr, AppCommMgr);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(DrvCommMgr, AppCommMgr) *params;
} MODULE_OUTPUT_LINK(DrvCommMgr, AppCommMgr);

/* DrvCommMgr_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(DrvCommMgr, AppCommMgr) *AppCommMgr_params;  /* → AppCommMgr */
} MODULE_OUTPUT(DrvCommMgr);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppCommMgr → DrvCommMgr  输入参数  (TX发送请求 → DrvCommMgr (替代 __weak DrvCommMgr_OnSendReq))
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_idx;     /* 炉头索引0-3 */
    uint8_t res[1];     /* 对齐 */
    uint8_t tx_data[64];     /* 发送帧数据 */
    uint16_t tx_len;     /* 发送帧长度 */
} MODULE_INPUT_PARAMS(AppCommMgr, DrvCommMgr);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppCommMgr, DrvCommMgr) *params;
} MODULE_INPUT_LINK(AppCommMgr, DrvCommMgr);

/* DrvCommMgr_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppCommMgr, DrvCommMgr) *AppCommMgr_params;  /* 指向 AppCommMgr 输出的 LINK 列 */
} MODULE_INPUT(DrvCommMgr);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(DrvCommMgr);

#endif /* DRVCOMMMGR_IO_H */

// ===== [END AI GENERATED] =====

