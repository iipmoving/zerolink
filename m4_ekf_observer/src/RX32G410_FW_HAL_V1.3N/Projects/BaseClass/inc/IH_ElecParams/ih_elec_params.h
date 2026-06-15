// ============================================================================
// ih_elec_params.h — IH 电参数计算模块
// 输入 IH_CycleDataDef[20], 输出 L/f_res/Q/R/P/Z
// 与 MATLAB export_golden.m 公式链逐行对齐
// ============================================================================
#ifndef IH_ELEC_PARAMS_H
#define IH_ELEC_PARAMS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ========== 标定常数 ==================================================
// ---------------------- 硬件参数 ----------------------
#define IH_C_FARAD          0.9e-6f     // 谐振电容 0.9 μF

// 电压采样电路参数 (单位: Ω)
#define IH_VDC_R_PULLUP     0.0f        // 母线电压上拉电阻
#define IH_VDC_R_PULLDOWN   0.0f        // 母线电压下拉电阻

// 谐振电流采样电路参数
#define IH_I_CT_RATIO       0.0f        // 互感器变比 (次级/初级)
#define IH_I_R_PULLUP       0.0f        // 谐振电流上拉电阻 (Ω)
#define IH_I_R_PULLDOWN     0.0f        // 谐振电流下拉电阻 (Ω)

// ---------------------- 标定系数 ----------------------
#define IH_I_SCALE          0.02523f    // I_adc → A (ADC值转实际电流)
#define IH_VDC_SCALE        0.10606f    // Vdc_adc → V (ADC值转实际电压)
#define IH_PHI_SCALE        0.1f        // phi 单位 0.1° → °

// ---------------------- 滤波参数 ----------------------
#define IH_I_PEAK_MIN_RATIO 0.3f        // I_peak < 中位数×0.3 丢弃
#define IH_L_MIN_uH        10.0f        // 有效 L 下限
#define IH_FRES_MIN_kHz    5.0f         // 有效 f_res 下限

// ========== HRTIM 时序状态 ============================================
typedef struct {
    uint16_t highOn;
    uint16_t highOff;
    uint16_t lowOn;
    uint16_t lowOff;
} IH_HrtimState;

// ========== 单周期输入 ================================================
typedef struct {
    IH_HrtimState hrtim;               // HRTIM 翻转点
    uint16_t peak_current;             // 峰值电流 (ADC) — KVL → L
    uint16_t active_current;           // 有功电流 (整数定标) — P = I × Vdc
    uint16_t voltage;                  // 瞬时电压均值 (ADC)
    uint16_t zero_cross_high;          // 上管过零点 HRTIM CNT
} IH_CycleDataDef;

// ========== 20ms 帧输入 ===============================================
typedef struct {
    IH_CycleDataDef cycle[20];         // 20 个周期一组
    uint8_t count;                     // 有效周期数 (≤20)
} IH_ElecInputDef;

// ========== 输出 ======================================================
typedef struct {
    float I_peak_A;         // A
    float Vdc_mean;         // V
    float phi_deg;          // °, 相位差归一化到 180°
    float f_sw_Hz;          // Hz

    float L_uH;             // μH, 中位数 + 权重修正
    float f_res_kHz;        // kHz
    float Q_factor;
    float R_ohm;            // Ω
    float I_rms;            // A
    float P_W;              // W
    float Z_mag_ohm;        // Ω
    float X_ohm;            // Ω

    float L_stable;         // μH, Kalman 稳定值
    float L_fast;           // μH, Kalman 即时值
    uint8_t event;          // 0=正常 1=抬锅 2=移锅 3=干烧

    bool valid;
} IH_ElecResult;

// ========== API ======================================================
void IH_CalculateParams(IH_ElecInputDef* in, IH_ElecResult* out);

#ifdef __cplusplus
}
#endif

#endif /* IH_ELEC_PARAMS_H */
