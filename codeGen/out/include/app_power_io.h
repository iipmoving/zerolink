/**
 * @file    app_power_io.h
 * @brief   AppPower Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * 功率控制 PID — 从 AppAdc 拉取数据，输出功率增量到 DrvHrtim
 *
 * 输入源:
 *   AppAdc → AppPower  (ADC → Power: 电压/电流/相位等功率控制参数)
 *   Calculator → AppPower  (Calculator → AppPower: 20ms 周期累积平均值 (谐振电流/电压/相位角))
 */

#ifndef APPPOWER_IO_H
//#define APPPOWER_IO_H   /* L0 阻断: 禁用 include guard */

#include <stdint.h>
#include "../src/RX32G410_FW_HAL_V1.3N/Projects/core/std_module.h"

#pragma pack(4)

/* ========== OUTPUT (无) — 本模块没有输出 ========== */

/* AppPower_Output — 无输出 */
typedef struct {
    uint8_t  res[4];
} MODULE_OUTPUT(AppPower);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppAdc → AppPower  输入参数  (ADC → Power: 电压/电流/相位等功率控制参数)
 * ------------------------------------------------------------------ */
typedef struct {
    uint16_t voltage;     /* 母线电压 ADC */
    uint16_t power[4];     /* 每炉头功率 ADC */
    uint16_t txa[4];     /* 每炉头谐振电流 ADC */
    uint16_t phase[4];     /* 每炉头相位 ADC */
} MODULE_INPUT_PARAMS(AppAdc, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppAdc, AppPower) *params;
} MODULE_INPUT_LINK(AppAdc, AppPower);

/* ------------------------------------------------------------------
 * Calculator → AppPower  输入参数  (Calculator → AppPower: 20ms 周期累积平均值 (谐振电流/电压/相位角))
 * ------------------------------------------------------------------ */
typedef struct {
    int32_t resonant_current;     /* 谐振电流均值 (0.01A) */
    int32_t voltage;     /* 母线电压均值 (0.01V) */
    int32_t phase_angle;     /* 相位角 (0.01°) */
    uint8_t valid;     /* 数据有效性 */
} MODULE_INPUT_PARAMS(Calculator, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(Calculator, AppPower) *params;
} MODULE_INPUT_LINK(Calculator, AppPower);

/* AppPower_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppAdc, AppPower) *AppAdc_params;  /* 指向 AppAdc 输出的 LINK 列 */
    MODULE_INPUT_LINK(Calculator, AppPower) *Calculator_params;  /* 指向 Calculator 输出的 LINK 列 */
} MODULE_INPUT(AppPower);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(AppPower);

#endif /* APPPOWER_IO_H */
