/*************************************************************************************************
* Copyright <2024> <Icore Technology (Nanjing) Co.,Ltd>
* All Rights Reserved,
*
* Redistribution and use in source and binary forms, with or without modification, are permitted
* provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this list of
*    conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of
*    conditions and the following disclaimer in the documentation and/or other materials provided
*    with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to
*    endorse or promote products derived from this software without specific prior written
*    permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************
@file
**************************************************************************************************
*                                              rx32g4xx
*                                          Library Function
*
*                                   Copyright 2024, RX Tech, Corp.
*                                        All Rights Reserved
*
*
*   Project : rx32g4xx
*   File    : rx32g4xx_hal_crc.c
*   By      : RX_DV_Team
**************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_common.h"
#include "rx32g4xx_hal_crc.h"

/** @defgroup CRC CRC
  * @{
  */ 
  
#ifdef HAL_CRC_MODULE_ENABLED
  
/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup CRC_Function_Definitions CRC Function Definitions
  * @{
  */

/**
  * @brief  Initialize the CRC according to the specified
  *         parameters in the CRC_InitTypeDef and create the associated handle.
  * @param  hcrc CRC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRC_Init(CRC_HandleTypeDef *hcrc)
{
    /* Check the CRC handle allocation */
    if (hcrc == NULL)
    {
        return HAL_ERROR;
    }
    
    /* Check the parameters */
    assert_param(IS_CRC_ALL_INSTANCE(hcrc->Instance));
    
    if (hcrc->State == HAL_CRC_STATE_RESET)
    {
        /* Allocate lock resource and initialize it */
        hcrc->Lock = HAL_UNLOCKED;
        /* Init the low level hardware */
        HAL_CRC_MspInit(hcrc);
    }
    
    /* Change CRC peripheral state */
    hcrc->State = HAL_CRC_STATE_READY;
    
    /* Return function status */
    return HAL_OK;
}

/**
  * @brief  DeInitialize the CRC peripheral.
  * @param  hcrc CRC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_CRC_DeInit(CRC_HandleTypeDef *hcrc)
{
    /* Check the CRC handle allocation */
    if (hcrc == NULL)
    {
        return HAL_ERROR;
    }
    
    /* Check the parameters */
    assert_param(IS_CRC_ALL_INSTANCE(hcrc->Instance));
    
    /* Check the CRC peripheral state */
    if (hcrc->State == HAL_CRC_STATE_BUSY)
    {
        return HAL_BUSY;
    }
    
    /* Change CRC peripheral state */
    hcrc->State = HAL_CRC_STATE_BUSY;
    
    /* Reset CRC calculation unit */
    __HAL_CRC_DR_RESET(hcrc);
    
    /* Reset IDR register content */
    CLEAR_REG(hcrc->Instance->IDR);
    
    /* DeInit the low level hardware */
    HAL_CRC_MspDeInit(hcrc);
    
    /* Change CRC peripheral state */
    hcrc->State = HAL_CRC_STATE_RESET;
    
    /* Process unlocked */
    __HAL_UNLOCK(hcrc);
    
    /* Return function status */
    return HAL_OK;
}

/**
  * @brief  Initializes the CRC MSP.
  * @param  hcrc CRC handle
  * @retval None
  */
__weak void HAL_CRC_MspInit(CRC_HandleTypeDef *hcrc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hcrc);
    
    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_CRC_MspInit can be implemented in the user file
    */
}

/**
  * @brief  DeInitialize the CRC MSP.
  * @param  hcrc CRC handle
  * @retval None
  */
__weak void HAL_CRC_MspDeInit(CRC_HandleTypeDef *hcrc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hcrc);
    
    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_CRC_MspDeInit can be implemented in the user file
    */
}

/**
  * @brief  Compute 32-bit CRC value of an 32-bit data buffer
  *         starting with the previously computed CRC as initialization value.
  * @param  hcrc CRC handle
  * @param  pBuffer pointer to the input data buffer, exact input data format is
  *         provided by hcrc->InputDataFormat.
  * @param  BufferLength input data buffer length
  * @retval uint32_t CRC (returned value LSBs for CRC shorter than 32 bits)
  */
uint32_t HAL_CRC_Accumulate(CRC_HandleTypeDef *hcrc, uint32_t pBuffer[], uint32_t BufferLength)
{
    uint32_t index;      /* CRC input data buffer index */
    uint32_t temp = 0U;  /* CRC output (read from hcrc->Instance->DR register) */
    
    /* Change CRC peripheral state */
    hcrc->State = HAL_CRC_STATE_BUSY;
    
    for (index = 0U; index < BufferLength; index++)
    {
      hcrc->Instance->DR = pBuffer[index];
    }
    temp = hcrc->Instance->DR;
    
    /* Change CRC peripheral state */
    hcrc->State = HAL_CRC_STATE_READY;
    
    /* Return the CRC computed value */
    return temp;
}

/**
  * @brief  Return the CRC handle state.
  * @param  hcrc CRC handle
  * @retval HAL state
  */
HAL_CRC_StateTypeDef HAL_CRC_GetState(const CRC_HandleTypeDef *hcrc)
{
    /* Return CRC handle state */
    return hcrc->State;
}
  
  
/** 
  * end of CRC_Function_Definitions @}  
  */
  
#endif /* HAL_OPAMP_MODULE_ENABLED */

/** 
  * end of CRC @}  
  */
