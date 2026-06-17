/**
 * app_power_io.h —— AppPower 输入输出接口定义 (v2.3)
 *
 * 数据流:
 *   INPUT: AppCommMgr → AppPower (寄存器数据)
 *   INPUT: AppProtect → AppPower (故障数据)
 *   INPUT: AppCooking → AppPower (烹饪状态)
 *   OUTPUT: AppPower → DrvCommMgr (功率命令)
 *   OUTPUT: AppPower → AppHmi (功率状态)
 *
 * Include 权限:
 *   - 仅 app_power.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/app_power_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef APP_POWER_IO_H
#define APP_POWER_IO_H

#include "../core/std_module.h"
#include <stdint.h>

/* ========== OUTPUT (AppPower 给别的模块提供的数据) ========== */

/* Power_to_DrvCommMgr_Params — 输出数据参数 */
typedef struct {
    uint8_t  head_index;      /* 炉头索引 0-3 */
    uint8_t  slave_addr;      /* MODBUS 站号 */
    uint8_t  power_on;        /* 功率开关 0/1 */
    uint8_t  res[1];
    uint16_t target_power;    /* 目标功率 W */
    uint16_t actual_power;    /* 实际功率 W */
} MODULE_OUTPUT_PARAMS(Power, DrvCommMgr);

/* Power_to_DrvCommMgr_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;          /* ST_NEW/ST_OUT */
    uint8_t  max_count;       /* 最大炉头数 = 4 */
    uint8_t  count;           /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(Power, DrvCommMgr) *params;
} MODULE_OUTPUT_LINK(Power, DrvCommMgr);

/* Power_to_Hmi_Params — 输出数据参数 */
typedef struct {
    uint8_t  head_index;      /* 炉头索引 0-3 */
    uint8_t  power_on;        /* 功率开关 0/1 */
    uint8_t  power_level;     /* 功率档位 0-9 */
    uint8_t  res[1];
    uint16_t actual_power;    /* 实际功率 W */
} MODULE_OUTPUT_PARAMS(Power, Hmi);

/* Power_to_Hmi_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(Power, Hmi) *params;
} MODULE_OUTPUT_LINK(Power, Hmi);

/* AppPower_Output — 输出聚合 (v2.3 对称命名: 成员名 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(Power, DrvCommMgr) *DrvCommMgr_params;  /* → DrvCommMgr */
    MODULE_OUTPUT_LINK(Power, Hmi)       *Hmi_params;          /* → AppHmi */
} MODULE_OUTPUT(AppPower);

/* ========== INPUT (AppPower 从别的模块得到的数据) ========== */

/* CommMgr_to_Power_Params — 输入数据参数 (布局与 AppCommMgr OUTPUT 一致) */
typedef struct {
    uint8_t  head_index;
    uint8_t  slave_addr;
    uint8_t  online;
    uint8_t  res[1];
    uint16_t regs[22];
} MODULE_INPUT_PARAMS(CommMgr, Power);

/* CommMgr_to_Power_Input_Link — 输入管道 (与 AppCommMgr OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(CommMgr, Power) *params;
} MODULE_INPUT_LINK(CommMgr, Power);

/* Protect_to_Power_Params — 输入数据参数 (布局与 AppProtect OUTPUT 一致) */
typedef struct {
    uint8_t  head_index;
    uint8_t  slave_addr;
    uint16_t fault;           /* ProtectFault_t: 故障位集合 */
    uint8_t  res[2];
} MODULE_INPUT_PARAMS(Protect, Power);

/* Protect_to_Power_Input_Link — 输入管道 (与 AppProtect OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(Protect, Power) *params;
} MODULE_INPUT_LINK(Protect, Power);

/* Cooking_to_Power_Params — 输入数据参数 (布局与 AppCooking OUTPUT 一致) */
typedef struct {
    uint8_t  head_index;
    uint8_t  cooking_state;
    uint8_t  power_level;
    uint8_t  res[1];
    uint16_t target_power;
    uint16_t actual_power;
    uint32_t cooking_time;
} MODULE_INPUT_PARAMS(Cooking, Power);

/* Cooking_to_Power_Input_Link — 输入管道 (与 AppCooking OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(Cooking, Power) *params;
} MODULE_INPUT_LINK(Cooking, Power);

/* AppPower_Input — 输入聚合 (v2.3 对称命名: 成员名 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(CommMgr, Power)   *CommMgr_params;   /* 从 AppCommMgr 得到 */
    MODULE_INPUT_LINK(Protect, Power)   *Protect_params;   /* 从 AppProtect 得到 */
    MODULE_INPUT_LINK(Cooking, Power)   *Cooking_params;   /* 从 AppCooking 得到 */
} MODULE_INPUT(AppPower);

#endif /* APP_POWER_IO_H */