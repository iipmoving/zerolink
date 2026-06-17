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
*   File    : rx32g4xx_hal_can.h
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _RX32G4xx_HAL_CAN_H_
#define _RX32G4xx_HAL_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"

/** @addtogroup CAN
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/******************************************************************************/
/*                            CAN Structures                                  */
/******************************************************************************/

/** @defgroup CAN_Structure_Definitions CAN Structure Definitions
  * @{
  */

/** @defgroup HAL_CAN_StateTypeDef HAL CAN StateTypeDef
  * @ingroup  CAN_Structure_Definitions
  * @{
  */

/**
  * @brief  HAL State structures definition
  */
typedef enum
{
  HAL_CAN_STATE_RESET             = 0x00U,  /*!< CAN not yet initialized or disabled */
  HAL_CAN_STATE_READY             = 0x01U,  /*!< CAN initialized and ready for use   */
  HAL_CAN_STATE_LISTENING         = 0x02U,  /*!< CAN receive process is ongoing      */
  HAL_CAN_STATE_SLEEP_PENDING     = 0x03U,  /*!< CAN sleep request is pending        */
  HAL_CAN_STATE_SLEEP_ACTIVE      = 0x04U,  /*!< CAN sleep mode is active            */
  HAL_CAN_STATE_ERROR             = 0x05U   /*!< CAN error state                     */

} HAL_CAN_StateTypeDef;

/**
  * end of HAL_CAN_StateTypeDef @}
  */

/** @defgroup CAN_InitTypeDef CAN InitTypeDef
  * @ingroup  CAN_Structure_Definitions
  * @{
  */

/**
  * @brief  CAN init structure definition
  */

typedef struct
{
  uint32_t Prescaler;                  /*!< Specifies the length of a time quantum.
                                            This parameter must be a number between Min_Data = 1 and Max_Data = 1024. */

  uint32_t Mode;                       /*!< Specifies the CAN operating mode.
                                            This parameter can be a value of @ref CAN_operating_mode */

  uint32_t SyncJumpWidth;              /*!< Specifies the maximum number of time quanta the CAN hardware
                                            is allowed to lengthen or shorten a bit to perform resynchronization.
                                            This parameter can be a value of @ref CAN_synchronisation_jump_width */

  uint32_t TimeSeg1;                   /*!< Specifies the number of time quanta in Bit Segment 1.
                                            This parameter can be a value of @ref CAN_time_quantum_in_bit_segment_1 */

  uint32_t TimeSeg2;                   /*!< Specifies the number of time quanta in Bit Segment 2.
                                            This parameter can be a value of @ref CAN_time_quantum_in_bit_segment_2 */

  FunctionalState TimeTriggeredMode;   /*!< Enable or disable the time triggered communication mode.
                                            This parameter can be set to ENABLE or DISABLE. */

  FunctionalState AutoBusOff;          /*!< Enable or disable the automatic bus-off management.
                                            This parameter can be set to ENABLE or DISABLE. */

  FunctionalState AutoWakeUp;          /*!< Enable or disable the automatic wake-up mode.
                                            This parameter can be set to ENABLE or DISABLE. */

  FunctionalState AutoRetransmission;  /*!< Enable or disable the non-automatic retransmission mode.
                                            This parameter can be set to ENABLE or DISABLE. */

  FunctionalState ReceiveFifoLocked;   /*!< Enable or disable the Receive __FIFO__ Locked mode.
                                            This parameter can be set to ENABLE or DISABLE. */

  FunctionalState TransmitFifoPriority;/*!< Enable or disable the transmit __FIFO__ priority.
                                            This parameter can be set to ENABLE or DISABLE. */

} CAN_InitTypeDef;

/**
  * end of CAN_InitTypeDef @}
  */

/** @defgroup CAN_FilterTypeDef CAN FilterTypeDef
  * @ingroup  CAN_Structure_Definitions
  * @{
  */

/**
  * @brief  CAN filter configuration structure definition
  */

typedef struct
{
  uint32_t FilterIdHigh;          /*!< Specifies the filter identification number (MSBs for a 32-bit
                                       configuration, first one for a 16-bit configuration).
                                       This parameter must be a number between
                                       Min_Data = 0x0000 and Max_Data = 0xFFFF. */

  uint32_t FilterIdLow;           /*!< Specifies the filter identification number (LSBs for a 32-bit
                                       configuration, second one for a 16-bit configuration).
                                       This parameter must be a number between
                                       Min_Data = 0x0000 and Max_Data = 0xFFFF. */

  uint32_t FilterMaskIdHigh;      /*!< Specifies the filter mask number or identification number,
                                       according to the mode (MSBs for a 32-bit configuration,
                                       first one for a 16-bit configuration).
                                       This parameter must be a number between
                                       Min_Data = 0x0000 and Max_Data = 0xFFFF. */

  uint32_t FilterMaskIdLow;       /*!< Specifies the filter mask number or identification number,
                                       according to the mode (LSBs for a 32-bit configuration,
                                       second one for a 16-bit configuration).
                                       This parameter must be a number between
                                       Min_Data = 0x0000 and Max_Data = 0xFFFF. */

  uint32_t FilterFIFOAssignment;  /*!< Specifies the __FIFO__ (0 or 1U) which will be assigned to the filter.
                                       This parameter can be a value of @ref CAN_filter_FIFO */

  uint32_t FilterBank;            /*!< Specifies the filter bank which will be initialized.
                                       For single CAN instance(14 dedicated filter banks),
                                       this parameter must be a number between Min_Data = 0 and Max_Data = 13.
                                       For dual CAN instances(28 filter banks shared),
                                       this parameter must be a number between Min_Data = 0 and Max_Data = 27. */

  uint32_t FilterMode;            /*!< Specifies the filter mode to be initialized.
                                       This parameter can be a value of @ref CAN_filter_mode */

  uint32_t FilterScale;           /*!< Specifies the filter scale.
                                       This parameter can be a value of @ref CAN_filter_scale */

  uint32_t FilterActivation;      /*!< Enable or disable the filter.
                                       This parameter can be a value of @ref CAN_filter_activation */

  uint32_t SlaveStartFilterBank;  /*!< Select the start filter bank for the slave CAN instance.
                                       For single CAN instances, this parameter is meaningless.
                                       For dual CAN instances, all filter banks with lower index are assigned to master
                                       CAN instance, whereas all filter banks with greater index are assigned to slave
                                       CAN instance.
                                       This parameter must be a number between Min_Data = 0 and Max_Data = 27. */

} CAN_FilterTypeDef;

typedef struct
{
  uint16_t FilterRTR1;           //0:DATA,1:REMOTE
  uint16_t FilterRTR2;           //0:DATA,1:REMOTE
  uint16_t FilterRTR3;           //0:DATA,1:REMOTE
  uint16_t FilterRTR4;           //0:DATA,1:REMOTE
  
  uint16_t FilterID1;    
  uint16_t FilterID2;    
  uint16_t FilterID3;    
  uint16_t FilterID4;    
  
  uint32_t FilterIDE1;           //0£ºSTID,1£ºEXID                            
  uint32_t FilterIDE2;           //0£ºSTID,1£ºEXID  
  uint32_t FilterIDE3;           //0£ºSTID,1£ºEXID                            
  uint32_t FilterIDE4;           //0£ºSTID,1£ºEXID  
                                 

  uint32_t FilterIdHigh;          /*!< Specifies the filter identification number (MSBs for a 32-bit
                                       configuration, first one for a 16-bit configuration).
                                       This parameter must be a number between
                                       Min_Data = 0x0000 and Max_Data = 0xFFFF. */

  uint32_t FilterIdLow;           /*!< Specifies the filter identification number (LSBs for a 32-bit
                                       configuration, second one for a 16-bit configuration).
                                       This parameter must be a number between
                                       Min_Data = 0x0000 and Max_Data = 0xFFFF. */

  uint32_t FilterMaskIdHigh;      /*!< Specifies the filter mask number or identification number,
                                       according to the mode (MSBs for a 32-bit configuration,
                                       first one for a 16-bit configuration).
                                       This parameter must be a number between
                                       Min_Data = 0x0000 and Max_Data = 0xFFFF. */

  uint32_t FilterMaskIdLow;       /*!< Specifies the filter mask number or identification number,
                                       according to the mode (LSBs for a 32-bit configuration,
                                       second one for a 16-bit configuration).
                                       This parameter must be a number between
                                       Min_Data = 0x0000 and Max_Data = 0xFFFF. */

  uint32_t FilterFIFOAssignment;  /*!< Specifies the __FIFO__ (0 or 1U) which will be assigned to the filter.
                                       This parameter can be a value of @ref CAN_filter_FIFO */

  uint32_t FilterBank;            /*!< Specifies the filter bank which will be initialized.
                                       For single CAN instance(14 dedicated filter banks),
                                       this parameter must be a number between Min_Data = 0 and Max_Data = 13.
                                       For dual CAN instances(28 filter banks shared),
                                       this parameter must be a number between Min_Data = 0 and Max_Data = 27. */

  uint32_t FilterMode;            /*!< Specifies the filter mode to be initialized.
                                       This parameter can be a value of @ref CAN_filter_mode */

  uint32_t FilterScale;           /*!< Specifies the filter scale.
                                       This parameter can be a value of @ref CAN_filter_scale */

  uint32_t FilterActivation;      /*!< Enable or disable the filter.
                                       This parameter can be a value of @ref CAN_filter_activation */

  uint32_t SlaveStartFilterBank;  /*!< Select the start filter bank for the slave CAN instance.
                                       For single CAN instances, this parameter is meaningless.
                                       For dual CAN instances, all filter banks with lower index are assigned to master
                                       CAN instance, whereas all filter banks with greater index are assigned to slave
                                       CAN instance.
                                       This parameter must be a number between Min_Data = 0 and Max_Data = 27. */

} CANEx_FilterTypeDef;

/**
  * end of CAN_FilterTypeDef @}
  */

/** @defgroup CAN_TxHeaderTypeDef CAN TxHeaderTypeDef
  * @ingroup  CAN_Structure_Definitions
  * @{
  */

/**
  * @brief  CAN Tx message header structure definition
  */
typedef struct
{
  uint32_t StdId;    /*!< Specifies the standard identifier.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0x7FF. */

  uint32_t ExtId;    /*!< Specifies the extended identifier.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0x1FFFFFFF. */

  uint32_t IDE;      /*!< Specifies the type of identifier for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_identifier_type */

  uint32_t RTR;      /*!< Specifies the type of frame for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_remote_transmission_request */

  uint32_t DLC;      /*!< Specifies the length of the frame that will be transmitted.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 8. */

  FunctionalState TransmitGlobalTime; /*!< Specifies whether the timestamp counter value captured on start
                          of frame transmission, is sent in DATA6 and DATA7 replacing pData[6] and pData[7].
                          @note  Time Triggered Communication Mode must be enabled.
                          @note  __DLC__ must be programmed as 8 bytes, in order these 2 bytes are sent.
                          This parameter can be set to ENABLE or DISABLE. */

} CAN_TxHeaderTypeDef;

/**
  * end of CAN_TxHeaderTypeDef @}
  */

/** @defgroup CAN_RxHeaderTypeDef CAN RxHeaderTypeDef
  * @ingroup  CAN_Structure_Definitions
  * @{
  */

/**
  * @brief  CAN Rx message header structure definition
  */
typedef struct
{
  uint32_t StdId;    /*!< Specifies the standard identifier.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0x7FF. */

  uint32_t ExtId;    /*!< Specifies the extended identifier.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0x1FFFFFFF. */

  uint32_t IDE;      /*!< Specifies the type of identifier for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_identifier_type */

  uint32_t RTR;      /*!< Specifies the type of frame for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_remote_transmission_request */

  uint32_t DLC;      /*!< Specifies the length of the frame that will be transmitted.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 8. */

  uint32_t Timestamp; /*!< Specifies the timestamp counter value captured on start of frame reception.
                          @note  Time Triggered Communication Mode must be enabled.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0xFFFF. */

  uint32_t FilterMatchIndex; /*!< Specifies the index of matching acceptance filter element.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0xFF. */

} CAN_RxHeaderTypeDef;

/**
  * end of CAN_RxHeaderTypeDef @}
  */

/** @defgroup CAN_HandleTypeDef CAN HandleTypeDef
  * @ingroup  CAN_Structure_Definitions
  * @{
  */

/**
  * @brief  CAN handle Structure definition
  */
typedef struct __HAL_CAN_HandleTypeDef
{
  CAN_TypeDef                 *Instance;                 /*!< Register base address */

  CAN_InitTypeDef             Init;                      /*!< CAN required parameters */

  __IO HAL_CAN_StateTypeDef   State;                     /*!< CAN communication state */

  __IO uint32_t               ErrorCode;                 /*!< CAN Error code.
                                                              This parameter can be a value of @ref CAN_Error_Code */

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
  void (* TxMailbox0CompleteCallback)(struct __HAL_CAN_HandleTypeDef *hcan);/*!< CAN Tx Mailbox 0 complete callback    */
  void (* TxMailbox1CompleteCallback)(struct __HAL_CAN_HandleTypeDef *hcan);/*!< CAN Tx Mailbox 1 complete callback    */
  void (* TxMailbox2CompleteCallback)(struct __HAL_CAN_HandleTypeDef *hcan);/*!< CAN Tx Mailbox 2 complete callback    */
  void (* TxMailbox0AbortCallback)(struct __HAL_CAN_HandleTypeDef *hcan);   /*!< CAN Tx Mailbox 0 abort callback       */
  void (* TxMailbox1AbortCallback)(struct __HAL_CAN_HandleTypeDef *hcan);   /*!< CAN Tx Mailbox 1 abort callback       */
  void (* TxMailbox2AbortCallback)(struct __HAL_CAN_HandleTypeDef *hcan);   /*!< CAN Tx Mailbox 2 abort callback       */
  void (* RxFifo0MsgPendingCallback)(struct __HAL_CAN_HandleTypeDef *hcan); /*!< CAN Rx __FIFO__ 0 msg pending callback    */
  void (* RxFifo0FullCallback)(struct __HAL_CAN_HandleTypeDef *hcan);       /*!< CAN Rx __FIFO__ 0 full callback           */
  void (* RxFifo1MsgPendingCallback)(struct __HAL_CAN_HandleTypeDef *hcan); /*!< CAN Rx __FIFO__ 1 msg pending callback    */
  void (* RxFifo1FullCallback)(struct __HAL_CAN_HandleTypeDef *hcan);       /*!< CAN Rx __FIFO__ 1 full callback           */
  void (* SleepCallback)(struct __HAL_CAN_HandleTypeDef *hcan);             /*!< CAN Sleep callback                    */
  void (* WakeUpFromRxMsgCallback)(struct __HAL_CAN_HandleTypeDef *hcan);   /*!< CAN Wake Up from Rx msg callback      */
  void (* ErrorCallback)(struct __HAL_CAN_HandleTypeDef *hcan);             /*!< CAN Error callback                    */

  void (* MspInitCallback)(struct __HAL_CAN_HandleTypeDef *hcan);           /*!< CAN Msp Init callback                 */
  void (* MspDeInitCallback)(struct __HAL_CAN_HandleTypeDef *hcan);         /*!< CAN Msp DeInit callback               */

#endif /* (USE_HAL_CAN_REGISTER_CALLBACKS) */
} CAN_HandleTypeDef;

/**
  * end of CAN_HandleTypeDef @}
  */

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1

/** @defgroup HAL_CAN_CallbackIDTypeDef HAL CAN CallbackIDTypeDef
  * @ingroup  CAN_Structure_Definitions
  * @{
  */

/**
  * @brief  HAL CAN common Callback ID enumeration definition
  */
typedef enum
{
  HAL_CAN_TX_MAILBOX0_COMPLETE_CB_ID       = 0x00U,    /*!< CAN Tx Mailbox 0 complete callback ID         */
  HAL_CAN_TX_MAILBOX1_COMPLETE_CB_ID       = 0x01U,    /*!< CAN Tx Mailbox 1 complete callback ID         */
  HAL_CAN_TX_MAILBOX2_COMPLETE_CB_ID       = 0x02U,    /*!< CAN Tx Mailbox 2 complete callback ID         */
  HAL_CAN_TX_MAILBOX0_ABORT_CB_ID          = 0x03U,    /*!< CAN Tx Mailbox 0 abort callback ID            */
  HAL_CAN_TX_MAILBOX1_ABORT_CB_ID          = 0x04U,    /*!< CAN Tx Mailbox 1 abort callback ID            */
  HAL_CAN_TX_MAILBOX2_ABORT_CB_ID          = 0x05U,    /*!< CAN Tx Mailbox 2 abort callback ID            */
  HAL_CAN_RX_FIFO0_MSG_PENDING_CB_ID       = 0x06U,    /*!< CAN Rx __FIFO__ 0 message pending callback ID     */
  HAL_CAN_RX_FIFO0_FULL_CB_ID              = 0x07U,    /*!< CAN Rx __FIFO__ 0 full callback ID                */
  HAL_CAN_RX_FIFO1_MSG_PENDING_CB_ID       = 0x08U,    /*!< CAN Rx __FIFO__ 1 message pending callback ID     */
  HAL_CAN_RX_FIFO1_FULL_CB_ID              = 0x09U,    /*!< CAN Rx __FIFO__ 1 full callback ID                */
  HAL_CAN_SLEEP_CB_ID                      = 0x0AU,    /*!< CAN Sleep callback ID                         */
  HAL_CAN_WAKEUP_FROM_RX_MSG_CB_ID         = 0x0BU,    /*!< CAN Wake Up from Rx msg callback ID          */
  HAL_CAN_ERROR_CB_ID                      = 0x0CU,    /*!< CAN Error callback ID                         */

  HAL_CAN_MSPINIT_CB_ID                    = 0x0DU,    /*!< CAN MspInit callback ID                       */
  HAL_CAN_MSPDEINIT_CB_ID                  = 0x0EU,    /*!< CAN MspDeInit callback ID                     */

} HAL_CAN_CallbackIDTypeDef;

/**
  * end of HAL_CAN_CallbackIDTypeDef @}
  */

/** @defgroup pCAN_CallbackTypeDef pCAN CallbackTypeDef
  * @ingroup  CAN_Structure_Definitions
  * @{
  */

/**
  * @brief  HAL CAN Callback pointer definition
  */
typedef  void (*pCAN_CallbackTypeDef)(CAN_HandleTypeDef *hcan); /*!< pointer to a CAN callback function   */


/**
  * end of pCAN_CallbackTypeDef @}
  */

#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
/**
  * end of CAN_Structure_Definitions @}
  */
  
/* Exported constants --------------------------------------------------------*/

/******************************************************************************/
/*                              CAN Parameters                                */
/******************************************************************************/

/** @defgroup CAN_Parameter_Definitions CAN Parameter Definitions
  * @{
  */

/** @defgroup CAN_Error_Code CAN Error Code
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define HAL_CAN_ERROR_NONE            (0x00000000U)            /*!< No error                                             */
#define HAL_CAN_ERROR_EWG             (0x00000001U)            /*!< Protocol Error Warning                               */
#define HAL_CAN_ERROR_EPV             (0x00000002U)            /*!< Error Passive                                        */
#define HAL_CAN_ERROR_BOF             (0x00000004U)            /*!< Bus-off error                                        */
#define HAL_CAN_ERROR_STF             (0x00000008U)            /*!< Stuff error                                          */
#define HAL_CAN_ERROR_FOR             (0x00000010U)            /*!< Form error                                           */
#define HAL_CAN_ERROR_ACK             (0x00000020U)            /*!< Acknowledgment error                                 */
#define HAL_CAN_ERROR_BR              (0x00000040U)            /*!< Bit recessive error                                  */
#define HAL_CAN_ERROR_BD              (0x00000080U)            /*!< Bit dominant error                                   */
#define HAL_CAN_ERROR_CRC             (0x00000100U)            /*!< CRC error                                            */
#define HAL_CAN_ERROR_RX_FOV0         (0x00000200U)            /*!< Rx FIFO0 overrun error                               */
#define HAL_CAN_ERROR_RX_FOV1         (0x00000400U)            /*!< Rx FIFO1 overrun error                               */
#define HAL_CAN_ERROR_TX_ALST0        (0x00000800U)            /*!< TxMailbox 0 transmit failure due to arbitration lost */
#define HAL_CAN_ERROR_TX_TERR0        (0x00001000U)            /*!< TxMailbox 0 transmit failure due to transmit error   */
#define HAL_CAN_ERROR_TX_ALST1        (0x00002000U)            /*!< TxMailbox 1 transmit failure due to arbitration lost */
#define HAL_CAN_ERROR_TX_TERR1        (0x00004000U)            /*!< TxMailbox 1 transmit failure due to transmit error   */
#define HAL_CAN_ERROR_TX_ALST2        (0x00008000U)            /*!< TxMailbox 2 transmit failure due to arbitration lost */
#define HAL_CAN_ERROR_TX_TERR2        (0x00010000U)            /*!< TxMailbox 2 transmit failure due to transmit error   */
#define HAL_CAN_ERROR_TIMEOUT         (0x00020000U)            /*!< Timeout error                                        */
#define HAL_CAN_ERROR_NOT_INITIALIZED (0x00040000U)            /*!< Peripheral not initialized                           */
#define HAL_CAN_ERROR_NOT_READY       (0x00080000U)            /*!< Peripheral not ready                                 */
#define HAL_CAN_ERROR_NOT_STARTED     (0x00100000U)            /*!< Peripheral not started                               */
#define HAL_CAN_ERROR_PARAM           (0x00200000U)            /*!< Parameter error                                      */

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
#define HAL_CAN_ERROR_INVALID_CALLBACK (0x00400000U) /*!< Invalid Callback error                               */
#endif /* USE_HAL_CAN_REGISTER_CALLBACKS */
#define HAL_CAN_ERROR_INTERNAL        (0x00800000U)  /*!< Internal error                                       */

/**
  * end of CAN_Error_Code @}
  */

/** @defgroup CAN_InitStatus CAN InitStatus
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_INITSTATUS_FAILED       (0x00000000U)  /*!< CAN initialization failed */
#define CAN_INITSTATUS_SUCCESS      (0x00000001U)  /*!< CAN initialization OK     */
/**
  * end of CAN_Error_Code @}
  */

/** @defgroup CAN_operating_mode CAN Operating Mode
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_MODE_NORMAL             (0x00000000U)                              /*!< Normal mode   */
#define CAN_MODE_LOOPBACK           ((uint32_t)CAN_BTR_LBKM)                   /*!< Loopback mode */
#define CAN_MODE_SILENT             ((uint32_t)CAN_BTR_SILM)                   /*!< Silent mode   */
#define CAN_MODE_SILENT_LOOPBACK    ((uint32_t)(CAN_BTR_LBKM | CAN_BTR_SILM))  /*!< Loopback combined with
                                                                                    silent mode   */
/**
  * end of CAN_operating_mode @}
  */


/** @defgroup CAN_synchronisation_jump_width CAN Synchronization Jump Width
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_SJW_1TQ                 (0x00000000U)              /*!< 1 time quantum */
#define CAN_SJW_2TQ                 ((uint32_t)CAN_BTR_SJW_BIT0)  /*!< 2 time quantum */
#define CAN_SJW_3TQ                 ((uint32_t)CAN_BTR_SJW_BIT1)  /*!< 3 time quantum */
#define CAN_SJW_4TQ                 ((uint32_t)CAN_BTR_SJW)    /*!< 4 time quantum */
/**
  * end of CAN_synchronisation_jump_width @}
  */

/** @defgroup CAN_time_quantum_in_bit_segment_1 CAN Time Quantum in Bit Segment 1
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_BS1_1TQ                 (0x00000000U)                                                                 /*!< 1  time quantum */
#define CAN_BS1_2TQ                 (                                                         CAN_BTR_TS1_BIT0)   /*!< 2  time quantum */
#define CAN_BS1_3TQ                 (                                      CAN_BTR_TS1_BIT1                   )   /*!< 3  time quantum */
#define CAN_BS1_4TQ                 (                                      CAN_BTR_TS1_BIT1 | CAN_BTR_TS1_BIT0)   /*!< 4  time quantum */
#define CAN_BS1_5TQ                 (                   CAN_BTR_TS1_BIT2                                      )   /*!< 5  time quantum */
#define CAN_BS1_6TQ                 (                   CAN_BTR_TS1_BIT2 |                    CAN_BTR_TS1_BIT0)   /*!< 6  time quantum */
#define CAN_BS1_7TQ                 (                   CAN_BTR_TS1_BIT2 | CAN_BTR_TS1_BIT1                   )   /*!< 7  time quantum */
#define CAN_BS1_8TQ                 (                   CAN_BTR_TS1_BIT2 | CAN_BTR_TS1_BIT1 | CAN_BTR_TS1_BIT0)   /*!< 8  time quantum */
#define CAN_BS1_9TQ                 (CAN_BTR_TS1_BIT3                                                         )   /*!< 9  time quantum */
#define CAN_BS1_10TQ                (CAN_BTR_TS1_BIT3 |                                       CAN_BTR_TS1_BIT0)   /*!< 10 time quantum */
#define CAN_BS1_11TQ                (CAN_BTR_TS1_BIT3 |                    CAN_BTR_TS1_BIT1                   )   /*!< 11 time quantum */
#define CAN_BS1_12TQ                (CAN_BTR_TS1_BIT3 |                    CAN_BTR_TS1_BIT1 | CAN_BTR_TS1_BIT0)   /*!< 12 time quantum */
#define CAN_BS1_13TQ                (CAN_BTR_TS1_BIT3 | CAN_BTR_TS1_BIT2                                      )   /*!< 13 time quantum */
#define CAN_BS1_14TQ                (CAN_BTR_TS1_BIT3 | CAN_BTR_TS1_BIT2 |                    CAN_BTR_TS1_BIT0)   /*!< 14 time quantum */
#define CAN_BS1_15TQ                (CAN_BTR_TS1_BIT3 | CAN_BTR_TS1_BIT2 | CAN_BTR_TS1_BIT1                   )   /*!< 15 time quantum */
#define CAN_BS1_16TQ                (CAN_BTR_TS1_BIT3 | CAN_BTR_TS1_BIT2 | CAN_BTR_TS1_BIT1 | CAN_BTR_TS1_BIT0)   /*!< 16 time quantum */

/**
  * end of CAN_time_quantum_in_bit_segment_1 @}
  */

/** @defgroup CAN_time_quantum_in_bit_segment_2 CAN Time Quantum in Bit Segment 2
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_BS2_1TQ                     (0x00000000U)                                              /*!< 1 time quantum */
#define CAN_BS2_2TQ                     (                                      CAN_BTR_TS2_BIT0)   /*!< 2 time quantum */
#define CAN_BS2_3TQ                     (                   CAN_BTR_TS2_BIT1                   )   /*!< 3 time quantum */
#define CAN_BS2_4TQ                     (                   CAN_BTR_TS2_BIT1 | CAN_BTR_TS2_BIT0)   /*!< 4 time quantum */
#define CAN_BS2_5TQ                     (CAN_BTR_TS2_BIT2                                      )   /*!< 5 time quantum */
#define CAN_BS2_6TQ                     (CAN_BTR_TS2_BIT2 |                    CAN_BTR_TS2_BIT0)   /*!< 6 time quantum */
#define CAN_BS2_7TQ                     (CAN_BTR_TS2_BIT2 | CAN_BTR_TS2_BIT1                   )   /*!< 7 time quantum */
#define CAN_BS2_8TQ                     (CAN_BTR_TS2_BIT2 | CAN_BTR_TS2_BIT1 | CAN_BTR_TS2_BIT0)   /*!< 8 time quantum */

/**
  * end of CAN_time_quantum_in_bit_segment_2 @}
  */

/** @defgroup CAN_filter_mode CAN Filter Mode
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_FILTERMODE_IDMASK       (0x00000000U)  /*!< Identifier mask mode */
#define CAN_FILTERMODE_IDLIST       (0x00000001U)  /*!< Identifier list mode */
/**
  * end of CAN_filter_mode @}
  */

/** @defgroup CAN_filter_scale CAN Filter Scale
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_FILTERSCALE_16BIT       (0x00000000U)  /*!< Two 16-bit filters */
#define CAN_FILTERSCALE_32BIT       (0x00000001U)  /*!< One 32-bit filter  */
/**
  * end of CAN_filter_scale @}
  */

/** @defgroup CAN_filter_activation CAN Filter Activation
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_FILTER_DISABLE          (0x00000000U)  /*!< Disable filter */
#define CAN_FILTER_ENABLE           (0x00000001U)  /*!< Enable filter  */
/**
  * end of CAN_filter_activation @}
  */

/** @defgroup CAN_filter_FIFO CAN Filter FIFO
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_FILTER_FIFO0            (0x00000000U)  /*!< Filter __FIFO__ 0 assignment for filter x */
#define CAN_FILTER_FIFO1            (0x00000001U)  /*!< Filter __FIFO__ 1 assignment for filter x */
/**
  * end of CAN_filter_FIFO @}
  */


/** @defgroup CAN_identifier_type CAN Identifier Type
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_ID_STD                  (0x00000000U)  /*!< Standard Id */
#define CAN_ID_EXT                  (0x00000004U)  /*!< Extended Id */
/**
  * end of CAN_identifier_type @}
  */

/** @defgroup CAN_remote_transmission_request CAN Remote Transmission Request
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_RTR_DATA                (0x00000000U)  /*!< Data frame   */
#define CAN_RTR_REMOTE              (0x00000002U)  /*!< Remote frame */
/**
  * end of CAN_remote_transmission_request @}
  */

/** @defgroup CAN_receive_FIFO_number CAN Receive FIFO Number
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_RX_FIFO0                (0x00000000U)  /*!< CAN receive __FIFO__ 0 */
#define CAN_RX_FIFO1                (0x00000001U)  /*!< CAN receive __FIFO__ 1 */
/**
  * end of CAN_receive_FIFO_number @}
  */

/** @defgroup CAN_Tx_Mailboxes CAN Tx Mailboxes
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_TX_MAILBOX0             (0x00000001U)  /*!< Tx Mailbox 0  */
#define CAN_TX_MAILBOX1             (0x00000002U)  /*!< Tx Mailbox 1  */
#define CAN_TX_MAILBOX2             (0x00000004U)  /*!< Tx Mailbox 2  */
/**
  * end of CAN_Tx_Mailboxes @}
  */

/** @defgroup CAN_flags CAN Flags
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
/* Transmit Flags */
#define CAN_FLAG_RQCP0              (0x00000500U)  /*!< Request complete MailBox 0 flag   */
#define CAN_FLAG_TXOK0              (0x00000501U)  /*!< Transmission OK MailBox 0 flag    */
#define CAN_FLAG_ALST0              (0x00000502U)  /*!< Arbitration Lost MailBox 0 flag   */
#define CAN_FLAG_TERR0              (0x00000503U)  /*!< Transmission error MailBox 0 flag */
#define CAN_FLAG_RQCP1              (0x00000508U)  /*!< Request complete MailBox1 flag    */
#define CAN_FLAG_TXOK1              (0x00000509U)  /*!< Transmission OK MailBox 1 flag    */
#define CAN_FLAG_ALST1              (0x0000050AU)  /*!< Arbitration Lost MailBox 1 flag   */
#define CAN_FLAG_TERR1              (0x0000050BU)  /*!< Transmission error MailBox 1 flag */
#define CAN_FLAG_RQCP2              (0x00000510U)  /*!< Request complete MailBox2 flag    */
#define CAN_FLAG_TXOK2              (0x00000511U)  /*!< Transmission OK MailBox 2 flag    */
#define CAN_FLAG_ALST2              (0x00000512U)  /*!< Arbitration Lost MailBox 2 flag   */
#define CAN_FLAG_TERR2              (0x00000513U)  /*!< Transmission error MailBox 2 flag */
#define CAN_FLAG_TME0               (0x0000051AU)  /*!< Transmit mailbox 0 empty flag     */
#define CAN_FLAG_TME1               (0x0000051BU)  /*!< Transmit mailbox 1 empty flag     */
#define CAN_FLAG_TME2               (0x0000051CU)  /*!< Transmit mailbox 2 empty flag     */
#define CAN_FLAG_LOW0               (0x0000051DU)  /*!< Lowest priority mailbox 0 flag    */
#define CAN_FLAG_LOW1               (0x0000051EU)  /*!< Lowest priority mailbox 1 flag    */
#define CAN_FLAG_LOW2               (0x0000051FU)  /*!< Lowest priority mailbox 2 flag    */

/* Receive Flags */
#define CAN_FLAG_FF0                (0x00000203U)  /*!< RX __FIFO__ 0 Full flag               */
#define CAN_FLAG_FOV0               (0x00000204U)  /*!< RX __FIFO__ 0 Overrun flag            */
#define CAN_FLAG_FF1                (0x00000403U)  /*!< RX __FIFO__ 1 Full flag               */
#define CAN_FLAG_FOV1               (0x00000404U)  /*!< RX __FIFO__ 1 Overrun flag            */

/* Operating Mode Flags */
#define CAN_FLAG_INAK               (0x00000100U)  /*!< Initialization acknowledge flag   */
#define CAN_FLAG_SLAK               (0x00000101U)  /*!< Sleep acknowledge flag            */
#define CAN_FLAG_ERRI               (0x00000102U)  /*!< Error flag                        */
#define CAN_FLAG_WKU                (0x00000103U)  /*!< Wake up interrupt flag            */
#define CAN_FLAG_SLAKI              (0x00000104U)  /*!< Sleep acknowledge interrupt flag  */

/* Error Flags */
#define CAN_FLAG_EWG                (0x00000300U)  /*!< Error warning flag                */
#define CAN_FLAG_EPV                (0x00000301U)  /*!< Error passive flag                */
#define CAN_FLAG_BOF                (0x00000302U)  /*!< Bus-Off flag                      */
/**
  * end of CAN_flags @}
  */

/** @defgroup CAN_Interrupts CAN Interrupts
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
/* Transmit Interrupt */
#define CAN_IT_TX_MAILBOX_EMPTY     ((uint32_t)CAN_IER_TMEIE)   /*!< Transmit mailbox empty interrupt */

/* Receive Interrupts */
#define CAN_IT_RX_FIFO0_MSG_PENDING ((uint32_t)CAN_IER_FMPIE0)  /*!< __FIFO__ 0 message pending interrupt */
#define CAN_IT_RX_FIFO0_FULL        ((uint32_t)CAN_IER_FFIE0)   /*!< __FIFO__ 0 full interrupt            */
#define CAN_IT_RX_FIFO0_OVERRUN     ((uint32_t)CAN_IER_FOVIE0)  /*!< __FIFO__ 0 overrun interrupt         */
#define CAN_IT_RX_FIFO1_MSG_PENDING ((uint32_t)CAN_IER_FMPIE1)  /*!< __FIFO__ 1 message pending interrupt */
#define CAN_IT_RX_FIFO1_FULL        ((uint32_t)CAN_IER_FFIE1)   /*!< __FIFO__ 1 full interrupt            */
#define CAN_IT_RX_FIFO1_OVERRUN     ((uint32_t)CAN_IER_FOVIE1)  /*!< __FIFO__ 1 overrun interrupt         */

#define CAN_IT_RX_FIFO0_ALL         (CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO0_FULL | CAN_IT_RX_FIFO0_OVERRUN)
#define CAN_IT_RX_FIFO1_ALL         (CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_RX_FIFO1_FULL | CAN_IT_RX_FIFO1_OVERRUN)

#define CAN_IT_RX_ALL               (CAN_IT_RX_FIFO0_ALL | CAN_IT_RX_FIFO1_ALL)

/* Operating Mode Interrupts */
#define CAN_IT_WAKEUP               ((uint32_t)CAN_IER_WKUIE)   /*!< Wake-up interrupt                */
#define CAN_IT_SLEEP_ACK            ((uint32_t)CAN_IER_SLKIE)   /*!< Sleep acknowledge interrupt      */

/* Error Interrupts */
#define CAN_IT_ERROR_WARNING        ((uint32_t)CAN_IER_EWGIE)   /*!< Error warning interrupt          */
#define CAN_IT_ERROR_PASSIVE        ((uint32_t)CAN_IER_EPVIE)   /*!< Error passive interrupt          */
#define CAN_IT_BUSOFF               ((uint32_t)CAN_IER_BOFIE)   /*!< Bus-off interrupt                */
#define CAN_IT_LAST_ERROR_CODE      ((uint32_t)CAN_IER_LECIE)   /*!< Last error code interrupt        */
#define CAN_IT_ERROR                ((uint32_t)CAN_IER_ERRIE)   /*!< Error Interrupt                  */
/**
  * end of CAN_Interrupts @}
  */

/** @defgroup CAN_Private_Constants CAN Private Constants
  * @ingroup  CAN_Parameter_Definitions
  * @{
  */
#define CAN_FLAG_MASK  (0x000000FFU)
/**
  * end of CAN_Private_Constants @}
  */

/**
  * end of CAN_Parameter_Definitions @}
  */

/* Exported macros -----------------------------------------------------------*/

/******************************************************************************/
/*                              CAN Macro                                     */
/******************************************************************************/

/** @defgroup CAN_Macro_Definitions CAN Macro Definitions
  * @{
  */

/**
  * @brief  Reset CAN handle state
  * @param  __HANDLE__ CAN handle.
  * @retval None
  */
#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
#define __HAL_CAN_RESET_HANDLE_STATE(__HANDLE__) do{                                              \
                                                     (__HANDLE__)->State = HAL_CAN_STATE_RESET;   \
                                                     (__HANDLE__)->MspInitCallback = NULL;        \
                                                     (__HANDLE__)->MspDeInitCallback = NULL;      \
                                                   } while(0)
#else
#define __HAL_CAN_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_CAN_STATE_RESET)
#endif /*USE_HAL_CAN_REGISTER_CALLBACKS */

/**
  * @brief  Enable the specified CAN interrupts.
  * @param  __HANDLE__ CAN handle.
  * @param  __INTERRUPT__ CAN Interrupt sources to enable.
  *         This parameter can be any combination of
  *           @arg CAN_Interrupts
  * @retval None
  */
#define __HAL_CAN_ENABLE_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->IER) |= (__INTERRUPT__))

/**
  * @brief  Disable the specified CAN interrupts.
  * @param  __HANDLE__ CAN handle.
  * @param  __INTERRUPT__ CAN Interrupt sources to disable.
  *         This parameter can be any combination of
  *           @arg CAN_Interrupts
  * @retval None
  */
#define __HAL_CAN_DISABLE_IT(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->IER) &= ~(__INTERRUPT__))

/**
  * @brief  Check if the specified CAN interrupt source is enabled or disabled.
  * @param  __HANDLE__ specifies the CAN Handle.
  * @param  __INTERRUPT__ specifies the CAN interrupt source to check.
  *         This parameter can be a value of
  *           @arg CAN_Interrupts
  * @retval The state of __IT__ (TRUE or FALSE).
  */
#define __HAL_CAN_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) (((__HANDLE__)->Instance->IER) & (__INTERRUPT__))

/**
  * @brief  Check whether the specified CAN flag is set or not.
  * @param  __HANDLE__ specifies the CAN Handle.
  * @param  __FLAG__ specifies the flag to check.
  *         This parameter can be one of
  *           @arg CAN_flags
  * @retval The state of __FLAG__ (TRUE or FALSE).
  */
#define __HAL_CAN_GET_FLAG(__HANDLE__, __FLAG__) \
  ((((__FLAG__) >> 8U) == 5U)? ((((__HANDLE__)->Instance->TSR) & (1U << ((__FLAG__) & CAN_FLAG_MASK))) == (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 2U)? ((((__HANDLE__)->Instance->RF0R) & (1U << ((__FLAG__) & CAN_FLAG_MASK))) == (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 4U)? ((((__HANDLE__)->Instance->RF1R) & (1U << ((__FLAG__) & CAN_FLAG_MASK))) == (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 1U)? ((((__HANDLE__)->Instance->MSR) & (1U << ((__FLAG__) & CAN_FLAG_MASK))) == (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 3U)? ((((__HANDLE__)->Instance->ESR) & (1U << ((__FLAG__) & CAN_FLAG_MASK))) == (1U << ((__FLAG__) & CAN_FLAG_MASK))): 0U)

/**
  * @brief  Clear the specified CAN pending flag.
  * @param  __HANDLE__ specifies the CAN Handle.
  * @param  __FLAG__ specifies the flag to check.
  *         This parameter can be one of the following values:
  *           @arg CAN_FLAG_RQCP0: Request complete MailBox 0 Flag
  *           @arg CAN_FLAG_TXOK0: Transmission OK MailBox 0 Flag
  *           @arg CAN_FLAG_ALST0: Arbitration Lost MailBox 0 Flag
  *           @arg CAN_FLAG_TERR0: Transmission error MailBox 0 Flag
  *           @arg CAN_FLAG_RQCP1: Request complete MailBox 1 Flag
  *           @arg CAN_FLAG_TXOK1: Transmission OK MailBox 1 Flag
  *           @arg CAN_FLAG_ALST1: Arbitration Lost MailBox 1 Flag
  *           @arg CAN_FLAG_TERR1: Transmission error MailBox 1 Flag
  *           @arg CAN_FLAG_RQCP2: Request complete MailBox 2 Flag
  *           @arg CAN_FLAG_TXOK2: Transmission OK MailBox 2 Flag
  *           @arg CAN_FLAG_ALST2: Arbitration Lost MailBox 2 Flag
  *           @arg CAN_FLAG_TERR2: Transmission error MailBox 2 Flag
  *           @arg CAN_FLAG_FF0:   RX __FIFO__ 0 Full Flag
  *           @arg CAN_FLAG_FOV0:  RX __FIFO__ 0 Overrun Flag
  *           @arg CAN_FLAG_FF1:   RX __FIFO__ 1 Full Flag
  *           @arg CAN_FLAG_FOV1:  RX __FIFO__ 1 Overrun Flag
  *           @arg CAN_FLAG_WKUI:  Wake up Interrupt Flag
  *           @arg CAN_FLAG_SLAKI: Sleep acknowledge Interrupt Flag
  * @retval None
  */
#define __HAL_CAN_CLEAR_FLAG(__HANDLE__, __FLAG__) \
  ((((__FLAG__) >> 8U) == 5U)? (((__HANDLE__)->Instance->TSR) = (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 2U)? (((__HANDLE__)->Instance->RF0R) = (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 4U)? (((__HANDLE__)->Instance->RF1R) = (1U << ((__FLAG__) & CAN_FLAG_MASK))): \
   (((__FLAG__) >> 8U) == 1U)? (((__HANDLE__)->Instance->MSR) = (1U << ((__FLAG__) & CAN_FLAG_MASK))): 0U)

/**
  * @brief  Check if the parameter __MODE__ is valid
  * @param  __MODE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_MODE(__MODE__)                    (((__MODE__) == CAN_MODE_NORMAL) ||  \
                                                  ((__MODE__) == CAN_MODE_LOOPBACK)|| \
                                                  ((__MODE__) == CAN_MODE_SILENT) ||  \
                                                  ((__MODE__) == CAN_MODE_SILENT_LOOPBACK))

/**
  * @brief  Check if the parameter __SJW__ is valid
  * @param  __SJW__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_SJW(__SJW__)                      (((__SJW__) == CAN_SJW_1TQ) || ((__SJW__) == CAN_SJW_2TQ) || \
                                                  ((__SJW__) == CAN_SJW_3TQ) || ((__SJW__) == CAN_SJW_4TQ))

/**
  * @brief  Check if the parameter __BS1__ is valid
  * @param  __BS1__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_BS1(__BS1__)                      (((__BS1__) == CAN_BS1_1TQ) || ((__BS1__) == CAN_BS1_2TQ) || \
                                                  ((__BS1__) == CAN_BS1_3TQ) || ((__BS1__) == CAN_BS1_4TQ) || \
                                                  ((__BS1__) == CAN_BS1_5TQ) || ((__BS1__) == CAN_BS1_6TQ) || \
                                                  ((__BS1__) == CAN_BS1_7TQ) || ((__BS1__) == CAN_BS1_8TQ) || \
                                                  ((__BS1__) == CAN_BS1_9TQ) || ((__BS1__) == CAN_BS1_10TQ)|| \
                                                  ((__BS1__) == CAN_BS1_11TQ)|| ((__BS1__) == CAN_BS1_12TQ)|| \
                                                  ((__BS1__) == CAN_BS1_13TQ)|| ((__BS1__) == CAN_BS1_14TQ)|| \
                                                  ((__BS1__) == CAN_BS1_15TQ)|| ((__BS1__) == CAN_BS1_16TQ))

/**
  * @brief  Check if the parameter __BS2__ is valid
  * @param  __BS2__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_BS2(__BS2__)                      (((__BS2__) == CAN_BS2_1TQ) || ((__BS2__) == CAN_BS2_2TQ) || \
                                                  ((__BS2__) == CAN_BS2_3TQ) || ((__BS2__) == CAN_BS2_4TQ) || \
                                                  ((__BS2__) == CAN_BS2_5TQ) || ((__BS2__) == CAN_BS2_6TQ) || \
                                                  ((__BS2__) == CAN_BS2_7TQ) || ((__BS2__) == CAN_BS2_8TQ))

/**
  * @brief  Check if the parameter __PRESCALER__ is valid
  * @param  __PRESCALER__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_PRESCALER(__PRESCALER__)          (((__PRESCALER__) >= 1U) && ((__PRESCALER__) <= 1024U))

/**
  * @brief  Check if the parameter __HALFWORD__ is valid
  * @param  __HALFWORD__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_FILTER_ID_HALFWORD(__HALFWORD__)  ((__HALFWORD__) <= 0xFFFFU)

/**
  * @brief  Check if the parameter __BANK__ is valid
  * @param  __BANK__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */

#define IS_CAN_FILTER_BANK_SINGLE(__BANK__)      ((__BANK__) <= 13U)

/**
  * @brief  Check if the parameter __MODE__ is valid
  * @param  __MODE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_FILTER_MODE(__MODE__)             (((__MODE__) == CAN_FILTERMODE_IDMASK) || \
                                                  ((__MODE__) == CAN_FILTERMODE_IDLIST))

/**
  * @brief  Check if the parameter __SCALE__ is valid
  * @param  __SCALE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_FILTER_SCALE(__SCALE__)           (((__SCALE__) == CAN_FILTERSCALE_16BIT) || \
                                                  ((__SCALE__) == CAN_FILTERSCALE_32BIT))

/**
  * @brief  Check if the parameter __ACTIVATION__ is valid
  * @param  __ACTIVATION__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_FILTER_ACTIVATION(__ACTIVATION__) (((__ACTIVATION__) == CAN_FILTER_DISABLE) || \
                                                  ((__ACTIVATION__) == CAN_FILTER_ENABLE))

/**
  * @brief  Check if the parameter __FIFO__ is valid
  * @param  __FIFO__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_FILTER_FIFO(__FIFO__)             (((__FIFO__) == CAN_FILTER_FIFO0) || \
                                                  ((__FIFO__) == CAN_FILTER_FIFO1))

#define IS_CAN_RX_FIFO(__FIFO__)                 (((__FIFO__) == CAN_RX_FIFO0) || ((__FIFO__) == CAN_RX_FIFO1))


                                                 /**
  * @brief  Check if the parameter __TRANSMITMAILBOX__ is valid
  * @param  __TRANSMITMAILBOX__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_TX_MAILBOX(__TRANSMITMAILBOX__)   (((__TRANSMITMAILBOX__) == CAN_TX_MAILBOX0 ) || \
                                                  ((__TRANSMITMAILBOX__) == CAN_TX_MAILBOX1 ) || \
                                                  ((__TRANSMITMAILBOX__) == CAN_TX_MAILBOX2 ))

#define IS_CAN_TX_MAILBOX_LIST(__TRANSMITMAILBOX__)  ((__TRANSMITMAILBOX__) <= (CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | \
                                                                                CAN_TX_MAILBOX2))

/**
  * @brief  Check if the parameter __STDID__ is valid
  * @param  __STDID__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_STDID(__STDID__)                  ((__STDID__) <= 0x7FFU)

/**
  * @brief  Check if the parameter __EXTID__ is valid
  * @param  __EXTID__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_EXTID(__EXTID__)                  ((__EXTID__) <= 0x1FFFFFFFU)

/**
  * @brief  Check if the parameter __DLC__ is valid
  * @param  __DLC__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_DLC(__DLC__)                      ((__DLC__) <= 8U)

/**
  * @brief  Check if the parameter __IDTYPE__ is valid
  * @param  __IDTYPE__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_IDTYPE(__IDTYPE__)                (((__IDTYPE__) == CAN_ID_STD) || \
                                                  ((__IDTYPE__) == CAN_ID_EXT))

/**
  * @brief  Check if the parameter __RTR__ is valid
  * @param  __RTR__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_RTR(__RTR__)                      (((__RTR__) == CAN_RTR_DATA) || ((__RTR__) == CAN_RTR_REMOTE))


/**
  * @brief  Check if the parameter __IT__ is valid
  * @param  __IT__
  * @retval Status
  *         @arg SET   (Valid)
  *         @arg RESET (Invalid)
  */
#define IS_CAN_IT(__IT__)                        ((__IT__) <= (CAN_IT_TX_MAILBOX_EMPTY     | CAN_IT_RX_FIFO0_MSG_PENDING      | \
                                                               CAN_IT_RX_FIFO0_FULL        | CAN_IT_RX_FIFO0_OVERRUN          | \
                                                               CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_RX_FIFO1_FULL             | \
                                                               CAN_IT_RX_FIFO1_OVERRUN     | CAN_IT_WAKEUP                    | \
                                                               CAN_IT_SLEEP_ACK            | CAN_IT_ERROR_WARNING             | \
                                                               CAN_IT_ERROR_PASSIVE        | CAN_IT_BUSOFF                    | \
                                                               CAN_IT_LAST_ERROR_CODE      | CAN_IT_ERROR))

/**
  * end of CAN_Macro_Definitions @}
  */

/* Exported functions --------------------------------------------------------*/

/******************************************************************************/
/*                             CAN Functions                                  */
/******************************************************************************/

/** @defgroup CAN_Function_Definitions CAN Function Definitions
  * @{
  */

/* Initialization and de-initialization functions *****************************/
HAL_StatusTypeDef HAL_CAN_Init(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_DeInit(CAN_HandleTypeDef *hcan);
void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan);
void HAL_CAN_MspDeInit(CAN_HandleTypeDef *hcan);

#if USE_HAL_CAN_REGISTER_CALLBACKS == 1
/* Callbacks Register/UnRegister functions  ***********************************/
HAL_StatusTypeDef HAL_CAN_RegisterCallback(CAN_HandleTypeDef *hcan, HAL_CAN_CallbackIDTypeDef CallbackID,
                                           void (* pCallback)(CAN_HandleTypeDef *_hcan));
HAL_StatusTypeDef HAL_CAN_UnRegisterCallback(CAN_HandleTypeDef *hcan, HAL_CAN_CallbackIDTypeDef CallbackID);

#endif /* (USE_HAL_CAN_REGISTER_CALLBACKS) */

/* Configuration functions ****************************************************/
HAL_StatusTypeDef HAL_CAN_ConfigFilter(CAN_HandleTypeDef *hcan, const CAN_FilterTypeDef *sFilterConfig);
HAL_StatusTypeDef HAL_CANEx_ConfigFilter(CAN_HandleTypeDef *hcan, CANEx_FilterTypeDef *sFilterConfig);

/* Control functions **********************************************************/
HAL_StatusTypeDef HAL_CAN_Start(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_Stop(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_RequestSleep(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_WakeUp(CAN_HandleTypeDef *hcan);
uint32_t HAL_CAN_IsSleepActive(const CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, const CAN_TxHeaderTypeDef *pHeader,
                                       const uint8_t aData[], uint32_t *pTxMailbox);
HAL_StatusTypeDef HAL_CAN_AbortTxRequest(CAN_HandleTypeDef *hcan, uint32_t TxMailboxes);
uint32_t HAL_CAN_GetTxMailboxesFreeLevel(const CAN_HandleTypeDef *hcan);
uint32_t HAL_CAN_IsTxMessagePending(const CAN_HandleTypeDef *hcan, uint32_t TxMailboxes);
uint32_t HAL_CAN_GetTxTimestamp(const CAN_HandleTypeDef *hcan, uint32_t TxMailbox);
HAL_StatusTypeDef HAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, uint32_t RxFifo,
                                       CAN_RxHeaderTypeDef *pHeader, uint8_t aData[]);
uint32_t HAL_CAN_GetRxFifoFillLevel(const CAN_HandleTypeDef *hcan, uint32_t RxFifo);

/* Interrupts management ******************************************************/
HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t ActiveITs);
HAL_StatusTypeDef HAL_CAN_DeactivateNotification(CAN_HandleTypeDef *hcan, uint32_t InactiveITs);
void HAL_CAN_IRQHandler(CAN_HandleTypeDef *hcan);

/* Callbacks functions ********************************************************/
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxMailbox0AbortCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxMailbox1AbortCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_TxMailbox2AbortCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo0FullCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_RxFifo1FullCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_SleepCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_WakeUpFromRxMsgCallback(CAN_HandleTypeDef *hcan);
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan);

/* Peripheral State and Error functions ***************************************/
HAL_CAN_StateTypeDef HAL_CAN_GetState(const CAN_HandleTypeDef *hcan);
uint32_t HAL_CAN_GetError(const CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef HAL_CAN_ResetError(CAN_HandleTypeDef *hcan);

/**
  * end of CAN_Function_Definitions @}
  */

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private Macros -----------------------------------------------------------*/

/**
  * end of CAN @}
  */

#ifdef __cplusplus
}
#endif

#endif /* _RX32G4xx_HAL_CAN_H_ */

