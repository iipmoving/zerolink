/**
 * drvkey_io.h —— DrvKey 输入输出接口定义 (v2.3)
 *
 * 数据流:
 *   OUTPUT: DrvKey → AppHmi
 *   OUTPUT: DrvKey → AppCooking
 *   OUTPUT: DrvKey → AppSegAlign

 *
 * Include 权限:
 *   - 仅 drvkey.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/drvkey_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef DRVKEY_IO_H
#define DRVKEY_IO_H

#include "../core/std_module.h"
#include <stdint.h>


/* ========== OUTPUT (DrvKey 给别的模块提供的数据) ========== */

/* DrvKey_to_AppHmi_Params — 输出给 AppHmi 的数据 */
typedef struct {
    // TODO: 添加输出字段
    uint8_t  res[4];
} MODULE_OUTPUT_PARAMS(DrvKey, AppHmi);
/* DrvKey_to_AppCooking_Params — 输出给 AppCooking 的数据 */
typedef struct {
    // TODO: 添加输出字段
    uint8_t  res[4];
} MODULE_OUTPUT_PARAMS(DrvKey, AppCooking);
/* DrvKey_to_AppSegAlign_Params — 输出给 AppSegAlign 的数据 */
typedef struct {
    // TODO: 添加输出字段
    uint8_t  res[4];
} MODULE_OUTPUT_PARAMS(DrvKey, AppSegAlign);


/* DrvKey_to_AppHmi_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大数量 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(DrvKey, AppHmi) *params;
} MODULE_OUTPUT_LINK(DrvKey, AppHmi);
/* DrvKey_to_AppCooking_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大数量 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(DrvKey, AppCooking) *params;
} MODULE_OUTPUT_LINK(DrvKey, AppCooking);
/* DrvKey_to_AppSegAlign_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大数量 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(DrvKey, AppSegAlign) *params;
} MODULE_OUTPUT_LINK(DrvKey, AppSegAlign);


/* DrvKey_Output — 输出聚合 (v2.3 对称命名: 成员名 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(DrvKey, AppHmi) *AppHmi_params;  /* → AppHmi */
    MODULE_OUTPUT_LINK(DrvKey, AppCooking) *AppCooking_params;  /* → AppCooking */
    MODULE_OUTPUT_LINK(DrvKey, AppSegAlign) *AppSegAlign_params;  /* → AppSegAlign */

} MODULE_OUTPUT(DrvKey);

/* ========== Consumer INPUT_LINK 定义在 Consumer 的 io.h ========== */
/* AppHmi_INPUT_LINK(DrvKey, AppHmi) 定义在 apphmi_io.h */
/* AppCooking_INPUT_LINK(DrvKey, AppCooking) 定义在 appcooking_io.h */
/* AppSegAlign_INPUT_LINK(DrvKey, AppSegAlign) 定义在 appsegalign_io.h */



/* ========== INPUT (无) ========== */
/* 本模块没有输入 */

#endif /* DRVKEY_IO_H */
