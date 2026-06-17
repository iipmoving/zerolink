/**
 * app_cooking_io.h —— AppCooking 输入输出接口定义 (v2.3)
 *
 * 数据流:
 *   INPUT: DrvKey → AppCooking (按键事件)
 *   INPUT: AppCommMgr → AppCooking (寄存器数据)
 *   OUTPUT: AppCooking → AppPower (烹饪状态)
 *
 * Include 权限:
 *   - 仅 app_cooking.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/app_cooking_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef APP_COOKING_IO_H
#define APP_COOKING_IO_H

#include "../core/std_module.h"
#include <stdint.h>

/* ========== OUTPUT (AppCooking 给别的模块提供的数据) ========== */

/* Cooking_to_Power_Params — 输出数据参数 */
typedef struct {
    uint8_t  head_index;      /* 炉头索引 0-3 */
    uint8_t  cooking_state;   /* COOKING_STATE_*: 烹饪状态 */
    uint8_t  power_level;     /* 功率档位 0-9 */
    uint8_t  res[1];
    uint16_t target_power;    /* 目标功率 W */
    uint16_t actual_power;    /* 实际功率 W */
    uint32_t cooking_time;    /* 烹饪时间 ms */
} MODULE_OUTPUT_PARAMS(Cooking, Power);

/* Cooking_to_Power_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;          /* ST_NEW/ST_OUT */
    uint8_t  max_count;       /* 最大炉头数 = 4 */
    uint8_t  count;           /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(Cooking, Power) *params;
} MODULE_OUTPUT_LINK(Cooking, Power);

/* AppCooking_Output — 输出聚合 */
typedef struct {
    MODULE_OUTPUT_LINK(Cooking, Power) *to_power;
} MODULE_OUTPUT(AppCooking);

/* ========== INPUT (AppCooking 从别的模块得到的数据) ========== */

/* Key_to_Cooking_Params — 输入数据参数 (布局与 DrvKey OUTPUT 一致) */
typedef struct {
    uint8_t  key_code;        /* KeyCode_t: 按键码 */
    uint8_t  key_state;       /* KEY_STATE_*: 按键状态 */
    uint8_t  head_index;      /* 炉头索引 */
    uint8_t  res[1];
} MODULE_INPUT_PARAMS(Key, Cooking);

/* Key_to_Cooking_Input_Link — 输入管道 (与 DrvKey OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(Key, Cooking) *params;
} MODULE_INPUT_LINK(Key, Cooking);

/* CommMgr_to_Cooking_Params — 输入数据参数 (布局与 AppCommMgr OUTPUT 一致) */
typedef struct {
    uint8_t  head_index;
    uint8_t  slave_addr;
    uint8_t  online;
    uint8_t  res[1];
    uint16_t regs[22];
} MODULE_INPUT_PARAMS(CommMgr, Cooking);

/* CommMgr_to_Cooking_Input_Link — 输入管道 (与 AppCommMgr OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(CommMgr, Cooking) *params;
} MODULE_INPUT_LINK(CommMgr, Cooking);

/* AppCooking_Input — 输入聚合 */
typedef struct {
    MODULE_INPUT_LINK(Key, Cooking)      *key;    /* 从 DrvKey 得到 */
    MODULE_INPUT_LINK(CommMgr, Cooking)  *comm;   /* 从 AppCommMgr 得到 */
} MODULE_INPUT(AppCooking);

#endif /* APP_COOKING_IO_H */