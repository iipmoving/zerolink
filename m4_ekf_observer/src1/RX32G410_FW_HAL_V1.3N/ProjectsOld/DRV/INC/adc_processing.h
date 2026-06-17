#ifndef __adc_processing_H__
#define __adc_processing_H__

// adc_processing.h
#pragma once

#include <stdint.h>

#define NUM_BLOCKS 4
#define SAMPLES_PER_BLOCK 8

typedef struct
{
	uint16_t getMax:	8;			//连续采集MAX的平均值（20次） 
	uint16_t awgT12A:	1;			//ADC1发生AWG过流保护（找当次采集那个通道触发）
	uint16_t awgT34A:	1;			//ADC1发生AWG过流保护（找当次采集那个通道触发）
}ADC_Buff1_FlagDef;




typedef struct {
	

	
    uint32_t T1A[NUM_BLOCKS * SAMPLES_PER_BLOCK];
    uint32_t T2A[NUM_BLOCKS * SAMPLES_PER_BLOCK];
    uint32_t T3A[NUM_BLOCKS * SAMPLES_PER_BLOCK];
    uint32_t T4A[NUM_BLOCKS * SAMPLES_PER_BLOCK];
    
    uint32_t sumT1A[NUM_BLOCKS];
    uint32_t sumT2A[NUM_BLOCKS];
    uint32_t sumT3A[NUM_BLOCKS];
    uint32_t sumT4A[NUM_BLOCKS];
	
	uint8_t start;							//DMA开始点（0，或HALF)
	uint8_t end;
	union{
	ADC_Buff1_FlagDef	flag;
	uint16_t	halfWord;
	};										//标志点，切换是否需要采最大点
	uint32_t hrtim_start;
	uint32_t hrtim_end;
		
	
	
	
} ADC_Buffer_t;

//extern ADC_Buffer_t TxA_ADC_TimDmaBuff;
extern void APP_ADC_AvgTxA20us(void);
void	API_POWER_PanCheckPluseAsm(void);

#endif
