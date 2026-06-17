/**
 * app_comm_mgr_io.h —— AppCommMgr 输出接口定义 (v2.3)
 *
 * 数据流: AppCommMgr → AppPower / AppCooking / AppProtect
 *
 * AppCommMgr 只有 OUTPUT (寄存器数据)，没有 INPUT
 *
 * Include 权限:
 *   - 仅 app_comm_mgr.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/app_comm_mgr_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef APP_COMM_MGR_IO_H
#define APP_COMM_MGR_IO_H

#include "../core/std_module.h"
#include <stdint.h>

/* ========== OUTPUT (AppCommMgr 给别的模块提供的数据) ========== */

/* CommMgr_to_Power_Params — 输出数据参数 */
typedef struct {
    uint8_t  head_index;      /* 炉头索引 0-3 */
    uint8_t  slave_addr;      /* MODBUS 站号 */
    uint8_t  online;          /* 是否在线 */
    uint8_t  res[1];
    uint16_t regs[22];        /* 寄存器值 0x1000-0x1015 */
} MODULE_OUTPUT_PARAMS(CommMgr, Power);

/* CommMgr_to_Power_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;          /* ST_NEW/ST_OUT */
    uint8_t  max_count;       /* 最大炉头数 = 4 */
    uint8_t  count;           /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(CommMgr, Power) *params;
} MODULE_OUTPUT_LINK(CommMgr, Power);

/* CommMgr_to_Cooking_Params */
typedef struct {
    uint8_t  head_index;
    uint8_t  slave_addr;
    uint8_t  online;
    uint8_t  res[1];
    uint16_t regs[22];
} MODULE_OUTPUT_PARAMS(CommMgr, Cooking);

/* CommMgr_to_Cooking_Output_Link */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(CommMgr, Cooking) *params;
} MODULE_OUTPUT_LINK(CommMgr, Cooking);

/* CommMgr_to_Protect_Params */
typedef struct {
    uint8_t  head_index;
    uint8_t  slave_addr;
    uint8_t  online;
    uint8_t  res[1];
    uint16_t regs[22];
} MODULE_OUTPUT_PARAMS(CommMgr, Protect);

/* CommMgr_to_Protect_Output_Link */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(CommMgr, Protect) *params;
} MODULE_OUTPUT_LINK(CommMgr, Protect);

/* AppCommMgr_Output — 输出聚合 */
typedef struct {
    MODULE_OUTPUT_LINK(CommMgr, Power)    *to_power;
    MODULE_OUTPUT_LINK(CommMgr, Cooking)  *to_cooking;
    MODULE_OUTPUT_LINK(CommMgr, Protect)  *to_protect;
} MODULE_OUTPUT(AppCommMgr);

/* ========== INPUT (无) ========== */
/* AppCommMgr 是通信轮询调度器，没有输入 */

/* ========== Consumer INPUT_LINK 定义在 Consumer 的 io.h ========== */
/* AppPower_INPUT_LINK(CommMgr, Power) 定义在 app_power_io.h */
/* AppCooking_INPUT_LINK(CommMgr, Cooking) 定义在 app_cooking_io.h */
/* AppProtect_INPUT_LINK(CommMgr, Protect) 定义在 app_protect_io.h */

#endif /* APP_COMM_MGR_IO_H */