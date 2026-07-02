#ifndef POWER_CALCULATOR_FULLBRIDGE_H
#define POWER_CALCULATOR_FULLBRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include "power_calculator.h"  // 继承原有数据结构

/*============================================================================
 *                    移相全桥电流采集窗口定义
 *============================================================================*/

/**
 * @brief 全桥对角管导通状态枚举
 */
typedef enum {
    FB_CONDUCTION_NONE = 0,     // 无对角管导通
    FB_CONDUCTION_FORWARD = 1,  // 正向电流窗口：Q1+Q4导通
    FB_CONDUCTION_REVERSE = 2,  // 反向电流窗口：Q2+Q3导通
} FBConductionStateTypeDef;

/**
 * @brief 移相全桥HRTIM比较参数（在PowerCalculatorInputDef基础上扩展lagDuty）
 * @note lagDuty已通过PowerCalculatorInputDef.lagDuty传递，此处保留兼容
 */
typedef PowerCalculatorInputDef FB_PowerCalculatorInputDef;

/*============================================================================
 *                      移相全桥API函数声明
 *============================================================================*/

/**
 * @brief 计算有效电流（全桥版，双HRTIM通道窗口判断）
 * @param resonant_current 谐振电流数组
 * @param hrtim_lead      超前臂(TimerB)计数器数组
 * @param hrtim_lag       滞后臂(TimerE)计数器数组
 * @param voltage_data    电压数据数组
 * @param start           起始索引（零交叉点）
 * @param input           输入参数（含超前臂highOff和滞后臂lagDuty）
 * @return CalculatorResultDef 电流积分和电压平均值
 */
CalculatorResultDef FB_CalculateAuctalCurrent(
    uint16_t* resonant_current,
    uint16_t* hrtim_lead,
    uint16_t* hrtim_lag,
    uint16_t* voltage_data,
    uint16_t start,
    PowerCalculatorInputDef* input
);

/**
 * @brief 移相全桥功率计算主函数
 * @param resonant_current 谐振电流数组
 * @param hrtim_lead      超前臂(TimerB)计数器数组
 * @param hrtim_lag       滞后臂(TimerE)计数器数组
 * @param voltage_values  电压数据数组
 * @param input           输入参数
 * @return PowerResult    功率计算结果
 */
PowerResult FB_CalculatePower(
    uint16_t* resonant_current,
    uint16_t* hrtim_lead,
    uint16_t* hrtim_lag,
    uint16_t* voltage_values,
    PowerCalculatorInputDef* input
);

// 数据缓冲区访问函数（与半桥版本对应）
int16_t* FB_Power_Calculator_GetHrtimLeadBuffAddress(uint8_t ch);
int16_t* FB_Power_Calculator_GetHrtimLagBuffAddress(uint8_t ch);
int16_t* FB_Power_Calculator_GetVoltageBuffAddress(uint8_t ch);
int16_t* FB_Power_Calculator_GetTxaBuffAddress(uint8_t ch);
int16_t  FB_Power_Calculator_GetTxaBuffSize(uint8_t ch);
PowerCalculatorInputDef* FB_Power_Calculator_GetInputArrayAddress(uint8_t ch);

#endif // POWER_CALCULATOR_FULLBRIDGE_H
