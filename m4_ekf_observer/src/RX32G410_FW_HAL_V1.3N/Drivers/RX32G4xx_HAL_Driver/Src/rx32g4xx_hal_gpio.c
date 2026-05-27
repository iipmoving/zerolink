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
*   File    : rx32g4xx_hal_gpio.c
*   By      : RX_DV_Team
**************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"


/** @defgroup GPIO GPIO
  * @{
  */

#ifdef HAL_GPIO_MODULE_ENABLED

/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** @addtogroup GPIO_Private_Constants GPIO Private Constants
  * @{
  */
#define GPIO_NUMBER           (16U)
/**
  * end of GPIO_Private_Constants @}
  */


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup GPIO_Function_Definitions GPIO Function Definitions
  * @{
  */

/**
  * @brief  Initialize the GPIOx peripheral according to the specified parameters in the GPIO_Init.
  * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral for RX32G4xx family
  * @param  GPIO_Init: pointer to a GPIO_InitTypeDef structure that contains
  *         the configuration information for the specified GPIO peripheral.
            @arg @ref GPIO_InitTypeDef
  * @retval None
  */
void HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
  uint32_t position = 0x00U;
  uint32_t iocurrent;
  uint32_t temp;

  /* Check the parameters */
  assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));
  assert_param(IS_GPIO_PIN(GPIO_Init->Pin));
  assert_param(IS_GPIO_MODE(GPIO_Init->Mode));

  /* Configure the port pins */
  while (((GPIO_Init->Pin) >> position) != 0U)
  {
    /* Get current io position */
    iocurrent = (GPIO_Init->Pin) & (1UL << position);

    if (iocurrent != 0x00u)
    {
      /*--------------------- GPIO Mode Configuration ------------------------*/
      /* In case of Output or Alternate function mode selection */
      if(((GPIO_Init->Mode & GPIO_MODE) == MODE_OUTPUT) ||
         ((GPIO_Init->Mode & GPIO_MODE) == MODE_AF))
      {
        /* Check the Speed parameter */
        assert_param(IS_GPIO_SPEED(GPIO_Init->Speed));
        /* Configure the IO Speed */
        temp = GPIOx->OSPEEDR;
        temp &= ~(GPIO_OSPEEDR << (position * 2U));
        temp |= (GPIO_Init->Speed << (position * 2U));
        GPIOx->OSPEEDR = temp;

        /* Configure the IO Output Type */
        temp = GPIOx->OTYPER;
        temp &= ~(GPIO_OTYPER << position) ;
        temp |= (((GPIO_Init->Mode & OUTPUT_TYPE) >> OUTPUT_TYPE_Pos) << position);
        GPIOx->OTYPER = temp;
      }

      if ((GPIO_Init->Mode & GPIO_MODE) != MODE_ANALOG)
      {
        /* Check the Pull parameter */
        assert_param(IS_GPIO_PULL(GPIO_Init->Pull));

        /* Activate the Pull-up or Pull down resistor for the current IO */
        temp = GPIOx->PUPDR;
        temp &= ~(GPIO_PUPDR << (position * 2U));
        temp |= ((GPIO_Init->Pull) << (position * 2U));
        GPIOx->PUPDR = temp;
      }

      /* In case of Alternate function mode selection */
      if ((GPIO_Init->Mode & GPIO_MODE) == MODE_AF)
      {
        /* Check the Alternate function parameters */
        assert_param(IS_GPIO_AF_INSTANCE(GPIOx));
        assert_param(IS_GPIO_AF(GPIO_Init->Alternate));

        /* Configure Alternate function mapped with the current IO */
        if ((position >> 3U) == 0)
        {
          temp = GPIOx->AFRL;
          temp &= ~(0xFU << ((position & 0x07U) * 4U));
          temp |= ((GPIO_Init->Alternate) << ((position & 0x07U) * 4U));
          GPIOx->AFRL = temp;
        }
        else
        {
          temp = GPIOx->AFRH;
          temp &= ~(0xFU << ((position & 0x07U) * 4U));
          temp |= ((GPIO_Init->Alternate) << ((position & 0x07U) * 4U));
          GPIOx->AFRH = temp;
        }
      }

      /* Configure IO Direction mode (Input, Output, Alternate or Analog) */
      temp = GPIOx->MODER;
      temp &= ~(GPIO_MODER << (position * 2U));
      temp |= ((GPIO_Init->Mode & GPIO_MODE) << (position * 2U));
      GPIOx->MODER = temp;

      /*--------------------- EXTI Mode Configuration ------------------------*/
      /* Configure the External Interrupt or event for the current IO */
      if ((GPIO_Init->Mode & EXTI_MODE) != 0x00u)
      {
        /* Enable SYSCFG Clock */
        __HAL_RCC_SYSCFG_CLK_ENABLE();

        temp = SYSCFG->EXTICR[position >> 2U];
        temp &= ~(0x0FUL << (4U * (position & 0x03U)));
        temp |= (GPIO_GET_INDEX(GPIOx) << (4U * (position & 0x03U)));
        SYSCFG->EXTICR[position >> 2U] = temp;

        /* Clear Rising Falling edge configuration */
        temp = EXTI->RTSR;
        temp &= ~(iocurrent);
        if ((GPIO_Init->Mode & TRIGGER_RISING) != 0x00U)
        {
          temp |= iocurrent;
        }
        EXTI->RTSR = temp;

        temp = EXTI->FTSR;
        temp &= ~(iocurrent);
        if ((GPIO_Init->Mode & TRIGGER_FALLING) != 0x00U)
        {
          temp |= iocurrent;
        }
        EXTI->FTSR = temp;

        temp = EXTI->EMR;
        temp &= ~(iocurrent);
        if ((GPIO_Init->Mode & EXTI_EVT) != 0x00U)
        {
          temp |= iocurrent;
        }
        EXTI->EMR = temp;

        /* Clear EXTI line configuration */
        temp = EXTI->IMR;
        temp &= ~(iocurrent);
        if ((GPIO_Init->Mode & EXTI_IT) != 0x00U)
        {
          temp |= iocurrent;
        }
        EXTI->IMR = temp;
      }
    }

    position++;
  }
}

/**
  * @brief  De-initialize the GPIOx peripheral registers to their default reset values.
  * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral for RX32G4xx family
  * @param  GPIO_Pin: specifies the port bit to be written.
  *         This parameter can be any combination of GPIO_PIN_x where x can be (0..15).
  *         @arg @ref GPIO_pins
  * @retval None
  */
void HAL_GPIO_DeInit(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin)
{
  uint32_t position = 0x00U;
  uint32_t iocurrent;
  uint32_t tmp;

  /* Check the parameters */
  assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));
  assert_param(IS_GPIO_PIN(GPIO_Pin));

  /* Configure the port pins */
  while ((GPIO_Pin >> position) != 0U)
  {
    /* Get current io position */
    iocurrent = (GPIO_Pin) & (1UL << position);

    if (iocurrent != 0x00u)
    {
      /*------------------------- EXTI Mode Configuration --------------------*/
      /* Clear the External Interrupt or Event for the current IO */
      tmp = SYSCFG->EXTICR[position >> 2U];
      tmp &= (0x0FUL << (4U * (position & 0x03U)));
      if (tmp == (GPIO_GET_INDEX(GPIOx) << (4U * (position & 0x03U))))
      {
        /* Clear EXTI line configuration */
        EXTI->IMR &= ~(iocurrent);
        EXTI->EMR &= ~(iocurrent);

        /* Clear Rising Falling edge configuration */
        EXTI->FTSR &= ~(iocurrent);
        EXTI->RTSR &= ~(iocurrent);

        tmp = 0x0FUL << (4U * (position & 0x03U));
        SYSCFG->EXTICR[position >> 2U] &= ~tmp;
      }

      /*------------------------- GPIO Mode Configuration --------------------*/
      /* Configure IO in default Mode */
      if ((GPIOx == GPIOA) && (position == 0x0D))
      {
        MODIFY_REG(GPIOx->MODER, (GPIO_MODER << (position * 2u)), (GPIO_MODER_AF << (position * 2u)));
      }
      else if ((GPIOx == GPIOA) && (position == 0x0E))
      {
        MODIFY_REG(GPIOx->MODER, (GPIO_MODER << (position * 2u)), (GPIO_MODER_AF << (position * 2u)));
      }
      else if ((GPIOx == GPIOA) && (position == 0x0F))
      {
        MODIFY_REG(GPIOx->MODER, (GPIO_MODER << (position * 2u)), (GPIO_MODER_AF << (position * 2u)));
      }
      else if ((GPIOx == GPIOB) && (position == 0x03))
      {
        MODIFY_REG(GPIOx->MODER, (GPIO_MODER << (position * 2u)), (GPIO_MODER_AF << (position * 2u)));
      }
      else if ((GPIOx == GPIOB) && (position == 0x04))
      {
        MODIFY_REG(GPIOx->MODER, (GPIO_MODER << (position * 2u)), (GPIO_MODER_AF << (position * 2u)));
      }
      else
      {
        MODIFY_REG(GPIOx->MODER, (GPIO_MODER << (position * 2u)), (GPIO_MODER_Analog << (position * 2u)));
      }

      /* Configure the default Alternate Function in current IO */
      if ((position >> 3U) == 0)
      {
        GPIOx->AFRL &= ~(0xFu << ((position & 0x07u) * 4u));
      }
      else
      {
        GPIOx->AFRH &= ~(0xFu << ((position & 0x07u) * 4u));
      }

      /* Deactivate the Pull-up and Pull-down resistor for the current IO */
      if ((GPIOx == GPIOA) && (position == 0x0D))
      {
        MODIFY_REG(GPIOx->PUPDR, (GPIO_PUPDR << (position * 2u)), (GPIO_PUPDR_Pullup << (position * 2u)));
      }
      else if ((GPIOx == GPIOA) && (position == 0x0E))
      {
        MODIFY_REG(GPIOx->PUPDR, (GPIO_PUPDR << (position * 2u)), (GPIO_PUPDR_Pulldown << (position * 2u)));
      }
      else if ((GPIOx == GPIOA) && (position == 0x0F))
      {
        MODIFY_REG(GPIOx->PUPDR, (GPIO_PUPDR << (position * 2u)), (GPIO_PUPDR_Pullup << (position * 2u)));
      }
      else if ((GPIOx == GPIOB) && (position == 0x04))
      {
        MODIFY_REG(GPIOx->PUPDR, (GPIO_PUPDR << (position * 2u)), (GPIO_PUPDR_Pullup << (position * 2u)));
      }
      else
      {
        MODIFY_REG(GPIOx->PUPDR, (GPIO_PUPDR << (position * 2u)), (GPIO_PUPDR_Floating << (position * 2u)));
      }

      /* Configure the default value IO Output Type */
      GPIOx->OTYPER  &= ~(GPIO_OTYPER << position);

      /* Configure the default value for IO Speed */
      if ((GPIOx == GPIOA) && (position == 0x0D))
      {
        MODIFY_REG(GPIOx->OSPEEDR, (GPIO_OSPEEDR << (position * 2u)), (GPIO_OSPEEDR_Very_High << (position * 2u)));
      }
      else if ((GPIOx == GPIOB) && (position == 0x03))
      {
        MODIFY_REG(GPIOx->OSPEEDR, (GPIO_OSPEEDR << (position * 2u)), (GPIO_OSPEEDR_Very_High << (position * 2u)));
      }
      else
      {
        MODIFY_REG(GPIOx->OSPEEDR, (GPIO_OSPEEDR << (position * 2u)), (GPIO_OSPEEDR_Low << (position * 2u)));
      }
    }

    position++;
  }
}

/**
  * @brief  Read the specified input port pin.
  * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral for RX32G4xx family
  * @param  GPIO_Pin: specifies the port bit to read.
  *         This parameter can be any combination of GPIO_PIN_x where x can be (0..15).
  *         @arg @ref GPIO_pins
  * @retval The input port pin value.
  *         @arg @ref GPIO_PinState
  */
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin)
{
  GPIO_PinState bitstatus;

  /* Check the parameters */
  assert_param(IS_GPIO_PIN(GPIO_Pin));

  if ((GPIOx->IDR & GPIO_Pin) != 0x00U)
  {
    bitstatus = GPIO_PIN_SET;
  }
  else
  {
    bitstatus = GPIO_PIN_RESET;
  }
  return bitstatus;
}

/**
  * @brief  Set or clear the selected data port bit.
  *
  * @note   This function uses GPIOx_BSRR and GPIOx_BRR registers to allow atomic read/modify
  *         accesses. In this way, there is no risk of an IRQ occurring between
  *         the read and the modify access.
  *
  * @param  GPIOx: where x can be (A..G) to select the GPIO peripheral for RX32G4xx family
  * @param  GPIO_Pin: specifies the port bit to be written.
  *         This parameter can be any combination of GPIO_PIN_x where x can be (0..15).
  *         @arg @ref GPIO_pins
  * @param  PinState: specifies the value to be written to the selected bit.
  *         This parameter can be one of the GPIO_PinState enum values:
  *         @arg @ref GPIO_PinState
  *         @arg GPIO_PIN_RESET: to clear the port pin
  *         @arg GPIO_PIN_SET: to set the port pin
  * @retval None
  */
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin, GPIO_PinState PinState)
{
  /* Check the parameters */
  assert_param(IS_GPIO_PIN(GPIO_Pin));
  assert_param(IS_GPIO_PIN_ACTION(PinState));

  if (PinState != GPIO_PIN_RESET)
  {
    GPIOx->BSRR = (uint32_t)GPIO_Pin;
  }
  else
  {
    GPIOx->BRR = (uint32_t)GPIO_Pin;
  }
}

/**
  * @brief  Toggle the specified GPIO pin.
  * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral for RX32G4xx family
  * @param  GPIO_Pin: specifies the pin to be toggled.
  *         This parameter can be any combination of GPIO_PIN_x where x can be (0..15).
  *         @arg @ref GPIO_pins
  * @retval None
  */
void HAL_GPIO_TogglePin(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin)
{
  uint32_t odr;

  /* Check the parameters */
  assert_param(IS_GPIO_PIN(GPIO_Pin));

  /* get current Output Data Register value */
  odr = GPIOx->ODR;

  /* Set selected pins that were at low level, and reset ones that were high */
  GPIOx->BSRR = ((odr & GPIO_Pin) << GPIO_NUMBER) | (~odr & GPIO_Pin);
}

/**
  * @brief  Lock GPIO Pins configuration registers.
  * @note   The locked registers are GPIOx_MODER, GPIOx_OTYPER, GPIOx_OSPEEDR,
  *         GPIOx_PUPDR, GPIOx_AFRL and GPIOx_AFRH.
  * @note   The configuration of the locked GPIO pins can no longer be modified
  *         until the next reset.
  * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral for RX32G4xx family
  * @param  GPIO_Pin: specifies the port bits to be locked.
  *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
  *         @arg @ref GPIO_pins
  * @retval None
  */
HAL_StatusTypeDef HAL_GPIO_LockPin(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin)
{
  __IO uint32_t tmp = GPIO_LCKR_LCKK;

  /* Check the parameters */
  assert_param(IS_GPIO_LOCK_INSTANCE(GPIOx));
  assert_param(IS_GPIO_PIN(GPIO_Pin));

  /* Apply lock key write sequence */
  tmp |= GPIO_Pin;
  /* Set LCKx bit(s): LCKK='1' + LCK[15-0] */
  GPIOx->LCKR = tmp;
  /* Reset LCKx bit(s): LCKK='0' + LCK[15-0] */
  GPIOx->LCKR = GPIO_Pin;
  /* Set LCKx bit(s): LCKK='1' + LCK[15-0] */
  GPIOx->LCKR = tmp;
  /* Read LCKK register. This read is mandatory to complete key lock sequence */
  tmp = GPIOx->LCKR;

  /* read again in order to confirm lock is active */
  if ((GPIOx->LCKR & GPIO_LCKR_LCKK) != 0x00u)
  {
    return HAL_OK;
  }
  else
  {
    return HAL_ERROR;
  }
}

/**
  * @brief  Handle EXTI interrupt request.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  *         @arg @ref GPIO_pins
  * @retval None
  */
void HAL_GPIO_EXTI_IRQHandler(uint32_t GPIO_Pin)
{
  /* EXTI line interrupt detected */
  if (__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != 0x00u)
  {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
    HAL_GPIO_EXTI_Callback(GPIO_Pin);
  }
}

/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
__weak void HAL_GPIO_EXTI_Callback(uint32_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_GPIO_EXTI_Callback could be implemented in the user file
   */
}

/**
  * end of GPIO_Function_Definitions @}
  */

#endif /* HAL_GPIO_MODULE_ENABLED */

/**
  * end of GPIO @}
  */
