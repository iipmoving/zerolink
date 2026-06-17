/**
 * apppower_io.h —— AppPower 输入输出接口定义 (v2.3)
 *
 * 数据流:
 *   INPUT: AppCommMgr → AppPower
 *   INPUT: AppProtect → AppPower
 *   INPUT: AppCooking → AppPower
 *   OUTPUT: AppPower → DrvCommMgr
 *   OUTPUT: AppPower → AppHmi

 *
 * Include 权限:
 *   - 仅 apppower.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/apppower_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef APPPOWER_IO_H
#define APPPOWER_IO_H

#include "../core/std_module.h"
#include <stdint.h>


/* ========== OUTPUT (AppPower 给别的模块提供的数据) ========== */

/* AppPower_to_DrvCommMgr_Params — 输出给 DrvCommMgr 的数据 */
typedef struct {
    // TODO: 添加输出字段
    uint8_t  res[4];
} MODULE_OUTPUT_PARAMS(AppPower, DrvCommMgr);
/* AppPower_to_AppHmi_Params — 输出给 AppHmi 的数据 */
typedef struct {
    // TODO: 添加输出字段
    uint8_t  res[4];
} MODULE_OUTPUT_PARAMS(AppPower, AppHmi);


/* AppPower_to_DrvCommMgr_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大数量 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppPower, DrvCommMgr) *params;
} MODULE_OUTPUT_LINK(AppPower, DrvCommMgr);
/* AppPower_to_AppHmi_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大数量 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppPower, AppHmi) *params;
} MODULE_OUTPUT_LINK(AppPower, AppHmi);


/* AppPower_Output — 输出聚合 (v2.3 对称命名: 成员名 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppPower, DrvCommMgr) *DrvCommMgr_params;  /* → DrvCommMgr */
    MODULE_OUTPUT_LINK(AppPower, AppHmi) *AppHmi_params;  /* → AppHmi */

} MODULE_OUTPUT(AppPower);

/* ========== Consumer INPUT_LINK 定义在 Consumer 的 io.h ========== */
/* DrvCommMgr_INPUT_LINK(AppPower, DrvCommMgr) 定义在 drvcommmgr_io.h */
/* AppHmi_INPUT_LINK(AppPower, AppHmi) 定义在 apphmi_io.h */




/* ========== INPUT (AppPower 从别的模块得到的数据) ========== */

/* AppCommMgr_to_AppPower_Input_Params — 从 AppCommMgr 得到的数据 */
typedef struct {
    // TODO: 添加输入字段（与 AppCommMgr OUTPUT 布局一致）
    uint8_t  res[4];
} MODULE_INPUT_PARAMS(AppCommMgr, AppPower);
/* AppProtect_to_AppPower_Input_Params — 从 AppProtect 得到的数据 */
typedef struct {
    // TODO: 添加输入字段（与 AppProtect OUTPUT 布局一致）
    uint8_t  res[4];
} MODULE_INPUT_PARAMS(AppProtect, AppPower);
/* AppCooking_to_AppPower_Input_Params — 从 AppCooking 得到的数据 */
typedef struct {
    // TODO: 添加输入字段（与 AppCooking OUTPUT 布局一致）
    uint8_t  res[4];
} MODULE_INPUT_PARAMS(AppCooking, AppPower);


/* AppCommMgr_to_AppPower_Input_Link — 输入管道 (与 AppCommMgr OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppCommMgr, AppPower) *params;
} MODULE_INPUT_LINK(AppCommMgr, AppPower);
/* AppProtect_to_AppPower_Input_Link — 输入管道 (与 AppProtect OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppProtect, AppPower) *params;
} MODULE_INPUT_LINK(AppProtect, AppPower);
/* AppCooking_to_AppPower_Input_Link — 输入管道 (与 AppCooking OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppCooking, AppPower) *params;
} MODULE_INPUT_LINK(AppCooking, AppPower);


/* AppPower_Input — 输入聚合 (v2.3 对称命名: 成员名 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppCommMgr, AppPower) *AppCommMgr_params;  /* 从 AppCommMgr 得到 */
    MODULE_INPUT_LINK(AppProtect, AppPower) *AppProtect_params;  /* 从 AppProtect 得到 */
    MODULE_INPUT_LINK(AppCooking, AppPower) *AppCooking_params;  /* 从 AppCooking 得到 */

} MODULE_INPUT(AppPower);


#endif /* APPPOWER_IO_H */
