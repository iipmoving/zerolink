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
*   File    : rx32g4xx_hal_rtc.h
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RX32G4xx_HAL_RTC_H__
#define __RX32G4xx_HAL_RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"

/** @addtogroup RTC
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup RTC_Structure_Definitions RTC Structure Definitions
  * @{
  */

/** @defgroup RTC_TimeTypeDef RTC TimeTypeDef
  * @ingroup  RTC_Structure_Definitions
  * @{
  */

/**
  * @brief  RTC Time structure definition
  */
typedef struct
{
  uint8_t Hours;            /*!< Specifies the RTC Time Hour.
                                 This parameter must be a number between Min_Data = 0 and Max_Data = 23 */

  uint8_t Minutes;          /*!< Specifies the RTC Time Minutes.
                                 This parameter must be a number between Min_Data = 0 and Max_Data = 59 */

  uint8_t Seconds;          /*!< Specifies the RTC Time Seconds.
                                 This parameter must be a number between Min_Data = 0 and Max_Data = 59 */

} RTC_TimeTypeDef;

/**
  * end of RTC_TimeTypeDef @}
  */

/** @defgroup RTC_AlarmTypeDef RTC AlarmTypeDef
  * @ingroup  RTC_Structure_Definitions
  * @{
  */

/**
  * @brief  RTC Alarm structure definition
  */
typedef struct
{
  RTC_TimeTypeDef AlarmTime;     /*!< Specifies the RTC Alarm Time members */

  uint32_t Alarm;                /*!< Specifies the alarm ID
                                      This parameter can be a value of @ref RTC_Alarms_Definitions */
} RTC_AlarmTypeDef;

/**
  * end of RTC_AlarmTypeDef @}
  */

/** @defgroup HAL_RTCStateTypeDef HAL RTCStateTypeDef
  * @ingroup  RTC_Structure_Definitions
  * @{
  */

/**
  * @brief  HAL State structures definition
  */

typedef enum
{
  HAL_RTC_STATE_RESET             = 0x00U,  /*!< RTC not yet initialized or disabled */
  HAL_RTC_STATE_READY             = 0x01U,  /*!< RTC initialized and ready for use   */
  HAL_RTC_STATE_BUSY              = 0x02U,  /*!< RTC process is ongoing              */
  HAL_RTC_STATE_TIMEOUT           = 0x03U,  /*!< RTC timeout state                   */
  HAL_RTC_STATE_ERROR             = 0x04U   /*!< RTC error state                     */

} HAL_RTCStateTypeDef;

/**
  * end of HAL_RTCStateTypeDef @}
  */

/** @defgroup RTC_InitTypeDef RTC InitTypeDef
  * @ingroup  RTC_Structure_Definitions
  * @{
  */

/**
  * @brief  RTC Configuration Structure definition
  */

typedef struct
{
  uint32_t AsynchPrediv;    /*!< Specifies the RTC Asynchronous Predivider value.
                                 This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFFFFF  or RTC_AUTO_1_SECOND
                                 If RTC_AUTO_1_SECOND is selected, AsynchPrediv will be set automatically to get 1sec timebase */

  uint32_t OutPut;          /*!< Specifies which signal will be routed to the RTC Tamper pin.
                                 This parameter can be a value of @ref RTC_output_source_to_output_on_the_Tamper_pin */

}RTC_InitTypeDef;

/**
  * end of RTC_InitTypeDef @}
  */

/** @defgroup RTC_DateTypeDef RTC DateTypeDef
  * @ingroup  RTC_Structure_Definitions
  * @{
  */

/**
  * @brief  RTC Date structure definition
  */
typedef struct
{
  uint8_t WeekDay;  /*!< Specifies the RTC Date WeekDay (not necessary for HAL_RTC_SetDate).
                         This parameter can be a value of @ref RTC_WeekDay_Definitions */

  uint8_t Month;    /*!< Specifies the RTC Date Month (in BCD format).
                         This parameter can be a value of @ref RTC_Month_Date_Definitions */

  uint8_t Date;     /*!< Specifies the RTC Date.
                         This parameter must be a number between Min_Data = 1 and Max_Data = 31 */

  uint8_t Year;     /*!< Specifies the RTC Date Year.
                         This parameter must be a number between Min_Data = 0 and Max_Data = 99 */

}RTC_DateTypeDef;

/**
  * end of RTC_DateTypeDef @}
  */

/** @defgroup RTC_HandleTypeDef RTC HandleTypeDef
  * @ingroup  RTC_Structure_Definitions
  * @{
  */

/**
  * @brief  Time Handle Structure definition
  */
#if (USE_HAL_RTC_REGISTER_CALLBACKS == 1)
typedef struct __RTC_HandleTypeDef
#else
typedef struct
#endif /* (USE_HAL_RTC_REGISTER_CALLBACKS) */
{
  RTC_TypeDef                 *Instance;          /*!< Register base address    */

  RTC_InitTypeDef             Init;               /*!< RTC required parameters  */

  RTC_DateTypeDef             DateToUpdate;       /*!< Current date set by user and updated automatically  */

  HAL_LockTypeDef             Lock;               /*!< RTC locking object       */

  __IO HAL_RTCStateTypeDef    State;              /*!< Time communication state */

#if (USE_HAL_RTC_REGISTER_CALLBACKS == 1)
  void (* AlarmAEventCallback)(struct __RTC_HandleTypeDef *hrtc);           /*!< RTC Alarm A Event callback         */

  void (* Tamper1EventCallback)(struct __RTC_HandleTypeDef *hrtc);          /*!< RTC Tamper 1 Event callback        */

  void (* MspInitCallback)(struct __RTC_HandleTypeDef *hrtc);               /*!< RTC Msp Init callback              */

  void (* MspDeInitCallback)(struct __RTC_HandleTypeDef *hrtc);             /*!< RTC Msp DeInit callback            */

#endif /* (USE_HAL_RTC_REGISTER_CALLBACKS) */

} RTC_HandleTypeDef;

/**
  * end of RTC_HandleTypeDef @}
  */

/** @defgroup HAL_RTC_CallbackIDTypeDef HAL RTC CallbackIDTypeDef
  * @ingroup  RTC_Structure_Definitions
  * @{
  */

#if (USE_HAL_RTC_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL RTC Callback ID enumeration definition
  */
typedef enum
{
  HAL_RTC_ALARM_A_EVENT_CB_ID           = 0x00u,    /*!< RTC Alarm A Event Callback ID       */
  HAL_RTC_TAMPER1_EVENT_CB_ID           = 0x04u,    /*!< RTC Tamper 1 Callback ID            */
  HAL_RTC_MSPINIT_CB_ID                 = 0x0Eu,    /*!< RTC Msp Init callback ID            */
  HAL_RTC_MSPDEINIT_CB_ID               = 0x0Fu     /*!< RTC Msp DeInit callback ID          */
} HAL_RTC_CallbackIDTypeDef;

/**
  * end of HAL_RTC_CallbackIDTypeDef @}
  */

/** @defgroup pRTC_CallbackTypeDef pRTC CallbackTypeDef
  * @ingroup  RTC_Structure_Definitions
  * @{
  */

/**
  * @brief  HAL RTC Callback pointer definition
  */
typedef  void (*pRTC_CallbackTypeDef)(RTC_HandleTypeDef *hrtc);  /*!< pointer to an RTC callback function */
#endif /* USE_HAL_RTC_REGISTER_CALLBACKS */

/**
  * end of pRTC_CallbackTypeDef @}
  */

/**
  * end of RTC_Structure_Definitions @}
  */

/******************************************************************************/
/*                            RTC Parameters                                  */
/******************************************************************************/

/* Exported constants --------------------------------------------------------*/

/** @defgroup RTC_Parameter_Definitions RTC Parameter Definitions
  * @{
  */

/** @defgroup RTC_Automatic_Prediv_1_Second Automatic calculation of prediv for 1sec timebase
  * @ingroup  RTC_Parameter_Definitions
  * @{
  */
#define RTC_AUTO_1_SECOND                      0xFFFFFFFFU

/**
  * end of RTC_Automatic_Prediv_1_Second @}
  */

/** @defgroup RTC_Input_parameter_format_definitions Input Parameter Format
  * @ingroup  RTC_Parameter_Definitions
  * @{
  */
#define RTC_FORMAT_BIN                         0x000000000U
#define RTC_FORMAT_BCD                         0x000000001U

/**
  * end of RTC_Input_parameter_format_definitions @}
  */

/** @defgroup RTC_Month_Date_Definitions Month Definitions
  * @ingroup  RTC_Parameter_Definitions
  * @{
  */

/* Coded in BCD format */
#define RTC_MONTH_JANUARY              ((uint8_t)0x01)
#define RTC_MONTH_FEBRUARY             ((uint8_t)0x02)
#define RTC_MONTH_MARCH                ((uint8_t)0x03)
#define RTC_MONTH_APRIL                ((uint8_t)0x04)
#define RTC_MONTH_MAY                  ((uint8_t)0x05)
#define RTC_MONTH_JUNE                 ((uint8_t)0x06)
#define RTC_MONTH_JULY                 ((uint8_t)0x07)
#define RTC_MONTH_AUGUST               ((uint8_t)0x08)
#define RTC_MONTH_SEPTEMBER            ((uint8_t)0x09)
#define RTC_MONTH_OCTOBER              ((uint8_t)0x10)
#define RTC_MONTH_NOVEMBER             ((uint8_t)0x11)
#define RTC_MONTH_DECEMBER             ((uint8_t)0x12)

/**
  * end of RTC_Month_Date_Definitions @}
  */

/** @defgroup RTC_WeekDay_Definitions WeekDay Definitions
  * @ingroup  RTC_Parameter_Definitions
  * @{
  */
#define RTC_WEEKDAY_MONDAY             ((uint8_t)0x01)
#define RTC_WEEKDAY_TUESDAY            ((uint8_t)0x02)
#define RTC_WEEKDAY_WEDNESDAY          ((uint8_t)0x03)
#define RTC_WEEKDAY_THURSDAY           ((uint8_t)0x04)
#define RTC_WEEKDAY_FRIDAY             ((uint8_t)0x05)
#define RTC_WEEKDAY_SATURDAY           ((uint8_t)0x06)
#define RTC_WEEKDAY_SUNDAY             ((uint8_t)0x00)

/**
  * end of RTC_WeekDay_Definitions @}
  */

/** @defgroup RTC_Alarms_Definitions Alarms Definitions
  * @ingroup  RTC_Parameter_Definitions
  * @{
  */
#define RTC_ALARM_A                        0U                                 /*!< Specify alarm ID (mainly for legacy purposes) */

/**
  * end of RTC_Alarms_Definitions @}
  */


/** @defgroup RTC_output_source_to_output_on_the_Tamper_pin Output source to output on the Tamper pin
  * @ingroup  RTC_Parameter_Definitions
  * @{
  */

#define RTC_OUTPUTSOURCE_NONE               0x00000000U                       /*!< No output on the TAMPER pin  */
#define RTC_OUTPUTSOURCE_CALIBCLOCK         BKP_RTCCR_CCO                     /*!< RTC clock with a frequency divided by 64 on the TAMPER pin  */
#define RTC_OUTPUTSOURCE_ALARM              BKP_RTCCR_ASOE                    /*!< Alarm pulse signal on the TAMPER pin  */
#define RTC_OUTPUTSOURCE_SECOND             (BKP_RTCCR_ASOS | BKP_RTCCR_ASOE) /*!< Second pulse signal on the TAMPER pin  */

/**
  * end of RTC_output_source_to_output_on_the_Tamper_pin @}
  */

/** @defgroup RTC_Interrupts_Definitions Interrupts Definitions
  * @ingroup  RTC_Parameter_Definitions
  * @{
  */
#define RTC_IT_OW            RTC_CRH_OWIE       /*!< Overflow interrupt */
#define RTC_IT_ALRA          RTC_CRH_ALRIE      /*!< Alarm interrupt */
#define RTC_IT_SEC           RTC_CRH_SECIE      /*!< Second interrupt */
#define RTC_IT_TAMP1         BKP_CSR_TPIE       /*!< TAMPER Pin interrupt enable */
/**
  * end of RTC_Interrupts_Definitions @}
  */

/** @defgroup RTC_Flags_Definitions Flags Definitions
  * @ingroup  RTC_Parameter_Definitions
  * @{
  */
#define RTC_FLAG_RTOFF       RTC_CRL_RTOFF      /*!< RTC Operation OFF flag */
#define RTC_FLAG_RSF         RTC_CRL_RSF        /*!< Registers Synchronized flag */
#define RTC_FLAG_OW          RTC_CRL_OWF        /*!< Overflow flag */
#define RTC_FLAG_ALRAF       RTC_CRL_ALRF       /*!< Alarm flag */
#define RTC_FLAG_SEC         RTC_CRL_SECF       /*!< Second flag */
#define RTC_FLAG_TAMP1F      BKP_CSR_TEF        /*!< Tamper Interrupt Flag */

/**
  * end of RTC_Flags_Definitions @}
  */

/** @defgroup RTC_Timeout_Value Default Timeout Value
  * @ingroup  RTC_Parameter_Definitions
  * @{
  */
#define RTC_TIMEOUT_VALUE           1000U

/**
  * end of RTC_Timeout_Value @}
  */

/** @defgroup RTC_EXTI_Line_Event RTC EXTI Line event
  * @ingroup  RTC_Parameter_Definitions
  * @{
  */
#define RTC_EXTI_LINE_ALARM_EVENT   ((uint32_t)EXTI_IMR_MR17)  /*!< External interrupt line 17 Connected to the RTC Alarm event */
/**
  * end of RTC_EXTI_Line_Event @}
  */

/**
  * end of RTC_Parameter_Definitions @}
  */

/******************************************************************************/
/*                              RTC Macro                                     */
/******************************************************************************/

/* Exported macro ------------------------------------------------------------*/

/** @defgroup RTC_Macro_Definitions RTC Macro Definitions
  * @{
  */

/**
  * @brief  Reset RTC handle state
  * @param  __HANDLE__: RTC handle.
  * @retval None
  */
#if (USE_HAL_RTC_REGISTER_CALLBACKS == 1)
#define __HAL_RTC_RESET_HANDLE_STATE(__HANDLE__) do{\
                                                      (__HANDLE__)->State = HAL_RTC_STATE_RESET;\
                                                      (__HANDLE__)->MspInitCallback = NULL;\
                                                      (__HANDLE__)->MspDeInitCallback = NULL;\
                                                     }while(0)
#else
#define __HAL_RTC_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_RTC_STATE_RESET)
#endif /* USE_HAL_RTC_REGISTER_CALLBACKS */

/**
  * @brief  Disable the write protection for RTC registers.
  * @param  __HANDLE__: specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_WRITEPROTECTION_DISABLE(__HANDLE__)         SET_BIT((__HANDLE__)->Instance->CRL, RTC_CRL_CNF)

/**
  * @brief  Enable the write protection for RTC registers.
  * @param  __HANDLE__: specifies the RTC handle.
  * @retval None
  */
#define __HAL_RTC_WRITEPROTECTION_ENABLE(__HANDLE__)          CLEAR_BIT((__HANDLE__)->Instance->CRL, RTC_CRL_CNF)

/**
  * @brief  Enable the RTC Alarm interrupt.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Alarm interrupt sources to be enabled or disabled.
  *          This parameter can be any combination of the following values:
  *            @arg RTC_IT_ALRA: Alarm A interrupt
  * @retval None
  */
#define __HAL_RTC_ALARM_ENABLE_IT(__HANDLE__, __INTERRUPT__)  SET_BIT((__HANDLE__)->Instance->CRH, (__INTERRUPT__))

/**
  * @brief  Disable the RTC Alarm interrupt.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Alarm interrupt sources to be enabled or disabled.
  *         This parameter can be any combination of the following values:
  *            @arg RTC_IT_ALRA: Alarm A interrupt
  * @retval None
  */
#define __HAL_RTC_ALARM_DISABLE_IT(__HANDLE__, __INTERRUPT__) CLEAR_BIT((__HANDLE__)->Instance->CRH, (__INTERRUPT__))

/**
  * @brief  Check whether the specified RTC Alarm interrupt has been enabled or not.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Alarm interrupt sources to be checked
  *         This parameter can be:
  *            @arg RTC_IT_ALRA: Alarm A interrupt
  * @retval None
  */
#define __HAL_RTC_ALARM_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__)     ((((((__HANDLE__)->Instance->CRH)& ((__INTERRUPT__)))) != RESET)? SET : RESET)

/**
  * @brief  Get the selected RTC Alarm's flag status.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __FLAG__: specifies the RTC Alarm Flag sources to be enabled or disabled.
  *          This parameter can be:
  *            @arg RTC_FLAG_ALRAF
  * @retval None
  */
#define __HAL_RTC_ALARM_GET_FLAG(__HANDLE__, __FLAG__)        (((((__HANDLE__)->Instance->CRL) & (__FLAG__)) != RESET)? SET : RESET)

/**
  * @brief  Check whether the specified RTC Alarm interrupt has occurred or not.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __INTERRUPT__: specifies the RTC Alarm interrupt sources to check.
  *         This parameter can be:
  *            @arg RTC_IT_ALRA: Alarm A interrupt
  * @retval None
  */
#define __HAL_RTC_ALARM_GET_IT(__HANDLE__, __INTERRUPT__)     (((((__HANDLE__)->Instance->CRL) & (__INTERRUPT__)) != RESET)? SET : RESET)

/**
  * @brief  Clear the RTC Alarm's pending flags.
  * @param  __HANDLE__: specifies the RTC handle.
  * @param  __FLAG__: specifies the RTC Alarm Flag sources to be enabled or disabled.
  *         This parameter can be:
  *            @arg RTC_FLAG_ALRAF
  * @retval None
  */
#define __HAL_RTC_ALARM_CLEAR_FLAG(__HANDLE__, __FLAG__)      ((__HANDLE__)->Instance->CRL) &= ~(__FLAG__)

/**
  * @brief Enable interrupt on ALARM Exti Line 17.
  * @retval None.
  */
#define __HAL_RTC_ALARM_EXTI_ENABLE_IT()                      SET_BIT(EXTI->IMR, RTC_EXTI_LINE_ALARM_EVENT)

/**
  * @brief Disable interrupt on ALARM Exti Line 17.
  * @retval None.
  */
#define __HAL_RTC_ALARM_EXTI_DISABLE_IT()                     CLEAR_BIT(EXTI->IMR, RTC_EXTI_LINE_ALARM_EVENT)

/**
  * @brief Enable event on ALARM Exti Line 17.
  * @retval None.
  */
#define __HAL_RTC_ALARM_EXTI_ENABLE_EVENT()                   SET_BIT(EXTI->EMR, RTC_EXTI_LINE_ALARM_EVENT)

/**
  * @brief Disable event on ALARM Exti Line 17.
  * @retval None.
  */
#define __HAL_RTC_ALARM_EXTI_DISABLE_EVENT()                  CLEAR_BIT(EXTI->EMR, RTC_EXTI_LINE_ALARM_EVENT)


/**
  * @brief  ALARM EXTI line configuration: set falling edge trigger.
  * @retval None.
  */
#define __HAL_RTC_ALARM_EXTI_ENABLE_FALLING_EDGE()            SET_BIT(EXTI->FTSR, RTC_EXTI_LINE_ALARM_EVENT)


/**
  * @brief Disable the ALARM Extended Interrupt Falling Trigger.
  * @retval None.
  */
#define __HAL_RTC_ALARM_EXTI_DISABLE_FALLING_EDGE()           CLEAR_BIT(EXTI->FTSR, RTC_EXTI_LINE_ALARM_EVENT)


/**
  * @brief  ALARM EXTI line configuration: set rising edge trigger.
  * @retval None.
  */
#define __HAL_RTC_ALARM_EXTI_ENABLE_RISING_EDGE()             SET_BIT(EXTI->RTSR, RTC_EXTI_LINE_ALARM_EVENT)

/**
  * @brief Disable the ALARM Extended Interrupt Rising Trigger.
  * This parameter can be:
  * @retval None.
  */
#define __HAL_RTC_ALARM_EXTI_DISABLE_RISING_EDGE()            CLEAR_BIT(EXTI->RTSR, RTC_EXTI_LINE_ALARM_EVENT)

/**
  * @brief  ALARM EXTI line configuration: set rising & falling edge trigger.
  * @retval None.
  */
#define __HAL_RTC_ALARM_EXTI_ENABLE_RISING_FALLING_EDGE()  \
          do{                                              \
              __HAL_RTC_ALARM_EXTI_ENABLE_RISING_EDGE();   \
              __HAL_RTC_ALARM_EXTI_ENABLE_FALLING_EDGE();  \
            } while(0U)

/**
  * @brief Disable the ALARM Extended Interrupt Rising & Falling Trigger.
  * This parameter can be:
  * @retval None.
  */
#define __HAL_RTC_ALARM_EXTI_DISABLE_RISING_FALLING_EDGE() \
          do{                                              \
              __HAL_RTC_ALARM_EXTI_DISABLE_RISING_EDGE();  \
              __HAL_RTC_ALARM_EXTI_DISABLE_FALLING_EDGE(); \
            } while(0U)

/**
  * @brief Check whether the specified ALARM EXTI interrupt flag is set or not.
  * @retval EXTI ALARM Line Status.
  */
#define __HAL_RTC_ALARM_EXTI_GET_FLAG()                   (EXTI->PR & (RTC_EXTI_LINE_ALARM_EVENT))

/**
  * @brief Clear the ALARM EXTI flag.
  * @retval None.
  */
#define __HAL_RTC_ALARM_EXTI_CLEAR_FLAG()                 (EXTI->PR = (RTC_EXTI_LINE_ALARM_EVENT))

/**
  * @brief Generate a Software interrupt on selected EXTI line.
  * @retval None.
  */
#define __HAL_RTC_ALARM_EXTI_GENERATE_SWIT()              SET_BIT(EXTI->SWIER, RTC_EXTI_LINE_ALARM_EVENT)

/**
  * @brief  Check if the parameter __PREDIV__ is valid
  * @param  __PREDIV__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_RTC_ASYNCH_PREDIV(__PREDIV__)    (((__PREDIV__) <= 0xFFFFFU) || ((__PREDIV__) == RTC_AUTO_1_SECOND))

/**
  * @brief  Check if the parameter __HOUR__ is valid
  * @param  __HOUR__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RTC_HOUR24(__HOUR__)             ((__HOUR__) <= 23U)

/**
  * @brief  Check if the parameter __MINUTES__ is valid
  * @param  __MINUTES__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_RTC_MINUTES(__MINUTES__)         ((__MINUTES__) <= 59U)

/**
  * @brief  Check if the parameter __SECONDS__ is valid
  * @param  __SECONDS__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RTC_SECONDS(__SECONDS__)         ((__SECONDS__) <= 59U)

/**
  * @brief  Check if the parameter __FORMAT__ is valid
  * @param  __FORMAT__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RTC_FORMAT(__FORMAT__)           (((__FORMAT__) == RTC_FORMAT_BIN) || ((__FORMAT__) == RTC_FORMAT_BCD))

/**
  * @brief  Check if the parameter __YEAR__ is valid
  * @param  __YEAR__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RTC_YEAR(__YEAR__)               ((__YEAR__) <= 99U)

/**
  * @brief  Check if the parameter __MONTH__ is valid
  * @param  __MONTH__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RTC_MONTH(__MONTH__)             (((__MONTH__) >= 1U) && ((__MONTH__) <= 12U))

/**
  * @brief  Check if the parameter __DATE__ is valid
  * @param  __DATE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RTC_DATE(__DATE__)               (((__DATE__) >= 1U) && ((__DATE__) <= 31U))

/**
  * @brief  Check if the parameter __ALARM__ is valid
  * @param  __ALARM__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RTC_ALARM(__ALARM__)             ((__ALARM__) == RTC_ALARM_A)

/**
  * @brief  Check if the parameter __OUTPUT__ is valid
  * @param  __OUTPUT__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_RTC_CALIB_OUTPUT(__OUTPUT__)     (((__OUTPUT__) == RTC_OUTPUTSOURCE_NONE) ||       \
                                             ((__OUTPUT__) == RTC_OUTPUTSOURCE_CALIBCLOCK) || \
                                             ((__OUTPUT__) == RTC_OUTPUTSOURCE_ALARM) ||      \
                                             ((__OUTPUT__) == RTC_OUTPUTSOURCE_SECOND))

/**
  * end of RTC_Macro_Definitions @}
  */

/* Include RTC HAL Extension module */
#include "rx32g4xx_hal_rtc_ex.h"

/* Exported functions --------------------------------------------------------*/

/** @defgroup RTC_Function_Definitions RTC Function Definitions
  * @{
  */

/* Initialization and de-initialization functions  ****************************/
HAL_StatusTypeDef HAL_RTC_Init(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTC_DeInit(RTC_HandleTypeDef *hrtc);
void              HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc);
void              HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc);

/* Callbacks Register/UnRegister functions  ***********************************/
#if (USE_HAL_RTC_REGISTER_CALLBACKS == 1)
HAL_StatusTypeDef HAL_RTC_RegisterCallback(RTC_HandleTypeDef *hrtc, HAL_RTC_CallbackIDTypeDef CallbackID, pRTC_CallbackTypeDef pCallback);
HAL_StatusTypeDef HAL_RTC_UnRegisterCallback(RTC_HandleTypeDef *hrtc, HAL_RTC_CallbackIDTypeDef CallbackID);
#endif /* USE_HAL_RTC_REGISTER_CALLBACKS */

/* RTC Time and Date functions ************************************************/
HAL_StatusTypeDef HAL_RTC_SetTime(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, uint32_t Format);
HAL_StatusTypeDef HAL_RTC_GetTime(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, uint32_t Format);
HAL_StatusTypeDef HAL_RTC_SetDate(RTC_HandleTypeDef *hrtc, RTC_DateTypeDef *sDate, uint32_t Format);
HAL_StatusTypeDef HAL_RTC_GetDate(RTC_HandleTypeDef *hrtc, RTC_DateTypeDef *sDate, uint32_t Format);

/* RTC Alarm functions ********************************************************/
HAL_StatusTypeDef HAL_RTC_SetAlarm(RTC_HandleTypeDef *hrtc, RTC_AlarmTypeDef *sAlarm, uint32_t Format);
HAL_StatusTypeDef HAL_RTC_SetAlarm_IT(RTC_HandleTypeDef *hrtc, RTC_AlarmTypeDef *sAlarm, uint32_t Format);
HAL_StatusTypeDef HAL_RTC_DeactivateAlarm(RTC_HandleTypeDef *hrtc, uint32_t Alarm);
HAL_StatusTypeDef HAL_RTC_GetAlarm(RTC_HandleTypeDef *hrtc, RTC_AlarmTypeDef *sAlarm, uint32_t Alarm, uint32_t Format);
void              HAL_RTC_AlarmIRQHandler(RTC_HandleTypeDef *hrtc);
HAL_StatusTypeDef HAL_RTC_PollForAlarmAEvent(RTC_HandleTypeDef *hrtc, uint32_t Timeout);
void              HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc);


/* Peripheral State functions *************************************************/
HAL_RTCStateTypeDef HAL_RTC_GetState(RTC_HandleTypeDef *hrtc);

HAL_StatusTypeDef   HAL_RTC_WaitForSynchro(RTC_HandleTypeDef *hrtc);

/**
  * end of RTC_Function_Definitions @}
  */

/**
  * end of RTC @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __RX32G4xx_HAL_RTC_H__ */
