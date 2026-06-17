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
*   File    : rx32g4xx_hal_rtc_ex.h
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RX32G4xx_HAL_RTC_EX_H__
#define __RX32G4xx_HAL_RTC_EX_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"

/** @addtogroup RTC
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** @addtogroup RTC_Structure_Definitions RTC Structure Definitions
  * @{
  */

/** @defgroup RTC_TamperTypeDef RTC TamperTypeDef
  * @ingroup  RTC_Structure_Definitions 
  * @{
  */

/**
  * @brief  RTC Tamper structure definition
  */
typedef struct
{
  uint32_t Tamper;                      /*!< Specifies the Tamper Pin.
                                             This parameter can be a value of @ref  RTC_Tamper_Pins_Definitions */

  uint32_t Trigger;                     /*!< Specifies the Tamper Trigger.
                                             This parameter can be a value of @ref  RTC_Tamper_Trigger_Definitions */

} RTC_TamperTypeDef;

/** 
  * end of RTC_TamperTypeDef @}  
  */
  
/** 
  * end of RTC_Structure_Definitions @}  
  */

/******************************************************************************/
/*                            RTC Parameters                                  */
/******************************************************************************/

/* Exported constants --------------------------------------------------------*/

/** @defgroup RTC_Parameter_Definitions RTC Parameters Definitions
  * @{
  */

/** @defgroup RTC_Tamper_Pins_Definitions RTC Tamper Pins Definitions
  * @ingroup  RTC_Parameter_Definitions 
  * @{
  */
#define RTC_TAMPER_1                        BKP_CR_TPE            /*!< Select tamper to be enabled (mainly for legacy purposes) */

/** 
  * end of RTC_Tamper_Pins_Definitions @}  
  */

/** @defgroup RTC_Tamper_Trigger_Definitions RTC Tamper Trigger Definitions
  * @ingroup  RTC_Parameter_Definitions 
  * @{
  */
#define RTC_TAMPERTRIGGER_LOWLEVEL          BKP_CR_TPAL           /*!< A high level on the TAMPER pin resets all data backup registers (if TPE bit is set) */
#define RTC_TAMPERTRIGGER_HIGHLEVEL         0x00000000U           /*!< A low level on the TAMPER pin resets all data backup registers (if TPE bit is set) */

/** 
  * end of RTC_Tamper_Trigger_Definitions @}  
  */

/** @defgroup RTC_Alias_For_Legacy RTC Alias define maintained for legacy
  * @ingroup  RTC_Parameter_Definitions 
  * @{
  */
#define HAL_RTCEx_TamperTimeStampIRQHandler   HAL_RTCEx_TamperIRQHandler

/** 
  * end of RTC_Alias_For_Legacy @}  
  */

/** 
  * end of RTC_Parameter_Definitions @}  
  */

/******************************************************************************/
/*                              RTC Macro                                     */
/******************************************************************************/

/* Exported macro ------------------------------------------------------------*/

/** @defgroup RTC_Macro_Definitions RTC Macro Definitions
  * @{
  */

/**
  * @brief  Enable the RTC Tamper interrupt.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Tamper interrupt sources to be enabled
  *         This parameter can be any combination of the following values:
  *           @arg RTC_IT_TAMP1: Tamper A interrupt
  * @retval None
  */
#define __HAL_RTC_TAMPER_ENABLE_IT(__HANDLE__, __INTERRUPT__) SET_BIT(BKP->CSR, (__INTERRUPT__))

/**
  * @brief  Disable the RTC Tamper interrupt.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Tamper interrupt sources to be disabled.
  *         This parameter can be any combination of the following values:
  *           @arg RTC_IT_TAMP1: Tamper A interrupt
  * @retval None
  */
#define __HAL_RTC_TAMPER_DISABLE_IT(__HANDLE__, __INTERRUPT__)  CLEAR_BIT(BKP->CSR, (__INTERRUPT__))

/**
  * @brief  Check whether the specified RTC Tamper interrupt has been enabled or not.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Tamper interrupt sources to be checked.
  *         This parameter can be:
  *           @arg  RTC_IT_TAMP1
  * @retval None
  */
#define __HAL_RTC_TAMPER_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)    ((((BKP->CSR) & ((__INTERRUPT__))) != RESET)? SET : RESET)

/**
  * @brief  Get the selected RTC Tamper's flag status.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __FLAG__: specifies the RTC Tamper Flag sources to be enabled or disabled.
  *         This parameter can be:
  *           @arg RTC_FLAG_TAMP1F
  * @retval None
  */
#define __HAL_RTC_TAMPER_GET_FLAG(__HANDLE__, __FLAG__)       ((((BKP->CSR) & (__FLAG__)) != RESET)? SET : RESET)

/**
  * @brief  Get the selected RTC Tamper's flag status.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Tamper interrupt sources to be checked.
  *         This parameter can be:
  *           @arg  RTC_IT_TAMP1
  * @retval None
  */
#define __HAL_RTC_TAMPER_GET_IT(__HANDLE__, __INTERRUPT__)       ((((BKP->CSR) & (BKP_CSR_TEF)) != RESET)? SET : RESET)

/**
  * @brief  Clear the RTC Tamper's pending flags.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __FLAG__: specifies the RTC Tamper Flag sources to be enabled or disabled.
  *         This parameter can be:
  *           @arg RTC_FLAG_TAMP1F
  * @retval None
  */
#define __HAL_RTC_TAMPER_CLEAR_FLAG(__HANDLE__, __FLAG__)     SET_BIT(BKP->CSR, BKP_CSR_CTE | BKP_CSR_CTI)

/**
  * @brief  Enable the RTC Second interrupt.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Second interrupt sources to be enabled
  *         This parameter can be any combination of the following values:
  *           @arg RTC_IT_SEC: Second A interrupt
  * @retval None
  */
#define __HAL_RTC_SECOND_ENABLE_IT(__HANDLE__, __INTERRUPT__)  SET_BIT((__HANDLE__)->Instance->CRH, (__INTERRUPT__))

/**
  * @brief  Disable the RTC Second interrupt.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Second interrupt sources to be disabled.
  *         This parameter can be any combination of the following values:
  *            @arg RTC_IT_SEC: Second A interrupt
  * @retval None
  */
#define __HAL_RTC_SECOND_DISABLE_IT(__HANDLE__, __INTERRUPT__) CLEAR_BIT((__HANDLE__)->Instance->CRH, (__INTERRUPT__))

/**
  * @brief  Check whether the specified RTC Second interrupt has occurred or not.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Second interrupt sources to be enabled or disabled.
  *         This parameter can be:
  *            @arg RTC_IT_SEC: Second A interrupt
  * @retval None
  */
#define __HAL_RTC_SECOND_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)      ((((((__HANDLE__)->Instance->CRH)& ((__INTERRUPT__)))) != RESET)? SET : RESET)

/**
  * @brief  Get the selected RTC Second's flag status.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __FLAG__: specifies the RTC Second Flag sources to be enabled or disabled.
  *         This parameter can be:
  *           @arg RTC_FLAG_SEC
  * @retval None
  */
#define __HAL_RTC_SECOND_GET_FLAG(__HANDLE__, __FLAG__)        (((((__HANDLE__)->Instance->CRL) & (__FLAG__)) != RESET)? SET : RESET)

/**
  * @brief  Clear the RTC Second's pending flags.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __FLAG__: specifies the RTC Second Flag sources to be enabled or disabled.
  *         This parameter can be:
  *           @arg RTC_FLAG_SEC
  * @retval None
  */
#define __HAL_RTC_SECOND_CLEAR_FLAG(__HANDLE__, __FLAG__)      ((__HANDLE__)->Instance->CRL) &= ~(__FLAG__)

/**
  * @brief  Enable the RTC Overflow interrupt.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Overflow interrupt sources to be enabled
  *         This parameter can be any combination of the following values:
  *           @arg RTC_IT_OW: Overflow A interrupt
  * @retval None
  */
#define __HAL_RTC_OVERFLOW_ENABLE_IT(__HANDLE__, __INTERRUPT__)  SET_BIT((__HANDLE__)->Instance->CRH, (__INTERRUPT__))

/**
  * @brief  Disable the RTC Overflow interrupt.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Overflow interrupt sources to be disabled.
  *         This parameter can be any combination of the following values:
  *           @arg RTC_IT_OW: Overflow A interrupt
  * @retval None
  */
#define __HAL_RTC_OVERFLOW_DISABLE_IT(__HANDLE__, __INTERRUPT__) CLEAR_BIT((__HANDLE__)->Instance->CRH, (__INTERRUPT__))

/**
  * @brief  Check whether the specified RTC Overflow interrupt has occurred or not.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Overflow interrupt sources to be enabled or disabled.
  *         This parameter can be:
  *           @arg RTC_IT_OW: Overflow A interrupt
  * @retval None
  */
#define __HAL_RTC_OVERFLOW_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)    ((((((__HANDLE__)->Instance->CRH)& ((__INTERRUPT__))) ) != RESET)? SET : RESET)

/**
  * @brief  Get the selected RTC Overflow's flag status.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __FLAG__: specifies the RTC Overflow Flag sources to be enabled or disabled.
  *         This parameter can be:
  *           @arg RTC_FLAG_OW
  * @retval None
  */
#define __HAL_RTC_OVERFLOW_GET_FLAG(__HANDLE__, __FLAG__)        (((((__HANDLE__)->Instance->CRL) & (__FLAG__)) != RESET)? SET : RESET)

/**
  * @brief  Clear the RTC Overflow's pending flags.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __FLAG__: specifies the RTC Overflow Flag sources to be enabled or disabled.
  *         This parameter can be:
  *           @arg RTC_FLAG_OW
  * @retval None
  */
#define __HAL_RTC_OVERFLOW_CLEAR_FLAG(__HANDLE__, __FLAG__)      ((__HANDLE__)->Instance->CRL) = ~(__FLAG__)

/**
  * @brief  Check if the parameter __TAMPER__ is valid
  * @param  __TAMPER__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RTC_TAMPER(__TAMPER__)            ((__TAMPER__) == RTC_TAMPER_1)

/**
  * @brief  Check if the parameter __TRIGGER__ is valid
  * @param  __TRIGGER__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RTC_TAMPER_TRIGGER(__TRIGGER__)   (((__TRIGGER__) == RTC_TAMPERTRIGGER_LOWLEVEL) || \
                                              ((__TRIGGER__) == RTC_TAMPERTRIGGER_HIGHLEVEL))

/**
  * @brief  Check if the parameter __VALUE__ is valid
  * @param  __VALUE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RTC_SMOOTH_CALIB_MINUS(__VALUE__) ((__VALUE__) <= 0x0000007FU)

/** 
  * end of RTC_Macro_Definitions @}  
  */

/******************************************************************************/
/*                             RTC Functions                                  */
/******************************************************************************/

/* Exported functions --------------------------------------------------------*/

/* RTC Tamper functions *****************************************/
HAL_StatusTypeDef HAL_RTCEx_SetTamper(RTC_HandleTypeDef *hrtc, RTC_TamperTypeDef *sTamper);
HAL_StatusTypeDef HAL_RTCEx_SetTamper_IT(RTC_HandleTypeDef *hrtc, RTC_TamperTypeDef *sTamper);
HAL_StatusTypeDef HAL_RTCEx_DeactivateTamper(RTC_HandleTypeDef *hrtc, uint32_t Tamper);
void              HAL_RTCEx_TamperIRQHandler(RTC_HandleTypeDef *hrtc);
void              HAL_RTCEx_Tamper1EventCallback(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_PollForTamper1Event(RTC_HandleTypeDef *hrtc, uint32_t Timeout);

/* RTC Second functions *****************************************/
HAL_StatusTypeDef HAL_RTCEx_SetSecond_IT(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTCEx_DeactivateSecond(RTC_HandleTypeDef *hrtc);
void              HAL_RTCEx_RTCIRQHandler(RTC_HandleTypeDef *hrtc);
void              HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc);
void              HAL_RTCEx_RTCEventErrorCallback(RTC_HandleTypeDef *hrtc);

/* Extension Control functions ************************************************/
HAL_StatusTypeDef HAL_RTCEx_SetSmoothCalib(RTC_HandleTypeDef *hrtc, uint32_t SmoothCalibPeriod, uint32_t SmoothCalibPlusPulses, uint32_t SmouthCalibMinusPulsesValue);

/** 
  * end of RTC_Macro_Definitions @}  
  */

/** 
  * end of RTC @}  
  */

#ifdef __cplusplus
}
#endif

#endif /* __RX32G4xx_HAL_RTC_EX_H__ */
