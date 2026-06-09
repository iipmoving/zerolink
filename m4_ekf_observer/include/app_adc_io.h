/**
 * @file    app_adc_io.h
 * @brief   APP_ADC Data Switcher IO interface
 * @layer   app (Data Switcher IO)
 *
 * 本文件定义 APP_ADC 模块对外暴露的 I/O 接口。
 * 仅 data_switcher.c 和 APP_ADC.C 自身可用全路径 include 本文件。
 *
 * === v2.2 PULL 范式 ===
 * AdcValues_t 暴露在公开头，中间层使用类型化指针 AdcValues_t* 路由。
 * Union AdcBlock_t 兼容旧代码 inputValue[AdcGroupXxx] 枚举下标访问。
 * 每炉头独立 AdcHeadValues_t，循环时只传 &fields.head[ch] 一个指针。
 */

#ifndef APP_ADC_IO_H
#define APP_ADC_IO_H

#include <stdint.h>
#include "std_module.h"

/* ---- 单炉头数据 — 每炉头独立空间 ---- */
//typedef struct {
//    uint32_t power;
//    uint32_t current;
//    uint32_t phase;
//    uint32_t phase_down;
//    uint32_t ceil_q;
//    uint32_t bottom_temp;
//    uint32_t igbt_temp;
//} AdcHeadValues_t;


#define	ADC_POTMAX	4
typedef struct	__attribute__((aligned(32))) {
	
	uint16_t 	voltage;	
	uint16_t 	Fan;
	
	uint16_t	power[ADC_POTMAX];		//MASTER CURRENT AD
	uint16_t 	txa[ADC_POTMAX];			//电
	uint16_t 	bottom[ADC_POTMAX];
	uint16_t 	igbt[ADC_POTMAX];
	uint16_t 	ceilQ[ADC_POTMAX];	
	uint16_t 	phase[ADC_POTMAX];		
	uint16_t 	phaseDown[ADC_POTMAX];		
	
}APP_Adc_Output_t;				//与APP_ADC_DEF	inputvalue[]一样 所以不需要实例



///* ---- 全局数据 + 炉头数组 ---- */
//typedef struct {
//    uint32_t        voltage;         /* 全局(非炉头) */
//    uint32_t        fan;             /* 全局 */
//    AdcHeadValues_t head[4];         /* 每炉头独立 */
//} AdcValues_t;

///* ---- Union — 旧代码兼容 + 新代码类型安全 ---- */
//typedef union {
//    uint32_t    inputValue[30];  /* 旧代码: inputValue[AdcGroupVoltage] */
//    AdcValues_t fields;          /* 新代码: fields.head[ch].phase      */
//} AdcBlock_t;

#pragma pack(4)



#pragma pack()

/* ---- v2.2 统一接口 ---- */
MODULE_IO_H(APP_Adc);

#endif /* APP_ADC_IO_H */
