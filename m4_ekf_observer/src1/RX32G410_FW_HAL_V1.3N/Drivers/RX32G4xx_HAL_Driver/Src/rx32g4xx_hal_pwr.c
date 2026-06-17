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
*   File    : rx32g4xx_hal_pwr.c
*   By      : RX_DV_Team
**************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"


/** @defgroup PWR PWR
  * @{
  */

#ifdef HAL_PWR_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** @defgroup PWR_Private_Defines PWR Private Defines
  * @{
  */

#define PVD_MODE_IT               ((uint32_t)0x00010000)  /*!< Mask for interruption yielded by PVD threshold crossing */
#define PVD_MODE_EVT              ((uint32_t)0x00020000)  /*!< Mask for event yielded by PVD threshold crossing        */
#define PVD_RISING_EDGE           ((uint32_t)0x00000001)  /*!< Mask for rising edge set as PVD trigger                 */
#define PVD_FALLING_EDGE          ((uint32_t)0x00000002)  /*!< Mask for falling edge set as PVD trigger                */
/**
  * end of PWR_Private_Defines @}
  */



/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup PWR_Exported_Functions PWR Exported Functions
  * @{
  */

/**
  * @brief Deinitialize the HAL PWR peripheral registers to their default reset values.
  * @retval None
  */
void HAL_PWR_DeInit(void)
{
  __HAL_RCC_PWR_FORCE_RESET();
  __HAL_RCC_PWR_RELEASE_RESET();
}

/**
  * @brief Enable access to the backup domain
  *        (RTC registers, RTC backup data registers).
  * @note  After reset, the backup domain is protected against
  *        possible unwanted write accesses.
  * @note  RTCSEL that sets the RTC clock source selection is in the RTC back-up domain.
  *        In order to set or modify the RTC clock, the backup domain access must be
  *        disabled.
  * @note  LSEON bit that switches on and off the LSE crystal belongs as well to the
  *        back-up domain.
  * @retval None
  */
void HAL_PWR_EnableBkUpAccess(void)
{
  SET_BIT(PWR->CR1, PWR_CR1_DBP);
}

/**
  * @brief Disable access to the backup domain
  *        (RTC registers, RTC backup data registers).
  * @retval None
  */
void HAL_PWR_DisableBkUpAccess(void)
{
  CLEAR_BIT(PWR->CR1, PWR_CR1_DBP);
}

/**
  * @brief Configure the voltage threshold detected by the Power Voltage Detector (PVD).
  * @param sConfigPVD: pointer to a PWR_PVDTypeDef structure that contains the PVD
  *        configuration information.
  * @note Refer to the electrical characteristics of your device datasheet for
  *         more details about the voltage thresholds corresponding to each
  *         detection level.
  * @retval None
  */
HAL_StatusTypeDef HAL_PWR_ConfigPVD(PWR_PVDTypeDef *sConfigPVD)
{
  /* Check the parameters */
  assert_param(IS_PWR_PVD_LEVEL(sConfigPVD->PVDLevel));
  assert_param(IS_PWR_PVD_MODE(sConfigPVD->Mode));

  /* Set PLS bits according to PVDLevel value */
  MODIFY_REG(PWR->CR2, PWR_CR2_PLS, sConfigPVD->PVDLevel);

  /* Clear any previous config. Keep it clear if no event or IT mode is selected */
  __HAL_PWR_PVD_EXTI_DISABLE_EVENT();
  __HAL_PWR_PVD_EXTI_DISABLE_IT();
  __HAL_PWR_PVD_EXTI_DISABLE_FALLING_EDGE();
  __HAL_PWR_PVD_EXTI_DISABLE_RISING_EDGE();

  /* Configure interrupt mode */
  if((sConfigPVD->Mode & PVD_MODE_IT) == PVD_MODE_IT)
  {
    __HAL_PWR_PVD_EXTI_ENABLE_IT();
  }

  /* Configure event mode */
  if((sConfigPVD->Mode & PVD_MODE_EVT) == PVD_MODE_EVT)
  {
    __HAL_PWR_PVD_EXTI_ENABLE_EVENT();
  }

  /* Configure the edge */
  if((sConfigPVD->Mode & PVD_RISING_EDGE) == PVD_RISING_EDGE)
  {
    __HAL_PWR_PVD_EXTI_ENABLE_RISING_EDGE();
  }

  if((sConfigPVD->Mode & PVD_FALLING_EDGE) == PVD_FALLING_EDGE)
  {
    __HAL_PWR_PVD_EXTI_ENABLE_FALLING_EDGE();
  }

  return HAL_OK;
}

/**
  * @brief Enable the Power Voltage Detector (PVD).
  * @retval None
  */
void HAL_PWR_EnablePVD(void)
{
  SET_BIT(PWR->CR2, PWR_CR2_PVDE);
}

/**
  * @brief Disable the Power Voltage Detector (PVD).
  * @retval None
  */
void HAL_PWR_DisablePVD(void)
{
  CLEAR_BIT(PWR->CR2, PWR_CR2_PVDE);
  CLEAR_BIT(SYSCFG->CFGR2, SYSCFG_CFGR2_PVDL);
}

/**
  * @brief Enable the WakeUp PINx functionality.
  * @param WakeUpPinPolarity: Specifies which Wake-Up pin to enable.
  *         This parameter can be one of the following legacy values which set the default polarity
  *         i.e. detection on high level (rising edge):
  *           @arg @ref PWR_WAKEUP_PIN1, PWR_WAKEUP_PIN2, PWR_WAKEUP_PIN3, PWR_WAKEUP_PIN4
  *
  *         or one of the following value where the user can explicitly specify the enabled pin and
  *         the chosen polarity:
  *           @arg @ref PWR_WAKEUP_PIN1_HIGH or PWR_WAKEUP_PIN1_LOW
  *           @arg @ref PWR_WAKEUP_PIN2_HIGH or PWR_WAKEUP_PIN2_LOW
  *           @arg @ref PWR_WAKEUP_PIN3_HIGH or PWR_WAKEUP_PIN3_LOW
  *           @arg @ref PWR_WAKEUP_PIN4_HIGH or PWR_WAKEUP_PIN4_LOW
  * @note  PWR_WAKEUP_PINx and PWR_WAKEUP_PINx_HIGH are equivalent.
  * @retval None
  */
void HAL_PWR_EnableWakeUpPin(uint32_t WakeUpPinPolarity)
{
  assert_param(IS_PWR_WAKEUP_PIN(WakeUpPinPolarity));
  /* Specifies the Wake-Up pin polarity for the event detection
    (rising or falling edge) */
  MODIFY_REG(PWR->CR4, (PWR_CR3_EWUP & WakeUpPinPolarity), (WakeUpPinPolarity >> PWR_WUP_POLARITY_SHIFT));

  /* Enable wake-up pin */
  SET_BIT(PWR->CR3, (PWR_CR3_EWUP & WakeUpPinPolarity));
}

/**
  * @brief Disable the WakeUp PINx functionality.
  * @param WakeUpPinx: Specifies the Power Wake-Up pin to disable.
  *         This parameter can be one of the following values:
  *           @arg @ref PWR_WAKEUP_PIN1, PWR_WAKEUP_PIN2, PWR_WAKEUP_PIN3, PWR_WAKEUP_PIN4
  * @retval None
  */
void HAL_PWR_DisableWakeUpPin(uint32_t WakeUpPinx)
{
  assert_param(IS_PWR_WAKEUP_PIN(WakeUpPinx));

  CLEAR_BIT(PWR->CR3, (PWR_CR3_EWUP & WakeUpPinx));
}

/**
  * @brief Enter Sleep mode.
  * @note  In Sleep mode, all I/O pins keep the same state as in Run mode.
  * @param SLEEPEntry: Specifies if Sleep mode is entered with WFI or WFE instruction.
  *           This parameter can be one of the following values:
  *            @arg @ref PWR_SLEEPENTRY_WFI enter Sleep mode with WFI instruction
  *            @arg @ref PWR_SLEEPENTRY_WFE enter Sleep mode with WFE instruction
  * @note  When WFI entry is used, tick interrupt have to be disabled if not desired as
  *        the interrupt wake up source.
  * @retval None
  */

void HAL_PWR_EnterSLEEPMode(uint8_t SLEEPEntry)
{
  /* Check the parameters */
  assert_param(IS_PWR_SLEEP_ENTRY(SLEEPEntry));

	/* Clear SLEEPDEEP bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));

	if(SLEEPEntry == PWR_SLEEPENTRY_WFI)
  {
    /* Request Wait For Interrupt */
    __WFI();
  }
  else
  {
    /* Request Wait For Event */
    __SEV();
    __WFE();
    __WFE();
  }
}



/**
  * @brief Enter Standby mode.
  * @note  In Standby mode, the PLL, the HSI and the HSE oscillators are switched
  *        off. The voltage regulator is disabled, except when SRAM2 content is preserved
  *        in which case the regulator is in low-power mode.
  *        SRAM1 and register contents are lost except for registers in the Backup domain and
  *        Standby circuitry. SRAM2 content can be preserved if the bit RRS is set in PWR_CR3 register.
  *        To enable this feature, the user can resort to HAL_PWREx_EnableSRAM2ContentRetention() API
  *        to set RRS bit.
  *        The BOR is available.
  * @note  The I/Os can be configured either with a pull-up or pull-down or can be kept in analog state.
  *        HAL_PWREx_EnableGPIOPullUp() and HAL_PWREx_EnableGPIOPullDown() respectively enable Pull Up and
  *        Pull Down state, HAL_PWREx_DisableGPIOPullUp() and HAL_PWREx_DisableGPIOPullDown() disable the
  *        same.
  *        These states are effective in Standby mode only if APC bit is set through
  *        HAL_PWREx_EnablePullUpPullDownConfig() API.
  * @retval None
  */
void HAL_PWR_EnterSTANDBYMode(void)
{
  /* Set Stand-by mode */
  MODIFY_REG(PWR->CR1, PWR_CR1_LPMS, PWR_MODE_STANDBY);

  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));

/* This option is used to ensure that store operations are completed */
#if defined ( __CC_ARM)
  __force_stores();
#endif
  /* Request Wait For Interrupt */
  __WFI();
}



/**
  * @brief Indicate Sleep-On-Exit when returning from Handler mode to Thread mode.
  * @note Set SLEEPONEXIT bit of SCR register. When this bit is set, the processor
  *       re-enters SLEEP mode when an interruption handling is over.
  *       Setting this bit is useful when the processor is expected to run only on
  *       interruptions handling.
  * @retval None
  */
void HAL_PWR_EnableSleepOnExit(void)
{
  /* Set SLEEPONEXIT bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPONEXIT_Msk));
}

/**
  * @brief Disable Sleep-On-Exit feature when returning from Handler mode to Thread mode.
  * @note Clear SLEEPONEXIT bit of SCR register. When this bit is set, the processor
  *       re-enters SLEEP mode when an interruption handling is over.
  * @retval None
  */
void HAL_PWR_DisableSleepOnExit(void)
{
  /* Clear SLEEPONEXIT bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPONEXIT_Msk));
}

/**
  * @brief Enable CORTEX M4 SEVONPEND bit.
  * @note Set SEVONPEND bit of SCR register. When this bit is set, this causes
  *       WFE to wake up when an interrupt moves from inactive to pended.
  * @retval None
  */
void HAL_PWR_EnableSEVOnPend(void)
{
  /* Set SEVONPEND bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SEVONPEND_Msk));
}

/**
  * @brief Disable CORTEX M4 SEVONPEND bit.
  * @note Clear SEVONPEND bit of SCR register. When this bit is set, this causes
  *       WFE to wake up when an interrupt moves from inactive to pended.
  * @retval None
  */
void HAL_PWR_DisableSEVOnPend(void)
{
  /* Clear SEVONPEND bit of Cortex System Control Register */
  CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SEVONPEND_Msk));
}

/**
  * @brief PWR PVD interrupt callback
  * @retval None
  */
__weak void HAL_PWR_PVDCallback(void)
{
  /* NOTE : This function should not be modified; when the callback is needed,
            the HAL_PWR_PVDCallback can be implemented in the user file
   */
}

/**
  * end of PWR_Exported_Functions @}
  */

/**
  * end of PWR @}
  */

#endif /* HAL_PWR_MODULE_ENABLED */

