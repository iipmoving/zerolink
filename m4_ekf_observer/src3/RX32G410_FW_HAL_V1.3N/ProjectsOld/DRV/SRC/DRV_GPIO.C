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
//#include "rx32g4xx_hal.h"
//#include	"drv_tim.h"

//#include  "TIM_DEFINE.H"

//#include "system_init.h"
//#include "system_bsp.h"

#include	"rx32g4xx_hal_gpio.h"
#include	"DRV_GPIO.h"
/* Private typedef -----------------------------------------------------------*/




/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/



/**
  * @brief  Initialize the GPIOx peripheral according to the specified parameters in the GPIO_Init.连带IO口号一起运算
  * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral for RX32G4xx family
  * @param  GPIO_Init: pointer to a GPIO_InitTypeDef structure that contains
  *         the configuration information for the specified GPIO peripheral.
            @arg @ref GPIO_InitTypeDef
  * @retval None
  */

void DRV_GPIO_BaseInit(GPIO_BaseInitTypeDef* gpio)
{
	HAL_GPIO_Init(gpio->base,&(gpio->initStr));
}	

void	HAL_GPIO_ClassInit(GPIO_ClaseDef*	ioClass,	GPIO_BaseInitTypeDef* ioset)
{
	
	ioClass->gpio=*(ioset);
	HAL_GPIO_Init(ioClass->gpio.base,&(ioClass->gpio.initStr));
	ioClass->ReadPin=DRV_GPIO_BaseReadPin;				//方法地址初始化
	ioClass->WritePin=DRV_GPIO_BaseWritePin;
}	


GPIO_PinState     DRV_GPIO_BaseReadPin(GPIO_BaseInitTypeDef *gpio)
{
	return HAL_GPIO_ReadPin(gpio->base,(gpio->initStr).Pin);
}	


void  DRV_GPIO_BaseWritePin(GPIO_BaseInitTypeDef *gpio, GPIO_PinState PinState)
{
			HAL_GPIO_WritePin(gpio->base,(gpio->initStr).Pin,PinState);
}

void    DRV_GPIO_BaseTogglePin(GPIO_BaseInitTypeDef *gpio)
{
		HAL_GPIO_TogglePin(gpio->base,(gpio->initStr).Pin);
}	

void DRV_GPIO_PinPull(GPIO_BaseInitTypeDef *gpio, uint32_t Pull)
{

		MODIFY_REG(gpio->base->PUPDR, (GPIO_PUPDR << (POSITION_VAL((gpio->initStr).Pin) * 2U)), (Pull << (POSITION_VAL((gpio->initStr).Pin) * 2U)));
//		MODIFY_REG(gpio->base->PUPDR, (GPIO_PUPDR << (POSITION_VAL((gpio->initStr).Pin)  * 2u)), (GPIO_PUPDR_Pulldown << (POSITION_VAL((gpio->initStr).Pin) * 2u)));

}


void DRV_GPIO_Mode(GPIO_BaseInitTypeDef *gpio, uint32_t inOrOut)
{
		uint32_t temp;
		uint32_t position=(POSITION_VAL((gpio->initStr).Pin)  * 2u);
				
		temp = gpio->base->MODER;
    temp &= ~(GPIO_MODER << position) ;
    temp |= (inOrOut << position);
    gpio->base->MODER = temp;

}

#if 0
#include	"rx32g4xx_hal_dma.h"
#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"


HAL_StatusTypeDef DRV_HAL_DMA_SetConfig_IT(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress,
                                   uint32_t DataLength)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the parameters */
  assert_param(IS_DMA_BUFFER_SIZE(DataLength));

  /* Process locked */
  __HAL_LOCK(hdma);

  if (HAL_DMA_STATE_READY == hdma->State)
  {
    /* Change DMA peripheral state */
    hdma->State = HAL_DMA_STATE_BUSY;
    hdma->ErrorCode = HAL_DMA_ERROR_NONE;

    /* Disable the peripheral */
    __HAL_DMA_DISABLE(hdma);

    /* Configure the source, destination address and the data length & clear flags*/
//    DMA_SetConfig(hdma, SrcAddress, DstAddress, DataLength);

  hdma->DMAmuxChannelStatus->CFR = hdma->DMAmuxChannelStatusMask;

  if (hdma->DMAmuxRequestGen != 0U)
  {
    /* Clear the DMAMUX request generator overrun flag */
    hdma->DMAmuxRequestGenStatus->RGCFR = hdma->DMAmuxRequestGenStatusMask;
  }

  /* Clear all flags */
  hdma->DmaBaseAddress->IFCR = (DMA_ISR_GIF1 << (hdma->ChannelIndex & 0x1FU));

  /* Configure DMA Channel data length */
  hdma->Instance->CNDTR = DataLength;

  /* Memory to Peripheral */
  if ((hdma->Init.Direction) == DMA_MEMORY_TO_PERIPH)
  {
    /* Configure DMA Channel destination address */
    hdma->Instance->CPAR = DstAddress;

    /* Configure DMA Channel source address */
    hdma->Instance->CMAR = SrcAddress;
  }
  /* Peripheral to Memory */
  else
  {
    /* Configure DMA Channel source address */
    hdma->Instance->CPAR = SrcAddress;

    /* Configure DMA Channel destination address */
    hdma->Instance->CMAR = DstAddress;
  }		
	//DMA_SetConfig	end
		
    /* Enable the transfer complete interrupt */
    /* Enable the transfer Error interrupt */
    if (NULL != hdma->XferHalfCpltCallback)
    {
      /* Enable the Half transfer complete interrupt as well */
      __HAL_DMA_ENABLE_IT(hdma, (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE));
    }
    else
    {
      __HAL_DMA_DISABLE_IT(hdma, DMA_IT_HT);
      __HAL_DMA_ENABLE_IT(hdma, (DMA_IT_TC | DMA_IT_TE));
    }

    /* Check if DMAMUX Synchronization is enabled*/
    if ((hdma->DMAmuxChannel->CCR & DMAMUX_CxCR_SE) != 0U)
    {
      /* Enable DMAMUX sync overrun IT*/
      hdma->DMAmuxChannel->CCR |= DMAMUX_CxCR_SOIE;
    }

    if (hdma->DMAmuxRequestGen != 0U)
    {
      /* if using DMAMUX request generator, enable the DMAMUX request generator overrun IT*/
      /* enable the request gen overrun IT*/
      hdma->DMAmuxRequestGen->RGCR |= DMAMUX_RGxCR_OIE;
    }

    /* Enable the Peripheral */
//    __HAL_DMA_ENABLE(hdma);
  }
  else
  {
    /* Process Unlocked */
    __HAL_UNLOCK(hdma);

    /* Remain BUSY */
    status = HAL_BUSY;
  }
  return status;
}
#endif



/*end gpio base*/
