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
*   File    : system_bsp.c
*   By      : RX_DV_Team
**************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "system_bsp.h"
#include "rx32g4xx_hal.h"
#include "rx32g4xx_config_def.h"


/** @defgroup System_BSP System BSP
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

#if (SYS_BSP_BUTTON_EN == 1)
static T_HAL_BSP_PB_Callback g_fpSYS_BSP_PBCallback = NULL;
#endif  // end of #if (SYS_BSP_BUTTON_EN == 1)

/* Private function prototypes -----------------------------------------------*/

#if (SYS_BSP_LED_EN == 1) || (SYS_BSP_BUTTON_EN == 1)
static void HAL_SYS_BSP_GpioClockEnable(GPIO_TypeDef* GPIOx);
#endif  // end of #if (SYS_BSP_LED_EN == 1) || (SYS_BSP_BUTTON_EN == 1)

/* Exported functions --------------------------------------------------------*/

/** @addtogroup System_BSP_Function_Definitions System BSP Function Definitions
  * @{
  */

#if (SYS_BSP_LED_EN == 1)

/**
  * @brief  Configures LED GPIO.
  * @retval BSP status
  *         @arg SYS_BSP_STATUS_OK
  *         @arg SYS_BSP_STATUS_ERROR
  */
SYS_BSP_StatusTypeDef HAL_BSP_LED_Init(void)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable the GPIO LED Clock */
  HAL_SYS_BSP_GpioClockEnable(SYS_BSP_LED_GPIO_PORT);

  /* Configure the GPIO_LED pin */
  gpio_init_structure.Pin   = SYS_BSP_LED_PIN;
  gpio_init_structure.Mode  = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull  = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  HAL_GPIO_Init(SYS_BSP_LED_GPIO_PORT, &gpio_init_structure);
  HAL_GPIO_WritePin(SYS_BSP_LED_GPIO_PORT, SYS_BSP_LED_PIN, GPIO_PIN_RESET);

  return SYS_BSP_STATUS_OK;
}

/**
  * @brief  DeInit LEDs.
  * @note   Led DeInit does not disable the GPIO clock
  * @retval BSP status
  *         @arg SYS_BSP_STATUS_OK
  *         @arg SYS_BSP_STATUS_ERROR
  */
SYS_BSP_StatusTypeDef HAL_BSP_LED_DeInit(void)
{
  /* Turn off LED */
  HAL_GPIO_WritePin(SYS_BSP_LED_GPIO_PORT, SYS_BSP_LED_PIN, GPIO_PIN_RESET);

  /* DeInit the GPIO_LED pin */
  HAL_GPIO_DeInit(SYS_BSP_LED_GPIO_PORT, SYS_BSP_LED_PIN);

  return SYS_BSP_STATUS_OK;
}

/**
  * @brief  Turns selected LED On.
  * @retval BSP status
  *         @arg SYS_BSP_STATUS_OK
  *         @arg SYS_BSP_STATUS_ERROR
  */
SYS_BSP_StatusTypeDef HAL_BSP_LED_On(void)
{
  HAL_GPIO_WritePin(SYS_BSP_LED_GPIO_PORT, SYS_BSP_LED_PIN, GPIO_PIN_SET);

  return SYS_BSP_STATUS_OK;
}

/**
  * @brief  Turns selected LED Off.
  * @retval BSP status
  *         @arg SYS_BSP_STATUS_OK
  *         @arg SYS_BSP_STATUS_ERROR
  */
SYS_BSP_StatusTypeDef HAL_BSP_LED_Off(void)
{
  HAL_GPIO_WritePin(SYS_BSP_LED_GPIO_PORT, SYS_BSP_LED_PIN, GPIO_PIN_RESET);

  return SYS_BSP_STATUS_OK;
}

/**
  * @brief  Toggles the selected LED.
  * @retval BSP status
  *         @arg SYS_BSP_STATUS_OK
  *         @arg SYS_BSP_STATUS_ERROR
  */
SYS_BSP_StatusTypeDef HAL_BSP_LED_Toggle(void)
{
  HAL_GPIO_TogglePin(SYS_BSP_LED_GPIO_PORT, SYS_BSP_LED_PIN);

  return SYS_BSP_STATUS_OK;
}

/**
  * @brief  Get the status of the selected LED.
  * @retval LED status
  *         @arg GPIO_PIN_RESET
  *         @arg GPIO_PIN_SET
  */
int32_t HAL_BSP_LED_GetState(void)
{
  return (int32_t)HAL_GPIO_ReadPin(SYS_BSP_LED_GPIO_PORT, SYS_BSP_LED_PIN);
}

#endif  // end of #if (SYS_BSP_LED_EN == 1)

#if (SYS_BSP_BUTTON_EN == 1)

/**
  * @brief  Configures Button GPIO and EXTI Line.
  * @param  fpCallback: function pointer to be stored as callback.
  * @retval BSP status
  *         @arg SYS_BSP_STATUS_OK
  *         @arg SYS_BSP_STATUS_ERROR
  */
SYS_BSP_StatusTypeDef HAL_BSP_PB_Init(T_HAL_BSP_PB_Callback fpCallback)
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable the BUTTON Clock */
  HAL_SYS_BSP_GpioClockEnable(SYS_BSP_BUTTON_GPIO_PORT);

  /* Configure Button pin as input with External interrupt */
  gpio_init_structure.Pin = SYS_BSP_BUTTON_PIN;
  gpio_init_structure.Pull = GPIO_PULLUP;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio_init_structure.Mode = GPIO_MODE_IT_RISING;
  HAL_GPIO_Init(SYS_BSP_BUTTON_GPIO_PORT, &gpio_init_structure);

  g_fpSYS_BSP_PBCallback = fpCallback;

  /* Enable and set Button EXTI Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(SYS_BSP_BUTTON_EXTI_IRQn, SYS_BSP_BUTTON_EXTI_IRQ_PRIORITY, 0x00);
  HAL_NVIC_EnableIRQ(SYS_BSP_BUTTON_EXTI_IRQn);

  return SYS_BSP_STATUS_OK;
}

/**
  * @brief  Push Button DeInit.
  * @note   PB DeInit does not disable the GPIO clock
  * @retval BSP status
  *         @arg SYS_BSP_STATUS_OK
  *         @arg SYS_BSP_STATUS_ERROR
  */
SYS_BSP_StatusTypeDef HAL_BSP_PB_DeInit(void)
{
  HAL_NVIC_DisableIRQ(SYS_BSP_BUTTON_EXTI_IRQn);
  HAL_GPIO_DeInit(SYS_BSP_BUTTON_GPIO_PORT, SYS_BSP_BUTTON_PIN);

  return SYS_BSP_STATUS_OK;
}

/**
  * @brief  Returns the selected Button state.
  * @retval The Button GPIO pin value
  *         @arg GPIO_PIN_RESET
  *         @arg GPIO_PIN_SET
  */
int32_t HAL_BSP_PB_GetState(void)
{
  return (int32_t)HAL_GPIO_ReadPin(SYS_BSP_BUTTON_GPIO_PORT, SYS_BSP_BUTTON_PIN);
}

/**
  * @brief  This function handles Push-Button interrupt requests.
  * @retval None
  */
void SYS_BSP_BUTTON_EXTI_IRQ_HANDLER(void)
{
  __HAL_GPIO_EXTI_CLEAR_IT(SYS_BSP_BUTTON_PIN);
  if (g_fpSYS_BSP_PBCallback != NULL)
  {
    g_fpSYS_BSP_PBCallback(SYS_BSP_BUTTON_PIN);
  }
}

#endif  // end of #if (SYS_BSP_BUTTON_EN == 1)

#if (SYS_BSP_LED_EN == 1) || (SYS_BSP_BUTTON_EN == 1)

/**
  * @brief  Enable the GPIO Clock
  * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral for RX32G4xx family
  * @retval None
  */
static void HAL_SYS_BSP_GpioClockEnable(GPIO_TypeDef* GPIOx)
{
    if (GPIOx == GPIOA)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if (GPIOx == GPIOB)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if (GPIOx == GPIOC)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if (GPIOx == GPIOD)
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else if (GPIOx == GPIOE)
    {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
    else if (GPIOx == GPIOF)
    {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }
}

#endif  // end of #if (SYS_BSP_LED_EN == 1) || (SYS_BSP_BUTTON_EN == 1)

/**
  * end of System_BSP_Function_Definitions @}
  */


/**
  * end of System_BSP @}
  */
