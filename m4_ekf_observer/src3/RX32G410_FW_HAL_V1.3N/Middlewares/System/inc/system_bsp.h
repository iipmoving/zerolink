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
*   File    : system_bsp.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _SYSTEM_BSP_H_
#define _SYSTEM_BSP_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"


/* Exported types ------------------------------------------------------------*/

/** @addtogroup System_BSP
  * @{
  */

/******************************************************************************/
/*                            System_BSP Structures                           */
/******************************************************************************/

/** @defgroup System_BSP_Structure_Definitions System BSP Structure Definitions
  * @{
  */

/**
  * @brief  BSP Status type definition
  */
typedef enum
{
  SYS_BSP_STATUS_OK    = 0x00U,
  SYS_BSP_STATUS_ERROR = 0x01U
} SYS_BSP_StatusTypeDef;

typedef void (*T_HAL_BSP_PB_Callback)(uint32_t GPIO_Pin);

/**
  * end of System_BSP_Structure_Definitions @}
  */


/******************************************************************************/
/*                            System_BSP Parameters                           */
/******************************************************************************/

/** @defgroup System_BSP_Parameter_Definitions System BSP Parameter Definitions
  * @{
  */

#if !defined(SYS_BSP_LED_GPIO_PORT)
#define SYS_BSP_LED_GPIO_PORT                       GPIOA
#endif
#if !defined(SYS_BSP_LED_PIN)
#define SYS_BSP_LED_PIN                             GPIO_PIN_5
#endif

#if !defined(SYS_BSP_BUTTON_GPIO_PORT)
#define SYS_BSP_BUTTON_GPIO_PORT                    GPIOC
#endif
#if !defined(SYS_BSP_BUTTON_PIN)
#define SYS_BSP_BUTTON_PIN                          GPIO_PIN_1
#endif
#if !defined(SYS_BSP_BUTTON_EXTI_IRQn)
#define SYS_BSP_BUTTON_EXTI_IRQn                    EXTI1_IRQn
#endif
#if !defined(SYS_BSP_BUTTON_EXTI_IRQ_PRIORITY)
#define SYS_BSP_BUTTON_EXTI_IRQ_PRIORITY            IRQ_PRIORITY_EXTI1
#endif
#if !defined(SYS_BSP_BUTTON_EXTI_IRQ_HANDLER)
#define SYS_BSP_BUTTON_EXTI_IRQ_HANDLER             EXTI1_IRQHandler
#endif

/**
  * end of System_BSP_Parameter_Definitions @}
  */


/******************************************************************************/
/*                              System_BSP Macro                              */
/******************************************************************************/

/** @defgroup System_BSP_Macro_Definitions System BSP Macro Definitions
  * @{
  */

/**
  * end of System_BSP_Macro_Definitions @}
  */


/******************************************************************************/
/*                             System_BSP Functions                           */
/******************************************************************************/

/** @defgroup System_BSP_Function_Definitions System BSP Function Definitions
  * @{
  */

SYS_BSP_StatusTypeDef HAL_BSP_LED_Init(void);
SYS_BSP_StatusTypeDef HAL_BSP_LED_DeInit(void);
SYS_BSP_StatusTypeDef HAL_BSP_LED_On(void);
SYS_BSP_StatusTypeDef HAL_BSP_LED_Off(void);
SYS_BSP_StatusTypeDef HAL_BSP_LED_Toggle(void);
int32_t               HAL_BSP_LED_GetState(void);

SYS_BSP_StatusTypeDef HAL_BSP_PB_Init(T_HAL_BSP_PB_Callback fpCallback);
SYS_BSP_StatusTypeDef HAL_BSP_PB_DeInit(void);
int32_t               HAL_BSP_PB_GetState(void);

/**
  * end of System_BSP @}
  */


#ifdef __cplusplus
}
#endif

#endif /* _SYSTEM_BSP_H_ */
