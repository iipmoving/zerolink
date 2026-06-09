/**
 * @file    power_protect.h
 * @brief   Power 保护功能包 — 浪涌/OVP/限流/过温判断
 * @layer   app
 *
 * 从 surge_Processing / getOvpValueAdj / igbt_derate / top_temp_stop 抽出。
 * 纯业务判断，不访问 HRTIM 硬件。
 * 硬件状态通过 in->hw_status.bk_flag / fault 输入。
 */
#ifndef POWER_PROTECT_H
//#define POWER_PROTECT_H

#include <stdint.h>
#include "app_power_io.h"

/**
 * @brief  保护检查（单炉头）
 * @param  in   APP 输入（含 ADC + hw_status 反馈）
 * @param  ch   炉头通道号
 * @return      非零 = 保护触发
 */
uint8_t PowerProtect_Run(PowerBase_Input_t *in, uint8_t ch);

#endif /* POWER_PROTECT_H */
