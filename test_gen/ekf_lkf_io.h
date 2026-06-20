/**
 * @file    ekf_lkf_io.h
 * @brief   EKF_LKF Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   base_class
 *
 * 扩展卡尔曼滤波观测器 — 从 ElecParams 读取观测值，估计 [R, L, f_res]
 *
 * 输入源:
 *   ElecParams → EKF_LKF  (ElecParams → EKF: 电参数观测值)
 *
 * 输出目标:
 *   EKF_LKF → AppPower  (EKF → Power: 卡尔曼滤波后的负载参数 (R/L/f_res))
 */

#ifndef EKF_LKF_IO_H
#define EKF_LKF_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * EKF_LKF → AppPower  输出参数  (EKF → Power: 卡尔曼滤波后的负载参数 (R/L/f_res))
 * ------------------------------------------------------------------ */
typedef struct {
    int32_t I_peak_A;     /* 峰值电流 (A) — 中值滤波 + 权重修正后 */
    int32_t Vdc_mean;     /* 母线电压均值 (V) — 20ms周期ADC值中值 */
    int32_t phi_deg;     /* 相位角 (度) — 高电流周期平均 */
    int32_t f_sw_Hz;     /* 开关频率 (Hz) — HRTIM_CLK/lowOff */
    int32_t L_uH;     /* 等效电感 (μH) — 基波等效电路法 + 权重修正 */
    int32_t f_res_kHz;     /* 谐振频率 (kHz) — f_res = 1/(2π√(LC)) */
    int32_t Q_factor;     /* 品质因数 — Q = tan(φ)/(f_sw/f_res - f_res/f_sw) */
    int32_t R_ohm;     /* 等效电阻 (Ω) — R = ωL/Q */
    int32_t I_rms;     /* 电流有效值 (A) — I_rms = I_peak × √2/2 */
    int32_t P_W;     /* 有功功率 (W) — P = median(I_active × Vdc) */
    int32_t Z_mag_ohm;     /* 阻抗模 (Ω) — Z = √(R² + X²) */
    int32_t X_ohm;     /* 净电抗 (Ω) — X = X_L - X_C */
    uint8_t valid;     /* 计算有效性标志 — 1=有效, 0=无效 */
    uint8_t res[3];     /* 保留字节，对齐填充 */
} MODULE_OUTPUT_PARAMS(EKF_LKF, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(EKF_LKF, AppPower) params[4];
} MODULE_OUTPUT_LINK(EKF_LKF, AppPower);

/* EKF_LKF_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(EKF_LKF, AppPower)  AppPower_params;  /* → AppPower */
} MODULE_OUTPUT(EKF_LKF);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * ElecParams → EKF_LKF  输入参数  (ElecParams → EKF: 电参数观测值)
 * ------------------------------------------------------------------ */
typedef struct {
    float I_peak_A;     /* 峰值电流 (A) — 中值滤波 + 权重修正后 */
    float Vdc_mean;     /* 母线电压均值 (V) — 20ms周期ADC值中值 */
    float phi_deg;     /* 相位角 (度) — 高电流周期平均 */
    float f_sw_Hz;     /* 开关频率 (Hz) — HRTIM_CLK/lowOff */
    float L_uH;     /* 等效电感 (μH) — 基波等效电路法 + 权重修正 */
    float f_res_kHz;     /* 谐振频率 (kHz) — f_res = 1/(2π√(LC)) */
    float Q_factor;     /* 品质因数 — Q = tan(φ)/(f_sw/f_res - f_res/f_sw) */
    float R_ohm;     /* 等效电阻 (Ω) — R = ωL/Q */
    float I_rms;     /* 电流有效值 (A) — I_rms = I_peak × √2/2 */
    float P_W;     /* 有功功率 (W) — P = median(I_active × Vdc) */
    float Z_mag_ohm;     /* 阻抗模 (Ω) — Z = √(R² + X²) */
    float X_ohm;     /* 净电抗 (Ω) — X = X_L - X_C */
    uint8_t valid;     /* 计算有效性标志 — 1=有效, 0=无效 */
    uint8_t res[3];     /* 保留字节，对齐填充 */
} MODULE_INPUT_PARAMS(ElecParams, EKF_LKF);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(ElecParams, EKF_LKF) params[4];
} MODULE_INPUT_LINK(ElecParams, EKF_LKF);

/* EKF_LKF_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(ElecParams, EKF_LKF) *ElecParams_params;  /* 指向 ElecParams 输出的 LINK 列 */
} MODULE_INPUT(EKF_LKF);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(EKF_LKF);

#endif /* EKF_LKF_IO_H */
