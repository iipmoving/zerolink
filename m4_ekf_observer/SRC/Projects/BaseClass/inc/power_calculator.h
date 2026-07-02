#ifndef POWER_CALCULATOR_H
#define POWER_CALCULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ========================== 工作周期定义 (保持不变) ========================== */
typedef struct {
    uint16_t start;             // HRTIM开始点
    uint16_t end;               // HRTIM结束点   
    uint16_t highOn;            // 死区后高端开通HRTIM值
    uint16_t highOff;           // 高端关闭(DUTY PPG占空比) 
    uint16_t lowOn;             // 死区后低端开通
    uint16_t lowOff;            // 低端关闭(prioed PPG周期)
    uint16_t zero_cross_high;   // 高端过零点索引
    uint16_t zero_cross_low;    // 低端过零点索引
    uint16_t perAdc;            // 每个ADC对应的HRTIM值
    uint16_t res;
} PowerCalculatorInputDef;

/* ========================== 峰值电流信息 ========================== */

typedef struct {
    uint16_t index;
    uint16_t value;
} PeakInfo;

/* ========================== 中间累加结果 ========================== */

typedef struct {
    uint32_t voltage;
    uint32_t current;
	uint16_t peak_current;
	uint16_t peak_num;
} CalculatorResultDef;


typedef struct {
    uint16_t highOn;
    uint16_t highOff;
    uint16_t lowOn;
    uint16_t lowOff;
}IH_HrtimState;


typedef struct {
	
	IH_HrtimState	hrtim;			
	
	uint16_t 	peak_current;          // 峰值电流 (ADC 值)
    uint16_t  active_current;        // 有功电流 (整数定标)
    uint16_t 	voltage;               // 瞬时电压均值 (ADC 值)    
	uint16_t 	zero_cross_high;       // 上管过零点索引

} IH_CycleDataDef;

typedef struct {
    IH_CycleDataDef cycle[20];
    uint8_t count;
} IH_ElecInputDef;



/* ========================== 功率计算结果 (扩展) ========================== */

typedef struct {
    /* --- 原有字段 (保持兼容) --- */
		IH_HrtimState	hrtim;
		uint16_t 	peak_current;          // 峰值电流 (ADC 值)
    uint16_t  active_current;        // 有功电流 (整数定标)
    uint16_t 	voltage;               // 瞬时电压均值 (ADC 值)    
		uint16_t 	zero_cross_high;       // 上管过零点索引
    uint16_t 	zero_cross_low;       	 // 下管过零点索引
	
	
	

    uint16_t 	zero_current;          // 过零点电流值
    int16_t  	phase_angleUp;         // 上管相位角 (度×10)
    int16_t  	phase_angleDown;       // 下管相位角 (度×10)
    uint16_t 	esr;                   // 等效电阻 (整数定标)
    int32_t  	active_power;          // 有功功率 (整数定标)

    /* --- 新增浮点字段 (物理单位) --- */
//    float P_W;                      // 有功功率 (W)
//    float I_rms;                    // 电流有效值 (A)
//    float I_peak;                   // 电流峰值 (A)
//    float Vdc_mean;                 // 母线电压均值 (V)
//    float phi_deg;                  // 相位角 (度)
} PowerResult;





/* ========================== 电参数 (20ms 计算) ========================== */

typedef struct {
    float f_sw_kHz;                 // 开关频率
    float f_res_kHz;                // 谐振频率
    float D_U_pct;                  // 上管占空比 %
    float DT1_us;                   // 死区1 (上关→下开)
    float DT2_us;                   // 死区2 (下关→上开)
    float I_rms;                    // 电流有效值 (A)
    float I_peak;                   // 电流峰值 (A)
    float Vdc_mean;                 // 母线电压均值 (V)
    float phi_deg;                  // 相位角 (度, 感性为正)
    float cos_phi;                  // 功率因数
    float P_W;                      // 有功功率 (W)
    float Z_mag_ohm;                // 阻抗模 (Ω)
    float R_ohm;                    // 等效电阻 (Ω)
    float X_ohm;                    // 电抗 (Ω)
    float L_uH;                     // 等效电感 (μH)
    float Q_factor;                 // 品质因数
    float L_corr_uH;                // B-H 修正后电感 (μH)
    float L_kalman_uH;              // Kalman 滤波后电感 (μH)
    uint8_t anomaly;               // 锅具变化检测标志
    uint8_t valid;                 // 数据有效标志
} ElecParamsDef;

/* ========================== Kalman 滤波器状态 ========================== */

typedef struct {
    float x_hat;                    // L 估计值 (μH)
    float P;                        // 误差协方差
    float sigma_run;                // 运行标准差
    float innov_sum;                // 新息累积
    uint16_t n_samples;             // 已处理样本数
    uint8_t  initialized;           // 初始化标志
} KalmanLState;

/* ========================== 硬件标定常数 ========================== */

#define V_SCALE         0.10606f    // V/count  (Vref/4096 / 分压比)
#define I_SCALE         0.02523f    // A/count  (Vref/4096 / I分压比)
#define VDC_SCALE       0.10606f    // V/count  (同 V 分压网络)

#define HRTIM_CLK_HZ    144000000   // HRTIM 时钟 144MHz
#define HRTIM_MAX_CNT   65535       // 16位计数器

#define C_RES_uF        0.9f        // 谐振电容标称值 (μF)
#define MIN_PULSE_CNT   864         // 最小脉宽 6μs × 144MHz

/* ========================== Kalman 参数 ========================== */

#define KALMAN_Q        0.10f       // 过程噪声协方差
#define KALMAN_R0       2.0f        // 基础测量噪声
#define KALMAN_ANOMALY_THRESH  3.5f // 突变检测阈值 (sigma 倍数)

/* ========================== 过零检测参数 ========================== */

#define ZERO_WINDOW_MIN     8       // 过零最小检测窗口

#define PHASE_DEG_BASE      1800    // 180.0 度定标

/* ========================== 调试辅助 ========================== */

#define testCh  0

/* ========================== 函数声明 ========================== */

/**
 * @brief  单周期功率计算 (1ms 快速路径, 每炉头)
 *
 * 算法: I×Vdc 直接积分 → P_W, 不依赖 φ 或 V_fund 近似.
 * 输入为单谐振周期的 ADC + HRTIM 采样数据.
 *
 * @param  resonant_current  谐振电流 ADC 数组
 * @param  hrtim_values      HRTIM CNT 时间戳数组
 * @param  voltage_data      母线电压 ADC 数组
 * @param  input             工作周期参数
 * @return PowerResult       原有整数字段 + 新增浮点字段
 */
PowerResult CalculatePower(
    uint16_t* resonant_current,
    uint16_t* hrtim_values,
    uint16_t* voltage_data,
    PowerCalculatorInputDef* input
);

/**
 * @brief  20ms 电参数计算 (4炉头统一)
 *
 * 计算 f_sw, f_res, φ, D_U, DT1/2, L, Q, Z, Kalman 平滑,
 * 锅具变化检测等. 每个炉头独立计算, Kalman 状态内部分别持有.
 *
 * @param  current_buf  4炉头电流 ADC 数组指针
 * @param  hrtim_buf    4炉头 HRTIM CNT 数组指针
 * @param  voltage_buf  4炉头电压 ADC 数组指针
 * @param  input        4炉头工作周期参数
 * @param  elec         输出: 4炉头电参数结果
 */
void CalculateElecParams_20ms(
    uint16_t* current_buf[4],
    uint16_t* hrtim_buf[4],
    uint16_t* voltage_buf[4],
    PowerCalculatorInputDef* input[4],
    ElecParamsDef elec[4]
);

/* ========================== 原有辅助函数 (保持不变) ========================== */

int16_t * Power_Calculator_GetVoltageBuffAddress(uint8_t ch);
int16_t * Power_Calculator_GetHrtimBuffAddress(uint8_t ch);
int16_t * Power_Calculator_GetTxaBuffAddress(uint8_t ch);
int16_t Power_Calculator_GetTxaBuffSize(uint8_t ch);
PowerCalculatorInputDef* Power_Calculator_GetInputArrayAddress(uint8_t ch);

#endif /* POWER_CALCULATOR_H */
