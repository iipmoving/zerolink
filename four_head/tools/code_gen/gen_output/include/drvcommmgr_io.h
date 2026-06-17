/**
 * drvcommmgr_io.h —— DrvCommMgr 输入输出接口定义 (v2.3)
 *
 * 数据流:
 *   INPUT: AppPower → DrvCommMgr

 *
 * Include 权限:
 *   - 仅 drvcommmgr.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/drvcommmgr_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef DRVCOMMMGR_IO_H
#define DRVCOMMMGR_IO_H

#include "../core/std_module.h"
#include <stdint.h>

/* ========== OUTPUT (无) ========== */
/* 本模块没有输出 */


/* ========== INPUT (DrvCommMgr 从别的模块得到的数据) ========== */

/* AppPower_to_DrvCommMgr_Input_Params — 从 AppPower 得到的数据 */
typedef struct {
    // TODO: 添加输入字段（与 AppPower OUTPUT 布局一致）
    uint8_t  res[4];
} MODULE_INPUT_PARAMS(AppPower, DrvCommMgr);


/* AppPower_to_DrvCommMgr_Input_Link — 输入管道 (与 AppPower OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppPower, DrvCommMgr) *params;
} MODULE_INPUT_LINK(AppPower, DrvCommMgr);


/* DrvCommMgr_Input — 输入聚合 (v2.3 对称命名: 成员名 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppPower, DrvCommMgr) *AppPower_params;  /* 从 AppPower 得到 */

} MODULE_INPUT(DrvCommMgr);


#endif /* DRVCOMMMGR_IO_H */
