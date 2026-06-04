/********************************************************************************
    FileName    :  ih_elec_params.h
    Brief       :  半桥IH电参数计算 — 20ms 汇总模块
                  输入 4 组 uint16_t[20] + 导通占比,
                  输出 L/f_res/Q/R/P/Z + Kalman 双通道.
                  与 MATLAB export_golden.m 公式链逐行对齐.
    Date        :  2026-06-04
********************************************************************************/
#ifndef IH_ELEC_PARAMS_H
#define IH_ELEC_PARAMS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 标定常数 ================================================== */
#define IH_C_FARAD          0.9e-6f     /* 谐振电容 0.9 μF */
#define IH_C_UF             0.9f

#define IH_I_SCALE          0.02523f    /* I_adc → A */
#define IH_VDC_SCALE        0.10606f    /* Vdc_adc → V */
#define IH_PHI_SCALE        0.1f        /* phi 单位 0.1° → ° */
#define IH_FSW_SCALE        1.0f        /* f_sw 单位 Hz */

#define IH_I_PEAK_MIN_RATIO 0.3f        /* I_peak < 中位数×0.3 丢弃 */
#define IH_L_MIN_uH        10.0f        /* 有效 L 下限 (μH) */
#define IH_FRES_MIN_kHz    5.0f         /* 有效 f_res 下限 (kHz) */

/* ========== 输入 ====================================================== */
typedef struct {
    uint16_t* I_peak_buf;   /* [20] 谐振电流峰值 ADC */
    uint16_t* Vdc_buf;      /* [20] 母线电压 ADC */
    uint16_t* phi_buf;      /* [20] 相位角 (0.1°) */
    uint16_t* f_sw_buf;     /* [20] 开关频率 (Hz) */
    uint16_t  duty_ratio;   /* 导通占比 0.01%: (highOff-highOn)×2/lowOff */
    uint8_t   count;        /* 有效个数 (≤20) */
} IH_ElecInput;

/* ========== 输出 ====================================================== */
typedef struct {
    float I_peak_A;         /* A */
    float Vdc_mean;         /* V */
    float phi_deg;          /* °, 已还原真实角度 */
    float f_sw_Hz;          /* Hz */

    float L_uH;             /* μH, 中位数聚合 + 权重修正 */
    float f_res_kHz;        /* kHz */
    float Q_factor;
    float R_ohm;            /* Ω */
    float I_rms;            /* A */
    float P_W;              /* W */
    float Z_mag_ohm;        /* Ω */
    float X_ohm;            /* Ω */

    float L_stable;         /* μH, Kalman 稳定值 */
    float L_fast;           /* μH, Kalman 即时值 */
    uint8_t event;          /* 0=正常 1=抬锅 2=移锅 3=干烧 */

    bool valid;
} IH_ElecResult;

/* ========== API ====================================================== */
void IH_CalculateParams(IH_ElecInput* in, IH_ElecResult* out);

#ifdef __cplusplus
}
#endif

#endif /* IH_ELEC_PARAMS_H */
