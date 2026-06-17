/**
 * drv_comm_mgr_io.h —— DrvCommMgr 输入接口定义 (v2.3)
 *
 * 数据流:
 *   INPUT: AppPower → DrvCommMgr (功率命令)
 *
 * DrvCommMgr 只有 INPUT，没有 OUTPUT
 *
 * Include 权限:
 *   - 仅 drv_comm_mgr.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/drv_comm_mgr_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef DRV_COMM_MGR_IO_H
#define DRV_COMM_MGR_IO_H

#include "../core/std_module.h"
#include <stdint.h>

/* ========== OUTPUT (无) ========== */
/* DrvCommMgr 是底层驱动，没有输出 */

/* ========== INPUT (DrvCommMgr 从别的模块得到的数据) ========== */

/* Power_to_DrvCommMgr_Params — 输入数据参数 (布局与 AppPower OUTPUT 一致) */
typedef struct {
    uint8_t  head_index;      /* 炉头索引 0-3 */
    uint8_t  slave_addr;      /* MODBUS 站号 */
    uint8_t  power_on;        /* 功率开关 0/1 */
    uint8_t  res[1];
    uint16_t target_power;    /* 目标功率 W */
    uint16_t actual_power;    /* 实际功率 W */
} MODULE_INPUT_PARAMS(Power, DrvCommMgr);

/* Power_to_DrvCommMgr_Input_Link — 输入管道 (与 AppPower OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(Power, DrvCommMgr) *params;
} MODULE_INPUT_LINK(Power, DrvCommMgr);

/* DrvCommMgr_Input — 输入聚合 */
typedef struct {
    MODULE_INPUT_LINK(Power, DrvCommMgr) *power;  /* 从 AppPower 得到 */
} MODULE_INPUT(DrvCommMgr);

#endif /* DRV_COMM_MGR_IO_H */