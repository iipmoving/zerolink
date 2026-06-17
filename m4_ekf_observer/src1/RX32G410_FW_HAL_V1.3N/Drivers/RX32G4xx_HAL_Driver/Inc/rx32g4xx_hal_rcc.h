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
*   File    : rx32g4xx_hal_rcc.h
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef RX32G4xx_HAL_RCC_H
#define RX32G4xx_HAL_RCC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"

extern const uint8_t AHBPrescTable[16U];
extern const uint8_t APBPrescTable[8U];
extern const uint8_t TIMPrescTable[5U];

/** @addtogroup RCC
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup RCC_Structure_Definitions RCC Structure Definitions
  * @{
  */

/** @defgroup RCC_PLLInitTypeDef RCC PLLInitTypeDef
  * @ingroup  RCC_Structure_Definitions
  * @{
  */

/**
  * @brief  RCC PLL configuration structure definition
  */
typedef struct
{
  uint32_t PLLState;   /*!< The new state of the PLL.
                            This parameter can be a value of @ref RCC_PLL_Config                      */

  uint32_t PLLSource;  /*!< RCC_PLLSource: PLL entry clock source.
                            This parameter must be a value of @ref RCC_PLL_Clock_Source               */

  uint32_t PLLN;       /*!< PLLN: Multiplication factor for PLL VCO output clock.
                            This parameter must be a number between Min_Data = 8 and Max_Data = 127    */

  uint32_t PLLP;       /*!< PLLP: Division factor for ADC clock.
                            This parameter must be a value of @ref RCC_PLLP_Clock_Divider             */

  uint32_t PLLQ;       /*!< PLLQ: Division factor for SAI, I2S, USB, FDCAN and QUADSPI clocks.
                            This parameter must be a value of @ref RCC_PLLQ_Clock_Divider             */

  uint32_t PLLR;       /*!< PLLR: Division for the main system clock.
                            User have to set the PLLR parameter correctly to not exceed max frequency 192MHZ.
                            This parameter must be a value of @ref RCC_PLLR_Clock_Divider             */

}RCC_PLLInitTypeDef;

/**
  * end of RCC_PLLInitTypeDef @}
  */

/** @defgroup RCC_OscInitTypeDef RCC OscInitTypeDef
  * @ingroup  RCC_Structure_Definitions
  * @{
  */

/**
  * @brief RCC Internal/External Oscillator (HSE, HSI and LSI) configuration structure definition
  */
typedef struct
{
  uint32_t OscillatorType;       /*!< The oscillators to be configured.
                                      This parameter can be a value of @ref RCC_Oscillator_Type                   */

  uint32_t HSEState;             /*!< The new state of the HSE.
                                      This parameter can be a value of @ref RCC_HSE_Config                        */

  uint32_t HSIState;             /*!< The new state of the HSI.
                                      This parameter can be a value of @ref RCC_HSI_Config                        */

  uint32_t LSIState;             /*!< The new state of the LSI.
                                      This parameter can be a value of @ref RCC_LSI_Config                        */

  RCC_PLLInitTypeDef PLL;        /*!< Main PLL structure parameters                                           */

}RCC_OscInitTypeDef;

/**
  * end of RCC_OscInitTypeDef @}
  */

/** @defgroup RCC_ClkInitTypeDef RCC ClkInitTypeDef
  * @ingroup  RCC_Structure_Definitions
  * @{
  */

/**
  * @brief  RCC System, AHB and APB busses clock configuration structure definition
  */
typedef struct
{
  uint32_t ClockType;             /*!< The clock to be configured.
                                       This parameter can be a value of @ref RCC_System_Clock_Type      */

  uint32_t SYSCLKSource;          /*!< The clock source used as system clock (SYSCLK).
                                       This parameter can be a value of @ref RCC_System_Clock_Source    */

  uint32_t AHBCLKDivider;         /*!< The AHB clock (HCLK) divider. This clock is derived from the system clock (SYSCLK).
                                       This parameter can be a value of @ref RCC_AHB_Clock_Source       */

  uint32_t APB1CLKDivider;        /*!< The APB1 clock (PCLK1) divider. This clock is derived from the AHB clock (HCLK).
                                       This parameter can be a value of @ref RCC_APB1_APB2_Clock_Source */

  uint32_t APB2CLKDivider;        /*!< The APB2 clock (PCLK2) divider. This clock is derived from the AHB clock (HCLK).
                                       This parameter can be a value of @ref RCC_APB1_APB2_Clock_Source */

}RCC_ClkInitTypeDef;

/**
  * end of RCC_ClkInitTypeDef @}
  */

/**
  * end of RCC_Structure_Definitions @}
  */

/* Exported constants --------------------------------------------------------*/

/******************************************************************************/
/*                               RCC Parameters                               */
/******************************************************************************/

/** @defgroup RCC_Parameter_Definitions RCC Parameter Definitions
  * @{
  */

/** @defgroup RCC_Timeout_Value Timeout Values
  * @ingroup  RCC_Parameter_Definitions
  * @{
  */

/**
  * @brief  Clock time out value
  */

#define RCC_DBP_TIMEOUT_VALUE          2U                        /* 2 ms (minimum Tick + 1) */

/**
  * end of RCC_Timeout_Value @}
  */


/** @defgroup RCC_Oscillator_Type Oscillator Type
  * @ingroup  RCC_Parameter_Definitions
  * @{
  */
#define RCC_OSCILLATORTYPE_NONE        0x00000000U               /*!< Oscillator configuration unchanged */
#define RCC_OSCILLATORTYPE_HSE         0x00000001U               /*!< HSE to configure */
#define RCC_OSCILLATORTYPE_HSI         0x00000002U               /*!< HSI to configure */
#define RCC_OSCILLATORTYPE_LSI         0x00000008U               /*!< LSI to configure */
/**
  * end of RCC_Oscillator_Type @}
  */

/** @defgroup RCC_HSE_Config HSE Config
  * @ingroup  RCC_Parameter_Definitions
  * @{
  */
#define RCC_HSE_OFF                    0x00000000U                                /*!< HSE clock deactivation */
#define RCC_HSE_ON                     RCC_CR_HSEON                               /*!< HSE clock activation */
#define RCC_HSE_BYPASS                 (RCC_CR_HSEBYP | RCC_CR_HSEON)             /*!< External clock source for HSE clock */
/**
  * end of RCC_HSE_Config @}
  */

/** @defgroup RCC_HSI_Config HSI Config
  * @{
  */
#define RCC_HSI_OFF                    0x00000000U            /*!< HSI clock deactivation */
#define RCC_HSI_ON                     RCC_CR_HSION           /*!< HSI clock activation */
/**
  * end of RCC_HSI_Config @}
  */


/** @defgroup RCC_LSI_Config LSI Config
  * @{
  */
#define RCC_LSI_OFF                    0x00000000U            /*!< LSI clock deactivation */
#define RCC_LSI_ON                     RCC_CSR_LSION          /*!< LSI clock activation */
/**
  * end of RCC_LSI_Config @}
  */

/** @defgroup RCC_PLL_Config PLL Config
  * @{
  */
#define RCC_PLL_NONE                   0x00000000U            /*!< PLL configuration unchanged */
#define RCC_PLL_OFF                    0x00000001U
#define RCC_PLL_ON                     0x00000002U           /*!< PLL activation */
/**
  * end of RCC_PLL_Config @}
  */


/** @defgroup RCC_PLL_Clock_Multiplier PLLP Clock Multiplier
  * @{
  */
#define RCC_PLLCFGR_MUL_6              (                                                RCC_PLLCFGR_PLLN_BIT2 | RCC_PLLCFGR_PLLN_BIT1                        )
#define RCC_PLLCFGR_MUL_7              (                                                RCC_PLLCFGR_PLLN_BIT2 | RCC_PLLCFGR_PLLN_BIT1 | RCC_PLLCFGR_PLLN_BIT0)
#define RCC_PLLCFGR_MUL_8              (                        RCC_PLLCFGR_PLLN_BIT3                                                                        )
#define RCC_PLLCFGR_MUL_9              (                        RCC_PLLCFGR_PLLN_BIT3                                                 | RCC_PLLCFGR_PLLN_BIT0)
#define RCC_PLLCFGR_MUL_10             (                        RCC_PLLCFGR_PLLN_BIT3                         | RCC_PLLCFGR_PLLN_BIT1                        )
#define RCC_PLLCFGR_MUL_11             (                        RCC_PLLCFGR_PLLN_BIT3                         | RCC_PLLCFGR_PLLN_BIT1 | RCC_PLLCFGR_PLLN_BIT0)
#define RCC_PLLCFGR_MUL_12             (                        RCC_PLLCFGR_PLLN_BIT3 | RCC_PLLCFGR_PLLN_BIT2                                                )
#define RCC_PLLCFGR_MUL_13             (                        RCC_PLLCFGR_PLLN_BIT3 | RCC_PLLCFGR_PLLN_BIT2                         | RCC_PLLCFGR_PLLN_BIT0)
#define RCC_PLLCFGR_MUL_14             (                        RCC_PLLCFGR_PLLN_BIT3 | RCC_PLLCFGR_PLLN_BIT2 | RCC_PLLCFGR_PLLN_BIT1                        )
#define RCC_PLLCFGR_MUL_15             (                        RCC_PLLCFGR_PLLN_BIT3 | RCC_PLLCFGR_PLLN_BIT2 | RCC_PLLCFGR_PLLN_BIT1 | RCC_PLLCFGR_PLLN_BIT0)
#define RCC_PLLCFGR_MUL_16             (RCC_PLLCFGR_PLLN_BIT4                                                                                                )
#define RCC_PLLCFGR_MUL_17             (RCC_PLLCFGR_PLLN_BIT4                                                                         | RCC_PLLCFGR_PLLN_BIT0)
#define RCC_PLLCFGR_MUL_18             (RCC_PLLCFGR_PLLN_BIT4                                                 | RCC_PLLCFGR_PLLN_BIT1                        )
#define RCC_PLLCFGR_MUL_19             (RCC_PLLCFGR_PLLN_BIT4                                                 | RCC_PLLCFGR_PLLN_BIT1 | RCC_PLLCFGR_PLLN_BIT0)
#define RCC_PLLCFGR_MUL_20             (RCC_PLLCFGR_PLLN_BIT4                         | RCC_PLLCFGR_PLLN_BIT2                                                )
#define RCC_PLLCFGR_MUL_21             (RCC_PLLCFGR_PLLN_BIT4                         | RCC_PLLCFGR_PLLN_BIT2                         | RCC_PLLCFGR_PLLN_BIT0)
#define RCC_PLLCFGR_MUL_22             (RCC_PLLCFGR_PLLN_BIT4                         | RCC_PLLCFGR_PLLN_BIT2 | RCC_PLLCFGR_PLLN_BIT1                        )
#define RCC_PLLCFGR_MUL_23             (RCC_PLLCFGR_PLLN_BIT4                         | RCC_PLLCFGR_PLLN_BIT2 | RCC_PLLCFGR_PLLN_BIT1 | RCC_PLLCFGR_PLLN_BIT0)
#define RCC_PLLCFGR_MUL_24             (RCC_PLLCFGR_PLLN_BIT4 | RCC_PLLCFGR_PLLN_BIT3                                                                        )
#define RCC_PLLCFGR_MUL_25             (RCC_PLLCFGR_PLLN_BIT4 | RCC_PLLCFGR_PLLN_BIT3                                                 | RCC_PLLCFGR_PLLN_BIT0)

#define RCC_PLLN_MUL6                  RCC_PLLCFGR_MUL_6
#define RCC_PLLN_MUL7                  RCC_PLLCFGR_MUL_8
#define RCC_PLLN_MUL8                  RCC_PLLCFGR_MUL_9
#define RCC_PLLN_MUL9                  RCC_PLLCFGR_MUL_10
#define RCC_PLLN_MUL10                 RCC_PLLCFGR_MUL_11
#define RCC_PLLN_MUL12                 RCC_PLLCFGR_MUL_12
#define RCC_PLLN_MUL13                 RCC_PLLCFGR_MUL_13
#define RCC_PLLN_MUL14                 RCC_PLLCFGR_MUL_14
#define RCC_PLLN_MUL15                 RCC_PLLCFGR_MUL_15
#define RCC_PLLN_MUL16                 RCC_PLLCFGR_MUL_16
#define RCC_PLLN_MUL17                 RCC_PLLCFGR_MUL_17
#define RCC_PLLN_MUL18                 RCC_PLLCFGR_MUL_18
#define RCC_PLLN_MUL19                 RCC_PLLCFGR_MUL_19
#define RCC_PLLN_MUL20                 RCC_PLLCFGR_MUL_20
#define RCC_PLLN_MUL21                 RCC_PLLCFGR_MUL_21
#define RCC_PLLN_MUL22                 RCC_PLLCFGR_MUL_22
#define RCC_PLLN_MUL23                 RCC_PLLCFGR_MUL_23
#define RCC_PLLN_MUL24                 RCC_PLLCFGR_MUL_24
#define RCC_PLLN_MUL25                 RCC_PLLCFGR_MUL_25
/**
  * end of RCC_PLL_Clock_Multiplier @}
  */


/** @defgroup RCC_PLLP_Clock_Divider PLLP Clock Divider
  * @{
  */
#define RCC_PLLCFGR_PLLPDIV_2          (                                                                                 RCC_PLLCFGR_PLLPDIV_BIT1                           )
#define RCC_PLLCFGR_PLLPDIV_3          (                                                                                 RCC_PLLCFGR_PLLPDIV_BIT1 | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_4          (                                                      RCC_PLLCFGR_PLLPDIV_BIT2                                                      )
#define RCC_PLLCFGR_PLLPDIV_5          (                                                      RCC_PLLCFGR_PLLPDIV_BIT2                            | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_6          (                                                      RCC_PLLCFGR_PLLPDIV_BIT2 | RCC_PLLCFGR_PLLPDIV_BIT1                           )
#define RCC_PLLCFGR_PLLPDIV_7          (                                                      RCC_PLLCFGR_PLLPDIV_BIT2 | RCC_PLLCFGR_PLLPDIV_BIT1 | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_8          (                           RCC_PLLCFGR_PLLPDIV_BIT3                                                                                 )
#define RCC_PLLCFGR_PLLPDIV_9          (                           RCC_PLLCFGR_PLLPDIV_BIT3                                                       | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_10         (                           RCC_PLLCFGR_PLLPDIV_BIT3                            | RCC_PLLCFGR_PLLPDIV_BIT1                           )
#define RCC_PLLCFGR_PLLPDIV_11         (                           RCC_PLLCFGR_PLLPDIV_BIT3                            | RCC_PLLCFGR_PLLPDIV_BIT1 | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_12         (                           RCC_PLLCFGR_PLLPDIV_BIT3 | RCC_PLLCFGR_PLLPDIV_BIT2                                                      )
#define RCC_PLLCFGR_PLLPDIV_13         (                           RCC_PLLCFGR_PLLPDIV_BIT3 | RCC_PLLCFGR_PLLPDIV_BIT2                            | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_14         (                           RCC_PLLCFGR_PLLPDIV_BIT3 | RCC_PLLCFGR_PLLPDIV_BIT2 | RCC_PLLCFGR_PLLPDIV_BIT1                           )
#define RCC_PLLCFGR_PLLPDIV_15         (                           RCC_PLLCFGR_PLLPDIV_BIT3 | RCC_PLLCFGR_PLLPDIV_BIT2 | RCC_PLLCFGR_PLLPDIV_BIT1 | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_16         (RCC_PLLCFGR_PLLPDIV_BIT4                                                                                                            )
#define RCC_PLLCFGR_PLLPDIV_17         (RCC_PLLCFGR_PLLPDIV_BIT4                                                                                  | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_18         (RCC_PLLCFGR_PLLPDIV_BIT4                                                       | RCC_PLLCFGR_PLLPDIV_BIT1                           )
#define RCC_PLLCFGR_PLLPDIV_19         (RCC_PLLCFGR_PLLPDIV_BIT4                                                       | RCC_PLLCFGR_PLLPDIV_BIT1 | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_20         (RCC_PLLCFGR_PLLPDIV_BIT4                            | RCC_PLLCFGR_PLLPDIV_BIT2                                                      )
#define RCC_PLLCFGR_PLLPDIV_21         (RCC_PLLCFGR_PLLPDIV_BIT4                            | RCC_PLLCFGR_PLLPDIV_BIT2                            | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_22         (RCC_PLLCFGR_PLLPDIV_BIT4                            | RCC_PLLCFGR_PLLPDIV_BIT2 | RCC_PLLCFGR_PLLPDIV_BIT1                           )
#define RCC_PLLCFGR_PLLPDIV_23         (RCC_PLLCFGR_PLLPDIV_BIT4                            | RCC_PLLCFGR_PLLPDIV_BIT2 | RCC_PLLCFGR_PLLPDIV_BIT1 | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_24         (RCC_PLLCFGR_PLLPDIV_BIT4 | RCC_PLLCFGR_PLLPDIV_BIT3                                                                                 )
#define RCC_PLLCFGR_PLLPDIV_25         (RCC_PLLCFGR_PLLPDIV_BIT4 | RCC_PLLCFGR_PLLPDIV_BIT3                                                       | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_26         (RCC_PLLCFGR_PLLPDIV_BIT4 | RCC_PLLCFGR_PLLPDIV_BIT3                            | RCC_PLLCFGR_PLLPDIV_BIT1                           )
#define RCC_PLLCFGR_PLLPDIV_27         (RCC_PLLCFGR_PLLPDIV_BIT4 | RCC_PLLCFGR_PLLPDIV_BIT3                            | RCC_PLLCFGR_PLLPDIV_BIT1 | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_28         (RCC_PLLCFGR_PLLPDIV_BIT4 | RCC_PLLCFGR_PLLPDIV_BIT3 | RCC_PLLCFGR_PLLPDIV_BIT2                                                      )
#define RCC_PLLCFGR_PLLPDIV_29         (RCC_PLLCFGR_PLLPDIV_BIT4 | RCC_PLLCFGR_PLLPDIV_BIT3 | RCC_PLLCFGR_PLLPDIV_BIT2                            | RCC_PLLCFGR_PLLPDIV_BIT0)
#define RCC_PLLCFGR_PLLPDIV_30         (RCC_PLLCFGR_PLLPDIV_BIT4 | RCC_PLLCFGR_PLLPDIV_BIT3 | RCC_PLLCFGR_PLLPDIV_BIT2 | RCC_PLLCFGR_PLLPDIV_BIT1                           )
#define RCC_PLLCFGR_PLLPDIV_31         (RCC_PLLCFGR_PLLPDIV_BIT4 | RCC_PLLCFGR_PLLPDIV_BIT3 | RCC_PLLCFGR_PLLPDIV_BIT2 | RCC_PLLCFGR_PLLPDIV_BIT1 | RCC_PLLCFGR_PLLPDIV_BIT0)

#define RCC_PLLP_DIV2                  RCC_PLLCFGR_PLLPDIV_2             /*!< PLLP division factor = 2  */
#define RCC_PLLP_DIV3                  RCC_PLLCFGR_PLLPDIV_3             /*!< PLLP division factor = 3  */
#define RCC_PLLP_DIV4                  RCC_PLLCFGR_PLLPDIV_4             /*!< PLLP division factor = 4  */
#define RCC_PLLP_DIV5                  RCC_PLLCFGR_PLLPDIV_5             /*!< PLLP division factor = 5  */
#define RCC_PLLP_DIV6                  RCC_PLLCFGR_PLLPDIV_6             /*!< PLLP division factor = 6  */
#define RCC_PLLP_DIV7                  RCC_PLLCFGR_PLLPDIV_7             /*!< PLLP division factor = 7  */
#define RCC_PLLP_DIV8                  RCC_PLLCFGR_PLLPDIV_8             /*!< PLLP division factor = 8  */
#define RCC_PLLP_DIV9                  RCC_PLLCFGR_PLLPDIV_9             /*!< PLLP division factor = 9  */
#define RCC_PLLP_DIV10                 RCC_PLLCFGR_PLLPDIV_10            /*!< PLLP division factor = 10 */
#define RCC_PLLP_DIV11                 RCC_PLLCFGR_PLLPDIV_11            /*!< PLLP division factor = 11 */
#define RCC_PLLP_DIV12                 RCC_PLLCFGR_PLLPDIV_12            /*!< PLLP division factor = 12 */
#define RCC_PLLP_DIV13                 RCC_PLLCFGR_PLLPDIV_13            /*!< PLLP division factor = 13 */
#define RCC_PLLP_DIV14                 RCC_PLLCFGR_PLLPDIV_14            /*!< PLLP division factor = 14 */
#define RCC_PLLP_DIV15                 RCC_PLLCFGR_PLLPDIV_15            /*!< PLLP division factor = 15 */
#define RCC_PLLP_DIV16                 RCC_PLLCFGR_PLLPDIV_16            /*!< PLLP division factor = 16 */
#define RCC_PLLP_DIV17                 RCC_PLLCFGR_PLLPDIV_17            /*!< PLLP division factor = 17 */
#define RCC_PLLP_DIV18                 RCC_PLLCFGR_PLLPDIV_18            /*!< PLLP division factor = 18 */
#define RCC_PLLP_DIV19                 RCC_PLLCFGR_PLLPDIV_19            /*!< PLLP division factor = 19 */
#define RCC_PLLP_DIV20                 RCC_PLLCFGR_PLLPDIV_20            /*!< PLLP division factor = 20 */
#define RCC_PLLP_DIV21                 RCC_PLLCFGR_PLLPDIV_21            /*!< PLLP division factor = 21 */
#define RCC_PLLP_DIV22                 RCC_PLLCFGR_PLLPDIV_22            /*!< PLLP division factor = 22 */
#define RCC_PLLP_DIV23                 RCC_PLLCFGR_PLLPDIV_23            /*!< PLLP division factor = 23 */
#define RCC_PLLP_DIV24                 RCC_PLLCFGR_PLLPDIV_24            /*!< PLLP division factor = 24 */
#define RCC_PLLP_DIV25                 RCC_PLLCFGR_PLLPDIV_25            /*!< PLLP division factor = 25 */
#define RCC_PLLP_DIV26                 RCC_PLLCFGR_PLLPDIV_26            /*!< PLLP division factor = 26 */
#define RCC_PLLP_DIV27                 RCC_PLLCFGR_PLLPDIV_27            /*!< PLLP division factor = 27 */
#define RCC_PLLP_DIV28                 RCC_PLLCFGR_PLLPDIV_28            /*!< PLLP division factor = 28 */
#define RCC_PLLP_DIV29                 RCC_PLLCFGR_PLLPDIV_29            /*!< PLLP division factor = 29 */
#define RCC_PLLP_DIV30                 RCC_PLLCFGR_PLLPDIV_30            /*!< PLLP division factor = 30 */
#define RCC_PLLP_DIV31                 RCC_PLLCFGR_PLLPDIV_31            /*!< PLLP division factor = 31 */
/**
  * end of RCC_PLLP_Clock_Divider @}
  */


/** @defgroup RCC_PLLQ_Clock_Divider PLLQ Clock Divider
  * @{
  */
#define RCC_PLLCFGR_PLLQDIV_2          (0x00000000U)                                                              /*!< 0x00000000 */
#define RCC_PLLCFGR_PLLQDIV_4          (                         | RCC_PLLCFGR_PLLQDIV_BIT0)                      /*!< 0x00200000 */
#define RCC_PLLCFGR_PLLQDIV_6          (RCC_PLLCFGR_PLLQDIV_BIT1                           )                      /*!< 0x00400000 */
#define RCC_PLLCFGR_PLLQDIV_8          (RCC_PLLCFGR_PLLQDIV_BIT1 | RCC_PLLCFGR_PLLQDIV_BIT0)                      /*!< 0x00600000 */

#define RCC_PLLQ_DIV2                  RCC_PLLCFGR_PLLQDIV_2             /*!< PLLQ division factor = 2 */
#define RCC_PLLQ_DIV4                  RCC_PLLCFGR_PLLQDIV_4             /*!< PLLQ division factor = 4 */
#define RCC_PLLQ_DIV6                  RCC_PLLCFGR_PLLQDIV_6             /*!< PLLQ division factor = 6 */
#define RCC_PLLQ_DIV8                  RCC_PLLCFGR_PLLQDIV_8             /*!< PLLQ division factor = 8 */
/**
  * end of RCC_PLLQ_Clock_Divider @}
  */


/** @defgroup RCC_PLLR_Clock_Divider PLLR Clock Divider
  * @{
  */
#define RCC_PLLCFGR_PLLRDIV_2          (0x00000000U)                                                              /*!< 0x00000000 */
#define RCC_PLLCFGR_PLLRDIV_4          (                           RCC_PLLCFGR_PLLRDIV_BIT0)                      /*!< 0x00200000 */
#define RCC_PLLCFGR_PLLRDIV_6          (RCC_PLLCFGR_PLLRDIV_BIT1                           )                      /*!< 0x00400000 */
#define RCC_PLLCFGR_PLLRDIV_8          (RCC_PLLCFGR_PLLRDIV_BIT1 | RCC_PLLCFGR_PLLRDIV_BIT0)                      /*!< 0x00600000 */

#define RCC_PLLR_DIV2                  RCC_PLLCFGR_PLLRDIV_2             /*!< PLLR division factor = 2 */
#define RCC_PLLR_DIV4                  RCC_PLLCFGR_PLLRDIV_4             /*!< PLLR division factor = 4 */
#define RCC_PLLR_DIV6                  RCC_PLLCFGR_PLLRDIV_6             /*!< PLLR division factor = 6 */
#define RCC_PLLR_DIV8                  RCC_PLLCFGR_PLLRDIV_8             /*!< PLLR division factor = 8 */
/**
  * end of RCC_PLLR_Clock_Divider @}
  */

/******************************************************************************/
/*                               ONLY RX32G410 SUPPORT                        */
/******************************************************************************/

/** @defgroup RCC_CAN_Clock_Divider CAN Clock Divider
  * @{
  */
#define RCC_CFGR_CANPRE_DIV2           (0x00000000U)
#define RCC_CFGR_CANPRE_DIV4           (                                              RCC_CFGR_CANPRE_BIT0)
#define RCC_CFGR_CANPRE_DIV8           (                       RCC_CFGR_CANPRE_BIT1                       )
#define RCC_CFGR_CANPRE_DIV16          (                       RCC_CFGR_CANPRE_BIT1 | RCC_CFGR_CANPRE_BIT0)
#define RCC_CFGR_CANPRE_DIV64          (RCC_CFGR_CANPRE_BIT2                                              )
#define RCC_CFGR_CANPRE_DIV128         (RCC_CFGR_CANPRE_BIT2                        | RCC_CFGR_CANPRE_BIT0)
#define RCC_CFGR_CANPRE_DIV256         (RCC_CFGR_CANPRE_BIT2 | RCC_CFGR_CANPRE_BIT1                       )
#define RCC_CFGR_CANPRE_DIV512         (RCC_CFGR_CANPRE_BIT2 | RCC_CFGR_CANPRE_BIT1 | RCC_CFGR_CANPRE_BIT0)

#define RCC_CANCLK_DIV2                RCC_CFGR_CANPRE_DIV2             /*!< CANPRE divided by 2   */
#define RCC_CANCLK_DIV4                RCC_CFGR_CANPRE_DIV4             /*!< CANPRE divided by 4   */
#define RCC_CANCLK_DIV8                RCC_CFGR_CANPRE_DIV8             /*!< CANPRE divided by 8   */
#define RCC_CANCLK_DIV16               RCC_CFGR_CANPRE_DIV16            /*!< CANPRE divided by 16  */
#define RCC_CANCLK_DIV64               RCC_CFGR_CANPRE_DIV64            /*!< CANPRE divided by 64  */
#define RCC_CANCLK_DIV128              RCC_CFGR_CANPRE_DIV128           /*!< CANPRE divided by 128 */
#define RCC_CANCLK_DIV256              RCC_CFGR_CANPRE_DIV256           /*!< CANPRE divided by 256 */
#define RCC_CANCLK_DIV512              RCC_CFGR_CANPRE_DIV512           /*!< CANPRE divided by 512 */
/**
  * end of RCC_CAN_Clock_Divider @}
  */


/** @defgroup RCC_CAN_Clock_Source CAN Clock Source
  * @{
  */
#define RCC_CFGR_CANSEL_PCLK1          (0x00000000U)                    /*!< CANSEL PCLK   */
#define RCC_CFGR_CANSEL_CANCLK         RCC_CFGR_CANSEL                  /*!< CANSEL CANCLK */
/**
  * end of RCC_CAN_Clock_Source @}
  */

/** @defgroup RCC_PPRE1_Clock_Multiper PPRE1 Clock Multiplier
  * @{
  */
#define RCC_CFGR1_PPRE1_TIM_MUL1       (0x00000000U)
#define RCC_CFGR1_PPRE1_TIM_MUL4       (                           RCC_CFGR1_PPRE1_TIM_BIT0)
#define RCC_CFGR1_PPRE1_TIM_MUL8       (RCC_CFGR1_PPRE1_TIM_BIT1                           )
#define RCC_CFGR1_PPRE1_TIM_MUL16      (RCC_CFGR1_PPRE1_TIM_BIT1 | RCC_CFGR1_PPRE1_TIM_BIT0)

#define RCC_PPRE1_MUL1                 RCC_CFGR1_PPRE1_TIM_MUL1             /*!< PPRE1 mulitiplication factor = 1 or 2  */
#define RCC_PPRE1_MUL4                 RCC_CFGR1_PPRE1_TIM_MUL4             /*!< PPRE1 mulitiplication factor = 4       */
#define RCC_PPRE1_MUL8                 RCC_CFGR1_PPRE1_TIM_MUL8             /*!< PPRE1 mulitiplication factor = 8       */
#define RCC_PPRE1_MUL16                RCC_CFGR1_PPRE1_TIM_MUL16            /*!< PPRE1 mulitiplication factor = 16      */
/**
  * end of RCC_PPRE1_Clock_Multiper @}
  */


/** @defgroup RCC_PPRE2_Clock_Multiper PPRE2 Clock Multiplier
  * @{
  */
#define RCC_CFGR1_PPRE2_TIM_MUL1       (0x00000000U)
#define RCC_CFGR1_PPRE2_TIM_MUL4       (                           RCC_CFGR1_PPRE2_TIM_BIT0)
#define RCC_CFGR1_PPRE2_TIM_MUL8       (RCC_CFGR1_PPRE2_TIM_BIT1                           )
#define RCC_CFGR1_PPRE2_TIM_MUL16      (RCC_CFGR1_PPRE2_TIM_BIT1 | RCC_CFGR1_PPRE2_TIM_BIT0)

#define RCC_PPRE2_MUL1                 RCC_CFGR1_PPRE2_TIM_MUL1             /*!< PPRE2 mulitiplication factor = 1 or 2  */
#define RCC_PPRE2_MUL4                 RCC_CFGR1_PPRE2_TIM_MUL4             /*!< PPRE2 mulitiplication factor = 4       */
#define RCC_PPRE2_MUL8                 RCC_CFGR1_PPRE2_TIM_MUL8             /*!< PPRE2 mulitiplication factor = 8       */
#define RCC_PPRE2_MUL16                RCC_CFGR1_PPRE2_TIM_MUL16            /*!< PPRE2 mulitiplication factor = 16      */
/**
  * end of RCC_PPRE2_Clock_Multiper @}
  */

/** @defgroup RCC_PLL_Clock_Source PLL Clock Source
  * @{
  */
#define RCC_PLLCFGR_PLLSRC_HSI         (RCC_PLLCFGR_PLLSRC_BIT1                          )     /*!< HSI oscillator source clock selected */
#define RCC_PLLCFGR_PLLSRC_HSE         (RCC_PLLCFGR_PLLSRC_BIT1 | RCC_PLLCFGR_PLLSRC_BIT0)     /*!< HSE oscillator source clock selected */


#define RCC_PLLSOURCE_HSI              RCC_PLLCFGR_PLLSRC_HSI  /*!< HSI clock selected as PLL entry clock source */
#define RCC_PLLSOURCE_HSE              RCC_PLLCFGR_PLLSRC_HSE  /*!< HSE clock selected as PLL entry clock source */
/**
  * end of RCC_PLL_Clock_Source @}
  */


/** @defgroup RCC_PLL_Clock_Output PLL Clock Output
  * @{
  */
#define RCC_PLL_ADCCLK                 RCC_PLLCFGR_PLLPEN      /*!< PLLADCCLK selection from main PLL */
#define RCC_PLL_USBCLK                 RCC_PLLCFGR_PLLQEN      /*!< PLL48M1CLK selection from main PLL */
#define RCC_PLL_SYSCLK                 RCC_PLLCFGR_PLLREN      /*!< PLLCLK selection from main PLL */
/**
  * end of RCC_PLL_Clock_Output @}
  */


/** @defgroup RCC_System_Clock_Type System Clock Type
  * @{
  */
#define RCC_CLOCKTYPE_SYSCLK           0x00000001U              /*!< SYSCLK to configure */
#define RCC_CLOCKTYPE_HCLK             0x00000002U              /*!< HCLK to configure */
#define RCC_CLOCKTYPE_PCLK1            0x00000004U              /*!< PCLK1 to configure */
#define RCC_CLOCKTYPE_PCLK2            0x00000008U              /*!< PCLK2 to configure */
/**
  * end of RCC_System_Clock_Type @}
  */


/** @defgroup RCC_System_Clock_Source System Clock Source
  * @{
  */
#define RCC_CFGR_SW_LSI                (0x00000000U)                             /*!< LSI oscillator selection as system clock */
#define RCC_CFGR_SW_HSI                (                   RCC_CFGR_SW_BIT0)     /*!< HSI oscillator selection as system clock */
#define RCC_CFGR_SW_HSE                (RCC_CFGR_SW_BIT1                   )     /*!< HSE oscillator selection as system clock */
#define RCC_CFGR_SW_PLL                (RCC_CFGR_SW_BIT1 | RCC_CFGR_SW_BIT0)     /*!< PLL            selection as system clock */

#define RCC_SYSCLKSOURCE_LSI           RCC_CFGR_SW_LSI    /*!< LSI selection as system clock */
#define RCC_SYSCLKSOURCE_HSI           RCC_CFGR_SW_HSI    /*!< HSI selection as system clock */
#define RCC_SYSCLKSOURCE_HSE           RCC_CFGR_SW_HSE    /*!< HSE selection as system clock */
#define RCC_SYSCLKSOURCE_PLLCLK        RCC_CFGR_SW_PLL    /*!< PLL selection as system clock */
/**
  * end of RCC_System_Clock_Source @}
  */


/** @defgroup RCC_System_Clock_Source_Status System Clock Source Status
  * @{
  */

#define RCC_CFGR_SWS_LSI               (0x00000000U)                              /*!< LSI   oscillator used as system clock */
#define RCC_CFGR_SWS_HSI               (                    RCC_CFGR_SWS_BIT0)    /*!< HSI16 oscillator used as system clock */
#define RCC_CFGR_SWS_HSE               (RCC_CFGR_SWS_BIT1                    )    /*!< HSE   oscillator used as system clock */
#define RCC_CFGR_SWS_PLL               (RCC_CFGR_SWS_BIT1 | RCC_CFGR_SWS_BIT0)    /*!< PLL                used as system clock */

#define RCC_SYSCLKSOURCE_STATUS_LSI    RCC_CFGR_SWS_LSI   /*!< LSI used as system clock */
#define RCC_SYSCLKSOURCE_STATUS_HSI    RCC_CFGR_SWS_HSI   /*!< HSI used as system clock */
#define RCC_SYSCLKSOURCE_STATUS_HSE    RCC_CFGR_SWS_HSE   /*!< HSE used as system clock */
#define RCC_SYSCLKSOURCE_STATUS_PLLCLK RCC_CFGR_SWS_PLL   /*!< PLL used as system clock */
/**
  * end of RCC_System_Clock_Source_Status @}
  */

/** @defgroup RCC_AHB_Clock_Source AHB Clock Source
  * @{
  */

#define RCC_CFGR_HPRE_DIV1             (0x00000000U)                                                                        /*!< HCLK not divided */
#define RCC_CFGR_HPRE_DIV2             (RCC_CFGR_HPRE_BIT3                                                               )  /*!< HCLK divided by 2 */
#define RCC_CFGR_HPRE_DIV4             (RCC_CFGR_HPRE_BIT3 |                                           RCC_CFGR_HPRE_BIT0)  /*!< HCLK divided by 4 */
#define RCC_CFGR_HPRE_DIV8             (RCC_CFGR_HPRE_BIT3 |                      RCC_CFGR_HPRE_BIT1                     )  /*!< HCLK divided by 8 */
#define RCC_CFGR_HPRE_DIV16            (RCC_CFGR_HPRE_BIT3 |                      RCC_CFGR_HPRE_BIT1 | RCC_CFGR_HPRE_BIT0)  /*!< HCLK divided by 16 */
#define RCC_CFGR_HPRE_DIV64            (RCC_CFGR_HPRE_BIT3 | RCC_CFGR_HPRE_BIT2                                          )  /*!< HCLK divided by 64 */
#define RCC_CFGR_HPRE_DIV128           (RCC_CFGR_HPRE_BIT3 | RCC_CFGR_HPRE_BIT2                      | RCC_CFGR_HPRE_BIT0)  /*!< HCLK divided by 128 */
#define RCC_CFGR_HPRE_DIV256           (RCC_CFGR_HPRE_BIT3 | RCC_CFGR_HPRE_BIT2 | RCC_CFGR_HPRE_BIT1                     )  /*!< HCLK divided by 256 */
#define RCC_CFGR_HPRE_DIV512           (RCC_CFGR_HPRE_BIT3 | RCC_CFGR_HPRE_BIT2 | RCC_CFGR_HPRE_BIT1 | RCC_CFGR_HPRE_BIT0)  /*!< HCLK divided by 512 */

#define RCC_SYSCLK_DIV1                RCC_CFGR_HPRE_DIV1   /*!< SYSCLK not divided */
#define RCC_SYSCLK_DIV2                RCC_CFGR_HPRE_DIV2   /*!< SYSCLK divided by 2 */
#define RCC_SYSCLK_DIV4                RCC_CFGR_HPRE_DIV4   /*!< SYSCLK divided by 4 */
#define RCC_SYSCLK_DIV8                RCC_CFGR_HPRE_DIV8   /*!< SYSCLK divided by 8 */
#define RCC_SYSCLK_DIV16               RCC_CFGR_HPRE_DIV16  /*!< SYSCLK divided by 16 */
#define RCC_SYSCLK_DIV64               RCC_CFGR_HPRE_DIV64  /*!< SYSCLK divided by 64 */
#define RCC_SYSCLK_DIV128              RCC_CFGR_HPRE_DIV128 /*!< SYSCLK divided by 128 */
#define RCC_SYSCLK_DIV256              RCC_CFGR_HPRE_DIV256 /*!< SYSCLK divided by 256 */
#define RCC_SYSCLK_DIV512              RCC_CFGR_HPRE_DIV512 /*!< SYSCLK divided by 512 */
/**
  * end of RCC_AHB_Clock_Source @}
  */


/** @defgroup RCC_APB1_APB2_Clock_Source APB1 APB2 Clock Source
  * @{
  */
#define RCC_CFGR_PPRE1_DIV1            (0x00000000U)                                                              /*!< PCLK1 not divided */
#define RCC_CFGR_PPRE1_DIV2            (RCC_CFGR_PPRE1_BIT2                                            )          /*!< PCLK1 divided by 2 */
#define RCC_CFGR_PPRE1_DIV4            (RCC_CFGR_PPRE1_BIT2 |                       RCC_CFGR_PPRE1_BIT0)          /*!< PCLK1 divided by 4 */
#define RCC_CFGR_PPRE1_DIV8            (RCC_CFGR_PPRE1_BIT2 | RCC_CFGR_PPRE1_BIT1                      )          /*!< PCLK1 divided by 8 */
#define RCC_CFGR_PPRE1_DIV16           (RCC_CFGR_PPRE1_BIT2 | RCC_CFGR_PPRE1_BIT1 | RCC_CFGR_PPRE1_BIT0)          /*!< PCLK1 divided by 16 */


#define RCC_HCLK_DIV1                  RCC_CFGR_PPRE1_DIV1  /*!< HCLK not divided */
#define RCC_HCLK_DIV2                  RCC_CFGR_PPRE1_DIV2  /*!< HCLK divided by 2 */
#define RCC_HCLK_DIV4                  RCC_CFGR_PPRE1_DIV4  /*!< HCLK divided by 4 */
#define RCC_HCLK_DIV8                  RCC_CFGR_PPRE1_DIV8  /*!< HCLK divided by 8 */
#define RCC_HCLK_DIV16                 RCC_CFGR_PPRE1_DIV16 /*!< HCLK divided by 16 */
/**
  * end of RCC_APB1_APB2_Clock_Source @}
  */

/** @defgroup RCC_RTC_Clock_Source RTC Clock Source
  * @{
  */
#define RCC_RTCCLKSOURCE_NONE          0x00000000U                  /*!< No clock used as RTC clock */
#define RCC_RTCCLKSOURCE_LSI           RCC_BDCR_RTCSEL_LSI          /*!< LSI oscillator clock used as RTC clock */
#define RCC_RTCCLKSOURCE_HSE_DIV128    RCC_BDCR_RTCSEL_HSE_DIV128   /*!< HSE oscillator clock divided by 32 used as RTC clock */
/**
  * end of RCC_RTC_Clock_Source @}
  */


/** @defgroup RCC_MCO_Index MCO Index
  * @{
  */
/* 32     28      20       16      0
   --------------------------------
   | MCO   | GPIO  | GPIO  | GPIO  |
   | Index |  AF   | Port  |  Pin  |
   -------------------------------*/

#define RCC_MCO_GPIOPORT_POS           16U
#define RCC_MCO_GPIOPORT_MASK          (0xFUL << RCC_MCO_GPIOPORT_POS)
#define RCC_MCO_GPIOAF_POS             20U
#define RCC_MCO_GPIOAF_MASK            (0xFFUL << RCC_MCO_GPIOAF_POS)
#define RCC_MCO_INDEX_POS              28U
#define RCC_MCO_INDEX_MASK             (0x1UL << RCC_MCO_INDEX_POS)
#define RCC_MCO1_INDEX                 (0x0UL << RCC_MCO_INDEX_POS)             /*!< MCO1 index */
#define RCC_MCO1_PA8                   (RCC_MCO1_INDEX | (GPIO_AF0_MCO << RCC_MCO_GPIOAF_POS) | (GPIO_GET_INDEX(GPIOA) << RCC_MCO_GPIOPORT_POS) | GPIO_PIN_8)

/* Legacy Defines*/
#define RCC_MCO1                       RCC_MCO1_PA8
/**
  * end of RCC_MCO_Index @}
  */


/** @defgroup RCC_MCO1_Clock_Source MCO1 Clock Source
  * @{
  */
#define RCC_CFGR_MCOSEL_NOCLK          (0x00000000U)
#define RCC_CFGR_MCOSEL_SYSCLK         (                                              RCC_CFGR_MCOSEL_BIT0)
#define RCC_CFGR_MCOSEL_HSI            (                       RCC_CFGR_MCOSEL_BIT1 | RCC_CFGR_MCOSEL_BIT0)
#define RCC_CFGR_MCOSEL_HSE            (RCC_CFGR_MCOSEL_BIT2                                              )
#define RCC_CFGR_MCOSEL_PLLR           (RCC_CFGR_MCOSEL_BIT2                        | RCC_CFGR_MCOSEL_BIT0)
#define RCC_CFGR_MCOSEL_LSI            (RCC_CFGR_MCOSEL_BIT2 | RCC_CFGR_MCOSEL_BIT1                       )

#define RCC_MCO1SOURCE_NOCLOCK         RCC_CFGR_MCOSEL_NOCLK                  /*!< MCO1 output disabled, no clock on MCO1 */
#define RCC_MCO1SOURCE_SYSCLK          RCC_CFGR_MCOSEL_SYSCLK                 /*!< SYSCLK selection as MCO1 source */
#define RCC_MCO1SOURCE_HSI             RCC_CFGR_MCOSEL_HSI                    /*!< HSI selection as MCO1 source */
#define RCC_MCO1SOURCE_HSE             RCC_CFGR_MCOSEL_HSE                    /*!< HSE selection as MCO1 source */
#define RCC_MCO1SOURCE_PLLCLK          RCC_CFGR_MCOSEL_PLLR                   /*!< PLLCLK selection as MCO1 source */
#define RCC_MCO1SOURCE_LSI             RCC_CFGR_MCOSEL_LSI                    /*!< LSI selection as MCO1 source */
/**
  * end of RCC_MCO1_Clock_Source @}
  */


/** @defgroup RCC_MCOx_Clock_Prescaler MCO1 Clock Prescaler
  * @{
  */
#define RCC_CFGR_MCOPRE_DIV1           (0x00000000U)                                                              /*!< MCO is divided by 1 */
#define RCC_CFGR_MCOPRE_DIV2           (                                            RCC_CFGR_MCOPRE_BIT0)         /*!< MCO is divided by 2 */
#define RCC_CFGR_MCOPRE_DIV4           (                     RCC_CFGR_MCOPRE_BIT1                       )         /*!< MCO is divided by 4 */
#define RCC_CFGR_MCOPRE_DIV8           (                     RCC_CFGR_MCOPRE_BIT1 | RCC_CFGR_MCOPRE_BIT0)         /*!< MCO is divided by 8 */
#define RCC_CFGR_MCOPRE_DIV16          (RCC_CFGR_MCOPRE_BIT2                                            )         /*!< MCO is divided by 16 */

#define RCC_MCODIV_1                   RCC_CFGR_MCOPRE_DIV1     /*!< MCO not divided  */
#define RCC_MCODIV_2                   RCC_CFGR_MCOPRE_DIV2     /*!< MCO divided by 2 */
#define RCC_MCODIV_4                   RCC_CFGR_MCOPRE_DIV4     /*!< MCO divided by 4 */
#define RCC_MCODIV_8                   RCC_CFGR_MCOPRE_DIV8     /*!< MCO divided by 8 */
#define RCC_MCODIV_16                  RCC_CFGR_MCOPRE_DIV16    /*!< MCO divided by 16 */
/**
  * end of RCC_MCOx_Clock_Prescaler @}
  */


/** @defgroup RCC_Interrupt Interrupts
  * @{
  */
#define RCC_IT_LSIRDY                  RCC_CIFR_LSIRDYF      /*!< LSI Ready Interrupt flag */
#define RCC_IT_HSIRDY                  RCC_CIFR_HSIRDYF      /*!< HSI16 Ready Interrupt flag */
#define RCC_IT_HSERDY                  RCC_CIFR_HSERDYF      /*!< HSE Ready Interrupt flag */
#define RCC_IT_PLLRDY                  RCC_CIFR_PLLRDYF      /*!< PLL Ready Interrupt flag */
#define RCC_IT_CSS                     RCC_CIFR_CSSF        /*!< Clock Security System Interrupt flag */
/**
  * @}
  */

/** @defgroup RCC_Flag Flags
  *        Elements values convention: XXXYYYYYb
  *           - YYYYY  : Flag position in the register
  *           - XXX  : Register index
  *                 - 001: CR register
  *                 - 010: BDCR register
  *                 - 011: CSR register
  *                 - 100: CRRCR register
  * @{
  */
/* Flags in the CR register */
#define RCC_FLAG_HSIRDY                ((CR_REG_INDEX << 5U) | RCC_CR_HSIRDY_Pos) /*!< HSI Ready flag */
#define RCC_FLAG_HSERDY                ((CR_REG_INDEX << 5U) | RCC_CR_HSERDY_Pos) /*!< HSE Ready flag */
#define RCC_FLAG_PLLRDY                ((CR_REG_INDEX << 5U) | RCC_CR_PLLRDY_Pos) /*!< PLL Ready flag */

/* Flags in the CSR register */
#define RCC_FLAG_LSIRDY                ((CSR_REG_INDEX << 5U) | RCC_CSR_LSIRDY_Pos)    /*!< LSI Ready flag */
#define RCC_FLAG_OBLRST                ((CSR_REG_INDEX << 5U) | RCC_CSR_OBLRSTF_Pos)   /*!< Option Byte Loader reset flag */
#define RCC_FLAG_PINRST                ((CSR_REG_INDEX << 5U) | RCC_CSR_PINRSTF_Pos)   /*!< PIN reset flag */
#define RCC_FLAG_PORRST                ((CSR_REG_INDEX << 5U) | RCC_CSR_PORRSTF_Pos)   /*!< POR reset flag */
#define RCC_FLAG_SFTRST                ((CSR_REG_INDEX << 5U) | RCC_CSR_SFTRSTF_Pos)   /*!< Software Reset flag */
#define RCC_FLAG_IWDGRST               ((CSR_REG_INDEX << 5U) | RCC_CSR_IWDGRSTF_Pos)  /*!< Independent Watchdog reset flag */
#define RCC_FLAG_WWDGRST               ((CSR_REG_INDEX << 5U) | RCC_CSR_WWDGRSTF_Pos)  /*!< Window watchdog reset flag */
#define RCC_FLAG_LPWRRST               ((CSR_REG_INDEX << 5U) | RCC_CSR_LPWRRSTF_Pos)  /*!< Low-Power reset flag */
/**
  * end of RCC_Flag @}
  */
/**
  * end of RCC_Parameter_Definitions @}
  */

/* Exported macros -----------------------------------------------------------*/

/******************************************************************************/
/*                                RCC Macro                                   */
/******************************************************************************/

/** @defgroup RCC_Macro_Definitions RCC Macro Definitions
  * @{
  */


/**
  * @brief  Enable or disable the AHB1 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @retval None
  */

#define __HAL_RCC_DMA1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_DMA2_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_DMAMUX1_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_CORDIC_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CORDICEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CORDICEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_FMAC_CLK_ENABLE()              do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FMACEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FMACEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_FLASH_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FLASHEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FLASHEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_CRC_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CRCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CRCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_DMA1_CLK_DISABLE()           CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN)

#define __HAL_RCC_DMA2_CLK_DISABLE()           CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN)

#define __HAL_RCC_DMAMUX1_CLK_DISABLE()        CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN)

#define __HAL_RCC_CORDIC_CLK_DISABLE()         CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CORDICEN)

#define __HAL_RCC_FMAC_CLK_DISABLE()           CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FMACEN)

#define __HAL_RCC_FLASH_CLK_DISABLE()          CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FLASHEN)

#define __HAL_RCC_CRC_CLK_DISABLE()            CLEAR_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CRCEN)


/**
  * @brief  Enable or disable the AHB2 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @retval None
  */

#define __HAL_RCC_GPIOA_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_GPIOB_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_GPIOC_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_GPIOD_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_GPIOE_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_GPIOF_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOFEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOFEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_ADC1_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_ADC2_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_ADC3_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC3EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC3EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_DAC1_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_DAC2_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_DAC3_CLK_ENABLE()             do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC3EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC3EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)


#define __HAL_RCC_GPIOA_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN)

#define __HAL_RCC_GPIOB_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN)

#define __HAL_RCC_GPIOC_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN)

#define __HAL_RCC_GPIOD_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN)

#define __HAL_RCC_GPIOE_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN)

#define __HAL_RCC_GPIOF_CLK_DISABLE()          CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOFEN)

#define __HAL_RCC_ADC1_CLK_DISABLE()           CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC1EN)

#define __HAL_RCC_ADC2_CLK_DISABLE()           CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC2EN)

#define __HAL_RCC_ADC3_CLK_DISABLE()           CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC3EN)

#define __HAL_RCC_DAC1_CLK_DISABLE()           CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC1EN)

#define __HAL_RCC_DAC2_CLK_DISABLE()           CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC2EN)

#define __HAL_RCC_DAC3_CLK_DISABLE()           CLEAR_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC3EN)

/**
  * @brief  Enable or disable the APB1 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @retval None
  */

#define __HAL_RCC_TIM2_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_TIM3_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM3EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM3EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_TIM4_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM4EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM4EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_TIM5_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM5EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM5EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_TIM6_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM6EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM6EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_TIM7_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM7EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM7EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_WWDG_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_WWDGEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_WWDGEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_SPI2_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_UART2_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_UART3_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART3EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART3EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_UART4_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART4EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART4EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_UART5_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART5EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART5EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_I2C1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_I2C2_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C2EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C2EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_USB_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USBEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USBEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_CAN_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CANEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CANEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_BKP_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_BKPEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_BKPEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_PWR_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_CLC_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CLCEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CLCEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)


#define __HAL_RCC_TIM2_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM2EN)

#define __HAL_RCC_TIM3_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM3EN)

#define __HAL_RCC_TIM4_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM4EN)

#define __HAL_RCC_TIM5_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM5EN)

#define __HAL_RCC_TIM6_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM6EN)

#define __HAL_RCC_TIM7_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM7EN)

#define __HAL_RCC_WWDG_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_WWDGEN)

#define __HAL_RCC_SPI2_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI2EN)

#define __HAL_RCC_UART2_CLK_DISABLE()         CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART2EN)

#define __HAL_RCC_UART3_CLK_DISABLE()         CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART3EN)

#define __HAL_RCC_UART4_CLK_DISABLE()          CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART4EN)

#define __HAL_RCC_UART5_CLK_DISABLE()          CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART5EN)

#define __HAL_RCC_I2C1_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN)

#define __HAL_RCC_I2C2_CLK_DISABLE()           CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C2EN)

#define __HAL_RCC_USB_CLK_DISABLE()            CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USBEN)

#define __HAL_RCC_CAN_CLK_DISABLE()            CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CANEN)

#define __HAL_RCC_BKP_CLK_DISABLE()            CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_BKPEN)

#define __HAL_RCC_PWR_CLK_DISABLE()            CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN)

#define __HAL_RCC_CLC_CLK_DISABLE()            CLEAR_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CLCEN)

/**
  * @brief  Enable or disable the APB2 peripheral clock.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @retval None
  */

#define __HAL_RCC_SYSCFG_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_TIM1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_SPI1_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_TIM8_CLK_ENABLE()            do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_UART1_CLK_ENABLE()          do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_UART1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_UART1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_TIM15_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_TIM16_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM16EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM16EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_TIM17_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM17EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM17EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_HRTIM1_CLK_ENABLE()           do { \
                                                 __IO uint32_t tmpreg; \
                                                 SET_BIT(RCC->APB2ENR, RCC_APB2ENR_HRTIM1EN); \
                                                 /* Delay after an RCC peripheral clock enabling */ \
                                                 tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_HRTIM1EN); \
                                                 UNUSED(tmpreg); \
                                               } while(0)

#define __HAL_RCC_SYSCFG_CLK_DISABLE()         CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN)

#define __HAL_RCC_TIM1_CLK_DISABLE()           CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN)

#define __HAL_RCC_SPI1_CLK_DISABLE()           CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN)

#define __HAL_RCC_TIM8_CLK_DISABLE()           CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN)

#define __HAL_RCC_UART1_CLK_DISABLE()         CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_UART1EN)

#define __HAL_RCC_TIM15_CLK_DISABLE()          CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN)

#define __HAL_RCC_TIM16_CLK_DISABLE()          CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM16EN)

#define __HAL_RCC_TIM17_CLK_DISABLE()          CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM17EN)

#define __HAL_RCC_HRTIM1_CLK_DISABLE()         CLEAR_BIT(RCC->APB2ENR, RCC_APB2ENR_HRTIM1EN)

/**
  * @brief  Check whether the AHB1 peripheral clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @retval Status
  *         @arg SET  (enabled)
  *         @arg RESET(disabled)
  */

#define __HAL_RCC_DMA1_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN) != 0U)

#define __HAL_RCC_DMA2_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN) != 0U)

#define __HAL_RCC_DMAMUX1_IS_CLK_ENABLED()     (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN) != 0U)

#define __HAL_RCC_CORDIC_IS_CLK_ENABLED()      (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CORDICEN) != 0U)

#define __HAL_RCC_FMAC_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FMACEN) != 0U)

#define __HAL_RCC_FLASH_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FLASHEN) != 0U)

#define __HAL_RCC_CRC_IS_CLK_ENABLED()         (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CRCEN) != 0U)

#define __HAL_RCC_DMA1_IS_CLK_DISABLED()       (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN) == 0U)

#define __HAL_RCC_DMA2_IS_CLK_DISABLED()       (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN) == 0U)

#define __HAL_RCC_DMAMUX1_IS_CLK_DISABLED()    (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_DMAMUX1EN) == 0U)

#define __HAL_RCC_CORDIC_IS_CLK_DISABLED()     (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CORDICEN) == 0U)

#define __HAL_RCC_FMAC_IS_CLK_DISABLED()       (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FMACEN) == 0U)

#define __HAL_RCC_FLASH_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_FLASHEN) == 0U)

#define __HAL_RCC_CRC_IS_CLK_DISABLED()        (READ_BIT(RCC->AHB1ENR, RCC_AHB1ENR_CRCEN) == 0U)

/**
  * @brief  Check whether the AHB2 peripheral clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @retval Status
  *         @arg SET   (enabled)
  *         @arg RESET (disabled)
  */

#define __HAL_RCC_GPIOA_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN) != 0U)

#define __HAL_RCC_GPIOB_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN) != 0U)

#define __HAL_RCC_GPIOC_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN) != 0U)

#define __HAL_RCC_GPIOD_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN) != 0U)

#define __HAL_RCC_GPIOE_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN) != 0U)

#define __HAL_RCC_GPIOF_IS_CLK_ENABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOFEN) != 0U)

#define __HAL_RCC_ADC1_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC1EN) != 0U)

#define __HAL_RCC_ADC2_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC2EN) != 0U)

#define __HAL_RCC_ADC3_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC3EN) != 0U)

#define __HAL_RCC_DAC1_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC1EN) != 0U)

#define __HAL_RCC_DAC2_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC2EN) != 0U)

#define __HAL_RCC_DAC3_IS_CLK_ENABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC3EN) != 0U)

#define __HAL_RCC_GPIOA_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN) == 0U)

#define __HAL_RCC_GPIOB_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOBEN) == 0U)

#define __HAL_RCC_GPIOC_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOCEN) == 0U)

#define __HAL_RCC_GPIOD_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIODEN) == 0U)

#define __HAL_RCC_GPIOE_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOEEN) == 0U)

#define __HAL_RCC_GPIOF_IS_CLK_DISABLED()      (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_GPIOFEN) == 0U)

#define __HAL_RCC_ADC1_IS_CLK_DISABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC1EN) == 0U)

#define __HAL_RCC_ADC2_IS_CLK_DISABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC2EN) == 0U)

#define __HAL_RCC_ADC3_IS_CLK_DISABLED()        (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_ADC3EN) == 0U)

#define __HAL_RCC_DAC1_IS_CLK_DISABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC1EN) == 0U)

#define __HAL_RCC_DAC2_IS_CLK_DISABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC2EN) == 0U)

#define __HAL_RCC_DAC3_IS_CLK_DISABLED()       (READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_DAC3EN) == 0U)

/**
  * @}
  */

/** @defgroup RCC_APB1_Clock_Enable_Disable_Status APB1 Peripheral Clock Enabled or Disabled Status
  * @brief  Check whether the APB1 peripheral clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @retval Status
  *         @arg SET   (enabled)
  *         @arg RESET (disabled)
  */

#define __HAL_RCC_TIM2_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM2EN) != 0U)

#define __HAL_RCC_TIM3_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM3EN) != 0U)

#define __HAL_RCC_TIM4_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM4EN) != 0U)

#define __HAL_RCC_TIM5_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM5EN) != 0U)

#define __HAL_RCC_TIM6_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM6EN) != 0U)

#define __HAL_RCC_TIM7_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM7EN) != 0U)

#define __HAL_RCC_WWDG_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_WWDGEN) != 0U)

#define __HAL_RCC_SPI2_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI2EN) != 0U)

#define __HAL_RCC_UART2_IS_CLK_ENABLED()      (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART2EN) != 0U)

#define __HAL_RCC_UART3_IS_CLK_ENABLED()      (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART3EN) != 0U)

#define __HAL_RCC_UART4_IS_CLK_ENABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART4EN) != 0U)

#define __HAL_RCC_UART5_IS_CLK_ENABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART5EN) != 0U)

#define __HAL_RCC_I2C1_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN) != 0U)

#define __HAL_RCC_I2C2_IS_CLK_ENABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C2EN) != 0U)

#define __HAL_RCC_USB_IS_CLK_ENABLED()         (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USBEN) != 0U)

#define __HAL_RCC_CAN_IS_CLK_ENABLED()         (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CANEN) != 0U)

#define __HAL_RCC_PWR_IS_CLK_ENABLED()         (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN) != 0U)

#define __HAL_RCC_BKP_IS_CLK_ENABLED()         (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_BKPEN) != 0U)

#define __HAL_RCC_CLC_IS_CLK_ENABLED()         (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CLCEN) != 0U)

#define __HAL_RCC_TIM2_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM2EN) == 0U)

#define __HAL_RCC_TIM3_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM3EN) == 0U)

#define __HAL_RCC_TIM4_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM4EN) == 0U)

#define __HAL_RCC_TIM5_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM5EN) == 0U)

#define __HAL_RCC_TIM6_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM6EN) == 0U)

#define __HAL_RCC_TIM7_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_TIM7EN) == 0U)

#define __HAL_RCC_WWDG_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_WWDGEN) == 0U)

#define __HAL_RCC_SPI2_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_SPI2EN) == 0U)

#define __HAL_RCC_UART2_IS_CLK_DISABLED()     (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART2EN) == 0U)

#define __HAL_RCC_UART3_IS_CLK_DISABLED()     (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART3EN) == 0U)

#define __HAL_RCC_UART4_IS_CLK_DISABLED()      (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART4EN) == 0U)

#define __HAL_RCC_UART5_IS_CLK_DISABLED()      (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_UART5EN) == 0U)

#define __HAL_RCC_I2C1_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C1EN) == 0U)

#define __HAL_RCC_I2C2_IS_CLK_DISABLED()       (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_I2C2EN) == 0U)

#define __HAL_RCC_USB_IS_CLK_DISABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_USBEN) == 0U)

#define __HAL_RCC_CAN_IS_CLK_DISABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CANEN) == 0U)

#define __HAL_RCC_PWR_IS_CLK_DISABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_PWREN) == 0U)

#define __HAL_RCC_CLC_IS_CLK_DISABLED()        (READ_BIT(RCC->APB1ENR1, RCC_APB1ENR1_CLCEN) == 0U)

/**
  * @brief  Check whether the APB2 peripheral clock is enabled or not.
  * @note   After reset, the peripheral clock (used for registers read/write access)
  *         is disabled and the application software has to enable this clock before
  *         using it.
  * @retval Status
  *         @arg SET   (enabled)
  *         @arg RESET (disabled)
  */

#define __HAL_RCC_SYSCFG_IS_CLK_ENABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN) != 0U)

#define __HAL_RCC_TIM1_IS_CLK_ENABLED()        (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN) != 0U)

#define __HAL_RCC_SPI1_IS_CLK_ENABLED()        (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN) != 0U)

#define __HAL_RCC_TIM8_IS_CLK_ENABLED()        (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN) != 0U)

#define __HAL_RCC_UART1_IS_CLK_ENABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_UART1EN) != 0U)

#define __HAL_RCC_TIM15_IS_CLK_ENABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN) != 0U)

#define __HAL_RCC_TIM16_IS_CLK_ENABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM16EN) != 0U)

#define __HAL_RCC_TIM17_IS_CLK_ENABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM17EN) != 0U)

#define __HAL_RCC_HRTIM1_IS_CLK_ENABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_HRTIM1EN) != 0U)

#define __HAL_RCC_SYSCFG_IS_CLK_DISABLED()     (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN) == 0U)

#define __HAL_RCC_TIM1_IS_CLK_DISABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN) == 0U)

#define __HAL_RCC_SPI1_IS_CLK_DISABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN) == 0U)

#define __HAL_RCC_TIM8_IS_CLK_DISABLED()       (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM8EN) == 0U)

#define __HAL_RCC_UART1_IS_CLK_DISABLED()     (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_UART1EN) == 0U)

#define __HAL_RCC_TIM15_IS_CLK_DISABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM15EN) == 0U)

#define __HAL_RCC_TIM16_IS_CLK_DISABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM16EN) == 0U)

#define __HAL_RCC_TIM17_IS_CLK_DISABLED()      (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM17EN) == 0U)

#define __HAL_RCC_HRTIM1_IS_CLK_DISABLED()     (READ_BIT(RCC->APB2ENR, RCC_APB2ENR_HRTIM1EN) == 0U)

/**
  * @brief  Force or release AHB1 peripheral reset.
  * @retval None
  */
#define __HAL_RCC_AHB1_FORCE_RESET()           WRITE_REG(RCC->AHB1RSTR, 0xFFFFFFFFU)

#define __HAL_RCC_DMA1_FORCE_RESET()           SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMA1RST)

#define __HAL_RCC_DMA2_FORCE_RESET()           SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMA2RST)

#define __HAL_RCC_DMAMUX1_FORCE_RESET()        SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMAMUX1RST)

#define __HAL_RCC_CORDIC_FORCE_RESET()         SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_CORDICRST)

#define __HAL_RCC_FMAC_FORCE_RESET()           SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_FMACRST)

#define __HAL_RCC_CRC_FORCE_RESET()            SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_CRCRST)


#define __HAL_RCC_AHB1_RELEASE_RESET()         WRITE_REG(RCC->AHB1RSTR, 0x00000000U)

#define __HAL_RCC_DMA1_RELEASE_RESET()         CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMA1RST)

#define __HAL_RCC_DMA2_RELEASE_RESET()         CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMA2RST)

#define __HAL_RCC_DMAMUX1_RELEASE_RESET()      CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_DMAMUX1RST)

#define __HAL_RCC_CORDIC_RELEASE_RESET()       CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_CORDICRST)

#define __HAL_RCC_FMAC_RELEASE_RESET()         CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_FMACRST)

#define __HAL_RCC_CRC_RELEASE_RESET()          CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_CRCRST)

/**
  * @brief  Force or release AHB2 peripheral reset.
  * @retval None
  */
#define __HAL_RCC_AHB2_FORCE_RESET()           WRITE_REG(RCC->AHB2RSTR, 0xFFFFFFFFU)

#define __HAL_RCC_GPIOA_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOARST)

#define __HAL_RCC_GPIOB_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOBRST)

#define __HAL_RCC_GPIOC_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOCRST)

#define __HAL_RCC_GPIOD_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIODRST)

#define __HAL_RCC_GPIOE_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOERST)

#define __HAL_RCC_GPIOF_FORCE_RESET()          SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOFRST)

#define __HAL_RCC_ADC1_FORCE_RESET()           SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_ADC1RST)

#define __HAL_RCC_ADC2_FORCE_RESET()           SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_ADC2RST)

#define __HAL_RCC_ADC3_FORCE_RESET()           SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_ADC3RST)

#define __HAL_RCC_DAC1_FORCE_RESET()           SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_DAC1RST)

#define __HAL_RCC_DAC2_FORCE_RESET()           SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_DAC2RST)

#define __HAL_RCC_DAC3_FORCE_RESET()           SET_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_DAC3RST)


#define __HAL_RCC_AHB2_RELEASE_RESET()         WRITE_REG(RCC->AHB2RSTR, 0x00000000U)

#define __HAL_RCC_GPIOA_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOARST)

#define __HAL_RCC_GPIOB_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOBRST)

#define __HAL_RCC_GPIOC_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOCRST)

#define __HAL_RCC_GPIOD_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIODRST)

#define __HAL_RCC_GPIOE_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOERST)

#define __HAL_RCC_GPIOF_RELEASE_RESET()        CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_GPIOFRST)

#define __HAL_RCC_ADC1_RELEASE_RESET()         CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_ADC1RST)

#define __HAL_RCC_ADC2_RELEASE_RESET()         CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_ADC2RST)

#define __HAL_RCC_ADC3_RELEASE_RESET()         CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_ADC3RST)

#define __HAL_RCC_DAC1_RELEASE_RESET()         CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_DAC1RST)

#define __HAL_RCC_DAC2_RELEASE_RESET()         CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_DAC2RST)

#define __HAL_RCC_DAC3_RELEASE_RESET()         CLEAR_BIT(RCC->AHB2RSTR, RCC_AHB2RSTR_DAC3RST)

/** @defgroup RCC_APB1_Force_Release_Reset APB1 Peripheral Force Release Reset
  * @brief  Force or release APB1 peripheral reset.
  * @retval None
  */
#define __HAL_RCC_APB1_FORCE_RESET()           WRITE_REG(RCC->APB1RSTR, 0xFFFFFFFFU)

#define __HAL_RCC_TIM2_FORCE_RESET()           SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_TIM2RST)

#define __HAL_RCC_TIM3_FORCE_RESET()           SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_TIM3RST)

#define __HAL_RCC_TIM4_FORCE_RESET()           SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_TIM4RST)

#define __HAL_RCC_TIM5_FORCE_RESET()           SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_TIM5RST)

#define __HAL_RCC_TIM6_FORCE_RESET()           SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_TIM6RST)

#define __HAL_RCC_TIM7_FORCE_RESET()           SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_TIM7RST)

#define __HAL_RCC_SPI2_FORCE_RESET()           SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_SPI2RST)

#define __HAL_RCC_UART2_FORCE_RESET()         SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_UART2RST)

#define __HAL_RCC_UART3_FORCE_RESET()         SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_UART3RST)

#define __HAL_RCC_UART4_FORCE_RESET()          SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_UART4RST)

#define __HAL_RCC_UART5_FORCE_RESET()          SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_UART5RST)

#define __HAL_RCC_I2C1_FORCE_RESET()           SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_I2C1RST)

#define __HAL_RCC_I2C2_FORCE_RESET()           SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_I2C2RST)

#define __HAL_RCC_USB_FORCE_RESET()            SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_USBRST)

#define __HAL_RCC_CAN_FORCE_RESET()            SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_CANRST)

#define __HAL_RCC_PWR_FORCE_RESET()            SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_PWRRST)

#define __HAL_RCC_CLC_FORCE_RESET()            SET_BIT(RCC->APB1RSTR, RCC_APB1RSTR_CLCRRST)


#define __HAL_RCC_APB1_RELEASE_RESET()         WRITE_REG(RCC->APB1RSTR, 0x00000000U)

#define __HAL_RCC_TIM2_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_TIM2RST)

#define __HAL_RCC_TIM3_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_TIM3RST)

#define __HAL_RCC_TIM4_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_TIM4RST)

#define __HAL_RCC_TIM5_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_TIM5RST)

#define __HAL_RCC_TIM6_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_TIM6RST)

#define __HAL_RCC_TIM7_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_TIM7RST)

#define __HAL_RCC_SPI2_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_SPI2RST)

#define __HAL_RCC_UART2_RELEASE_RESET()       CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_UART2RST)

#define __HAL_RCC_UART3_RELEASE_RESET()       CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_UART3RST)

#define __HAL_RCC_UART4_RELEASE_RESET()        CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_UART4RST)

#define __HAL_RCC_UART5_RELEASE_RESET()        CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_UART5RST)

#define __HAL_RCC_I2C1_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_I2C1RST)

#define __HAL_RCC_I2C2_RELEASE_RESET()         CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_I2C2RST)

#define __HAL_RCC_USB_RELEASE_RESET()          CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_USBRST)

#define __HAL_RCC_CAN_RELEASE_RESET()          CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_CANRST)

#define __HAL_RCC_PWR_RELEASE_RESET()          CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_PWRRST)

#define __HAL_RCC_CLC_RELEASE_RESET()          CLEAR_BIT(RCC->APB1RSTR, RCC_APB1RSTR_CLCRRST)

/**
  * @brief  Force or release APB2 peripheral reset.
  * @retval None
  */
#define __HAL_RCC_APB2_FORCE_RESET()           WRITE_REG(RCC->APB2RSTR, 0xFFFFFFFFU)

#define __HAL_RCC_SYSCFG_FORCE_RESET()         SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SYSCFGRST)

#define __HAL_RCC_TIM1_FORCE_RESET()           SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM1RST)

#define __HAL_RCC_SPI1_FORCE_RESET()           SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SPI1RST)

#define __HAL_RCC_TIM8_FORCE_RESET()           SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM8RST)

#define __HAL_RCC_UART1_FORCE_RESET()         SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_UART1RST)

#define __HAL_RCC_TIM15_FORCE_RESET()          SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM15RST)

#define __HAL_RCC_TIM16_FORCE_RESET()          SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM16RST)

#define __HAL_RCC_TIM17_FORCE_RESET()          SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM17RST)

#define __HAL_RCC_HRTIM1_FORCE_RESET()         SET_BIT(RCC->APB2RSTR, RCC_APB2RSTR_HRTIM1RST)


#define __HAL_RCC_APB2_RELEASE_RESET()         WRITE_REG(RCC->APB2RSTR, 0x00000000U)

#define __HAL_RCC_SYSCFG_RELEASE_RESET()       CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SYSCFGRST)

#define __HAL_RCC_TIM1_RELEASE_RESET()         CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM1RST)

#define __HAL_RCC_SPI1_RELEASE_RESET()         CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_SPI1RST)

#define __HAL_RCC_TIM8_RELEASE_RESET()         CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM8RST)

#define __HAL_RCC_UART1_RELEASE_RESET()       CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_UART1RST)

#define __HAL_RCC_TIM15_RELEASE_RESET()        CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM15RST)

#define __HAL_RCC_TIM16_RELEASE_RESET()        CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM16RST)

#define __HAL_RCC_TIM17_RELEASE_RESET()        CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_TIM17RST)

#define __HAL_RCC_HRTIM1_RELEASE_RESET()       CLEAR_BIT(RCC->APB2RSTR, RCC_APB2RSTR_HRTIM1RST)

/**
  * @brief  Macros to force or release the Backup domain reset.
  * @note   This function resets the RTC peripheral (including the backup registers)
  *         and the RTC clock source selection in RCC_CSR register.
  * @note   The BKPSRAM is not affected by this reset.
  * @retval None
  */
#define __HAL_RCC_BACKUPRESET_FORCE()   SET_BIT(RCC->BDCR, RCC_BDCR_BDRST)

#define __HAL_RCC_BACKUPRESET_RELEASE() CLEAR_BIT(RCC->BDCR, RCC_BDCR_BDRST)

/**
  * @brief  Macros to enable or disable the RTC clock.
  * @note   As the RTC is in the Backup domain and write access is denied to
  *         this domain after reset, you have to enable write access using
  *         HAL_PWR_EnableBkUpAccess() function before to configure the RTC
  *         (to be done once after reset).
  * @note   These macros must be used after the RTC clock source was selected.
  * @retval None
  */
#define __HAL_RCC_RTC_ENABLE()         SET_BIT(RCC->BDCR, RCC_BDCR_RTCEN)

#define __HAL_RCC_RTC_DISABLE()        CLEAR_BIT(RCC->BDCR, RCC_BDCR_RTCEN)

/**
  * @brief  Macros to enable or disable the Internal High Speed 16MHz oscillator (HSI).
  * @note   The HSI is stopped by hardware when entering STOP and STANDBY modes.
  *         It is used (enabled by hardware) as system clock source after startup
  *         from Reset, wakeup from STOP and STANDBY mode, or in case of failure
  *         of the HSE used directly or indirectly as system clock (if the Clock
  *         Security System CSS is enabled).
  * @note   HSI can not be stopped if it is used as system clock source. In this case,
  *         you have to select another source of the system clock then stop the HSI.
  * @note   After enabling the HSI, the application software should wait on HSIRDY
  *         flag to be set indicating that HSI clock is stable and can be used as
  *         system clock source.
  *         This parameter can be: ENABLE or DISABLE.
  * @note   When the HSI is stopped, HSIRDY flag goes low after 6 HSI oscillator
  *         clock cycles.
  * @retval None
  */
#define __HAL_RCC_HSI_ENABLE()  SET_BIT(RCC->CR, RCC_CR_HSION)

#define __HAL_RCC_HSI_DISABLE() CLEAR_BIT(RCC->CR, RCC_CR_HSION)

/**
  * @brief  Macros to enable or disable the Internal Low Speed oscillator (LSI).
  * @note   After enabling the LSI, the application software should wait on
  *         LSIRDY flag to be set indicating that LSI clock is stable and can
  *         be used to clock the IWDG and/or the RTC.
  * @note   LSI can not be disabled if the IWDG is running.
  * @note   When the LSI is stopped, LSIRDY flag goes low after 6 LSI oscillator
  *         clock cycles.
  * @retval None
  */
#define __HAL_RCC_LSI_ENABLE()         SET_BIT(RCC->CSR, RCC_CSR_LSION)

#define __HAL_RCC_LSI_DISABLE()        CLEAR_BIT(RCC->CSR, RCC_CSR_LSION)

/**
  * @brief  Macro to configure the External High Speed oscillator (HSE).
  * @note   Transition HSE Bypass to HSE On and HSE On to HSE Bypass are not
  *         supported by this macro. User should request a transition to HSE Off
  *         first and then HSE On or HSE Bypass.
  * @note   After enabling the HSE (RCC_HSE_ON or RCC_HSE_Bypass), the application
  *         software should wait on HSERDY flag to be set indicating that HSE clock
  *         is stable and can be used to clock the PLL and/or system clock.
  * @note   HSE state can not be changed if it is used directly or through the
  *         PLL as system clock. In this case, you have to select another source
  *         of the system clock then change the HSE state (ex. disable it).
  * @note   The HSE is stopped by hardware when entering STOP and STANDBY modes.
  * @note   This function reset the CSSON bit, so if the clock security system(CSS)
  *         was previously enabled you have to enable it again after calling this
  *         function.
  * @param  __STATE__ specifies the new state of the HSE.
  *         This parameter can be one of the following values:
  *           @arg RCC_HSE_OFF  Turn OFF the HSE oscillator, HSERDY flag goes low after
  *                                  6 HSE oscillator clock cycles.
  *           @arg RCC_HSE_ON  Turn ON the HSE oscillator.
  *           @arg RCC_HSE_BYPASS  HSE oscillator bypassed with external clock.
  * @retval None
  */
#define __HAL_RCC_HSE_CONFIG(__STATE__)                                    \
                    do {                                                   \
                      if((__STATE__) == RCC_HSE_ON)                        \
                      {                                                    \
                        SET_BIT(RCC->CR, RCC_CR_HSEBIAS);                  \
                        SET_BIT(RCC->CR, RCC_CR_HSEON);                    \
                      }                                                    \
                      else if((__STATE__) == RCC_HSE_BYPASS)               \
                      {                                                    \
                        SET_BIT(RCC->CR, RCC_CR_HSEBYP);                   \
                        SET_BIT(RCC->CR, RCC_CR_HSEBIAS);                  \
                        SET_BIT(RCC->CR, RCC_CR_HSEON);                    \
                      }                                                    \
                      else                                                 \
                      {                                                    \
                        CLEAR_BIT(RCC->CR, RCC_CR_HSEON);                  \
                        CLEAR_BIT(RCC->CR, RCC_CR_HSEBYP);                 \
                        CLEAR_BIT(RCC->CR, RCC_CR_HSEBIAS);                \
                      }                                                    \
                    } while(0)

/**
  * @brief  Macros to configure the RTC clock (RTCCLK).
  * @note   As the RTC clock configuration bits are in the Backup domain and write
  *         access is denied to this domain after reset, you have to enable write
  *         access using the Power Backup Access macro before to configure
  *         the RTC clock source (to be done once after reset).
  * @note   Once the RTC clock is configured it cannot be changed unless the
  *         Backup domain is reset using __HAL_RCC_BACKUPRESET_FORCE() macro, or by
  *         a Power On Reset (POR).
  *
  * @param  __RTC_CLKSOURCE__ specifies the RTC clock source.
  *         This parameter can be one of the following values:
  *           @arg RCC_RTCCLKSOURCE_NONE  No clock selected as RTC clock.
  *           @arg RCC_RTCCLKSOURCE_LSI  LSI selected as RTC clock.
  *           @arg RCC_RTCCLKSOURCE_HSE_DIV128  HSE clock divided by 128 selected
  *
  * @note   If the LSI is used as RTC clock source, the RTC continues to
  *         work in STOP and STANDBY modes, and can be used as wakeup source.
  *         However, when the HSE clock is used as RTC clock source, the RTC
  *         cannot be used in STOP and STANDBY modes.
  * @note   The maximum input clock frequency for RTC is 1MHz (when using HSE as
  *         RTC clock source).
  * @retval None
  */
#define __HAL_RCC_RTC_CONFIG(__RTC_CLKSOURCE__)  \
                  MODIFY_REG( RCC->BDCR, RCC_BDCR_RTCSEL, (__RTC_CLKSOURCE__))

/**
  * @brief  Macro to get the RTC clock source.
  * @retval The returned value can be one of the following:
  *           @arg RCC_RTCCLKSOURCE_NONE  No clock selected as RTC clock.
  *           @arg RCC_RTCCLKSOURCE_LSI  LSI selected as RTC clock.
  *           @arg RCC_RTCCLKSOURCE_HSE_DIV128  HSE clock divided by 128 selected
  */
#define  __HAL_RCC_GET_RTC_SOURCE() (READ_BIT(RCC->BDCR, RCC_BDCR_RTCSEL))

/**
  * @brief  Macros to enable or disable the main PLL.
  * @note   After enabling the main PLL, the application software should wait on
  *         PLLRDY flag to be set indicating that PLL clock is stable and can
  *         be used as system clock source.
  * @note   The main PLL can not be disabled if it is used as system clock source
  * @note   The main PLL is disabled by hardware when entering STOP and STANDBY modes.
  * @retval None
  */
#define __HAL_RCC_PLL_ENABLE()         SET_BIT(RCC->CR, RCC_CR_PLLON)

#define __HAL_RCC_PLL_DISABLE()        CLEAR_BIT(RCC->CR, RCC_CR_PLLON)

/**
  * @brief  Macro to configure the PLL clock source.
  * @note   This function must be used only when the main PLL is disabled.
  * @param  __PLLSOURCE__ specifies the PLL entry clock source.
  *         This parameter can be one of the following values:
  *           @arg RCC_PLLSOURCE_NONE  No clock selected as PLL clock entry
  *           @arg RCC_PLLSOURCE_HSI  HSI oscillator clock selected as PLL clock entry
  *           @arg RCC_PLLSOURCE_HSE  HSE oscillator clock selected as PLL clock entry
  * @retval None
  *
  */
#define __HAL_RCC_PLL_PLLSOURCE_CONFIG(__PLLSOURCE__) \
                  MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC, (__PLLSOURCE__))

/**
  * @brief  Macro to configure the main PLL clock source, multiplication and division factors.
  * @note   This macro must be used only when the main PLL is disabled.
  * @note   This macro preserves the PLL's output clocks enable state.
  *
  * @param  __PLLSOURCE__ specifies the PLL entry clock source.
  *         This parameter can be one of the following values:
  *           @arg RCC_PLLSOURCE_NONE  No clock selected as PLL clock entry
  *           @arg RCC_PLLSOURCE_HSI  HSI oscillator clock selected as PLL clock entry
  *           @arg RCC_PLLSOURCE_HSE  HSE oscillator clock selected as PLL clock entry
  *
  * @param  __PLLN__ specifies the multiplication factor for PLL VCO output clock.
  *          This parameter must be a number between 8 and 127.
  * @note   You have to set the PLLN parameter correctly to ensure that the VCO
  *         output frequency is between 64 and 344 MHz.
  *
  * @param  __PLLP__ specifies the division factor for ADC clock.
  *          This parameter must be a number in the range (2 to 31).
  *
  * @param  __PLLQ__ specifies the division factor for OTG FS, SDMMC1 and RNG clocks.
  *          This parameter must be in the range (2, 4, 6 or 8).
  * @note   If the USB OTG FS is used in your application, you have to set the
  *         PLLQ parameter correctly to have 48 MHz clock for the USB. However,
  *         the SDMMC1 and RNG need a frequency lower than or equal to 48 MHz to work
  *         correctly.
  * @param  __PLLR__ specifies the division factor for the main system clock.
  * @note   You have to set the PLLR parameter correctly to not exceed 170MHZ.
  *          This parameter must be in the range (2, 4, 6 or 8).
  * @retval None
  */
#define __HAL_RCC_PLL_CONFIG(__PLLSOURCE__, __PLLN__, __PLLP__, __PLLQ__,__PLLR__ )              \
                               MODIFY_REG(RCC->PLLCFGR,                                          \
                              (RCC_PLLCFGR_PLLSRC  | RCC_PLLCFGR_PLLN    |                       \
                               RCC_PLLCFGR_PLLQDIV | RCC_PLLCFGR_PLLRDIV | RCC_PLLCFGR_PLLPDIV), \
                              ((__PLLSOURCE__) |                                                 \
                               ((__PLLN__) << RCC_PLLCFGR_PLLN_Pos) |                            \
                               (__PLLQ__) | (__PLLR__) | (__PLLP__)))

/**
  * @brief  Macro to get the oscillator used as PLL clock source.
  * @retval The oscillator used as PLL clock source. The returned value can be one
  *         of the following:
  *           @arg RCC_PLLSOURCE_NONE: No oscillator is used as PLL clock source.
  *           @arg RCC_PLLSOURCE_HSI: HSI oscillator is used as PLL clock source.
  *           @arg RCC_PLLSOURCE_HSE: HSE oscillator is used as PLL clock source.
  */
#define __HAL_RCC_GET_PLL_OSCSOURCE() (READ_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC))

/**
  * @brief  Enable or disable each clock output (RCC_PLL_SYSCLK, RCC_PLL_USBCLK, RCC_PLL_ADCCLK)
  * @note   Enabling/disabling clock outputs RCC_PLL_ADCCLK and RCC_PLL_USBCLK can be done at anytime
  *         without the need to stop the PLL in order to save power. But RCC_PLL_SYSCLK cannot
  *         be stopped if used as System Clock.
  * @param  __PLLCLOCKOUT__ specifies the PLL clock to be output.
  *         This parameter can be one or a combination of the following values:
  *           @arg RCC_PLL_ADCCLK  This clock is used to generate a clock on ADC.
  *           @arg RCC_PLL_USBCLK  This Clock is used to generate the clock for the USB (48 MHz),
  *           @arg RCC_PLL_SYSCLK  This Clock is used to generate the high speed system clock (up to 192MHz)
  * @retval None
  */
#define __HAL_RCC_PLLCLKOUT_ENABLE(__PLLCLOCKOUT__)   SET_BIT(RCC->PLLCFGR, (__PLLCLOCKOUT__))

#define __HAL_RCC_PLLCLKOUT_DISABLE(__PLLCLOCKOUT__)  CLEAR_BIT(RCC->PLLCFGR, (__PLLCLOCKOUT__))

/**
  * @brief  Get clock output enable status (RCC_PLL_SYSCLK, RCC_PLL_USBCLK)
  * @param  __PLLCLOCKOUT__ specifies the output PLL clock to be checked.
  *         This parameter can be one of the following values:
  *           @arg RCC_PLL_ADCCLK  This clock is used to generate a clock on ADC.
  *           @arg RCC_PLL_USBCLK  This Clock is used to generate the clock for the USB (48 MHz),
  *           @arg RCC_PLL_SYSCLK  This Clock is used to generate the high speed system clock (up to 192MHz)
  * @retval SET  (Enabled)
  *         RESET(Disabled)
  */
#define __HAL_RCC_GET_PLLCLKOUT_CONFIG(__PLLCLOCKOUT__)  READ_BIT(RCC->PLLCFGR, (__PLLCLOCKOUT__))

/**
  * @brief  Macro to configure the system clock source.
  * @param  __SYSCLKSOURCE__ specifies the system clock source.
  *         This parameter can be one of the following values:
  *           @arg RCC_SYSCLKSOURCE_LSI: LSI oscillator is used as system clock source.
  *           @arg RCC_SYSCLKSOURCE_HSI: HSI oscillator is used as system clock source.
  *           @arg RCC_SYSCLKSOURCE_HSE: HSE oscillator is used as system clock source.
  *           @arg RCC_SYSCLKSOURCE_PLLCLK: PLL output is used as system clock source.
  * @retval None
  */
#define __HAL_RCC_SYSCLK_CONFIG(__SYSCLKSOURCE__) \
                  MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, (__SYSCLKSOURCE__))

/**
  * @brief  Macro to get the clock source used as system clock.
  * @retval The clock source used as system clock. The returned value can be one
  *         of the following:
  *           @arg RCC_SYSCLKSOURCE_STATUS_LSI: LSI used as system clock.
  *           @arg RCC_SYSCLKSOURCE_STATUS_HSI: HSI used as system clock.
  *           @arg RCC_SYSCLKSOURCE_STATUS_HSE: HSE used as system clock.
  *           @arg RCC_SYSCLKSOURCE_STATUS_PLLCLK: PLL used as system clock.
  */
#define __HAL_RCC_GET_SYSCLK_SOURCE() (READ_BIT(RCC->CFGR, RCC_CFGR_SWS))

/**
  * @brief  Macro to configure the MCO clock.
  * @param  __MCOCLKSOURCE__ specifies the MCO clock source.
  *         This parameter can be one of the following values:
  *           @arg RCC_MCO1SOURCE_NOCLOCK  MCO output disabled
  *           @arg RCC_MCO1SOURCE_SYSCLK  System  clock selected as MCO source
  *           @arg RCC_MCO1SOURCE_HSI  HSI clock selected as MCO source
  *           @arg RCC_MCO1SOURCE_HSE  HSE clock selected as MCO source
  *           @arg RCC_MCO1SOURCE_PLLCLK  Main PLL clock selected as MCO source
  *           @arg RCC_MCO1SOURCE_LSI  LSI clock selected as MCO source
  * @param  __MCODIV__ specifies the MCO clock prescaler.
  *         This parameter can be one of the following values:
  *           @arg RCC_MCODIV_1   MCO clock source is divided by 1
  *           @arg RCC_MCODIV_2   MCO clock source is divided by 2
  *           @arg RCC_MCODIV_4   MCO clock source is divided by 4
  *           @arg RCC_MCODIV_8   MCO clock source is divided by 8
  *           @arg RCC_MCODIV_16  MCO clock source is divided by 16
  */
#define __HAL_RCC_MCO1_CONFIG(__MCOCLKSOURCE__, __MCODIV__) \
                 MODIFY_REG(RCC->CFGR, (RCC_CFGR_MCOSEL | RCC_CFGR_MCOPRE), ((__MCOCLKSOURCE__) | (__MCODIV__)))

/**
  * @brief  Enable RCC interrupt (Perform Byte access to RCC_CIR[14:8] bits to enable
  *         the selected interrupts).
  * @param  __INTERRUPT__ specifies the RCC interrupt sources to be enabled.
  *         This parameter can be any combination of the following values:
  *           @arg RCC_IT_LSIRDY  LSI ready interrupt
  *           @arg RCC_IT_HSIRDY  HSI ready interrupt
  *           @arg RCC_IT_HSERDY  HSE ready interrupt
  *           @arg RCC_IT_PLLRDY  Main PLL ready interrupt
  *           @arg RCC_IT_CSS     HSE Clock Security System interrupt

  * @retval None
  */
#define __HAL_RCC_ENABLE_IT(__INTERRUPT__) SET_BIT(RCC->CIER, (__INTERRUPT__))

/**
  * @brief Disable RCC interrupt (Perform Byte access to RCC_CIR[14:8] bits to disable
  *        the selected interrupts).
  * @param  __INTERRUPT__ specifies the RCC interrupt sources to be disabled.
  *         This parameter can be any combination of the following values:
  *           @arg RCC_IT_LSIRDY  LSI ready interrupt
  *           @arg RCC_IT_HSIRDY  HSI ready interrupt
  *           @arg RCC_IT_HSERDY  HSE ready interrupt
  *           @arg RCC_IT_PLLRDY  Main PLL ready interrupt
  *           @arg RCC_IT_CSS     HSE Clock Security System interrupt
  * @retval None
  */
#define __HAL_RCC_DISABLE_IT(__INTERRUPT__) CLEAR_BIT(RCC->CIER, (__INTERRUPT__))

/**
  * @brief  Clear the RCC's interrupt pending bits (Perform Byte access to RCC_CIR[23:16]
  *         bits to clear the selected interrupt pending bits.
  * @param  __INTERRUPT__ specifies the interrupt pending bit to clear.
  *         This parameter can be any combination of the following values:
  *           @arg RCC_IT_LSIRDY  LSI ready interrupt
  *           @arg RCC_IT_HSIRDY  HSI ready interrupt
  *           @arg RCC_IT_HSERDY  HSE ready interrupt
  *           @arg RCC_IT_PLLRDY  Main PLL ready interrupt
  *           @arg RCC_IT_CSS  HSE Clock security system interrupt
  * @retval None
  */
#define __HAL_RCC_CLEAR_IT(__INTERRUPT__) (RCC->CICR = (__INTERRUPT__))

/**
  * @brief  Check whether the RCC interrupt has occurred or not.
  * @param  __INTERRUPT__ specifies the RCC interrupt source to check.
  *         This parameter can be one of the following values:
  *           @arg RCC_IT_LSIRDY  LSI ready interrupt
  *           @arg RCC_IT_HSIRDY  HSI ready interrupt
  *           @arg RCC_IT_HSERDY  HSE ready interrupt
  *           @arg RCC_IT_PLLRDY  Main PLL ready interrupt
  *           @arg RCC_IT_CSS  HSE Clock security system interrupt
  * @retval The new state of __INTERRUPT__ (TRUE or FALSE).
  */
#define __HAL_RCC_GET_IT(__INTERRUPT__) ((RCC->CIFR & (__INTERRUPT__)) == (__INTERRUPT__))

/**
  * @brief Set RMVF bit to clear the reset flags.
  *        The reset flags are: RCC_FLAG_OBLRST, RCC_FLAG_PINRST, RCC_FLAG_PORRST,
  *        RCC_FLAG_SFTRST, RCC_FLAG_IWDGRST, RCC_FLAG_WWDGRST and RCC_FLAG_LPWRRST.
  * @retval None
 */
#define __HAL_RCC_CLEAR_RESET_FLAGS() (RCC->CSR |= RCC_CSR_RMVF)

/** @brief  Check whether the selected RCC flag is set or not.
  * @param  __FLAG__ specifies the flag to check.
  *         This parameter can be one of the following values:
  *            @arg RCC_FLAG_HSIRDY  HSI oscillator clock ready
  *            @arg RCC_FLAG_HSERDY  HSE oscillator clock ready
  *            @arg RCC_FLAG_PLLRDY  Main PLL clock ready
  *            @arg RCC_FLAG_LSIRDY  LSI oscillator clock ready
  *            @arg RCC_FLAG_OBLRST  OBLRST reset
  *            @arg RCC_FLAG_PINRST  Pin reset
  *            @arg RCC_FLAG_PORRST  Power on reset
  *            @arg RCC_FLAG_SFTRST  Software reset
  *            @arg RCC_FLAG_IWDGRST  Independent Watchdog reset
  *            @arg RCC_FLAG_WWDGRST  Window Watchdog reset
  *            @arg RCC_FLAG_LPWRRST  Low Power reset
  * @retval The new state of __FLAG__ (TRUE or FALSE).
  */
#define __HAL_RCC_GET_FLAG(__FLAG__) (((((((__FLAG__) >> 5U) == 1U) ? RCC->CR :                     \
                                        ((((__FLAG__) >> 5U) == 2U) ? RCC->BDCR :                   \
                                        ((((__FLAG__) >> 5U) == 3U) ? RCC->CSR : RCC->CIFR))) &    \
                                          ((uint32_t)1U << ((__FLAG__) & RCC_FLAG_MASK))) != 0U) \
                                            ? 1U : 0U)

/* Private constants ---------------------------------------------------------*/

/* Defines used for Flags */
#define CR_REG_INDEX              1U
#define BDCR_REG_INDEX            2U
#define CSR_REG_INDEX             3U
#define CRRCR_REG_INDEX           4U

#define RCC_FLAG_MASK             0x1FU

/* Define used for IS_RCC_CLOCKTYPE() */
#define RCC_CLOCKTYPE_ALL              (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2)  /*!< All clcoktype to configure */

/* Private macros ------------------------------------------------------------*/


#define IS_RCC_OSCILLATORTYPE(__OSCILLATOR__) (((__OSCILLATOR__) == RCC_OSCILLATORTYPE_NONE)                               || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_HSE)   == RCC_OSCILLATORTYPE_HSE)   || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_HSI)   == RCC_OSCILLATORTYPE_HSI)   || \
                                               (((__OSCILLATOR__) & RCC_OSCILLATORTYPE_LSI)   == RCC_OSCILLATORTYPE_LSI))

/**
  * @brief  Check if the parameter __HSE__ is valid
  * @param  __HSE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_HSE(__HSE__)  (((__HSE__) == RCC_HSE_OFF) || ((__HSE__) == RCC_HSE_ON) || \
                              ((__HSE__) == RCC_HSE_BYPASS))

/**
  * @brief  Check if the parameter __HSI__ is valid
  * @param  __HSI__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_HSI(__HSI__)  (((__HSI__) == RCC_HSI_OFF) || ((__HSI__) == RCC_HSI_ON))

/**
  * @brief  Check if the parameter __LSI__ is valid
  * @param  __LSI__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_LSI(__LSI__)  (((__LSI__) == RCC_LSI_OFF) || ((__LSI__) == RCC_LSI_ON))

/**
  * @brief  Check if the parameter __PLL__ is valid
  * @param  __PLL__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_PLL(__PLL__) (((__PLL__) == RCC_PLL_NONE) || ((__PLL__) == RCC_PLL_ON))

/**
  * @brief  Check if the parameter __SOURCE__ is valid
  * @param  __SOURCE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_PLLSOURCE(__SOURCE__) (((__SOURCE__) == RCC_PLLSOURCE_HSI)  || \
                                      ((__SOURCE__) == RCC_PLLSOURCE_HSE))

/**
  * @brief  Check if the parameter __VALUE__ is valid
  * @param  __VALUE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_PLLN_VALUE(__VALUE__) ((8U <= (__VALUE__)) && ((__VALUE__) <= 127U))

/**
  * @brief  Check if the parameter __VALUE__ is valid
  * @param  __VALUE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_PLLP_VALUE(__VALUE__) (((__VALUE__ >> RCC_PLLCFGR_PLLPDIV_Pos) >= 2U) && ((__VALUE__ >> RCC_PLLCFGR_PLLPDIV_Pos) <= 31U))

/**
  * @brief  Check if the parameter __VALUE__ is valid
  * @param  __VALUE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_PLLQ_VALUE(__VALUE__) (((((__VALUE__ >> RCC_PLLCFGR_PLLQDIV_Pos)+1)*2) == 2U) || \
                                      ((((__VALUE__ >> RCC_PLLCFGR_PLLQDIV_Pos)+1)*2) == 4U) || \
                                      ((((__VALUE__ >> RCC_PLLCFGR_PLLQDIV_Pos)+1)*2) == 6U) || \
                                      ((((__VALUE__ >> RCC_PLLCFGR_PLLQDIV_Pos)+1)*2) == 8U))

/**
  * @brief  Check if the parameter __VALUE__ is valid
  * @param  __VALUE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_PLLR_VALUE(__VALUE__) (((((__VALUE__ >> RCC_PLLCFGR_PLLRDIV_Pos)+1)*2) == 2U) || \
                                      ((((__VALUE__ >> RCC_PLLCFGR_PLLRDIV_Pos)+1)*2) == 4U) || \
                                      ((((__VALUE__ >> RCC_PLLCFGR_PLLRDIV_Pos)+1)*2) == 6U) || \
                                      ((((__VALUE__ >> RCC_PLLCFGR_PLLRDIV_Pos)+1)*2) == 8U))

/**
  * @brief  Check if the parameter __CLK__ is valid
  * @param  __CLK__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_CLOCKTYPE(__CLK__)  ((((__CLK__) & RCC_CLOCKTYPE_ALL) != 0x00UL) && (((__CLK__) & ~RCC_CLOCKTYPE_ALL) == 0x00UL))

/**
  * @brief  Check if the parameter __SOURCE__ is valid
  * @param  __SOURCE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_SYSCLKSOURCE(__SOURCE__) (((__SOURCE__) == RCC_SYSCLKSOURCE_HSI) || \
                                         ((__SOURCE__) == RCC_SYSCLKSOURCE_HSE) || \
                                         ((__SOURCE__) == RCC_SYSCLKSOURCE_PLLCLK))

/**
  * @brief  Check if the parameter __HCLK__ is valid
  * @param  __HCLK__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_HCLK(__HCLK__)           (((__HCLK__) == RCC_SYSCLK_DIV1)   || ((__HCLK__) == RCC_SYSCLK_DIV2)   || \
                                         ((__HCLK__) == RCC_SYSCLK_DIV4)   || ((__HCLK__) == RCC_SYSCLK_DIV8)   || \
                                         ((__HCLK__) == RCC_SYSCLK_DIV16)  || ((__HCLK__) == RCC_SYSCLK_DIV64)  || \
                                         ((__HCLK__) == RCC_SYSCLK_DIV128) || ((__HCLK__) == RCC_SYSCLK_DIV256) || \
                                         ((__HCLK__) == RCC_SYSCLK_DIV512))

/**
  * @brief  Check if the parameter __PCLK__ is valid
  * @param  __PCLK__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_PCLK(__PCLK__)          (((__PCLK__) == RCC_HCLK_DIV1) || ((__PCLK__) == RCC_HCLK_DIV2) || \
                                        ((__PCLK__) == RCC_HCLK_DIV4) || ((__PCLK__) == RCC_HCLK_DIV8) || \
                                        ((__PCLK__) == RCC_HCLK_DIV16))

/**
  * @brief  Check if the parameter __SOURCE__ is valid
  * @param  __SOURCE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_RTCCLKSOURCE(__SOURCE__) (((__SOURCE__) == RCC_RTCCLKSOURCE_NONE)   || \
                                         ((__SOURCE__) == RCC_RTCCLKSOURCE_LSI)    || \
                                         ((__SOURCE__) == RCC_RTCCLKSOURCE_HSE_DIV128))

/**
  * @brief  Check if the parameter __MCOX__ is valid
  * @param  __MCOX__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_MCO(__MCOX__)            (((__MCOX__) == RCC_MCO1_PA8))

/**
  * @brief  Check if the parameter __SOURCE__ is valid
  * @param  __SOURCE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_MCO1SOURCE(__SOURCE__) (((__SOURCE__) == RCC_MCO1SOURCE_NOCLOCK) || \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_SYSCLK) ||  \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HSI) ||     \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_HSE) ||     \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_PLLCLK) ||  \
                                       ((__SOURCE__) == RCC_MCO1SOURCE_LSI))

/**
  * @brief  Check if the parameter __DIV__ is valid
  * @param  __DIV__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_MCODIV(__DIV__)        (((__DIV__) == RCC_MCODIV_1) || ((__DIV__) == RCC_MCODIV_2) || \
                                       ((__DIV__) == RCC_MCODIV_4) || ((__DIV__) == RCC_MCODIV_8) || \
                                       ((__DIV__) == RCC_MCODIV_16))
/**
  * end of RCC_Macro_Definitions @}
  */

/* Include RCC HAL Extended module */
#include "rx32g4xx_hal_rcc_ex.h"

/* Exported functions --------------------------------------------------------*/

/******************************************************************************/
/*                                RCC Functions                               */
/******************************************************************************/

/** @defgroup RCC_Function_Definitions RCC Function Definitions
  * @{
  */

/* Initialization and de-initialization functions  ******************************/
HAL_StatusTypeDef HAL_RCC_DeInit(void);
HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct);
HAL_StatusTypeDef HAL_RCC_ClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t FLatency);
HAL_StatusTypeDef HAL_RCC_SetPLLPClock(uint32_t PLLP_DIV);
void              HAL_RCC_SetPPRE1TIM_MUL(uint32_t PPRE1_MUL);
void              HAL_RCC_SetPPRE2TIM_MUL(uint32_t PPRE2_MUL);

/* Peripheral Control functions  ************************************************/
void              HAL_RCC_MCOConfig(uint32_t RCC_MCOx, uint32_t RCC_MCOSource, uint32_t RCC_MCODiv);
void              HAL_RCC_EnableCSS(void);
uint32_t          HAL_RCC_GetSysClockFreq(void);
uint32_t          HAL_RCC_GetHCLKFreq(void);
uint32_t          HAL_RCC_GetPCLK1Freq(void);
uint32_t          HAL_RCC_GetPCLK2Freq(void);
void              HAL_RCC_GetOscConfig(RCC_OscInitTypeDef *RCC_OscInitStruct);
void              HAL_RCC_GetClockConfig(RCC_ClkInitTypeDef *RCC_ClkInitStruct, uint32_t *pFLatency);
/* CSS NMI IRQ handler */
void              HAL_RCC_NMI_IRQHandler(void);
/* User Callbacks in non blocking mode (IT mode) */
void              HAL_RCC_CSSCallback(void);
/**
  * end of RCC_Function_Definitions @}
  */

/**
  * end of RCC @}
  */
#ifdef __cplusplus
}
#endif

#endif /* RX32G4xx_HAL_RCC_H */

