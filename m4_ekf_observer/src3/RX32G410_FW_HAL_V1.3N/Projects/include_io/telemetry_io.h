/**
 * @file    telemetry_io.h
 * @brief   Telemetry I/O (v2.3 LINK+PARAMS)
 * @layer   base_class
 *
 * 数据流遥测 — 从 Calculator/ElecParams 转发数据到 MODBUS Area 5
 *   管道1: Calculator → Telemetry (指针，转发 Calculator 输入副本)
 *   管道2: ElecParams → Telemetry (指针，转发 ElecParams 输出实例)
 *
 * 输入:
 *   Calculator (通过 Calculator 输出的 Telemetry 管道转发)
 *   ElecParams (通过 ElecParams 输出的 Telemetry 管道转发)
 * 输出: 无 (纯捕获，MODBUS Area 5 映射内部缓冲区)
 *
 * 注意: INPUT_PARAMS / INPUT_LINK 布局与 producer 的 OUTPUT_PARAMS /
 *       OUTPUT_LINK 完全一致，独立 typedef，不跨模块 #include。
 *       管道 status 位共享同一实例，由生产端统一清除 ST_NEW。
 */

#ifndef TELEMETRY_IO_H
#define TELEMETRY_IO_H

#include <stdint.h>
#include "std_module.h"

/* ==========================================================================
 * 输入 — 管道1: Calculator → Telemetry (转发 Calculator 输入副本)
 * ========================================================================== */

/* 数据参数: 布局与 MODULE_OUTPUT_PARAMS(Calculator, ElecParams) 一致 */
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
} MODULE_INPUT_PARAMS(Calculator, Telemetry);

/* 输入管道: 布局与 MODULE_OUTPUT_LINK(Calculator, Telemetry) 一致 */
typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  count;            /* seq */
    uint8_t  seq;            /* 周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(Calculator, Telemetry) params[4][20];
} MODULE_INPUT_LINK(Calculator, Telemetry);

/* ==========================================================================
 * 输入 — 管道2: ElecParams → Telemetry (转发 ElecParams 输出实例)
 * ========================================================================== */

/* 数据参数: 布局与 MODULE_OUTPUT_PARAMS(ElecParams, EKF_LKF) 一致 */
typedef struct {
    float   I_peak_A;      /**< 峰值电流 (A) — 中值滤波 + 权重修正后 */
    float   Vdc_mean;      /**< 母线电压均值 (V) — 20ms周期ADC值中值 */
    float   phi_deg;       /**< 相位角 (度) — 高电流周期平均 */
    float   f_sw_Hz;       /**< 开关频率 (Hz) — HRTIM_CLK/lowOff */
    float   L_uH;          /**< 等效电感 (μH) — 基波等效电路法 + 权重修正 */
    float   f_res_Hz;     /**< 谐振频率 (Hz) — f_res = 1/(2π√(LC)) */
    float   Q_factor;       /**< 品质因数 — Q = tan(φ)/(f_sw/f_res - f_res/f_sw) */
    float   R_ohm;         /**< 等效电阻 (Ω) — R = ωL/Q */
    float   I_rms;         /**< 电流有效值 (A) — I_rms = I_peak × √2/2 */
    float   P_W;           /**< 有功功率 (W) — P = median(I_active × Vdc) */
    float   Z_mag_ohm;     /**< 阻抗模 (Ω) — Z = √(R² + X²) */
    float   X_ohm;         /**< 净电抗 (Ω) — X = X_L - X_C */
    uint8_t valid;         /**< 计算有效性标志 — 1=有效, 0=无效 */
    uint8_t res[3];        /**< 保留字节，对齐填充 */
} MODULE_INPUT_PARAMS(ElecParams, Telemetry);

/* 输入管道: 布局与 MODULE_OUTPUT_LINK(ElecParams, EKF_LKF) 一致 */
typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  count;            /* seq */
    uint8_t  seq;            /* 周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(ElecParams, Telemetry) params[4];
} MODULE_INPUT_LINK(ElecParams, Telemetry);

/* ==========================================================================
 * 输入聚合
 * ========================================================================== */
typedef struct {
    MODULE_INPUT_LINK(Calculator, Telemetry) *Calculator_params;  /* 指向 Calculator 输出的 Telemetry 管道 */
    MODULE_INPUT_LINK(ElecParams, Telemetry) *ElecParams_params;  /* 指向 ElecParams 输出的 Telemetry 管道 */
} MODULE_INPUT(Telemetry);

/* ==========================================================================
 * 输出 — 本模块没有输出
 * ========================================================================== */
typedef struct {
    uint8_t  res[4];
} MODULE_OUTPUT(Telemetry);

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(Telemetry);

#endif /* TELEMETRY_IO_H */
