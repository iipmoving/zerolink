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
*   File    : rx32g4xx_hal_hrtim.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_HRTIM_H_
#define _RX32G4XX_HAL_HRTIM_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"

/** @addtogroup HRTIM
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup HRTIM_Max_Timer HRTIM Max Timer
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */
#define MAX_HRTIM_TIMER 7U

/**
  * end of HRTIM_Max_Timer @}
  */

/******************************************************************************/
/*                            HRTIM Structures                                */
/******************************************************************************/

/** @defgroup HRTIM_Structure_Definitions HRTIM Structure Definitions
  * @{
  */

/** @defgroup HRTIM_InitTypeDef HRTIM InitTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  HRTIM Configuration Structure definition - Time base related parameters
  */
typedef struct
{
  uint32_t HRTIMInterruptResquests;  /*!< Specifies which interrupts requests must enabled for the HRTIM instance.
                                          This parameter can be any combination of  @ref HRTIM_Common_Interrupt_Enable */
  uint32_t SyncOptions;              /*!< Specifies how the HRTIM instance handles the external synchronization signals.
                                          The HRTIM instance can be configured to act as a slave (waiting for a trigger
                                          to be synchronized) or a master (generating a synchronization signal) or both.
                                          This parameter can be a combination of @ref HRTIM_Synchronization_Options.*/
  uint32_t SyncInputSource;          /*!< Specifies the external synchronization input source (significant only when
                                          the HRTIM instance is configured as a slave).
                                          This parameter can be a value of @ref HRTIM_Synchronization_Input_Source. */
  uint32_t SyncOutputSource;         /*!< Specifies the source and event to be sent on the external synchronization outputs
                                         (significant only when the HRTIM instance is configured as a master).
                                          This parameter can be a value of @ref HRTIM_Synchronization_Output_Source */
  uint32_t SyncOutputPolarity;       /*!< Specifies the conditioning of the event to be sent on the external synchronization
                                          outputs (significant only when the HRTIM instance is configured as a master).
                                          This parameter can be a value of @ref HRTIM_Synchronization_Output_Polarity */
} HRTIM_InitTypeDef;

/**
  * end of HRTIM_InitTypeDef @}
  */

/** @defgroup HAL_HRTIM_StateTypeDef HAL HRTIM StateTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  HAL State structures definition
  */
typedef enum
{
  HAL_HRTIM_STATE_RESET            = 0x00U,    /*!< Peripheral is not yet Initialized                  */
  HAL_HRTIM_STATE_READY            = 0x01U,    /*!< Peripheral Initialized and ready for use           */
  HAL_HRTIM_STATE_BUSY             = 0x02U,    /*!< an internal process is ongoing                     */
  HAL_HRTIM_STATE_TIMEOUT          = 0x06U,    /*!< Timeout state                                      */
  HAL_HRTIM_STATE_ERROR            = 0x07U,    /*!< Error state                                        */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
  HAL_HRTIM_STATE_INVALID_CALLBACK = 0x08U    /*!< Invalid Callback error */
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
} HAL_HRTIM_StateTypeDef;

/**
  * end of HAL_HRTIM_StateTypeDef @}
  */

/** @defgroup HRTIM_TimerParamTypeDef HRTIM TimerParamTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief HRTIM Timer Structure definition
  */
typedef struct
{
  uint32_t CaptureTrigger1;        /*!< Event(s) triggering capture unit 1.
                                        When the timer operates in Simple mode, this parameter can be a value of @ref HRTIM_External_Event_Channels.
                                        When the timer operates in Waveform mode, this parameter can be a combination of @ref HRTIM_Capture_Unit_Trigger. */
  uint32_t CaptureTrigger2;        /*!< Event(s) triggering capture unit 2.
                                        When the timer operates in Simple mode, this parameter can be a value of @ref HRTIM_External_Event_Channels.
                                        When the timer operates in Waveform mode, this parameter can be a combination of @ref HRTIM_Capture_Unit_Trigger. */
  uint32_t InterruptRequests;      /*!< Interrupts requests enabled for the timer. */
  uint32_t DMARequests;            /*!< DMA requests enabled for the timer. */
  uint32_t DMASrcAddress;          /*!< Address of the source address of the DMA transfer. */
  uint32_t DMADstAddress;          /*!< Address of the destination address of the DMA transfer. */
  uint32_t DMASize;                /*!< Size of the DMA transfer */
} HRTIM_TimerParamTypeDef;

/**
  * end of HRTIM_TimerParamTypeDef @}
  */

/** @defgroup HRTIM_HandleTypeDef HRTIM HandleTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  HRTIM Handle Structure definition
  */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
typedef struct __HRTIM_HandleTypeDef
#else
typedef struct
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
{
  HRTIM_TypeDef *              Instance;                     /*!< Register base address */

  HRTIM_InitTypeDef            Init;                         /*!< HRTIM required parameters */

  HRTIM_TimerParamTypeDef      TimerParam[MAX_HRTIM_TIMER];  /*!< HRTIM timers - including the master - parameters */

  HAL_LockTypeDef              Lock;                         /*!< Locking object          */

  __IO HAL_HRTIM_StateTypeDef  State;                        /*!< HRTIM communication state */

  DMA_HandleTypeDef *          hdmaMaster;                   /*!< Master timer DMA handle parameters */
  DMA_HandleTypeDef *          hdmaTimerA;                   /*!< Timer A DMA handle parameters */
  DMA_HandleTypeDef *          hdmaTimerB;                   /*!< Timer B DMA handle parameters */
  DMA_HandleTypeDef *          hdmaTimerC;                   /*!< Timer C DMA handle parameters */
  DMA_HandleTypeDef *          hdmaTimerD;                   /*!< Timer D DMA handle parameters */
  DMA_HandleTypeDef *          hdmaTimerE;                   /*!< Timer E DMA handle parameters */
  DMA_HandleTypeDef *          hdmaTimerF;                   /*!< Timer F DMA handle parameters */

#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
  void (* Fault1Callback)(struct __HRTIM_HandleTypeDef *hhrtim);                               /*!< Fault 1 interrupt callback function pointer                         */
  void (* Fault2Callback)(struct __HRTIM_HandleTypeDef *hhrtim);                               /*!< Fault 2 interrupt callback function pointer                         */
  void (* Fault3Callback)(struct __HRTIM_HandleTypeDef *hhrtim);                               /*!< Fault 3 interrupt callback function pointer                         */
  void (* Fault4Callback)(struct __HRTIM_HandleTypeDef *hhrtim);                               /*!< Fault 4 interrupt callback function pointer                         */
  void (* Fault5Callback)(struct __HRTIM_HandleTypeDef *hhrtim);                               /*!< Fault 5 interrupt callback function pointer                         */
  void (* Fault6Callback)(struct __HRTIM_HandleTypeDef *hhrtim);                               /*!< Fault 6 interrupt callback function pointer                         */
  void (* SystemFaultCallback)(struct __HRTIM_HandleTypeDef *hhrtim);                          /*!< System fault interrupt callback function pointer                    */
  void (* DLLCalibrationReadyCallback)(struct __HRTIM_HandleTypeDef *hhrtim);                   /*!< DLL Ready interrupt callback function pointer                       */
  void (* BurstModePeriodCallback)(struct __HRTIM_HandleTypeDef *hhrtim);                      /*!< Burst mode period interrupt callback function pointer               */
  void (* SynchronizationEventCallback)(struct __HRTIM_HandleTypeDef *hhrtim);                 /*!< Sync Input interrupt callback function pointer                      */
  void (* ErrorCallback)(struct __HRTIM_HandleTypeDef *hhrtim);                                /*!< DMA error callback function pointer                                 */

  void (* RegistersUpdateCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);   /*!< Timer x Update interrupt callback function pointer                  */
  void (* RepetitionEventCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);   /*!< Timer x Repetition interrupt callback function pointer              */
  void (* Compare1EventCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);     /*!< Timer x Compare 1 match interrupt callback function pointer         */
  void (* Compare2EventCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);     /*!< Timer x Compare 2 match interrupt callback function pointer         */
  void (* Compare3EventCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);     /*!< Timer x Compare 3 match interrupt callback function pointer         */
  void (* Compare4EventCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);     /*!< Timer x Compare 4 match interrupt callback function pointer         */
  void (* Capture1EventCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);     /*!< Timer x Capture 1 interrupts callback function pointer              */
  void (* Capture2EventCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);     /*!< Timer x Capture 2 interrupts callback function pointer              */
  void (* DelayedProtectionCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx); /*!< Timer x Delayed protection interrupt callback function pointer      */
  void (* CounterResetCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);      /*!< Timer x counter reset/roll-over interrupt callback function pointer */
  void (* Output1SetCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);        /*!< Timer x output 1 set interrupt callback function pointer            */
  void (* Output1ResetCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);      /*!< Timer x output 1 reset interrupt callback function pointer          */
  void (* Output2SetCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);        /*!< Timer x output 2 set interrupt callback function pointer            */
  void (* Output2ResetCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);      /*!< Timer x output 2 reset interrupt callback function pointer          */
  void (* BurstDMATransferCallback)(struct __HRTIM_HandleTypeDef *hhrtim, uint32_t TimerIdx);  /*!< Timer x Burst DMA completed interrupt callback function pointer     */

  void (* MspInitCallback)(struct __HRTIM_HandleTypeDef *hhrtim);                              /*!< HRTIM MspInit callback function pointer                             */
  void (* MspDeInitCallback)(struct __HRTIM_HandleTypeDef *hhrtim);                            /*!< HRTIM MspInit callback function pointer                             */
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
} HRTIM_HandleTypeDef;

/**
  * end of HRTIM_HandleTypeDef @}
  */

/** @defgroup HRTIM_TimeBaseCfgTypeDef HRTIM TimeBaseCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Simple output compare mode configuration definition
  */
typedef struct
{
  uint32_t Period;                   /*!< Specifies the timer period.
                                          The period value must be above 3 periods of the fHRTIM clock.
                                          Maximum value is = 0xFFDFU */
  uint32_t RepetitionCounter;        /*!< Specifies the timer repetition period.
                                          This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF. */
  uint32_t PrescalerRatio;           /*!< Specifies the timer clock prescaler ratio.
                                          This parameter can be any value of @ref HRTIM_Prescaler_Ratio   */
  uint32_t Mode;                     /*!< Specifies the counter operating mode.
                                          This parameter can be any value of @ref HRTIM_Counter_Operating_Mode   */
} HRTIM_TimeBaseCfgTypeDef;

/**
  * end of HRTIM_TimeBaseCfgTypeDef @}
  */

/** @defgroup HRTIM_SimpleOCChannelCfgTypeDef HRTIM SimpleOCChannelCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Simple output compare mode configuration definition
  */
typedef struct
{
  uint32_t Mode;       /*!< Specifies the output compare mode (toggle, active, inactive).
                            This parameter can be any value of of @ref HRTIM_Simple_OC_Mode */
  uint32_t Pulse;      /*!< Specifies the compare value to be loaded into the Compare Register.
                            The compare value must be above or equal to 3 periods of the fHRTIM clock */
  uint32_t Polarity;   /*!< Specifies the output polarity.
                            This parameter can be any value of @ref HRTIM_Output_Polarity */
  uint32_t IdleLevel;  /*!< Specifies whether the output level is active or inactive when in IDLE state.
                            This parameter can be any value of @ref HRTIM_Output_IDLE_Level */
} HRTIM_SimpleOCChannelCfgTypeDef;

/**
  * end of HRTIM_SimpleOCChannelCfgTypeDef @}
  */

/** @defgroup HRTIM_SimplePWMChannelCfgTypeDef HRTIM SimplePWMChannelCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Simple PWM output mode configuration definition
  */
typedef struct
{
  uint32_t Pulse;            /*!< Specifies the compare value to be loaded into the Compare Register.
                                  The compare value must be above or equal to 3 periods of the fHRTIM clock */
  uint32_t Polarity;         /*!< Specifies the output polarity.
                                  This parameter can be any value of @ref HRTIM_Output_Polarity */
  uint32_t IdleLevel;        /*!< Specifies whether the output level is active or inactive when in IDLE state.
                                  This parameter can be any value of @ref HRTIM_Output_IDLE_Level */
} HRTIM_SimplePWMChannelCfgTypeDef;

/**
  * end of HRTIM_SimplePWMChannelCfgTypeDef @}
  */

/** @defgroup HRTIM_SimpleCaptureChannelCfgTypeDef HRTIM SimpleCaptureChannelCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Simple capture mode configuration definition
  */
typedef struct
{
  uint32_t Event;             /*!< Specifies the external event triggering the capture.
                                   This parameter can be any 'EEVx' value of @ref HRTIM_External_Event_Channels */
  uint32_t EventPolarity;     /*!< Specifies the polarity of the external event (in case of level sensitivity).
                                   This parameter can be a value of @ref HRTIM_External_Event_Polarity */
  uint32_t EventSensitivity;  /*!< Specifies the sensitivity of the external event.
                                   This parameter can be a value of @ref HRTIM_External_Event_Sensitivity */
  uint32_t EventFilter;       /*!< Defines the frequency used to sample the External Event and the length of the digital filter.
                                   This parameter can be a value of @ref HRTIM_External_Event_Filter */
} HRTIM_SimpleCaptureChannelCfgTypeDef;

/**
  * end of HRTIM_SimpleCaptureChannelCfgTypeDef @}
  */

/** @defgroup HRTIM_SimpleOnePulseChannelCfgTypeDef HRTIM SimpleOnePulseChannelCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Simple One Pulse mode configuration definition
  */
typedef struct
{
  uint32_t Pulse;             /*!< Specifies the compare value to be loaded into the Compare Register.
                                   The compare value must be above or equal to 3 periods of the fHRTIM clock */
  uint32_t OutputPolarity;    /*!< Specifies the output polarity.
                                   This parameter can be any value of @ref HRTIM_Output_Polarity */
  uint32_t OutputIdleLevel;   /*!< Specifies whether the output level is active or inactive when in IDLE state.
                                   This parameter can be any value of @ref HRTIM_Output_IDLE_Level */
  uint32_t Event;             /*!< Specifies the external event triggering the pulse generation.
                                   This parameter can be any 'EEVx' value of @ref HRTIM_External_Event_Channels */
  uint32_t EventPolarity;     /*!< Specifies the polarity of the external event (in case of level sensitivity).
                                   This parameter can be a value of @ref HRTIM_External_Event_Polarity */
  uint32_t EventSensitivity;  /*!< Specifies the sensitivity of the external event.
                                   This parameter can be a value of @ref HRTIM_External_Event_Sensitivity. */
  uint32_t EventFilter;       /*!< Defines the frequency used to sample the External Event and the length of the digital filter.
                                   This parameter can be a value of @ref HRTIM_External_Event_Filter */
} HRTIM_SimpleOnePulseChannelCfgTypeDef;

/**
  * end of HRTIM_SimpleOnePulseChannelCfgTypeDef @}
  */

/** @defgroup HRTIM_TimerCfgTypeDef HRTIM TimerCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Timer configuration definition
  */
typedef struct
{
  uint32_t InterruptRequests;      /*!< Relevant for all HRTIM timers, including the master.
                                       Specifies which interrupts requests must enabled for the timer.
                                       This parameter can be any combination of  @ref HRTIM_Master_Interrupt_Enable
                                       or @ref HRTIM_Timing_Unit_Interrupt_Enable */
  uint32_t DMARequests;            /*!< Relevant for all HRTIM timers, including the master.
                                       Specifies which DMA requests must be enabled for the timer.
                                       This parameter can be any combination of  @ref HRTIM_Master_DMA_Request_Enable
                                       or @ref HRTIM_Timing_Unit_DMA_Request_Enable */
  uint32_t DMASrcAddress;          /*!< Relevant for all HRTIM timers, including the master.
                                       Specifies the address of the source address of the DMA transfer */
  uint32_t DMADstAddress;          /*!< Relevant for all HRTIM timers, including the master.
                                       Specifies the address of the destination address of the DMA transfer */
  uint32_t DMASize;                /*!< Relevant for all HRTIM timers, including the master.
                                       Specifies the size of the DMA transfer */
  uint32_t HalfModeEnable;         /*!< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not half mode is enabled
                                        This parameter can be any value of @ref HRTIM_Half_Mode_Enable  */
  uint32_t InterleavedMode;         /*!< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not half mode is enabled
                                        This parameter can be any value of @ref HRTIM_Interleaved_Mode  */
  uint32_t StartOnSync;            /*!< Relevant for all HRTIM timers, including the master.
                                       Specifies whether or not timer is reset by a rising edge on the synchronization input (when enabled).
                                        This parameter can be any value of @ref HRTIM_Start_On_Sync_Input_Event  */
  uint32_t ResetOnSync;            /*!< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not timer is reset by a rising edge on the synchronization input (when enabled).
                                        This parameter can be any value of @ref HRTIM_Reset_On_Sync_Input_Event  */
  uint32_t DACSynchro;             /*!< Relevant for all HRTIM timers, including the master.
                                        Indicates whether or not the a DAC synchronization event is generated.
                                        This parameter can be any value of @ref HRTIM_DAC_Synchronization   */
  uint32_t PreloadEnable;          /*!< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not register preload is enabled.
                                        This parameter can be any value of @ref HRTIM_Register_Preload_Enable  */
  uint32_t UpdateGating;           /*!< Relevant for all HRTIM timers, including the master.
                                        Specifies how the update occurs with respect to a burst DMA transaction or
                                        update enable inputs (Slave timers only).
                                        This parameter can be any value of @ref HRTIM_Update_Gating   */
  uint32_t BurstMode;              /*!< Relevant for all HRTIM timers, including the master.
                                        Specifies how the timer behaves during a burst mode operation.
                                        This parameter can be any value of @ref HRTIM_Timer_Burst_Mode  */
  uint32_t RepetitionUpdate;       /*!< Relevant for all HRTIM timers, including the master.
                                        Specifies whether or not registers update is triggered by the repetition event.
                                        This parameter can be any value of @ref HRTIM_Timer_Repetition_Update */
  uint32_t PushPull;               /*!< Relevant for Timer A to Timer F.
                                        Specifies whether or not the push-pull mode is enabled.
                                        This parameter can be any value of @ref HRTIM_Timer_Push_Pull_Mode */
  uint32_t FaultEnable;            /*!< Relevant for Timer A to Timer F.
                                        Specifies which fault channels are enabled for the timer.
                                        This parameter can be a combination of @ref HRTIM_Timer_Fault_Enabling  */
  uint32_t FaultLock;              /*!< Relevant for Timer A to Timer F.
                                        Specifies whether or not fault enabling status is write protected.
                                        This parameter can be a value of @ref HRTIM_Timer_Fault_Lock */
  uint32_t DeadTimeInsertion;      /*!< Relevant for Timer A to Timer F.
                                        Specifies whether or not dead-time insertion is enabled for the timer.
                                        This parameter can be a value of @ref HRTIM_Timer_Deadtime_Insertion */
  uint32_t DelayedProtectionMode;  /*!< Relevant for Timer A to Timer F.
                                        Specifies the delayed protection mode.
                                        This parameter can be a value of @ref HRTIM_Timer_Delayed_Protection_Mode */
  uint32_t BalancedIdleAutomaticResume; /*!< Indicates whether or not outputs are automatically re-enabled after a balanced idle event.
                                             This parameters can be any value of @ref HRTIM_Output_Balanced_Idle_Auto_Resume */
  uint32_t UpdateTrigger;          /*!< Relevant for Timer A to Timer F.
                                        Specifies source(s) triggering the timer registers update.
                                        This parameter can be a combination of @ref HRTIM_Timer_Update_Trigger */
  uint32_t ResetTrigger;           /*!< Relevant for Timer A to Timer F.
                                        Specifies source(s) triggering the timer counter reset.
                                        This parameter can be a combination of @ref HRTIM_Timer_Reset_Trigger */
  uint32_t ResetUpdate;           /*!<  Relevant for Timer A to Timer F.
                                        Specifies whether or not registers update is triggered when the timer counter is reset.
                                        This parameter can be a value of @ref HRTIM_Timer_Reset_Update */
  uint32_t ReSyncUpdate;          /*!<  Relevant for Timer A to Timer F.
                                        Specifies whether update source is coming from the timing unit @ref HRTIM_Timer_ReSyncUpdate */

} HRTIM_TimerCfgTypeDef;

/**
  * end of HRTIM_TimerCfgTypeDef @}
  */

/** @defgroup HRTIM_TimerCtlTypeDef HRTIM TimerCtlTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Timer control definition
  */
typedef struct
{
  uint32_t UpDownMode;            /*!<  Relevant for Timer A to Timer F.
                                        Specifies whether or not counter is operating in up or up-down counting mode.
                                        This parameter can be a value of @ref HRTIM_Timer_UpDown_Mode */
  uint32_t TrigHalf;              /*!<  Relevant for Timer A to Timer F.
                                        Specifies whether or not compare 2 is operating in Trigger half mode.
                                        This parameter can be a value of @ref HRTIM_Timer_TrigHalf_Mode */
  uint32_t GreaterCMP3;           /*!<  Relevant for Timer A to Timer F.
                                        Specifies whether or not compare 3 is operating in compare match or greater mode.
                                        This parameter can be a value of @ref HRTIM_Timer_GreaterCMP3_Mode */
  uint32_t GreaterCMP1;           /*!<  Relevant for Timer A to Timer F.
                                        Specifies whether or not compare 1 is operating in compare match or greater mode.
                                        This parameter can be a value of @ref HRTIM_Timer_GreaterCMP1_Mode */
  uint32_t DualChannelDacReset;   /*!<  Relevant for Timer A to Timer F.
                                        Specifies how the hrtim_dac_reset_trgx trigger is generated.
                                        This parameter can be a value of @ref HRTIM_Timer_DualChannelDac_Reset */
  uint32_t DualChannelDacStep;    /*!<  Relevant for Timer A to Timer F.
                                        Specifies how the hrtim_dac_step_trgx trigger is generated.
                                        This parameter can be a value of @ref HRTIM_Timer_DualChannelDac_Step */
  uint32_t DualChannelDacEnable;  /*!<  Relevant for Timer A to Timer F.
                                        Enables or not the dual channel DAC triggering mechanism.
                                        This parameter can be a value of @ref HRTIM_Timer_DualChannelDac_Enable */
} HRTIM_TimerCtlTypeDef;

/**
  * end of HRTIM_TimerCtlTypeDef @}
  */

/** @defgroup HRTIM_CompareCfgTypeDef HRTIM CompareCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Compare unit configuration definition
  */
typedef struct
{
  uint32_t CompareValue;         /*!< Specifies the compare value of the timer compare unit.
                                      The minimum value must be greater than or equal to 3 periods of the fHRTIM clock.
                                      The maximum value must be less than or equal to 0xFFFFU - 1 periods of the fHRTIM clock */
  uint32_t AutoDelayedMode;      /*!< Specifies the auto delayed mode for compare unit 2 or 4.
                                      This parameter can be a value of @ref HRTIM_Compare_Unit_Auto_Delayed_Mode */
  uint32_t AutoDelayedTimeout;   /*!< Specifies compare value for timing unit 1 or 3 when auto delayed mode with time out is selected.
                                      CompareValue +  AutoDelayedTimeout must be less than 0xFFFFU */
} HRTIM_CompareCfgTypeDef;

/**
  * end of HRTIM_CompareCfgTypeDef @}
  */

/** @defgroup HRTIM_CaptureValueTypeDef HRTIM CaptureValueTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Capture unit content definition
  */
typedef struct
{
  uint32_t Value;     /*!< Holds the counter value when the capture event occurred.
                           This parameter can be a number between 0x0 and 0xFFFFU */
  uint32_t Dir ;     /*!< Holds the counting direction value  when the capture event occurred.
                           This parameter can be a value of @ref HRTIM_Timer_UpDown_Mode  */
} HRTIM_CaptureValueTypeDef;

/**
  * end of HRTIM_CaptureValueTypeDef @}
  */

/** @defgroup HRTIM_CaptureCfgTypeDef HRTIM CaptureCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Capture unit configuration definition
  */
typedef struct
{
  uint64_t Trigger;          /*!< Specifies source(s) triggering the capture.
                                  This parameter can be a combination of @ref HRTIM_Capture_Unit_Trigger */
} HRTIM_CaptureCfgTypeDef;

/**
  * end of HRTIM_CaptureCfgTypeDef @}
  */

/** @defgroup HRTIM_OutputCfgTypeDef HRTIM OutputCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Output configuration definition
  */
typedef struct
{
  uint32_t Polarity;                    /*!< Specifies the output polarity.
                                             This parameter can be any value of @ref HRTIM_Output_Polarity */
  uint32_t SetSource;                   /*!< Specifies the event(s) transitioning the output from its inactive level to its active level.
                                             This parameter can be a combination of @ref HRTIM_Output_Set_Source */
  uint32_t ResetSource;                 /*!< Specifies the event(s) transitioning the output from its active level to its inactive level.
                                             This parameter can be a combination of @ref HRTIM_Output_Reset_Source */
  uint32_t IdleMode;                    /*!< Specifies whether or not the output is affected by a burst mode operation.
                                             This parameter can be any value of @ref HRTIM_Output_Idle_Mode */
  uint32_t IdleLevel;                   /*!< Specifies whether the output level is active or inactive when in IDLE state.
                                             This parameter can be any value of @ref HRTIM_Output_IDLE_Level */
  uint32_t FaultLevel;                  /*!< Specifies whether the output level is active or inactive when in FAULT state.
                                             This parameter can be any value of @ref HRTIM_Output_FAULT_Level */
  uint32_t ChopperModeEnable;           /*!< Indicates whether or not the chopper mode is enabled
                                             This parameter can be any value of @ref HRTIM_Output_Chopper_Mode_Enable */
  uint32_t BurstModeEntryDelayed;       /*!< Indicates whether or not dead-time is inserted when entering the IDLE state during a burst mode operation.
                                             This parameters can be any value of @ref HRTIM_Output_Burst_Mode_Entry_Delayed */
} HRTIM_OutputCfgTypeDef;

/**
  * end of HRTIM_OutputCfgTypeDef @}
  */

/** @defgroup HRTIM_TimerEventFilteringCfgTypeDef HRTIM TimerEventFilteringCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  External event filtering in timing units configuration definition
  */
typedef struct
{
  uint32_t Filter;       /*!< Specifies the type of event filtering within the timing unit.
                             This parameter can be a value of @ref HRTIM_Timer_External_Event_Filter */
  uint32_t Latch;       /*!< Specifies whether or not the signal is latched.
                             This parameter can be a value of @ref HRTIM_Timer_External_Event_Latch */
} HRTIM_TimerEventFilteringCfgTypeDef;

/**
  * end of HRTIM_TimerEventFilteringCfgTypeDef @}
  */

/** @defgroup HRTIM_DeadTimeCfgTypeDef HRTIM DeadTimeCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Dead time feature configuration definition
  */
typedef struct
{
  uint32_t Prescaler;        /*!< Specifies the dead-time prescaler.
                                  This parameter can be a value of @ref  HRTIM_Deadtime_Prescaler_Ratio */
  uint32_t RisingValue;      /*!< Specifies the dead-time following a rising edge.
                                  This parameter can be a number between 0x0 and 0x1FFU */
  uint32_t RisingSign;       /*!< Specifies whether the dead-time is positive or negative on rising edge.
                                  This parameter can be a value of @ref HRTIM_Deadtime_Rising_Sign */
  uint32_t RisingLock;       /*!< Specifies whether or not dead-time rising settings (value and sign) are write protected.
                                  This parameter can be a value of @ref HRTIM_Deadtime_Rising_Lock */
  uint32_t RisingSignLock;   /*!< Specifies whether or not dead-time rising sign is write protected.
                                  This parameter can be a value of @ref HRTIM_Deadtime_Rising_Sign_Lock */
  uint32_t FallingValue;     /*!< Specifies the dead-time following a falling edge.
                                  This parameter can be a number between 0x0 and 0x1FFU */
  uint32_t FallingSign;      /*!< Specifies whether the dead-time is positive or negative on falling edge.
                                  This parameter can be a value of @ref HRTIM_Deadtime_Falling_Sign */
  uint32_t FallingLock;      /*!< Specifies whether or not dead-time falling settings (value and sign) are write protected.
                                  This parameter can be a value of @ref HRTIM_Deadtime_Falling_Lock */
  uint32_t FallingSignLock;  /*!< Specifies whether or not dead-time falling sign is write protected.
                                  This parameter can be a value of @ref HRTIM_Deadtime_Falling_Sign_Lock */
} HRTIM_DeadTimeCfgTypeDef;

/**
  * end of HRTIM_DeadTimeCfgTypeDef @}
  */

/** @defgroup HRTIM_ChopperModeCfgTypeDef HRTIM ChopperModeCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Chopper mode configuration definition
  */
typedef struct
{
  uint32_t CarrierFreq;  /*!< Specifies the Timer carrier frequency value.
                              This parameter can be a value of @ref HRTIM_Chopper_Frequency */
  uint32_t DutyCycle;    /*!< Specifies the Timer chopper duty cycle value.
                              This parameter can be a value of @ref HRTIM_Chopper_Duty_Cycle */
  uint32_t StartPulse;   /*!< Specifies the Timer pulse width value.
                              This parameter can be a value of @ref HRTIM_Chopper_Start_Pulse_Width */
} HRTIM_ChopperModeCfgTypeDef;

/**
  * end of HRTIM_ChopperModeCfgTypeDef @}
  */

/** @defgroup HRTIM_EventCfgTypeDef HRTIM EventCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  External event channel configuration definition
  */
typedef struct
{
  uint32_t Source;        /*!< Identifies the source of the external event.
                               This parameter can be a value of @ref HRTIM_External_Event_Sources */
  uint32_t Polarity;      /*!< Specifies the polarity of the external event (in case of level sensitivity).
                               This parameter can be a value of @ref HRTIM_External_Event_Polarity */
  uint32_t Sensitivity;   /*!< Specifies the sensitivity of the external event.
                               This parameter can be a value of @ref HRTIM_External_Event_Sensitivity */
  uint32_t Filter;        /*!< Defines the frequency used to sample the External Event and the length of the digital filter.
                               This parameter can be a value of @ref HRTIM_External_Event_Filter */
  uint32_t FastMode;      /*!< Indicates whether or not low latency mode is enabled for the external event.
                               This parameter can be a value of @ref HRTIM_External_Event_Fast_Mode */
} HRTIM_EventCfgTypeDef;

/**
  * end of HRTIM_EventCfgTypeDef @}
  */

/** @defgroup HRTIM_FaultCfgTypeDef HRTIM FaultCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Fault channel configuration definition
  */
typedef struct
{
  uint32_t Source;        /*!< Identifies the source of the fault.
                               This parameter can be a value of @ref HRTIM_Fault_Sources */
  uint32_t Polarity;      /*!< Specifies the polarity of the fault event.
                               This parameter can be a value of @ref HRTIM_Fault_Polarity */
  uint32_t Filter;        /*!< Defines the frequency used to sample the Fault input and the length of the digital filter.
                               This parameter can be a value of @ref HRTIM_Fault_Filter */
  uint32_t Lock;          /*!< Indicates whether or not fault programming bits are write protected.
                               This parameter can be a value of @ref HRTIM_Fault_Lock */
} HRTIM_FaultCfgTypeDef;

/**
  * end of HRTIM_FaultCfgTypeDef @}
  */

/** @defgroup HRTIM_FaultBlankingCfgTypeDef HRTIM FaultBlankingCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Fault channel blanking configuration definition
  */
typedef struct
{
  uint32_t Threshold;     /*!< Specifies the Fault counter Threshold.
                               This parameter can be a number between 0x0 and 0xF  */
  uint32_t ResetMode;     /*!< Specifies the reset mode of a fault event counter.
                               This parameter can be a value of @ref HRTIM_Fault_ResetMode */
  uint32_t BlankingSource;/*!< Specifies the blanking source of a fault event.
                               This parameter can be a value of @ref HRTIM_Fault_Blanking */
} HRTIM_FaultBlankingCfgTypeDef;

/**
  * end of HRTIM_FaultBlankingCfgTypeDef @}
  */

/** @defgroup HRTIM_BurstModeCfgTypeDef HRTIM BurstModeCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  Burst mode configuration definition
  */
typedef struct
{
  uint32_t Mode;           /*!< Specifies the burst mode operating mode.
                                This parameter can be a value of @ref HRTIM_Burst_Mode_Operating_Mode */
  uint32_t ClockSource;    /*!< Specifies the burst mode clock source.
                                This parameter can be a value of @ref HRTIM_Burst_Mode_Clock_Source */
  uint32_t Prescaler;      /*!< Specifies the burst mode prescaler.
                                This parameter can be a value of @ref HRTIM_Burst_Mode_Prescaler */
  uint32_t PreloadEnable;  /*!< Specifies whether or not preload is enabled for burst mode related registers (HRTIM_BMCMPR and HRTIM_BMPER).
                                This parameter can be a combination of @ref HRTIM_Burst_Mode_Register_Preload_Enable  */
  uint32_t Trigger;        /*!< Specifies the event(s) triggering the burst operation.
                                This parameter can be a combination of @ref HRTIM_Burst_Mode_Trigger  */
  uint32_t IdleDuration;   /*!< Specifies number of periods during which the selected timers are in idle state.
                                This parameter can be a number between 0x0 and 0xFFFF  */
  uint32_t Period;         /*!< Specifies burst mode repetition period.
                                This parameter can be a number between 0x1 and 0xFFFF  */
} HRTIM_BurstModeCfgTypeDef;

/**
  * end of HRTIM_BurstModeCfgTypeDef @}
  */

/** @defgroup HRTIM_ADCTriggerCfgTypeDef HRTIM ADCTriggerCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  ADC trigger configuration definition
  */
typedef struct
{
  uint32_t UpdateSource;  /*!< Specifies the ADC trigger update source.
                               This parameter can be a value of @ref HRTIM_ADC_Trigger_Update_Source  */
  uint32_t Trigger;       /*!< Specifies the event(s) triggering the ADC conversion.
                               This parameter can be a combination of @ref HRTIM_ADC_Trigger_Event  */
} HRTIM_ADCTriggerCfgTypeDef;

/**
  * end of HRTIM_ADCTriggerCfgTypeDef @}
  */

/** @defgroup HRTIM_ExternalEventCfgTypeDef HRTIM ExternalEventCfgTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  External Event Counter A or B configuration definition
  */
typedef struct
{
  uint32_t ResetMode;      /*!< Specifies the External Event Counter A or B Reset Mode.
                                This parameter can be a value of @ref HRTIM_Timer_External_Event_ResetMode  */
  uint32_t Source;         /*!< Specifies the External Event Counter source selection.
                                This parameter can be one of @ref HRTIM_External_Event_Channels  */
  uint32_t Counter;        /*!< Specifies the External Event Counter Threshold.
                                This parameter can be a number between 0x0 and 0x3F  */
} HRTIM_ExternalEventCfgTypeDef;

/**
  * end of HRTIM_ExternalEventCfgTypeDef @}
  */

/** @defgroup HAL_HRTIM_CallbackIDTypeDef HAL HRTIM HAL CallbackIDTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
/**
  * @brief  HAL HRTIM Callback ID enumeration definition
*/
typedef enum {
  HAL_HRTIM_FAULT1CALLBACK_CB_ID               = 0x00U, /*!< Fault 1 interrupt callback ID                         */
  HAL_HRTIM_FAULT2CALLBACK_CB_ID               = 0x01U, /*!< Fault 2 interrupt callback ID                         */
  HAL_HRTIM_FAULT3CALLBACK_CB_ID               = 0x02U, /*!< Fault 3 interrupt callback ID                         */
  HAL_HRTIM_FAULT4CALLBACK_CB_ID               = 0x03U, /*!< Fault 4 interrupt callback ID                         */
  HAL_HRTIM_FAULT5CALLBACK_CB_ID               = 0x04U, /*!< Fault 5 interrupt callback ID                         */
  HAL_HRTIM_SYSTEMFAULTCALLBACK_CB_ID          = 0x05U, /*!< System fault interrupt callback ID                    */
  HAL_HRTIM_DLLCALBRATIONREADYCALLBACK_CB_ID   = 0x06U, /*!< DLL Ready interrupt callback ID                       */
  HAL_HRTIM_BURSTMODEPERIODCALLBACK_CB_ID      = 0x07U, /*!< Burst mode period interrupt callback ID               */
  HAL_HRTIM_SYNCHRONIZATIONEVENTCALLBACK_CB_ID = 0x08U, /*!< Sync Input interrupt callback ID                      */
  HAL_HRTIM_ERRORCALLBACK_CB_ID                = 0x09U, /*!< DMA error callback ID                                 */

  HAL_HRTIM_REGISTERSUPDATECALLBACK_CB_ID      = 0x10U, /*!< Timer x Update interrupt callback ID                  */
  HAL_HRTIM_REPETITIONEVENTCALLBACK_CB_ID      = 0x11U, /*!< Timer x Repetition interrupt callback ID              */
  HAL_HRTIM_COMPARE1EVENTCALLBACK_CB_ID        = 0x12U, /*!< Timer x Compare 1 match interrupt callback ID         */
  HAL_HRTIM_COMPARE2EVENTCALLBACK_CB_ID        = 0x13U, /*!< Timer x Compare 2 match interrupt callback ID         */
  HAL_HRTIM_COMPARE3EVENTCALLBACK_CB_ID        = 0x14U, /*!< Timer x Compare 3 match interrupt callback ID         */
  HAL_HRTIM_COMPARE4EVENTCALLBACK_CB_ID        = 0x15U, /*!< Timer x Compare 4 match interrupt callback ID         */
  HAL_HRTIM_CAPTURE1EVENTCALLBACK_CB_ID        = 0x16U, /*!< Timer x Capture 1 interrupts callback ID              */
  HAL_HRTIM_CAPTURE2EVENTCALLBACK_CB_ID        = 0x17U, /*!< Timer x Capture 2 interrupts callback ID              */
  HAL_HRTIM_DELAYEDPROTECTIONCALLBACK_CB_ID    = 0x18U, /*!< Timer x Delayed protection interrupt callback ID      */
  HAL_HRTIM_COUNTERRESETCALLBACK_CB_ID         = 0x19U, /*!< Timer x counter reset/roll-over interrupt callback ID */
  HAL_HRTIM_OUTPUT1SETCALLBACK_CB_ID           = 0x1AU, /*!< Timer x output 1 set interrupt callback ID            */
  HAL_HRTIM_OUTPUT1RESETCALLBACK_CB_ID         = 0x1BU, /*!< Timer x output 1 reset interrupt callback ID          */
  HAL_HRTIM_OUTPUT2SETCALLBACK_CB_ID           = 0x1CU, /*!< Timer x output 2 set interrupt callback ID            */
  HAL_HRTIM_OUTPUT2RESETCALLBACK_CB_ID         = 0x1DU, /*!< Timer x output 2 reset interrupt callback ID          */
  HAL_HRTIM_BURSTDMATRANSFERCALLBACK_CB_ID     = 0x1EU, /*!< Timer x Burst DMA completed interrupt callback ID     */

  HAL_HRTIM_MSPINIT_CB_ID                      = 0x20U, /*!< HRTIM MspInit callback ID                             */
  HAL_HRTIM_MSPDEINIT_CB_ID                    = 0x21U, /*!< HRTIM MspInit callback ID                             */
  HAL_HRTIM_FAULT6CALLBACK_CB_ID               = 0x22U, /*!< Fault 6 interrupt callback ID                         */
}HAL_HRTIM_CallbackIDTypeDef;

/**
  * end of HAL_HRTIM_CallbackIDTypeDef @}
  */

/** @defgroup pHRTIM_CallbackTypeDef pHRTIM CallbackTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

/**
  * @brief  HAL HRTIM Callback function pointer definitions
  */
typedef void (* pHRTIM_CallbackTypeDef)(HRTIM_HandleTypeDef *hhrtim);       /*!< HRTIM related callback function pointer         */

/**
  * end of pHRTIM_CallbackTypeDef @}
  */

/** @defgroup pHRTIM_TIMxCallbackTypeDef pHRTIM TIMxCallbackTypeDef
  * @ingroup  HRTIM_Structure_Definitions
  * @{
  */

typedef void (* pHRTIM_TIMxCallbackTypeDef)(HRTIM_HandleTypeDef *hhrtim,    /*!< HRTIM Timer x related callback function pointer */
                                            uint32_t TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */

/**
  * end of pHRTIM_TIMxCallbackTypeDef @}
  */

/**
  * end of HRTIM_Structure_Definitions @}
  */

/******************************************************************************/
/*                            HRTIM Parameters                                */
/******************************************************************************/

/* Exported constants --------------------------------------------------------*/

/** @defgroup HRTIM_Parameter_Definitions HRTIM Parameter Definitions
  * @{
  */

/** @defgroup HRTIM_Timer_Index HRTIM Timer Index
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the timer indexes
  *
  */

#define HRTIM_TIMERINDEX_TIMER_A    0x0U   /*!< Index used to access timer A registers */
#define HRTIM_TIMERINDEX_TIMER_B    0x1U   /*!< Index used to access timer B registers */
#define HRTIM_TIMERINDEX_TIMER_C    0x2U   /*!< Index used to access timer C registers */
#define HRTIM_TIMERINDEX_TIMER_D    0x3U   /*!< Index used to access timer D registers */
#define HRTIM_TIMERINDEX_TIMER_E    0x4U   /*!< Index used to access timer E registers */
#define HRTIM_TIMERINDEX_TIMER_F    0x5U   /*!< Index used to access timer F registers */
#define HRTIM_TIMERINDEX_MASTER     0x6U   /*!< Index used to access master registers  */
#define HRTIM_TIMERINDEX_COMMON     0xFFU  /*!< Index used to access HRTIM common registers */

/**
  * end of HRTIM_Timer_Index @}
  */


/** @defgroup HRTIM_Timer_identifier HRTIM Timer identifier
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining timer identifiers
  *
  */

#define HRTIM_TIMERID_MASTER    (HRTIM_MCR_MCEN)   /*!< Master identifier  */
#define HRTIM_TIMERID_TIMER_A   (HRTIM_MCR_TACEN)  /*!< Timer A identifier */
#define HRTIM_TIMERID_TIMER_B   (HRTIM_MCR_TBCEN)  /*!< Timer B identifier */
#define HRTIM_TIMERID_TIMER_C   (HRTIM_MCR_TCCEN)  /*!< Timer C identifier */
#define HRTIM_TIMERID_TIMER_D   (HRTIM_MCR_TDCEN)  /*!< Timer D identifier */
#define HRTIM_TIMERID_TIMER_E   (HRTIM_MCR_TECEN)  /*!< Timer E identifier */
#define HRTIM_TIMERID_TIMER_F   (HRTIM_MCR_TFCEN)  /*!< Timer F identifier */

/**
  * end of HRTIM_Timer_identifier @}
  */

/** @defgroup HRTIM_Compare_Unit HRTIM Compare Unit
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining compare unit identifiers
  *
  */

#define HRTIM_COMPAREUNIT_1    0x00000001U  /*!< Compare unit 1 identifier */
#define HRTIM_COMPAREUNIT_2    0x00000002U  /*!< Compare unit 2 identifier */
#define HRTIM_COMPAREUNIT_3    0x00000004U  /*!< Compare unit 3 identifier */
#define HRTIM_COMPAREUNIT_4    0x00000008U  /*!< Compare unit 4 identifier */
/**
  * end of HRTIM_Compare_Unit @}
  */

/** @defgroup HRTIM_Capture_Unit HRTIM Capture Unit
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining capture unit identifiers
  *
  */

#define HRTIM_CAPTUREUNIT_1 0x00000001U  /*!< Capture unit 1 identifier */
#define HRTIM_CAPTUREUNIT_2 0x00000002U  /*!< Capture unit 2 identifier */
/**
  * end of HRTIM_Capture_Unit @}
  */


/** @defgroup HRTIM_Timer_Output HRTIM Timer Output
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining capture unit identifiers
  *
  */

#define HRTIM_OUTPUT_TA1  0x00000001U  /*!< Timer A - Output 1 identifier */
#define HRTIM_OUTPUT_TA2  0x00000002U  /*!< Timer A - Output 2 identifier */
#define HRTIM_OUTPUT_TB1  0x00000004U  /*!< Timer B - Output 1 identifier */
#define HRTIM_OUTPUT_TB2  0x00000008U  /*!< Timer B - Output 2 identifier */
#define HRTIM_OUTPUT_TC1  0x00000010U  /*!< Timer C - Output 1 identifier */
#define HRTIM_OUTPUT_TC2  0x00000020U  /*!< Timer C - Output 2 identifier */
#define HRTIM_OUTPUT_TD1  0x00000040U  /*!< Timer D - Output 1 identifier */
#define HRTIM_OUTPUT_TD2  0x00000080U  /*!< Timer D - Output 2 identifier */
#define HRTIM_OUTPUT_TE1  0x00000100U  /*!< Timer E - Output 1 identifier */
#define HRTIM_OUTPUT_TE2  0x00000200U  /*!< Timer E - Output 2 identifier */
#define HRTIM_OUTPUT_TF1  0x00000400U  /*!< Timer F - Output 1 identifier */
#define HRTIM_OUTPUT_TF2  0x00000800U  /*!< Timer F - Output 2 identifier */
/**
  * end of HRTIM_Timer_Output @}
  */

/** @defgroup HRTIM_ADC_Trigger HRTIM ADC Trigger
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining ADC triggers identifiers
  *
  */

#define HRTIM_ADCTRIGGER_1  0x00000001U  /*!< ADC trigger 1 identifier */
#define HRTIM_ADCTRIGGER_2  0x00000002U  /*!< ADC trigger 2 identifier */
#define HRTIM_ADCTRIGGER_3  0x00000004U  /*!< ADC trigger 3 identifier */
#define HRTIM_ADCTRIGGER_4  0x00000008U  /*!< ADC trigger 4 identifier */
/**
  * end of HRTIM_ADC_Trigger @}
  */


/** @defgroup HRTIM_ADC_Ext_Trigger HRTIM ADC Extended Trigger
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining ADC Extended triggers identifiers
  *
  */

#define HRTIM_ADCTRIGGER_5  0x00000010U  /*!< ADC trigger 5 identifier  */
#define HRTIM_ADCTRIGGER_6  0x00000020U  /*!< ADC trigger 6 identifier  */
#define HRTIM_ADCTRIGGER_7  0x00000040U  /*!< ADC trigger 7 identifier  */
#define HRTIM_ADCTRIGGER_8  0x00000080U  /*!< ADC trigger 8 identifier  */
#define HRTIM_ADCTRIGGER_9  0x00000100U  /*!< ADC trigger 9 identifier  */
#define HRTIM_ADCTRIGGER_10 0x00000200U  /*!< ADC trigger 10 identifier */

#define IS_HRTIM_ADCTRIGGER(ADCTRIGGER)\
                                          (((ADCTRIGGER) == HRTIM_ADCTRIGGER_1)   || \
                                           ((ADCTRIGGER) == HRTIM_ADCTRIGGER_2)   || \
                                           ((ADCTRIGGER) == HRTIM_ADCTRIGGER_3)   || \
                                           ((ADCTRIGGER) == HRTIM_ADCTRIGGER_4)   || \
                                           ((ADCTRIGGER) == HRTIM_ADCTRIGGER_5)   || \
                                           ((ADCTRIGGER) == HRTIM_ADCTRIGGER_6)   || \
                                           ((ADCTRIGGER) == HRTIM_ADCTRIGGER_7)   || \
                                           ((ADCTRIGGER) == HRTIM_ADCTRIGGER_8)   || \
                                           ((ADCTRIGGER) == HRTIM_ADCTRIGGER_9)   || \
                                           ((ADCTRIGGER) == HRTIM_ADCTRIGGER_10))

#define IS_HRTIM_ADCEXTTRIGGER(ADCTRIGGER)\
                                           (((ADCTRIGGER) == HRTIM_ADCTRIGGER_5)   || \
                                            ((ADCTRIGGER) == HRTIM_ADCTRIGGER_6)   || \
                                            ((ADCTRIGGER) == HRTIM_ADCTRIGGER_7)   || \
                                            ((ADCTRIGGER) == HRTIM_ADCTRIGGER_8)   || \
                                            ((ADCTRIGGER) == HRTIM_ADCTRIGGER_9)   || \
                                            ((ADCTRIGGER) == HRTIM_ADCTRIGGER_10))
/**
  * end of HRTIM_ADC_Ext_Trigger @}
  */

/** @defgroup HRTIM_External_Event_Channels HRTIM External Event Channels
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining external event channel identifiers
  *
  */

#define HRTIM_EVENT_NONE    (0x00000000U)     /*!< Undefined event channel */

#define HRTIM_EVENT_1       (0x00000001U)     /*!< External event channel 1  identifier */
#define HRTIM_EVENT_2       (0x00000002U)     /*!< External event channel 2  identifier */
#define HRTIM_EVENT_3       (0x00000003U)     /*!< External event channel 3  identifier */
#define HRTIM_EVENT_4       (0x00000004U)     /*!< External event channel 4  identifier */
#define HRTIM_EVENT_5       (0x00000005U)     /*!< External event channel 5  identifier */
#define HRTIM_EVENT_6       (0x00000006U)     /*!< External event channel 6  identifier */
#define HRTIM_EVENT_7       (0x00000007U)     /*!< External event channel 7  identifier */
#define HRTIM_EVENT_8       (0x00000008U)     /*!< External event channel 8  identifier */
#define HRTIM_EVENT_9       (0x00000009U)     /*!< External event channel 9  identifier */
#define HRTIM_EVENT_10      (0x0000000AU)     /*!< External event channel 10 identifier */
/**
  * end of HRTIM_ADC_Ext_Trigger @}
  */

/** @defgroup HRTIM_Fault_Channel HRTIM Fault Channel
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining fault channel identifiers
  *
  */

#define HRTIM_FAULT_1      (0x01U)     /*!< Fault channel 1 identifier */
#define HRTIM_FAULT_2      (0x02U)     /*!< Fault channel 2 identifier */
#define HRTIM_FAULT_3      (0x04U)     /*!< Fault channel 3 identifier */
#define HRTIM_FAULT_4      (0x08U)     /*!< Fault channel 4 identifier */
#define HRTIM_FAULT_5      (0x10U)     /*!< Fault channel 5 identifier */
#define HRTIM_FAULT_6      (0x20U)     /*!< Fault channel 6 identifier */

/**
  * end of HRTIM_Fault_Channel @}
  */

/** @defgroup HRTIM_Prescaler_Ratio HRTIM Prescaler Ratio
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining timer high-resolution clock prescaler ratio.
  *
  */

#define HRTIM_PRESCALERRATIO_MUL32    (0x00000000U)  /*!< fHRCK: fHRTIM x 32U = 4.608 GHz - Resolution: 217 ps - Min PWM frequency: 70.3 kHz (fHRTIM=144MHz) */
#define HRTIM_PRESCALERRATIO_MUL16    (0x00000001U)  /*!< fHRCK: fHRTIM x 16U = 2.304 GHz - Resolution: 434 ps - Min PWM frequency: 35.1 KHz (fHRTIM=144MHz) */
#define HRTIM_PRESCALERRATIO_MUL8     (0x00000002U)  /*!< fHRCK: fHRTIM x 8U = 1.152 GHz - Resolution: 868 ps - Min PWM frequency: 17.6 kHz (fHRTIM=144MHz)  */
#define HRTIM_PRESCALERRATIO_MUL4     (0x00000003U)  /*!< fHRCK: fHRTIM x 4U = 576 MHz - Resolution: 1.73 ns - Min PWM frequency: 8.8 kHz (fHRTIM=144MHz)    */
#define HRTIM_PRESCALERRATIO_MUL2     (0x00000004U)  /*!< fHRCK: fHRTIM x 2U = 288 MHz - Resolution: 3.47 ns - Min PWM frequency: 4.4 kHz (fHRTIM=144MHz)    */
#define HRTIM_PRESCALERRATIO_DIV1     (0x00000005U)  /*!< fHRCK: fHRTIM = 144 MHz - Resolution: 6.95 ns - Min PWM frequency: 2.2 kHz (fHRTIM=144MHz)         */
#define HRTIM_PRESCALERRATIO_DIV2     (0x00000006U)  /*!< fHRCK: fHRTIM / 2U = 72 MHz - Resolution: 13.88 ns- Min PWM frequency: 1.1 kHz (fHRTIM=144MHz)     */
#define HRTIM_PRESCALERRATIO_DIV4     (0x00000007U)  /*!< fHRCK: fHRTIM / 4U = 36 MHz - Resolution: 27.7 ns- Min PWM frequency: 550Hz (fHRTIM=144MHz)        */

/**
  * end of HRTIM_Prescaler_Ratio @}
  */

/** @defgroup HRTIM_Counter_Operating_Mode HRTIM Counter Operating Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining timer counter operating mode.
  *
  */

#define HRTIM_MODE_CONTINUOUS               (0x00000008U)  /*!< The timer operates in continuous (free-running) mode */
#define HRTIM_MODE_SINGLESHOT               (0x00000000U)  /*!< The timer operates in non retriggerable single-shot mode */
#define HRTIM_MODE_SINGLESHOT_RETRIGGERABLE (0x00000010U)  /*!< The timer operates in retriggerable single-shot mode */

/**
  * end of HRTIM_Counter_Operating_Mode @}
  */

/** @defgroup HRTIM_Half_Mode_Enable HRTIM Half Mode Enable
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining half mode enabling status
  *
  */

#define HRTIM_HALFMODE_DISABLED (0x00000000U)  /*!< Half mode is disabled */
#define HRTIM_HALFMODE_ENABLED  (0x00000020U)  /*!< Half mode is enabled */

/**
  * end of HRTIM_Half_Mode_Enable @}
  */

/** @defgroup HRTIM_Interleaved_Mode HRTIM Interleaved Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining interleaved mode enabling status.
  *
  */

#define HRTIM_INTERLEAVED_MODE_DISABLED      0x000U               /*!< HRTIM interleaved Mode is disabled */
#define HRTIM_INTERLEAVED_MODE_DUAL          0x002U               /*!< HRTIM interleaved Mode is Half */
#define HRTIM_INTERLEAVED_MODE_TRIPLE        0x003U               /*!< HRTIM interleaved Mode is Triple */
#define HRTIM_INTERLEAVED_MODE_QUAD          0x004U               /*!< HRTIM interleaved Mode is Quad */

/**
  * end of HRTIM_Interleaved_Mode @}
  */

/** @defgroup HRTIM_Start_On_Sync_Input_Event HRTIM Start On Sync Input Event
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the timer behavior following the synchronization event
  *
  */

#define HRTIM_SYNCSTART_DISABLED    (0x00000000U)           /*!< Synchronization input event has effect on the timer */
#define HRTIM_SYNCSTART_ENABLED     (HRTIM_MCR_SYNCSTRTM)   /*!< Synchronization input event starts the timer */

/**
  * end of HRTIM_Start_On_Sync_Input_Event @}
  */

/** @defgroup HRTIM_Reset_On_Sync_Input_Event HRTIM Reset On Sync Input Event
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the timer behavior following the synchronization event
  *
  */

#define HRTIM_SYNCRESET_DISABLED    (0x00000000U)           /*!< Synchronization input event has effect on the timer */
#define HRTIM_SYNCRESET_ENABLED     (HRTIM_MCR_SYNCRSTM)    /*!< Synchronization input event resets the timer */

/**
  * end of HRTIM_Reset_On_Sync_Input_Event @}
  */

/** @defgroup HRTIM_DAC_Synchronization HRTIM DAC Synchronization
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining on which output the DAC synchronization event is sent
  *
  */

#define HRTIM_DACSYNC_NONE          0x00000000U                                          /*!< No DAC synchronization event generated */
#define HRTIM_DACSYNC_DACTRIGOUT_1  (                         HRTIM_MCR_DACSYNC_BIT0)    /*!< DAC synchronization event generated on DACTrigOut1 output upon timer update */
#define HRTIM_DACSYNC_DACTRIGOUT_2  (HRTIM_MCR_DACSYNC_BIT1                         )    /*!< DAC synchronization event generated on DACTrigOut2 output upon timer update */
#define HRTIM_DACSYNC_DACTRIGOUT_3  (HRTIM_MCR_DACSYNC_BIT1 | HRTIM_MCR_DACSYNC_BIT0)    /*!< DAC update generated on DACTrigOut3 output upon timer update */

/**
  * end of HRTIM_DAC_Synchronization @}
  */

/** @defgroup HRTIM_Register_Preload_Enable HRTIM Register Preload Enable
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether a write access into a preloadable
  *        register is done into the active or the preload register.
  *
  */

#define HRTIM_PRELOAD_DISABLED    (0x00000000U)           /*!< Preload disabled: the write access is directly done into the active register */
#define HRTIM_PRELOAD_ENABLED     (HRTIM_MCR_PREEN)       /*!< Preload enabled: the write access is done into the preload register */

/**
  * end of HRTIM_Register_Preload_Enable @}
  */


/** @defgroup HRTIM_Update_Gating HRTIM Update Gating
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining how the update occurs relatively to the burst DMA
  *        transaction and the external update request on update enable inputs 1 to 3.
  *
  */

#define HRTIM_UPDATEGATING_INDEPENDENT      0x00000000U                                                                                                /*!< Update done independently from the DMA burst transfer completion */
#define HRTIM_UPDATEGATING_DMABURST         (                                                                              HRTIM_TIMCR_UPDGAT_BIT0)    /*!< Update done when the DMA burst transfer is completed */
#define HRTIM_UPDATEGATING_DMABURST_UPDATE  (                                                    HRTIM_TIMCR_UPDGAT_BIT1                          )    /*!< Update done on timer roll-over following a DMA burst transfer completion*/
#define HRTIM_UPDATEGATING_UPDEN1           (                                                    HRTIM_TIMCR_UPDGAT_BIT1 | HRTIM_TIMCR_UPDGAT_BIT0)    /*!< Slave timer only - Update done on a rising edge of HRTIM update enable input 1U */
#define HRTIM_UPDATEGATING_UPDEN2           (                          HRTIM_TIMCR_UPDGAT_BIT2                                                    )    /*!< Slave timer only - Update done on a rising edge of HRTIM update enable input 2U */
#define HRTIM_UPDATEGATING_UPDEN3           (                          HRTIM_TIMCR_UPDGAT_BIT2                           | HRTIM_TIMCR_UPDGAT_BIT0)    /*!< Slave timer only - Update done on a rising edge of HRTIM update enable input 3U */
#define HRTIM_UPDATEGATING_UPDEN1_UPDATE    (                          HRTIM_TIMCR_UPDGAT_BIT2 | HRTIM_TIMCR_UPDGAT_BIT1                          )    /*!< Slave timer only -  Update done on the update event following a rising edge of HRTIM update enable input 1U */
#define HRTIM_UPDATEGATING_UPDEN2_UPDATE    (                          HRTIM_TIMCR_UPDGAT_BIT2 | HRTIM_TIMCR_UPDGAT_BIT1 | HRTIM_TIMCR_UPDGAT_BIT0)    /*!< Slave timer only -  Update done on the update event following a rising edge of HRTIM update enable input 2U */
#define HRTIM_UPDATEGATING_UPDEN3_UPDATE    (HRTIM_TIMCR_UPDGAT_BIT3                                                                              )    /*!< Slave timer only -  Update done on the update event following a rising edge of HRTIM update enable input 3U */

/**
  * end of HRTIM_Update_Gating @}
  */

/** @defgroup HRTIM_Timer_Burst_Mode HRTIM Timer Burst Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining how the timer behaves during a burst
  *        mode operation.
  *
  */

#define HRTIM_TIMERBURSTMODE_MAINTAINCLOCK 0x00000000U           /*!< Timer counter clock is maintained and the timer operates normally */
#define HRTIM_TIMERBURSTMODE_RESETCOUNTER  (HRTIM_BMCR_MTBM)     /*!< Timer counter clock is stopped and the counter is reset */
/**
  * end of HRTIM_Timer_Burst_Mode @}
  */


/** @defgroup HRTIM_Timer_UpDown_Mode HRTIM Timer UpDown Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining how the timer counter operates
  *
  */

#define HRTIM_TIMERUPDOWNMODE_UP           0x00000000U           /*!< Timer counter is operating in up-counting mode */
#define HRTIM_TIMERUPDOWNMODE_UPDOWN       0x00000001U           /*!< Timer counter is operating in up-down counting mode */
/**
  * end of HRTIM_Timer_UpDown_Mode @}
  */


/** @defgroup HRTIM_Timer_TrigHalf_Mode HRTIM Timer Triggered-Half Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining how the timer counter operates
  *
  */

#define HRTIM_TIMERTRIGHALF_DISABLED       0x00000000U           /*!< Timer Compare 2 register is behaving in standard mode */
#define HRTIM_TIMERTRIGHALF_ENABLED        (HRTIM_TIMCR2_TRGHLF) /*!< Timer Compare 2 register is behaving in triggered-half mode */
/**
  * end of HRTIM_Timer_TrigHalf_Mode @}
  */


/** @defgroup HRTIM_Timer_GreaterCMP3_Mode HRTIM Timer Greater than Compare 3 PWM Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining how the timer compare operates
  *
  */

#define HRTIM_TIMERGTCMP3_EQUAL            0x00000000U           /*!< Timer Compare 3 event is generated when counter is equal */
#define HRTIM_TIMERGTCMP3_GREATER          (HRTIM_TIMCR2_GTCMP3) /*!< Timer Compare 3 Reset event is generated when counter is greater */
/**
  * end of HRTIM_Timer_GreaterCMP3_Mode @}
  */


/** @defgroup HRTIM_Timer_GreaterCMP1_Mode HRTIM Timer Greater than Compare 1 PWM Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining how the timer compare operates
  *
  */

#define HRTIM_TIMERGTCMP1_EQUAL            0x00000000U           /*!< Timer Compare 1 event is generated when counter is equal */
#define HRTIM_TIMERGTCMP1_GREATER          (HRTIM_TIMCR2_GTCMP1) /*!< Timer Compare 1 event is generated when counter is greater */
/**
  * end of HRTIM_Timer_GreaterCMP1_Mode @}
  */


/** @defgroup HRTIM_Timer_DualChannelDac_Reset HRTIM Dual Channel Dac Reset Trigger
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining when the hrtim_dac_reset_trgx trigger is generated
  *
  */

#define HRTIM_TIMER_DCDR_COUNTER           0x00000000U           /*!< the trigger is generated on counter reset or roll-over event */
#define HRTIM_TIMER_DCDR_OUT1SET           (HRTIM_TIMCR2_DCDR)   /*!< the trigger is generated on output 1 set event */
/**
  * end of HRTIM_Timer_DualChannelDac_Reset @}
  */


/** @defgroup HRTIM_Timer_DualChannelDac_Step HRTIM Dual Channel Dac Step Trigger
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining when the hrtim_dac_step_trgx trigger is generated
  *        is generated
  *
  */

#define HRTIM_TIMER_DCDS_CMP2              0x00000000U           /*!< the trigger is generated on compare 2 event */
#define HRTIM_TIMER_DCDS_OUT1RST           (HRTIM_TIMCR2_DCDS)   /*!< the trigger is generated on output 1 reset event */
/**
  * end of HRTIM_Timer_DualChannelDac_Step @}
  */


/** @defgroup HRTIM_Timer_DualChannelDac_Enable HRTIM Dual Channel DAC Trigger Enable
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants enabling the dual channel DAC triggering mechanism
  *
  */

#define HRTIM_TIMER_DCDE_DISABLED          0x00000000U           /*!< the Dual channel DAC trigger is disabled */
#define HRTIM_TIMER_DCDE_ENABLED           (HRTIM_TIMCR2_DCDE)   /*!< the Dual channel DAC trigger is enabled */
/**
  * end of HRTIM_Timer_DualChannelDac_Enable @}
  */


/** @defgroup HRTIM_Timer_Repetition_Update HRTIM Timer Repetition Update
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether registers are updated when the timer
  *        repetition period is completed (either due to roll-over or
  *        reset events)
  *
  */

#define HRTIM_UPDATEONREPETITION_DISABLED  0x00000000U           /*!< Update on repetition disabled */
#define HRTIM_UPDATEONREPETITION_ENABLED   (HRTIM_MCR_MREPU)     /*!< Update on repetition enabled */
/**
  * end of HRTIM_Timer_Repetition_Update @}
  */



/** @defgroup HRTIM_Timer_Push_Pull_Mode HRTIM Timer Push Pull Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not the push-pull mode is enabled for
  *        a timer.
  *
  */

#define HRTIM_TIMPUSHPULLMODE_DISABLED     0x00000000U           /*!< Push-Pull mode disabled */
#define HRTIM_TIMPUSHPULLMODE_ENABLED      (HRTIM_TIMCR_PSHPLL)  /*!< Push-Pull mode enabled */
/**
  * end of HRTIM_Timer_Push_Pull_Mode @}
  */

/** @defgroup HRTIM_Timer_Fault_Enabling HRTIM Timer Fault Enabling
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether a fault channel is enabled for a timer
  *
  */

#define HRTIM_TIMFAULTENABLE_NONE     0x00000000U           /*!< No fault enabled */
#define HRTIM_TIMFAULTENABLE_FAULT1   (HRTIM_FLTR_FLT1EN)   /*!< Fault 1 enabled */
#define HRTIM_TIMFAULTENABLE_FAULT2   (HRTIM_FLTR_FLT2EN)   /*!< Fault 2 enabled */
#define HRTIM_TIMFAULTENABLE_FAULT3   (HRTIM_FLTR_FLT3EN)   /*!< Fault 3 enabled */
#define HRTIM_TIMFAULTENABLE_FAULT4   (HRTIM_FLTR_FLT4EN)   /*!< Fault 4 enabled */
#define HRTIM_TIMFAULTENABLE_FAULT5   (HRTIM_FLTR_FLT5EN)   /*!< Fault 5 enabled */
#define HRTIM_TIMFAULTENABLE_FAULT6   (HRTIM_FLTR_FLT6EN)   /*!< Fault 6 enabled */
/**
  * end of HRTIM_Timer_Fault_Enabling @}
  */


/** @defgroup HRTIM_Timer_Fault_Lock HRTIM Timer Fault Lock
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not fault enabling bits are write
  *        protected for a timer
  *
  */

#define HRTIM_TIMFAULTLOCK_READWRITE    (0x00000000U)           /*!< Timer fault enabling bits are read/write */
#define HRTIM_TIMFAULTLOCK_READONLY     (HRTIM_FLTR_FLTLCK)     /*!< Timer fault enabling bits are read only */
/**
  * end of HRTIM_Timer_Fault_Lock @}
  */


/** @defgroup HRTIM_Timer_Deadtime_Insertion HRTIM Timer Dead-time Insertion
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not fault the dead time insertion
  *        feature is enabled for a timer
  *
  */

#define HRTIM_TIMDEADTIMEINSERTION_DISABLED   (0x00000000U)           /*!< Output 1 and output 2 signals are independent */
#define HRTIM_TIMDEADTIMEINSERTION_ENABLED    HRTIM_OUTR_DTEN         /*!< Dead-time is inserted between output 1 and output 2U */
/**
  * end of HRTIM_Timer_Deadtime_Insertion @}
  */


/** @defgroup HRTIM_Timer_Delayed_Protection_Mode HRTIM Timer Delayed Protection Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining all possible delayed protection modes
  *        for a timer. Also define the source and outputs on which the delayed
  *        protection schemes are applied
  *
  */

#define HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED          (0x00000000U)                                                                           /*!< No action */
#define HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DELAYEDOUT1_EEV6  (HRTIM_OUTR_DLYPRTEN)                                                                   /*!< Timers A, B, C: Output 1 delayed Idle on external Event 6U */
#define HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DELAYEDOUT2_EEV6  (HRTIM_OUTR_DLYPRT_BIT0 | HRTIM_OUTR_DLYPRTEN)                                             /*!< Timers A, B, C: Output 2 delayed Idle on external Event 6U */
#define HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DELAYEDBOTH_EEV6  (HRTIM_OUTR_DLYPRT_BIT1 | HRTIM_OUTR_DLYPRTEN)                                             /*!< Timers A, B, C: Output 1 and output 2 delayed Idle on external Event 6U */
#define HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_BALANCED_EEV6     (HRTIM_OUTR_DLYPRT_BIT1 | HRTIM_OUTR_DLYPRT_BIT0 | HRTIM_OUTR_DLYPRTEN)                       /*!< Timers A, B, C: Balanced Idle on external Event 6U */
#define HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DELAYEDOUT1_DEEV7 (HRTIM_OUTR_DLYPRT_BIT2 | HRTIM_OUTR_DLYPRTEN)                                             /*!< Timers A, B, C: Output 1 delayed Idle on external Event 7U */
#define HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DELAYEDOUT2_DEEV7 (HRTIM_OUTR_DLYPRT_BIT2 | HRTIM_OUTR_DLYPRT_BIT0 | HRTIM_OUTR_DLYPRTEN)                       /*!< Timers A, B, C: Output 2 delayed Idle on external Event 7U */
#define HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DELAYEDBOTH_EEV7  (HRTIM_OUTR_DLYPRT_BIT2 | HRTIM_OUTR_DLYPRT_BIT1 | HRTIM_OUTR_DLYPRTEN)                       /*!< Timers A, B, C: Output 1 and output2 delayed Idle on external Event 7U */
#define HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_BALANCED_EEV7     (HRTIM_OUTR_DLYPRT_BIT2 | HRTIM_OUTR_DLYPRT_BIT1 | HRTIM_OUTR_DLYPRT_BIT0 | HRTIM_OUTR_DLYPRTEN) /*!< Timers A, B, C: Balanced Idle on external Event 7U */

#define HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED            (0x00000000U)                                                                             /*!< No action */
#define HRTIM_TIMER_D_E_DELAYEDPROTECTION_DELAYEDOUT1_EEV8    (HRTIM_OUTR_DLYPRTEN)                                                                     /*!< Timers D, E: Output 1 delayed Idle on external Event 6U */
#define HRTIM_TIMER_D_E_DELAYEDPROTECTION_DELAYEDOUT2_EEV8    (HRTIM_OUTR_DLYPRT_BIT0 | HRTIM_OUTR_DLYPRTEN)                                               /*!< Timers D, E: Output 2 delayed Idle on external Event 6U */
#define HRTIM_TIMER_D_E_DELAYEDPROTECTION_DELAYEDBOTH_EEV8    (HRTIM_OUTR_DLYPRT_BIT1 | HRTIM_OUTR_DLYPRTEN)                                               /*!< Timers D, E: Output 1 and output 2 delayed Idle on external Event 6U */
#define HRTIM_TIMER_D_E_DELAYEDPROTECTION_BALANCED_EEV8       (HRTIM_OUTR_DLYPRT_BIT1 | HRTIM_OUTR_DLYPRT_BIT0 | HRTIM_OUTR_DLYPRTEN)                         /*!< Timers D, E: Balanced Idle on external Event 6U */
#define HRTIM_TIMER_D_E_DELAYEDPROTECTION_DELAYEDOUT1_DEEV9   (HRTIM_OUTR_DLYPRT_BIT2 | HRTIM_OUTR_DLYPRTEN)                                               /*!< Timers D, E: Output 1 delayed Idle on external Event 7U */
#define HRTIM_TIMER_D_E_DELAYEDPROTECTION_DELAYEDOUT2_DEEV9   (HRTIM_OUTR_DLYPRT_BIT2 | HRTIM_OUTR_DLYPRT_BIT0 | HRTIM_OUTR_DLYPRTEN)                         /*!< Timers D, E: Output 2 delayed Idle on external Event 7U */
#define HRTIM_TIMER_D_E_DELAYEDPROTECTION_DELAYEDBOTH_EEV9    (HRTIM_OUTR_DLYPRT_BIT2 | HRTIM_OUTR_DLYPRT_BIT1 | HRTIM_OUTR_DLYPRTEN)                         /*!< Timers D, E: Output 1 and output2 delayed Idle on external Event 7U */
#define HRTIM_TIMER_D_E_DELAYEDPROTECTION_BALANCED_EEV9       (HRTIM_OUTR_DLYPRT_BIT2 | HRTIM_OUTR_DLYPRT_BIT1 | HRTIM_OUTR_DLYPRT_BIT0 | HRTIM_OUTR_DLYPRTEN)   /*!< Timers D, E: Balanced Idle on external Event 7U */

#define HRTIM_TIMER_F_DELAYEDPROTECTION_DISABLED              (0x00000000U)                                                                             /*!< No action */
#define HRTIM_TIMER_F_DELAYEDPROTECTION_DELAYEDOUT1_EEV8      (HRTIM_OUTR_DLYPRTEN)                                                                     /*!< Timers F: Output 1 delayed Idle on external Event 6U */
#define HRTIM_TIMER_F_DELAYEDPROTECTION_DELAYEDOUT2_EEV8      (HRTIM_OUTR_DLYPRT_BIT0 | HRTIM_OUTR_DLYPRTEN)                                               /*!< Timers F: Output 2 delayed Idle on external Event 6U */
#define HRTIM_TIMER_F_DELAYEDPROTECTION_DELAYEDBOTH_EEV8      (HRTIM_OUTR_DLYPRT_BIT1 | HRTIM_OUTR_DLYPRTEN)                                               /*!< Timers F: Output 1 and output 2 delayed Idle on external Event 6U */
#define HRTIM_TIMER_F_DELAYEDPROTECTION_BALANCED_EEV8         (HRTIM_OUTR_DLYPRT_BIT1 | HRTIM_OUTR_DLYPRT_BIT0 | HRTIM_OUTR_DLYPRTEN)                         /*!< Timers F: Balanced Idle on external Event 6U */
#define HRTIM_TIMER_F_DELAYEDPROTECTION_DELAYEDOUT1_DEEV9     (HRTIM_OUTR_DLYPRT_BIT2 | HRTIM_OUTR_DLYPRTEN)                                               /*!< Timers F: Output 1 delayed Idle on external Event 7U */
#define HRTIM_TIMER_F_DELAYEDPROTECTION_DELAYEDOUT2_DEEV9     (HRTIM_OUTR_DLYPRT_BIT2 | HRTIM_OUTR_DLYPRT_BIT0 | HRTIM_OUTR_DLYPRTEN)                         /*!< Timers F: Output 2 delayed Idle on external Event 7U */
#define HRTIM_TIMER_F_DELAYEDPROTECTION_DELAYEDBOTH_EEV9      (HRTIM_OUTR_DLYPRT_BIT2 | HRTIM_OUTR_DLYPRT_BIT1 | HRTIM_OUTR_DLYPRTEN)                         /*!< Timers F: Output 1 and output2 delayed Idle on external Event 7U */
#define HRTIM_TIMER_F_DELAYEDPROTECTION_BALANCED_EEV9         (HRTIM_OUTR_DLYPRT_BIT2 | HRTIM_OUTR_DLYPRT_BIT1 | HRTIM_OUTR_DLYPRT_BIT0 | HRTIM_OUTR_DLYPRTEN)   /*!< Timers F: Balanced Idle on external Event 7U */
/**
  * end of HRTIM_Timer_Delayed_Protection_Mode @}
  */


/** @defgroup HRTIM_Timer_Update_Trigger HRTIM Timer Update Trigger
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether the registers update is done synchronously
  *        with any other timer or master update
  *
  */

#define HRTIM_TIMUPDATETRIGGER_NONE     0x00000000U          /*!< Register update is disabled */
#define HRTIM_TIMUPDATETRIGGER_MASTER   (HRTIM_TIMCR_MSTU)   /*!< Register update is triggered by the master timer update */
#define HRTIM_TIMUPDATETRIGGER_TIMER_A  (HRTIM_TIMCR_TAU)    /*!< Register update is triggered by the timer A update */
#define HRTIM_TIMUPDATETRIGGER_TIMER_B  (HRTIM_TIMCR_TBU)    /*!< Register update is triggered by the timer B update */
#define HRTIM_TIMUPDATETRIGGER_TIMER_C  (HRTIM_TIMCR_TCU)    /*!< Register update is triggered by the timer C update*/
#define HRTIM_TIMUPDATETRIGGER_TIMER_D  (HRTIM_TIMCR_TDU)    /*!< Register update is triggered by the timer D update */
#define HRTIM_TIMUPDATETRIGGER_TIMER_E  (HRTIM_TIMCR_TEU)    /*!< Register update is triggered by the timer E update */
#define HRTIM_TIMUPDATETRIGGER_TIMER_F  (HRTIM_TIMCR_TFU)    /*!< Register update is triggered by the timer F update */
/**
  * end of HRTIM_Timer_Update_Trigger @}
  */


/** @defgroup HRTIM_Timer_Reset_Trigger HRTIM Timer Reset Trigger
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the events that can be selected to trigger the reset
  *        of the timer counter
  *
  */

#define HRTIM_TIMRESETTRIGGER_NONE        0x00000000U            /*!< No counter reset trigger */
#define HRTIM_TIMRESETTRIGGER_UPDATE      (HRTIM_RSTR_UPDATE)    /*!< The timer counter is reset upon update event */
#define HRTIM_TIMRESETTRIGGER_CMP2        (HRTIM_RSTR_CMP2)      /*!< The timer counter is reset upon Timer Compare 2 event */
#define HRTIM_TIMRESETTRIGGER_CMP4        (HRTIM_RSTR_CMP4)      /*!< The timer counter is reset upon Timer Compare 4 event */
#define HRTIM_TIMRESETTRIGGER_MASTER_PER  (HRTIM_RSTR_MSTPER)    /*!< The timer counter is reset upon master timer period event */
#define HRTIM_TIMRESETTRIGGER_MASTER_CMP1 (HRTIM_RSTR_MSTCMP1)   /*!< The timer counter is reset upon master timer Compare 1 event */
#define HRTIM_TIMRESETTRIGGER_MASTER_CMP2 (HRTIM_RSTR_MSTCMP2)   /*!< The timer counter is reset upon master timer Compare 2 event */
#define HRTIM_TIMRESETTRIGGER_MASTER_CMP3 (HRTIM_RSTR_MSTCMP3)   /*!< The timer counter is reset upon master timer Compare 3 event */
#define HRTIM_TIMRESETTRIGGER_MASTER_CMP4 (HRTIM_RSTR_MSTCMP4)   /*!< The timer counter is reset upon master timer Compare 4 event */
#define HRTIM_TIMRESETTRIGGER_EEV_1       (HRTIM_RSTR_EXTEVNT1)  /*!< The timer counter is reset upon external event 1U */
#define HRTIM_TIMRESETTRIGGER_EEV_2       (HRTIM_RSTR_EXTEVNT2)  /*!< The timer counter is reset upon external event 2U */
#define HRTIM_TIMRESETTRIGGER_EEV_3       (HRTIM_RSTR_EXTEVNT3)  /*!< The timer counter is reset upon external event 3U */
#define HRTIM_TIMRESETTRIGGER_EEV_4       (HRTIM_RSTR_EXTEVNT4)  /*!< The timer counter is reset upon external event 4U */
#define HRTIM_TIMRESETTRIGGER_EEV_5       (HRTIM_RSTR_EXTEVNT5)  /*!< The timer counter is reset upon external event 5U */
#define HRTIM_TIMRESETTRIGGER_EEV_6       (HRTIM_RSTR_EXTEVNT6)  /*!< The timer counter is reset upon external event 6U */
#define HRTIM_TIMRESETTRIGGER_EEV_7       (HRTIM_RSTR_EXTEVNT7)  /*!< The timer counter is reset upon external event 7U */
#define HRTIM_TIMRESETTRIGGER_EEV_8       (HRTIM_RSTR_EXTEVNT8)  /*!< The timer counter is reset upon external event 8U */
#define HRTIM_TIMRESETTRIGGER_EEV_9       (HRTIM_RSTR_EXTEVNT9)  /*!< The timer counter is reset upon external event 9U */
#define HRTIM_TIMRESETTRIGGER_EEV_10      (HRTIM_RSTR_EXTEVNT10) /*!< The timer counter is reset upon external event 10U */
#define HRTIM_TIMRESETTRIGGER_OTHER1_CMP1 (HRTIM_RSTR_TIMBCMP1)  /*!< The timer counter is reset upon other timer Compare 1 event */
#define HRTIM_TIMRESETTRIGGER_OTHER1_CMP2 (HRTIM_RSTR_TIMBCMP2)  /*!< The timer counter is reset upon other timer Compare 2 event */
#define HRTIM_TIMRESETTRIGGER_OTHER1_CMP4 (HRTIM_RSTR_TIMBCMP4)  /*!< The timer counter is reset upon other timer Compare 4 event */
#define HRTIM_TIMRESETTRIGGER_OTHER2_CMP1 (HRTIM_RSTR_TIMCCMP1)  /*!< The timer counter is reset upon other timer Compare 1 event */
#define HRTIM_TIMRESETTRIGGER_OTHER2_CMP2 (HRTIM_RSTR_TIMCCMP2)  /*!< The timer counter is reset upon other timer Compare 2 event */
#define HRTIM_TIMRESETTRIGGER_OTHER2_CMP4 (HRTIM_RSTR_TIMCCMP4)  /*!< The timer counter is reset upon other timer Compare 4 event */
#define HRTIM_TIMRESETTRIGGER_OTHER3_CMP1 (HRTIM_RSTR_TIMDCMP1)  /*!< The timer counter is reset upon other timer Compare 1 event */
#define HRTIM_TIMRESETTRIGGER_OTHER3_CMP2 (HRTIM_RSTR_TIMDCMP2)  /*!< The timer counter is reset upon other timer Compare 2 event */
#define HRTIM_TIMRESETTRIGGER_OTHER3_CMP4 (HRTIM_RSTR_TIMDCMP4)  /*!< The timer counter is reset upon other timer Compare 4 event */
#define HRTIM_TIMRESETTRIGGER_OTHER4_CMP1 (HRTIM_RSTR_TIMECMP1)  /*!< The timer counter is reset upon other timer Compare 1 event */
#define HRTIM_TIMRESETTRIGGER_OTHER4_CMP2 (HRTIM_RSTR_TIMECMP2)  /*!< The timer counter is reset upon other timer Compare 2 event */
#define HRTIM_TIMRESETTRIGGER_OTHER4_CMP4 (HRTIM_RSTR_TIMECMP4)  /*!< The timer counter is reset upon other timer Compare 4 event */
#define HRTIM_TIMRESETTRIGGER_OTHER5_CMP1 (HRTIM_RSTR_TIMFCMP1)  /*!< The timer counter is reset upon other timer Compare 1 event */
#define HRTIM_TIMRESETTRIGGER_OTHER5_CMP2 (HRTIM_RSTR_TIMFCMP2)  /*!< The timer counter is reset upon other timer Compare 2 event */
/**
  * end of HRTIM_Timer_Reset_Trigger @}
  */

/** @defgroup HRTIM_Timer_Reset_Update HRTIM Timer Reset Update
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether the register are updated upon Timerx
  *        counter reset or roll-over to 0 after reaching the period value
  *        in continuous mode
  *
  */

#define HRTIM_TIMUPDATEONRESET_DISABLED 0x00000000U           /*!< Update by timer x reset / roll-over disabled */
#define HRTIM_TIMUPDATEONRESET_ENABLED (HRTIM_TIMCR_TRSTU)    /*!< Update by timer x reset / roll-over enabled */
/**
  * end of HRTIM_Timer_Reset_Update @}
  */


/** @defgroup HRTIM_Timer_RollOver_Mode HRTIM Timer RollOver Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining when the roll-over is generated upon Timerx
  *        event generated when the counter is equal to 0 ('VALLEY' mode) or to HRTIM_PERxR value ('CREST' mode) or BOTH
  *        This setting only applies when the UDM bit is set. It is not significant otherwise.
  *
  */

#define HRTIM_TIM_FEROM_BOTH      0x00000000U              /*!< Roll-over event used by  */
#define HRTIM_TIM_FEROM_CREST     (HRTIM_TIMCR2_FEROM_BIT1)   /*!< the Fault and */
#define HRTIM_TIM_FEROM_VALLEY    (HRTIM_TIMCR2_FEROM_BIT0)   /*!< Event counters */
#define HRTIM_TIM_BMROM_BOTH      0x00000000U              /*!< Roll-over event used in the Burst mode controller */
#define HRTIM_TIM_BMROM_CREST     (HRTIM_TIMCR2_BMROM_BIT1)   /*!< as clock  */
#define HRTIM_TIM_BMROM_VALLEY    (HRTIM_TIMCR2_BMROM_BIT0)   /*!< and as burst mode trigger */
#define HRTIM_TIM_ADROM_BOTH      0x00000000U              /*!< Roll-over event which triggers */
#define HRTIM_TIM_ADROM_CREST     (HRTIM_TIMCR2_ADROM_BIT1)   /*!< the */
#define HRTIM_TIM_ADROM_VALLEY    (HRTIM_TIMCR2_ADROM_BIT0)   /*!< ADC */
#define HRTIM_TIM_OUTROM_BOTH     0x00000000U              /*!< Roll-over event which sets and/or resets the outputs */
#define HRTIM_TIM_OUTROM_CREST    (HRTIM_TIMCR2_OUTROM_BIT1)  /*!< as per HRTIM_SETxyR */
#define HRTIM_TIM_OUTROM_VALLEY   (HRTIM_TIMCR2_OUTROM_BIT0)  /*!< and HRTIM_RSTxyR settings */
#define HRTIM_TIM_ROM_BOTH        0x00000000U              /*!< Roll-over event with the following destinations: IRQ and DMA requests,*/
#define HRTIM_TIM_ROM_CREST       (HRTIM_TIMCR2_ROM_BIT1)     /*!< Update trigger (to transfer content from preload to active registers), */
#define HRTIM_TIM_ROM_VALLEY      (HRTIM_TIMCR2_ROM_BIT0)     /*!< repetition counter decrement and External Event filtering */

#define IS_HRTIM_ROLLOVERMODE(ROLLOVER)\
              ((((ROLLOVER) == HRTIM_TIM_FEROM_BOTH)  || ((ROLLOVER) == HRTIM_TIM_FEROM_CREST)  || ((ROLLOVER) == HRTIM_TIM_FEROM_VALLEY))  ||\
               (((ROLLOVER) == HRTIM_TIM_ADROM_BOTH)  || ((ROLLOVER) == HRTIM_TIM_ADROM_CREST)  || ((ROLLOVER) == HRTIM_TIM_ADROM_VALLEY))  ||\
               (((ROLLOVER) == HRTIM_TIM_BMROM_BOTH)  || ((ROLLOVER) == HRTIM_TIM_BMROM_CREST)  || ((ROLLOVER) == HRTIM_TIM_BMROM_VALLEY))  ||\
               (((ROLLOVER) == HRTIM_TIM_OUTROM_BOTH) || ((ROLLOVER) == HRTIM_TIM_OUTROM_CREST) || ((ROLLOVER) == HRTIM_TIM_OUTROM_VALLEY)) ||\
               (((ROLLOVER) == HRTIM_TIM_ROM_BOTH)    || ((ROLLOVER) == HRTIM_TIM_ROM_CREST)    || ((ROLLOVER) == HRTIM_TIM_ROM_VALLEY)))
/**
  * end of HRTIM_Timer_RollOver_Mode @}
  */

/** @defgroup HRTIM_Compare_Unit_Auto_Delayed_Mode HRTIM Compare Unit Auto Delayed Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief  Constants defining whether the compare register is behaving in
  *        regular mode (compare match issued as soon as counter equal compare),
  *        or in auto-delayed mode
  */


#define HRTIM_AUTODELAYEDMODE_REGULAR                 (0x00000000U)                                   /*!< standard compare mode */
#define HRTIM_AUTODELAYEDMODE_AUTODELAYED_NOTIMEOUT   (HRTIM_TIMCR_DELCMP2_BIT0)                         /*!< Compare event generated only if a capture has occurred */
#define HRTIM_AUTODELAYEDMODE_AUTODELAYED_TIMEOUTCMP1 (HRTIM_TIMCR_DELCMP2_BIT1)                         /*!< Compare event generated if a capture has occurred or after a Compare 1 match (timeout if capture event is missing) */
#define HRTIM_AUTODELAYEDMODE_AUTODELAYED_TIMEOUTCMP3 (HRTIM_TIMCR_DELCMP2_BIT1 | HRTIM_TIMCR_DELCMP2_BIT0) /*!< Compare event generated if a capture has occurred or after a Compare 3 match (timeout if capture event is missing) */
/**
  * end of HRTIM_Compare_Unit_Auto_Delayed_Mode @}
  */


/** @defgroup HRTIM_Simple_OC_Mode HRTIM Simple OC Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the behavior of the output signal when the timer
  *        operates in basic output compare mode
  *
  */

#define HRTIM_BASICOCMODE_TOGGLE    (0x00000001U)  /*!< Output toggles when the timer counter reaches the compare value */
#define HRTIM_BASICOCMODE_INACTIVE  (0x00000002U)  /*!< Output forced to active level when the timer counter reaches the compare value */
#define HRTIM_BASICOCMODE_ACTIVE    (0x00000003U)  /*!< Output forced to inactive level when the timer counter reaches the compare value */

#define IS_HRTIM_BASICOCMODE(BASICOCMODE)\
              (((BASICOCMODE) == HRTIM_BASICOCMODE_TOGGLE)   || \
               ((BASICOCMODE) == HRTIM_BASICOCMODE_INACTIVE) || \
               ((BASICOCMODE) == HRTIM_BASICOCMODE_ACTIVE))
/**
  * end of HRTIM_Simple_OC_Mode @}
  */


/** @defgroup HRTIM_Output_Polarity HRTIM Output Polarity
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the polarity of a timer output
  *
  */

#define HRTIM_OUTPUTPOLARITY_HIGH    (0x00000000U)           /*!< Output is active HIGH */
#define HRTIM_OUTPUTPOLARITY_LOW     (HRTIM_OUTR_POL1)       /*!< Output is active LOW */
/**
  * end of HRTIM_Output_Polarity @}
  */


/** @defgroup HRTIM_Output_Set_Source HRTIM Output Set Source
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the events that can be selected to configure the
  *        set crossbar of a timer output
  *
  */

#define HRTIM_OUTPUTSET_NONE       0x00000000U                      /*!< Reset the output set crossbar */
#define HRTIM_OUTPUTSET_RESYNC     (HRTIM_SET1R_RESYNC)             /*!< Timer reset event coming solely from software or SYNC input forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMPER     (HRTIM_SET1R_PER)                /*!< Timer period event forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMCMP1    (HRTIM_SET1R_CMP1)               /*!< Timer compare 1 event forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMCMP2    (HRTIM_SET1R_CMP2)               /*!< Timer compare 2 event forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMCMP3    (HRTIM_SET1R_CMP3)               /*!< Timer compare 3 event forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMCMP4    (HRTIM_SET1R_CMP4)               /*!< Timer compare 4 event forces the output to its active state */
#define HRTIM_OUTPUTSET_MASTERPER  (HRTIM_SET1R_MSTPER)             /*!< The master timer period event forces the output to its active state */
#define HRTIM_OUTPUTSET_MASTERCMP1 (HRTIM_SET1R_MSTCMP1)            /*!< Master Timer compare 1 event forces the output to its active state */
#define HRTIM_OUTPUTSET_MASTERCMP2 (HRTIM_SET1R_MSTCMP2)            /*!< Master Timer compare 2 event forces the output to its active state */
#define HRTIM_OUTPUTSET_MASTERCMP3 (HRTIM_SET1R_MSTCMP3)            /*!< Master Timer compare 3 event forces the output to its active state */
#define HRTIM_OUTPUTSET_MASTERCMP4 (HRTIM_SET1R_MSTCMP4)            /*!< Master Timer compare 4 event forces the output to its active state */
/* Timer Events mapping for Timer A */
#define HRTIM_OUTPUTSET_TIMAEV1_TIMBCMP1    (HRTIM_SET1R_TIMEVNT1)  /*!< Timer event 1 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMAEV2_TIMBCMP2    (HRTIM_SET1R_TIMEVNT2)  /*!< Timer event 2 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMAEV3_TIMCCMP2    (HRTIM_SET1R_TIMEVNT3)  /*!< Timer event 3 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMAEV4_TIMCCMP3    (HRTIM_SET1R_TIMEVNT4)  /*!< Timer event 4 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMAEV5_TIMDCMP1    (HRTIM_SET1R_TIMEVNT5)  /*!< Timer event 5 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMAEV6_TIMDCMP2    (HRTIM_SET1R_TIMEVNT6)  /*!< Timer event 6 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMAEV7_TIMECMP3    (HRTIM_SET1R_TIMEVNT7)  /*!< Timer event 7 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMAEV8_TIMECMP4    (HRTIM_SET1R_TIMEVNT8)  /*!< Timer event 8 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMAEV9_TIMFCMP4    (HRTIM_SET1R_TIMEVNT9)  /*!< Timer event 9 forces the output to its active state */
/* Timer Events mapping for Timer B */
#define HRTIM_OUTPUTSET_TIMBEV1_TIMACMP1    (HRTIM_SET1R_TIMEVNT1)  /*!< Timer event 1 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMBEV2_TIMACMP2    (HRTIM_SET1R_TIMEVNT2)  /*!< Timer event 2 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMBEV3_TIMCCMP3    (HRTIM_SET1R_TIMEVNT3)  /*!< Timer event 3 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMBEV4_TIMCCMP4    (HRTIM_SET1R_TIMEVNT4)  /*!< Timer event 4 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMBEV5_TIMDCMP3    (HRTIM_SET1R_TIMEVNT5)  /*!< Timer event 5 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMBEV6_TIMDCMP4    (HRTIM_SET1R_TIMEVNT6)  /*!< Timer event 6 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMBEV7_TIMECMP1    (HRTIM_SET1R_TIMEVNT7)  /*!< Timer event 7 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMBEV8_TIMECMP2    (HRTIM_SET1R_TIMEVNT8)  /*!< Timer event 8 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMBEV9_TIMFCMP3    (HRTIM_SET1R_TIMEVNT9)  /*!< Timer event 9 forces the output to its active state */
/* Timer Events mapping for Timer C */
#define HRTIM_OUTPUTSET_TIMCEV1_TIMACMP2    (HRTIM_SET1R_TIMEVNT1)  /*!< Timer event 1 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMCEV2_TIMACMP3    (HRTIM_SET1R_TIMEVNT2)  /*!< Timer event 2 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMCEV3_TIMBCMP2    (HRTIM_SET1R_TIMEVNT3)  /*!< Timer event 3 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMCEV4_TIMBCMP3    (HRTIM_SET1R_TIMEVNT4)  /*!< Timer event 4 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMCEV5_TIMDCMP2    (HRTIM_SET1R_TIMEVNT5)  /*!< Timer event 5 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMCEV6_TIMDCMP4    (HRTIM_SET1R_TIMEVNT6)  /*!< Timer event 6 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMCEV7_TIMECMP3    (HRTIM_SET1R_TIMEVNT7)  /*!< Timer event 7 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMCEV8_TIMECMP4    (HRTIM_SET1R_TIMEVNT8)  /*!< Timer event 8 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMCEV9_TIMFCMP2    (HRTIM_SET1R_TIMEVNT9)  /*!< Timer event 9 forces the output to its active state */
/* Timer Events mapping for Timer D */
#define HRTIM_OUTPUTSET_TIMDEV1_TIMACMP1    (HRTIM_SET1R_TIMEVNT1)  /*!< Timer event 1 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMDEV2_TIMACMP4    (HRTIM_SET1R_TIMEVNT2)  /*!< Timer event 2 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMDEV3_TIMBCMP2    (HRTIM_SET1R_TIMEVNT3)  /*!< Timer event 3 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMDEV4_TIMBCMP4    (HRTIM_SET1R_TIMEVNT4)  /*!< Timer event 4 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMDEV5_TIMCCMP4    (HRTIM_SET1R_TIMEVNT5)  /*!< Timer event 5 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMDEV6_TIMECMP1    (HRTIM_SET1R_TIMEVNT6)  /*!< Timer event 6 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMDEV7_TIMECMP4    (HRTIM_SET1R_TIMEVNT7)  /*!< Timer event 7 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMDEV8_TIMFCMP1    (HRTIM_SET1R_TIMEVNT8)  /*!< Timer event 8 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMDEV9_TIMFCMP3    (HRTIM_SET1R_TIMEVNT9)  /*!< Timer event 9 forces the output to its active state */
/* Timer Events mapping for Timer E */
#define HRTIM_OUTPUTSET_TIMEEV1_TIMACMP4    (HRTIM_SET1R_TIMEVNT1)  /*!< Timer event 1 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMEEV2_TIMBCMP3    (HRTIM_SET1R_TIMEVNT2)  /*!< Timer event 2 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMEEV3_TIMBCMP4    (HRTIM_SET1R_TIMEVNT3)  /*!< Timer event 3 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMEEV4_TIMCCMP1    (HRTIM_SET1R_TIMEVNT4)  /*!< Timer event 4 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMEEV5_TIMCCMP2    (HRTIM_SET1R_TIMEVNT5)  /*!< Timer event 5 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMEEV6_TIMDCMP1    (HRTIM_SET1R_TIMEVNT6)  /*!< Timer event 6 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMEEV7_TIMDCMP2    (HRTIM_SET1R_TIMEVNT7)  /*!< Timer event 7 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMEEV8_TIMFCMP3    (HRTIM_SET1R_TIMEVNT8)  /*!< Timer event 8 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMEEV9_TIMFCMP4    (HRTIM_SET1R_TIMEVNT9)  /*!< Timer event 9 forces the output to its active state */
/* Timer Events mapping for Timer F */
#define HRTIM_OUTPUTSET_TIMFEV1_TIMACMP3    (HRTIM_SET1R_TIMEVNT1)  /*!< Timer event 1 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMFEV2_TIMBCMP1    (HRTIM_SET1R_TIMEVNT2)  /*!< Timer event 2 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMFEV3_TIMBCMP4    (HRTIM_SET1R_TIMEVNT3)  /*!< Timer event 3 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMFEV4_TIMCCMP1    (HRTIM_SET1R_TIMEVNT4)  /*!< Timer event 4 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMFEV5_TIMCCMP4    (HRTIM_SET1R_TIMEVNT5)  /*!< Timer event 5 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMFEV6_TIMDCMP3    (HRTIM_SET1R_TIMEVNT6)  /*!< Timer event 6 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMFEV7_TIMDCMP4    (HRTIM_SET1R_TIMEVNT7)  /*!< Timer event 7 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMFEV8_TIMECMP2    (HRTIM_SET1R_TIMEVNT8)  /*!< Timer event 8 forces the output to its active state */
#define HRTIM_OUTPUTSET_TIMFEV9_TIMECMP3    (HRTIM_SET1R_TIMEVNT9)  /*!< Timer event 9 forces the output to its active state */
#define HRTIM_OUTPUTSET_EEV_1      (HRTIM_SET1R_EXTVNT1)            /*!< External event 1 forces the output to its active state */
#define HRTIM_OUTPUTSET_EEV_2      (HRTIM_SET1R_EXTVNT2)            /*!< External event 2 forces the output to its active state */
#define HRTIM_OUTPUTSET_EEV_3      (HRTIM_SET1R_EXTVNT3)            /*!< External event 3 forces the output to its active state */
#define HRTIM_OUTPUTSET_EEV_4      (HRTIM_SET1R_EXTVNT4)            /*!< External event 4 forces the output to its active state */
#define HRTIM_OUTPUTSET_EEV_5      (HRTIM_SET1R_EXTVNT5)            /*!< External event 5 forces the output to its active state */
#define HRTIM_OUTPUTSET_EEV_6      (HRTIM_SET1R_EXTVNT6)            /*!< External event 6 forces the output to its active state */
#define HRTIM_OUTPUTSET_EEV_7      (HRTIM_SET1R_EXTVNT7)            /*!< External event 7 forces the output to its active state */
#define HRTIM_OUTPUTSET_EEV_8      (HRTIM_SET1R_EXTVNT8)            /*!< External event 8 forces the output to its active state */
#define HRTIM_OUTPUTSET_EEV_9      (HRTIM_SET1R_EXTVNT9)            /*!< External event 9 forces the output to its active state */
#define HRTIM_OUTPUTSET_EEV_10     (HRTIM_SET1R_EXTVNT10)           /*!< External event 10 forces the output to its active state */
#define HRTIM_OUTPUTSET_UPDATE     (HRTIM_SET1R_UPDATE)             /*!< Timer register update event forces the output to its active state */
/**
  * end of HRTIM_Output_Set_Source @}
  */

/** @defgroup HRTIM_Output_Reset_Source HRTIM Output Reset Source
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the events that can be selected to configure the
  *        reset crossbar of a timer output
  *
  */

#define HRTIM_OUTPUTRESET_NONE       0x00000000U                      /*!< Reset the output reset crossbar */
#define HRTIM_OUTPUTRESET_RESYNC     (HRTIM_RST1R_RESYNC)             /*!< Timer reset event coming solely from software or SYNC input forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMPER     (HRTIM_RST1R_PER)                /*!< Timer period event forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMCMP1    (HRTIM_RST1R_CMP1)               /*!< Timer compare 1 event forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMCMP2    (HRTIM_RST1R_CMP2)               /*!< Timer compare 2 event forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMCMP3    (HRTIM_RST1R_CMP3)               /*!< Timer compare 3 event forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMCMP4    (HRTIM_RST1R_CMP4)               /*!< Timer compare 4 event forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_MASTERPER  (HRTIM_RST1R_MSTPER)             /*!< The master timer period event forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_MASTERCMP1 (HRTIM_RST1R_MSTCMP1)            /*!< Master Timer compare 1 event forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_MASTERCMP2 (HRTIM_RST1R_MSTCMP2)            /*!< Master Timer compare 2 event forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_MASTERCMP3 (HRTIM_RST1R_MSTCMP3)            /*!< Master Timer compare 3 event forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_MASTERCMP4 (HRTIM_RST1R_MSTCMP4)            /*!< Master Timer compare 4 event forces the output to its inactive state */
/* Timer Events mapping for Timer A */
#define HRTIM_OUTPUTRESET_TIMAEV1_TIMBCMP1    (HRTIM_RST1R_TIMEVNT1)  /*!< Timer event 1 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMAEV2_TIMBCMP2    (HRTIM_RST1R_TIMEVNT2)  /*!< Timer event 2 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMAEV3_TIMCCMP2    (HRTIM_RST1R_TIMEVNT3)  /*!< Timer event 3 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMAEV4_TIMCCMP3    (HRTIM_RST1R_TIMEVNT4)  /*!< Timer event 4 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMAEV5_TIMDCMP1    (HRTIM_RST1R_TIMEVNT5)  /*!< Timer event 5 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMAEV6_TIMDCMP2    (HRTIM_RST1R_TIMEVNT6)  /*!< Timer event 6 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMAEV7_TIMECMP3    (HRTIM_RST1R_TIMEVNT7)  /*!< Timer event 7 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMAEV8_TIMECMP4    (HRTIM_RST1R_TIMEVNT8)  /*!< Timer event 8 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMAEV9_TIMFCMP4    (HRTIM_RST1R_TIMEVNT9)  /*!< Timer event 9 forces the output to its inactive state */
/* Timer Events mapping for Timer B */
#define HRTIM_OUTPUTRESET_TIMBEV1_TIMACMP1    (HRTIM_RST1R_TIMEVNT1)  /*!< Timer event 1 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMBEV2_TIMACMP2    (HRTIM_RST1R_TIMEVNT2)  /*!< Timer event 2 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMBEV3_TIMCCMP3    (HRTIM_RST1R_TIMEVNT3)  /*!< Timer event 3 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMBEV4_TIMCCMP4    (HRTIM_RST1R_TIMEVNT4)  /*!< Timer event 4 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMBEV5_TIMDCMP3    (HRTIM_RST1R_TIMEVNT5)  /*!< Timer event 5 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMBEV6_TIMDCMP4    (HRTIM_RST1R_TIMEVNT6)  /*!< Timer event 6 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMBEV7_TIMECMP1    (HRTIM_RST1R_TIMEVNT7)  /*!< Timer event 7 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMBEV8_TIMECMP2    (HRTIM_RST1R_TIMEVNT8)  /*!< Timer event 8 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMBEV9_TIMFCMP3    (HRTIM_RST1R_TIMEVNT9)  /*!< Timer event 9 forces the output to its inactive state */
/* Timer Events mapping for Timer C */
#define HRTIM_OUTPUTRESET_TIMCEV1_TIMACMP2    (HRTIM_RST1R_TIMEVNT1)  /*!< Timer event 1 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMCEV2_TIMACMP3    (HRTIM_RST1R_TIMEVNT2)  /*!< Timer event 2 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMCEV3_TIMBCMP2    (HRTIM_RST1R_TIMEVNT3)  /*!< Timer event 3 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMCEV4_TIMBCMP3    (HRTIM_RST1R_TIMEVNT4)  /*!< Timer event 4 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMCEV5_TIMDCMP2    (HRTIM_RST1R_TIMEVNT5)  /*!< Timer event 5 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMCEV6_TIMDCMP4    (HRTIM_RST1R_TIMEVNT6)  /*!< Timer event 6 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMCEV7_TIMECMP3    (HRTIM_RST1R_TIMEVNT7)  /*!< Timer event 7 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMCEV8_TIMECMP4    (HRTIM_RST1R_TIMEVNT8)  /*!< Timer event 8 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMCEV9_TIMFCMP2    (HRTIM_RST1R_TIMEVNT9)  /*!< Timer event 9 forces the output to its inactive state */
/* Timer Events mapping for Timer D */
#define HRTIM_OUTPUTRESET_TIMDEV1_TIMACMP1    (HRTIM_RST1R_TIMEVNT1)  /*!< Timer event 1 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMDEV2_TIMACMP4    (HRTIM_RST1R_TIMEVNT2)  /*!< Timer event 2 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMDEV3_TIMBCMP2    (HRTIM_RST1R_TIMEVNT3)  /*!< Timer event 3 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMDEV4_TIMBCMP4    (HRTIM_RST1R_TIMEVNT4)  /*!< Timer event 4 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMDEV5_TIMCCMP4    (HRTIM_RST1R_TIMEVNT5)  /*!< Timer event 5 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMDEV6_TIMECMP1    (HRTIM_RST1R_TIMEVNT6)  /*!< Timer event 6 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMDEV7_TIMECMP4    (HRTIM_RST1R_TIMEVNT7)  /*!< Timer event 7 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMDEV8_TIMFCMP1    (HRTIM_RST1R_TIMEVNT8)  /*!< Timer event 8 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMDEV9_TIMFCMP3    (HRTIM_RST1R_TIMEVNT9)  /*!< Timer event 9 forces the output to its inactive state */
/* Timer Events mapping for Timer E */
#define HRTIM_OUTPUTRESET_TIMEEV1_TIMACMP4    (HRTIM_RST1R_TIMEVNT1)  /*!< Timer event 1 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMEEV2_TIMBCMP3    (HRTIM_RST1R_TIMEVNT2)  /*!< Timer event 2 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMEEV3_TIMBCMP4    (HRTIM_RST1R_TIMEVNT3)  /*!< Timer event 3 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMEEV4_TIMCCMP1    (HRTIM_RST1R_TIMEVNT4)  /*!< Timer event 4 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMEEV5_TIMCCMP2    (HRTIM_RST1R_TIMEVNT5)  /*!< Timer event 5 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMEEV6_TIMDCMP1    (HRTIM_RST1R_TIMEVNT6)  /*!< Timer event 6 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMEEV7_TIMDCMP2    (HRTIM_RST1R_TIMEVNT7)  /*!< Timer event 7 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMEEV8_TIMFCMP3    (HRTIM_RST1R_TIMEVNT8)  /*!< Timer event 8 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMEEV9_TIMFCMP4    (HRTIM_RST1R_TIMEVNT9)  /*!< Timer event 9 forces the output to its inactive state */
/* Timer Events mapping for Timer F */
#define HRTIM_OUTPUTRESET_TIMFEV1_TIMACMP3    (HRTIM_RST1R_TIMEVNT1)  /*!< Timer event 1 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMFEV2_TIMBCMP1    (HRTIM_RST1R_TIMEVNT2)  /*!< Timer event 2 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMFEV3_TIMBCMP4    (HRTIM_RST1R_TIMEVNT3)  /*!< Timer event 3 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMFEV4_TIMCCMP1    (HRTIM_RST1R_TIMEVNT4)  /*!< Timer event 4 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMFEV5_TIMCCMP4    (HRTIM_RST1R_TIMEVNT5)  /*!< Timer event 5 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMFEV6_TIMDCMP3    (HRTIM_RST1R_TIMEVNT6)  /*!< Timer event 6 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMFEV7_TIMDCMP4    (HRTIM_RST1R_TIMEVNT7)  /*!< Timer event 7 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMFEV8_TIMECMP2    (HRTIM_RST1R_TIMEVNT8)  /*!< Timer event 8 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_TIMFEV9_TIMECMP3    (HRTIM_RST1R_TIMEVNT9)  /*!< Timer event 9 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_EEV_1      (HRTIM_RST1R_EXTVNT1)            /*!< External event 1 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_EEV_2      (HRTIM_RST1R_EXTVNT2)            /*!< External event 2 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_EEV_3      (HRTIM_RST1R_EXTVNT3)            /*!< External event 3 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_EEV_4      (HRTIM_RST1R_EXTVNT4)            /*!< External event 4 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_EEV_5      (HRTIM_RST1R_EXTVNT5)            /*!< External event 5 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_EEV_6      (HRTIM_RST1R_EXTVNT6)            /*!< External event 6 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_EEV_7      (HRTIM_RST1R_EXTVNT7)            /*!< External event 7 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_EEV_8      (HRTIM_RST1R_EXTVNT8)            /*!< External event 8 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_EEV_9      (HRTIM_RST1R_EXTVNT9)            /*!< External event 9 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_EEV_10     (HRTIM_RST1R_EXTVNT10)           /*!< External event 10 forces the output to its inactive state */
#define HRTIM_OUTPUTRESET_UPDATE     (HRTIM_RST1R_UPDATE)             /*!< Timer register update event forces the output to its inactive state */
/**
  * end of HRTIM_Output_Reset_Source @}
  */


/** @defgroup HRTIM_Output_Idle_Mode HRTIM Output Idle Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not the timer output transition to its
  *        IDLE state when burst mode is entered
  *
  */

#define HRTIM_OUTPUTIDLEMODE_NONE     0x00000000U           /*!< The output is not affected by the burst mode operation */
#define HRTIM_OUTPUTIDLEMODE_IDLE     (HRTIM_OUTR_IDLM1)    /*!< The output is in idle state when requested by the burst mode controller */
/**
  * end of HRTIM_Output_Idle_Mode @}
  */


/** @defgroup HRTIM_Output_IDLE_Level HRTIM Output IDLE Level
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the output level when output is in IDLE state
  *
  */

#define HRTIM_OUTPUTIDLELEVEL_INACTIVE   0x00000000U           /*!< Output at inactive level when in IDLE state */
#define HRTIM_OUTPUTIDLELEVEL_ACTIVE     (HRTIM_OUTR_IDLES1)   /*!< Output at active level when in IDLE state */
/**
  * end of HRTIM_Output_IDLE_Level @}
  */


/** @defgroup HRTIM_Output_FAULT_Level HRTIM Output FAULT Level
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the output level when output is in FAULT state
  *
  */

#define HRTIM_OUTPUTFAULTLEVEL_NONE      0x00000000U                                  /*!< The output is not affected by the fault input */
#define HRTIM_OUTPUTFAULTLEVEL_ACTIVE    (HRTIM_OUTR_FAULT1_BIT0)                        /*!< Output at active level when in FAULT state */
#define HRTIM_OUTPUTFAULTLEVEL_INACTIVE  (HRTIM_OUTR_FAULT1_BIT1)                        /*!< Output at inactive level when in FAULT state */
#define HRTIM_OUTPUTFAULTLEVEL_HIGHZ     (HRTIM_OUTR_FAULT1_BIT1 | HRTIM_OUTR_FAULT1_BIT0)  /*!< Output is tri-stated when in FAULT state */
/**
  * end of HRTIM_Output_FAULT_Level @}
  */


/** @defgroup HRTIM_Output_Chopper_Mode_Enable HRTIM Output Chopper Mode Enable
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not chopper mode is enabled for a timer
  *        output
  *
  */

#define HRTIM_OUTPUTCHOPPERMODE_DISABLED   0x00000000U           /*!< Output signal is not altered  */
#define HRTIM_OUTPUTCHOPPERMODE_ENABLED    (HRTIM_OUTR_CHP1)     /*!< Output signal is chopped by a carrier signal  */
/**
  * end of HRTIM_Output_Chopper_Mode_Enable @}
  */


/** @defgroup HRTIM_Output_Burst_Mode_Entry_Delayed HRTIM Output Burst Mode Entry Delayed
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the idle mode entry is delayed by forcing a
  *        dead-time insertion before switching the outputs to their idle state
  */

#define HRTIM_OUTPUTBURSTMODEENTRY_REGULAR   0x00000000U           /*!< The programmed Idle state is applied immediately to the Output */
#define HRTIM_OUTPUTBURSTMODEENTRY_DELAYED   (HRTIM_OUTR_DIDL1)    /*!< Dead-time is inserted on output before entering the idle mode */
/**
  * end of HRTIM_Output_Burst_Mode_Entry_Delayed @}
  */


/** @defgroup HRTIM_Output_Balanced_Idle_Auto_Resume HRTIM Output Balanced Idle Automatic Resume
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining if the outputs are automatically
  *        re-enabled after a balanced idle event.
  */

#define HRTIM_OUTPUTBIAR_DISABLED   0x00000000U            /*!< output is not automatically re-enabled */
#define HRTIM_OUTPUTBIAR_ENABLED    (HRTIM_OUTR_BIAR)      /*!< output is automatically re-enabled */
/**
  * end of HRTIM_Output_Balanced_Idle_Auto_Resume @}
  */

/** @defgroup HRTIM_Capture_Unit_Trigger HRTIM Capture Unit Trigger
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the events that can be selected to trigger the
  *        capture of the timing unit counter
  */

#define HRTIM_CAPTURETRIGGER_NONE         0x00000000U              /*!< Capture trigger is disabled */
#define HRTIM_CAPTURETRIGGER_UPDATE       (HRTIM_CPT1CR_UPDCPT)    /*!< The update event triggers the Capture */
#define HRTIM_CAPTURETRIGGER_EEV_1        (HRTIM_CPT1CR_EXEV1CPT)  /*!< The External event 1 triggers the Capture */
#define HRTIM_CAPTURETRIGGER_EEV_2        (HRTIM_CPT1CR_EXEV2CPT)  /*!< The External event 2 triggers the Capture */
#define HRTIM_CAPTURETRIGGER_EEV_3        (HRTIM_CPT1CR_EXEV3CPT)  /*!< The External event 3 triggers the Capture */
#define HRTIM_CAPTURETRIGGER_EEV_4        (HRTIM_CPT1CR_EXEV4CPT)  /*!< The External event 4 triggers the Capture */
#define HRTIM_CAPTURETRIGGER_EEV_5        (HRTIM_CPT1CR_EXEV5CPT)  /*!< The External event 5 triggers the Capture */
#define HRTIM_CAPTURETRIGGER_EEV_6        (HRTIM_CPT1CR_EXEV6CPT)  /*!< The External event 6 triggers the Capture */
#define HRTIM_CAPTURETRIGGER_EEV_7        (HRTIM_CPT1CR_EXEV7CPT)  /*!< The External event 7 triggers the Capture */
#define HRTIM_CAPTURETRIGGER_EEV_8        (HRTIM_CPT1CR_EXEV8CPT)  /*!< The External event 8 triggers the Capture */
#define HRTIM_CAPTURETRIGGER_EEV_9        (HRTIM_CPT1CR_EXEV9CPT)  /*!< The External event 9 triggers the Capture */
#define HRTIM_CAPTURETRIGGER_EEV_10       (HRTIM_CPT1CR_EXEV10CPT) /*!< The External event 10 triggers the Capture */
#define HRTIM_CAPTURETRIGGER_TA1_SET      (HRTIM_CPT1CR_TA1SET)    /*!< Capture is triggered by TA1 output inactive to active transition */
#define HRTIM_CAPTURETRIGGER_TA1_RESET    (HRTIM_CPT1CR_TA1RST)    /*!< Capture is triggered by TA1 output active to inactive transition */
#define HRTIM_CAPTURETRIGGER_TIMERA_CMP1  (HRTIM_CPT1CR_TIMACMP1)  /*!< Timer A Compare 1 triggers Capture */
#define HRTIM_CAPTURETRIGGER_TIMERA_CMP2  (HRTIM_CPT1CR_TIMACMP2)  /*!< Timer A Compare 2 triggers Capture */
#define HRTIM_CAPTURETRIGGER_TB1_SET      (HRTIM_CPT1CR_TB1SET)    /*!< Capture is triggered by TB1 output inactive to active transition */
#define HRTIM_CAPTURETRIGGER_TB1_RESET    (HRTIM_CPT1CR_TB1RST)    /*!< Capture is triggered by TB1 output active to inactive transition */
#define HRTIM_CAPTURETRIGGER_TIMERB_CMP1  (HRTIM_CPT1CR_TIMBCMP1)  /*!< Timer B Compare 1 triggers Capture */
#define HRTIM_CAPTURETRIGGER_TIMERB_CMP2  (HRTIM_CPT1CR_TIMBCMP2)  /*!< Timer B Compare 2 triggers Capture */
#define HRTIM_CAPTURETRIGGER_TC1_SET      (HRTIM_CPT1CR_TC1SET)    /*!< Capture is triggered by TC1 output inactive to active transition */
#define HRTIM_CAPTURETRIGGER_TC1_RESET    (HRTIM_CPT1CR_TC1RST)    /*!< Capture is triggered by TC1 output active to inactive transition */
#define HRTIM_CAPTURETRIGGER_TIMERC_CMP1  (HRTIM_CPT1CR_TIMCCMP1)  /*!< Timer C Compare 1 triggers Capture */
#define HRTIM_CAPTURETRIGGER_TIMERC_CMP2  (HRTIM_CPT1CR_TIMCCMP2)  /*!< Timer C Compare 2 triggers Capture */
#define HRTIM_CAPTURETRIGGER_TD1_SET      (HRTIM_CPT1CR_TD1SET)    /*!< Capture is triggered by TD1 output inactive to active transition */
#define HRTIM_CAPTURETRIGGER_TD1_RESET    (HRTIM_CPT1CR_TD1RST)    /*!< Capture is triggered by TD1 output active to inactive transition */
#define HRTIM_CAPTURETRIGGER_TIMERD_CMP1  (HRTIM_CPT1CR_TIMDCMP1)  /*!< Timer D Compare 1 triggers Capture */
#define HRTIM_CAPTURETRIGGER_TIMERD_CMP2  (HRTIM_CPT1CR_TIMDCMP2)  /*!< Timer D Compare 2 triggers Capture */
#define HRTIM_CAPTURETRIGGER_TE1_SET      (HRTIM_CPT1CR_TE1SET)    /*!< Capture is triggered by TE1 output inactive to active transition */
#define HRTIM_CAPTURETRIGGER_TE1_RESET    (HRTIM_CPT1CR_TE1RST)    /*!< Capture is triggered by TE1 output active to inactive transition */
#define HRTIM_CAPTURETRIGGER_TIMERE_CMP1  (HRTIM_CPT1CR_TIMECMP1)  /*!< Timer E Compare 1 triggers Capture */
#define HRTIM_CAPTURETRIGGER_TIMERE_CMP2  (HRTIM_CPT1CR_TIMECMP2)  /*!< Timer E Compare 2 triggers Capture */
/**
  * end of HRTIM_Capture_Unit_Trigger @}
  */

/** @defgroup HRTIM_Capture_Unit_TimerF_Trigger HRTIM Capture Unit TimerF Trigger
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the events that can be selected to trigger the
  *        capture of the timing unit counter
  */

#define HRTIM_CAPTURETRIGGER_TF1_SET       ((uint64_t)(HRTIM_CPT1CR_TF1SET  ) << 32)  /*!< Capture is triggered by TF1 output inactive to active transition */
#define HRTIM_CAPTURETRIGGER_TF1_RESET     ((uint64_t)(HRTIM_CPT1CR_TF1RST  ) << 32)  /*!< Capture is triggered by TF1 output active to inactive transition */
#define HRTIM_CAPTURETRIGGER_TIMERF_CMP1   ((uint64_t)(HRTIM_CPT1CR_TIMFCMP1) << 32)  /*!< Timer F Compare 1 triggers Capture */
#define HRTIM_CAPTURETRIGGER_TIMERF_CMP2   ((uint64_t)(HRTIM_CPT1CR_TIMFCMP2) << 32)  /*!< Timer F Compare 2 triggers Capture */
/**
  * end of HRTIM_Capture_Unit_TimerF_Trigger @}
  */

/** @defgroup HRTIM_Timer_External_Event_Filter HRTIM Timer External Event Filter
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the event filtering applied to external events
  *        by a timer
  *
  */

#define HRTIM_TIMEEVFLT_NONE                            (0x00000000U)
#define HRTIM_TIMEEVFLT_BLANKINGCMP1                    (HRTIM_EEFR1_EE1FLTR_BIT0)                                                   /*!< Blanking from counter reset/roll-over to Compare 1U */
#define HRTIM_TIMEEVFLT_BLANKINGCMP2                    (HRTIM_EEFR1_EE1FLTR_BIT1)                                                   /*!< Blanking from counter reset/roll-over to Compare 2U */
#define HRTIM_TIMEEVFLT_BLANKINGCMP3                    (HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from counter reset/roll-over to Compare 3U */
#define HRTIM_TIMEEVFLT_BLANKINGCMP4                    (HRTIM_EEFR1_EE1FLTR_BIT2)                                                   /*!< Blanking from counter reset/roll-over to Compare 4U */
/* Blanking Filter for TIMER A */
#define HRTIM_TIMEEVFLT_BLANKING_TIMAEEF1_TIMBCMP1      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from another timing unit: TIMFLTR1 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMAEEF2_TIMBCMP4      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1)                           /*!< Blanking from another timing unit: TIMFLTR2 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMAEEF3_TIMBOUT2      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Blanking from another timing unit: TIMFLTR3 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMAEEF4_TIMCCMP1      (HRTIM_EEFR1_EE1FLTR_BIT3)                                                   /*!< Blanking from another timing unit: TIMFLTR4 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMAEEF5_TIMCCMP4      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from another timing unit: TIMFLTR5 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMAEEF6_TIMFCMP1      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT1)                           /*!< Blanking from another timing unit: TIMFLTR6 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMAEEF7_TIMDCMP1      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Blanking from another timing unit: TIMFLTR7 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMAEEF8_TIMECMP2      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT2)                           /*!< Blanking from another timing unit: TIMFLTR8 source */
/* Blanking Filter for TIMER B */
#define HRTIM_TIMEEVFLT_BLANKING_TIMBEEF1_TIMACMP1      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from another timing unit: TIMFLTR1 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMBEEF2_TIMACMP4      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1)                           /*!< Blanking from another timing unit: TIMFLTR2 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMBEEF3_TIMAOUT2      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Blanking from another timing unit: TIMFLTR3 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMBEEF4_TIMCCMP1      (HRTIM_EEFR1_EE1FLTR_BIT3)                                                   /*!< Blanking from another timing unit: TIMFLTR4 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMBEEF5_TIMCCMP2      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from another timing unit: TIMFLTR5 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMBEEF6_TIMFCMP2      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT1)                           /*!< Blanking from another timing unit: TIMFLTR6 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMBEEF7_TIMDCMP2      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Blanking from another timing unit: TIMFLTR7 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMBEEF8_TIMECMP1      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT2)                           /*!< Blanking from another timing unit: TIMFLTR8 source */
/* Blanking Filter for TIMER C */
#define HRTIM_TIMEEVFLT_BLANKING_TIMCEEF1_TIMACMP2      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from another timing unit: TIMFLTR1 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMCEEF2_TIMBCMP1      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1)                           /*!< Blanking from another timing unit: TIMFLTR2 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMCEEF3_TIMBCMP4      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Blanking from another timing unit: TIMFLTR3 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMCEEF4_TIMFCMP1      (HRTIM_EEFR1_EE1FLTR_BIT3)                                                   /*!< Blanking from another timing unit: TIMFLTR4 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMCEEF5_TIMDCMP1      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from another timing unit: TIMFLTR5 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMCEEF6_TIMDCMP4      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT1)                           /*!< Blanking from another timing unit: TIMFLTR6 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMCEEF7_TIMDOUT2      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Blanking from another timing unit: TIMFLTR7 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMCEEF8_TIMECMP4      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT2)                           /*!< Blanking from another timing unit: TIMFLTR8 source */
/* Blanking Filter for TIMER D */
#define HRTIM_TIMEEVFLT_BLANKING_TIMDEEF1_TIMACMP1      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from another timing unit: TIMFLTR1 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMDEEF2_TIMBCMP2      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1)                           /*!< Blanking from another timing unit: TIMFLTR2 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMDEEF3_TIMCCMP1      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Blanking from another timing unit: TIMFLTR3 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMDEEF4_TIMCCMP2      (HRTIM_EEFR1_EE1FLTR_BIT3)                                                   /*!< Blanking from another timing unit: TIMFLTR4 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMDEEF5_TIMCOUT2      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from another timing unit: TIMFLTR5 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMDEEF6_TIMECMP1      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT1)                           /*!< Blanking from another timing unit: TIMFLTR6 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMDEEF7_TIMECMP4      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Blanking from another timing unit: TIMFLTR7 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMDEEF8_TIMFCMP4      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT2)                           /*!< Blanking from another timing unit: TIMFLTR8 source */
/* Blanking Filter for TIMER E */
#define HRTIM_TIMEEVFLT_BLANKING_TIMEEEF1_TIMACMP2      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from another timing unit: TIMFLTR1 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMEEEF2_TIMBCMP1      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1)                           /*!< Blanking from another timing unit: TIMFLTR2 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMEEEF3_TIMCCMP1      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Blanking from another timing unit: TIMFLTR3 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMEEEF4_TIMFCMP4      (HRTIM_EEFR1_EE1FLTR_BIT3)                                                   /*!< Blanking from another timing unit: TIMFLTR4 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMEEEF5_TIMFOUT2      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from another timing unit: TIMFLTR5 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMEEEF6_TIMDCMP1      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT1)                           /*!< Blanking from another timing unit: TIMFLTR6 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMEEEF7_TIMDCMP4      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Blanking from another timing unit: TIMFLTR7 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMEEEF8_TIMDOUT2      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT2)                           /*!< Blanking from another timing unit: TIMFLTR8 source */
/* Blanking Filter for TIMER F */
#define HRTIM_TIMEEVFLT_BLANKING_TIMFEEF1_TIMACMP4      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from another timing unit: TIMFLTR1 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMFEEF2_TIMBCMP2      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1)                           /*!< Blanking from another timing unit: TIMFLTR2 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMFEEF3_TIMCCMP4      (HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Blanking from another timing unit: TIMFLTR3 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMFEEF4_TIMDCMP2      (HRTIM_EEFR1_EE1FLTR_BIT3)                                                   /*!< Blanking from another timing unit: TIMFLTR4 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMFEEF5_TIMDCMP4      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT0)                           /*!< Blanking from another timing unit: TIMFLTR5 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMFEEF6_TIMECMP1      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT1)                           /*!< Blanking from another timing unit: TIMFLTR6 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMFEEF7_TIMECMP4      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT1 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Blanking from another timing unit: TIMFLTR7 source */
#define HRTIM_TIMEEVFLT_BLANKING_TIMFEEF8_TIMEOUT2      (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT2)                           /*!< Blanking from another timing unit: TIMFLTR8 source */

#define HRTIM_TIMEEVFLT_WINDOWINGCMP2                   (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Windowing from counter reset/roll-over to Compare 2U */
#define HRTIM_TIMEEVFLT_WINDOWINGCMP3                   (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1)   /*!< Windowing from counter reset/roll-over to Compare 3U */
#define HRTIM_TIMEEVFLT_WINDOWINGTIM                    (HRTIM_EEFR1_EE1FLTR_BIT3 | HRTIM_EEFR1_EE1FLTR_BIT2 | HRTIM_EEFR1_EE1FLTR_BIT1\
                                                                                                             | HRTIM_EEFR1_EE1FLTR_BIT0)   /*!< Windowing from another timing unit: TIMWIN source */
/**
  * end of HRTIM_Timer_External_Event_Filter @}
  */

/** @defgroup HRTIM_Timer_External_Event_Latch HRTIM Timer External Event Latch
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not the external event is
  *        memorized (latched) and generated as soon as the blanking period
  *        is completed or the window ends
  *
  */

#define HRTIM_TIMEVENTLATCH_DISABLED    (0x00000000U)           /*!< Event is ignored if it happens during a blank, or passed through during a window */
#define HRTIM_TIMEVENTLATCH_ENABLED     HRTIM_EEFR1_EE1LTCH     /*!< Event is latched and delayed till the end of the blanking or windowing period */
/**
  * end of HRTIM_Timer_External_Event_Latch @}
  */

/** @defgroup HRTIM_Timer_External_Event HRTIM Timer External Event Counter A or B
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the External Event Counter A or B
  *
  */

#define HRTIM_EVENTCOUNTER_A    (HRTIM_EEFR3_EEVACE)           /*!< External Event Counter A */
/**
  * end of HRTIM_Timer_External_Event @}
  */

/** @defgroup HRTIM_Timer_External_Event_ResetMode HRTIM Timer External Counter Reset Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants enabling the External Event Counter A or B Reset Mode
  *
  */

#define HRTIM_EVENTCOUNTER_RSTMODE_UNCONDITIONAL   (0x00000000U)   /*!< External Event Counter is reset on each reset / roll-over event */
#define HRTIM_EVENTCOUNTER_RSTMODE_CONDITIONAL     (0x00000001U)   /*!< External Event Counter is reset on each reset / roll-over event only
                                                                        if no event occurs during last counting period */
/**
  * end of HRTIM_Timer_External_Event_ResetMode @}
  */

/** @defgroup HRTIM_Timer_ReSyncUpdate HRTIM Timer Re-Synchronized update
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the update coming condition
  *
  */

#define HRTIM_TIMERESYNC_UPDATE_UNCONDITIONAL   (0x00000000U)   /*!< update taken into account immediately */
#define HRTIM_TIMERESYNC_UPDATE_CONDITIONAL     (0x00000001U)   /*!< update taken into account on the following Reset/Roll-over event */

/**
  * end of HRTIM_Timer_ReSyncUpdate @}
  */

/** @defgroup HRTIM_Deadtime_Prescaler_Ratio HRTIM Dead-time Prescaler Ratio
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining division ratio between the timer clock frequency
  *        (fHRTIM) and the dead-time generator clock (fDTG)
  */

#define HRTIM_TIMDEADTIME_PRESCALERRATIO_MUL8    (0x00000000U)                                                   /*!< fDTG = fHRTIM * 8U */
#define HRTIM_TIMDEADTIME_PRESCALERRATIO_MUL4    (HRTIM_DTR_DTPRSC_BIT0)                                            /*!< fDTG = fHRTIM * 4U */
#define HRTIM_TIMDEADTIME_PRESCALERRATIO_MUL2    (HRTIM_DTR_DTPRSC_BIT1)                                            /*!< fDTG = fHRTIM * 2U */
#define HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV1    (HRTIM_DTR_DTPRSC_BIT1 | HRTIM_DTR_DTPRSC_BIT0)                       /*!< fDTG = fHRTIM */
#define HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV2    (HRTIM_DTR_DTPRSC_BIT2)                                            /*!< fDTG = fHRTIM / 2U */
#define HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV4    (HRTIM_DTR_DTPRSC_BIT2 | HRTIM_DTR_DTPRSC_BIT0)                       /*!< fDTG = fHRTIM / 4U */
#define HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV8    (HRTIM_DTR_DTPRSC_BIT2 | HRTIM_DTR_DTPRSC_BIT1)                       /*!< fDTG = fHRTIM / 8U */
#define HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV16   (HRTIM_DTR_DTPRSC_BIT2 | HRTIM_DTR_DTPRSC_BIT1 | HRTIM_DTR_DTPRSC_BIT0)  /*!< fDTG = fHRTIM / 16U */
/**
  * end of HRTIM_Deadtime_Prescaler_Ratio @}
  */

/** @defgroup HRTIM_Deadtime_Rising_Sign HRTIM Dead-time Rising Sign
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**Constants defining whether the dead-time is positive or negative
  *        (overlapping signal) on rising edgeConstants defining the timer indexes
  *
  */

#define HRTIM_TIMDEADTIME_RISINGSIGN_POSITIVE    (0x00000000U)           /*!< Positive dead-time on rising edge */
#define HRTIM_TIMDEADTIME_RISINGSIGN_NEGATIVE    (HRTIM_DTR_SDTR)        /*!< Negative dead-time on rising edge */
/**
  * end of HRTIM_Deadtime_Rising_Sign @}
  */

/** @defgroup HRTIM_Deadtime_Rising_Lock HRTIM Dead-time Rising Lock
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not the dead-time (rising sign and
  *        value) is write protected
  *
  */

#define HRTIM_TIMDEADTIME_RISINGLOCK_WRITE    (0x00000000U)           /*!< Dead-time rising value and sign is writeable */
#define HRTIM_TIMDEADTIME_RISINGLOCK_READONLY (HRTIM_DTR_DTRLK)       /*!< Dead-time rising value and sign is read-only */
/**
  * end of HRTIM_Deadtime_Rising_Lock @}
  */

/** @defgroup HRTIM_Deadtime_Rising_Sign_Lock HRTIM Dead-time Rising Sign Lock
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not the dead-time rising sign is write
  *        protected
  *
  */

#define HRTIM_TIMDEADTIME_RISINGSIGNLOCK_WRITE    (0x00000000U)           /*!< Dead-time rising sign is writeable */
#define HRTIM_TIMDEADTIME_RISINGSIGNLOCK_READONLY (HRTIM_DTR_DTRSLK)      /*!< Dead-time rising sign is read-only */
/**
  * end of HRTIM_Deadtime_Rising_Lock @}
  */

/** @defgroup HRTIM_Deadtime_Falling_Sign HRTIM Dead-time Falling Sign
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether the dead-time is positive or negative
  *        (overlapping signal) on falling edge
  *
  */

#define HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE    (0x00000000U)           /*!< Positive dead-time on falling edge */
#define HRTIM_TIMDEADTIME_FALLINGSIGN_NEGATIVE    (HRTIM_DTR_SDTF)        /*!< Negative dead-time on falling edge */
/**
  * end of HRTIM_Deadtime_Falling_Sign @}
  */

/** @defgroup HRTIM_Deadtime_Falling_Lock HRTIM Dead-time Falling Lock
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not the dead-time (falling sign and
  *        value) is write protected
  *
  */

#define HRTIM_TIMDEADTIME_FALLINGLOCK_WRITE    (0x00000000U)           /*!< Dead-time falling value and sign is writeable */
#define HRTIM_TIMDEADTIME_FALLINGLOCK_READONLY (HRTIM_DTR_DTFLK)       /*!< Dead-time falling value and sign is read-only */
/**
  * end of HRTIM_Deadtime_Falling_Lock @}
  */

/** @defgroup HRTIM_Deadtime_Falling_Sign_Lock HRTIM Dead-time Falling Sign Lock
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not the dead-time falling sign is write
  *        protected
  *
  */

#define HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_WRITE    (0x00000000U)           /*!< Dead-time falling sign is writeable */
#define HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_READONLY (HRTIM_DTR_DTFSLK)      /*!< Dead-time falling sign is read-only */
/**
  * end of HRTIM_Deadtime_Falling_Sign_Lock @}
  */

/** @defgroup HRTIM_Chopper_Frequency HRTIM Chopper Frequency
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the frequency of the generated high frequency carrier
  *
  */

#define HRTIM_CHOPPER_PRESCALERRATIO_DIV16  (0x000000U)                                                                     /*!< fCHPFRQ = fHRTIM / 16  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV32  (HRTIM_CHPR_CARFRQ_BIT0)                                                                    /*!< fCHPFRQ = fHRTIM / 32  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV48  (HRTIM_CHPR_CARFRQ_BIT1)                                                                    /*!< fCHPFRQ = fHRTIM / 48  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV64  (HRTIM_CHPR_CARFRQ_BIT1 | HRTIM_CHPR_CARFRQ_BIT0)                                              /*!< fCHPFRQ = fHRTIM / 64  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV80  (HRTIM_CHPR_CARFRQ_BIT2)                                                                    /*!< fCHPFRQ = fHRTIM / 80  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV96  (HRTIM_CHPR_CARFRQ_BIT2 | HRTIM_CHPR_CARFRQ_BIT0)                                              /*!< fCHPFRQ = fHRTIM / 96  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV112 (HRTIM_CHPR_CARFRQ_BIT2 | HRTIM_CHPR_CARFRQ_BIT1)                                              /*!< fCHPFRQ = fHRTIM / 112  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV128 (HRTIM_CHPR_CARFRQ_BIT2 | HRTIM_CHPR_CARFRQ_BIT1 | HRTIM_CHPR_CARFRQ_BIT0)                        /*!< fCHPFRQ = fHRTIM / 128  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV144 (HRTIM_CHPR_CARFRQ_BIT3)                                                                    /*!< fCHPFRQ = fHRTIM / 144  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV160 (HRTIM_CHPR_CARFRQ_BIT3 | HRTIM_CHPR_CARFRQ_BIT0)                                              /*!< fCHPFRQ = fHRTIM / 160  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV176 (HRTIM_CHPR_CARFRQ_BIT3 | HRTIM_CHPR_CARFRQ_BIT1)                                              /*!< fCHPFRQ = fHRTIM / 176  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV192 (HRTIM_CHPR_CARFRQ_BIT3 | HRTIM_CHPR_CARFRQ_BIT1 | HRTIM_CHPR_CARFRQ_BIT0)                        /*!< fCHPFRQ = fHRTIM / 192  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV208 (HRTIM_CHPR_CARFRQ_BIT3 | HRTIM_CHPR_CARFRQ_BIT2)                                              /*!< fCHPFRQ = fHRTIM / 208  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV224 (HRTIM_CHPR_CARFRQ_BIT3 | HRTIM_CHPR_CARFRQ_BIT2 | HRTIM_CHPR_CARFRQ_BIT0)                        /*!< fCHPFRQ = fHRTIM / 224  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV240 (HRTIM_CHPR_CARFRQ_BIT3 | HRTIM_CHPR_CARFRQ_BIT2 | HRTIM_CHPR_CARFRQ_BIT1)                        /*!< fCHPFRQ = fHRTIM / 240  */
#define HRTIM_CHOPPER_PRESCALERRATIO_DIV256 (HRTIM_CHPR_CARFRQ_BIT3 | HRTIM_CHPR_CARFRQ_BIT2 | HRTIM_CHPR_CARFRQ_BIT1 | HRTIM_CHPR_CARFRQ_BIT0)  /*!< fCHPFRQ = fHRTIM / 256  */

/**
  * end of HRTIM_Chopper_Frequency @}
  */

/** @defgroup HRTIM_Chopper_Duty_Cycle HRTIM Chopper Duty Cycle
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the duty cycle of the generated high frequency carrier
  *        Duty cycle can be adjusted by 1/8 step (from 0/8 up to 7/8)
  *
  */

#define HRTIM_CHOPPER_DUTYCYCLE_0    (0x000000U)                                                       /*!< Only 1st pulse is present */
#define HRTIM_CHOPPER_DUTYCYCLE_125  (HRTIM_CHPR_CARDTY_BIT0)                                             /*!< Duty cycle of the carrier signal is 12.5U % */
#define HRTIM_CHOPPER_DUTYCYCLE_250  (HRTIM_CHPR_CARDTY_BIT1)                                             /*!< Duty cycle of the carrier signal is 25U % */
#define HRTIM_CHOPPER_DUTYCYCLE_375  (HRTIM_CHPR_CARDTY_BIT1 | HRTIM_CHPR_CARDTY_BIT0)                       /*!< Duty cycle of the carrier signal is 37.5U % */
#define HRTIM_CHOPPER_DUTYCYCLE_500  (HRTIM_CHPR_CARDTY_BIT2)                                             /*!< Duty cycle of the carrier signal is 50U % */
#define HRTIM_CHOPPER_DUTYCYCLE_625  (HRTIM_CHPR_CARDTY_BIT2 | HRTIM_CHPR_CARDTY_BIT0)                       /*!< Duty cycle of the carrier signal is 62.5U % */
#define HRTIM_CHOPPER_DUTYCYCLE_750  (HRTIM_CHPR_CARDTY_BIT2 | HRTIM_CHPR_CARDTY_BIT1)                       /*!< Duty cycle of the carrier signal is 75U % */
#define HRTIM_CHOPPER_DUTYCYCLE_875  (HRTIM_CHPR_CARDTY_BIT2 | HRTIM_CHPR_CARDTY_BIT1 | HRTIM_CHPR_CARDTY_BIT0) /*!< Duty cycle of the carrier signal is 87.5U % */
/**
  * end of HRTIM_Chopper_Duty_Cycle @}
  */

/** @defgroup HRTIM_Chopper_Start_Pulse_Width HRTIM Chopper Start Pulse Width
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the pulse width of the first pulse of the generated
  *        high frequency carrier
  *
  */

#define HRTIM_CHOPPER_PULSEWIDTH_16   (0x000000U)                                                                          /*!< tSTPW = tHRTIM x 16  */
#define HRTIM_CHOPPER_PULSEWIDTH_32   (HRTIM_CHPR_STRPW_BIT0)                                                                 /*!< tSTPW = tHRTIM x 32  */
#define HRTIM_CHOPPER_PULSEWIDTH_48   (HRTIM_CHPR_STRPW_BIT1)                                                                 /*!< tSTPW = tHRTIM x 48  */
#define HRTIM_CHOPPER_PULSEWIDTH_64   (HRTIM_CHPR_STRPW_BIT1 | HRTIM_CHPR_STRPW_BIT0)                                            /*!< tSTPW = tHRTIM x 64  */
#define HRTIM_CHOPPER_PULSEWIDTH_80   (HRTIM_CHPR_STRPW_BIT2)                                                                 /*!< tSTPW = tHRTIM x 80  */
#define HRTIM_CHOPPER_PULSEWIDTH_96   (HRTIM_CHPR_STRPW_BIT2 | HRTIM_CHPR_STRPW_BIT0)                                            /*!< tSTPW = tHRTIM x 96  */
#define HRTIM_CHOPPER_PULSEWIDTH_112  (HRTIM_CHPR_STRPW_BIT2 | HRTIM_CHPR_STRPW_BIT1)                                            /*!< tSTPW = tHRTIM x 112  */
#define HRTIM_CHOPPER_PULSEWIDTH_128  (HRTIM_CHPR_STRPW_BIT2 | HRTIM_CHPR_STRPW_BIT1 | HRTIM_CHPR_STRPW_BIT0)                       /*!< tSTPW = tHRTIM x 128  */
#define HRTIM_CHOPPER_PULSEWIDTH_144  (HRTIM_CHPR_STRPW_BIT3)                                                                 /*!< tSTPW = tHRTIM x 144  */
#define HRTIM_CHOPPER_PULSEWIDTH_160  (HRTIM_CHPR_STRPW_BIT3 | HRTIM_CHPR_STRPW_BIT0)                                            /*!< tSTPW = tHRTIM x 160  */
#define HRTIM_CHOPPER_PULSEWIDTH_176  (HRTIM_CHPR_STRPW_BIT3 | HRTIM_CHPR_STRPW_BIT1)                                            /*!< tSTPW = tHRTIM x 176  */
#define HRTIM_CHOPPER_PULSEWIDTH_192  (HRTIM_CHPR_STRPW_BIT3 | HRTIM_CHPR_STRPW_BIT1 | HRTIM_CHPR_STRPW_BIT0)                       /*!< tSTPW = tHRTIM x 192  */
#define HRTIM_CHOPPER_PULSEWIDTH_208  (HRTIM_CHPR_STRPW_BIT3 | HRTIM_CHPR_STRPW_BIT2)                                            /*!< tSTPW = tHRTIM x 208  */
#define HRTIM_CHOPPER_PULSEWIDTH_224  (HRTIM_CHPR_STRPW_BIT3 | HRTIM_CHPR_STRPW_BIT2 | HRTIM_CHPR_STRPW_BIT0)                       /*!< tSTPW = tHRTIM x 224  */
#define HRTIM_CHOPPER_PULSEWIDTH_240  (HRTIM_CHPR_STRPW_BIT3 | HRTIM_CHPR_STRPW_BIT2 | HRTIM_CHPR_STRPW_BIT1)                       /*!< tSTPW = tHRTIM x 240  */
#define HRTIM_CHOPPER_PULSEWIDTH_256  (HRTIM_CHPR_STRPW_BIT3 | HRTIM_CHPR_STRPW_BIT2 | HRTIM_CHPR_STRPW_BIT1 | HRTIM_CHPR_STRPW_BIT0)  /*!< tSTPW = tHRTIM x 256  */
/**
  * end of HRTIM_Chopper_Start_Pulse_Width @}
  */

/** @defgroup HRTIM_Synchronization_Options HRTIM Synchronization Options
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the options for synchronizing multiple HRTIM
  *        instances, as a master unit (generating a synchronization signal)
  *        or as a slave (waiting for a trigger to be synchronized)
  */

#define HRTIM_SYNCOPTION_NONE   0x00000000U   /*!< HRTIM instance doesn't handle external synchronization signals (SYNCIN, SYNCOUT) */
#define HRTIM_SYNCOPTION_MASTER 0x00000001U   /*!< HRTIM instance acts as a MASTER, i.e. generates external synchronization output (SYNCOUT)*/
#define HRTIM_SYNCOPTION_SLAVE  0x00000002U   /*!< HRTIM instance acts as a SLAVE, i.e. it is synchronized by external sources (SYNCIN) */
/**
  * end of HRTIM_Synchronization_Options @}
  */

/** @defgroup HRTIM_Synchronization_Input_Source HRTIM Synchronization Input Source
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining defining the synchronization input source
  *
  */

#define HRTIM_SYNCINPUTSOURCE_NONE           0x00000000U                                  /*!< disabled. HRTIM is not synchronized and runs in standalone mode */
#define HRTIM_SYNCINPUTSOURCE_INTERNALEVENT  HRTIM_MCR_SYNC_IN_BIT1                          /*!< The HRTIM is synchronized with the on-chip timer */
#define HRTIM_SYNCINPUTSOURCE_EXTERNALEVENT  (HRTIM_MCR_SYNC_IN_BIT1 | HRTIM_MCR_SYNC_IN_BIT0)  /*!< A positive pulse on SYNCIN input triggers the HRTIM */
/**
  * end of HRTIM_Synchronization_Input_Source @}
  */

/** @defgroup HRTIM_Synchronization_Output_Source HRTIM Synchronization Output Source
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the source and event to be sent on the
  *        synchronization outputs
  *
  */

#define HRTIM_SYNCOUTPUTSOURCE_MASTER_START 0x00000000U                                    /*!< A pulse is sent on HRTIM_SCOUT output and hrtim_out_sync2 upon master timer start event      */
#define HRTIM_SYNCOUTPUTSOURCE_MASTER_CMP1  (HRTIM_MCR_SYNC_SRC_BIT0)                         /*!< A pulse is sent on HRTIM_SCOUT output and hrtim_out_sync2 upon master timer compare 1 event  */
#define HRTIM_SYNCOUTPUTSOURCE_TIMA_START   (HRTIM_MCR_SYNC_SRC_BIT1)                         /*!< A pulse is sent on HRTIM_SCOUT output and hrtim_out_sync2 upon timer A start or reset events */
#define HRTIM_SYNCOUTPUTSOURCE_TIMA_CMP1    (HRTIM_MCR_SYNC_SRC_BIT1 | HRTIM_MCR_SYNC_SRC_BIT0)  /*!< A pulse is sent on HRTIM_SCOUT output and hrtim_out_sync2 upon timer A compare 1 event       */
/**
  * end of HRTIM_Synchronization_Output_Source @}
  */

/** @defgroup HRTIM_Synchronization_Output_Polarity HRTIM Synchronization Output Polarity
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the routing and conditioning of the synchronization output event
  *
  */

#define HRTIM_SYNCOUTPUTPOLARITY_NONE      0x00000000U                                   /*!< Synchronization output event is disabled */
#define HRTIM_SYNCOUTPUTPOLARITY_POSITIVE  (HRTIM_MCR_SYNC_OUT_BIT1)                        /*!< SCOUT pin has a low idle level and issues a positive pulse of 16 fHRTIM clock cycles length for the synchronization */
#define HRTIM_SYNCOUTPUTPOLARITY_NEGATIVE  (HRTIM_MCR_SYNC_OUT_BIT1 | HRTIM_MCR_SYNC_OUT_BIT0) /*!< SCOUT pin has a high idle level and issues a negative pulse of 16 fHRTIM clock cycles length for the synchronization */
/**
  * end of HRTIM_Synchronization_Output_Polarity @}
  */

/** @defgroup HRTIM_External_Event_Sources HRTIM External Event Sources
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining available sources associated to external events
  *
  */

#define HRTIM_EEV1SRC_GPIO        0x00000000U                                         /*!< External event source 1U for External Event 1 */
#define HRTIM_EEV2SRC_GPIO        0x00000000U                                         /*!< External event source 1U for External Event 2 */
#define HRTIM_EEV3SRC_GPIO        0x00000000U                                         /*!< External event source 1U for External Event 3 */
#define HRTIM_EEV4SRC_GPIO        0x00000000U                                         /*!< External event source 1U for External Event 4 */
#define HRTIM_EEV5SRC_GPIO        0x00000000U                                         /*!< External event source 1U for External Event 5 */
#define HRTIM_EEV6SRC_GPIO        0x00000000U                                         /*!< External event source 1U for External Event 6 */
#define HRTIM_EEV7SRC_GPIO        0x00000000U                                         /*!< External event source 1U for External Event 7 */
#define HRTIM_EEV8SRC_GPIO        0x00000000U                                         /*!< External event source 1U for External Event 8 */
#define HRTIM_EEV9SRC_GPIO        0x00000000U                                         /*!< External event source 1U for External Event 9 */
#define HRTIM_EEV10SRC_GPIO       0x00000000U                                         /*!< External event source 1U for External Event 10 */
#define HRTIM_EEV1SRC_COMP2_OUT   (                          HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 2U for External Event 1 */
#define HRTIM_EEV2SRC_COMP4_OUT   (                          HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 2U for External Event 2 */
#define HRTIM_EEV3SRC_COMP6_OUT   (                          HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 2U for External Event 3 */
#define HRTIM_EEV4SRC_COMP1_OUT   (                          HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 2U for External Event 4 */
#define HRTIM_EEV5SRC_COMP3_OUT   (                          HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 2U for External Event 5 */
#define HRTIM_EEV6SRC_COMP2_OUT   (                          HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 2U for External Event 6 */
#define HRTIM_EEV7SRC_COMP4_OUT   (                          HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 2U for External Event 7 */
#define HRTIM_EEV8SRC_COMP6_OUT   (                          HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 2U for External Event 8 */
#define HRTIM_EEV9SRC_COMP5_OUT   (                          HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 2U for External Event 9 */
#define HRTIM_EEV10SRC_COMP7_OUT  (                          HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 2U for External Event 10 */
#define HRTIM_EEV1SRC_TIM1_TRGO   (HRTIM_EECR1_EE1SRC_BIT1                          ) /*!< External event source 3U for External Event 1 */
#define HRTIM_EEV2SRC_TIM2_TRGO   (HRTIM_EECR1_EE1SRC_BIT1                          ) /*!< External event source 3U for External Event 2 */
#define HRTIM_EEV3SRC_TIM3_TRGO   (HRTIM_EECR1_EE1SRC_BIT1                          ) /*!< External event source 3U for External Event 3 */
#define HRTIM_EEV4SRC_COMP5_OUT   (HRTIM_EECR1_EE1SRC_BIT1                          ) /*!< External event source 3U for External Event 4 */
#define HRTIM_EEV5SRC_COMP7_OUT   (HRTIM_EECR1_EE1SRC_BIT1                          ) /*!< External event source 3U for External Event 5 */
#define HRTIM_EEV6SRC_COMP1_OUT   (HRTIM_EECR1_EE1SRC_BIT1                          ) /*!< External event source 3U for External Event 6 */
#define HRTIM_EEV7SRC_TIM7_TRGO   (HRTIM_EECR1_EE1SRC_BIT1                          ) /*!< External event source 3U for External Event 7 */
#define HRTIM_EEV8SRC_COMP3_OUT   (HRTIM_EECR1_EE1SRC_BIT1                          ) /*!< External event source 3U for External Event 8 */
#define HRTIM_EEV9SRC_TIM15_TRGO  (HRTIM_EECR1_EE1SRC_BIT1                          ) /*!< External event source 3U for External Event 9 */
#define HRTIM_EEV10SRC_TIM6_TRGO  (HRTIM_EECR1_EE1SRC_BIT1                          ) /*!< External event source 3U for External Event 10 */
#define HRTIM_EEV1SRC_ADC1_AWD1   (HRTIM_EECR1_EE1SRC_BIT1 | HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 4U for External Event 1 */
#define HRTIM_EEV2SRC_ADC1_AWD2   (HRTIM_EECR1_EE1SRC_BIT1 | HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 4U for External Event 2 */
#define HRTIM_EEV3SRC_ADC1_AWD3   (HRTIM_EECR1_EE1SRC_BIT1 | HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 4U for External Event 3 */
#define HRTIM_EEV4SRC_ADC2_AWD1   (HRTIM_EECR1_EE1SRC_BIT1 | HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 4U for External Event 4 */
#define HRTIM_EEV5SRC_ADC2_AWD2   (HRTIM_EECR1_EE1SRC_BIT1 | HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 4U for External Event 5 */
#define HRTIM_EEV6SRC_ADC2_AWD3   (HRTIM_EECR1_EE1SRC_BIT1 | HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 4U for External Event 6 */
#define HRTIM_EEV7SRC_ADC3_AWD1   (HRTIM_EECR1_EE1SRC_BIT1 | HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 4U for External Event 7 */
#define HRTIM_EEV8SRC_ADC4_AWD1   (HRTIM_EECR1_EE1SRC_BIT1 | HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 4U for External Event 8 */
#define HRTIM_EEV9SRC_COMP4_OUT   (HRTIM_EECR1_EE1SRC_BIT1 | HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 4U for External Event 9 */
#define HRTIM_EEV10SRC_ADC5_AWD1  (HRTIM_EECR1_EE1SRC_BIT1 | HRTIM_EECR1_EE1SRC_BIT0) /*!< External event source 4U for External Event 10 */
/**
  * end of HRTIM_External_Event_Sources @}
  */

/** @defgroup HRTIM_External_Event_Polarity HRTIM External Event Polarity
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the polarity of an external event
  *
  */

#define HRTIM_EVENTPOLARITY_HIGH    (0x00000000U)           /*!< External event is active high */
#define HRTIM_EVENTPOLARITY_LOW     (HRTIM_EECR1_EE1POL)    /*!< External event is active low */

/**
  * end of HRTIM_External_Event_Polarity @}
  */

/** @defgroup HRTIM_External_Event_Sensitivity HRTIM External Event Sensitivity
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the sensitivity (level-sensitive or edge-sensitive)
  *        of an external event
  *
  */

#define HRTIM_EVENTSENSITIVITY_LEVEL          (0x00000000U)                                        /*!< External event is active on level */
#define HRTIM_EVENTSENSITIVITY_RISINGEDGE     (HRTIM_EECR1_EE1SNS_BIT0)                            /*!< External event is active on Rising edge */
#define HRTIM_EVENTSENSITIVITY_FALLINGEDGE    (HRTIM_EECR1_EE1SNS_BIT1)                            /*!< External event is active on Falling edge */
#define HRTIM_EVENTSENSITIVITY_BOTHEDGES      (HRTIM_EECR1_EE1SNS_BIT1 | HRTIM_EECR1_EE1SNS_BIT0)  /*!< External event is active on Rising and Falling edges */

/**
  * end of HRTIM_External_Event_Sensitivity @}
  */

/** @defgroup HRTIM_External_Event_Fast_Mode HRTIM External Event Fast Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not an external event is programmed in
  *        fast mode
  *
  */

#define HRTIM_EVENTFASTMODE_DISABLE    (0x00000000U)               /*!< External Event is re-synchronized by the HRTIM logic before acting on outputs */
#define HRTIM_EVENTFASTMODE_ENABLE     (HRTIM_EECR1_EE1FAST)       /*!< External Event is acting asynchronously on outputs (low latency mode) */

/**
  * end of HRTIM_External_Event_Fast_Mode @}
  */

/** @defgroup HRTIM_External_Event_Filter HRTIM External Event Filter
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the frequency used to sample an external event 6
  *        input and the length (N) of the digital filter applied
  *
  */

#define HRTIM_EVENTFILTER_NONE      (0x00000000U)                                                                         /*!< Filter disabled */
#define HRTIM_EVENTFILTER_1         (HRTIM_EECR3_EE6F_BIT0)                                                                  /*!< fSAMPLING= fHRTIM, N=2U */
#define HRTIM_EVENTFILTER_2         (HRTIM_EECR3_EE6F_BIT1)                                                                  /*!< fSAMPLING= fHRTIM, N=4U */
#define HRTIM_EVENTFILTER_3         (HRTIM_EECR3_EE6F_BIT1 | HRTIM_EECR3_EE6F_BIT0)                                             /*!< fSAMPLING= fHRTIM, N=8U */
#define HRTIM_EVENTFILTER_4         (HRTIM_EECR3_EE6F_BIT2)                                                                  /*!< fSAMPLING= fEEVS/2U, N=6U */
#define HRTIM_EVENTFILTER_5         (HRTIM_EECR3_EE6F_BIT2 | HRTIM_EECR3_EE6F_BIT0)                                             /*!< fSAMPLING= fEEVS/2U, N=8U */
#define HRTIM_EVENTFILTER_6         (HRTIM_EECR3_EE6F_BIT2 | HRTIM_EECR3_EE6F_BIT1)                                             /*!< fSAMPLING= fEEVS/4U, N=6U */
#define HRTIM_EVENTFILTER_7         (HRTIM_EECR3_EE6F_BIT2 | HRTIM_EECR3_EE6F_BIT1 | HRTIM_EECR3_EE6F_BIT0)                        /*!< fSAMPLING= fEEVS/4U, N=8U */
#define HRTIM_EVENTFILTER_8         (HRTIM_EECR3_EE6F_BIT3)                                                                  /*!< fSAMPLING= fEEVS/8U, N=6U */
#define HRTIM_EVENTFILTER_9         (HRTIM_EECR3_EE6F_BIT3 | HRTIM_EECR3_EE6F_BIT0)                                             /*!< fSAMPLING= fEEVS/8U, N=8U */
#define HRTIM_EVENTFILTER_10        (HRTIM_EECR3_EE6F_BIT3 | HRTIM_EECR3_EE6F_BIT1)                                             /*!< fSAMPLING= fEEVS/16U, N=5U */
#define HRTIM_EVENTFILTER_11        (HRTIM_EECR3_EE6F_BIT3 | HRTIM_EECR3_EE6F_BIT1 | HRTIM_EECR3_EE6F_BIT0)                        /*!< fSAMPLING= fEEVS/16U, N=6U */
#define HRTIM_EVENTFILTER_12        (HRTIM_EECR3_EE6F_BIT3 | HRTIM_EECR3_EE6F_BIT2)                                             /*!< fSAMPLING= fEEVS/16U, N=8U */
#define HRTIM_EVENTFILTER_13        (HRTIM_EECR3_EE6F_BIT3 | HRTIM_EECR3_EE6F_BIT2  | HRTIM_EECR3_EE6F_BIT0)                       /*!< fSAMPLING= fEEVS/32U, N=5U */
#define HRTIM_EVENTFILTER_14        (HRTIM_EECR3_EE6F_BIT3 | HRTIM_EECR3_EE6F_BIT2  | HRTIM_EECR3_EE6F_BIT1)                       /*!< fSAMPLING= fEEVS/32U, N=6U */
#define HRTIM_EVENTFILTER_15        (HRTIM_EECR3_EE6F_BIT3 | HRTIM_EECR3_EE6F_BIT2  | HRTIM_EECR3_EE6F_BIT1 | HRTIM_EECR3_EE6F_BIT0)  /*!< fSAMPLING= fEEVS/32U, N=8U */

/**
  * end of HRTIM_External_Event_Filter @}
  */

/** @defgroup HRTIM_External_Event_Prescaler HRTIM External Event Prescaler
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining division ratio between the timer clock frequency
  *        fHRTIM) and the external event signal sampling clock (fEEVS)
  *        used by the digital filters
  *
  */

#define HRTIM_EVENTPRESCALER_DIV1    (0x00000000U)                                       /*!< fEEVS=fHRTIM */
#define HRTIM_EVENTPRESCALER_DIV2    (                         HRTIM_EECR3_EEVSD_BIT0)   /*!< fEEVS=fHRTIM / 2U */
#define HRTIM_EVENTPRESCALER_DIV4    (HRTIM_EECR3_EEVSD_BIT1                         )   /*!< fEEVS=fHRTIM / 4U */
#define HRTIM_EVENTPRESCALER_DIV8    (HRTIM_EECR3_EEVSD_BIT1 | HRTIM_EECR3_EEVSD_BIT0)   /*!< fEEVS=fHRTIM / 8U */

/**
  * end of HRTIM_External_Event_Prescaler @}
  */

/** @defgroup HRTIM_Fault_Sources HRTIM Fault Sources
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether a fault is triggered by any external
  *        or internal fault source
  *
  */

#define HRTIM_FAULTSOURCE_DIGITALINPUT      (0x00000000U)              /*!< Fault input is FLT input pin */
#define HRTIM_FAULTSOURCE_INTERNAL          (0x00000001U)              /*!< Fault input is FLT_Int signal (e.g. internal comparator) */
#define HRTIM_FAULTSOURCE_EEVINPUT          (0x00000002U)              /*!< Fault input is EEV pin */

/**
  * end of HRTIM_Fault_Sources @}
  */

/** @defgroup HRTIM_Fault_Polarity HRTIM Fault Polarity
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the polarity of a fault event
  *
  */

#define HRTIM_FAULTPOLARITY_LOW     (0x00000000U)            /*!< Fault input is active low */
#define HRTIM_FAULTPOLARITY_HIGH    (HRTIM_FLTINR1_FLT1P)    /*!< Fault input is active high */

/**
  * end of HRTIM_Fault_Polarity @}
  */

/** @defgroup HRTIM_Fault_Blanking HRTIM Fault Blanking Source
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the blanking source of a fault event
  *
  */

#define HRTIM_FAULTBLANKINGMODE_RSTALIGNED  (0x00000000U)     /*!< Fault blanking source is Reset-aligned window */
#define HRTIM_FAULTBLANKINGMODE_MOVING      (0x00000001U)     /*!< Fault blanking source is Moving window */

/**
  * end of HRTIM_Fault_Blanking @}
  */

/** @defgroup HRTIM_Fault_ResetMode HRTIM Fault Reset Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the Counter reset mode of a fault event
  *
  */

#define HRTIM_FAULTCOUNTERRST_UNCONDITIONAL  (0x00000000U)       /*!< Fault counter is reset on each reset / roll-over event */
#define HRTIM_FAULTCOUNTERRST_CONDITIONAL    (0x00000001U)       /*!< Fault counter is reset on each reset / roll-over event only if no fault occurred during last countingperiod.*/

/**
  * end of HRTIM_Fault_ResetMode @}
  */

/** @defgroup HRTIM_Fault_Blanking_Control  HRTIM Fault Blanking Control
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants used to enable or disable the blanking mode of a fault channel
  *
  */

#define HRTIM_FAULTBLANKINGCTL_DISABLED 0x00000000U /*!< No blanking on Fault */
#define HRTIM_FAULTBLANKINGCTL_ENABLED  0x00000001U /*!< Fault blanking mode */

/**
  * end of HRTIM_Fault_Blanking_Control @}
  */

/** @defgroup HRTIM_Fault_Filter HRTIM Fault Filter
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the frequency used to sample the fault input and
  *        the length (N) of the digital filter applied
  *
  */

#define HRTIM_FAULTFILTER_NONE      (0x00000000U)                                                                                    /*!< Filter disabled */
#define HRTIM_FAULTFILTER_1         (HRTIM_FLTINR1_FLT1F_BIT0)                                                                          /*!< fSAMPLING= fHRTIM, N=2U */
#define HRTIM_FAULTFILTER_2         (HRTIM_FLTINR1_FLT1F_BIT1)                                                                          /*!< fSAMPLING= fHRTIM, N=4U */
#define HRTIM_FAULTFILTER_3         (HRTIM_FLTINR1_FLT1F_BIT1 | HRTIM_FLTINR1_FLT1F_BIT0)                                                  /*!< fSAMPLING= fHRTIM, N=8U */
#define HRTIM_FAULTFILTER_4         (HRTIM_FLTINR1_FLT1F_BIT2)                                                                          /*!< fSAMPLING= fFLTS/2U, N=6U */
#define HRTIM_FAULTFILTER_5         (HRTIM_FLTINR1_FLT1F_BIT2 | HRTIM_FLTINR1_FLT1F_BIT0)                                                  /*!< fSAMPLING= fFLTS/2U, N=8U */
#define HRTIM_FAULTFILTER_6         (HRTIM_FLTINR1_FLT1F_BIT2 | HRTIM_FLTINR1_FLT1F_BIT1)                                                  /*!< fSAMPLING= fFLTS/4U, N=6U */
#define HRTIM_FAULTFILTER_7         (HRTIM_FLTINR1_FLT1F_BIT2 | HRTIM_FLTINR1_FLT1F_BIT1 | HRTIM_FLTINR1_FLT1F_BIT0)                          /*!< fSAMPLING= fFLTS/4U, N=8U */
#define HRTIM_FAULTFILTER_8         (HRTIM_FLTINR1_FLT1F_BIT3)                                                                          /*!< fSAMPLING= fFLTS/8U, N=6U */
#define HRTIM_FAULTFILTER_9         (HRTIM_FLTINR1_FLT1F_BIT3 | HRTIM_FLTINR1_FLT1F_BIT0)                                                  /*!< fSAMPLING= fFLTS/8U, N=8U */
#define HRTIM_FAULTFILTER_10        (HRTIM_FLTINR1_FLT1F_BIT3 | HRTIM_FLTINR1_FLT1F_BIT1)                                                  /*!< fSAMPLING= fFLTS/16U, N=5U */
#define HRTIM_FAULTFILTER_11        (HRTIM_FLTINR1_FLT1F_BIT3 | HRTIM_FLTINR1_FLT1F_BIT1 | HRTIM_FLTINR1_FLT1F_BIT0)                          /*!< fSAMPLING= fFLTS/16U, N=6U */
#define HRTIM_FAULTFILTER_12        (HRTIM_FLTINR1_FLT1F_BIT3 | HRTIM_FLTINR1_FLT1F_BIT2)                                                  /*!< fSAMPLING= fFLTS/16U, N=8U */
#define HRTIM_FAULTFILTER_13        (HRTIM_FLTINR1_FLT1F_BIT3 | HRTIM_FLTINR1_FLT1F_BIT2 | HRTIM_FLTINR1_FLT1F_BIT0)                          /*!< fSAMPLING= fFLTS/32U, N=5U */
#define HRTIM_FAULTFILTER_14        (HRTIM_FLTINR1_FLT1F_BIT3 | HRTIM_FLTINR1_FLT1F_BIT2 | HRTIM_FLTINR1_FLT1F_BIT1)                          /*!< fSAMPLING= fFLTS/32U, N=6U */
#define HRTIM_FAULTFILTER_15        (HRTIM_FLTINR1_FLT1F_BIT3 | HRTIM_FLTINR1_FLT1F_BIT2 | HRTIM_FLTINR1_FLT1F_BIT1 | HRTIM_FLTINR1_FLT1F_BIT0)  /*!< fSAMPLING= fFLTS/32U, N=8U */

/**
  * end of HRTIM_Fault_Filter @}
  */

/** @defgroup HRTIM_Fault_Counter HRTIM Fault counter threshold value
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the FAULT Counter threshold
  *
  */

#define HRTIM_FAULTCOUNTER_NONE     ((uint32_t)0U )  /*!< Counter threshold = 0U */
#define HRTIM_FAULTCOUNTER_1        ((uint32_t)1U )  /*!< Counter threshold = 1U */
#define HRTIM_FAULTCOUNTER_2        ((uint32_t)2U )  /*!< Counter threshold = 2U */
#define HRTIM_FAULTCOUNTER_3        ((uint32_t)3U )  /*!< Counter threshold = 3U */
#define HRTIM_FAULTCOUNTER_4        ((uint32_t)4U )  /*!< Counter threshold = 4U */
#define HRTIM_FAULTCOUNTER_5        ((uint32_t)5U )  /*!< Counter threshold = 5U */
#define HRTIM_FAULTCOUNTER_6        ((uint32_t)6U )  /*!< Counter threshold = 6U */
#define HRTIM_FAULTCOUNTER_7        ((uint32_t)7U )  /*!< Counter threshold = 7U */
#define HRTIM_FAULTCOUNTER_8        ((uint32_t)8U )  /*!< Counter threshold = 8U */
#define HRTIM_FAULTCOUNTER_9        ((uint32_t)9U )  /*!< Counter threshold = 9U */
#define HRTIM_FAULTCOUNTER_10       ((uint32_t)10U)  /*!< Counter threshold = 10U */
#define HRTIM_FAULTCOUNTER_11       ((uint32_t)11U)  /*!< Counter threshold = 11U */
#define HRTIM_FAULTCOUNTER_12       ((uint32_t)12U)  /*!< Counter threshold = 12U */
#define HRTIM_FAULTCOUNTER_13       ((uint32_t)13U)  /*!< Counter threshold = 13U */
#define HRTIM_FAULTCOUNTER_14       ((uint32_t)14U)  /*!< Counter threshold = 14U */
#define HRTIM_FAULTCOUNTER_15       ((uint32_t)15U)  /*!< Counter threshold = 15U */

/**
  * end of HRTIM_Fault_Counter @}
  */

/** @defgroup HRTIM_Fault_Lock HRTIM Fault Lock
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not the fault programming bits are
           write protected
  *
  */

#define HRTIM_FAULTLOCK_READWRITE       (0x00000000U)               /*!< Fault settings bits are read/write */
#define HRTIM_FAULTLOCK_READONLY        (HRTIM_FLTINR1_FLT1LCK)     /*!< Fault settings bits are read only */

/**
  * end of HRTIM_Fault_Lock @}
  */

/** @defgroup HRTIM_External_Fault_Prescaler HRTIM External Fault Prescaler
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the division ratio between the timer clock
  *        frequency (fHRTIM) and the fault signal sampling clock (fFLTS) used
  *        by the digital filters.
  *
  */

#define HRTIM_FAULTPRESCALER_DIV1    (0x00000000U)                                     /*!< fFLTS=fHRTIM */
#define HRTIM_FAULTPRESCALER_DIV2    (HRTIM_FLTINR2_FLTSD_BIT0)                           /*!< fFLTS=fHRTIM / 2U */
#define HRTIM_FAULTPRESCALER_DIV4    (HRTIM_FLTINR2_FLTSD_BIT1)                           /*!< fFLTS=fHRTIM / 4U */
#define HRTIM_FAULTPRESCALER_DIV8    (HRTIM_FLTINR2_FLTSD_BIT1 | HRTIM_FLTINR2_FLTSD_BIT0)   /*!< fFLTS=fHRTIM / 8U */

/**
  * end of HRTIM_External_Fault_Prescaler @}
  */

/** @defgroup HRTIM_Burst_Mode_Operating_Mode HRTIM Burst Mode Operating Mode
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining if the burst mode is entered once or if it is
  *        continuously operating
  *
  */

#define HRTIM_BURSTMODE_SINGLESHOT (0x00000000U)           /*!< Burst mode operates in single shot mode */
#define HRTIM_BURSTMODE_CONTINOUS   (HRTIM_BMCR_BMOM)      /*!< Burst mode operates in continuous mode */

/**
  * end of HRTIM_Burst_Mode_Operating_Mode @}
  */

/** @defgroup HRTIM_Burst_Mode_Clock_Source HRTIM Burst Mode Clock Source
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the clock source for the burst mode counter
  *
  */

#define HRTIM_BURSTMODECLOCKSOURCE_MASTER     (0x00000000U)                                                   /*!< Master timer counter reset/roll-over is used as clock source for the burst mode counter */
#define HRTIM_BURSTMODECLOCKSOURCE_TIMER_A    (HRTIM_BMCR_BMCLK_BIT0)                                            /*!< Timer A counter reset/roll-over is used as clock source for the burst mode counter */
#define HRTIM_BURSTMODECLOCKSOURCE_TIMER_B    (HRTIM_BMCR_BMCLK_BIT1)                                            /*!< Timer B counter reset/roll-over is used as clock source for the burst mode counter */
#define HRTIM_BURSTMODECLOCKSOURCE_TIMER_C    (HRTIM_BMCR_BMCLK_BIT1 | HRTIM_BMCR_BMCLK_BIT0)                       /*!< Timer C counter reset/roll-over is used as clock source for the burst mode counter */
#define HRTIM_BURSTMODECLOCKSOURCE_TIMER_D    (HRTIM_BMCR_BMCLK_BIT2)                                            /*!< Timer D counter reset/roll-over is used as clock source for the burst mode counter */
#define HRTIM_BURSTMODECLOCKSOURCE_TIMER_E    (HRTIM_BMCR_BMCLK_BIT2 | HRTIM_BMCR_BMCLK_BIT0)                       /*!< Timer E counter reset/roll-over is used as clock source for the burst mode counter */
#define HRTIM_BURSTMODECLOCKSOURCE_TIMER_F    (HRTIM_BMCR_BMCLK_BIT2 | HRTIM_BMCR_BMCLK_BIT0)                       /*!< Timer F counter reset/roll-over is used as clock source for the burst mode counter */
#define HRTIM_BURSTMODECLOCKSOURCE_TIM16_OC   (HRTIM_BMCR_BMCLK_BIT2 | HRTIM_BMCR_BMCLK_BIT1)                       /*!< On-chip Event 1 (BMClk[1]), acting as a burst mode counter clock */
#define HRTIM_BURSTMODECLOCKSOURCE_TIM17_OC   (HRTIM_BMCR_BMCLK_BIT2 | HRTIM_BMCR_BMCLK_BIT1 | HRTIM_BMCR_BMCLK_BIT0)  /*!< On-chip Event 2 (BMClk[2]), acting as a burst mode counter clock */
#define HRTIM_BURSTMODECLOCKSOURCE_TIM7_TRGO  (HRTIM_BMCR_BMCLK_BIT3)                                            /*!< On-chip Event 3 (BMClk[3]), acting as a burst mode counter clock */
#define HRTIM_BURSTMODECLOCKSOURCE_FHRTIM     (HRTIM_BMCR_BMCLK_BIT3 | HRTIM_BMCR_BMCLK_BIT1)                       /*!< Prescaled fHRTIM clock is used as clock source for the burst mode counter */

/**
  * end of HRTIM_Burst_Mode_Clock_Source @}
  */

/** @defgroup HRTIM_Burst_Mode_Prescaler HRTIM Burst Mode Prescaler
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the prescaling ratio of the fHRTIM clock
  *        for the burst mode controller
  *
  */

#define HRTIM_BURSTMODEPRESCALER_DIV1     (0x00000000U)                                                                           /*!< fBRST = fHRTIM */
#define HRTIM_BURSTMODEPRESCALER_DIV2     (HRTIM_BMCR_BMPRSC_BIT0)                                                                   /*!< fBRST = fHRTIM/2U */
#define HRTIM_BURSTMODEPRESCALER_DIV4     (HRTIM_BMCR_BMPRSC_BIT1)                                                                   /*!< fBRST = fHRTIM/4U */
#define HRTIM_BURSTMODEPRESCALER_DIV8     (HRTIM_BMCR_BMPRSC_BIT1 | HRTIM_BMCR_BMPRSC_BIT0)                                             /*!< fBRST = fHRTIM/8U */
#define HRTIM_BURSTMODEPRESCALER_DIV16    (HRTIM_BMCR_BMPRSC_BIT2)                                                                   /*!< fBRST = fHRTIM/16U */
#define HRTIM_BURSTMODEPRESCALER_DIV32    (HRTIM_BMCR_BMPRSC_BIT2 | HRTIM_BMCR_BMPRSC_BIT0)                                             /*!< fBRST = fHRTIM/32U */
#define HRTIM_BURSTMODEPRESCALER_DIV64    (HRTIM_BMCR_BMPRSC_BIT2 | HRTIM_BMCR_BMPRSC_BIT1)                                             /*!< fBRST = fHRTIM/64U */
#define HRTIM_BURSTMODEPRESCALER_DIV128   (HRTIM_BMCR_BMPRSC_BIT2 | HRTIM_BMCR_BMPRSC_BIT1 | HRTIM_BMCR_BMPRSC_BIT0)                       /*!< fBRST = fHRTIM/128U */
#define HRTIM_BURSTMODEPRESCALER_DIV256   (HRTIM_BMCR_BMPRSC_BIT3)                                                                   /*!< fBRST = fHRTIM/256U */
#define HRTIM_BURSTMODEPRESCALER_DIV512   (HRTIM_BMCR_BMPRSC_BIT3 | HRTIM_BMCR_BMPRSC_BIT0)                                             /*!< fBRST = fHRTIM/512U */
#define HRTIM_BURSTMODEPRESCALER_DIV1024  (HRTIM_BMCR_BMPRSC_BIT3 | HRTIM_BMCR_BMPRSC_BIT1)                                             /*!< fBRST = fHRTIM/1024U */
#define HRTIM_BURSTMODEPRESCALER_DIV2048  (HRTIM_BMCR_BMPRSC_BIT3 | HRTIM_BMCR_BMPRSC_BIT1 | HRTIM_BMCR_BMPRSC_BIT0)                       /*!< fBRST = fHRTIM/2048U*/
#define HRTIM_BURSTMODEPRESCALER_DIV4096  (HRTIM_BMCR_BMPRSC_BIT3 | HRTIM_BMCR_BMPRSC_BIT2)                                             /*!< fBRST = fHRTIM/4096U */
#define HRTIM_BURSTMODEPRESCALER_DIV8192  (HRTIM_BMCR_BMPRSC_BIT3 | HRTIM_BMCR_BMPRSC_BIT2 | HRTIM_BMCR_BMPRSC_BIT0)                       /*!< fBRST = fHRTIM/8192U */
#define HRTIM_BURSTMODEPRESCALER_DIV16384 (HRTIM_BMCR_BMPRSC_BIT3 | HRTIM_BMCR_BMPRSC_BIT2 | HRTIM_BMCR_BMPRSC_BIT1)                       /*!< fBRST = fHRTIM/16384U */
#define HRTIM_BURSTMODEPRESCALER_DIV32768 (HRTIM_BMCR_BMPRSC_BIT3 | HRTIM_BMCR_BMPRSC_BIT2 | HRTIM_BMCR_BMPRSC_BIT1 | HRTIM_BMCR_BMPRSC_BIT0) /*!< fBRST = fHRTIM/32768U */

/**
  * end of HRTIM_Burst_Mode_Prescaler @}
  */

/** @defgroup HRTIM_Burst_Mode_Register_Preload_Enable HRTIM Burst Mode Register Preload Enable
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining whether or not burst mode registers preload
           mechanism is enabled, i.e. a write access into a preloadable register
          (HRTIM_BMCMPR, HRTIM_BMPER) is done into the active or the preload register
  *
  */

#define HRTIM_BURSTMODEPRELOAD_DISABLED (0x00000000U)           /*!< Preload disabled: the write access is directly done into active registers */
#define HRTIM_BURSTMODEPRELOAD_ENABLED  (HRTIM_BMCR_BMPREN)     /*!< Preload enabled: the write access is done into preload registers */

/**
  * end of HRTIM_Burst_Mode_Register_Preload_Enable @}
  */

/** @defgroup HRTIM_Burst_Mode_Trigger HRTIM Burst Mode Trigger
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the events that can be used to trig the burst
  *        mode operation
  *
  */

#define HRTIM_BURSTMODETRIGGER_NONE               0x00000000U             /*!<  No trigger */
#define HRTIM_BURSTMODETRIGGER_MASTER_RESET       (HRTIM_BMTRGR_MSTRST)   /*!<  Master reset */
#define HRTIM_BURSTMODETRIGGER_MASTER_REPETITION  (HRTIM_BMTRGR_MSTREP)   /*!<  Master repetition */
#define HRTIM_BURSTMODETRIGGER_MASTER_CMP1        (HRTIM_BMTRGR_MSTCMP1)  /*!<  Master compare 1U */
#define HRTIM_BURSTMODETRIGGER_MASTER_CMP2        (HRTIM_BMTRGR_MSTCMP2)  /*!<  Master compare 2U */
#define HRTIM_BURSTMODETRIGGER_MASTER_CMP3        (HRTIM_BMTRGR_MSTCMP3)  /*!<  Master compare 3U */
#define HRTIM_BURSTMODETRIGGER_MASTER_CMP4        (HRTIM_BMTRGR_MSTCMP4)  /*!<  Master compare 4U */
#define HRTIM_BURSTMODETRIGGER_TIMERA_RESET       (HRTIM_BMTRGR_TARST)    /*!< Timer A reset  */
#define HRTIM_BURSTMODETRIGGER_TIMERA_REPETITION  (HRTIM_BMTRGR_TAREP)    /*!< Timer A repetition  */
#define HRTIM_BURSTMODETRIGGER_TIMERA_CMP1        (HRTIM_BMTRGR_TACMP1)   /*!< Timer A compare 1  */
#define HRTIM_BURSTMODETRIGGER_TIMERA_CMP2        (HRTIM_BMTRGR_TACMP2)   /*!< Timer A compare 2  */
#define HRTIM_BURSTMODETRIGGER_TIMERB_RESET       (HRTIM_BMTRGR_TBRST)    /*!< Timer B reset  */
#define HRTIM_BURSTMODETRIGGER_TIMERB_REPETITION  (HRTIM_BMTRGR_TBREP)    /*!< Timer B repetition  */
#define HRTIM_BURSTMODETRIGGER_TIMERB_CMP1        (HRTIM_BMTRGR_TBCMP1)   /*!< Timer B compare 1  */
#define HRTIM_BURSTMODETRIGGER_TIMERB_CMP2        (HRTIM_BMTRGR_TBCMP2)   /*!< Timer B compare 2  */
#define HRTIM_BURSTMODETRIGGER_TIMERC_RESET       (HRTIM_BMTRGR_TCRST)    /*!< Timer C reset  */
#define HRTIM_BURSTMODETRIGGER_TIMERC_REPETITION  (HRTIM_BMTRGR_TCREP)    /*!< Timer C repetition  */
#define HRTIM_BURSTMODETRIGGER_TIMERC_CMP1        (HRTIM_BMTRGR_TCCMP1)   /*!< Timer C compare 1  */
#define HRTIM_BURSTMODETRIGGER_TIMERF_RESET       (HRTIM_BMTRGR_TFRST)    /*!< Timer F reset  */
#define HRTIM_BURSTMODETRIGGER_TIMERD_RESET       (HRTIM_BMTRGR_TDRST)    /*!< Timer D reset  */
#define HRTIM_BURSTMODETRIGGER_TIMERD_REPETITION  (HRTIM_BMTRGR_TDREP)    /*!< Timer D repetition  */
#define HRTIM_BURSTMODETRIGGER_TIMERF_REPETITION  (HRTIM_BMTRGR_TFREP)    /*!< Timer F repetition  */
#define HRTIM_BURSTMODETRIGGER_TIMERD_CMP2        (HRTIM_BMTRGR_TDCMP2)   /*!< Timer D compare 2  */
#define HRTIM_BURSTMODETRIGGER_TIMERF_CMP1        (HRTIM_BMTRGR_TFCMP1)   /*!< Timer F compare 1  */
#define HRTIM_BURSTMODETRIGGER_TIMERE_REPETITION  (HRTIM_BMTRGR_TEREP)    /*!< Timer E repetition  */
#define HRTIM_BURSTMODETRIGGER_TIMERE_CMP1        (HRTIM_BMTRGR_TECMP1)   /*!< Timer E compare 1  */
#define HRTIM_BURSTMODETRIGGER_TIMERE_CMP2        (HRTIM_BMTRGR_TECMP2)   /*!< Timer E compare 2  */
#define HRTIM_BURSTMODETRIGGER_TIMERA_EVENT7      (HRTIM_BMTRGR_TAEEV7)   /*!< Timer A period following External Event 7  */
#define HRTIM_BURSTMODETRIGGER_TIMERD_EVENT8      (HRTIM_BMTRGR_TDEEV8)   /*!< Timer D period following External Event 8  */
#define HRTIM_BURSTMODETRIGGER_EVENT_7            (HRTIM_BMTRGR_EEV7)     /*!< External Event 7 (timer A filters applied) */
#define HRTIM_BURSTMODETRIGGER_EVENT_8            (HRTIM_BMTRGR_EEV8)     /*!< External Event 8 (timer D filters applied)*/
#define HRTIM_BURSTMODETRIGGER_EVENT_ONCHIP       (HRTIM_BMTRGR_OCHPEV)   /*!< On-chip Event */

/**
  * end of HRTIM_Burst_Mode_Trigger @}
  */

/** @defgroup HRTIM_ADC_Trigger_Update_Source HRTIM ADC Trigger Update Source
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief constants defining the source triggering the update of the
  *  HRTIM_ADCxR register (transfer from preload to active register).
  *
  */

#define HRTIM_ADCTRIGGERUPDATE_MASTER  0x00000000U                                   /*!< Master timer */
#define HRTIM_ADCTRIGGERUPDATE_TIMER_A (HRTIM_CR1_ADC1USRC_BIT0)                        /*!< Timer A */
#define HRTIM_ADCTRIGGERUPDATE_TIMER_B (HRTIM_CR1_ADC1USRC_BIT1)                        /*!< Timer B */
#define HRTIM_ADCTRIGGERUPDATE_TIMER_C (HRTIM_CR1_ADC1USRC_BIT1 | HRTIM_CR1_ADC1USRC_BIT0) /*!< Timer C */
#define HRTIM_ADCTRIGGERUPDATE_TIMER_D (HRTIM_CR1_ADC1USRC_BIT2)                        /*!< Timer D */
#define HRTIM_ADCTRIGGERUPDATE_TIMER_E (HRTIM_CR1_ADC1USRC_BIT2 | HRTIM_CR1_ADC1USRC_BIT0) /*!< Timer E */
#define HRTIM_ADCTRIGGERUPDATE_TIMER_F (HRTIM_CR1_ADC1USRC_BIT2 | HRTIM_CR1_ADC1USRC_BIT1) /*!< Timer F */

/**
  * end of HRTIM_ADC_Trigger_Update_Source @}
  */

/** @defgroup HRTIM_ADC_Trigger_Event HRTIM ADC Trigger Event
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief constants defining the events triggering ADC conversion.
  *        HRTIM_ADCTRIGGEREVENT13_*: ADC Triggers 1 and 3
  *        HRTIM_ADCTRIGGEREVENT24_*: ADC Triggers 2 and 4
  *        HRTIM_ADCTRIGGEREVENT579_*: ADC Triggers 5 and 7 and 9
  *        HRTIM_ADCTRIGGEREVENT6810_*: ADC Triggers 6 and 8 and 10
  *
  */

#define HRTIM_ADCTRIGGEREVENT13_NONE           0x00000000U              /*!< No ADC trigger event */
#define HRTIM_ADCTRIGGEREVENT13_MASTER_CMP1    (HRTIM_ADC1R_AD1MC1)     /*!< ADC Trigger on master compare 1U */
#define HRTIM_ADCTRIGGEREVENT13_MASTER_CMP2    (HRTIM_ADC1R_AD1MC2)     /*!< ADC Trigger on master compare 2U */
#define HRTIM_ADCTRIGGEREVENT13_MASTER_CMP3    (HRTIM_ADC1R_AD1MC3)     /*!< ADC Trigger on master compare 3U */
#define HRTIM_ADCTRIGGEREVENT13_MASTER_CMP4    (HRTIM_ADC1R_AD1MC4)     /*!< ADC Trigger on master compare 4U */
#define HRTIM_ADCTRIGGEREVENT13_MASTER_PERIOD  (HRTIM_ADC1R_AD1MPER)    /*!< ADC Trigger on master period */
#define HRTIM_ADCTRIGGEREVENT13_EVENT_1        (HRTIM_ADC1R_AD1EEV1)    /*!< ADC Trigger on external event 1U */
#define HRTIM_ADCTRIGGEREVENT13_EVENT_2        (HRTIM_ADC1R_AD1EEV2)    /*!< ADC Trigger on external event 2U */
#define HRTIM_ADCTRIGGEREVENT13_EVENT_3        (HRTIM_ADC1R_AD1EEV3)    /*!< ADC Trigger on external event 3U */
#define HRTIM_ADCTRIGGEREVENT13_EVENT_4        (HRTIM_ADC1R_AD1EEV4)    /*!< ADC Trigger on external event 4U */
#define HRTIM_ADCTRIGGEREVENT13_EVENT_5        (HRTIM_ADC1R_AD1EEV5)    /*!< ADC Trigger on external event 5U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERF_CMP2    (HRTIM_ADC1R_AD1TFC2)    /*!< ADC Trigger on Timer F compare 2U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP3    (HRTIM_ADC1R_AD1TAC3)    /*!< ADC Trigger on Timer A compare 3U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERA_CMP4    (HRTIM_ADC1R_AD1TAC4)    /*!< ADC Trigger on Timer A compare 4U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERA_PERIOD  (HRTIM_ADC1R_AD1TAPER)   /*!< ADC Trigger on Timer A period */
#define HRTIM_ADCTRIGGEREVENT13_TIMERA_RESET   (HRTIM_ADC1R_AD1TARST)   /*!< ADC Trigger on Timer A reset */
#define HRTIM_ADCTRIGGEREVENT13_TIMERF_CMP3    (HRTIM_ADC1R_AD1TFC3)    /*!< ADC Trigger on Timer F compare 3U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP3    (HRTIM_ADC1R_AD1TBC3)    /*!< ADC Trigger on Timer B compare 3U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP4    (HRTIM_ADC1R_AD1TBC4)    /*!< ADC Trigger on Timer B compare 4U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERB_PERIOD  (HRTIM_ADC1R_AD1TBPER)   /*!< ADC Trigger on Timer B period */
#define HRTIM_ADCTRIGGEREVENT13_TIMERB_RESET   (HRTIM_ADC1R_AD1TBRST)   /*!< ADC Trigger on Timer B reset */
#define HRTIM_ADCTRIGGEREVENT13_TIMERF_CMP4    (HRTIM_ADC1R_AD1TFC4)    /*!< ADC Trigger on Timer F compare 4U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERC_CMP3    (HRTIM_ADC1R_AD1TCC3)    /*!< ADC Trigger on Timer C compare 3U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERC_CMP4    (HRTIM_ADC1R_AD1TCC4)    /*!< ADC Trigger on Timer C compare 4U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERC_PERIOD  (HRTIM_ADC1R_AD1TCPER)   /*!< ADC Trigger on Timer C period */
#define HRTIM_ADCTRIGGEREVENT13_TIMERF_PERIOD  (HRTIM_ADC1R_AD1TFPER)   /*!< ADC Trigger on Timer F period */
#define HRTIM_ADCTRIGGEREVENT13_TIMERD_CMP3    (HRTIM_ADC1R_AD1TDC3)    /*!< ADC Trigger on Timer D compare 3U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERD_CMP4    (HRTIM_ADC1R_AD1TDC4)    /*!< ADC Trigger on Timer D compare 4U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERD_PERIOD  (HRTIM_ADC1R_AD1TDPER)   /*!< ADC Trigger on Timer D period */
#define HRTIM_ADCTRIGGEREVENT13_TIMERF_RESET   (HRTIM_ADC1R_AD1TFRST)   /*!< ADC Trigger on Timer F reset */
#define HRTIM_ADCTRIGGEREVENT13_TIMERE_CMP3    (HRTIM_ADC1R_AD1TEC3)    /*!< ADC Trigger on Timer E compare 3U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERE_CMP4    (HRTIM_ADC1R_AD1TEC4)    /*!< ADC Trigger on Timer E compare 4U */
#define HRTIM_ADCTRIGGEREVENT13_TIMERE_PERIOD  (HRTIM_ADC1R_AD1TEPER)   /*!< ADC Trigger on Timer E period */

#define HRTIM_ADCTRIGGEREVENT24_NONE           0x00000000U               /*!< No ADC trigger event */
#define HRTIM_ADCTRIGGEREVENT24_MASTER_CMP1    (HRTIM_ADC2R_AD2MC1)     /*!< ADC Trigger on master compare 1U */
#define HRTIM_ADCTRIGGEREVENT24_MASTER_CMP2    (HRTIM_ADC2R_AD2MC2)     /*!< ADC Trigger on master compare 2U */
#define HRTIM_ADCTRIGGEREVENT24_MASTER_CMP3    (HRTIM_ADC2R_AD2MC3)     /*!< ADC Trigger on master compare 3U */
#define HRTIM_ADCTRIGGEREVENT24_MASTER_CMP4    (HRTIM_ADC2R_AD2MC4)     /*!< ADC Trigger on master compare 4U */
#define HRTIM_ADCTRIGGEREVENT24_MASTER_PERIOD  (HRTIM_ADC2R_AD2MPER)    /*!< ADC Trigger on master period */
#define HRTIM_ADCTRIGGEREVENT24_EVENT_6        (HRTIM_ADC2R_AD2EEV6)    /*!< ADC Trigger on external event 6U */
#define HRTIM_ADCTRIGGEREVENT24_EVENT_7        (HRTIM_ADC2R_AD2EEV7)    /*!< ADC Trigger on external event 7U */
#define HRTIM_ADCTRIGGEREVENT24_EVENT_8        (HRTIM_ADC2R_AD2EEV8)    /*!< ADC Trigger on external event 8U */
#define HRTIM_ADCTRIGGEREVENT24_EVENT_9        (HRTIM_ADC2R_AD2EEV9)    /*!< ADC Trigger on external event 9U */
#define HRTIM_ADCTRIGGEREVENT24_EVENT_10       (HRTIM_ADC2R_AD2EEV10)   /*!< ADC Trigger on external event 10U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERA_CMP2    (HRTIM_ADC2R_AD2TAC2)    /*!< ADC Trigger on Timer A compare 2U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERF_CMP2    (HRTIM_ADC2R_AD2TFC2)    /*!< ADC Trigger on Timer F compare 2U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERA_CMP4    (HRTIM_ADC2R_AD2TAC4)    /*!< ADC Trigger on Timer A compare 4U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERA_PERIOD  (HRTIM_ADC2R_AD2TAPER)   /*!< ADC Trigger on Timer A period */
#define HRTIM_ADCTRIGGEREVENT24_TIMERB_CMP2    (HRTIM_ADC2R_AD2TBC2)    /*!< ADC Trigger on Timer B compare 2U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERF_CMP3    (HRTIM_ADC2R_AD2TFC3)    /*!< ADC Trigger on Timer F compare 3U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERB_CMP4    (HRTIM_ADC2R_AD2TBC4)    /*!< ADC Trigger on Timer B compare 4U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERB_PERIOD  (HRTIM_ADC2R_AD2TBPER)   /*!< ADC Trigger on Timer B period */
#define HRTIM_ADCTRIGGEREVENT24_TIMERC_CMP2    (HRTIM_ADC2R_AD2TCC2)    /*!< ADC Trigger on Timer C compare 2U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERF_CMP4    (HRTIM_ADC2R_AD2TFC4)    /*!< ADC Trigger on Timer F compare 4U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERC_CMP4    (HRTIM_ADC2R_AD2TCC4)    /*!< ADC Trigger on Timer C compare 4U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERC_PERIOD  (HRTIM_ADC2R_AD2TCPER)   /*!< ADC Trigger on Timer C period */
#define HRTIM_ADCTRIGGEREVENT24_TIMERC_RESET   (HRTIM_ADC2R_AD2TCRST)   /*!< ADC Trigger on Timer C reset */
#define HRTIM_ADCTRIGGEREVENT24_TIMERD_CMP2    (HRTIM_ADC2R_AD2TDC2)    /*!< ADC Trigger on Timer D compare 2U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERF_PERIOD  (HRTIM_ADC2R_AD2TFPER)   /*!< ADC Trigger on Timer F period */
#define HRTIM_ADCTRIGGEREVENT24_TIMERD_CMP4    (HRTIM_ADC2R_AD2TDC4)    /*!< ADC Trigger on Timer D compare 4U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERD_PERIOD  (HRTIM_ADC2R_AD2TDPER)   /*!< ADC Trigger on Timer D period */
#define HRTIM_ADCTRIGGEREVENT24_TIMERD_RESET   (HRTIM_ADC2R_AD2TDRST)   /*!< ADC Trigger on Timer D reset */
#define HRTIM_ADCTRIGGEREVENT24_TIMERE_CMP2    (HRTIM_ADC2R_AD2TEC2)    /*!< ADC Trigger on Timer E compare 2U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERE_CMP3    (HRTIM_ADC2R_AD2TEC3)    /*!< ADC Trigger on Timer E compare 3U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERE_CMP4    (HRTIM_ADC2R_AD2TEC4)    /*!< ADC Trigger on Timer E compare 4U */
#define HRTIM_ADCTRIGGEREVENT24_TIMERE_RESET   (HRTIM_ADC2R_AD2TERST)   /*!< ADC Trigger on Timer E reset */


#define HRTIM_ADCTRIGGEREVENT6810_MASTER_CMP1    ((uint32_t)0x00U)  /*!< ADC Trigger on master compare 1U */
#define HRTIM_ADCTRIGGEREVENT6810_MASTER_CMP2    ((uint32_t)0x01U)  /*!< ADC Trigger on master compare 2U */
#define HRTIM_ADCTRIGGEREVENT6810_MASTER_CMP3    ((uint32_t)0x02U)  /*!< ADC Trigger on master compare 3U */
#define HRTIM_ADCTRIGGEREVENT6810_MASTER_CMP4    ((uint32_t)0x03U)  /*!< ADC Trigger on master compare 4U */
#define HRTIM_ADCTRIGGEREVENT6810_MASTER_PERIOD  ((uint32_t)0x04U)  /*!< ADC Trigger on master period */
#define HRTIM_ADCTRIGGEREVENT6810_EVENT_6        ((uint32_t)0x05U)  /*!< ADC Trigger on external event 6U */
#define HRTIM_ADCTRIGGEREVENT6810_EVENT_7        ((uint32_t)0x06U)  /*!< ADC Trigger on external event 7U */
#define HRTIM_ADCTRIGGEREVENT6810_EVENT_8        ((uint32_t)0x07U)  /*!< ADC Trigger on external event 8U */
#define HRTIM_ADCTRIGGEREVENT6810_EVENT_9        ((uint32_t)0x08U)  /*!< ADC Trigger on external event 9U */
#define HRTIM_ADCTRIGGEREVENT6810_EVENT_10       ((uint32_t)0x09U)  /*!< ADC Trigger on external event 10U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERA_CMP2    ((uint32_t)0x0AU)  /*!< ADC Trigger on Timer A compare 2U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERA_CMP4    ((uint32_t)0x0BU)  /*!< ADC Trigger on Timer A compare 4U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERA_PERIOD  ((uint32_t)0x0CU)  /*!< ADC Trigger on Timer A period */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERB_CMP2    ((uint32_t)0x0DU)  /*!< ADC Trigger on Timer B compare 2U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERB_CMP4    ((uint32_t)0x0EU)  /*!< ADC Trigger on Timer B compare 4U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERB_PERIOD  ((uint32_t)0x0FU)  /*!< ADC Trigger on Timer B period */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERC_CMP2    ((uint32_t)0x10U)  /*!< ADC Trigger on Timer C compare 2U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERC_CMP4    ((uint32_t)0x11U)  /*!< ADC Trigger on Timer C compare 4U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERC_PERIOD  ((uint32_t)0x12U)  /*!< ADC Trigger on Timer C period */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERC_RESET   ((uint32_t)0x13U)  /*!< ADC Trigger on Timer C reset */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERD_CMP2    ((uint32_t)0x14U)  /*!< ADC Trigger on Timer D compare 2U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERD_CMP4    ((uint32_t)0x15U)  /*!< ADC Trigger on Timer D compare 4U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERD_PERIOD  ((uint32_t)0x16U)  /*!< ADC Trigger on Timer D period */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERD_RESET   ((uint32_t)0x17U)  /*!< ADC Trigger on Timer D reset */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERE_CMP2    ((uint32_t)0x18U)  /*!< ADC Trigger on Timer E compare 2U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERE_CMP3    ((uint32_t)0x19U)  /*!< ADC Trigger on Timer E compare 3U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERE_CMP4    ((uint32_t)0x1AU)  /*!< ADC Trigger on Timer E compare 4U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERE_RESET   ((uint32_t)0x1BU)  /*!< ADC Trigger on Timer E reset */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERF_CMP2    ((uint32_t)0x1CU)  /*!< ADC Trigger on Timer F compare 2U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERF_CMP3    ((uint32_t)0x1DU)  /*!< ADC Trigger on Timer F compare 3U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERF_CMP4    ((uint32_t)0x1EU)  /*!< ADC Trigger on Timer F compare 4U */
#define HRTIM_ADCTRIGGEREVENT6810_TIMERF_PERIOD  ((uint32_t)0x1FU)  /*!< ADC Trigger on Timer F period */

#define HRTIM_ADCTRIGGEREVENT579_MASTER_CMP1    ((uint32_t)0x00U)  /*!< ADC Trigger on master compare 1U */
#define HRTIM_ADCTRIGGEREVENT579_MASTER_CMP2    ((uint32_t)0x01U)  /*!< ADC Trigger on master compare 2U */
#define HRTIM_ADCTRIGGEREVENT579_MASTER_CMP3    ((uint32_t)0x02U)  /*!< ADC Trigger on master compare 3U */
#define HRTIM_ADCTRIGGEREVENT579_MASTER_CMP4    ((uint32_t)0x03U)  /*!< ADC Trigger on master compare 4U */
#define HRTIM_ADCTRIGGEREVENT579_MASTER_PERIOD  ((uint32_t)0x04U)  /*!< ADC Trigger on master period */
#define HRTIM_ADCTRIGGEREVENT579_EVENT_1        ((uint32_t)0x05U)  /*!< ADC Trigger on external event 1U */
#define HRTIM_ADCTRIGGEREVENT579_EVENT_2        ((uint32_t)0x06U)  /*!< ADC Trigger on external event 2U */
#define HRTIM_ADCTRIGGEREVENT579_EVENT_3        ((uint32_t)0x07U)  /*!< ADC Trigger on external event 3U */
#define HRTIM_ADCTRIGGEREVENT579_EVENT_4        ((uint32_t)0x08U)  /*!< ADC Trigger on external event 4U */
#define HRTIM_ADCTRIGGEREVENT579_EVENT_5        ((uint32_t)0x09U)  /*!< ADC Trigger on external event 5U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERA_CMP3    ((uint32_t)0x0AU)  /*!< ADC Trigger on Timer A compare 3U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERA_CMP4    ((uint32_t)0x0BU)  /*!< ADC Trigger on Timer A compare 4U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERA_PERIOD  ((uint32_t)0x0CU)  /*!< ADC Trigger on Timer A period */
#define HRTIM_ADCTRIGGEREVENT579_TIMERA_RESET   ((uint32_t)0x0DU)  /*!< ADC Trigger on Timer A reset */
#define HRTIM_ADCTRIGGEREVENT579_TIMERB_CMP3    ((uint32_t)0x0EU)  /*!< ADC Trigger on Timer B compare 3U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERB_CMP4    ((uint32_t)0x0FU)  /*!< ADC Trigger on Timer B compare 4U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERB_PERIOD  ((uint32_t)0x10U)  /*!< ADC Trigger on Timer B period */
#define HRTIM_ADCTRIGGEREVENT579_TIMERB_RESET   ((uint32_t)0x11U)  /*!< ADC Trigger on Timer B reset */
#define HRTIM_ADCTRIGGEREVENT579_TIMERC_CMP3    ((uint32_t)0x12U)  /*!< ADC Trigger on Timer C compare 3U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERC_CMP4    ((uint32_t)0x13U)  /*!< ADC Trigger on Timer C compare 4U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERC_PERIOD  ((uint32_t)0x14U)  /*!< ADC Trigger on Timer C period */
#define HRTIM_ADCTRIGGEREVENT579_TIMERD_CMP3    ((uint32_t)0x15U)  /*!< ADC Trigger on Timer D compare 3U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERD_CMP4    ((uint32_t)0x16U)  /*!< ADC Trigger on Timer D compare 4U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERD_PERIOD  ((uint32_t)0x17U)  /*!< ADC Trigger on Timer D period */
#define HRTIM_ADCTRIGGEREVENT579_TIMERE_CMP3    ((uint32_t)0x18U)  /*!< ADC Trigger on Timer E compare 3U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERE_CMP4    ((uint32_t)0x19U)  /*!< ADC Trigger on Timer E compare 4U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERE_PERIOD  ((uint32_t)0x1AU)  /*!< ADC Trigger on Timer E period */
#define HRTIM_ADCTRIGGEREVENT579_TIMERF_CMP2    ((uint32_t)0x1BU)  /*!< ADC Trigger on Timer F compare 2U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERF_CMP3    ((uint32_t)0x1CU)  /*!< ADC Trigger on Timer F compare 3U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERF_CMP4    ((uint32_t)0x1DU)  /*!< ADC Trigger on Timer F compare 4U */
#define HRTIM_ADCTRIGGEREVENT579_TIMERF_PERIOD  ((uint32_t)0x1EU)  /*!< ADC Trigger on Timer F period */
#define HRTIM_ADCTRIGGEREVENT579_TIMERF_RESET   ((uint32_t)0x1FU)  /*!< ADC Trigger on Timer F reset */

/**
  * end of HRTIM_ADC_Trigger_Event @}
  */

/** @defgroup HRTIM_DLL_Calibration_Rate HRTIM DLL Calibration Rate
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the DLL calibration periods (in micro seconds)
  *
  */
#define HRTIM_SINGLE_CALIBRATION    0xFFFFFFFFU                                    /*!< Non periodic DLL calibration */
#define HRTIM_CALIBRATIONRATE_0     0x00000000U                                    /*!< Periodic DLL calibration: T = 1048576U * tHRTIM (6.168 ms) */
#define HRTIM_CALIBRATIONRATE_1     (HRTIM_DLLCR_CALRTE_BIT0)                         /*!< Periodic DLL calibration: T = 131072U * tHRTIM (0.771 ms) */
#define HRTIM_CALIBRATIONRATE_2     (HRTIM_DLLCR_CALRTE_BIT1)                         /*!< Periodic DLL calibration: T = 16384U * tHRTIM (0.096 ms) */
#define HRTIM_CALIBRATIONRATE_3     (HRTIM_DLLCR_CALRTE_BIT1 | HRTIM_DLLCR_CALRTE_BIT0)  /*!< Periodic DLL calibration: T = 2048U * tHRTIM (0.012 ms) */

/**
  * end of HRTIM_DLL_Calibration_Rate @}
  */

/** @defgroup HRTIM_Burst_DMA_Registers_Update HRTIM Burst DMA Registers Update
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the registers that can be written during a burst
  *        DMA operation
  *
  */

#define HRTIM_BURSTDMA_NONE  0x00000000U               /*!< No register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_CR    (HRTIM_BDTUPR_TIMCR)      /*!< MCR or TIMxCR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_ICR   (HRTIM_BDTUPR_TIMICR)     /*!< MICR or TIMxICR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_DIER  (HRTIM_BDTUPR_TIMDIER)    /*!< MDIER or TIMxDIER register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_CNT   (HRTIM_BDTUPR_TIMCNT)     /*!< MCNTR or CNTxCR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_PER   (HRTIM_BDTUPR_TIMPER)     /*!< MPER or PERxR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_REP   (HRTIM_BDTUPR_TIMREP)     /*!< MREPR or REPxR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_CMP1  (HRTIM_BDTUPR_TIMCMP1)    /*!< MCMP1R or CMP1xR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_CMP2  (HRTIM_BDTUPR_TIMCMP2)    /*!< MCMP2R or CMP2xR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_CMP3  (HRTIM_BDTUPR_TIMCMP3)    /*!< MCMP3R or CMP3xR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_CMP4  (HRTIM_BDTUPR_TIMCMP4)    /*!< MCMP4R or CMP4xR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_DTR   (HRTIM_BDTUPR_TIMDTR)     /*!< TDxR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_SET1R (HRTIM_BDTUPR_TIMSET1R)   /*!< SET1R register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_RST1R (HRTIM_BDTUPR_TIMRST1R)   /*!< RST1R register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_SET2R (HRTIM_BDTUPR_TIMSET2R)   /*!< SET2R register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_RST2R (HRTIM_BDTUPR_TIMRST2R)   /*!< RST1R register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_EEFR1 (HRTIM_BDTUPR_TIMEEFR1)   /*!< EEFxR1 register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_EEFR2 (HRTIM_BDTUPR_TIMEEFR2)   /*!< EEFxR2 register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_RSTR  (HRTIM_BDTUPR_TIMRSTR)    /*!< RSTxR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_CHPR  (HRTIM_BDTUPR_TIMCHPR)    /*!< CHPxR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_OUTR  (HRTIM_BDTUPR_TIMOUTR)    /*!< OUTxR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_FLTR  (HRTIM_BDTUPR_TIMFLTR)    /*!< FLTxR register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_CR2   (HRTIM_BDTUPR_TIMCR2)     /*!< TIMxCR2 register is updated by Burst DMA accesses */
#define HRTIM_BURSTDMA_EEFR3 (HRTIM_BDTUPR_TIMEEFR3)   /*!< EEFxR3 register is updated by Burst DMA accesses */

/**
  * end of HRTIM_Burst_DMA_Registers_Update @}
  */

/** @defgroup HRTIM_Burst_Mode_Control HRTIM Burst Mode Control
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants used to enable or disable the burst mode controller
  *
  */

#define HRTIM_BURSTMODECTL_DISABLED 0x00000000U          /*!< Burst mode disabled */
#define HRTIM_BURSTMODECTL_ENABLED  (HRTIM_BMCR_BME)     /*!< Burst mode enabled */

/**
  * end of HRTIM_Burst_Mode_Control @}
  */

/** @defgroup HRTIM_Fault_Mode_Control  HRTIM Fault Mode Control
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants used to enable or disable a fault channel
  *
  */

#define HRTIM_FAULTMODECTL_DISABLED 0x00000000U /*!< Fault channel is disabled */
#define HRTIM_FAULTMODECTL_ENABLED  0x00000001U /*!< Fault channel is  enabled */

/**
  * end of HRTIM_Fault_Mode_Control @}
  */

/** @defgroup HRTIM_Software_Timer_Update HRTIM Software Timer Update
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants used to force timer registers update
  *
  */

#define HRTIM_TIMERUPDATE_MASTER    (HRTIM_CR2_MSWU)     /*!< Force an immediate transfer from the preload to the active register in the master timer */
#define HRTIM_TIMERUPDATE_A         (HRTIM_CR2_TASWU)    /*!< Force an immediate transfer from the preload to the active register in the timer A */
#define HRTIM_TIMERUPDATE_B         (HRTIM_CR2_TBSWU)    /*!< Force an immediate transfer from the preload to the active register in the timer B */
#define HRTIM_TIMERUPDATE_C         (HRTIM_CR2_TCSWU)    /*!< Force an immediate transfer from the preload to the active register in the timer C */
#define HRTIM_TIMERUPDATE_D         (HRTIM_CR2_TDSWU)    /*!< Force an immediate transfer from the preload to the active register in the timer D */
#define HRTIM_TIMERUPDATE_E         (HRTIM_CR2_TESWU)    /*!< Force an immediate transfer from the preload to the active register in the timer E */
#define HRTIM_TIMERUPDATE_F         (HRTIM_CR2_TFSWU)    /*!< Forces an immediate transfer from the preload to the active register in the timer F */

/**
  * end of HRTIM_Software_Timer_Update @}
  */

/** @defgroup HRTIM_Software_Timer_SwapOutput  HRTIM Software Timer swap Output
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants used to swap the output of the timer registers
  *
  */

#define HRTIM_TIMERSWAP_A         (HRTIM_CR2_SWPA)    /*!< Swap the output of the Timer A */
#define HRTIM_TIMERSWAP_B         (HRTIM_CR2_SWPB)    /*!< Swap the output of the Timer B */
#define HRTIM_TIMERSWAP_C         (HRTIM_CR2_SWPC)    /*!< Swap the output of the Timer C */
#define HRTIM_TIMERSWAP_D         (HRTIM_CR2_SWPD)    /*!< Swap the output of the Timer D */
#define HRTIM_TIMERSWAP_E         (HRTIM_CR2_SWPE)    /*!< Swap the output of the Timer E */
#define HRTIM_TIMERSWAP_F         (HRTIM_CR2_SWPF)    /*!< Swap the output of the Timer F */

/**
  * end of HRTIM_Software_Timer_SwapOutput @}
  */

/** @defgroup HRTIM_Software_Timer_Reset HRTIM Software Timer Reset
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants used to force timer counter reset
  *
  */

#define HRTIM_TIMERRESET_MASTER    (HRTIM_CR2_MRST)     /*!< Reset the master timer counter */
#define HRTIM_TIMERRESET_TIMER_A   (HRTIM_CR2_TARST)    /*!< Reset the timer A counter */
#define HRTIM_TIMERRESET_TIMER_B   (HRTIM_CR2_TBRST)    /*!< Reset the timer B counter */
#define HRTIM_TIMERRESET_TIMER_C   (HRTIM_CR2_TCRST)    /*!< Reset the timer C counter */
#define HRTIM_TIMERRESET_TIMER_D   (HRTIM_CR2_TDRST)    /*!< Reset the timer D counter */
#define HRTIM_TIMERRESET_TIMER_E   (HRTIM_CR2_TERST)    /*!< Reset the timer E counter */
#define HRTIM_TIMERRESET_TIMER_F   (HRTIM_CR2_TFRST)    /*!< Reset the timer F counter */

/**
  * end of HRTIM_Software_Timer_Reset @}
  */

/** @defgroup HRTIM_Output_Level HRTIM Output Level
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the level of a timer output
  *
  */

#define HRTIM_OUTPUTLEVEL_ACTIVE     (0x00000001U) /*!< Force the output to its active state */
#define HRTIM_OUTPUTLEVEL_INACTIVE   (0x00000002U) /*!< Force the output to its inactive state */

#define IS_HRTIM_OUTPUTLEVEL(OUTPUTLEVEL)                                                   \
                                           (((OUTPUTLEVEL) == HRTIM_OUTPUTLEVEL_ACTIVE)  || \
                                            ((OUTPUTLEVEL) == HRTIM_OUTPUTLEVEL_INACTIVE))

/**
  * end of HRTIM_Output_Level @}
  */

/** @defgroup HRTIM_Output_State HRTIM Output State
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the state of a timer output
  *
  */

#define HRTIM_OUTPUTSTATE_IDLE     (0x00000001U)  /*!< Main operating mode, where the output can take the active or
                                                                    inactive level as programmed in the crossbar unit */
#define HRTIM_OUTPUTSTATE_RUN      (0x00000002U)  /*!< Default operating state (e.g. after an HRTIM reset, when the
                                                                    outputs are disabled by software or during a burst mode operation */
#define HRTIM_OUTPUTSTATE_FAULT    (0x00000003U)  /*!< Safety state, entered in case of a shut-down request on
                                                                    FAULTx inputs */

/**
  * end of HRTIM_Output_State @}
  */

/** @defgroup HRTIM_Burst_Mode_Status HRTIM Burst Mode Status
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the operating state of the burst mode controller
  *
  */

#define HRTIM_BURSTMODESTATUS_NORMAL   0x00000000U          /*!< Normal operation */
#define HRTIM_BURSTMODESTATUS_ONGOING (HRTIM_BMCR_BMSTAT)   /*!< Burst operation on-going */

/**
  * end of HRTIM_Burst_Mode_Status @}
  */

/** @defgroup HRTIM_Current_Push_Pull_Status HRTIM Current Push Pull Status
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining on which output the signal is currently applied
  *        in push-pull mode
  *
  */

#define HRTIM_PUSHPULL_CURRENTSTATUS_OUTPUT1    0x00000000U            /*!< Signal applied on output 1 and output 2 forced inactive */
#define HRTIM_PUSHPULL_CURRENTSTATUS_OUTPUT2   (HRTIM_TIMISR_CPPSTAT)  /*!< Signal applied on output 2 and output 1 forced inactive */

/**
  * end of HRTIM_Current_Push_Pull_Status @}
  */

/** @defgroup HRTIM_Idle_Push_Pull_Status HRTIM Idle Push Pull Status
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining on which output the signal was applied, in
  *        push-pull mode balanced fault mode or delayed idle mode, when the
  *        protection was triggered
  */

#define HRTIM_PUSHPULL_IDLESTATUS_OUTPUT1    0x00000000U               /*!< Protection occurred when the output 1 was active and output 2 forced inactive */
#define HRTIM_PUSHPULL_IDLESTATUS_OUTPUT2   (HRTIM_TIMISR_IPPSTAT)     /*!< Protection occurred when the output 2 was active and output 1 forced inactive */

/**
  * end of HRTIM_Idle_Push_Pull_Status @}
  */

/** @defgroup HRTIM_Common_Interrupt_Enable HRTIM Common Interrupt Enable
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the timer indexes
  *
  */

#define HRTIM_IT_NONE           0x00000000U           /*!< No interrupt enabled */
#define HRTIM_IT_FLT1           HRTIM_IER_FLT1        /*!< Fault 1 interrupt enable */
#define HRTIM_IT_FLT2           HRTIM_IER_FLT2        /*!< Fault 2 interrupt enable */
#define HRTIM_IT_FLT3           HRTIM_IER_FLT3        /*!< Fault 3 interrupt enable */
#define HRTIM_IT_FLT4           HRTIM_IER_FLT4        /*!< Fault 4 interrupt enable */
#define HRTIM_IT_FLT5           HRTIM_IER_FLT5        /*!< Fault 5 interrupt enable */
#define HRTIM_IT_FLT6           HRTIM_IER_FLT6        /*!< Fault 6 interrupt enable */
#define HRTIM_IT_SYSFLT         HRTIM_IER_SYSFLT      /*!< System Fault interrupt enable */
#define HRTIM_IT_DLLRDY         HRTIM_IER_DLLRDY      /*!< DLL ready interrupt enable */
#define HRTIM_IT_BMPER          HRTIM_IER_BMPER       /*!<  Burst mode period interrupt enable */

/**
  * end of HRTIM_Common_Interrupt_Enable @}
  */

/** @defgroup HRTIM_Master_Interrupt_Enable HRTIM Master Interrupt Enable
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the timer indexes
  *
  */

#define HRTIM_MASTER_IT_NONE         0x00000000U           /*!< No interrupt enabled */
#define HRTIM_MASTER_IT_MCMP1        HRTIM_MDIER_MCMP1IE   /*!< Master compare 1 interrupt enable */
#define HRTIM_MASTER_IT_MCMP2        HRTIM_MDIER_MCMP2IE   /*!< Master compare 2 interrupt enable */
#define HRTIM_MASTER_IT_MCMP3        HRTIM_MDIER_MCMP3IE   /*!< Master compare 3 interrupt enable */
#define HRTIM_MASTER_IT_MCMP4        HRTIM_MDIER_MCMP4IE   /*!< Master compare 4 interrupt enable */
#define HRTIM_MASTER_IT_MREP         HRTIM_MDIER_MREPIE    /*!< Master Repetition interrupt enable */
#define HRTIM_MASTER_IT_SYNC         HRTIM_MDIER_SYNCIE    /*!< Synchronization input interrupt enable */
#define HRTIM_MASTER_IT_MUPD         HRTIM_MDIER_MUPDIE    /*!< Master update interrupt enable */

/**
  * end of HRTIM_Master_Interrupt_Enable @}
  */

/** @defgroup HRTIM_Timing_Unit_Interrupt_Enable HRTIM Timing Unit Interrupt Enable
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the timer indexes
  *
  */

#define HRTIM_TIM_IT_NONE       0x00000000U               /*!< No interrupt enabled */
#define HRTIM_TIM_IT_CMP1       HRTIM_TIMDIER_CMP1IE      /*!< Timer compare 1 interrupt enable */
#define HRTIM_TIM_IT_CMP2       HRTIM_TIMDIER_CMP2IE      /*!< Timer compare 2 interrupt enable */
#define HRTIM_TIM_IT_CMP3       HRTIM_TIMDIER_CMP3IE      /*!< Timer compare 3 interrupt enable */
#define HRTIM_TIM_IT_CMP4       HRTIM_TIMDIER_CMP4IE      /*!< Timer compare 4 interrupt enable */
#define HRTIM_TIM_IT_REP        HRTIM_TIMDIER_REPIE       /*!< Timer repetition interrupt enable */
#define HRTIM_TIM_IT_UPD        HRTIM_TIMDIER_UPDIE       /*!< Timer update interrupt enable */
#define HRTIM_TIM_IT_CPT1       HRTIM_TIMDIER_CPT1IE      /*!< Timer capture 1 interrupt enable */
#define HRTIM_TIM_IT_CPT2       HRTIM_TIMDIER_CPT2IE      /*!< Timer capture 2 interrupt enable */
#define HRTIM_TIM_IT_SET1       HRTIM_TIMDIER_SET1IE      /*!< Timer output 1 set interrupt enable */
#define HRTIM_TIM_IT_RST1       HRTIM_TIMDIER_RST1IE      /*!< Timer output 1 reset interrupt enable */
#define HRTIM_TIM_IT_SET2       HRTIM_TIMDIER_SET2IE      /*!< Timer output 2 set interrupt enable */
#define HRTIM_TIM_IT_RST2       HRTIM_TIMDIER_RST2IE      /*!< Timer output 2 reset interrupt enable */
#define HRTIM_TIM_IT_RST        HRTIM_TIMDIER_RSTIE       /*!< Timer reset interrupt enable */
#define HRTIM_TIM_IT_DLYPRT     HRTIM_TIMDIER_DLYPRTIE    /*!< Timer delay protection interrupt enable */

/**
  * end of HRTIM_Timing_Unit_Interrupt_Enable @}
  */

/** @defgroup HRTIM_Common_Interrupt_Flag HRTIM Common Interrupt Flag
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the timer indexes
  *
  */

#define HRTIM_FLAG_FLT1           HRTIM_ISR_FLT1    /*!< Fault 1 interrupt flag */
#define HRTIM_FLAG_FLT2           HRTIM_ISR_FLT2    /*!< Fault 2 interrupt flag */
#define HRTIM_FLAG_FLT3           HRTIM_ISR_FLT3    /*!< Fault 3 interrupt flag */
#define HRTIM_FLAG_FLT4           HRTIM_ISR_FLT4    /*!< Fault 4 interrupt flag */
#define HRTIM_FLAG_FLT5           HRTIM_ISR_FLT5    /*!< Fault 5 interrupt flag */
#define HRTIM_FLAG_FLT6           HRTIM_ISR_FLT6    /*!< Fault 6 interrupt flag */
#define HRTIM_FLAG_SYSFLT         HRTIM_ISR_SYSFLT  /*!< System Fault interrupt flag */
#define HRTIM_FLAG_DLLRDY         HRTIM_ISR_DLLRDY  /*!< DLL ready interrupt flag */
#define HRTIM_FLAG_BMPER          HRTIM_ISR_BMPER   /*!< Burst mode period interrupt flag */

/**
  * end of HRTIM_Common_Interrupt_Flag @}
  */

/** @defgroup HRTIM_Master_Interrupt_Flag HRTIM Master Interrupt Flag
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the timer indexes
  *
  */

#define HRTIM_MASTER_FLAG_MCMP1        HRTIM_MISR_MCMP1    /*!< Master compare 1 interrupt flag */
#define HRTIM_MASTER_FLAG_MCMP2        HRTIM_MISR_MCMP2    /*!< Master compare 2 interrupt flag */
#define HRTIM_MASTER_FLAG_MCMP3        HRTIM_MISR_MCMP3    /*!< Master compare 3 interrupt flag */
#define HRTIM_MASTER_FLAG_MCMP4        HRTIM_MISR_MCMP4    /*!< Master compare 4 interrupt flag */
#define HRTIM_MASTER_FLAG_MREP         HRTIM_MISR_MREP     /*!< Master Repetition interrupt flag */
#define HRTIM_MASTER_FLAG_SYNC         HRTIM_MISR_SYNC     /*!< Synchronization input interrupt flag */
#define HRTIM_MASTER_FLAG_MUPD         HRTIM_MISR_MUPD     /*!< Master update interrupt flag */

/**
  * end of HRTIM_Master_Interrupt_Flag @}
  */

/** @defgroup HRTIM_Timing_Unit_Interrupt_Flag HRTIM Timing Unit Interrupt Flag
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the timer indexes
  *
  */

#define HRTIM_TIM_FLAG_CMP1       HRTIM_TIMISR_CMP1      /*!< Timer compare 1 interrupt flag */
#define HRTIM_TIM_FLAG_CMP2       HRTIM_TIMISR_CMP2      /*!< Timer compare 2 interrupt flag */
#define HRTIM_TIM_FLAG_CMP3       HRTIM_TIMISR_CMP3      /*!< Timer compare 3 interrupt flag */
#define HRTIM_TIM_FLAG_CMP4       HRTIM_TIMISR_CMP4      /*!< Timer compare 4 interrupt flag */
#define HRTIM_TIM_FLAG_REP        HRTIM_TIMISR_REP       /*!< Timer repetition interrupt flag */
#define HRTIM_TIM_FLAG_UPD        HRTIM_TIMISR_UPD       /*!< Timer update interrupt flag */
#define HRTIM_TIM_FLAG_CPT1       HRTIM_TIMISR_CPT1      /*!< Timer capture 1 interrupt flag */
#define HRTIM_TIM_FLAG_CPT2       HRTIM_TIMISR_CPT2      /*!< Timer capture 2 interrupt flag */
#define HRTIM_TIM_FLAG_SET1       HRTIM_TIMISR_SET1      /*!< Timer output 1 set interrupt flag */
#define HRTIM_TIM_FLAG_RST1       HRTIM_TIMISR_RST1      /*!< Timer output 1 reset interrupt flag */
#define HRTIM_TIM_FLAG_SET2       HRTIM_TIMISR_SET2      /*!< Timer output 2 set interrupt flag */
#define HRTIM_TIM_FLAG_RST2       HRTIM_TIMISR_RST2      /*!< Timer output 2 reset interrupt flag */
#define HRTIM_TIM_FLAG_RST        HRTIM_TIMISR_RST       /*!< Timer reset interrupt flag */
#define HRTIM_TIM_FLAG_DLYPRT     HRTIM_TIMISR_DLYPRT    /*!< Timer delay protection interrupt flag */

/**
  * end of HRTIM_Timing_Unit_Interrupt_Flag @}
  */

/** @defgroup HRTIM_Master_DMA_Request_Enable HRTIM Master DMA Request Enable
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the timer indexes
  *
  */

#define HRTIM_MASTER_DMA_NONE         0x00000000U            /*!< No DMA request enable */
#define HRTIM_MASTER_DMA_MCMP1        HRTIM_MDIER_MCMP1DE    /*!< Master compare 1 DMA request enable */
#define HRTIM_MASTER_DMA_MCMP2        HRTIM_MDIER_MCMP2DE    /*!< Master compare 2 DMA request enable */
#define HRTIM_MASTER_DMA_MCMP3        HRTIM_MDIER_MCMP3DE    /*!< Master compare 3 DMA request enable */
#define HRTIM_MASTER_DMA_MCMP4        HRTIM_MDIER_MCMP4DE    /*!< Master compare 4 DMA request enable */
#define HRTIM_MASTER_DMA_MREP         HRTIM_MDIER_MREPDE     /*!< Master Repetition DMA request enable */
#define HRTIM_MASTER_DMA_SYNC         HRTIM_MDIER_SYNCDE     /*!< Synchronization input DMA request enable */
#define HRTIM_MASTER_DMA_MUPD         HRTIM_MDIER_MUPDDE     /*!< Master update DMA request enable */

/**
  * end of HRTIM_Master_DMA_Request_Enable @}
  */

/** @defgroup HRTIM_Timing_Unit_DMA_Request_Enable HRTIM Timing Unit DMA Request Enable
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the timer indexes
  *
  */

#define HRTIM_TIM_DMA_NONE       0x00000000U               /*!< No DMA request enable */
#define HRTIM_TIM_DMA_CMP1       HRTIM_TIMDIER_CMP1DE      /*!< Timer compare 1 DMA request enable */
#define HRTIM_TIM_DMA_CMP2       HRTIM_TIMDIER_CMP2DE      /*!< Timer compare 2 DMA request enable */
#define HRTIM_TIM_DMA_CMP3       HRTIM_TIMDIER_CMP3DE      /*!< Timer compare 3 DMA request enable */
#define HRTIM_TIM_DMA_CMP4       HRTIM_TIMDIER_CMP4DE      /*!< Timer compare 4 DMA request enable */
#define HRTIM_TIM_DMA_REP        HRTIM_TIMDIER_REPDE       /*!< Timer repetition DMA request enable */
#define HRTIM_TIM_DMA_UPD        HRTIM_TIMDIER_UPDDE       /*!< Timer update DMA request enable */
#define HRTIM_TIM_DMA_CPT1       HRTIM_TIMDIER_CPT1DE      /*!< Timer capture 1 DMA request enable */
#define HRTIM_TIM_DMA_CPT2       HRTIM_TIMDIER_CPT2DE      /*!< Timer capture 2 DMA request enable */
#define HRTIM_TIM_DMA_SET1       HRTIM_TIMDIER_SET1DE      /*!< Timer output 1 set DMA request enable */
#define HRTIM_TIM_DMA_RST1       HRTIM_TIMDIER_RST1DE      /*!< Timer output 1 reset DMA request enable */
#define HRTIM_TIM_DMA_SET2       HRTIM_TIMDIER_SET2DE      /*!< Timer output 2 set DMA request enable */
#define HRTIM_TIM_DMA_RST2       HRTIM_TIMDIER_RST2DE      /*!< Timer output 2 reset DMA request enable */
#define HRTIM_TIM_DMA_RST        HRTIM_TIMDIER_RSTDE       /*!< Timer reset DMA request enable */
#define HRTIM_TIM_DMA_DLYPRT     HRTIM_TIMDIER_DLYPRTDE    /*!< Timer delay protection DMA request enable */

/**
  * end of HRTIM_Timing_Unit_DMA_Request_Enable @}
  */


/* Private Constants --------------------------------------------------------*/

/** @defgroup HRTIM_Caputre_Triiger HRTIM Caputre Triiger
  * @ingroup  HRTIM_Parameter_Definitions
  * @{
  */

/**
  * @brief Constants defining the timer indexes
  *
  */

#define HRTIM_CAPTUREFTRIGGER_NONE         0x00000000U                  /*!< 32bit value Capture trigger is disabled */
#define HRTIM_CAPTUREFTRIGGER_TF1_SET      (HRTIM_CPT1CR_TF1SET)        /*!< 32bit value Capture is triggered by TF1 output inactive to active transition */
#define HRTIM_CAPTUREFTRIGGER_TF1_RESET    (HRTIM_CPT1CR_TF1RST)        /*!< 32bit value Capture is triggered by TF1 output active to inactive transition */
#define HRTIM_CAPTUREFTRIGGER_TIMERF_CMP1  (HRTIM_CPT1CR_TIMFCMP1)      /*!< 32bit value Timer F Compare 1 triggers Capture */
#define HRTIM_CAPTUREFTRIGGER_TIMERF_CMP2  (HRTIM_CPT1CR_TIMFCMP2)      /*!< 32bit value Timer F Compare 2 triggers Capture */

/**
  * end of HRTIM_Caputre_Triiger @}
  */


/**
  * end of HRTIM_Parameter_Definitions @}
  */

/******************************************************************************/
/*                              HRTIM Macro                                   */
/******************************************************************************/

/** @defgroup HRTIM_Macro_Definitions HRTIM Macro Definitions
  * @{
  */

/* Private macros --------------------------------------------------------*/

/**
  * @brief  Check if the parameter __TIMERINDEX__ is valid
  * @param  __TIMERINDEX__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMERINDEX(__TIMERINDEX__)\
    (((__TIMERINDEX__) == HRTIM_TIMERINDEX_MASTER)   || \
     ((__TIMERINDEX__) == HRTIM_TIMERINDEX_TIMER_A)  || \
     ((__TIMERINDEX__) == HRTIM_TIMERINDEX_TIMER_B)  || \
     ((__TIMERINDEX__) == HRTIM_TIMERINDEX_TIMER_C)  || \
     ((__TIMERINDEX__) == HRTIM_TIMERINDEX_TIMER_D)  || \
     ((__TIMERINDEX__) == HRTIM_TIMERINDEX_TIMER_E)  || \
     ((__TIMERINDEX__) == HRTIM_TIMERINDEX_TIMER_F))

#define IS_HRTIM_TIMING_UNIT(__TIMERINDEX__)\
     (((__TIMERINDEX__) == HRTIM_TIMERINDEX_TIMER_A)  || \
      ((__TIMERINDEX__) == HRTIM_TIMERINDEX_TIMER_B)  || \
      ((__TIMERINDEX__) == HRTIM_TIMERINDEX_TIMER_C)  || \
      ((__TIMERINDEX__) == HRTIM_TIMERINDEX_TIMER_D)  || \
      ((__TIMERINDEX__) == HRTIM_TIMERINDEX_TIMER_E)  || \
      ((__TIMERINDEX__) == HRTIM_TIMERINDEX_TIMER_F))

/**
  * @brief  Check if the parameter __TIMERID__ is valid
  * @param  __TIMERID__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMERID(__TIMERID__) (((__TIMERID__) & 0xFF80FFFFU) == 0x00000000U)

/**
  * @brief  Check if the parameter __COMPAREUNIT__ is valid
  * @param  __COMPAREUNIT__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_COMPAREUNIT(__COMPAREUNIT__)\
    (((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_1)  || \
     ((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_2)  || \
     ((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_3)  || \
     ((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_4))

/**
  * @brief  Check if the parameter __CAPTUREUNIT__ is valid
  * @param  __CAPTUREUNIT__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_CAPTUREUNIT(__CAPTUREUNIT__)\
    (((__CAPTUREUNIT__) == HRTIM_CAPTUREUNIT_1)   || \
     ((__CAPTUREUNIT__) == HRTIM_CAPTUREUNIT_2))

/**
  * @brief  Check if the parameter __OUTPUT__ is valid
  * @param  __OUTPUT__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_OUTPUT(__OUTPUT__) (((__OUTPUT__) & 0xFFFFF000U) == 0x00000000U)

/**
  * @brief  Check if the parameter __TIMER__, __OUTPUT__ is valid
  * @param  __TIMER__, __OUTPUT__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMER_OUTPUT(__TIMER__, __OUTPUT__)     \
    ((((__TIMER__) == HRTIM_TIMERINDEX_TIMER_A) &&       \
     (((__OUTPUT__) == HRTIM_OUTPUT_TA1) ||          \
      ((__OUTPUT__) == HRTIM_OUTPUT_TA2)))           \
    ||                                                   \
    (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_B) &&        \
     (((__OUTPUT__) == HRTIM_OUTPUT_TB1) ||          \
      ((__OUTPUT__) == HRTIM_OUTPUT_TB2)))           \
    ||                                                   \
    (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_C) &&        \
     (((__OUTPUT__) == HRTIM_OUTPUT_TC1) ||          \
      ((__OUTPUT__) == HRTIM_OUTPUT_TC2)))           \
    ||                                                   \
    (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_D) &&        \
     (((__OUTPUT__) == HRTIM_OUTPUT_TD1) ||          \
      ((__OUTPUT__) == HRTIM_OUTPUT_TD2)))           \
    ||                                                   \
    (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_E) &&        \
     (((__OUTPUT__) == HRTIM_OUTPUT_TE1) ||          \
      ((__OUTPUT__) == HRTIM_OUTPUT_TE2)))           \
    ||                                                   \
    (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_F) &&        \
     (((__OUTPUT__) == HRTIM_OUTPUT_TF1) ||          \
      ((__OUTPUT__) == HRTIM_OUTPUT_TF2))))

/**
  * @brief  Check if the parameter __EVENT__ is valid
  * @param  __EVENT__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMEEVENT(__EVENT__)\
      (((__EVENT__) == HRTIM_EVENTCOUNTER_A))

/**
  * @brief  Check if the parameter __EVENT__, EVENTSRC is valid
  * @param  __EVENT__, EVENTSRC
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMEEVENT_RESETMODE(__EVENT__)\
      (((__EVENT__) == HRTIM_EVENTCOUNTER_RSTMODE_UNCONDITIONAL)   || \
       ((__EVENT__) == HRTIM_EVENTCOUNTER_RSTMODE_CONDITIONAL))

#define IS_HRTIM_TIMSYNCUPDATE(__EVENT__)\
      (((__EVENT__) == HRTIM_TIMERESYNC_UPDATE_UNCONDITIONAL)   || \
       ((__EVENT__) == HRTIM_TIMERESYNC_UPDATE_CONDITIONAL))

/**
  * @brief  Check if the parameter __COUNTER__ is valid
  * @param  __COUNTER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMEEVENT_COUNTER(__COUNTER__)                                       \
       ((((__COUNTER__) > (uint32_t)0x00U) && ((__COUNTER__) <= (uint32_t)0x3FU)) ||  \
         ((__COUNTER__) == (uint32_t)0x00U))

/**
  * @brief  Check if the parameter SOURCE is valid
  * @param  SOURCE
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMEEVENT_SOURCE(SOURCE)\
       (((SOURCE) >= (uint32_t)0x00U) && ((SOURCE) <= (uint32_t)0x9U))

/**
  * @brief  Check if the parameter __EVENT__ is valid
  * @param  __EVENT__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_EVENT(__EVENT__)\
      (((__EVENT__) == HRTIM_EVENT_NONE)|| \
       ((__EVENT__) == HRTIM_EVENT_1)   || \
       ((__EVENT__) == HRTIM_EVENT_2)   || \
       ((__EVENT__) == HRTIM_EVENT_3)   || \
       ((__EVENT__) == HRTIM_EVENT_4)   || \
       ((__EVENT__) == HRTIM_EVENT_5)   || \
       ((__EVENT__) == HRTIM_EVENT_6)   || \
       ((__EVENT__) == HRTIM_EVENT_7)   || \
       ((__EVENT__) == HRTIM_EVENT_8)   || \
       ((__EVENT__) == HRTIM_EVENT_9)   || \
       ((__EVENT__) == HRTIM_EVENT_10))

/**
  * @brief  Check if the parameter __EVENT__, __EVENTSRC__ is valid
  * @param  __EVENT__, __EVENTSRC__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_EVENTSRC(__EVENT__, __EVENTSRC__)                      \
    ((((__EVENT__) == HRTIM_EVENT_1) &&                             \
                 (((__EVENTSRC__) == HRTIM_EEV1SRC_GPIO      )   || \
                  ((__EVENTSRC__) == HRTIM_EEV1SRC_COMP2_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV1SRC_TIM1_TRGO )   || \
                  ((__EVENTSRC__) == HRTIM_EEV1SRC_ADC1_AWD1 )))    \
    ||                                                          \
     (((__EVENT__) == HRTIM_EVENT_2) &&                             \
                 (((__EVENTSRC__) == HRTIM_EEV2SRC_GPIO      )   || \
                  ((__EVENTSRC__) == HRTIM_EEV2SRC_COMP4_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV2SRC_TIM2_TRGO )   || \
                  ((__EVENTSRC__) == HRTIM_EEV2SRC_ADC1_AWD2 )))    \
    ||                                                              \
     (((__EVENT__) == HRTIM_EVENT_3) &&                             \
                 (((__EVENTSRC__) == HRTIM_EEV3SRC_GPIO      )   || \
                  ((__EVENTSRC__) == HRTIM_EEV3SRC_COMP6_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV3SRC_TIM3_TRGO )   || \
                  ((__EVENTSRC__) == HRTIM_EEV3SRC_ADC1_AWD3 )))    \
    ||                                                          \
     (((__EVENT__) == HRTIM_EVENT_4) &&                             \
                 (((__EVENTSRC__) == HRTIM_EEV4SRC_GPIO      )   || \
                  ((__EVENTSRC__) == HRTIM_EEV4SRC_COMP1_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV4SRC_COMP5_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV4SRC_ADC2_AWD1 )))    \
    ||                                                          \
     (((__EVENT__) == HRTIM_EVENT_5) &&                             \
                 (((__EVENTSRC__) == HRTIM_EEV5SRC_GPIO      )   || \
                  ((__EVENTSRC__) == HRTIM_EEV5SRC_COMP3_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV5SRC_COMP7_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV5SRC_ADC2_AWD2 )))    \
    ||                                                          \
     (((__EVENT__) == HRTIM_EVENT_6) &&                             \
                 (((__EVENTSRC__) == HRTIM_EEV6SRC_GPIO      )   || \
                  ((__EVENTSRC__) == HRTIM_EEV6SRC_COMP2_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV6SRC_COMP1_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV6SRC_ADC2_AWD3 )))    \
    ||                                                          \
     (((__EVENT__) == HRTIM_EVENT_7) &&                             \
                 (((__EVENTSRC__) == HRTIM_EEV7SRC_GPIO      )   || \
                  ((__EVENTSRC__) == HRTIM_EEV7SRC_COMP4_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV7SRC_TIM7_TRGO )   || \
                  ((__EVENTSRC__) == HRTIM_EEV7SRC_ADC3_AWD1 )))    \
    ||                                                          \
     (((__EVENT__) == HRTIM_EVENT_8) &&                             \
                 (((__EVENTSRC__) == HRTIM_EEV8SRC_GPIO      )   || \
                  ((__EVENTSRC__) == HRTIM_EEV8SRC_COMP6_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV8SRC_COMP3_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV8SRC_ADC4_AWD1 )))    \
    ||                                                          \
     (((__EVENT__) == HRTIM_EVENT_9) &&                             \
                 (((__EVENTSRC__) == HRTIM_EEV9SRC_GPIO      )   || \
                  ((__EVENTSRC__) == HRTIM_EEV9SRC_COMP5_OUT )   || \
                  ((__EVENTSRC__) == HRTIM_EEV9SRC_TIM15_TRGO)   || \
                  ((__EVENTSRC__) == HRTIM_EEV9SRC_COMP4_OUT )))    \
    ||                                                          \
     (((__EVENT__) == HRTIM_EVENT_10) &&                            \
                 (((__EVENTSRC__) == HRTIM_EEV10SRC_GPIO     )   || \
                  ((__EVENTSRC__) == HRTIM_EEV10SRC_COMP7_OUT)   || \
                  ((__EVENTSRC__) == HRTIM_EEV10SRC_TIM6_TRGO)   || \
                  ((__EVENTSRC__) == HRTIM_EEV10SRC_ADC5_AWD1))))

/**
  * @brief  Check if the parameter __EVENT__, __FASTMODE__ is valid
  * @param  __EVENT__, __FASTMODE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_EVENTFASTMODE(__EVENT__, __FASTMODE__)\
    (((((__EVENT__) == HRTIM_EVENT_1) ||                 \
       ((__EVENT__) == HRTIM_EVENT_2) ||                 \
       ((__EVENT__) == HRTIM_EVENT_3) ||                 \
       ((__EVENT__) == HRTIM_EVENT_4) ||                 \
       ((__EVENT__) == HRTIM_EVENT_5)) &&                \
      (((__FASTMODE__) == HRTIM_EVENTFASTMODE_ENABLE) || \
       ((__FASTMODE__) == HRTIM_EVENTFASTMODE_DISABLE))) \
    ||                                               \
    (((__EVENT__) == HRTIM_EVENT_6) ||                   \
     ((__EVENT__) == HRTIM_EVENT_7) ||                   \
     ((__EVENT__) == HRTIM_EVENT_8) ||                   \
     ((__EVENT__) == HRTIM_EVENT_9) ||                   \
     ((__EVENT__) == HRTIM_EVENT_10)))

/**
  * @brief  Check if the parameter __FAULT__ is valid
  * @param  __FAULT__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_FAULT(__FAULT__)\
      (((__FAULT__) == HRTIM_FAULT_1)   || \
       ((__FAULT__) == HRTIM_FAULT_2)   || \
       ((__FAULT__) == HRTIM_FAULT_3)   || \
       ((__FAULT__) == HRTIM_FAULT_4)   || \
       ((__FAULT__) == HRTIM_FAULT_5)   || \
       ((__FAULT__) == HRTIM_FAULT_6))

/**
  * @brief  Check if the parameter __PRESCALERRATIO__ is valid
  * @param  __PRESCALERRATIO__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_PRESCALERRATIO(__PRESCALERRATIO__)\
        (((__PRESCALERRATIO__) == HRTIM_PRESCALERRATIO_MUL32) || \
         ((__PRESCALERRATIO__) == HRTIM_PRESCALERRATIO_MUL16) || \
         ((__PRESCALERRATIO__) == HRTIM_PRESCALERRATIO_MUL8)  || \
         ((__PRESCALERRATIO__) == HRTIM_PRESCALERRATIO_MUL4)  || \
         ((__PRESCALERRATIO__) == HRTIM_PRESCALERRATIO_MUL2)  || \
         ((__PRESCALERRATIO__) == HRTIM_PRESCALERRATIO_DIV1)  || \
         ((__PRESCALERRATIO__) == HRTIM_PRESCALERRATIO_DIV2)  || \
         ((__PRESCALERRATIO__) == HRTIM_PRESCALERRATIO_DIV4))

/**
  * @brief  Check if the parameter __MODE__ is valid
  * @param  __MODE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_MODE(__MODE__)\
          (((__MODE__) == HRTIM_MODE_CONTINUOUS)  ||  \
           ((__MODE__) == HRTIM_MODE_SINGLESHOT) || \
           ((__MODE__) == HRTIM_MODE_SINGLESHOT_RETRIGGERABLE))

#define IS_HRTIM_MODE_ONEPULSE(__MODE__)\
          (((__MODE__) == HRTIM_MODE_SINGLESHOT) || \
           ((__MODE__) == HRTIM_MODE_SINGLESHOT_RETRIGGERABLE))

#define IS_HRTIM_TIMERBURSTMODE(__MODE__)                               \
                (((__MODE__) == HRTIM_TIMERBURSTMODE_MAINTAINCLOCK)  || \
                 ((__MODE__) == HRTIM_TIMERBURSTMODE_RESETCOUNTER))

#define IS_HRTIM_TIMERUPDOWNMODE(__MODE__)                           \
                (((__MODE__) == HRTIM_TIMERUPDOWNMODE_UP)  ||        \
                 ((__MODE__) == HRTIM_TIMERUPDOWNMODE_UPDOWN))

#define IS_HRTIM_TIMERTRGHLFMODE(__MODE__)                           \
                (((__MODE__) == HRTIM_TIMERTRIGHALF_DISABLED)  ||    \
                 ((__MODE__) == HRTIM_TIMERTRIGHALF_ENABLED))


#define IS_HRTIM_TIMERGTCMP3(__MODE__)                               \
                (((__MODE__) == HRTIM_TIMERGTCMP3_EQUAL)  ||         \
                 ((__MODE__) == HRTIM_TIMERGTCMP3_GREATER))

#define IS_HRTIM_TIMERGTCMP1(__MODE__)                               \
                (((__MODE__) == HRTIM_TIMERGTCMP1_EQUAL)  ||         \
                 ((__MODE__) == HRTIM_TIMERGTCMP1_GREATER))
/**
  * @brief  Check if the parameter __HALFMODE__ is valid
  * @param  __HALFMODE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_HALFMODE(__HALFMODE__)\
            (((__HALFMODE__) == HRTIM_HALFMODE_DISABLED)  ||  \
             ((__HALFMODE__) == HRTIM_HALFMODE_ENABLED))

/**
  * @brief  Check if the parameter __INTLVDMODE__ is valid
  * @param  __INTLVDMODE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_INTERLEAVEDMODE(__INTLVDMODE__)\
            (((__INTLVDMODE__) == HRTIM_INTERLEAVED_MODE_DISABLED)  ||  \
             ((__INTLVDMODE__) == HRTIM_INTERLEAVED_MODE_DUAL)      ||  \
             ((__INTLVDMODE__) == HRTIM_INTERLEAVED_MODE_DISABLED)  ||  \
             ((__INTLVDMODE__) == HRTIM_INTERLEAVED_MODE_TRIPLE)    ||  \
             ((__INTLVDMODE__) == HRTIM_INTERLEAVED_MODE_DISABLED)  ||  \
             ((__INTLVDMODE__) == HRTIM_INTERLEAVED_MODE_QUAD))

/**
  * @brief  Check if the parameter __SYNCSTART__ is valid
  * @param  __SYNCSTART__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_SYNCSTART(__SYNCSTART__)\
              (((__SYNCSTART__) == HRTIM_SYNCSTART_DISABLED)  ||  \
               ((__SYNCSTART__) == HRTIM_SYNCSTART_ENABLED))

/**
  * @brief  Check if the parameter __SYNCRESET__ is valid
  * @param  __SYNCRESET__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_SYNCRESET(__SYNCRESET__)\
                (((__SYNCRESET__) == HRTIM_SYNCRESET_DISABLED)  ||  \
                 ((__SYNCRESET__) == HRTIM_SYNCRESET_ENABLED))

/**
  * @brief  Check if the parameter __DACSYNC__ is valid
  * @param  __DACSYNC__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_DACSYNC(__DACSYNC__)\
                (((__DACSYNC__) == HRTIM_DACSYNC_NONE)          ||  \
                 ((__DACSYNC__) == HRTIM_DACSYNC_DACTRIGOUT_1)  ||  \
                 ((__DACSYNC__) == HRTIM_DACSYNC_DACTRIGOUT_2)  ||  \
                 ((__DACSYNC__) == HRTIM_DACSYNC_DACTRIGOUT_3))

/**
  * @brief  Check if the parameter __PRELOAD__ is valid
  * @param  __PRELOAD__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_PRELOAD(__PRELOAD__)\
                (((__PRELOAD__) == HRTIM_PRELOAD_DISABLED)  ||  \
                 ((__PRELOAD__) == HRTIM_PRELOAD_ENABLED))

/**
  * @brief  Check if the parameter __UPDATEGATING__ is valid
  * @param  __UPDATEGATING__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_UPDATEGATING_MASTER(__UPDATEGATING__)                           \
                (((__UPDATEGATING__) == HRTIM_UPDATEGATING_INDEPENDENT)      ||  \
                 ((__UPDATEGATING__) == HRTIM_UPDATEGATING_DMABURST)         ||  \
                 ((__UPDATEGATING__) == HRTIM_UPDATEGATING_DMABURST_UPDATE))

#define IS_HRTIM_UPDATEGATING_TIM(__UPDATEGATING__)                              \
                (((__UPDATEGATING__) == HRTIM_UPDATEGATING_INDEPENDENT)      ||  \
                 ((__UPDATEGATING__) == HRTIM_UPDATEGATING_DMABURST)         ||  \
                 ((__UPDATEGATING__) == HRTIM_UPDATEGATING_DMABURST_UPDATE)  ||  \
                 ((__UPDATEGATING__) == HRTIM_UPDATEGATING_UPDEN1)           ||  \
                 ((__UPDATEGATING__) == HRTIM_UPDATEGATING_UPDEN2)           ||  \
                 ((__UPDATEGATING__) == HRTIM_UPDATEGATING_UPDEN3)           ||  \
                 ((__UPDATEGATING__) == HRTIM_UPDATEGATING_UPDEN1_UPDATE)    ||  \
                 ((__UPDATEGATING__) == HRTIM_UPDATEGATING_UPDEN2_UPDATE)    ||  \
                 ((__UPDATEGATING__) == HRTIM_UPDATEGATING_UPDEN3_UPDATE))


/**
  * @brief  Check if the parameter __DUALCHANNELDAC__ is valid
  * @param  __DUALCHANNELDAC__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_DUALDAC_RESET(__DUALCHANNELDAC__)                              \
                (((__DUALCHANNELDAC__) == HRTIM_TIMER_DCDR_COUNTER)  ||         \
                 ((__DUALCHANNELDAC__) == HRTIM_TIMER_DCDR_OUT1SET))

#define IS_HRTIM_DUALDAC_STEP(__DUALCHANNELDAC__)                               \
                (((__DUALCHANNELDAC__) == HRTIM_TIMER_DCDS_CMP2)  ||            \
                 ((__DUALCHANNELDAC__) == HRTIM_TIMER_DCDS_OUT1RST))

#define IS_HRTIM_DUALDAC_ENABLE(__DUALCHANNELDAC__)                             \
                (((__DUALCHANNELDAC__) == HRTIM_TIMER_DCDE_DISABLED)  ||        \
                 ((__DUALCHANNELDAC__) == HRTIM_TIMER_DCDE_ENABLED ))

/**
  * @brief  Check if the parameter __UPDATEONREPETITION__ is valid
  * @param  __UPDATEONREPETITION__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_UPDATEONREPETITION(__UPDATEONREPETITION__)                            \
                (((__UPDATEONREPETITION__) == HRTIM_UPDATEONREPETITION_DISABLED)  ||   \
                 ((__UPDATEONREPETITION__) == HRTIM_UPDATEONREPETITION_ENABLED))

/**
  * @brief  Check if the parameter __TIMPUSHPULLMODE__ is valid
  * @param  __TIMPUSHPULLMODE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMPUSHPULLMODE(__TIMPUSHPULLMODE__)                           \
                  (((__TIMPUSHPULLMODE__) == HRTIM_TIMPUSHPULLMODE_DISABLED) || \
                   ((__TIMPUSHPULLMODE__) == HRTIM_TIMPUSHPULLMODE_ENABLED))

/**
  * @brief  Check if the parameter __TIMFAULTENABLE__ is valid
  * @param  __TIMFAULTENABLE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMFAULTENABLE(__TIMFAULTENABLE__)   (((__TIMFAULTENABLE__) & 0xFFFFFFC0U) == 0x00000000U)

/**
  * @brief  Check if the parameter __TIMFAULTLOCK__ is valid
  * @param  __TIMFAULTLOCK__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMFAULTLOCK(__TIMFAULTLOCK__)\
      (((__TIMFAULTLOCK__) == HRTIM_TIMFAULTLOCK_READWRITE) || \
       ((__TIMFAULTLOCK__) == HRTIM_TIMFAULTLOCK_READONLY))

/**
  * @brief  Check if the parameter __TIMPUSHPULLMODE__, TIMDEADTIMEINSERTION is valid
  * @param  __TIMPUSHPULLMODE__, TIMDEADTIMEINSERTION
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMDEADTIMEINSERTION(__TIMPUSHPULLMODE__, TIMDEADTIMEINSERTION)\
    (((TIMDEADTIMEINSERTION) == HRTIM_TIMDEADTIMEINSERTION_DISABLED) || \
          ((TIMDEADTIMEINSERTION) == HRTIM_TIMDEADTIMEINSERTION_ENABLED))

/**
  * @brief  Check if the parameter __TIMPUSHPULLMODE__, __TIMDELAYEDPROTECTION__ is valid
  * @param  __TIMPUSHPULLMODE__, __TIMDELAYEDPROTECTION__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMDELAYEDPROTECTION(__TIMPUSHPULLMODE__, __TIMDELAYEDPROTECTION__)\
          ((((__TIMDELAYEDPROTECTION__) == HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED)          || \
            ((__TIMDELAYEDPROTECTION__) == HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DELAYEDOUT1_EEV6)  || \
            ((__TIMDELAYEDPROTECTION__) == HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DELAYEDOUT2_EEV6)  || \
            ((__TIMDELAYEDPROTECTION__) == HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DELAYEDBOTH_EEV6)  || \
            ((__TIMDELAYEDPROTECTION__) == HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DELAYEDOUT1_DEEV7) || \
            ((__TIMDELAYEDPROTECTION__) == HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DELAYEDOUT2_DEEV7) || \
            ((__TIMDELAYEDPROTECTION__) == HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DELAYEDBOTH_EEV7))    \
            ||                                                                           \
            (((__TIMPUSHPULLMODE__) ==  HRTIM_TIMPUSHPULLMODE_ENABLED) &&                    \
             (((__TIMDELAYEDPROTECTION__) == HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_BALANCED_EEV6)     || \
             ((__TIMDELAYEDPROTECTION__) == HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_BALANCED_EEV7))))

/**
  * @brief  Check if the parameter __TIMUPDATETRIGGER__ is valid
  * @param  __TIMUPDATETRIGGER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMUPDATETRIGGER(__TIMUPDATETRIGGER__)    (((__TIMUPDATETRIGGER__) & 0xFE06FFFFU) == 0x00000000U)

/**
  * @brief  Check if the parameter __TIMRESETTRIGGER__ is valid
  * @param  __TIMRESETTRIGGER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMRESETTRIGGER(__TIMRESETTRIGGER__)    (((__TIMRESETTRIGGER__) & 0x00000000U) == 0x00000000U)


/**
  * @brief  Check if the parameter __TIMUPDATEONRESET__ is valid
  * @param  __TIMUPDATEONRESET__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMUPDATEONRESET(__TIMUPDATEONRESET__)                       \
              (((__TIMUPDATEONRESET__) == HRTIM_TIMUPDATEONRESET_DISABLED) || \
               ((__TIMUPDATEONRESET__) == HRTIM_TIMUPDATEONRESET_ENABLED))

/**
  * @brief  Check if the parameter __AUTODELAYEDMODE__ is valid
  * @param  __AUTODELAYEDMODE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_AUTODELAYEDMODE(__AUTODELAYEDMODE__)                                       \
              (((__AUTODELAYEDMODE__) == HRTIM_AUTODELAYEDMODE_REGULAR)                  || \
               ((__AUTODELAYEDMODE__) == HRTIM_AUTODELAYEDMODE_AUTODELAYED_NOTIMEOUT)    || \
               ((__AUTODELAYEDMODE__) == HRTIM_AUTODELAYEDMODE_AUTODELAYED_TIMEOUTCMP1)  || \
               ((__AUTODELAYEDMODE__) == HRTIM_AUTODELAYEDMODE_AUTODELAYED_TIMEOUTCMP3))

/**
  * @brief  Check if the parameter __COMPAREUNIT__, __AUTODELAYEDMODE__ is valid
  * @brief  Auto delayed mode is only available for compare units 2 and 4U
  * @param  __COMPAREUNIT__, __AUTODELAYEDMODE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_COMPAREUNIT_AUTODELAYEDMODE(__COMPAREUNIT__, __AUTODELAYEDMODE__)     \
    ((((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_2) &&                                 \
     (((__AUTODELAYEDMODE__) == HRTIM_AUTODELAYEDMODE_REGULAR)                 ||  \
      ((__AUTODELAYEDMODE__) == HRTIM_AUTODELAYEDMODE_AUTODELAYED_NOTIMEOUT)   ||  \
      ((__AUTODELAYEDMODE__) == HRTIM_AUTODELAYEDMODE_AUTODELAYED_TIMEOUTCMP1) ||  \
      ((__AUTODELAYEDMODE__) == HRTIM_AUTODELAYEDMODE_AUTODELAYED_TIMEOUTCMP3)))   \
    ||                                                                         \
    (((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_4) &&                                 \
     (((__AUTODELAYEDMODE__) == HRTIM_AUTODELAYEDMODE_REGULAR)                 ||  \
      ((__AUTODELAYEDMODE__) == HRTIM_AUTODELAYEDMODE_AUTODELAYED_NOTIMEOUT)   ||  \
      ((__AUTODELAYEDMODE__) == HRTIM_AUTODELAYEDMODE_AUTODELAYED_TIMEOUTCMP1) ||  \
      ((__AUTODELAYEDMODE__) == HRTIM_AUTODELAYEDMODE_AUTODELAYED_TIMEOUTCMP3))))

/**
  * @brief  Check if the parameter __OUTPUTPOLARITY__ is valid
  * @param  __OUTPUTPOLARITY__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_OUTPUTPOLARITY(__OUTPUTPOLARITY__)\
              (((__OUTPUTPOLARITY__) == HRTIM_OUTPUTPOLARITY_HIGH) || \
               ((__OUTPUTPOLARITY__) == HRTIM_OUTPUTPOLARITY_LOW))

/**
  * @brief  Check if the parameter __OUTPUTPULSE__ is valid
  * @param  __OUTPUTPULSE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_OUTPUTPULSE(__OUTPUTPULSE__)    ((__OUTPUTPULSE__) <= 0x0000FFFFU)

/**
  * @brief  Check if the parameter __OUTPUTSET__ is valid
  * @param  __OUTPUTSET__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_OUTPUTSET(__OUTPUTSET__)                               \
              (((__OUTPUTSET__) == HRTIM_OUTPUTSET_NONE)             || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_RESYNC)           || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMPER)           || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMCMP1)          || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMCMP2)          || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMCMP3)          || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMCMP4)          || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_MASTERPER)        || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_MASTERCMP1)       || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_MASTERCMP2)       || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_MASTERCMP3)       || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_MASTERCMP4)       || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMAEV1_TIMBCMP1) || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMAEV2_TIMBCMP2) || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMAEV3_TIMCCMP2) || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMAEV4_TIMCCMP3) || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMAEV5_TIMDCMP1) || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMAEV6_TIMDCMP2) || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMAEV7_TIMECMP3) || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMAEV8_TIMECMP4) || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_TIMAEV9_TIMFCMP4) || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_EEV_1)            || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_EEV_2)            || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_EEV_3)            || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_EEV_4)            || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_EEV_5)            || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_EEV_6)            || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_EEV_7)            || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_EEV_8)            || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_EEV_9)            || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_EEV_10)           || \
               ((__OUTPUTSET__) == HRTIM_OUTPUTSET_UPDATE))

/**
  * @brief  Check if the parameter __OUTPUTRESET__ is valid
  * @param  __OUTPUTRESET__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_OUTPUTRESET(__OUTPUTRESET__)                               \
              (((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_NONE)             || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_RESYNC)           || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMPER)           || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMCMP1)          || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMCMP2)          || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMCMP3)          || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMCMP4)          || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_MASTERPER)        || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_MASTERCMP1)       || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_MASTERCMP2)       || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_MASTERCMP3)       || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_MASTERCMP4)       || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMAEV1_TIMBCMP1) || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMAEV2_TIMBCMP2) || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMAEV3_TIMCCMP2) || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMAEV4_TIMCCMP3) || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMAEV5_TIMDCMP1) || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMAEV6_TIMDCMP2) || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMAEV7_TIMECMP3) || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMAEV8_TIMECMP4) || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_TIMAEV9_TIMFCMP4) || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_EEV_1)            || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_EEV_2)            || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_EEV_3)            || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_EEV_4)            || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_EEV_5)            || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_EEV_6)            || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_EEV_7)            || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_EEV_8)            || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_EEV_9)            || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_EEV_10)           || \
               ((__OUTPUTRESET__) == HRTIM_OUTPUTRESET_UPDATE))

/**
  * @brief  Check if the parameter __OUTPUTIDLEMODE__ is valid
  * @param  __OUTPUTIDLEMODE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_OUTPUTIDLEMODE(__OUTPUTIDLEMODE__)\
              (((__OUTPUTIDLEMODE__) == HRTIM_OUTPUTIDLEMODE_NONE) || \
               ((__OUTPUTIDLEMODE__) == HRTIM_OUTPUTIDLEMODE_IDLE))
/**
  * @brief  Check if the parameter __OUTPUTIDLELEVEL__ is valid
  * @param  __OUTPUTIDLELEVEL__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_OUTPUTIDLELEVEL(__OUTPUTIDLELEVEL__)\
              (((__OUTPUTIDLELEVEL__) == HRTIM_OUTPUTIDLELEVEL_INACTIVE) || \
               ((__OUTPUTIDLELEVEL__) == HRTIM_OUTPUTIDLELEVEL_ACTIVE))

/**
  * @brief  Check if the parameter __OUTPUTFAULTLEVEL__ is valid
  * @param  __OUTPUTFAULTLEVEL__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_OUTPUTFAULTLEVEL(__OUTPUTFAULTLEVEL__)\
              (((__OUTPUTFAULTLEVEL__) == HRTIM_OUTPUTFAULTLEVEL_NONE)     || \
               ((__OUTPUTFAULTLEVEL__) == HRTIM_OUTPUTFAULTLEVEL_ACTIVE)   || \
               ((__OUTPUTFAULTLEVEL__) == HRTIM_OUTPUTFAULTLEVEL_INACTIVE) || \
               ((__OUTPUTFAULTLEVEL__) == HRTIM_OUTPUTFAULTLEVEL_HIGHZ))

/**
  * @brief  Check if the parameter __OUTPUTCHOPPERMODE__ is valid
  * @param  __OUTPUTCHOPPERMODE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_OUTPUTCHOPPERMODE(__OUTPUTCHOPPERMODE__)\
              (((__OUTPUTCHOPPERMODE__) == HRTIM_OUTPUTCHOPPERMODE_DISABLED)  || \
               ((__OUTPUTCHOPPERMODE__) == HRTIM_OUTPUTCHOPPERMODE_ENABLED))

/**
  * @brief  Check if the parameter __OUTPUTBURSTMODEENTRY__ is valid
  * @param  __OUTPUTBURSTMODEENTRY__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_OUTPUTBURSTMODEENTRY(__OUTPUTBURSTMODEENTRY__)\
              (((__OUTPUTBURSTMODEENTRY__) == HRTIM_OUTPUTBURSTMODEENTRY_REGULAR)  || \
               ((__OUTPUTBURSTMODEENTRY__) == HRTIM_OUTPUTBURSTMODEENTRY_DELAYED))

/**
  * @brief  Check if the parameter __OUTPUTBIAR__ is valid
  * @param  __OUTPUTBIAR__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_OUTPUTBALANCEDIDLE(__OUTPUTBIAR__)\
              (((__OUTPUTBIAR__) == HRTIM_OUTPUTBIAR_DISABLED)  || \
               ((__OUTPUTBIAR__) == HRTIM_OUTPUTBIAR_ENABLED))

/**
  * @brief  Check if the parameter TIMER, __CAPTURETRIGGER__ is valid
  * @param  __TIMER__, __CAPTURETRIGGER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMER_CAPTURETRIGGER(__TIMER__, __CAPTURETRIGGER__)    \
   (((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_NONE)          || \
   ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_UPDATE)         || \
   ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_EEV_1)          || \
   ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_EEV_2)          || \
   ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_EEV_3)          || \
   ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_EEV_4)          || \
   ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_EEV_5)          || \
   ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_EEV_6)          || \
   ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_EEV_7)          || \
   ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_EEV_8)          || \
   ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_EEV_9)          || \
   ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_EEV_10)            \
   ||                                                           \
   (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_A) &&                    \
     (((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TB1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TB1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERB_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERB_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TC1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TC1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERC_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERC_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TD1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TD1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERD_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERD_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TE1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TE1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERE_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERE_CMP2)))  \
    ||                                                          \
   (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_B) &&                    \
     (((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TA1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TA1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERA_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERA_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TC1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TC1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERC_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERC_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TD1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TD1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERD_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERD_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TE1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TE1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERE_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERE_CMP2)))  \
    ||                                                          \
   (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_C) &&                    \
     (((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TA1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TA1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERA_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERA_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TB1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TB1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERB_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERB_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TD1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TD1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERD_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERD_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TE1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TE1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERE_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERE_CMP2)))  \
    ||                                                          \
   (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_D) &&                    \
     (((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TA1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TA1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERA_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERA_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TB1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TB1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERB_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERB_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TC1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TC1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERC_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERC_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TE1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TE1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERE_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERE_CMP2)))  \
    ||                                                          \
   (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_E) &&                    \
     (((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TA1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TA1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERA_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERA_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TB1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TB1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERB_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERB_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TC1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TC1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERC_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERC_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TD1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TD1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERD_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERD_CMP2)))  \
    ||                                                          \
   (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_F) &&                    \
     (((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TA1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TA1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERA_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERA_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TB1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TB1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERB_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERB_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TC1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TC1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERC_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERC_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TD1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TD1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERD_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERD_CMP2) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TE1_SET)     || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TE1_RESET)   || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERE_CMP1) || \
      ((__CAPTURETRIGGER__) == HRTIM_CAPTURETRIGGER_TIMERE_CMP2))))

/**
  * @brief  Check if the parameter TIMER, __CAPTUREFTRIGGER__ is valid
  * @param  __TIMER__, __CAPTUREFTRIGGER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMER_CAPTUREFTRIGGER(__TIMER__, __CAPTUREFTRIGGER__)    \
   (  ((__CAPTUREFTRIGGER__) == HRTIM_CAPTUREFTRIGGER_NONE)        || \
      ((__CAPTUREFTRIGGER__) == HRTIM_CAPTUREFTRIGGER_TF1_SET)     || \
      ((__CAPTUREFTRIGGER__) == HRTIM_CAPTUREFTRIGGER_TF1_RESET)   || \
      ((__CAPTUREFTRIGGER__) == HRTIM_CAPTUREFTRIGGER_TIMERF_CMP1) || \
      ((__CAPTUREFTRIGGER__) == HRTIM_CAPTUREFTRIGGER_TIMERF_CMP2))

/**
  * @brief  Check if the parameter __TIMER__,__TIMEVENTFILTER__ is valid
  * @param  __TIMER__, __TIMEVENTFILTER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMEVENTFILTER(__TIMER__,__TIMEVENTFILTER__)\
   (((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_NONE)           || \
    ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKINGCMP1)   || \
    ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKINGCMP2)   || \
    ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKINGCMP3)   || \
    ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKINGCMP4)   || \
    ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_WINDOWINGCMP2)  || \
    ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_WINDOWINGCMP3)  || \
    ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_WINDOWINGTIM)      \
  ||                                                             \
   (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_A) &&                     \
     (((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMAEEF1_TIMBCMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMAEEF2_TIMBCMP4)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMAEEF3_TIMBOUT2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMAEEF4_TIMCCMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMAEEF5_TIMCCMP4)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMAEEF6_TIMFCMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMAEEF7_TIMDCMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMAEEF8_TIMECMP2)))   \
    ||                                                           \
   (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_B) &&                     \
     (((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMBEEF1_TIMACMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMBEEF2_TIMACMP4)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMBEEF3_TIMAOUT2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMBEEF4_TIMCCMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMBEEF5_TIMCCMP2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMBEEF6_TIMFCMP2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMBEEF7_TIMDCMP2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMBEEF8_TIMECMP1)))   \
    ||                                                           \
   (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_C) &&                     \
     (((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMCEEF1_TIMACMP2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMCEEF2_TIMBCMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMCEEF3_TIMBCMP4)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMCEEF4_TIMFCMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMCEEF5_TIMDCMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMCEEF6_TIMDCMP4)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMCEEF7_TIMDOUT2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMCEEF8_TIMECMP4)))   \
    ||                                                           \
   (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_D) &&                     \
     (((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMDEEF1_TIMACMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMDEEF2_TIMBCMP2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMDEEF3_TIMCCMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMDEEF4_TIMCCMP2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMDEEF5_TIMCOUT2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMDEEF6_TIMECMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMDEEF7_TIMECMP4)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMDEEF8_TIMFCMP4)))   \
    ||                                                           \
   (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_E) &&                     \
     (((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMEEEF1_TIMACMP2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMEEEF2_TIMBCMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMEEEF3_TIMCCMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMEEEF4_TIMFCMP4)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMEEEF5_TIMFOUT2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMEEEF6_TIMDCMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMEEEF7_TIMDCMP4)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMEEEF8_TIMDOUT2)))   \
    ||                                                           \
   (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_F) &&                     \
     (((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMFEEF1_TIMACMP4)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMFEEF2_TIMBCMP2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMFEEF3_TIMCCMP4)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMFEEF4_TIMDCMP2)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMFEEF5_TIMDCMP4)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMFEEF6_TIMECMP1)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMFEEF7_TIMECMP4)  || \
      ((__TIMEVENTFILTER__) == HRTIM_TIMEEVFLT_BLANKING_TIMFEEF8_TIMEOUT2))))

/**
  * @brief  Check if the parameter __TIMEVENTLATCH__ is valid
  * @param  __TIMEVENTLATCH__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMEVENTLATCH(__TIMEVENTLATCH__)\
              (((__TIMEVENTLATCH__) == HRTIM_TIMEVENTLATCH_DISABLED) || \
               ((__TIMEVENTLATCH__) == HRTIM_TIMEVENTLATCH_ENABLED))

/**
  * @brief  Check if the parameter __PRESCALERRATIO__ is valid
  * @param  __PRESCALERRATIO__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMDEADTIME_PRESCALERRATIO(__PRESCALERRATIO__)\
                (((__PRESCALERRATIO__) == HRTIM_TIMDEADTIME_PRESCALERRATIO_MUL8) || \
                 ((__PRESCALERRATIO__) == HRTIM_TIMDEADTIME_PRESCALERRATIO_MUL4) || \
                 ((__PRESCALERRATIO__) == HRTIM_TIMDEADTIME_PRESCALERRATIO_MUL2) || \
                 ((__PRESCALERRATIO__) == HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV1) || \
                 ((__PRESCALERRATIO__) == HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV2) || \
                 ((__PRESCALERRATIO__) == HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV4) || \
                 ((__PRESCALERRATIO__) == HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV8) || \
                 ((__PRESCALERRATIO__) == HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV16))

/**
  * @brief  Check if the parameter __RISINGSIGN__ is valid
  * @param  __RISINGSIGN__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMDEADTIME_RISINGSIGN(__RISINGSIGN__)\
                (((__RISINGSIGN__) == HRTIM_TIMDEADTIME_RISINGSIGN_POSITIVE)    || \
                 ((__RISINGSIGN__) == HRTIM_TIMDEADTIME_RISINGSIGN_NEGATIVE))

/**
  * @brief  Check if the parameter __RISINGLOCK__ is valid
  * @param  __RISINGLOCK__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMDEADTIME_RISINGLOCK(__RISINGLOCK__)\
                    (((__RISINGLOCK__) == HRTIM_TIMDEADTIME_RISINGLOCK_WRITE)    || \
                     ((__RISINGLOCK__) == HRTIM_TIMDEADTIME_RISINGLOCK_READONLY))

/**
  * @brief  Check if the parameter __RISINGSIGNLOCK__ is valid
  * @param  __RISINGSIGNLOCK__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMDEADTIME_RISINGSIGNLOCK(__RISINGSIGNLOCK__)\
                  (((__RISINGSIGNLOCK__) == HRTIM_TIMDEADTIME_RISINGSIGNLOCK_WRITE)    || \
                   ((__RISINGSIGNLOCK__) == HRTIM_TIMDEADTIME_RISINGSIGNLOCK_READONLY))

/**
  * @brief  Check if the parameter __FALLINGSIGN__ is valid
  * @param  __FALLINGSIGN__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMDEADTIME_FALLINGSIGN(__FALLINGSIGN__)\
                      (((__FALLINGSIGN__) == HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE)    || \
                       ((__FALLINGSIGN__) == HRTIM_TIMDEADTIME_FALLINGSIGN_NEGATIVE))

/**
  * @brief  Check if the parameter __FALLINGLOCK__ is valid
  * @param  __FALLINGLOCK__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMDEADTIME_FALLINGLOCK(__FALLINGLOCK__)\
                          (((__FALLINGLOCK__) == HRTIM_TIMDEADTIME_FALLINGLOCK_WRITE)    || \
                           ((__FALLINGLOCK__) == HRTIM_TIMDEADTIME_FALLINGLOCK_READONLY))

/**
  * @brief  Check if the parameter __FALLINGSIGNLOCK__ is valid
  * @param  __FALLINGSIGNLOCK__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMDEADTIME_FALLINGSIGNLOCK(__FALLINGSIGNLOCK__)\
                        (((__FALLINGSIGNLOCK__) == HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_WRITE)    || \
                         ((__FALLINGSIGNLOCK__) == HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_READONLY))

/**
  * @brief  Check if the parameter __PRESCALERRATIO__ is valid
  * @param  __PRESCALERRATIO__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_CHOPPER_PRESCALERRATIO(__PRESCALERRATIO__)\
                        (((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV16)    || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV32)    || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV48)    || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV64)    || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV80)    || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV96)    || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV112)   || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV128)   || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV144)   || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV160)   || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV176)   || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV192)   || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV208)   || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV224)   || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV240)   || \
                         ((__PRESCALERRATIO__) == HRTIM_CHOPPER_PRESCALERRATIO_DIV256))

/**
  * @brief  Check if the parameter __DUTYCYCLE__ is valid
  * @param  __DUTYCYCLE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_CHOPPER_DUTYCYCLE(__DUTYCYCLE__)                             \
                        (((__DUTYCYCLE__) == HRTIM_CHOPPER_DUTYCYCLE_0)    || \
                         ((__DUTYCYCLE__) == HRTIM_CHOPPER_DUTYCYCLE_125)  || \
                         ((__DUTYCYCLE__) == HRTIM_CHOPPER_DUTYCYCLE_250)  || \
                         ((__DUTYCYCLE__) == HRTIM_CHOPPER_DUTYCYCLE_375)  || \
                         ((__DUTYCYCLE__) == HRTIM_CHOPPER_DUTYCYCLE_500)  || \
                         ((__DUTYCYCLE__) == HRTIM_CHOPPER_DUTYCYCLE_625)  || \
                         ((__DUTYCYCLE__) == HRTIM_CHOPPER_DUTYCYCLE_750)  || \
                         ((__DUTYCYCLE__) == HRTIM_CHOPPER_DUTYCYCLE_875))

/**
  * @brief  Check if the parameter __PULSEWIDTH__ is valid
  * @param  __PULSEWIDTH__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_CHOPPER_PULSEWIDTH(__PULSEWIDTH__)\
                        (((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_16)   || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_32)   || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_48)   || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_64)   || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_80)   || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_96)   || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_112)  || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_128)  || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_144)  || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_160)  || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_176)  || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_192)  || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_208)  || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_224)  || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_240)  || \
                         ((__PULSEWIDTH__) == HRTIM_CHOPPER_PULSEWIDTH_256))

/**
  * @brief  Check if the parameter __SYNCINPUTSOURCE__ is valid
  * @param  __SYNCINPUTSOURCE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_SYNCINPUTSOURCE(__SYNCINPUTSOURCE__)\
              (((__SYNCINPUTSOURCE__) == HRTIM_SYNCINPUTSOURCE_NONE)             || \
               ((__SYNCINPUTSOURCE__) == HRTIM_SYNCINPUTSOURCE_INTERNALEVENT)    || \
               ((__SYNCINPUTSOURCE__) == HRTIM_SYNCINPUTSOURCE_EXTERNALEVENT))

/**
  * @brief  Check if the parameter __SYNCOUTPUTSOURCE__ is valid
  * @param  __SYNCOUTPUTSOURCE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_SYNCOUTPUTSOURCE(__SYNCOUTPUTSOURCE__)\
              (((__SYNCOUTPUTSOURCE__) == HRTIM_SYNCOUTPUTSOURCE_MASTER_START)  || \
               ((__SYNCOUTPUTSOURCE__) == HRTIM_SYNCOUTPUTSOURCE_MASTER_CMP1)   || \
               ((__SYNCOUTPUTSOURCE__) == HRTIM_SYNCOUTPUTSOURCE_TIMA_START)    || \
               ((__SYNCOUTPUTSOURCE__) == HRTIM_SYNCOUTPUTSOURCE_TIMA_CMP1))

/**
  * @brief  Check if the parameter __SYNCOUTPUTPOLARITY__ is valid
  * @param  __SYNCOUTPUTPOLARITY__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_SYNCOUTPUTPOLARITY(__SYNCOUTPUTPOLARITY__)\
              (((__SYNCOUTPUTPOLARITY__) == HRTIM_SYNCOUTPUTPOLARITY_NONE)  || \
               ((__SYNCOUTPUTPOLARITY__) == HRTIM_SYNCOUTPUTPOLARITY_POSITIVE)  || \
               ((__SYNCOUTPUTPOLARITY__) == HRTIM_SYNCOUTPUTPOLARITY_NEGATIVE))

/**
  * @brief  Check if the parameter __EVENTSENSITIVITY__, __EVENTPOLARITY__ is valid
  * @param  __EVENTSENSITIVITY__, __EVENTPOLARITY__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_EVENTPOLARITY(__EVENTSENSITIVITY__, __EVENTPOLARITY__)\
    ((((__EVENTSENSITIVITY__) == HRTIM_EVENTSENSITIVITY_LEVEL)  &&      \
       (((__EVENTPOLARITY__) == HRTIM_EVENTPOLARITY_HIGH)  ||           \
        ((__EVENTPOLARITY__) == HRTIM_EVENTPOLARITY_LOW)))              \
      ||                                                            \
      (((__EVENTSENSITIVITY__) == HRTIM_EVENTSENSITIVITY_RISINGEDGE) || \
       ((__EVENTSENSITIVITY__) == HRTIM_EVENTSENSITIVITY_FALLINGEDGE)|| \
       ((__EVENTSENSITIVITY__) == HRTIM_EVENTSENSITIVITY_BOTHEDGES)))

/**
  * @brief  Check if the parameter __EVENTSENSITIVITY__ is valid
  * @param  __EVENTSENSITIVITY__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_EVENTSENSITIVITY(__EVENTSENSITIVITY__)\
                    (((__EVENTSENSITIVITY__) == HRTIM_EVENTSENSITIVITY_LEVEL)       || \
                     ((__EVENTSENSITIVITY__) == HRTIM_EVENTSENSITIVITY_RISINGEDGE)  || \
                     ((__EVENTSENSITIVITY__) == HRTIM_EVENTSENSITIVITY_FALLINGEDGE) || \
                     ((__EVENTSENSITIVITY__) == HRTIM_EVENTSENSITIVITY_BOTHEDGES))

/**
  * @brief  Check if the parameter __EVENT__, __FILTER__ is valid
  * @param  __EVENT__, __FILTER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_EVENTFILTER(__EVENT__, __FILTER__)  \
      ((((__EVENT__) == HRTIM_EVENT_1) ||            \
        ((__EVENT__) == HRTIM_EVENT_2) ||            \
        ((__EVENT__) == HRTIM_EVENT_3) ||            \
        ((__EVENT__) == HRTIM_EVENT_4) ||            \
        ((__EVENT__) == HRTIM_EVENT_5))              \
      ||                                             \
      ((((__EVENT__) == HRTIM_EVENT_6) ||            \
        ((__EVENT__) == HRTIM_EVENT_7) ||            \
        ((__EVENT__) == HRTIM_EVENT_8) ||            \
        ((__EVENT__) == HRTIM_EVENT_9) ||            \
        ((__EVENT__) == HRTIM_EVENT_10)) &&          \
        (((__FILTER__) == HRTIM_EVENTFILTER_NONE) || \
        ((__FILTER__) == HRTIM_EVENTFILTER_1)     || \
        ((__FILTER__) == HRTIM_EVENTFILTER_2)     || \
        ((__FILTER__) == HRTIM_EVENTFILTER_3)     || \
        ((__FILTER__) == HRTIM_EVENTFILTER_4)     || \
        ((__FILTER__) == HRTIM_EVENTFILTER_5)     || \
        ((__FILTER__) == HRTIM_EVENTFILTER_6)     || \
        ((__FILTER__) == HRTIM_EVENTFILTER_7)     || \
        ((__FILTER__) == HRTIM_EVENTFILTER_8)     || \
        ((__FILTER__) == HRTIM_EVENTFILTER_9)     || \
        ((__FILTER__) == HRTIM_EVENTFILTER_10)    || \
        ((__FILTER__) == HRTIM_EVENTFILTER_11)    || \
        ((__FILTER__) == HRTIM_EVENTFILTER_12)    || \
        ((__FILTER__) == HRTIM_EVENTFILTER_13)    || \
        ((__FILTER__) == HRTIM_EVENTFILTER_14)    || \
        ((__FILTER__) == HRTIM_EVENTFILTER_15))))

/**
  * @brief  Check if the parameter __EVENTPRESCALER__ is valid
  * @param  __EVENTPRESCALER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_EVENTPRESCALER(__EVENTPRESCALER__)\
             (((__EVENTPRESCALER__) == HRTIM_EVENTPRESCALER_DIV1)   || \
              ((__EVENTPRESCALER__) == HRTIM_EVENTPRESCALER_DIV2)   || \
              ((__EVENTPRESCALER__) == HRTIM_EVENTPRESCALER_DIV4)   || \
              ((__EVENTPRESCALER__) == HRTIM_EVENTPRESCALER_DIV8))

/**
  * @brief  Check if the parameter __FAULTSOURCE__ is valid
  * @param  __FAULTSOURCE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_FAULTSOURCE(__FAULTSOURCE__)\
              (((__FAULTSOURCE__) == HRTIM_FAULTSOURCE_DIGITALINPUT) || \
               ((__FAULTSOURCE__) == HRTIM_FAULTSOURCE_INTERNAL)     || \
               ((__FAULTSOURCE__) == HRTIM_FAULTSOURCE_EEVINPUT))

/**
  * @brief  Check if the parameter __HRTIM_FAULTPOLARITY__ is valid
  * @param  __HRTIM_FAULTPOLARITY__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_FAULTPOLARITY(__HRTIM_FAULTPOLARITY__)\
              (((__HRTIM_FAULTPOLARITY__) == HRTIM_FAULTPOLARITY_LOW) || \
               ((__HRTIM_FAULTPOLARITY__) == HRTIM_FAULTPOLARITY_HIGH))

/**
  * @brief  Check if the parameter __FAULTMODECTL__ is valid
  * @param  __FAULTMODECTL__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_FAULTMODECTL(__FAULTMODECTL__)\
    (((__FAULTMODECTL__) == HRTIM_FAULTMODECTL_DISABLED)  || \
     ((__FAULTMODECTL__) == HRTIM_FAULTMODECTL_ENABLED))

/**
  * @brief  Check if the parameter __FAULTBLANKINGMODE__ is valid
  * @param  __FAULTBLANKINGMODE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_FAULTBLANKNGMODE(__FAULTBLANKINGMODE__)\
              (((__FAULTBLANKINGMODE__) == HRTIM_FAULTBLANKINGMODE_RSTALIGNED) || \
               ((__FAULTBLANKINGMODE__) == HRTIM_FAULTBLANKINGMODE_MOVING))

/**
  * @brief  Check if the parameter __HRTIM_FAULTCOUNTERRST__ is valid
  * @param  __HRTIM_FAULTCOUNTERRST__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_FAULTCOUNTERRST(__HRTIM_FAULTCOUNTERRST__)\
              (((__HRTIM_FAULTCOUNTERRST__) == HRTIM_FAULTCOUNTERRST_UNCONDITIONAL) || \
               ((__HRTIM_FAULTCOUNTERRST__) == HRTIM_FAULTCOUNTERRST_CONDITIONAL))

/**
  * @brief  Check if the parameter __FAULTBLANKINGCTL__ is valid
  * @param  __FAULTBLANKINGCTL__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_FAULTBLANKINGCTL(__FAULTBLANKINGCTL__)\
    (((__FAULTBLANKINGCTL__) == HRTIM_FAULTBLANKINGCTL_DISABLED)  || \
     ((__FAULTBLANKINGCTL__) == HRTIM_FAULTBLANKINGCTL_ENABLED))

/**
  * @brief  Check if the parameter __FAULTCOUNTER__ is valid
  * @param  __FAULTCOUNTER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_FAULTCOUNTER(__FAULTCOUNTER__)\
                (((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_NONE) || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_1)    || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_2)    || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_3)    || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_4)    || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_5)    || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_6)    || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_7)    || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_8)    || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_9)    || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_10)   || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_11)   || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_12)   || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_13)   || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_14)   || \
                 ((__FAULTCOUNTER__) == HRTIM_FAULTCOUNTER_15))

/**
  * @brief  Check if the parameter __FAULTFILTER__ is valid
  * @param  __FAULTFILTER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_FAULTFILTER(__FAULTFILTER__)\
                (((__FAULTFILTER__) == HRTIM_FAULTFILTER_NONE) || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_1)    || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_2)    || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_3)    || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_4)    || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_5)    || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_6)    || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_7)    || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_8)    || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_9)    || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_10)   || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_11)   || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_12)   || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_13)   || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_14)   || \
                 ((__FAULTFILTER__) == HRTIM_FAULTFILTER_15))

/**
  * @brief  Check if the parameter __FAULTLOCK__ is valid
  * @param  __FAULTLOCK__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_FAULTLOCK(__FAULTLOCK__)\
              (((__FAULTLOCK__) == HRTIM_FAULTLOCK_READWRITE) || \
               ((__FAULTLOCK__) == HRTIM_FAULTLOCK_READONLY))

/**
  * @brief  Check if the parameter __FAULTPRESCALER__ is valid
  * @param  __FAULTPRESCALER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_FAULTPRESCALER(__FAULTPRESCALER__)\
             (((__FAULTPRESCALER__) == HRTIM_FAULTPRESCALER_DIV1)   || \
              ((__FAULTPRESCALER__) == HRTIM_FAULTPRESCALER_DIV2)   || \
              ((__FAULTPRESCALER__) == HRTIM_FAULTPRESCALER_DIV4)   || \
              ((__FAULTPRESCALER__) == HRTIM_FAULTPRESCALER_DIV8))

/**
  * @brief  Check if the parameter __BURSTMODE__ is valid
  * @param  __BURSTMODE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_BURSTMODE(__BURSTMODE__)\
              (((__BURSTMODE__) == HRTIM_BURSTMODE_SINGLESHOT)  || \
               ((__BURSTMODE__) == HRTIM_BURSTMODE_CONTINOUS))

/**
  * @brief  Check if the parameter __BURSTMODECLOCKSOURCE__ is valid
  * @param  __BURSTMODECLOCKSOURCE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_BURSTMODECLOCKSOURCE(__BURSTMODECLOCKSOURCE__)\
              (((__BURSTMODECLOCKSOURCE__) == HRTIM_BURSTMODECLOCKSOURCE_MASTER)      || \
               ((__BURSTMODECLOCKSOURCE__) == HRTIM_BURSTMODECLOCKSOURCE_TIMER_A)     || \
               ((__BURSTMODECLOCKSOURCE__) == HRTIM_BURSTMODECLOCKSOURCE_TIMER_B)     || \
               ((__BURSTMODECLOCKSOURCE__) == HRTIM_BURSTMODECLOCKSOURCE_TIMER_C)     || \
               ((__BURSTMODECLOCKSOURCE__) == HRTIM_BURSTMODECLOCKSOURCE_TIMER_D)     || \
               ((__BURSTMODECLOCKSOURCE__) == HRTIM_BURSTMODECLOCKSOURCE_TIMER_E)     || \
               ((__BURSTMODECLOCKSOURCE__) == HRTIM_BURSTMODECLOCKSOURCE_TIMER_F)     || \
               ((__BURSTMODECLOCKSOURCE__) == HRTIM_BURSTMODECLOCKSOURCE_TIM16_OC)    || \
               ((__BURSTMODECLOCKSOURCE__) == HRTIM_BURSTMODECLOCKSOURCE_TIM17_OC)    || \
               ((__BURSTMODECLOCKSOURCE__) == HRTIM_BURSTMODECLOCKSOURCE_TIM7_TRGO)   || \
               ((__BURSTMODECLOCKSOURCE__) == HRTIM_BURSTMODECLOCKSOURCE_FHRTIM))

/**
  * @brief  Check if the parameter __BURSTMODEPRESCALER__ is valid
  * @param  __BURSTMODEPRESCALER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_HRTIM_BURSTMODEPRESCALER(__BURSTMODEPRESCALER__)\
              (((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV1)     || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV2)     || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV4)     || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV8)     || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV16)    || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV32)    || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV64)    || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV128)   || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV256)   || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV512)   || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV1024)  || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV2048)  || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV4096)  || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV8192)  || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV16384) || \
               ((__BURSTMODEPRESCALER__) == HRTIM_BURSTMODEPRESCALER_DIV32768))

/**
  * @brief  Check if the parameter __BURSTMODEPRELOAD__ is valid
  * @param  __BURSTMODEPRELOAD__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_BURSTMODEPRELOAD(__BURSTMODEPRELOAD__)\
              (((__BURSTMODEPRELOAD__) == HRTIM_BURSTMODEPRELOAD_DISABLED)  || \
               ((__BURSTMODEPRELOAD__) == HRTIM_BURSTMODEPRELOAD_ENABLED))

/**
  * @brief  Check if the parameter __BURSTMODETRIGGER__ is valid
  * @param  __BURSTMODETRIGGER__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_BURSTMODETRIGGER(__BURSTMODETRIGGER__)                                 \
              (((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_NONE)               || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_MASTER_RESET)       || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_MASTER_REPETITION)  || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_MASTER_CMP1)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_MASTER_CMP2)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_MASTER_CMP3)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_MASTER_CMP4)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERA_RESET)       || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERA_REPETITION)  || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERA_CMP1)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERA_CMP2)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERB_RESET)       || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERB_REPETITION)  || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERB_CMP1)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERB_CMP2)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERC_RESET)       || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERC_REPETITION)  || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERC_CMP1)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERF_RESET)       || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERD_RESET)       || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERD_REPETITION)  || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERF_REPETITION)  || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERD_CMP2)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERF_CMP1)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERE_REPETITION)  || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERE_CMP1)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERE_CMP2)        || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERA_EVENT7)      || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_TIMERD_EVENT8)      || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_EVENT_7)            || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_EVENT_8)            || \
               ((__BURSTMODETRIGGER__) == HRTIM_BURSTMODETRIGGER_EVENT_ONCHIP))

/**
  * @brief  Check if the parameter __ADCTRIGGERUPDATE__ is valid
  * @param  __ADCTRIGGERUPDATE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_ADCTRIGGERUPDATE(__ADCTRIGGERUPDATE__)\
             (((__ADCTRIGGERUPDATE__) == HRTIM_ADCTRIGGERUPDATE_MASTER)   || \
              ((__ADCTRIGGERUPDATE__) == HRTIM_ADCTRIGGERUPDATE_TIMER_A)  || \
              ((__ADCTRIGGERUPDATE__) == HRTIM_ADCTRIGGERUPDATE_TIMER_B)  || \
              ((__ADCTRIGGERUPDATE__) == HRTIM_ADCTRIGGERUPDATE_TIMER_C)  || \
              ((__ADCTRIGGERUPDATE__) == HRTIM_ADCTRIGGERUPDATE_TIMER_D)  || \
              ((__ADCTRIGGERUPDATE__) == HRTIM_ADCTRIGGERUPDATE_TIMER_E)  || \
              ((__ADCTRIGGERUPDATE__) == HRTIM_ADCTRIGGERUPDATE_TIMER_F))

/**
  * @brief  Check if the parameter __CALIBRATIONRATE__ is valid
  * @param  __CALIBRATIONRATE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_HRTIM_CALIBRATIONRATE(CALIBRATIONRATE)\
    (((CALIBRATIONRATE) == HRTIM_SINGLE_CALIBRATION) || \
     ((CALIBRATIONRATE) == HRTIM_CALIBRATIONRATE_0)  || \
     ((CALIBRATIONRATE) == HRTIM_CALIBRATIONRATE_1)  || \
     ((CALIBRATIONRATE) == HRTIM_CALIBRATIONRATE_2)  || \
     ((CALIBRATIONRATE) == HRTIM_CALIBRATIONRATE_3))

/**
  * @brief  Check if the parameter __TIMER__, __BURSTDMA__ is valid
  * @param  __TIMER__, __BURSTDMA__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMER_BURSTDMA(__TIMER__, __BURSTDMA__)                                            \
    ((((__TIMER__) == HRTIM_TIMERINDEX_MASTER)  && (((__BURSTDMA__) & 0xFFFFC000U) == 0x00000000U)) \
  || (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_A) && (((__BURSTDMA__) & 0xFF800000U) == 0x00000000U)) \
  || (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_B) && (((__BURSTDMA__) & 0xFF800000U) == 0x00000000U)) \
  || (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_C) && (((__BURSTDMA__) & 0xFF800000U) == 0x00000000U)) \
  || (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_D) && (((__BURSTDMA__) & 0xFF800000U) == 0x00000000U)) \
  || (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_E) && (((__BURSTDMA__) & 0xFF800000U) == 0x00000000U)) \
  || (((__TIMER__) == HRTIM_TIMERINDEX_TIMER_F) && (((__BURSTDMA__) & 0xFF800000U) == 0x00000000U)))

/**
  * @brief  Check if the parameter __BURSTMODECTL__ is valid
  * @param  __BURSTMODECTL__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_BURSTMODECTL(__BURSTMODECTL__)\
    (((__BURSTMODECTL__) == HRTIM_BURSTMODECTL_DISABLED)  || \
     ((__BURSTMODECTL__) == HRTIM_BURSTMODECTL_ENABLED))

/**
  * @brief  Check if the parameter __TIMERUPDATE__ is valid
  * @param  __TIMERUPDATE__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMERUPDATE(__TIMERUPDATE__)    (((__TIMERUPDATE__) & 0xFFFFFF80U) == 0x00000000U)

/**
  * @brief  Check if the parameter __TIMERRESET__ is valid
  * @param  __TIMERRESET__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMERRESET(__TIMERRESET__)    (((__TIMERRESET__) & 0xFFFF80FFU) == 0x00000000U)

/**
  * @brief  Check if the parameter __TIMERSWAP__ is valid
  * @param  __TIMERSWAP__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIMERSWAP(__TIMERSWAP__)    (((__TIMERSWAP__) & 0xFFC0FFFFU) == 0x00000000U)

/**
  * @brief  Check if the parameter __IT__ is valid
  * @param  __IT__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_IT(__IT__)     (((__IT__) & 0xFFFCFF80U) == 0x00000000U)

/**
  * @brief  Check if the parameter __MASTER_IT__ is valid
  * @param  __MASTER_IT__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_MASTER_IT(__MASTER_IT__)    (((__MASTER_IT__) & 0xFFFFFF80U) == 0x00000000U)


/**
  * @brief  Check if the parameter __TIM_IT__ is valid
  * @param  __TIM_IT__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIM_IT(__TIM_IT__)    (((__TIM_IT__) & 0xFFFF8020U) == 0x00000000U)


/**
  * @brief  Check if the parameter __MASTER_DMA__ is valid
  * @param  __MASTER_DMA__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_MASTER_DMA(__MASTER_DMA__)      (((__MASTER_DMA__) & 0xFF80FFFFU) == 0x00000000U)

/**
  * @brief  Check if the parameter __TIM_DMA__ is valid
  * @param  __TIM_DMA__
  * @retval Status
  *         @arg SET (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_HRTIM_TIM_DMA(__TIM_DMA__)     (((__TIM_DMA__) & 0x8020FFFFU) == 0x00000000U)


/* Exported macros -----------------------------------------------------------*/

/**
  * @brief  configures the actual direction of the counter to UP counting mode
  * @param   __HANDLE__ : HRTIM handle.
  * @param   __TIMER__  : Timer index
  *         This parameter can be a combination of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval none
  */
#define __HAL_HRTIM_COUNTER_MODE_UP(__HANDLE__, __TIMERS__)                                                              \
  do {                                                                                                                   \
         if (((__TIMERS__) & HRTIM_TIMERINDEX_TIMER_A) == HRTIM_TIMERINDEX_TIMER_A)                                      \
           {                                                                                                             \
               CLEAR_BIT((__HANDLE__)->Instance->sTimerxRegs[(HRTIM_TIMERINDEX_TIMER_A)].TIMxCR2 , (HRTIM_TIMCR2_UDM));  \
           }                                                                                                             \
         if (((__TIMERS__) & HRTIM_TIMERINDEX_TIMER_B) == HRTIM_TIMERINDEX_TIMER_B)                                      \
           {                                                                                                             \
               CLEAR_BIT((__HANDLE__)->Instance->sTimerxRegs[(HRTIM_TIMERINDEX_TIMER_B)].TIMxCR2 , (HRTIM_TIMCR2_UDM));  \
           }                                                                                                             \
         if (((__TIMERS__) & HRTIM_TIMERINDEX_TIMER_C) == HRTIM_TIMERINDEX_TIMER_C)                                      \
           {                                                                                                             \
                CLEAR_BIT((__HANDLE__)->Instance->sTimerxRegs[(HRTIM_TIMERINDEX_TIMER_C)].TIMxCR2 , (HRTIM_TIMCR2_UDM)); \
           }                                                                                                             \
         if (((__TIMERS__) & HRTIM_TIMERINDEX_TIMER_D) == HRTIM_TIMERINDEX_TIMER_D)                                      \
           {                                                                                                             \
               CLEAR_BIT((__HANDLE__)->Instance->sTimerxRegs[(HRTIM_TIMERINDEX_TIMER_D)].TIMxCR2 , (HRTIM_TIMCR2_UDM));  \
           }                                                                                                             \
         if (((__TIMERS__) & HRTIM_TIMERINDEX_TIMER_E) == HRTIM_TIMERINDEX_TIMER_E)                                      \
           {                                                                                                             \
               CLEAR_BIT((__HANDLE__)->Instance->sTimerxRegs[(HRTIM_TIMERINDEX_TIMER_E)].TIMxCR2 , (HRTIM_TIMCR2_UDM));  \
          }                                                                                                              \
         if (((__TIMERS__) & HRTIM_TIMERINDEX_TIMER_F) == HRTIM_TIMERINDEX_TIMER_F)                                      \
           {                                                                                                             \
               CLEAR_BIT((__HANDLE__)->Instance->sTimerxRegs[(HRTIM_TIMERINDEX_TIMER_F)].TIMxCR2 , (HRTIM_TIMCR2_UDM));  \
           }                                                                                                             \
     } while(0U)

/**
  * @brief  configures the actual direction of the counter to UP-DOWN counting mode
  * @param   __HANDLE__ : HRTIM handle.
  * @param   __TIMER__  : Timer index
  *         This parameter can be a combination of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval none
  */
#define __HAL_HRTIM_COUNTER_MODE_UPDOWN(__HANDLE__, __TIMERS__)                                                      \
  do {                                                                                                               \
         if (((__TIMERS__) & HRTIM_TIMERINDEX_TIMER_A) == HRTIM_TIMERINDEX_TIMER_A)                                  \
           {                                                                                                         \
               SET_BIT((__HANDLE__)->Instance->sTimerxRegs[(HRTIM_TIMERINDEX_TIMER_A)].TIMxCR2 , (HRTIM_TIMCR2_UDM));  \
           }                                                                                                         \
         if (((__TIMERS__) & HRTIM_TIMERINDEX_TIMER_B) == HRTIM_TIMERINDEX_TIMER_B)                                  \
           {                                                                                                         \
               SET_BIT((__HANDLE__)->Instance->sTimerxRegs[(HRTIM_TIMERINDEX_TIMER_B)].TIMxCR2 , (HRTIM_TIMCR2_UDM));  \
           }                                                                                                         \
         if (((__TIMERS__) & HRTIM_TIMERINDEX_TIMER_C) == HRTIM_TIMERINDEX_TIMER_C)                                  \
           {                                                                                                         \
               SET_BIT((__HANDLE__)->Instance->sTimerxRegs[(HRTIM_TIMERINDEX_TIMER_C)].TIMxCR2 , (HRTIM_TIMCR2_UDM)); \
           }                                                                                                         \
         if (((__TIMERS__) & HRTIM_TIMERINDEX_TIMER_D) == HRTIM_TIMERINDEX_TIMER_D)                                  \
           {                                                                                                         \
               SET_BIT((__HANDLE__)->Instance->sTimerxRegs[(HRTIM_TIMERINDEX_TIMER_D)].TIMxCR2 , (HRTIM_TIMCR2_UDM));  \
           }                                                                                                         \
         if (((__TIMERS__) & HRTIM_TIMERINDEX_TIMER_E) == HRTIM_TIMERINDEX_TIMER_E)                                  \
           {                                                                                                         \
               SET_BIT((__HANDLE__)->Instance->sTimerxRegs[(HRTIM_TIMERINDEX_TIMER_E)].TIMxCR2 , (HRTIM_TIMCR2_UDM));  \
          }                                                                                                          \
         if (((__TIMERS__) & HRTIM_TIMERINDEX_TIMER_F) == HRTIM_TIMERINDEX_TIMER_F)                                  \
           {                                                                                                         \
               SET_BIT((__HANDLE__)->Instance->sTimerxRegs[(HRTIM_TIMERINDEX_TIMER_F)].TIMxCR2 , (HRTIM_TIMCR2_UDM));  \
           }                                                                                                         \
     } while(0U)

 /**
  * @brief  swap the output of the timer
  *         HRTIM_SETA1R and HRTIM_RSTA1R are coding for the output A2,
  *         HRTIM_SETA2R and HRTIM_RSTA2R are coding for the output A1
  * @param   __HANDLE__ : HRTIM handle.
  * @param   __TIMER__  : Timer index
  *         This parameter can be a combination of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval none
  */
#define __HAL_HRTIM_TIMER_OUTPUT_SWAP(__HANDLE__, __TIMERS__)                       \
  do {                                                                              \
         if (((__TIMERS__) & HRTIM_TIMERID_TIMER_A) == HRTIM_TIMERID_TIMER_A)       \
             {                                                                      \
             SET_BIT((__HANDLE__)->Instance->sCommonRegs.CR2 , (HRTIM_CR2_SWPA));   \
             }                                                                      \
         if (((__TIMERS__) & HRTIM_TIMERID_TIMER_B) == HRTIM_TIMERID_TIMER_B)       \
             {                                                                      \
             SET_BIT((__HANDLE__)->Instance->sCommonRegs.CR2 , (HRTIM_CR2_SWPB));   \
             }                                                                      \
         if (((__TIMERS__) & HRTIM_TIMERID_TIMER_C) == HRTIM_TIMERID_TIMER_C)       \
             {                                                                      \
             SET_BIT((__HANDLE__)->Instance->sCommonRegs.CR2 , (HRTIM_CR2_SWPC));   \
             }                                                                      \
         if (((__TIMERS__) & HRTIM_TIMERID_TIMER_D) == HRTIM_TIMERID_TIMER_D)       \
             {                                                                      \
             SET_BIT((__HANDLE__)->Instance->sCommonRegs.CR2 , (HRTIM_CR2_SWPD));   \
             }                                                                      \
         if (((__TIMERS__) & HRTIM_TIMERID_TIMER_E) == HRTIM_TIMERID_TIMER_E)       \
             {                                                                      \
             SET_BIT((__HANDLE__)->Instance->sCommonRegs.CR2 , (HRTIM_CR2_SWPE));   \
         }                                                                          \
         if (((__TIMERS__) & HRTIM_TIMERID_TIMER_F) == HRTIM_TIMERID_TIMER_F)       \
             {                                                                      \
             SET_BIT((__HANDLE__)->Instance->sCommonRegs.CR2 , (HRTIM_CR2_SWPF));   \
             }                                                                      \
     } while(0U)

/**
  * @brief  Un-swap the output of the timer
  *         HRTIM_SETA1R and HRTIM_RSTA1R are coding for the output A1,
  *         HRTIM_SETA2R and HRTIM_RSTA2R are coding for the output A2
  * @param   __HANDLE__ : HRTIM handle.
  * @param   __TIMER__  : Timer index
  *         This parameter can be a combination of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval none

  */
#define __HAL_HRTIM_TIMER_OUTPUT_NOSWAP(__HANDLE__, __TIMERS__)                       \
  do {                                                                                \
         if (((__TIMERS__) & HRTIM_TIMERID_TIMER_A) == HRTIM_TIMERID_TIMER_A)         \
           {                                                                          \
               CLEAR_BIT((__HANDLE__)->Instance->sCommonRegs.CR2 , (HRTIM_CR2_SWPA)); \
           }                                                                          \
         if (((__TIMERS__) & HRTIM_TIMERID_TIMER_B) == HRTIM_TIMERID_TIMER_B)         \
           {                                                                          \
               CLEAR_BIT((__HANDLE__)->Instance->sCommonRegs.CR2 , (HRTIM_CR2_SWPB)); \
           }                                                                          \
         if (((__TIMERS__) & HRTIM_TIMERID_TIMER_C) == HRTIM_TIMERID_TIMER_C)         \
           {                                                                          \
               CLEAR_BIT((__HANDLE__)->Instance->sCommonRegs.CR2 , (HRTIM_CR2_SWPC)); \
           }                                                                          \
         if (((__TIMERS__) & HRTIM_TIMERID_TIMER_D) == HRTIM_TIMERID_TIMER_D)         \
           {                                                                          \
               CLEAR_BIT((__HANDLE__)->Instance->sCommonRegs.CR2 , (HRTIM_CR2_SWPD)); \
           }                                                                          \
         if (((__TIMERS__) & HRTIM_TIMERID_TIMER_E) == HRTIM_TIMERID_TIMER_E)         \
           {                                                                          \
               CLEAR_BIT((__HANDLE__)->Instance->sCommonRegs.CR2 , (HRTIM_CR2_SWPE)); \
          }                                                                           \
         if (((__TIMERS__) & HRTIM_TIMERID_TIMER_F) == HRTIM_TIMERID_TIMER_F)         \
           {                                                                          \
               CLEAR_BIT((__HANDLE__)->Instance->sCommonRegs.CR2 , (HRTIM_CR2_SWPF)); \
           }                                                                          \
  } while(0U)

/** @brief Reset HRTIM handle state
  * @param  __HANDLE__ HRTIM handle.
  * @retval None
  */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
#define __HAL_HRTIM_RESET_HANDLE_STATE(__HANDLE__)     do{                                                            \
                                                             (__HANDLE__)->State             = HAL_HRTIM_STATE_RESET; \
                                                             (__HANDLE__)->MspInitCallback   = NULL;                  \
                                                             (__HANDLE__)->MspDeInitCallback = NULL;                  \
                                                       } while(0)
#else
#define __HAL_HRTIM_RESET_HANDLE_STATE(__HANDLE__)     ((__HANDLE__)->State = HAL_HRTIM_STATE_RESET)
#endif

/** @brief  Enables or disables the timer counter(s)
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __TIMERS__ timers to enable/disable
  *        This parameter can be any combinations of the following values:
  *            @arg HRTIM_TIMERID_MASTER: Master timer identifier
  *            @arg HRTIM_TIMERID_TIMER_A: Timer A identifier
  *            @arg HRTIM_TIMERID_TIMER_B: Timer B identifier
  *            @arg HRTIM_TIMERID_TIMER_C: Timer C identifier
  *            @arg HRTIM_TIMERID_TIMER_D: Timer D identifier
  *            @arg HRTIM_TIMERID_TIMER_E: Timer E identifier
  *            @arg HRTIM_TIMERID_TIMER_F: Timer F identifier
  * @retval None
  */
#define __HAL_HRTIM_ENABLE(__HANDLE__, __TIMERS__)   ((__HANDLE__)->Instance->sMasterRegs.MCR |= (__TIMERS__))

/* The counter of a timing unit is disabled only if all the timer outputs */
/* are disabled and no capture is configured                              */
#define HRTIM_TAOEN_MASK (HRTIM_OENR_TA2OEN | HRTIM_OENR_TA1OEN)
#define HRTIM_TBOEN_MASK (HRTIM_OENR_TB2OEN | HRTIM_OENR_TB1OEN)
#define HRTIM_TCOEN_MASK (HRTIM_OENR_TC2OEN | HRTIM_OENR_TC1OEN)
#define HRTIM_TDOEN_MASK (HRTIM_OENR_TD2OEN | HRTIM_OENR_TD1OEN)
#define HRTIM_TEOEN_MASK (HRTIM_OENR_TE2OEN | HRTIM_OENR_TE1OEN)
#define HRTIM_TFOEN_MASK (HRTIM_OENR_TF2OEN | HRTIM_OENR_TF1OEN)

#define __HAL_HRTIM_DISABLE(__HANDLE__, __TIMERS__)                                              \
  do {                                                                                           \
    if (((__TIMERS__) & HRTIM_TIMERID_MASTER) == HRTIM_TIMERID_MASTER)                           \
      {                                                                                          \
        ((__HANDLE__)->Instance->sMasterRegs.MCR &= ~HRTIM_TIMERID_MASTER);                      \
      }                                                                                          \
    if (((__TIMERS__) & HRTIM_TIMERID_TIMER_A) == HRTIM_TIMERID_TIMER_A)                         \
      {                                                                                          \
        if (((__HANDLE__)->Instance->sCommonRegs.OENR & HRTIM_TAOEN_MASK) == (uint32_t)RESET)    \
          {                                                                                      \
            ((__HANDLE__)->Instance->sMasterRegs.MCR &= ~HRTIM_TIMERID_TIMER_A);                 \
          }                                                                                      \
      }                                                                                          \
    if (((__TIMERS__) & HRTIM_TIMERID_TIMER_B) == HRTIM_TIMERID_TIMER_B)                         \
      {                                                                                          \
        if (((__HANDLE__)->Instance->sCommonRegs.OENR & HRTIM_TBOEN_MASK) == (uint32_t)RESET)    \
          {                                                                                      \
            ((__HANDLE__)->Instance->sMasterRegs.MCR &= ~HRTIM_TIMERID_TIMER_B);                 \
          }                                                                                      \
      }                                                                                          \
    if (((__TIMERS__) & HRTIM_TIMERID_TIMER_C) == HRTIM_TIMERID_TIMER_C)                         \
      {                                                                                          \
        if (((__HANDLE__)->Instance->sCommonRegs.OENR & HRTIM_TCOEN_MASK) == (uint32_t)RESET)    \
          {                                                                                      \
            ((__HANDLE__)->Instance->sMasterRegs.MCR &= ~HRTIM_TIMERID_TIMER_C);                 \
          }                                                                                      \
      }                                                                                          \
    if (((__TIMERS__) & HRTIM_TIMERID_TIMER_D) == HRTIM_TIMERID_TIMER_D)                         \
      {                                                                                          \
        if (((__HANDLE__)->Instance->sCommonRegs.OENR & HRTIM_TDOEN_MASK) == (uint32_t)RESET)    \
          {                                                                                      \
            ((__HANDLE__)->Instance->sMasterRegs.MCR &= ~HRTIM_TIMERID_TIMER_D);                 \
          }                                                                                      \
      }                                                                                          \
    if (((__TIMERS__) & HRTIM_TIMERID_TIMER_E) == HRTIM_TIMERID_TIMER_E)                         \
      {                                                                                          \
        if (((__HANDLE__)->Instance->sCommonRegs.OENR & HRTIM_TEOEN_MASK) == (uint32_t)RESET)    \
          {                                                                                      \
            ((__HANDLE__)->Instance->sMasterRegs.MCR &= ~HRTIM_TIMERID_TIMER_E);                 \
          }                                                                                      \
      }                                                                                          \
    if (((__TIMERS__) & HRTIM_TIMERID_TIMER_F) == HRTIM_TIMERID_TIMER_F)                         \
      {                                                                                          \
        if (((__HANDLE__)->Instance->sCommonRegs.OENR & HRTIM_TFOEN_MASK) == (uint32_t)RESET)    \
          {                                                                                      \
            ((__HANDLE__)->Instance->sMasterRegs.MCR &= ~HRTIM_TIMERID_TIMER_F);                 \
          }                                                                                      \
      }                                                                                          \
  } while(0U)

/** @brief  Enables the External Event counter
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __TIMERS__ timers to enable/disable
  *        This parameter can be one of the following values:
  *            @arg HRTIM_TIMERINDEX_TIMER_A: Timer A identifier
  *            @arg HRTIM_TIMERINDEX_TIMER_B: Timer B identifier
  *            @arg HRTIM_TIMERINDEX_TIMER_C: Timer C identifier
  *            @arg HRTIM_TIMERINDEX_TIMER_D: Timer D identifier
  *            @arg HRTIM_TIMERINDEX_TIMER_E: Timer E identifier
  *            @arg HRTIM_TIMERINDEX_TIMER_F: Timer F identifier
  * @param  Event external event Counter A for which timer event must be enabled
  *         This parameter can be one of the following values:
  *           @arg HRTIM_EVENTCOUNTER_A
  * @retval None
  */
#define __HAL_HRTIM_EXTERNAL_EVENT_COUNTER_ENABLE(__HANDLE__, __TIMER__, __EVENT__)                                  \
  do {                                                                                                               \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_A) == HRTIM_TIMERINDEX_TIMER_A)                                   \
         {                                                                                                           \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                         \
             {                                                                                                       \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].EEFxR3) |= HRTIM_EEFR3_EEVACE;     \
             }                                                                                                       \                                                                                                     \
         }                                                                                                           \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_B) == HRTIM_TIMERINDEX_TIMER_B)                                   \
         {                                                                                                           \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                         \
             {                                                                                                       \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].EEFxR3) |= HRTIM_EEFR3_EEVACE;     \
             }                                                                                                       \                                                                                                      \
         }                                                                                                           \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_C) == HRTIM_TIMERINDEX_TIMER_C)                                   \
         {                                                                                                           \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                         \
             {                                                                                                       \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].EEFxR3) |= HRTIM_EEFR3_EEVACE;     \
             }                                                                                                       \                                                                                                      \
         }                                                                                                           \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_D) == HRTIM_TIMERINDEX_TIMER_D)                                   \
         {                                                                                                           \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                         \
             {                                                                                                       \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].EEFxR3) |= HRTIM_EEFR3_EEVACE;     \
             }                                                                                                       \                                                                                                      \
         }                                                                                                           \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_E) == HRTIM_TIMERINDEX_TIMER_E)                                   \
         {                                                                                                           \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                         \
             {                                                                                                       \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_E].EEFxR3) |= HRTIM_EEFR3_EEVACE;     \
             }                                                                                                       \                                                                                                      \
         }                                                                                                           \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_F) == HRTIM_TIMERINDEX_TIMER_F)                                   \
         {                                                                                                           \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                         \
             {                                                                                                       \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_F].EEFxR3) |= HRTIM_EEFR3_EEVACE;     \
             }                                                                                                       \                                                                                                      \
         }                                                                                                           \
  } while(0U)

/** @brief  Disables the External Event counter
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __TIMERS__ timers to enable/disable
  *        This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A: Timer A identifier
  *           @arg HRTIM_TIMERINDEX_TIMER_B: Timer B identifier
  *           @arg HRTIM_TIMERINDEX_TIMER_C: Timer C identifier
  *           @arg HRTIM_TIMERINDEX_TIMER_D: Timer D identifier
  *           @arg HRTIM_TIMERINDEX_TIMER_E: Timer E identifier
  *           @arg HRTIM_TIMERINDEX_TIMER_F: Timer F identifier
  * @param  Event external event A for which timer event must be disabled
  *         This parameter can be one of the following values:
  *           @arg HRTIM_EVENTCOUNTER_A
  * @retval None
  */
#define __HAL_HRTIM_EXTERNAL_EVENT_COUNTER_DISABLE(__HANDLE__, __TIMER__, __EVENT__)                                 \
  do {                                                                                                               \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_A) == HRTIM_TIMERINDEX_TIMER_A)                                   \
         {                                                                                                           \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                         \
             {                                                                                                       \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].EEFxR3) &= ~HRTIM_EEFR3_EEVACE;    \
             }                                                                                                       \                                                                                                     \
         }                                                                                                           \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_B) == HRTIM_TIMERINDEX_TIMER_B)                                   \
         {                                                                                                           \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                         \
             {                                                                                                       \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].EEFxR3) &= ~HRTIM_EEFR3_EEVACE;    \
             }                                                                                                       \                                                                                                      \
         }                                                                                                           \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_C) == HRTIM_TIMERINDEX_TIMER_C)                                   \
         {                                                                                                           \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                         \
             {                                                                                                       \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].EEFxR3) &= ~HRTIM_EEFR3_EEVACE;    \
             }                                                                                                       \                                                                                                      \
         }                                                                                                           \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_D) == HRTIM_TIMERINDEX_TIMER_D)                                   \
         {                                                                                                           \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                         \
             {                                                                                                       \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].EEFxR3) &= ~HRTIM_EEFR3_EEVACE;    \
             }                                                                                                       \                                                                                                      \
         }                                                                                                           \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_E) == HRTIM_TIMERINDEX_TIMER_E)                                   \
         {                                                                                                           \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                         \
             {                                                                                                       \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_E].EEFxR3) &= ~HRTIM_EEFR3_EEVACE;    \
             }                                                                                                       \                                                                                                      \
         }                                                                                                           \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_F) == HRTIM_TIMERINDEX_TIMER_F)                                   \
         {                                                                                                           \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                         \
             {                                                                                                       \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_F].EEFxR3) &= ~HRTIM_EEFR3_EEVACE;    \
             }                                                                                                       \                                                                                                       \
         }                                                                                                           \
  } while(0U)

/** @brief  Resets the External Event counter
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __TIMERS__ timers to enable/disable
  *        This parameter can be one of the following values:
  *            @arg HRTIM_TIMERINDEX_TIMER_A: Timer A identifier
  *            @arg HRTIM_TIMERINDEX_TIMER_B: Timer B identifier
  *            @arg HRTIM_TIMERINDEX_TIMER_C: Timer C identifier
  *            @arg HRTIM_TIMERINDEX_TIMER_D: Timer D identifier
  *            @arg HRTIM_TIMERINDEX_TIMER_E: Timer E identifier
  *            @arg HRTIM_TIMERINDEX_TIMER_F: Timer F identifier
  * @param  Event external event A for which timer event must be reset
  *         This parameter can be one of the following values:
  *           @arg HRTIM_EVENTCOUNTER_A
  * @retval None
  */
#define __HAL_HRTIM_EXTERNAL_EVENT_COUNTER_RESET(__HANDLE__, __TIMER__, __EVENT__)                                    \
  do {                                                                                                                \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_A) == HRTIM_TIMERINDEX_TIMER_A)                                    \
         {                                                                                                            \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                          \
             {                                                                                                        \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].EEFxR3) |= HRTIM_EEFR3_EEVACRES;    \
             }                                                                                                        \                                                                                                       \
         }                                                                                                            \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_B) == HRTIM_TIMERINDEX_TIMER_B)                                    \
         {                                                                                                            \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                          \
             {                                                                                                        \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].EEFxR3) |= HRTIM_EEFR3_EEVACRES;    \
             }                                                                                                        \                                                                                                       \
         }                                                                                                            \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_C) == HRTIM_TIMERINDEX_TIMER_C)                                    \
         {                                                                                                            \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                          \
             {                                                                                                        \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].EEFxR3) |= HRTIM_EEFR3_EEVACRES;    \
             }                                                                                                        \                                                                                                       \
         }                                                                                                            \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_D) == HRTIM_TIMERINDEX_TIMER_D)                                    \
         {                                                                                                            \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                          \
             {                                                                                                        \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].EEFxR3) |= HRTIM_EEFR3_EEVACRES;    \
             }                                                                                                        \                                                                                                       \
         }                                                                                                            \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_E) == HRTIM_TIMERINDEX_TIMER_E)                                    \
         {                                                                                                            \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                          \
             {                                                                                                        \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_E].EEFxR3) |= HRTIM_EEFR3_EEVACRES;    \
             }                                                                                                        \                                                                                                      \
         }                                                                                                            \
         if (((__TIMER__) & HRTIM_TIMERINDEX_TIMER_F) == HRTIM_TIMERINDEX_TIMER_F)                                    \
         {                                                                                                            \
           if (((__EVENT__) & HRTIM_EVENTCOUNTER_A) == HRTIM_EVENTCOUNTER_A)                                          \
             {                                                                                                        \
                   ((__HANDLE__)->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_F].EEFxR3) |= HRTIM_EEFR3_EEVACRES;    \
             }                                                                                                        \                                                                                                       \
         }                                                                                                            \
  } while(0U)


/** @brief  Enables or disables the specified HRTIM common interrupts.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __INTERRUPT__ specifies the interrupt source to enable or disable.
  *        This parameter can be one of the following values:
  *            @arg HRTIM_IT_FLT1: Fault 1 interrupt enable
  *            @arg HRTIM_IT_FLT2: Fault 2 interrupt enable
  *            @arg HRTIM_IT_FLT3: Fault 3 interrupt enable
  *            @arg HRTIM_IT_FLT4: Fault 4 interrupt enable
  *            @arg HRTIM_IT_FLT5: Fault 5 interrupt enable
  *            @arg HRTIM_IT_FLT6: Fault 6 interrupt enable
  *            @arg HRTIM_IT_SYSFLT: System Fault interrupt enable
  *            @arg HRTIM_IT_DLLRDY: DLL ready interrupt enable
  *            @arg HRTIM_IT_BMPER: Burst mode period interrupt enable
  * @retval None
  */
#define __HAL_HRTIM_ENABLE_IT(__HANDLE__, __INTERRUPT__)   ((__HANDLE__)->Instance->sCommonRegs.IER |= (__INTERRUPT__))
#define __HAL_HRTIM_DISABLE_IT(__HANDLE__, __INTERRUPT__) ((__HANDLE__)->Instance->sCommonRegs.IER &= ~(__INTERRUPT__))

/** @brief  Enables or disables the specified HRTIM Master timer interrupts.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __INTERRUPT__ specifies the interrupt source to enable or disable.
  *        This parameter can be one of the following values:
  *            @arg HRTIM_MASTER_IT_MCMP1: Master compare 1 interrupt enable
  *            @arg HRTIM_MASTER_IT_MCMP2: Master compare 2 interrupt enable
  *            @arg HRTIM_MASTER_IT_MCMP3: Master compare 3 interrupt enable
  *            @arg HRTIM_MASTER_IT_MCMP4: Master compare 4 interrupt enable
  *            @arg HRTIM_MASTER_IT_MREP: Master Repetition interrupt enable
  *            @arg HRTIM_MASTER_IT_SYNC: Synchronization input interrupt enable
  *            @arg HRTIM_MASTER_IT_MUPD: Master update interrupt enable
  * @retval None
  */
#define __HAL_HRTIM_MASTER_ENABLE_IT(__HANDLE__, __INTERRUPT__)   ((__HANDLE__)->Instance->sMasterRegs.MDIER |= (__INTERRUPT__))
#define __HAL_HRTIM_MASTER_DISABLE_IT(__HANDLE__, __INTERRUPT__) ((__HANDLE__)->Instance->sMasterRegs.MDIER &= ~(__INTERRUPT__))

/** @brief  Enables or disables the specified HRTIM Timerx interrupts.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __TIMER__ specified the timing unit (Timer A to F)
  * @param  __INTERRUPT__ specifies the interrupt source to enable or disable.
  *        This parameter can be one of the following values:
  *            @arg HRTIM_TIM_IT_CMP1: Timer compare 1 interrupt enable
  *            @arg HRTIM_TIM_IT_CMP2: Timer compare 2 interrupt enable
  *            @arg HRTIM_TIM_IT_CMP3: Timer compare 3 interrupt enable
  *            @arg HRTIM_TIM_IT_CMP4: Timer compare 4 interrupt enable
  *            @arg HRTIM_TIM_IT_REP: Timer repetition interrupt enable
  *            @arg HRTIM_TIM_IT_UPD: Timer update interrupt enable
  *            @arg HRTIM_TIM_IT_CPT1: Timer capture 1 interrupt enable
  *            @arg HRTIM_TIM_IT_CPT2: Timer capture 2 interrupt enable
  *            @arg HRTIM_TIM_IT_SET1: Timer output 1 set interrupt enable
  *            @arg HRTIM_TIM_IT_RST1: Timer output 1 reset interrupt enable
  *            @arg HRTIM_TIM_IT_SET2: Timer output 2 set interrupt enable
  *            @arg HRTIM_TIM_IT_RST2: Timer output 2 reset interrupt enable
  *            @arg HRTIM_TIM_IT_RST: Timer reset interrupt enable
  *            @arg HRTIM_TIM_IT_DLYPRT: Timer delay protection interrupt enable
  * @retval None
  */
#define __HAL_HRTIM_TIMER_ENABLE_IT(__HANDLE__, __TIMER__, __INTERRUPT__)   ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].TIMxDIER |= (__INTERRUPT__))
#define __HAL_HRTIM_TIMER_DISABLE_IT(__HANDLE__, __TIMER__, __INTERRUPT__) ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].TIMxDIER &= ~(__INTERRUPT__))

/** @brief  Checks if the specified HRTIM common interrupt  source  is enabled or disabled.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __INTERRUPT__ specifies the interrupt source to check.
  *        This parameter can be one of the following values:
  *            @arg HRTIM_IT_FLT1: Fault 1 interrupt enable
  *            @arg HRTIM_IT_FLT2: Fault 2 interrupt enable
  *            @arg HRTIM_IT_FLT3: Fault 3 enable
  *            @arg HRTIM_IT_FLT4: Fault 4 enable
  *            @arg HRTIM_IT_FLT5: Fault 5 enable
  *            @arg HRTIM_IT_FLT6: Fault 6 enable
  *            @arg HRTIM_IT_SYSFLT: System Fault interrupt enable
  *            @arg HRTIM_IT_DLLRDY: DLL ready interrupt enable
  *            @arg HRTIM_IT_BMPER: Burst mode period interrupt enable
  * @retval The new state of __INTERRUPT__ (TRUE or FALSE).
  */
#define __HAL_HRTIM_GET_ITSTATUS(__HANDLE__, __INTERRUPT__)     ((((__HANDLE__)->Instance->sCommonRegs.IER & (__INTERRUPT__)) == (__INTERRUPT__)) ? SET : RESET)

/** @brief  Checks if the specified HRTIM Master interrupt source  is enabled or disabled.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __INTERRUPT__ specifies the interrupt source to check.
  *        This parameter can be one of the following values:
  *            @arg HRTIM_MASTER_IT_MCMP1: Master compare 1 interrupt enable
  *            @arg HRTIM_MASTER_IT_MCMP2: Master compare 2 interrupt enable
  *            @arg HRTIM_MASTER_IT_MCMP3: Master compare 3 interrupt enable
  *            @arg HRTIM_MASTER_IT_MCMP4: Master compare 4 interrupt enable
  *            @arg HRTIM_MASTER_IT_MREP: Master Repetition interrupt enable
  *            @arg HRTIM_MASTER_IT_SYNC: Synchronization input interrupt enable
  *            @arg HRTIM_MASTER_IT_MUPD: Master update interrupt enable
  * @retval The new state of __INTERRUPT__ (TRUE or FALSE).
  */
#define __HAL_HRTIM_MASTER_GET_ITSTATUS(__HANDLE__, __INTERRUPT__)     ((((__HANDLE__)->Instance->sMasterRegs.MDIER & (__INTERRUPT__)) == (__INTERRUPT__)) ? SET : RESET)

/** @brief  Checks if the specified HRTIM Timerx interrupt source  is enabled or disabled.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __TIMER__ specified the timing unit (Timer A to F)
  * @param  __INTERRUPT__ specifies the interrupt source to check.
  *        This parameter can be one of the following values:
  *            @arg HRTIM_MASTER_IT_MCMP1: Master compare 1 interrupt enable
  *            @arg HRTIM_MASTER_IT_MCMP2: Master compare 2 interrupt enable
  *            @arg HRTIM_MASTER_IT_MCMP3: Master compare 3 interrupt enable
  *            @arg HRTIM_MASTER_IT_MCMP4: Master compare 4 interrupt enable
  *            @arg HRTIM_MASTER_IT_MREP: Master Repetition interrupt enable
  *            @arg HRTIM_MASTER_IT_SYNC: Synchronization input interrupt enable
  *            @arg HRTIM_MASTER_IT_MUPD: Master update interrupt enable
  *            @arg HRTIM_TIM_IT_CMP1: Timer compare 1 interrupt enable
  *            @arg HRTIM_TIM_IT_CMP2: Timer compare 2 interrupt enable
  *            @arg HRTIM_TIM_IT_CMP3: Timer compare 3 interrupt enable
  *            @arg HRTIM_TIM_IT_CMP4: Timer compare 4 interrupt enable
  *            @arg HRTIM_TIM_IT_REP: Timer repetition interrupt enable
  *            @arg HRTIM_TIM_IT_UPD: Timer update interrupt enable
  *            @arg HRTIM_TIM_IT_CPT1: Timer capture 1 interrupt enable
  *            @arg HRTIM_TIM_IT_CPT2: Timer capture 2 interrupt enable
  *            @arg HRTIM_TIM_IT_SET1: Timer output 1 set interrupt enable
  *            @arg HRTIM_TIM_IT_RST1: Timer output 1 reset interrupt enable
  *            @arg HRTIM_TIM_IT_SET2: Timer output 2 set interrupt enable
  *            @arg HRTIM_TIM_IT_RST2: Timer output 2 reset interrupt enable
  *            @arg HRTIM_TIM_IT_RST: Timer reset interrupt enable
  *            @arg HRTIM_TIM_IT_DLYPRT: Timer delay protection interrupt enable
  * @retval The new state of __INTERRUPT__ (TRUE or FALSE).
  */
#define __HAL_HRTIM_TIMER_GET_ITSTATUS(__HANDLE__, __TIMER__, __INTERRUPT__)     ((((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].TIMxDIER & (__INTERRUPT__)) == (__INTERRUPT__)) ? SET : RESET)

/** @brief  Clears the specified HRTIM common pending flag.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __INTERRUPT__ specifies the interrupt pending bit to clear.
  *        This parameter can be one of the following values:
  *            @arg HRTIM_IT_FLT1: Fault 1 interrupt clear flag
  *            @arg HRTIM_IT_FLT2: Fault 2 interrupt clear flag
  *            @arg HRTIM_IT_FLT3: Fault 3 clear flag
  *            @arg HRTIM_IT_FLT4: Fault 4 clear flag
  *            @arg HRTIM_IT_FLT5: Fault 5 clear flag
  *            @arg HRTIM_IT_FLT6: Fault 6 clear flag
  *            @arg HRTIM_IT_SYSFLT: System Fault interrupt clear flag
  *            @arg HRTIM_IT_DLLRDY: DLL ready interrupt clear flag
  *            @arg HRTIM_IT_BMPER: Burst mode period interrupt clear flag
  * @retval None
  */
#define __HAL_HRTIM_CLEAR_IT(__HANDLE__, __INTERRUPT__)   ((__HANDLE__)->Instance->sCommonRegs.ICR = (__INTERRUPT__))

/** @brief  Clears the specified HRTIM Master pending flag.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __INTERRUPT__ specifies the interrupt pending bit to clear.
  *        This parameter can be one of the following values:
  *            @arg HRTIM_MASTER_IT_MCMP1: Master compare 1 interrupt clear flag
  *            @arg HRTIM_MASTER_IT_MCMP2: Master compare 2 interrupt clear flag
  *            @arg HRTIM_MASTER_IT_MCMP3: Master compare 3 interrupt clear flag
  *            @arg HRTIM_MASTER_IT_MCMP4: Master compare 4 interrupt clear flag
  *            @arg HRTIM_MASTER_IT_MREP: Master Repetition interrupt clear flag
  *            @arg HRTIM_MASTER_IT_SYNC: Synchronization input interrupt clear flag
  *            @arg HRTIM_MASTER_IT_MUPD: Master update interrupt clear flag
  * @retval None
  */
#define __HAL_HRTIM_MASTER_CLEAR_IT(__HANDLE__, __INTERRUPT__)   ((__HANDLE__)->Instance->sMasterRegs.MICR = (__INTERRUPT__))

/** @brief  Clears the specified HRTIM Timerx pending flag.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __TIMER__ specified the timing unit (Timer A to F)
  * @param  __INTERRUPT__ specifies the interrupt pending bit to clear.
  *        This parameter can be one of the following values:
  *            @arg HRTIM_TIM_IT_CMP1: Timer compare 1 interrupt clear flag
  *            @arg HRTIM_TIM_IT_CMP2: Timer compare 2 interrupt clear flag
  *            @arg HRTIM_TIM_IT_CMP3: Timer compare 3 interrupt clear flag
  *            @arg HRTIM_TIM_IT_CMP4: Timer compare 4 interrupt clear flag
  *            @arg HRTIM_TIM_IT_REP: Timer repetition interrupt clear flag
  *            @arg HRTIM_TIM_IT_UPD: Timer update interrupt clear flag
  *            @arg HRTIM_TIM_IT_CPT1: Timer capture 1 interrupt clear flag
  *            @arg HRTIM_TIM_IT_CPT2: Timer capture 2 interrupt clear flag
  *            @arg HRTIM_TIM_IT_SET1: Timer output 1 set interrupt clear flag
  *            @arg HRTIM_TIM_IT_RST1: Timer output 1 reset interrupt clear flag
  *            @arg HRTIM_TIM_IT_SET2: Timer output 2 set interrupt clear flag
  *            @arg HRTIM_TIM_IT_RST2: Timer output 2 reset interrupt clear flag
  *            @arg HRTIM_TIM_IT_RST: Timer reset interrupt clear flag
  *            @arg HRTIM_TIM_IT_DLYPRT: Timer output 1 delay protection interrupt clear flag
  * @retval None
  */
#define __HAL_HRTIM_TIMER_CLEAR_IT(__HANDLE__, __TIMER__, __INTERRUPT__)   ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].TIMxICR = (__INTERRUPT__))

/* DMA HANDLING */
/** @brief  Enables or disables the specified HRTIM Master timer DMA requests.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __DMA__ specifies the DMA request to enable or disable.
  *        This parameter can be one of the following values:
  *            @arg HRTIM_MASTER_DMA_MCMP1: Master compare 1 DMA request enable
  *            @arg HRTIM_MASTER_DMA_MCMP2: Master compare 2 DMA request enable
  *            @arg HRTIM_MASTER_DMA_MCMP3: Master compare 3 DMA request enable
  *            @arg HRTIM_MASTER_DMA_MCMP4: Master compare 4 DMA request enable
  *            @arg HRTIM_MASTER_DMA_MREP: Master Repetition DMA request enable
  *            @arg HRTIM_MASTER_DMA_SYNC: Synchronization input DMA request enable
  *            @arg HRTIM_MASTER_DMA_MUPD: Master update DMA request enable
  * @retval None
  */
#define __HAL_HRTIM_MASTER_ENABLE_DMA(__HANDLE__, __DMA__)   ((__HANDLE__)->Instance->sMasterRegs.MDIER |= (__DMA__))
#define __HAL_HRTIM_MASTER_DISABLE_DMA(__HANDLE__, __DMA__) ((__HANDLE__)->Instance->sMasterRegs.MDIER &= ~(__DMA__))

/** @brief  Enables or disables the specified HRTIM Timerx DMA requests.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __TIMER__ specified the timing unit (Timer A to F)
  * @param  __DMA__ specifies the DMA request to enable or disable.
  *        This parameter can be one of the following values:
  *            @arg HRTIM_TIM_DMA_CMP1: Timer compare 1 DMA request enable
  *            @arg HRTIM_TIM_DMA_CMP2: Timer compare 2 DMA request enable
  *            @arg HRTIM_TIM_DMA_CMP3: Timer compare 3 DMA request enable
  *            @arg HRTIM_TIM_DMA_CMP4: Timer compare 4 DMA request enable
  *            @arg HRTIM_TIM_DMA_REP: Timer repetition DMA request enable
  *            @arg HRTIM_TIM_DMA_UPD: Timer update DMA request enable
  *            @arg HRTIM_TIM_DMA_CPT1: Timer capture 1 DMA request enable
  *            @arg HRTIM_TIM_DMA_CPT2: Timer capture 2 DMA request enable
  *            @arg HRTIM_TIM_DMA_SET1: Timer output 1 set DMA request enable
  *            @arg HRTIM_TIM_DMA_RST1: Timer output 1 reset DMA request enable
  *            @arg HRTIM_TIM_DMA_SET2: Timer output 2 set DMA request enable
  *            @arg HRTIM_TIM_DMA_RST2: Timer output 2 reset DMA request enable
  *            @arg HRTIM_TIM_DMA_RST: Timer reset DMA request enable
  *            @arg HRTIM_TIM_DMA_DLYPRT: Timer delay protection DMA request enable
  * @retval None
  */
#define __HAL_HRTIM_TIMER_ENABLE_DMA(__HANDLE__, __TIMER__, __DMA__)   ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].TIMxDIER |= (__DMA__))
#define __HAL_HRTIM_TIMER_DISABLE_DMA(__HANDLE__, __TIMER__, __DMA__) ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].TIMxDIER &= ~(__DMA__))

/** @brief  Get the specified HRTIM MASTER FLAG.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __FLAG__ specifies the FLAG .
  *        This parameter can be one of the following values:
  *            @arg HRTIM_ISR_FLT1
  *            @arg HRTIM_ISR_FLT2
  *            @arg HRTIM_ISR_FLT3
  *            @arg HRTIM_ISR_FLT4
  *            @arg HRTIM_ISR_FLT5
  *            @arg HRTIM_ISR_SYSFLT
  *            @arg HRTIM_ISR_FLT6
  *            @arg HRTIM_ISR_DLLRDY
  *            @arg HRTIM_ISR_BMPER
  * @retval None
  */
#define __HAL_HRTIM_GET_FLAG(__HANDLE__, __FLAG__)        (((__HANDLE__)->Instance->sCommonRegs.ISR & (__FLAG__)) == (__FLAG__))

/** @brief  Clear the specified HRTIM MASTER FLAG.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __FLAG__ specifies the FLAG .
  *        This parameter can be one of the following values:
  *            @arg HRTIM_ICR_FLT1C
  *            @arg HRTIM_ICR_FLT2C
  *            @arg HRTIM_ICR_FLT3C
  *            @arg HRTIM_ICR_FLT4C
  *            @arg HRTIM_ICR_FLT5C
  *            @arg HRTIM_ICR_SYSFLTC
  *            @arg HRTIM_ICR_FLT6C
  *            @arg HRTIM_ICR_DLLRDYC
  *            @arg HRTIM_ICR_BMPERC
  * @retval None
  */
#define __HAL_HRTIM_CLEAR_FLAG(__HANDLE__, __FLAG__)      ((__HANDLE__)->Instance->sCommonRegs.ICR = (__FLAG__))

/** @brief  Get the specified HRTIM MASTER FLAG.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __FLAG__ specifies the FLAG .
  *        This parameter can be one of the following values:
  *            @arg HRTIM_MISR_MCMP1
  *            @arg HRTIM_MISR_MCMP2
  *            @arg HRTIM_MISR_MCMP3
  *            @arg HRTIM_MISR_MCMP4
  *            @arg HRTIM_MISR_MREP
  *            @arg HRTIM_MISR_SYNC
  *            @arg HRTIM_MISR_MUPD
  * @retval None
  */
#define __HAL_HRTIM_MASTER_GET_FLAG(__HANDLE__, __FLAG__)        (((__HANDLE__)->Instance->sMasterRegs.MISR & (__FLAG__)) == (__FLAG__))

/** @brief  Clear the specified HRTIM MASTER FLAG.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __FLAG__ specifies the FLAG .
  *        This parameter can be one of the following values:
  *            @arg HRTIM_MICR_MCMP1
  *            @arg HRTIM_MICR_MCMP2
  *            @arg HRTIM_MICR_MCMP3
  *            @arg HRTIM_MICR_MCMP4
  *            @arg HRTIM_MICR_MREP
  *            @arg HRTIM_MICR_SYNC
  *            @arg HRTIM_MICR_MUPD
  * @retval None
  */
#define __HAL_HRTIM_MASTER_CLEAR_FLAG(__HANDLE__, __FLAG__)      ((__HANDLE__)->Instance->sMasterRegs.MICR = (__FLAG__))

/** @brief  Get the specified HRTIM Timerx FLAG.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __TIMER__ specified the timing unit (Timer A to F)
  * @param  __FLAG__ specifies the FLAG .
  *        This parameter can be one of the following values:
  *            @arg HRTIM_TIMISR_CMP1
  *            @arg HRTIM_TIMISR_CMP2
  *            @arg HRTIM_TIMISR_CMP3
  *            @arg HRTIM_TIMISR_CMP4
  *            @arg HRTIM_TIMISR_REP
  *            @arg HRTIM_TIMISR_UPD
  *            @arg HRTIM_TIMISR_CPT1
  *            @arg HRTIM_TIMISR_CPT2
  *            @arg HRTIM_TIMISR_SET1
  *            @arg HRTIM_TIMISR_RST1
  *            @arg HRTIM_TIMISR_SET2
  *            @arg HRTIM_TIMISR_RST2
  *            @arg HRTIM_TIMISR_RST
  *            @arg HRTIM_TIMISR_DLYPRT
  *            @arg HRTIM_TIMISR_CPPSTAT
  *            @arg HRTIM_TIMISR_IPPSTAT
  *            @arg HRTIM_TIMISR_O1STAT
  *            @arg HRTIM_TIMISR_O2STAT
  *            @arg HRTIM_TIMISR_O1CPY
  *            @arg HRTIM_TIMISR_O2CPY
  * @retval None
  */
#define __HAL_HRTIM_TIMER_GET_FLAG(__HANDLE__,  __TIMER__, __FLAG__)        (((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].TIMxISR & (__FLAG__)) == (__FLAG__))

/** @brief  Clear the specified HRTIM Timerx FLAG.
  * @param  __HANDLE__ specifies the HRTIM Handle.
  * @param  __TIMER__ specified the timing unit (Timer A to F)
  * @param  __FLAG__ specifies the FLAG .
  *        This parameter can be one of the following values:
  *            @arg HRTIM_TIMICR_CMP1C
  *            @arg HRTIM_TIMICR_CMP2C
  *            @arg HRTIM_TIMICR_CMP3C
  *            @arg HRTIM_TIMICR_CMP4C
  *            @arg HRTIM_TIMICR_REPC
  *            @arg HRTIM_TIMICR_UPDC
  *            @arg HRTIM_TIMICR_CPT1C
  *            @arg HRTIM_TIMICR_CPT2C
  *            @arg HRTIM_TIMICR_SET1C
  *            @arg HRTIM_TIMICR_RST1C
  *            @arg HRTIM_TIMICR_SET2C
  *            @arg HRTIM_TIMICR_RST2C
  *            @arg HRTIM_TIMICR_RSTC
  *            @arg HRTIM_TIMICR_DLYPRTC
  * @retval None
  */
#define __HAL_HRTIM_TIMER_CLEAR_FLAG(__HANDLE__,  __TIMER__, __FLAG__)      ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].TIMxICR = (__FLAG__))

/** @brief  Sets the HRTIM timer Counter Register value on runtime
  * @param  __HANDLE__ HRTIM Handle.
  * @param  __TIMER__ HRTIM timer
  *         This parameter can be one of the following values:
  *           @arg 0x6 for master timer
  *           @arg 0x0 to 0x5 for timers A to F
  * @param  __COUNTER__ specifies the Counter Register new value.
  * @retval None
  */
#define __HAL_HRTIM_SETCOUNTER(__HANDLE__, __TIMER__, __COUNTER__) \
  (((__TIMER__) == HRTIM_TIMERINDEX_MASTER) ? ((__HANDLE__)->Instance->sMasterRegs.MCNTR = (__COUNTER__)) :\
   ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].CNTxR = (__COUNTER__)))

/** @brief  Gets the HRTIM timer Counter Register value on runtime
  * @param  __HANDLE__ HRTIM Handle.
  * @param  __TIMER__ HRTIM timer
  *         This parameter can be one of the following values:
  *           @arg 0x6 for master timer
  *           @arg 0x0 to 0x5 for timers A to F
  * @retval HRTIM timer Counter Register value
  */
#define __HAL_HRTIM_GETCOUNTER(__HANDLE__, __TIMER__) \
  (((__TIMER__) == HRTIM_TIMERINDEX_MASTER) ? ((__HANDLE__)->Instance->sMasterRegs.MCNTR) :\
   ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].CNTxR))

/** @brief  Sets the HRTIM timer Period value on runtime
  * @param  __HANDLE__ HRTIM Handle.
  * @param  __TIMER__ HRTIM timer
  *         This parameter can be one of the following values:
  *           @arg 0x6 for master timer
  *           @arg 0x0 to 0x5 for timers A to F
  * @param  __PERIOD__ specifies the Period Register new value.
  * @retval None
  */
#define __HAL_HRTIM_SETPERIOD(__HANDLE__, __TIMER__, __PERIOD__) \
  (((__TIMER__) == HRTIM_TIMERINDEX_MASTER) ? ((__HANDLE__)->Instance->sMasterRegs.MPER = (__PERIOD__)) :\
   ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].PERxR = (__PERIOD__)))

/** @brief  Gets the HRTIM timer Period Register value on runtime
  * @param  __HANDLE__ HRTIM Handle.
  * @param  __TIMER__ HRTIM timer
  *         This parameter can be one of the following values:
  *           @arg 0x6 for master timer
  *           @arg 0x0 to 0x5 for timers A to F
  * @retval timer Period Register
  */
#define __HAL_HRTIM_GETPERIOD(__HANDLE__, __TIMER__) \
  (((__TIMER__) == HRTIM_TIMERINDEX_MASTER) ? ((__HANDLE__)->Instance->sMasterRegs.MPER) :\
   ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].PERxR))

/** @brief  Sets the HRTIM timer clock prescaler value on runtime
  * @param  __HANDLE__ HRTIM Handle.
  * @param  __TIMER__ HRTIM timer
  *         This parameter can be one of the following values:
  *           @arg 0x6 for master timer
  *           @arg 0x0 to 0x5 for timers A to F
  * @param  __PRESCALER__ specifies the clock prescaler new value.
  *         This parameter can be one of the following values:
  *           @arg HRTIM_PRESCALERRATIO_MUL32: fHRCK: 4.608 GHz - Resolution: 217 ps - Min PWM frequency: 70.3 kHz (fHRTIM=144MHz)
  *           @arg HRTIM_PRESCALERRATIO_MUL16: fHRCK: 2.304 GHz - Resolution: 434 ps - Min PWM frequency: 35.1 KHz (fHRTIM=144MHz)
  *           @arg HRTIM_PRESCALERRATIO_MUL8: fHRCK: 1.152 GHz - Resolution: 868 ps - Min PWM frequency: 17.6 kHz (fHRTIM=144MHz)
  *           @arg HRTIM_PRESCALERRATIO_MUL4: fHRCK: 576 MHz - Resolution: 1.73 ns - Min PWM frequency: 8.8 kHz (fHRTIM=144MHz)
  *           @arg HRTIM_PRESCALERRATIO_MUL2: fHRCK: 288 MHz - Resolution: 3.47 ns - Min PWM frequency: 4.4 kHz (fHRTIM=144MHz)
  *           @arg HRTIM_PRESCALERRATIO_DIV1: fHRCK: 144 MHz - Resolution: 6.95 ns - Min PWM frequency: 2.2 kHz (fHRTIM=144MHz)
  *           @arg HRTIM_PRESCALERRATIO_DIV2: fHRCK: 72 MHz - Resolution: 13.88 ns- Min PWM frequency: 1.1 kHz (fHRTIM=144MHz)
  *           @arg HRTIM_PRESCALERRATIO_DIV4: fHRCK: 36 MHz - Resolution: 27.7 ns- Min PWM frequency: 550Hz (fHRTIM=144MHz)
  * @retval None
  */
#define __HAL_HRTIM_SETCLOCKPRESCALER(__HANDLE__, __TIMER__, __PRESCALER__) \
  (((__TIMER__) == HRTIM_TIMERINDEX_MASTER) ? (MODIFY_REG((__HANDLE__)->Instance->sMasterRegs.MCR, HRTIM_MCR_CK_PSC, (__PRESCALER__))) :\
   (MODIFY_REG((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].TIMxCR, HRTIM_TIMCR_CK_PSC, (__PRESCALER__))))

/** @brief  Gets the HRTIM timer clock prescaler value on runtime
  * @param  __HANDLE__ HRTIM Handle.
  * @param  __TIMER__ HRTIM timer
  *         This parameter can be one of the following values:
  *           @arg 0x6 for master timer
  *           @arg 0x0 to 0x5 for timers A to F
  * @retval timer clock prescaler value
  */
#define __HAL_HRTIM_GETCLOCKPRESCALER(__HANDLE__, __TIMER__) \
  (((__TIMER__) == HRTIM_TIMERINDEX_MASTER) ? ((__HANDLE__)->Instance->sMasterRegs.MCR & HRTIM_MCR_CK_PSC) :\
   ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].TIMxCR  & HRTIM_TIMCR_CK_PSC))

/** @brief  Sets the HRTIM timer Compare Register value on runtime
  * @param  __HANDLE__ HRTIM Handle.
  * @param  __TIMER__ HRTIM timer
  *         This parameter can be one of the following values:
  *           @arg 0x0 to 0x5 for timers A to F
  * @param  __COMPAREUNIT__ timer compare unit
  *         This parameter can be one of the following values:
  *           @arg HRTIM_COMPAREUNIT_1: Compare unit 1
  *           @arg HRTIM_COMPAREUNIT_2: Compare unit 2
  *           @arg HRTIM_COMPAREUNIT_3: Compare unit 3
  *           @arg HRTIM_COMPAREUNIT_4: Compare unit 4
  * @param  __COMPARE__ specifies the Compare new value.
  * @retval None
  */
#define __HAL_HRTIM_SETCOMPARE(__HANDLE__, __TIMER__, __COMPAREUNIT__, __COMPARE__) \
      (((__TIMER__) == HRTIM_TIMERINDEX_MASTER) ? \
        (((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_1) ? ((__HANDLE__)->Instance->sMasterRegs.MCMP1R = (__COMPARE__)) :\
         ((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_2) ? ((__HANDLE__)->Instance->sMasterRegs.MCMP2R = (__COMPARE__)) :\
         ((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_3) ? ((__HANDLE__)->Instance->sMasterRegs.MCMP3R = (__COMPARE__)) :\
         ((__HANDLE__)->Instance->sMasterRegs.MCMP4R = (__COMPARE__))) \
         : \
        (((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_1) ? ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].CMP1xR = (__COMPARE__)) :\
         ((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_2) ? ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].CMP2xR = (__COMPARE__)) :\
         ((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_3) ? ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].CMP3xR = (__COMPARE__)) :\
         ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].CMP4xR = (__COMPARE__))))

/** @brief  Gets the HRTIM timer Compare Register value on runtime
  * @param  __HANDLE__ HRTIM Handle.
  * @param  __TIMER__ HRTIM timer
  *         This parameter can be one of the following values:
  *           @arg 0x0 to 0x5 for timers A to F
  * @param  __COMPAREUNIT__ timer compare unit
  *         This parameter can be one of the following values:
  *           @arg HRTIM_COMPAREUNIT_1: Compare unit 1
  *           @arg HRTIM_COMPAREUNIT_2: Compare unit 2
  *           @arg HRTIM_COMPAREUNIT_3: Compare unit 3
  *           @arg HRTIM_COMPAREUNIT_4: Compare unit 4
  * @retval Compare value
  */
#define __HAL_HRTIM_GETCOMPARE(__HANDLE__, __TIMER__, __COMPAREUNIT__) \
      (((__TIMER__) == HRTIM_TIMERINDEX_MASTER) ? \
        (((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_1) ? ((__HANDLE__)->Instance->sMasterRegs.MCMP1R) :\
         ((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_2) ? ((__HANDLE__)->Instance->sMasterRegs.MCMP2R) :\
         ((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_3) ? ((__HANDLE__)->Instance->sMasterRegs.MCMP3R) :\
         ((__HANDLE__)->Instance->sMasterRegs.MCMP4R)) \
         : \
        (((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_1) ? ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].CMP1xR) :\
         ((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_2) ? ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].CMP2xR) :\
         ((__COMPAREUNIT__) == HRTIM_COMPAREUNIT_3) ? ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].CMP3xR) :\
         ((__HANDLE__)->Instance->sTimerxRegs[(__TIMER__)].CMP4xR)))

/**
  * @brief  Enables the Fault Counter
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Fault fault input to enable
  *         This parameter can be one of the following values:
  *           @arg HRTIM_FAULT_1: Fault input 1
  *           @arg HRTIM_FAULT_2: Fault input 2
  *           @arg HRTIM_FAULT_3: Fault input 3
  *           @arg HRTIM_FAULT_4: Fault input 4
  *           @arg HRTIM_FAULT_5: Fault input 5
  *           @arg HRTIM_FAULT_6: Fault input 6
  * @note This function must be called when fault is not enabled
  * @retval HAL status
  */
#define __HAL_HRTIM_FAULT_BLANKING_ENABLE(__HANDLE__, __FAULT__)\
  do {\
    if (((__FAULT__) & HRTIM_FAULT_1) == HRTIM_FAULT_1)\
    {\
              ((__HANDLE__)->Instance->sCommonRegs.FLTINR3) |= HRTIM_FLTINR3_FLT1BLKE;\
    }\
    if (((__FAULT__) & HRTIM_FAULT_2) == HRTIM_FAULT_2)\
    {\
              ((__HANDLE__)->Instance->sCommonRegs.FLTINR3) |= HRTIM_FLTINR3_FLT2BLKE;\
    }\
    if (((__FAULT__) & HRTIM_FAULT_3) == HRTIM_FAULT_3)\
    {\
              ((__HANDLE__)->Instance->sCommonRegs.FLTINR3) |= HRTIM_FLTINR3_FLT3BLKE;\
    }\
    if (((__FAULT__) & HRTIM_FAULT_4) == HRTIM_FAULT_4)\
    {\
              ((__HANDLE__)->Instance->sCommonRegs.FLTINR3) |= HRTIM_FLTINR3_FLT4BLKE;\
    }\
    if (((__FAULT__) & HRTIM_FAULT_5) == HRTIM_FAULT_5)\
    {\
              ((__HANDLE__)->Instance->sCommonRegs.FLTINR4) |= HRTIM_FLTINR4_FLT5BLKE;\
    }\
    if (((__FAULT__) & HRTIM_FAULT_6) == HRTIM_FAULT_6)\
    {\
              ((__HANDLE__)->Instance->sCommonRegs.FLTINR4) |= HRTIM_FLTINR4_FLT6BLKE;\
    }\
  } while(0U)

/**
  * @brief  Disables the Fault Counter
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Fault fault input to disable
  *         This parameter can be one of the following values:
  *           @arg HRTIM_FAULT_1: Fault input 1
  *           @arg HRTIM_FAULT_2: Fault input 2
  *           @arg HRTIM_FAULT_3: Fault input 3
  *           @arg HRTIM_FAULT_4: Fault input 4
  *           @arg HRTIM_FAULT_5: Fault input 5
  *           @arg HRTIM_FAULT_6: Fault input 6
  * @retval HAL status
  */
#define __HAL_HRTIM_FAULT_BLANKING_DISABLE(__HANDLE__, __FAULT__)\
  do {\
    if (((__FAULT__) & HRTIM_FAULT_1) == HRTIM_FAULT_1)\
    {\
              ((__HANDLE__)->Instance->sCommonRegs.FLTINR3) &= ~HRTIM_FLTINR3_FLT1BLKE;\
    }\
    if (((__FAULT__) & HRTIM_FAULT_2) == HRTIM_FAULT_2)\
    {\
              ((__HANDLE__)->Instance->sCommonRegs.FLTINR3) &= ~HRTIM_FLTINR3_FLT2BLKE;\
    }\
    if (((__FAULT__) & HRTIM_FAULT_3) == HRTIM_FAULT_3)\
    {\
              ((__HANDLE__)->Instance->sCommonRegs.FLTINR3) &= ~HRTIM_FLTINR3_FLT3BLKE;\
    }\
    if (((__FAULT__) & HRTIM_FAULT_4) == HRTIM_FAULT_4)\
    {\
              ((__HANDLE__)->Instance->sCommonRegs.FLTINR3) &= ~HRTIM_FLTINR3_FLT4BLKE;\
    }\
    if (((__FAULT__) & HRTIM_FAULT_5) == HRTIM_FAULT_5)\
    {\
              ((__HANDLE__)->Instance->sCommonRegs.FLTINR4) &= ~HRTIM_FLTINR4_FLT5BLKE;\
    }\
    if (((__FAULT__) & HRTIM_FAULT_6) == HRTIM_FAULT_6)\
    {\
              ((__HANDLE__)->Instance->sCommonRegs.FLTINR4) &= ~HRTIM_FLTINR4_FLT6BLKE;\
    }\
  } while(0U)

/**
  * end of HRTIM_Macro_Definitions @}
  */

/******************************************************************************/
/*                             HRTIM Functions                             */
/******************************************************************************/

/** @defgroup HRTIM_Function_Definitions HRTIM Function Definitions
  * @{
  */

/* Exported functions --------------------------------------------------------*/

/* Initialization and Configuration functions  ********************************/
HAL_StatusTypeDef HAL_HRTIM_Init(HRTIM_HandleTypeDef *hhrtim);

HAL_StatusTypeDef HAL_HRTIM_DeInit (HRTIM_HandleTypeDef *hhrtim);

void HAL_HRTIM_MspInit(HRTIM_HandleTypeDef *hhrtim);

void HAL_HRTIM_MspDeInit(HRTIM_HandleTypeDef *hhrtim);

HAL_StatusTypeDef HAL_HRTIM_TimeBaseConfig(HRTIM_HandleTypeDef *hhrtim,
                                           uint32_t TimerIdx,
                                           const HRTIM_TimeBaseCfgTypeDef * pTimeBaseCfg);

HAL_StatusTypeDef HAL_HRTIM_DLLCalibrationStart(HRTIM_HandleTypeDef *hhrtim,
                                                uint32_t CalibrationRate);

HAL_StatusTypeDef HAL_HRTIM_DLLCalibrationStart_IT(HRTIM_HandleTypeDef *hhrtim,
                                                   uint32_t CalibrationRate);

HAL_StatusTypeDef HAL_HRTIM_PollForDLLCalibration(HRTIM_HandleTypeDef *hhrtim,
                                                  uint32_t Timeout);

/* Simple time base related functions  *****************************************/
HAL_StatusTypeDef HAL_HRTIM_SimpleBaseStart(HRTIM_HandleTypeDef *hhrtim,
                                           uint32_t TimerIdx);

HAL_StatusTypeDef HAL_HRTIM_SimpleBaseStop(HRTIM_HandleTypeDef *hhrtim,
                                          uint32_t TimerIdx);

HAL_StatusTypeDef HAL_HRTIM_SimpleBaseStart_IT(HRTIM_HandleTypeDef *hhrtim,
                                              uint32_t TimerIdx);

HAL_StatusTypeDef HAL_HRTIM_SimpleBaseStop_IT(HRTIM_HandleTypeDef *hhrtim,
                                             uint32_t TimerIdx);

HAL_StatusTypeDef HAL_HRTIM_SimpleBaseStart_DMA(HRTIM_HandleTypeDef *hhrtim,
                                               uint32_t TimerIdx,
                                               uint32_t SrcAddr,
                                               uint32_t DestAddr,
                                               uint32_t Length);

HAL_StatusTypeDef HAL_HRTIM_SimpleBaseStop_DMA(HRTIM_HandleTypeDef *hhrtim,
                                              uint32_t TimerIdx);

/* Simple output compare related functions  ************************************/
HAL_StatusTypeDef HAL_HRTIM_SimpleOCChannelConfig(HRTIM_HandleTypeDef *hhrtim,
                                                 uint32_t TimerIdx,
                                                 uint32_t OCChannel,
                                                 const HRTIM_SimpleOCChannelCfgTypeDef* pSimpleOCChannelCfg);

HAL_StatusTypeDef HAL_HRTIM_SimpleOCStart(HRTIM_HandleTypeDef *hhrtim,
                                         uint32_t TimerIdx,
                                         uint32_t OCChannel);

HAL_StatusTypeDef HAL_HRTIM_SimpleOCStop(HRTIM_HandleTypeDef *hhrtim,
                                        uint32_t TimerIdx,
                                        uint32_t OCChannel);

HAL_StatusTypeDef HAL_HRTIM_SimpleOCStart_IT(HRTIM_HandleTypeDef *hhrtim,
                                            uint32_t TimerIdx,
                                            uint32_t OCChannel);

HAL_StatusTypeDef HAL_HRTIM_SimpleOCStop_IT(HRTIM_HandleTypeDef *hhrtim,
                                           uint32_t TimerIdx,
                                           uint32_t OCChannel);

HAL_StatusTypeDef HAL_HRTIM_SimpleOCStart_DMA(HRTIM_HandleTypeDef *hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t OCChannel,
                                             uint32_t SrcAddr,
                                             uint32_t DestAddr,
                                             uint32_t Length);

HAL_StatusTypeDef HAL_HRTIM_SimpleOCStop_DMA(HRTIM_HandleTypeDef *hhrtim,
                                            uint32_t TimerIdx,
                                            uint32_t OCChannel);

/* Simple PWM output related functions  ****************************************/
HAL_StatusTypeDef HAL_HRTIM_SimplePWMChannelConfig(HRTIM_HandleTypeDef *hhrtim,
                                                  uint32_t TimerIdx,
                                                  uint32_t PWMChannel,
                                                  const HRTIM_SimplePWMChannelCfgTypeDef* pSimplePWMChannelCfg);

HAL_StatusTypeDef HAL_HRTIM_SimplePWMStart(HRTIM_HandleTypeDef *hhrtim,
                                          uint32_t TimerIdx,
                                          uint32_t PWMChannel);

HAL_StatusTypeDef HAL_HRTIM_SimplePWMStop(HRTIM_HandleTypeDef *hhrtim,
                                         uint32_t TimerIdx,
                                         uint32_t PWMChannel);

HAL_StatusTypeDef HAL_HRTIM_SimplePWMStart_IT(HRTIM_HandleTypeDef *hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t PWMChannel);

HAL_StatusTypeDef HAL_HRTIM_SimplePWMStop_IT(HRTIM_HandleTypeDef *hhrtim,
                                            uint32_t TimerIdx,
                                            uint32_t PWMChannel);

HAL_StatusTypeDef HAL_HRTIM_SimplePWMStart_DMA(HRTIM_HandleTypeDef *hhrtim,
                                              uint32_t TimerIdx,
                                              uint32_t PWMChannel,
                                              uint32_t SrcAddr,
                                              uint32_t DestAddr,
                                              uint32_t Length);

HAL_StatusTypeDef HAL_HRTIM_SimplePWMStop_DMA(HRTIM_HandleTypeDef *hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t PWMChannel);

/* Simple capture related functions  *******************************************/
HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureChannelConfig(HRTIM_HandleTypeDef *hhrtim,
                                                      uint32_t TimerIdx,
                                                      uint32_t CaptureChannel,
                                                      const HRTIM_SimpleCaptureChannelCfgTypeDef* pSimpleCaptureChannelCfg);

HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureStart(HRTIM_HandleTypeDef *hhrtim,
                                              uint32_t TimerIdx,
                                              uint32_t CaptureChannel);

HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureStop(HRTIM_HandleTypeDef *hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t CaptureChannel);

HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureStart_IT(HRTIM_HandleTypeDef *hhrtim,
                                                 uint32_t TimerIdx,
                                                 uint32_t CaptureChannel);

HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureStop_IT(HRTIM_HandleTypeDef *hhrtim,
                                                uint32_t TimerIdx,
                                                uint32_t CaptureChannel);

HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureStart_DMA(HRTIM_HandleTypeDef *hhrtim,
                                                  uint32_t TimerIdx,
                                                  uint32_t CaptureChannel,
                                                  uint32_t SrcAddr,
                                                  uint32_t DestAddr,
                                                  uint32_t Length);

HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureStop_DMA(HRTIM_HandleTypeDef *hhrtim,
                                                 uint32_t TimerIdx,
                                                 uint32_t CaptureChannel);

/* Simple one pulse related functions  *****************************************/
HAL_StatusTypeDef HAL_HRTIM_SimpleOnePulseChannelConfig(HRTIM_HandleTypeDef *hhrtim,
                                                       uint32_t TimerIdx,
                                                       uint32_t OnePulseChannel,
                                                       const HRTIM_SimpleOnePulseChannelCfgTypeDef* pSimpleOnePulseChannelCfg);

HAL_StatusTypeDef HAL_HRTIM_SimpleOnePulseStart(HRTIM_HandleTypeDef *hhrtim,
                                               uint32_t TimerIdx,
                                               uint32_t OnePulseChannel);

HAL_StatusTypeDef HAL_HRTIM_SimpleOnePulseStop(HRTIM_HandleTypeDef *hhrtim,
                                              uint32_t TimerIdx,
                                             uint32_t OnePulseChannel);

HAL_StatusTypeDef HAL_HRTIM_SimpleOnePulseStart_IT(HRTIM_HandleTypeDef *hhrtim,
                                                  uint32_t TimerIdx,
                                                  uint32_t OnePulseChannel);

HAL_StatusTypeDef HAL_HRTIM_SimpleOnePulseStop_IT(HRTIM_HandleTypeDef *hhrtim,
                                                 uint32_t TimerIdx,
                                                 uint32_t OnePulseChannel);

HAL_StatusTypeDef HAL_HRTIM_BurstModeConfig(HRTIM_HandleTypeDef *hhrtim,
                                            const HRTIM_BurstModeCfgTypeDef* pBurstModeCfg);

HAL_StatusTypeDef HAL_HRTIM_EventConfig(HRTIM_HandleTypeDef *hhrtim,
                                        uint32_t Event,
                                        const HRTIM_EventCfgTypeDef* pEventCfg);

HAL_StatusTypeDef HAL_HRTIM_EventPrescalerConfig(HRTIM_HandleTypeDef *hhrtim,
                                                 uint32_t Prescaler);

HAL_StatusTypeDef HAL_HRTIM_FaultConfig(HRTIM_HandleTypeDef *hhrtim,
                                        uint32_t Fault,
                                        const HRTIM_FaultCfgTypeDef* pFaultCfg);

HAL_StatusTypeDef HAL_HRTIM_FaultPrescalerConfig(HRTIM_HandleTypeDef *hhrtim,
                                                 uint32_t Prescaler);

HAL_StatusTypeDef HAL_HRTIM_FaultBlankingConfigAndEnable(HRTIM_HandleTypeDef * hhrtim,
                                               uint32_t Fault,
                                               const HRTIM_FaultBlankingCfgTypeDef* pFaultBlkCfg);

HAL_StatusTypeDef HAL_HRTIM_FaultCounterConfig(HRTIM_HandleTypeDef * hhrtim,
                                               uint32_t Fault,
                                               const HRTIM_FaultBlankingCfgTypeDef* pFaultBlkCfg);

HAL_StatusTypeDef HAL_HRTIM_FaultCounterReset(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t Fault);

HAL_StatusTypeDef HAL_HRTIM_SwapTimerOutput(HRTIM_HandleTypeDef * hhrtim,
                                           uint32_t Timers);
void HAL_HRTIM_FaultModeCtl(HRTIM_HandleTypeDef * hhrtim,
                            uint32_t Faults,
                            uint32_t Enable);

HAL_StatusTypeDef HAL_HRTIM_ADCTriggerConfig(HRTIM_HandleTypeDef *hhrtim,
                                             uint32_t ADCTrigger,
                                             const HRTIM_ADCTriggerCfgTypeDef* pADCTriggerCfg);

HAL_StatusTypeDef HAL_HRTIM_ADCPostScalerConfig(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t ADCTrigger,
                                             uint32_t Postscaler);

HAL_StatusTypeDef HAL_HRTIM_RollOverModeConfig(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t RollOverCfg);

HAL_StatusTypeDef HAL_HRTIM_OutputSwapEnable(HRTIM_HandleTypeDef * hhrtim,
                                          uint32_t Timers);

HAL_StatusTypeDef HAL_HRTIM_OutputSwapDisable(HRTIM_HandleTypeDef * hhrtim,
                                          uint32_t Timers);

/* Waveform related functions *************************************************/
HAL_StatusTypeDef HAL_HRTIM_WaveformTimerConfig(HRTIM_HandleTypeDef *hhrtim,
                                                uint32_t TimerIdx,
                                                const HRTIM_TimerCfgTypeDef * pTimerCfg);

HAL_StatusTypeDef HAL_HRTIM_WaveformTimerControl(HRTIM_HandleTypeDef * hhrtim,
                                                uint32_t TimerIdx,
                                                const HRTIM_TimerCtlTypeDef * pTimerCtl);

HAL_StatusTypeDef HAL_HRTIM_TimerDualChannelDacConfig(HRTIM_HandleTypeDef * hhrtim,
                                                uint32_t TimerIdx,
                                                const HRTIM_TimerCtlTypeDef * pTimerCtl);

HAL_StatusTypeDef HAL_HRTIM_WaveformCompareConfig(HRTIM_HandleTypeDef *hhrtim,
                                                  uint32_t TimerIdx,
                                                  uint32_t CompareUnit,
                                                  const HRTIM_CompareCfgTypeDef* pCompareCfg);

HAL_StatusTypeDef HAL_HRTIM_WaveformCaptureConfig(HRTIM_HandleTypeDef *hhrtim,
                                                  uint32_t TimerIdx,
                                                  uint32_t CaptureUnit,
                                                  const HRTIM_CaptureCfgTypeDef* pCaptureCfg);

HAL_StatusTypeDef HAL_HRTIM_WaveformOutputConfig(HRTIM_HandleTypeDef *hhrtim,
                                                 uint32_t TimerIdx,
                                                 uint32_t Output,
                                                 const HRTIM_OutputCfgTypeDef * pOutputCfg);

HAL_StatusTypeDef HAL_HRTIM_WaveformSetOutputLevel(HRTIM_HandleTypeDef *hhrtim,
                                                   uint32_t TimerIdx,
                                                   uint32_t Output,
                                                   uint32_t OutputLevel);

HAL_StatusTypeDef HAL_HRTIM_TimerEventFilteringConfig(HRTIM_HandleTypeDef *hhrtim,
                                                      uint32_t TimerIdx,
                                                      uint32_t Event,
                                                      const HRTIM_TimerEventFilteringCfgTypeDef * pTimerEventFilteringCfg);

HAL_StatusTypeDef HAL_HRTIM_ExtEventCounterConfig(HRTIM_HandleTypeDef * hhrtim,
                                                     uint32_t TimerIdx,
                                                     uint32_t EventCounter,
                                                     const HRTIM_ExternalEventCfgTypeDef* pTimerExternalEventCfg);

HAL_StatusTypeDef HAL_HRTIM_ExtEventCounterEnable(HRTIM_HandleTypeDef * hhrtim,
                                                      uint32_t TimerIdx,
                                                      uint32_t EventCounter);

HAL_StatusTypeDef HAL_HRTIM_ExtEventCounterDisable(HRTIM_HandleTypeDef * hhrtim,
                                                      uint32_t TimerIdx,
                                                      uint32_t EventCounter);

HAL_StatusTypeDef HAL_HRTIM_ExtEventCounterReset(HRTIM_HandleTypeDef * hhrtim,
                                                           uint32_t TimerIdx,
                                                           uint32_t EventCounter);

HAL_StatusTypeDef HAL_HRTIM_DeadTimeConfig(HRTIM_HandleTypeDef *hhrtim,
                                           uint32_t TimerIdx,
                                           const HRTIM_DeadTimeCfgTypeDef* pDeadTimeCfg);

HAL_StatusTypeDef HAL_HRTIM_ChopperModeConfig(HRTIM_HandleTypeDef *hhrtim,
                                              uint32_t TimerIdx,
                                              const HRTIM_ChopperModeCfgTypeDef* pChopperModeCfg);

HAL_StatusTypeDef HAL_HRTIM_BurstDMAConfig(HRTIM_HandleTypeDef *hhrtim,
                                           uint32_t TimerIdx,
                                           uint32_t RegistersToUpdate);


HAL_StatusTypeDef HAL_HRTIM_WaveformCountStart(HRTIM_HandleTypeDef *hhrtim,
                                                 uint32_t Timers);

HAL_StatusTypeDef HAL_HRTIM_WaveformCountStop(HRTIM_HandleTypeDef *hhrtim,
                                                 uint32_t Timers);

HAL_StatusTypeDef HAL_HRTIM_WaveformCountStart_IT(HRTIM_HandleTypeDef *hhrtim,
                                                 uint32_t Timers);

HAL_StatusTypeDef HAL_HRTIM_WaveformCountStop_IT(HRTIM_HandleTypeDef *hhrtim,
                                                 uint32_t Timers);

HAL_StatusTypeDef HAL_HRTIM_WaveformCountStart_DMA(HRTIM_HandleTypeDef *hhrtim,
                                                     uint32_t Timers);

HAL_StatusTypeDef HAL_HRTIM_WaveformCountStop_DMA(HRTIM_HandleTypeDef *hhrtim,
                                                    uint32_t Timers);

HAL_StatusTypeDef HAL_HRTIM_WaveformOutputStart(HRTIM_HandleTypeDef *hhrtim,
                                                uint32_t OutputsToStart);

HAL_StatusTypeDef HAL_HRTIM_WaveformOutputStop(HRTIM_HandleTypeDef *hhrtim,
                                               uint32_t OutputsToStop);

HAL_StatusTypeDef HAL_HRTIM_BurstModeCtl(HRTIM_HandleTypeDef *hhrtim,
                                         uint32_t Enable);

HAL_StatusTypeDef HAL_HRTIM_BurstModeSoftwareTrigger(HRTIM_HandleTypeDef *hhrtim);

HAL_StatusTypeDef HAL_HRTIM_SoftwareCapture(HRTIM_HandleTypeDef *hhrtim,
                                            uint32_t TimerIdx,
                                            uint32_t CaptureUnit);

HAL_StatusTypeDef HAL_HRTIM_SoftwareUpdate(HRTIM_HandleTypeDef *hhrtim,
                                           uint32_t Timers);

HAL_StatusTypeDef HAL_HRTIM_SoftwareReset(HRTIM_HandleTypeDef *hhrtim,
                                          uint32_t Timers);

HAL_StatusTypeDef HAL_HRTIM_BurstDMATransfer(HRTIM_HandleTypeDef *hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t BurstBufferAddress,
                                             uint32_t BurstBufferLength);

HAL_StatusTypeDef HAL_HRTIM_UpdateEnable(HRTIM_HandleTypeDef *hhrtim,
                                          uint32_t Timers);

HAL_StatusTypeDef HAL_HRTIM_UpdateDisable(HRTIM_HandleTypeDef *hhrtim,
                                          uint32_t Timers);

/* HRTIM peripheral state functions */
HAL_HRTIM_StateTypeDef HAL_HRTIM_GetState(const HRTIM_HandleTypeDef* hhrtim);

uint32_t HAL_HRTIM_GetCapturedValue(const HRTIM_HandleTypeDef * hhrtim,
                                    uint32_t TimerIdx,
                                    uint32_t CaptureUnit);

uint32_t HAL_HRTIM_GetCapturedDir(const HRTIM_HandleTypeDef * hhrtim,
                                    uint32_t TimerIdx,
                                    uint32_t CaptureUnit);

HRTIM_CaptureValueTypeDef HAL_HRTIM_GetCaptured(const HRTIM_HandleTypeDef * hhrtim,
                                    uint32_t TimerIdx,
                                    uint32_t CaptureUnit);

uint32_t HAL_HRTIM_WaveformGetOutputLevel(const HRTIM_HandleTypeDef *hhrtim,
                                          uint32_t TimerIdx,
                                          uint32_t Output);

uint32_t HAL_HRTIM_WaveformGetOutputState(const HRTIM_HandleTypeDef * hhrtim,
                                          uint32_t TimerIdx,
                                          uint32_t Output);

uint32_t HAL_HRTIM_GetDelayedProtectionStatus(const HRTIM_HandleTypeDef *hhrtim,
                                              uint32_t TimerIdx,
                                              uint32_t Output);

uint32_t HAL_HRTIM_GetBurstStatus(const HRTIM_HandleTypeDef *hhrtim);

uint32_t HAL_HRTIM_GetCurrentPushPullStatus(const HRTIM_HandleTypeDef *hhrtim,
                                            uint32_t TimerIdx);

uint32_t HAL_HRTIM_GetIdlePushPullStatus(const HRTIM_HandleTypeDef *hhrtim,
                                         uint32_t TimerIdx);

/* IRQ handler */
void HAL_HRTIM_IRQHandler(HRTIM_HandleTypeDef *hhrtim,
                          uint32_t TimerIdx);

/* HRTIM events related callback functions */
void HAL_HRTIM_Fault1Callback(HRTIM_HandleTypeDef *hhrtim);
void HAL_HRTIM_Fault2Callback(HRTIM_HandleTypeDef *hhrtim);
void HAL_HRTIM_Fault3Callback(HRTIM_HandleTypeDef *hhrtim);
void HAL_HRTIM_Fault4Callback(HRTIM_HandleTypeDef *hhrtim);
void HAL_HRTIM_Fault5Callback(HRTIM_HandleTypeDef *hhrtim);
void HAL_HRTIM_Fault6Callback(HRTIM_HandleTypeDef *hhrtim);
void HAL_HRTIM_SystemFaultCallback(HRTIM_HandleTypeDef *hhrtim);
void HAL_HRTIM_DLLCalibrationReadyCallback(HRTIM_HandleTypeDef *hhrtim);
void HAL_HRTIM_BurstModePeriodCallback(HRTIM_HandleTypeDef *hhrtim);
void HAL_HRTIM_SynchronizationEventCallback(HRTIM_HandleTypeDef *hhrtim);

/* Timer events related callback functions */
void HAL_HRTIM_RegistersUpdateCallback(HRTIM_HandleTypeDef *hhrtim,
                                              uint32_t TimerIdx);
void HAL_HRTIM_RepetitionEventCallback(HRTIM_HandleTypeDef *hhrtim,
                                              uint32_t TimerIdx);
void HAL_HRTIM_Compare1EventCallback(HRTIM_HandleTypeDef *hhrtim,
                                            uint32_t TimerIdx);
void HAL_HRTIM_Compare2EventCallback(HRTIM_HandleTypeDef *hhrtim,
                                            uint32_t TimerIdx);
void HAL_HRTIM_Compare3EventCallback(HRTIM_HandleTypeDef *hhrtim,
                                            uint32_t TimerIdx);
void HAL_HRTIM_Compare4EventCallback(HRTIM_HandleTypeDef *hhrtim,
                                            uint32_t TimerIdx);
void HAL_HRTIM_Capture1EventCallback(HRTIM_HandleTypeDef *hhrtim,
                                            uint32_t TimerIdx);
void HAL_HRTIM_Capture2EventCallback(HRTIM_HandleTypeDef *hhrtim,
                                            uint32_t TimerIdx);
void HAL_HRTIM_DelayedProtectionCallback(HRTIM_HandleTypeDef *hhrtim,
                                                uint32_t TimerIdx);
void HAL_HRTIM_CounterResetCallback(HRTIM_HandleTypeDef *hhrtim,
                                           uint32_t TimerIdx);
void HAL_HRTIM_Output1SetCallback(HRTIM_HandleTypeDef *hhrtim,
                                         uint32_t TimerIdx);
void HAL_HRTIM_Output1ResetCallback(HRTIM_HandleTypeDef *hhrtim,
                                           uint32_t TimerIdx);
void HAL_HRTIM_Output2SetCallback(HRTIM_HandleTypeDef *hhrtim,
                                         uint32_t TimerIdx);
void HAL_HRTIM_Output2ResetCallback(HRTIM_HandleTypeDef *hhrtim,
                                           uint32_t TimerIdx);
void HAL_HRTIM_BurstDMATransferCallback(HRTIM_HandleTypeDef *hhrtim,
                                               uint32_t TimerIdx);
void HAL_HRTIM_ErrorCallback(HRTIM_HandleTypeDef *hhrtim);

#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
HAL_StatusTypeDef HAL_HRTIM_RegisterCallback(HRTIM_HandleTypeDef *       hhrtim,
                                             HAL_HRTIM_CallbackIDTypeDef CallbackID,
                                             pHRTIM_CallbackTypeDef      pCallback);

HAL_StatusTypeDef HAL_HRTIM_UnRegisterCallback(HRTIM_HandleTypeDef *       hhrtim,
                                               HAL_HRTIM_CallbackIDTypeDef CallbackID);

HAL_StatusTypeDef HAL_HRTIM_TIMxRegisterCallback(HRTIM_HandleTypeDef *        hhrtim,
                                                 HAL_HRTIM_CallbackIDTypeDef  CallbackID,
                                                 pHRTIM_TIMxCallbackTypeDef   pCallback);

HAL_StatusTypeDef HAL_HRTIM_TIMxUnRegisterCallback(HRTIM_HandleTypeDef *       hhrtim,
                                                   HAL_HRTIM_CallbackIDTypeDef CallbackID);

#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
/**
  * end of HRTIM_Function_Definitions @}
  */


/**
  * end of HRTIM @}
  */

#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_HRTIM_H_ */
