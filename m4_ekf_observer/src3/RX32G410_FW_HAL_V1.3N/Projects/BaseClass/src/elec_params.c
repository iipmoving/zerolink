// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
#include "../../include_io/elec_params_io.h"

MODULE_SKELETON(ElecParams);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;
    struct {
        uint8_t calculator   : 1;  /* Calculator 数据就绪 */
    } bits;
} ElecParams_PipeFlags_t;

/* ---- 数据实体（模块私有）---- */
static MODULE_INPUT(ElecParams)*   s_inPara;    // 输入参数实体在ADC， 这里只调用不修改
static MODULE_OUTPUT(ElecParams)  	s_outPara;   // 输出参数缓冲区实体

/* ---- 内部 OUTPUT_LINK 实例 (指针直穿目标) ---- */
static MODULE_OUTPUT_LINK(ElecParams, EKF_LKF)   s_elecToEkfLink;
static MODULE_OUTPUT_LINK(ElecParams, AppPower)   s_elecToAppPowerLink;

static void user_Process(MODULE_INPUT(ElecParams) *in, MODULE_OUTPUT(ElecParams) *out, ElecParams_PipeFlags_t flags);

static void ProcessInput(void)
{
    MODULE_INPUT(ElecParams) *in  = (MODULE_INPUT(ElecParams)*)g_input.para;
    MODULE_OUTPUT(ElecParams) *out = (MODULE_OUTPUT(ElecParams)*)g_output.para;

    /* === 输入段: 数据有效检查 === */
    ElecParams_PipeFlags_t flags = {0};
    flags.bits.calculator = (in->Calculator_params->status & ST_NEW) ? 1 : 0;

    /* === 计算段: 用户业务 === */
    user_Process(in, out, flags);
}

MODULE_EXPORT(ElecParams);

// ===== [END AI GENERATED] =====

// ============================================================================
// elec_params.c — IH 电参数计算模块 (v2.3 LINK+PARAMS 架构)
// 
// 功能概述：从 Calculator 模块获取的采样数据中计算 IH 系统的关键电参数
// ============================================================================
// 
// ********************* 输入数据结构 (Calculator → ElecParams) ********************
// 
// 输入来源: Calculator_to_ElecParams_Output_Link (指针直穿，无本地拷贝)
// 
// 主要数据成员:
//   - count: uint8_t, 有效周期数 (1~20)
//   - params[count]: Calculator_to_ElecParams_Output_Params, 每周期采样数据数组
// 
// params[cycle] 单周期数据结构详解:
// 
//   struct Calculator_to_ElecParams_Output_Params {
//       uint16_t peak_current;      // 【ADC值】峰值电流采样值
//                                   // 原始 ADC 12位 (0~4095) → 经 IH_I_SCALE 转换为安培
//                                   // 含义：谐振电流的峰值点采样
//       
//       uint16_t voltage;          // 【ADC值】母线电压均值采样值
//                                   // 原始 ADC 12位 (0~4095) → 经 IH_VDC_SCALE 转换为伏特
//                                   // 含义：20ms 周期内的母线电压平均值
//       
//       uint16_t active_current;  // 【整型定标】有功电流分量
//                                   // 用于计算有功功率 P = I_active × Vdc
//       
//       IH_HrtimState hrtim;      // HRTIM 定时器翻转点状态
//                                   // 用于计算开关频率和相位角
//           hrtim.highOn:  uint16_t  // 上管开通时刻 HRTIM 计数值
//           hrtim.highOff: uint16_t  // 上管关断时刻 HRTIM 计数值
//           hrtim.lowOn:   uint16_t  // 下管开通时刻 HRTIM 计数值
//           hrtim.lowOff:  uint16_t  // 下管关断时刻 HRTIM 计数值 (即周期长度)
//       
//       uint16_t zero_cross_high; // 上管过零点 HRTIM 计数值
//                                   // 用于计算相位角 φ
//   };
// 
// ********************* 输出数据结构 (ElecParams → PowerBase) ********************
// 
// 输出目标: ElecParams_to_PowerBase_Output_Params (每炉头独立存储)
// 输出格式: 整型定标 (×100)，浮点值 × 100 + 0.5 (四舍五入)
// 
// 主要输出参数:
// 
//   struct ElecParams_to_PowerBase_Output_Params {
//       int32_t I_peak_A;      // 峰值电流 (单位: 0.01A)
//                               // 计算方式: 中值滤波 + 权重修正后的峰值电流
//       
//       int32_t Vdc_mean;      // 母线电压均值 (单位: 0.01V)
//                               // 计算方式: 20ms周期内电压ADC值的中值
//       
//       int32_t phi_deg;       // 相位角 (单位: 0.01°)
//                               // 计算方式: 高电流周期的相位角平均值
//                               // 物理意义: 电流相对于电压的相位差
//       
//       int32_t f_sw_Hz;       // 开关频率 (单位: 0.01Hz)
//                               // 计算方式: f_sw = HRTIM_CLK / lowOff
//       
//       int32_t L_uH;          // 等效电感 (单位: 0.01μH)
//                               // 计算方式: 基波等效电路法 + 中值滤波 + 权重修正
//       
//       int32_t f_res_kHz;     // 谐振频率 (单位: 0.01kHz)
//                               // 计算方式: f_res = 1 / (2π√(LC))
//       
//       int32_t Q_factor;      // 品质因数 (单位: 0.01)
//                               // 计算方式: Q = tan(φ) / (f_sw/f_res - f_res/f_sw)
//       
//       int32_t R_ohm;         // 等效电阻 (单位: 0.01Ω)
//                               // 计算方式: R = ω_res × L / Q
//       
//       int32_t I_rms;         // 电流有效值 (单位: 0.01A)
//                               // 计算方式: I_rms = I_peak × √2/2
//       
//       int32_t P_W;           // 有功功率 (单位: 0.01W)
//                               // 计算方式: P = median(I_active × Vdc)
//       
//       int32_t Z_mag_ohm;     // 阻抗模 (单位: 0.01Ω)
//                               // 计算方式: Z = √(R² + X²)
//       
//       int32_t X_ohm;         // 净电抗 (单位: 0.01Ω)
//                               // 计算方式: X = X_L - X_C
//       
//       int32_t L_stable;      // 稳定电感值 (单位: 0.01μH)
//                               // 用途: Kalman 滤波稳定值，用于长时间参数估计
//       
//       int32_t L_fast;        // 快速电感值 (单位: 0.01μH)
//                               // 用途: Kalman 滤波即时值，用于快速响应
//       
//       uint8_t event;         // 事件标志
//                               // 0=正常 1=抬锅检测 2=移锅检测 3=干烧检测
//       
//       uint8_t valid;         // 计算有效性标志
//                               // 0=无效 1=有效 (L > 10μH 且 f_res > 5kHz)
//   };
// 
// ********************* 数据流概述 ********************
// 
// 数据流向:
//   [ADC采样] → [Calculator模块] → [ElecParams模块] → [PowerBase模块]
//                ↓                    ↓                    ↓
//            peak_current         L_uH, f_res          P_W, I_rms
//            voltage, hrtim       Q_factor, R         event, valid
// 
// 调用时机:
//   - 20ms 周期结束时触发
//   - 由 InputCallback 路由将 Calculator 输出 LINK 注入到 ElecParams 输入
//   - ElecParams 处理完成后输出到 PowerBase，供上层逻辑使用
// 
// 核心算法流程：
//  1. 逐周期提取峰值电流、母线电压、开关频率、相位角
//  2. 基于基波等效电路计算电感 L = (Vdc/2 + Vcp) / (I_pk * ω)
//  3. 中值滤波去除异常值，峰值电流权重修正电感值
//  4. 计算谐振频率 f_res = 1/(2π√(LC))
//  5. 计算 Q 值和等效电阻 R = ωL/Q
// ============================================================================

//#include "../include/elec_params_io.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// HRTIM 时钟配置
// 时钟频率: 768 MHz
// perAdc = 384 cnt/step → 每 ADC 采样步长 = 384/768MHz = 0.5 μs
#define HRTIM_CLK_HZ        768000000.0f



/* ---- LINK params 存储 (每炉头独立) ---- */


#define	 PERIO_CNT   20 

#define	ELEC_POTMAX		4



#define IH_C_FARAD          0.9e-6f     // 谐振电容 0.9 μF
#define IH_I_SCALE          0.02523f    // I_adc → A
#define IH_VDC_SCALE        0.10606f    // Vdc_adc → V
#define IH_PHI_SCALE        0.1f        // phi 单位 0.1° → °
#define IH_I_PEAK_MIN_RATIO 0.3f        // I_peak < 中位数×0.3 丢弃
#define IH_L_MIN_uH        10.0f        // 有效 L 下限
#define IH_FRES_MIN_kHz    5.0f         // 有效 f_

#define		IH_HV_WIN_CNT_MIN		4						//相位角取值范围（20个点分成2个10ms 里面的高点， 4和5）



// ========== 内部辅助函数 ====================================================

/**
 * @brief 浮点数组插入排序
 * @param buf 待排序数组
 * @param n 数组元素个数
 */
static void sort_f(float* buf, uint8_t n) {
    uint8_t i, j;
    for (i = 1; i < n; i++) {
        float key = buf[i];
        j = i;
        while (j > 0 && buf[j - 1] > key) { buf[j] = buf[j - 1]; j--; }
        buf[j] = key;
    }
}

/**
 * @brief 计算浮点数组的中值
 * @param buf 输入数组
 * @param n 数组元素个数
 * @return 中值（偶数个元素时返回中间两个的平均值）
 * 
 * 中值滤波特性：对极端异常值具有很好的鲁棒性，适合去除采样噪声
 */
static float median_f(float* buf, uint8_t n) {
    if (n == 0) return 0.0f;
    float tmp[20];
    for (uint8_t i = 0; i < n; i++) { tmp[i] = buf[i]; }
    sort_f(tmp, n);
    if (n % 2 == 1) return tmp[n / 2];
    return (tmp[n / 2 - 1] + tmp[n / 2]) * 0.5f;
}

// ========== 内部计算结果 — 纯浮点，不含 I/O 结构体类型 ==========
// 修改 I/O 接口 (ElecParams_to_PowerBase_Output_Params) 不影响此类型
// 此结构体用于 ElecParams_Calc 函数的中间计算结果存储
// 设计原则：内部计算使用 float 提高精度，最终输出时转换为整型定标 (×100)
/**
 * @brief 内部计算结果结构体
 * 
 * 存储 ElecParams_Calc 函数的所有中间计算结果，使用 float 类型保证计算精度。
 * 与输出结构体 ElecParams_to_PowerBase_Output_Params 的区别：
 *   - 本结构体：float 原始计算值，用于内部传递
 *   - 输出结构体：int32_t ×100 定标值，用于模块间接口
 */
//typedef struct {
//    float   I_peak_A;      /**< 峰值电流 (A) — 中值滤波 + 权重修正后 */
//    float   Vdc_mean;      /**< 母线电压均值 (V) — 20ms周期ADC值中值 */
//    float   phi_deg;       /**< 相位角 (度) — 高电流周期平均 */
//    float   f_sw_Hz;       /**< 开关频率 (Hz) — HRTIM_CLK/lowOff */
//    float   L_uH;          /**< 等效电感 (μH) — 基波等效电路法 + 权重修正 */
//    float   f_res_kHz;     /**< 谐振频率 (kHz) — f_res = 1/(2π√(LC)) */
//    float   Q_factor;       /**< 品质因数 — Q = tan(φ)/(f_sw/f_res - f_res/f_sw) */
//    float   R_ohm;         /**< 等效电阻 (Ω) — R = ωL/Q */
//    float   I_rms;         /**< 电流有效值 (A) — I_rms = I_peak × √2/2 */
//    float   P_W;           /**< 有功功率 (W) — P = median(I_active × Vdc) */
//    float   Z_mag_ohm;     /**< 阻抗模 (Ω) — Z = √(R² + X²) */
//    float   X_ohm;         /**< 净电抗 (Ω) — X = X_L - X_C */
//    uint8_t valid;         /**< 计算有效性标志 — 1=有效, 0=无效 */
//    uint8_t res[3];        /**< 保留字节，对齐填充 */
//} ElecParams_CalcResult;

// ========== 暂态工作区类型 (实例由 shareBuff 外部传入) ==========
typedef struct {
    float f_sw[PERIO_CNT];      // 开关频率，用于中值
    float P_arr[PERIO_CNT];     // 有功功率，用于中值
} ElecParams_Ws;                // 2×20 = 160 bytes

// ========== 纯计算函数 ====================================================
// 不引用本模块的 MODULE_INPUT/MODULE_OUTPUT 类型，只依赖 Calculator → ElecParams 输入 LINK
// 核心算法：基于基波等效电路模型计算 IH 电参数
/**
 * @brief IH 电参数核心计算函数
 * 
 * @param result    输出：计算结果结构体
 * @param cycles    输入：指向某炉头 20 周期数据块的指针 (ElecParams_CycleInput[PERIO_CNT])
 * @param ws        输入：暂态工作区 (ElecParams_Ws)，由 shareBuff 外部传入
 * @return 1=计算成功，0=计算失败
 * 
 * 核心计算公式说明：
 * 
 * 1. 开关频率计算：
 *    f_sw = HRTIM_CLK_HZ / lowOff
 *    其中 lowOff 是 HRTIM 周期计数值
 * 
 * 2. 相位角计算：
 *    φ = (zero_cross_high - highOn) / (highOff - highOn) × 180°
 *    表示上管过零点相对于上管导通区间的相位位置
 * 
 * 3. 电感计算（基波等效电路法）：
 *    Vcp = I_pk / (ω × C)          // 电容电压峰值
 *    L = (Vdc/2 + Vcp) / (I_pk × ω)  // 等效电感
 *    原理：谐振腔电压平衡，输入电压等于电感电压降
 * 
 * 4. 谐振频率计算：
 *    f_res = 1 / (2π√(L × C))
 * 
 * 5. Q 值计算（基于相位偏移）：
 *    Q = tan(φ) / (f_sw/f_res - f_res/f_sw)
 *    推导：Q = ωL/R = tan(φ) / (f/f0 - f0/f)
 * 
 * 6. 等效电阻计算：
 *    R = ω_res × L / Q
 * 
 * 7. 阻抗计算：
 *    X_L = ω_sw × L        // 感抗
 *    X_C = 1 / (ω_sw × C)  // 容抗
 *    Z = √(R² + (X_L - X_C)²)  // 总阻抗模
 */

/* ---- P_W 两种算法: avg_avg = 平均电流×平均电压, inst_avg = 逐周期即时功率平均 ---- */
typedef struct {
    float avg_avg;    /* 平均电流×平均电压 */
    float inst_avg;   /* 逐周期即时功率平均 */
} PwResult_t;

static void CalcPw(const MODULE_INPUT_PARAMS(Calculator, ElecParams) *cycles, PwResult_t *pw)
{
    float i_sum = 0.0f, v_sum = 0.0f, p_sum = 0.0f;

    for (uint8_t i = 1; i < PERIO_CNT; i++) {
        float loff = (float)cycles[i].hrtim_lowOff;
        float i_high = (loff > 0.0f) ? ((float)cycles[i].active_current_sum_high / loff) : 0.0f;
        float i_low  = (loff > 0.0f) ? ((float)cycles[i].active_current_sum_low  / loff) : 0.0f;
        float p_avg = (cycles[i].voltage_count > 0)
            ? (float)cycles[i].voltage_sum / (float)cycles[i].voltage_count : 0.0f;
        float i_total = i_high + i_low;

        i_sum += i_total;
        v_sum += p_avg;
        p_sum += i_total * p_avg;
    }

    /* 多加一次 cycles[10] 补偿缺失的 cycles[0]（50Hz 半周期对称，参考 APP_ADC_TxaAvgSum） */
    {
        float loff = (float)cycles[10].hrtim_lowOff;
        float i_total = ((float)cycles[10].active_current_sum_high + (float)cycles[10].active_current_sum_low) / loff;
        float p_avg = (cycles[10].voltage_count > 0)
            ? (float)cycles[10].voltage_sum / (float)cycles[10].voltage_count : 0.0f;
        i_sum += i_total;
        v_sum += p_avg;
        p_sum += i_total * p_avg;
    }

    pw->avg_avg  = (i_sum / 20.0f) * IH_I_SCALE * (v_sum / 20.0f) * IH_VDC_SCALE;
    pw->inst_avg = (p_sum / 20.0f) * IH_I_SCALE * IH_VDC_SCALE;
}
 

 
static uint8_t ElecParams_Calc(MODULE_OUTPUT_PARAMS(ElecParams, EKF_LKF) *result,
                                const MODULE_INPUT_PARAMS(Calculator, ElecParams) *cycles,
                                ElecParams_Ws *ws)
{
    if (result == NULL || cycles == NULL ) return 0;
//    memset(result, 0, sizeof(ElecParams_CalcResult));

    // ---- 单循环：逐周期提取 + 累加（i=1..19, 高压段平均替代中值）----
    uint8_t valid_cnt = 0;
    float I_sum = 0.0f, L_sum = 0.0f, Vdc_sum = 0.0f;
    float phi_sum = 0.0f;
    uint8_t phi_n = 0, vn = 0;

    for (uint8_t i = 1; i < PERIO_CNT; i++) {
        const MODULE_INPUT_PARAMS(Calculator, ElecParams) *c = &cycles[i];

        // ADC 值 → 实际物理量转换
        float I_pk = (float)c->peak_current  * IH_I_SCALE;
        float v_avg = (c->voltage_count > 0)
            ? (float)c->voltage_sum / (float)c->voltage_count : 0.0f;
        float Vdc = v_avg * IH_VDC_SCALE;

        float loff = (float)c->hrtim_lowOff;
        ws->f_sw[i] = (loff > 0.0f) ? (HRTIM_CLK_HZ / loff) : 0.0f;

        float cond  = (float)(c->hrtim_highOff - c->hrtim_highOn);
        float delta = (float)c->zero_cross_high - (float)c->hrtim_highOn;
        float phi_i = (cond > 0.0f) ? (delta / cond * 180.0f) : 0.0f;

        // ---- L_raw: 只从高电压段有效周期（累加求平均）----
        if (I_pk > 0.0f && ws->f_sw[i] > 0.0f && c->voltage_count >= IH_HV_WIN_CNT_MIN) {
            float omega = 2.0f * (float)M_PI * ws->f_sw[i];
            float Vcp  = I_pk / (omega * IH_C_FARAD);
            float num  = Vdc * 0.5f + Vcp;
            float den  = I_pk * omega;
            if (den > 1e-12f) {
                float Lv = (num / den) * 1e6f;
                if (Lv > 0.0f) {
                    I_sum += I_pk;
                    L_sum += Lv;
                    valid_cnt++;
                }
            }
        }

        // ---- Vdc_mean: 全周期参与（电压低时自然贡献小）----
        Vdc_sum += Vdc;
        vn++;

        // ---- φ: 只取高电压段平均 ----
        if (c->voltage_count >= IH_HV_WIN_CNT_MIN) {
            phi_sum += phi_i;
            phi_n++;
        }

    }
    if (valid_cnt < 1) return 0;  // 无有效数据

    // ---- 聚合结果（平均替代中值）-----------------------------------------
    result->I_peak_A = I_sum / (float)valid_cnt;
    result->Vdc_mean = (vn > 0) ? (Vdc_sum / (float)vn) : 0.0f;
    result->f_sw_Hz  = (ws->f_sw[1] + ws->f_sw[2] + ws->f_sw[3] + ws->f_sw[4] + ws->f_sw[5] + ws->f_sw[6] + ws->f_sw[7] + ws->f_sw[8] + ws->f_sw[9] + ws->f_sw[10] + ws->f_sw[11] + ws->f_sw[12] + ws->f_sw[13] + ws->f_sw[14] + ws->f_sw[15] + ws->f_sw[16] + ws->f_sw[17] + ws->f_sw[18] + ws->f_sw[19]) / 19.0f;
    result->L_uH     = L_sum / (float)valid_cnt;
    result->phi_deg  = (phi_n > 0) ? (phi_sum / (float)phi_n) : 0.0f;
    {
        PwResult_t pw;
        CalcPw(cycles, &pw);
        result->P_W = pw.inst_avg;  /* 逐周期即时功率平均 */
    }
    // ---- 谐振参数计算 -------------------------------------------------
    // f_res = 1/(2π√(LC)) — 谐振频率
    float LC = (result->L_uH * 1e-6f) * IH_C_FARAD;  // L(μH→H) × C(F)
    float f_res = (LC > 1e-20f)
        ? (1.0f / (2.0f * (float)M_PI * sqrtf(LC)))
        : result->f_sw_Hz;  // 异常时使用开关频率作为谐振频率
    result->f_res_Hz = f_res;  // 单位 Hz

    // Q 值和等效电阻计算
    float omega_res = 2.0f * (float)M_PI * f_res;    // 谐振角频率
    float omega_sw  = 2.0f * (float)M_PI * result->f_sw_Hz;  // 开关角频率
    float phi_rad   = result->phi_deg * (float)M_PI / 180.0f;  // 相位角转弧度
    float fr_ratio  = result->f_sw_Hz / f_res;       // f_sw / f_res
    float denom_q   = fr_ratio - 1.0f / fr_ratio;    // f/f0 - f0/f

    // Q = tan(φ) / (f/f0 - f0/f) — 基于相位偏移的 Q 值计算
    result->Q_factor = (fabsf(denom_q) > 1e-6f) ? (tanf(phi_rad) / denom_q) : 0.0f;
    
    // R = ω_res × L / Q — 等效串联电阻
    result->R_ohm= (result->Q_factor > 1e-6f)
        ? (omega_res * (result->L_uH * 1e-6f) / result->Q_factor) : 0.0f;
    
    // I_rms = I_peak × √2/2 ≈ I_peak × 0.7071 — 正弦电流有效值
    result->I_rms = result->I_peak_A * 0.70710678f;

    // 阻抗计算
    float X_L = omega_sw * (result->L_uH * 1e-6f);   // 感抗 X_L = ωL
    float X_C = 1.0f / (omega_sw * IH_C_FARAD);      // 容抗 X_C = 1/(ωC)
    result->X_ohm = X_L - X_C;                        // 净电抗
    result->Z_mag_ohm = sqrtf(result->R_ohm * result->R_ohm + result->X_ohm * result->X_ohm);  // 阻抗模

    // 有效性判定：电感和谐振频率必须大于最小值
    result->valid = (result->L_uH > IH_L_MIN_uH) && (result->f_res_Hz > IH_FRES_MIN_kHz * 1000.0f);
    return result->valid;
}

// ========== 初始化 ==========================================================
/**
 * @brief 模块初始化函数
 * 
 * 初始化 I/O 实例和参数缓冲区，建立输出 LINK 与参数的关联
 * 当前支持最大4个炉头（单炉头模式下使用 head[0]）
 */
static void Init(void) {
    s_inPara = NULL;
    memset(&s_outPara, 0, sizeof(s_outPara));

	/* 绑定 OUTPUT_LINK 指钺 */
	s_outPara.EKF_LKF_params   = &s_elecToEkfLink;
	s_outPara.AppPower_params  = &s_elecToAppPowerLink;

//		s_outPara.AppPower_params->res[0]=4;				//CONSET_OUT :ELC_TO_APPPOWER
			s_outPara.EKF_LKF_params->res[0]=5;			//CONSET_OUT :ELC_TO_EKF

    g_input.para  = &s_inPara;
    g_output.para = &s_outPara;
}

// ========== 主计算逻辑 ======================================================
/**
 * @brief 主处理函数 — 处理输入并生成输出
 * 
 * 执行流程：
 *  1. 检查输入状态，判断是否有新数据
 *  2. 从输入 LINK 读取数据（由 InputCallback 预先填充）
 *  3. 调用核心计算函数 ElecParams_Calc
 *  4. 将浮点结果转换为整型定标输出（×100）
 *  5. 更新输出状态标志
 */
/* ---- P_W 低通滤波状态 (每炉头) ---- */
#define P_W_ALPHA 0.3f   /* 滤波系数: 越小越平滑, 响应越慢 */
static float s_pw_filt[ELEC_POTMAX];
static uint8_t s_pw_init[ELEC_POTMAX];
static	ElecParams_Ws		new;
#include	"API_gpio.h"
static void user_Process(MODULE_INPUT(ElecParams) *in, MODULE_OUTPUT(ElecParams) *out, ElecParams_PipeFlags_t flags)
 {


    /* ====== 输入段 ====== */
    // 检查是否有新的输入数据（数据层 status）
    if (!(in->Calculator_params->status & ST_NEW)) return;

    API_GPIO_WritePin(DebugB_pin, 1);
    in->Calculator_params->status &= ~ST_NEW;
    // 从输入 LINK 获取共享工作区
	

		ElecParams_Ws *ws =&new;	//(ElecParams_Ws *)in->input->shareBuff;
	
		if(ws<(ElecParams_Ws *)0x10000000)//地址不符
		{
				return;
		}		
	
		


    // 逐炉头计算：每炉头 20 周期独立计算
    for (uint8_t h = 0; h < ELEC_POTMAX; h++)
    {
        MODULE_OUTPUT_PARAMS(ElecParams, EKF_LKF)* result;
//        memset(&result, 0, sizeof(ElecParams_CalcResult));

				result=&out->EKF_LKF_params->params[h];
			
        // 指向当前炉头的 20 周期数据块
        const MODULE_INPUT_PARAMS(Calculator, ElecParams) *cycles = &in->Calculator_params->params[h][0];
        uint8_t calc_ok = ElecParams_Calc(result, cycles, ws);

            /* ---- P_W 一阶低通滤波 (20ms平均值 → 平滑输出) ---- */
            float pw_raw = result->P_W;
            if (!s_pw_init[h]) {
                s_pw_filt[h] = pw_raw;
                s_pw_init[h] = 1;
            } else {
                s_pw_filt[h] = s_pw_filt[h] * (1.0f - P_W_ALPHA) + pw_raw * P_W_ALPHA;
            }
            result->P_W = s_pw_filt[h];


		}
    API_GPIO_WritePin(DebugB_pin, 0);
    // 更新状态 — 写输出 LINK status + 清除输入 LINK status
//    out->AppPower_params->status |= ST_NEW;
		out->EKF_LKF_params->status |= ST_NEW;
    	out->EKF_LKF_params->seq++;

    /* ---- 设置 Telemetry 输出管道指针 ---- */

        out->Telemetry_params=out->EKF_LKF_params;

}

