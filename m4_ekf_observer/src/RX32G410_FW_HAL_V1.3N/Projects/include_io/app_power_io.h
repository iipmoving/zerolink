/**
 * @file    app_power_io.h
 * @brief   AppPower Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * 功率控制 PID — 从 AppAdc/Calculator/EKF_LKF 拉取数据
 *
 * 输入源:
 *   AppAdc → AppPower  (ADC → Power: 电压/电流/相位等功率控制参数)
 *   Calculator → AppPower  (Calculator → AppPower: 20ms 周期累积平均值 (谐振电流/电压/相位角))
 *   ElecParams → AppPower  (ElecParams → AppPower: 电参数计算结果 (含阻抗/有效值))
 *   EKF_LKF → AppPower  (EKF → Power: 卡尔曼滤波后的负载参数 (R/L/f_res))
 */

#ifndef APPPOWER_IO_H
#define APPPOWER_IO_H

#include <stdint.h>
#include "std_module.h"

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
    uint16_t current;     /* 每炉头谐振电流 ADC */
		uint16_t igbt;
		uint16_t bottom;	
    uint16_t phase;     /* 每炉头相位 ADC */
		uint16_t res;
} MODULE_INPUT_PARAMS(AppAdc, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppAdc, AppPower) params[4];
} MODULE_INPUT_LINK(AppAdc, AppPower);






/* ------------------------------------------------------------------
 * ElecParams → AppPower  输入参数  (ElecParams → AppPower: 电参数计算结果 (含阻抗/有效值))
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t valid;     /* 数据有效性 */
    int32_t I_peak_A;     /* 峰值电流 (0.01A) */
    int32_t Vdc_mean;     /* 母线电压均值 (0.01V) */
    int32_t phi_deg;     /* 相位角 (0.01°) */
    int32_t L_uH;     /* 等效电感 (0.01μH) */
    int32_t f_res_Hz;     /* 谐振频率 (0.01Hz) */
    int32_t R_ohm;     /* 等效电阻 (0.01Ω) */
    int32_t f_sw_Hz;     /* 开关频率 (0.01Hz) */
    int32_t P_W;     /* 有功功率 (0.01W) */
    int32_t Q_factor;     /* 品质因数 (0.01) */
    int32_t I_rms;     /* 电流有效值 (A) — I_rms = I_peak × √2/2 */
    int32_t Z_mag_ohm;     /* 阻抗模 (Ω) — Z = √(R² + X²) */
    int32_t X_ohm;     /* 净电抗 (Ω) — X = X_L - X_C */
} MODULE_INPUT_PARAMS(ElecParams, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(ElecParams, AppPower) params[4];
} MODULE_INPUT_LINK(ElecParams, AppPower);

/* ------------------------------------------------------------------
 * EKF_LKF → AppPower  输入参数  (EKF → Power: 卡尔曼滤波后的负载参数 (R/L/f_res))
 * ------------------------------------------------------------------ */
typedef struct {
    int32_t   I_peak_A;      /**< 峰值电流 (A) — 中值滤波 + 权重修正后 */
    int32_t   Vdc_mean;      /**< 母线电压均值 (V) — 20ms周期ADC值中值 */
    int32_t   phi_deg;       /**< 相位角 (度) — 高电流周期平均 */
    int32_t   f_sw_Hz;       /**< 开关频率 (Hz) — HRTIM_CLK/lowOff */
    int32_t   L_uH;          /**< 等效电感 (μH) — 基波等效电路法 + 权重修正 */
    int32_t   f_res_kHz;     /**< 谐振频率 (kHz) — f_res = 1/(2π√(LC)) */
    int32_t   Q_factor;       /**< 品质因数 — Q = tan(φ)/(f_sw/f_res - f_res/f_sw) */
    int32_t   R_ohm;         /**< 等效电阻 (Ω) — R = ωL/Q */
    int32_t   I_rms;         /**< 电流有效值 (A) — I_rms = I_peak × √2/2 */
    int32_t   P_W;           /**< 有功功率 (W) — P = median(I_active × Vdc) */
    int32_t   Z_mag_ohm;     /**< 阻抗模 (Ω) — Z = √(R² + X²) */
    int32_t   X_ohm;         /**< 净电抗 (Ω) — X = X_L - X_C */
    uint8_t valid;         /**< 计算有效性标志 — 1=有效, 0=无效 */
    uint8_t res[3];        /**< 保留字节，对齐填充 */
} MODULE_INPUT_PARAMS(EKF_LKF, AppPower);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(EKF_LKF, AppPower) params[4];
} MODULE_INPUT_LINK(EKF_LKF, AppPower);

/* AppPower_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppAdc, AppPower) *AppAdc_params;  /* 指向 AppAdc 输出的 LINK 列 */
    MODULE_INPUT_LINK(EKF_LKF, AppPower) *EKF_LKF_params;  /* 指向 EKF_LKF 输出的 LINK 列 */
} MODULE_INPUT(AppPower);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(AppPower);

#endif /* APPPOWER_IO_H */

