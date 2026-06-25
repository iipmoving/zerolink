/**************************************************************************//**
 * @file     system_rx32g4xx.c
 * @brief    CMSIS Device System Source File for
 *           rx32g4xx Device
 * @version  V1.0.1
 * @date     15. November 2019
 ******************************************************************************/
/*
 * Copyright (c) 2009-2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define  XTAL            (50000000UL)     /* Oscillator frequency */
#define  SYSTEM_CLOCK    (XTAL / 2U )

/**
*=============================================================================
*-----------------------------------------------------------------------------
*		 System Clock source					| HSI
*-----------------------------------------------------------------------------
*		 SYSCLK(Hz) 							| 72000000
*-----------------------------------------------------------------------------
*		 HCLK(Hz)								| 72000000
*-----------------------------------------------------------------------------
*		 AHB Prescaler							| 1
*-----------------------------------------------------------------------------
*		 APB1 Prescaler 						| 2
*-----------------------------------------------------------------------------
*		 APB2 Prescaler 						| 2
*-----------------------------------------------------------------------------
*		 PLL_N									| 18
*-----------------------------------------------------------------------------
*		 PLL_P									| need to define
*-----------------------------------------------------------------------------
*		 PLL_Q									| need to define
*-----------------------------------------------------------------------------
*		 PLL_R									| 4
*-----------------------------------------------------------------------------
*=============================================================================

**/

#include "rx32g4xx.h"
#include "rx32g4xx_config_def.h"

#if !defined(HSI_VALUE)
#define HSI_VALUE     16000000U            // Value of the Internel oscillator in Hz
#endif

#if !defined(LSI_VALUE)
#define LSI_VALUE     32000U               // Value of the Internel oscillator in Hz
#endif

#if !defined(HSE_VALUE)
#define HSE_VALUE     HSE_USER_DEF_VALUE   // Value of the Externel oscillator in Hz
#endif

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = HSI_VALUE;  /* System Core Clock Frequency */

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
extern const VECTOR_TABLE_Type __VECTOR_TABLE[240];

/*----------------------------------------------------------------------------
  System Core Clock (HCLK) update function
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)
{
}

/*----------------------------------------------------------------------------
  System initialization function
 *----------------------------------------------------------------------------*/
void SystemInit (void)
{

#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
  SCB->VTOR = (uint32_t) &(__VECTOR_TABLE[0]);
#endif

#if defined (__FPU_USED) && (__FPU_USED == 1U)
  SCB->CPACR |= ((3U << 10U*2U) |           /* enable CP10 Full Access */
                 (3U << 11U*2U)  );         /* enable CP11 Full Access */
#endif

#ifdef UNALIGNED_SUPPORT_DISABLE
  SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif

  SystemCoreClock = SYSTEM_CLOCK;
  
  //disable all TIM break IO
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN|RCC_APB2ENR_TIM8EN|RCC_APB2ENR_TIM15EN);
  TIM1->AF1 = 0;
  TIM1->AF2 = 0;
  TIM8->AF1 = 0;
  TIM8->AF2 = 0;
  TIM15->AF1 = 0;
  CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN|RCC_APB2ENR_TIM8EN|RCC_APB2ENR_TIM15EN);
}
