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
3. Neither the name of the copyright
holder nor the names of its contributors may be used to
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
**********************************************************************************************
*                                              rx32g4xx
*                                           Library Function
*
*                                   Copyright 2024, RX Tech, Corp.
*                                        All Rights Reserved
*
*
* Project      : rx32g4xx
* File         : rx32g4xx_it.c
* By           : RX_DV_Team
**********************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_it.h"
#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include	"API_gpio.h"
#include	"api_hrtim.h"
#include	"main.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/




__weak	void API_HRTIM1_TEST1_IRQHandler(void)		//中断处理虚函数 在HRTIM.c 处理
{
}
__weak	void API_TIM6_IRQHandler(void)		//中断处理虚函数 在HRTIM.c 处理
{
}
__weak	void API_I2C1_EV_IRQHandler(void)
{

}	
__weak	void API_I2C1_ER_IRQHandler(void)
{


}

__weak	void API_ADC1_2_IRQHandler(void)
{

}	
__weak	void API_ADC3_IRQHandler(void)
{


}	
__weak	void	API_DMA1_Channel4_IRQHandler(void)//m2m
{

}
__weak	void	API_DMA2_Channel3_IRQHandler(void)//m2mlow
{

}
__weak	void	API_DMA1_Current_IRQHandler(void)
{

}
__weak	void	API_DMA2_PAN_IRQHandler(void)
{

}
__weak	void	API_DMA2_Channel7_IRQHandler(void)    // TXA  缓存中断 单通道
{

}



__weak	void	API_HRTIM1_TEST2_IRQHandler(void)
{}	

__weak	void API_ADC_PENDV_IRQHandler(void)
{


}
__weak	void	API_HRTIM1_PAN_IRQHandler(uint8_t ch )
{}	
	
__weak	void	API_TIM5_IRQHandler(void)	
{}	
__weak	void	API_TIM7_IRQHandler(void)	
{}	

__weak	void API_UART_IRQHandler(void)
{}	
__weak	void	API_DMA_FmacPreload_IRQHandler(void)
{}
__weak	void	API_DMA_FmacWrite_IRQHandler(void)
{}
__weak	void	API_DMA_FmacRead_IRQHandler(void)
{}

__weak	void	API_FMAC_IRQHandler(void)
{}		
__weak	void	API_UART_RxIdle_IRQHandler(void)
{}	
__weak	void	API_UART_TxOver_IRQHandler(void)
{}	
/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
				API_GPIO_WritePin(DebugB_pin,1);	
//				API_GPIO_WritePin(DebugB_pin,0);	
    }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
						API_GPIO_WritePin(DebugA_pin,1);
    }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
	API_ADC_PENDV_IRQHandler();
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/******************************************************************************/
/*                  Peripherals Interrupt Handlers                            */
/*  Add the Interrupt Handler here for the used peripheral(s), for the        */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_rx32g4xx.s).                                               */
/******************************************************************************/
#include	"API_gpio.h"

void ADC1_2_IRQHandler(void)
{
	
	API_ADC1_2_IRQHandler();	

  
  
}

void ADC3_IRQHandler(void)
{
	
	API_ADC3_IRQHandler();

}


void TIM5_IRQHandler(void)
{

	API_TIM5_IRQHandler();
	
}	
void TIM7_DAC_IRQHandler(void)
{

	API_TIM7_IRQHandler();
	
}	


void TIM6_DAC_IRQHandler(void)
{

	API_TIM6_IRQHandler();
	
}	



//void	HRTIM1_TEST_IRQHandler(void)
//#define		HRTIM1_TEST1_IRQHandler					HRTIM1_TIMC_IRQHandler
/*	Pot1_TimerIndex=HRTIM_TIMERINDEX_TIMER_B,
	Pot2_TimerIndex=HRTIM_TIMERINDEX_TIMER_E,
	Pot3_TimerIndex=HRTIM_TIMERINDEX_TIMER_A,
	Pot4_TimerIndex=HRTIM_TIMERINDEX_TIMER_D,*/
//-----------------------------------------
void	HRTIM1_TIMA_IRQHandler(void)
{
	API_HRTIM1_PAN_IRQHandler(PotCh3);
}
void	HRTIM1_TIMB_IRQHandler(void)
{
	API_HRTIM1_PAN_IRQHandler(PotCh1);
}
void	HRTIM1_TIMD_IRQHandler(void)
{
	API_HRTIM1_PAN_IRQHandler(PotCh4);
}
void	HRTIM1_TIME_IRQHandler(void)
{
	API_HRTIM1_PAN_IRQHandler(PotCh2);
}
//----------------------------------------

void	HRTIM1_TIMC_IRQHandler(void)
{
	API_HRTIM1_TEST1_IRQHandler();
}

//#define		HRTIM1_TEST2_IRQHandler					HRTIM1_TIMF_IRQHandler
void	HRTIM1_TIMF_IRQHandler(void)
{
	API_HRTIM1_TEST2_IRQHandler();
//  }
}

void HRTIM1_Master_IRQHandler(void)
{
	API_HRTIM1_Master_IRQHandler();
}


#ifdef	I2C1_PORT
/**
  * @brief  This function handles I2C event interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to I2C data transmission
  */

void I2C1_EV_IRQHandler(void)
{
	

    API_I2C1_EV_IRQHandler();


}

/**
  * @brief  This function handles I2C error interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to I2C error
  */
void I2C1_ER_IRQHandler(void)
{
    API_I2C1_ER_IRQHandler();
}

#else
/**
  * @brief  This function handles I2C event interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to I2C data transmission
  */

void I2C2_EV_IRQHandler(void)
{
	

    API_I2C1_EV_IRQHandler();


}

/**
  * @brief  This function handles I2C error interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to I2C error
  */
void I2C2_ER_IRQHandler(void)
{
    API_I2C1_ER_IRQHandler();
}


#endif

#ifdef	COMM_UART
void UART3_IRQHandler(void)
{
	API_UART_RxIdle_IRQHandler();
}	
#else
void UART2_IRQHandler(void)
{
	API_UART_RxIdle_IRQHandler();
}
#endif
void DMA1_Channel4_IRQHandler(void)
{
	API_DMA1_Channel4_IRQHandler();
	
}
void DMA2_Channel3_IRQHandler(void)
{
	API_DMA2_Channel3_IRQHandler();
	
}

void DMA1_Channel7_IRQHandler(void)    // T12A  缓存中断 双通道
{
	
	API_DMA1_Current_IRQHandler();
	
}
void DMA2_Channel2_IRQHandler(void)   //检锅数据缓存
{
	API_DMA2_PAN_IRQHandler();

}


void DMA2_Channel7_IRQHandler(void)     // TXA  缓存中断 单通道
{
	API_DMA2_Channel7_IRQHandler();
}

void DMA2_Channel4_IRQHandler(void)     // TXA  缓存中断 单通道
{
	API_UART_TxOver_IRQHandler();
}



/**
  * @brief This function handles DMA1 channel2 global interrupt.
  */
void DMA1_Channel5_IRQHandler(void)
{
//    HAL_DMA_IRQHandler(&hdma_fmac_write);
		API_DMA_FmacWrite_IRQHandler();
	
}
void DMA1_Channel6_IRQHandler(void)
{
//    HAL_DMA_IRQHandler(&hdma_fmac_preload);
		API_DMA_FmacPreload_IRQHandler();
	
}
void DMA2_Channel6_IRQHandler(void)
{
//    HAL_DMA_IRQHandler(&hdma_fmac_write);

		API_DMA_FmacRead_IRQHandler();

}


void FMAC_IRQHandler(void)
{
		API_FMAC_IRQHandler();
//	
//    HAL_FMAC_IRQHandler(&hfmac);
}


