/**
 * apphmi_io.h —— AppHmi 输入输出接口定义 (v2.3)
 *
 * 数据流:
 *   INPUT: DrvKey → AppHmi
 *   INPUT: AppPower → AppHmi
 *   OUTPUT: AppHmi → DrvDisplay
 *   OUTPUT: AppHmi → DrvBuzzer

 *
 * Include 权限:
 *   - 仅 apphmi.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/apphmi_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef APPHMI_IO_H
#define APPHMI_IO_H

#include "../core/std_module.h"
#include <stdint.h>


/* ========== OUTPUT (AppHmi 给别的模块提供的数据) ========== */

/* AppHmi_to_DrvDisplay_Params — 输出给 DrvDisplay 的数据 */
typedef struct {
    // TODO: 添加输出字段
    uint8_t  res[4];
} MODULE_OUTPUT_PARAMS(AppHmi, DrvDisplay);
/* AppHmi_to_DrvBuzzer_Params — 输出给 DrvBuzzer 的数据 */
typedef struct {
    // TODO: 添加输出字段
    uint8_t  res[4];
} MODULE_OUTPUT_PARAMS(AppHmi, DrvBuzzer);


/* AppHmi_to_DrvDisplay_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大数量 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppHmi, DrvDisplay) *params;
} MODULE_OUTPUT_LINK(AppHmi, DrvDisplay);
/* AppHmi_to_DrvBuzzer_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;        /* ST_NEW/ST_OUT */
    uint8_t  max_count;     /* 最大数量 */
    uint8_t  count;         /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppHmi, DrvBuzzer) *params;
} MODULE_OUTPUT_LINK(AppHmi, DrvBuzzer);


/* AppHmi_Output — 输出聚合 (v2.3 对称命名: 成员名 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppHmi, DrvDisplay) *DrvDisplay_params;  /* → DrvDisplay */
    MODULE_OUTPUT_LINK(AppHmi, DrvBuzzer) *DrvBuzzer_params;  /* → DrvBuzzer */

} MODULE_OUTPUT(AppHmi);

/* ========== Consumer INPUT_LINK 定义在 Consumer 的 io.h ========== */
/* DrvDisplay_INPUT_LINK(AppHmi, DrvDisplay) 定义在 drvdisplay_io.h */
/* DrvBuzzer_INPUT_LINK(AppHmi, DrvBuzzer) 定义在 drvbuzzer_io.h */




/* ========== INPUT (AppHmi 从别的模块得到的数据) ========== */

/* DrvKey_to_AppHmi_Input_Params — 从 DrvKey 得到的数据 */
typedef struct {
    // TODO: 添加输入字段（与 DrvKey OUTPUT 布局一致）
    uint8_t  res[4];
} MODULE_INPUT_PARAMS(DrvKey, AppHmi);
/* AppPower_to_AppHmi_Input_Params — 从 AppPower 得到的数据 */
typedef struct {
    // TODO: 添加输入字段（与 AppPower OUTPUT 布局一致）
    uint8_t  res[4];
} MODULE_INPUT_PARAMS(AppPower, AppHmi);


/* DrvKey_to_AppHmi_Input_Link — 输入管道 (与 DrvKey OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(DrvKey, AppHmi) *params;
} MODULE_INPUT_LINK(DrvKey, AppHmi);
/* AppPower_to_AppHmi_Input_Link — 输入管道 (与 AppPower OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppPower, AppHmi) *params;
} MODULE_INPUT_LINK(AppPower, AppHmi);


/* AppHmi_Input — 输入聚合 (v2.3 对称命名: 成员名 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(DrvKey, AppHmi) *DrvKey_params;  /* 从 DrvKey 得到 */
    MODULE_INPUT_LINK(AppPower, AppHmi) *AppPower_params;  /* 从 AppPower 得到 */

} MODULE_INPUT(AppHmi);


#endif /* APPHMI_IO_H */
