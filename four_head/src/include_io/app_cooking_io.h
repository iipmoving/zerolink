// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
/**
 * @file    app_cooking_io.h
 * @brief   AppCooking Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * 烹饪流程控制 — 从 Key/CommMgr 输入，输出烹饪状态到 Power
 *
 * 输入源:
 *   DrvKey → AppCooking  (按键事件 → Cooking 状态机)
 *   AppCommMgr → AppCooking  (Modbus 寄存器数据 → Cooking)
 *
 * 输出目标:
 *   AppCooking → AppPower  (烹饪状态 → Power)
 */

#ifndef APPCOOKING_IO_H
#define APPCOOKING_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppCooking → AppPower  输出参数  (烹饪状态 → Power)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 */
    uint8_t cooking_state;     /* 烹饪状态 COOKING_STATE_* */
    uint8_t power_level;     /* 功率档位 0-9 */
    uint16_t target_power;     /* 目标功率 W */
    uint16_t actual_power;     /* 实际功率 W */
    uint32_t cooking_time;     /* 烹饪时间 ms */
} MODULE_OUTPUT_PARAMS(AppCooking, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppCooking, AppPower) *params;
} MODULE_OUTPUT_LINK(AppCooking, AppPower);

/* AppCooking_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppCooking, AppPower) *AppPower_params;  /* → AppPower */
} MODULE_OUTPUT(AppCooking);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * DrvKey → AppCooking  输入参数  (按键事件 → Cooking 状态机)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t key_code;     /* 按键码 KeyCode_t */
    uint8_t key_state;     /* 按键状态 KEY_STATE_* */
    uint8_t head_index;     /* 关联炉头索引 */
} MODULE_INPUT_PARAMS(DrvKey, AppCooking);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(DrvKey, AppCooking) *params;
} MODULE_INPUT_LINK(DrvKey, AppCooking);

/* ------------------------------------------------------------------
 * AppCommMgr → AppCooking  输入参数  (Modbus 寄存器数据 → Cooking)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 0-3 */
    uint8_t slave_addr;     /* Modbus 站号 */
    uint8_t online;     /* 是否在线 */
    uint16_t regs[22];     /* 寄存器值 0x1000-0x1015 */
} MODULE_INPUT_PARAMS(AppCommMgr, AppCooking);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppCommMgr, AppCooking) *params;
} MODULE_INPUT_LINK(AppCommMgr, AppCooking);

/* AppCooking_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(DrvKey, AppCooking) *DrvKey_params;  /* 指向 DrvKey 输出的 LINK 列 */
    MODULE_INPUT_LINK(AppCommMgr, AppCooking) *AppCommMgr_params;  /* 指向 AppCommMgr 输出的 LINK 列 */
} MODULE_INPUT(AppCooking);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(AppCooking);

#endif /* APPCOOKING_IO_H */

// ===== [END AI GENERATED] =====

