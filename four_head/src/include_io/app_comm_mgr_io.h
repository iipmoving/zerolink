// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
/**
 * @file    app_comm_mgr_io.h
 * @brief   AppCommMgr Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * Modbus 通信轮询调度器 — 分发寄存器数据到 Power/Cooking/Protect
 *
 * 输入源:
 *   ProtoModbus → AppCommMgr  (协议响应 → AppCommMgr (编码帧/解析结果, 2tick响应))
 *   DrvCommMgr → AppCommMgr  (TX完成/RX数据事件 → AppCommMgr (替代上行回调 OnTxDone/OnDataUpdate))
 *   AppPower → AppCommMgr  (功率命令 → AppCommMgr (MODBUS帧构建后发 DrvCommMgr))
 *
 * 输出目标:
 *   AppCommMgr → AppPower  (Modbus 寄存器数据 → Power)
 *   AppCommMgr → AppCooking  (Modbus 寄存器数据 → Cooking)
 *   AppCommMgr → AppProtect  (Modbus 寄存器数据 → Protect)
 *   AppCommMgr → ProtoModbus  (协议请求 → ProtoModbus (BuildRead/BuildWriteSingle/Parse, 2tick请求-响应))
 *   AppCommMgr → DrvCommMgr  (TX发送请求 → DrvCommMgr (替代 __weak DrvCommMgr_OnSendReq))
 */

#ifndef APPCOMMMGR_IO_H
#define APPCOMMMGR_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppCommMgr → AppPower  输出参数  (Modbus 寄存器数据 → Power)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 0-3 */
    uint8_t slave_addr;     /* Modbus 站号 */
    uint8_t online;     /* 是否在线 */
    uint16_t regs[22];     /* 寄存器值 0x1000-0x1015 */
} MODULE_OUTPUT_PARAMS(AppCommMgr, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppCommMgr, AppPower) *params;
} MODULE_OUTPUT_LINK(AppCommMgr, AppPower);

/* ------------------------------------------------------------------
 * AppCommMgr → AppCooking  输出参数  (Modbus 寄存器数据 → Cooking)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 0-3 */
    uint8_t slave_addr;     /* Modbus 站号 */
    uint8_t online;     /* 是否在线 */
    uint16_t regs[22];     /* 寄存器值 0x1000-0x1015 */
} MODULE_OUTPUT_PARAMS(AppCommMgr, AppCooking);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppCommMgr, AppCooking) *params;
} MODULE_OUTPUT_LINK(AppCommMgr, AppCooking);

/* ------------------------------------------------------------------
 * AppCommMgr → AppProtect  输出参数  (Modbus 寄存器数据 → Protect)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 0-3 */
    uint8_t slave_addr;     /* Modbus 站号 */
    uint8_t online;     /* 是否在线 */
    uint16_t regs[22];     /* 寄存器值 0x1000-0x1015 */
} MODULE_OUTPUT_PARAMS(AppCommMgr, AppProtect);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppCommMgr, AppProtect) *params;
} MODULE_OUTPUT_LINK(AppCommMgr, AppProtect);

/* ------------------------------------------------------------------
 * AppCommMgr → ProtoModbus  输出参数  (协议请求 → ProtoModbus (BuildRead/BuildWriteSingle/Parse, 2tick请求-响应))
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
} MODULE_OUTPUT_PARAMS(AppCommMgr, ProtoModbus);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppCommMgr, ProtoModbus) *params;
} MODULE_OUTPUT_LINK(AppCommMgr, ProtoModbus);

/* ------------------------------------------------------------------
 * AppCommMgr → DrvCommMgr  输出参数  (TX发送请求 → DrvCommMgr (替代 __weak DrvCommMgr_OnSendReq))
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_idx;     /* 炉头索引0-3 */
    uint8_t res[1];     /* 对齐 */
    uint8_t tx_data[64];     /* 发送帧数据 */
    uint16_t tx_len;     /* 发送帧长度 */
} MODULE_OUTPUT_PARAMS(AppCommMgr, DrvCommMgr);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppCommMgr, DrvCommMgr) *params;
} MODULE_OUTPUT_LINK(AppCommMgr, DrvCommMgr);

/* AppCommMgr_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppCommMgr, AppPower) *AppPower_params;  /* → AppPower */
    MODULE_OUTPUT_LINK(AppCommMgr, AppCooking) *AppCooking_params;  /* → AppCooking */
    MODULE_OUTPUT_LINK(AppCommMgr, AppProtect) *AppProtect_params;  /* → AppProtect */
    MODULE_OUTPUT_LINK(AppCommMgr, ProtoModbus) *ProtoModbus_params;  /* → ProtoModbus */
    MODULE_OUTPUT_LINK(AppCommMgr, DrvCommMgr) *DrvCommMgr_params;  /* → DrvCommMgr */
} MODULE_OUTPUT(AppCommMgr);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * ProtoModbus → AppCommMgr  输入参数  (协议响应 → AppCommMgr (编码帧/解析结果, 2tick响应))
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
} MODULE_INPUT_PARAMS(ProtoModbus, AppCommMgr);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(ProtoModbus, AppCommMgr) *params;
} MODULE_INPUT_LINK(ProtoModbus, AppCommMgr);

/* ------------------------------------------------------------------
 * DrvCommMgr → AppCommMgr  输入参数  (TX完成/RX数据事件 → AppCommMgr (替代上行回调 OnTxDone/OnDataUpdate))
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t event;     /* 1=TX_DONE 2=RX_DATA */
    uint8_t head_idx;     /* 炉头索引0-3 */
    uint8_t res[2];     /* 对齐 */
    uint8_t rx_data[256];     /* RX_DATA: 接收帧数据 */
    uint16_t rx_len;     /* RX_DATA: 接收帧长度 */
} MODULE_INPUT_PARAMS(DrvCommMgr, AppCommMgr);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(DrvCommMgr, AppCommMgr) *params;
} MODULE_INPUT_LINK(DrvCommMgr, AppCommMgr);

/* ------------------------------------------------------------------
 * AppPower → AppCommMgr  输入参数  (功率命令 → AppCommMgr (MODBUS帧构建后发 DrvCommMgr))
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_idx;     /* 炉头索引 0-3 */
    uint8_t power_on;     /* 功率开关 0/1 */
    uint16_t target_power;     /* 目标功率 W */
} MODULE_INPUT_PARAMS(AppPower, AppCommMgr);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppPower, AppCommMgr) *params;
} MODULE_INPUT_LINK(AppPower, AppCommMgr);

/* AppCommMgr_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(ProtoModbus, AppCommMgr) *ProtoModbus_params;  /* 指向 ProtoModbus 输出的 LINK 列 */
    MODULE_INPUT_LINK(DrvCommMgr, AppCommMgr) *DrvCommMgr_params;  /* 指向 DrvCommMgr 输出的 LINK 列 */
    MODULE_INPUT_LINK(AppPower, AppCommMgr) *AppPower_params;  /* 指向 AppPower 输出的 LINK 列 */
} MODULE_INPUT(AppCommMgr);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(AppCommMgr);

#endif /* APPCOMMMGR_IO_H */

// ===== [END AI GENERATED] =====

