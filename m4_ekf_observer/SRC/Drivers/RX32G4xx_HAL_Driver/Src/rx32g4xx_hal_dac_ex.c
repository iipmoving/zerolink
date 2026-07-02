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
*   File    : rx32g4xx_hal_Template.c
*   By      : RX_DV_Team
**************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"

/** @addtogroup DAC
  * @{
  */ 
  
#ifdef HAL_DAC_MODULE_ENABLED
  
/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** @addtogroup  DAC_Private_Constants
  * @{
  */
#define DAC_DELAY_TRIM_US          (50UL)     /*!< Delay for DAC minimum trimming time */
#define DAC_DELAY_STARTUP_US       (15UL)      /*!< Delay for DAC channel voltage settling time from DAC channel startup (transition from disable to enable) */

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
  * @brief  Enables DAC and starts conversion of both channels.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_DualStart(DAC_HandleTypeDef *hdac)
{
    uint32_t tmp_swtrig = 0UL;
    __IO uint32_t wait_loop_index;
    
    /* Check the DAC peripheral handle */
    if (hdac == NULL)
    {
        return HAL_ERROR;
    }
    
    /* Process locked */
    __HAL_LOCK(hdac);
    
    /* Change DAC state */
    hdac->State = HAL_DAC_STATE_BUSY;
    
    /* Enable the Peripheral */
    __HAL_DAC_ENABLE(hdac, DAC_CHANNEL_1);
    __HAL_DAC_ENABLE(hdac, DAC_CHANNEL_2);
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
    
    /* Check if software trigger enabled */
    if ((hdac->Instance->CR & (DAC_CR_TEN1 | DAC_CR_TSEL1)) == DAC_TRIGGER_SOFTWARE)
    {
        tmp_swtrig |= DAC_SWTRIGR_SWTRIG1;
    }
    if ((hdac->Instance->CR & (DAC_CR_TEN2 | DAC_CR_TSEL2)) == (DAC_TRIGGER_SOFTWARE << (DAC_CHANNEL_2 & 0x10UL)))
    {
        tmp_swtrig |= DAC_SWTRIGR_SWTRIG2;
    }
    /* Enable the selected DAC software conversion*/
    SET_BIT(hdac->Instance->SWTRIGR, tmp_swtrig);
    
    /* Change DAC state */
    hdac->State = HAL_DAC_STATE_READY;
    
    /* Process unlocked */
    __HAL_UNLOCK(hdac);
    
    /* Return function status */
    return HAL_OK;
}

/**
  * @brief  Disables DAC and stop conversion of both channels.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_DualStop(DAC_HandleTypeDef *hdac)
{
    /* Check the DAC peripheral handle */
    if (hdac == NULL)
    {
        return HAL_ERROR;
    }
    
    /* Disable the Peripheral */
    __HAL_DAC_DISABLE(hdac, DAC_CHANNEL_1);
    __HAL_DAC_DISABLE(hdac, DAC_CHANNEL_2);
    
    /* Change DAC state */
    hdac->State = HAL_DAC_STATE_READY;
    
    /* Return function status */
    return HAL_OK;
}

/**
  * @brief  Enables DAC and starts conversion of both channel 1 and 2 of the same DAC.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The DAC channel that will request data from DMA.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @param  pData The destination peripheral Buffer address.
  * @param  Length The length of data to be transferred from memory to DAC peripheral
  * @param  Alignment Specifies the data alignment for DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_ALIGN_8B_R: 8bit right data alignment selected
  *            @arg DAC_ALIGN_12B_L: 12bit left data alignment selected
  *            @arg DAC_ALIGN_12B_R: 12bit right data alignment selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_DualStart_DMA(DAC_HandleTypeDef *hdac, uint32_t Channel,
                                          const uint32_t *pData, uint32_t Length, uint32_t Alignment)
{
    HAL_StatusTypeDef status;
    uint32_t tmpreg = 0UL;
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
    }
    
    switch (Alignment)
    {
        case DAC_ALIGN_12B_R:
            /* Get DHR12R1 address */
            tmpreg = (uint32_t)&hdac->Instance->DHR12RD;
            break;
        case DAC_ALIGN_12B_L:
            /* Get DHR12L1 address */
            tmpreg = (uint32_t)&hdac->Instance->DHR12LD;
            break;
        case DAC_ALIGN_8B_R:
            /* Get DHR8R1 address */
            tmpreg = (uint32_t)&hdac->Instance->DHR8RD;
            break;
        default:
            break;
    }
    
    /* Enable the DMA channel */
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
        __HAL_DAC_ENABLE(hdac, DAC_CHANNEL_1);
        __HAL_DAC_ENABLE(hdac, DAC_CHANNEL_2);
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
  * @brief  Disables DAC and stop conversion both channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The DAC channel that requests data from DMA.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_DualStop_DMA(DAC_HandleTypeDef *hdac, uint32_t Channel)
{
    HAL_StatusTypeDef status;
    
    /* Check the DAC peripheral handle */
    if (hdac == NULL)
    {
        return HAL_ERROR;
    }
    
    /* Ensure Channel 2 exists for this particular DAC instance */
    assert_param(IS_DAC_CHANNEL(Channel));
    
    /* Disable the selected DAC channel DMA request */
    CLEAR_BIT(hdac->Instance->CR, DAC_CR_DMAEN2 | DAC_CR_DMAEN1);
    
    /* Disable the Peripheral */
    __HAL_DAC_DISABLE(hdac, DAC_CHANNEL_1);
    __HAL_DAC_DISABLE(hdac, DAC_CHANNEL_2);
    
    /* Disable the DMA channel */
    
    /* Channel1 is used */
    if (Channel == DAC_CHANNEL_1)
    {
        /* Disable the DMA channel */
        status = HAL_DMA_Abort(hdac->DMA_Handle1);
    
        /* Disable the DAC DMA underrun interrupt */
        __HAL_DAC_DISABLE_IT(hdac, DAC_IT_DMAUDR1);
    }
    else
    {
        /* Disable the DMA channel */
        status = HAL_DMA_Abort(hdac->DMA_Handle2);
    
        /* Disable the DAC DMA underrun interrupt */
        __HAL_DAC_DISABLE_IT(hdac, DAC_IT_DMAUDR2);
    }
    
    /* Check if DMA Channel effectively disabled */
    if (status != HAL_OK)
    {
        /* Update DAC state machine to error */
        hdac->State = HAL_DAC_STATE_ERROR;
    }
    else
    {
        /* Change DAC state */
        hdac->State = HAL_DAC_STATE_READY;
    }
    
    /* Return function status */
    return status;
}


/**
  * @brief  Enable or disable the selected DAC channel wave generation.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_WaveGenerate(DAC_HandleTypeDef *hdac, uint32_t Channel)
{
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
    
    /* Enable the triangle wave generation for the selected DAC channel */
    SET_BIT(hdac->Instance->CR, (DAC_CR_TEN1 << (Channel & 0x10UL)));
    
    /* Change DAC state */
    hdac->State = HAL_DAC_STATE_READY;
    
    /* Process unlocked */
    __HAL_UNLOCK(hdac);
    
    /* Return function status */
    return HAL_OK;
}

/**
  * @brief  Enable or disable the selected DAC channel sawtooth wave generation.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected (1)
  *
  *         (1) On this STM32 series, parameter not available on all instances.
  *             Refer to device datasheet for channels availability.
  * @param  Polarity polarity to be used for wave generation.
  *          This parameter can be one of the following values:
  *            @arg DAC_SAWTOOTH_POLARITY_DECREMENT
  *            @arg DAC_SAWTOOTH_POLARITY_INCREMENT
  * @param  ResetData Sawtooth wave reset value.
  *          Range is from 0 to DAC full range 4095 (0xFFF)
  * @param  StepData Sawtooth wave step value.
  *          12.4 bit format, unsigned: 12 bits exponent / 4 bits mantissa
  *          Step value step is 1/16 = 0.0625
  *          Step value range is 0.0000 to 4095.9375 (0xFFF.F)
  * @note    Sawtooth reset and step triggers are configured by calling @ref HAL_DAC_ConfigChannel
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_SawtoothWaveGenerate(DAC_HandleTypeDef *hdac, uint32_t Channel, uint32_t Polarity,
                                                 uint32_t ResetData, uint32_t StepData)
{
  /* Check the parameters */
  assert_param(IS_DAC_CHANNEL(Channel));
  assert_param(IS_DAC_SAWTOOTH_POLARITY(Polarity));
  assert_param(IS_DAC_RESET_DATA(ResetData));
  assert_param(IS_DAC_STEP_DATA(StepData));

  /* Process locked */
  __HAL_LOCK(hdac);

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_BUSY;

  if (Channel == DAC_CHANNEL_1)
  {
    /* Configure the sawtooth wave generation data parameters */
    MODIFY_REG(hdac->Instance->STR1,
               DAC_STR1_STINCDATA1 | DAC_STR1_STDIR1 | DAC_STR1_STRSTDATA1,
               (StepData << DAC_STR1_STINCDATA1_Pos)
               | Polarity
               | (ResetData << DAC_STR1_STRSTDATA1_Pos));
  }
  else
  {
    /* Configure the sawtooth wave generation data parameters */
    MODIFY_REG(hdac->Instance->STR2,
               DAC_STR2_STINCDATA2 | DAC_STR2_STDIR2 | DAC_STR2_STRSTDATA2,
               (StepData << DAC_STR2_STINCDATA2_Pos)
               | Polarity
               | (ResetData << DAC_STR2_STRSTDATA2_Pos));
  }

  /* Enable the sawtooth wave generation for the selected DAC channel */
  MODIFY_REG(hdac->Instance->CR, (DAC_CR_WAVE1) << (Channel & 0x10UL), (uint32_t)(DAC_CR_WAVE1_BIT1 | DAC_CR_WAVE1_BIT0) << (Channel & 0x10UL));

  /* Change DAC state */
  hdac->State = HAL_DAC_STATE_READY;

  /* Process unlocked */
  __HAL_UNLOCK(hdac);

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Trig sawtooth wave reset
  * @note   This function allows to reset sawtooth wave in case of SW trigger
  *         has been configured for this usage.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_SawtoothWaveDataReset(DAC_HandleTypeDef *hdac, uint32_t Channel)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the DAC peripheral handle */
    if (hdac == NULL)
    {
        return HAL_ERROR;
    }
    
    /* Check the parameters */
    assert_param(IS_DAC_CHANNEL(Channel));
    
    /* Process locked */
    __HAL_LOCK(hdac);
    
    if (((hdac->Instance->STMODR >> (Channel & 0x10UL)) & DAC_STMODR_STRSTTRIGSEL1) == 0UL /* SW TRIGGER */)
    {
        /* Change DAC state */
        hdac->State = HAL_DAC_STATE_BUSY;
    
        if (Channel == DAC_CHANNEL_1)
        {
            /* Enable the selected DAC software conversion */
            SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIG1);
        }
        else
        {
            /* Enable the selected DAC software conversion */
            SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIG2);
        }
    
        /* Change DAC state */
        hdac->State = HAL_DAC_STATE_READY;
    }
    else
    {
        status = HAL_ERROR;
    }
    
    /* Process unlocked */
    __HAL_UNLOCK(hdac);
    
    /* Return function status */
    return status;
}

/**
  * @brief  Trig sawtooth wave step
  * @note   This function allows to generate step  in sawtooth wave in case of
  *         SW trigger has been configured for this usage.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_SawtoothWaveDataStep(DAC_HandleTypeDef *hdac, uint32_t Channel)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the DAC peripheral handle */
    if (hdac == NULL)
    {
        return HAL_ERROR;
    }
    
    /* Check the parameters */
    assert_param(IS_DAC_CHANNEL(Channel));
    
    /* Process locked */
    __HAL_LOCK(hdac);
    
    if (((hdac->Instance->STMODR >> (Channel & 0x10UL)) & DAC_STMODR_STINCTRIGSEL1) == 0UL /* SW TRIGGER */)
    {
        /* Change DAC state */
        hdac->State = HAL_DAC_STATE_BUSY;
    
        if (Channel == DAC_CHANNEL_1)
        {
            /* Enable the selected DAC software conversion */
            SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIGB1);
        }
        else
        {
            /* Enable the selected DAC software conversion */
            SET_BIT(hdac->Instance->SWTRIGR, DAC_SWTRIGR_SWTRIGB2);
        }
    
        /* Change DAC state */
        hdac->State = HAL_DAC_STATE_READY;
    }
    else
    {
        status = HAL_ERROR;
    }
    
    /* Process unlocked */
    __HAL_UNLOCK(hdac);
    
    /* Return function status */
    return status;
}


/**
  * @brief  Set the specified data holding register value for dual DAC channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *               the configuration information for the specified DAC.
  * @param  Alignment Specifies the data alignment for dual channel DAC.
  *          This parameter can be one of the following values:
  *            DAC_ALIGN_8B_R: 8bit right data alignment selected
  *            DAC_ALIGN_12B_L: 12bit left data alignment selected
  *            DAC_ALIGN_12B_R: 12bit right data alignment selected
  * @param  Data1 Data for DAC Channel1 to be loaded in the selected data holding register.
  * @param  Data2 Data for DAC Channel2 to be loaded in the selected data  holding register.
  * @note   In dual mode, a unique register access is required to write in both
  *          DAC channels at the same time.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_DualSetValue(DAC_HandleTypeDef *hdac, uint32_t Alignment, uint32_t Data1, uint32_t Data2)
{
    uint32_t data;
    uint32_t tmp;
    
    /* Check the DAC peripheral handle */
    if (hdac == NULL)
    {
        return HAL_ERROR;
    }
    
    /* Check the parameters */
    assert_param(IS_DAC_ALIGN(Alignment));
    assert_param(IS_DAC_DATA(Data1));
    assert_param(IS_DAC_DATA(Data2));
    
    /* Calculate and set dual DAC data holding register value */
    if (Alignment == DAC_ALIGN_8B_R)
    {
        data = ((uint32_t)Data2 << 8U) | Data1;
    }
    else
    {
        data = ((uint32_t)Data2 << 16U) | Data1;
    }
    
    tmp = (uint32_t)hdac->Instance;
    tmp += DAC_DHR12RD_ALIGNMENT(Alignment);
    
    /* Set the dual DAC selected data holding register */
    *(__IO uint32_t *)tmp = data;
    
    /* Return function status */
    return HAL_OK;
}

/**
  * @brief  Conversion complete callback in non-blocking mode for Channel2.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef *hdac)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hdac);

    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_DACEx_ConvCpltCallbackCh2 could be implemented in the user file
    */
}

/**
  * @brief  Conversion half DMA transfer callback in non-blocking mode for Channel2.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DACEx_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef *hdac)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hdac);
    
    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_DACEx_ConvHalfCpltCallbackCh2 could be implemented in the user file
    */
}

/**
  * @brief  Error DAC callback for Channel2.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DACEx_ErrorCallbackCh2(DAC_HandleTypeDef *hdac)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hdac);
    
    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_DACEx_ErrorCallbackCh2 could be implemented in the user file
    */
}

/**
  * @brief  DMA underrun DAC callback for Channel2.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval None
  */
__weak void HAL_DACEx_DMAUnderrunCallbackCh2(DAC_HandleTypeDef *hdac)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hdac);
    
    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_DACEx_DMAUnderrunCallbackCh2 could be implemented in the user file
    */
}


/**
  * @brief  Run the self calibration of one DAC channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  sConfig DAC channel configuration structure.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval Updates DAC_TrimmingValue. , DAC_UserTrimming set to DAC_UserTrimming
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_SelfCalibrate(DAC_HandleTypeDef *hdac, DAC_ChannelConfTypeDef *sConfig, uint32_t Channel)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    uint32_t trimmingvalue;
    uint32_t delta;
    __IO uint32_t wait_loop_index;
    
    /* Check the parameters */
    assert_param(IS_DAC_CHANNEL(Channel));
    
    /* Check the DAC handle allocation */
    /* Check if DAC running */
    if ((hdac == NULL) || (sConfig == NULL))
    {
        status = HAL_ERROR;
    }
    else if (hdac->State == HAL_DAC_STATE_BUSY)
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Process locked */
        __HAL_LOCK(hdac);
    
        /* Disable the selected DAC channel */
        CLEAR_BIT((hdac->Instance->CR), (DAC_CR_EN1 << (Channel & 0x10UL)));
        /* Wait for ready bit to be de-asserted */
        HAL_Delay(1);
    
        /* Enable the selected DAC channel calibration */
        /* i.e. set DAC_CR_CENx bit */
        SET_BIT((hdac->Instance->CR), (DAC_CR_CEN1 << (Channel & 0x10UL)));
    
        /* Init trimming counter */
        /* Medium value */
        trimmingvalue = 16UL;
        delta = 8UL;
        while (delta != 0UL)
        {
            /* Set candidate trimming */
            MODIFY_REG(hdac->Instance->CCR, (DAC_CCR_OTRIM1 << (Channel & 0x10UL)), (trimmingvalue << (Channel & 0x10UL)));
        
            /* Wait minimum time needed between two calibration steps (OTRIM) */
            /* Wait loop initialization and execution */
            /* Note: Variable divided by 2 to compensate partially CPU processing cycles, scaling in us split to not exceed */
            /*       32 bits register capacity and handle low frequency. */
            wait_loop_index = ((DAC_DELAY_TRIM_US / 10UL) * ((SystemCoreClock / (100000UL * 2UL)) + 1UL));
            while (wait_loop_index != 0UL)
            {
                wait_loop_index--;
            }
        
            if ((hdac->Instance->SR & (DAC_SR_CAL1 << (Channel & 0x10UL))) == (DAC_SR_CAL1 << (Channel & 0x10UL)))
            {
                /* DAC_SR_CAL_FLAGx is HIGH try higher trimming */
                trimmingvalue -= delta;
            }
            else
            {
                /* DAC_SR_CAL_FLAGx is LOW try lower trimming */
                trimmingvalue += delta;
            }
            delta >>= 1UL;
        }
    
        /* Still need to check if right calibration is current value or one step below */
        /* Indeed the first value that causes the DAC_SR_CAL_FLAGx bit to change from 0 to 1  */
        /* Set candidate trimming */
        MODIFY_REG(hdac->Instance->CCR, (DAC_CCR_OTRIM1 << (Channel & 0x10UL)), (trimmingvalue << (Channel & 0x10UL)));
    
        /* Wait minimum time needed between two calibration steps (OTRIM) */
        /* Wait loop initialization and execution */
        /* Note: Variable divided by 2 to compensate partially CPU processing cycles, scaling in us split to not exceed */
        /*       32 bits register capacity and handle low frequency. */
        wait_loop_index = ((DAC_DELAY_TRIM_US / 10UL) * ((SystemCoreClock / (100000UL * 2UL)) + 1UL));
        while (wait_loop_index != 0UL)
        {
            wait_loop_index--;
        }
    
        if ((hdac->Instance->SR & (DAC_SR_CAL1 << (Channel & 0x10UL))) == 0UL)
        {
            /* Trimming is actually one value more */
            trimmingvalue++;
            /* Set right trimming */
            MODIFY_REG(hdac->Instance->CCR, (DAC_CCR_OTRIM1 << (Channel & 0x10UL)), (trimmingvalue << (Channel & 0x10UL)));
        }
    
        /* Disable the selected DAC channel calibration */
        /* i.e. clear DAC_CR_CENx bit */
        CLEAR_BIT((hdac->Instance->CR), (DAC_CR_CEN1 << (Channel & 0x10UL)));
    
        sConfig->DAC_TrimmingValue = trimmingvalue;
        sConfig->DAC_UserTrimming = DAC_TRIMMING_USER;
    
        /* Process unlocked */
        __HAL_UNLOCK(hdac);
    }
    
    return status;
}

/**
  * @brief  Set the trimming mode and trimming value (user trimming mode applied).
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @param  sConfig DAC configuration structure updated with new DAC trimming value.
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @param  NewTrimmingValue DAC new trimming value
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DACEx_SetUserTrimming(DAC_HandleTypeDef *hdac, DAC_ChannelConfTypeDef *sConfig, uint32_t Channel,
                                            uint32_t NewTrimmingValue)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    /* Check the parameters */
    assert_param(IS_DAC_CHANNEL(Channel));
    assert_param(IS_DAC_NEWTRIMMINGVALUE(NewTrimmingValue));
    
    /* Check the DAC handle and channel configuration struct allocation */
    if ((hdac == NULL) || (sConfig == NULL))
    {
        status = HAL_ERROR;
    }
    else
    {
        /* Process locked */
        __HAL_LOCK(hdac);
    
        /* Set new trimming */
        MODIFY_REG(hdac->Instance->CCR, (DAC_CCR_OTRIM1 << (Channel & 0x10UL)), (NewTrimmingValue << (Channel & 0x10UL)));
    
        /* Update trimming mode */
        sConfig->DAC_UserTrimming = DAC_TRIMMING_USER;
        sConfig->DAC_TrimmingValue = NewTrimmingValue;
    
        /* Process unlocked */
        __HAL_UNLOCK(hdac);
    }
    return status;
}

/**
  * @brief  Return the DAC trimming value.
  * @param  hdac DAC handle
  * @param  Channel The selected DAC channel.
  *          This parameter can be one of the following values:
  *            @arg DAC_CHANNEL_1: DAC Channel1 selected
  *            @arg DAC_CHANNEL_2: DAC Channel2 selected
  * @retval Trimming value : range: 0->31
  *
 */
uint32_t HAL_DACEx_GetTrimOffset(const DAC_HandleTypeDef *hdac, uint32_t Channel)
{
    /* Check the parameter */
    assert_param(IS_DAC_CHANNEL(Channel));
    
    /* Retrieve trimming */
    return ((hdac->Instance->CCR & (DAC_CCR_OTRIM1 << (Channel & 0x10UL))) >> (Channel & 0x10UL));
}

/**
  * @brief  Return the last data output value of the selected DAC channel.
  * @param  hdac pointer to a DAC_HandleTypeDef structure that contains
  *         the configuration information for the specified DAC.
  * @retval The selected DAC channel data output value.
  */
uint32_t HAL_DACEx_DualGetValue(const DAC_HandleTypeDef *hdac)
{
    uint32_t tmp = 0UL;
    
    tmp |= hdac->Instance->DOR1;
    
    tmp |= hdac->Instance->DOR2 << 16UL;
    
    /* Returns the DAC channel data output register value */
    return tmp;
}

/**
  * @brief  DMA conversion complete callback.
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
void DAC_DMAConvCpltCh2(DMA_HandleTypeDef *hdma)
{
    DAC_HandleTypeDef *hdac = (DAC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
    hdac->ConvCpltCallbackCh2(hdac);
#else
    HAL_DACEx_ConvCpltCallbackCh2(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

    hdac->State = HAL_DAC_STATE_READY;
}

/**
  * @brief  DMA half transfer complete callback.
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
void DAC_DMAHalfConvCpltCh2(DMA_HandleTypeDef *hdma)
{
    DAC_HandleTypeDef *hdac = (DAC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
    /* Conversion complete callback */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
    hdac->ConvHalfCpltCallbackCh2(hdac);
#else
    HAL_DACEx_ConvHalfCpltCallbackCh2(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA error callback.
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
void DAC_DMAErrorCh2(DMA_HandleTypeDef *hdma)
{
    DAC_HandleTypeDef *hdac = (DAC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

    /* Set DAC error code to DMA error */
    hdac->ErrorCode |= HAL_DAC_ERROR_DMA;

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
    hdac->ErrorCallbackCh2(hdac);
#else
    HAL_DACEx_ErrorCallbackCh2(hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

    hdac->State = HAL_DAC_STATE_READY;
}

/** 
  * end of DAC_Function_Definitions @}  
  */
  
#endif /* HAL_OPAMP_MODULE_ENABLED */

/** 
  * end of DAC @}  
  */
