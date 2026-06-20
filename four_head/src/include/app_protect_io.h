/**
 * @file    app_protect_io.h
 * @brief   AppProtect Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * 故障保护逻辑 — 从 CommMgr 获取寄存器，输出故障状态到 Power
 *
 * 输入源:
 *   AppCommMgr → AppProtect  (Modbus 寄存器数据 → Protect)
 *
 * 输出目标:
 *   AppProtect → AppPower  (故障状态 → Power)
 */

#ifndef APPPROTECT_IO_H
#define APPPROTECT_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppProtect → AppPower  输出参数  (故障状态 → Power)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 0-3 */
    uint8_t slave_addr;     /* Modbus 站号 */
    uint16_t fault;     /* 故障位集合 ProtectFault_t */
} MODULE_OUTPUT_PARAMS(AppProtect, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppProtect, AppPower) *params;
} MODULE_OUTPUT_LINK(AppProtect, AppPower);

/* AppProtect_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppProtect, AppPower) *AppPower_params;  /* → AppPower */
} MODULE_OUTPUT(AppProtect);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppCommMgr → AppProtect  输入参数  (Modbus 寄存器数据 → Protect)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 0-3 */
    uint8_t slave_addr;     /* Modbus 站号 */
    uint8_t online;     /* 是否在线 */
    uint16_t regs[22];     /* 寄存器值 0x1000-0x1015 */
} MODULE_INPUT_PARAMS(AppCommMgr, AppProtect);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppCommMgr, AppProtect) *params;
} MODULE_INPUT_LINK(AppCommMgr, AppProtect);

/* AppProtect_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppCommMgr, AppProtect) *AppCommMgr_params;  /* 指向 AppCommMgr 输出的 LINK 列 */
} MODULE_INPUT(AppProtect);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(AppProtect);

#endif /* APPPROTECT_IO_H */
