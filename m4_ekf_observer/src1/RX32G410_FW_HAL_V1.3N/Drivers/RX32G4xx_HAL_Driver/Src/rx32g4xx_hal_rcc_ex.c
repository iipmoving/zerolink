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
*   File    : rx32g4xx_hal_rcc_ex.c
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"

/** @defgroup RCC RCC
  * @{
  */

#ifdef HAL_RCC_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/** @defgroup RCC_Private_Constants RCC Private Constants
 * @{
 */
#define DIVIDER_P_UPDATE          0U
#define DIVIDER_Q_UPDATE          1U
#define DIVIDER_R_UPDATE          2U

/** 
  * end of RCC_Private_Constants @}  
  */

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @defgroup RCC_Private_Functions RCC Private Functions
  * @{
  */

/**
  * @brief  Initialize the RCC extended peripherals clocks according to the specified
  *         parameters in the RCC_PeriphCLKInitTypeDef.
  * @param  PeriphClkInit  pointer to an RCC_PeriphCLKInitTypeDef structure that
  *         contains a field PeriphClockSelection which can be a combination of the following values:
  *           @arg RCC_PERIPHCLK_RTC  RTC peripheral clock
  *           @arg RCC_PERIPHCLK_ADC123  ADC1, ADC2 AND ADC3 peripheral clock
  *
  * @note   Care must be taken when HAL_RCCEx_PeriphCLKConfig() is used to select
  *         the RTC clock source: in this case the access to Backup domain is enabled.
  *
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_RCCEx_PeriphCLKConfig(RCC_PeriphCLKInitTypeDef  *PeriphClkInit)
{
  uint32_t tmpregister;
  uint32_t tickstart;
  HAL_StatusTypeDef ret = HAL_OK;      /* Intermediate status */
  HAL_StatusTypeDef status = HAL_OK;   /* Final status */

  /* Check the parameters */
  assert_param(IS_RCC_PERIPHCLOCK(PeriphClkInit->PeriphClockSelection));

  /*-------------------------- RTC clock source configuration ----------------------*/
  if((PeriphClkInit->PeriphClockSelection & RCC_PERIPHCLK_RTC) == RCC_PERIPHCLK_RTC)
  {
    FlagStatus       pwrclkchanged = RESET;
      
    /* Check for RTC Parameters used to output RTCCLK */
    assert_param(IS_RCC_RTCCLKSOURCE(PeriphClkInit->RTCClockSelection));

    /* Enable Power Clock */
    if(__HAL_RCC_PWR_IS_CLK_DISABLED())
    {
      __HAL_RCC_PWR_CLK_ENABLE();
      pwrclkchanged = SET;
    }

    /* Enable write access to Backup domain */
    SET_BIT(PWR->CR1, PWR_CR1_DBP);

    /* Wait for Backup domain Write protection disable */
    tickstart = HAL_GetTick();

    while((PWR->CR1 & PWR_CR1_DBP) == 0U)
    {
      if((HAL_GetTick() - tickstart) > RCC_DBP_TIMEOUT_VALUE)
      {
        ret = HAL_TIMEOUT;
        break;
      }
    }

    if(ret == HAL_OK)
    {
      /* Reset the Backup domain only if the RTC Clock source selection is modified from default */
      tmpregister = READ_BIT(RCC->BDCR, RCC_BDCR_RTCSEL);

      if((tmpregister != RCC_RTCCLKSOURCE_NONE) && (tmpregister != PeriphClkInit->RTCClockSelection))
      {
        /* Store the content of BDCR register before the reset of Backup Domain */
        tmpregister = READ_BIT(RCC->BDCR, ~(RCC_BDCR_RTCSEL));
        /* RTC Clock selection can be changed only if the Backup Domain is reset */
        __HAL_RCC_BACKUPRESET_FORCE();
        __HAL_RCC_BACKUPRESET_RELEASE();
        /* Restore the Content of BDCR register */
        RCC->BDCR = tmpregister;
      }

      if(ret == HAL_OK)
      {
        /* Apply new RTC clock source selection */
        __HAL_RCC_RTC_CONFIG(PeriphClkInit->RTCClockSelection);
      }
      else
      {
        /* set overall return value */
        status = ret;
      }
    }
    else
    {
      /* set overall return value */
      status = ret;
    }

    /* Restore clock configuration if changed */
    if(pwrclkchanged == SET)
    {
      __HAL_RCC_PWR_CLK_DISABLE();
    }
  }

  /*-------------------------- ADC123 clock source configuration ----------------------*/
  if(((PeriphClkInit->PeriphClockSelection) & RCC_PERIPHCLK_ADC123) == RCC_PERIPHCLK_ADC123)
  {
    /* Check the parameters */
    assert_param(IS_RCC_ADC123CLKSOURCE(PeriphClkInit->Adc123ClockSelection));

    /* Configure the ADC12 interface clock source */
    __HAL_RCC_ADC123_CONFIG(PeriphClkInit->Adc123ClockSelection);

    if(PeriphClkInit->Adc123ClockSelection == RCC_ADC123CLKSOURCE_PLL)
    {
      /* Enable PLLADCCLK output */
      __HAL_RCC_PLLCLKOUT_ENABLE(RCC_PLL_ADCCLK);
    }
  }

  return status;
}

/**
  * @brief  Get the RCC_ClkInitStruct according to the internal RCC configuration registers.
  * @param  PeriphClkInit : pointer to an RCC_PeriphCLKInitTypeDef structure that
  *         returns the configuration information for the Extended Peripherals
  *         clocks(ADCx, RTC).
  * @retval None
  */
void HAL_RCCEx_GetPeriphCLKConfig(RCC_PeriphCLKInitTypeDef  *PeriphClkInit)
{
  /* Set all possible values for the extended clock type parameter------------*/
  PeriphClkInit->PeriphClockSelection = RCC_PERIPHCLK_ADC123 | RCC_PERIPHCLK_RTC;
    
  /* Get the ADC123 clock source ------------------------------------------------*/
  PeriphClkInit->Adc123ClockSelection = __HAL_RCC_GET_ADC123_SOURCE();

  /* Get the RTC clock source ------------------------------------------------*/
  PeriphClkInit->RTCClockSelection    = __HAL_RCC_GET_RTC_SOURCE();

}

/**
  * @brief  Return the peripheral clock frequency for peripherals with clock source from PLL
  * @note   Return 0 if peripheral clock identifier not managed by this API
  * @param  PeriphClk  Peripheral clock identifier
  *         This parameter can be one of the following values:
  *           @arg RCC_PERIPHCLK_ADC123  ADC1, ADC2 and ADC3 peripheral clock
  *           @arg RCC_PERIPHCLK_RTC  RTC peripheral clock
  * @retval Frequency in Hz
  */
uint32_t HAL_RCCEx_GetPeriphCLKFreq(uint32_t PeriphClk)
{
  uint32_t frequency = 0U;
  uint32_t srcclk;
  uint32_t pllvco, plln, pllp;

  /* Check the parameters */
  assert_param(IS_RCC_PERIPHCLOCK(PeriphClk));

  if(PeriphClk == RCC_PERIPHCLK_RTC)
  {
    /* Get the current RTC source */
    srcclk = __HAL_RCC_GET_RTC_SOURCE();

    if ((HAL_IS_BIT_SET(RCC->CSR, RCC_CSR_LSIRDY)) && (srcclk == RCC_RTCCLKSOURCE_LSI))
    {
      frequency = LSI_VALUE;
    }
    /* Check if HSE is ready  and if RTC clock selection is HSI_DIV128*/
    else if ((HAL_IS_BIT_SET(RCC->CR, RCC_CR_HSERDY)) && (srcclk == RCC_RTCCLKSOURCE_HSE_DIV128))
    {
      frequency = HSE_VALUE / 128U;
    }
    /* Clock not enabled for RTC*/
    else
    {
      /* nothing to do: frequency already initialized to 0 */
    }
  }
  else
  {
    /* Other external peripheral clock source than RTC */

    /* Compute PLL clock input */
    if(__HAL_RCC_GET_PLL_OSCSOURCE() == RCC_PLLSOURCE_HSI)   /* HSI ? */
    {
      if(HAL_IS_BIT_SET(RCC->CR, RCC_CR_HSIRDY))
      {
        pllvco = HSI_VALUE;
      }
      else
      {
        pllvco = 0U;
      }
    }
    else if(__HAL_RCC_GET_PLL_OSCSOURCE() == RCC_PLLSOURCE_HSE)   /* HSE ? */
    {
      if(HAL_IS_BIT_SET(RCC->CR, RCC_CR_HSERDY))
      {
        pllvco = HSE_VALUE;
      }
      else
      {
        pllvco = 0U;
      }
    }
    else /* No source */
    {
      pllvco = 0U;
    }


    switch(PeriphClk)
    {
      case RCC_PERIPHCLK_ADC123:
        /* Get the current ADC123 source */
        srcclk = __HAL_RCC_GET_ADC123_SOURCE();
      
        if(srcclk == RCC_ADC123CLKSOURCE_PLL)
        {
          if(__HAL_RCC_GET_PLLCLKOUT_CONFIG(RCC_PLL_ADCCLK) != 0U)
          {
            /* f(PLLP) = f(VCO input) * PLLN / PLLP */
            plln = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos;
            pllp = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLPDIV) >> RCC_PLLCFGR_PLLPDIV_Pos;
            if(pllp == 0U)
            {
              if(READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLP) != 0U)
              {
                pllp = 17U;
              }
              else
              {
                pllp = 7U;
              }
            }
            frequency = (pllvco * plln) / pllp;
          }
        }
        else if(srcclk == RCC_ADC123CLKSOURCE_SYSCLK)
        {
          frequency = HAL_RCC_GetSysClockFreq();
        }
        /* Clock not enabled for ADC123 */
        else
        {
          /* nothing to do: frequency already initialized to 0 */
        }
        break;

    default:
      break;
    }
  }

  return(frequency);
}

/** 
  * end of RCC_Function_Definitions @}  
  */
  
#endif /* HAL_RCC_MODULE_ENABLED */

/** 
  * end of RCC @}  
  */
