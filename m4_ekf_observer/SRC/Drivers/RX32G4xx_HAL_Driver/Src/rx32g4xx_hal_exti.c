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
*   File    : rx32g4xx_hal_exti.c
*   By      : RX_DV_Team
**************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"


/** @defgroup EXTI EXTI
  * @{
  */

#ifdef HAL_EXTI_MODULE_ENABLED

/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define EXTI_MODE_OFFSET                    0x08U   /* 0x20: offset between MCU IMR/EMR registers */
#define EXTI_CONFIG_OFFSET                  0x08U   /* 0x20: offset between MCU Rising/Falling configuration registers */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup EXTI_Function_Definitions EXTI Function Definitions
  * @{
  */

/**
  * @brief  Set configuration of a dedicated Exti line.
  * @param  hexti: Exti handle.
  * @param  pExtiConfig: Pointer on EXTI configuration to be set.
  *         @arg @ref EXTI_ConfigTypeDef
  * @retval HAL Status.
  */
HAL_StatusTypeDef HAL_EXTI_SetConfigLine(EXTI_HandleTypeDef *hexti, EXTI_ConfigTypeDef *pExtiConfig)
{
  __IO uint32_t *regaddr;
  uint32_t regval;
  uint32_t linepos;
  uint32_t maskline;
  uint32_t offset;

  /* Check null pointer */
  if ((hexti == NULL) || (pExtiConfig == NULL))
  {
    return HAL_ERROR;
  }

  /* Check parameters */
  assert_param(IS_EXTI_LINE(pExtiConfig->Line));
  assert_param(IS_EXTI_MODE(pExtiConfig->Mode));

  /* Assign line number to handle */
  hexti->Line = pExtiConfig->Line;

  /* Compute line register offset */
  offset = ((pExtiConfig->Line & EXTI_REG_MASK) >> EXTI_REG_SHIFT);
  /* Compute line position */
  linepos = (pExtiConfig->Line & EXTI_PIN_MASK);
  /* Compute line mask */
  maskline = (1uL << linepos);

  /* Configure triggers for configurable lines */
  if ((pExtiConfig->Line & EXTI_CONFIG) != 0x00u)
  {
    assert_param(IS_EXTI_TRIGGER(pExtiConfig->Trigger));

    /* Configure rising trigger */
    regaddr = (&EXTI->RTSR + (EXTI_CONFIG_OFFSET * offset));
    regval = *regaddr;

    /* Mask or set line */
    if ((pExtiConfig->Trigger & EXTI_TRIGGER_RISING) != 0x00u)
    {
      regval |= maskline;
    }
    else
    {
      regval &= ~maskline;
    }

    /* Store rising trigger mode */
    *regaddr = regval;

    /* Configure falling trigger */
    regaddr = (&EXTI->FTSR + (EXTI_CONFIG_OFFSET * offset));
    regval = *regaddr;

    /* Mask or set line */
    if ((pExtiConfig->Trigger & EXTI_TRIGGER_FALLING) != 0x00u)
    {
      regval |= maskline;
    }
    else
    {
      regval &= ~maskline;
    }

    /* Store falling trigger mode */
    *regaddr = regval;

    /* Configure gpio port selection in case of gpio exti line */
    if ((pExtiConfig->Line & EXTI_GPIO) == EXTI_GPIO)
    {
      assert_param(IS_EXTI_GPIO_PORT(pExtiConfig->GPIOSel));
      assert_param(IS_EXTI_GPIO_PIN(linepos));

      regval = SYSCFG->EXTICR[linepos >> 2u];
      regval &= ~(SYSCFG_EXTICR1_EXTI0 << (SYSCFG_EXTICR1_EXTI1_Pos * (linepos & 0x03u)));
      regval |= (pExtiConfig->GPIOSel << (SYSCFG_EXTICR1_EXTI1_Pos * (linepos & 0x03u)));
      SYSCFG->EXTICR[linepos >> 2u] = regval;
    }
  }

  /* Configure interrupt mode : read current mode */
  regaddr = (&EXTI->IMR + (EXTI_MODE_OFFSET * offset));
  regval = *regaddr;

  /* Mask or set line */
  if ((pExtiConfig->Mode & EXTI_MODE_INTERRUPT) != 0x00u)
  {
    regval |= maskline;
  }
  else
  {
    regval &= ~maskline;
  }

  /* Store interrupt mode */
  *regaddr = regval;

  /* Configure event mode : read current mode */
  regaddr = (&EXTI->EMR + (EXTI_MODE_OFFSET * offset));
  regval = *regaddr;

  /* Mask or set line */
  if ((pExtiConfig->Mode & EXTI_MODE_EVENT) != 0x00u)
  {
    regval |= maskline;
  }
  else
  {
    regval &= ~maskline;
  }

  /* Store event mode */
  *regaddr = regval;

  return HAL_OK;
}


/**
  * @brief  Get configuration of a dedicated Exti line.
  * @param  hexti: Exti handle.
  * @param  pExtiConfig: Pointer on structure to store Exti configuration.
  *         @arg @ref EXTI_ConfigTypeDef
  * @retval HAL Status.
  */
HAL_StatusTypeDef HAL_EXTI_GetConfigLine(EXTI_HandleTypeDef *hexti, EXTI_ConfigTypeDef *pExtiConfig)
{
  __IO uint32_t *regaddr;
  uint32_t regval;
  uint32_t linepos;
  uint32_t maskline;
  uint32_t offset;

  /* Check null pointer */
  if ((hexti == NULL) || (pExtiConfig == NULL))
  {
    return HAL_ERROR;
  }

  /* Check the parameter */
  assert_param(IS_EXTI_LINE(hexti->Line));

  /* Store handle line number to configuration structure */
  pExtiConfig->Line = hexti->Line;

  /* Compute line register offset and line mask */
  offset = ((pExtiConfig->Line & EXTI_REG_MASK) >> EXTI_REG_SHIFT);
  /* Compute line position */
  linepos = (pExtiConfig->Line & EXTI_PIN_MASK);
  /* Compute mask */
  maskline = (1uL << linepos);

  /* 1] Get core mode : interrupt */
  regaddr = (&EXTI->IMR + (EXTI_MODE_OFFSET * offset));
  regval = *regaddr;

  /* Check if selected line is enable */
  if ((regval & maskline) != 0x00u)
  {
    pExtiConfig->Mode = EXTI_MODE_INTERRUPT;
  }
  else
  {
    pExtiConfig->Mode = EXTI_MODE_NONE;
  }

  /* Get event mode */
  regaddr = (&EXTI->EMR + (EXTI_MODE_OFFSET * offset));
  regval = *regaddr;

  /* Check if selected line is enable */
  if ((regval & maskline) != 0x00u)
  {
    pExtiConfig->Mode |= EXTI_MODE_EVENT;
  }

  /* Get default Trigger and GPIOSel configuration */
  pExtiConfig->Trigger = EXTI_TRIGGER_NONE;
  pExtiConfig->GPIOSel = 0x00u;

  /* 2] Get trigger for configurable lines : rising */
  if ((pExtiConfig->Line & EXTI_CONFIG) != 0x00u)
  {
    regaddr = (&EXTI->RTSR + (EXTI_CONFIG_OFFSET * offset));
    regval = *regaddr;

    /* Check if configuration of selected line is enable */
    if ((regval & maskline) != 0x00u)
    {
      pExtiConfig->Trigger = EXTI_TRIGGER_RISING;
    }

    /* Get falling configuration */
    regaddr = (&EXTI->FTSR + (EXTI_CONFIG_OFFSET * offset));
    regval = *regaddr;

    /* Check if configuration of selected line is enable */
    if ((regval & maskline) != 0x00u)
    {
      pExtiConfig->Trigger |= EXTI_TRIGGER_FALLING;
    }

    /* Get Gpio port selection for gpio lines */
    if ((pExtiConfig->Line & EXTI_GPIO) == EXTI_GPIO)
    {
      assert_param(IS_EXTI_GPIO_PIN(linepos));

      regval = SYSCFG->EXTICR[linepos >> 2u];
      pExtiConfig->GPIOSel = (regval >> (SYSCFG_EXTICR1_EXTI1_Pos * (linepos & 0x03u))) & SYSCFG_EXTICR1_EXTI0;
    }
  }

  return HAL_OK;
}


/**
  * @brief  Clear whole configuration of a dedicated Exti line.
  * @param  hexti: Exti handle.
  * @retval HAL Status.
  */
HAL_StatusTypeDef HAL_EXTI_ClearConfigLine(EXTI_HandleTypeDef *hexti)
{
  __IO uint32_t *regaddr;
  uint32_t regval;
  uint32_t linepos;
  uint32_t maskline;
  uint32_t offset;

  /* Check null pointer */
  if (hexti == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameter */
  assert_param(IS_EXTI_LINE(hexti->Line));

  /* compute line register offset and line mask */
  offset = ((hexti->Line & EXTI_REG_MASK) >> EXTI_REG_SHIFT);
  /* compute line position */
  linepos = (hexti->Line & EXTI_PIN_MASK);
  /* compute line mask */
  maskline = (1uL << linepos);

  /* 1] Clear interrupt mode */
  regaddr = (&EXTI->IMR + (EXTI_MODE_OFFSET * offset));
  regval = (*regaddr & ~maskline);
  *regaddr = regval;

  /* 2] Clear event mode */
  regaddr = (&EXTI->EMR + (EXTI_MODE_OFFSET * offset));
  regval = (*regaddr & ~maskline);
  *regaddr = regval;

  /* 3] Clear triggers in case of configurable lines */
  if ((hexti->Line & EXTI_CONFIG) != 0x00u)
  {
    regaddr = (&EXTI->RTSR + (EXTI_CONFIG_OFFSET * offset));
    regval = (*regaddr & ~maskline);
    *regaddr = regval;

    regaddr = (&EXTI->FTSR + (EXTI_CONFIG_OFFSET * offset));
    regval = (*regaddr & ~maskline);
    *regaddr = regval;

    /* Get Gpio port selection for gpio lines */
    if ((hexti->Line & EXTI_GPIO) == EXTI_GPIO)
    {
      assert_param(IS_EXTI_GPIO_PIN(linepos));

      regval = SYSCFG->EXTICR[linepos >> 2u];
      regval &= ~(SYSCFG_EXTICR1_EXTI0 << (SYSCFG_EXTICR1_EXTI1_Pos * (linepos & 0x03u)));
      SYSCFG->EXTICR[linepos >> 2u] = regval;
    }
  }

  return HAL_OK;
}


/**
  * @brief  Register callback for a dedicated Exti line.
  * @param  hexti: Exti handle.
  * @param  CallbackID: User callback identifier.
  *         This parameter can be one of @arg @ref EXTI_CallbackIDTypeDef values.
  *         @arg HAL_EXTI_COMMON_CB_ID
  * @param  pPendingCbfn: function pointer to be stored as callback.
  * @retval HAL Status.
  */
HAL_StatusTypeDef HAL_EXTI_RegisterCallback(EXTI_HandleTypeDef *hexti, EXTI_CallbackIDTypeDef CallbackID, void (*pPendingCbfn)(void))
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Check the parameters */
  assert_param(IS_EXTI_CB(CallbackID));

  switch (CallbackID)
  {
    /* set common callback */
    case  HAL_EXTI_COMMON_CB_ID:
      hexti->PendingCallback = pPendingCbfn;
      break;

    default:
      hexti->PendingCallback = NULL;
      status = HAL_ERROR;
      break;
  }

  return status;
}


/**
  * @brief  Store line number as handle private field.
  * @param  hexti: Exti handle.
  * @param  ExtiLine: Exti line number.
  *         This parameter can be from 0 to @ref EXTI_LINE_NB.
  * @retval HAL Status.
  */
HAL_StatusTypeDef HAL_EXTI_GetHandle(EXTI_HandleTypeDef *hexti, uint32_t ExtiLine)
{
  /* Check the parameters */
  assert_param(IS_EXTI_LINE(ExtiLine));

  /* Check null pointer */
  if (hexti == NULL)
  {
    return HAL_ERROR;
  }
  else
  {
    /* Store line number as handle private field */
    hexti->Line = ExtiLine;

    return HAL_OK;
  }
}

/**
  * @brief  Handle EXTI interrupt request.
  * @param  hexti: Exti handle.
  * @retval none.
  */
void HAL_EXTI_IRQHandler(EXTI_HandleTypeDef *hexti)
{
  __IO uint32_t *regaddr;
  uint32_t regval;
  uint32_t maskline;
  uint32_t offset;

  /* Compute line register offset */
  offset = ((hexti->Line & EXTI_REG_MASK) >> EXTI_REG_SHIFT);
  /* compute line mask */
  maskline = (1uL << (hexti->Line & EXTI_PIN_MASK));

  /* Get pending bit  */
  regaddr = (&EXTI->PR + (EXTI_CONFIG_OFFSET * offset));
  regval = (*regaddr & maskline);

  if (regval != 0x00u)
  {
    /* Clear pending bit */
    *regaddr = maskline;

    /* Call pending callback */
    if (hexti->PendingCallback != NULL)
    {
      hexti->PendingCallback();
    }
  }
}

/**
  * @brief  Get interrupt pending bit of a dedicated line.
  * @param  hexti: Exti handle.
  * @param  Edge: unused
  * @retval 1 if interrupt is pending else 0.
  */
uint32_t HAL_EXTI_GetPending(EXTI_HandleTypeDef *hexti, uint32_t Edge)
{
  __IO uint32_t *regaddr;
  uint32_t regval;
  uint32_t linepos;
  uint32_t maskline;
  uint32_t offset;

  /* Check parameters */
  assert_param(IS_EXTI_LINE(hexti->Line));
  assert_param(IS_EXTI_CONFIG_LINE(hexti->Line));
  UNUSED(Edge);

  /* Compute line register offset */
  offset = ((hexti->Line & EXTI_REG_MASK) >> EXTI_REG_SHIFT);
  /* Compute line position */
  linepos = (hexti->Line & EXTI_PIN_MASK);
  /* Compute line mask */
  maskline = (1uL << linepos);

  /* Get pending bit */
  regaddr = (&EXTI->PR + (EXTI_CONFIG_OFFSET * offset));

  /* return 1 if bit is set else 0 */
  regval = ((*regaddr & maskline) >> linepos);
  return regval;
}


/**
  * @brief  Clear interrupt pending bit of a dedicated line.
  * @param  hexti: Exti handle.
  * @param  Edge: unused
  * @retval None.
  */
void HAL_EXTI_ClearPending(EXTI_HandleTypeDef *hexti, uint32_t Edge)
{
  __IO uint32_t *regaddr;
  uint32_t maskline;
  uint32_t offset;

  /* Check parameters */
  assert_param(IS_EXTI_LINE(hexti->Line));
  assert_param(IS_EXTI_CONFIG_LINE(hexti->Line));
  UNUSED(Edge);

  /* Compute line register offset */
  offset = ((hexti->Line & EXTI_REG_MASK) >> EXTI_REG_SHIFT);
  /* Compute line mask */
  maskline = (1uL << (hexti->Line & EXTI_PIN_MASK));

  /* Get pending register address */
  regaddr = (&EXTI->PR + (EXTI_CONFIG_OFFSET * offset));

  /* Clear Pending bit */
  *regaddr =  maskline;
}


/**
  * @brief  Generate a software interrupt for a dedicated line.
  * @param  hexti: Exti handle.
  * @retval None.
  */
void HAL_EXTI_GenerateSWI(EXTI_HandleTypeDef *hexti)
{
  __IO uint32_t *regaddr;
  uint32_t maskline;
  uint32_t offset;

  /* Check parameter */
  assert_param(IS_EXTI_LINE(hexti->Line));
  assert_param(IS_EXTI_CONFIG_LINE(hexti->Line));

  /* compute line register offset */
  offset = ((hexti->Line & EXTI_REG_MASK) >> EXTI_REG_SHIFT);
  /* compute line mask */
  maskline = (1uL << (hexti->Line & EXTI_PIN_MASK));

  regaddr = (&EXTI->SWIER + (EXTI_CONFIG_OFFSET * offset));
  *regaddr = maskline;
}

/**
  * end of EXTI_Function_Definitions @}
  */

#endif /* HAL_EXTI_MODULE_ENABLED */

/**
  * end of EXTI @}
  */
