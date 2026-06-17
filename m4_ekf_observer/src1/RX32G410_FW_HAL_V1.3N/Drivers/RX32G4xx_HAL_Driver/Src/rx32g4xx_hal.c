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
*   File    : rx32g4xx_hal.c
*   By      : RX_DV_Team
**************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"


/** @defgroup HAL HAL
  * @{
  */

#ifdef HAL_MODULE_ENABLED

/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define VREFBUF_TIMEOUT_VALUE     10U   /* 10 ms */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported variables ---------------------------------------------------------*/

/** @defgroup HAL_Exported_Variables HAL Exported Variables
  * @{
  */
__IO uint32_t uwTick;
uint32_t uwTickPrio = (1UL << __NVIC_PRIO_BITS);    /* Invalid PRIO */
uint32_t uwTickFreq = HAL_TICK_FREQ_DEFAULT;        /* 1KHz */
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/

/** @addtogroup HAL_Function_Definitions HAL Function Definitions
  * @{
  */

/**
  * @brief  This function is used to configure the Flash prefetch, the Instruction and Data caches,
  *         the time base source, NVIC and any required global low level hardware
  *         by calling the HAL_MspInit() callback function.
  * @note   HAL_Init() function is called at the beginning of program after reset and before
  *         the clock configuration.
  * @note   In the default implementation the System Timer (Systick) is used as source of time base.
  *         The Systick configuration is based on HSI clock, as HSI is the clock
  *         used after a system Reset and the NVIC configuration is set to Priority group 4.
  *         Once done, time base tick starts incrementing: the tick variable counter is incremented
  *         each 1ms in the SysTick_Handler() interrupt handler.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_Init(void)
{
  HAL_StatusTypeDef  status = HAL_OK;
  /* Configure Flash prefetch, Instruction cache, Data cache */
  /* Default configuration at reset is:                      */
  /* - Prefetch disabled                                     */
  /* - Instruction cache enabled                             */
  /* - Data cache enabled                                    */
#if (INSTRUCTION_CACHE_ENABLE == 0U)
  __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
#endif /* INSTRUCTION_CACHE_ENABLE */

#if (DATA_CACHE_ENABLE == 0U)
  __HAL_FLASH_DATA_CACHE_DISABLE();
#endif /* DATA_CACHE_ENABLE */

#if (PREFETCH_ENABLE != 0U)
  __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
#endif /* PREFETCH_ENABLE */

  /* Set Interrupt Group Priority */
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* Use SysTick as time base source and configure 1ms tick (default clock after Reset is HSI) */
  if (HAL_InitTick(IRQ_PRIORITY_SysTick) != HAL_OK)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Init the low level hardware */
    HAL_MspInit();
  }

  /* Return function status */
  return status;

}

/**
  * @brief  This function de-initializes common part of the HAL and stops the source of time base.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DeInit(void)
{
  /* Reset of all peripherals */
  __HAL_RCC_APB1_FORCE_RESET();
  __HAL_RCC_APB1_RELEASE_RESET();

  __HAL_RCC_APB2_FORCE_RESET();
  __HAL_RCC_APB2_RELEASE_RESET();

  __HAL_RCC_AHB1_FORCE_RESET();
  __HAL_RCC_AHB1_RELEASE_RESET();

  __HAL_RCC_AHB2_FORCE_RESET();
  __HAL_RCC_AHB2_RELEASE_RESET();
  
  //disable all TIM break IO
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN|RCC_APB2ENR_TIM8EN|RCC_APB2ENR_TIM15EN);
  TIM1->AF1 = 0;
  TIM1->AF2 = 0;
  TIM8->AF1 = 0;
  TIM8->AF2 = 0;
  TIM15->AF1 = 0;
  CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN|RCC_APB2ENR_TIM8EN|RCC_APB2ENR_TIM15EN);

  /* De-Init the low level hardware */
  HAL_MspDeInit();

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the MSP.
  * @retval None
  */
__weak void HAL_MspInit(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_MspInit could be implemented in the user file
   */
}

/**
  * @brief  DeInitializes the MSP.
  * @retval None
  */
__weak void HAL_MspDeInit(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_MspDeInit could be implemented in the user file
   */
}

/**
  * @brief  This function configures the source of the time base:
  *         The time source is configured to have 1ms time base with a dedicated
  *         Tick interrupt priority.
  * @note   This function is called automatically at the beginning of program after
  *         reset by HAL_Init() or at any time when clock is reconfigured by HAL_RCC_ClockConfig().
  * @note   In the default implementation, SysTick timer is the source of time base.
  *         It is used to generate interrupts at regular time intervals.
  *         Care must be taken if HAL_Delay() is called from a peripheral ISR process,
  *         The SysTick interrupt must have higher priority (numerically lower)
  *         than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
  *         The function is declared as __weak to be overwritten  in case of other
  *         implementation in user file.
  * @param  TickPriority: Tick interrupt priority.
  *         @arg IRQ_PRIORITY_SysTick
  * @retval HAL status
  */
__weak HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  HAL_StatusTypeDef  status = HAL_OK;

  if (uwTickFreq != 0U)
  {
    /* Configure the SysTick to have interrupt in 1ms time basis*/
    if (HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq)) == 0U)
    {
      /* Configure the SysTick IRQ priority */
      if (TickPriority < (1UL << __NVIC_PRIO_BITS))
      {
        HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority, 0U);
        uwTickPrio = TickPriority;
      }
      else
      {
        status = HAL_ERROR;
      }
    }
    else
    {
      status = HAL_ERROR;
    }
  }
  else
  {
    status = HAL_ERROR;
  }

  /* Return function status */
  return status;
}

/**
  * @brief  This function is called to increment a global variable "uwTick"
  *         used as application time base.
  * @note   In the default implementation, this variable is incremented each 1ms
  *         in SysTick ISR.
  * @note   This function is declared as __weak to be overwritten in case of other
  *         implementations in user file.
  * @retval None
  */
__weak void HAL_IncTick(void)
{
  uwTick += uwTickFreq;
}

/**
  * @brief  Provides a tick value in millisecond.
  * @note   This function is declared as __weak to be overwritten in case of other
  *         implementations in user file.
  * @retval tick value
  */
__weak uint32_t HAL_GetTick(void)
{
  return uwTick;
}

/**
  * @brief  This function returns a tick priority.
  * @retval tick priority
  */
uint32_t HAL_GetTickPrio(void)
{
  return uwTickPrio;
}

/**
  * @brief  Set the new tick Freq.
  * @param  Freq: Tick Freq.
  *         @arg HAL_TICK_FREQ_10HZ
  *         @arg HAL_TICK_FREQ_100HZ
  *         @arg HAL_TICK_FREQ_1KHZ
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SetTickFreq(uint32_t Freq)
{
  HAL_StatusTypeDef status  = HAL_OK;
  uint32_t prevTickFreq;

  assert_param(IS_TICKFREQ(Freq));

  if (uwTickFreq != Freq)
  {
    /* Back up uwTickFreq frequency */
    prevTickFreq = uwTickFreq;

    /* Update uwTickFreq global variable used by HAL_InitTick() */
    uwTickFreq = Freq;

    /* Apply the new tick Freq  */
    status = HAL_InitTick(uwTickPrio);

    if (status != HAL_OK)
    {
      /* Restore previous tick frequency */
      uwTickFreq = prevTickFreq;
    }
  }

  return status;
}

/**
  * @brief  Returns the tick frequency.
  * @retval Tick frequency.
  *         @arg HAL_TICK_FREQ_10HZ
  *         @arg HAL_TICK_FREQ_100HZ
  *         @arg HAL_TICK_FREQ_1KHZ
  */
uint32_t HAL_GetTickFreq(void)
{
  return uwTickFreq;
}

/**
  * @brief  Returns the difference tick value from Base to Current tick.
  * @param  u32Base: specifies the base tick value, in milliseconds.
  * @retval tick value
  */
uint32_t HAL_Tick_Diff(uint32_t u32Base)
{
    uint32_t u32CurrentCnt = HAL_GetTick();

    if (u32CurrentCnt >= u32Base)
    {
        return u32CurrentCnt - u32Base;
    }
    else
    {
        return HAL_MAX_DELAY - u32Base + u32CurrentCnt + 1;
    }
}

/**
  * @brief  This function provides minimum delay (in milliseconds) based
  *         on variable incremented.
  * @note   In the default implementation , SysTick timer is the source of time base.
  *         It is used to generate interrupts at regular time intervals where uwTick
  *         is incremented.
  * @note   This function is declared as __weak to be overwritten in case of other
  *         implementations in user file.
  * @param  Delay: specifies the delay time length, in milliseconds.
  * @retval None
  */
__weak void HAL_Delay(uint32_t Delay)
{
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = Delay;

  /* Add a freq to guarantee minimum wait */
  if (wait < HAL_MAX_DELAY)
  {
    wait += (uint32_t)(uwTickFreq);
  }

  while (HAL_Tick_Diff(tickstart) < wait)
  {
  }
}

/**
  * @brief  Suspends Tick increment.
  * @note   In the default implementation , SysTick timer is the source of time base. It is
  *         used to generate interrupts at regular time intervals. Once HAL_SuspendTick()
  *         is called, the SysTick interrupt will be disabled and so Tick increment
  *         is suspended.
  * @note   This function is declared as __weak to be overwritten in case of other
  *         implementations in user file.
  * @retval None
  */
__weak void HAL_SuspendTick(void)
{
  /* Disable SysTick Interrupt */
  CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

/**
  * @brief  Resume Tick increment.
  * @note   In the default implementation , SysTick timer is the source of time base. It is
  *         used to generate interrupts at regular time intervals. Once HAL_ResumeTick()
  *         is called, the SysTick interrupt will be enabled and so Tick increment
  *         is resumed.
  * @note   This function is declared as __weak to be overwritten in case of other
  *         implementations in user file.
  * @retval None
  */
__weak void HAL_ResumeTick(void)
{
  /* Enable SysTick Interrupt */
  SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

/**
  * @brief  Start a hardware CCMSRAM erase operation.
  * @note   As long as CCMSRAM is not erased the CCMER bit will be set.
  *         This bit is automatically reset at the end of the CCMSRAM erase operation.
  * @retval None
  */
void HAL_SYSCFG_CCMSRAMErase(void)
{
  /* unlock the write protection of the CCMER bit */
  SYSCFG->SKR = 0xCA;
  SYSCFG->SKR = 0x53;
  /* Starts a hardware CCMSRAM erase operation*/
  SET_BIT(SYSCFG->SCSR, SYSCFG_SCSR_CCMER);
}

/**
  * @brief  Configure the internal voltage reference buffer voltage scale.
  * @param  VoltageScaling: specifies the output voltage to achieve
  *         This parameter can be one of the following values:
  *         @arg SYSCFG_VREFBUF_VOLTAGE_SCALE0: VREFBUF_OUT around 2.0 V.
  *         @arg SYSCFG_VREFBUF_VOLTAGE_SCALE1: VREFBUF_OUT around 2.5 V.
  *         @arg SYSCFG_VREFBUF_VOLTAGE_SCALE2: VREFBUF_OUT around 2.8 V.
  * @retval None
  */
void HAL_SYSCFG_VREFBUF_VoltageScalingConfig(uint32_t VoltageScaling)
{
  /* Check the parameters */
  assert_param(IS_SYSCFG_VREFBUF_VOLTAGE_SCALE(VoltageScaling));

  MODIFY_REG(VREFBUF->CSR, VREFBUF_CSR_VRS, VoltageScaling);
}

/**
  * @brief  Configure the internal voltage reference buffer high impedance mode.
  * @param  Mode: specifies the high impedance mode
  *         This parameter can be one of the following values:
  *         @arg SYSCFG_VREFBUF_HIGH_IMPEDANCE_DISABLE: VREF+ pin is internally connect to VREFINT output.
  *         @arg SYSCFG_VREFBUF_HIGH_IMPEDANCE_ENABLE: VREF+ pin is high impedance.
  * @retval None
  */
void HAL_SYSCFG_VREFBUF_HighImpedanceConfig(uint32_t Mode)
{
  /* Check the parameters */
  assert_param(IS_SYSCFG_VREFBUF_HIGH_IMPEDANCE(Mode));

  MODIFY_REG(VREFBUF->CSR, VREFBUF_CSR_HIZ, Mode);
}

/**
  * @brief  Tune the Internal Voltage Reference buffer (VREFBUF).
  * @param  TrimmingValue: specifies trimming code for VREFBUF calibration
  *         This parameter can be a number between Min_Data = 0x00 and Max_Data = 0x0F
  * @retval None
  */
void HAL_SYSCFG_VREFBUF_TrimmingConfig(uint32_t TrimmingValue)
{
  /* Check the parameters */
  assert_param(IS_SYSCFG_VREFBUF_TRIMMING(TrimmingValue));

  MODIFY_REG(VREFBUF->CCR, VREFBUF_CCR_TRIM, TrimmingValue);
}

/**
  * @brief  Enable the Internal Voltage Reference buffer (VREFBUF).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_SYSCFG_EnableVREFBUF(void)
{
  uint32_t tickstart;

  SET_BIT(VREFBUF->CSR, VREFBUF_CSR_ENVR);

  /* Get Start Tick*/
  tickstart = HAL_GetTick();

  /* Wait for VRR bit  */
  while (READ_BIT(VREFBUF->CSR, VREFBUF_CSR_VRR) == 0x00U)
  {
    if (HAL_Tick_Diff(tickstart) > VREFBUF_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  return HAL_OK;
}

/**
  * @brief  Disable the Internal Voltage Reference buffer (VREFBUF).
  * @retval None
  */
void HAL_SYSCFG_DisableVREFBUF(void)
{
  CLEAR_BIT(VREFBUF->CSR, VREFBUF_CSR_ENVR);
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
__weak void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  while (1);
}
#endif /* USE_FULL_ASSERT */

/**
  * end of HAL_Function_Definitions @}
  */

#endif /* HAL_MODULE_ENABLED */

/**
  * end of HAL @}
  */
