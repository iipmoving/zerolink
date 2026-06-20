/**
 * @file    app_adc_io.h
 * @brief   AppAdc Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * ADC 采样 — 输出谐振电流/电压/HRTIM 时序到 Calculator 和功率参数到 AppPower
 *
 * 输入源:
 *   Calculator → AppAdc  (Calculator → AppAdc: 20ms 周期累积平均值 (谐振电流/电压/相位角))
 *
 * 输出目标:
 *   AppAdc → AppPower  (ADC → Power: 电压/电流/相位等功率控制参数)
 *   AppAdc → Calculator  (ADC → Calculator: 谐振电流/HRTIM 时序原始数据)
 */

#ifndef APPADC_IO_H
#define APPADC_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppAdc → AppPower  输出参数  (ADC → Power: 电压/电流/相位等功率控制参数)
 * ------------------------------------------------------------------ */
typedef struct {
    uint16_t voltage;     /* 母线电压 ADC */
    uint16_t current;     /* 每炉头谐振电流 ADC */
    uint16_t igbt;
    uint16_t bottom;
    uint16_t phase;     /* 每炉头相位 ADC */
    uint16_t res;
} MODULE_OUTPUT_PARAMS(AppAdc, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppAdc, AppPower) params[4];
} MODULE_OUTPUT_LINK(AppAdc, AppPower);

/* ------------------------------------------------------------------
 * AppAdc → Calculator  输出参数  (ADC → Calculator: 谐振电流/HRTIM 时序原始数据)
 * ------------------------------------------------------------------ */
typedef struct {
    uint16_t** resonant_current;     /* 谐振电流 ADC 数组指针数组 */
    uint16_t** hrtim_values;     /* HRTIM 时间戳数组指针数组 */
    uint16_t** voltage_data;     /* 母线电压数组指针数组 */
    AppAdc_OutputParams_t* input;     /* 每通道 HRTIM 工作周期参数 (AppAdc 侧独立 typedef) */
} MODULE_OUTPUT_PARAMS(AppAdc, Calculator);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppAdc, Calculator) *params;
} MODULE_OUTPUT_LINK(AppAdc, Calculator);

/* AppAdc_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppAdc, AppPower)  AppPower_params;  /* → AppPower */
    MODULE_OUTPUT_LINK(AppAdc, Calculator)  Calculator_params;  /* → Calculator */
} MODULE_OUTPUT(AppAdc);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * Calculator → AppAdc  输入参数  (Calculator → AppAdc: 20ms 周期累积平均值 (谐振电流/电压/相位角))
 * ------------------------------------------------------------------ */
typedef struct {
    int32_t resonant_current;     /* 谐振电流均值 (0.01A) */
    int32_t voltage;     /* 母线电压均值 (0.01V) */
    int32_t phase_angle;     /* 相位角 (0.01°) */
} MODULE_INPUT_PARAMS(Calculator, AppAdc);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(Calculator, AppAdc) params[4];
} MODULE_INPUT_LINK(Calculator, AppAdc);

/* AppAdc_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(Calculator, AppAdc) *Calculator_params;  /* 指向 Calculator 输出的 LINK 列 */
} MODULE_INPUT(AppAdc);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(AppAdc);

#endif /* APPADC_IO_H */
