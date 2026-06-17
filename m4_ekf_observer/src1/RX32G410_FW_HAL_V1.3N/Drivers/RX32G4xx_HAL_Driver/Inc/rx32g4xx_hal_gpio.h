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
*   File    : rx32g4xx_hal_gpio.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_GPIO_H_
#define _RX32G4XX_HAL_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"
#include "rx32g4xx_hal_gpio_ex.h"


/* Exported types ------------------------------------------------------------*/

/** @addtogroup GPIO
  * @{
  */

/******************************************************************************/
/*                            GPIO Structures                                 */
/******************************************************************************/

/** @defgroup GPIO_Structure_Definitions GPIO Structure Definitions
  * @{
  */

/** @defgroup GPIO_InitTypeDef GPIO InitTypeDef
  * @ingroup  GPIO_Structure_Definitions
  * @{
  */
/**
  * @brief  GPIO Init structure definition
  */
typedef struct
{
  uint32_t Pin;        /*!< Specifies the GPIO pins to be configured.
                           This parameter can be any value of @ref GPIO_pins */

  uint32_t Mode;       /*!< Specifies the operating mode for the selected pins.
                           This parameter can be a value of @ref GPIO_mode */

  uint32_t Pull;       /*!< Specifies the Pull-up or Pull-Down activation for the selected pins.
                           This parameter can be a value of @ref GPIO_pull */

  uint32_t Speed;      /*!< Specifies the speed for the selected pins.
                           This parameter can be a value of @ref GPIO_speed */

  uint32_t Alternate;  /*!< Peripheral to be connected to the selected pins
                            This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
} GPIO_InitTypeDef;
/**
  * end of GPIO_InitTypeDef @}
  */

/** @defgroup GPIO_PinState GPIO PinState
  * @ingroup  GPIO_Structure_Definitions
  * @{
  */
/**
  * @brief  GPIO Bit SET and Bit RESET enumeration
  */
typedef enum
{
  GPIO_PIN_RESET = 0U,
  GPIO_PIN_SET
} GPIO_PinState;
/**
  * end of GPIO_PinState @}
  */

/**
  * end of GPIO_Structure_Definitions @}
  */


/******************************************************************************/
/*                            GPIO Parameters                                 */
/******************************************************************************/

/** @defgroup GPIO_Parameter_Definitions GPIO Parameter Definitions
  * @{
  */

/** @defgroup GPIO_pins GPIO pins
  * @ingroup  GPIO_Parameter_Definitions
  * @{
  */
#define GPIO_PIN_0                 ((uint32_t)0x0001)  /* Pin 0 selected    */
#define GPIO_PIN_1                 ((uint32_t)0x0002)  /* Pin 1 selected    */
#define GPIO_PIN_2                 ((uint32_t)0x0004)  /* Pin 2 selected    */
#define GPIO_PIN_3                 ((uint32_t)0x0008)  /* Pin 3 selected    */
#define GPIO_PIN_4                 ((uint32_t)0x0010)  /* Pin 4 selected    */
#define GPIO_PIN_5                 ((uint32_t)0x0020)  /* Pin 5 selected    */
#define GPIO_PIN_6                 ((uint32_t)0x0040)  /* Pin 6 selected    */
#define GPIO_PIN_7                 ((uint32_t)0x0080)  /* Pin 7 selected    */
#define GPIO_PIN_8                 ((uint32_t)0x0100)  /* Pin 8 selected    */
#define GPIO_PIN_9                 ((uint32_t)0x0200)  /* Pin 9 selected    */
#define GPIO_PIN_10                ((uint32_t)0x0400)  /* Pin 10 selected   */
#define GPIO_PIN_11                ((uint32_t)0x0800)  /* Pin 11 selected   */
#define GPIO_PIN_12                ((uint32_t)0x1000)  /* Pin 12 selected   */
#define GPIO_PIN_13                ((uint32_t)0x2000)  /* Pin 13 selected   */
#define GPIO_PIN_14                ((uint32_t)0x4000)  /* Pin 14 selected   */
#define GPIO_PIN_15                ((uint32_t)0x8000)  /* Pin 15 selected   */
#define GPIO_PIN_All               ((uint32_t)0xFFFF)  /* All pins selected */

#define GPIO_PIN_MASK              (0x0000FFFFU)       /* PIN mask for assert test */
/**
  * end of GPIO_pins @}
  */

/** @defgroup GPIO_mode GPIO mode
  * @ingroup  GPIO_Parameter_Definitions
  * @{
  */
/**
  * @brief  GPIO Configuration Mode
  *         Elements values convention: 0x00WX00YZ
  *           - W  : EXTI trigger detection on 3 bits
  *           - X  : EXTI mode (IT or Event) on 2 bits
  *           - Y  : Output type (Push Pull or Open Drain) on 1 bit
  *           - Z  : GPIO mode (Input, Output, Alternate or Analog) on 2 bits
  */
#define GPIO_MODE_INPUT                        MODE_INPUT                                                  /*!< Input Floating Mode                   */
#define GPIO_MODE_OUTPUT_PP                    (MODE_OUTPUT | OUTPUT_PP)                                   /*!< Output Push Pull Mode                 */
#define GPIO_MODE_OUTPUT_OD                    (MODE_OUTPUT | OUTPUT_OD)                                   /*!< Output Open Drain Mode                */
#define GPIO_MODE_AF_PP                        (MODE_AF | OUTPUT_PP)                                       /*!< Alternate Function Push Pull Mode     */
#define GPIO_MODE_AF_OD                        (MODE_AF | OUTPUT_OD)                                       /*!< Alternate Function Open Drain Mode    */

#define GPIO_MODE_ANALOG                       MODE_ANALOG                                                 /*!< Analog Mode  */

#define GPIO_MODE_IT_RISING                    (MODE_INPUT | EXTI_IT | TRIGGER_RISING)                     /*!< External Interrupt Mode with Rising edge trigger detection          */
#define GPIO_MODE_IT_FALLING                   (MODE_INPUT | EXTI_IT | TRIGGER_FALLING)                    /*!< External Interrupt Mode with Falling edge trigger detection         */
#define GPIO_MODE_IT_RISING_FALLING            (MODE_INPUT | EXTI_IT | TRIGGER_RISING | TRIGGER_FALLING)   /*!< External Interrupt Mode with Rising/Falling edge trigger detection  */

#define GPIO_MODE_EVT_RISING                   (MODE_INPUT | EXTI_EVT | TRIGGER_RISING)                    /*!< External Event Mode with Rising edge trigger detection              */
#define GPIO_MODE_EVT_FALLING                  (MODE_INPUT | EXTI_EVT | TRIGGER_FALLING)                   /*!< External Event Mode with Falling edge trigger detection             */
#define GPIO_MODE_EVT_RISING_FALLING           (MODE_INPUT | EXTI_EVT | TRIGGER_RISING | TRIGGER_FALLING)  /*!< External Event Mode with Rising/Falling edge trigger detection      */
/**
  * end of GPIO_mode @}
  */

/** @defgroup GPIO_speed GPIO speed
  * @ingroup  GPIO_Parameter_Definitions
  * @{
  */
/**
  * @brief  GPIO Output Maximum frequency
  */
#define GPIO_SPEED_FREQ_LOW        (0x00000000U)   /*!< range up to 5 MHz, please refer to the product datasheet */
#define GPIO_SPEED_FREQ_MEDIUM     (0x00000001U)   /*!< range  5 MHz to 25 MHz, please refer to the product datasheet */
#define GPIO_SPEED_FREQ_HIGH       (0x00000002U)   /*!< range 25 MHz to 50 MHz, please refer to the product datasheet */
#define GPIO_SPEED_FREQ_VERY_HIGH  (0x00000003U)   /*!< range 50 MHz to 120 MHz, please refer to the product datasheet */
/**
  * end of GPIO_speed @}
  */

/** @defgroup GPIO_pull GPIO pull
  * @ingroup  GPIO_Parameter_Definitions
  * @{
  */
/**
  * @brief  GPIO Pull-Up or Pull-Down Activation
  */
#define GPIO_NOPULL        (0x00000000U)   /*!< No Pull-up or Pull-down activation  */
#define GPIO_PULLUP        (0x00000001U)   /*!< Pull-up activation                  */
#define GPIO_PULLDOWN      (0x00000002U)   /*!< Pull-down activation                */
/**
  * end of GPIO_pull @}
  */

/**
  * end of GPIO_Parameter_Definitions @}
  */


/******************************************************************************/
/*                              GPIO Macro                                    */
/******************************************************************************/

/** @defgroup GPIO_Macro_Definitions GPIO Macro Definitions
  * @{
  */

/**
  * @brief  Check whether the specified EXTI line flag is set or not.
  * @param  __EXTI_LINE__: specifies the EXTI line flag to check.
  *         This parameter can be GPIO_PIN_x where x can be(0..15)
  *         @arg @ref GPIO_pins
  * @retval The new state of __EXTI_LINE__ (SET or RESET).
  */
#define __HAL_GPIO_EXTI_GET_FLAG(__EXTI_LINE__)       (EXTI->PR & (__EXTI_LINE__))

/**
  * @brief  Clear the EXTI's line pending flags.
  * @param  __EXTI_LINE__: specifies the EXTI lines flags to clear.
  *         This parameter can be any combination of GPIO_PIN_x where x can be (0..15)
  *         @arg @ref GPIO_pins
  * @retval None
  */
#define __HAL_GPIO_EXTI_CLEAR_FLAG(__EXTI_LINE__)     (EXTI->PR = (__EXTI_LINE__))

/**
  * @brief  Check whether the specified EXTI line is asserted or not.
  * @param  __EXTI_LINE__: specifies the EXTI line to check.
  *          This parameter can be GPIO_PIN_x where x can be(0..15)
  *         @arg @ref GPIO_pins
  * @retval The new state of __EXTI_LINE__ (SET or RESET).
  */
#define __HAL_GPIO_EXTI_GET_IT(__EXTI_LINE__)         (EXTI->PR & (__EXTI_LINE__))

/**
  * @brief  Clear the EXTI's line pending bits.
  * @param  __EXTI_LINE__: specifies the EXTI lines to clear.
  *          This parameter can be any combination of GPIO_PIN_x where x can be (0..15)
  *         @arg @ref GPIO_pins
  * @retval None
  */
#define __HAL_GPIO_EXTI_CLEAR_IT(__EXTI_LINE__)       (EXTI->PR = (__EXTI_LINE__))

/**
  * @brief  Generate a Software interrupt on selected EXTI line.
  * @param  __EXTI_LINE__: specifies the EXTI line to check.
  *          This parameter can be GPIO_PIN_x where x can be(0..15)
  *         @arg @ref GPIO_pins
  * @retval None
  */
#define __HAL_GPIO_EXTI_GENERATE_SWIT(__EXTI_LINE__)  (EXTI->SWIER |= (__EXTI_LINE__))

/** @defgroup GPIO_Private_Constants GPIO Private Constants
  * @{
  */
#define GPIO_MODE_Pos                           0U
#define GPIO_MODE                               (0x3UL << GPIO_MODE_Pos)
#define MODE_INPUT                              (0x0UL << GPIO_MODE_Pos)
#define MODE_OUTPUT                             (0x1UL << GPIO_MODE_Pos)
#define MODE_AF                                 (0x2UL << GPIO_MODE_Pos)
#define MODE_ANALOG                             (0x3UL << GPIO_MODE_Pos)
#define OUTPUT_TYPE_Pos                         4U
#define OUTPUT_TYPE                             (0x1UL << OUTPUT_TYPE_Pos)
#define OUTPUT_PP                               (0x0UL << OUTPUT_TYPE_Pos)
#define OUTPUT_OD                               (0x1UL << OUTPUT_TYPE_Pos)
#define EXTI_MODE_Pos                           16U
#define EXTI_MODE                               (0x3UL << EXTI_MODE_Pos)
#define EXTI_IT                                 (0x1UL << EXTI_MODE_Pos)
#define EXTI_EVT                                (0x2UL << EXTI_MODE_Pos)
#define TRIGGER_MODE_Pos                        20U
#define TRIGGER_MODE                            (0x7UL << TRIGGER_MODE_Pos)
#define TRIGGER_RISING                          (0x1UL << TRIGGER_MODE_Pos)
#define TRIGGER_FALLING                         (0x2UL << TRIGGER_MODE_Pos)
/**
  * end of GPIO_Private_Constants @}
  */

/** @defgroup GPIO_Private_Macros GPIO Private Macros
  * @{
  */
#define IS_GPIO_PIN_ACTION(ACTION)  (((ACTION) == GPIO_PIN_RESET) || ((ACTION) == GPIO_PIN_SET))

#define IS_GPIO_PIN(__PIN__)        ((((uint32_t)(__PIN__) & GPIO_PIN_MASK) != 0x00U) &&\
                                     (((uint32_t)(__PIN__) & ~GPIO_PIN_MASK) == 0x00U))

#define IS_GPIO_MODE(__MODE__)      (((__MODE__) == GPIO_MODE_INPUT)              ||\
                                     ((__MODE__) == GPIO_MODE_OUTPUT_PP)          ||\
                                     ((__MODE__) == GPIO_MODE_OUTPUT_OD)          ||\
                                     ((__MODE__) == GPIO_MODE_AF_PP)              ||\
                                     ((__MODE__) == GPIO_MODE_AF_OD)              ||\
                                     ((__MODE__) == GPIO_MODE_IT_RISING)          ||\
                                     ((__MODE__) == GPIO_MODE_IT_FALLING)         ||\
                                     ((__MODE__) == GPIO_MODE_IT_RISING_FALLING)  ||\
                                     ((__MODE__) == GPIO_MODE_EVT_RISING)         ||\
                                     ((__MODE__) == GPIO_MODE_EVT_FALLING)        ||\
                                     ((__MODE__) == GPIO_MODE_EVT_RISING_FALLING) ||\
                                     ((__MODE__) == GPIO_MODE_ANALOG))

#define IS_GPIO_SPEED(__SPEED__)    (((__SPEED__) == GPIO_SPEED_FREQ_LOW)       ||\
                                     ((__SPEED__) == GPIO_SPEED_FREQ_MEDIUM)    ||\
                                     ((__SPEED__) == GPIO_SPEED_FREQ_HIGH)      ||\
                                     ((__SPEED__) == GPIO_SPEED_FREQ_VERY_HIGH))

#define IS_GPIO_PULL(__PULL__)      (((__PULL__) == GPIO_NOPULL)   ||\
                                     ((__PULL__) == GPIO_PULLUP)   || \
                                     ((__PULL__) == GPIO_PULLDOWN))
/**
  * end of GPIO_Private_Macros @}
  */

/**
  * end of GPIO_Macro_Definitions @}
  */


/******************************************************************************/
/*                             GPIO Functions                                 */
/******************************************************************************/

/** @defgroup GPIO_Function_Definitions GPIO Function Definitions
  * @{
  */

/* Initialization and de-initialization functions *****************************/
void              HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init);
void              HAL_GPIO_DeInit(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin);

/* IO operation functions *****************************************************/
GPIO_PinState     HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin);
void              HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin, GPIO_PinState PinState);
void              HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin);
HAL_StatusTypeDef HAL_GPIO_LockPin(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin);
void              HAL_GPIO_EXTI_IRQHandler(uint32_t GPIO_Pin);
void              HAL_GPIO_EXTI_Callback(uint32_t GPIO_Pin);

/**
  * end of GPIO_Function_Definitions @}
  */


/**
  * end of GPIO @}
  */


#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_GPIO_H_ */
