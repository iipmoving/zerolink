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
*   File    : rx32g4xx_hal_rtc_ex.c
*   By      : RX_DV_Team
**************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"

#ifdef HAL_RTC_MODULE_ENABLED

/** @defgroup RTC RTC
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup RTC_Function_Definitions RTC Function Definitions
  * @{
  */

/**
  * @brief  Sets Tamper
  * @note   By calling this API we disable the tamper interrupt for all tampers.
  * @param  hrtc : Pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @param  sTamper: Pointer to Tamper Structure.
  * @note   Tamper can be enabled only if ASOE and CCO bit are reset
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_RTCEx_SetTamper(RTC_HandleTypeDef *hrtc, RTC_TamperTypeDef *sTamper)
{
  /* Check input parameters */
  if ((hrtc == NULL) || (sTamper == NULL))
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_RTC_TAMPER(sTamper->Tamper));
  assert_param(IS_RTC_TAMPER_TRIGGER(sTamper->Trigger));

  /* Process Locked */
  __HAL_LOCK(hrtc);

  hrtc->State = HAL_RTC_STATE_BUSY;

  if (HAL_IS_BIT_SET(BKP->RTCCR, (BKP_RTCCR_CCO | BKP_RTCCR_ASOE)))
  {
    hrtc->State = HAL_RTC_STATE_ERROR;

    /* Process Unlocked */
    __HAL_UNLOCK(hrtc);

    return HAL_ERROR;
  }

  MODIFY_REG(BKP->CR, (BKP_CR_TPE | BKP_CR_TPAL), (sTamper->Tamper | (sTamper->Trigger)));

  hrtc->State = HAL_RTC_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hrtc);

  return HAL_OK;
}

/**
  * @brief  Sets Tamper with interrupt.
  * @note   By calling this API we force the tamper interrupt for all tampers.
  * @param  hrtc : pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @param  sTamper: Pointer to RTC Tamper.
  * @note   Tamper can be enabled only if ASOE and CCO bit are reset
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_RTCEx_SetTamper_IT(RTC_HandleTypeDef *hrtc, RTC_TamperTypeDef *sTamper)
{
  /* Check input parameters */
  if ((hrtc == NULL) || (sTamper == NULL))
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_RTC_TAMPER(sTamper->Tamper));
  assert_param(IS_RTC_TAMPER_TRIGGER(sTamper->Trigger));

  /* Process Locked */
  __HAL_LOCK(hrtc);

  hrtc->State = HAL_RTC_STATE_BUSY;

  if (HAL_IS_BIT_SET(BKP->RTCCR, (BKP_RTCCR_CCO | BKP_RTCCR_ASOE)))
  {
    hrtc->State = HAL_RTC_STATE_ERROR;

    /* Process Unlocked */
    __HAL_UNLOCK(hrtc);

    return HAL_ERROR;
  }

  MODIFY_REG(BKP->CR, (BKP_CR_TPE | BKP_CR_TPAL), (sTamper->Tamper | (sTamper->Trigger)));

  /* Configure the Tamper Interrupt in the BKP->CSR */
  __HAL_RTC_TAMPER_ENABLE_IT(hrtc, RTC_IT_TAMP1);

  hrtc->State = HAL_RTC_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hrtc);

  return HAL_OK;
}

/**
  * @brief  Deactivates Tamper.
  * @param  hrtc : pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @param  Tamper : Selected tamper pin.
  *           @ref RTCEx_Tamper_Pins_Definitions
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_RTCEx_DeactivateTamper(RTC_HandleTypeDef *hrtc, uint32_t Tamper)
{
  /* Check input parameters */
  if (hrtc == NULL)
  {
    return HAL_ERROR;
  }
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Tamper);

  assert_param(IS_RTC_TAMPER(Tamper));

  /* Process Locked */
  __HAL_LOCK(hrtc);

  hrtc->State = HAL_RTC_STATE_BUSY;

  /* Disable the selected Tamper pin */
  CLEAR_BIT(BKP->CR, BKP_CR_TPE);

  /* Disable the Tamper Interrupt in the BKP->CSR */
  /* Configure the Tamper Interrupt in the BKP->CSR */
  __HAL_RTC_TAMPER_DISABLE_IT(hrtc, RTC_IT_TAMP1);

  /* Clear the Tamper interrupt pending bit */
  __HAL_RTC_TAMPER_CLEAR_FLAG(hrtc, RTC_FLAG_TAMP1F);
  SET_BIT(BKP->CSR, BKP_CSR_CTE);

  hrtc->State = HAL_RTC_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hrtc);

  return HAL_OK;
}

/**
  * @brief  This function handles Tamper interrupt request.
  * @param  hrtc : pointer to a RTC_HandleTypeDef structure that contains
  *               the configuration information for RTC.
  * @retval None
  */
void HAL_RTCEx_TamperIRQHandler(RTC_HandleTypeDef *hrtc)
{
  /* Get the status of the Interrupt */
  if (__HAL_RTC_TAMPER_GET_IT_SOURCE(hrtc, RTC_IT_TAMP1))
  {
    /* Get the TAMPER Interrupt enable bit and pending bit */
    if (__HAL_RTC_TAMPER_GET_FLAG(hrtc, RTC_FLAG_TAMP1F) != (uint32_t)RESET)
    {
      /* Tamper callback */
#if (USE_HAL_RTC_REGISTER_CALLBACKS == 1)
      hrtc->Tamper1EventCallback(hrtc);
#else
      HAL_RTCEx_Tamper1EventCallback(hrtc);
#endif /* USE_HAL_RTC_REGISTER_CALLBACKS */

      /* Clear the Tamper interrupt pending bit */
      __HAL_RTC_TAMPER_CLEAR_FLAG(hrtc, RTC_FLAG_TAMP1F);
    }
  }

  /* Change RTC state */
  hrtc->State = HAL_RTC_STATE_READY;
}

/**
  * @brief  Tamper 1 callback.
  * @param  hrtc : pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @retval None
  */
__weak void HAL_RTCEx_Tamper1EventCallback(RTC_HandleTypeDef *hrtc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hrtc);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_RTCEx_Tamper1EventCallback could be implemented in the user file
   */
}

/**
  * @brief  This function handles Tamper1 Polling.
  * @param  hrtc : pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @param  Timeout: Timeout duration
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_RTCEx_PollForTamper1Event(RTC_HandleTypeDef *hrtc, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();

  /* Check input parameters */
  if (hrtc == NULL)
  {
    return HAL_ERROR;
  }

  /* Get the status of the Interrupt */
  while (__HAL_RTC_TAMPER_GET_FLAG(hrtc, RTC_FLAG_TAMP1F) == RESET)
  {
    if (Timeout != HAL_MAX_DELAY)
    {
      if ((Timeout == 0U) || ((HAL_GetTick() - tickstart) > Timeout))
      {
        hrtc->State = HAL_RTC_STATE_TIMEOUT;
        return HAL_TIMEOUT;
      }
    }
  }

  /* Clear the Tamper Flag */
  __HAL_RTC_TAMPER_CLEAR_FLAG(hrtc, RTC_FLAG_TAMP1F);

  /* Change RTC state */
  hrtc->State = HAL_RTC_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Sets Interrupt for second
  * @param  hrtc : pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_RTCEx_SetSecond_IT(RTC_HandleTypeDef *hrtc)
{
  /* Check input parameters */
  if (hrtc == NULL)
  {
    return HAL_ERROR;
  }

  /* Process Locked */
  __HAL_LOCK(hrtc);

  hrtc->State = HAL_RTC_STATE_BUSY;

  /* Enable Second interruption */
  __HAL_RTC_SECOND_ENABLE_IT(hrtc, RTC_IT_SEC);

  hrtc->State = HAL_RTC_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hrtc);

  return HAL_OK;
}

/**
  * @brief  Deactivates Second.
  * @param  hrtc : pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_RTCEx_DeactivateSecond(RTC_HandleTypeDef *hrtc)
{
  /* Check input parameters */
  if (hrtc == NULL)
  {
    return HAL_ERROR;
  }

  /* Process Locked */
  __HAL_LOCK(hrtc);


  hrtc->State = HAL_RTC_STATE_BUSY;

  /* Deactivate Second interruption*/
  __HAL_RTC_SECOND_DISABLE_IT(hrtc, RTC_IT_SEC);

  hrtc->State = HAL_RTC_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hrtc);

  return HAL_OK;
}

/**
  * @brief  This function handles second interrupt request.
  * @param  hrtc : pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @retval None
  */
void HAL_RTCEx_RTCIRQHandler(RTC_HandleTypeDef *hrtc)
{
  if (__HAL_RTC_SECOND_GET_IT_SOURCE(hrtc, RTC_IT_SEC))
  {
    /* Get the status of the Interrupt */
    if (__HAL_RTC_SECOND_GET_FLAG(hrtc, RTC_FLAG_SEC))
    {
      /* Check if Overrun occurred */
      if (__HAL_RTC_SECOND_GET_FLAG(hrtc, RTC_FLAG_OW))
      {
        /* Second error callback */
        HAL_RTCEx_RTCEventErrorCallback(hrtc);

        /* Clear flag Second */
        __HAL_RTC_OVERFLOW_CLEAR_FLAG(hrtc, RTC_FLAG_OW);

        /* Change RTC state */
        hrtc->State = HAL_RTC_STATE_ERROR;
      }
      else
      {
        /* Second callback */
        HAL_RTCEx_RTCEventCallback(hrtc);

        /* Change RTC state */
        hrtc->State = HAL_RTC_STATE_READY;
      }

      /* Clear flag Second */
      __HAL_RTC_SECOND_CLEAR_FLAG(hrtc, RTC_FLAG_SEC);
    }
  }
}

/**
  * @brief  Second event callback.
  * @param  hrtc : pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @retval None
  */
__weak void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hrtc);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_RTCEx_RTCEventCallback could be implemented in the user file
   */
}

/**
  * @brief  Second event error callback.
  * @param  hrtc : pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @retval None
  */
__weak void HAL_RTCEx_RTCEventErrorCallback(RTC_HandleTypeDef *hrtc)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hrtc);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_RTCEx_RTCEventErrorCallback could be implemented in the user file
   */
}

/**
  * @brief  Sets the Smooth calibration parameters.
  * @param  hrtc : RTC handle
  * @param  SmoothCalibPeriod: Not used (only present for compatibility with another families)
  * @param  SmoothCalibPlusPulses: Not used (only present for compatibility with another families)
  * @param  SmouthCalibMinusPulsesValue: specifies the RTC Clock Calibration value.
  *          This parameter must be a number between 0 and 0x7F.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_RTCEx_SetSmoothCalib(RTC_HandleTypeDef *hrtc, uint32_t SmoothCalibPeriod, uint32_t SmoothCalibPlusPulses, uint32_t SmouthCalibMinusPulsesValue)
{
  /* Check input parameters */
  if (hrtc == NULL)
  {
    return HAL_ERROR;
  }
  /* Prevent unused argument(s) compilation warning */
  UNUSED(SmoothCalibPeriod);
  UNUSED(SmoothCalibPlusPulses);

  /* Check the parameters */
  assert_param(IS_RTC_SMOOTH_CALIB_MINUS(SmouthCalibMinusPulsesValue));

  /* Process Locked */
  __HAL_LOCK(hrtc);

  hrtc->State = HAL_RTC_STATE_BUSY;

  /* Sets RTC Clock Calibration value.*/
  MODIFY_REG(BKP->RTCCR, BKP_RTCCR_CAL, SmouthCalibMinusPulsesValue);

  /* Change RTC state */
  hrtc->State = HAL_RTC_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hrtc);

  return HAL_OK;
}

/**
  * end of RTC_Function_Definitions @}
  */

#endif /* HAL_RTC_MODULE_ENABLED */

/**
  * end of RTC @}
  */
