/**
 * app_protect_io.h —— AppProtect 输入输出接口定义 (v2.3)
 *
 * 数据流:
 *   INPUT: AppCommMgr → AppProtect (寄存器数据)
 *   OUTPUT: AppProtect → AppPower (故障数据)
 *
 * Include 权限:
 *   - 仅 app_protect.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/app_protect_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef APP_PROTECT_IO_H
#define APP_PROTECT_IO_H

#include "../core/std_module.h"
#include <stdint.h>

/* ========== OUTPUT (AppProtect 给别的模块提供的数据) ========== */

/* Protect_to_Power_Params — 输出数据参数 */
typedef struct {
    uint8_t  head_index;      /* 炉头索引 0-3 */
    uint8_t  slave_addr;      /* MODBUS 站号 */
    uint16_t fault;           /* ProtectFault_t: 故障位集合 */
    uint8_t  res[2];
} MODULE_OUTPUT_PARAMS(Protect, Power);

/* Protect_to_Power_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;          /* ST_NEW/ST_OUT */
    uint8_t  max_count;       /* 最大炉头数 = 4 */
    uint8_t  count;           /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(Protect, Power) *params;
} MODULE_OUTPUT_LINK(Protect, Power);

/* AppProtect_Output — 输出聚合 */
typedef struct {
    MODULE_OUTPUT_LINK(Protect, Power) *to_power;
} MODULE_OUTPUT(AppProtect);

/* ========== INPUT (AppProtect 从别的模块得到的数据) ========== */

/* CommMgr_to_Protect_Params — 输入数据参数 (布局与 AppCommMgr OUTPUT 一致) */
typedef struct {
    uint8_t  head_index;
    uint8_t  slave_addr;
    uint8_t  online;
    uint8_t  res[1];
    uint16_t regs[22];
} MODULE_INPUT_PARAMS(CommMgr, Protect);

/* CommMgr_to_Protect_Input_Link — 输入管道 (与 AppCommMgr OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(CommMgr, Protect) *params;
} MODULE_INPUT_LINK(CommMgr, Protect);

/* AppProtect_Input — 输入聚合 */
typedef struct {
    MODULE_INPUT_LINK(CommMgr, Protect) *comm;  /* 从 AppCommMgr 得到 */
} MODULE_INPUT(AppProtect);

#endif /* APP_PROTECT_IO_H */