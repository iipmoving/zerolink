/**
 * @file    app_power_io.h
 * @brief   PowerBase Data Switcher IO interface
 * @layer   app (Data Switcher IO)
 *
 * 本文件定义 PowerBase 模块对外暴露的输入/输出接口。
 * 每炉头独立逻辑单元 — PowerBase_InHead_t / PowerBase_OutHead_t。
 * 中间层通过 InputCallback 类型化指针直传。
 */

#ifndef APP_POWER_IO_H
#define APP_POWER_IO_H

#include <stdint.h>
#include "std_module.h"
//#include "app_adc_io.h"

#pragma pack(4)

/* ---- 前置声明 ---- */

#define	POWER_POTMAX	4

//struct PowerCalc_Output;

/* ---- 单炉头输入 — MODBUS 命令 ---- */
typedef struct {
    uint16_t target_power;
    uint8_t  power_on;
} PowerComm_InHead_t;


typedef struct {
	
		uint16_t current;
		uint16_t voltage;
	
		uint16_t igbt;
		uint16_t bottom;
	
		uint16_t phase;
		uint16_t phaseDown;
	
		uint16_t ceilQ;
		uint16_t power;

	
}PowerAdc_InHead_t;	



/* ---- 输入: 类型化指针 + 炉头数组 ---- */
typedef struct {
    PowerAdc_InHead_t        pAdc[POWER_POTMAX];          /* → AppAdc                       */
//    struct PowerCalc_Output *pCalc;         /* → PowerCalc 反馈                */
    PowerComm_InHead_t       head[POWER_POTMAX];       /* 每炉头独立命令                   */
    
} PowerBase_Input_t;

/* ---- 单炉头输出 ---- */
typedef struct {
    int16_t ppg_delta;
    uint8_t delta_valid;
    uint8_t power_state;
} PowerBase_OutHead_t;

/* ---- 输出: 炉头数组 ---- */
typedef struct PowerBase_Output {

    PowerBase_OutHead_t  head[4];

} PowerBase_Output_t;

#pragma pack()

/* ---- v2.2 统一接口 ---- */
MODULE_IO_H(PowerBase);

#endif /* APP_POWER_IO_H */
