/**
 * @file    app_comm_mgr_io.h
 * @brief   AppCommMgr Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * Modbus 通信轮询调度器 — 分发寄存器数据到 Power/Cooking/Protect
 *
 * 输出目标:
 *   AppCommMgr → AppPower  (Modbus 寄存器数据 → Power)
 *   AppCommMgr → AppCooking  (Modbus 寄存器数据 → Cooking)
 *   AppCommMgr → AppProtect  (Modbus 寄存器数据 → Protect)
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
    uint8_t  count;            /* 当前周期索引 */
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
    uint8_t  count;            /* 当前周期索引 */
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
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppCommMgr, AppProtect) *params;
} MODULE_OUTPUT_LINK(AppCommMgr, AppProtect);

/* AppCommMgr_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppCommMgr, AppPower) *AppPower_params;  /* → AppPower */
    MODULE_OUTPUT_LINK(AppCommMgr, AppCooking) *AppCooking_params;  /* → AppCooking */
    MODULE_OUTPUT_LINK(AppCommMgr, AppProtect) *AppProtect_params;  /* → AppProtect */
} MODULE_OUTPUT(AppCommMgr);

/* ========== INPUT (无) — 本模块没有输入 ========== */

/* AppCommMgr_Input — 无输入 */
typedef struct {
    uint8_t  res[4];
} MODULE_INPUT(AppCommMgr);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(AppCommMgr);

#endif /* APPCOMMMGR_IO_H */
