/********************************************************************************
    FileName    :  drv_hrtim.h
    Author      :  rsl
    Version     :  V1.0.0
    Brief       :  HRTIM 驱动层封装头文件

    Date        :  2026-06-09
    Modify      :
                   2026-06-09 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/

#ifndef __DRV_HRTIM_H__
#define __DRV_HRTIM_H__

#include <stdint.h>

/* ===================================================================
 * 类型定义
 * =================================================================== */

/**
 * @brief PPG 值结构体
 */
typedef struct {
    uint16_t period;  // PPG 周期
    uint16_t duty;    // PPG 占空比
} drv_hrtim_ppg_value_t;

/* ===================================================================
 * 限流保护相关接口
 * =================================================================== */

void drv_hrtim_disable_it_rest(void);
void drv_hrtim_enable_it_rest(void);

/* ===================================================================
 * 检锅相关接口
 * =================================================================== */

void drv_hrtim_check_pan_pluse(uint8_t ch);

/* ===================================================================
 * 主同步模式接口
 * =================================================================== */

void drv_hrtim_master_sync_set_period(uint16_t period);

/* ===================================================================
 * PPG 通用接口（带通道参数）
 * =================================================================== */

void drv_hrtim_ppg_set_value(uint8_t ch, drv_hrtim_ppg_value_t value);
drv_hrtim_ppg_value_t drv_hrtim_ppg_get_value(uint8_t ch);
void drv_hrtim_ppg_set_duty(uint8_t ch, uint16_t duty);
void drv_hrtim_ppg_dead_time(uint8_t ch, uint8_t upDts, uint8_t downDts);
void drv_hrtim_ppg_on_off(uint8_t ch, uint8_t flag);
uint8_t drv_hrtim_ppg_get_bk_flag(uint8_t ch);
void drv_hrtim_ppg_set_continuous(uint8_t ch);
void drv_hrtim_ppg_set_single(uint8_t ch);
void drv_hrtim_ppg_set_pulse(uint8_t ppgCh, uint16_t value);
void drv_hrtim_ppg_set_value_chx(uint8_t ch, uint16_t period, uint16_t duty);

/* ===================================================================
 * PPG 通道特定接口（用于函数指针绑定）
 * =================================================================== */

// 通道 1
void drv_hrtim_ppg_ch1_set_value(drv_hrtim_ppg_value_t value);
drv_hrtim_ppg_value_t drv_hrtim_ppg_ch1_get_value(void);
void drv_hrtim_ppg_ch1_set_duty(uint16_t duty);
void drv_hrtim_ppg_ch1_dead_time(uint8_t upDts, uint8_t downDts);
void drv_hrtim_ppg_ch1_on_off(uint8_t flag);
uint8_t drv_hrtim_ppg_ch1_get_bk_flag(void);

// 通道 2
void drv_hrtim_ppg_ch2_set_value(drv_hrtim_ppg_value_t value);
drv_hrtim_ppg_value_t drv_hrtim_ppg_ch2_get_value(void);
void drv_hrtim_ppg_ch2_set_duty(uint16_t duty);
void drv_hrtim_ppg_ch2_dead_time(uint8_t upDts, uint8_t downDts);
void drv_hrtim_ppg_ch2_on_off(uint8_t flag);
uint8_t drv_hrtim_ppg_ch2_get_bk_flag(void);

// 通道 3
void drv_hrtim_ppg_ch3_set_value(drv_hrtim_ppg_value_t value);
drv_hrtim_ppg_value_t drv_hrtim_ppg_ch3_get_value(void);
void drv_hrtim_ppg_ch3_set_duty(uint16_t duty);
void drv_hrtim_ppg_ch3_dead_time(uint8_t upDts, uint8_t downDts);
void drv_hrtim_ppg_ch3_on_off(uint8_t flag);
uint8_t drv_hrtim_ppg_ch3_get_bk_flag(void);

// 通道 4
void drv_hrtim_ppg_ch4_set_value(drv_hrtim_ppg_value_t value);
drv_hrtim_ppg_value_t drv_hrtim_ppg_ch4_get_value(void);
void drv_hrtim_ppg_ch4_set_duty(uint16_t duty);
void drv_hrtim_ppg_ch4_dead_time(uint8_t upDts, uint8_t downDts);
void drv_hrtim_ppg_ch4_on_off(uint8_t flag);
uint8_t drv_hrtim_ppg_ch4_get_bk_flag(void);

#endif /* __DRV_HRTIM_H__ */