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
* File         : DRV_MCU.c
* By           : MOVING
*********************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
//#include "main.h"
//#include "rx32g4xx_config_def.h"
//#include "rx32g4xx_hal.h"
//#include	"drv_tim.h"

//#include  "TIM_DEFINE.H"

//#include "system_init.h"
//#include "system_bsp.h"



#include "s_time_base.h"

//#include	"APP_ZERO.H"
#include	"API_gpio.h"
//#include 	"API_hrtim.h"
#include 	"API_TIM.h"
#include 	"API_ADC.h"
//#include 	"API_COMP.h"
//#include	"API_I2C.H"

#include	"APP_ADC.H"
//  #include "cmsis_gcc.h"
#include	"APP_ZERO.H"
#include	"app_power.h"  
/* Private typedef -----------------------------------------------------------*/






/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/



/* Private functions ---------------------------------------------------------*/


void 	TimIrqHandleZeroOccur(void);

/* CallBack function prototypes ----------------------------------------------*/
__attribute__((weak))	void	APP_ZERO_IRQ_PPGstepChangeFunCallBack(void)				//100US回调函数, 在APP_POWER 中实现PPG逼近，自动增减
{}	

__attribute__((weak))		void APP_ZERO_Adc100usCallBack(void)			//100us ADC回调
{

}	
__attribute__((weak))		void APP_ZERO_Adc1msCallBack(void)			//100us ADC回调
{

}	



__attribute__((weak))		void	APP_ADC_ZERO_IrqFun(void)
{}	
	
/**
  * @brief  Initialize the GPIOx peripheral according to the specified parameters in the GPIO_Init.连带IO口号一起运算
  * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral for RX32G4xx family
  * @param  GPIO_Init: pointer to a GPIO_InitTypeDef structure that contains
  *         the configuration information for the specified GPIO peripheral.
            @arg @ref GPIO_InitTypeDef
  * @retval None
  */


typedef struct {

	unsigned char ZERO_Count_UP;	//高电平宽度
	unsigned char ZERO_Count_DOWN;	//低电平宽度
	unsigned char ZERO_Count_NOW;	//当前电平宽度
	unsigned char ZERO_DIC;			//过零方向

	unsigned char count;			//计数	
	unsigned char res3;			//计数	
	unsigned char res2;			//计数	
	unsigned char res1;			//计数	
			
	
	
}STR_ZRERO;

STR_ZRERO	zero_str;


#define	ZeroStatus 		zero_str.ZERO_DIC
#define	Zero_Count_UP 	zero_str.ZERO_Count_UP
#define	Zero_Count_DOWN zero_str.ZERO_Count_DOWN
#define	ACZeroCount 	zero_str.ZERO_Count_NOW
#define Time_ac_count	zero_str.count





void	API_MCU_100US_IRQHandler(void)
{


		uint8_t zeroCount=0;
//		API_ADC_VcIc_ENABLE_IT_EOC();			//	开启VCIC中断 


		ACZeroCount++;
		Time_ac_count++;
	
	
//		I2cIrqCn=0;	
//		Adc20msCount++;

	

	
//		if(Adc20msCount<AdcAvageCount)
//		{	
//			
////			ADC_Start_JSW(ADC1);									//开启注入通道自动转换	
//		}
		
	

#if 1					


		if(API_GPIO_ReadPin(Zero_pin))
		{
				zeroCount++;
		}	
		
		
		
		
		if(API_GPIO_ReadPin(Zero_pin))
		{
				zeroCount++;
		}

//		RtosFunTable[TIM6_PPGstepChangeFun].Function();				//过零信号产生回调函数，由用户实现，主要用来同步系统时间片调度清零
						
		APP_ZERO_IRQ_PPGstepChangeFunCallBack();				//PPG逼近，自动增减
		
		
		if(API_GPIO_ReadPin(Zero_pin))
		{
				zeroCount++;
		}	
#endif
		
		switch (zeroCount)
		{
			case	0:
				
				if(ACZeroCount>90)
				{
					if(ZeroStatus)
					{										//产生下降沿翻转
							Zero_Count_UP=ACZeroCount;
	


							TimIrqHandleZeroOccur();

						

							
					}
					ZeroStatus=0;
				}	
			break;
			case	3:
				
				if(ACZeroCount>190)
				{	
					if(ZeroStatus==0)
					{
						Zero_Count_DOWN=ACZeroCount;	//低电平宽度
						ACZeroCount = 0;	//上升沿到达，重置

					}				
					ZeroStatus=1;
					
				}	
			break;

		}		
		
		if((Time_ac_count%10)==0)
		{
			//每一ms初始化一次HRTIM 同步采樣T1A		
			Time_SetMsFlg();
			APP_ZERO_Adc1msCallBack();
		}		
		APP_ZERO_Adc100usCallBack();
}	





/* Exported function prototypes ----------------------------------------------*/



static uint16_t 	AdcOvpValue[2];												//ADC限流设置值 

											//20ms采样完成标志
static	uint8_t 	AdcTempe=0;				//切换ADC VOLATAG与TEMPE
static	uint8_t     AdcSetEoc=0;			//ADC规则通道后是否打开注入通道中断  1：打开，读取谐振电流值一直到下一100us 
static 	uint8_t		s_50HZ_60HZ_times=0;

#define		Systick50Hz		152*1000				//1ms
#define		Systick60Hz		152*833					//0.833ms



void	s_power_50HZ_fun(void)
{

	
		if(Zero_Count_DOWN>AdcAvageCount+10)	 	//时间太长，说明是当前为50HZ工作，
		{					  
		   s_50HZ_60HZ_times++;
		   if(s_50HZ_60HZ_times>30)
		   {
		   		s_50HZ_60HZ_times=35;
			   
#if 0	
在TIM6中封装			   
				TIM_Set_OC_CompareCH3(Timx100us, TIM8_CCR);
				TIM_Set_AutoReload(Timx100us,TIM6_ARR_50hz-1);	//初始按60HZ设置
									//从60HZ切换为50HZ
#endif 	
				API_TIM_100US_SET_50Hz();
			   
		   }
		}
		else
		{
		   if(s_50HZ_60HZ_times<30)
		   {			
		   		s_50HZ_60HZ_times=0;
		   }	
		}

 }


#include	"app_power.h"



void 	TimIrqHandleZeroOccur(void)					//过零点
{ 
						s_power_50HZ_fun();			//判断50/60hz
						Time_SetZeroFlg();
						Time_ac_count=0;

	
//封装回调函数	
//						RtosFunTable[TIM6_125usIrqFun].Function();				//过零信号产生回调函数，由用户实现，主要用来同步系统时间片调度清零

						powerZeroChange();
	
						APP_ADC_ZERO_IrqFun();

	
						API_TIM_100US_RESET();									//更新计数器马上触发	ADC



	
}




