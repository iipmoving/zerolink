/**
 * @file    calculator_io.h
 * @brief   Calculator Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   base_class
 *
 * Calculator Data Switcher IO interface (v2.3 LINK+PARAMS)
 *
 * 输入源:
 *   AppAdc → Calculator  ()
 *
 * 输出目标:
 *   Calculator → AppAdc  ()
 *   Calculator → ElecParams  ()
 */

#ifndef CALCULATOR_IO_H
#define CALCULATOR_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * Calculator → AppAdc  输出参数  ()
 * ------------------------------------------------------------------ */
typedef struct {
    int32_t voltage;     /* 母线电压均值 (0.01V) */
    int32_t phase_angle;     /* 相位角 (0.01°) */
} MODULE_OUTPUT_PARAMS(Calculator, AppAdc);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(Calculator, AppAdc) *params;
} MODULE_OUTPUT_LINK(Calculator, AppAdc);

/* ------------------------------------------------------------------
 * Calculator → ElecParams  输出参数  ()
 * ------------------------------------------------------------------ */
typedef struct {
    uint16_t hrtim_highOff;     /* 上管关断 HRTIM 值 */
    uint16_t hrtim_lowOff;     /* 下管关断 HRTIM 值 */
    uint16_t hrtim_highOn;     /* 上管开通 HRTIM 值 */
    uint16_t hrtim_lowOn;     /* 下管开通 HRTIM 值 */
    uint16_t peak_current;     /* 峰值电流 ADC */
    uint32_t active_current_sum_high;     /* 上管电流积分和 */
    uint32_t active_current_sum_low;     /* 下管电流积分和 */
    uint32_t voltage_sum;     /* 电压原始累加 */
    uint16_t voltage_count;     /* 电压累加计数 */
    uint16_t zero_cross_high;     /* 上管过零点 */
    uint16_t zero_cross_low;     /* 下管过零点 */
    uint16_t peak_point;     /* 峰值点 HRTIM 值 */
} MODULE_OUTPUT_PARAMS(Calculator, ElecParams);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(Calculator, ElecParams) *params;
} MODULE_OUTPUT_LINK(Calculator, ElecParams);

/* Calculator_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(Calculator, AppAdc)  AppAdc_params;  /* → AppAdc */
    MODULE_OUTPUT_LINK(Calculator, ElecParams)  ElecParams_params;  /* → ElecParams */
} MODULE_OUTPUT(Calculator);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppAdc → Calculator  输入参数  ()
 * ------------------------------------------------------------------ */
typedef struct {
    uint16_t** hrtim_values;     /* HRTIM 时间戳数组指针数组 */
    uint16_t** voltage_data;     /* 母线电压数组指针数组 */
    AppAdc_OutputParams_t* input;     /* 每通道 HRTIM 工作周期参数 (AppAdc 侧独立 typedef) */
} MODULE_INPUT_PARAMS(AppAdc, Calculator);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppAdc, Calculator) *params;
} MODULE_INPUT_LINK(AppAdc, Calculator);

/* Calculator_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppAdc, Calculator) *AppAdc_params;  /* 指向 AppAdc 输出的 LINK 列 */
} MODULE_INPUT(Calculator);
// ===== [END AI GENERATED] =====

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(Calculator);

#endif /* CALCULATOR_IO_H */
