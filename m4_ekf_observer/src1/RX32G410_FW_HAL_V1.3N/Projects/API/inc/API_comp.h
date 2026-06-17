#ifndef __COMP_H__
#define __COMP_H__



//extern COMP_HandleTypeDef hcomp1;
//extern COMP_HandleTypeDef hcomp2;
//extern COMP_HandleTypeDef hcomp3;
//extern COMP_HandleTypeDef hcomp4;

//extern void Error_Handler(void);

#define		COMP_CR_OFLT_VALUE		COMP_CR_OFLT_DIV16		//输出消抖次数

typedef enum
{
	ChComp1=0,		//COMP1_P1 PA0		ADC1_4//对应API ADC内部的HANDLE
	ChComp2,		//COMP2_P1 PA1		ADC1_5
	ChComp3,		//COMP3_P1 PB13		ADC3_5	
	ChComp4,		//COMP4_P1 PB14	 	ADC3_6
//	ChAdc1_Tempe,
//	ChAdc2_FanAd,
//	ADC_Select_WaitZero,						//等过零信号
}COMP_CH_ENUM;//ADCtemp



void API_COMP_Init(void);

#endif
