/**
 * appcommmgr_io.h —— AppCommMgr 输入输出接口定义 (v2.3)
 *
 * 数据流:
 *   OUTPUT: AppCommMgr → AppPower
 *   OUTPUT: AppCommMgr → AppCooking
 *   OUTPUT: AppCommMgr → AppProtect

 *
 * Include 权限:
 *   - 仅 appcommmgr.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/appcommmgr_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef APPCOMMMGR_IO_H
#define APPCOMMMGR_IO_H

#include "../core/std_module.h"
#include <stdint.h>


/* ========== OUTPUT (AppCommMgr 给别的模块提供的数据) ========== */

/* AppCommMgr_to_AppPower_Params — 输出给 AppPower 的数据 */
typedef struct {
    // TODO: 添加输出字段
    uint8_t  res[4];
} MODULE_OUTPUT_PARAMS(AppCommMgr, AppPower);
/* AppCommMgr_to_AppCooking_Params — 输出给 AppCooking 的数据 */
typedef struct {
    // TODO: 添加输出字段
    uint8_t  res[4];
} MODULE_OUTPUT_PARAMS(AppCommMgr, AppCooking);
/* AppCommMgr_to_AppProtect_Params — 输出给 AppProtect 的数据 */
typedef struct {
    // TODO: 添加输出字段
    uint8_t  res[4];
} MODULE_OUTPUT_PARAMS(AppCommMgr, AppProtect);


/* AppCommMgr_to_AppPower_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大数量 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppCommMgr, AppPower) *params;
} MODULE_OUTPUT_LINK(AppCommMgr, AppPower);
/* AppCommMgr_to_AppCooking_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大数量 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppCommMgr, AppCooking) *params;
} MODULE_OUTPUT_LINK(AppCommMgr, AppCooking);
/* AppCommMgr_to_AppProtect_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大数量 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppCommMgr, AppProtect) *params;
} MODULE_OUTPUT_LINK(AppCommMgr, AppProtect);


/* AppCommMgr_Output — 输出聚合 (v2.3 对称命名: 成员名 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppCommMgr, AppPower) *AppPower_params;  /* → AppPower */
    MODULE_OUTPUT_LINK(AppCommMgr, AppCooking) *AppCooking_params;  /* → AppCooking */
    MODULE_OUTPUT_LINK(AppCommMgr, AppProtect) *AppProtect_params;  /* → AppProtect */

} MODULE_OUTPUT(AppCommMgr);

/* ========== Consumer INPUT_LINK 定义在 Consumer 的 io.h ========== */
/* AppPower_INPUT_LINK(AppCommMgr, AppPower) 定义在 apppower_io.h */
/* AppCooking_INPUT_LINK(AppCommMgr, AppCooking) 定义在 appcooking_io.h */
/* AppProtect_INPUT_LINK(AppCommMgr, AppProtect) 定义在 appprotect_io.h */



/* ========== INPUT (无) ========== */
/* 本模块没有输入 */

#endif /* APPCOMMMGR_IO_H */
