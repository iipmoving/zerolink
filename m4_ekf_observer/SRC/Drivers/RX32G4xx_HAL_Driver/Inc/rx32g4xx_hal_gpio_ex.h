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
*   File    : rx32g4xx_hal_GPIOEx.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_TEMPLATE_H_
#define _RX32G4XX_HAL_TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"


/* Exported types ------------------------------------------------------------*/

/** @addtogroup GPIOEx
  * @{
  */

/******************************************************************************/
/*                            GPIOEx Structures                               */
/******************************************************************************/

/** @defgroup GPIOEx_Structure_Definitions GPIOEx Structure Definitions
  * @{
  */

/**
  * end of GPIOEx_Structure_Definitions @}
  */


/******************************************************************************/
/*                            GPIOEx Parameters                             */
/******************************************************************************/

/** @defgroup GPIOEx_Parameter_Definitions GPIOEx Parameter Definitions
  * @{
  */

/** @defgroup GPIOEx_Alternate_function_selection GPIOEx Alternate function selection
  * @ingroup  GPIOEx_Parameter_Definitions
  * @{
  */

/**
  * @brief  AF 0 selection
  */
#define GPIO_AF0_MCO           ((uint8_t)0x00)  /* MCO Alternate Function mapping                */
#define GPIO_AF0_SWJ           ((uint8_t)0x00)  /* SWJ (SWD and JTAG) Alternate Function mapping */
#define GPIO_AF0_TRACE         ((uint8_t)0x00)  /* TRACE Alternate Function mapping              */

/**
  * @brief  AF 1 selection
  */
#define GPIO_AF1_TIM2          ((uint8_t)0x01)  /* TIM2 Alternate Function mapping  */
#define GPIO_AF1_TIM5          ((uint8_t)0x01)  /* TIM5 Alternate Function mapping  */
#define GPIO_AF1_TIM16         ((uint8_t)0x01)  /* TIM16 Alternate Function mapping */
#define GPIO_AF1_TIM17         ((uint8_t)0x01)  /* TIM17 Alternate Function mapping */

/**
  * @brief  AF 2 selection
  */
#define GPIO_AF2_TIM1          ((uint8_t)0x02)  /* TIM1 Alternate Function mapping  */
#define GPIO_AF2_TIM2          ((uint8_t)0x02)  /* TIM2 Alternate Function mapping  */
#define GPIO_AF2_TIM3          ((uint8_t)0x02)  /* TIM3 Alternate Function mapping  */
#define GPIO_AF2_TIM4          ((uint8_t)0x02)  /* TIM4 Alternate Function mapping  */
#define GPIO_AF2_TIM5          ((uint8_t)0x02)  /* TIM5 Alternate Function mapping  */
#define GPIO_AF2_TIM8          ((uint8_t)0x02)  /* TIM8 Alternate Function mapping  */
#define GPIO_AF2_TIM15         ((uint8_t)0x02)  /* TIM15 Alternate Function mapping */

/**
  * @brief  AF 3 selection
  */
#define GPIO_AF3_TIM1          ((uint8_t)0x03)  /* TIM1 Alternate Function mapping   */
#define GPIO_AF3_TIM3          ((uint8_t)0x03)  /* TIM3 Alternate Function mapping   */
#define GPIO_AF3_TIM8          ((uint8_t)0x03)  /* TIM8 Alternate Function mapping   */
#define GPIO_AF3_HRTIM1        ((uint8_t)0x03)  /* HRTIM1 Alternate Function mapping */
#define GPIO_AF3_COMP3         ((uint8_t)0x03)  /* COMP3 Alternate Function mapping  */
#define GPIO_AF3_COMP4         ((uint8_t)0x03)  /* COMP4 Alternate Function mapping  */

/**
  * @brief  AF 4 selection
  */
#define GPIO_AF4_TIM1          ((uint8_t)0x04)  /* TIM1 Alternate Function mapping */
#define GPIO_AF4_TIM8          ((uint8_t)0x04)  /* TIM8 Alternate Function mapping */
#define GPIO_AF4_I2C1          ((uint8_t)0x04)  /* I2C1 Alternate Function mapping */
#define GPIO_AF4_I2C2          ((uint8_t)0x04)  /* I2C2 Alternate Function mapping */

/**
  * @brief  AF 5 selection
  */
#define GPIO_AF5_SPI1          ((uint8_t)0x05)  /* SPI1 Alternate Function mapping  */
#define GPIO_AF5_SPI2          ((uint8_t)0x05)  /* SPI2 Alternate Function mapping  */
#define GPIO_AF5_TIM1          ((uint8_t)0x05)  /* TIM1 Alternate Function mapping  */
#define GPIO_AF5_TIM8          ((uint8_t)0x05)  /* TIM8 Alternate Function mapping  */
#define GPIO_AF5_I2C2          ((uint8_t)0x05)  /* I2C2 Alternate Function mapping  */
#define GPIO_AF5_UART4         ((uint8_t)0x05)  /* UART4 Alternate Function mapping */
#define GPIO_AF5_UART5         ((uint8_t)0x05)  /* UART5 Alternate Function mapping */

/**
  * @brief  AF 6 selection
  */
#define GPIO_AF6_TIM1          ((uint8_t)0x06)  /* TIM1 Alternate Function mapping */
#define GPIO_AF6_TIM8          ((uint8_t)0x06)  /* TIM8 Alternate Function mapping */

/**
  * @brief  AF 7 selection
  */
#define GPIO_AF7_UART1        ((uint8_t)0x07)  /* UART1 Alternate Function mapping */
#define GPIO_AF7_UART2        ((uint8_t)0x07)  /* UART2 Alternate Function mapping */
#define GPIO_AF7_UART3        ((uint8_t)0x07)  /* UART3 Alternate Function mapping */

/**
  * @brief  AF 8 selection
  */
#define GPIO_AF8_COMP1         ((uint8_t)0x08)  /* COMP1 Alternate Function mapping */
#define GPIO_AF8_COMP2         ((uint8_t)0x08)  /* COMP2 Alternate Function mapping */
#define GPIO_AF8_COMP3         ((uint8_t)0x08)  /* COMP3 Alternate Function mapping */
#define GPIO_AF8_COMP4         ((uint8_t)0x08)  /* COMP4 Alternate Function mapping */
#define GPIO_AF8_TIM3          ((uint8_t)0x08)  /* TIM3 Alternate Function mapping  */
#define GPIO_AF8_TIM8          ((uint8_t)0x08)  /* TIM8 Alternate Function mapping  */
#define GPIO_AF8_UART4         ((uint8_t)0x08)  /* UART4 Alternate Function mapping */
#define GPIO_AF8_UART5         ((uint8_t)0x08)  /* UART5 Alternate Function mapping */

/**
  * @brief  AF 9 selection
  */
#define GPIO_AF9_TIM8          ((uint8_t)0x09)  /* TIM8 Alternate Function mapping  */
#define GPIO_AF9_TIM15         ((uint8_t)0x09)  /* TIM15 Alternate Function mapping */
#define GPIO_AF9_CAN1          ((uint8_t)0x09)  /* CAN1 Alternate Function mapping  */
#define GPIO_AF9_CAN2          ((uint8_t)0x09)  /* CAN2 Alternate Function mapping  */

/**
  * @brief  AF 10 selection
  */
#define GPIO_AF10_TIM2         ((uint8_t)0x0A)  /* TIM2 Alternate Function mapping  */
#define GPIO_AF10_TIM3         ((uint8_t)0x0A)  /* TIM3 Alternate Function mapping  */
#define GPIO_AF10_TIM4         ((uint8_t)0x0A)  /* TIM4 Alternate Function mapping  */
#define GPIO_AF10_TIM8         ((uint8_t)0x0A)  /* TIM8 Alternate Function mapping  */
#define GPIO_AF10_TIM17        ((uint8_t)0x0A)  /* TIM17 Alternate Function mapping */
#define GPIO_AF10_CLC          ((uint8_t)0x0A)  /* CLC Alternate Function mapping   */

/**
  * @brief  AF 11 selection
  */
#define GPIO_AF11_TIM1         ((uint8_t)0x0B)  /* TIM1 Alternate Function mapping  */
#define GPIO_AF11_TIM8         ((uint8_t)0x0B)  /* TIM8 Alternate Function mapping  */

/**
  * @brief  AF 12 selection
  */
#define GPIO_AF12_TIM1         ((uint8_t)0x0C)  /* TIM1 Alternate Function mapping   */
#define GPIO_AF12_TIM8         ((uint8_t)0x0C)  /* TIM8 Alternate Function mapping   */
#define GPIO_AF12_HRTIM1       ((uint8_t)0x0C)  /* HRTIM1 Alternate Function mapping */

/**
  * @brief  AF 13 selection
  */
#define GPIO_AF13_HRTIM1       ((uint8_t)0x0D)  /* HRTIM1 Alternate Function mapping */

/**
  * @brief  AF 14 selection
  */
#define GPIO_AF14_TIM2         ((uint8_t)0x0E)  /* TIM2 Alternate Function mapping  */
#define GPIO_AF14_UART4        ((uint8_t)0x0E)  /* UART4 Alternate Function mapping */
#define GPIO_AF14_UART5        ((uint8_t)0x0E)  /* UART5 Alternate Function mapping */

/**
  * @brief  AF 15 selection
  */
#define GPIO_AF15_EVENTOUT     ((uint8_t)0x0F)  /* EVENTOUT Alternate Function mapping */

#define IS_GPIO_AF(AF)         ((AF) <= (uint8_t)0x0F)

/**
  * end of GPIOEx_Alternate_function_selection @}
  */

/**
  * end of GPIOEx_Parameter_Definitions @}
  */


/******************************************************************************/
/*                              GPIOEx Macro                                  */
/******************************************************************************/

/** @defgroup GPIOEx_Macro_Definitions GPIOEx Macro Definitions
  * @{
  */

/** @defgroup GPIOEx_Get_Port_Index GPIOEx Get Port Index
  * @{
  */
#define GPIO_GET_INDEX(__GPIOx__)    (((__GPIOx__) == (GPIOA))? 0UL :\
                                      ((__GPIOx__) == (GPIOB))? 1UL :\
                                      ((__GPIOx__) == (GPIOC))? 2UL :\
                                      ((__GPIOx__) == (GPIOD))? 3UL :\
                                      ((__GPIOx__) == (GPIOE))? 4UL : 5UL)
/**
  * end of GPIOEx_Get_Port_Index @}
  */

/**
  * end of GPIOEx_Macro_Definitions @}
  */


/******************************************************************************/
/*                             GPIOEx Functions                             */
/******************************************************************************/

/** @defgroup GPIOEx_Function_Definitions GPIOEx Function Definitions
  * @{
  */

/**
  * end of GPIOEx_Function_Definitions @}
  */


/**
  * end of GPIOEx @}
  */


#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_TEMPLATE_H_ */
