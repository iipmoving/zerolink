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
*   File    : rx32g4xx_hal_adc.h
*   By      : RX_DV_Team
**************************************************************************************************
*/



#ifndef _RX32G4XX_HAL_ADC_H_
#define _RX32G4XX_HAL_ADC_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"
#include "rx32g4xx_hal_dma.h"

/* Exported types ------------------------------------------------------------*/
/** @addtogroup ADC
  * @{
  */ 

/******************************************************************************/
/*                              ADC Structures                                */
/******************************************************************************/
  
/** @defgroup ADC_Structure_Definitions ADC Structure Definitions
  * @{
  */
  
/** @defgroup ADC_InitTypeDef ADC InitTypeDef
  * @ingroup  ADC_Structure_Definitions 
  * @{
  */
/**
  * @brief  Structure definition of ADC and regular group initialization 
  * @note   The setting of these parameters with function HAL_ADC_Init() is conditioned to ADC state.
  *         ADC can be either disabled or enabled without conversion on going on regular group.
  */
typedef struct
{
  uint32_t DataAlign;                        /*!< Specifies ADC data alignment to right or to left.
                                                  This parameter can be a value of @ref ADC_Data_align */
  uint32_t ScanConvMode;                     /*!< Configures the sequencer of regular and injected groups.
                                                  If disabled: Conversion is performed in single mode.
                                                  If enabled:  Conversions are performed in sequence mode
                                                  This parameter can be a value of @ref ADC_Scan_mode */
  FunctionalState ContinuousConvMode;         /*!< Specifies whether the conversion is performed in single mode (one conversion) or continuous mode for regular group,
                                                  after the selected trigger occurred (software start or external trigger).
                                                  This parameter can be set to ENABLE or DISABLE. */
  uint32_t NbrOfConversion;                  /*!< Specifies the number of ranks that will be converted within the regular group sequencer.
                                                  To use regular group sequencer and convert several ranks, parameter 'ScanConvMode' must be enabled.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  FunctionalState  DiscontinuousConvMode;    /*!< Specifies whether the conversions sequence of regular group is performed in Complete-sequence/Discontinuous-sequence (main sequence subdivided in successive parts).
                                                  Discontinuous mode is used only if sequencer is enabled (parameter 'ScanConvMode'). If sequencer is disabled, this parameter is discarded.
                                                  Discontinuous mode can be enabled only if continuous mode is disabled. If continuous mode is enabled, this parameter setting is discarded.
                                                  This parameter can be set to ENABLE or DISABLE. */
  uint32_t NbrOfDiscConversion;              /*!< Specifies the number of discontinuous conversions in which the  main sequence of regular group (parameter NbrOfConversion) will be subdivided.
                                                  If parameter 'DiscontinuousConvMode' is disabled, this parameter is discarded.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  uint32_t ExternalTrigConv;                 /*!< Selects the external event used to trigger the conversion start of regular group.
                                                  If set to ADC1_CR2_EXTSEL_SWSTART, external triggers are disabled.
                                                  If set to external trigger source, triggering is on event rising edge.
                                                  This parameter can be a value of @ref ADC_External_trigger_Regular */
} ADC_InitTypeDef;
  
/** 
  * end of ADC_InitTypeDef @}  
  */
  
/** @defgroup ADC_ChannelConfTypeDef ADC ChannelConfTypeDef
  * @ingroup  ADC_Structure_Definitions 
  * @{
  */
/** 
  * @brief  Structure definition of ADC channel for regular group   
  * @note   The setting of these parameters with function HAL_ADC_ConfigChannel() is conditioned to ADC state.
  *         ADC can be either disabled or enabled without conversion on going on regular group.
  */ 
typedef struct 
{
  uint32_t Channel;                /*!< Specifies the channel to configure into ADC regular group.
                                        This parameter can be a value of @ref ADC_channels */
  uint32_t Rank;                   /*!< Specifies the rank in the regular group sequencer 
                                        This parameter can be a value of @ref ADC_regular_rank */
  uint32_t SamplingTime;           /*!< Sampling time value to be set for the selected channel.
                                        Unit: ADC clock cycles
                                        Conversion time is the addition of sampling time and processing time (12.5 ADC clock cycles at ADC resolution 12 bits).
                                        This parameter can be a value of @ref ADC_sampling_times */
} ADC_ChannelConfTypeDef;
  
/** 
  * end of ADC_ChannelConfTypeDef @}  
  */
  
/** @defgroup ADC_AnalogWDGConfTypeDef ADC AnalogWDGConfTypeDef
  * @ingroup  ADC_Structure_Definitions 
  * @{
  */
/**
  * @brief  ADC Configuration analog watchdog definition
  * @note   The setting of these parameters with function is conditioned to ADC state.
  *         ADC state can be either disabled or enabled without conversion on going on regular and injected groups.
  */
typedef struct
{
  uint32_t WatchdogMode;      /*!< Configures the ADC analog watchdog mode: single/all channels, regular/injected group.
                                   This parameter can be a value of @ref ADC_analog_watchdog_mode. */
  uint32_t Channel;           /*!< Selects which ADC channel to monitor by analog watchdog.
                                   This parameter has an effect only if watchdog mode is configured on single channel (parameter WatchdogMode)
                                   This parameter can be a value of @ref ADC_channels. */
  FunctionalState  ITMode;    /*!< Specifies whether the analog watchdog is configured in interrupt or polling mode.
                                   This parameter can be set to ENABLE or DISABLE */
  uint32_t HighThreshold;     /*!< Configures the ADC analog watchdog High threshold value.
                                   This parameter must be a number between Min_Data = 0x000 and Max_Data = 0xFFF. */
  uint32_t LowThreshold;      /*!< Configures the ADC analog watchdog High threshold value.
                                   This parameter must be a number between Min_Data = 0x000 and Max_Data = 0xFFF. */
  uint32_t WatchdogFilter;    /*!< Configures the ADC analog watchdog filter
                                   This parameter can a value of @ref ADC_analog_watchdog_filter.  */
  
} ADC_AnalogWDGConfTypeDef;
  
/** 
  * end of ADC_AnalogWDGConfTypeDef @}  
  */
  
/** @defgroup ADC_OptionalConfTypeDef ADC OptionalConfTypeDef
  * @ingroup  ADC_Structure_Definitions 
  * @{
  */
/** 
  * @brief ADC optional definition for additionanl setting
  * @note  The setting is for addtional function. Use ADC_OptioanlConfig() to config the setting
  */
typedef struct
{
    uint32_t TPSOffset;                   /*!< Configure the ADC Temperature sensor offset
                                             This parameter must be a value between Min_Data = 0x0 and Max_Data = 0xFF 
                                             Notice the MSB is sign-bit */
    uint32_t SampleTime0;                 /*!< Configure the ADC sample time idx0 be user definition
                                             This parameter must be a number between Min_Data = 0x1 and Max_Data = 0xFF */
    uint32_t SampleTime1;                 /*!< Configure the ADC sample time idx1 be user definition
                                             This parameter must be a number between Min_Data = 0x1 and Max_Data = 0xFF */
    uint32_t SampleTime2;                 /*!< Configure the ADC sample time idx2 be user definition
                                             This parameter must be a number between Min_Data = 0x1 and Max_Data = 0xFF */
    uint32_t SampleTime3;                 /*!< Configure the ADC sample time idx3 be user definition
                                             This parameter must be a number between Min_Data = 0x1 and Max_Data = 0xFF */
    uint32_t BlankingSampleTimeMode;      /*!< Configure the COMP blanking time by ADC sample time or sample time + 12p5 cycles
                                             This parameter must be ENABLE or DISABLE */
    uint32_t EOCOptionalFunction;         /*!< Configure the ADC EOC optional setting
                                             This parameter must be ENABLE or DISABLE */
} ADC_OptionalConfTypeDef;

/** 
  * end of ADC_OptionalConfTypeDef @}  
  */

/** @defgroup ADC_HandleTypeDef ADC HandleTypeDef
  * @ingroup  ADC_Structure_Definitions 
  * @{
  */
/**
  * @brief  ADC handle Structure definition  
  */ 
typedef struct __ADC_HandleTypeDef
{
  ADC_TypeDef                   *Instance;              /*!< Register base address */

  ADC_InitTypeDef                Init;                  /*!< ADC required parameters @ref HAL_ADC_InitTypeDef */

  DMA_HandleTypeDef             *DMA_Handle;            /*!< Pointer DMA Handler @ref DMA_HandleTypeDef */

  HAL_LockTypeDef               Lock;                   /*!< ADC locking object @ref HAL_LockTypeDef */
  
  __IO uint32_t                 State;                  /*!< ADC communication state (bitmap of ADC states) @ref ADC_State_Machiine */

  __IO uint32_t                 ErrorCode;              /*!< ADC Error code @ref ADC_Error_code */

#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
  void (* ConvCpltCallback)(struct __ADC_HandleTypeDef *hadc);              /*!< ADC conversion complete callback */
  void (* ConvHalfCpltCallback)(struct __ADC_HandleTypeDef *hadc);          /*!< ADC conversion DMA half-transfer callback */
  void (* LevelOutOfWindowCallback)(struct __ADC_HandleTypeDef *hadc);      /*!< ADC analog watchdog 1 callback */
  void (* ErrorCallback)(struct __ADC_HandleTypeDef *hadc);                 /*!< ADC error callback */
  void (* InjectedConvCpltCallback)(struct __ADC_HandleTypeDef *hadc);      /*!< ADC group injected conversion complete callback */       /*!< ADC end of sampling callback */
  void (* MspInitCallback)(struct __ADC_HandleTypeDef *hadc);               /*!< ADC Msp Init callback */
  void (* MspDeInitCallback)(struct __ADC_HandleTypeDef *hadc);             /*!< ADC Msp DeInit callback */
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */
} ADC_HandleTypeDef;


#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL ADC Callback ID enumeration definition
  */
typedef enum
{
  HAL_ADC_CONVERSION_COMPLETE_CB_ID     = 0x00U,  /*!< ADC conversion complete callback ID */
  HAL_ADC_CONVERSION_HALF_CB_ID         = 0x01U,  /*!< ADC conversion DMA half-transfer callback ID */
  HAL_ADC_LEVEL_OUT_OF_WINDOW_1_CB_ID   = 0x02U,  /*!< ADC analog watchdog 1 callback ID */
  HAL_ADC_ERROR_CB_ID                   = 0x03U,  /*!< ADC error callback ID */
  HAL_ADC_INJ_CONVERSION_COMPLETE_CB_ID = 0x04U,  /*!< ADC group injected conversion complete callback ID */
  HAL_ADC_MSPINIT_CB_ID                 = 0x09U,  /*!< ADC Msp Init callback ID          */
  HAL_ADC_MSPDEINIT_CB_ID               = 0x0AU   /*!< ADC Msp DeInit callback ID        */
} HAL_ADC_CallbackIDTypeDef;

/**
  * @brief  HAL ADC Callback pointer definition
  */
typedef  void (*pADC_CallbackTypeDef)(ADC_HandleTypeDef *hadc); /*!< pointer to a ADC callback function */

#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */


/** 
  * end of ADC_HandleTypeDef @}  
  */
/** 
  * end of ADC_Structure_Definitions @}  
  */

/******************************************************************************/
/*                               ADC Parameters                               */
/******************************************************************************/
  
/** @defgroup ADC_Parameter_Definitions ADC Parameter Definitions
  * @{
  */
  
/** @defgroup ADC_State_Machiine ADC State Machiine
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
/** 
  * @brief  HAL ADC state machine: ADC states definition (bitfields)
  */ 
/* States of ADC global scope */
#define HAL_ADC_STATE_RESET             0x00000000U    /*!< ADC not yet initialized or disabled */
#define HAL_ADC_STATE_READY             0x00000001U    /*!< ADC peripheral ready for use */
#define HAL_ADC_STATE_BUSY_INTERNAL     0x00000002U    /*!< ADC is busy to internal process (initialization, calibration) */
#define HAL_ADC_STATE_TIMEOUT           0x00000004U    /*!< TimeOut occurrence */

/* States of ADC errors */
#define HAL_ADC_STATE_ERROR_INTERNAL    0x00000010U    /*!< Internal error occurrence */
#define HAL_ADC_STATE_ERROR_CONFIG      0x00000020U    /*!< Configuration error occurrence */
#define HAL_ADC_STATE_ERROR_DMA         0x00000040U    /*!< DMA error occurrence */

/* States of ADC group regular */
#define HAL_ADC_STATE_REG_BUSY          0x00000100U    /*!< A conversion on group regular is ongoing or can occur (either by continuous mode,
                                                           external trigger, low power auto power-on, multimode ADC master control) */
#define HAL_ADC_STATE_REG_EOC           0x00000200U    /*!< Conversion data available on group regular */

/* States of ADC group injected */
#define HAL_ADC_STATE_INJ_BUSY          0x00001000U    /*!< A conversion on group injected is ongoing or can occur (either by auto-injection mode,
                                                           external trigger, low power auto power-on, multimode ADC master control) */
#define HAL_ADC_STATE_INJ_EOC           0x00002000U    /*!< Conversion data available on group injected */

/* States of ADC analog watchdogs */
#define HAL_ADC_STATE_AWD1              0x00010000U    /*!< Out-of-window occurrence of analog watchdog 1 */

/* States of ADC multi-mode */
#define HAL_ADC_STATE_MULTIMODE_SLAVE   0x00100000U    /*!< ADC in multimode slave state, controlled by another ADC master ( */
/** 
  * end of ADC_State_Machiine @}  
  */
  

/** @defgroup ADC_Error_code ADC Error code
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define HAL_ADC_ERROR_NONE                0x00U   /*!< No error                                              */
#define HAL_ADC_ERROR_INTERNAL            0x01U   /*!< ADC IP internal error: if problem of clocking, 
                                                       enable/disable, erroneous state                       */
#define HAL_ADC_ERROR_OVR                 0x02U   /*!< Overrun error                                         */
#define HAL_ADC_ERROR_DMA                 0x04U   /*!< DMA transfer error                                    */

#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
#define HAL_ADC_ERROR_INVALID_CALLBACK  (0x10U)   /*!< Invalid Callback error */
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */  
/** 
  * end of ADC_Error_code @}  
  */  
  
/** @defgroup ADC_channels ADC channels
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_CHANNEL_0                          0x00000000
#define ADC_CHANNEL_1                          0x00000001
#define ADC_CHANNEL_2                          0x00000002
#define ADC_CHANNEL_3                          0x00000003
#define ADC_CHANNEL_4                          0x00000004
#define ADC_CHANNEL_5                          0x00000005
#define ADC_CHANNEL_6                          0x00000006
#define ADC_CHANNEL_7                          0x00000007
#define ADC_CHANNEL_8                          0x00000008
#define ADC_CHANNEL_9                          0x00000009
#define ADC_CHANNEL_10                         0x0000000A 
#define ADC_CHANNEL_11                         0x0000000B 
#define ADC_CHANNEL_12                         0x0000000C 
#define ADC_CHANNEL_13                         0x0000000D 
#define ADC_CHANNEL_14                         0x0000000E 
#define ADC_CHANNEL_15                         0x0000000F 
#define ADC_CHANNEL_TPS                        ADC_CHANNEL_15
#define ADC_CHANNEL_VREF                       ADC_CHANNEL_15
/** 
  * end of ADC_channels @}  
  */

/** @defgroup ADC_analog_watchdog_filter ADC analog watchdog filter
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_AWD_FILTERING_NONE                ((uint32_t)0x00000000)
#define ADC_AWD_FILTERING_2SAMPLES            (                                              ADC_CR1_AWDFILT_BIT0)
#define ADC_AWD_FILTERING_3SAMPLES            (                       ADC_CR1_AWDFILT_BIT1                       )
#define ADC_AWD_FILTERING_4SAMPLES            (                       ADC_CR1_AWDFILT_BIT1 | ADC_CR1_AWDFILT_BIT0)
#define ADC_AWD_FILTERING_5SAMPLES            (ADC_CR1_AWDFILT_BIT2                                              )
#define ADC_AWD_FILTERING_6SAMPLES            (ADC_CR1_AWDFILT_BIT2                        | ADC_CR1_AWDFILT_BIT0)
#define ADC_AWD_FILTERING_7SAMPLES            (ADC_CR1_AWDFILT_BIT2 | ADC_CR1_AWDFILT_BIT1                       )     
#define ADC_AWD_FILTERING_8SAMPLES            (ADC_CR1_AWDFILT_BIT2 | ADC_CR1_AWDFILT_BIT1 | ADC_CR1_AWDFILT_BIT0)
/** 
  * end of ADC_analog_watchdog_filter @}  
  */ 
  
/** @defgroup ADC_sampling_times ADC sample time
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_SAMPLINGTIME_3CYCLES_5            ((uint32_t)0x00000000)
#define ADC_SAMPLINGTIME_7CYCLES_5            (                                            ADC_SMPR2_SMP0_BIT0)
#define ADC_SAMPLINGTIME_11CYCLES_5           (                      ADC_SMPR2_SMP0_BIT1                      )
#define ADC_SAMPLINGTIME_13CYCLES_5           (                      ADC_SMPR2_SMP0_BIT1 | ADC_SMPR2_SMP0_BIT0)
#define ADC_SAMPLINGTIME_28CYCLES_5           (ADC_SMPR2_SMP0_BIT2                                            )
#define ADC_SAMPLINGTIME_55CYCLES_5           (ADC_SMPR2_SMP0_BIT2                       | ADC_SMPR2_SMP0_BIT0)
#define ADC_SAMPLINGTIME_71CYCLES_5           (ADC_SMPR2_SMP0_BIT2 | ADC_SMPR2_SMP0_BIT1                      )
#define ADC_SAMPLINGTIME_640CYCLES_5          (ADC_SMPR2_SMP0_BIT2 | ADC_SMPR2_SMP0_BIT1 | ADC_SMPR2_SMP0_BIT0)  
/** 
  * end of ADC_sampling_times @}  
  */
  
/** @defgroup ADC_Data_align ADC data alignment
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_DATAALIGN_RIGHT      0x00000000U
#define ADC_DATAALIGN_LEFT       ((uint32_t)ADC_CR2_ALIGN)
/** 
  * end of ADC_Data_align @}  
  */
  
/** @defgroup ADC_Scan_mode ADC scan mode
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_SCAN_DISABLE         0x00000000U
#define ADC_SCAN_ENABLE          ((uint32_t)ADC_CR1_SCAN)
/** 
  * end of ADC_Scan_mode @}  
  */

/** @defgroup ADC_External_trigger_Regular ADC regular group external trigger
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC1_CR2_EXTSEL_TIM1_TRGO             ((uint32_t)0x00000000)           
#define ADC1_CR2_EXTSEL_TIM1_TRGO2            (                                                                   ADC_CR2_EXTSEL_BIT0) 
#define ADC1_CR2_EXTSEL_TIM8_TRGO             (                                             ADC_CR2_EXTSEL_BIT1                      ) 
#define ADC1_CR2_EXTSEL_TIM8_TRGO2            (                                             ADC_CR2_EXTSEL_BIT1 | ADC_CR2_EXTSEL_BIT0) 
#define ADC1_CR2_EXTSEL_TIM2_TRGO             (                       ADC_CR2_EXTSEL_BIT2                                            ) 
#define ADC1_CR2_EXTSEL_TIM3_TRGO             (                       ADC_CR2_EXTSEL_BIT2                       | ADC_CR2_EXTSEL_BIT0) 
#define ADC1_CR2_EXTSEL_TIM4_TRGO             (                       ADC_CR2_EXTSEL_BIT2 | ADC_CR2_EXTSEL_BIT1                      ) 
#define ADC1_CR2_EXTSEL_SWSTART               (                       ADC_CR2_EXTSEL_BIT2 | ADC_CR2_EXTSEL_BIT1 | ADC_CR2_EXTSEL_BIT0) 
#define ADC1_CR2_EXTSEL_EXTI2                 (ADC_CR2_EXTSEL_BIT3                                                                   )         
#define ADC1_CR2_EXTSEL_TIM6_TRGO             (ADC_CR2_EXTSEL_BIT3 |                                              ADC_CR2_EXTSEL_BIT0) 
#define ADC1_CR2_EXTSEL_hrtim_adc_trg1        (ADC_CR2_EXTSEL_BIT3 |                        ADC_CR2_EXTSEL_BIT1                      ) 
#define ADC1_CR2_EXTSEL_hrtim_adc_trg6        (ADC_CR2_EXTSEL_BIT3 |                        ADC_CR2_EXTSEL_BIT1 | ADC_CR2_EXTSEL_BIT0) 
#define ADC1_CR2_EXTSEL_hrtim_adc_trg7        (ADC_CR2_EXTSEL_BIT3 |  ADC_CR2_EXTSEL_BIT2                                            ) 
#define ADC1_CR2_EXTSEL_hrtim_adc_trg8        (ADC_CR2_EXTSEL_BIT3 |  ADC_CR2_EXTSEL_BIT2                       | ADC_CR2_EXTSEL_BIT0) 
#define ADC1_CR2_EXTSEL_hrtim_adc_trg9        (ADC_CR2_EXTSEL_BIT3 |  ADC_CR2_EXTSEL_BIT2 | ADC_CR2_EXTSEL_BIT1                      ) 
#define ADC1_CR2_EXTSEL_hrtim_adc_trg10       (ADC_CR2_EXTSEL_BIT3 |  ADC_CR2_EXTSEL_BIT2 | ADC_CR2_EXTSEL_BIT1 | ADC_CR2_EXTSEL_BIT0) 
                                              
#define ADC2_CR2_EXTSEL_TIM1_TRGO             ADC1_CR2_EXTSEL_TIM1_TRGO
#define ADC2_CR2_EXTSEL_TIM1_TRGO2            ADC1_CR2_EXTSEL_TIM1_TRGO2
#define ADC2_CR2_EXTSEL_TIM8_TRGO             ADC1_CR2_EXTSEL_TIM8_TRGO
#define ADC2_CR2_EXTSEL_TIM8_TRGO2            ADC1_CR2_EXTSEL_TIM8_TRGO2 
#define ADC2_CR2_EXTSEL_TIM2_TRGO             ADC1_CR2_EXTSEL_TIM2_TRGO
#define ADC2_CR2_EXTSEL_TIM3_TRGO             ADC1_CR2_EXTSEL_TIM3_TRGO    
#define ADC2_CR2_EXTSEL_TIM4_TRGO             ADC1_CR2_EXTSEL_TIM4_TRGO
#define ADC2_CR2_EXTSEL_SWSTART               ADC1_CR2_EXTSEL_SWSTART 
#define ADC2_CR2_EXTSEL_EXTI3                 ADC1_CR2_EXTSEL_EXTI2    
#define ADC2_CR2_EXTSEL_TIM6_TRGO             ADC1_CR2_EXTSEL_TIM6_TRGO
#define ADC2_CR2_EXTSEL_hrtim_adc_trg3        ADC1_CR2_EXTSEL_hrtim_adc_trg1
#define ADC2_CR2_EXTSEL_hrtim_adc_trg6        ADC1_CR2_EXTSEL_hrtim_adc_trg6
#define ADC2_CR2_EXTSEL_hrtim_adc_trg7        ADC1_CR2_EXTSEL_hrtim_adc_trg7
#define ADC2_CR2_EXTSEL_hrtim_adc_trg8        ADC1_CR2_EXTSEL_hrtim_adc_trg8
#define ADC2_CR2_EXTSEL_hrtim_adc_trg9        ADC1_CR2_EXTSEL_hrtim_adc_trg9
#define ADC2_CR2_EXTSEL_hrtim_adc_trg10       ADC1_CR2_EXTSEL_hrtim_adc_trg10
                                              
#define ADC3_CR2_EXTSEL_TIM1_TRGO             ADC1_CR2_EXTSEL_TIM1_TRGO
#define ADC3_CR2_EXTSEL_TIM1_TRGO2            ADC1_CR2_EXTSEL_TIM1_TRGO2
#define ADC3_CR2_EXTSEL_TIM8_TRGO             ADC1_CR2_EXTSEL_TIM8_TRGO
#define ADC3_CR2_EXTSEL_TIM8_TRGO2            ADC1_CR2_EXTSEL_TIM8_TRGO2 
#define ADC3_CR2_EXTSEL_TIM2_TRGO             ADC1_CR2_EXTSEL_TIM2_TRGO
#define ADC3_CR2_EXTSEL_TIM3_TRGO             ADC1_CR2_EXTSEL_TIM3_TRGO    
#define ADC3_CR2_EXTSEL_TIM4_TRGO             ADC1_CR2_EXTSEL_TIM4_TRGO
#define ADC3_CR2_EXTSEL_SWSTART               ADC1_CR2_EXTSEL_SWSTART 
#define ADC3_CR2_EXTSEL_EXTI4                 ADC1_CR2_EXTSEL_EXTI2    
#define ADC3_CR2_EXTSEL_TIM6_TRGO             ADC1_CR2_EXTSEL_TIM6_TRGO
#define ADC3_CR2_EXTSEL_hrtim_adc_trg5        ADC1_CR2_EXTSEL_hrtim_adc_trg1
#define ADC3_CR2_EXTSEL_hrtim_adc_trg6        ADC1_CR2_EXTSEL_hrtim_adc_trg6
#define ADC3_CR2_EXTSEL_hrtim_adc_trg7        ADC1_CR2_EXTSEL_hrtim_adc_trg7
#define ADC3_CR2_EXTSEL_hrtim_adc_trg8        ADC1_CR2_EXTSEL_hrtim_adc_trg8
#define ADC3_CR2_EXTSEL_hrtim_adc_trg9        ADC1_CR2_EXTSEL_hrtim_adc_trg9
#define ADC3_CR2_EXTSEL_hrtim_adc_trg10       ADC1_CR2_EXTSEL_hrtim_adc_trg10
                                              
#define ADC_CR2_EXTSEL_SWSTART                ADC1_CR2_EXTSEL_SWSTART 
/** 
  * end of ADC_External_trigger_Regular @}  
  */

/** @defgroup ADC_regular_rank ADC rank into regular group
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_REGULAR_RANK_1                 0x00000001U
#define ADC_REGULAR_RANK_2                 0x00000002U
#define ADC_REGULAR_RANK_3                 0x00000003U
#define ADC_REGULAR_RANK_4                 0x00000004U
#define ADC_REGULAR_RANK_5                 0x00000005U
#define ADC_REGULAR_RANK_6                 0x00000006U
#define ADC_REGULAR_RANK_7                 0x00000007U
#define ADC_REGULAR_RANK_8                 0x00000008U
/** 
  * end of ADC_regular_rank @}  
  */

/** @defgroup ADC_analog_watchdog_mode ADC analog watchdog mode
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_ANALOGWATCHDOG_NONE                 0x00000000U
#define ADC_ANALOGWATCHDOG_SINGLE_REG           (ADC_CR1_AWDSGL | ADC_CR1_AWDEN                   )
#define ADC_ANALOGWATCHDOG_SINGLE_INJEC         (ADC_CR1_AWDSGL                 | ADC_CR1_JAWDEN  )
#define ADC_ANALOGWATCHDOG_SINGLE_REGINJEC      (ADC_CR1_AWDSGL | ADC_CR1_AWDEN | ADC_CR1_JAWDEN  )
#define ADC_ANALOGWATCHDOG_ALL_REG              (                 ADC_CR1_AWDEN                   )
#define ADC_ANALOGWATCHDOG_ALL_INJEC            (                                 ADC_CR1_JAWDEN  )
#define ADC_ANALOGWATCHDOG_ALL_REGINJEC         (                 ADC_CR1_AWDEN | ADC_CR1_JAWDEN  )
/** 
  * end of ADC_analog_watchdog_mode @}  
  */

/** @defgroup ADC_conversion_group ADC conversion group
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_REGULAR_GROUP                       (ADC_SR_EOC               )
#define ADC_INJECTED_GROUP                      (             ADC_SR_JEOC )
#define ADC_REGULAR_INJECTED_GROUP              (ADC_SR_EOC | ADC_SR_JEOC )
/** 
  * end of ADC_conversion_group @}  
  */
  
/** @defgroup ADC_Blanking_Sample_Time ADC Comparitor Blanking Sample Time Option
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_BLANK_SAMPLE_TIME                     0x00000000U
#define ADC_BLANK_SAMPLE_TIME_ADD_12P5T           ADC_CR3_BASW
/** 
  * end of ADC_Blanking_Sample_Time @}  
  */

/** @defgroup ADC_Event_type ADC Event type
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_AWD_EVENT                              ADC_SR_AWD   /*!< ADC Analog watchdog event */
/** 
  * end of ADC_Event_type @}  
  */

/** @defgroup ADC_interrupts_definition ADC interrupts definition
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_IT_EOC                                ADC_CR1_EOCIE        /*!< ADC End of Regular Conversion interrupt source */
#define ADC_IT_JEOC                               ADC_CR1_JEOCIE       /*!< ADC End of Injected Conversion interrupt source */
#define ADC_IT_AWD                                ADC_CR1_AWDIE        /*!< ADC Analog watchdog interrupt source */
/** 
  * end of ADC_interrupts_definition @}  
  */

/** @defgroup ADC_flags_definition ADC flags definition
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_FLAG_STRT                             ADC_SR_STRT     /*!< ADC Regular group start flag */
#define ADC_FLAG_JSTRT                            ADC_SR_JSTRT    /*!< ADC Injected group start flag */
#define ADC_FLAG_EOC                              ADC_SR_EOC      /*!< ADC End of Regular conversion flag */
#define ADC_FLAG_JEOC                             ADC_SR_JEOC     /*!< ADC End of Injected conversion flag */
#define ADC_FLAG_AWD                              ADC_SR_AWD      /*!< ADC Analog watchdog flag */
/** 
  * end of ADC_flags_definition @}  
  */

/** @defgroup ADC_Private_Constants ADC Private Constants
  * @ingroup  ADC_Parameter_Definitions
  * @{
  */
#define ADC_FLAG_POSTCONV_ALL                     (ADC_FLAG_EOC | ADC_FLAG_JEOC | ADC_FLAG_AWD )
/** 
  * end of ADC_Private_Constants @}  
  */ 
/** 
  * end of ADC_Parameter_Definitions @}  
  */


/******************************************************************************/
/*                                ADC Macro                                   */
/******************************************************************************/
/** @defgroup ADC_Macro_Definitions ADC Macro Definitions
  * @{
  */

/**
  * @brief Enable the ADC peripheral
  * @note ADC enable requires a delay for ADC stabilization time
  * @param __HANDLE__: ADC handle
  * @retval None
  */
#define __HAL_ADC_ENABLE(__HANDLE__)                                           \
  (SET_BIT((__HANDLE__)->Instance->CR2, (ADC_CR2_ADON)))
    
/**
  * @brief Disable the ADC peripheral
  * @param __HANDLE__: ADC handle
  * @retval None
  */
#define __HAL_ADC_DISABLE(__HANDLE__)                                          \
  (CLEAR_BIT((__HANDLE__)->Instance->CR2, (ADC_CR2_ADON)))
    
/** @brief Enable the ADC end of conversion interrupt.
  * @param __HANDLE__: ADC handle
  * @param __INTERRUPT__: ADC Interrupt
  *        This parameter can be any combination of the following values:
  *        @arg ADC_IT_EOC: ADC End of Regular Conversion interrupt source
  *        @arg ADC_IT_JEOC: ADC End of Injected Conversion interrupt source
  *        @arg ADC_IT_AWD: ADC Analog watchdog interrupt source
  * @retval None
  */
#define __HAL_ADC_ENABLE_IT(__HANDLE__, __INTERRUPT__)                         \
  (SET_BIT((__HANDLE__)->Instance->CR1, (__INTERRUPT__)))
    
/** @brief Disable the ADC end of conversion interrupt.
  * @param __HANDLE__: ADC handle
  * @param __INTERRUPT__: ADC Interrupt
  *        This parameter can be any combination of the following values:
  *        @arg ADC_IT_EOC: ADC End of Regular Conversion interrupt source
  *        @arg ADC_IT_JEOC: ADC End of Injected Conversion interrupt source
  *        @arg ADC_IT_AWD: ADC Analog watchdog interrupt source
  * @retval None
  */
#define __HAL_ADC_DISABLE_IT(__HANDLE__, __INTERRUPT__)                        \
  (CLEAR_BIT((__HANDLE__)->Instance->CR1, (__INTERRUPT__)))

/** @brief  Checks if the specified ADC interrupt source is enabled or disabled.
  * @param __HANDLE__: ADC handle
  * @param __INTERRUPT__: ADC interrupt source to check
  *        This parameter can be any combination of the following values:
  *        @arg ADC_IT_EOC: ADC End of Regular Conversion interrupt source
  *        @arg ADC_IT_JEOC: ADC End of Injected Conversion interrupt source
  *        @arg ADC_IT_AWD: ADC Analog watchdog interrupt source
  * @retval None
  */
#define __HAL_ADC_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)                     \
  (((__HANDLE__)->Instance->CR1 & (__INTERRUPT__)) == (__INTERRUPT__))

/** @brief Get the selected ADC's flag status.
  * @param __HANDLE__: ADC handle
  * @param __FLAG__: ADC flag
  *        This parameter can be any combination of the following values:
  *        @arg ADC_FLAG_STRT: ADC Regular group start flag
  *        @arg ADC_FLAG_JSTRT: ADC Injected group start flag
  *        @arg ADC_FLAG_EOC: ADC End of Regular conversion flag
  *        @arg ADC_FLAG_JEOC: ADC End of Injected conversion flag
  *        @arg ADC_FLAG_AWD: ADC Analog watchdog flag
  * @retval None
  */
#define __HAL_ADC_GET_FLAG(__HANDLE__, __FLAG__)                               \
  ((((__HANDLE__)->Instance->SR) & (__FLAG__)) == (__FLAG__))
    
/** @brief Clear the ADC's pending flags
  * @param __HANDLE__: ADC handle
  * @param __FLAG__: ADC flag
  *        This parameter can be any combination of the following values:
  *        @arg ADC_FLAG_STRT: ADC Regular group start flag
  *        @arg ADC_FLAG_JSTRT: ADC Injected group start flag
  *        @arg ADC_FLAG_EOC: ADC End of Regular conversion flag
  *        @arg ADC_FLAG_JEOC: ADC End of Injected conversion flag
  *        @arg ADC_FLAG_AWD: ADC Analog watchdog flag
  * @retval None
  */
#define __HAL_ADC_CLEAR_FLAG(__HANDLE__, __FLAG__)                             \
  (WRITE_REG((__HANDLE__)->Instance->SR, ~(__FLAG__)))

/** @brief  Reset ADC handle state
  * @param  __HANDLE__: ADC handle
  * @retval None
  */
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
#define __HAL_ADC_RESET_HANDLE_STATE(__HANDLE__)                               \
  do{                                                                          \
     (__HANDLE__)->State = HAL_ADC_STATE_RESET;                                \
     (__HANDLE__)->MspInitCallback = NULL;                                     \
     (__HANDLE__)->MspDeInitCallback = NULL;                                   \
    } while(0)
#else
#define __HAL_ADC_RESET_HANDLE_STATE(__HANDLE__)                               \
  ((__HANDLE__)->State = HAL_ADC_STATE_RESET)
#endif

/**
  * @brief  Verification of ADC state: enabled or disabled
  * @param  __HANDLE__: ADC handle
  * @retval Status
  *         @arg SET (ADC enabled)
  *         @arg RESET (ADC disabled)
  */
#define ADC_IS_ENABLE(__HANDLE__)                                              \
  ((( ((__HANDLE__)->Instance->CR2 & ADC_CR2_ADON) == ADC_CR2_ADON )           \
   ) ? SET : RESET)

/**
  * @brief Test if conversion trigger of regular group is software start
  *        or external trigger.
  * @param __HANDLE__: ADC handle
  * @retval Status
  *         @arg SET (software start)
  *         @arg RESET (external trigger)
  */
#define ADC_IS_SOFTWARE_START_REGULAR(__HANDLE__)                              \
  (READ_BIT((__HANDLE__)->Instance->CR2, ADC_CR2_EXTSEL) == ADC1_CR2_EXTSEL_SWSTART)

/**
  * @brief Test if conversion trigger of injected group is software start
  *        or external trigger.
  * @param __HANDLE__: ADC handle
  * @retval Status
  *         @arg SET (software start)
  *         @arg RESET (external trigger)
  */
#define ADC_IS_SOFTWARE_START_INJECTED(__HANDLE__)                             \
  (READ_BIT((__HANDLE__)->Instance->CR2, ADC_CR2_JEXTSEL) == ADC1_CR2_JEXTSEL_JSWSTART)

/**
  * @brief Simultaneously clears and sets specific bits of the handle State
  * @note  ADC_STATE_CLR_SET() macro is merely aliased to generic macro MODIFY_REG(),
  *        the first parameter is the ADC handle State, the second parameter is the
  *        bit field to clear, the third and last parameter is the bit field to set.
  * @retval None
  */
#define ADC_STATE_CLR_SET MODIFY_REG

/**
  * @brief Clear ADC error code (set it to error code: "no error")
  * @param __HANDLE__: ADC handle
  * @retval None
  */
#define ADC_CLEAR_ERRORCODE(__HANDLE__)                                        \
  ((__HANDLE__)->ErrorCode = HAL_ADC_ERROR_NONE)

/**
  * @brief Set ADC number of conversions into regular channel sequence length.
  * @param _NbrOfConversion_: Regular channel sequence length 
  * @retval None
  */
#define ADC_SQR1_L_SHIFT(_NbrOfConversion_)                                    \
  (((_NbrOfConversion_) - (uint8_t)1) << ADC_SQR1_L_Pos)

/**
  * @brief Set the ADC's sample time for channel numbers between 10 and 18.
  * @param _SAMPLETIME_: Sample time parameter.
  * @param _CHANNELNB_: Channel number.  
  * @retval None
  */
#define ADC_SMPR1(_SAMPLETIME_, _CHANNELNB_)                                   \
  ((_SAMPLETIME_) << (ADC_SMPR1_SMP11_Pos * ((_CHANNELNB_) - 10)))

/**
  * @brief Set the ADC's sample time for channel numbers between 0 and 9.
  * @param _SAMPLETIME_: Sample time parameter.
  * @param _CHANNELNB_: Channel number.  
  * @retval None
  */
#define ADC_SMPR2(_SAMPLETIME_, _CHANNELNB_)                                   \
  ((_SAMPLETIME_) << (ADC_SMPR2_SMP1_Pos * (_CHANNELNB_)))

/**
  * @brief Set the selected regular channel rank for rank between 1 and 6.
  * @param _CHANNELNB_: Channel number.
  * @param _RANKNB_: Rank number.    
  * @retval None
  */
#define ADC_SQR3_RK(_CHANNELNB_, _RANKNB_)                                     \
  ((_CHANNELNB_) << (ADC_SQR3_SQ2_Pos * ((_RANKNB_) - 1)))

/**
  * @brief Set the selected regular channel rank for rank between 7 and 12.
  * @param _CHANNELNB_: Channel number.
  * @param _RANKNB_: Rank number.    
  * @retval None
  */
#define ADC_SQR2_RK(_CHANNELNB_, _RANKNB_)                                     \
  ((_CHANNELNB_) << (ADC_SQR2_SQ8_Pos * ((_RANKNB_) - 7)))

/**
  * @brief Set the injected sequence length.
  * @param _JSQR_JL_: Sequence length.
  * @retval None
  */
#define ADC_JSQR_JL_SHIFT(_JSQR_JL_)                                           \
  (((_JSQR_JL_) -1) << ADC_JSQR_JL_Pos)

/**
  * @brief Set the selected injected channel rank
  * @param _CHANNELNB_: Channel number.
  * @param _RANKNB_: Rank number.
  * @param _JSQR_JL_: Sequence length.
  * @retval None
  */
#define ADC_JSQR_RK_JL(_CHANNELNB_, _RANKNB_, _JSQR_JL_)                       \
  ((_CHANNELNB_) << (ADC_JSQR_JSQ2_Pos * ((4 - ((_JSQR_JL_) - (_RANKNB_))) - 1)))

/**
  * @brief Enable ADC continuous conversion mode.
  * @param _CONTINUOUS_MODE_: Continuous mode.
  * @retval None
  */
#define ADC_CR2_CONTINUOUS(_CONTINUOUS_MODE_)                                  \
  ((_CONTINUOUS_MODE_) << 0x1)

/**
  * @brief Configures the number of discontinuous conversions for the regular group channels.
  * @param _NBR_DISCONTINUOUS_CONV_: Number of discontinuous conversions.
  * @retval None
  */
#define ADC_CR1_DISCONTINUOUS_NUM(_NBR_DISCONTINUOUS_CONV_)                    \
  (((_NBR_DISCONTINUOUS_CONV_) - 1) << ADC_CR1_DISCNUM_Pos)

/**
  * @brief Enable ADC scan mode to convert multiple ranks with sequencer.
  * @param _SCAN_MODE_: Scan conversion mode.
  * @retval None
  */
#define ADC_CR1_SCAN_SET(_SCAN_MODE_)                                          \
  (( ((_SCAN_MODE_) == ADC_SCAN_ENABLE) || ((_SCAN_MODE_) == ENABLE)           \
   )? (ADC_SCAN_ENABLE) : (ADC_SCAN_DISABLE)                                   \
  )

/**
  * @brief  Check if the parameter ALIGN is valid
  * @param  ALIGN
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_DATA_ALIGN(ALIGN) (((ALIGN) == ADC_DATAALIGN_RIGHT) || \
                                  ((ALIGN) == ADC_DATAALIGN_LEFT)    )

/**
  * @brief  Check if the parameter SCAN_MODE is valid
  * @param  SCAN_MODE
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_SCAN_MODE(SCAN_MODE) (((SCAN_MODE) == ADC_SCAN_DISABLE) || \
                                     ((SCAN_MODE) == ADC_SCAN_ENABLE)    )

/**
  * @brief  Check if the parameter CHANNEL is valid
  * @param  CHANNEL
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_CHANNEL(CHANNEL) (((CHANNEL) == ADC_CHANNEL_0)           || \
                                 ((CHANNEL) == ADC_CHANNEL_1)           || \
                                 ((CHANNEL) == ADC_CHANNEL_2)           || \
                                 ((CHANNEL) == ADC_CHANNEL_3)           || \
                                 ((CHANNEL) == ADC_CHANNEL_4)           || \
                                 ((CHANNEL) == ADC_CHANNEL_5)           || \
                                 ((CHANNEL) == ADC_CHANNEL_6)           || \
                                 ((CHANNEL) == ADC_CHANNEL_7)           || \
                                 ((CHANNEL) == ADC_CHANNEL_8)           || \
                                 ((CHANNEL) == ADC_CHANNEL_9)           || \
                                 ((CHANNEL) == ADC_CHANNEL_10)          || \
                                 ((CHANNEL) == ADC_CHANNEL_11)          || \
                                 ((CHANNEL) == ADC_CHANNEL_12)          || \
                                 ((CHANNEL) == ADC_CHANNEL_13)          || \
                                 ((CHANNEL) == ADC_CHANNEL_14)          || \
                                 ((CHANNEL) == ADC_CHANNEL_15)            )

/**
  * @brief  Check if the parameter RANK is valid
  * @param  RANK
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_REGULAR_RANK(RANK)    (((RANK) == ADC_REGULAR_RANK_1 ) || \
                                      ((RANK) == ADC_REGULAR_RANK_2 ) || \
                                      ((RANK) == ADC_REGULAR_RANK_3 ) || \
                                      ((RANK) == ADC_REGULAR_RANK_4 ) || \
                                      ((RANK) == ADC_REGULAR_RANK_5 ) || \
                                      ((RANK) == ADC_REGULAR_RANK_6 ) || \
                                      ((RANK) == ADC_REGULAR_RANK_7 ) || \
                                      ((RANK) == ADC_REGULAR_RANK_8 )    )

/**
  * @brief  Check if the parameter WATCHDOG is valid
  * @param  WATCHDOG
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_ANALOG_WATCHDOG_MODE(WATCHDOG) (((WATCHDOG) == ADC_ANALOGWATCHDOG_NONE)             || \
                                               ((WATCHDOG) == ADC_ANALOGWATCHDOG_SINGLE_REG)       || \
                                               ((WATCHDOG) == ADC_ANALOGWATCHDOG_SINGLE_INJEC)     || \
                                               ((WATCHDOG) == ADC_ANALOGWATCHDOG_SINGLE_REGINJEC)  || \
                                               ((WATCHDOG) == ADC_ANALOGWATCHDOG_ALL_REG)          || \
                                               ((WATCHDOG) == ADC_ANALOGWATCHDOG_ALL_INJEC)        || \
                                               ((WATCHDOG) == ADC_ANALOGWATCHDOG_ALL_REGINJEC)       )

/**
  * @brief  Check if the parameter CONVERSION is valid
  * @param  CONVERSION
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_CONVERSION_GROUP(CONVERSION) (((CONVERSION) == ADC_REGULAR_GROUP)         || \
                                             ((CONVERSION) == ADC_INJECTED_GROUP)        || \
                                             ((CONVERSION) == ADC_REGULAR_INJECTED_GROUP)  )

/**
  * @brief  Check if the parameter EVENT is valid
  * @param  EVENT
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_EVENT_TYPE(EVENT) ((EVENT) == ADC_AWD_EVENT)

/**
  * @brief  Check if the parameter ADC_VALUE is valid
  * @param  ADC_VALUE
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_RANGE(ADC_VALUE) ((ADC_VALUE) <= 0x0FFFU)

/**
  * @brief  Check if the parameter LENGTH is valid
  * @param  LENGTH
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_REGULAR_NB_CONV(LENGTH) (((LENGTH) >= 1U) && ((LENGTH) <= 8U))  

/**
  * @brief  Check if the parameter NUMBER is valid
  * @param  NUMBER
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_REGULAR_DISCONT_NUMBER(NUMBER) (((NUMBER) >= 1U) && ((NUMBER) <= 8U))

/**
  * @brief  Check if the parameter NUMBER is valid
  * @param  NUMBER
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_TPS_OFFSET(NUMBER) (NUMBER <= 0xFFU)

/**
  * @brief  Check if the parameter NUMBER is valid
  * @param  NUMBER
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_SAMPLETIME_USER_SETTING(NUMBER) ((NUMBER) <= 0xFFU)
/** 
  * end of ADC_Macro_Definitions @}  
  */
  
/******************************************************************************/
/*                                ADC Functions                               */
/******************************************************************************/

/** @defgroup ADC_Function_Definitions ADC Function Definitions
  * @{
  */
HAL_StatusTypeDef       HAL_ADC_Init(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADC_DeInit(ADC_HandleTypeDef *hadc);
void                    HAL_ADC_MspInit(ADC_HandleTypeDef* hadc);
void                    HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc);
 
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
/* Callbacks Register/UnRegister functions  ***********************************/
HAL_StatusTypeDef       HAL_ADC_RegisterCallback(ADC_HandleTypeDef *hadc, HAL_ADC_CallbackIDTypeDef CallbackID, pADC_CallbackTypeDef pCallback);
HAL_StatusTypeDef       HAL_ADC_UnRegisterCallback(ADC_HandleTypeDef *hadc, HAL_ADC_CallbackIDTypeDef CallbackID);
#endif /* USE_HAL_ADC_REGISTER_CALLBACKS */ 

/*!< Blocking mode: Polling */
HAL_StatusTypeDef       HAL_ADC_Start(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADC_Stop(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADC_PollForConversion(ADC_HandleTypeDef* hadc, uint32_t Timeout);
HAL_StatusTypeDef       HAL_ADC_PollForEvent(ADC_HandleTypeDef* hadc, uint32_t EventType, uint32_t Timeout);

/*!< Non-blocking mode: Interruption */
HAL_StatusTypeDef       HAL_ADC_Start_IT(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADC_Stop_IT(ADC_HandleTypeDef* hadc);

/*!< Non-blocking mode: DMA */
HAL_StatusTypeDef       HAL_ADC_Start_DMA(ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length);
HAL_StatusTypeDef       HAL_ADC_Stop_DMA(ADC_HandleTypeDef* hadc);

/*!< ADC retrieve conversion value intended to be used with polling or interruption */
uint32_t                HAL_ADC_GetValue(ADC_HandleTypeDef* hadc);

/*!< ADC IRQHandler and Callbacks used in non-blocking modes (Interruption and DMA) */
void                    HAL_ADC_IRQHandler(ADC_HandleTypeDef* hadc);
void                    HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
void                    HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc);
void                    HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc);
void                    HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc); 
 
HAL_StatusTypeDef       HAL_ADC_ConfigChannel(ADC_HandleTypeDef* hadc, ADC_ChannelConfTypeDef* sConfig);
HAL_StatusTypeDef       HAL_ADC_AnalogWDGConfig(ADC_HandleTypeDef* hadc, ADC_AnalogWDGConfTypeDef* AnalogWDGConfig);

uint32_t                HAL_ADC_GetState(ADC_HandleTypeDef* hadc);
uint32_t                HAL_ADC_GetError(ADC_HandleTypeDef *hadc);

HAL_StatusTypeDef       ADC_Enable(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       ADC_ConversionStop_Disable(ADC_HandleTypeDef* hadc);

void                    ADC_DMAConvCplt(DMA_HandleTypeDef *hdma);
void                    ADC_DMAHalfConvCplt(DMA_HandleTypeDef *hdma);
void                    ADC_DMAError(DMA_HandleTypeDef *hdma);

/*!< ADC additional function */
HAL_StatusTypeDef       HAL_ADC_OptionalConfig(ADC_HandleTypeDef* hadc, ADC_OptionalConfTypeDef* OpConfig);
/** 
  * end of ADC_Function_Definitions @}  
  */


/** 
  * end of ADC @}  
  */

#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_ADC_H_ */

