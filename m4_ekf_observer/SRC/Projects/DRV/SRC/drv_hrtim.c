/********************************************************************************
    FileName    :  drv_hrtim.c
    Author      :  rsl
    Version     :  V1.0.0
    Brief       :  HRTIM 驱动层封装，隔离 APP_POWER 与 API_HRTIM 的直接依赖

    Date        :  2026-06-09
    Modify      :
                   2026-06-09 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/

#include "drv_hrtim.h"
#include "API_hrtim.h"

/* ===================================================================
 * 限流保护相关接口
 * =================================================================== */

void drv_hrtim_disable_it_rest(void)
{
    API_HRTIM_DISABLE_IT_REST();
}

void drv_hrtim_enable_it_rest(void)
{
    API_HRTIM_ENABLE_IT_REST();
}

/* ===================================================================
 * 检锅相关接口
 * =================================================================== */

void drv_hrtim_check_pan_pluse(uint8_t ch)
{
    API_HRTIM_CHECK_PAN_PLUSE(ch);
}

/* ===================================================================
 * 主同步模式接口
 * =================================================================== */

void drv_hrtim_master_sync_set_period(uint16_t period)
{
    API_HRTIM_MasterSync_SetPeriod(period);
}

/* ===================================================================
 * PPG 通用接口（带通道参数）
 * =================================================================== */

void drv_hrtim_ppg_set_value(uint8_t ch, drv_hrtim_ppg_value_t value)
{
    PPGvalueDef apiValue;
    apiValue.prioed = value.period;
    apiValue.duty = value.duty;
    API_PPG_setValue(ch, apiValue);
}

drv_hrtim_ppg_value_t drv_hrtim_ppg_get_value(uint8_t ch)
{
    PPGvalueDef apiValue = API_PPG_getValue(ch);
    drv_hrtim_ppg_value_t value;
    value.period = apiValue.prioed;
    value.duty = apiValue.duty;
    return value;
}

void drv_hrtim_ppg_set_duty(uint8_t ch, uint16_t duty)
{
    API_PPG_setValueChx(ch, 0, duty);
}

void drv_hrtim_ppg_dead_time(uint8_t ch, uint8_t upDts, uint8_t downDts)
{
    API_PPG_DeadTime(ch, upDts, downDts);
}

void drv_hrtim_ppg_on_off(uint8_t ch, uint8_t flag)
{
    API_PPG_OnOff(ch, flag);
}

uint8_t drv_hrtim_ppg_get_bk_flag(uint8_t ch)
{
    switch(ch)
    {
        case 1: return API_PPG_BkFlag_Pot1();
        case 2: return API_PPG_BkFlag_Pot2();
        case 3: return API_PPG_BkFlag_Pot3();
        case 4: return API_PPG_BkFlag_Pot4();
        default: return 0;
    }
}

void drv_hrtim_ppg_set_continuous(uint8_t ch)
{
    API_PPG_SET_CONTINUOUS(ch);
}

void drv_hrtim_ppg_set_single(uint8_t ch)
{
    (void)ch;
    // API_PPG_SET_SINGLE 未实现，暂时为空函数
}

void drv_hrtim_ppg_set_pulse(uint8_t ppgCh, uint16_t value)
{
    API_PPG_setPluse(ppgCh, value);
}

void drv_hrtim_ppg_set_value_chx(uint8_t ch, uint16_t period, uint16_t duty)
{
    API_PPG_setValueChx(ch, period, duty);
}

/* ===================================================================
 * PPG 通道特定接口（用于函数指针绑定）
 * =================================================================== */

// 通道 1
void drv_hrtim_ppg_ch1_set_value(drv_hrtim_ppg_value_t value)
{
    drv_hrtim_ppg_set_value(1, value);
}

drv_hrtim_ppg_value_t drv_hrtim_ppg_ch1_get_value(void)
{
    return drv_hrtim_ppg_get_value(1);
}

void drv_hrtim_ppg_ch1_set_duty(uint16_t duty)
{
    drv_hrtim_ppg_set_duty(1, duty);
}

void drv_hrtim_ppg_ch1_dead_time(uint8_t upDts, uint8_t downDts)
{
    drv_hrtim_ppg_dead_time(1, upDts, downDts);
}

void drv_hrtim_ppg_ch1_on_off(uint8_t flag)
{
    drv_hrtim_ppg_on_off(1, flag);
}

uint8_t drv_hrtim_ppg_ch1_get_bk_flag(void)
{
    return drv_hrtim_ppg_get_bk_flag(1);
}

// 通道 2
void drv_hrtim_ppg_ch2_set_value(drv_hrtim_ppg_value_t value)
{
    drv_hrtim_ppg_set_value(2, value);
}

drv_hrtim_ppg_value_t drv_hrtim_ppg_ch2_get_value(void)
{
    return drv_hrtim_ppg_get_value(2);
}

void drv_hrtim_ppg_ch2_set_duty(uint16_t duty)
{
    drv_hrtim_ppg_set_duty(2, duty);
}

void drv_hrtim_ppg_ch2_dead_time(uint8_t upDts, uint8_t downDts)
{
    drv_hrtim_ppg_dead_time(2, upDts, downDts);
}

void drv_hrtim_ppg_ch2_on_off(uint8_t flag)
{
    drv_hrtim_ppg_on_off(2, flag);
}

uint8_t drv_hrtim_ppg_ch2_get_bk_flag(void)
{
    return drv_hrtim_ppg_get_bk_flag(2);
}

// 通道 3
void drv_hrtim_ppg_ch3_set_value(drv_hrtim_ppg_value_t value)
{
    drv_hrtim_ppg_set_value(3, value);
}

drv_hrtim_ppg_value_t drv_hrtim_ppg_ch3_get_value(void)
{
    return drv_hrtim_ppg_get_value(3);
}

void drv_hrtim_ppg_ch3_set_duty(uint16_t duty)
{
    drv_hrtim_ppg_set_duty(3, duty);
}

void drv_hrtim_ppg_ch3_dead_time(uint8_t upDts, uint8_t downDts)
{
    drv_hrtim_ppg_dead_time(3, upDts, downDts);
}

void drv_hrtim_ppg_ch3_on_off(uint8_t flag)
{
    drv_hrtim_ppg_on_off(3, flag);
}

uint8_t drv_hrtim_ppg_ch3_get_bk_flag(void)
{
    return drv_hrtim_ppg_get_bk_flag(3);
}

// 通道 4
void drv_hrtim_ppg_ch4_set_value(drv_hrtim_ppg_value_t value)
{
    drv_hrtim_ppg_set_value(4, value);
}

drv_hrtim_ppg_value_t drv_hrtim_ppg_ch4_get_value(void)
{
    return drv_hrtim_ppg_get_value(4);
}

void drv_hrtim_ppg_ch4_set_duty(uint16_t duty)
{
    drv_hrtim_ppg_set_duty(4, duty);
}

void drv_hrtim_ppg_ch4_dead_time(uint8_t upDts, uint8_t downDts)
{
    drv_hrtim_ppg_dead_time(4, upDts, downDts);
}

void drv_hrtim_ppg_ch4_on_off(uint8_t flag)
{
    drv_hrtim_ppg_on_off(4, flag);
}

uint8_t drv_hrtim_ppg_ch4_get_bk_flag(void)
{
    return drv_hrtim_ppg_get_bk_flag(4);
}