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
*   File    : rx32g4xx_hal_tim_ex.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_TIM_EX_H_
#define _RX32G4XX_HAL_TIM_EX_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"
#include "rx32g4xx_hal_tim.h"


/* Exported types ------------------------------------------------------------*/

/** @addtogroup TIM
  * @{
  */

/******************************************************************************/
/*                               TIM Structures                               */
/******************************************************************************/
  
/** @addtogroup TIM_Structure_Definitions TIM Structure Definitions
  * @{
  */
  
/** @defgroup TIM_HallSensor_InitTypeDef TIM HallSensor InitTypeDef
  * @ingroup  TIM_Structure_Definitions 
  * @{
  */

/**
  * @brief  TIM Hall sensor Configuration Structure definition
  */

typedef struct
{
  uint32_t IC1Polarity;         /*!< Specifies the active edge of the input signal.
                                     This parameter can be a value of @ref TIM_Input_Capture_Polarity */

  uint32_t IC1Prescaler;        /*!< Specifies the Input Capture Prescaler.
                                     This parameter can be a value of @ref TIM_Input_Capture_Prescaler */

  uint32_t IC1Filter;           /*!< Specifies the input capture filter.
                                     This parameter can be a number between Min_Data = 0x0 and Max_Data = 0xF */

  uint32_t Commutation_Delay;   /*!< Specifies the pulse value to be loaded into the Capture Compare Register.
                                     This parameter can be a number between Min_Data = 0x0000 and Max_Data = 0xFFFF */
} TIM_HallSensor_InitTypeDef;
/** 
  * end of TIM_HallSensor_InitTypeDef @}  
  */
  
/** @defgroup TIMEx_BreakInputConfigTypeDef TIMEx BreakInputConfigTypeDef
  * @ingroup  TIM_Structure_Definitions 
  * @{
  */
/**
  * @brief  TIM Break/Break2 input configuration
  */
typedef struct
{
  uint32_t Source;         /*!< Specifies the source of the timer break input.
                                This parameter can be a value of @ref TIMEx_Break_Input_Source */
  uint32_t Enable;         /*!< Specifies whether or not the break input source is enabled.
                                This parameter can be a value of @ref TIMEx_Break_Input_Source_Enable */
  uint32_t Polarity;       /*!< Specifies the break input source polarity.
                                This parameter can be a value of @ref TIMEx_Break_Input_Source_Polarity */
} TIMEx_BreakInputConfigTypeDef;
/** 
  * end of TIMEx_BreakInputConfigTypeDef @}  
  */
  
/** @defgroup TIMEx_EncoderIndexConfigTypeDef TIMEx EncoderIndexConfigTypeDef
  * @ingroup  TIM_Structure_Definitions 
  * @{
  */
/**
  * @brief  TIM Encoder index configuration
  */
typedef struct
{
  uint32_t Polarity;                  /*!< TIM Encoder index polarity.This parameter can be a value of @ref TIMEx_Encoder_Index_Polarity */

  uint32_t Prescaler;                 /*!< TIM Encoder index prescaler.This parameter can be a value of @ref TIMEx_Encoder_Index_Prescaler */

  uint32_t Filter;                    /*!< TIM Encoder index filter.This parameter can be a number between Min_Data = 0x0 and Max_Data = 0xF */

  FunctionalState  FirstIndexEnable;  /*!< Specifies whether or not the encoder first index is enabled.This parameter value can be ENABLE or DISABLE. */

  uint32_t Position;                  /*!< Specifies in which AB input configuration the index event resets the counter.This parameter can be a value of @ref TIMEx_Encoder_Index_Position */

  uint32_t Direction;                 /*!< Specifies in which counter direction the index event resets the counter.This parameter can be a value of @ref TIMEx_Encoder_Index_Direction */

} TIMEx_EncoderIndexConfigTypeDef;
/** 
  * end of TIMEx_EncoderIndexConfigTypeDef @}  
  */
  
/** 
  * end of TIM_Structure_Definitions @}  
  */


/******************************************************************************/
/*                               TIM Parameters                               */
/******************************************************************************/
  
/** @addtogroup TIM_Parameter_Definitions TIM Parameter Definitions
  * @{
  */
  
/** @defgroup TIMEx_Remap TIM Extended Remapping
  * @ingroup  TIM_Parameter_Definitions
  * @{
  */
#define TIM_TIM1_ETR_GPIO        0x00000000U                                                     /*!< ETR input is connected to GPIO */
#define TIM_TIM1_ETR_COMP1       TIM_AF1_ETRSEL_BIT0                                             /*!< ETR input is connected to COMP1_OUT */
#define TIM_TIM1_ETR_COMP2       TIM_AF1_ETRSEL_BIT1                                             /*!< ETR input is connected to COMP2_OUT */
#define TIM_TIM1_ETR_COMP3       (TIM_AF1_ETRSEL_BIT1 | TIM_AF1_ETRSEL_BIT0)                     /*!< ETR input is connected to COMP3_OUT */
#define TIM_TIM1_ETR_COMP4       TIM_AF1_ETRSEL_BIT2                                             /*!< ETR input is connected to COMP4_OUT */
#define TIM_TIM1_ETR_ADC1_AWD1   TIM_AF1_ETRSEL_BIT3                                             /*!< ADC1 analog watchdog 1 */

#define TIM_TIM2_ETR_GPIO         0x00000000U                                                    /*!< ETR input is connected to GPIO */
#define TIM_TIM2_ETR_COMP1        TIM_AF1_ETRSEL_BIT0                                            /*!< ETR input is connected to COMP1_OUT */
#define TIM_TIM2_ETR_COMP2        TIM_AF1_ETRSEL_BIT1                                            /*!< ETR input is connected to COMP2_OUT */
#define TIM_TIM2_ETR_COMP3        (TIM_AF1_ETRSEL_BIT1 | TIM_AF1_ETRSEL_BIT0)                    /*!< ETR input is connected to COMP3_OUT */
#define TIM_TIM2_ETR_COMP4        TIM_AF1_ETRSEL_BIT2                                            /*!< ETR input is connected to COMP4_OUT */
#define TIM_TIM2_ETR_TIM3_ETR     TIM_AF1_ETRSEL_BIT3                                            /*!< ETR input is connected to TIM3 ETR */
#define TIM_TIM2_ETR_TIM4_ETR     (TIM_AF1_ETRSEL_BIT3 | TIM_AF1_ETRSEL_BIT0)                    /*!< ETR input is connected to TIM4 ETR */
#define TIM_TIM2_ETR_TIM5_ETR     (TIM_AF1_ETRSEL_BIT3 | TIM_AF1_ETRSEL_BIT1)                    /*!< ETR input is connected to TIM5 ETR */

#define TIM_TIM3_ETR_GPIO         0x00000000U                                                     /*!< ETR input is connected to GPIO */
#define TIM_TIM3_ETR_COMP1        TIM_AF1_ETRSEL_BIT0                                             /*!< ETR input is connected to COMP1_OUT */
#define TIM_TIM3_ETR_COMP2        TIM_AF1_ETRSEL_BIT1                                             /*!< ETR input is connected to COMP2_OUT */
#define TIM_TIM3_ETR_COMP3        (TIM_AF1_ETRSEL_BIT1 | TIM_AF1_ETRSEL_BIT0)                     /*!< ETR input is connected to COMP3_OUT */
#define TIM_TIM3_ETR_COMP4        TIM_AF1_ETRSEL_BIT2                                             /*!< ETR input is connected to COMP4_OUT */
#define TIM_TIM3_ETR_TIM2_ETR     TIM_AF1_ETRSEL_BIT3                                             /*!< ETR input is connected to TIM2 ETR */
#define TIM_TIM3_ETR_TIM4_ETR     (TIM_AF1_ETRSEL_BIT3 | TIM_AF1_ETRSEL_BIT0)                     /*!< ETR input is connected to TIM4 ETR */
#define TIM_TIM3_ETR_ADC2_AWD1    (TIM_AF1_ETRSEL_BIT3 | TIM_AF1_ETRSEL_BIT1 | TIM_AF1_ETRSEL_BIT0) /*!< ADC2 analog watchdog 1 */

#define TIM_TIM4_ETR_GPIO         0x00000000U                                                     /*!< ETR input is connected to GPIO */
#define TIM_TIM4_ETR_COMP1        TIM_AF1_ETRSEL_BIT0                                             /*!< ETR input is connected to COMP1_OUT */
#define TIM_TIM4_ETR_COMP2        TIM_AF1_ETRSEL_BIT1                                             /*!< ETR input is connected to COMP2_OUT */
#define TIM_TIM4_ETR_COMP3        (TIM_AF1_ETRSEL_BIT1 | TIM_AF1_ETRSEL_BIT0)                     /*!< ETR input is connected to COMP3_OUT */
#define TIM_TIM4_ETR_COMP4        TIM_AF1_ETRSEL_BIT2                                             /*!< ETR input is connected to COMP4_OUT */
#define TIM_TIM4_ETR_TIM3_ETR     TIM_AF1_ETRSEL_BIT3                                             /*!< ETR input is connected to TIM3 ETR */
#define TIM_TIM4_ETR_TIM5_ETR     (TIM_AF1_ETRSEL_BIT3 | TIM_AF1_ETRSEL_BIT0)                     /*!< ETR input is connected to TIM5 ETR */


#define TIM_TIM5_ETR_GPIO         0x00000000U                                                      /*!< ETR input is connected to GPIO */
#define TIM_TIM5_ETR_COMP1        TIM_AF1_ETRSEL_BIT0                                              /*!< ETR input is connected to COMP1_OUT */
#define TIM_TIM5_ETR_COMP2        TIM_AF1_ETRSEL_BIT1                                              /*!< ETR input is connected to COMP2_OUT */
#define TIM_TIM5_ETR_COMP3        (TIM_AF1_ETRSEL_BIT1 | TIM_AF1_ETRSEL_BIT0)                      /*!< ETR input is connected to COMP3_OUT */
#define TIM_TIM5_ETR_COMP4        TIM_AF1_ETRSEL_BIT2                                              /*!< ETR input is connected to COMP4_OUT */
#define TIM_TIM5_ETR_TIM2_ETR     TIM_AF1_ETRSEL_BIT3                                              /*!< ETR input is connected to TIM2 ETR */
#define TIM_TIM5_ETR_TIM3_ETR     (TIM_AF1_ETRSEL_BIT3 | TIM_AF1_ETRSEL_BIT0)                      /*!< ETR input is connected to TIM3 ETR */

#define TIM_TIM8_ETR_GPIO        0x00000000U                                                        /*!< ETR input is connected to GPIO */
#define TIM_TIM8_ETR_COMP1       TIM_AF1_ETRSEL_BIT0                                                /*!< ETR input is connected to COMP1_OUT */
#define TIM_TIM8_ETR_COMP2       TIM_AF1_ETRSEL_BIT1                                                /*!< ETR input is connected to COMP2_OUT */
#define TIM_TIM8_ETR_COMP3       (TIM_AF1_ETRSEL_BIT1 | TIM_AF1_ETRSEL_BIT0)                        /*!< ETR input is connected to COMP3_OUT */
#define TIM_TIM8_ETR_COMP4       TIM_AF1_ETRSEL_BIT2                                                /*!< ETR input is connected to COMP4_OUT */
#define TIM_TIM8_ETR_ADC2_AWD1   TIM_AF1_ETRSEL_BIT3                                                /*!< ADC2 analog watchdog 1 */
#define TIM_TIM8_ETR_ADC3_AWD1   (TIM_AF1_ETRSEL_BIT3 | TIM_AF1_ETRSEL_BIT1 | TIM_AF1_ETRSEL_BIT0)  /*!< ADC3 analog watchdog 1 */
/** 
  * end of TIMEx_Remap @}  
  */
  
/** @defgroup TIMEx_Break_Input TIM Extended Break input
  * @ingroup  TIM_Parameter_Definitions
  * @{
  */
#define TIM_BREAKINPUT_BRK     0x00000001U                                      /*!< Timer break input  */
#define TIM_BREAKINPUT_BRK2    0x00000002U                                      /*!< Timer break2 input */
/** 
  * end of TIMEx_Break_Input @}  
  */
  
/** @defgroup TIMEx_Break_Input_Source TIM Extended Break input source
  * @ingroup  TIM_Parameter_Definitions
  * @{
  */
#define TIM_BREAKINPUTSOURCE_BKIN     0x00000001U                               /*!< An external source (GPIO) is connected to the BKIN pin  */
#define TIM_BREAKINPUTSOURCE_COMP1    0x00000002U                               /*!< The COMP1 output is connected to the break input */
#define TIM_BREAKINPUTSOURCE_COMP2    0x00000004U                               /*!< The COMP2 output is connected to the break input */
#define TIM_BREAKINPUTSOURCE_COMP3    0x00000008U                               /*!< The COMP3 output is connected to the break input */
#define TIM_BREAKINPUTSOURCE_COMP4    0x00000010U                               /*!< The COMP4 output is connected to the break input */
/** 
  * end of TIMEx_Break_Input_Source @}  
  */
  
/** @defgroup TIMEx_Break_Input_Source_Enable TIM Extended Break input source enabling
  * @ingroup  TIM_Parameter_Definitions
  * @{
  */
#define TIM_BREAKINPUTSOURCE_DISABLE     0x00000000U                            /*!< Break input source is disabled */
#define TIM_BREAKINPUTSOURCE_ENABLE      0x00000001U                            /*!< Break input source is enabled */
/** 
  * end of TIMEx_Break_Input_Source_Enable @}  
  */
  
/** @defgroup TIMEx_Break_Input_Source_Polarity TIM Extended Break input polarity
  * @ingroup  TIM_Parameter_Definitions
  * @{
  */
#define TIM_BREAKINPUTSOURCE_POLARITY_LOW     0x00000001U                       /*!< Break input source is active low */
#define TIM_BREAKINPUTSOURCE_POLARITY_HIGH    0x00000000U                       /*!< Break input source is active_high */
/** 
  * end of TIMEx_Break_Input_Source_Polarity @}  
  */
  
/** @defgroup TIMEx_Timer_Input_Selection TIM Extended Timer input selection
  * @ingroup  TIM_Parameter_Definitions
  * @{
  */
#define TIM_TIM1_TI1_GPIO       0x00000000U                                                               /*!< TIM1 input 1 is connected to GPIO */
#define TIM_TIM1_TI1_COMP1      TIM_TISEL_TI1SEL_BIT0                                                     /*!< TIM1 input 1 is connected to COMP1_OUT */
#define TIM_TIM1_TI1_COMP2      TIM_TISEL_TI1SEL_BIT1                                                     /*!< TIM1 input 1 is connected to COMP2_OUT */
#define TIM_TIM1_TI1_COMP3      (TIM_TISEL_TI1SEL_BIT1 | TIM_TISEL_TI1SEL_BIT0)                           /*!< TIM1 input 1 is connected to COMP3_OUT */
#define TIM_TIM1_TI1_COMP4      TIM_TISEL_TI1SEL_BIT2                                                     /*!< TIM1 input 1 is connected to COMP4_OUT */
                        
#define TIM_TIM2_TI1_GPIO       0x00000000U                                                                /*!< TIM2 input 1 is connected to GPIO */
#define TIM_TIM2_TI1_COMP1      TIM_TISEL_TI1SEL_BIT0                                                      /*!< TIM2 input 1 is connected to COMP1_OUT */
#define TIM_TIM2_TI1_COMP2      TIM_TISEL_TI1SEL_BIT1                                                      /*!< TIM2 input 1 is connected to COMP2_OUT */
#define TIM_TIM2_TI1_COMP3      (TIM_TISEL_TI1SEL_BIT1 | TIM_TISEL_TI1SEL_BIT0)                            /*!< TIM2 input 1 is connected to COMP3_OUT */
#define TIM_TIM2_TI1_COMP4      TIM_TISEL_TI1SEL_BIT2                                                      /*!< TIM2 input 1 is connected to COMP4_OUT */
                        
#define TIM_TIM2_TI2_GPIO       0x00000000U                                                               /*!< TIM2 input 2 is connected to GPIO */
#define TIM_TIM2_TI2_COMP1      TIM_TISEL_TI2SEL_BIT0                                                     /*!< TIM2 input 2 is connected to COMP1_OUT */
#define TIM_TIM2_TI2_COMP2      TIM_TISEL_TI2SEL_BIT1                                                     /*!< TIM2 input 2 is connected to COMP2_OUT */
#define TIM_TIM2_TI2_COMP3      (TIM_TISEL_TI2SEL_BIT1 | TIM_TISEL_TI2SEL_BIT0)                           /*!< TIM2 input 2 is connected to COMP3_OUT */
#define TIM_TIM2_TI2_COMP4      TIM_TISEL_TI2SEL_BIT2                                                     /*!< TIM2 input 2 is connected to COMP4_OUT */
                        
#define TIM_TIM2_TI3_GPIO       0x00000000U                                                             /*!< TIM2 input 3 is connected to GPIO */
#define TIM_TIM2_TI3_COMP4      TIM_TISEL_TI3SEL_BIT0                                                   /*!< TIM2 input 3 is connected to COMP4_OUT */

#define TIM_TIM2_TI4_GPIO       0x00000000U                                                             /*!< TIM2 input 4 is connected to GPIO */
#define TIM_TIM2_TI4_COMP1      TIM_TISEL_TI4SEL_BIT0                                                   /*!< TIM2 input 4 is connected to COMP1_OUT */
#define TIM_TIM2_TI4_COMP2      TIM_TISEL_TI4SEL_BIT1                                                   /*!< TIM2 input 4 is connected to COMP2_OUT */
                        
#define TIM_TIM3_TI1_GPIO       0x00000000U                                                               /*!< TIM3 input 1 is connected to GPIO */
#define TIM_TIM3_TI1_COMP1      TIM_TISEL_TI1SEL_BIT0                                                     /*!< TIM3 input 1 is connected to COMP1_OUT */
#define TIM_TIM3_TI1_COMP2      TIM_TISEL_TI1SEL_BIT1                                                     /*!< TIM3 input 1 is connected to COMP2_OUT */
#define TIM_TIM3_TI1_COMP3      (TIM_TISEL_TI1SEL_BIT1 | TIM_TISEL_TI1SEL_BIT0)                           /*!< TIM3 input 1 is connected to COMP3_OUT */
#define TIM_TIM3_TI1_COMP4      TIM_TISEL_TI1SEL_BIT2                                                     /*!< TIM3 input 1 is connected to COMP4_OUT */
                        
#define TIM_TIM3_TI2_GPIO       0x00000000U                                                               /*!< TIM3 input 2 is connected to GPIO */
#define TIM_TIM3_TI2_COMP1      TIM_TISEL_TI2SEL_BIT0                                                     /*!< TIM3 input 2 is connected to COMP1_OUT */
#define TIM_TIM3_TI2_COMP2      TIM_TISEL_TI2SEL_BIT1                                                     /*!< TIM3 input 2 is connected to COMP2_OUT */
#define TIM_TIM3_TI2_COMP3      (TIM_TISEL_TI2SEL_BIT1 | TIM_TISEL_TI2SEL_BIT0)                           /*!< TIM3 input 2 is connected to COMP3_OUT */
#define TIM_TIM3_TI2_COMP4      TIM_TISEL_TI2SEL_BIT2                                                     /*!< TIM3 input 2 is connected to COMP4_OUT */
                        
#define TIM_TIM3_TI3_GPIO       0x00000000U                                                            /*!< TIM3 input 3 is connected to GPIO */
#define TIM_TIM3_TI3_COMP3      TIM_TISEL_TI3SEL_BIT0                                                  /*!< TIM3 input 3 is connected to COMP3_OUT */
                        
#define TIM_TIM3_TI4_GPIO       0x00000000U                                                            /*!< TIM3 input 3 is connected to GPIO */
#define TIM_TIM4_TI1_GPIO       0x00000000U                                                               /*!< TIM4 input 1 is connected to GPIO */
#define TIM_TIM4_TI1_COMP1      TIM_TISEL_TI1SEL_BIT0                                                     /*!< TIM4 input 1 is connected to COMP1_OUT */
#define TIM_TIM4_TI1_COMP2      TIM_TISEL_TI1SEL_BIT1                                                     /*!< TIM4 input 1 is connected to COMP2_OUT */
#define TIM_TIM4_TI1_COMP3      (TIM_TISEL_TI1SEL_BIT1 | TIM_TISEL_TI1SEL_BIT0)                           /*!< TIM4 input 1 is connected to COMP3_OUT */
#define TIM_TIM4_TI1_COMP4      TIM_TISEL_TI1SEL_BIT2                                                     /*!< TIM4 input 1 is connected to COMP4_OUT */
                        
#define TIM_TIM4_TI2_GPIO       0x00000000U                                                               /*!< TIM4 input 2 is connected to GPIO */
#define TIM_TIM4_TI2_COMP1      TIM_TISEL_TI2SEL_BIT0                                                     /*!< TIM4 input 2 is connected to COMP1_OUT */
#define TIM_TIM4_TI2_COMP2      TIM_TISEL_TI2SEL_BIT1                                                     /*!< TIM4 input 2 is connected to COMP2_OUT */
#define TIM_TIM4_TI2_COMP3      (TIM_TISEL_TI2SEL_BIT1 | TIM_TISEL_TI2SEL_BIT0)                           /*!< TIM4 input 2 is connected to COMP3_OUT */
#define TIM_TIM4_TI2_COMP4      TIM_TISEL_TI2SEL_BIT2                                                     /*!< TIM4 input 2 is connected to COMP4_OUT */
#define TIM_TIM4_TI3_GPIO       0x00000000U                                                               /*!< TIM4 input 3 is connected to GPIO */
#define TIM_TIM4_TI4_GPIO       0x00000000U                                                               /*!< TIM4 input 4 is connected to GPIO */
                    
#define TIM_TIM5_TI1_GPIO       0x00000000U                                                                  /*!< TIM5 input 1 is connected to GPIO */
#define TIM_TIM5_TI1_LSI        TIM_TISEL_TI1SEL_BIT0                                                        /*!< TIM5 input 1 is connected to LSI */
#define TIM_TIM5_TI1_RTC_WK     (TIM_TISEL_TI1SEL_BIT1 | TIM_TISEL_TI1SEL_BIT0)                              /*!< TIM5 input 1 is connected to RTC_WAKEUP */
#define TIM_TIM5_TI1_COMP1      TIM_TISEL_TI1SEL_BIT2                                                        /*!< TIM5 input 1 is connected to COMP1_OUT */
#define TIM_TIM5_TI1_COMP2      (TIM_TISEL_TI1SEL_BIT2 | TIM_TISEL_TI1SEL_BIT0)                              /*!< TIM5 input 1 is connected to COMP2_OUT */
#define TIM_TIM5_TI1_COMP3      (TIM_TISEL_TI1SEL_BIT2 | TIM_TISEL_TI1SEL_BIT1)                              /*!< TIM5 input 1 is connected to COMP3_OUT */
#define TIM_TIM5_TI1_COMP4      (TIM_TISEL_TI1SEL_BIT2 | TIM_TISEL_TI1SEL_BIT1 | TIM_TISEL_TI1SEL_BIT0)      /*!< TIM5 input 1 is connected to COMP4_OUT */
    
#define TIM_TIM5_TI2_GPIO       0x00000000U                                                               /*!< TIM5 input 2 is connected to GPIO */
#define TIM_TIM5_TI2_COMP1      TIM_TISEL_TI2SEL_BIT0                                                     /*!< TIM5 input 2 is connected to COMP1_OUT */
#define TIM_TIM5_TI2_COMP2      TIM_TISEL_TI2SEL_BIT1                                                     /*!< TIM5 input 2 is connected to COMP2_OUT */
#define TIM_TIM5_TI2_COMP3      (TIM_TISEL_TI2SEL_BIT1 | TIM_TISEL_TI2SEL_BIT0)                           /*!< TIM5 input 2 is connected to COMP3_OUT */
#define TIM_TIM5_TI2_COMP4      TIM_TISEL_TI2SEL_BIT2                                                     /*!< TIM5 input 2 is connected to COMP4_OUT */
#define TIM_TIM5_TI3_GPIO       0x00000000U                                                               /*!< TIM5 input 3 is connected to GPIO */
#define TIM_TIM5_TI4_GPIO       0x00000000U                                                               /*!< TIM5 input 4 is connected to GPIO */
                        
#define TIM_TIM8_TI1_GPIO       0x00000000U                                                               /*!< TIM8 input 1 is connected to GPIO */
#define TIM_TIM8_TI1_COMP1      TIM_TISEL_TI1SEL_BIT0                                                     /*!< TIM8 input 1 is connected to COMP1_OUT */
#define TIM_TIM8_TI1_COMP2      TIM_TISEL_TI1SEL_BIT1                                                     /*!< TIM8 input 1 is connected to COMP2_OUT */
#define TIM_TIM8_TI1_COMP3      (TIM_TISEL_TI1SEL_BIT1 | TIM_TISEL_TI1SEL_BIT0)                           /*!< TIM8 input 1 is connected to COMP3_OUT */
#define TIM_TIM8_TI1_COMP4      TIM_TISEL_TI1SEL_BIT2                                                     /*!< TIM8 input 1 is connected to COMP4_OUT */
                    
#define TIM_TIM15_TI1_GPIO      0x00000000U                                                               /*!< TIM15 input 1 is connected to GPIO */
#define TIM_TIM15_TI1_COMP1     TIM_TISEL_TI1SEL_BIT1                                                     /*!< TIM15 input 1 is connected to COMP1_OUT */
#define TIM_TIM15_TI1_COMP2     (TIM_TISEL_TI1SEL_BIT1 | TIM_TISEL_TI1SEL_BIT0)                           /*!< TIM15 input 1 is connected to COMP2_OUT */
#define TIM_TIM15_TI2_GPIO      0x00000000U                                                               /*!< TIM15 input 2 is connected to GPIO */
#define TIM_TIM15_TI2_COMP2     TIM_TISEL_TI2SEL_BIT0                                                     /*!< TIM15 input 2 is connected to COMP2_OUT */
#define TIM_TIM15_TI2_COMP3     TIM_TISEL_TI2SEL_BIT1                                                     /*!< TIM15 input 2 is connected to COMP3_OUT */

#define TIM_TIM16_TI1_GPIO      0x00000000U                                                               /*!< TIM16 input 1 is connected to GPIO */
#define TIM_TIM16_TI1_MCO       TIM_TISEL_TI1SEL_BIT1                                                     /*!< TIM16 input 1 is connected to MCO */
#define TIM_TIM16_TI1_HSE_32    (TIM_TISEL_TI1SEL_BIT1 | TIM_TISEL_TI1SEL_BIT0)                           /*!< TIM16 input 1 is connected to HSE/32 */
#define TIM_TIM16_TI1_RTC_WK    TIM_TISEL_TI1SEL_BIT2                                                     /*!< TIM16 input 1 is connected to RTC_WAKEUP */
#define TIM_TIM16_TI1_LSI       (TIM_TISEL_TI1SEL_BIT2 | TIM_TISEL_TI1SEL_BIT1)                           /*!< TIM16 input 1 is connected to LSI */
                        
#define TIM_TIM17_TI1_GPIO      0x00000000U                                                               /*!< TIM17 input 1 is connected to GPIO */
#define TIM_TIM17_TI1_MCO       TIM_TISEL_TI1SEL_BIT1                                                     /*!< TIM17 input 1 is connected to MCO */
#define TIM_TIM17_TI1_HSE_32    (TIM_TISEL_TI1SEL_BIT1 | TIM_TISEL_TI1SEL_BIT0)                           /*!< TIM17 input 1 is connected to HSE/32 */
#define TIM_TIM17_TI1_RTC_WK    TIM_TISEL_TI1SEL_BIT2                                                     /*!< TIM17 input 1 is connected to RTC_WAKEUP */
#define TIM_TIM17_TI1_LSI       (TIM_TISEL_TI1SEL_BIT2 | TIM_TISEL_TI1SEL_BIT1)                           /*!< TIM17 input 1 is connected to LSI */
/** 
  * end of TIMEx_Timer_Input_Selection @}  
  */
  
/** @defgroup TIMEx_SMS_Preload_Enable TIM Extended Bitfield SMS preload enabling
  * @ingroup  TIM_Parameter_Definitions
  * @{
  */
#define TIM_SMS_PRELOAD_SOURCE_UPDATE     0x00000000U                            /*!< Prelaod of SMS bitfield is disabled */
#define TIM_SMS_PRELOAD_SOURCE_INDEX      TIM_SMCR_SMSPS                         /*!< Preload of SMS bitfield is enabled  */
/** 
  * end of TIMEx_SMS_Preload_Enable @}  
  */
  
/** @defgroup TIMEx_Encoder_Index_Position TIM Extended Encoder index position
  * @ingroup  TIM_Parameter_Definitions
  * @{
  */
#define TIM_ENCODERINDEX_POSITION_00        0x00000000U                                 /*!< Encoder index position is AB=00 */
#define TIM_ENCODERINDEX_POSITION_01        TIM_ECR_IPOS_BIT0                           /*!< Encoder index position is AB=01 */
#define TIM_ENCODERINDEX_POSITION_10        TIM_ECR_IPOS_BIT1                           /*!< Encoder index position is AB=10 */
#define TIM_ENCODERINDEX_POSITION_11        (TIM_ECR_IPOS_BIT1 | TIM_ECR_IPOS_BIT0)     /*!< Encoder index position is AB=11 */
#define TIM_ENCODERINDEX_POSITION_0         0x00000000U                                 /*!< In directional clock mode or clock plus direction mode, index resets the counter when clock is 0 */
#define TIM_ENCODERINDEX_POSITION_1         TIM_ECR_IPOS_BIT0                           /*!< In directional clock mode or clock plus direction mode, index resets the counter when clock is 1 */
/** 
  * end of TIMEx_Encoder_Index_Position @}  
  */
  
/** @defgroup TIMEx_Encoder_Index_Direction TIM Extended Encoder index direction
  * @ingroup  TIM_Parameter_Definitions
  * @{
  */
#define TIM_ENCODERINDEX_DIRECTION_UP_DOWN 0x00000000U           /*!< Index resets the counter whatever the direction  */
#define TIM_ENCODERINDEX_DIRECTION_UP      TIM_ECR_IDIR_BIT0     /*!< Index resets the counter when up-counting only   */
#define TIM_ENCODERINDEX_DIRECTION_DOWN    TIM_ECR_IDIR_BIT1     /*!< Index resets the counter when down-counting only */
/** 
  * end of TIMEx_Encoder_Index_Direction @}  
  */
/** @defgroup TIMEx_Encoder_Index_Polarity TIM Extended Encoder index polarity
  * @ingroup  TIM_Parameter_Definitions
  * @{
  */
#define TIM_ENCODERINDEX_POLARITY_INVERTED           TIM_ETRPOLARITY_INVERTED      /*!< Polarity for ETRx pin */
#define TIM_ENCODERINDEX_POLARITY_NONINVERTED        TIM_ETRPOLARITY_NONINVERTED   /*!< Polarity for ETRx pin */
/** 
  * end of TIMEx_Encoder_Index_Polarity @}  
  */
/** @defgroup TIMEx_Encoder_Index_Prescaler TIM Extended Encodder index prescaler
  * @ingroup  TIM_Parameter_Definitions
  * @{
  */
#define TIM_ENCODERINDEX_PRESCALER_DIV1              TIM_ETRPRESCALER_DIV1         /*!< No prescaler is used                                                   */
#define TIM_ENCODERINDEX_PRESCALER_DIV2              TIM_ETRPRESCALER_DIV2         /*!< Prescaler for External ETR pin: Capture performed once every 2 events. */
#define TIM_ENCODERINDEX_PRESCALER_DIV4              TIM_ETRPRESCALER_DIV4         /*!< Prescaler for External ETR pin: Capture performed once every 4 events. */
#define TIM_ENCODERINDEX_PRESCALER_DIV8              TIM_ETRPRESCALER_DIV8         /*!< Prescaler for External ETR pin: Capture performed once every 8 events. */
/** 
  * end of TIMEx_Encoder_Index_Prescaler @}  
  */
  
/** 
  * end of TIM_Parameter_Definitions @}  
  */


/******************************************************************************/
/*                                 TIM Macro                                  */
/******************************************************************************/

/** @addtogroup TIM_Macro_Definitions TIM Macro Definitions
  * @{
  */
  
/**
  * @brief  HELPER macro calculating the prescaler value to achieve the required counter clock frequency.
  * @note   ex: @ref __HAL_TIM_CALC_PSC(80000000, 1000000);
  * @param  __TIMCLK__ timer input clock frequency (in Hz)
  * @param  __CNTCLK__ counter clock frequency (in Hz)
  * @retval Prescaler value  (between Min_Data=0 and Max_Data=65535)
  */
#define __HAL_TIM_CALC_PSC(__TIMCLK__, __CNTCLK__)   \
  ((__TIMCLK__) >= (__CNTCLK__)) ? (uint32_t)((__TIMCLK__)/(__CNTCLK__) - 1U) : 0U

/**
  * @brief  HELPER macro calculating the auto-reload value to achieve the required output signal frequency.
  * @note   ex: @ref __HAL_TIM_CALC_PERIOD(1000000, 0, 10000);
  * @param  __TIMCLK__ timer input clock frequency (in Hz)
  * @param  __PSC__ prescaler
  * @param  __FREQ__ output signal frequency (in Hz)
  * @retval  Auto-reload value  (between Min_Data=0 and Max_Data=65535)
  */
#define __HAL_TIM_CALC_PERIOD(__TIMCLK__, __PSC__, __FREQ__) \
  (((__TIMCLK__)/((__PSC__) + 1U)) >= (__FREQ__)) ? ((__TIMCLK__)/((__FREQ__) * ((__PSC__) + 1U)) - 1U) : 0U

/**
  * @brief  HELPER macro calculating the auto-reload value, with dithering feature enabled, to achieve the required
  *         output signal frequency.
  * @note   ex: @ref __HAL_TIM_CALC_PERIOD_DITHER(1000000, 0, 10000);
  * @note   This macro should be used only if dithering is already enabled
  * @param  __TIMCLK__ timer input clock frequency (in Hz)
  * @param  __PSC__ prescaler
  * @param  __FREQ__ output signal frequency (in Hz)
  * @retval  Auto-reload value  (between Min_Data=0 and Max_Data=65519)
  */
#define __HAL_TIM_CALC_PERIOD_DITHER(__TIMCLK__, __PSC__, __FREQ__) \
  (((__TIMCLK__)/((__PSC__) + 1U)) >= (__FREQ__)) ? \
  (uint32_t)(((uint64_t)(__TIMCLK__)*16/((__FREQ__) * ((__PSC__) + 1U)) - 16U)) : 0U

/**
  * @brief  HELPER macro calculating the compare value required to achieve the required timer output compare
  *         active/inactive delay.
  * @note   ex: @ref __HAL_TIM_CALC_PULSE(1000000, 0, 10);
  * @param  __TIMCLK__ timer input clock frequency (in Hz)
  * @param  __PSC__ prescaler
  * @param  __DELAY__ timer output compare active/inactive delay (in us)
  * @retval Compare value  (between Min_Data=0 and Max_Data=65535)
  */
#define __HAL_TIM_CALC_PULSE(__TIMCLK__, __PSC__, __DELAY__)  \
  ((uint32_t)(((uint64_t)(__TIMCLK__) * (uint64_t)(__DELAY__)) \
              / ((uint64_t)1000000U * (uint64_t)((__PSC__) + 1U))))

/**
  * @brief  HELPER macro calculating the compare value, with dithering feature enabled, to achieve the required timer
  *         output compare active/inactive delay.
  * @note   ex: @ref __HAL_TIM_CALC_PULSE_DITHER(1000000, 0, 10);
  * @note   This macro should be used only if dithering is already enabled
  * @param  __TIMCLK__ timer input clock frequency (in Hz)
  * @param  __PSC__ prescaler
  * @param  __DELAY__ timer output compare active/inactive delay (in us)
  * @retval Compare value  (between Min_Data=0 and Max_Data=65519)
  */
#define __HAL_TIM_CALC_PULSE_DITHER(__TIMCLK__, __PSC__, __DELAY__)  \
  ((uint32_t)(((uint64_t)(__TIMCLK__) * (uint64_t)(__DELAY__) * 16U) \
              / ((uint64_t)1000000U * (uint64_t)((__PSC__) + 1U))))

/**
  * @brief  HELPER macro calculating the auto-reload value to achieve the required pulse duration
  *        (when the timer operates in one pulse mode).
  * @note   ex: @ref __HAL_TIM_CALC_PERIOD_BY_DELAY(1000000, 0, 10, 20);
  * @param  __TIMCLK__ timer input clock frequency (in Hz)
  * @param  __PSC__ prescaler
  * @param  __DELAY__ timer output compare active/inactive delay (in us)
  * @param  __PULSE__ pulse duration (in us)
  * @retval Auto-reload value  (between Min_Data=0 and Max_Data=65535)
  */
#define __HAL_TIM_CALC_PERIOD_BY_DELAY(__TIMCLK__, __PSC__, __DELAY__, __PULSE__)  \
  ((uint32_t)(__HAL_TIM_CALC_PULSE((__TIMCLK__), (__PSC__), (__PULSE__)) \
              + __HAL_TIM_CALC_PULSE((__TIMCLK__), (__PSC__), (__DELAY__))))

/**
  * @brief  HELPER macro calculating the auto-reload value, with dithering feature enabled, to achieve the required
  *         pulse duration (when the timer operates in one pulse mode).
  * @note   ex: @ref __HAL_TIM_CALC_PERIOD_DITHER_BY_DELAY(1000000, 0, 10, 20);
  * @note   This macro should be used only if dithering is already enabled
  * @param  __TIMCLK__ timer input clock frequency (in Hz)
  * @param  __PSC__ prescaler
  * @param  __DELAY__ timer output compare active/inactive delay (in us)
  * @param  __PULSE__ pulse duration (in us)
  * @retval Auto-reload value  (between Min_Data=0 and Max_Data=65519)
  */
#define __HAL_TIM_CALC_PERIOD_DITHER_BY_DELAY(__TIMCLK__, __PSC__, __DELAY__, __PULSE__)  \
  ((uint32_t)(__HAL_TIM_CALC_PULSE_DITHER((__TIMCLK__), (__PSC__), (__PULSE__)) \
              + __HAL_TIM_CALC_PULSE_DITHER((__TIMCLK__), (__PSC__), (__DELAY__))))

/** @brief  Check if the parameter __REMAP__ is valid
  * @param  __REMAP__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_REMAP(__REMAP__) ((((__REMAP__) & 0xFFFC3FFFU) == 0x00000000U))

/** @brief  Check if the parameter __REMAP__ is valid
  * @param  __REMAP__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_BREAKINPUT(__BREAKINPUT__)  (((__BREAKINPUT__) == TIM_BREAKINPUT_BRK)  || \
                                            ((__BREAKINPUT__) == TIM_BREAKINPUT_BRK2))

/** @brief  Check if the parameter __REMAP__ is valid
  * @param  __REMAP__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_BREAKINPUTSOURCE(__SOURCE__)  (((__SOURCE__) == TIM_BREAKINPUTSOURCE_BKIN)  || \
                                              ((__SOURCE__) == TIM_BREAKINPUTSOURCE_COMP1) || \
                                              ((__SOURCE__) == TIM_BREAKINPUTSOURCE_COMP2) || \
                                              ((__SOURCE__) == TIM_BREAKINPUTSOURCE_COMP3) || \
                                              ((__SOURCE__) == TIM_BREAKINPUTSOURCE_COMP4))

/** @brief  Check if the parameter __REMAP__ is valid
  * @param  __REMAP__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_BREAKINPUTSOURCE_STATE(__STATE__)  (((__STATE__) == TIM_BREAKINPUTSOURCE_DISABLE)  || \
                                                   ((__STATE__) == TIM_BREAKINPUTSOURCE_ENABLE))

/** @brief  Check if the parameter __REMAP__ is valid
  * @param  __REMAP__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_BREAKINPUTSOURCE_POLARITY(__POLARITY__)  (((__POLARITY__) == TIM_BREAKINPUTSOURCE_POLARITY_LOW)  || \
                                                         ((__POLARITY__) == TIM_BREAKINPUTSOURCE_POLARITY_HIGH))

/** @brief  Check if the parameter __REMAP__ is valid
  * @param  __REMAP__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_TISEL(__TISEL__) ((((__TISEL__) & 0xF0F0F0F0U) == 0x00000000U))

/** @brief  Check if the parameter __REMAP__ is valid
  * @param  __REMAP__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_TISEL_TIX_INSTANCE(INSTANCE, CHANNEL) \
  (IS_TIM_CCX_INSTANCE(INSTANCE, CHANNEL) && ((CHANNEL) < TIM_CHANNEL_5))

/** @brief  Check if the parameter __REMAP__ is valid
  * @param  __REMAP__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_CLOCKSOURCE_INSTANCE(INSTANCE, __CLOCK__) \
  ((((INSTANCE) == TIM1) &&                                 \
    (((__CLOCK__) == TIM_CLOCKSOURCE_ETRMODE1)  ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1ED)     ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI2)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR1)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR2)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR3)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR4)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR5)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR6)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR7)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR8)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR10)))            \
   ||                                        \
   (((INSTANCE) == TIM2) &&                  \
    (((__CLOCK__) == TIM_CLOCKSOURCE_ETRMODE1)  ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1ED)     ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI2)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR0)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR2)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR3)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR4)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR5)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR6)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR7)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR8)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR10)))            \
   ||                                        \
   (((INSTANCE) == TIM3) &&                  \
    (((__CLOCK__) == TIM_CLOCKSOURCE_ETRMODE1)  ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1ED)     ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI2)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR0)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR1)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR3)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR4)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR5)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR6)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR7)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR8)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR10)))            \
   ||                                        \
   (((INSTANCE) == TIM4) &&                  \
    (((__CLOCK__) == TIM_CLOCKSOURCE_ETRMODE1)  ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1ED)     ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI2)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR0)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR1)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR2)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR4)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR5)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR6)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR7)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR8)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR10)))            \
   ||                                        \
   (((INSTANCE) == TIM5) &&                  \
    (((__CLOCK__) == TIM_CLOCKSOURCE_ETRMODE1)  ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1ED)     ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI2)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR0)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR1)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR2)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR3)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR5)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR6)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR7)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR8)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR10)))            \
   ||                                        \
   (((INSTANCE) == TIM8) &&                  \
    (((__CLOCK__) == TIM_CLOCKSOURCE_ETRMODE1)  ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1ED)     ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI2)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR0)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR1)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR2)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR3)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR4)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR6)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR7)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR8)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR10)))            \
   ||                                        \
   (((INSTANCE) == TIM15) &&                 \
    (((__CLOCK__) == TIM_CLOCKSOURCE_TI1ED)     ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI2)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR0)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR1)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR2)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR3)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR4)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR5)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR7)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR8)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR10)))\
   ||                                        \
   (((INSTANCE) == TIM16) &&                 \
    (((__CLOCK__) == TIM_CLOCKSOURCE_TI1ED)     ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI2)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR0)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR1)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR2)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR3)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR4)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR5)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR7)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR8)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR10)))\
   ||                                        \
   (((INSTANCE) == TIM17) &&                 \
    (((__CLOCK__) == TIM_CLOCKSOURCE_TI1ED)     ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI1)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_TI2)       ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR0)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR1)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR2)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR3)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR4)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR5)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR7)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR8)      ||          \
     ((__CLOCK__) == TIM_CLOCKSOURCE_ITR10))))

/** @brief  Check if the parameter __REMAP__ is valid
  * @param  __REMAP__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_TRIGGER_INSTANCE(INSTANCE, __SELECTION__) \
  ((((INSTANCE) == TIM1) &&                  \
    (((__SELECTION__) == TIM_TS_TI1F_ED) ||          \
     ((__SELECTION__) == TIM_TS_TI1FP1)  ||          \
     ((__SELECTION__) == TIM_TS_TI2FP2)  ||          \
     ((__SELECTION__) == TIM_TS_ETRF)    ||          \
     ((__SELECTION__) == TIM_TS_ITR1)    ||          \
     ((__SELECTION__) == TIM_TS_ITR2)    ||          \
     ((__SELECTION__) == TIM_TS_ITR3)    ||          \
     ((__SELECTION__) == TIM_TS_ITR4)    ||          \
     ((__SELECTION__) == TIM_TS_ITR5)    ||          \
     ((__SELECTION__) == TIM_TS_ITR6)    ||          \
     ((__SELECTION__) == TIM_TS_ITR7)    ||          \
     ((__SELECTION__) == TIM_TS_ITR8)    ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))             \
   ||                                        \
   (((INSTANCE) == TIM2) &&                  \
    (((__SELECTION__) == TIM_TS_TI1F_ED) ||          \
     ((__SELECTION__) == TIM_TS_TI1FP1)  ||          \
     ((__SELECTION__) == TIM_TS_TI2FP2)  ||          \
     ((__SELECTION__) == TIM_TS_ETRF)    ||          \
     ((__SELECTION__) == TIM_TS_ITR0)    ||          \
     ((__SELECTION__) == TIM_TS_ITR2)    ||          \
     ((__SELECTION__) == TIM_TS_ITR3)    ||          \
     ((__SELECTION__) == TIM_TS_ITR4)    ||          \
     ((__SELECTION__) == TIM_TS_ITR5)    ||          \
     ((__SELECTION__) == TIM_TS_ITR6)    ||          \
     ((__SELECTION__) == TIM_TS_ITR7)    ||          \
     ((__SELECTION__) == TIM_TS_ITR8)    ||          \
     ((__SELECTION__) == TIM_TS_ITR10)   ||          \
     ((__SELECTION__) == TIM_TS_ITR11)))             \
   ||                                        \
   (((INSTANCE) == TIM3) &&                  \
    (((__SELECTION__) == TIM_TS_TI1F_ED) ||          \
     ((__SELECTION__) == TIM_TS_TI1FP1)  ||          \
     ((__SELECTION__) == TIM_TS_TI2FP2)  ||          \
     ((__SELECTION__) == TIM_TS_ETRF)    ||          \
     ((__SELECTION__) == TIM_TS_ITR0)    ||          \
     ((__SELECTION__) == TIM_TS_ITR1)    ||          \
     ((__SELECTION__) == TIM_TS_ITR3)    ||          \
     ((__SELECTION__) == TIM_TS_ITR4)    ||          \
     ((__SELECTION__) == TIM_TS_ITR5)    ||          \
     ((__SELECTION__) == TIM_TS_ITR6)    ||          \
     ((__SELECTION__) == TIM_TS_ITR7)    ||          \
     ((__SELECTION__) == TIM_TS_ITR8)    ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))             \
   ||                                        \
   (((INSTANCE) == TIM4) &&                  \
    (((__SELECTION__) == TIM_TS_TI1F_ED) ||          \
     ((__SELECTION__) == TIM_TS_TI1FP1)  ||          \
     ((__SELECTION__) == TIM_TS_TI2FP2)  ||          \
     ((__SELECTION__) == TIM_TS_ETRF)    ||          \
     ((__SELECTION__) == TIM_TS_ITR0)    ||          \
     ((__SELECTION__) == TIM_TS_ITR1)    ||          \
     ((__SELECTION__) == TIM_TS_ITR2)    ||          \
     ((__SELECTION__) == TIM_TS_ITR4)    ||          \
     ((__SELECTION__) == TIM_TS_ITR5)    ||          \
     ((__SELECTION__) == TIM_TS_ITR6)    ||          \
     ((__SELECTION__) == TIM_TS_ITR7)    ||          \
     ((__SELECTION__) == TIM_TS_ITR8)    ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))             \
   ||                                        \
   (((INSTANCE) == TIM5) &&                  \
    (((__SELECTION__) == TIM_TS_TI1F_ED) ||          \
     ((__SELECTION__) == TIM_TS_TI1FP1)  ||          \
     ((__SELECTION__) == TIM_TS_TI2FP2)  ||          \
     ((__SELECTION__) == TIM_TS_ETRF)    ||          \
     ((__SELECTION__) == TIM_TS_ITR0)    ||          \
     ((__SELECTION__) == TIM_TS_ITR1)    ||          \
     ((__SELECTION__) == TIM_TS_ITR2)    ||          \
     ((__SELECTION__) == TIM_TS_ITR3)    ||          \
     ((__SELECTION__) == TIM_TS_ITR5)    ||          \
     ((__SELECTION__) == TIM_TS_ITR6)    ||          \
     ((__SELECTION__) == TIM_TS_ITR7)    ||          \
     ((__SELECTION__) == TIM_TS_ITR8)    ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))             \
   ||                                        \
   (((INSTANCE) == TIM8) &&                  \
    (((__SELECTION__) == TIM_TS_TI1F_ED) ||          \
     ((__SELECTION__) == TIM_TS_TI1FP1)  ||          \
     ((__SELECTION__) == TIM_TS_TI2FP2)  ||          \
     ((__SELECTION__) == TIM_TS_ETRF)    ||          \
     ((__SELECTION__) == TIM_TS_ITR0)    ||          \
     ((__SELECTION__) == TIM_TS_ITR1)    ||          \
     ((__SELECTION__) == TIM_TS_ITR2)    ||          \
     ((__SELECTION__) == TIM_TS_ITR3)    ||          \
     ((__SELECTION__) == TIM_TS_ITR4)    ||          \
     ((__SELECTION__) == TIM_TS_ITR6)    ||          \
     ((__SELECTION__) == TIM_TS_ITR7)    ||          \
     ((__SELECTION__) == TIM_TS_ITR8)    ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))             \
   ||                                        \
   (((INSTANCE) == TIM15) &&                 \
    (((__SELECTION__) == TIM_TS_TI1F_ED) ||          \
     ((__SELECTION__) == TIM_TS_TI1FP1)  ||          \
     ((__SELECTION__) == TIM_TS_TI2FP2)  ||          \
     ((__SELECTION__) == TIM_TS_ITR0)    ||          \
     ((__SELECTION__) == TIM_TS_ITR1)    ||          \
     ((__SELECTION__) == TIM_TS_ITR2)    ||          \
     ((__SELECTION__) == TIM_TS_ITR3)    ||          \
     ((__SELECTION__) == TIM_TS_ITR4)    ||          \
     ((__SELECTION__) == TIM_TS_ITR5)    ||          \
     ((__SELECTION__) == TIM_TS_ITR7)    ||          \
     ((__SELECTION__) == TIM_TS_ITR8)    ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))     \
   ||                                        \
   (((INSTANCE) == TIM16) &&                 \
    (((__SELECTION__) == TIM_TS_TI1F_ED) ||          \
     ((__SELECTION__) == TIM_TS_TI1FP1)  ||          \
     ((__SELECTION__) == TIM_TS_TI2FP2)  ||          \
     ((__SELECTION__) == TIM_TS_ITR0)    ||          \
     ((__SELECTION__) == TIM_TS_ITR1)    ||          \
     ((__SELECTION__) == TIM_TS_ITR2)    ||          \
     ((__SELECTION__) == TIM_TS_ITR3)    ||          \
     ((__SELECTION__) == TIM_TS_ITR4)    ||          \
     ((__SELECTION__) == TIM_TS_ITR5)    ||          \
     ((__SELECTION__) == TIM_TS_ITR7)    ||          \
     ((__SELECTION__) == TIM_TS_ITR8)    ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))     \
   ||                                        \
   (((INSTANCE) == TIM17) &&                 \
    (((__SELECTION__) == TIM_TS_TI1F_ED) ||          \
     ((__SELECTION__) == TIM_TS_TI1FP1)  ||          \
     ((__SELECTION__) == TIM_TS_TI2FP2)  ||          \
     ((__SELECTION__) == TIM_TS_ITR0)    ||          \
     ((__SELECTION__) == TIM_TS_ITR1)    ||          \
     ((__SELECTION__) == TIM_TS_ITR2)    ||          \
     ((__SELECTION__) == TIM_TS_ITR3)    ||          \
     ((__SELECTION__) == TIM_TS_ITR4)    ||          \
     ((__SELECTION__) == TIM_TS_ITR5)    ||          \
     ((__SELECTION__) == TIM_TS_ITR7)    ||          \
     ((__SELECTION__) == TIM_TS_ITR8)    ||          \
     ((__SELECTION__) == TIM_TS_ITR10))))

/** @brief  Check if the parameter __REMAP__ is valid
  * @param  __REMAP__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_INTERNAL_TRIGGEREVENT_INSTANCE(INSTANCE, __SELECTION__) \
  ((((INSTANCE) == TIM1) &&                  \
    (((__SELECTION__) == TIM_TS_ITR1) ||          \
     ((__SELECTION__) == TIM_TS_ITR2) ||          \
     ((__SELECTION__) == TIM_TS_ITR3) ||          \
     ((__SELECTION__) == TIM_TS_ITR4) ||          \
     ((__SELECTION__) == TIM_TS_ITR5) ||          \
     ((__SELECTION__) == TIM_TS_ITR6) ||          \
     ((__SELECTION__) == TIM_TS_ITR7) ||          \
     ((__SELECTION__) == TIM_TS_ITR8) ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))           \
   ||                                        \
   (((INSTANCE) == TIM2) &&                  \
    (((__SELECTION__) == TIM_TS_ITR0) ||          \
     ((__SELECTION__) == TIM_TS_ITR2) ||          \
     ((__SELECTION__) == TIM_TS_ITR3) ||          \
     ((__SELECTION__) == TIM_TS_ITR4) ||          \
     ((__SELECTION__) == TIM_TS_ITR5) ||          \
     ((__SELECTION__) == TIM_TS_ITR6) ||          \
     ((__SELECTION__) == TIM_TS_ITR7) ||          \
     ((__SELECTION__) == TIM_TS_ITR8) ||          \
     ((__SELECTION__) == TIM_TS_ITR10)||          \
     ((__SELECTION__) == TIM_TS_ITR11)))           \
   ||                                        \
   (((INSTANCE) == TIM3) &&                  \
    (((__SELECTION__) == TIM_TS_ITR0) ||          \
     ((__SELECTION__) == TIM_TS_ITR1) ||          \
     ((__SELECTION__) == TIM_TS_ITR3) ||          \
     ((__SELECTION__) == TIM_TS_ITR4) ||          \
     ((__SELECTION__) == TIM_TS_ITR5) ||          \
     ((__SELECTION__) == TIM_TS_ITR6) ||          \
     ((__SELECTION__) == TIM_TS_ITR7) ||          \
     ((__SELECTION__) == TIM_TS_ITR8) ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))           \
   ||                                        \
   (((INSTANCE) == TIM4) &&                  \
    (((__SELECTION__) == TIM_TS_ITR0) ||          \
     ((__SELECTION__) == TIM_TS_ITR1) ||          \
     ((__SELECTION__) == TIM_TS_ITR2) ||          \
     ((__SELECTION__) == TIM_TS_ITR4) ||          \
     ((__SELECTION__) == TIM_TS_ITR5) ||          \
     ((__SELECTION__) == TIM_TS_ITR6) ||          \
     ((__SELECTION__) == TIM_TS_ITR7) ||          \
     ((__SELECTION__) == TIM_TS_ITR8) ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))           \
   ||                                        \
   (((INSTANCE) == TIM5) &&                  \
    (((__SELECTION__) == TIM_TS_ITR0) ||          \
     ((__SELECTION__) == TIM_TS_ITR1) ||          \
     ((__SELECTION__) == TIM_TS_ITR2) ||          \
     ((__SELECTION__) == TIM_TS_ITR3) ||          \
     ((__SELECTION__) == TIM_TS_ITR5) ||          \
     ((__SELECTION__) == TIM_TS_ITR6) ||          \
     ((__SELECTION__) == TIM_TS_ITR7) ||          \
     ((__SELECTION__) == TIM_TS_ITR8) ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))           \
   ||                                        \
   (((INSTANCE) == TIM8) &&                  \
    (((__SELECTION__) == TIM_TS_ITR0) ||          \
     ((__SELECTION__) == TIM_TS_ITR1) ||          \
     ((__SELECTION__) == TIM_TS_ITR2) ||          \
     ((__SELECTION__) == TIM_TS_ITR3) ||          \
     ((__SELECTION__) == TIM_TS_ITR4) ||          \
     ((__SELECTION__) == TIM_TS_ITR6) ||          \
     ((__SELECTION__) == TIM_TS_ITR7) ||          \
     ((__SELECTION__) == TIM_TS_ITR8) ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))           \
   ||                                        \
   (((INSTANCE) == TIM15) &&                 \
    (((__SELECTION__) == TIM_TS_ITR0) ||          \
     ((__SELECTION__) == TIM_TS_ITR1) ||          \
     ((__SELECTION__) == TIM_TS_ITR2) ||          \
     ((__SELECTION__) == TIM_TS_ITR3) ||          \
     ((__SELECTION__) == TIM_TS_ITR4) ||          \
     ((__SELECTION__) == TIM_TS_ITR5) ||          \
     ((__SELECTION__) == TIM_TS_ITR7) ||          \
     ((__SELECTION__) == TIM_TS_ITR8) ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))\
   ||                                        \
   (((INSTANCE) == TIM16) &&                 \
    (((__SELECTION__) == TIM_TS_ITR0) ||          \
     ((__SELECTION__) == TIM_TS_ITR1) ||          \
     ((__SELECTION__) == TIM_TS_ITR2) ||          \
     ((__SELECTION__) == TIM_TS_ITR3) ||          \
     ((__SELECTION__) == TIM_TS_ITR4) ||          \
     ((__SELECTION__) == TIM_TS_ITR5) ||          \
     ((__SELECTION__) == TIM_TS_ITR7) ||          \
     ((__SELECTION__) == TIM_TS_ITR8) ||          \
     ((__SELECTION__) == TIM_TS_ITR10)))\
   ||                                        \
   (((INSTANCE) == TIM17) &&                 \
    (((__SELECTION__) == TIM_TS_ITR0) ||          \
     ((__SELECTION__) == TIM_TS_ITR1) ||          \
     ((__SELECTION__) == TIM_TS_ITR2) ||          \
     ((__SELECTION__) == TIM_TS_ITR3) ||          \
     ((__SELECTION__) == TIM_TS_ITR4) ||          \
     ((__SELECTION__) == TIM_TS_ITR5) ||          \
     ((__SELECTION__) == TIM_TS_ITR7) ||          \
     ((__SELECTION__) == TIM_TS_ITR8) ||          \
     ((__SELECTION__) == TIM_TS_ITR10))))

/** @brief  Check if the parameter __MODE__, __CHANNEL__ is valid
  * @param  __MODE__
  * @param  __CHANNEL__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_OC_CHANNEL_MODE(__MODE__, __CHANNEL__)   \
  (IS_TIM_OC_MODE(__MODE__) \
   && ((((__MODE__) == TIM_OCMODE_DIRECTION_OUTPUT) || ((__MODE__) == TIM_OCMODE_PULSE_ON_COMPARE)) \
       ? (((__CHANNEL__) == TIM_CHANNEL_3) || ((__CHANNEL__) == TIM_CHANNEL_4)) : (1 == 1)))

/** @brief  Check if the parameter __CHANNEL__ is valid
  * @param  __CHANNEL__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_PULSEONCOMPARE_CHANNEL(__CHANNEL__)  \
  (((__CHANNEL__) == TIM_CHANNEL_3) ||    \
   ((__CHANNEL__) == TIM_CHANNEL_4))

/** @brief  Check if the parameter INSTANCE is valid
  * @param  INSTANCE
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_PULSEONCOMPARE_INSTANCE(INSTANCE)  IS_TIM_CC3_INSTANCE(INSTANCE)

/** @brief  Check if the parameter __WIDTH__ is valid
  * @param  __WIDTH__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_PULSEONCOMPARE_WIDTH(__WIDTH__)    ((__WIDTH__) <= 0xFFU)

/** @brief  Check if the parameter __PRESCALER__ is valid
  * @param  __PRESCALER__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_PULSEONCOMPARE_WIDTHPRESCALER(__PRESCALER__)    ((__PRESCALER__) <= 0x7U)

/** @brief  Check if the parameter __SOURCE__ is valid
  * @param  __SOURCE__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_SLAVE_PRELOAD_SOURCE(__SOURCE__)    (((__SOURCE__) == TIM_SMS_PRELOAD_SOURCE_UPDATE) \
                                                    || ((__SOURCE__) == TIM_SMS_PRELOAD_SOURCE_INDEX))

/** @brief  Check if the parameter __POLARITY__ is valid
  * @param  __POLARITY__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_ENCODERINDEX_POLARITY(__POLARITY__)        (((__POLARITY__) == TIM_ENCODERINDEX_POLARITY_INVERTED)  || \
                                                           ((__POLARITY__) == TIM_ENCODERINDEX_POLARITY_NONINVERTED))

/** @brief  Check if the parameter __PRESCALER__ is valid
  * @param  __PRESCALER__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_ENCODERINDEX_PRESCALER(__PRESCALER__)      (((__PRESCALER__) == TIM_ENCODERINDEX_PRESCALER_DIV1) || \
                                                           ((__PRESCALER__) == TIM_ENCODERINDEX_PRESCALER_DIV2) || \
                                                           ((__PRESCALER__) == TIM_ENCODERINDEX_PRESCALER_DIV4) || \
                                                           ((__PRESCALER__) == TIM_ENCODERINDEX_PRESCALER_DIV8))

/** @brief  Check if the parameter __FILTER__ is valid
  * @param  __FILTER__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_ENCODERINDEX_FILTER(__FILTER__)            ((__FILTER__) <= 0xFUL)

/** @brief  Check if the parameter __POSITION__ is valid
  * @param  __POSITION__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_ENCODERINDEX_POSITION(__POSITION__)        (((__POSITION__) == TIM_ENCODERINDEX_POSITION_00) || \
                                                           ((__POSITION__) == TIM_ENCODERINDEX_POSITION_01) || \
                                                           ((__POSITION__) == TIM_ENCODERINDEX_POSITION_10) || \
                                                           ((__POSITION__) == TIM_ENCODERINDEX_POSITION_11) || \
                                                           ((__POSITION__) == TIM_ENCODERINDEX_POSITION_0)  || \
                                                           ((__POSITION__) == TIM_ENCODERINDEX_POSITION_1))

/** @brief  Check if the parameter __DIRECTION__ is valid
  * @param  __DIRECTION__
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_TIM_ENCODERINDEX_DIRECTION(__DIRECTION__)      (((__DIRECTION__) == TIM_ENCODERINDEX_DIRECTION_UP_DOWN) || \
                                                           ((__DIRECTION__) == TIM_ENCODERINDEX_DIRECTION_UP)      || \
                                                           ((__DIRECTION__) == TIM_ENCODERINDEX_DIRECTION_DOWN))
/** 
  * end of TIM_Macro_Definitions @}  
  */

  
/******************************************************************************/
/*                                TIM Functions                               */
/******************************************************************************/

/** @addtogroup TIM_Function_Definitions TIM Function Definitions
  * @{
  */

/*  Timer Hall Sensor functions  **********************************************/
HAL_StatusTypeDef               HAL_TIMEx_HallSensor_Init(TIM_HandleTypeDef *htim, const TIM_HallSensor_InitTypeDef *sConfig);
HAL_StatusTypeDef               HAL_TIMEx_HallSensor_DeInit(TIM_HandleTypeDef *htim);

void                            HAL_TIMEx_HallSensor_MspInit(TIM_HandleTypeDef *htim);
void                            HAL_TIMEx_HallSensor_MspDeInit(TIM_HandleTypeDef *htim);

/* Blocking mode: Polling */
HAL_StatusTypeDef               HAL_TIMEx_HallSensor_Start(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_HallSensor_Stop(TIM_HandleTypeDef *htim);
/* Non-Blocking mode: Interrupt */
HAL_StatusTypeDef               HAL_TIMEx_HallSensor_Start_IT(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_HallSensor_Stop_IT(TIM_HandleTypeDef *htim);
/* Non-Blocking mode: DMA */
HAL_StatusTypeDef               HAL_TIMEx_HallSensor_Start_DMA(TIM_HandleTypeDef *htim, uint32_t *pData, uint16_t Length);
HAL_StatusTypeDef               HAL_TIMEx_HallSensor_Stop_DMA(TIM_HandleTypeDef *htim);

/*  Timer Complementary Output Compare functions  *****************************/
/* Blocking mode: Polling */
HAL_StatusTypeDef               HAL_TIMEx_OCN_Start(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef               HAL_TIMEx_OCN_Stop(TIM_HandleTypeDef *htim, uint32_t Channel);

/* Non-Blocking mode: Interrupt */
HAL_StatusTypeDef               HAL_TIMEx_OCN_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef               HAL_TIMEx_OCN_Stop_IT(TIM_HandleTypeDef *htim, uint32_t Channel);

/* Non-Blocking mode: DMA */
HAL_StatusTypeDef               HAL_TIMEx_OCN_Start_DMA(TIM_HandleTypeDef *htim, uint32_t Channel, const uint32_t *pData,
                                                        uint16_t Length);
HAL_StatusTypeDef               HAL_TIMEx_OCN_Stop_DMA(TIM_HandleTypeDef *htim, uint32_t Channel);

/*  Timer Complementary PWM functions  ****************************************/
/* Blocking mode: Polling */
HAL_StatusTypeDef               HAL_TIMEx_PWMN_Start(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef               HAL_TIMEx_PWMN_Stop(TIM_HandleTypeDef *htim, uint32_t Channel);

/* Non-Blocking mode: Interrupt */
HAL_StatusTypeDef               HAL_TIMEx_PWMN_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel);
HAL_StatusTypeDef               HAL_TIMEx_PWMN_Stop_IT(TIM_HandleTypeDef *htim, uint32_t Channel);
/* Non-Blocking mode: DMA */
HAL_StatusTypeDef               HAL_TIMEx_PWMN_Start_DMA(TIM_HandleTypeDef *htim, uint32_t Channel, const uint32_t *pData,
                                                        uint16_t Length);
HAL_StatusTypeDef               HAL_TIMEx_PWMN_Stop_DMA(TIM_HandleTypeDef *htim, uint32_t Channel);

/*  Timer Complementary One Pulse functions  **********************************/
/* Blocking mode: Polling */
HAL_StatusTypeDef               HAL_TIMEx_OnePulseN_Start(TIM_HandleTypeDef *htim, uint32_t OutputChannel);
HAL_StatusTypeDef               HAL_TIMEx_OnePulseN_Stop(TIM_HandleTypeDef *htim, uint32_t OutputChannel);

/* Non-Blocking mode: Interrupt */
HAL_StatusTypeDef               HAL_TIMEx_OnePulseN_Start_IT(TIM_HandleTypeDef *htim, uint32_t OutputChannel);
HAL_StatusTypeDef               HAL_TIMEx_OnePulseN_Stop_IT(TIM_HandleTypeDef *htim, uint32_t OutputChannel);

/* Extended Control functions  ************************************************/
HAL_StatusTypeDef               HAL_TIMEx_ConfigCommutEvent(TIM_HandleTypeDef *htim, uint32_t  InputTrigger,
                                                            uint32_t  CommutationSource);
HAL_StatusTypeDef               HAL_TIMEx_ConfigCommutEvent_IT(TIM_HandleTypeDef *htim, uint32_t  InputTrigger,
                                                                uint32_t  CommutationSource);
HAL_StatusTypeDef               HAL_TIMEx_ConfigCommutEvent_DMA(TIM_HandleTypeDef *htim, uint32_t  InputTrigger,
                                                                uint32_t  CommutationSource);
HAL_StatusTypeDef               HAL_TIMEx_MasterConfigSynchronization(TIM_HandleTypeDef *htim,
                                                                        const TIM_MasterConfigTypeDef *sMasterConfig);
HAL_StatusTypeDef               HAL_TIMEx_ConfigBreakDeadTime(TIM_HandleTypeDef *htim,
                                                                const TIM_BreakDeadTimeConfigTypeDef *sBreakDeadTimeConfig);
HAL_StatusTypeDef               HAL_TIMEx_ConfigBreakInput(TIM_HandleTypeDef *htim, uint32_t BreakInput,
                                                            const TIMEx_BreakInputConfigTypeDef *sBreakInputConfig);
HAL_StatusTypeDef               HAL_TIMEx_GroupChannel5(TIM_HandleTypeDef *htim, uint32_t Channels);
HAL_StatusTypeDef               HAL_TIMEx_RemapConfig(TIM_HandleTypeDef *htim, uint32_t Remap);
HAL_StatusTypeDef               HAL_TIMEx_TISelection(TIM_HandleTypeDef *htim, uint32_t TISelection, uint32_t Channel);

HAL_StatusTypeDef               HAL_TIMEx_DisarmBreakInput(TIM_HandleTypeDef *htim, uint32_t BreakInput);
HAL_StatusTypeDef               HAL_TIMEx_ReArmBreakInput(const TIM_HandleTypeDef *htim, uint32_t BreakInput);
HAL_StatusTypeDef               HAL_TIMEx_DitheringEnable(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_DitheringDisable(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_OC_ConfigPulseOnCompare(TIM_HandleTypeDef *htim, uint32_t PulseWidthPrescaler,
                                                                    uint32_t PulseWidth);
HAL_StatusTypeDef               HAL_TIMEx_ConfigSlaveModePreload(TIM_HandleTypeDef *htim, uint32_t Source);
HAL_StatusTypeDef               HAL_TIMEx_EnableSlaveModePreload(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_DisableSlaveModePreload(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_EnableDeadTimePreload(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_DisableDeadTimePreload(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_ConfigDeadTime(TIM_HandleTypeDef *htim, uint32_t Deadtime);
HAL_StatusTypeDef               HAL_TIMEx_ConfigAsymmetricalDeadTime(TIM_HandleTypeDef *htim, uint32_t FallingDeadtime);
HAL_StatusTypeDef               HAL_TIMEx_EnableAsymmetricalDeadTime(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_DisableAsymmetricalDeadTime(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_ConfigEncoderIndex(TIM_HandleTypeDef *htim,
                                                            TIMEx_EncoderIndexConfigTypeDef *sEncoderIndexConfig);
HAL_StatusTypeDef               HAL_TIMEx_EnableEncoderIndex(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_DisableEncoderIndex(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_EnableEncoderFirstIndex(TIM_HandleTypeDef *htim);
HAL_StatusTypeDef               HAL_TIMEx_DisableEncoderFirstIndex(TIM_HandleTypeDef *htim);

/* Extended Callback **********************************************************/
void                            HAL_TIMEx_CommutCallback(TIM_HandleTypeDef *htim);
void                            HAL_TIMEx_CommutHalfCpltCallback(TIM_HandleTypeDef *htim);
void                            HAL_TIMEx_BreakCallback(TIM_HandleTypeDef *htim);
void                            HAL_TIMEx_Break2Callback(TIM_HandleTypeDef *htim);
void                            HAL_TIMEx_EncoderIndexCallback(TIM_HandleTypeDef *htim);
void                            HAL_TIMEx_DirectionChangeCallback(TIM_HandleTypeDef *htim);
void                            HAL_TIMEx_IndexErrorCallback(TIM_HandleTypeDef *htim);
void                            HAL_TIMEx_TransitionErrorCallback(TIM_HandleTypeDef *htim);

/* Extended Peripheral State functions  ***************************************/
HAL_TIM_StateTypeDef            HAL_TIMEx_HallSensor_GetState(const TIM_HandleTypeDef *htim);
HAL_TIM_ChannelStateTypeDef     HAL_TIMEx_GetChannelNState(const TIM_HandleTypeDef *htim,  uint32_t ChannelN);

void                            TIMEx_DMACommutationCplt(DMA_HandleTypeDef *hdma);
void                            TIMEx_DMACommutationHalfCplt(DMA_HandleTypeDef *hdma);

/** 
  * end of TIM @}  
  */


#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_TIM_EX_H_ */
