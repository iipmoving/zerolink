/**
 * @file    power_pan_if.h
 * @brief   检锅触发接口 — 纯业务判断
 * @layer   app
 *
 * 只判断"当前是否需要发起检锅"，设置 pan_request 标志。
 * 不操作任何 HW（脉冲产生/计数等全在 DRV pan_detect Route）。
 *
 * 检锅状态机全在 DRV 侧 drv_pan_detect.c，本文件只管"何时触发"。
 */
#ifndef POWER_PAN_IF_H
//#define POWER_PAN_IF_H

#include <stdint.h>
#include "app_power_io.h"

/**
 * @brief  检锅触发判断（单炉头）
 * @param  in          APP 输入（含 ADC + hw_status）
 * @param  ch          炉头通道号
 * @param  pan_request 输出: 1=请求检锅, 0=无请求
 * @param  pan_ch      输出: 请求检锅的通道号
 */
void PowerPanIf_Run(PowerBase_Input_t *in, uint8_t ch, uint8_t *pan_request, uint8_t *pan_ch);

#endif /* POWER_PAN_IF_H */
