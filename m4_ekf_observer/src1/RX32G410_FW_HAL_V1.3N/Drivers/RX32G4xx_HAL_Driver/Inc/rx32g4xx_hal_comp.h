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
*   File    : rx32g4xx_hal_comp.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_COMP_H_
#define _RX32G4XX_HAL_COMP_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_common.h"
#include "rx32g4xx_hal_exti.h"

/* Exported types ------------------------------------------------------------*/

/** @addtogroup COMP
  * @{
  */

/******************************************************************************/
/*                               COMP Structures                              */
/******************************************************************************/
  
/** @defgroup COMP_Structure_Definitions COMP Structure Definitions
  * @{
  */
  
/** @defgroup COMP_InitTypeDef COMP InitTypeDef
  * @ingroup  COMP_Structure_Definitions 
  * @{
  */
/**
  * @brief  COMP Init structure definition
  */
typedef struct
{

  uint32_t InputPlus;                 /*!< Set comparator input plus (non-inverting input).
                                           This parameter can be a value of @ref COMP_InputPlus */

  uint32_t InputMinus;                /*!< Set comparator input minus (inverting input).
                                           This parameter can be a value of @ref COMP_InputMinus */

  uint32_t Rising_Hysteresis;         /*!< Set comparator hysteresis mode of the input minus.
                                           This parameter can be a value of @ref COMP_Rising_Hysteresis */

  uint32_t Falling_Hysteresis;        /*!< Set comparator hysteresis mode of the input minus.
                                           This parameter can be a value of @ref COMP_Falling_Hysteresis */

  uint32_t OutputPol;                 /*!< Set comparator output polarity.
                                           This parameter can be a value of @ref COMP_OutputPolarity */

  uint32_t BlankingSrc;               /*!< Set comparator blanking source.
                                           This parameter can be a value of @ref COMP_Blanking_Source */

  uint32_t TriggerMode;               /*!< Set the comparator output triggering External Interrupt Line (EXTI).
                                           This parameter can be a value of @ref COMP_EXTI_TriggerMode */
                                           
  uint32_t IT;                       /*!< Set the comparator output triggering Rising edge/Falling edge/High Level/Low Level Interrupt.
                                           This parameter can be a value of @ref COMP_IT */                                         
  uint32_t OFLT;	                    /*!< Set comparator filter.
                                           This parameter can be a value of @ref COMP_OFLT */
} COMP_InitTypeDef; 
/** 
  * end of COMP_InitTypeDef @}  
  */

/** @defgroup HAL_COMP_StateTypeDef HAL COMP StateTypeDef
  * @ingroup  COMP_Structure_Definitions 
  * @{
  */  
/**
  * @brief  HAL COMP state machine: HAL COMP states definition
  */
#define COMP_STATE_BITFIELD_LOCK  (0x10U)
typedef enum
{
    HAL_COMP_STATE_RESET             = 0x00U,                                             /*!< COMP not yet initialized                             */
    HAL_COMP_STATE_RESET_LOCKED      = (HAL_COMP_STATE_RESET | COMP_STATE_BITFIELD_LOCK), /*!< COMP not yet initialized and configuration is locked */
    HAL_COMP_STATE_READY             = 0x01U,                                             /*!< COMP initialized and ready for use                   */
    HAL_COMP_STATE_READY_LOCKED      = (HAL_COMP_STATE_READY | COMP_STATE_BITFIELD_LOCK), /*!< COMP initialized but configuration is locked         */
    HAL_COMP_STATE_BUSY              = 0x02U,                                             /*!< COMP is running                                      */
    HAL_COMP_STATE_BUSY_LOCKED       = (HAL_COMP_STATE_BUSY | COMP_STATE_BITFIELD_LOCK)   /*!< COMP is running and configuration is locked          */
} HAL_COMP_StateTypeDef;

/** 
  * end of HAL_COMP_StateTypeDef @}  
  */
  
/** @defgroup COMP_HandleTypeDef COMP HandleTypeDef
  * @ingroup  COMP_Structure_Definitions 
  * @{
  */  
/**
  * @brief  COMP Handle Structure definition
  */
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
typedef struct __COMP_HandleTypeDef
#else
typedef struct
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */
{
    COMP_TypeDef                *Instance;       /*!< Register base address    */
    COMP_InitTypeDef            Init;            /*!< COMP required parameters */
    HAL_LockTypeDef             Lock;            /*!< Locking object           */
    __IO HAL_COMP_StateTypeDef  State;           /*!< COMP communication state */
    __IO uint32_t               ErrorCode;       /*!< COMP error code */
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
    void (* TriggerCallback)(struct __COMP_HandleTypeDef *hcomp);   /*!< COMP trigger callback */
    void (* MspInitCallback)(struct __COMP_HandleTypeDef *hcomp);   /*!< COMP Msp Init callback */
    void (* MspDeInitCallback)(struct __COMP_HandleTypeDef *hcomp); /*!< COMP Msp DeInit callback */
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */
} COMP_HandleTypeDef;
/** 
  * end of COMP_HandleTypeDef @}  
  */
  
/** @defgroup HAL_COMP_CallbackIDTypeDef HAL COMP CallbackIDTypeDef
  * @ingroup  COMP_Structure_Definitions 
  * @{
  */ 
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL COMP Callback ID enumeration definition
  */
typedef enum
{
    HAL_COMP_TRIGGER_CB_ID                = 0x00U,  /*!< COMP trigger callback ID */
    HAL_COMP_MSPINIT_CB_ID                = 0x01U,  /*!< COMP Msp Init callback ID */
    HAL_COMP_MSPDEINIT_CB_ID              = 0x02U   /*!< COMP Msp DeInit callback ID */
} HAL_COMP_CallbackIDTypeDef;
/** 
  * end of HAL_COMP_CallbackIDTypeDef @}  
  */
  
/** @defgroup pCOMP_CallbackTypeDef pCOMP CallbackTypeDef
  * @ingroup  COMP_Structure_Definitions 
  * @{
  */
/**
  * @brief  HAL COMP Callback pointer definition
  */
typedef  void (*pCOMP_CallbackTypeDef)(COMP_HandleTypeDef *hcomp); /*!< pointer to a COMP callback function */

#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */  
/** 
  * end of pCOMP_CallbackTypeDef @}  
  */
/** 
  * end of COMP_Structure_Definitions @}  
  */

/******************************************************************************/
/*                               COMP Parameters                              */
/******************************************************************************/
  
/** @defgroup COMP_Parameter_Definitions COMP Parameter Definitions
  * @{
  */
  
/** @defgroup COMP_Error_Code COMP Error Code
  * @ingroup  COMP_Parameter_Definitions
  * @{
  */
#define HAL_COMP_ERROR_NONE             (0x00UL)  /*!< No error */
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
#define HAL_COMP_ERROR_INVALID_CALLBACK (0x01UL)  /*!< Invalid Callback error */
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */
/** 
  * end of COMP_Error_Code @}  
  */
  
/** @defgroup COMP_InputPlus COMP Input Plus
  * @ingroup  COMP_Parameter_Definitions
  * @{
  */
#define COMP_INPUT_PLUS_IO1            (0x00000000UL)    
#define COMP_INPUT_PLUS_IO2            (COMP_CR_INP_SEL) 
/** 
  * end of COMP_Error_Code @}  
  */
  
/** @defgroup COMP_InputMinus COMP Input Minus
  * @ingroup  COMP_Parameter_Definitions
  * @{
  */
#define COMP_INPUT_MINUS_1_4VREFINT    (                                                                     COMP_CR_BRGEN)        
#define COMP_INPUT_MINUS_1_2VREFINT    (                                              COMP_CR_INM_SEL_BIT0 | COMP_CR_BRGEN)        
#define COMP_INPUT_MINUS_3_4VREFINT    (                       COMP_CR_INM_SEL_BIT1                        | COMP_CR_BRGEN)        
#define COMP_INPUT_MINUS_VREFINT       (                       COMP_CR_INM_SEL_BIT1 | COMP_CR_INM_SEL_BIT0 | COMP_CR_BRGEN)        
#define COMP_INPUT_MINUS_DAC1_CH1      (COMP_CR_INM_SEL_BIT2                        | COMP_CR_INM_SEL_BIT0                )        
#define COMP_INPUT_MINUS_DAC1_CH2      (COMP_CR_INM_SEL_BIT2                        | COMP_CR_INM_SEL_BIT0                )        
#define COMP_INPUT_MINUS_DAC2_CH1      (COMP_CR_INM_SEL_BIT2                        | COMP_CR_INM_SEL_BIT0                )         
#define COMP_INPUT_MINUS_DAC2_CH2      (COMP_CR_INM_SEL_BIT2                        | COMP_CR_INM_SEL_BIT0                )       
#define COMP_INPUT_MINUS_DAC3_CH1      (COMP_CR_INM_SEL_BIT2                                                              )        
#define COMP_INPUT_MINUS_DAC3_CH2      (COMP_CR_INM_SEL_BIT2                                                              )        
#define COMP_INPUT_MINUS_IO1           (COMP_CR_INM_SEL_BIT2 | COMP_CR_INM_SEL_BIT1                                       )        
#define COMP_INPUT_MINUS_IO2           (COMP_CR_INM_SEL_BIT2 | COMP_CR_INM_SEL_BIT1 | COMP_CR_INM_SEL_BIT0                )        

/** 
  * end of COMP_InputMinus @}  
  */
  
/** @defgroup COMP_Rising_Hysteresis COMP Rising Hysteresis
  * @ingroup  COMP_Parameter_Definitions
  * @{
  */
#define COMP_CR_RHYST_0mV                 ((uint32_t)0x00000000)
#define COMP_CR_RHYST_20mV                (                      COMP_CR_RHYST_BIT0 )
#define COMP_CR_RHYST_40mV                ( COMP_CR_RHYST_BIT1                      )
#define COMP_CR_RHYST_60mV                ( COMP_CR_RHYST_BIT1 | COMP_CR_RHYST_BIT0 )
/** 
  * end of COMP_Rising_Hysteresis @}  
  */
  
/** @defgroup COMP_Falling_Hysteresis COMP Falling Hysteresis
  * @ingroup  COMP_Parameter_Definitions
  * @{
  */
#define COMP_CR_FHYST_0mV                 ((uint32_t)0x00000000)
#define COMP_CR_FHYST_20mV                (                      COMP_CR_FHYST_BIT0 )
#define COMP_CR_FHYST_40mV                ( COMP_CR_FHYST_BIT1                      )
#define COMP_CR_FHYST_60mV                ( COMP_CR_FHYST_BIT1 | COMP_CR_FHYST_BIT0 )
/** 
  * end of COMP_Falling_Hysteresis @}  
  */
  
/** @defgroup COMP_OutputPolarity COMP Output Polarity
  * @ingroup  COMP_Parameter_Definitions
  * @{
  */
#define COMP_OUTPUTPOL_NONINVERTED     (0x00000000UL)   
#define COMP_OUTPUTPOL_INVERTED        (COMP_CR_POL)    
/** 
  * end of COMP_OutputPolarity @}  
  */
  
/** @defgroup COMP_Blanking_Source COMP Blanking Source
  * @ingroup  COMP_Parameter_Definitions
  * @{
  */
#define COMP_CR_BLANKING_SEL_NoBlanking   ((uint32_t)0x00000000)
#define COMP_CR_BLANKING_SEL_TIM1_OC5     (                                                         COMP_CR_BLANKING_SEL_BIT0 )
#define COMP_CR_BLANKING_SEL_TIM2_OC3     (                             COMP_CR_BLANKING_SEL_BIT1                             )
#define COMP_CR_BLANKING_SEL_TIM3_OC3     (                             COMP_CR_BLANKING_SEL_BIT1 | COMP_CR_BLANKING_SEL_BIT0 )
#define COMP_CR_BLANKING_SEL_TIM8_OC5     ( COMP_CR_BLANKING_SEL_BIT2                                                         )
#define COMP_CR_BLANKING_SEL_TIM5_OC3     ( COMP_CR_BLANKING_SEL_BIT2                             | COMP_CR_BLANKING_SEL_BIT0 )
#define COMP_CR_BLANKING_SEL_TIM15_OC1    ( COMP_CR_BLANKING_SEL_BIT2 | COMP_CR_BLANKING_SEL_BIT1                             )
#define COMP_CR_BLANKING_SEL_TIM4_OC3     ( COMP_CR_BLANKING_SEL_BIT2 | COMP_CR_BLANKING_SEL_BIT1 | COMP_CR_BLANKING_SEL_BIT0 )
/** 
  * end of COMP_Blanking_Source @}  
  */
  
/** @defgroup COMP_OutputLevel COMP Output Level
  * @ingroup  COMP_Parameter_Definitions
  * @{
  */
/* When output polarity is not inverted, comparator output is low when
   the input plus is at a lower voltage than the input minus */
#define COMP_OUTPUT_LEVEL_LOW              (0x00000000UL)
/* When output polarity is not inverted, comparator output is high when
   the input plus is at a higher voltage than the input minus */
#define COMP_OUTPUT_LEVEL_HIGH             (0x00000001UL)
/** 
  * end of COMP_OutputLevel @}  
  */
  
/** @defgroup COMP_IT COMP IT
  * @ingroup  COMP_IT
  * @{
  */
#define COMP_IT_NONE                          (0)
#define COMP_IT_RISING                        (COMP_CR_RIE)
#define COMP_IT_FALLING                       (COMP_CR_FIE)
#define COMP_IT_HIGH                          (COMP_CR_HIE)
#define COMP_IT_LOW                           (COMP_CR_LIE)
/** 
  * end of COMP_IT @}  
  */
  
  
/** @defgroup COMP_TriggerMode COMP TriggerMode
  * @ingroup  COMP_Parameter_Definitions 
  * @{
  */
#define COMP_IT_NONE                          (0)
#define COMP_IT_RISING                        (COMP_CR_RIE)
#define COMP_IT_FALLING                       (COMP_CR_FIE)
#define COMP_IT_HIGH                          (COMP_CR_HIE)
#define COMP_IT_LOW                           (COMP_CR_LIE)

#define COMP_EXTI_IT                          (0x00000001UL)
#define COMP_EXTI_EVENT                       (0x00000002UL)
#define COMP_EXTI_RISING                      (0x00000010UL)
#define COMP_EXTI_FALLING                     (0x00000020UL)

#define COMP_TRIGGERMODE_NONE                 (0x00000000UL)                                           /*!< Comparator output triggering no External Interrupt Line */
#define COMP_TRIGGERMODE_IT_RISING            (COMP_EXTI_IT | COMP_EXTI_RISING)                        /*!< Comparator output triggering External Interrupt Line event with interruption, on rising edge */
#define COMP_TRIGGERMODE_IT_FALLING           (COMP_EXTI_IT | COMP_EXTI_FALLING)                       /*!< Comparator output triggering External Interrupt Line event with interruption, on falling edge */
#define COMP_TRIGGERMODE_IT_RISING_FALLING    (COMP_EXTI_IT | COMP_EXTI_RISING | COMP_EXTI_FALLING)    /*!< Comparator output triggering External Interrupt Line event with interruption, on both rising and falling edges */
#define COMP_TRIGGERMODE_EVENT_RISING         (COMP_EXTI_EVENT | COMP_EXTI_RISING)                     /*!< Comparator output triggering External Interrupt Line event only (without interruption), on rising edge */
#define COMP_TRIGGERMODE_EVENT_FALLING        (COMP_EXTI_EVENT | COMP_EXTI_FALLING)                    /*!< Comparator output triggering External Interrupt Line event only (without interruption), on falling edge */
#define COMP_TRIGGERMODE_EVENT_RISING_FALLING (COMP_EXTI_EVENT | COMP_EXTI_RISING | COMP_EXTI_FALLING) /*!< Comparator output triggering External Interrupt Line event only (without interruption), on both rising and falling edges */
/** 
  * end of COMP_TriggerMode @}  
  */
  
/** @defgroup COMP_Output_Filter COMP Output Filter
  * @ingroup  COMP_Parameter_Definitions
  * @{
  */
#define COMP_CR_OFLT_DIV1                 ((uint32_t)0x00000000)
#define COMP_CR_OFLT_DIV16                (                                         COMP_CR_OFLT_BIT0 )
#define COMP_CR_OFLT_DIV32                (                     COMP_CR_OFLT_BIT1                     )
#define COMP_CR_OFLT_DIV64                (                     COMP_CR_OFLT_BIT1 | COMP_CR_OFLT_BIT0 )
#define COMP_CR_OFLT_DIV128               ( COMP_CR_OFLT_BIT2                                         )
#define COMP_CR_OFLT_DIV256               ( COMP_CR_OFLT_BIT2                     | COMP_CR_OFLT_BIT0 )
#define COMP_CR_OFLT_DIV512               ( COMP_CR_OFLT_BIT2 | COMP_CR_OFLT_BIT1                     )
#define COMP_CR_OFLT_DIV1024              ( COMP_CR_OFLT_BIT2 | COMP_CR_OFLT_BIT1 | COMP_CR_OFLT_BIT0 )
/** 
  * end of COMP_Output_Filter @}  
  */

/** @defgroup COMP_EXTI_LINE COMP EXTI LINE
  * @ingroup  COMP_Parameter_Definitions
  * @{
  */
#define COMP_EXTI_LINE_COMP1              (1U << (EXTI_LINE_21 & 0xFF))
#define COMP_EXTI_LINE_COMP2              (1U << (EXTI_LINE_22 & 0xFF))
#define COMP_EXTI_LINE_COMP3              (1U << (EXTI_LINE_29 & 0xFF))
#define COMP_EXTI_LINE_COMP4              (1U << (EXTI_LINE_30 & 0xFF))
/** 
  * end of COMP_EXTI_LINE @}  
  */  
/** 
  * end of COMP_Parameter_Definitions @}  
  */


/******************************************************************************/
/*                                 COMP Macro                                 */
/******************************************************************************/
/** @defgroup COMP_Macro_Definitions COMP Macro Definitions
  * @{
  */
 
/** @brief  Reset COMP handle state.
  * @param  __HANDLE__  COMP handle
  * @retval None
  */
#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
#define __HAL_COMP_RESET_HANDLE_STATE(__HANDLE__) do{                                                  \
                                                      (__HANDLE__)->State = HAL_COMP_STATE_RESET;      \
                                                      (__HANDLE__)->MspInitCallback = NULL;            \
                                                      (__HANDLE__)->MspDeInitCallback = NULL;          \
                                                    } while(0)
#else
#define __HAL_COMP_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_COMP_STATE_RESET)
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */

/**
  * @brief Clear COMP error code (set it to no error code "HAL_COMP_ERROR_NONE").
  * @param __HANDLE__ COMP handle
  * @retval None
  */
#define COMP_CLEAR_ERRORCODE(__HANDLE__) ((__HANDLE__)->ErrorCode = HAL_COMP_ERROR_NONE)

/**
  * @brief  Enable the specified comparator.
  * @param  __HANDLE__  COMP handle
  * @retval None
  */
#define __HAL_COMP_ENABLE(__HANDLE__)       (SET_BIT((__HANDLE__)->Instance->CR, COMP_CR_EN))

/**
  * @brief  Disable the specified comparator.
  * @param  __HANDLE__  COMP handle
  * @retval None
  */
#define __HAL_COMP_DISABLE(__HANDLE__)      (CLEAR_BIT((__HANDLE__)->Instance->CR, COMP_CR_EN))

/**
  * @brief    Enable Interrupt of the selected CMP
  * @param    COMPx:
  *           @arg CMP Instance
  * @param    Flag:
  *           @arg  COMP_CR_HIE
  *           @arg  COMP_CR_FIE
  *           @arg  COMP_CR_LIE
  *           @arg  COMP_CR_RIE
  * @retval State of bit (1 or 0).
  */
#define __HAL_COMP_ENABLE_IT(__HANDLE__, __IT__)      (SET_BIT((__HANDLE__)->Instance->CR, __IT__))

/**
  * @brief    Disable Interrupt of the selected CMP
  * @param    COMPx:
  *           @arg CMP Instance
  * @param    Flag:
  *           @arg  COMP_CR_HIE
  *           @arg  COMP_CR_FIE
  *           @arg  COMP_CR_LIE
  *           @arg  COMP_CR_RIE
  * @retval State of bit (1 or 0).
  */
#define __HAL_COMP_DISABLE_IT(__HANDLE__, __IT__)      (CLEAR_BIT((__HANDLE__)->Instance->CR, __IT__))

/**
  * @brief  Disable the CMP BAS
  * @param  None
  * @retval None
  */
#define __HAL_COMP_DISABLE_BAS(__HANDLE__)             (CLEAR_BIT((__HANDLE__)->Instance->CR, COMP_CR_BASEN))

/**
  * @brief  Enable the CMP BAS
  * @param  COMPx:
  *         @arg CMP Instance
  * @param  Continuous:
  *         @arg CMP_CxCR_BASEN
  *         @arg CMP_CxCR_CMP_BAS_EN_OPA
  * @retval None
  */
#define __HAL_COMP_ENABLE_BAS(__HANDLE__)              (SET_BIT((__HANDLE__)->Instance->CR, COMP_CR_BASEN)) 
  
/**
  * @brief  Enable PEN of the selected CMP
  * @param  COMPx:
  *          @arg CMP Instance
  * @retval None
  */
#define __HAL_COMP_ENABLE_CALP(__HANDLE__)             (SET_BIT((__HANDLE__)->Instance->CAL, COMP_CAL_PEN))

/**
  * @brief  Disable PEN of the selected CMP
  * @param  COMPx:
  *         @arg CMP Instance 
  * @retval None
  */
#define __HAL_COMP_DISABLE_CALP(__HANDLE__)            (CLEAR_BIT((__HANDLE__)->Instance->CAL, COMP_CAL_PEN))

/**
  * @brief  Enable NEN of the selected CMP
  * @param  COMPx:
  *         @arg CMP Instance
  * @retval None
  */
#define __HAL_COMP_ENABLE_CALN(__HANDLE__)             (SET_BIT((__HANDLE__)->Instance->CAL, COMP_CAL_NEN))    

/**
  * @brief  Disable NEN of the selected CMP
  * @param  COMPx:
  *         @arg CMP Instance 
  * @retval None
  */
#define __HAL_COMP_DISABLE_CALN(__HANDLE__)            (CLEAR_BIT((__HANDLE__)->Instance->CAL, COMP_CAL_NEN)) 

/**
  * @brief  Lock the specified comparator configuration.
  * @note   Using this macro induce HAL COMP handle state machine being no
  *         more in line with COMP instance state.
  *         To keep HAL COMP handle state machine updated, it is recommended
  *         to use function "HAL_COMP_Lock')".
  * @param  __HANDLE__  COMP handle
  * @retval None
  */
#define __HAL_COMP_LOCK(__HANDLE__)         (SET_BIT((__HANDLE__)->Instance->CR, COMP_CR_LOCK))

/**
  * @brief  Check whether the specified comparator is locked.
  * @param  __HANDLE__  COMP handle
  * @retval Value 0 if COMP instance is not locked, value 1 if COMP instance is locked
  */
#define __HAL_COMP_IS_LOCKED(__HANDLE__)    (READ_BIT((__HANDLE__)->Instance->CR, COMP_CR_LOCK) == COMP_CR_LOCK)

/**
  * @brief  Enable the COMP1 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_ENABLE_RISING_EDGE()    (SET_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP1))

/**
  * @brief  Disable the COMP1 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_DISABLE_RISING_EDGE()   (CLEAR_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP1))

/**
  * @brief  Enable the COMP1 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_ENABLE_FALLING_EDGE()   (SET_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP1))

/**
  * @brief  Disable the COMP1 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_DISABLE_FALLING_EDGE()  (CLEAR_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP1))

/**
  * @brief  Enable the COMP1 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_ENABLE_RISING_FALLING_EDGE() do { \
                                                                SET_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP1); \
                                                                SET_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP1);\
                                                              } while(0)

/**
  * @brief  Disable the COMP1 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_DISABLE_RISING_FALLING_EDGE() do { \
                                                                 CLEAR_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP1); \
                                                                 CLEAR_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP1);\
                                                               } while(0)

/**
  * @brief  Enable the COMP1 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_ENABLE_IT()             (SET_BIT(EXTI->IMR, COMP_EXTI_LINE_COMP1))

/**
  * @brief  Disable the COMP1 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_DISABLE_IT()            (CLEAR_BIT(EXTI->IMR, COMP_EXTI_LINE_COMP1))

/**
  * @brief  Generate a software interrupt on the COMP1 EXTI line.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_GENERATE_SWIT()         (SET_BIT(EXTI->SWIER, COMP_EXTI_LINE_COMP1))

/**
  * @brief  Enable the COMP1 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_ENABLE_EVENT()          (SET_BIT(EXTI->EMR, COMP_EXTI_LINE_COMP1))

/**
  * @brief  Disable the COMP1 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_DISABLE_EVENT()         (CLEAR_BIT(EXTI->EMR, COMP_EXTI_LINE_COMP1))

/**
  * @brief  Check whether the COMP1 EXTI line flag is set.
  * @retval RESET or SET
  */
#define __HAL_COMP_COMP1_EXTI_GET_FLAG()              (((EXTI->PR & COMP_EXTI_LINE_COMP1) == COMP_EXTI_LINE_COMP1) ? SET : RESET)

/**
  * @brief  Clear the COMP1 EXTI flag.
  * @retval None
  */
#define __HAL_COMP_COMP1_EXTI_CLEAR_FLAG()            CLEAR_BIT(EXTI->PR, COMP_EXTI_LINE_COMP1);

/**
  * @brief  Enable the COMP2 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_ENABLE_RISING_EDGE()    (SET_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP2))

/**
  * @brief  Disable the COMP2 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_DISABLE_RISING_EDGE()   (CLEAR_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP2))

/**
  * @brief  Enable the COMP2 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_ENABLE_FALLING_EDGE()   (SET_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP2))

/**
  * @brief  Disable the COMP2 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_DISABLE_FALLING_EDGE()  (CLEAR_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP2))

/**
  * @brief  Enable the COMP2 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_ENABLE_RISING_FALLING_EDGE() do { \
                                                                SET_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP2); \
                                                                SET_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP2);\
                                                              } while(0)

/**
  * @brief  Disable the COMP2 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_DISABLE_RISING_FALLING_EDGE() do { \
                                                                 CLEAR_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP2); \
                                                                 CLEAR_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP2);\
                                                               } while(0)

/**
  * @brief  Enable the COMP2 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_ENABLE_IT()             (SET_BIT(EXTI->IMR, COMP_EXTI_LINE_COMP2))

/**
  * @brief  Disable the COMP2 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_DISABLE_IT()            (CLEAR_BIT(EXTI->IMR, COMP_EXTI_LINE_COMP2))

/**
  * @brief  Generate a software interrupt on the COMP2 EXTI line.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_GENERATE_SWIT()         (SET_BIT(EXTI->SWIER, COMP_EXTI_LINE_COMP2))

/**
  * @brief  Enable the COMP2 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_ENABLE_EVENT()          (SET_BIT(EXTI->EMR, COMP_EXTI_LINE_COMP2))

/**
  * @brief  Disable the COMP2 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_DISABLE_EVENT()         (CLEAR_BIT(EXTI->EMR, COMP_EXTI_LINE_COMP2))

/**
  * @brief  Check whether the COMP2 EXTI line flag is set.
  * @retval RESET or SET
  */
#define __HAL_COMP_COMP2_EXTI_GET_FLAG()              (((EXTI->PR & COMP_EXTI_LINE_COMP2) == COMP_EXTI_LINE_COMP2) ? SET : RESET)

/**
  * @brief  Clear the COMP2 EXTI flag.
  * @retval None
  */
#define __HAL_COMP_COMP2_EXTI_CLEAR_FLAG()            CLEAR_BIT(EXTI->PR, COMP_EXTI_LINE_COMP2);


/**
  * @brief  Enable the COMP3 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_ENABLE_RISING_EDGE()    (SET_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP3))

/**
  * @brief  Disable the COMP3 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_DISABLE_RISING_EDGE()   (CLEAR_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP3))

/**
  * @brief  Enable the COMP3 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_ENABLE_FALLING_EDGE()   (SET_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP3))

/**
  * @brief  Disable the COMP3 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_DISABLE_FALLING_EDGE()  (CLEAR_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP3))

/**
  * @brief  Enable the COMP3 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_ENABLE_RISING_FALLING_EDGE() do { \
                                                                SET_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP3); \
                                                                SET_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP3);\
                                                              } while(0)

/**
  * @brief  Disable the COMP3 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_DISABLE_RISING_FALLING_EDGE() do { \
                                                                 CLEAR_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP3); \
                                                                 CLEAR_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP3);\
                                                               } while(0)

/**
  * @brief  Enable the COMP3 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_ENABLE_IT()             (SET_BIT(EXTI->IMR, COMP_EXTI_LINE_COMP3))

/**
  * @brief  Disable the COMP3 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_DISABLE_IT()            (CLEAR_BIT(EXTI->IMR, COMP_EXTI_LINE_COMP3))

/**
  * @brief  Generate a software interrupt on the COMP3 EXTI line.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_GENERATE_SWIT()         (SET_BIT(EXTI->SWIER, COMP_EXTI_LINE_COMP3))

/**
  * @brief  Enable the COMP3 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_ENABLE_EVENT()          (SET_BIT(EXTI->EMR, COMP_EXTI_LINE_COMP3))

/**
  * @brief  Disable the COMP3 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_DISABLE_EVENT()         (CLEAR_BIT(EXTI->EMR, COMP_EXTI_LINE_COMP3))

/**
  * @brief  Check whether the COMP3 EXTI line flag is set.
  * @retval RESET or SET
  */
#define __HAL_COMP_COMP3_EXTI_GET_FLAG()              (((EXTI->PR & COMP_EXTI_LINE_COMP3) == COMP_EXTI_LINE_COMP3) ? SET : RESET)

/**
  * @brief  Clear the COMP3 EXTI flag.
  * @retval None
  */
#define __HAL_COMP_COMP3_EXTI_CLEAR_FLAG()            CLEAR_BIT(EXTI->PR, COMP_EXTI_LINE_COMP3);


/**
  * @brief  Enable the COMP4 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP4_EXTI_ENABLE_RISING_EDGE()    (SET_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP4))

/**
  * @brief  Disable the COMP4 EXTI line rising edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP4_EXTI_DISABLE_RISING_EDGE()   (CLEAR_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP4))

/**
  * @brief  Enable the COMP4 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP4_EXTI_ENABLE_FALLING_EDGE()   (SET_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP4))

/**
  * @brief  Disable the COMP4 EXTI line falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP4_EXTI_DISABLE_FALLING_EDGE()  (CLEAR_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP4))

/**
  * @brief  Enable the COMP4 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP4_EXTI_ENABLE_RISING_FALLING_EDGE() do { \
                                                                SET_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP4); \
                                                                SET_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP4); \
                                                              } while(0)

/**
  * @brief  Disable the COMP4 EXTI line rising & falling edge trigger.
  * @retval None
  */
#define __HAL_COMP_COMP4_EXTI_DISABLE_RISING_FALLING_EDGE() do { \
                                                                 CLEAR_BIT(EXTI->RTSR, COMP_EXTI_LINE_COMP4); \
                                                                 CLEAR_BIT(EXTI->FTSR, COMP_EXTI_LINE_COMP4); \
                                                               } while(0)

/**
  * @brief  Enable the COMP4 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP4_EXTI_ENABLE_IT()             (SET_BIT(EXTI->IMR, COMP_EXTI_LINE_COMP4))

/**
  * @brief  Disable the COMP4 EXTI line in interrupt mode.
  * @retval None
  */
#define __HAL_COMP_COMP4_EXTI_DISABLE_IT()            (CLEAR_BIT(EXTI->IMR, COMP_EXTI_LINE_COMP4))

/**
  * @brief  Generate a software interrupt on the COMP4 EXTI line.
  * @retval None
  */
#define __HAL_COMP_COMP4_EXTI_GENERATE_SWIT()         (SET_BIT(EXTI->SWIER, COMP_EXTI_LINE_COMP4))

/**
  * @brief  Enable the COMP4 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP4_EXTI_ENABLE_EVENT()          (SET_BIT(EXTI->EMR, COMP_EXTI_LINE_COMP4))

/**
  * @brief  Disable the COMP4 EXTI line in event mode.
  * @retval None
  */
#define __HAL_COMP_COMP4_EXTI_DISABLE_EVENT()         (CLEAR_BIT(EXTI->EMR, COMP_EXTI_LINE_COMP4))

/**
  * @brief  Check whether the COMP4 EXTI line flag is set.
  * @retval RESET or SET
  */
#define __HAL_COMP_COMP4_EXTI_GET_FLAG()              (((EXTI->PR & COMP_EXTI_LINE_COMP4) == COMP_EXTI_LINE_COMP4) ? SET : RESET)

/**
  * @brief  Clear the COMP4 EXTI flag.
  * @retval None
  */
#define __HAL_COMP_COMP4_EXTI_CLEAR_FLAG()            CLEAR_BIT(EXTI->PR, COMP_EXTI_LINE_COMP4);


/**
  * @brief  Enable the COMP EXTI line rising edge trigger.
  * @param  __INSTANCE__: COMP instance
  * @retval None
  */
#define __HAL_COMP_EXTI_ENABLE_RISING_EDGE(__INSTANCE__)    (((__INSTANCE__) == COMP1) ? __HAL_COMP_COMP1_EXTI_ENABLE_RISING_EDGE()   \
                                                            :((__INSTANCE__) == COMP2) ? __HAL_COMP_COMP2_EXTI_ENABLE_RISING_EDGE()   \
                                                            :((__INSTANCE__) == COMP3) ? __HAL_COMP_COMP3_EXTI_ENABLE_RISING_EDGE()   \
                                                            : __HAL_COMP_COMP4_EXTI_ENABLE_RISING_EDGE())
/**
  * @brief  Disable the COMP EXTI line rising edge trigger.
  * @param  __INSTANCE__: COMP instance
  * @retval None
  */
#define __HAL_COMP_EXTI_DISABLE_RISING_EDGE(__INSTANCE__)   (((__INSTANCE__) == COMP1) ? __HAL_COMP_COMP1_EXTI_DISABLE_RISING_EDGE()   \
                                                            :((__INSTANCE__) == COMP2) ? __HAL_COMP_COMP2_EXTI_DISABLE_RISING_EDGE()   \
                                                            :((__INSTANCE__) == COMP3) ? __HAL_COMP_COMP3_EXTI_DISABLE_RISING_EDGE()   \
                                                            : __HAL_COMP_COMP4_EXTI_DISABLE_RISING_EDGE())

/**
  * @brief  Enable the COMP EXTI line falling edge trigger.
  * @param  __INSTANCE__: COMP instance
  * @retval None
  */
#define __HAL_COMP_EXTI_ENABLE_FALLING_EDGE(__INSTANCE__)   (((__INSTANCE__) == COMP1) ? __HAL_COMP_COMP1_EXTI_ENABLE_FALLING_EDGE()   \
                                                            :((__INSTANCE__) == COMP2) ? __HAL_COMP_COMP2_EXTI_ENABLE_FALLING_EDGE()   \
                                                            :((__INSTANCE__) == COMP3) ? __HAL_COMP_COMP3_EXTI_ENABLE_FALLING_EDGE()   \
                                                            : __HAL_COMP_COMP4_EXTI_ENABLE_FALLING_EDGE())

/**
  * @brief  Disable the COMP EXTI line falling edge trigger.
  * @param  __INSTANCE__: COMP instance
  * @retval None
  */
#define __HAL_COMP_EXTI_DISABLE_FALLING_EDGE(__INSTANCE__)  (((__INSTANCE__) == COMP1) ? __HAL_COMP_COMP1_EXTI_DISABLE_FALLING_EDGE()   \
                                                            :((__INSTANCE__) == COMP2) ? __HAL_COMP_COMP2_EXTI_DISABLE_FALLING_EDGE()   \
                                                            :((__INSTANCE__) == COMP3) ? __HAL_COMP_COMP3_EXTI_DISABLE_FALLING_EDGE()   \
                                                            : __HAL_COMP_COMP4_EXTI_DISABLE_FALLING_EDGE())
/**
  * @brief  Enable the EXTI line in interrupt mode.
  * @param  __INSTANCE__: COMP instance
  * @retval None
  */
#define __HAL_COMP_EXTI_ENABLE_IT(__INSTANCE__)             (((__INSTANCE__) == COMP1) ? __HAL_COMP_COMP1_EXTI_ENABLE_IT()   \
                                                            :((__INSTANCE__) == COMP2) ? __HAL_COMP_COMP2_EXTI_ENABLE_IT()   \
                                                            :((__INSTANCE__) == COMP3) ? __HAL_COMP_COMP3_EXTI_ENABLE_IT()   \
                                                            : __HAL_COMP_COMP4_EXTI_ENABLE_IT())
/**
  * @brief  Disable the EXTI line in interrupt mode.
  * @param  __INSTANCE__: COMP instance
  * @retval None
  */
#define __HAL_COMP_EXTI_DISABLE_IT(__INSTANCE__)            (((__INSTANCE__) == COMP1) ? __HAL_COMP_COMP1_EXTI_DISABLE_IT()   \
                                                            :((__INSTANCE__) == COMP2) ? __HAL_COMP_COMP2_EXTI_DISABLE_IT()   \
                                                            :((__INSTANCE__) == COMP3) ? __HAL_COMP_COMP3_EXTI_DISABLE_IT()   \
                                                            : __HAL_COMP_COMP4_EXTI_DISABLE_IT())

/**
  * @brief  Generate a software interrupt on the COMP EXTI line.
  * @param  __INSTANCE__: COMP instance
  * @retval None
  */
#define __HAL_COMP_EXTI_GENERATE_SWIT(__INSTANCE__)          (((__INSTANCE__) == COMP1) ? __HAL_COMP_COMP1_EXTI_GENERATE_SWIT()   \
                                                             :((__INSTANCE__) == COMP2) ? __HAL_COMP_COMP2_EXTI_GENERATE_SWIT()   \
                                                             :((__INSTANCE__) == COMP3) ? __HAL_COMP_COMP3_EXTI_GENERATE_SWIT()   \
                                                             : __HAL_COMP_COMP4_EXTI_GENERATE_SWIT())

/**
  * @brief  Enable the COMP EXTI line in event mode.
  * @param  __INSTANCE__: COMP instance
  * @retval None
  */
#define __HAL_COMP_EXTI_ENABLE_EVENT(__INSTANCE__)           (((__INSTANCE__) == COMP1) ? __HAL_COMP_COMP1_EXTI_ENABLE_EVENT()   \
                                                             :((__INSTANCE__) == COMP2) ? __HAL_COMP_COMP2_EXTI_ENABLE_EVENT()   \
                                                             :((__INSTANCE__) == COMP3) ? __HAL_COMP_COMP3_EXTI_ENABLE_EVENT()   \
                                                             : __HAL_COMP_COMP4_EXTI_ENABLE_EVENT())

/**
  * @brief  Disable the COMP4 EXTI line in event mode.
  * @param  __INSTANCE__: COMP instance
  * @retval None
  */
#define __HAL_COMP_EXTI_DISABLE_EVENT(__INSTANCE__)          (((__INSTANCE__) == COMP1) ? __HAL_COMP_COMP1_EXTI_DISABLE_EVENT()   \
                                                             :((__INSTANCE__) == COMP2) ? __HAL_COMP_COMP2_EXTI_DISABLE_EVENT()   \
                                                             :((__INSTANCE__) == COMP3) ? __HAL_COMP_COMP3_EXTI_DISABLE_EVENT()   \
                                                             : __HAL_COMP_COMP4_EXTI_DISABLE_EVENT())
/**
  * @brief  Get the specified EXTI line for a comparator instance.
  * @param  __INSTANCE__  specifies the COMP instance.
  * @retval value of @ref COMP_EXTI_LINE
  */
#define COMP_GET_EXTI_LINE(__INSTANCE__)    (((__INSTANCE__) == COMP1) ? COMP_EXTI_LINE_COMP1  \
                                             :((__INSTANCE__) == COMP2) ? COMP_EXTI_LINE_COMP2 \
                                             :((__INSTANCE__) == COMP3) ? COMP_EXTI_LINE_COMP3 \
                                             : COMP_EXTI_LINE_COMP4 )

/**
  * @brief  Check if the parameter __INPUT_PLUS__ is valid
  * @param  __INPUT_PLUS__ 
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_COMP_INPUT_PLUS(__INPUT_PLUS__)                      (((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO1) || \
                                                                 ((__INPUT_PLUS__) == COMP_INPUT_PLUS_IO2))
/**
  * @brief  Check if the parameter __INSTANCE__, __INPUT_MINUS__ is valid
  * @param  __INSTANCE__ 
  * @param  __INPUT_MINUS__ 
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_COMP_INPUT_MINUS(__COMP_INSTANCE__, __INPUT_MINUS__)                    (((__INPUT_MINUS__) == COMP_INPUT_MINUS_1_4VREFINT)  ||\
                                                                                    ((__INPUT_MINUS__) == COMP_INPUT_MINUS_1_2VREFINT)  ||\
                                                                                    ((__INPUT_MINUS__) == COMP_INPUT_MINUS_3_4VREFINT)  ||\
                                                                                    ((__INPUT_MINUS__) == COMP_INPUT_MINUS_VREFINT)     ||\
                                                                                    ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO1)         ||\
                                                                                    ((__INPUT_MINUS__) == COMP_INPUT_MINUS_IO2)         ||\
                                                                                    (((__COMP_INSTANCE__) == COMP1)                        && \
                                                                                     (((__INPUT_MINUS__) == COMP_INPUT_MINUS_DAC1_CH1)  ||\
                                                                                      ((__INPUT_MINUS__) == COMP_INPUT_MINUS_DAC3_CH1))   \
                                                                                    )                                                      || \
                                                                                    (((__COMP_INSTANCE__) == COMP2)                        && \
                                                                                     (((__INPUT_MINUS__) == COMP_INPUT_MINUS_DAC1_CH2)  ||\
                                                                                      ((__INPUT_MINUS__) == COMP_INPUT_MINUS_DAC3_CH2))   \
                                                                                    )                                                      || \
                                                                                    (((__COMP_INSTANCE__) == COMP3)                        && \
                                                                                     (((__INPUT_MINUS__) == COMP_INPUT_MINUS_DAC2_CH1)  ||\
                                                                                      ((__INPUT_MINUS__) == COMP_INPUT_MINUS_DAC3_CH1))   \
                                                                                    )                                                      || \
                                                                                    (((__COMP_INSTANCE__) == COMP4)                        && \
                                                                                     (((__INPUT_MINUS__) == COMP_INPUT_MINUS_DAC2_CH2)  ||\
                                                                                      ((__INPUT_MINUS__) == COMP_INPUT_MINUS_DAC3_CH2))   \
                                                                                    ))
/**
  * @brief  Check if the parameter __HYSTERESIS__ is valid
  * @param  __HYSTERESIS__ 
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_COMP_Rising_HYSTERESIS(__HYSTERESIS__)  (((__HYSTERESIS__) == COMP_CR_RHYST_0mV)   || \
                                             ((__HYSTERESIS__) == COMP_CR_RHYST_20mV)   || \
                                             ((__HYSTERESIS__) == COMP_CR_RHYST_40mV)   || \
                                             ((__HYSTERESIS__) == COMP_CR_RHYST_60mV))
                                             
#define IS_COMP_Falling_HYSTERESIS(__HYSTERESIS__)  (((__HYSTERESIS__) == COMP_CR_FHYST_0mV)   || \
                                             ((__HYSTERESIS__) == COMP_CR_FHYST_20mV)   || \
                                             ((__HYSTERESIS__) == COMP_CR_FHYST_40mV)   || \
                                             ((__HYSTERESIS__) == COMP_CR_FHYST_60mV))          


/**
  * @brief  Check if the parameter __POL__ is valid
  * @param  __POL__ 
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_COMP_OUTPUTPOL(__POL__)          (((__POL__) == COMP_OUTPUTPOL_NONINVERTED) || \
                                             ((__POL__) == COMP_OUTPUTPOL_INVERTED))
/**
  * @brief  Check if the parameter __INSTANCE__, __OUTPUT_BLANKING_SOURCE__ is valid
  * @param  __INSTANCE__ 
  * @param  __OUTPUT_BLANKING_SOURCE__ 
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_COMP_BLANKINGSRC_INSTANCE(__OUTPUT_BLANKING_SOURCE__)  \
    (((__OUTPUT_BLANKING_SOURCE__) == COMP_CR_BLANKING_SEL_NoBlanking )  ||      \
     ((__OUTPUT_BLANKING_SOURCE__) == COMP_CR_BLANKING_SEL_TIM1_OC5   )  ||      \
     ((__OUTPUT_BLANKING_SOURCE__) == COMP_CR_BLANKING_SEL_TIM2_OC3   )  ||      \
     ((__OUTPUT_BLANKING_SOURCE__) == COMP_CR_BLANKING_SEL_TIM3_OC3   )  ||      \
     ((__OUTPUT_BLANKING_SOURCE__) == COMP_CR_BLANKING_SEL_TIM8_OC5   )  ||      \
     ((__OUTPUT_BLANKING_SOURCE__) == COMP_CR_BLANKING_SEL_TIM5_OC3   )  ||      \
     ((__OUTPUT_BLANKING_SOURCE__) == COMP_CR_BLANKING_SEL_TIM15_OC1  )  ||      \
     ((__OUTPUT_BLANKING_SOURCE__) == COMP_CR_BLANKING_SEL_TIM4_OC3   )) 

/**
  * @brief  Check if the parameter __MODE__ is valid
  * @param  __MODE__ 
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_COMP_TRIGGERMODE(__MODE__)       (((__MODE__) == COMP_TRIGGERMODE_NONE)                 || \
                                             ((__MODE__) == COMP_TRIGGERMODE_IT_RISING)            || \
                                             ((__MODE__) == COMP_TRIGGERMODE_IT_FALLING)           || \
                                             ((__MODE__) == COMP_TRIGGERMODE_IT_RISING_FALLING)    || \
                                             ((__MODE__) == COMP_TRIGGERMODE_EVENT_RISING)         || \
                                             ((__MODE__) == COMP_TRIGGERMODE_EVENT_FALLING)        || \
                                             ((__MODE__) == COMP_TRIGGERMODE_EVENT_RISING_FALLING))   
                                             
/**
  * @brief  Check if the parameter __MODE__ is valid
  * @param  __MODE__ 
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_COMP_IT(__MODE__)        (((__MODE__) == COMP_IT_NONE   )    || \
                                    ((__MODE__) == COMP_IT_RISING )    || \
                                    ((__MODE__) == COMP_IT_FALLING)    || \
                                    ((__MODE__) == COMP_IT_HIGH   )    || \
                                    ((__MODE__) == COMP_IT_LOW    ))                                         

/**
  * @brief  Check if the parameter __OUTPUT_LEVEL__ is valid
  * @param  __OUTPUT_LEVEL__ 
  * @retval Result
  *         @arg SET (valid)
  *         @arg RESET (invalid)
  */
#define IS_COMP_OUTPUT_LEVEL(__OUTPUT_LEVEL__) (((__OUTPUT_LEVEL__) == COMP_OUTPUT_LEVEL_LOW)     || \
                                                ((__OUTPUT_LEVEL__) == COMP_OUTPUT_LEVEL_HIGH))

/** 
  * end of COMP_Macro_Definitions @}  
  */
  
/******************************************************************************/
/*                                COMP Functions                              */
/******************************************************************************/

/** @defgroup COMP_Function_Definitions COMP Function Definitions
  * @{
  */

/* Initialization and de-initialization functions  **********************************/
HAL_StatusTypeDef       HAL_COMP_Init(COMP_HandleTypeDef *hcomp);
HAL_StatusTypeDef       HAL_COMP_DeInit(COMP_HandleTypeDef *hcomp);
void                    HAL_COMP_MspInit(COMP_HandleTypeDef *hcomp);
void                    HAL_COMP_MspDeInit(COMP_HandleTypeDef *hcomp);

#if (USE_HAL_COMP_REGISTER_CALLBACKS == 1)
/* Callbacks Register/UnRegister functions  ***********************************/
HAL_StatusTypeDef       HAL_COMP_RegisterCallback(COMP_HandleTypeDef *hcomp, HAL_COMP_CallbackIDTypeDef CallbackID,
                                            pCOMP_CallbackTypeDef pCallback);
HAL_StatusTypeDef       HAL_COMP_UnRegisterCallback(COMP_HandleTypeDef *hcomp, HAL_COMP_CallbackIDTypeDef CallbackID);
#endif /* USE_HAL_COMP_REGISTER_CALLBACKS */

HAL_StatusTypeDef       HAL_COMP_Start(COMP_HandleTypeDef *hcomp);
HAL_StatusTypeDef       HAL_COMP_Stop(COMP_HandleTypeDef *hcomp);

HAL_StatusTypeDef       HAL_COMP_Start_IT(COMP_HandleTypeDef *hcomp, uint32_t Interrupt);
HAL_StatusTypeDef       HAL_COMP_Stop_IT(COMP_HandleTypeDef *hcomp, uint32_t Interrupt);
void                    HAL_COMP_IRQHandler(COMP_HandleTypeDef *hcomp);

HAL_StatusTypeDef       HAL_COMP_Lock(COMP_HandleTypeDef *hcomp);
uint32_t                HAL_COMP_GetOutputLevel(const COMP_HandleTypeDef *hcomp);
/* Callback in interrupt mode */
void                    HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp);

HAL_COMP_StateTypeDef   HAL_COMP_GetState(const COMP_HandleTypeDef *hcomp);
uint32_t                HAL_COMP_GetError(const COMP_HandleTypeDef *hcomp);
  
/** 
  * end of COMP_Function_Definitions @}  
  */
/** 
  * end of COMP @}  
  */


#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_COMP_H_ */

