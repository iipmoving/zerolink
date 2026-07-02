/**
 * @file    elec_params_io.h
 * @brief   ElecParams Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   base_class
 *
 * 电参数计算 — 20ms 周期计算 I_peak/L/f_res/Q/R/P 等
 *
 * 输入源:
 *   Calculator → ElecParams  (Calculator → ElecParams: 电流积分/电压/过零点等中间结果)
 *
 * 输出目标:
 *   ElecParams → EKF_LKF  (ElecParams → EKF: 电参数观测值)
 *   ElecParams → AppPower  (ElecParams → AppPower: 电参数计算结果 (含阻抗/有效值))
 *   ElecParams → Telemetry  (ElecParams → Telemetry: 转发 ElecParams 输出实例)
 */

#ifndef ELECPARAMS_IO_H
#define ELECPARAMS_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * ElecParams → EKF_LKF  输出参数  (ElecParams → EKF: 电参数观测值)
 * ------------------------------------------------------------------ */
typedef struct {
    float   I_peak_A;      /**< 峰值电流 (A) — 中值滤波 + 权重修正后 */
    float   Vdc_mean;      /**< 母线电压均值 (V) — 20ms周期ADC值中值 */
    float   phi_deg;       /**< 相位角 (度) — 高电流周期平均 */
    float   f_sw_Hz;       /**< 开关频率 (Hz) — HRTIM_CLK/lowOff */
    float   L_uH;          /**< 等效电感 (μH) — 基波等效电路法 + 权重修正 */
    float   f_res_Hz;      /**< 谐振频率 (Hz) — f_res = 1/(2π√(LC)) */
    float   Q_factor;       /**< 品质因数 — Q = tan(φ)/(f_sw/f_res - f_res/f_sw) */
    float   R_ohm;         /**< 等效电阻 (Ω) — R = ωL/Q */
    float   I_rms;         /**< 电流有效值 (A) — I_rms = I_peak × √2/2 */
    float   P_W;           /**< 有功功率 (W) — P = median(I_active × Vdc) */
    float   Z_mag_ohm;     /**< 阻抗模 (Ω) — Z = √(R² + X²) */
    float   X_ohm;         /**< 净电抗 (Ω) — X = X_L - X_C */
    uint8_t valid;         /**< 计算有效性标志 — 1=有效, 0=无效 */
    uint8_t res[3];        /**< 保留字节，对齐填充 */
} MODULE_OUTPUT_PARAMS(ElecParams, EKF_LKF);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  count;            /* count */
    uint8_t  seq;              /* seq */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(ElecParams, EKF_LKF) params[4];
} MODULE_OUTPUT_LINK(ElecParams, EKF_LKF);

/* ------------------------------------------------------------------
 * ElecParams → Telemetry  输出参数  (转发 ElecParams 输出实例)
 * ------------------------------------------------------------------ */
/* 直接重命名 OUTPUT_PARAMS，不复述成员 */
typedef MODULE_OUTPUT_PARAMS(ElecParams, EKF_LKF) MODULE_OUTPUT_PARAMS(ElecParams, Telemetry);

/* 直接重命名 OUTPUT_LINK，不复述成员 */
typedef MODULE_OUTPUT_LINK(ElecParams, EKF_LKF) MODULE_OUTPUT_LINK(ElecParams, Telemetry);

/* ------------------------------------------------------------------
 * ElecParams → AppPower  输出参数  (ElecParams → AppPower: 电参数计算结果 (含阻抗/有效值))
 * ------------------------------------------------------------------ */
// typedef struct {
//     uint8_t valid;     /* 数据有效性 */
//     int32_t I_peak_A;     /* 峰值电流 (0.01A) */
//     int32_t Vdc_mean;     /* 母线电压均值 (0.01V) */
//     int32_t phi_deg;     /* 相位角 (0.01°) */
//     int32_t L_uH;     /* 等效电感 (0.01μH) */
//     int32_t f_res_Hz;      /* 谐振频率 (0.01Hz) */
//     int32_t R_ohm;     /* 等效电阻 (0.01Ω) */
//     int32_t f_sw_Hz;     /* 开关频率 (0.01Hz) */
//     int32_t P_W;     /* 有功功率 (0.01W) */
//     int32_t Q_factor;     /* 品质因数 (0.01) */
//     int32_t I_rms;     /* 电流有效值 (A) — I_rms = I_peak × √2/2 */
//     int32_t Z_mag_ohm;     /* 阻抗模 (Ω) — Z = √(R² + X²) */
//     int32_t X_ohm;     /* 净电抗 (Ω) — X = X_L - X_C */
// } MODULE_OUTPUT_PARAMS(ElecParams, AppPower);

// typedef struct {
//     uint8_t  status;           /* ST_NEW / ST_OUT */
//     uint8_t  count;            /* count */
//     uint8_t  seq;              /* seq */
//     uint8_t  res[1];
//     MODULE_OUTPUT_PARAMS(ElecParams, AppPower) params[4];
// } MODULE_OUTPUT_LINK(ElecParams, AppPower);

/* ElecParams_Output — 输出聚合 */
typedef struct {
    MODULE_OUTPUT_LINK(ElecParams, EKF_LKF)         *EKF_LKF_params;
    MODULE_OUTPUT_LINK(ElecParams, Telemetry)      *Telemetry_params;
    // MODULE_OUTPUT_LINK(ElecParams, AppPower)        *AppPower_params;
} MODULE_OUTPUT(ElecParams);

/* ==========================================================================
 * INPUT — 本模块输入的数据管道
 * ========================================================================== */

/* ------------------------------------------------------------------
 * Calculator → ElecParams  输入参数  (Calculator → ElecParams: 电流积分/电压/过零点等中间结果)
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
} MODULE_INPUT_PARAMS(Calculator, ElecParams);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  count;            /* count */
    uint8_t  seq;              /* seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(Calculator, ElecParams) params[4][20];
} MODULE_INPUT_LINK(Calculator, ElecParams);

/* ElecParams_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(Calculator, ElecParams) *Calculator_params;  /* 指向 Calculator 输出的 LINK 列 */
} MODULE_INPUT(ElecParams);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(ElecParams);

#endif /* ELECPARAMS_IO_H */
