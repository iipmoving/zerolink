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
*   File    : rx32g4xx_hal_rcc_ex.h
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef RX32G4xx_HAL_RCC_EX_H
#define RX32G4xx_HAL_RCC_EX_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"

/** @addtogroup RCC
  * @{
  */

/** @addtogroup RCC_Structure_Definitions RCC Structure Definitions
  * @{
  */
  
/* Exported types ------------------------------------------------------------*/

/** @defgroup RCC_PeriphCLKInitTypeDef RCC PeriphCLKInitTypeDef
  * @ingroup  RCC_Structure_Definitions
  * @{
  */

/**
  * @brief  RCC extended clocks structure definition
  */
typedef struct
{
  uint32_t PeriphClockSelection;   /*!< The Extended Clock to be configured.
                                        This parameter can be a value of @ref RCCEx_Periph_Clock_Selection */

  uint32_t Adc123ClockSelection;   /*!< Specifies ADC123 interface clock source.
                                        This parameter can be a value of @ref RCCEx_ADC123_Clock_Source */

  uint32_t RTCClockSelection;      /*!< Specifies RTC clock source.
                                        This parameter can be a value of @ref RCC_RTC_Clock_Source */
}RCC_PeriphCLKInitTypeDef;

/**
  * end of RCC_PeriphCLKInitTypeDef @}
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

/** @defgroup RCC_Periph_Clock_Selection RCC Periph Clock Selection
  * @{
  */

#define RCC_PERIPHCLK_USART1           0x00000001U
#define RCC_PERIPHCLK_USART2           0x00000002U
#define RCC_PERIPHCLK_USART3           0x00000004U
#define RCC_PERIPHCLK_UART4            0x00000008U
#define RCC_PERIPHCLK_UART5            0x00000010U
#define RCC_PERIPHCLK_LPUART1          0x00000020U
#define RCC_PERIPHCLK_I2C1             0x00000040U
#define RCC_PERIPHCLK_I2C2             0x00000080U
#define RCC_PERIPHCLK_I2C3             0x00000100U
#define RCC_PERIPHCLK_LPTIM1           0x00000200U
#define RCC_PERIPHCLK_SAI1             0x00000400U
#define RCC_PERIPHCLK_I2S              0x00000800U
#define RCC_PERIPHCLK_CAN              0x00001000U
#define RCC_PERIPHCLK_USB              0x00002000U
#define RCC_PERIPHCLK_ADC123           0x00008000U
#define RCC_PERIPHCLK_RTC              0x00080000U
/**
  * end of RCC_Periph_Clock_Selection @}
  */

/** @defgroup RCC_ADC123_Clock_Source RCC ADC123 Clock Source
  * @{
  */
  
#define RCC_CCIPR_ADCSEL_PLL           (                       RCC_CCIPR_ADCSEL_BIT0)
#define RCC_CCIPR_ADCSEL_SYSCLK        (RCC_CCIPR_ADCSEL_BIT1                       )
  
#define RCC_ADC123CLKSOURCE_NONE       0x00000000U
#define RCC_ADC123CLKSOURCE_PLL        RCC_CCIPR_ADCSEL_PLL
#define RCC_ADC123CLKSOURCE_SYSCLK     RCC_CCIPR_ADCSEL_SYSCLK
/**
  * end of RCC_ADC123_Clock_Source @}
  */

/** @defgroup RCC_RTC_Clock_Source RCC RTC Clock Source
  * @{
  */
#define RCC_BDCR_RTCSEL_LSI            (RCC_BDCR_RTCSEL_BIT1                       )
#define RCC_BDCR_RTCSEL_HSE_DIV128     (RCC_BDCR_RTCSEL_BIT1 | RCC_BDCR_RTCSEL_BIT0)
  
#define RCC_RTCCLKSOURCE_NONE          0x00000000U
#define RCC_RTCCLKSOURCE_PLL           RCC_BDCR_RTCSEL_LSI
#define RCC_RTCCLKSOURCE_SYSCLK        RCC_BDCR_RTCSEL_HSE_DIV128

/**
  * end of RCC_RTC_Clock_Source @}
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
  * @brief  Macro to get the ADC123 clock source.
  * @retval The clock source can be one of the following values:
  *           @arg RCC_ADC123CLKSOURCE_NONE    No clock     selected as ADC123 clock
  *           @arg RCC_ADC123CLKSOURCE_PLL     PLLP Clock   selected as ADC123 clock
  *           @arg RCC_ADC123CLKSOURCE_SYSCLK  System Clock selected as ADC123 clock        
  */
#define __HAL_RCC_GET_ADC123_SOURCE() (READ_BIT(RCC->CCIPR, RCC_CCIPR_ADCSEL))

/** 
  * @brief  Macro to configure the ADC123 interface clock.
  * @param  __ADC123_CLKSOURCE__ specifies the ADC12 digital interface clock source.
  *         This parameter can be one of the following values:
  *           @arg RCC_ADC123CLKSOURCE_NONE    No clock selected as ADC123 clock
  *           @arg RCC_ADC123CLKSOURCE_PLL     PLL Clock selected as ADC123 clock
  *           @arg RCC_ADC123CLKSOURCE_SYSCLK  System Clock selected as ADC123 clock
  * @retval None
  */
#define __HAL_RCC_ADC123_CONFIG(__ADC123_CLKSOURCE__) \
                  MODIFY_REG(RCC->CCIPR, RCC_CCIPR_ADCSEL, (__ADC123_CLKSOURCE__))

/**
  * @brief  Macro to configure the CAN clock pre.
  * @param  __CAN_CLKPRE__ defines the CAN clock pre. This clock is from SYSYCLK;devided
  *         from 2 to 512.
  *          This parameter can be one of the following values:
  *             @arg @ref RCC_CFGR_CANPRE_DIV2   CAN clock pre = 2
  *             @arg @ref RCC_CFGR_CANPRE_DIV4   CAN clock pre = 4
  *             @arg @ref RCC_CFGR_CANPRE_DIV8   CAN clock pre = 8
  *             @arg @ref RCC_CFGR_CANPRE_DIV16   CAN clock pre = 16
  *             @arg @ref RCC_CFGR_CANPRE_DIV64   CAN clock pre = 64
  *             @arg @ref RCC_CFGR_CANPRE_DIV128   CAN clock pre = 128
  *             @arg @ref RCC_CFGR_CANPRE_DIV256   CAN clock pre = 256
  *             @arg @ref RCC_CFGR_CANPRE_DIV512   CAN clock pre = 512
  *
  * @retval None
  */
#define __HAL_RCC_CAN_PRE(__CAN_CLKPRE__)\
                  MODIFY_REG(RCC->CFGR, RCC_CFGR_CANPRE, (uint32_t)(__CAN_CLKPRE__))
									
/**
  * @brief  Macro to configure the CAN clock source.
  * @param  __CAN_CLKSOURCE__ defines the CAN clock source. This clock is derived
  *         from the system clk or PCLK1.
  *          This parameter can be one of the following values:
  *             @arg @ref RCC_CFGR_CANSEL_PCLK1   CAN clock = APB1CLK
  *             @arg @ref RCC_CFGR_CANSEL_CANCLK   CAN clock = CANCLK
  *
  * @retval None
  */
#define __HAL_RCC_CAN_SOURCE(__CAN_CLKSOURCE__)\
                  MODIFY_REG(RCC->CFGR, RCC_CFGR_CANSEL, (uint32_t)(__CAN_CLKSOURCE__))
									
/**
  * @brief  Check if the parameter __SELECTION__ is valid
  * @param  __SELECTION__
  * @retval Status
  *           @arg SET   (Valid)
  *           @arg RESET (Invalid)
  */
#define IS_RCC_PERIPHCLOCK(__SELECTION__)  \
               ((((__SELECTION__) & RCC_PERIPHCLK_USART1)      == RCC_PERIPHCLK_USART1)  || \
                (((__SELECTION__) & RCC_PERIPHCLK_USART2)      == RCC_PERIPHCLK_USART2)  || \
                (((__SELECTION__) & RCC_PERIPHCLK_USART3)      == RCC_PERIPHCLK_USART3)  || \
                (((__SELECTION__) & RCC_PERIPHCLK_UART4)       == RCC_PERIPHCLK_UART4)   || \
                (((__SELECTION__) & RCC_PERIPHCLK_UART5)       == RCC_PERIPHCLK_UART5)   || \
                (((__SELECTION__) & RCC_PERIPHCLK_I2C1)        == RCC_PERIPHCLK_I2C1)    || \
                (((__SELECTION__) & RCC_PERIPHCLK_I2C2)        == RCC_PERIPHCLK_I2C2)    || \
                (((__SELECTION__) & RCC_PERIPHCLK_CAN)         == RCC_PERIPHCLK_CAN)     || \
                (((__SELECTION__) & RCC_PERIPHCLK_USB)         == RCC_PERIPHCLK_USB)     || \
                (((__SELECTION__) & RCC_PERIPHCLK_ADC123)      == RCC_PERIPHCLK_ADC123)  || \
                (((__SELECTION__) & RCC_PERIPHCLK_RTC)         == RCC_PERIPHCLK_RTC))

/**
  * @brief  Check if the parameter __SOURCE__ is valid
  * @param  __SOURCE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RCC_ADC123CLKSOURCE(__SOURCE__)  \
               (((__SOURCE__) == RCC_ADC123CLKSOURCE_NONE)    || \
                ((__SOURCE__) == RCC_ADC123CLKSOURCE_PLL)     || \
                ((__SOURCE__) == RCC_ADC123CLKSOURCE_SYSCLK))


/** 
  * end of RCC_Macro_Definitions @}  
  */

/* Exported functions --------------------------------------------------------*/

/******************************************************************************/
/*                                RCC Functions                               */
/******************************************************************************/

/** @defgroup RCC_Function_Definitions RCC Function Definitions
  * @{
  */

HAL_StatusTypeDef HAL_RCCEx_PeriphCLKConfig(RCC_PeriphCLKInitTypeDef  *PeriphClkInit);
void              HAL_RCCEx_GetPeriphCLKConfig(RCC_PeriphCLKInitTypeDef  *PeriphClkInit);
uint32_t          HAL_RCCEx_GetPeriphCLKFreq(uint32_t PeriphClk);
/** 
  * end of RCC_Function_Definitions @}  
  */

/** 
  * end of RCC @}  
  */

#ifdef __cplusplus
}
#endif

#endif /* RX32G4xx_HAL_RCC_EX_H */

