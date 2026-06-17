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
*   File    : rx32g4xx_hal_dac.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_DAC_H_
#define _RX32G4XX_HAL_DAC_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"
#include "rx32g4xx_hal_dma.h"


/* Exported types ------------------------------------------------------------*/

/** @addtogroup DAC
  * @{
  */

/******************************************************************************/
/*                               DAC Structures                               */
/******************************************************************************/
  
/** @defgroup DAC_Structure_Definitions DAC Structure Definitions
  * @{
  */
  
/** @defgroup HAL_DAC_StateTypeDef HAL DAC StateTypeDef
  * @ingroup  DAC_Structure_Definitions 
  * @{
  */
/**
  * @brief  HAL State structures definition
  */
typedef enum
{
  HAL_DAC_STATE_RESET             = 0x00U,  /*!< DAC not yet initialized or disabled  */
  HAL_DAC_STATE_READY             = 0x01U,  /*!< DAC initialized and ready for use    */
  HAL_DAC_STATE_BUSY              = 0x02U,  /*!< DAC internal processing is ongoing   */
  HAL_DAC_STATE_TIMEOUT           = 0x03U,  /*!< DAC timeout state                    */
  HAL_DAC_STATE_ERROR             = 0x04U   /*!< DAC error state                      */

} HAL_DAC_StateTypeDef;
   
/** 
  * end of HAL_DAC_StateTypeDef @}  
  */
  
/** @defgroup DAC_HandleTypeDef DAC HandleTypeDef
  * @ingroup  DAC_Structure_Definitions 
  * @{
  */
/**
  * @brief  DAC handle Structure definition
  */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
typedef struct __DAC_HandleTypeDef
#else
typedef struct
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */
{
  DAC_TypeDef                 *Instance;     /*!< Register base address             */

  __IO HAL_DAC_StateTypeDef   State;         /*!< DAC communication state           */

  HAL_LockTypeDef             Lock;          /*!< DAC locking object                */

  DMA_HandleTypeDef           *DMA_Handle1;  /*!< Pointer DMA handler for channel 1 */

  DMA_HandleTypeDef           *DMA_Handle2;  /*!< Pointer DMA handler for channel 2 */

  __IO uint32_t               ErrorCode;     /*!< DAC Error code                    */

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
  void (* ConvCpltCallbackCh1)(struct __DAC_HandleTypeDef *hdac);
  void (* ConvHalfCpltCallbackCh1)(struct __DAC_HandleTypeDef *hdac);
  void (* ErrorCallbackCh1)(struct __DAC_HandleTypeDef *hdac);
  void (* DMAUnderrunCallbackCh1)(struct __DAC_HandleTypeDef *hdac);

  void (* ConvCpltCallbackCh2)(struct __DAC_HandleTypeDef *hdac);
  void (* ConvHalfCpltCallbackCh2)(struct __DAC_HandleTypeDef *hdac);
  void (* ErrorCallbackCh2)(struct __DAC_HandleTypeDef *hdac);
  void (* DMAUnderrunCallbackCh2)(struct __DAC_HandleTypeDef *hdac);

  void (* MspInitCallback)(struct __DAC_HandleTypeDef *hdac);
  void (* MspDeInitCallback)(struct __DAC_HandleTypeDef *hdac);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

} DAC_HandleTypeDef;
  
/** 
  * end of DAC_HandleTypeDef @}  
  */

/** @defgroup DAC_ChannelConfTypeDef DAC ChannelConfTypeDef
  * @ingroup  DAC_Structure_Definitions 
  * @{
  */
/**
  * @brief   DAC Configuration regular Channel structure definition
  */
typedef struct
{
  uint32_t DAC_Trigger;                  /*!< Specifies the external trigger for the selected DAC channel.
                                              This parameter can be a value of @ref DAC_trigger_selection.
                                              Note: In case of sawtooth wave generation, this
                                              trigger corresponds to the reset trigger. */

  uint32_t DAC_Trigger2;                 /*!< Specifies the external secondary trigger for the selected DAC channel.
                                              This parameter can be a value of @ref DAC_trigger_selection.
                                              Note: In case of sawtooth wave generation, this
                                              trigger corresponds to the step trigger.*/
                                              
  uint32_t DAC_Wave;                     /*!< Specifies the generating wave for the selected DAC channel.
                                              This parameter can be a value of @ref DAC_output_wave */
                                              
  uint32_t DAC_Dir;                      /*!< Specifies the sawtooth wave generating direction for the selected DAC channel
                                              This parameter can be a value of @ref DAC_Sawtooth_Direction */
                                              
  uint32_t DAC_MAMP;                     /*!< Specifies the amplitude of triangle wave and the mask of the noise wave
                                              This parameter can be a value of @ref DAC_wave_mamp
                                              Note: This parameter will only be configured when "Triangle wave" or "Noise wave" is selected */

  uint32_t DAC_OutputBuffer;             /*!< Specifies whether the DAC channel output buffer is enabled or disabled.
                                               This parameter can be a value of @ref DAC_output_buffer */

  uint32_t DAC_UserTrimming;             /*!< Specifies the trimming mode
                                              This parameter must be a value of @ref DAC_UserTrimming
                                              DAC_UserTrimming is either factory or user trimming */

  uint32_t DAC_TrimmingValue;             /*!< Specifies the offset trimming value
                                               This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
} DAC_ChannelConfTypeDef;
  
/** 
  * end of DAC_ChannelConfTypeDef @}  
  */

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)  
/** @defgroup HAL_DAC_CallbackIDTypeDef HAL DAC CallbackIDTypeDef
  * @ingroup  DAC_Structure_Definitions 
  * @{
  */
/**
  * @brief  HAL DAC Callback ID enumeration definition
  */
typedef enum
{
  HAL_DAC_CH1_COMPLETE_CB_ID                 = 0x00U,  /*!< DAC CH1 Complete Callback ID      */
  HAL_DAC_CH1_HALF_COMPLETE_CB_ID            = 0x01U,  /*!< DAC CH1 half Complete Callback ID */
  HAL_DAC_CH1_ERROR_ID                       = 0x02U,  /*!< DAC CH1 error Callback ID         */
  HAL_DAC_CH1_UNDERRUN_CB_ID                 = 0x03U,  /*!< DAC CH1 underrun Callback ID      */

  HAL_DAC_CH2_COMPLETE_CB_ID                 = 0x04U,  /*!< DAC CH2 Complete Callback ID      */
  HAL_DAC_CH2_HALF_COMPLETE_CB_ID            = 0x05U,  /*!< DAC CH2 half Complete Callback ID */
  HAL_DAC_CH2_ERROR_ID                       = 0x06U,  /*!< DAC CH2 error Callback ID         */
  HAL_DAC_CH2_UNDERRUN_CB_ID                 = 0x07U,  /*!< DAC CH2 underrun Callback ID      */

  HAL_DAC_MSPINIT_CB_ID                      = 0x08U,  /*!< DAC MspInit Callback ID           */
  HAL_DAC_MSPDEINIT_CB_ID                    = 0x09U,  /*!< DAC MspDeInit Callback ID         */
  HAL_DAC_ALL_CB_ID                          = 0x0AU   /*!< DAC All ID                        */
} HAL_DAC_CallbackIDTypeDef;  
/** 
  * end of HAL_DAC_CallbackIDTypeDef @}  
  */
  
/** @defgroup pDAC_CallbackTypeDef pDAC CallbackTypeDef
  * @ingroup  DAC_Structure_Definitions 
  * @{
  */
/**
  * @brief  HAL DAC Callback pointer definition
  */
typedef void (*pDAC_CallbackTypeDef)(DAC_HandleTypeDef *hdac);
/** 
  * end of pDAC_CallbackTypeDef @}  
  */
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */  
/** 
  * end of DAC_Structure_Definitions @}  
  */

/******************************************************************************/
/*                               DAC Parameters                               */
/******************************************************************************/
  
/** @defgroup DAC_Parameter_Definitions DAC Parameter Definitions
  * @{
  */
  
/** @defgroup DAC_Error_Code DAC Error Code
  * @ingroup  DAC_Parameter_Definitions
  * @{
  */
#define  HAL_DAC_ERROR_NONE              0x00U    /*!< No error                          */
#define  HAL_DAC_ERROR_DMAUNDERRUNCH1    0x01U    /*!< DAC channel1 DMA underrun error   */
#define  HAL_DAC_ERROR_DMAUNDERRUNCH2    0x02U    /*!< DAC channel2 DMA underrun error   */
#define  HAL_DAC_ERROR_DMA               0x04U    /*!< DMA error                         */
#define  HAL_DAC_ERROR_TIMEOUT           0x08U    /*!< Timeout error                     */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
#define HAL_DAC_ERROR_INVALID_CALLBACK   0x10U    /*!< Invalid callback error            */
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */  
/** 
  * end of DAC_Error_Code @}  
  */

/** @defgroup DAC_output_wave DAC output wave
  * @ingroup  DAC_Parameter_Definitions
  * @{
  */ 
#define DAC_WAVE_NOWAVE                   0x00000000
#define DAC_WAVE_NOISE                    DAC_CR_WAVE1_BIT0
#define DAC_WAVE_TRIANGLE                 DAC_CR_WAVE1_BIT1
#define DAC_WAVE_SAWTOOTH                 (DAC_CR_WAVE1_BIT1 | DAC_CR_WAVE1_BIT0)
/** 
  * end of DAC_output_wave @}  
  */  

/** @defgroup DAC_trigger_selection DAC trigger selection
  * @ingroup  DAC_Parameter_Definitions
  * @{
  */  
#define DAC_TRIGGER_NONE                0x00000000UL                                                                      
#define DAC_TRIGGER_SOFTWARE            (                                                                                DAC_CR_TEN1)
#define DAC_TRIGGER_T1_TRGO             (                                                            DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_T8_TRGO             (                                                            DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_T7_TRGO             (                                        DAC_CR_TSEL1_BIT1                     | DAC_CR_TEN1) 
#define DAC_TRIGGER_T15_TRGO            (                                        DAC_CR_TSEL1_BIT1 | DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_T2_TRGO             (                    DAC_CR_TSEL1_BIT2                                         | DAC_CR_TEN1) 
#define DAC_TRIGGER_T4_TRGO             (                    DAC_CR_TSEL1_BIT2                     | DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_EXT_IT9             (                    DAC_CR_TSEL1_BIT2 | DAC_CR_TSEL1_BIT1                     | DAC_CR_TEN1) 
#define DAC_TRIGGER_EXT_IT10            (                    DAC_CR_TSEL1_BIT2 | DAC_CR_TSEL1_BIT1                     | DAC_CR_TEN1) 
#define DAC_TRIGGER_T6_TRGO             (                    DAC_CR_TSEL1_BIT2 | DAC_CR_TSEL1_BIT1 | DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_T3_TRGO             (DAC_CR_TSEL1_BIT3                                                             | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_RST_TRG1      (DAC_CR_TSEL1_BIT3                                         | DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_STEP_TRG1     (DAC_CR_TSEL1_BIT3                                         | DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_RST_TRG2      (DAC_CR_TSEL1_BIT3                     | DAC_CR_TSEL1_BIT1                     | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_STEP_TRG2     (DAC_CR_TSEL1_BIT3                     | DAC_CR_TSEL1_BIT1                     | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_RST_TRG3      (DAC_CR_TSEL1_BIT3                     | DAC_CR_TSEL1_BIT1 | DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_STEP_TRG3     (DAC_CR_TSEL1_BIT3                     | DAC_CR_TSEL1_BIT1 | DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_RST_TRG4      (DAC_CR_TSEL1_BIT3 | DAC_CR_TSEL1_BIT2                                         | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_STEP_TRG4     (DAC_CR_TSEL1_BIT3 | DAC_CR_TSEL1_BIT2                                         | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_RST_TRG5      (DAC_CR_TSEL1_BIT3 | DAC_CR_TSEL1_BIT2                     | DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_STEP_TRG5     (DAC_CR_TSEL1_BIT3 | DAC_CR_TSEL1_BIT2                     | DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_RST_TRG6      (DAC_CR_TSEL1_BIT3 | DAC_CR_TSEL1_BIT2 | DAC_CR_TSEL1_BIT1                     | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_STEP_TRG6     (DAC_CR_TSEL1_BIT3 | DAC_CR_TSEL1_BIT2 | DAC_CR_TSEL1_BIT1                     | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_TRG01         (DAC_CR_TSEL1_BIT3 | DAC_CR_TSEL1_BIT2 | DAC_CR_TSEL1_BIT1 | DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_TRG02         (DAC_CR_TSEL1_BIT3 | DAC_CR_TSEL1_BIT2 | DAC_CR_TSEL1_BIT1 | DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
#define DAC_TRIGGER_HRTIM_TRG03         (DAC_CR_TSEL1_BIT3 | DAC_CR_TSEL1_BIT2 | DAC_CR_TSEL1_BIT1 | DAC_CR_TSEL1_BIT0 | DAC_CR_TEN1) 
/** 
  * end of DAC_trigger_selection @}  
  */
  
/** @defgroup DAC_wave_mamp DAC DAC wave mamp
  * @ingroup  DAC_Parameter_Definitions
  * @{
  */ 
#define DAC_LFSRUNMASK_BIT0                0x00000000UL                                                                   /*!< Unmask DAC channel LFSR bit0 for noise wave generation */
#define DAC_LFSRUNMASK_BITS1_0             (                                                           DAC_CR_MAMP1_BIT0) /*!< Unmask DAC channel LFSR bit[1:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS2_0             (                                       DAC_CR_MAMP1_BIT1                    ) /*!< Unmask DAC channel LFSR bit[2:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS3_0             (                                       DAC_CR_MAMP1_BIT1 | DAC_CR_MAMP1_BIT0) /*!< Unmask DAC channel LFSR bit[3:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS4_0             (                   DAC_CR_MAMP1_BIT2                                        ) /*!< Unmask DAC channel LFSR bit[4:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS5_0             (                   DAC_CR_MAMP1_BIT2                     | DAC_CR_MAMP1_BIT0) /*!< Unmask DAC channel LFSR bit[5:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS6_0             (                   DAC_CR_MAMP1_BIT2 | DAC_CR_MAMP1_BIT1                    ) /*!< Unmask DAC channel LFSR bit[6:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS7_0             (                   DAC_CR_MAMP1_BIT2 | DAC_CR_MAMP1_BIT1 | DAC_CR_MAMP1_BIT0) /*!< Unmask DAC channel LFSR bit[7:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS8_0             (DAC_CR_MAMP1_BIT3                                                           ) /*!< Unmask DAC channel LFSR bit[8:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS9_0             (DAC_CR_MAMP1_BIT3                                        | DAC_CR_MAMP1_BIT0) /*!< Unmask DAC channel LFSR bit[9:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS10_0            (DAC_CR_MAMP1_BIT3                    | DAC_CR_MAMP1_BIT1                    ) /*!< Unmask DAC channel LFSR bit[10:0] for noise wave generation */
#define DAC_LFSRUNMASK_BITS11_0            (DAC_CR_MAMP1_BIT3                    | DAC_CR_MAMP1_BIT1 | DAC_CR_MAMP1_BIT0) /*!< Unmask DAC channel LFSR bit[11:0] for noise wave generation */
#define DAC_TRIANGLEAMPLITUDE_1            0x00000000UL                                                                   /*!< Select max triangle amplitude of 1 */
#define DAC_TRIANGLEAMPLITUDE_3            (                                                           DAC_CR_MAMP1_BIT0) /*!< Select max triangle amplitude of 3 */
#define DAC_TRIANGLEAMPLITUDE_7            (                                       DAC_CR_MAMP1_BIT1                    ) /*!< Select max triangle amplitude of 7 */
#define DAC_TRIANGLEAMPLITUDE_15           (                                       DAC_CR_MAMP1_BIT1 | DAC_CR_MAMP1_BIT0) /*!< Select max triangle amplitude of 15 */
#define DAC_TRIANGLEAMPLITUDE_31           (                   DAC_CR_MAMP1_BIT2                                        ) /*!< Select max triangle amplitude of 31 */
#define DAC_TRIANGLEAMPLITUDE_63           (                   DAC_CR_MAMP1_BIT2                     | DAC_CR_MAMP1_BIT0) /*!< Select max triangle amplitude of 63 */
#define DAC_TRIANGLEAMPLITUDE_127          (                   DAC_CR_MAMP1_BIT2 | DAC_CR_MAMP1_BIT1                    ) /*!< Select max triangle amplitude of 127 */
#define DAC_TRIANGLEAMPLITUDE_255          (                   DAC_CR_MAMP1_BIT2 | DAC_CR_MAMP1_BIT1 | DAC_CR_MAMP1_BIT0) /*!< Select max triangle amplitude of 255 */
#define DAC_TRIANGLEAMPLITUDE_511          (DAC_CR_MAMP1_BIT3                                                           ) /*!< Select max triangle amplitude of 511 */
#define DAC_TRIANGLEAMPLITUDE_1023         (DAC_CR_MAMP1_BIT3                                        | DAC_CR_MAMP1_BIT0) /*!< Select max triangle amplitude of 1023 */
#define DAC_TRIANGLEAMPLITUDE_2047         (DAC_CR_MAMP1_BIT3                    | DAC_CR_MAMP1_BIT1                    ) /*!< Select max triangle amplitude of 2047 */
#define DAC_TRIANGLEAMPLITUDE_4095         (DAC_CR_MAMP1_BIT3                    | DAC_CR_MAMP1_BIT1 | DAC_CR_MAMP1_BIT0) /*!< Select max triangle amplitude of 4095 */
/** 
  * end of DAC_wave_mamp @}  
  */

/** @defgroup DAC_Sawtooth_Direction DAC Sawtooth Direction
  * @ingroup  DAC_Parameter_Definitions
  * @{
  */
#define DAC_SAWTOOTH_POLARITY_DECREMENT        0x00000000UL            /*!< Sawtooth wave generation, polarity is decrement */
#define DAC_SAWTOOTH_POLARITY_INCREMENT        (DAC_STR1_STDIR1)       /*!< Sawtooth wave generation, polarity is increment */
/** 
  * end of DAC_Sawtooth_Direction @}  
  */
  
/** @defgroup DAC_output_buffer DAC output buffer
  * @ingroup  DAC_Parameter_Definitions
  * @{
  */ 
#define DAC_OUTPUTBUFFER_ENABLE            0x00000000U
#define DAC_OUTPUTBUFFER_DISABLE           (DAC_CR_BOFF1)
/** 
  * end of DAC_output_buffer @}  
  */

/** @defgroup DAC_Channel_selection DAC Channel selection
  * @ingroup  DAC_Parameter_Definitions
  * @{
  */  
#define DAC_CHANNEL_1                      0x00000000U
#define DAC_CHANNEL_2                      0x00000010U
/** 
  * end of DAC_Channel_selection @}  
  */

/** @defgroup DAC_data_alignment DAC data alignment
  * @ingroup  DAC_Parameter_Definitions
  * @{
  */  
#define DAC_ALIGN_12B_R                    0x00000000U
#define DAC_ALIGN_12B_L                    0x00000004U
#define DAC_ALIGN_8B_R                     0x00000008U
/** 
  * end of DAC_data_alignment @}  
  */

/** @defgroup DAC_flags_definition DAC flags definition
  * @ingroup  DAC_Parameter_Definitions
  * @{
  */  
#define DAC_FLAG_DMAUDR1                   (DAC_SR_DMAUDR1)
#define DAC_FLAG_DMAUDR2                   (DAC_SR_DMAUDR2)
#define DAC_FLAG_CAL1                      (DAC_SR_CAL1)
#define DAC_FLAG_CAL2                      (DAC_SR_CAL2)
/** 
  * end of DAC_flags_definition @}  
  */

/** @defgroup DAC_IT_definition DAC IT definition
  * @ingroup  DAC_Parameter_Definitions
  * @{
  */  
#define DAC_IT_DMAUDR1                   (DAC_CR_DMAUDRIE1)
#define DAC_IT_DMAUDR2                   (DAC_CR_DMAUDRIE2)
/** 
  * end of DAC_IT_definition @}  
  */

/** @defgroup DAC_UserTrimming DAC UserTrimming
  * @ingroup  DAC_Parameter_Definitions
  * @{
  */  
#define DAC_TRIMMING_FACTORY        (0x00000000UL)        /*!< Factory trimming */
#define DAC_TRIMMING_USER           (0x00000001UL)        /*!< User trimming */
/** 
  * end of DAC_UserTrimming @}  
  */

/** @defgroup DAC_HighFrequency DAC HighFrequency
  * @ingroup  DAC_Parameter_Definitions
  * @{
  */  
#define DAC_HIGH_FREQUENCY_INTERFACE_MODE_DISABLE        0x00000000UL          /*!< High frequency interface mode disabled */
#define DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_80MHZ    (DAC_MCR_HFSEL_BIT0)  /*!< High frequency interface mode compatible to AHB>80MHz enabled */
#define DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_160MHZ   (DAC_MCR_HFSEL_BIT1)  /*!< High frequency interface mode compatible to AHB>160MHz enabled */
/** 
  * end of DAC_HighFrequency @}  
  */  
/** 
  * end of DAC_Parameter_Definitions @}  
  */


/******************************************************************************/
/*                                 DAC Macro                                  */
/******************************************************************************/
/** @defgroup DAC_Macro_Definitions DAC Macro Definitions
  * @{
  */

/** @brief Reset DAC handle state.
  * @param  __HANDLE__ specifies the DAC handle.
  * @retval None
  */
#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
#define __HAL_DAC_RESET_HANDLE_STATE(__HANDLE__) do {                                                        \
                                                      (__HANDLE__)->State             = HAL_DAC_STATE_RESET; \
                                                      (__HANDLE__)->MspInitCallback   = NULL;                \
                                                      (__HANDLE__)->MspDeInitCallback = NULL;                \
                                                     } while(0)
#else
#define __HAL_DAC_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_DAC_STATE_RESET)
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */

/** @brief Enable the DAC channel.
  * @param  __HANDLE__ specifies the DAC handle.
  * @param  __DAC_Channel__ specifies the DAC channel
  * @retval None
  */
#define __HAL_DAC_ENABLE(__HANDLE__, __DAC_Channel__) \
  ((__HANDLE__)->Instance->CR |=  (DAC_CR_EN1 << ((__DAC_Channel__) & 0x10UL)))

/** @brief Disable the DAC channel.
  * @param  __HANDLE__ specifies the DAC handle
  * @param  __DAC_Channel__ specifies the DAC channel.
  * @retval None
  */
#define __HAL_DAC_DISABLE(__HANDLE__, __DAC_Channel__) \
  ((__HANDLE__)->Instance->CR &=  ~(DAC_CR_EN1 << ((__DAC_Channel__) & 0x10UL)))

/** @brief Set DHR12R1 alignment.
  * @param  __ALIGNMENT__ specifies the DAC alignment
  * @retval None
  */
#define DAC_DHR12R1_ALIGNMENT(__ALIGNMENT__) (0x00000008UL + (__ALIGNMENT__))


/** @brief  Set DHR12R2 alignment.
  * @param  __ALIGNMENT__ specifies the DAC alignment
  * @retval None
  */
#define DAC_DHR12R2_ALIGNMENT(__ALIGNMENT__) (0x00000014UL + (__ALIGNMENT__))


/** @brief  Set DHR12RD alignment.
  * @param  __ALIGNMENT__ specifies the DAC alignment
  * @retval None
  */
#define DAC_DHR12RD_ALIGNMENT(__ALIGNMENT__) (0x00000020UL + (__ALIGNMENT__))

/** @brief Enable the DAC interrupt.
  * @param  __HANDLE__ specifies the DAC handle
  * @param  __INTERRUPT__ specifies the DAC interrupt.
  *          This parameter can be any combination of the following values:
  *            @arg DAC_IT_DMAUDR1 DAC channel 1 DMA underrun interrupt
  *            @arg DAC_IT_DMAUDR2 DAC channel 2 DMA underrun interrupt
  * @retval None
  */
#define __HAL_DAC_ENABLE_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->CR) |= (__INTERRUPT__))

/** @brief Disable the DAC interrupt.
  * @param  __HANDLE__ specifies the DAC handle
  * @param  __INTERRUPT__ specifies the DAC interrupt.
  *          This parameter can be any combination of the following values:
  *            @arg DAC_IT_DMAUDR1 DAC channel 1 DMA underrun interrupt
  *            @arg DAC_IT_DMAUDR2 DAC channel 2 DMA underrun interrupt
  * @retval None
  */
#define __HAL_DAC_DISABLE_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->CR) &= ~(__INTERRUPT__))

/** @brief  Check whether the specified DAC interrupt source is enabled or not.
  * @param __HANDLE__ DAC handle
  * @param __INTERRUPT__ DAC interrupt source to check
  *          This parameter can be any combination of the following values:
  *            @arg DAC_IT_DMAUDR1 DAC channel 1 DMA underrun interrupt
  *            @arg DAC_IT_DMAUDR2 DAC channel 2 DMA underrun interrupt
  * @retval State of interruption (SET or RESET)
  */
#define __HAL_DAC_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->CR\
                                                             & (__INTERRUPT__)) == (__INTERRUPT__))

/** @brief  Get the selected DAC's flag status.
  * @param  __HANDLE__ specifies the DAC handle.
  * @param  __FLAG__ specifies the DAC flag to get.
  *          This parameter can be any combination of the following values:
  *            @arg DAC_FLAG_DMAUDR1 DAC channel 1 DMA underrun flag
  *            @arg DAC_FLAG_DMAUDR2 DAC channel 2 DMA underrun flag
  * @retval None
  */
#define __HAL_DAC_GET_FLAG(__HANDLE__, __FLAG__) ((((__HANDLE__)->Instance->SR) & (__FLAG__)) == (__FLAG__))

/** @brief  Clear the DAC's flag.
  * @param  __HANDLE__ specifies the DAC handle.
  * @param  __FLAG__ specifies the DAC flag to clear.
  *          This parameter can be any combination of the following values:
  *            @arg DAC_FLAG_DMAUDR1 DAC channel 1 DMA underrun flag
  *            @arg DAC_FLAG_DMAUDR2 DAC channel 2 DMA underrun flag
  * @retval None
  */
#define __HAL_DAC_CLEAR_FLAG(__HANDLE__, __FLAG__) (((__HANDLE__)->Instance->SR) = (__FLAG__))

/** @brief  Check of the parameter STATE is valid
  * @param  STATE
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET(invalid)
  */
#define IS_DAC_OUTPUT_BUFFER_STATE(STATE) (((STATE) == DAC_OUTPUTBUFFER_ENABLE) || \
                                           ((STATE) == DAC_OUTPUTBUFFER_DISABLE))

/** @brief  Check of the parameter CHANNEL is valid
  * @param  CHANNEL
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET(invalid)
  */
#define IS_DAC_CHANNEL(CHANNEL)              \
  (((CHANNEL) == DAC_CHANNEL_1)     ||       \
   ((CHANNEL) == DAC_CHANNEL_2))

/** @brief  Check of the parameter ALIGN is valid
  * @param  ALIGN
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET(invalid)
  */
#define IS_DAC_ALIGN(ALIGN) (((ALIGN) == DAC_ALIGN_12B_R) || \
                             ((ALIGN) == DAC_ALIGN_12B_L) || \
                             ((ALIGN) == DAC_ALIGN_8B_R))

/** @brief  Check of the parameter DATA is valid
  * @param  DATA
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET(invalid)
  */
#define IS_DAC_DATA(DATA) ((DATA) <= 0xFFF0UL)

/** @brief  Check of the parameter DACX, TRIGGER is valid
  * @param  DACX
  * @param  TRIGGER
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET(invalid)
  */
#define IS_DAC_TRIGGER(DACX, TRIGGER) \
  (((TRIGGER) == DAC_TRIGGER_NONE)           || \
   ((TRIGGER) == DAC_TRIGGER_SOFTWARE)       || \
   ((TRIGGER) == DAC_TRIGGER_T7_TRGO)        || \
   ((TRIGGER) == DAC_TRIGGER_T15_TRGO)       || \
   ((TRIGGER) == DAC_TRIGGER_T2_TRGO)        || \
   ((TRIGGER) == DAC_TRIGGER_T4_TRGO)        || \
   ((TRIGGER) == DAC_TRIGGER_EXT_IT9)        || \
   ((TRIGGER) == DAC_TRIGGER_EXT_IT10)       || \
   ((TRIGGER) == DAC_TRIGGER_T6_TRGO)        || \
   ((TRIGGER) == DAC_TRIGGER_T3_TRGO)        || \
   ((TRIGGER) == DAC_TRIGGER_HRTIM_RST_TRG1) || \
   ((TRIGGER) == DAC_TRIGGER_HRTIM_RST_TRG2) || \
   ((TRIGGER) == DAC_TRIGGER_HRTIM_RST_TRG3) || \
   ((TRIGGER) == DAC_TRIGGER_HRTIM_RST_TRG4) || \
   ((TRIGGER) == DAC_TRIGGER_HRTIM_RST_TRG5) || \
   ((TRIGGER) == DAC_TRIGGER_HRTIM_RST_TRG6) || \
   (((DACX) == DAC1) &&                         \
    (((TRIGGER) == DAC_TRIGGER_T8_TRGO)      || \
     ((TRIGGER) == DAC_TRIGGER_HRTIM_TRG01))     \
   )                                         || \
   (((DACX) == DAC2) &&                         \
    (((TRIGGER) == DAC_TRIGGER_T8_TRGO)      || \
     ((TRIGGER) == DAC_TRIGGER_HRTIM_TRG02))     \
   )                                         || \
   (((DACX) == DAC3) &&                         \
    (((TRIGGER) == DAC_TRIGGER_T1_TRGO)      || \
     ((TRIGGER) == DAC_TRIGGER_HRTIM_TRG03))     \
   )                                           \
  )

/** @brief  Check of the parameter TRIMMINGVALUE is valid
  * @param  TRIMMINGVALUE
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET(invalid)
  */
#define IS_DAC_TRIMMINGVALUE(TRIMMINGVALUE) ((TRIMMINGVALUE) <= 0x1FU)

/** @brief  Check of the parameter TRIMMINGVALUE is valid
  * @param  TRIMMINGVALUE
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET(invalid)
  */
#define IS_DAC_NEWTRIMMINGVALUE(TRIMMINGVALUE) ((TRIMMINGVALUE) <= 0x1FU)

/** @brief  Check of the parameter TRIMMING is valid
  * @param  TRIMMING
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET(invalid)
  */
#define IS_DAC_TRIMMING(TRIMMING) (((TRIMMING) == DAC_TRIMMING_FACTORY) || \
                                   ((TRIMMING) == DAC_TRIMMING_USER))

/** @brief  Check of the parameter VALUE is valid
  * @param  VALUE
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET(invalid)
  */
#define IS_DAC_LFSR_UNMASK_TRIANGLE_AMPLITUDE(VALUE) (((VALUE) == DAC_LFSRUNMASK_BIT0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS1_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS2_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS3_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS4_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS5_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS6_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS7_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS8_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS9_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS10_0) || \
                                                      ((VALUE) == DAC_LFSRUNMASK_BITS11_0) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_1) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_3) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_7) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_15) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_31) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_63) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_127) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_255) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_511) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_1023) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_2047) || \
                                                      ((VALUE) == DAC_TRIANGLEAMPLITUDE_4095))

/** @brief  Check of the parameter POLARITY is valid
  * @param  POLARITY
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET(invalid)
  */
#define IS_DAC_SAWTOOTH_POLARITY(POLARITY) (((POLARITY) == DAC_SAWTOOTH_POLARITY_DECREMENT) || \
                                            ((POLARITY) == DAC_SAWTOOTH_POLARITY_INCREMENT))

/** @brief  Check of the parameter DATA is valid
  * @param  DATA
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET(invalid)
  */
#define IS_DAC_RESET_DATA(DATA) ((DATA) <= 0x00000FFFUL)

/** @brief  Check of the parameter DATA is valid
  * @param  DATA
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET(invalid)
  */
#define IS_DAC_STEP_DATA(DATA)  ((DATA) <= 0x0000FFFFUL)  
/** 
  * end of DAC_Macro_Definitions @}  
  */
  
/******************************************************************************/
/*                                DAC Functions                               */
/******************************************************************************/

/** @defgroup DAC_Function_Definitions DAC Function Definitions
  * @{
  */
  
/* Initialization and de-initialization functions *****************************/
HAL_StatusTypeDef               HAL_DAC_Init(DAC_HandleTypeDef *hdac);
HAL_StatusTypeDef               HAL_DAC_DeInit(DAC_HandleTypeDef *hdac);
void                            HAL_DAC_MspInit(DAC_HandleTypeDef *hdac);
void                            HAL_DAC_MspDeInit(DAC_HandleTypeDef *hdac);

/* IO operation functions *****************************************************/
HAL_StatusTypeDef               HAL_DAC_Start(DAC_HandleTypeDef *hdac, uint32_t Channel);
HAL_StatusTypeDef               HAL_DAC_Stop(DAC_HandleTypeDef *hdac, uint32_t Channel);
HAL_StatusTypeDef               HAL_DAC_Start_DMA(DAC_HandleTypeDef *hdac, uint32_t Channel, const uint32_t *pData, uint32_t Length, uint32_t Alignment);
HAL_StatusTypeDef               HAL_DAC_Stop_DMA(DAC_HandleTypeDef *hdac, uint32_t Channel);
void                            HAL_DAC_IRQHandler(DAC_HandleTypeDef *hdac);
HAL_StatusTypeDef               HAL_DAC_SetValue(DAC_HandleTypeDef *hdac, uint32_t Channel, uint32_t Alignment, uint32_t Data);

void                            HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac);
void                            HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac);
void                            HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac);
void                            HAL_DAC_DMAUnderrunCallbackCh1(DAC_HandleTypeDef *hdac);

#if (USE_HAL_DAC_REGISTER_CALLBACKS == 1)
/* DAC callback registering/unregistering */
HAL_StatusTypeDef               HAL_DAC_RegisterCallback(DAC_HandleTypeDef *hdac, HAL_DAC_CallbackIDTypeDef CallbackID,
                                               pDAC_CallbackTypeDef pCallback);
HAL_StatusTypeDef               HAL_DAC_UnRegisterCallback(DAC_HandleTypeDef *hdac, HAL_DAC_CallbackIDTypeDef CallbackID);
#endif /* USE_HAL_DAC_REGISTER_CALLBACKS */


/* Peripheral Control functions ***********************************************/
uint32_t                        HAL_DAC_GetValue(const DAC_HandleTypeDef *hdac, uint32_t Channel);
HAL_StatusTypeDef               HAL_DAC_ConfigChannel(DAC_HandleTypeDef *hdac, const DAC_ChannelConfTypeDef *sConfig, uint32_t Channel);

/* Peripheral State and Error functions ***************************************/
HAL_DAC_StateTypeDef            HAL_DAC_GetState(const DAC_HandleTypeDef *hdac);
uint32_t                        HAL_DAC_GetError(const DAC_HandleTypeDef *hdac);

void                            DAC_DMAConvCpltCh1(DMA_HandleTypeDef *hdma);
void                            DAC_DMAErrorCh1(DMA_HandleTypeDef *hdma);
void                            DAC_DMAHalfConvCpltCh1(DMA_HandleTypeDef *hdma);
   
/** 
  * end of DAC_Function_Definitions @}  
  */
/** 
  * end of DAC @}  
  */


#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_DAC_H_ */

