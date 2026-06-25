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
*   File    : rx32g4xx_hal_iwdg.h
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _RX32G4xx_HAL_IWDG_H_
#define _RX32G4xx_HAL_IWDG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"

/** @defgroup IWDG
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/******************************************************************************/
/*                            IWDG Structures                                 */
/******************************************************************************/

/** @defgroup IWDG_Structure_Definitions IWDG Structure Definitions
  * @{
  */

/** @defgroup IWDG_InitTypeDef IWDG InitTypeDef
  * @ingroup  IWDG_Structure_Definitions
  * @{
  */

typedef struct
{
  uint32_t Prescaler;  /*!< Select the prescaler of the IWDG.
                            This parameter can be a value of @ref IWDG_Prescaler */

  uint32_t Reload;     /*!< Specifies the IWDG down-counter reload value.
                            This parameter must be a number between Min_Data = 0 and Max_Data = 0x0FFF */

} IWDG_InitTypeDef;

/**
  * end of IWDG_InitTypeDef @}
  */

/** @defgroup IWDG_HandleTypeDef IWDG HandleTypeDef
  * @ingroup  IWDG_Structure_Definitions
  * @{
  */

typedef struct
{
  IWDG_TypeDef                 *Instance;  /*!< Register base address    */

  IWDG_InitTypeDef             Init;       /*!< IWDG required parameters */
} IWDG_HandleTypeDef;

/**
  * end of IWDG_HandleTypeDef @}
  */

/**
  * end of IWDG_Structure_Definitions @}
  */

/* Exported constants --------------------------------------------------------*/

/******************************************************************************/
/*                            IWDG Parameters                                 */
/******************************************************************************/

/** @defgroup IWDG_Parameter_Definitions IWDG Parameter Definitions
  * @{
  */

/** @defgroup IWDG_Prescaler IWDG Prescaler
  * @ingroup  IWDG_Parameter_Definitions
  * @{
  */
#define IWDG_PRESCALER_4        0x00000000U                                              /*!< IWDG prescaler set to 4   */
#define IWDG_PRESCALER_8        (                                    IWDG_PR_PR_BIT0)    /*!< IWDG prescaler set to 8   */
#define IWDG_PRESCALER_16       (                  IWDG_PR_PR_BIT1                  )    /*!< IWDG prescaler set to 16  */
#define IWDG_PRESCALER_32       (                  IWDG_PR_PR_BIT1 | IWDG_PR_PR_BIT0)    /*!< IWDG prescaler set to 32  */
#define IWDG_PRESCALER_64       (IWDG_PR_PR_BIT2                                    )    /*!< IWDG prescaler set to 64  */
#define IWDG_PRESCALER_128      (IWDG_PR_PR_BIT2                   | IWDG_PR_PR_BIT0)    /*!< IWDG prescaler set to 128 */
#define IWDG_PRESCALER_256      (IWDG_PR_PR_BIT2 | IWDG_PR_PR_BIT1                  )    /*!< IWDG prescaler set to 256 */

/**
  * end of IWDG_Prescaler @}
  */

/** @defgroup IWDG_Key IWDG Key
  * @ingroup  IWDG_Parameter_Definitions
  * @{
  */

/* Private constants ---------------------------------------------------------*/

/**
  * @brief  IWDG Key Register BitMask
  */
#define IWDG_KEY_RELOAD                 0x0000AAAAu  /*!< IWDG Reload Counter Enable   */
#define IWDG_KEY_ENABLE                 0x0000CCCCu  /*!< IWDG Peripheral Enable       */
#define IWDG_KEY_WRITE_ACCESS_ENABLE    0x00005555u  /*!< IWDG KR Write Access Enable  */
#define IWDG_KEY_WRITE_ACCESS_DISABLE   0x00000000u  /*!< IWDG KR Write Access Disable */

/**
  * end of IWDG_Key @}
  */

/**
  * end of IWDG_Parameter_Definitions @}
  */

/* Exported macros -----------------------------------------------------------*/

/******************************************************************************/
/*                              IWDG Macro                                    */
/******************************************************************************/

/** @defgroup IWDG_Macro_Definitions IWDG Macro Definitions
  * @{
  */

/**
  * @brief  Enable the IWDG peripheral.
  * @param  __HANDLE__  IWDG handle
  * @retval None
  */
#define __HAL_IWDG_START(__HANDLE__)                WRITE_REG((__HANDLE__)->Instance->KR, IWDG_KEY_ENABLE)

/**
  * @brief  Reload IWDG counter with value defined in the reload register
  *         (write access to IWDG_PR and IWDG_RLR registers disabled).
  * @param  __HANDLE__  IWDG handle
  * @retval None
  */
#define __HAL_IWDG_RELOAD_COUNTER(__HANDLE__)       WRITE_REG((__HANDLE__)->Instance->KR, IWDG_KEY_RELOAD)


/* Private macros ------------------------------------------------------------*/

/** @defgroup IWDG_Private_Macros IWDG Private Macros
  * @{
  */

/**
  * @brief  Enable write access to IWDG_PR and IWDG_RLR registers.
  * @param  __HANDLE__  IWDG handle
  * @retval None
  */
#define IWDG_ENABLE_WRITE_ACCESS(__HANDLE__)  WRITE_REG((__HANDLE__)->Instance->KR, IWDG_KEY_WRITE_ACCESS_ENABLE)

/**
  * @brief  Disable write access to IWDG_PR and IWDG_RLR registers.
  * @param  __HANDLE__  IWDG handle
  * @retval None
  */
#define IWDG_DISABLE_WRITE_ACCESS(__HANDLE__) WRITE_REG((__HANDLE__)->Instance->KR, IWDG_KEY_WRITE_ACCESS_DISABLE)

/**
  * @brief  Check IWDG prescaler value.
  * @param  __PRESCALER__  IWDG prescaler value
  * @retval None
  */
#define IS_IWDG_PRESCALER(__PRESCALER__)      (((__PRESCALER__) == IWDG_PRESCALER_4)  || \
                                               ((__PRESCALER__) == IWDG_PRESCALER_8)  || \
                                               ((__PRESCALER__) == IWDG_PRESCALER_16) || \
                                               ((__PRESCALER__) == IWDG_PRESCALER_32) || \
                                               ((__PRESCALER__) == IWDG_PRESCALER_64) || \
                                               ((__PRESCALER__) == IWDG_PRESCALER_128)|| \
                                               ((__PRESCALER__) == IWDG_PRESCALER_256))

/**
  * @brief  Check IWDG reload value.
  * @param  __RELOAD__  IWDG reload value
  * @retval None
  */
#define IS_IWDG_RELOAD(__RELOAD__)            ((__RELOAD__) <= IWDG_RLR_RL)

/**
  * end of IWDG_Macro_Definitions @}
  */

/* Exported functions --------------------------------------------------------*/

/******************************************************************************/
/*                             IWDG Functions                             */
/******************************************************************************/

/** @defgroup IWDG_Function_Definitions IWDG Function Definitions
  * @{
  */

/* Initialization/Start functions  ********************************************/
HAL_StatusTypeDef HAL_IWDG_Init(IWDG_HandleTypeDef *hiwdg);

/* I/O operation functions ****************************************************/
HAL_StatusTypeDef HAL_IWDG_Refresh(IWDG_HandleTypeDef *hiwdg);

/**
  * end of IWDG_Function_Definitions @}
  */

/**
  * end of IWDG @}
  */

#ifdef __cplusplus
}
#endif

#endif /* _RX32G4xx_HAL_IWDG_H_ */
