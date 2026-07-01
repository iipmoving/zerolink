/**
 * @file    calculator_io.h
 * @brief   Calculator Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   base_class
 *
 * 功率计算器 — 从 ADC 原始数据计算谐振电流/相位角/电压
 *
 * 输入源:
 *   AppAdc → Calculator  (ADC → Calculator: 谐振电流/HRTIM 时序原始数据)
 *
 * 输出目标:
 *   Calculator → ElecParams  (Calculator → ElecParams: 电流积分/电压/过零点等中间结果)
 *   Calculator → AppPower  (Calculator → AppPower: 20ms 周期累积平均值 (谐振电流/电压/相位角))
 */

#ifndef CALCULATOR_IO_H
#define CALCULATOR_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * Calculator → ElecParams  输出参数  (Calculator → ElecParams: 电流积分/电压/过零点等中间结果)
 * ------------------------------------------------------------------ */
typedef struct {
    struct {  /* HRTIM 时序参数 */
        uint16_t highOff;     /* 上管关断 HRTIM 值 */
        uint16_t lowOff;     /* 下管关断 HRTIM 值 */
        uint16_t highOn;     /* 上管开通 HRTIM 值 */
        uint16_t lowOn;     /* 下管开通 HRTIM 值 */
    } hrtim;
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
    MODULE_OUTPUT_PARAMS(Calculator, ElecParams) params[80];
} MODULE_OUTPUT_LINK(Calculator, ElecParams);

/* ------------------------------------------------------------------
 * Calculator → AppPower  输出参数  (Calculator → AppPower: 20ms 周期累积平均值 (谐振电流/电压/相位角))
 * ------------------------------------------------------------------ */
typedef struct {
    int32_t resonant_current;     /* 谐振电流均值 (0.01A) */
    int32_t voltage;     /* 母线电压均值 (0.01V) */
    int32_t phase_angle;     /* 相位角 (0.01°) */
    uint8_t valid;     /* 数据有效性 */
} MODULE_OUTPUT_PARAMS(Calculator, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(Calculator, AppPower) params[4];
} MODULE_OUTPUT_LINK(Calculator, AppPower);

/* Calculator_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(Calculator, ElecParams)  ElecParams_params;  /* → ElecParams */
    MODULE_OUTPUT_LINK(Calculator, AppPower)  AppPower_params;  /* → AppPower */
} MODULE_OUTPUT(Calculator);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppAdc → Calculator  输入参数  (ADC → Calculator: 谐振电流/HRTIM 时序原始数据)
 * ------------------------------------------------------------------ */
typedef struct {
    uint16_t** resonant_current;     /* 谐振电流 ADC 数组指针数组 */
    uint16_t** hrtim_values;     /* HRTIM 时间戳数组指针数组 */
    uint16_t** voltage_data;     /* 母线电压数组指针数组 */
    AppAdc_OutputParams_t* input;     /* 每通道 HRTIM 工作周期参数 (外部定义类型) */
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

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(Calculator);

#endif /* CALCULATOR_IO_H */
