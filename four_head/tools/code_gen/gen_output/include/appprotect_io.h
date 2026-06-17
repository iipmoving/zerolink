/**
 * appprotect_io.h —— AppProtect 输入输出接口定义 (v2.3)
 *
 * 数据流:
 *   INPUT: AppCommMgr → AppProtect
 *   OUTPUT: AppProtect → AppPower

 *
 * Include 权限:
 *   - 仅 appprotect.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/appprotect_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef APPPROTECT_IO_H
#define APPPROTECT_IO_H

#include "../core/std_module.h"
#include <stdint.h>


/* ========== OUTPUT (AppProtect 给别的模块提供的数据) ========== */

/* AppProtect_to_AppPower_Params — 输出给 AppPower 的数据 */
typedef struct {
    // TODO: 添加输出字段
    uint8_t  res[4];
} MODULE_OUTPUT_PARAMS(AppProtect, AppPower);


/* AppProtect_to_AppPower_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大数量 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppProtect, AppPower) *params;
} MODULE_OUTPUT_LINK(AppProtect, AppPower);


/* AppProtect_Output — 输出聚合 (v2.3 对称命名: 成员名 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppProtect, AppPower) *AppPower_params;  /* → AppPower */

} MODULE_OUTPUT(AppProtect);

/* ========== Consumer INPUT_LINK 定义在 Consumer 的 io.h ========== */
/* AppPower_INPUT_LINK(AppProtect, AppPower) 定义在 apppower_io.h */




/* ========== INPUT (AppProtect 从别的模块得到的数据) ========== */

/* AppCommMgr_to_AppProtect_Input_Params — 从 AppCommMgr 得到的数据 */
typedef struct {
    // TODO: 添加输入字段（与 AppCommMgr OUTPUT 布局一致）
    uint8_t  res[4];
} MODULE_INPUT_PARAMS(AppCommMgr, AppProtect);


/* AppCommMgr_to_AppProtect_Input_Link — 输入管道 (与 AppCommMgr OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppCommMgr, AppProtect) *params;
} MODULE_INPUT_LINK(AppCommMgr, AppProtect);


/* AppProtect_Input — 输入聚合 (v2.3 对称命名: 成员名 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppCommMgr, AppProtect) *AppCommMgr_params;  /* 从 AppCommMgr 得到 */

} MODULE_INPUT(AppProtect);


#endif /* APPPROTECT_IO_H */
