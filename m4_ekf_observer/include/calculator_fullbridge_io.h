/**
 * @file    calculator_fullbridge_io.h
 * @brief   Full-Bridge Calculator Data Switcher IO interface
 * @layer   base_class (Data Switcher IO)
 */
#ifndef CALCULATOR_FULLBRIDGE_IO_H
#define CALCULATOR_FULLBRIDGE_IO_H

#include <stdint.h>
#include <stdbool.h>
#include "std_module.h"

/* ---- 前置声明 ---- */
#define FB_CALC_POTMAX    4

/* ---- 对角管导通状态枚举 ---- */
typedef enum {
    FB_CONDUCTION_NONE = 0,     // 无对角管导通
    FB_CONDUCTION_FORWARD = 1,  // 正向电流窗口：Q1+Q4导通
    FB_CONDUCTION_REVERSE = 2,  // 反向电流窗口：Q2+Q3导通
} FBConductionStateTypeDef;

/* ---- HRTIM 状态 ---- */
typedef struct {
    uint16_t highOn;       // 超前臂死区后上管开通 HRTIM 值
    uint16_t highOff;      // 超前臂上管关断 (DUTY) HRTIM 值
    uint16_t lowOn;        // 超前臂死区后下管开通 HRTIM 值
    uint16_t lowOff;       // 超前臂下管关断 (period) HRTIM 值
    uint16_t lagDuty;      // 滞后臂占空比 CMP 值
} FB_Calculator_HrtimState;

/* ---- 单炉头输入参数 ---- */
typedef struct {
    uint16_t start;              // HRTIM 开始点索引
    uint16_t end;                // HRTIM 结束点索引
    uint16_t highOn;             // 超前臂死区后上管开通
    uint16_t highOff;            // 超前臂上管关断
    uint16_t lowOn;              // 超前臂死区后下管开通
    uint16_t lowOff;             // 超前臂下管关断
    uint16_t zero_cross_high;    // 上管过零点索引
    uint16_t zero_cross_low;     // 下管过零点索引
    uint16_t perAdc;             // 每个 ADC 采样对应的 HRTIM 计数
    uint16_t lagDuty;            // 滞后臂占空比 CMP 值
} FB_Calculator_InputParams_t;

/* ---- 单炉头输入数据 ---- */
typedef struct {
    uint16_t* resonant_current;   // 谐振电流 ADC 数组
    uint16_t* hrtim_lead;         // 超前臂(TimerB)计数器数组
    uint16_t* hrtim_lag;          // 滞后臂(TimerE)计数器数组
    uint16_t* voltage_data;       // 母线电压 ADC 数组
    FB_Calculator_InputParams_t params; // 工作周期参数
} FB_Calculator_InHead_t;

/* ---- 输入: 炉头数组 ---- */
typedef struct {
    FB_Calculator_InHead_t head[FB_CALC_POTMAX];
} FB_Calculator_Input_t;

/* ---- 单炉头输出 ---- */
typedef struct {
    FB_Calculator_HrtimState hrtim;
    uint16_t peak_current;        // 峰值电流 (ADC 值)
    uint16_t active_current;      // 有功电流 (整数定标)
    uint16_t voltage;             // 瞬时电压均值 (ADC 值)
    uint16_t zero_cross_high;     // 上管过零点索引
    uint16_t zero_cross_low;      // 下管过零点索引
    uint16_t zero_current;        // 过零点电流值
    int16_t  phase_angleUp;       // 上管相位角 (度×10)
    int16_t  phase_angleDown;     // 下管相位角 (度×10)
    uint16_t esr;                 // 等效电阻 (整数定标)
    int32_t  active_power;        // 有功功率 (整数定标)
} FB_Calculator_OutHead_t;

/* ---- 输出: 炉头数组 ---- */
typedef struct {
    FB_Calculator_OutHead_t head[FB_CALC_POTMAX];
} FB_Calculator_Output_t;

/* ---- v2.2 统一接口声明 ---- */
MODULE_IO_H(FB_Calculator);

#endif /* CALCULATOR_FULLBRIDGE_IO_H */