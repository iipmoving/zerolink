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
*   File    : rx32g4xx_hal_adc.c
*   By      : RX_DV_Team
**************************************************************************************************
*/



/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/** @defgroup ADC ADC
  * @{
  */ 
  
#ifdef HAL_ADC_MODULE_ENABLED

/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** @defgroup ADC_Private_Constants ADC Private Constants
  * @{
  */
#define ADC_ENABLE_TIMEOUT              2U
#define ADC_DISABLE_TIMEOUT             2U
#define ADC_STAB_DELAY_US               1U
#define ADC_TEMPSENSOR_DELAY_US         10U
/** 
  * end of ADC_Private_Constants @}  
  */
  
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup ADC_Function_Definitions
  * @{
  */

/**
  * @brief  Initializes the ADC peripheral and regular group according to  
  *         parameters specified in structure "ADC_InitTypeDef".
  * @note   Possibility to update parameters on the fly:
  *         This function initializes the ADC MSP (HAL_ADC_MspInit()) only when
  *         coming from ADC state reset. Following calls to this function can
  *         be used to reconfigure some parameters of ADC_InitTypeDef  
  *         structure on the fly, without modifying MSP configuration. If ADC  
  *         MSP has to be modified again, HAL_ADC_DeInit() must be called
  *         before HAL_ADC_Init().
  *         The setting of these parameters is conditioned to ADC state.
  *         For parameters constraints, see comments of structure 
  *         "ADC_InitTypeDef".
  * @note   This function configures the ADC within 2 scopes: scope of entire 
  *         ADC and scope of regular group. For parameters details, see comments 
  *         of structure "ADC_InitTypeDef".
  * @param  hadc: ADC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_Init(ADC_HandleTypeDef* hadc)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  uint32_t tmp_cr1 = 0U;
  uint32_t tmp_cr2 = 0U;
  uint32_t tmp_sqr1 = 0U;
  
  /* Check ADC handle */
  if(hadc == NULL)
  {
    return HAL_ERROR;
  }
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  assert_param(IS_ADC_DATA_ALIGN(hadc->Init.DataAlign));
  assert_param(IS_ADC_SCAN_MODE(hadc->Init.ScanConvMode));
  assert_param(IS_FUNCTIONAL_STATE(hadc->Init.ContinuousConvMode));
  assert_param(IS_ADC_EXTTRIG(hadc->Init.ExternalTrigConv));
  
  if(hadc->Init.ScanConvMode != ADC_SCAN_DISABLE)
  {
    assert_param(IS_ADC_REGULAR_NB_CONV(hadc->Init.NbrOfConversion));
    assert_param(IS_FUNCTIONAL_STATE(hadc->Init.DiscontinuousConvMode));
    if(hadc->Init.DiscontinuousConvMode != DISABLE)
    {
      assert_param(IS_ADC_REGULAR_DISCONT_NUMBER(hadc->Init.NbrOfDiscConversion));
    }
  }
  
  /* As prerequisite, into HAL_ADC_MspInit(), ADC clock must be configured    */
  /* at RCC top level.                                                        */
  /* Refer to header of this file for more details on clock enabling          */
  /* procedure.                                                               */

  /* Actions performed only if ADC is coming from state reset:                */
  /* - Initialization of ADC MSP                                              */
  if (hadc->State == HAL_ADC_STATE_RESET)
  {
    /* Initialize ADC error code */
    ADC_CLEAR_ERRORCODE(hadc);
    
    /* Allocate lock resource and initialize it */
    hadc->Lock = HAL_UNLOCKED;
    
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
    /* Init the ADC Callback settings */
    hadc->ConvCpltCallback              = HAL_ADC_ConvCpltCallback;                 /* Legacy weak callback */
    hadc->ConvHalfCpltCallback          = HAL_ADC_ConvHalfCpltCallback;             /* Legacy weak callback */
    hadc->LevelOutOfWindowCallback      = HAL_ADC_LevelOutOfWindowCallback;         /* Legacy weak callback */
    hadc->ErrorCallback                 = HAL_ADC_ErrorCallback;                    /* Legacy weak callback */
    hadc->InjectedConvCpltCallback      = HAL_ADCEx_InjectedConvCpltCallback;       /* Legacy weak callback */
    
    if (hadc->MspInitCallback == NULL)
    {
      hadc->MspInitCallback = HAL_ADC_MspInit; /* Legacy weak MspInit  */
    }
    
    /* Init the low level hardware */
    hadc->MspInitCallback(hadc);
#else
    /* Init the low level hardware */
    HAL_ADC_MspInit(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
  }
  
  /* Stop potential conversion on going, on regular and injected groups */
  /* Disable ADC peripheral */
  /* Note: In case of ADC already enabled, precaution to not launch an        */
  /*       unwanted conversion while modifying register CR2 by writing 1 to   */
  /*       bit ADON.                                                          */
  tmp_hal_status = ADC_ConversionStop_Disable(hadc);
  
  
  /* Configuration of ADC parameters if previous preliminary actions are      */ 
  /* correctly completed.                                                     */
  if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL) &&
      (tmp_hal_status == HAL_OK)                                  )
  {
    /* Set ADC state */
    ADC_STATE_CLR_SET(hadc->State,
                      HAL_ADC_STATE_REG_BUSY | HAL_ADC_STATE_INJ_BUSY,
                      HAL_ADC_STATE_BUSY_INTERNAL);
    
    /* Set ADC parameters */
    
    /* Configuration of ADC:                                                  */
    /*  - data alignment                                                      */
    /*  - external trigger to start conversion                                */
    /*  - external trigger polarity (always set to 1, because needed for all  */
    /*    triggers: external trigger of SW start)                             */
    /*  - continuous conversion mode                                          */
    /* Note: External trigger polarity (ADC_CR2_EXTTRIG) is set into          */
    /*       HAL_ADC_Start_xxx functions because if set in this function,     */
    /*       a conversion on injected group would start a conversion also on  */
    /*       regular group after ADC enabling.                                */
    tmp_cr2 |= (hadc->Init.DataAlign                                          |
                ADC_CFGR_EXTSEL(hadc, hadc->Init.ExternalTrigConv)            |
                ADC_CR2_CONTINUOUS((uint32_t)hadc->Init.ContinuousConvMode)   );

    /* Configuration of ADC:                                                  */
    /*  - scan mode                                                           */
    /*  - discontinuous mode disable/enable                                   */
    /*  - discontinuous mode number of conversions                            */
    tmp_cr1 |= (ADC_CR1_SCAN_SET(hadc->Init.ScanConvMode));
    
    /* Enable discontinuous mode only if continuous mode is disabled */
    /* Note: If parameter "Init.ScanConvMode" is set to disable, parameter    */
    /*       discontinuous is set anyway, but will have no effect on ADC HW.  */
    if (hadc->Init.DiscontinuousConvMode == ENABLE)
    {
      if (hadc->Init.ContinuousConvMode == DISABLE)
      {
        /* Enable the selected ADC regular discontinuous mode */
        /* Set the number of channels to be converted in discontinuous mode */
        SET_BIT(tmp_cr1, ADC_CR1_DISCEN                                            |
                         ADC_CR1_DISCONTINUOUS_NUM(hadc->Init.NbrOfDiscConversion)  );
      }
      else
      {
        /* ADC regular group settings continuous and sequencer discontinuous*/
        /* cannot be enabled simultaneously.                                */
        
        /* Update ADC state machine to error */
        SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_CONFIG);
        
        /* Set ADC error code to ADC IP internal error */
        SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_INTERNAL);
      }
    }
    
    /* Update ADC configuration register CR1 with previous settings */
      MODIFY_REG(hadc->Instance->CR1,
                 ADC_CR1_SCAN    |
                 ADC_CR1_DISCEN  |
                 ADC_CR1_DISCNUM    ,
                 tmp_cr1             );
    
    /* Update ADC configuration register CR2 with previous settings */
      MODIFY_REG(hadc->Instance->CR2,
                 ADC_CR2_ALIGN   |
                 ADC_CR2_EXTSEL  |
                 ADC_CR2_EXTTRIG |
                 ADC_CR2_CONT       ,
                 tmp_cr2             );

    /* Configuration of regular group sequencer:                              */
    /* - if scan mode is disabled, regular channels sequence length is set to */
    /*   0x00: 1 channel converted (channel on regular rank 1)                */
    /*   Parameter "NbrOfConversion" is discarded.                            */
    if (ADC_CR1_SCAN_SET(hadc->Init.ScanConvMode) == ADC_SCAN_ENABLE)
    {
      tmp_sqr1 = ADC_SQR1_L_SHIFT(hadc->Init.NbrOfConversion);
    }
      
    MODIFY_REG(hadc->Instance->SQR1,
               ADC_SQR1_L          ,
               tmp_sqr1             );
    
    /* Check back that ADC registers have effectively been configured to      */
    /* ensure of no potential problem of ADC core IP clocking.                */
    /* Check through register CR2 (excluding bits set in other functions:     */
    /* execution control bits (ADON, JSWSTART, SWSTART), regular group bits   */
    /* (DMA), injected group bits (JEXTTRIG and JEXTSEL), channel internal    */
    /* measurement path bit (TSVREFE).                                        */
    if (READ_BIT(hadc->Instance->CR2, ~(ADC_CR2_ADON | ADC_CR2_DMA |
                                        ADC_CR2_SWSTART | ADC_CR2_JSWSTART |
                                        ADC_CR2_JEXTTRIG | ADC_CR2_JEXTSEL |
                                        ADC_CR2_TSVREFE                     ))
         == tmp_cr2)
    {
      /* Set ADC error code to none */
      ADC_CLEAR_ERRORCODE(hadc);
      
      /* Set the ADC state */
      ADC_STATE_CLR_SET(hadc->State,
                        HAL_ADC_STATE_BUSY_INTERNAL,
                        HAL_ADC_STATE_READY);
    }
    else
    {
      /* Update ADC state machine to error */
      ADC_STATE_CLR_SET(hadc->State,
                        HAL_ADC_STATE_BUSY_INTERNAL,
                        HAL_ADC_STATE_ERROR_INTERNAL);
      
      /* Set ADC error code to ADC IP internal error */
      SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_INTERNAL);
      
      tmp_hal_status = HAL_ERROR;
    }
  
  }
  else
  {
    /* Update ADC state machine to error */
    SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL);
        
    tmp_hal_status = HAL_ERROR;
  }
  
  /* Return function status */
  return tmp_hal_status;
}

/**
  * @brief  Deinitialize the ADC peripheral registers to their default reset
  *         values, with deinitialization of the ADC MSP.
  *         If needed, the example code can be copied and uncommented into
  *         function HAL_ADC_MspDeInit().
  * @param  hadc: ADC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_DeInit(ADC_HandleTypeDef* hadc)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  
  /* Check ADC handle */
  if(hadc == NULL)
  {
     return HAL_ERROR;
  }
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  
  /* Set ADC state */
  SET_BIT(hadc->State, HAL_ADC_STATE_BUSY_INTERNAL);
  
  /* Stop potential conversion on going, on regular and injected groups */
  /* Disable ADC peripheral */
  tmp_hal_status = ADC_ConversionStop_Disable(hadc);
  
  
  /* Configuration of ADC parameters if previous preliminary actions are      */ 
  /* correctly completed.                                                     */
  if (tmp_hal_status == HAL_OK)
  {
    /* ========== Reset ADC registers ========== */

    /* Reset register SR */
    __HAL_ADC_CLEAR_FLAG(hadc, (ADC_FLAG_AWD | ADC_FLAG_JEOC | ADC_FLAG_EOC |
                                ADC_FLAG_JSTRT | ADC_FLAG_STRT));
                         
    /* Reset register CR1 */
    CLEAR_BIT(hadc->Instance->CR1, (ADC_CR1_AWDFILT | ADC_CR1_AWDEN  | ADC_CR1_JAWDEN  | ADC_CR1_DISCNUM | 
                                    ADC_CR1_JDISCEN | ADC_CR1_DISCEN | ADC_CR1_JAUTO   | 
                                    ADC_CR1_AWDSGL  | ADC_CR1_SCAN   | ADC_CR1_JEOCIE  |   
                                    ADC_CR1_AWDIE   | ADC_CR1_EOCIE  | ADC_CR1_AWDCH    ));
    
    /* Reset register CR2 */
    CLEAR_BIT(hadc->Instance->CR2, (ADC_CR2_TSVREFE | ADC_CR2_SWSTART | ADC_CR2_JSWSTART | 
                                    ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL  | ADC_CR2_JEXTTRIG |  
                                    ADC_CR2_JEXTSEL | ADC_CR2_ALIGN   | ADC_CR2_DMA      |        
                                    ADC_CR2_RSTCAL  | ADC_CR2_CAL     | ADC_CR2_CONT     |          
                                    ADC_CR2_ADON                                          ));
    
    /* Reset register SMPR1 */
    CLEAR_BIT(hadc->Instance->SMPR1, (ADC_SMPR1_SMP15 | 
                                      ADC_SMPR1_SMP14 | ADC_SMPR1_SMP13 | ADC_SMPR1_SMP12 | 
                                      ADC_SMPR1_SMP11 | ADC_SMPR1_SMP10                    ));
    
    /* Reset register SMPR2 */
    CLEAR_BIT(hadc->Instance->SMPR2, (ADC_SMPR2_SMP9 | ADC_SMPR2_SMP8 | ADC_SMPR2_SMP7 | 
                                      ADC_SMPR2_SMP6 | ADC_SMPR2_SMP5 | ADC_SMPR2_SMP4 | 
                                      ADC_SMPR2_SMP3 | ADC_SMPR2_SMP2 | ADC_SMPR2_SMP1 | 
                                      ADC_SMPR2_SMP0                                    ));

    /* Reset register JOFR1 */
    CLEAR_BIT(hadc->Instance->JOFR1, ADC_JOFR1_JOFFSET1);
    /* Reset register JOFR2 */
    CLEAR_BIT(hadc->Instance->JOFR2, ADC_JOFR2_JOFFSET2);
    /* Reset register JOFR3 */
    CLEAR_BIT(hadc->Instance->JOFR3, ADC_JOFR3_JOFFSET3);
    /* Reset register JOFR4 */
    CLEAR_BIT(hadc->Instance->JOFR4, ADC_JOFR4_JOFFSET4);
    
    /* Reset register HTR */
    CLEAR_BIT(hadc->Instance->HTR, ADC_HTR_HT);
    /* Reset register LTR */
    CLEAR_BIT(hadc->Instance->LTR, ADC_LTR_LT);
    
    /* Reset register SQR1 */
    CLEAR_BIT(hadc->Instance->SQR1, ADC_SQR1_L  );
    
    /* Reset register SQR1 */
    CLEAR_BIT(hadc->Instance->SQR1, ADC_SQR1_L  );
    
    /* Reset register SQR2 */
    CLEAR_BIT(hadc->Instance->SQR2, ADC_SQR2_SQ8  | ADC_SQR2_SQ7   );
    
    /* Reset register SQR3 */
    CLEAR_BIT(hadc->Instance->SQR3, ADC_SQR3_SQ6 | ADC_SQR3_SQ5 | ADC_SQR3_SQ4 | 
                                    ADC_SQR3_SQ3 | ADC_SQR3_SQ2 | ADC_SQR3_SQ1  );
    
    /* Reset register JSQR */
    CLEAR_BIT(hadc->Instance->JSQR, ADC_JSQR_JL |
                                    ADC_JSQR_JSQ4 | ADC_JSQR_JSQ3 | 
                                    ADC_JSQR_JSQ2 | ADC_JSQR_JSQ1  );
    
    /* Reset register DR */
    /* bits in access mode read only, no direct reset applicable*/
    
    /* Reset registers JDR1, JDR2, JDR3, JDR4 */
    /* bits in access mode read only, no direct reset applicable*/
    
    /* ========== Hard reset ADC peripheral ========== */
    /* Performs a global reset of the entire ADC peripheral: ADC state is     */
    /* forced to a similar state after device power-on.                       */
    /* If needed, copy-paste and uncomment the following reset code into      */
    /* function "void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)":              */
    /*                                                                        */
    /*  __HAL_RCC_ADC1_FORCE_RESET()                                          */
    /*  __HAL_RCC_ADC1_RELEASE_RESET()                                        */
    
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
    if (hadc->MspDeInitCallback == NULL)
    {
      hadc->MspDeInitCallback = HAL_ADC_MspDeInit; /* Legacy weak MspDeInit  */
    }
    
    /* DeInit the low level hardware */
    hadc->MspDeInitCallback(hadc);
#else
    /* DeInit the low level hardware */
    HAL_ADC_MspDeInit(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
    
    /* Set ADC error code to none */
    ADC_CLEAR_ERRORCODE(hadc);
    
    /* Set ADC state */
    hadc->State = HAL_ADC_STATE_RESET; 
  
  }
  
  /* Process unlocked */
  __HAL_UNLOCK(hadc);
  
  /* Return function status */
  return tmp_hal_status;
}

/**
  * @brief  Initializes the ADC MSP.
  * @param  hadc: ADC handle
  * @retval None
  */
__weak void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_ADC_MspInit must be implemented in the user file.
   */ 
}

/**
  * @brief  DeInitializes the ADC MSP.
  * @param  hadc: ADC handle
  * @retval None
  */
__weak void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_ADC_MspDeInit must be implemented in the user file.
   */ 
}

#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User ADC Callback
  *         To be used instead of the weak predefined callback
  * @param  hadc Pointer to a ADC_HandleTypeDef structure that contains
  *                the configuration information for the specified ADC.
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_ADC_CONVERSION_COMPLETE_CB_ID      ADC conversion complete callback ID
  *          @arg @ref HAL_ADC_CONVERSION_HALF_CB_ID          ADC conversion complete callback ID
  *          @arg @ref HAL_ADC_LEVEL_OUT_OF_WINDOW_1_CB_ID    ADC analog watchdog 1 callback ID
  *          @arg @ref HAL_ADC_ERROR_CB_ID                    ADC error callback ID
  *          @arg @ref HAL_ADC_INJ_CONVERSION_COMPLETE_CB_ID  ADC group injected conversion complete callback ID
  *          @arg @ref HAL_ADC_MSPINIT_CB_ID                  ADC Msp Init callback ID
  *          @arg @ref HAL_ADC_MSPDEINIT_CB_ID                ADC Msp DeInit callback ID
  *          @arg @ref HAL_ADC_MSPINIT_CB_ID MspInit callback ID
  *          @arg @ref HAL_ADC_MSPDEINIT_CB_ID MspDeInit callback ID
  * @param  pCallback pointer to the Callback function
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_RegisterCallback(ADC_HandleTypeDef *hadc, HAL_ADC_CallbackIDTypeDef CallbackID, pADC_CallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  if (pCallback == NULL)
  {
    /* Update the error code */
    hadc->ErrorCode |= HAL_ADC_ERROR_INVALID_CALLBACK;

    return HAL_ERROR;
  }
  
  if ((hadc->State & HAL_ADC_STATE_READY) != 0)
  {
    switch (CallbackID)
    {
      case HAL_ADC_CONVERSION_COMPLETE_CB_ID :
        hadc->ConvCpltCallback = pCallback;
        break;
      
      case HAL_ADC_CONVERSION_HALF_CB_ID :
        hadc->ConvHalfCpltCallback = pCallback;
        break;
      
      case HAL_ADC_LEVEL_OUT_OF_WINDOW_1_CB_ID :
        hadc->LevelOutOfWindowCallback = pCallback;
        break;
      
      case HAL_ADC_ERROR_CB_ID :
        hadc->ErrorCallback = pCallback;
        break;
      
      case HAL_ADC_INJ_CONVERSION_COMPLETE_CB_ID :
        hadc->InjectedConvCpltCallback = pCallback;
        break;
      
      case HAL_ADC_MSPINIT_CB_ID :
        hadc->MspInitCallback = pCallback;
        break;
      
      case HAL_ADC_MSPDEINIT_CB_ID :
        hadc->MspDeInitCallback = pCallback;
        break;
      
      default :
        /* Update the error code */
        hadc->ErrorCode |= HAL_ADC_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status = HAL_ERROR;
        break;
    }
  }
  else if (HAL_ADC_STATE_RESET == hadc->State)
  {
    switch (CallbackID)
    {
      case HAL_ADC_MSPINIT_CB_ID :
        hadc->MspInitCallback = pCallback;
        break;
      
      case HAL_ADC_MSPDEINIT_CB_ID :
        hadc->MspDeInitCallback = pCallback;
        break;
      
      default :
        /* Update the error code */
        hadc->ErrorCode |= HAL_ADC_ERROR_INVALID_CALLBACK;
      
        /* Return error status */
        status = HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hadc->ErrorCode |= HAL_ADC_ERROR_INVALID_CALLBACK;
    
    /* Return error status */
    status =  HAL_ERROR;
  }
  
  return status;
}

/**
  * @brief  Unregister a ADC Callback
  *         ADC callback is redirected to the weak predefined callback
  * @param  hadc Pointer to a ADC_HandleTypeDef structure that contains
  *                the configuration information for the specified ADC.
  * @param  CallbackID ID of the callback to be unregistered
  *         This parameter can be one of the following values:
  *          @arg @ref HAL_ADC_CONVERSION_COMPLETE_CB_ID      ADC conversion complete callback ID
  *          @arg @ref HAL_ADC_CONVERSION_HALF_CB_ID          ADC conversion complete callback ID
  *          @arg @ref HAL_ADC_LEVEL_OUT_OF_WINDOW_1_CB_ID    ADC analog watchdog 1 callback ID
  *          @arg @ref HAL_ADC_ERROR_CB_ID                    ADC error callback ID
  *          @arg @ref HAL_ADC_INJ_CONVERSION_COMPLETE_CB_ID  ADC group injected conversion complete callback ID
  *          @arg @ref HAL_ADC_MSPINIT_CB_ID                  ADC Msp Init callback ID
  *          @arg @ref HAL_ADC_MSPDEINIT_CB_ID                ADC Msp DeInit callback ID
  *          @arg @ref HAL_ADC_MSPINIT_CB_ID MspInit callback ID
  *          @arg @ref HAL_ADC_MSPDEINIT_CB_ID MspDeInit callback ID
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_UnRegisterCallback(ADC_HandleTypeDef *hadc, HAL_ADC_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  if ((hadc->State & HAL_ADC_STATE_READY) != 0)
  {
    switch (CallbackID)
    {
      case HAL_ADC_CONVERSION_COMPLETE_CB_ID :
        hadc->ConvCpltCallback = HAL_ADC_ConvCpltCallback;
        break;
      
      case HAL_ADC_CONVERSION_HALF_CB_ID :
        hadc->ConvHalfCpltCallback = HAL_ADC_ConvHalfCpltCallback;
        break;
      
      case HAL_ADC_LEVEL_OUT_OF_WINDOW_1_CB_ID :
        hadc->LevelOutOfWindowCallback = HAL_ADC_LevelOutOfWindowCallback;
        break;
      
      case HAL_ADC_ERROR_CB_ID :
        hadc->ErrorCallback = HAL_ADC_ErrorCallback;
        break;
      
      case HAL_ADC_INJ_CONVERSION_COMPLETE_CB_ID :
        hadc->InjectedConvCpltCallback = HAL_ADCEx_InjectedConvCpltCallback;
        break;
      
      case HAL_ADC_MSPINIT_CB_ID :
        hadc->MspInitCallback = HAL_ADC_MspInit; /* Legacy weak MspInit              */
        break;
      
      case HAL_ADC_MSPDEINIT_CB_ID :
        hadc->MspDeInitCallback = HAL_ADC_MspDeInit; /* Legacy weak MspDeInit            */
        break;
      
      default :
        /* Update the error code */
        hadc->ErrorCode |= HAL_ADC_ERROR_INVALID_CALLBACK;
        
        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else if (HAL_ADC_STATE_RESET == hadc->State)
  {
    switch (CallbackID)
    {
      case HAL_ADC_MSPINIT_CB_ID :
        hadc->MspInitCallback = HAL_ADC_MspInit;                   /* Legacy weak MspInit              */
        break;
        
      case HAL_ADC_MSPDEINIT_CB_ID :
        hadc->MspDeInitCallback = HAL_ADC_MspDeInit;               /* Legacy weak MspDeInit            */
        break;
        
      default :
        /* Update the error code */
        hadc->ErrorCode |= HAL_ADC_ERROR_INVALID_CALLBACK;
        
        /* Return error status */
        status =  HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hadc->ErrorCode |= HAL_ADC_ERROR_INVALID_CALLBACK;
    
    /* Return error status */
    status =  HAL_ERROR;
  }
  
  return status;
}

#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */

/**
  * @brief  Enables ADC, starts conversion of regular group.
  *         Interruptions enabled in this function: None.
  * @param  hadc: ADC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_Start(ADC_HandleTypeDef* hadc)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  
  /* Process locked */
  __HAL_LOCK(hadc);
   
  /* Enable the ADC peripheral */
  tmp_hal_status = ADC_Enable(hadc);
  
  /* Start conversion if ADC is effectively enabled */
  if (tmp_hal_status == HAL_OK)
  {
    /* Set ADC state                                                          */
    /* - Clear state bitfield related to regular group conversion results     */
    /* - Set state bitfield related to regular operation                      */
    ADC_STATE_CLR_SET(hadc->State,
                      HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC,
                      HAL_ADC_STATE_REG_BUSY);
    
    /* Set group injected state (from auto-injection) and multimode state     */
    /* for all cases of multimode: independent mode, multimode ADC master     */
    /* or multimode ADC slave (for devices with several ADCs):                */
    if (ADC_NONMULTIMODE_OR_MULTIMODEMASTER(hadc))
    {
      /* Set ADC state (ADC independent or master) */
      CLEAR_BIT(hadc->State, HAL_ADC_STATE_MULTIMODE_SLAVE);
      
      /* If conversions on group regular are also triggering group injected,  */
      /* update ADC state.                                                    */
      if (READ_BIT(hadc->Instance->CR1, ADC_CR1_JAUTO) != RESET)
      {
        ADC_STATE_CLR_SET(hadc->State, HAL_ADC_STATE_INJ_EOC, HAL_ADC_STATE_INJ_BUSY);  
      }
    }
    else
    {
      /* Set ADC state (ADC slave) */
      SET_BIT(hadc->State, HAL_ADC_STATE_MULTIMODE_SLAVE);
      
      /* If conversions on group regular are also triggering group injected,  */
      /* update ADC state.                                                    */
      if (ADC_MULTIMODE_AUTO_INJECTED(hadc))
      {
        ADC_STATE_CLR_SET(hadc->State, HAL_ADC_STATE_INJ_EOC, HAL_ADC_STATE_INJ_BUSY);
      }
    }
    
    /* State machine update: Check if an injected conversion is ongoing */
    if (HAL_IS_BIT_SET(hadc->State, HAL_ADC_STATE_INJ_BUSY))
    {
      /* Reset ADC error code fields related to conversions on group regular */
      CLEAR_BIT(hadc->ErrorCode, (HAL_ADC_ERROR_OVR | HAL_ADC_ERROR_DMA));         
    }
    else
    {
      /* Reset ADC all error code fields */
      ADC_CLEAR_ERRORCODE(hadc);
    }
    
    /* Process unlocked */
    /* Unlock before starting ADC conversions: in case of potential           */
    /* interruption, to let the process to ADC IRQ Handler.                   */
    __HAL_UNLOCK(hadc);
  
    /* Clear regular group conversion flag */
    /* (To ensure of no unknown state from potential previous ADC operations) */
    __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_EOC);
    
    /* Enable conversion of regular group.                                    */
    /* If software start has been selected, conversion starts immediately.    */
    /* If external trigger has been selected, conversion will start at next   */
    /* trigger event.                                                         */
    /* Case of multimode enabled:                                             */ 
    /*  - if ADC is slave, ADC is enabled only (conversion is not started).   */
    /*  - if ADC is master, ADC is enabled and conversion is started.         */
    /* If ADC is master, ADC is enabled and conversion is started.            */
    /* Note: Alternate trigger for single conversion could be to force an     */
    /*       additional set of bit ADON "hadc->Instance->CR2 |= ADC_CR2_ADON;"*/
    if (ADC_IS_SOFTWARE_START_REGULAR(hadc)      &&
        ADC_NONMULTIMODE_OR_MULTIMODEMASTER(hadc)  )
    {
      /* Start ADC conversion on regular group with SW start */
      SET_BIT(hadc->Instance->CR2, (ADC_CR2_SWSTART | ADC_CR2_EXTTRIG));
    }
    else
    {
      /* Start ADC conversion on regular group with external trigger */
      SET_BIT(hadc->Instance->CR2, ADC_CR2_EXTTRIG);
    }
  }
  else
  {
    /* Process unlocked */
    __HAL_UNLOCK(hadc);
  }
    
  /* Return function status */
  return tmp_hal_status;
}

/**
  * @brief  Stop ADC conversion of regular group (and injected channels in 
  *         case of auto_injection mode), disable ADC peripheral.
  * @note   ADC peripheral disable is forcing stop of potential 
  *         conversion on injected group. If injected group is under use, it
  *         should be preliminarily stopped using HAL_ADCEx_InjectedStop function.
  * @param  hadc: ADC handle
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_ADC_Stop(ADC_HandleTypeDef* hadc)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
     
  /* Process locked */
  __HAL_LOCK(hadc);
  
  /* Stop potential conversion on going, on regular and injected groups */
  /* Disable ADC peripheral */
  tmp_hal_status = ADC_ConversionStop_Disable(hadc);
  
  /* Check if ADC is effectively disabled */
  if (tmp_hal_status == HAL_OK)
  {
    /* Set ADC state */
    ADC_STATE_CLR_SET(hadc->State,
                      HAL_ADC_STATE_REG_BUSY | HAL_ADC_STATE_INJ_BUSY,
                      HAL_ADC_STATE_READY);
  }
  
  /* Process unlocked */
  __HAL_UNLOCK(hadc);
  
  /* Return function status */
  return tmp_hal_status;
}

/**
  * @brief  Wait for regular group conversion to be completed.
  * @note   This function cannot be used in a particular setup: ADC configured 
  *         in DMA mode.
  *         In this case, DMA resets the flag EOC and polling cannot be
  *         performed on each conversion.
  * @param  hadc: ADC handle
  * @param  Timeout: Timeout value in millisecond.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_PollForConversion(ADC_HandleTypeDef* hadc, uint32_t Timeout)
{
  uint32_t tickstart = 0U;
  
  /* Variables for polling in case of scan mode enabled and polling for each  */
  /* conversion.                                                              */
  __IO uint32_t Conversion_Timeout_CPU_cycles = 0U;
  uint32_t Conversion_Timeout_CPU_cycles_max = 0U;
 
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  
  /* Get tick count */
  tickstart = HAL_GetTick();
  
  /* Verification that ADC configuration is compliant with polling for        */
  /* each conversion:                                                         */
  /* Particular case is ADC configured in DMA mode                            */
  if (HAL_IS_BIT_SET(hadc->Instance->CR2, ADC_CR2_DMA))
  {
    /* Update ADC state machine to error */
    SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_CONFIG);
    
    /* Process unlocked */
    __HAL_UNLOCK(hadc);
    
    return HAL_ERROR;
  }
  
  /* Polling for end of conversion: differentiation if single/sequence        */
  /* conversion.                                                              */
  /*  - If single conversion for regular group (Scan mode disabled or enabled */
  /*    with NbrOfConversion =1), flag EOC is used to determine the           */
  /*    conversion completion.                                                */
  /*  - If sequence conversion for regular group (scan mode enabled and       */
  /*    NbrOfConversion >=2), flag EOC is set only at the end of the          */
  /*    sequence.                                                             */
  if (HAL_IS_BIT_CLR(hadc->Instance->CR1, ADC_CR1_SCAN) &&
      HAL_IS_BIT_CLR(hadc->Instance->SQR1, ADC_SQR1_L)    )
  {
    /* Wait until End of Conversion flag is raised */
    while(HAL_IS_BIT_CLR(hadc->Instance->SR, ADC_FLAG_EOC))
    {
      /* Check if timeout is disabled (set to infinite wait) */
      if(Timeout != HAL_MAX_DELAY)
      {
        if((Timeout == 0U) || ((HAL_GetTick() - tickstart ) > Timeout))
        {
          /* New check to avoid false timeout detection in case of preemption */
          if(HAL_IS_BIT_CLR(hadc->Instance->SR, ADC_FLAG_EOC))
          {
            /* Update ADC state machine to timeout */
            SET_BIT(hadc->State, HAL_ADC_STATE_TIMEOUT);
            
            /* Process unlocked */
            __HAL_UNLOCK(hadc);
            
            return HAL_TIMEOUT;
          }
        }
      }
    }
  }
  else
  {
    /* Replace polling by wait for maximum conversion time */
    /*  - Computation of CPU clock cycles corresponding to ADC clock cycles   */
    /*    and ADC maximum conversion cycles on all channels.                  */
    /*  - Wait for the expected ADC clock cycles delay                        */
    Conversion_Timeout_CPU_cycles_max = ((SystemCoreClock
                                          / HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_ADC123))
                                         * ADC_SAMPLINGTIME_640CYCLES_5                 );
    
    while(Conversion_Timeout_CPU_cycles < Conversion_Timeout_CPU_cycles_max)
    {
      /* Check if timeout is disabled (set to infinite wait) */
      if(Timeout != HAL_MAX_DELAY)
      {
        if((Timeout == 0U) || ((HAL_GetTick() - tickstart) > Timeout))
        {
          /* New check to avoid false timeout detection in case of preemption */
          if(Conversion_Timeout_CPU_cycles < Conversion_Timeout_CPU_cycles_max)
          {
            /* Update ADC state machine to timeout */
            SET_BIT(hadc->State, HAL_ADC_STATE_TIMEOUT);

            /* Process unlocked */
            __HAL_UNLOCK(hadc);

            return HAL_TIMEOUT;
          }
        }
      }
      Conversion_Timeout_CPU_cycles ++;
    }
  }
  
  /* Clear regular group conversion flag */
  __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_STRT | ADC_FLAG_EOC);
  
  /* Update ADC state machine */
  SET_BIT(hadc->State, HAL_ADC_STATE_REG_EOC);
  
  if(ADC_IS_SOFTWARE_START_REGULAR(hadc)        && 
     (hadc->Init.ContinuousConvMode == DISABLE)   )
  {   
    /* Set ADC state */
    CLEAR_BIT(hadc->State, HAL_ADC_STATE_REG_BUSY);   

    if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_INJ_BUSY))
    { 
      SET_BIT(hadc->State, HAL_ADC_STATE_READY);
    }
  }
  
  /* Return ADC state */
  return HAL_OK;
}

/**
  * @brief  Poll for conversion event.
  * @param  hadc: ADC handle
  * @param  EventType: the ADC event type.
  *          This parameter can be one of the following values:
  *            @arg ADC_AWD_EVENT: ADC Analog watchdog event.
  * @param  Timeout: Timeout value in millisecond.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_PollForEvent(ADC_HandleTypeDef* hadc, uint32_t EventType, uint32_t Timeout)
{
  uint32_t tickstart = 0U; 

  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  assert_param(IS_ADC_EVENT_TYPE(EventType));
  
  /* Get tick count */
  tickstart = HAL_GetTick();
  
  /* Check selected event flag */
  while(__HAL_ADC_GET_FLAG(hadc, EventType) == RESET)
  {
    /* Check if timeout is disabled (set to infinite wait) */
    if(Timeout != HAL_MAX_DELAY)
    {
      if((Timeout == 0U) || ((HAL_GetTick() - tickstart ) > Timeout))
      {
        /* New check to avoid false timeout detection in case of preemption */
        if(__HAL_ADC_GET_FLAG(hadc, EventType) == RESET)
        {
          /* Update ADC state machine to timeout */
          SET_BIT(hadc->State, HAL_ADC_STATE_TIMEOUT);

          /* Process unlocked */
          __HAL_UNLOCK(hadc);

          return HAL_TIMEOUT;
        }
      }
    }
  }
  
  /* Analog watchdog (level out of window) event */
  /* Set ADC state */
  SET_BIT(hadc->State, HAL_ADC_STATE_AWD1);
    
  /* Clear ADC analog watchdog flag */
  __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_AWD);
  
  /* Return ADC state */
  return HAL_OK;
}

/**
  * @brief  Enables ADC, starts conversion of regular group with interruption.
  *         Interruptions enabled in this function:
  *          - EOC (end of conversion of regular group)
  *         Each of these interruptions has its dedicated callback function.
  * @param  hadc: ADC handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_Start_IT(ADC_HandleTypeDef* hadc)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  
  /* Process locked */
  __HAL_LOCK(hadc);
    
  /* Enable the ADC peripheral */
  tmp_hal_status = ADC_Enable(hadc);
  
  /* Start conversion if ADC is effectively enabled */
  if (tmp_hal_status == HAL_OK)
  {
    /* Set ADC state                                                          */
    /* - Clear state bitfield related to regular group conversion results     */
    /* - Set state bitfield related to regular operation                      */
    ADC_STATE_CLR_SET(hadc->State,
                      HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC, HAL_ADC_STATE_REG_BUSY);
    
    /* Set group injected state (from auto-injection) and multimode state     */
    /* for all cases of multimode: independent mode, multimode ADC master     */
    /* or multimode ADC slave (for devices with several ADCs):                */
    if (ADC_NONMULTIMODE_OR_MULTIMODEMASTER(hadc))
    {
      /* Set ADC state (ADC independent or master) */
      CLEAR_BIT(hadc->State, HAL_ADC_STATE_MULTIMODE_SLAVE);
      
      /* If conversions on group regular are also triggering group injected,  */
      /* update ADC state.                                                    */
      if (READ_BIT(hadc->Instance->CR1, ADC_CR1_JAUTO) != RESET)
      {
        ADC_STATE_CLR_SET(hadc->State, HAL_ADC_STATE_INJ_EOC, HAL_ADC_STATE_INJ_BUSY);  
      }
    }
    else
    {
      /* Set ADC state (ADC slave) */
      SET_BIT(hadc->State, HAL_ADC_STATE_MULTIMODE_SLAVE);
      
      /* If conversions on group regular are also triggering group injected,  */
      /* update ADC state.                                                    */
      if (ADC_MULTIMODE_AUTO_INJECTED(hadc))
      {
        ADC_STATE_CLR_SET(hadc->State, HAL_ADC_STATE_INJ_EOC, HAL_ADC_STATE_INJ_BUSY);
      }
    }
    
    /* State machine update: Check if an injected conversion is ongoing */
    if (HAL_IS_BIT_SET(hadc->State, HAL_ADC_STATE_INJ_BUSY))
    {
      /* Reset ADC error code fields related to conversions on group regular */
      CLEAR_BIT(hadc->ErrorCode, (HAL_ADC_ERROR_OVR | HAL_ADC_ERROR_DMA));         
    }
    else
    {
      /* Reset ADC all error code fields */
      ADC_CLEAR_ERRORCODE(hadc);
    }
    
    /* Process unlocked */
    /* Unlock before starting ADC conversions: in case of potential           */
    /* interruption, to let the process to ADC IRQ Handler.                   */
    __HAL_UNLOCK(hadc);
    
    /* Clear regular group conversion flag and overrun flag */
    /* (To ensure of no unknown state from potential previous ADC operations) */
    __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_EOC);
    
    /* Enable end of conversion interrupt for regular group */
    __HAL_ADC_ENABLE_IT(hadc, ADC_IT_EOC);
    
    /* Enable conversion of regular group.                                    */
    /* If software start has been selected, conversion starts immediately.    */
    /* If external trigger has been selected, conversion will start at next   */
    /* trigger event.                                                         */
    /* Case of multimode enabled:                                             */ 
    /*  - if ADC is slave, ADC is enabled only (conversion is not started).   */
    /*  - if ADC is master, ADC is enabled and conversion is started.         */
    if (ADC_IS_SOFTWARE_START_REGULAR(hadc)      &&
        ADC_NONMULTIMODE_OR_MULTIMODEMASTER(hadc)  )
    {
      /* Start ADC conversion on regular group with SW start */
      SET_BIT(hadc->Instance->CR2, (ADC_CR2_SWSTART | ADC_CR2_EXTTRIG));
    }
    else
    {
      /* Start ADC conversion on regular group with external trigger */
      SET_BIT(hadc->Instance->CR2, ADC_CR2_EXTTRIG);
    }
  }
  else
  {
    /* Process unlocked */
    __HAL_UNLOCK(hadc);
  }
  
  /* Return function status */
  return tmp_hal_status;
}

/**
  * @brief  Stop ADC conversion of regular group (and injected group in 
  *         case of auto_injection mode), disable interrution of 
  *         end-of-conversion, disable ADC peripheral.
  * @param  hadc: ADC handle
  * @retval None
  */
HAL_StatusTypeDef HAL_ADC_Stop_IT(ADC_HandleTypeDef* hadc)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
     
  /* Process locked */
  __HAL_LOCK(hadc);
  
  /* Stop potential conversion on going, on regular and injected groups */
  /* Disable ADC peripheral */
  tmp_hal_status = ADC_ConversionStop_Disable(hadc);
  
  /* Check if ADC is effectively disabled */
  if (tmp_hal_status == HAL_OK)
  {
    /* Disable ADC end of conversion interrupt for regular group */
    __HAL_ADC_DISABLE_IT(hadc, ADC_IT_EOC);
    
    /* Set ADC state */
    ADC_STATE_CLR_SET(hadc->State,
                      HAL_ADC_STATE_REG_BUSY | HAL_ADC_STATE_INJ_BUSY,
                      HAL_ADC_STATE_READY);
  }
  
  /* Process unlocked */
  __HAL_UNLOCK(hadc);
  
  /* Return function status */
  return tmp_hal_status;
}

/**
  * @brief  Enables ADC, starts conversion of regular group and transfers result
  *         through DMA.
  *         Interruptions enabled in this function:
  *          - DMA transfer complete
  *          - DMA half transfer
  *         Each of these interruptions has its dedicated callback function.
  * @note   For devices with several ADCs: This function is for single-ADC mode 
  *         only. For multimode, use the dedicated MultimodeStart function.
  * @param  hadc: ADC handle
  * @param  pData: The destination Buffer address.
  * @param  Length: The length of data to be transferred from ADC peripheral to memory.
  * @retval None
  */
HAL_StatusTypeDef HAL_ADC_Start_DMA(ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
    
  /* Verification if multimode is disabled (for devices with several ADC)     */
  /* If multimode is enabled, dedicated function multimode conversion         */
  /* start DMA must be used.                                                  */
  if(ADC_MULTIMODE_IS_ENABLE(hadc) == RESET)
  {
    /* Process locked */
    __HAL_LOCK(hadc);
    
    /* Enable the ADC peripheral */
    tmp_hal_status = ADC_Enable(hadc);
    
    /* Start conversion if ADC is effectively enabled */
    if (tmp_hal_status == HAL_OK)
    {
      /* Set ADC state                                                        */
      /* - Clear state bitfield related to regular group conversion results   */
      /* - Set state bitfield related to regular operation                    */
      ADC_STATE_CLR_SET(hadc->State,
                        HAL_ADC_STATE_READY | HAL_ADC_STATE_REG_EOC, HAL_ADC_STATE_REG_BUSY);
    
    /* Set group injected state (from auto-injection) and multimode state     */
    /* for all cases of multimode: independent mode, multimode ADC master     */
    /* or multimode ADC slave (for devices with several ADCs):                */
    if (ADC_NONMULTIMODE_OR_MULTIMODEMASTER(hadc))
    {
      /* Set ADC state (ADC independent or master) */
      CLEAR_BIT(hadc->State, HAL_ADC_STATE_MULTIMODE_SLAVE);
      
      /* If conversions on group regular are also triggering group injected,  */
      /* update ADC state.                                                    */
      if (READ_BIT(hadc->Instance->CR1, ADC_CR1_JAUTO) != RESET)
      {
        ADC_STATE_CLR_SET(hadc->State, HAL_ADC_STATE_INJ_EOC, HAL_ADC_STATE_INJ_BUSY);  
      }
    }
    else
    {
      /* Set ADC state (ADC slave) */
      SET_BIT(hadc->State, HAL_ADC_STATE_MULTIMODE_SLAVE);
      
      /* If conversions on group regular are also triggering group injected,  */
      /* update ADC state.                                                    */
      if (ADC_MULTIMODE_AUTO_INJECTED(hadc))
      {
        ADC_STATE_CLR_SET(hadc->State, HAL_ADC_STATE_INJ_EOC, HAL_ADC_STATE_INJ_BUSY);
      }
    }
      
      /* State machine update: Check if an injected conversion is ongoing */
      if (HAL_IS_BIT_SET(hadc->State, HAL_ADC_STATE_INJ_BUSY))
      {
        /* Reset ADC error code fields related to conversions on group regular */
        CLEAR_BIT(hadc->ErrorCode, (HAL_ADC_ERROR_OVR | HAL_ADC_ERROR_DMA));         
      }
      else
      {
        /* Reset ADC all error code fields */
        ADC_CLEAR_ERRORCODE(hadc);
      }
      
      /* Process unlocked */
      /* Unlock before starting ADC conversions: in case of potential         */
      /* interruption, to let the process to ADC IRQ Handler.                 */
      __HAL_UNLOCK(hadc);
      
      /* Set the DMA transfer complete callback */
      hadc->DMA_Handle->XferCpltCallback = ADC_DMAConvCplt;

      /* Set the DMA half transfer complete callback */
      hadc->DMA_Handle->XferHalfCpltCallback = ADC_DMAHalfConvCplt;
      
      /* Set the DMA error callback */
      hadc->DMA_Handle->XferErrorCallback = ADC_DMAError;

      
      /* Manage ADC and DMA start: ADC overrun interruption, DMA start, ADC   */
      /* start (in case of SW start):                                         */
      
      /* Clear regular group conversion flag and overrun flag */
      /* (To ensure of no unknown state from potential previous ADC           */
      /* operations)                                                          */
      __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_EOC);
      
      /* Enable ADC DMA mode */
      SET_BIT(hadc->Instance->CR2, ADC_CR2_DMA);
      
      /* Start the DMA channel */
      HAL_DMA_Start_IT(hadc->DMA_Handle, (uint32_t)&hadc->Instance->DR, (uint32_t)pData, Length);
      
      /* Enable conversion of regular group.                                  */
      /* If software start has been selected, conversion starts immediately.  */
      /* If external trigger has been selected, conversion will start at next */
      /* trigger event.                                                       */
      if (ADC_IS_SOFTWARE_START_REGULAR(hadc))
      {
        /* Start ADC conversion on regular group with SW start */
        SET_BIT(hadc->Instance->CR2, (ADC_CR2_SWSTART | ADC_CR2_EXTTRIG));
      }
      else
      {
        /* Start ADC conversion on regular group with external trigger */
        SET_BIT(hadc->Instance->CR2, ADC_CR2_EXTTRIG);
      }
    }
    else
    {
      /* Process unlocked */
      __HAL_UNLOCK(hadc);
    }
  }
  else
  {
    tmp_hal_status = HAL_ERROR;
  }
  
  /* Return function status */
  return tmp_hal_status;
}

/**
  * @brief  Stop ADC conversion of regular group (and injected group in 
  *         case of auto_injection mode), disable ADC DMA transfer, disable 
  *         ADC peripheral.
  * @note   ADC peripheral disable is forcing stop of potential 
  *         conversion on injected group. If injected group is under use, it
  *         should be preliminarily stopped using HAL_ADCEx_InjectedStop function.
  * @param  hadc: ADC handle
  * @retval HAL status.
  */
HAL_StatusTypeDef HAL_ADC_Stop_DMA(ADC_HandleTypeDef* hadc)
{
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
     
  /* Process locked */
  __HAL_LOCK(hadc);
  
  /* Stop potential conversion on going, on regular and injected groups */
  /* Disable ADC peripheral */
  tmp_hal_status = ADC_ConversionStop_Disable(hadc);
  
  /* Check if ADC is effectively disabled */
  if (tmp_hal_status == HAL_OK)
  {
    /* Disable ADC DMA mode */
    CLEAR_BIT(hadc->Instance->CR2, ADC_CR2_DMA);
    
    /* Disable the DMA channel (in case of DMA in circular mode or stop while */
    /* DMA transfer is on going)                                              */
    if (hadc->DMA_Handle->State == HAL_DMA_STATE_BUSY)
    {
      tmp_hal_status = HAL_DMA_Abort(hadc->DMA_Handle);
      
      /* Check if DMA channel effectively disabled */
      if (tmp_hal_status == HAL_OK)
      {
        /* Set ADC state */
        ADC_STATE_CLR_SET(hadc->State,
                          HAL_ADC_STATE_REG_BUSY | HAL_ADC_STATE_INJ_BUSY,
                          HAL_ADC_STATE_READY);
      }
      else
      {
        /* Update ADC state machine to error */
        SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_DMA);
      }
    }
  }
  
  /* Process unlocked */
  __HAL_UNLOCK(hadc);
    
  /* Return function status */
  return tmp_hal_status;
}

/**
  * @brief  Get ADC regular group conversion result.
  * @note   Reading register DR automatically clears ADC flag EOC
  *         (ADC group regular end of unitary conversion).
  * @note   This function does not clear ADC flag EOC 
  *         (ADC group regular end of sequence conversion).
  *         To clear this flag, either use function: 
  *         in programming model IT: @ref HAL_ADC_IRQHandler(), in programming
  *         model polling: @ref HAL_ADC_PollForConversion() 
  *         or @ref __HAL_ADC_CLEAR_FLAG(&hadc, ADC_FLAG_EOC).
  * @param  hadc: ADC handle
  * @retval ADC group regular conversion data
  */
uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef* hadc)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));

  /* Note: EOC flag is not cleared here by software because automatically     */
  /*       cleared by hardware when reading register DR.                      */
  
  /* Return ADC converted value */ 
  return hadc->Instance->DR;
}

/**
  * @brief  Handles ADC interrupt request  
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_IRQHandler(ADC_HandleTypeDef* hadc)
{
  uint32_t tmp_sr = hadc->Instance->SR;
  uint32_t tmp_cr1 = hadc->Instance->CR1;

  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  assert_param(IS_FUNCTIONAL_STATE(hadc->Init.ContinuousConvMode));
  assert_param(IS_ADC_REGULAR_NB_CONV(hadc->Init.NbrOfConversion));
  
  
  /* ========== Check End of Conversion flag for regular group ========== */
  if((tmp_cr1 & ADC_IT_EOC) == ADC_IT_EOC)
  {
    if((tmp_sr & ADC_FLAG_EOC) == ADC_FLAG_EOC)
    {
      /* Update state machine on conversion status if not in error state */
      if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL))
      {
        /* Set ADC state */
        SET_BIT(hadc->State, HAL_ADC_STATE_REG_EOC); 
      }
      
      /* Determine whether any further conversion upcoming on group regular   */
      /* by external trigger, continuous mode or scan sequence on going.      */
      if(ADC_IS_SOFTWARE_START_REGULAR(hadc)        && 
         (hadc->Init.ContinuousConvMode == DISABLE)   )
      {
        /* Disable ADC end of conversion interrupt on group regular */
        __HAL_ADC_DISABLE_IT(hadc, ADC_IT_EOC);
        
        /* Set ADC state */
        CLEAR_BIT(hadc->State, HAL_ADC_STATE_REG_BUSY);   
        
        if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_INJ_BUSY))
        {
          SET_BIT(hadc->State, HAL_ADC_STATE_READY);
        }
      }

      /* Conversion complete callback */
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
      hadc->ConvCpltCallback(hadc);
#else
      HAL_ADC_ConvCpltCallback(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
      
      /* Clear regular group conversion flag */
      __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_STRT | ADC_FLAG_EOC);
    }
  }
  
  /* ========== Check End of Conversion flag for injected group ========== */
  if((tmp_cr1 & ADC_IT_JEOC) == ADC_IT_JEOC)
  {
    if((tmp_sr & ADC_FLAG_JEOC) == ADC_FLAG_JEOC)
    {
      /* Update state machine on conversion status if not in error state */
      if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL))
      {
        /* Set ADC state */
        SET_BIT(hadc->State, HAL_ADC_STATE_INJ_EOC);
      }

      /* Determine whether any further conversion upcoming on group injected  */
      /* by external trigger, scan sequence on going or by automatic injected */
      /* conversion from group regular (same conditions as group regular      */
      /* interruption disabling above).                                       */
      if(ADC_IS_SOFTWARE_START_INJECTED(hadc)                     || 
         (HAL_IS_BIT_CLR(hadc->Instance->CR1, ADC_CR1_JAUTO) &&     
         (ADC_IS_SOFTWARE_START_REGULAR(hadc)        &&
          (hadc->Init.ContinuousConvMode == DISABLE)   )        )   )
      {
        /* Disable ADC end of conversion interrupt on group injected */
        __HAL_ADC_DISABLE_IT(hadc, ADC_IT_JEOC);
        
        /* Set ADC state */
        CLEAR_BIT(hadc->State, HAL_ADC_STATE_INJ_BUSY);   

        if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_REG_BUSY))
        { 
          SET_BIT(hadc->State, HAL_ADC_STATE_READY);
        }
      }

      /* Conversion complete callback */ 
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
      hadc->InjectedConvCpltCallback(hadc);
#else
      HAL_ADCEx_InjectedConvCpltCallback(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
      
      /* Clear injected group conversion flag */
      __HAL_ADC_CLEAR_FLAG(hadc, (ADC_FLAG_JSTRT | ADC_FLAG_JEOC));
    }
  }
   
  /* ========== Check Analog watchdog flags ========== */
  if((tmp_cr1 & ADC_IT_AWD) == ADC_IT_AWD)
  {
    if((tmp_sr & ADC_FLAG_AWD) == ADC_FLAG_AWD)
    {
      /* Set ADC state */
      SET_BIT(hadc->State, HAL_ADC_STATE_AWD1);
      
      /* Level out of window callback */ 
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
      hadc->LevelOutOfWindowCallback(hadc);
#else
      HAL_ADC_LevelOutOfWindowCallback(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
      
      /* Clear the ADC analog watchdog flag */
      __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_AWD);
    }
  }
  
}

/**
  * @brief  Conversion complete callback in non blocking mode 
  * @param  hadc: ADC handle
  * @retval None
  */
__weak void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_ADC_ConvCpltCallback must be implemented in the user file.
   */
}

/**
  * @brief  Conversion DMA half-transfer callback in non blocking mode 
  * @param  hadc: ADC handle
  * @retval None
  */
__weak void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_ADC_ConvHalfCpltCallback must be implemented in the user file.
  */
}

/**
  * @brief  Analog watchdog callback in non blocking mode. 
  * @param  hadc: ADC handle
  * @retval None
  */
__weak void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_ADC_LevelOutOfWindowCallback must be implemented in the user file.
  */
}

/**
  * @brief  ADC error callback in non blocking mode
  *        (ADC conversion with interruption or transfer by DMA)
  * @param  hadc: ADC handle
  * @retval None
  */
__weak void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hadc);
  /* NOTE : This function should not be modified. When the callback is needed,
            function HAL_ADC_ErrorCallback must be implemented in the user file.
  */
}
  
/**
  * @brief  Configures the the selected channel to be linked to the regular
  *         group.
  * @note   In case of usage of internal measurement channels:
  *         Vbat/VrefInt/TempSensor.
  *         These internal paths can be be disabled using function 
  *         HAL_ADC_DeInit().
  * @note   Possibility to update parameters on the fly:
  *         This function initializes channel into regular group, following  
  *         calls to this function can be used to reconfigure some parameters 
  *         of structure "ADC_ChannelConfTypeDef" on the fly, without resetting 
  *         the ADC.
  *         The setting of these parameters is conditioned to ADC state.
  *         For parameters constraints, see comments of structure 
  *         "ADC_ChannelConfTypeDef".
  * @param  hadc: ADC handle
  * @param  sConfig: Structure of ADC channel for regular group.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_ConfigChannel(ADC_HandleTypeDef* hadc, ADC_ChannelConfTypeDef* sConfig)
{ 
  HAL_StatusTypeDef tmp_hal_status = HAL_OK;
  __IO uint32_t wait_loop_index = 0U;
  
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  assert_param(IS_ADC_CHANNEL(sConfig->Channel));
  assert_param(IS_ADC_REGULAR_RANK(sConfig->Rank));
  assert_param(IS_ADC_SAMPLE_TIME(sConfig->SamplingTime));
  
  /* Process locked */
  __HAL_LOCK(hadc);
  
  /* Regular sequence configuration */
  /* For Rank 1 to 6 */
  if (sConfig->Rank < 7U)
  {
    MODIFY_REG(hadc->Instance->SQR3                        ,
               ADC_SQR3_RK(ADC_SQR3_SQ1, sConfig->Rank)    ,
               ADC_SQR3_RK(sConfig->Channel, sConfig->Rank) );
  }
  /* For Rank 7 to 8 */
  else
  {
    MODIFY_REG(hadc->Instance->SQR2                        ,
               ADC_SQR2_RK(ADC_SQR2_SQ7, sConfig->Rank)    ,
               ADC_SQR2_RK(sConfig->Channel, sConfig->Rank) );
  }
  
  /* Channel sampling time configuration */
  /* For channels 10 to 15 */
  if (sConfig->Channel >= ADC_CHANNEL_10)
  {
    MODIFY_REG(hadc->Instance->SMPR1                             ,
               ADC_SMPR1(ADC_SMPR1_SMP10, sConfig->Channel)      ,
               ADC_SMPR1(sConfig->SamplingTime, sConfig->Channel) );
  }
  else /* For channels 0 to 9 */
  {
    MODIFY_REG(hadc->Instance->SMPR2                             ,
               ADC_SMPR2(ADC_SMPR2_SMP0, sConfig->Channel)       ,
               ADC_SMPR2(sConfig->SamplingTime, sConfig->Channel) );
  }
  
  /* If ADC1 Channel_15/ ADC2 channel_15 is selected, enable Temperature sensor  */
  /* and VREFINT measurement path.                                               */
  if (sConfig->Channel == ADC_CHANNEL_15)
  {
    if (hadc->Instance == ADC1 || hadc->Instance == ADC2)
    {
      if (READ_BIT(ADC1->CR2, ADC_CR2_TSVREFE) == RESET)
      {
        SET_BIT(ADC1->CR2, ADC_CR2_TSVREFE);
        
        if (sConfig->Channel == ADC_CHANNEL_15)
        {
          /* Delay for temperature sensor stabilization time */
          /* Compute number of CPU cycles to wait for */
          wait_loop_index = (ADC_TEMPSENSOR_DELAY_US * (SystemCoreClock / 1000000U));
          while(wait_loop_index != 0U)
          {
            wait_loop_index--;
          }
        }
      }
    }
  }
  
  /* Process unlocked */
  __HAL_UNLOCK(hadc);
  
  /* Return function status */
  return tmp_hal_status;
}

/**
  * @brief  Configures the analog watchdog.
  * @note   Analog watchdog thresholds can be modified while ADC conversion
  *         is on going.
  *         In this case, some constraints must be taken into account:
  *         the programmed threshold values are effective from the next
  *         ADC EOC (end of unitary conversion).
  *         Considering that registers write delay may happen due to
  *         bus activity, this might cause an uncertainty on the
  *         effective timing of the new programmed threshold values.
  * @param  hadc: ADC handle
  * @param  AnalogWDGConfig: Structure of ADC analog watchdog configuration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_AnalogWDGConfig(ADC_HandleTypeDef* hadc, ADC_AnalogWDGConfTypeDef* AnalogWDGConfig)
{
  /* Check the parameters */
  assert_param(IS_ADC_ALL_INSTANCE(hadc->Instance));
  assert_param(IS_ADC_ANALOG_WATCHDOG_MODE(AnalogWDGConfig->WatchdogMode));
  assert_param(IS_FUNCTIONAL_STATE(AnalogWDGConfig->ITMode));
  assert_param(IS_ADC_RANGE(AnalogWDGConfig->HighThreshold));
  assert_param(IS_ADC_RANGE(AnalogWDGConfig->LowThreshold));
  
  if((AnalogWDGConfig->WatchdogMode == ADC_ANALOGWATCHDOG_SINGLE_REG)     ||
     (AnalogWDGConfig->WatchdogMode == ADC_ANALOGWATCHDOG_SINGLE_INJEC)   ||
     (AnalogWDGConfig->WatchdogMode == ADC_ANALOGWATCHDOG_SINGLE_REGINJEC)  )
  {
    assert_param(IS_ADC_CHANNEL(AnalogWDGConfig->Channel));
  }
  
  /* Process locked */
  __HAL_LOCK(hadc);
  
  /* Analog watchdog configuration */

  /* Configure ADC Analog watchdog interrupt */
  if(AnalogWDGConfig->ITMode == ENABLE)
  {
    /* Enable the ADC Analog watchdog interrupt */
    __HAL_ADC_ENABLE_IT(hadc, ADC_IT_AWD);
  }
  else
  {
    /* Disable the ADC Analog watchdog interrupt */
    __HAL_ADC_DISABLE_IT(hadc, ADC_IT_AWD);
  }
  
  /* Configuration of analog watchdog:                                        */
  /*  - Set the analog watchdog enable mode: regular and/or injected groups,  */
  /*    one or all channels.                                                  */
  /*  - Set the Analog watchdog channel (is not used if watchdog              */
  /*    mode "all channels": ADC_CFGR_AWD1SGL=0).                             */
  MODIFY_REG(hadc->Instance->CR1            ,
             ADC_CR1_AWDFILT   |
             ADC_CR1_JAWDEN    |
             ADC_CR1_AWDEN                  ,
             AnalogWDGConfig->WatchdogFilter|
             AnalogWDGConfig->WatchdogMode  );
  
  if((AnalogWDGConfig->WatchdogMode & ADC_CR1_AWDSGL) == ADC_CR1_AWDSGL)
  {
    MODIFY_REG(hadc->Instance->CR1, ADC_CR1_AWDCH, AnalogWDGConfig->Channel);
  }
  
  /* Set the high threshold */
  WRITE_REG(hadc->Instance->HTR, AnalogWDGConfig->HighThreshold);
  
  /* Set the low threshold */
  WRITE_REG(hadc->Instance->LTR, AnalogWDGConfig->LowThreshold);

  /* Process unlocked */
  __HAL_UNLOCK(hadc);
  
  /* Return function status */
  return HAL_OK;
}


/**
  * @brief  return the ADC state
  * @param  hadc: ADC handle
  * @retval HAL state
  */
uint32_t HAL_ADC_GetState(ADC_HandleTypeDef* hadc)
{
  /* Return ADC state */
  return hadc->State;
}

/**
  * @brief  Return the ADC error code
  * @param  hadc: ADC handle
  * @retval ADC Error Code
  */
uint32_t HAL_ADC_GetError(ADC_HandleTypeDef *hadc)
{
  return hadc->ErrorCode;
}


/**
  * @brief  Enable the selected ADC.
  * @note   Prerequisite condition to use this function: ADC must be disabled
  *         and voltage regulator must be enabled (done into HAL_ADC_Init()).
  * @param  hadc: ADC handle
  * @retval HAL status.
  */
HAL_StatusTypeDef ADC_Enable(ADC_HandleTypeDef* hadc)
{
  uint32_t tickstart = 0U;
  __IO uint32_t wait_loop_index = 0U;
  
  /* ADC enable and wait for ADC ready (in case of ADC is disabled or         */
  /* enabling phase not yet completed: flag ADC ready not yet set).           */
  /* Timeout implemented to not be stuck if ADC cannot be enabled (possible   */
  /* causes: ADC clock not running, ...).                                     */
  if (ADC_IS_ENABLE(hadc) == RESET)
  {
    /* Enable the Peripheral */
    __HAL_ADC_ENABLE(hadc);
    
    /* Delay for ADC stabilization time */
    /* Compute number of CPU cycles to wait for */
    wait_loop_index = (ADC_STAB_DELAY_US * (SystemCoreClock / 1000000U));
    while(wait_loop_index != 0U)
    {
      wait_loop_index--;
    }
    
    /* Get tick count */
    tickstart = HAL_GetTick();

    /* Wait for ADC effectively enabled */
    while(ADC_IS_ENABLE(hadc) == RESET)
    {
      if((HAL_GetTick() - tickstart) > ADC_ENABLE_TIMEOUT)
      {
        /* New check to avoid false timeout detection in case of preemption */
        if(ADC_IS_ENABLE(hadc) == RESET)
        {
          /* Update ADC state machine to error */
          SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL);

          /* Set ADC error code to ADC IP internal error */
          SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_INTERNAL);

          /* Process unlocked */
          __HAL_UNLOCK(hadc);

          return HAL_ERROR;
        }
      }
    }
  }
   
  /* Return HAL status */
  return HAL_OK;
}

/**
  * @brief  Stop ADC conversion and disable the selected ADC
  * @note   Prerequisite condition to use this function: ADC conversions must be
  *         stopped to disable the ADC.
  * @param  hadc: ADC handle
  * @retval HAL status.
  */
HAL_StatusTypeDef ADC_ConversionStop_Disable(ADC_HandleTypeDef* hadc)
{
  uint32_t tickstart = 0U;
  
  /* Verification if ADC is not already disabled */
  if (ADC_IS_ENABLE(hadc) != RESET)
  {
    /* Disable the ADC peripheral */
    __HAL_ADC_DISABLE(hadc);
     
    /* Get tick count */
    tickstart = HAL_GetTick();
    
    /* Wait for ADC effectively disabled */
    while(ADC_IS_ENABLE(hadc) != RESET)
    {
      if((HAL_GetTick() - tickstart) > ADC_DISABLE_TIMEOUT)
      {
        /* New check to avoid false timeout detection in case of preemption */
        if(ADC_IS_ENABLE(hadc) != RESET)
        {
          /* Update ADC state machine to error */
          SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL);

          /* Set ADC error code to ADC IP internal error */
          SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_INTERNAL);

          return HAL_ERROR;
        }
      }
    }
  }
  
  /* Return HAL status */
  return HAL_OK;
}

/**
  * @brief  DMA transfer complete callback. 
  * @param  hdma: pointer to DMA handle.
  * @retval None
  */
void ADC_DMAConvCplt(DMA_HandleTypeDef *hdma)
{
  /* Retrieve ADC handle corresponding to current DMA handle */
  ADC_HandleTypeDef* hadc = ( ADC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
 
  /* Update state machine on conversion status if not in error state */
  if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_ERROR_INTERNAL | HAL_ADC_STATE_ERROR_DMA))
  {
    /* Update ADC state machine */
    SET_BIT(hadc->State, HAL_ADC_STATE_REG_EOC);
    
    /* Determine whether any further conversion upcoming on group regular     */
    /* by external trigger, continuous mode or scan sequence on going.        */
    if(ADC_IS_SOFTWARE_START_REGULAR(hadc)        && 
       (hadc->Init.ContinuousConvMode == DISABLE)   )
    {
      /* Set ADC state */
      CLEAR_BIT(hadc->State, HAL_ADC_STATE_REG_BUSY);   
      
      if (HAL_IS_BIT_CLR(hadc->State, HAL_ADC_STATE_INJ_BUSY))
      {
        SET_BIT(hadc->State, HAL_ADC_STATE_READY);
      }
    }
    
    /* Conversion complete callback */
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
    hadc->ConvCpltCallback(hadc);
#else
    HAL_ADC_ConvCpltCallback(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
  }
  else
  {
    /* Call DMA error callback */
    hadc->DMA_Handle->XferErrorCallback(hdma);
  }
}

/**
  * @brief  DMA half transfer complete callback. 
  * @param  hdma: pointer to DMA handle.
  * @retval None
  */
void ADC_DMAHalfConvCplt(DMA_HandleTypeDef *hdma)   
{
  /* Retrieve ADC handle corresponding to current DMA handle */
  ADC_HandleTypeDef* hadc = ( ADC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
  
  /* Half conversion callback */
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
    hadc->ConvHalfCpltCallback(hadc);
#else
  HAL_ADC_ConvHalfCpltCallback(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA error callback 
  * @param  hdma: pointer to DMA handle.
  * @retval None
  */
void ADC_DMAError(DMA_HandleTypeDef *hdma)   
{
  /* Retrieve ADC handle corresponding to current DMA handle */
  ADC_HandleTypeDef* hadc = ( ADC_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;
  
  /* Set ADC state */
  SET_BIT(hadc->State, HAL_ADC_STATE_ERROR_DMA);
  
  /* Set ADC error code to DMA error */
  SET_BIT(hadc->ErrorCode, HAL_ADC_ERROR_DMA);
  
  /* Error callback */
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
  hadc->ErrorCallback(hadc);
#else
  HAL_ADC_ErrorCallback(hadc);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
}

/**
  * @brief  Configure the optional setting for ADC, include the TPS
  * @param  hadc: ADC handle
  * @param  Config: ADC Option configuration type
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_ADC_OptionalConfig(ADC_HandleTypeDef* hadc, ADC_OptionalConfTypeDef* OpConfig)
{
    assert_param(IS_ADC_TPS_OFFSET(OpConfig->TPSOffset));
    assert_param(IS_ADC_SAMPLETIME_USER_SETTING(OpConfig->SampleTime0));
    assert_param(IS_ADC_SAMPLETIME_USER_SETTING(OpConfig->SampleTime1));
    assert_param(IS_ADC_SAMPLETIME_USER_SETTING(OpConfig->SampleTime2));
    assert_param(IS_ADC_SAMPLETIME_USER_SETTING(OpConfig->SampleTime3));
    
    /* Process locked */
    __HAL_LOCK(hadc);
    
    /* ADC set the TPS offset */
    MODIFY_REG(hadc->Instance->TPSOFFSET, ADC_TPSOFFSET, OpConfig->TPSOffset);
    
    /* ADC ANA register unlocked */
    SET_BIT(hadc->Instance->CR3, ADC_CR3_CFG_WPL);
    
    
    if(OpConfig->BlankingSampleTimeMode == ENABLE)
    {
        /* ADC Set COMP blanking setting with additional 12.5 cycles */
        SET_BIT(hadc->Instance->CR3, ADC_CR3_BASW);
    }
    else
    {
        /* ADC Set COMP blanking setting as original */
        CLEAR_BIT(hadc->Instance->CR3, ADC_CR3_BASW);
    }
    
    /* ADC ANA register locked */
    CLEAR_BIT(hadc->Instance->CR3, ADC_CR3_CFG_WPL);
    
    if(OpConfig->EOCOptionalFunction == ENABLE)
    {
        /* ADC Enable EOC optional setting */
        SET_BIT(hadc->Instance->CR4, ADC_CR4_EOCOPT);
    }
    else
    {
        /* ADC Disable EOC optional setting */
        CLEAR_BIT(hadc->Instance->CR4, ADC_CR4_EOCOPT);
    }
 
    /* Process unlocked */
    __HAL_UNLOCK(hadc);
    
    /* Return function status */
    return HAL_OK;   
    
}
  
/** 
  * end of ADC_Function_Definitions @}  
  */

#endif /* HAL_ADC_MODULE_ENABLED */
  
/** 
  * end of ADC @}  
  */
