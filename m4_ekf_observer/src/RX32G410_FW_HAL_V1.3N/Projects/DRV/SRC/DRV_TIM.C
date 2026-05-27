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
* File         : main.c
* By           : RX_DV_Team
*********************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
//#include "main.h"
//#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include	"drv_tim.h"

//#include  "TIM_DEFINE.H"

//#include "system_init.h"
//#include "system_bsp.h"

/* Private typedef -----------------------------------------------------------*/


//void	TIM_PWM_INIT(TIMstrDef* tim);					//互补PWM输出初始化
//void	TIM_OnePluse_INIT(TIMstrDef* tim);				//单脉冲输出初始化	

TIMstrDef		TIMstr;
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/
void HAL_TIM_MspPostInit(uint8_t  ch);
void Error_Handler(void);


/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */

GPIO_ClaseDef	PIN_SCL;

void		GPIOsetGroupInit(GPIOsetGroupDef* gpio)
{
		uint8_t 	count=gpio->num;			//得到IO设置数量
		
		uint32_t* gpioInit=(uint32_t*)&(gpio->gpio1);
		
		for(uint8_t i=0;i<count;i++)				//将结构中的IO全部初始化完成
		{
			HAL_GPIO_BaseInit((GPIO_BaseInitTypeDef*)*gpioInit);
			gpioInit++;
		}	

}	

void	TIM_OnePluse_INIT(TIMstrDef* tim)
{
	TIM_HandleTypeDef*		timHandle;

	uint8_t timCh=tim->timSet->timch;
	
	switch(timCh)
    {
        case    TIM1CH:
            __HAL_RCC_TIM1_CLK_ENABLE();
          
        break;		
		
        case    TIM2CH:
            __HAL_RCC_TIM2_CLK_ENABLE();
            
        break;

        case    TIM3CH:
            __HAL_RCC_TIM3_CLK_ENABLE();
           
        break;              

        case    TIM4CH:
            __HAL_RCC_TIM4_CLK_ENABLE();

        break;
		
        case    TIM5CH:
            __HAL_RCC_TIM5_CLK_ENABLE();
   
        break;
        case    TIM6CH:
            __HAL_RCC_TIM6_CLK_ENABLE();
    
        break;
        case    TIM7CH:
            __HAL_RCC_TIM7_CLK_ENABLE();
  
        break;		
        break;
    }

    timHandle=&tim->htimx;


	timHandle->Instance=tim->timSet->timx;
	timHandle->Init=*(tim->initBase);


    if (HAL_TIM_Base_Init(timHandle) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_OnePulse_Init(timHandle,TIM_OPMODE_SINGLE) != HAL_OK)
    {
        Error_Handler();
    }	
    if (HAL_TIM_SlaveConfigSynchro(timHandle, tim->slaveConfig) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_TIMEx_MasterConfigSynchronization(timHandle, tim->masterConfig) != HAL_OK)
    {
        Error_Handler();
    }	

	
	GPIOsetGroupInit(tim->gpioGroup);

//		/* Start Slave1 PWM generation */
//		if (HAL_TIM_PWM_Start(timHandle, tim->timSet->channel) != HAL_OK)
//		{
//        /* PWM generation Error */
//        Error_Handler();
//		}	
	
	if (HAL_TIM_OnePulse_Start(timHandle, tim->timSet->channel) != HAL_OK)
    {
        /* PWM generation Error */
        Error_Handler();
    }
    /* Enable the Peripheral */
	__HAL_TIM_MOE_ENABLE(timHandle);
    __HAL_TIM_ENABLE(timHandle);		
}	

void	TIM_PWM_INIT(TIMstrDef* tim)				//互补输出
{
	TIM_HandleTypeDef*		timHandle;

	uint8_t timCh=tim->timSet->timch;
	
	switch(timCh)
    {
        case    TIM1CH:
            __HAL_RCC_TIM1_CLK_ENABLE();
            
        break;

        case    TIM3CH:
            __HAL_RCC_TIM3_CLK_ENABLE();
             
        break;              

        case    TIM8CH:
            __HAL_RCC_TIM8_CLK_ENABLE();
  
        break;
		
        case    TIM15CH:
            __HAL_RCC_TIM15_CLK_ENABLE();

        break;
        case    TIM17CH:
            __HAL_RCC_TIM17_CLK_ENABLE();

        break;
        break;
    }

    timHandle=&tim->htimx;


	timHandle->Instance=tim->timSet->timx;
	timHandle->Init=*(tim->initBase);


    if (HAL_TIM_Base_Init(timHandle) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_PWM_Init(timHandle) != HAL_OK)
    {
        Error_Handler();
    }	
		#include "rx32g4xx_hal_tim.h"	
	if(tim->timSet->onePluse)
	{

		timHandle->Instance->CR1 &= ~TIM_CR1_OPM;
    
    /* Configure the OPM Mode */
		timHandle->Instance->CR1 |= TIM_CR1_OPM;
	}		
	
    if (HAL_TIM_SlaveConfigSynchro(timHandle, tim->slaveConfig) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_TIMEx_MasterConfigSynchronization(timHandle, tim->masterConfig) != HAL_OK)
    {
        Error_Handler();
    }	




	
	if (IS_TIM_BREAK_INSTANCE(timHandle->Instance) != RESET)//是否支持BREAK
	{
		if (HAL_TIM_PWM_ConfigChannel(timHandle, tim->ocInit, tim->timSet->channel) != HAL_OK)
        {
            Error_Handler();
        }

		if (HAL_TIMEx_ConfigBreakInput(timHandle, TIM_BREAKINPUT_BRK, tim->breakInput) != HAL_OK)
		{
			Error_Handler();
		}


		if (HAL_TIMEx_ConfigBreakInput(timHandle, TIM_BREAKINPUT_BRK2, tim->breakInput) != HAL_OK)
		{
			Error_Handler();
		}

	
		if (HAL_TIMEx_ConfigBreakDeadTime(timHandle, tim->breakDeadTime) != HAL_OK)
		{
        Error_Handler();
		}		

		/* Start Slave1 PWM generation */
		if (HAL_TIM_PWM_Start(timHandle, tim->timSet->channel) != HAL_OK)
		{
        /* PWM generation Error */
        Error_Handler();
		}
		/* Start Slave1 PWMn generation */
		if (HAL_TIMEx_PWMN_Start(timHandle, tim->timSet->channel) != HAL_OK)
		{
        /* PWM generation Error */
        Error_Handler();
		}		
		
		__HAL_TIM_MOE_DISABLE_UNCONDITIONALLY(timHandle);			//MOE不需要判断

		
	}
	GPIOsetGroupInit(tim->gpioGroup);	
	HAL_TIM_MspPostInit(tim->timSet->timch);
		
		

		
}	

void Error_Handler(void)
{
    printf("not need\n");
}


void HAL_TIM_MspPostInit(uint8_t  ch)
{	
	
		switch(ch)
		{
			case	TIM1CH:
				__HAL_RCC_TIM1_CLK_ENABLE();
			break;
			case	TIM2CH:
				__HAL_RCC_TIM2_CLK_ENABLE();
			break;
			case	TIM3CH:
				__HAL_RCC_TIM3_CLK_ENABLE();
			break;
			case	TIM4CH:
				__HAL_RCC_TIM4_CLK_ENABLE();
			break;
			case	TIM5CH:
				__HAL_RCC_TIM5_CLK_ENABLE();
			break;
			case	TIM6CH:
				__HAL_RCC_TIM6_CLK_ENABLE();
			break;
			case	TIM7CH:
				__HAL_RCC_TIM7_CLK_ENABLE();
			break;
			case	TIM8CH:
				__HAL_RCC_TIM8_CLK_ENABLE();
			break;
			case	TIM15CH:
				__HAL_RCC_TIM15_CLK_ENABLE();
			break;
			case	TIM16CH:
				__HAL_RCC_TIM16_CLK_ENABLE();
			break;
			case	TIM17CH:
				__HAL_RCC_TIM17_CLK_ENABLE();
			break;
	
			
		}		

	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOD_CLK_ENABLE();
	  __HAL_RCC_GPIOE_CLK_ENABLE();
	  __HAL_RCC_GPIOF_CLK_ENABLE();
		

}



