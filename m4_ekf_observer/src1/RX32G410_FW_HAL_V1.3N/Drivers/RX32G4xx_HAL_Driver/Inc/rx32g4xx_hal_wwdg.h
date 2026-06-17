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
*   File    : rx32g4xx_hal_wwdg.h
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _RX3G4xx_HAL_WWDG_H_
#define _RX3G4xx_HAL_WWDG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"

/** @addtogroup WWDG
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup WWDG_Structure_Definitions WWDG Structure Definitions
  * @{
  */

/** @defgroup WWDG_InitTypeDef WWDG InitTypeDef
  * @ingroup  WWDG_Structure_Definitions
  * @{
  */
  
/**
  * @brief  WWDG Init structure definition
  */
typedef struct
{
  uint32_t Prescaler;     /*!< Specifies the prescaler value of the WWDG.
                               This parameter can be a value of @ref WWDG_Prescaler */

  uint32_t Window;        /*!< Specifies the WWDG window value to be compared to the downcounter.
                               This parameter must be a number Min_Data = 0x40 and Max_Data = 0x7F */

  uint32_t Counter;       /*!< Specifies the WWDG free-running downcounter  value.
                               This parameter must be a number between Min_Data = 0x40 and Max_Data = 0x7F */

  uint32_t EWIMode ;      /*!< Specifies if WWDG Early Wakeup Interrupt is enable or not.
                               This parameter can be a value of @ref WWDG_EWI_Mode */

} WWDG_InitTypeDef;

/**
  * end of WWDG_InitTypeDef @}
  */

/** @defgroup WWDG_HandleTypeDef WWDG HandleTypeDef
  * @ingroup  WWDG_Structure_Definitions
  * @{
  */

/**
  * @brief  WWDG handle Structure definition
  */
#if (USE_HAL_WWDG_REGISTER_CALLBACKS == 1)
typedef struct __WWDG_HandleTypeDef
#else
typedef struct
#endif /* USE_HAL_WWDG_REGISTER_CALLBACKS */
{
  WWDG_TypeDef      *Instance;  /*!< Register base address */

  WWDG_InitTypeDef  Init;       /*!< WWDG required parameters */

#if (USE_HAL_WWDG_REGISTER_CALLBACKS == 1)
  void (* EwiCallback)(struct __WWDG_HandleTypeDef *hwwdg);                  /*!< WWDG Early WakeUp Interrupt callback */

  void (* MspInitCallback)(struct __WWDG_HandleTypeDef *hwwdg);              /*!< WWDG Msp Init callback */
#endif /* USE_HAL_WWDG_REGISTER_CALLBACKS */
} WWDG_HandleTypeDef;

/**
  * end of WWDG_HandleTypeDef @}
  */

/** @defgroup HAL_WWDG_CallbackIDTypeDef HAL WWDG CallbackIDTypeDef
  * @ingroup  WWDG_Structure_Definitions
  * @{
  */

#if (USE_HAL_WWDG_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL WWDG common Callback ID enumeration definition
  */
typedef enum
{
  HAL_WWDG_EWI_CB_ID          = 0x00U,    /*!< WWDG EWI callback ID */
  HAL_WWDG_MSPINIT_CB_ID      = 0x01U,    /*!< WWDG MspInit callback ID */
} HAL_WWDG_CallbackIDTypeDef;

/**
  * end of HAL_WWDG_CallbackIDTypeDef @}
  */

/** @defgroup pWWDG_CallbackTypeDef pWWDG CallbackTypeDef
  * @ingroup  WWDG_Structure_Definitions
  * @{
  */

/**
  * @brief  HAL WWDG Callback pointer definition
  */
typedef void (*pWWDG_CallbackTypeDef)(WWDG_HandleTypeDef *hppp);  /*!< pointer to a WWDG common callback functions */

#endif /* USE_HAL_WWDG_REGISTER_CALLBACKS */

/**
  * end of pWWDG_CallbackTypeDef @}
  */

/* Exported constants --------------------------------------------------------*/

/******************************************************************************/
/*                               WWDG Parameters                              */
/******************************************************************************/

/** @defgroup WWDG_Parameter_Definitions WWDG Parameter Definitions
  * @{
  */

/** @defgroup WWDG_Interrupt_definition WWDG Interrupt definition
  * @ingroup  WWDG_Parameter_Definitions
  * @{
  */

#define WWDG_IT_EWI                         WWDG_CFR_EWI  /*!< Early wakeup interrupt */
/**
  * end of WWDG_Interrupt_definition @}
  */

/** @defgroup WWDG_Flag_definition WWDG Flag definition
  * @ingroup  WWDG_Parameter_Definitions
  * @{
  */
  
#define WWDG_FLAG_EWIF                      WWDG_SR_EWIF  /*!< Early wakeup interrupt flag */
/**
  * end of WWDG_Flag_definition @}
  */

/** @defgroup WWDG_Counter_Value WWDG Counter value
  * @{
  */
  
#define	WWDG_CR_T0                          WWDG_CR_T_BIT0
#define	WWDG_CR_T1                          WWDG_CR_T_BIT1
#define	WWDG_CR_T2                          WWDG_CR_T_BIT2
#define	WWDG_CR_T3                          WWDG_CR_T_BIT3
#define	WWDG_CR_T4                          WWDG_CR_T_BIT4
#define	WWDG_CR_T5                          WWDG_CR_T_BIT5
#define	WWDG_CR_T6                          WWDG_CR_T_BIT6

/**
  * end of WWDG_Counter_Value @}
  */

/** @defgroup WWDG_Time_Base WWDG Time base
  * @{
  */
  
#define	WWDG_CFR_WDGTB0                     WWDG_CFR_WDGTB_BIT0
#define	WWDG_CFR_WDGTB1                     WWDG_CFR_WDGTB_BIT1

/**
  * end of WWDG_Time_Base @}
  */


/** @defgroup WWDG_Window_Value WWDG Window value
  * @{
  */
  
#define	WWDG_CFR_W0                         WWDG_CFR_W_BIT0
#define	WWDG_CFR_W1                         WWDG_CFR_W_BIT1
#define	WWDG_CFR_W2                         WWDG_CFR_W_BIT2
#define	WWDG_CFR_W3                         WWDG_CFR_W_BIT3
#define	WWDG_CFR_W4                         WWDG_CFR_W_BIT4
#define	WWDG_CFR_W5                         WWDG_CFR_W_BIT5
#define	WWDG_CFR_W6                         WWDG_CFR_W_BIT6

/**
  * end of WWDG_Window_Value @}
  */

/** @defgroup WWDG_Prescaler WWDG Prescaler
  * @{
  */
  
#define WWDG_PRESCALER_1                   0x00000000u                                    /*!< WWDG counter clock = (PCLK1/4096)/1 */
#define WWDG_PRESCALER_2                   (                      WWDG_CFR_WDGTB_BIT0)    /*!< WWDG counter clock = (PCLK1/4096)/2 */
#define WWDG_PRESCALER_4                   (WWDG_CFR_WDGTB_BIT1                      )    /*!< WWDG counter clock = (PCLK1/4096)/4 */
#define WWDG_PRESCALER_8                   (WWDG_CFR_WDGTB_BIT1 | WWDG_CFR_WDGTB_BIT0)    /*!< WWDG counter clock = (PCLK1/4096)/8 */
/**
  * end of WWDG_Prescaler @}
  */

/** @defgroup WWDG_EWI_Mode WWDG Early Wakeup Interrupt Mode
  * @{
  */
  
#define WWDG_EWI_DISABLE                    0x00000000u       /*!< EWI Disable */
#define WWDG_EWI_ENABLE                     WWDG_CFR_EWI      /*!< EWI Enable */
/**
  * end of WWDG_EWI_Mode @}
  */

/**
  * end of WWDG_Parameter_Definitions @}
  */

/* Private macros ------------------------------------------------------------*/

/******************************************************************************/
/*                              WWDG Macro                                    */
/******************************************************************************/

/** @defgroup WWDG_Macro_Definitions WWDG Macro Definitions
  * @{
  */
  
/**
  * @brief  Check if the parameter __PRESCALER__ is valid
  * @param  __PRESCALER__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_WWDG_PRESCALER(__PRESCALER__)    (((__PRESCALER__) == WWDG_PRESCALER_1)  || \
                                             ((__PRESCALER__) == WWDG_PRESCALER_2)  || \
                                             ((__PRESCALER__) == WWDG_PRESCALER_4)  || \
                                             ((__PRESCALER__) == WWDG_PRESCALER_8))

/**
  * @brief  Check if the parameter __WINDOW__ is valid
  * @param  __WINDOW__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_WWDG_WINDOW(__WINDOW__)          (((__WINDOW__) >= WWDG_CFR_W6) && ((__WINDOW__) <= WWDG_CFR_W))

/**
  * @brief  Check if the parameter __COUNTER__ is valid
  * @param  __COUNTER__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_WWDG_COUNTER(__COUNTER__)        (((__COUNTER__) >= WWDG_CR_T6) && ((__COUNTER__) <= WWDG_CR_T))

/**
  * @brief  Check if the parameter __MODE__ is valid
  * @param  __MODE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_WWDG_EWI_MODE(__MODE__)          (((__MODE__) == WWDG_EWI_ENABLE) || \
                                             ((__MODE__) == WWDG_EWI_DISABLE))
/* Exported macros ------------------------------------------------------------*/

/**
  * @brief  Enable the WWDG peripheral.
  * @param  __HANDLE__  WWDG handle
  * @retval None
  */
#define __HAL_WWDG_ENABLE(__HANDLE__)                         SET_BIT((__HANDLE__)->Instance->CR, WWDG_CR_WDGA)

/**
  * @brief  Enable the WWDG early wakeup interrupt.
  * @param  __HANDLE__     WWDG handle
  * @param  __INTERRUPT__  specifies the interrupt to enable.
  *         This parameter can be one of the following values:
  *            @arg WWDG_IT_EWI: Early wakeup interrupt
  * @note   Once enabled this interrupt cannot be disabled except by a system reset.
  * @retval None
  */
#define __HAL_WWDG_ENABLE_IT(__HANDLE__, __INTERRUPT__)       SET_BIT((__HANDLE__)->Instance->CFR, (__INTERRUPT__))

/**
  * @brief  Check whether the selected WWDG interrupt has occurred or not.
  * @param  __HANDLE__  WWDG handle
  * @param  __INTERRUPT__  specifies the it to check.
  *        This parameter can be one of the following values:
  *            @arg WWDG_FLAG_EWIF: Early wakeup interrupt IT
  * @retval The new state of WWDG_FLAG (SET or RESET).
  */
#define __HAL_WWDG_GET_IT(__HANDLE__, __INTERRUPT__)        __HAL_WWDG_GET_FLAG((__HANDLE__),(__INTERRUPT__))

/** @brief  Clear the WWDG interrupt pending bits.
  *         bits to clear the selected interrupt pending bits.
  * @param  __HANDLE__  WWDG handle
  * @param  __INTERRUPT__  specifies the interrupt pending bit to clear.
  *         This parameter can be one of the following values:
  *            @arg WWDG_FLAG_EWIF: Early wakeup interrupt flag
  */
#define __HAL_WWDG_CLEAR_IT(__HANDLE__, __INTERRUPT__)      __HAL_WWDG_CLEAR_FLAG((__HANDLE__), (__INTERRUPT__))

/**
  * @brief  Check whether the specified WWDG flag is set or not.
  * @param  __HANDLE__  WWDG handle
  * @param  __FLAG__  specifies the flag to check.
  *         This parameter can be one of the following values:
  *            @arg WWDG_FLAG_EWIF: Early wakeup interrupt flag
  * @retval The new state of WWDG_FLAG (SET or RESET).
  */
#define __HAL_WWDG_GET_FLAG(__HANDLE__, __FLAG__)           (((__HANDLE__)->Instance->SR & (__FLAG__)) == (__FLAG__))

/**
  * @brief  Clear the WWDG's pending flags.
  * @param  __HANDLE__  WWDG handle
  * @param  __FLAG__  specifies the flag to clear.
  *         This parameter can be one of the following values:
  *            @arg WWDG_FLAG_EWIF: Early wakeup interrupt flag
  * @retval None
  */
#define __HAL_WWDG_CLEAR_FLAG(__HANDLE__, __FLAG__)         ((__HANDLE__)->Instance->SR = ~(__FLAG__))

/** @brief  Check whether the specified WWDG interrupt source is enabled or not.
  * @param  __HANDLE__  WWDG Handle.
  * @param  __INTERRUPT__  specifies the WWDG interrupt source to check.
  *         This parameter can be one of the following values:
  *            @arg WWDG_IT_EWI: Early Wakeup Interrupt
  * @retval state of __INTERRUPT__ (TRUE or FALSE).
  */
#define __HAL_WWDG_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->CFR\
                                                              & (__INTERRUPT__)) == (__INTERRUPT__))

/** 
  * end of WWDG_Macro_Definitions @}  
  */

/* Exported functions --------------------------------------------------------*/

/** @defgroup WWDG_Function_Definitions WWDG Function Definitions
  * @{
  */

/* Initialization/de-initialization functions  **********************************/
HAL_StatusTypeDef     HAL_WWDG_Init(WWDG_HandleTypeDef *hwwdg);
void                  HAL_WWDG_MspInit(WWDG_HandleTypeDef *hwwdg);
/* Callbacks Register/UnRegister functions  ***********************************/
#if (USE_HAL_WWDG_REGISTER_CALLBACKS == 1)
HAL_StatusTypeDef     HAL_WWDG_RegisterCallback(WWDG_HandleTypeDef *hwwdg, HAL_WWDG_CallbackIDTypeDef CallbackID,
                                                pWWDG_CallbackTypeDef pCallback);
HAL_StatusTypeDef     HAL_WWDG_UnRegisterCallback(WWDG_HandleTypeDef *hwwdg, HAL_WWDG_CallbackIDTypeDef CallbackID);
#endif /* USE_HAL_WWDG_REGISTER_CALLBACKS */

/* I/O operation functions ******************************************************/
HAL_StatusTypeDef     HAL_WWDG_Refresh(WWDG_HandleTypeDef *hwwdg);
void                  HAL_WWDG_IRQHandler(WWDG_HandleTypeDef *hwwdg);
void                  HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg);

/** 
  * end of WWDG_Function_Definitions @}  
  */
/** 
  * end of WWDG @}  
  */

#ifdef __cplusplus
}
#endif

#endif /* _RX3G4xx_HAL_WWDG_H_ */
