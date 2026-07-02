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
*   File    : rx32g4xx_hal_cordic.c
*   By      : RX_DV_Team
**************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"


/** @defgroup CORDIC CORDIC
  * @{
  */

#ifdef HAL_CORDIC_MODULE_ENABLED

/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static void CORDIC_WriteInDataIncrementPtr(const CORDIC_HandleTypeDef *hcordic, const int32_t **ppInBuff);
static void CORDIC_ReadOutDataIncrementPtr(const CORDIC_HandleTypeDef *hcordic, int32_t **ppOutBuff);
static void CORDIC_DMAInCplt(DMA_HandleTypeDef *hdma);
static void CORDIC_DMAOutCplt(DMA_HandleTypeDef *hdma);
static void CORDIC_DMAError(DMA_HandleTypeDef *hdma);

/* Exported functions --------------------------------------------------------*/

/** @addtogroup CORDIC_Function_Definitions CORDIC Function Definitions
  * @{
  */

/**
  * @brief  Initialize the CORDIC peripheral and the associated handle.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure.
  *         @arg @ref CORDIC_HandleTypeDef
  * @retval HAL status
  *         @arg @ref HAL_Status_Structure_Definitions
  */
HAL_StatusTypeDef HAL_CORDIC_Init(CORDIC_HandleTypeDef *hcordic)
{
  /* Check the CORDIC handle allocation */
  if (hcordic == NULL)
  {
    /* Return error status */
    return HAL_ERROR;
  }

  /* Check the instance */
  assert_param(IS_CORDIC_ALL_INSTANCE(hcordic->Instance));

#if USE_HAL_CORDIC_REGISTER_CALLBACKS == 1
  if (hcordic->State == HAL_CORDIC_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hcordic->Lock = HAL_UNLOCKED;

    /* Reset callbacks to legacy functions */
    hcordic->ErrorCallback         = HAL_CORDIC_ErrorCallback;         /* Legacy weak ErrorCallback */
    hcordic->CalculateCpltCallback = HAL_CORDIC_CalculateCpltCallback; /* Legacy weak CalculateCpltCallback */

    if (hcordic->MspInitCallback == NULL)
    {
      hcordic->MspInitCallback = HAL_CORDIC_MspInit;                   /* Legacy weak MspInit */
    }

    /* Initialize the low level hardware */
    hcordic->MspInitCallback(hcordic);
  }
#else
  if (hcordic->State == HAL_CORDIC_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hcordic->Lock = HAL_UNLOCKED;

    /* Initialize the low level hardware */
    HAL_CORDIC_MspInit(hcordic);
  }
#endif /* (USE_HAL_CORDIC_REGISTER_CALLBACKS) */

  /* Set CORDIC error code to none */
  hcordic->ErrorCode = HAL_CORDIC_ERROR_NONE;

  /* Reset pInBuff and pOutBuff */
  hcordic->pInBuff = NULL;
  hcordic->pOutBuff = NULL;

  /* Reset NbCalcToOrder and NbCalcToGet */
  hcordic->NbCalcToOrder = 0U;
  hcordic->NbCalcToGet = 0U;

  /* Reset DMADirection */
  hcordic->DMADirection = CORDIC_DMA_DIR_NONE;

  /* Change CORDIC peripheral state */
  hcordic->State = HAL_CORDIC_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  DeInitialize the CORDIC peripheral.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure.
  *         @arg @ref CORDIC_HandleTypeDef
  * @retval HAL status
  *         @arg @ref HAL_Status_Structure_Definitions
  */
HAL_StatusTypeDef HAL_CORDIC_DeInit(CORDIC_HandleTypeDef *hcordic)
{
  /* Check the CORDIC handle allocation */
  if (hcordic == NULL)
  {
    /* Return error status */
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_CORDIC_ALL_INSTANCE(hcordic->Instance));

  /* Change CORDIC peripheral state */
  hcordic->State = HAL_CORDIC_STATE_BUSY;

#if USE_HAL_CORDIC_REGISTER_CALLBACKS == 1
  if (hcordic->MspDeInitCallback == NULL)
  {
    hcordic->MspDeInitCallback = HAL_CORDIC_MspDeInit;
  }

  /* De-Initialize the low level hardware */
  hcordic->MspDeInitCallback(hcordic);
#else
  /* De-Initialize the low level hardware: CLOCK, NVIC, DMA */
  HAL_CORDIC_MspDeInit(hcordic);
#endif /* USE_HAL_CORDIC_REGISTER_CALLBACKS */

  /* Set CORDIC error code to none */
  hcordic->ErrorCode = HAL_CORDIC_ERROR_NONE;

  /* Reset pInBuff and pOutBuff */
  hcordic->pInBuff = NULL;
  hcordic->pOutBuff = NULL;

  /* Reset NbCalcToOrder and NbCalcToGet */
  hcordic->NbCalcToOrder = 0U;
  hcordic->NbCalcToGet = 0U;

  /* Reset DMADirection */
  hcordic->DMADirection = CORDIC_DMA_DIR_NONE;

  /* Change CORDIC peripheral state */
  hcordic->State = HAL_CORDIC_STATE_RESET;

  /* Reset Lock */
  hcordic->Lock = HAL_UNLOCKED;

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the CORDIC MSP.
  * @param  hcordic: CORDIC handle
  *         @arg @ref CORDIC_HandleTypeDef
  * @retval None
  */
__weak void HAL_CORDIC_MspInit(CORDIC_HandleTypeDef *hcordic)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcordic);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_CORDIC_MspInit can be implemented in the user file
   */
}

/**
  * @brief  DeInitialize the CORDIC MSP.
  * @param  hcordic: CORDIC handle
  *         @arg @ref CORDIC_HandleTypeDef
  * @retval None
  */
__weak void HAL_CORDIC_MspDeInit(CORDIC_HandleTypeDef *hcordic)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcordic);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_CORDIC_MspDeInit can be implemented in the user file
   */
}

#if USE_HAL_CORDIC_REGISTER_CALLBACKS == 1
/**
  * @brief  Register a CORDIC CallBack.
  *         To be used instead of the weak predefined callback.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module
  *         @arg @ref CORDIC_HandleTypeDef
  * @param  CallbackID: ID of the callback to be registered
  *         This parameter can be one of the following values:
  *         @arg @ref HAL_CORDIC_ERROR_CB_ID error Callback ID
  *         @arg @ref HAL_CORDIC_CALCULATE_CPLT_CB_ID calculate complete Callback ID
  *         @arg @ref HAL_CORDIC_MSPINIT_CB_ID MspInit callback ID
  *         @arg @ref HAL_CORDIC_MSPDEINIT_CB_ID MspDeInit callback ID
  * @param  pCallback: pointer to the Callback function
  * @retval HAL status
  *         @arg @ref HAL_Status_Structure_Definitions
  */
HAL_StatusTypeDef HAL_CORDIC_RegisterCallback(CORDIC_HandleTypeDef *hcordic, HAL_CORDIC_CallbackIDTypeDef CallbackID,
                                              void (* pCallback)(CORDIC_HandleTypeDef *_hcordic))
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* Update the error code */
    hcordic->ErrorCode |= HAL_CORDIC_ERROR_INVALID_CALLBACK;

    /* Return error status */
    return HAL_ERROR;
  }

  if (hcordic->State == HAL_CORDIC_STATE_READY)
  {
    switch (CallbackID)
    {
      case HAL_CORDIC_ERROR_CB_ID :
        hcordic->ErrorCallback = pCallback;
        break;

      case HAL_CORDIC_CALCULATE_CPLT_CB_ID :
        hcordic->CalculateCpltCallback = pCallback;
        break;

      case HAL_CORDIC_MSPINIT_CB_ID :
        hcordic->MspInitCallback = pCallback;
        break;

      case HAL_CORDIC_MSPDEINIT_CB_ID :
        hcordic->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the error code */
        hcordic->ErrorCode |= HAL_CORDIC_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status = HAL_ERROR;
        break;
    }
  }
  else if (hcordic->State == HAL_CORDIC_STATE_RESET)
  {
    switch (CallbackID)
    {
      case HAL_CORDIC_MSPINIT_CB_ID :
        hcordic->MspInitCallback = pCallback;
        break;

      case HAL_CORDIC_MSPDEINIT_CB_ID :
        hcordic->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the error code */
        hcordic->ErrorCode |= HAL_CORDIC_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status = HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hcordic->ErrorCode |= HAL_CORDIC_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status = HAL_ERROR;
  }

  return status;
}
#endif /* USE_HAL_CORDIC_REGISTER_CALLBACKS */

#if USE_HAL_CORDIC_REGISTER_CALLBACKS == 1
/**
  * @brief  Unregister a CORDIC CallBack.
  *         CORDIC callback is redirected to the weak predefined callback.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module
  *         @arg @ref CORDIC_HandleTypeDef
  * @param  CallbackID: ID of the callback to be unregistered
  *         This parameter can be one of the following values:
  *         @arg @ref HAL_CORDIC_ERROR_CB_ID error Callback ID
  *         @arg @ref HAL_CORDIC_CALCULATE_CPLT_CB_ID calculate complete Callback ID
  *         @arg @ref HAL_CORDIC_MSPINIT_CB_ID MspInit callback ID
  *         @arg @ref HAL_CORDIC_MSPDEINIT_CB_ID MspDeInit callback ID
  * @retval HAL status
  *         @arg @ref HAL_Status_Structure_Definitions
  */
HAL_StatusTypeDef HAL_CORDIC_UnRegisterCallback(CORDIC_HandleTypeDef *hcordic, HAL_CORDIC_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (hcordic->State == HAL_CORDIC_STATE_READY)
  {
    switch (CallbackID)
    {
      case HAL_CORDIC_ERROR_CB_ID :
        hcordic->ErrorCallback = HAL_CORDIC_ErrorCallback;
        break;

      case HAL_CORDIC_CALCULATE_CPLT_CB_ID :
        hcordic->CalculateCpltCallback = HAL_CORDIC_CalculateCpltCallback;
        break;

      case HAL_CORDIC_MSPINIT_CB_ID :
        hcordic->MspInitCallback = HAL_CORDIC_MspInit;
        break;

      case HAL_CORDIC_MSPDEINIT_CB_ID :
        hcordic->MspDeInitCallback = HAL_CORDIC_MspDeInit;
        break;

      default :
        /* Update the error code */
        hcordic->ErrorCode |= HAL_CORDIC_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status = HAL_ERROR;
        break;
    }
  }
  else if (hcordic->State == HAL_CORDIC_STATE_RESET)
  {
    switch (CallbackID)
    {
      case HAL_CORDIC_MSPINIT_CB_ID :
        hcordic->MspInitCallback = HAL_CORDIC_MspInit;
        break;

      case HAL_CORDIC_MSPDEINIT_CB_ID :
        hcordic->MspDeInitCallback = HAL_CORDIC_MspDeInit;
        break;

      default :
        /* Update the error code */
        hcordic->ErrorCode |= HAL_CORDIC_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status = HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the error code */
    hcordic->ErrorCode |= HAL_CORDIC_ERROR_INVALID_CALLBACK;

    /* Return error status */
    status = HAL_ERROR;
  }

  return status;
}
#endif /* USE_HAL_CORDIC_REGISTER_CALLBACKS */

/**
  * @brief  Configure the CORDIC processing according to the specified
            parameters in the CORDIC_ConfigTypeDef structure.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module
  *         @arg @ref CORDIC_HandleTypeDef
  * @param  sConfig: pointer to a CORDIC_ConfigTypeDef structure that
  *         contains the CORDIC configuration information.
  *         @arg @ref CORDIC_ConfigTypeDef
  * @retval HAL status
  *         @arg @ref HAL_Status_Structure_Definitions
  */
HAL_StatusTypeDef HAL_CORDIC_Configure(CORDIC_HandleTypeDef *hcordic, const CORDIC_ConfigTypeDef *sConfig)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the parameters */
  assert_param(IS_CORDIC_FUNCTION(sConfig->Function));
  assert_param(IS_CORDIC_PRECISION(sConfig->Precision));
  assert_param(IS_CORDIC_SCALE(sConfig->Scale));
  assert_param(IS_CORDIC_NBWRITE(sConfig->NbWrite));
  assert_param(IS_CORDIC_NBREAD(sConfig->NbRead));
  assert_param(IS_CORDIC_INSIZE(sConfig->InSize));
  assert_param(IS_CORDIC_OUTSIZE(sConfig->OutSize));

  /* Check handle state is ready */
  if (hcordic->State == HAL_CORDIC_STATE_READY)
  {
    /* Apply all configuration parameters in CORDIC control register */
    MODIFY_REG(hcordic->Instance->CSR,                                                         \
               (CORDIC_CSR_FUNC | CORDIC_CSR_PRECISION | CORDIC_CSR_SCALE |                    \
                CORDIC_CSR_NARGS | CORDIC_CSR_NRES | CORDIC_CSR_ARGSIZE | CORDIC_CSR_RESSIZE), \
               (sConfig->Function | sConfig->Precision | sConfig->Scale |                      \
                sConfig->NbWrite | sConfig->NbRead | sConfig->InSize | sConfig->OutSize));
  }
  else
  {
    /* Set CORDIC error code */
    hcordic->ErrorCode |= HAL_CORDIC_ERROR_NOT_READY;

    /* Return error status */
    status = HAL_ERROR;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  Carry out data of CORDIC processing in polling mode,
  *         according to the existing CORDIC configuration.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module.
  *         @arg @ref CORDIC_HandleTypeDef
  * @param  pInBuff: Pointer to buffer containing input data for CORDIC processing.
  * @param  pOutBuff: Pointer to buffer where output data of CORDIC processing will be stored.
  * @param  NbCalc: Number of CORDIC calculation to process.
  * @param  Timeout: Specify Timeout value
  * @retval HAL status
  *         @arg @ref HAL_Status_Structure_Definitions
  */
HAL_StatusTypeDef HAL_CORDIC_Calculate(CORDIC_HandleTypeDef *hcordic, const int32_t *pInBuff, int32_t *pOutBuff,
                                       uint32_t NbCalc, uint32_t Timeout)
{
  uint32_t tickstart;
  uint32_t index;
  const int32_t *p_tmp_in_buff = pInBuff;
  int32_t *p_tmp_out_buff = pOutBuff;

  /* Check parameters setting */
  if ((pInBuff == NULL) || (pOutBuff == NULL) || (NbCalc == 0U))
  {
    /* Update the error code */
    hcordic->ErrorCode |= HAL_CORDIC_ERROR_PARAM;

    /* Return error status */
    return HAL_ERROR;
  }

  /* Check handle state is ready */
  if (hcordic->State == HAL_CORDIC_STATE_READY)
  {
    /* Reset CORDIC error code */
    hcordic->ErrorCode = HAL_CORDIC_ERROR_NONE;

    /* Change the CORDIC state */
    hcordic->State = HAL_CORDIC_STATE_BUSY;

    /* Get tick */
    tickstart = HAL_GetTick();

    /* Write of input data in Write Data register, and increment input buffer pointer */
    CORDIC_WriteInDataIncrementPtr(hcordic, &p_tmp_in_buff);

    /* Calculation is started.
       Provide next set of input data, until number of calculation is achieved */
    for (index = (NbCalc - 1U); index > 0U; index--)
    {
      /* Write of input data in Write Data register, and increment input buffer pointer */
      CORDIC_WriteInDataIncrementPtr(hcordic, &p_tmp_in_buff);

      /* Wait for RRDY flag to be raised */
      do
      {
        /* Check for the Timeout */
        if (Timeout != HAL_MAX_DELAY)
        {
          if ((HAL_GetTick() - tickstart) > Timeout)
          {
            /* Set CORDIC error code */
            hcordic->ErrorCode = HAL_CORDIC_ERROR_TIMEOUT;

            /* Change the CORDIC state */
            hcordic->State = HAL_CORDIC_STATE_READY;

            /* Return function status */
            return HAL_ERROR;
          }
        }
      } while (HAL_IS_BIT_CLR(hcordic->Instance->CSR, CORDIC_CSR_RRDY));

      /* Read output data from Read Data register, and increment output buffer pointer */
      CORDIC_ReadOutDataIncrementPtr(hcordic, &p_tmp_out_buff);
    }

    /* Read output data from Read Data register, and increment output buffer pointer */
    CORDIC_ReadOutDataIncrementPtr(hcordic, &p_tmp_out_buff);

    /* Change the CORDIC state */
    hcordic->State = HAL_CORDIC_STATE_READY;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Set CORDIC error code */
    hcordic->ErrorCode |= HAL_CORDIC_ERROR_NOT_READY;

    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Carry out data of CORDIC processing in Zero-Overhead mode (output data being read
  *         soon as input data are written), according to the existing CORDIC configuration.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module.
  *         @arg @ref CORDIC_HandleTypeDef
  * @param  pInBuff: Pointer to buffer containing input data for CORDIC processing.
  * @param  pOutBuff: Pointer to buffer where output data of CORDIC processing will be stored.
  * @param  NbCalc: Number of CORDIC calculation to process.
  * @param  Timeout: Specify Timeout value
  * @retval HAL status
  *         @arg @ref HAL_Status_Structure_Definitions
  */
HAL_StatusTypeDef HAL_CORDIC_CalculateZO(CORDIC_HandleTypeDef *hcordic, const int32_t *pInBuff, int32_t *pOutBuff,
                                         uint32_t NbCalc, uint32_t Timeout)
{
  uint32_t tickstart;
  uint32_t index;
  const int32_t *p_tmp_in_buff = pInBuff;
  int32_t *p_tmp_out_buff = pOutBuff;

  /* Check parameters setting */
  if ((pInBuff == NULL) || (pOutBuff == NULL) || (NbCalc == 0U))
  {
    /* Update the error code */
    hcordic->ErrorCode |= HAL_CORDIC_ERROR_PARAM;

    /* Return error status */
    return HAL_ERROR;
  }

  /* Check handle state is ready */
  if (hcordic->State == HAL_CORDIC_STATE_READY)
  {
    /* Reset CORDIC error code */
    hcordic->ErrorCode = HAL_CORDIC_ERROR_NONE;

    /* Change the CORDIC state */
    hcordic->State = HAL_CORDIC_STATE_BUSY;

    /* Get tick */
    tickstart = HAL_GetTick();

    /* Write of input data in Write Data register, and increment input buffer pointer */
    CORDIC_WriteInDataIncrementPtr(hcordic, &p_tmp_in_buff);

    /* Calculation is started.
       Provide next set of input data, until number of calculation is achieved */
    for (index = (NbCalc - 1U); index > 0U; index--)
    {
      /* Write of input data in Write Data register, and increment input buffer pointer */
      CORDIC_WriteInDataIncrementPtr(hcordic, &p_tmp_in_buff);

      /* Read output data from Read Data register, and increment output buffer pointer
         The reading is performed in Zero-Overhead mode:
         reading is ordered immediately without waiting result ready flag */
      CORDIC_ReadOutDataIncrementPtr(hcordic, &p_tmp_out_buff);

      /* Check for the Timeout */
      if (Timeout != HAL_MAX_DELAY)
      {
        if ((HAL_GetTick() - tickstart) > Timeout)
        {
          /* Set CORDIC error code */
          hcordic->ErrorCode = HAL_CORDIC_ERROR_TIMEOUT;

          /* Change the CORDIC state */
          hcordic->State = HAL_CORDIC_STATE_READY;

          /* Return function status */
          return HAL_ERROR;
        }
      }
    }

    /* Read output data from Read Data register, and increment output buffer pointer
       The reading is performed in Zero-Overhead mode:
       reading is ordered immediately without waiting result ready flag */
    CORDIC_ReadOutDataIncrementPtr(hcordic, &p_tmp_out_buff);

    /* Change the CORDIC state */
    hcordic->State = HAL_CORDIC_STATE_READY;

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Set CORDIC error code */
    hcordic->ErrorCode |= HAL_CORDIC_ERROR_NOT_READY;

    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Carry out data of CORDIC processing in interrupt mode,
  *         according to the existing CORDIC configuration.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module.
  *         @arg @ref CORDIC_HandleTypeDef
  * @param  pInBuff: Pointer to buffer containing input data for CORDIC processing.
  * @param  pOutBuff: Pointer to buffer where output data of CORDIC processing will be stored.
  * @param  NbCalc: Number of CORDIC calculation to process.
  * @retval HAL status
  *         @arg @ref HAL_Status_Structure_Definitions
  */
HAL_StatusTypeDef HAL_CORDIC_Calculate_IT(CORDIC_HandleTypeDef *hcordic, const int32_t *pInBuff, int32_t *pOutBuff,
                                          uint32_t NbCalc)
{
  const int32_t *tmp_pInBuff = pInBuff;

  /* Check parameters setting */
  if ((pInBuff == NULL) || (pOutBuff == NULL) || (NbCalc == 0U))
  {
    /* Update the error code */
    hcordic->ErrorCode |= HAL_CORDIC_ERROR_PARAM;

    /* Return error status */
    return HAL_ERROR;
  }

  /* Check handle state is ready */
  if (hcordic->State == HAL_CORDIC_STATE_READY)
  {
    /* Reset CORDIC error code */
    hcordic->ErrorCode = HAL_CORDIC_ERROR_NONE;

    /* Change the CORDIC state */
    hcordic->State = HAL_CORDIC_STATE_BUSY;

    /* Store the buffers addresses and number of calculations in handle,
       provisioning initial write of input data that will be done */
    if (HAL_IS_BIT_SET(hcordic->Instance->CSR, CORDIC_CSR_NARGS))
    {
      /* Two writes of input data are expected */
      tmp_pInBuff++;
      tmp_pInBuff++;
    }
    else
    {
      /* One write of input data is expected */
      tmp_pInBuff++;
    }
    hcordic->pInBuff = tmp_pInBuff;
    hcordic->pOutBuff = pOutBuff;
    hcordic->NbCalcToOrder = NbCalc - 1U;
    hcordic->NbCalcToGet = NbCalc;

    /* Enable Result Ready Interrupt */
    __HAL_CORDIC_ENABLE_IT(hcordic, CORDIC_IT_IEN);

    /* Set back pointer to start of input data buffer */
    tmp_pInBuff = pInBuff;

    /* Initiate the processing by providing input data
       in the Write Data register */
    WRITE_REG(hcordic->Instance->WDATA, (uint32_t)*tmp_pInBuff);

    /* Check if second write of input data is expected */
    if (HAL_IS_BIT_SET(hcordic->Instance->CSR, CORDIC_CSR_NARGS))
    {
      /* Increment pointer to input data */
      tmp_pInBuff++;

      /* Perform second write of input data */
      WRITE_REG(hcordic->Instance->WDATA, (uint32_t)*tmp_pInBuff);
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Set CORDIC error code */
    hcordic->ErrorCode |= HAL_CORDIC_ERROR_NOT_READY;

    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  Carry out input and/or output data of CORDIC processing in DMA mode,
  *         according to the existing CORDIC configuration.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module.
  *         @arg @ref CORDIC_HandleTypeDef
  * @param  pInBuff: Pointer to buffer containing input data for CORDIC processing.
  * @param  pOutBuff: Pointer to buffer where output data of CORDIC processing will be stored.
  * @param  NbCalc: Number of CORDIC calculation to process.
  * @param  DMADirection: Direction of DMA transfers.
  *         This parameter can be one of the following values:
  *         @arg @ref CORDIC_DMA_Direction CORDIC DMA direction
  * @note   pInBuff or pOutBuff is unused in case of unique DMADirection transfer, and can
  *         be set to NULL value in this case.
  * @note   pInBuff and pOutBuff buffers must be 32-bit aligned to ensure a correct
  *         DMA transfer to and from the Peripheral.
  * @retval HAL status
  *         @arg @ref HAL_Status_Structure_Definitions
  */
HAL_StatusTypeDef HAL_CORDIC_Calculate_DMA(CORDIC_HandleTypeDef *hcordic, const int32_t *pInBuff, int32_t *pOutBuff,
                                           uint32_t NbCalc, uint32_t DMADirection)
{
  uint32_t sizeinbuff;
  uint32_t sizeoutbuff;

  /* Check the parameters */
  assert_param(IS_CORDIC_DMA_DIRECTION(DMADirection));

  /* Check parameters setting */
  if (NbCalc == 0U)
  {
    /* Update the error code */
    hcordic->ErrorCode |= HAL_CORDIC_ERROR_PARAM;

    /* Return error status */
    return HAL_ERROR;
  }

  /* Check if CORDIC DMA direction "Out" is requested */
  if ((DMADirection == CORDIC_DMA_DIR_OUT) || (DMADirection == CORDIC_DMA_DIR_IN_OUT))
  {
    /* Check parameters setting */
    if (pOutBuff == NULL)
    {
      /* Update the error code */
      hcordic->ErrorCode |= HAL_CORDIC_ERROR_PARAM;

      /* Return error status */
      return HAL_ERROR;
    }
  }

  /* Check if CORDIC DMA direction "In" is requested */
  if ((DMADirection == CORDIC_DMA_DIR_IN) || (DMADirection == CORDIC_DMA_DIR_IN_OUT))
  {
    /* Check parameters setting */
    if (pInBuff == NULL)
    {
      /* Update the error code */
      hcordic->ErrorCode |= HAL_CORDIC_ERROR_PARAM;

      /* Return error status */
      return HAL_ERROR;
    }
  }

  if (hcordic->State == HAL_CORDIC_STATE_READY)
  {
    /* Reset CORDIC error code */
    hcordic->ErrorCode = HAL_CORDIC_ERROR_NONE;

    /* Change the CORDIC state */
    hcordic->State = HAL_CORDIC_STATE_BUSY;

    /* Get DMA direction */
    hcordic->DMADirection = DMADirection;

    /* Check if CORDIC DMA direction "Out" is requested */
    if ((DMADirection == CORDIC_DMA_DIR_OUT) || (DMADirection == CORDIC_DMA_DIR_IN_OUT))
    {
      /* Set the CORDIC DMA transfer complete callback */
      hcordic->hdmaOut->XferCpltCallback = CORDIC_DMAOutCplt;
      /* Set the DMA error callback */
      hcordic->hdmaOut->XferErrorCallback = CORDIC_DMAError;

      /* Check number of output data at each calculation,
         to retrieve the size of output data buffer */
      if (HAL_IS_BIT_SET(hcordic->Instance->CSR, CORDIC_CSR_NRES))
      {
        sizeoutbuff = 2U * NbCalc;
      }
      else
      {
        sizeoutbuff = NbCalc;
      }

      /* Enable the DMA stream managing CORDIC output data read */
      if (HAL_DMA_Start_IT(hcordic->hdmaOut, (uint32_t)&hcordic->Instance->RDATA, (uint32_t) pOutBuff, sizeoutbuff)
          != HAL_OK)
      {
        /* Update the error code */
        hcordic->ErrorCode |= HAL_CORDIC_ERROR_DMA;

        /* Return error status */
        return HAL_ERROR;
      }

      /* Enable output data Read DMA requests */
      SET_BIT(hcordic->Instance->CSR, CORDIC_DMA_REN);
    }

    /* Check if CORDIC DMA direction "In" is requested */
    if ((DMADirection == CORDIC_DMA_DIR_IN) || (DMADirection == CORDIC_DMA_DIR_IN_OUT))
    {
      /* Set the CORDIC DMA transfer complete callback */
      hcordic->hdmaIn->XferCpltCallback = CORDIC_DMAInCplt;
      /* Set the DMA error callback */
      hcordic->hdmaIn->XferErrorCallback = CORDIC_DMAError;

      /* Check number of input data expected for each calculation,
         to retrieve the size of input data buffer */
      if (HAL_IS_BIT_SET(hcordic->Instance->CSR, CORDIC_CSR_NARGS))
      {
        sizeinbuff = 2U * NbCalc;
      }
      else
      {
        sizeinbuff = NbCalc;
      }

      /* Enable the DMA stream managing CORDIC input data write */
      if (HAL_DMA_Start_IT(hcordic->hdmaIn, (uint32_t) pInBuff, (uint32_t)&hcordic->Instance->WDATA, sizeinbuff)
          != HAL_OK)
      {
        /* Update the error code */
        hcordic->ErrorCode |= HAL_CORDIC_ERROR_DMA;

        /* Return error status */
        return HAL_ERROR;
      }

      /* Enable input data Write DMA request */
      SET_BIT(hcordic->Instance->CSR, CORDIC_DMA_WEN);
    }

    /* Return function status */
    return HAL_OK;
  }
  else
  {
    /* Set CORDIC error code */
    hcordic->ErrorCode |= HAL_CORDIC_ERROR_NOT_READY;

    /* Return function status */
    return HAL_ERROR;
  }
}

/**
  * @brief  CORDIC error callback.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module
  *         @arg @ref CORDIC_HandleTypeDef
  * @retval None
  */
__weak void HAL_CORDIC_ErrorCallback(CORDIC_HandleTypeDef *hcordic)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcordic);

  /* NOTE : This function should not be modified; when the callback is needed,
            the HAL_CORDIC_ErrorCallback can be implemented in the user file
   */
}

/**
  * @brief  CORDIC calculate complete callback.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module
  *         @arg @ref CORDIC_HandleTypeDef
  * @retval None
  */
__weak void HAL_CORDIC_CalculateCpltCallback(CORDIC_HandleTypeDef *hcordic)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hcordic);

  /* NOTE : This function should not be modified; when the callback is needed,
            the HAL_CORDIC_CalculateCpltCallback can be implemented in the user file
   */
}

/**
  * @brief  Handle CORDIC interrupt request.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module
  *         @arg @ref CORDIC_HandleTypeDef
  * @retval None
  */
void HAL_CORDIC_IRQHandler(CORDIC_HandleTypeDef *hcordic)
{
  /* Check if calculation complete interrupt is enabled and if result ready
     flag is raised */
  if (__HAL_CORDIC_GET_IT_SOURCE(hcordic, CORDIC_IT_IEN) != 0U)
  {
    if (__HAL_CORDIC_GET_FLAG(hcordic, CORDIC_FLAG_RRDY) != 0U)
    {
      /* Decrement number of calculations to get */
      hcordic->NbCalcToGet--;

      /* Read output data from Read Data register, and increment output buffer pointer */
      CORDIC_ReadOutDataIncrementPtr(hcordic, &(hcordic->pOutBuff));

      /* Check if calculations are still to be ordered */
      if (hcordic->NbCalcToOrder > 0U)
      {
        /* Decrement number of calculations to order */
        hcordic->NbCalcToOrder--;

        /* Continue the processing by providing another write of input data
           in the Write Data register, and increment input buffer pointer */
        CORDIC_WriteInDataIncrementPtr(hcordic, &(hcordic->pInBuff));
      }

      /* Check if all calculations results are got */
      if (hcordic->NbCalcToGet == 0U)
      {
        /* Disable Result Ready Interrupt */
        __HAL_CORDIC_DISABLE_IT(hcordic, CORDIC_IT_IEN);

        /* Change the CORDIC state */
        hcordic->State = HAL_CORDIC_STATE_READY;

        /* Call calculation complete callback */
#if USE_HAL_CORDIC_REGISTER_CALLBACKS == 1
        /*Call registered callback*/
        hcordic->CalculateCpltCallback(hcordic);
#else
        /*Call legacy weak callback*/
        HAL_CORDIC_CalculateCpltCallback(hcordic);
#endif /* USE_HAL_CORDIC_REGISTER_CALLBACKS */
      }
    }
  }
}

/**
  * @brief  Return the CORDIC handle state.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module
  *         @arg @ref CORDIC_HandleTypeDef
  * @retval CORDIC state
  *         @arg @ref HAL_CORDIC_StateTypeDef
  */
HAL_CORDIC_StateTypeDef HAL_CORDIC_GetState(const CORDIC_HandleTypeDef *hcordic)
{
  /* Return CORDIC handle state */
  return hcordic->State;
}

/**
  * @brief  Return the CORDIC peripheral error.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module
  *         @arg @ref CORDIC_HandleTypeDef
  * @note   The returned error is a bit-map combination of possible errors
  * @retval Error bit-map
  *         @arg @ref CORDIC_Error_Code
  */
uint32_t HAL_CORDIC_GetError(const CORDIC_HandleTypeDef *hcordic)
{
  /* Return CORDIC error code */
  return hcordic->ErrorCode;
}

/**
  * @brief  Write input data for CORDIC processing, and increment input buffer pointer.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module.
  *         @arg @ref CORDIC_HandleTypeDef
  * @param  ppInBuff Pointer to pointer to input buffer.
  * @retval none
  */
static void CORDIC_WriteInDataIncrementPtr(const CORDIC_HandleTypeDef *hcordic, const int32_t **ppInBuff)
{
  /* First write of input data in the Write Data register */
  WRITE_REG(hcordic->Instance->WDATA, (uint32_t) **ppInBuff);

  /* Increment input data pointer */
  (*ppInBuff)++;

  /* Check if second write of input data is expected */
  if (HAL_IS_BIT_SET(hcordic->Instance->CSR, CORDIC_CSR_NARGS))
  {
    /* Second write of input data in the Write Data register */
    WRITE_REG(hcordic->Instance->WDATA, (uint32_t) **ppInBuff);

    /* Increment input data pointer */
    (*ppInBuff)++;
  }
}

/**
  * @brief  Read output data of CORDIC processing, and increment output buffer pointer.
  * @param  hcordic: pointer to a CORDIC_HandleTypeDef structure that contains
  *         the configuration information for CORDIC module.
  *         @arg @ref CORDIC_HandleTypeDef
  * @param  ppOutBuff Pointer to pointer to output buffer.
  * @retval none
  */
static void CORDIC_ReadOutDataIncrementPtr(const CORDIC_HandleTypeDef *hcordic, int32_t **ppOutBuff)
{
  /* First read of output data from the Read Data register */
  **ppOutBuff = (int32_t)READ_REG(hcordic->Instance->RDATA);

  /* Increment output data pointer */
  (*ppOutBuff)++;

  /* Check if second read of output data is expected */
  if (HAL_IS_BIT_SET(hcordic->Instance->CSR, CORDIC_CSR_NRES))
  {
    /* Second read of output data from the Read Data register */
    **ppOutBuff = (int32_t)READ_REG(hcordic->Instance->RDATA);

    /* Increment output data pointer */
    (*ppOutBuff)++;
  }
}

/**
  * @brief  DMA CORDIC Input Data process complete callback.
  * @param  hdma: DMA handle.
  *         @arg @ref DMA_HandleTypeDef
  * @retval None
  */
static void CORDIC_DMAInCplt(DMA_HandleTypeDef *hdma)
{
  CORDIC_HandleTypeDef *hcordic = (CORDIC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  /* Disable the DMA transfer for input request */
  CLEAR_BIT(hcordic->Instance->CSR, CORDIC_DMA_WEN);

  /* Check if DMA direction is CORDIC Input only (no DMA for CORDIC Output) */
  if (hcordic->DMADirection == CORDIC_DMA_DIR_IN)
  {
    /* Change the CORDIC DMA direction to none */
    hcordic->DMADirection = CORDIC_DMA_DIR_NONE;

    /* Change the CORDIC state to ready */
    hcordic->State = HAL_CORDIC_STATE_READY;

    /* Call calculation complete callback */
#if USE_HAL_CORDIC_REGISTER_CALLBACKS == 1
    /*Call registered callback*/
    hcordic->CalculateCpltCallback(hcordic);
#else
    /*Call legacy weak callback*/
    HAL_CORDIC_CalculateCpltCallback(hcordic);
#endif /* USE_HAL_CORDIC_REGISTER_CALLBACKS */
  }
}

/**
  * @brief  DMA CORDIC Output Data process complete callback.
  * @param  hdma: DMA handle.
  *         @arg @ref DMA_HandleTypeDef
  * @retval None
  */
static void CORDIC_DMAOutCplt(DMA_HandleTypeDef *hdma)
{
  CORDIC_HandleTypeDef *hcordic = (CORDIC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  /* Disable the DMA transfer for output request */
  CLEAR_BIT(hcordic->Instance->CSR, CORDIC_DMA_REN);

  /* Change the CORDIC DMA direction to none */
  hcordic->DMADirection = CORDIC_DMA_DIR_NONE;

  /* Change the CORDIC state to ready */
  hcordic->State = HAL_CORDIC_STATE_READY;

  /* Call calculation complete callback */
#if USE_HAL_CORDIC_REGISTER_CALLBACKS == 1
  /*Call registered callback*/
  hcordic->CalculateCpltCallback(hcordic);
#else
  /*Call legacy weak callback*/
  HAL_CORDIC_CalculateCpltCallback(hcordic);
#endif /* USE_HAL_CORDIC_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA CORDIC communication error callback.
  * @param  hdma: DMA handle.
  *         @arg @ref DMA_HandleTypeDef
  * @retval None
  */
static void CORDIC_DMAError(DMA_HandleTypeDef *hdma)
{
  CORDIC_HandleTypeDef *hcordic = (CORDIC_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  /* Set CORDIC handle state to error */
  hcordic->State = HAL_CORDIC_STATE_READY;

  /* Set CORDIC handle error code to DMA error */
  hcordic->ErrorCode |= HAL_CORDIC_ERROR_DMA;

  /* Call user callback */
#if USE_HAL_CORDIC_REGISTER_CALLBACKS == 1
  /*Call registered callback*/
  hcordic->ErrorCallback(hcordic);
#else
  /*Call legacy weak callback*/
  HAL_CORDIC_ErrorCallback(hcordic);
#endif /* USE_HAL_CORDIC_REGISTER_CALLBACKS */
}

/**
  * end of CORDIC_Function_Definitions @}
  */

#endif /* HAL_CORDIC_MODULE_ENABLED */

/**
  * end of CORDIC @}
  */
