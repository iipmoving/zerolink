/********************************************************************************
    FileName    :  drv_pan_single_pulse.h
    Author      :  rsl
    Version     :  V1.0.0
    Brief       :  检锅单脉冲 HRTIM SimpleOnePulse 驱动

    Date        :  2026-06-24
    Modify      :
                   2026-06-24 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/

#ifndef __DRV_PAN_SINGLE_PULSE_H__
#define __DRV_PAN_SINGLE_PULSE_H__

#include <stdint.h>

/**
 * @brief 启动检锅单脉冲 (HRTIM SimpleOnePulse 模式)
 *        输出 1 个激励脉冲后自动停止
 * @param ch PotCh (1-4)
 * @note  脉冲宽度 = 当前 HRTIM 周期 × 60%，与原有多脉冲逻辑一致
 *        完成后触发 API_HRTIM_PanOffCallBack，可在此回调中启动 ADC DMA 采集
 */
void drv_pan_single_pulse_start(uint8_t ch);

#endif /* __DRV_PAN_SINGLE_PULSE_H__ */