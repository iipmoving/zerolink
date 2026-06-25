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
*   File    : rx32g4xx_hal_dac.c
*   By      : RX_DV_Team
**************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"

/** @defgroup DAC DAC
  * @{
  */ 
  
#ifdef HAL_DAC_MODULE_ENABLED
  
/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** @defgroup DAC_Private_Constants DAC Private Constants
  * @{
  */
/* Delay for DAC channel voltage settling time from DAC channel startup       */
/* (transition from disable to enable).                                       */
#define DAC_DELAY_STARTUP_US          (15UL)      /*!< Delay for DAC channel voltage settling time from DAC channel startup (transition from disable to enable) */
#define TIMEOUT_DAC_CALIBCONFIG        1U         /*!< 1   ms        */
#define HFSEL_ENABLE_THRESHOLD_80MHZ   80000000U  /*!< 80 MHz        */
#define HFSEL_ENABLE_THRESHOLD_160MHZ  160000000U /*!< 160 MHz       */
/**
  * end of DAC_Private_Constants @}
  */
  
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup DAC_Function_Definitions Template Function Definitions
  * @{
  */
 
/**
  * @brief  Initialize the DAC peripheral according to the specified parameters
  *         in the DAC_InitStruct and initialize the associated handle.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Init(DAC_HandleTypeDef *hdac)
{
  /* Check the DAC peripheral handle */
  if (hdac == NULL)
  {
    return HAL_ERROR;
  }
  /* Check the parameters */
  assert_param(IS_DAC_ALL_INSTANCE(hdac->Instance));

  if (hdac->State == HAL_DAC_STATE_RESET)
  {
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
    /* Init the DAC Callback settings */
    hdac->ConvCpltCallbackCh1           = HAL_DAC_ConvCpltCallbackCh1;
    hdac->ConvHalfCpltCallbackCh1       = HAL_DAC_ConvHalfCpltCallbackCh1;
    hdac->ErrorCallbackCh1              = HAL_DAC_ErrorCallbackCh1;
    hdac->DMAUnderrunCallbackCh1        = HAL_DAC_DMAUnderrunCallbackCh1;

    hdac->ConvCpltCallbackCh2           = HAL_DACEx_ConvCpltCallbackCh2;
    hdac->ConvHalfCpltCallbackCh2       = HAL_DACEx_ConvHalfCpltCallbackCh2;
    hdac->ErrorCallbackCh2              = HAL_DACEx_ErrorCallbackCh2;
    hdac->DMAUnderrunCallbackCh2        = HAL_DACEx_DMAUnderrunCallbackCh2;

    if (hdac->MspInitCallback == NULL)
    {
      hdac->MspInitCallback             = HAL_DAC_MspInit;
    }
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

    /* Allocate lock resource and initialize it */
    hdac->Lock = HAL_UNLOCKED;

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
    /* Init the low level hardware */
    hdac->MspInitCallback(hdac);
#else
    /* Init the low level hardware */
    HAL_DAC_MspInit(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */
  }

  /* Initialize the DAC state*/
  hdac->State = HAL_DAC_STATE_BUSY;

  /* Set DAC error code to none */
  hdac->ErrorCode = HAL_DAC_ERROR_NONE;

  /* Initialize the DAC state*/
  hdac->State = HAL_DAC_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Deinitialize the DAC peripheral registers to their default reset values.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_DeInit(DAC_HandleTypeDef *hdac)
{
  /* Check the DAC peripheral handle */
  if (hdac == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_DAC_ALL_INSTANCE(hdac->Instance));

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
  if (hdac->MspDeInitCallback == NULL)
  {
    hdac->MspDeInitCallback = HAL_DAC_MspDeInit;
  }
  /* DeInit the low level hardware */
  hdac->MspDeInitCallback(hdac);
#else
  /* DeInit the low level hardware */
  HAL_DAC_MspDeInit(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

  /* Set DAC error code to none */
  hdac->ErrorCode = HAL_DAC_ERROR_NONE;

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_RESET;

  /* Release Lock */
  __HAL_UNLOCK(hdac);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the DAC MSP.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_MspInit(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_MspInit could be implemented in the user file
   */
}

/**
  * @brief  DeInitialize the DAC MSP.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_MspDeInit(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_MspDeInit could be implemented in the user file
   */
}

/**
  * @brief  Enables DAC and starts conversion of channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Start(DAC_HandleTypeDef *hdac, uint32_t Channel)
{
  __IO uint32_t wait_loop_index;

  /* Check the DAC peripheral handle */
  if (hdac == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));

  /* Process locked */
  __HAL_LOCK(hdac);

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;

  /* Enable the Peripheral */
  __HAL_DAC_ENABLE(hdac, Channel);
  /* Ensure minimum wait before using peripheral after enabling it */
  /* Wait loop initialization and execution */
  /* Note: Variable divided by 2 to compensate partially CPU processing cycles, scaling in us split to not exceed 32 */
  /*       bits register capacity and handle low frequency. */
  wait_loop_index = ((DAC_DELAY_STARTUP_US / 10UL) * ((SystemCoreClock / (100000UL * 2UL)) + 1UL));
  while (wait_loop_index != 0UL)
  {
    wait_loop_index--;
  }

  if (Channel == DAC_CHANNEL_1)
  {
    /* Check if software trigger enabled */
    if ((hdac->Instance->CR & (DAC_CR_TEN1 | DAC_CR_TSEL1)) == DAC_TRIGGER_SOFTWARE)
    {
      /* Enable the selected DAC software conversion */
      SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIG1);
    }
  }

  else
  {
    /* Check if software trigger enabled */
    if ((hdac->Instance->CR & (DAC_CR_TEN2 | DAC_CR_TSEL2)) == (DAC_TRIGGER_SOFTWARE << (Channel & 0x10UL)))
    {
      /* Enable the selected DAC software conversion*/
      SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIG2);
    }
  }


  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;

  /* Process unlocked */
  __HAL_UNLOCK(hdac);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Disables DAC and stop conversion of channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Stop(DAC_HandleTypeDef *hdac, uint32_t Channel)
{
  /* Check the DAC peripheral handle */
  if (hdac == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));

  /* Disable the Peripheral */
  __HAL_DAC_DISABLE(hdac, Channel);

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Enables DAC and starts conversion of channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @param  pData The source Buffer address.
  * @param  Length The length of data to be transferred from memory to DAC peripheral
  * @param  Alignment Specifies the data alignment for DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_ALIGN_8B_R: 8bit right data alignment selected
  *            @arg DAC_ALIGN_12B_L: 12bit left data alignment selected
  *            @arg DAC_ALIGN_12B_R: 12bit right data alignment selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Start_DMA(DAC_HandleTypeDef *hdac, uint32_t Channel, const uint32_t *pData, uint32_t Length,
                                    uint32_t Alignment)
{
  HAL_StatusTypeDef status;
  uint32_t tmpreg;
  __IO uint32_t wait_loop_index;

  /* Check the DAC peripheral handle */
  if (hdac == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  assert_param(IS_DAC_ALIGN(Alignment));

  /* Process locked */
  __HAL_LOCK(hdac);

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;

  if (Channel == DAC_CHANNEL_1)
  {
    /* Set the DMA transfer complete callback for channel1 */
    hdac->DMA_Handle1->XferCpltCallback = DAC_DMAConvCpltCh1;

    /* Set the DMA half transfer complete callback for channel1 */
    hdac->DMA_Handle1->XferHalfCpltCallback = DAC_DMAHalfConvCpltCh1;

    /* Set the DMA error callback for channel1 */
    hdac->DMA_Handle1->XferErrorCallback = DAC_DMAErrorCh1;

    /* Enable the selected DAC channel1 DMA request */
    SET_BIT(hdac->Instance->CR, DAC_CR_DMAEN1);

    /* Case of use of channel 1 */
    switch (Alignment)
    {
      case DAC_ALIGN_12B_R:
        /* Get DHR12R1 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR12R1;
        break;
      case DAC_ALIGN_12B_L:
        /* Get DHR12L1 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR12L1;
        break;
      case DAC_ALIGN_8B_R:
        /* Get DHR8R1 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR8R1;
        break;
      default:
        break;
    }
  }

  else
  {
    /* Set the DMA transfer complete callback for channel2 */
    hdac->DMA_Handle2->XferCpltCallback = DAC_DMAConvCpltCh2;

    /* Set the DMA half transfer complete callback for channel2 */
    hdac->DMA_Handle2->XferHalfCpltCallback = DAC_DMAHalfConvCpltCh2;

    /* Set the DMA error callback for channel2 */
    hdac->DMA_Handle2->XferErrorCallback = DAC_DMAErrorCh2;

    /* Enable the selected DAC channel2 DMA request */
    SET_BIT(hdac->Instance->CR, DAC_CR_DMAEN2);

    /* Case of use of channel 2 */
    switch (Alignment)
    {
      case DAC_ALIGN_12B_R:
        /* Get DHR12R2 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR12R2;
        break;
      case DAC_ALIGN_12B_L:
        /* Get DHR12L2 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR12L2;
        break;
      case DAC_ALIGN_8B_R:
        /* Get DHR8R2 address */
        tmpreg = (uint32_t)&hdac->Instance->DHR8R2;
        break;
      default:
        break;
    }
  }

  if (Channel == DAC_CHANNEL_1)
  {
    /* Enable the DAC DMA underrun interrupt */
    __HAL_DAC_ENABLE_IT(hdac, DAC_IT_DMAUDR1);

    /* Enable the DMA channel */
    status = HAL_DMA_Start_IT(hdac->DMA_Handle1, (uint32_t)pData, tmpreg, Length);
  }

  else
  {
    /* Enable the DAC DMA underrun interrupt */
    __HAL_DAC_ENABLE_IT(hdac, DAC_IT_DMAUDR2);

    /* Enable the DMA channel */
    status = HAL_DMA_Start_IT(hdac->DMA_Handle2, (uint32_t)pData, tmpreg, Length);
  }


  /* Process Unlocked */
  __HAL_UNLOCK(hdac);

  if (status == HAL_OK)
  {
    /* Enable the Peripheral */
    __HAL_DAC_ENABLE(hdac, Channel);
    SET_BIT(hdac->Instance->CR, DAC_CR_TEN1 << (Channel & 0x10));
    /* Ensure minimum wait before using peripheral after enabling it */
    /* Wait loop initialization and execution */
    /* Note: Variable divided by 2 to compensate partially              */
    /*       CPU processing cycles, scaling in us split to not          */
    /*       exceed 32 bits register capacity and handle low frequency. */
    wait_loop_index = ((DAC_DELAY_STARTUP_US / 10UL) * ((SystemCoreClock / (100000UL * 2UL)) + 1UL));
    while (wait_loop_index != 0UL)
    {
      wait_loop_index--;
    }
  }
  else
  {
    hdac->ErrorCode |= HAL_DAC_ERROR_DMA;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Disables DAC and stop conversion of channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_Stop_DMA(DAC_HandleTypeDef *hdac, uint32_t Channel)
{
  /* Check the DAC peripheral handle */
  if (hdac == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));

  /* Disable the selected DAC channel DMA request */
  hdac->Instance->CR &= ~(DAC_CR_DMAEN1 << (Channel & 0x10UL));

  /* Disable the Peripheral */
  __HAL_DAC_DISABLE(hdac, Channel);

  /* Disable the DMA channel */

  /* Channel1 is used */
  if (Channel == DAC_CHANNEL_1)
  {
    /* Disable the DMA channel */
    (void)HAL_DMA_Abort(hdac->DMA_Handle1);

    /* Disable the DAC DMA underrun interrupt */
    __HAL_DAC_DISABLE_IT(hdac, DAC_IT_DMAUDR1);
  }

  else /* Channel2 is used for */
  {
    /* Disable the DMA channel */
    (void)HAL_DMA_Abort(hdac->DMA_Handle2);

    /* Disable the DAC DMA underrun interrupt */
    __HAL_DAC_DISABLE_IT(hdac, DAC_IT_DMAUDR2);
  }


  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Handles DAC interrupt request
  *         This function uses the interruption of DMA
  *         underrun.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
void HAL_DAC_IRQHandler(DAC_HandleTypeDef *hdac)
{
  uint32_t itsource = hdac->Instance->CR;
  uint32_t itflag   = hdac->Instance->SR;

  if ((itsource & DAC_IT_DMAUDR1) == DAC_IT_DMAUDR1)
  {
    /* Check underrun flag of DAC channel 1 */
    if ((itflag & DAC_FLAG_DMAUDR1) == DAC_FLAG_DMAUDR1)
    {
      /* Change DAC state to error state */
      hdac->State = HAL_DAC_STATE_ERROR;

      /* Set DAC error code to channel1 DMA underrun error */
      SET_BIT(hdac->ErrorCode, HAL_DAC_ERROR_DMAUNDERRUNCH1);

      /* Clear the underrun flag */
      __HAL_DAC_CLEAR_FLAG(hdac, DAC_FLAG_DMAUDR1);

      /* Disable the selected DAC channel1 DMA request */
      __HAL_DAC_DISABLE_IT(hdac, DAC_CR_DMAEN1);

      /* Error callback */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
      hdac->DMAUnderrunCallbackCh1(hdac);
#else
      HAL_DAC_DMAUnderrunCallbackCh1(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */
    }
  }


  if ((itsource & DAC_IT_DMAUDR2) == DAC_IT_DMAUDR2)
  {
    /* Check underrun flag of DAC channel 2 */
    if ((itflag & DAC_FLAG_DMAUDR2) == DAC_FLAG_DMAUDR2)
    {
      /* Change DAC state to error state */
      hdac->State = HAL_DAC_STATE_ERROR;

      /* Set DAC error code to channel2 DMA underrun error */
      SET_BIT(hdac->ErrorCode, HAL_DAC_ERROR_DMAUNDERRUNCH2);

      /* Clear the underrun flag */
      __HAL_DAC_CLEAR_FLAG(hdac, DAC_FLAG_DMAUDR2);

      /* Disable the selected DAC channel2 DMA request */
      __HAL_DAC_DISABLE_IT(hdac, DAC_CR_DMAEN2);

      /* Error callback */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
      hdac->DMAUnderrunCallbackCh2(hdac);
#else
      HAL_DACEx_DMAUnderrunCallbackCh2(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */
    }
  }

}

/**
  * @brief  Set the specified data holding register value for DAC channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @param  Alignment Specifies the data alignment.
  *          This parameter can be one of the following values:
  *            @arg DAC_ALIGN_8B_R: 8bit right data alignment selected
  *            @arg DAC_ALIGN_12B_L: 12bit left data alignment selected
  *            @arg DAC_ALIGN_12B_R: 12bit right data alignment selected
  * @param  Data Data to be loaded in the selected data holding register.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_SetValue(DAC_HandleTypeDef *hdac, uint32_t Channel, uint32_t Alignment, uint32_t Data)
{
  __IO uint32_t tmp = 0UL;

  /* Check the DAC peripheral handle */
  if (hdac == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  assert_param(IS_DAC_ALIGN(Alignment));

  tmp = (uint32_t)hdac->Instance;
  if (Channel == DAC_CHANNEL_1)
  {
    tmp += DAC_DHR12R1_ALIGNMENT(Alignment);
  }

  else
  {
    tmp += DAC_DHR12R2_ALIGNMENT(Alignment);
  }


  /* Set the DAC channel selected data holding register */
  *(__IO uint32_t *) tmp = Data;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Conversion complete callback in non-blocking mode for Channel1
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_ConvCpltCallbackCh1 could be implemented in the user file
   */
}

/**
  * @brief  Conversion half DMA transfer callback in non-blocking mode for Channel1
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_ConvHalfCpltCallbackCh1 could be implemented in the user file
   */
}

/**
  * @brief  Error DAC callback for Channel1.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_ErrorCallbackCh1 could be implemented in the user file
   */
}

/**
  * @brief  DMA underrun DAC callback for channel1.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DAC_DMAUnderrunCallbackCh1(DAC_HandleTypeDef *hdac)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hdac);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_DAC_DMAUnderrunCallbackCh1 could be implemented in the user file
   */
}

/**
  * @brief  Returns the last data output value of the selected DAC channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval The selected DAC channel data output value.
  */
uint32_t HAL_DAC_GetValue(const DAC_HandleTypeDef *hdac, uint32_t Channel)
{
  uint32_t result;

  /* Check the DAC peripheral handle */
  assert_param(hdac != NULL);

  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));

  if (Channel == DAC_CHANNEL_1)
  {
    result = hdac->Instance->DOR1;
  }

  else
  {
    result = hdac->Instance->DOR2;
  }

  /* Returns the DAC channel data output register value */
  return result;
}

/**
  * @brief  Configures the selected DAC channel.
  * @note   By calling this function, the high frequency interface mode (HFSEL bits)
  *         will be set automatically.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  sConfig DAC configuration structure.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selecte
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DAC_ConfigChannel(DAC_HandleTypeDef *hdac,
                                        const DAC_ChannelConfTypeDef *sConfig, uint32_t Channel)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t hclkfreq;
    
    /* Check the DAC peripheral handle and channel configuration struct */
    if ((hdac == NULL) || (sConfig == NULL))
    {
        return HAL_ERROR;
    }
    
    /* Check the DAC parameters */
    assert_param(IS_DAC_TRIGGER(hdac->Instance, sConfig->DAC_Trigger));
    assert_param(IS_DAC_TRIGGER(hdac->Instance, sConfig->DAC_Trigger2));
    assert_param(IS_DAC_OUTPUT_BUFFER_STATE(sConfig->DAC_OutputBuffer));
    assert_param(IS_DAC_TRIMMING(sConfig->DAC_UserTrimming));
    
    if ((sConfig->DAC_UserTrimming) == DAC_TRIMMING_USER)
    {
        assert_param(IS_DAC_TRIMMINGVALUE(sConfig->DAC_TrimmingValue));
    }
    
    assert_param(IS_DAC_CHANNEL(Channel));
    
    /* Process locked */
    __HAL_LOCK(hdac);
    
    /* Change DAC state */
    hdac->State = HAL_DAC_STATE_BUSY;
    
    /*!< Disable the DAC channel*/
    CLEAR_BIT(hdac->Instance->CR, (DAC_CR_EN1 << (Channel & 0x10UL)));
    
    /* USER TRIMMING */
    if (sConfig->DAC_UserTrimming == DAC_TRIMMING_USER)
    {
        SET_BIT(hdac->Instance->CR, DAC_CR_CEN1 << (Channel & 0x10UL));
        MODIFY_REG(hdac->Instance->CCR, (DAC_CCR_OTRIM1 << (Channel & 0x10UL)), sConfig->DAC_TrimmingValue);      
    }
    
    /*!< DAC in normal operating mode hence clear DAC_CR_CENx bit */
    CLEAR_BIT(hdac->Instance->CR, (DAC_CR_CEN1 << (Channel & 0x10UL)));
    
    /*!< Set the high frequency setting for DAC according the HCLK frequency */
    hclkfreq = HAL_RCC_GetHCLKFreq();
    
    if (hclkfreq > HFSEL_ENABLE_THRESHOLD_160MHZ)
    {
        MODIFY_REG(hdac->Instance->MCR, DAC_MCR_HFSEL, DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_160MHZ);
    }
    else if (hclkfreq > HFSEL_ENABLE_THRESHOLD_80MHZ)
    {
        MODIFY_REG(hdac->Instance->MCR, DAC_MCR_HFSEL, DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_80MHZ);
    }
    else
    {
        MODIFY_REG(hdac->Instance->MCR, DAC_MCR_HFSEL, DAC_HIGH_FREQUENCY_INTERFACE_MODE_DISABLE);
    }
    
    /*!< Set generating wave */
    MODIFY_REG(hdac->Instance->CR, (DAC_CR_WAVE1 << (Channel & 0x10UL)), (sConfig->DAC_Wave << (Channel & 0x10UL)));
    
    /*!< Set the DAC output buffer */
    MODIFY_REG(hdac->Instance->CR, (DAC_CR_BOFF1 << (Channel & 0x10UL)), (sConfig->DAC_OutputBuffer << (Channel & 0x10UL)));
    
    /*!< Set the DAC trigger and MAMP*/
    if( sConfig->DAC_Wave == DAC_WAVE_SAWTOOTH)
    {
        MODIFY_REG(hdac->Instance->STMODR, (DAC_STMODR_STRSTTRIGSEL1 << (Channel & 0x10UL)), ((sConfig->DAC_Trigger  & DAC_CR_TSEL1) << (Channel & 0x10UL)));
        MODIFY_REG(hdac->Instance->STMODR, (DAC_STMODR_STINCTRIGSEL1 << (Channel & 0x10UL)), ((sConfig->DAC_Trigger2 & DAC_CR_TSEL1) << (Channel & 0x10UL)));
        MODIFY_REG(hdac->Instance->STMODR, (DAC_STR1_STDIR1 << (Channel & 0x10UL)), (sConfig->DAC_Dir << (Channel & 0x10UL)));
    }
    else
    {
        MODIFY_REG(hdac->Instance->CR, (DAC_CR_TSEL1 << (Channel & 0x10UL)), (sConfig->DAC_Trigger& DAC_CR_TSEL1) << (Channel & 0x10UL));
        
        if(sConfig->DAC_Wave != DAC_WAVE_NOWAVE)
        {
            /*!< Set the amplitude/ mask for triangle wave/ noise wave */
            MODIFY_REG(hdac->Instance->CR, (DAC_CR_MAMP1 << (Channel & 0x10UL)), (sConfig->DAC_MAMP << (Channel & 0x10UL)));
        }
    }
    
    /* Change DAC state */
    hdac->State = HAL_DAC_STATE_READY;
    
    /* Process unlocked */
    __HAL_UNLOCK(hdac);
    
    /* Return function status */
    return status;
}

/**
  * @brief  return the DAC handle state
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval HAL state
  */
HAL_DAC_StateTypeDef HAL_DAC_GetState(const DAC_HandleTypeDef *hdac)
{
    /* Return DAC handle state */
    return hdac->State;
}

/**
  * @brief  Return the DAC error code
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval DAC Error Code
  */
uint32_t HAL_DAC_GetError(const DAC_HandleTypeDef *hdac)
{
    return hdac->ErrorCode;
}

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User DAC Callback
  *         To be used instead of the weak (overridden) predefined callback
  * @note   The HAL_DAC_RegisterCallback() may be called before HAL_DAC_Init() in HAL_DAC_STATE_RESET to register
  *         callbacks for HAL_DAC_MSPINIT_CB_ID and HAL_DAC_MSPDEINIT_CB_ID
  * @param  hdac DAC handle
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_DAC_ERROR_INVALID_CALLBACK   DAC Error Callback ID
  *          @arg @ref HAL_DAC_CH1_COMPLETE_CB_ID       DAC CH1 Complete Callback ID
  *          @arg @ref HAL_DAC_CH1_HALF_COMPLETE_CB_ID  DAC CH1 Half Complete Callback ID
  *          @arg @ref HAL_DAC_CH1_ERROR_ID             DAC CH1 Error Callback ID
  *          @arg @ref HAL_DAC_CH1_UNDERRUN_CB_ID       DAC CH1 UnderRun Callback ID
  *          @arg @ref HAL_DAC_CH2_COMPLETE_CB_ID       DAC CH2 Complete Callback ID
  *          @arg @ref HAL_DAC_CH2_HALF_COMPLETE_CB_ID  DAC CH2 Half Complete Callback ID
  *          @arg @ref HAL_DAC_CH2_ERROR_ID             DAC CH2 Error Callback ID
  *          @arg @ref HAL_DAC_CH2_UNDERRUN_CB_ID       DAC CH2 UnderRun Callback ID
  *          @arg @ref HAL_DAC_MSPINIT_CB_ID            DAC MSP Init Callback ID
  *          @arg @ref HAL_DAC_MSPDEINIT_CB_ID          DAC MSP DeInit Callback ID
  *
  * @param  pCallback pointer to the Callback function
  * @retval status
  */
HAL_StatusTypeDef HAL_DAC_RegisterCallback(DAC_HandleTypeDef *hdac, HAL_DAC_CallbackIDTypeDef CallbackID,
                                           pDAC_CallbackTypeDef pCallback)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the DAC peripheral handle */
    if (hdac == NULL)
    {
        return HAL_ERROR;
    }
    
    if (pCallback == NULL)
    {
        /* Update the error code */
        hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
        return HAL_ERROR;
    }
    
    if (hdac->State == HAL_DAC_STATE_READY)
    {
        switch (CallbackID)
        {
            case HAL_DAC_CH1_COMPLETE_CB_ID :
                hdac->ConvCpltCallbackCh1 = pCallback;
                break;
            case HAL_DAC_CH1_HALF_COMPLETE_CB_ID :
                hdac->ConvHalfCpltCallbackCh1 = pCallback;
                break;
            case HAL_DAC_CH1_ERROR_ID :
                hdac->ErrorCallbackCh1 = pCallback;
                break;
            case HAL_DAC_CH1_UNDERRUN_CB_ID :
                hdac->DMAUnderrunCallbackCh1 = pCallback;
                break;
        
            case HAL_DAC_CH2_COMPLETE_CB_ID :
                hdac->ConvCpltCallbackCh2 = pCallback;
                break;
            case HAL_DAC_CH2_HALF_COMPLETE_CB_ID :
                hdac->ConvHalfCpltCallbackCh2 = pCallback;
                break;
            case HAL_DAC_CH2_ERROR_ID :
                hdac->ErrorCallbackCh2 = pCallback;
                break;
            case HAL_DAC_CH2_UNDERRUN_CB_ID :
                hdac->DMAUnderrunCallbackCh2 = pCallback;
                break;
        
            case HAL_DAC_MSPINIT_CB_ID :
                hdac->MspInitCallback = pCallback;
                break;
            case HAL_DAC_MSPDEINIT_CB_ID :
                hdac->MspDeInitCallback = pCallback;
                break;
            default :
                /* Update the error code */
                hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
                /* update return status */
                status =  HAL_ERROR;
                break;
        }
    }
    else if (hdac->State == HAL_DAC_STATE_RESET)
    {
        switch (CallbackID)
        {
            case HAL_DAC_MSPINIT_CB_ID :
                hdac->MspInitCallback = pCallback;
                break;
            case HAL_DAC_MSPDEINIT_CB_ID :
                hdac->MspDeInitCallback = pCallback;
                break;
            default :
                /* Update the error code */
                hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
                /* update return status */
                status =  HAL_ERROR;
                break;
        }
    }
    else
    {
        /* Update the error code */
        hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
        /* update return status */
        status =  HAL_ERROR;
    }
    
    return status;
}

/**
  * @brief  Unregister a User DAC Callback
  *         DAC Callback is redirected to the weak (overridden) predefined callback
  * @note   The HAL_DAC_UnRegisterCallback() may be called before HAL_DAC_Init() in HAL_DAC_STATE_RESET to un-register
  *         callbacks for HAL_DAC_MSPINIT_CB_ID and HAL_DAC_MSPDEINIT_CB_ID
  * @param  hdac DAC handle
  * @param  CallbackID ID of the callback to be unregistered
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_DAC_CH1_COMPLETE_CB_ID          DAC CH1 transfer Complete Callback ID
  *          @arg @ref HAL_DAC_CH1_HALF_COMPLETE_CB_ID     DAC CH1 Half Complete Callback ID
  *          @arg @ref HAL_DAC_CH1_ERROR_ID                DAC CH1 Error Callback ID
  *          @arg @ref HAL_DAC_CH1_UNDERRUN_CB_ID          DAC CH1 UnderRun Callback ID
  *          @arg @ref HAL_DAC_CH2_COMPLETE_CB_ID          DAC CH2 Complete Callback ID
  *          @arg @ref HAL_DAC_CH2_HALF_COMPLETE_CB_ID     DAC CH2 Half Complete Callback ID
  *          @arg @ref HAL_DAC_CH2_ERROR_ID                DAC CH2 Error Callback ID
  *          @arg @ref HAL_DAC_CH2_UNDERRUN_CB_ID          DAC CH2 UnderRun Callback ID
  *          @arg @ref HAL_DAC_MSPINIT_CB_ID               DAC MSP Init Callback ID
  *          @arg @ref HAL_DAC_MSPDEINIT_CB_ID             DAC MSP DeInit Callback ID
  *          @arg @ref HAL_DAC_ALL_CB_ID                   DAC All callbacks
  * @retval status
  */
HAL_StatusTypeDef HAL_DAC_UnRegisterCallback(DAC_HandleTypeDef *hdac, HAL_DAC_CallbackIDTypeDef CallbackID)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the DAC peripheral handle */
    if (hdac == NULL)
    {
        return HAL_ERROR;
    }
    
    if (hdac->State == HAL_DAC_STATE_READY)
    {
        switch (CallbackID)
        {
            case HAL_DAC_CH1_COMPLETE_CB_ID :
                hdac->ConvCpltCallbackCh1 = HAL_DAC_ConvCpltCallbackCh1;
                break;
            case HAL_DAC_CH1_HALF_COMPLETE_CB_ID :
                hdac->ConvHalfCpltCallbackCh1 = HAL_DAC_ConvHalfCpltCallbackCh1;
                break;
            case HAL_DAC_CH1_ERROR_ID :
                hdac->ErrorCallbackCh1 = HAL_DAC_ErrorCallbackCh1;
                break;
            case HAL_DAC_CH1_UNDERRUN_CB_ID :
                hdac->DMAUnderrunCallbackCh1 = HAL_DAC_DMAUnderrunCallbackCh1;
                break;
        
            case HAL_DAC_CH2_COMPLETE_CB_ID :
                hdac->ConvCpltCallbackCh2 = HAL_DACEx_ConvCpltCallbackCh2;
                break;
            case HAL_DAC_CH2_HALF_COMPLETE_CB_ID :
                hdac->ConvHalfCpltCallbackCh2 = HAL_DACEx_ConvHalfCpltCallbackCh2;
                break;
            case HAL_DAC_CH2_ERROR_ID :
                hdac->ErrorCallbackCh2 = HAL_DACEx_ErrorCallbackCh2;
                break;
            case HAL_DAC_CH2_UNDERRUN_CB_ID :
                hdac->DMAUnderrunCallbackCh2 = HAL_DACEx_DMAUnderrunCallbackCh2;
                break;
        
            case HAL_DAC_MSPINIT_CB_ID :
                hdac->MspInitCallback = HAL_DAC_MspInit;
                break;
            case HAL_DAC_MSPDEINIT_CB_ID :
                hdac->MspDeInitCallback = HAL_DAC_MspDeInit;
                break;
            case HAL_DAC_ALL_CB_ID :
                hdac->ConvCpltCallbackCh1 = HAL_DAC_ConvCpltCallbackCh1;
                hdac->ConvHalfCpltCallbackCh1 = HAL_DAC_ConvHalfCpltCallbackCh1;
                hdac->ErrorCallbackCh1 = HAL_DAC_ErrorCallbackCh1;
                hdac->DMAUnderrunCallbackCh1 = HAL_DAC_DMAUnderrunCallbackCh1;
        
                hdac->ConvCpltCallbackCh2 = HAL_DACEx_ConvCpltCallbackCh2;
                hdac->ConvHalfCpltCallbackCh2 = HAL_DACEx_ConvHalfCpltCallbackCh2;
                hdac->ErrorCallbackCh2 = HAL_DACEx_ErrorCallbackCh2;
                hdac->DMAUnderrunCallbackCh2 = HAL_DACEx_DMAUnderrunCallbackCh2;
        
                hdac->MspInitCallback = HAL_DAC_MspInit;
                hdac->MspDeInitCallback = HAL_DAC_MspDeInit;
                break;
            default :
                /* Update the error code */
                hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
                /* update return status */
                status =  HAL_ERROR;
                break;
        }
    }
    else if (hdac->State == HAL_DAC_STATE_RESET)
    {
        switch (CallbackID)
        {
            case HAL_DAC_MSPINIT_CB_ID :
                hdac->MspInitCallback = HAL_DAC_MspInit;
                break;
            case HAL_DAC_MSPDEINIT_CB_ID :
                hdac->MspDeInitCallback = HAL_DAC_MspDeInit;
                break;
            default :
                /* Update the error code */
                hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
                /* update return status */
                status =  HAL_ERROR;
                break;
        }
    }
    else
    {
        /* Update the error code */
        hdac->ErrorCode |= HAL_DAC_ERROR_INVALID_CALLBACK;
        /* update return status */
        status =  HAL_ERROR;
    }
    
    return status;
}
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

/**
  * @brief  DMA conversion complete callback.
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *         the configuration information for the specified DMA module.
  * @retval None
  */
void DAC_DMAConvCpltCh1(DMA_HandleTypeDef *hdma)
{
    DAC_HandleTypeDef *hdac = (DAC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
    hdac->ConvCpltCallbackCh1(hdac);
#else
    HAL_DAC_ConvCpltCallbackCh1(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

    hdac->State = HAL_DAC_STATE_READY;
}

/**
  * @brief  DMA half transfer complete callback.
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *         the configuration information for the specified DMA module.
  * @retval None
  */
void DAC_DMAHalfConvCpltCh1(DMA_HandleTypeDef *hdma)
{
    DAC_HandleTypeDef *hdac = (DAC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
    /* Conversion complete callback */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
    hdac->ConvHalfCpltCallbackCh1(hdac);
#else
    HAL_DAC_ConvHalfCpltCallbackCh1(hdac);
#endif  /* USE_HAL_DAC_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA error callback
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *         the configuration information for the specified DMA module.
  * @retval None
  */
void DAC_DMAErrorCh1(DMA_HandleTypeDef *hdma)
{
    DAC_HandleTypeDef *hdac = (DAC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
    
    /* Set DAC error code to DMA error */
    hdac->ErrorCode |= HAL_DAC_ERROR_DMA;
    
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
    hdac->ErrorCallbackCh1(hdac);
#else
    HAL_DAC_ErrorCallbackCh1(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

    hdac->State = HAL_DAC_STATE_READY;
}
  
/** 
  * end of DAC_Function_Definitions @}  
  */

#endif /* HAL_DAC_MODULE_ENABLED */

/** 
  * end of DAC @}  
  */
