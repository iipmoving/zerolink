/**
 * @file    app_power_calc_io.h
 * @brief   PowerCalc Data Switcher IO interface
 * @layer   app (Data Switcher IO)
 *
 * 本文件定义 PowerCalc 模块对外暴露的输入/输出接口。
 * 输入: AdcBlock_t* (来自 AppAdc, raw ptrs) + PowerBase_Output_t* (ppg_delta)
 * 输出: PowerCalc_OutHead_t 数组 (窗口积分: active_power + peak_current)
 * 每炉头独立 PowerCalc_OutHead_t。
 */

#ifndef APP_POWER_CALC_IO_H
#define APP_POWER_CALC_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ---- 前置声明 (避免循环引用) ---- */
struct PowerBase_Output;

/* ---- 输入: 指针聚合 ---- */
typedef struct {
    uint8_t                   status;
    uint8_t                   res[3];
    AdcBlock_t               *pAdc;          /* → AppAdc (raw ptrs)             */
    struct PowerBase_Output  *pBase;         /* → PowerBase (ppg_delta)         */
} PowerCalc_Input_t;

/* ---- 单炉头输出 — 窗口积分结果 ---- */
typedef struct {
    int32_t  active_power;
    int32_t  active_current;
    uint16_t peak_current;
    uint16_t voltage;
} PowerCalc_OutHead_t;

/* ---- 输出: 炉头数组 ---- */
typedef struct PowerCalc_Output {
    uint8_t              status;
    uint8_t              res[3];
    PowerCalc_OutHead_t  head[4];
    uint8_t              valid;
} PowerCalc_Output_t;

#pragma pack()

/* ---- v2.2 统一接口 ---- */
MODULE_IO_H(PowerCalc);

#endif /* APP_POWER_CALC_IO_H */
