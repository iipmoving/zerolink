/**
 * @file    power_pid.h
 * @brief   Power PID 功能包 — 功率闭环计算
 * @layer   app
 *
 * 从 s_ppg_fun() 抽出。输入 ADC + 目标功率，输出 ppg_delta。
 * 不访问任何 HRTIM 硬件。
 */
#ifndef POWER_PID_H
//#define POWER_PID_H

#include <stdint.h>
#include "app_power_io.h"

/**
 * @brief  PID 初始化（单炉头）
 * @param  ch      炉头通道号
 * @param  kp      比例系数
 * @param  ki      积分系数
 * @param  kd      微分系数
 * @param  factor  定点数比例因子
 */
void PowerPid_Init(uint8_t ch, float kp, float ki, float kd, int factor);

/**
 * @brief  设置功率校准系数
 * @param  calib  校准值 (对应 g_p25_ad)
 */
void PowerPid_SetCalib(uint16_t calib);

/**
 * @brief  PID 计算（单炉头）
 * @param  in           APP 输入（含 ADC + hw_status 反馈）
 * @param  out          当前炉头输出 (ppg_delta / delta_valid)
 * @param  ch           炉头通道号
 * @param  target_power 目标功率 ADC 值（来自 MODBUS）
 */
void PowerPid_Compute(PowerBase_Input_t *in, PowerBase_OutHead_t *out, uint8_t ch, uint16_t target_power);

#endif /* POWER_PID_H */
