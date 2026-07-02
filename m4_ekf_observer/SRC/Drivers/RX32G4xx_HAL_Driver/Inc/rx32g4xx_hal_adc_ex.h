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
*   File    : rx32g4xx_hal_adc_ex.h
*   By      : RX_DV_Team
**************************************************************************************************
*/



#ifndef _RX32G4XX_HAL_ADC_EX_H_
#define _RX32G4XX_HAL_ADC_EX_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"
#include "rx32g4xx_hal_adc.h"

/* Exported types ------------------------------------------------------------*/
/** @addtogroup ADC
  * @{
  */ 

/******************************************************************************/
/*                               ADC Structures                               */
/******************************************************************************/
  
/** @addtogroup ADC_Structure_Definitions
  * @{
  */
  
/** @defgroup ADC_InjectionConfTypeDef ADC InjectionConfTypeDef
  * @{
  */
/** 
  * @brief  ADC Configuration injected Channel structure definition
  * @note   Parameters of this structure are shared within 2 scopes:
  *          - Scope channel: InjectedChannel, InjectedRank, InjectedSamplingTime, InjectedOffset
  *          - Scope injected group (affects all channels of injected group): InjectedNbrOfConversion, InjectedDiscontinuousConvMode,
  *            AutoInjectedConv, ExternalTrigInjecConvEdge, ExternalTrigInjecConv.
  * @note   The setting of these parameters with function HAL_ADCEx_InjectedConfigChannel() is conditioned to ADC state.
  *         ADC state can be either:
  *          - For all parameters: ADC disabled (this is the only possible ADC state to modify parameter 'ExternalTrigInjecConv')
  *          - For all except parameters 'ExternalTrigInjecConv': ADC enabled without conversion on going on injected group.
  */
typedef struct 
{
  uint32_t InjectedChannel;                       /*!< Selection of ADC channel to configure
                                                       This parameter can be a value of @ref ADC_channels */
  uint32_t InjectedRank;                          /*!< Rank in the injected group sequencer
                                                       This parameter must be a value of @ref ADCEx_injected_rank*/
  uint32_t InjectedSamplingTime;                  /*!< Sampling time value to be set for the selected channel.
                                                       Unit: ADC clock cycles
                                                       Conversion time is the addition of sampling time and processing time (12.5 ADC clock cycles at ADC resolution 12 bits).
                                                       This parameter can be a value of @ref ADC_sampling_times */
  uint32_t InjectedOffset;                        /*!< Defines the offset to be subtracted from the raw converted data (for channels set on injected group only).
                                                       this parameter must be a number between Min_Data = 0x000 and Max_Data = 0xFFF. */
  uint32_t InjectedNbrOfConversion;               /*!< Specifies the number of ranks that will be converted within the injected group sequencer.
                                                       To use the injected group sequencer and convert several ranks, parameter 'ScanConvMode' must be enabled.
                                                       This parameter must be a number between Min_Data = 1 and Max_Data = 4. */
  FunctionalState InjectedDiscontinuousConvMode;  /*!< Specifies whether the conversions sequence of injected group is performed in Complete-sequence/Discontinuous-sequence (main sequence subdivided in successive parts).
                                                       Discontinuous mode is used only if sequencer is enabled (parameter 'ScanConvMode'). If sequencer is disabled, this parameter is discarded.
                                                       Discontinuous mode can be enabled only if continuous mode is disabled. If continuous mode is enabled, this parameter setting is discarded.
                                                       This parameter can be set to ENABLE or DISABLE. */
  FunctionalState AutoInjectedConv;               /*!< Enables or disables the selected ADC automatic injected group conversion after regular one
                                                       This parameter can be set to ENABLE or DISABLE. */
  uint32_t ExternalTrigInjecConv;                 /*!< Selects the external event used to trigger the conversion start of injected group.
                                                       If set to ADC1_CR2_JEXTSEL_JSWSTART, external triggers are disabled.
                                                       If set to external trigger source, triggering is on event rising edge.
                                                       This parameter can be a value of @ref ADCEx_External_trigger_source_Injected */
} ADC_InjectionConfTypeDef;
  
/** 
  * end of ADC_InjectionConfTypeDef @}  
  */
  
/** @defgroup ADC_MultiModeTypeDef ADC MultiModeTypeDef
  * @{
  */
/** 
  * @brief  Structure definition of ADC multimode
  * @note   The setting of these parameters with function HAL_ADCEx_MultiModeConfigChannel() is conditioned to ADCs state (both ADCs of the common group).
  *         State of ADCs of the common group must be: disabled.
  */
typedef struct
{
  uint32_t Mode;              /*!< Configures the ADC to operate in independent or multi mode. 
                                   This parameter can be a value of @ref ADCEx_Common_mode */ 
} ADC_MultiModeTypeDef;                                                          

/** 
  * end of ADC_MultiModeTypeDef @}  
  */
/** 
  * end of ADC_Structure_Definitions @}  
  */

/******************************************************************************/
/*                               ADC Parameters                               */
/******************************************************************************/
  
/** @addtogroup ADC_Parameter_Definitions
  * @{
  */
  
/** @defgroup ADCEx_injected_rank ADCEx rank into injected group
  * @{
  */
#define ADC_INJECTED_RANK_1                           0x00000001U
#define ADC_INJECTED_RANK_2                           0x00000002U
#define ADC_INJECTED_RANK_3                           0x00000003U
#define ADC_INJECTED_RANK_4                           0x00000004U
/** 
  * end of ADCEx_injected_rank @}  
  */

  
/** @defgroup ADCEx_Common_mode ADC Extended Dual ADC Mode
  * @{
  */
#define ADC_CR1_DUALMOD_INDEPENDENT                    ((uint32_t)0x00000000)          
#define ADC_CR1_DUALMOD_REG_SIM_INJ_SIM                (                                                                     ADC_CR1_DUALMOD_BIT0) 
#define ADC_CR1_DUALMOD_REG_SIM_INJ_ALT                (                                              ADC_CR1_DUALMOD_BIT1                       )  
#define ADC_CR1_DUALMOD_REG_INTFAST_INJ_SIM            (                                              ADC_CR1_DUALMOD_BIT1 | ADC_CR1_DUALMOD_BIT0)
#define ADC_CR1_DUALMOD_REG_INTSLOW_INJ_SIM            (                       ADC_CR1_DUALMOD_BIT2                                              ) 
#define ADC_CR1_DUALMOD_INJ_SIMULT                     (                       ADC_CR1_DUALMOD_BIT2                        | ADC_CR1_DUALMOD_BIT0)                                              
#define ADC_CR1_DUALMOD_REG_SIMULT                     (                       ADC_CR1_DUALMOD_BIT2 | ADC_CR1_DUALMOD_BIT1                       )
#define ADC_CR1_DUALMOD_REG_INTERL_FAST                (                       ADC_CR1_DUALMOD_BIT2 | ADC_CR1_DUALMOD_BIT1 | ADC_CR1_DUALMOD_BIT0) 
#define ADC_CR1_DUALMOD_REG_INTERL_SLOW                (ADC_CR1_DUALMOD_BIT3                                                                     ) 
#define ADC_CR1_DUALMOD_INJ_ALTERN                     (ADC_CR1_DUALMOD_BIT3                                               | ADC_CR1_DUALMOD_BIT0) 
/** 
  * end of ADCEx_Common_mode @}  
  */  
  
/** @defgroup ADCEx_External_trigger_source_Injected ADC external trigger source injected
  * @{
  */
#define ADC1_CR2_JEXTSEL_TIM1_TRGO                     ((uint32_t)0x00000000)           
#define ADC1_CR2_JEXTSEL_TIM1_TRGO2                    (                                                                     ADC_CR2_JEXTSEL_BIT0) 
#define ADC1_CR2_JEXTSEL_TIM8_TRGO                     (                                              ADC_CR2_JEXTSEL_BIT1                       ) 
#define ADC1_CR2_JEXTSEL_TIM8_TRGO2                    (                                              ADC_CR2_JEXTSEL_BIT1 | ADC_CR2_JEXTSEL_BIT0) 
#define ADC1_CR2_JEXTSEL_TIM1_CC4_CC5                  (                       ADC_CR2_JEXTSEL_BIT2                                              ) 
#define ADC1_CR2_JEXTSEL_TIM2_TRGO                     (                       ADC_CR2_JEXTSEL_BIT2                        | ADC_CR2_JEXTSEL_BIT0) 
#define ADC1_CR2_JEXTSEL_EXTI13                        (                       ADC_CR2_JEXTSEL_BIT2 | ADC_CR2_JEXTSEL_BIT1                       ) 
#define ADC1_CR2_JEXTSEL_JSWSTART                      (                       ADC_CR2_JEXTSEL_BIT2 | ADC_CR2_JEXTSEL_BIT1 | ADC_CR2_JEXTSEL_BIT0) 
#define ADC1_CR2_JEXTSEL_TIM15_TRGO                    (ADC_CR2_JEXTSEL_BIT3                                                                     )         
#define ADC1_CR2_JEXTSEL_TIM7_TRGO                     (ADC_CR2_JEXTSEL_BIT3 |                                               ADC_CR2_JEXTSEL_BIT0) 
#define ADC1_CR2_JEXTSEL_hrtim_adc_trg2                (ADC_CR2_JEXTSEL_BIT3 |                        ADC_CR2_JEXTSEL_BIT1                       ) 
#define ADC1_CR2_JEXTSEL_hrtim_adc_trg6                (ADC_CR2_JEXTSEL_BIT3 |                        ADC_CR2_JEXTSEL_BIT1 | ADC_CR2_JEXTSEL_BIT0) 
#define ADC1_CR2_JEXTSEL_hrtim_adc_trg7                (ADC_CR2_JEXTSEL_BIT3 | ADC_CR2_JEXTSEL_BIT2                                              ) 
#define ADC1_CR2_JEXTSEL_hrtim_adc_trg8                (ADC_CR2_JEXTSEL_BIT3 | ADC_CR2_JEXTSEL_BIT2                        | ADC_CR2_JEXTSEL_BIT0) 
#define ADC1_CR2_JEXTSEL_hrtim_adc_trg9                (ADC_CR2_JEXTSEL_BIT3 | ADC_CR2_JEXTSEL_BIT2 | ADC_CR2_JEXTSEL_BIT1                       ) 
#define ADC1_CR2_JEXTSEL_hrtim_adc_trg10               (ADC_CR2_JEXTSEL_BIT3 | ADC_CR2_JEXTSEL_BIT2 | ADC_CR2_JEXTSEL_BIT1 | ADC_CR2_JEXTSEL_BIT0) 
                                                       
#define ADC2_CR2_JEXTSEL_TIM1_TRGO                     ADC1_CR2_JEXTSEL_TIM1_TRGO       
#define ADC2_CR2_JEXTSEL_TIM1_TRGO2                    ADC1_CR2_JEXTSEL_TIM1_TRGO2
#define ADC2_CR2_JEXTSEL_TIM8_TRGO                     ADC1_CR2_JEXTSEL_TIM8_TRGO
#define ADC2_CR2_JEXTSEL_TIM8_TRGO2                    ADC1_CR2_JEXTSEL_TIM8_TRGO2
#define ADC2_CR2_JEXTSEL_TIM8_CC4_CC5                  ADC1_CR2_JEXTSEL_TIM1_CC4_CC5
#define ADC2_CR2_JEXTSEL_TIM3_TRGO                     ADC1_CR2_JEXTSEL_TIM2_TRGO
#define ADC2_CR2_JEXTSEL_EXTI14                        ADC1_CR2_JEXTSEL_EXTI13
#define ADC2_CR2_JEXTSEL_JSWSTART                      ADC1_CR2_JEXTSEL_JSWSTART
#define ADC2_CR2_JEXTSEL_TIM15_TRGO                    ADC1_CR2_JEXTSEL_TIM15_TRGO
#define ADC2_CR2_JEXTSEL_TIM7_TRGO                     ADC1_CR2_JEXTSEL_TIM7_TRGO
#define ADC2_CR2_JEXTSEL_hrtim_adc_trg4                ADC1_CR2_JEXTSEL_hrtim_adc_trg2
#define ADC2_CR2_JEXTSEL_hrtim_adc_trg6                ADC1_CR2_JEXTSEL_hrtim_adc_trg6
#define ADC2_CR2_JEXTSEL_hrtim_adc_trg7                ADC1_CR2_JEXTSEL_hrtim_adc_trg7 
#define ADC2_CR2_JEXTSEL_hrtim_adc_trg8                ADC1_CR2_JEXTSEL_hrtim_adc_trg8
#define ADC2_CR2_JEXTSEL_hrtim_adc_trg9                ADC1_CR2_JEXTSEL_hrtim_adc_trg9 
#define ADC2_CR2_JEXTSEL_hrtim_adc_trg10               ADC1_CR2_JEXTSEL_hrtim_adc_trg10
                                                       
#define ADC3_CR2_JEXTSEL_TIM1_TRGO                     ADC1_CR2_JEXTSEL_TIM1_TRGO       
#define ADC3_CR2_JEXTSEL_TIM1_TRGO2                    ADC1_CR2_JEXTSEL_TIM1_TRGO2
#define ADC3_CR2_JEXTSEL_TIM8_TRGO                     ADC1_CR2_JEXTSEL_TIM8_TRGO
#define ADC3_CR2_JEXTSEL_TIM8_TRGO2                    ADC1_CR2_JEXTSEL_TIM8_TRGO2
#define ADC3_CR2_JEXTSEL_TIM8_CC4_CC5                  ADC1_CR2_JEXTSEL_TIM1_CC4_CC5
#define ADC3_CR2_JEXTSEL_TIM4_TRGO                     ADC1_CR2_JEXTSEL_TIM2_TRGO
#define ADC3_CR2_JEXTSEL_EXTI15                        ADC1_CR2_JEXTSEL_EXTI13
#define ADC3_CR2_JEXTSEL_JSWSTART                      ADC1_CR2_JEXTSEL_JSWSTART
#define ADC3_CR2_JEXTSEL_TIM15_TRGO                    ADC1_CR2_JEXTSEL_TIM15_TRGO
#define ADC3_CR2_JEXTSEL_TIM7_TRGO                     ADC1_CR2_JEXTSEL_TIM7_TRGO
#define ADC3_CR2_JEXTSEL_hrtim_adc_trg5                ADC1_CR2_JEXTSEL_hrtim_adc_trg2
#define ADC3_CR2_JEXTSEL_hrtim_adc_trg6                ADC1_CR2_JEXTSEL_hrtim_adc_trg6
#define ADC3_CR2_JEXTSEL_hrtim_adc_trg7                ADC1_CR2_JEXTSEL_hrtim_adc_trg7 
#define ADC3_CR2_JEXTSEL_hrtim_adc_trg8                ADC1_CR2_JEXTSEL_hrtim_adc_trg8
#define ADC3_CR2_JEXTSEL_hrtim_adc_trg9                ADC1_CR2_JEXTSEL_hrtim_adc_trg9 
#define ADC3_CR2_JEXTSEL_hrtim_adc_trg10               ADC1_CR2_JEXTSEL_hrtim_adc_trg10
                                                       
#define ADC_CR2_JEXTSEL_JSWSTART                       ADC1_CR2_JEXTSEL_JSWSTART   
/** 
  * end of ADCEx_External_trigger_source_Injected @}  
  */  
/** 
  * end of ADC_Parameter_Definitions @}  
  */


/******************************************************************************/
/*                                 ADC Macro                                  */
/******************************************************************************/
/** @addtogroup ADC_Macro_Definitions
  * @{
  */

/**
  * @brief For devices with 3 ADCs: Defines the external trigger source 
  *        for regular group according to ADC into common group ADC1&ADC2 or 
  *        ADC3 (some triggers with same source have different value to
  *        be programmed into ADC EXTSEL bits of CR2 register).
  * @param __HANDLE__: ADC handle
  * @param __EXT_TRIG_CONV__: External trigger selected for regular group.
  * @retval External trigger to be programmed into EXTSEL bits of CR2 register
  */
#define ADC_CFGR_EXTSEL(__HANDLE__, __EXT_TRIG_CONV__)                         \
 (( (((__HANDLE__)->Instance) == ADC3)                                         \
  )?                                                                           \
   ( ( (__EXT_TRIG_CONV__) == ADC3_CR2_EXTSEL_TIM8_TRGO2                       \
     )?                                                                        \
      (ADC3_CR2_EXTSEL_TIM8_TRGO2)                                             \
      :                                                                        \
      (__EXT_TRIG_CONV__)                                                      \
   )                                                                           \
   :                                                                           \
   (__EXT_TRIG_CONV__)                                                         \
 )

/**
  * @brief For devices with 3 ADCs: Defines the external trigger source 
  *        for injected group according to ADC into common group ADC1&ADC2 or 
  *        ADC3 (some triggers with same source have different value to
  *        be programmed into ADC JEXTSEL bits of CR2 register).
  *        For devices with 2 ADCs or less: this macro makes no change.
  * @param __HANDLE__: ADC handle
  * @param __EXT_TRIG_INJECTCONV__: External trigger selected for injected group.
  * @retval External trigger to be programmed into JEXTSEL bits of CR2 register
  */
#define ADC_CFGR_JEXTSEL(__HANDLE__, __EXT_TRIG_INJECTCONV__)                  \
 (( (((__HANDLE__)->Instance) == ADC3)                                         \
  )?                                                                           \
   ( ( (__EXT_TRIG_INJECTCONV__) == ADC3_CR2_JEXTSEL_TIM8_CC4_CC5           \
     )?                                                                        \
      (ADC3_CR2_JEXTSEL_TIM8_CC4_CC5)                                          \
      :                                                                        \
      (__EXT_TRIG_INJECTCONV__)                                                \
   )                                                                           \
   :                                                                           \
   (__EXT_TRIG_INJECTCONV__)                                                   \
 )

/**
  * @brief Verification if multimode is enabled for the selected ADC (multimode ADC master or ADC slave) (applicable for devices with several ADCs)
  * @param __HANDLE__: ADC handle
  * @retval Multimode state: RESET if multimode is disabled, other value if multimode is enabled
  */
#define ADC_MULTIMODE_IS_ENABLE(__HANDLE__)                                    \
 (( (((__HANDLE__)->Instance) == ADC1) || (((__HANDLE__)->Instance) == ADC2)   \
  )?                                                                           \
   (ADC1->CR1 & ADC_CR1_DUALMOD)                                               \
   :                                                                           \
   (RESET)                                                                     \
 )
 
/**
  * @brief Verification of condition for ADC start conversion: ADC must be in non-multimode, or multimode with handle of ADC master (applicable for devices with several ADCs)
  * @param __HANDLE__: ADC handle
  * @retval None
  */
#define ADC_NONMULTIMODE_OR_MULTIMODEMASTER(__HANDLE__)                        \
  (( (((__HANDLE__)->Instance) == ADC2)                                        \
   )?                                                                          \
    ((ADC1->CR1 & ADC_CR1_DUALMOD) == RESET)                                   \
    :                                                                          \
    (!RESET)                                                                   \
  )

/**
  * @brief Check ADC multimode setting: In case of multimode, check whether ADC master of the selected ADC has feature auto-injection enabled (applicable for devices with several ADCs)
  * @param __HANDLE__: ADC handle
  * @retval None
  */
#define ADC_MULTIMODE_AUTO_INJECTED(__HANDLE__)                                \
  (( (((__HANDLE__)->Instance) == ADC1) || (((__HANDLE__)->Instance) == ADC2)  \
   )?                                                                          \
    (ADC1->CR1 & ADC_CR1_JAUTO)                                                \
    :                                                                          \
    (RESET)                                                                    \
  )

/**
  * @brief Set handle of the other ADC sharing the common multimode settings
  * @param __HANDLE__: ADC handle
  * @param __HANDLE_OTHER_ADC__: other ADC handle
  * @retval None
  */
#define ADC_COMMON_ADC_OTHER(__HANDLE__, __HANDLE_OTHER_ADC__)                 \
  ((__HANDLE_OTHER_ADC__)->Instance = ADC2)

/**
  * @brief Set handle of the ADC slave associated to the ADC master
  * @param __HANDLE_MASTER__: ADC master handle
  * @param __HANDLE_SLAVE__: ADC slave handle
  * @retval None
  */
#define ADC_MULTI_SLAVE(__HANDLE_MASTER__, __HANDLE_SLAVE__)                   \
  ((__HANDLE_SLAVE__)->Instance = ADC2)
       
/**
  * @brief Check if the parameter RANK is valid
  * @param RANK
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_INJECTED_RANK(RANK)    (((RANK) == ADC_INJECTED_RANK_1) || \
                                       ((RANK) == ADC_INJECTED_RANK_2) || \
                                       ((RANK) == ADC_INJECTED_RANK_3) || \
                                       ((RANK) == ADC_INJECTED_RANK_4))


/**
  * @brief Check if the parameter LENGTH is valid
  * @param LENGTH
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_INJECTED_NB_CONV(LENGTH)  (((LENGTH) >= 1U) && ((LENGTH) <= 4U))

/**
  * @brief Check if the parameter REGTRIG is valid
  * @param REGTRIG
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_EXTTRIG(REGTRIG) (((REGTRIG) == ADC1_CR2_EXTSEL_TIM1_TRGO         ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_TIM1_TRGO2        ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_TIM8_TRGO         ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_TIM8_TRGO2        ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_TIM2_TRGO         ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_TIM3_TRGO         ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_TIM4_TRGO         ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_SWSTART           ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_EXTI2             ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_TIM6_TRGO         ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_hrtim_adc_trg1    ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_hrtim_adc_trg6    ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_hrtim_adc_trg7    ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_hrtim_adc_trg8    ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_hrtim_adc_trg9    ) || \
                                 ((REGTRIG) == ADC1_CR2_EXTSEL_hrtim_adc_trg10   )) 
                                               
/**
  * @brief Check if the parameter INJTRIG is valid
  * @param INJTRIG
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */                                               
#define IS_ADC_EXTTRIGINJEC(INJTRIG) (((INJTRIG) == ADC1_CR2_JEXTSEL_TIM1_TRGO       )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_TIM1_TRGO2      )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_TIM8_TRGO       )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_TIM8_TRGO2      )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_TIM1_CC4_CC5    )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_TIM2_TRGO       )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_EXTI13          )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_JSWSTART        )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_TIM15_TRGO      )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_TIM7_TRGO       )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_hrtim_adc_trg2  )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_hrtim_adc_trg6  )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_hrtim_adc_trg7  )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_hrtim_adc_trg8  )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_hrtim_adc_trg9  )|| \
                                      ((INJTRIG) == ADC1_CR2_JEXTSEL_hrtim_adc_trg10 ))

/**
  * @brief Check if the parameter MODE is valid
  * @param MODE
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_ADC_MODE(MODE) (((MODE) == ADC_CR1_DUALMOD_INDEPENDENT        ) || \
                           ((MODE) == ADC_CR1_DUALMOD_REG_SIM_INJ_SIM    ) || \
                           ((MODE) == ADC_CR1_DUALMOD_REG_SIM_INJ_ALT    ) || \
                           ((MODE) == ADC_CR1_DUALMOD_REG_INTFAST_INJ_SIM) || \
                           ((MODE) == ADC_CR1_DUALMOD_REG_INTSLOW_INJ_SIM) || \
                           ((MODE) == ADC_CR1_DUALMOD_INJ_SIMULT         ) || \
                           ((MODE) == ADC_CR1_DUALMOD_REG_SIMULT         ) || \
                           ((MODE) == ADC_CR1_DUALMOD_REG_INTERL_FAST    ) || \
                           ((MODE) == ADC_CR1_DUALMOD_REG_INTERL_SLOW    ) || \
                           ((MODE) == ADC_CR1_DUALMOD_INJ_ALTERN         )) 
                           
#define IS_ADC_SAMPLE_TIME(SAMPLE_TIME) (((SAMPLE_TIME) == ADC_SAMPLINGTIME_3CYCLES_5  ) || \
                                         ((SAMPLE_TIME) == ADC_SAMPLINGTIME_7CYCLES_5  ) || \
                                         ((SAMPLE_TIME) == ADC_SAMPLINGTIME_11CYCLES_5 ) || \
                                         ((SAMPLE_TIME) == ADC_SAMPLINGTIME_13CYCLES_5 ) || \
                                         ((SAMPLE_TIME) == ADC_SAMPLINGTIME_28CYCLES_5 ) || \
                                         ((SAMPLE_TIME) == ADC_SAMPLINGTIME_55CYCLES_5 ) || \
                                         ((SAMPLE_TIME) == ADC_SAMPLINGTIME_71CYCLES_5 ) || \
                                         ((SAMPLE_TIME) == ADC_SAMPLINGTIME_640CYCLES_5))


/** 
  * end of ADC_Macro_Definitions @}  
  */
  
/******************************************************************************/
/*                                ADC Functions                               */
/******************************************************************************/

/** @addtogroup ADC_Function_Definitions
  * @{
  */

/*!< ADC calibration */
HAL_StatusTypeDef       HAL_ADCEx_Calibration_Start(ADC_HandleTypeDef* hadc);

/*!< Blocking mode: Polling */
HAL_StatusTypeDef       HAL_ADCEx_InjectedStart(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADCEx_InjectedStop(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADCEx_InjectedPollForConversion(ADC_HandleTypeDef* hadc, uint32_t Timeout);

/*!< Non-blocking mode: Interruption */
HAL_StatusTypeDef       HAL_ADCEx_InjectedStart_IT(ADC_HandleTypeDef* hadc);
HAL_StatusTypeDef       HAL_ADCEx_InjectedStop_IT(ADC_HandleTypeDef* hadc);

/*!< ADC multimode */
HAL_StatusTypeDef       HAL_ADCEx_MultiModeStart_DMA(ADC_HandleTypeDef *hadc, uint32_t *pData, uint32_t Length);
HAL_StatusTypeDef       HAL_ADCEx_MultiModeStop_DMA(ADC_HandleTypeDef *hadc); 

/*!< ADC retrieve conversion value intended to be used with polling or interruption */
uint32_t                HAL_ADCEx_InjectedGetValue(ADC_HandleTypeDef* hadc, uint32_t InjectedRank);
uint32_t                HAL_ADCEx_MultiModeGetValue(ADC_HandleTypeDef *hadc);

/*!< ADC IRQHandler and Callbacks used in non-blocking modes (Interruption) */
void                    HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc);

HAL_StatusTypeDef       HAL_ADCEx_InjectedConfigChannel(ADC_HandleTypeDef* hadc,ADC_InjectionConfTypeDef* sConfigInjected);
HAL_StatusTypeDef       HAL_ADCEx_MultiModeConfigChannel(ADC_HandleTypeDef *hadc, ADC_MultiModeTypeDef *multimode); 
/** 
  * end of ADC_Function_Definitions @}  
  */

/** 
  * end of ADC @}  
  */

#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_ADC_EX_H_ */

