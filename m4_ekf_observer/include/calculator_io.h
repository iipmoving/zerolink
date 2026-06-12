/**
 * @file    calculator_io.h
 * @brief   Calculator Data Switcher IO interface
 * @layer   base_class (Data Switcher IO)
 */
#ifndef CALCULATOR_IO_H
#define CALCULATOR_IO_H

#include <stdint.h>
#include "std_module.h"

/* ---- 前置声明 ---- */
#define CALC_POTMAX      4
#define CALC_CYCLE_MAX   20    /* 每炉头周期数 (与 ELEC_CYCLE_MAX 一致) */

/* ---- HRTIM 状态 ---- */
typedef struct {
    uint16_t highOn;
    uint16_t highOff;
    uint16_t lowOn;
    uint16_t lowOff;
} Calculator_HrtimState;

/* ---- 单炉头输入参数 ---- */
typedef struct {
    uint16_t start;              // HRTIM 开始点索引
    uint16_t end;                // HRTIM 结束点索引
    uint16_t highOn;             // 死区后上管开通 HRTIM 值
    uint16_t highOff;            // 上管关断 (DUTY) HRTIM 值
    uint16_t lowOn;              // 死区后下管开通 HRTIM 值
    uint16_t lowOff;             // 下管关断 (period) HRTIM 值
    uint16_t zero_cross_high;    // 上管过零点索引
    uint16_t zero_cross_low;     // 下管过零点索引
    uint16_t perAdc;             // 每个 ADC 采样对应的 HRTIM 计数
    uint16_t lagDuty;            // 滞后臂占空比 CMP 值 (全桥)
} Calculator_InputParams_t;   





/* ---- 单炉头输入数据 ---- */
typedef struct {

    uint8_t   status;             //Fmac 完成 
    uint8_t   max_count;          //最大爐頭數
    uint8_t   count;              // 当前20ms 周期的索引
    uint8_t   res[1];



    /**
     * @brief 谐振电流 ADC 数组地址的指针数组首地址（4个炉头一组）
     * 
     * 特殊设计说明：为减少参数传递数量，此字段存储的是指针数组的首地址。
     * 实际指向类似如下结构的数组：
     * @code
     * uint16_t* TxaFmacBuff[4];  // 4个炉头的谐振电流缓存指针数组
     * @endcode
     * 访问方式：resonant_current[i] 可获取第i个炉头的电流数据指针
     */
    uint16_t** resonant_current;   
    
    /**
     * @brief HRTIM 时间戳数组地址的指针数组首地址（4个炉头一组）
     * 
     * 特殊设计说明：为减少参数传递数量，此字段存储的是指针数组的首地址。
     * 实际指向类似如下结构的数组：
     * @code
     * uint16_t* TxaHrtimBuff[4];  // 4个炉头的HRTIM缓存指针数组
     * @endcode
     * 访问方式：hrtim_values[i] 可获取第i个炉头的HRTIM数据指针
     */
    uint16_t** hrtim_values;       
    
    /**
     * @brief 母线电压 ADC 数组地址的指针数组首地址（4个炉头一组）
     * 
     * 特殊设计说明：为减少参数传递数量，此字段存储的是指针数组的首地址。
     * 实际指向类似如下结构的数组：
     * @code
     * uint16_t* TxaVcBuff[4];  // 4个炉头的母线电压缓存指针数组
     * @endcode
     * 访问方式：voltage_data[i] 可获取第i个炉头的电压数据指针
     */
    uint16_t** voltage_data;       
    
    Calculator_InputParams_t* params; // 工作周期参数
} MODULE_INPUT_LINK(APP_Adc, Calculator);

/* ---- 输入: 炉头数组 ---- */
typedef struct {
    MODULE_INPUT_LINK(APP_Adc, Calculator)* input_params;
} MODULE_INPUT(Calculator);

typedef struct {

    Calculator_HrtimState hrtim;
    uint16_t peak_current;              // 峰值电流 (ADC 值)
    uint8_t  res[2];                    // 对齐到 32 位
    uint32_t active_current_sum_high;   // 上管电流积分和 (原始累加，ElecParams 浮点除)
    uint32_t active_current_sum_low;    // 下管电流积分和 (对称模式 = sum_high)
    uint32_t voltage_sum;               // 电压原始累加 (ElecParams 浮点除 voltage_count)
    uint16_t zero_cross_high;           // 上管过零点 HRTIM 值
    uint16_t zero_cross_low;            // 下管过零点 HRTIM 值
    uint16_t peak_point;                // 峰值电流采样点索引
    uint16_t voltage_count;             // 电压采样点数
} MODULE_OUTPUT_PARAMS(Calculator, ElecParams);  // 32 bytes

/* ---- Calculator → PowerBase: 直接参数 (每炉头平均) ---- */
typedef struct {
    int32_t resonant_current;        // 峰值电流平均值 (ADC值)
    int32_t phase_angle;             // 相位角 (0.01°)
    int32_t voltage;                 // 电压平均值 (ADC值)
    uint8_t valid;
    uint8_t res[3];
} MODULE_OUTPUT_PARAMS(Calculator, PowerBase);

/* Calculator → PowerBase: 输出 LINK */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  res[2];
    MODULE_OUTPUT_PARAMS(Calculator, PowerBase) params[CALC_POTMAX];
} MODULE_OUTPUT_LINK(Calculator, PowerBase);

/* ---- 单炉头输出 (PowerResult) ---- */
typedef struct {
    uint8_t   status;             // ST_NEW (产出后置位, Consumer 消费后清除)
    uint8_t   max_count;          //最大爐頭數
    uint8_t   count;              // 当前20ms 周期的索引
    uint8_t   _enable;            //Fmac 完成
		uint32_t *			shareBuff;					//共享缓存地址（message)
    MODULE_OUTPUT_PARAMS(Calculator, ElecParams) params[CALC_POTMAX][CALC_CYCLE_MAX];
} MODULE_OUTPUT_LINK(Calculator, ElecParams);



/* ---- 输出: 炉头数组 ---- */
typedef struct Calculator_Output {
    MODULE_OUTPUT_LINK(Calculator, ElecParams) elec_params;
    MODULE_OUTPUT_LINK(Calculator, PowerBase)  power_direct;
}MODULE_OUTPUT(Calculator);

/* ---- v2.2 统一接口声明 ---- */
MODULE_IO_H(Calculator);

#endif /* CALCULATOR_IO_H */