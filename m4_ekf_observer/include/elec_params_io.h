// ============================================================================
// elec_params_io.h — 电参数计算模块 I/O 结构体 (v2.3 LINK+PARAMS)
// ============================================================================
// 输入: Calculator (HRTIM 时序 + 电流电压) — InputCallback 直穿赋值
// 输出: PowerBase  (L, f_res, Q, R, P 等)
// ============================================================================
//
// IO 头独立规则:
//   elec_params_io.h 不 #include 任何其他模块的 IO 头。
//   本模块所需的输入类型在此自包含定义。
//   InputCallback (在 data_switcher.c) 将 Calculator 输出 LINK 地址
//   直穿赋值给本模块的输入指针。
//
//   布局兼容性: ElecParams_CycleInput 与 Calculator_to_ElecParams_Output_Params
//   的字段顺序/大小一致，确保指针直穿后字段偏移正确。
//
// ============================================================================
#ifndef ELEC_PARAMS_IO_H
#define ELEC_PARAMS_IO_H

#include <stdint.h>
#include <stdbool.h>
#include "std_module.h"

// ========== 标定常数 ==================================================
#define IH_C_FARAD          0.9e-6f     // 谐振电容 0.9 μF
#define IH_I_SCALE          0.02523f    // I_adc → A
#define IH_VDC_SCALE        0.10606f    // Vdc_adc → V
#define IH_PHI_SCALE        0.1f        // phi 单位 0.1° → °

#define IH_I_PEAK_MIN_RATIO 0.3f        // I_peak < 中位数×0.3 丢弃
#define IH_L_MIN_uH        10.0f        // 有效 L 下限
#define IH_FRES_MIN_kHz    5.0f         // 有效 f_res 下限

// 高电压采样窗口 (对应 adc_sensor.c num10=[4,5])
// 相位角和 L 只使用此窗口内的周期
#define IH_HV_WIN_NUM10_MIN 4           // 高电压段 ADC sample 起始序号
#define IH_HV_WIN_NUM10_MAX 5           // 高电压段 ADC sample 结束序号
#define IH_HV_WIN_CNT_MIN   1           // 每周期至少 N 次电压采样才算有效

#define ELEC_CYCLE_MAX      20          // 内部计算最大周期数
#define ELEC_POTMAX         4           // 炉头数 (与 CALC_POTMAX 一致)

// ========== HRTIM 时序状态 ============================================
typedef struct {
    uint16_t highOn;
    uint16_t highOff;
    uint16_t lowOn;
    uint16_t lowOff;
} ElecParams_HrtimState;

/* ===================================================================
 * 输入定义 — 布局与 Calculator_to_ElecParams_Output_Params 兼容
 * =================================================================== */

/* 单周期数据 (与 Calculator 输出 params[i] 字段对齐, 32 bytes) */
typedef struct {
    ElecParams_HrtimState hrtim;        //  8 bytes
    uint16_t peak_current;              //  2
    uint8_t  pad[2];                    //  2
    uint32_t active_current_sum_high;   //  4
    uint32_t active_current_sum_low;    //  4
    uint32_t voltage_sum;               //  4
    uint16_t zero_cross_high;           //  2
    uint16_t zero_cross_low;            //  2
    uint16_t peak_point;                //  2
    uint16_t voltage_count;             //  2
} ElecParams_CycleInput;               // 32 bytes

/* Calculator → ElecParams: 输入 LINK (布局与 Calculator 输出 LINK 一致) */
typedef struct {
    uint8_t  status;             // ST_NEW (Calculator 产出后置位)
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  _enable;

    uint32_t *    shareBuff;           // ElecParams_Ws 工作区指针 (外部传入)
    ElecParams_CycleInput cycles[ELEC_POTMAX * ELEC_CYCLE_MAX];
} MODULE_INPUT_LINK(Calculator, ElecParams);

/* 输入包装: InputCallback 直穿赋值 input 指针 */
typedef struct {
    MODULE_INPUT_LINK(Calculator, ElecParams)* input;
} MODULE_INPUT(ElecParams);

/* ===================================================================
 * 输出定义 (ElecParams → PowerBase)
 * =================================================================== */

typedef struct {
    int32_t I_peak_A;
    int32_t Vdc_mean;
    int32_t phi_deg;
    int32_t f_sw_Hz;

    int32_t L_uH;
    int32_t f_res_kHz;
    int32_t Q_factor;
    int32_t R_ohm;
    int32_t I_rms;
    int32_t P_W;
    int32_t Z_mag_ohm;
    int32_t X_ohm;

    int32_t L_stable;
    int32_t L_fast;
    uint8_t event;          // 0=正常 1=抬锅 2=移锅 3=干烧
    uint8_t valid;
    uint8_t res[2];
} MODULE_OUTPUT_PARAMS(ElecParams, PowerBase);

/* ElecParams → PowerBase: 输出 LINK (每炉头) */
typedef struct {
    uint8_t  status;
    uint8_t  event;
    uint8_t  res[2];
    MODULE_OUTPUT_PARAMS(ElecParams, PowerBase) params[ELEC_POTMAX];
} MODULE_OUTPUT_LINK(ElecParams, PowerBase);

typedef struct {
    MODULE_OUTPUT_LINK(ElecParams, PowerBase)  head;
} MODULE_OUTPUT(ElecParams);

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(ElecParams);

#endif /* ELEC_PARAMS_IO_H */
