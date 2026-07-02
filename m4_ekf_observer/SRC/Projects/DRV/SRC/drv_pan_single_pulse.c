/********************************************************************************
    FileName    :  drv_pan_single_pulse.c
    Author      :  rsl
    Version     :  V1.0.0
    Brief       :  检锅单脉冲驱动 — 包装 API_PPG_SET_SINGLE 给 APP 层调用

                    替代原有连续多脉冲模式 (drv_hrtim_check_pan_pluse):
                    API_HRTIM_CHECK_PAN_PLUSE 输出多个脉冲后关断
                    本函数调用 API_PPG_SET_SINGLE 输出 1 个脉冲后自动停止

    Date        :  2026-06-24
    Modify      :
                   2026-06-24 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/

#include "drv_pan_single_pulse.h"
#include "API_HRTIM.h"

void drv_pan_single_pulse_start(uint8_t ch)
{
    if (ch == 0 || ch > 4) return;
    API_PPG_SET_SINGLE(ch);
}