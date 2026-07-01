// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
/**
 * @file    proto_modbus_io.h
 * @brief   ProtoModbus Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   proto
 *
 * MODBUS RTU 协议编解码 — 从 AppCommMgr 接收请求，返回编码帧/解析结果 (2tick)
 *
 * 输入源:
 *   AppCommMgr → ProtoModbus  (协议请求 → ProtoModbus (BuildRead/BuildWriteSingle/Parse, 2tick请求-响应))
 *
 * 输出目标:
 *   ProtoModbus → AppCommMgr  (协议响应 → AppCommMgr (编码帧/解析结果, 2tick响应))
 */

#ifndef PROTOMODBUS_IO_H
#define PROTOMODBUS_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * ProtoModbus → AppCommMgr  输出参数  (协议响应 → AppCommMgr (编码帧/解析结果, 2tick响应))
 * ------------------------------------------------------------------ */
typedef struct {
    int8_t result;     /* 0=OK -1=ERR_CRC -2=ERR_EXC -3=ERR_LEN */
    uint8_t slave;     /* 解析出的站号(Build:回显) */
    uint8_t func;     /* 解析出的功能码(Build:0x03/0x06) */
    uint8_t pad1;     /* 对齐 */
    uint8_t tx_data[64];     /* Build: 编码后的帧数据 */
    uint16_t tx_len;     /* Build: 帧长度(含CRC) */
    uint16_t data[22];     /* Parse: 解析出的寄存器数据 */
    uint16_t data_count;     /* Parse: 数据数量 */
} MODULE_OUTPUT_PARAMS(ProtoModbus, AppCommMgr);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(ProtoModbus, AppCommMgr) *params;
} MODULE_OUTPUT_LINK(ProtoModbus, AppCommMgr);

/* ProtoModbus_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(ProtoModbus, AppCommMgr) *AppCommMgr_params;  /* → AppCommMgr */
} MODULE_OUTPUT(ProtoModbus);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppCommMgr → ProtoModbus  输入参数  (协议请求 → ProtoModbus (BuildRead/BuildWriteSingle/Parse, 2tick请求-响应))
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t cmd;     /* 1=BuildRead 2=BuildWriteSingle 3=Parse */
    uint8_t slave;     /* Modbus站号 */
    uint8_t res[2];     /* 对齐保留 */
    uint16_t read_reg;     /* BuildRead: 起始寄存器 */
    uint16_t read_count;     /* BuildRead: 寄存器数量(max22) */
    uint16_t write_reg;     /* BuildWriteSingle: 寄存器地址 */
    uint16_t write_val;     /* BuildWriteSingle: 写入值 */
    uint8_t rx_data[256];     /* Parse: 接收帧缓冲 */
    uint16_t rx_len;     /* Parse: 接收帧长度 */
} MODULE_INPUT_PARAMS(AppCommMgr, ProtoModbus);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppCommMgr, ProtoModbus) *params;
} MODULE_INPUT_LINK(AppCommMgr, ProtoModbus);

/* ProtoModbus_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppCommMgr, ProtoModbus) *AppCommMgr_params;  /* 指向 AppCommMgr 输出的 LINK 列 */
} MODULE_INPUT(ProtoModbus);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(ProtoModbus);

#endif /* PROTOMODBUS_IO_H */

// ===== [END AI GENERATED] =====

