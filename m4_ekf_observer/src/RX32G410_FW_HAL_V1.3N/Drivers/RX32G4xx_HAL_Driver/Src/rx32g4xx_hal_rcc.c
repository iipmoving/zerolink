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
*   File    : rx32g4xx_hal_rcc.c
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
/* Private define ------------------------------------------------------------*/
/** @defgroup RCC_Private_Constants RCC Private Constants
 * @{
 */
#define HSE_TIMEOUT_VALUE                100U
#define HSI_TIMEOUT_VALUE                2U                /* 2 ms (minimum Tick + 1) */
#define LSI_TIMEOUT_VALUE                2U                /* 2 ms (minimum Tick + 1) */
#define PLL_TIMEOUT_VALUE                20U               /* 10 ms  */
#define CLOCKSWITCH_TIMEOUT_VALUE        5000U             /* 5 s    */
#define LATENCY_TIMEOUT_VALUE            2U                /* 2 ms   */

/**
  * end of RCC_Private_Constants @}
  */

/* Private macro -------------------------------------------------------------*/
/** @defgroup RCC_Private_Macros RCC Private Macros
  * @{
  */
#define RCC_GET_MCO_GPIO_PIN(__RCC_MCOx__)               ((__RCC_MCOx__) & GPIO_PIN_MASK)

#define RCC_GET_MCO_GPIO_AF(__RCC_MCOx__)                (((__RCC_MCOx__) & RCC_MCO_GPIOAF_MASK) >> RCC_MCO_GPIOAF_POS)

#define RCC_GET_MCO_GPIO_INDEX(__RCC_MCOx__)             (((__RCC_MCOx__) & RCC_MCO_GPIOPORT_MASK) >> RCC_MCO_GPIOPORT_POS)

#define RCC_GET_MCO_GPIO_PORT(__RCC_MCOx__)              (AHB2PERIPH_BASE + ((0x00000400UL) * RCC_GET_MCO_GPIO_INDEX(__RCC_MCOx__)))

#define RCC_PLL_OSCSOURCE_CONFIG(__HAL_RCC_PLLSOURCE__)  (MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, (__HAL_RCC_PLLSOURCE__)))
/**
  * end of RCC_Private_Macros @}
  */

/* Private variables ---------------------------------------------------------*/

/** @defgroup RCC_Private_Variables RCC Private Variables
  * @{
  */

const uint8_t AHBPrescTable[16U] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};   /*!< AHB prescalers table values */
const uint8_t APBPrescTable[8U] =  {0, 0, 0, 0, 1, 2, 3, 4};                           /*!< APB prescalers table values */
const uint8_t TIMPrescTable[5U] =  {0, 2, 0, 3, 4};                                    /*!< TIM prescalers table values */
/**
  * end of RCC_Private_Variables @}
  */

/* Private function prototypes -----------------------------------------------*/

/** @defgroup RCC_Private_Functions RCC Private Functions
  * @{
  */

static uint32_t          RCC_GetSysClockFreqFromPLLSource(void);
/**
  * end of RCC_Private_Functions @}
  */

/* Exported functions --------------------------------------------------------*/

/** @defgroup RCC_Function_Definitions RCC Function Definitions
  * @{
  */

/**
  * @brief  Reset the RCC clock configuration to the default reset state.
  * @note   The default reset state of the clock configuration is given below:
  *            - HSI ON and used as system clock source
  *            - HSE, PLL OFF
  *            - AHB, APB1 and APB2 prescaler set to 1.
  *            - CSS, MCO1 OFF
  *            - All interrupts disabled
  *            - All interrupt and reset flags cleared
  * @note   This function doesn't modify the configuration of the
  *            - Peripheral clocks
  *            - LSI and RTC clocks
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_RCC_DeInit(void)
{
  uint32_t tickstart;

  /* Get Start Tick*/
  tickstart = HAL_GetTick();

  /* Set HSION bit to the reset value */
  SET_BIT(RCC->CR, RCC_CR_HSION);

  /* Wait till HSI is ready */
  while (READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0U)
  {
    if ((HAL_GetTick() - tickstart) > HSI_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  /* Get Start Tick*/
  tickstart = HAL_GetTick();

  /* Reset CFGR register (HSI is selected as system clock source) */
  RCC->CFGR = 0x00000001u;

  /* Wait till HSI is ready */
  while (READ_BIT(RCC->CFGR, RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
  {
    if ((HAL_GetTick() - tickstart) > CLOCKSWITCH_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  /* Update the SystemCoreClock global variable */
  SystemCoreClock = HSI_VALUE;

  /* Adapt Systick interrupt period */
  if (HAL_InitTick(uwTickPrio) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* Clear CR register in 2 steps: first to clear HSEON in case bypass was enabled */
  RCC->CR = RCC_CR_HSION;

  /* Then again to HSEBYP in case bypass was enabled */
  RCC->CR = RCC_CR_HSION;

  /* Get Start Tick*/
  tickstart = HAL_GetTick();

  /* Wait till PLL is OFF */
  while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) != 0U)
  {
    if ((HAL_GetTick() - tickstart) > PLL_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  /* once PLL is OFF, reset PLLCFGR register to default value */
  RCC->PLLCFGR = 0x00000000;

  /* Disable all interrupts */
  CLEAR_REG(RCC->CIER);

  /* Clear all interrupt flags */
  WRITE_REG(RCC->CICR, 0xFFFFFFFFU);

  /* Clear all reset flags */
  SET_BIT(RCC->CSR, RCC_CSR_RMVF);

  return HAL_OK;
}

/**
  * @brief  Initialize the RCC Oscillators according to the specified parameters in the
  *         RCC_OscInitTypeDef.
  * @param  RCC_OscInitStruct: pointer to an RCC_OscInitTypeDef structure that
  *         contains the configuration information for the RCC Oscillators.
  * @note   The PLL is not disabled when used as system clock.
  * @note   Transitions LSE Bypass to LSE On and LSE On to LSE Bypass are not
  *         supported by this macro. User should request a transition to LSE Off
  *         first and then LSE On or LSE Bypass.
  * @note   Transition HSE Bypass to HSE On and HSE On to HSE Bypass are not
  *         supported by this macro. User should request a transition to HSE Off
  *         first and then HSE On or HSE Bypass.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef  *RCC_OscInitStruct)
{
  uint32_t tickstart;
  uint32_t temp_sysclksrc;
  uint32_t temp_pllckcfg;

  /* Check Null pointer */
  if (RCC_OscInitStruct == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_RCC_OSCILLATORTYPE(RCC_OscInitStruct->OscillatorType));

  /*------------------------------- HSE Configuration ------------------------*/
  if (((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_HSE) == RCC_OSCILLATORTYPE_HSE)
  {
    /* Check the parameters */
    assert_param(IS_RCC_HSE(RCC_OscInitStruct->HSEState));

    temp_sysclksrc = __HAL_RCC_GET_SYSCLK_SOURCE();
    temp_pllckcfg = __HAL_RCC_GET_PLL_OSCSOURCE();

    /* When the HSE is used as system clock or clock source for PLL in these cases it is not allowed to be disabled */
    if (((temp_sysclksrc == RCC_CFGR_SWS_PLL) && (temp_pllckcfg == RCC_PLLSOURCE_HSE)) || (temp_sysclksrc == RCC_CFGR_SWS_HSE))
    {
      if ((READ_BIT(RCC->CR, RCC_CR_HSERDY) != 0U) && (RCC_OscInitStruct->HSEState == RCC_HSE_OFF))
      {
        return HAL_ERROR;
      }
    }
    else
    {
      /* Set the new HSE configuration ---------------------------------------*/
      __HAL_RCC_HSE_CONFIG(RCC_OscInitStruct->HSEState);

      /* Check the HSE State */
      if (RCC_OscInitStruct->HSEState != RCC_HSE_OFF)
      {
        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till HSE is ready */
        while (READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0U)
        {
          if ((HAL_GetTick() - tickstart) > HSE_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
      else
      {
        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till HSE is disabled */
        while (READ_BIT(RCC->CR, RCC_CR_HSERDY) != 0U)
        {
          if ((HAL_GetTick() - tickstart) > HSE_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
    }
  }
  /*----------------------------- HSI Configuration --------------------------*/
  if (((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_HSI) == RCC_OSCILLATORTYPE_HSI)
  {
    /* Check the parameters */
    assert_param(IS_RCC_HSI(RCC_OscInitStruct->HSIState));

    /* Check if HSI is used as system clock or as PLL source when PLL is selected as system clock */
    temp_sysclksrc = __HAL_RCC_GET_SYSCLK_SOURCE();
    temp_pllckcfg = __HAL_RCC_GET_PLL_OSCSOURCE();

    if (((temp_sysclksrc == RCC_CFGR_SWS_PLL) && (temp_pllckcfg == RCC_PLLSOURCE_HSI)) || (temp_sysclksrc == RCC_CFGR_SWS_HSI))
    {
      /* When HSI is used as system clock it will not be disabled */
      if ((READ_BIT(RCC->CR, RCC_CR_HSIRDY) != 0U) && (RCC_OscInitStruct->HSIState == RCC_HSI_OFF))
      {
        return HAL_ERROR;
      }
    }
    else
    {
      /* Check the HSI State */
      if (RCC_OscInitStruct->HSIState != RCC_HSI_OFF)
      {
        /* Enable the Internal High Speed oscillator (HSI). */
        __HAL_RCC_HSI_ENABLE();

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till HSI is ready */
        while (READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0U)
        {
          if ((HAL_GetTick() - tickstart) > HSI_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
      else
      {
        /* Disable the Internal High Speed oscillator (HSI). */
        __HAL_RCC_HSI_DISABLE();

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till HSI is disabled */
        while (READ_BIT(RCC->CR, RCC_CR_HSIRDY) != 0U)
        {
          if ((HAL_GetTick() - tickstart) > HSI_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
    }
  }
  /*------------------------------ LSI Configuration -------------------------*/
  if (((RCC_OscInitStruct->OscillatorType) & RCC_OSCILLATORTYPE_LSI) == RCC_OSCILLATORTYPE_LSI)
  {
    /* Check the parameters */
    assert_param(IS_RCC_LSI(RCC_OscInitStruct->LSIState));

    /* Check the LSI State */
    if(RCC_OscInitStruct->LSIState != RCC_LSI_OFF)
    {
      /* Enable the Internal Low Speed oscillator (LSI). */
      __HAL_RCC_LSI_ENABLE();

      /* Get Start Tick*/
      tickstart = HAL_GetTick();

      /* Wait till LSI is ready */
      while (READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) == 0U)
      {
        if ((HAL_GetTick() - tickstart) > LSI_TIMEOUT_VALUE)
        {
          return HAL_TIMEOUT;
        }
      }
    }
    else
    {
      /* Disable the Internal Low Speed oscillator (LSI). */
      __HAL_RCC_LSI_DISABLE();

      /* Get Start Tick*/
      tickstart = HAL_GetTick();

      /* Wait till LSI is disabled */
      while(READ_BIT(RCC->CSR, RCC_CSR_LSIRDY) != 0U)
      {
        if((HAL_GetTick() - tickstart) > LSI_TIMEOUT_VALUE)
        {
          return HAL_TIMEOUT;
        }
      }
    }
  }
  /*-------------------------------- PLL Configuration -----------------------*/
  /* Check the parameters */
  assert_param(IS_RCC_PLL(RCC_OscInitStruct->PLL.PLLState));

  if (RCC_OscInitStruct->PLL.PLLState != RCC_PLL_NONE)
  {
    /* Check if the PLL is used as system clock or not */
    if (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_CFGR_SWS_PLL)
    {
      if (RCC_OscInitStruct->PLL.PLLState == RCC_PLL_ON)
      {
        /* Check the parameters */
        assert_param(IS_RCC_PLLSOURCE(RCC_OscInitStruct->PLL.PLLSource));
        assert_param(IS_RCC_PLLN_VALUE(RCC_OscInitStruct->PLL.PLLN));
        assert_param(IS_RCC_PLLP_VALUE(RCC_OscInitStruct->PLL.PLLP));
        assert_param(IS_RCC_PLLQ_VALUE(RCC_OscInitStruct->PLL.PLLQ));
        assert_param(IS_RCC_PLLR_VALUE(RCC_OscInitStruct->PLL.PLLR));

        /* Disable the main PLL. */
        __HAL_RCC_PLL_DISABLE();

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till PLL is disabled */
        while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) != 0U)
        {
          if ((HAL_GetTick() - tickstart) > PLL_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }

        /* Configure the main PLL clock source, multiplication and division factors. */
        __HAL_RCC_PLL_CONFIG(RCC_OscInitStruct->PLL.PLLSource,
                             RCC_OscInitStruct->PLL.PLLN,
                             RCC_OscInitStruct->PLL.PLLP,
                             RCC_OscInitStruct->PLL.PLLQ,
                             RCC_OscInitStruct->PLL.PLLR);

        /* Enable the main PLL. */
        __HAL_RCC_PLL_ENABLE();

        /* Enable PLL System Clock output. */
         __HAL_RCC_PLLCLKOUT_ENABLE(RCC_PLL_SYSCLK);

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till PLL is ready */
        while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0U)
        {
          if ((HAL_GetTick() - tickstart) > PLL_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }
      }
      else
      {
        /* Disable the main PLL. */
        __HAL_RCC_PLL_DISABLE();

        /* Get Start Tick*/
        tickstart = HAL_GetTick();

        /* Wait till PLL is disabled */
        while (READ_BIT(RCC->CR, RCC_CR_PLLRDY) != 0U)
        {
          if ((HAL_GetTick() - tickstart) > PLL_TIMEOUT_VALUE)
          {
            return HAL_TIMEOUT;
          }
        }

        /* Unselect PLL clock source and disable outputs to save power */
        RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC | RCC_PLL_SYSCLK | RCC_PLL_USBCLK | RCC_PLL_ADCCLK);
      }
    }
    else
    {
      /* Check if there is a request to disable the PLL used as System clock source */
      if((RCC_OscInitStruct->PLL.PLLState) == RCC_PLL_OFF)
      {
        return HAL_ERROR;
      }
      else
      {
        /* Do not return HAL_ERROR if request repeats the current configuration */
        temp_pllckcfg = RCC->PLLCFGR;
       if((READ_BIT(temp_pllckcfg, RCC_PLLCFGR_PLLSRC)  != RCC_OscInitStruct->PLL.PLLSource) ||
           (READ_BIT(temp_pllckcfg, RCC_PLLCFGR_PLLN)    != ((RCC_OscInitStruct->PLL.PLLN)   << RCC_PLLCFGR_PLLN_Pos)) ||
           (READ_BIT(temp_pllckcfg, RCC_PLLCFGR_PLLPDIV) != (RCC_OscInitStruct->PLL.PLLP) ) ||
           (READ_BIT(temp_pllckcfg, RCC_PLLCFGR_PLLQDIV) != (RCC_OscInitStruct->PLL.PLLQ) ) ||
           (READ_BIT(temp_pllckcfg, RCC_PLLCFGR_PLLRDIV) != (RCC_OscInitStruct->PLL.PLLR) ))
        {
          return HAL_ERROR;
        }
      }
    }
  }

  return HAL_OK;
}

/**
  * @brief  Initialize the CPU, AHB and APB buses clocks according to the specified
  *         parameters in the RCC_ClkInitStruct.
  * @param  RCC_ClkInitStruct : pointer to an RCC_OscInitTypeDef structure that
  *         contains the configuration information for the RCC peripheral.
  * @param  FLatency : FLASH Latency
  *         This parameter can be one of the following values:
  *            @arg FLASH_LATENCY_0   FLASH 0 Latency cycle
  *            @arg FLASH_LATENCY_1   FLASH 1 Latency cycle
  *            @arg FLASH_LATENCY_2   FLASH 2 Latency cycles
  *            @arg FLASH_LATENCY_3   FLASH 3 Latency cycles
  *            @arg FLASH_LATENCY_4   FLASH 4 Latency cycles
  *
  * @note   The SystemCoreClock CMSIS variable is used to store System Clock Frequency
  *         and updated by HAL_RCC_GetHCLKFreq() function called within this function
  *
  * @note   The HSI is used by default as system clock source after
  *         startup from Reset, wake-up from STANDBY mode. After restart from Reset,
  *         the HSI frequency is set to its default value 16 MHz.
  *
  * @note   The HSI can be selected as system clock source after
  *         from STOP modes or in case of failure of the HSE used directly or indirectly
  *         as system clock (if the Clock Security System CSS is enabled).
  *
  * @note   A switch from one clock source to another occurs only if the target
  *         clock source is ready (clock stable after startup delay or PLL locked).
  *         If a clock source which is not yet ready is selected, the switch will
  *         occur when the clock source is ready.
  *
  * @note   You can use HAL_RCC_GetClockConfig() function to know which clock is
  *         currently used as system clock source.
  *
  * @note   Depending on the device voltage range, the software has to set correctly
  *         HPRE[3:0] bits to ensure that HCLK not exceed the maximum allowed frequency
  *         (for more details refer to section above "Initialization/de-initialization functions")
  * @retval None
  */
HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef  *RCC_ClkInitStruct, uint32_t FLatency)
{
  uint32_t tickstart;
  uint32_t pllfreq;
  uint32_t hpre = RCC_SYSCLK_DIV1;

  /* Check Null pointer */
  if (RCC_ClkInitStruct == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_RCC_CLOCKTYPE(RCC_ClkInitStruct->ClockType));
  assert_param(IS_FLASH_LATENCY(FLatency));

  /* To correctly read data from FLASH memory, the number of wait states (LATENCY)
    must be correctly programmed according to the frequency of the CPU clock
    (HCLK) and the supply voltage of the device. */

  /* Increasing the number of wait states because of higher CPU frequency */
  if (FLatency > __HAL_FLASH_GET_LATENCY())
  {
    /* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
    __HAL_FLASH_SET_LATENCY(FLatency);

    tickstart = HAL_GetTick();
    /* Check that the new number of wait states is taken into account to access the Flash
    memory by reading the FLASH_ACR register */
    while (__HAL_FLASH_GET_LATENCY() != FLatency)
    {
      if ((HAL_GetTick() - tickstart) > LATENCY_TIMEOUT_VALUE)
    {
      return HAL_ERROR;
    }
  }
  }

  /*------------------------- SYSCLK Configuration ---------------------------*/
  if(((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_SYSCLK) == RCC_CLOCKTYPE_SYSCLK)
  {
    assert_param(IS_RCC_SYSCLKSOURCE(RCC_ClkInitStruct->SYSCLKSource));

    /* PLL is selected as System Clock Source */
    if (RCC_ClkInitStruct->SYSCLKSource == RCC_SYSCLKSOURCE_PLLCLK)
    {
      /* Check the PLL ready flag */
      if (READ_BIT(RCC->CR, RCC_CR_PLLRDY) == 0U)
      {
        return HAL_ERROR;
      }
      /* Undershoot management when selection PLL as SYSCLK source and frequency above 80Mhz */
      /* Compute target PLL output frequency */
      pllfreq = RCC_GetSysClockFreqFromPLLSource();

      /* Intermediate step with HCLK prescaler 2 necessary before to go over 80Mhz */
      if(pllfreq > 80000000U)
      {
        if (((READ_BIT(RCC->CFGR, RCC_CFGR_HPRE) == RCC_SYSCLK_DIV1)) ||
            (((((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_HCLK) == RCC_CLOCKTYPE_HCLK) &&
              (RCC_ClkInitStruct->AHBCLKDivider == RCC_SYSCLK_DIV1))))
        {
          MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_SYSCLK_DIV2);
          hpre = RCC_SYSCLK_DIV2;
        }
      }
    }
    else
    {
      /* HSE is selected as System Clock Source */
      if (RCC_ClkInitStruct->SYSCLKSource == RCC_SYSCLKSOURCE_HSE)
      {
        /* Check the HSE ready flag */
        if(READ_BIT(RCC->CR, RCC_CR_HSERDY) == 0U)
        {
          return HAL_ERROR;
        }
      }
      /* HSI is selected as System Clock Source */
      else
      {
        /* Check the HSI ready flag */
        if(READ_BIT(RCC->CR, RCC_CR_HSIRDY) == 0U)
        {
          return HAL_ERROR;
        }
      }
      /* Overshoot management when going down from PLL as SYSCLK source and frequency above 80Mhz */
      pllfreq = HAL_RCC_GetSysClockFreq();

      /* Intermediate step with HCLK prescaler 2 necessary before to go under 80Mhz */
      if(pllfreq > 80000000U)
      {
        MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_SYSCLK_DIV2);
        hpre = RCC_SYSCLK_DIV2;
      }

    }

    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_ClkInitStruct->SYSCLKSource);

    /* Get Start Tick*/
    tickstart = HAL_GetTick();

    while (__HAL_RCC_GET_SYSCLK_SOURCE() != (RCC_ClkInitStruct->SYSCLKSource << RCC_CFGR_SWS_Pos))
    {
      if ((HAL_GetTick() - tickstart) > CLOCKSWITCH_TIMEOUT_VALUE)
      {
        return HAL_TIMEOUT;
      }
    }
  }

  /*-------------------------- HCLK Configuration --------------------------*/
  if (((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_HCLK) == RCC_CLOCKTYPE_HCLK)
  {
    /* Set the highest APB divider in order to ensure that we do not go through
       a non-spec phase whatever we decrease or increase HCLK. */
    if (((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_PCLK1) == RCC_CLOCKTYPE_PCLK1)
    {
      MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_HCLK_DIV16);
    }
    if (((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_PCLK2) == RCC_CLOCKTYPE_PCLK2)
    {
      MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, RCC_HCLK_DIV16);
    }

    /* Set the new HCLK clock divider */
    assert_param(IS_RCC_HCLK(RCC_ClkInitStruct->AHBCLKDivider));
    MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_ClkInitStruct->AHBCLKDivider);
  }
  else
  {
    /* Is intermediate HCLK prescaler 2 applied internally, complete with HCLK prescaler 1 */
    if(hpre == RCC_SYSCLK_DIV2)
    {
      MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_SYSCLK_DIV1);
    }
  }

  /* Decreasing the number of wait states because of lower CPU frequency */
  if (FLatency < __HAL_FLASH_GET_LATENCY())
  {
    /* Program the new number of wait states to the LATENCY bits in the FLASH_ACR register */
    __HAL_FLASH_SET_LATENCY(FLatency);

    /* Check that the new number of wait states is taken into account to access the Flash
    memory by polling the FLASH_ACR register */
    tickstart = HAL_GetTick();

    while (__HAL_FLASH_GET_LATENCY() != FLatency)
    {
      if ((HAL_GetTick() - tickstart) > CLOCKSWITCH_TIMEOUT_VALUE)
      {
        return HAL_TIMEOUT;
      }
    }
  }

  /*-------------------------- PCLK1 Configuration ---------------------------*/
  if (((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_PCLK1) == RCC_CLOCKTYPE_PCLK1)
  {
    assert_param(IS_RCC_PCLK(RCC_ClkInitStruct->APB1CLKDivider));
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_ClkInitStruct->APB1CLKDivider);
  }

  /*-------------------------- PCLK2 Configuration ---------------------------*/
  if(((RCC_ClkInitStruct->ClockType) & RCC_CLOCKTYPE_PCLK2) == RCC_CLOCKTYPE_PCLK2)
  {
    assert_param(IS_RCC_PCLK(RCC_ClkInitStruct->APB2CLKDivider));
    MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, ((RCC_ClkInitStruct->APB2CLKDivider) << 3U));
  }

  /* Update the SystemCoreClock global variable */
  SystemCoreClock = HAL_RCC_GetSysClockFreq() >> (AHBPrescTable[READ_BIT(RCC->CFGR, RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos] & 0x1FU);

  /* Configure the source of time base considering new system clocks settings*/
  return HAL_InitTick(uwTickPrio);
}

/**
  * @brief  Select the clock source to output on MCO pin(PA8).
  * @note   PA8 should be configured in alternate function mode.
  *         The @ref HAL_FLASHEx_OBProgram() API can be used to configure the NRST_MODE Bit value.
  * @param  RCC_MCOx  specifies the output direction for the clock source.
  *           @arg RCC_MCO1_PA8  Clock source to output on MCO1 pin(PA8).
  * @param  RCC_MCOSource  specifies the clock source to output.
  *         This parameter can be one of the following values:
  *           @arg RCC_MCO1SOURCE_NOCLOCK  MCO output disabled, no clock on MCO
  *           @arg RCC_MCO1SOURCE_SYSCLK  system  clock selected as MCO source
  *           @arg RCC_MCO1SOURCE_HSI  HSI clock selected as MCO source
  *           @arg RCC_MCO1SOURCE_HSE  HSE clock selected as MCO source
  *           @arg RCC_MCO1SOURCE_PLLCLK  main PLL clock selected as MCO source
  *           @arg RCC_MCO1SOURCE_LSI  LSI clock selected as MCO source
  * @param  RCC_MCODiv  specifies the MCO prescaler.
  *          This parameter can be one of the following values:
  *           @arg RCC_MCODIV_1   no division applied to MCO clock
  *           @arg RCC_MCODIV_2   division by 2  applied to MCO clock
  *           @arg RCC_MCODIV_4   division by 4  applied to MCO clock
  *           @arg RCC_MCODIV_8   division by 8  applied to MCO clock
  *           @arg RCC_MCODIV_16  division by 16 applied to MCO clock
  * @retval None
  */
void HAL_RCC_MCOConfig(uint32_t RCC_MCOx, uint32_t RCC_MCOSource, uint32_t RCC_MCODiv)
{
  GPIO_InitTypeDef gpio_initstruct;
  uint32_t mcoindex;
  uint32_t mco_gpio_index;
  GPIO_TypeDef * mco_gpio_port;

  /* Check the parameters */
  assert_param(IS_RCC_MCO(RCC_MCOx));

  /* Common GPIO init parameters */
  gpio_initstruct.Mode      = GPIO_MODE_AF_PP;
  gpio_initstruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_initstruct.Pull      = GPIO_NOPULL;

  /* Get MCOx selection */
  mcoindex = RCC_MCOx & RCC_MCO_INDEX_MASK;

  /* Get MCOx GPIO Port */
  mco_gpio_port = (GPIO_TypeDef *) RCC_GET_MCO_GPIO_PORT(RCC_MCOx);

  /* MCOx Clock Enable */
  mco_gpio_index = RCC_GET_MCO_GPIO_INDEX(RCC_MCOx);
  SET_BIT(RCC->AHB2ENR, (1UL << mco_gpio_index ));

  /* Configure the MCOx pin in alternate function mode */
  gpio_initstruct.Pin = RCC_GET_MCO_GPIO_PIN(RCC_MCOx);
  gpio_initstruct.Alternate = RCC_GET_MCO_GPIO_AF(RCC_MCOx);
  HAL_GPIO_Init(mco_gpio_port, &gpio_initstruct);

   if (mcoindex == RCC_MCO1_INDEX)
  {
    assert_param(IS_RCC_MCODIV(RCC_MCODiv));
    assert_param(IS_RCC_MCO1SOURCE(RCC_MCOSource));
    /* Mask MCOSEL[] and MCOPRE[] bits then set MCO clock source and prescaler */
    MODIFY_REG(RCC->CFGR, (RCC_CFGR_MCOSEL | RCC_CFGR_MCOPRE), (RCC_MCOSource | RCC_MCODiv));
  }
}

/**
  * @brief  Return the SYSCLK frequency.
  *
  * @note   The system frequency computed by this function is not the real
  *         frequency in the chip. It is calculated based on the predefined
  *         constant and the selected clock source:
  * @note     If SYSCLK source is HSI, function returns values based on HSI_VALUE(*)
  * @note     If SYSCLK source is HSE, function returns values based on HSE_VALUE(**)
  * @note     If SYSCLK source is PLL, function returns values based on HSE_VALUE(**),
  *           HSI_VALUE(*) Value multiplied/divided by the PLL factors.
  * @note     (*) HSI_VALUE is a constant defined in rx32g4xx_hal_conf.h file (default value
  *               16 MHz) but the real value may vary depending on the variations
  *               in voltage and temperature.
  * @note     (**) HSE_VALUE is a constant defined in rx32g4xx_hal_conf.h file (default value
  *                24 MHz), user has to ensure that HSE_VALUE is same as the real
  *                frequency of the crystal used. Otherwise, this function may
  *                have wrong result.
  *
  * @note   The result of this function could be not correct when using fractional
  *         value for HSE crystal.
  *
  * @note   This function can be used by the user application to compute the
  *         baudrate for the communication peripherals or configure other parameters.
  *
  * @note   Each time SYSCLK changes, this function must be called to update the
  *         right SYSCLK value. Otherwise, any configuration based on this function will be incorrect.
  *
  *
  * @retval SYSCLK frequency
  */
uint32_t HAL_RCC_GetSysClockFreq(void)
{
  uint32_t pllvco, pllsource, pllr;
  uint32_t sysclockfreq;

  if (__HAL_RCC_GET_SYSCLK_SOURCE() == RCC_CFGR_SWS_HSI)
  {
    /* HSI used as system clock source */
    sysclockfreq = HSI_VALUE;
  }
  else if (__HAL_RCC_GET_SYSCLK_SOURCE() == RCC_CFGR_SWS_HSE)
  {
    /* HSE used as system clock source */
    sysclockfreq = HSE_VALUE;
  }
  else if (__HAL_RCC_GET_SYSCLK_SOURCE() == RCC_CFGR_SWS_PLL)
  {
    /* PLL used as system clock  source */

    /* PLL_VCO = (HSE_VALUE or HSI_VALUE) * PLLN
    SYSCLK = PLL_VCO / PLLR
    */
    pllsource = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC);

    switch (pllsource)
    {
    case RCC_PLLSOURCE_HSE:  /* HSE used as PLL clock source */
      pllvco = (HSE_VALUE) * (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos);
      break;

    case RCC_PLLSOURCE_HSI:  /* HSI used as PLL clock source */
    default:
      pllvco = (HSI_VALUE) * (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos);
      break;
    }
    pllr = ((READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLRDIV) >> RCC_PLLCFGR_PLLRDIV_Pos) + 1U ) * 2U;
    sysclockfreq = pllvco/pllr;
  }
  else
  {
    sysclockfreq = 0U;
  }

  return sysclockfreq;
}

/**
  * @brief  Return the HCLK frequency.
  * @note   Each time HCLK changes, this function must be called to update the
  *         right HCLK value. Otherwise, any configuration based on this function will be incorrect.
  *
  * @note   The SystemCoreClock CMSIS variable is used to store System Clock Frequency.
  * @retval HCLK frequency in Hz
  */
uint32_t HAL_RCC_GetHCLKFreq(void)
{
  return SystemCoreClock;
}

/**
  * @brief  Return the PCLK1 frequency.
  * @note   Each time PCLK1 changes, this function must be called to update the
  *         right PCLK1 value. Otherwise, any configuration based on this function will be incorrect.
  * @retval PCLK1 frequency in Hz
  */
uint32_t HAL_RCC_GetPCLK1Freq(void)
{
  /* Get HCLK source and Compute PCLK1 frequency ---------------------------*/
  return (HAL_RCC_GetHCLKFreq() >> (APBPrescTable[READ_BIT(RCC->CFGR, RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos] & 0x1FU));
}

/**
  * @brief  Return the PCLK2 frequency.
  * @note   Each time PCLK2 changes, this function must be called to update the
  *         right PCLK2 value. Otherwise, any configuration based on this function will be incorrect.
  * @retval PCLK2 frequency in Hz
  */
uint32_t HAL_RCC_GetPCLK2Freq(void)
{
  /* Get HCLK source and Compute PCLK2 frequency ---------------------------*/
  return (HAL_RCC_GetHCLKFreq()>> (APBPrescTable[READ_BIT(RCC->CFGR, RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos] & 0x1FU));
}

/**
  * @brief  Configure the RCC_OscInitStruct according to the internal
  *         RCC configuration registers.
  * @param  RCC_OscInitStruct  pointer to an RCC_OscInitTypeDef structure that
  *         will be configured.
  * @retval None
  */
void HAL_RCC_GetOscConfig(RCC_OscInitTypeDef  *RCC_OscInitStruct)
{
  /* Check the parameters */
  assert_param(RCC_OscInitStruct != (void *)NULL);

  RCC_OscInitStruct->OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;

  /* Get the HSE configuration -----------------------------------------------*/
  if(READ_BIT(RCC->CR, RCC_CR_HSEBYP) == RCC_CR_HSEBYP)
  {
    RCC_OscInitStruct->HSEState = RCC_HSE_BYPASS;
  }
  else if(READ_BIT(RCC->CR, RCC_CR_HSEON) == RCC_CR_HSEON)
  {
    RCC_OscInitStruct->HSEState = RCC_HSE_ON;
  }
  else
  {
    RCC_OscInitStruct->HSEState = RCC_HSE_OFF;
  }

  /* Get the HSI configuration -----------------------------------------------*/
  if(READ_BIT(RCC->CR, RCC_CR_HSION) == RCC_CR_HSION)
  {
    RCC_OscInitStruct->HSIState = RCC_HSI_ON;
  }
  else
  {
    RCC_OscInitStruct->HSIState = RCC_HSI_OFF;
  }

  /* Get the LSI configuration -----------------------------------------------*/
  if(READ_BIT(RCC->CSR, RCC_CSR_LSION) == RCC_CSR_LSION)
  {
    RCC_OscInitStruct->LSIState = RCC_LSI_ON;
  }
  else
  {
    RCC_OscInitStruct->LSIState = RCC_LSI_OFF;
  }

  /* Get the PLL configuration -----------------------------------------------*/
  if(READ_BIT(RCC->CR, RCC_CR_PLLON) == RCC_CR_PLLON)
  {
    RCC_OscInitStruct->PLL.PLLState = RCC_PLL_ON;
  }
  else
  {
    RCC_OscInitStruct->PLL.PLLState = RCC_PLL_OFF;
  }
  RCC_OscInitStruct->PLL.PLLSource = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC);

  RCC_OscInitStruct->PLL.PLLN = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos;
  RCC_OscInitStruct->PLL.PLLQ = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLQDIV);
  RCC_OscInitStruct->PLL.PLLR = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLRDIV);
  RCC_OscInitStruct->PLL.PLLP = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLPDIV);
}

/**
  * @brief  Configure the RCC_ClkInitStruct according to the internal
  *         RCC configuration registers.
  * @param  RCC_ClkInitStruct  pointer to an RCC_ClkInitTypeDef structure that
  *         will be configured.
  * @param  pFLatency  Pointer on the Flash Latency.
  * @retval None
  */
void HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef  *RCC_ClkInitStruct, uint32_t *pFLatency)
{
  /* Check the parameters */
  assert_param(RCC_ClkInitStruct != (void  *)NULL);
  assert_param(pFLatency != (void *)NULL);

  /* Set all possible values for the Clock type parameter --------------------*/
  RCC_ClkInitStruct->ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;

  /* Get the SYSCLK configuration --------------------------------------------*/
  RCC_ClkInitStruct->SYSCLKSource = READ_BIT(RCC->CFGR, RCC_CFGR_SW);

  /* Get the HCLK configuration ----------------------------------------------*/
  RCC_ClkInitStruct->AHBCLKDivider = READ_BIT(RCC->CFGR, RCC_CFGR_HPRE);

  /* Get the APB1 configuration ----------------------------------------------*/
  RCC_ClkInitStruct->APB1CLKDivider = READ_BIT(RCC->CFGR, RCC_CFGR_PPRE1);

  /* Get the APB2 configuration ----------------------------------------------*/
  RCC_ClkInitStruct->APB2CLKDivider = (READ_BIT(RCC->CFGR, RCC_CFGR_PPRE2) >> 3U);

  /* Get the Flash Wait State (Latency) configuration ------------------------*/
  *pFLatency = __HAL_FLASH_GET_LATENCY();
}

/**
  * @brief  Enable the Clock Security System.
  * @note   If a failure is detected on the HSE oscillator clock, this oscillator
  *         is automatically disabled and an interrupt is generated to inform the
  *         software about the failure (Clock Security System Interrupt, CSSI),
  *         allowing the MCU to perform rescue operations. The CSSI is linked to
  *         the Cortex-M4 NMI (Non-Maskable Interrupt) exception vector.
  * @note   The Clock Security System can only be cleared by reset.
  * @retval None
  */
void HAL_RCC_EnableCSS(void)
{
  SET_BIT(RCC->CR, RCC_CR_CSSON) ;
}

/**
  * @brief Handle the RCC Clock Security System interrupt request.
  * @note This API should be called under the NMI_Handler().
  * @retval None
  */
void HAL_RCC_NMI_IRQHandler(void)
{
  /* Check RCC CSSF interrupt flag  */
  if(__HAL_RCC_GET_IT(RCC_IT_CSS))
  {
    /* RCC Clock Security System interrupt user callback */
    HAL_RCC_CSSCallback();

    /* Clear RCC CSS pending bit */
    __HAL_RCC_CLEAR_IT(RCC_IT_CSS);
  }
}

/**
  * @brief  Configure PLL used for ADC domain clock
  *
  * @param  PLLP_DIV : PLLP Clock Divider
  *         This parameter can be one of the following values:
  *           @arg RCC_PLLP_DIV2    PLLP division factor = 2
  *           @arg RCC_PLLP_DIV3    PLLP division factor = 3
  *           @arg RCC_PLLP_DIV4    PLLP division factor = 4
  *           @arg RCC_PLLP_DIV5    PLLP division factor = 5
  *           @arg RCC_PLLP_DIV6    PLLP division factor = 6
  *           @arg RCC_PLLP_DIV7    PLLP division factor = 7
  *           @arg RCC_PLLP_DIV8    PLLP division factor = 8
  *           @arg RCC_PLLP_DIV9    PLLP division factor = 9
  *           @arg RCC_PLLP_DIV10   PLLP division factor = 10
  *           @arg RCC_PLLP_DIV11   PLLP division factor = 11
  *           @arg RCC_PLLP_DIV12   PLLP division factor = 12
  *           @arg RCC_PLLP_DIV13   PLLP division factor = 13
  *           @arg RCC_PLLP_DIV14   PLLP division factor = 14
  *           @arg RCC_PLLP_DIV15   PLLP division factor = 15
  *           @arg RCC_PLLP_DIV16   PLLP division factor = 16
  *           @arg RCC_PLLP_DIV17   PLLP division factor = 17
  *           @arg RCC_PLLP_DIV18   PLLP division factor = 18
  *           @arg RCC_PLLP_DIV19   PLLP division factor = 19
  *           @arg RCC_PLLP_DIV20   PLLP division factor = 20
  *           @arg RCC_PLLP_DIV21   PLLP division factor = 21
  *           @arg RCC_PLLP_DIV22   PLLP division factor = 22
  *           @arg RCC_PLLP_DIV23   PLLP division factor = 23
  *           @arg RCC_PLLP_DIV24   PLLP division factor = 24
  *           @arg RCC_PLLP_DIV25   PLLP division factor = 25
  *           @arg RCC_PLLP_DIV26   PLLP division factor = 26
  *           @arg RCC_PLLP_DIV27   PLLP division factor = 27
  *           @arg RCC_PLLP_DIV28   PLLP division factor = 28
  *           @arg RCC_PLLP_DIV29   PLLP division factor = 29
  *           @arg RCC_PLLP_DIV30   PLLP division factor = 30
  *           @arg RCC_PLLP_DIV31   PLLP division factor = 31
  *
  * @note   Max ADC Clock is 64MHz
  * @retval HAL Status
  */

HAL_StatusTypeDef HAL_RCC_SetPLLPClock(uint32_t PLLP_DIV)
{
    HAL_StatusTypeDef result;

    MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLPDIV, PLLP_DIV);

	/* Check moditfy result */
	result = (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLPDIV) == PLLP_DIV) ?  HAL_OK :  HAL_ERROR;

    return result;
}

/**
  * @brief  Configure TIMx Clock multiper.
  *
  * @param  PPRE1_MUL : RCC PPRE1 Clock Multiper
  *         This parameter can be one of the following values:
  *           @arg RCC_PPRE1_MUL1     PPRE1 mulitiplication factor = 1 or 2
  *                                   PCLK1 1x (if RCC_CFGR_PPRE1(PCLK1_DIV)=1)
  *                                   PCLK1 2x (if RCC_CFGR_PPRE1(PCLK1_DIV)>=2)
  *           @arg RCC_PPRE1_MUL4     PPRE1 mulitiplication factor = 4
  *                                   PCLK1 4x (need RCC_CFGR_PPRE1(PCLK1_DIV)>=4 else invalid 2x)
  *           @arg RCC_PPRE1_MUL8     PPRE1 mulitiplication factor = 8
  *                                   PCLK1 8x (need RCC_CFGR_PPRE1(PCLK1_DIV)>=8 else invalid 2x)
  *           @arg RCC_PPRE1_MUL16    PPRE1 mulitiplication factor = 16
  *                                    PCLK1 16x(need RCC_CFGR_PPRE1(PCLK1_DIV)>=16 else invalid 2x)
  *
  * @retval None
  */
void HAL_RCC_SetPPRE1TIM_MUL(uint32_t PPRE1_MUL)
{
	MODIFY_REG(RCC->CFGR1, RCC_CFGR1_PPRE1_TIM_Msk, (PPRE1_MUL << RCC_CFGR1_PPRE1_TIM_Pos) );
}

/**
  * @brief  Configure TIMx and HRTIM Clock multiper.
  *
  * @param  PPRE2_MUL : RCC PPRE2 Clock Multiper
  *         This parameter can be one of the following values:
  *           @arg RCC_PPRE2_MUL1     PPRE2 mulitiplication factor = 1 or 2
  *                                   PCLK2 1x  (if RCC_CFGR_PPRE2(PCLK2_DIV)=1)
  *                                   PCLK2 2x  (if RCC_CFGR_PPRE2(PCLK2_DIV)>=2)
  *           @arg RCC_PPRE2_MUL4     PPRE2 mulitiplication factor = 4
  *                                   PCLK2 4x  (need RCC_CFGR_PPRE2(PCLK2_DIV)>=4 else invalid 2x)
  *           @arg RCC_PPRE2_MUL8     PPRE2 mulitiplication factor = 8
  *                                   PCLK2 8x  (need RCC_CFGR_PPRE2(PCLK2_DIV)>=8 else invalid 2x)
  *           @arg RCC_PPRE2_MUL16    PPRE2 mulitiplication factor = 16
  *                                   PCLK2 16x (need RCC_CFGR_PPRE2(PCLK2_DIV)>=16 else invalid 2x)
  *
  * @retval None
  */
void HAL_RCC_SetPPRE2TIM_MUL(uint32_t PPRE2_MUL)
{
	MODIFY_REG(RCC->CFGR1, RCC_CFGR1_PPRE2_TIM_Msk, (PPRE2_MUL << RCC_CFGR1_PPRE2_TIM_Pos) );
}

/**
  * @brief  RCC Clock Security System interrupt callback.
  * @retval none
  */
__weak void HAL_RCC_CSSCallback(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_RCC_CSSCallback should be implemented in the user file
   */
}


/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Compute SYSCLK frequency based on PLL SYSCLK source.
  * @retval SYSCLK frequency
  */
static uint32_t RCC_GetSysClockFreqFromPLLSource(void)
{
  uint32_t pllvco, pllsource, pllr;
  uint32_t sysclockfreq;

  /* PLL_VCO = (HSE_VALUE or HSI_VALUE) * PLLN
     SYSCLK = PLL_VCO / PLLR
   */
  pllsource = READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC);

  switch (pllsource)
  {
  case RCC_PLLSOURCE_HSE:  /* HSE used as PLL clock source */
    pllvco = HSE_VALUE  * (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos);
    break;

  case RCC_PLLSOURCE_HSI:  /* HSI used as PLL clock source */
  default:
    pllvco = HSI_VALUE * (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos);
    break;
  }

  pllr = ((READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLRDIV) >> RCC_PLLCFGR_PLLRDIV_Pos) + 1U ) * 2U;
  sysclockfreq = pllvco/pllr;

  return sysclockfreq;
}

/**
  * end of RCC_Function_Definitions @}
  */

#endif /* HAL_RCC_MODULE_ENABLED */

/**
  * end of RCC @}
  */
