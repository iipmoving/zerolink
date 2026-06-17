/**
 * @file    drvcommmgr_io.h
 * @brief   DrvCommMgr Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   drv
 *
 * 通信驱动管理 — 从 Power 接收功率命令，发送 Modbus
 *
 * 输入源:
 *   AppPower → DrvCommMgr  (功率命令 → DrvCommMgr)
 */

#ifndef DRVCOMMMGR_IO_H
#define DRVCOMMMGR_IO_H

#include <stdint.h>
#include "../core/std_module.h"

#pragma pack(4)

/* ========== OUTPUT (无) — 本模块没有输出 ========== */

/* DrvCommMgr_Output — 无输出 */
typedef struct {
    uint8_t  res[4];
} MODULE_OUTPUT(DrvCommMgr);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppPower → DrvCommMgr  输入参数  (功率命令 → DrvCommMgr)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 0-3 */
    uint8_t slave_addr;     /* Modbus 站号 */
    uint8_t power_on;     /* 功率开关 0/1 */
    uint16_t target_power;     /* 目标功率 W */
    uint16_t actual_power;     /* 实际功率 W */
} MODULE_INPUT_PARAMS(AppPower, DrvCommMgr);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppPower, DrvCommMgr) *params;
} MODULE_INPUT_LINK(AppPower, DrvCommMgr);

/* DrvCommMgr_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppPower, DrvCommMgr)  AppPower_params;  /* 从 AppPower 来 */
} MODULE_INPUT(DrvCommMgr);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(DrvCommMgr);

#endif /* DRVCOMMMGR_IO_H */
