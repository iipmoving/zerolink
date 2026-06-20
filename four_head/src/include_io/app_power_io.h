/**
 * @file    app_power_io.h
 * @brief   AppPower Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * 功率控制 PID — 聚合 CommMgr/Protect/Cooking 输入，输出功率命令到 CommMgr 和状态到 Hmi
 *
 * 输入源:
 *   AppCommMgr → AppPower  (Modbus 寄存器数据 → Power)
 *   AppCooking → AppPower  (烹饪状态 → Power)
 *   AppProtect → AppPower  (故障状态 → Power)
 *
 * 输出目标:
 *   AppPower → AppHmi  (功率状态 → Hmi)
 *   AppPower → DrvCommMgr  (功率命令 → DrvCommMgr)
 */

#ifndef APPPOWER_IO_H
#define APPPOWER_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppPower → AppHmi  输出参数  (功率状态 → Hmi)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 0-3 */
    uint8_t power_on;     /* 功率开关 0/1 */
    uint8_t power_level;     /* 功率档位 0-9 */
    uint16_t actual_power;     /* 实际功率 W */
} MODULE_OUTPUT_PARAMS(AppPower, AppHmi);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppPower, AppHmi) *params;
} MODULE_OUTPUT_LINK(AppPower, AppHmi);

/* ------------------------------------------------------------------
 * AppPower → DrvCommMgr  输出参数  (功率命令 → DrvCommMgr)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 0-3 */
    uint8_t slave_addr;     /* Modbus 站号 */
    uint8_t power_on;     /* 功率开关 0/1 */
    uint16_t target_power;     /* 目标功率 W */
    uint16_t actual_power;     /* 实际功率 W */
} MODULE_OUTPUT_PARAMS(AppPower, DrvCommMgr);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppPower, DrvCommMgr) *params;
} MODULE_OUTPUT_LINK(AppPower, DrvCommMgr);

/* AppPower_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppPower, AppHmi) *AppHmi_params;  /* → AppHmi */
    MODULE_OUTPUT_LINK(AppPower, DrvCommMgr) *DrvCommMgr_params;  /* → DrvCommMgr */
} MODULE_OUTPUT(AppPower);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppCommMgr → AppPower  输入参数  (Modbus 寄存器数据 → Power)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 0-3 */
    uint8_t slave_addr;     /* Modbus 站号 */
    uint8_t online;     /* 是否在线 */
    uint16_t regs[22];     /* 寄存器值 0x1000-0x1015 */
} MODULE_INPUT_PARAMS(AppCommMgr, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppCommMgr, AppPower) *params;
} MODULE_INPUT_LINK(AppCommMgr, AppPower);

/* ------------------------------------------------------------------
 * AppCooking → AppPower  输入参数  (烹饪状态 → Power)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 */
    uint8_t cooking_state;     /* 烹饪状态 COOKING_STATE_* */
    uint8_t power_level;     /* 功率档位 0-9 */
    uint16_t target_power;     /* 目标功率 W */
    uint16_t actual_power;     /* 实际功率 W */
    uint32_t cooking_time;     /* 烹饪时间 ms */
} MODULE_INPUT_PARAMS(AppCooking, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppCooking, AppPower) *params;
} MODULE_INPUT_LINK(AppCooking, AppPower);

/* ------------------------------------------------------------------
 * AppProtect → AppPower  输入参数  (故障状态 → Power)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 0-3 */
    uint8_t slave_addr;     /* Modbus 站号 */
    uint16_t fault;     /* 故障位集合 ProtectFault_t */
} MODULE_INPUT_PARAMS(AppProtect, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppProtect, AppPower) *params;
} MODULE_INPUT_LINK(AppProtect, AppPower);

/* AppPower_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppCommMgr, AppPower) *AppCommMgr_params;  /* 指向 AppCommMgr 输出的 LINK 列 */
    MODULE_INPUT_LINK(AppCooking, AppPower) *AppCooking_params;  /* 指向 AppCooking 输出的 LINK 列 */
    MODULE_INPUT_LINK(AppProtect, AppPower) *AppProtect_params;  /* 指向 AppProtect 输出的 LINK 列 */
} MODULE_INPUT(AppPower);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(AppPower);

#endif /* APPPOWER_IO_H */
