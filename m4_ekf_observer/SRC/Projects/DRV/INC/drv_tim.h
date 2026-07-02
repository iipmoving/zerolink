/*********************************************************************************************
Copyright <2024> <Icore Technology (Nanjing)  Co.,Ltd>
All Rights Reserved,
Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials provided
with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to
endorse or promote products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************/

/*
*********************************************************************************************************
*                                              rx32g4xx
*                                           Library Function
*
*                                   Copyright 2024, RX Tech, Corp.
*                                        All Rights Reserved
*
*
* Project      : rx32g4xx
* File         : drv_tim.h
* By           : moving
*********************************************************************************************************
*/

#ifndef __DRV_TIM_H__
#define __DRV_TIM_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
//#include <stdio.h>
//#include <stdint.h>
//#include <stdbool.h>
//#include "rx32g4xx.h"

#include "rx32g4xx_hal_tim.h"
#include "rx32g4xx_hal_tim_ex.h"
#include "rx32g4xx_hal_gpio.h"
#include "drv_gpio.h"


/* Exported typedef ----------------------------------------------------------*/

typedef enum                //TIMX对应数字序号，
{
  TIM1CH = 0x00U,
  TIM2CH,   
  TIM3CH,
  TIM4CH,
  TIM5CH,
  TIM6CH,
  TIM7CH,
  TIM8CH,
  TIM15CH,
  TIM16CH,
  TIM17CH,  
} TIMCH_ENUM_DEF;





typedef struct                          //与TIMX对应的差别项，
{
    TIMCH_ENUM_DEF  timch;                  //与TIMX对应的序号
    uint8_t         onePluse;                   //保留
    uint8_t         res2;                   //保留
    uint8_t         res3;                   //保留
    TIM_TypeDef*    timx;                   //TIM1,~17   
	int	            channel;                //输出的通道    @arg TIM_CHANNEL_1: TIM Channel 1 selected
	IRQn_Type	    irqx;                   //中断号


}TIMsetDef;	





typedef struct                      //完整TIM设置集合
{
		TIMsetDef*									timSet;                 //定时器差异点设置 
		TIM_Base_InitTypeDef*						initBase;               //定时器时基设置 
		TIM_MasterConfigTypeDef*					masterConfig;           //主设备设置
		TIM_SlaveConfigTypeDef*						slaveConfig;            //从设备设置 
		TIM_OC_InitTypeDef*							ocInit;					//TIME输出设置	
		TIMEx_BreakInputConfigTypeDef*		        breakInput;				//BREAK源设置	
		TIMEx_BreakInputConfigTypeDef*		        break2Input;			//BREAK2源设置
		TIM_BreakDeadTimeConfigTypeDef*		        breakDeadTime;		    //BREAK设置
		GPIOsetGroupDef*							gpioGroup;				//TIM GPIO设置
		TIM_HandleTypeDef	htimx;
	
}TIMstrDef;	



/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

/* Exported function prototypes ----------------------------------------------*/
void	TIM_PWM_INIT(TIMstrDef* tim);					//互补PWM输出初始化
void	TIM_OnePluse_INIT(TIMstrDef* tim);				//单脉冲输出初始化	



#ifdef __cplusplus
}
#endif

#endif /* __DRV_TIM_H__ */
