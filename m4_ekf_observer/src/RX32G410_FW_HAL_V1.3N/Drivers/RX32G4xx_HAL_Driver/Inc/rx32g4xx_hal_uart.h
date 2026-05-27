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
*   File    : rx32g4xx_hal_uart.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_UART_H_
#define _RX32G4XX_HAL_UART_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"


/* Exported types ------------------------------------------------------------*/

/** @addtogroup UART
  * @{
  */

/******************************************************************************/
/*                              UART Structures                              */
/******************************************************************************/

/** @defgroup UART_Structure_Definitions UART Structure Definitions
  * @{
  */

/** @defgroup UART_InitTypeDef UART InitTypeDef
  * @ingroup  UART_Structure_Definitions
  * @{
  */
/**
  * @brief UART Init Structure definition
  */
typedef struct
{
  uint32_t BaudRate;                  /*!< This member configures the UART communication baud rate.
                                           The baud rate is computed using the following formula:
                                           - IntegerDivider = ((PCLKx) / (16 * (huart->Init.BaudRate)))
                                           - FractionalDivider = ((IntegerDivider - ((uint32_t) IntegerDivider)) * 16) + 0.5 */

  uint32_t WordLength;                /*!< Specifies the number of data bits transmitted or received in a frame.
                                           This parameter can be a value of @ref UART_Word_Length */

  uint32_t StopBits;                  /*!< Specifies the number of stop bits transmitted.
                                           This parameter can be a value of @ref UART_Stop_Bits */

  uint32_t Parity;                    /*!< Specifies the parity mode.
                                           This parameter can be a value of @ref UART_Parity
                                           @note When parity is enabled, the computed parity is inserted
                                                 at the MSB position of the transmitted data (9th bit when
                                                 the word length is set to 9 data bits; 8th bit when the
                                                 word length is set to 8 data bits). */

  uint32_t Mode;                      /*!< Specifies whether the Receive or Transmit mode is enabled or disabled.
                                           This parameter can be a value of @ref UART_Mode */

  uint32_t HwFlowCtl;                 /*!< Specifies whether the hardware flow control mode is enabled or disabled.
                                           This parameter can be a value of @ref UART_Hardware_Flow_Control */
} UART_InitTypeDef;

/**
  * end of UART_InitTypeDef @}
  */


/** @defgroup HAL_UART_StateTypeDef HAL UART StateTypeDef
  * @ingroup  UART_Structure_Definitions
  * @{
  */
/**
  * @brief HAL UART State structures definition
  * @note  HAL UART State value is a combination of 2 different substates: gState and RxState.
  *        - gState contains UART state information related to global Handle management
  *          and also information related to Tx operations.
  *          gState value coding follow below described bitmap :
  *          b7-b6  Error information
  *             00 : No Error
  *             01 : (Not Used)
  *             10 : Timeout
  *             11 : Error
  *          b5     Peripheral initialization status
  *             0  : Reset (Peripheral not initialized)
  *             1  : Init done (Peripheral initialized. HAL UART Init function already called)
  *          b4-b3  (not used)
  *             xx : Should be set to 00
  *          b2     Intrinsic process state
  *             0  : Ready
  *             1  : Busy (Peripheral busy with some configuration or internal operations)
  *          b1     (not used)
  *             x  : Should be set to 0
  *          b0     Tx state
  *             0  : Ready (no Tx operation ongoing)
  *             1  : Busy (Tx operation ongoing)
  *        - RxState contains information related to Rx operations.
  *          RxState value coding follow below described bitmap :
  *          b7-b6  (not used)
  *             xx : Should be set to 00
  *          b5     Peripheral initialization status
  *             0  : Reset (Peripheral not initialized)
  *             1  : Init done (Peripheral initialized)
  *          b4-b2  (not used)
  *            xxx : Should be set to 000
  *          b1     Rx state
  *             0  : Ready (no Rx operation ongoing)
  *             1  : Busy (Rx operation ongoing)
  *          b0     (not used)
  *             x  : Should be set to 0.
  */
typedef enum
{
  HAL_UART_STATE_RESET             = 0x00U,    /*!< Peripheral is not yet Initialized
                                                   Value is allowed for gState and RxState */
  HAL_UART_STATE_READY             = 0x20U,    /*!< Peripheral Initialized and ready for use
                                                   Value is allowed for gState and RxState */
  HAL_UART_STATE_BUSY              = 0x24U,    /*!< an internal process is ongoing
                                                   Value is allowed for gState only */
  HAL_UART_STATE_BUSY_TX           = 0x21U,    /*!< Data Transmission process is ongoing
                                                   Value is allowed for gState only */
  HAL_UART_STATE_BUSY_RX           = 0x22U,    /*!< Data Reception process is ongoing
                                                   Value is allowed for RxState only */
  HAL_UART_STATE_BUSY_TX_RX        = 0x23U,    /*!< Data Transmission and Reception process is ongoing
                                                   Not to be used for neither gState nor RxState.
                                                   Value is result of combination (Or) between gState and RxState values */
  HAL_UART_STATE_TIMEOUT           = 0xA0U,    /*!< Timeout state
                                                   Value is allowed for gState only */
  HAL_UART_STATE_ERROR             = 0xE0U     /*!< Error
                                                   Value is allowed for gState only */
} HAL_UART_StateTypeDef;

/**
  * end of HAL_UART_StateTypeDef @}
  */

/** @defgroup HAL_UART_RxTypeTypeDef HAL UART RxTypeTypeDef
  * @ingroup  UART_Structure_Definitions
  * @{
  */
/**
  * @brief HAL UART Reception type definition
  * @note  HAL UART Reception type value aims to identify which type of Reception is ongoing.
  *        This parameter can be a value of @ref UART_Reception_Type_Values :
  *           HAL_UART_RECEPTION_STANDARD         = 0x00U,
  *           HAL_UART_RECEPTION_TOIDLE           = 0x01U,
  */
typedef uint32_t HAL_UART_RxTypeTypeDef;

/**
  * end of HAL_UART_RxTypeTypeDef @}
  */

/** @defgroup HAL_UART_RxEventTypeTypeDef HAL UART RxEventTypeTypeDef
  * @ingroup  UART_Structure_Definitions
  * @{
  */
/**
  * @brief HAL UART Rx Event type definition
  * @note  HAL UART Rx Event type value aims to identify which type of Event has occurred
  *        leading to call of the RxEvent callback.
  *        This parameter can be a value of @ref UART_RxEvent_Type_Values :
  *           HAL_UART_RXEVENT_TC                 = 0x00U,
  *           HAL_UART_RXEVENT_HT                 = 0x01U,
  *           HAL_UART_RXEVENT_IDLE               = 0x02U,
  */
typedef uint32_t HAL_UART_RxEventTypeTypeDef;

/**
  * end of HAL_UART_RxEventTypeTypeDef @}
  */

/** @defgroup UART_HandleTypeDef UART HandleTypeDef
  * @ingroup  UART_Structure_Definitions
  * @{
  */
/**
  * @brief  UART handle Structure definition
  */
typedef struct __UART_HandleTypeDef
{
  UART_TypeDef                 *Instance;        /*!< UART registers base address        */

  UART_InitTypeDef             Init;             /*!< UART communication parameters      */

  const uint8_t                 *pTxBuffPtr;      /*!< Pointer to UART Tx transfer Buffer */

  uint16_t                      TxXferSize;       /*!< UART Tx Transfer size              */

  __IO uint16_t                 TxXferCount;      /*!< UART Tx Transfer Counter           */

  uint8_t                       *pRxBuffPtr;      /*!< Pointer to UART Rx transfer Buffer */

  uint16_t                      RxXferSize;       /*!< UART Rx Transfer size              */

  __IO uint16_t                 RxXferCount;      /*!< UART Rx Transfer Counter           */

  __IO HAL_UART_RxTypeTypeDef ReceptionType;      /*!< Type of ongoing reception          */

  __IO HAL_UART_RxEventTypeTypeDef RxEventType;   /*!< Type of Rx Event                   */

  DMA_HandleTypeDef             *hdmatx;          /*!< UART Tx DMA Handle parameters      */

  DMA_HandleTypeDef             *hdmarx;          /*!< UART Rx DMA Handle parameters      */

  HAL_LockTypeDef               Lock;             /*!< Locking object                     */

  __IO HAL_UART_StateTypeDef    gState;           /*!< UART state information related to global Handle management
                                                       and also related to Tx operations.
                                                       This parameter can be a value of @ref HAL_UART_StateTypeDef */

  __IO HAL_UART_StateTypeDef    RxState;          /*!< UART state information related to Rx operations.
                                                       This parameter can be a value of @ref HAL_UART_StateTypeDef */

  __IO uint32_t                 ErrorCode;        /*!< UART Error code                    */

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
  void (* TxHalfCpltCallback)(struct __UART_HandleTypeDef *huart);        /*!< UART Tx Half Complete Callback        */
  void (* TxCpltCallback)(struct __UART_HandleTypeDef *huart);            /*!< UART Tx Complete Callback             */
  void (* RxHalfCpltCallback)(struct __UART_HandleTypeDef *huart);        /*!< UART Rx Half Complete Callback        */
  void (* RxCpltCallback)(struct __UART_HandleTypeDef *huart);            /*!< UART Rx Complete Callback             */
  void (* ErrorCallback)(struct __UART_HandleTypeDef *huart);             /*!< UART Error Callback                   */
  void (* AbortCpltCallback)(struct __UART_HandleTypeDef *huart);         /*!< UART Abort Complete Callback          */
  void (* AbortTransmitCpltCallback)(struct __UART_HandleTypeDef *huart); /*!< UART Abort Transmit Complete Callback */
  void (* AbortReceiveCpltCallback)(struct __UART_HandleTypeDef *huart);  /*!< UART Abort Receive Complete Callback  */
  void (* WakeupCallback)(struct __UART_HandleTypeDef *huart);            /*!< UART Wakeup Callback                  */
  void (* RxEventCallback)(struct __UART_HandleTypeDef *huart, uint16_t Pos); /*!< UART Reception Event Callback     */

  void (* MspInitCallback)(struct __UART_HandleTypeDef *huart);           /*!< UART Msp Init callback                */
  void (* MspDeInitCallback)(struct __UART_HandleTypeDef *huart);         /*!< UART Msp DeInit callback              */
#endif  /* USE_HAL_UART_REGISTER_CALLBACKS */

} UART_HandleTypeDef;
/**
  * end of UART_HandleTypeDef @}
  */

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
/** @defgroup HAL_UART_CallbackIDTypeDef HAL UART CallbackIDTypeDef
  * @ingroup  UART_Structure_Definitions
  * @{
  */
/**
  * @brief  HAL UART Callback ID enumeration definition
  */
typedef enum
{
  HAL_UART_TX_HALFCOMPLETE_CB_ID         = 0x00U,    /*!< UART Tx Half Complete Callback ID        */
  HAL_UART_TX_COMPLETE_CB_ID             = 0x01U,    /*!< UART Tx Complete Callback ID             */
  HAL_UART_RX_HALFCOMPLETE_CB_ID         = 0x02U,    /*!< UART Rx Half Complete Callback ID        */
  HAL_UART_RX_COMPLETE_CB_ID             = 0x03U,    /*!< UART Rx Complete Callback ID             */
  HAL_UART_ERROR_CB_ID                   = 0x04U,    /*!< UART Error Callback ID                   */
  HAL_UART_ABORT_COMPLETE_CB_ID          = 0x05U,    /*!< UART Abort Complete Callback ID          */
  HAL_UART_ABORT_TRANSMIT_COMPLETE_CB_ID = 0x06U,    /*!< UART Abort Transmit Complete Callback ID */
  HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID  = 0x07U,    /*!< UART Abort Receive Complete Callback ID  */
  HAL_UART_WAKEUP_CB_ID                  = 0x08U,    /*!< UART Wakeup Callback ID                  */

  HAL_UART_MSPINIT_CB_ID                 = 0x0BU,    /*!< UART MspInit callback ID                 */
  HAL_UART_MSPDEINIT_CB_ID               = 0x0CU     /*!< UART MspDeInit callback ID               */

} HAL_UART_CallbackIDTypeDef;
/**
  * end of HAL_UART_CallbackIDTypeDef @}
  */


/** @defgroup HAL_UART_Callback_pointer HAL UART Callback pointer
  * @ingroup  UART_Structure_Definitions
  * @{
  */
/**
  * @brief  HAL UART Callback pointer definition
  */
typedef  void (*pUART_CallbackTypeDef)(HAL_UART_HandleTypeDef *huart);                             /*!< pointer to an UART callback function */
typedef  void (*pUART_RxEventCallbackTypeDef)(struct __UART_HandleTypeDef *huart, uint16_t Pos);   /*!< pointer to a UART Rx Event specific callback function */

/**
  * end of HAL_UART_Callback_pointer @}
  */

#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

/**
  * end of UART_Structure_Definitions @}
  */


/******************************************************************************/
/*                               UART Parameters                             */
/******************************************************************************/

/** @defgroup UART_Parameter_Definitions UART Parameter Definitions
  * @{
  */

/** @defgroup UART_Error_Code UART Error Code
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define HAL_UART_ERROR_NONE              0x00000000U   /*!< No error            */
#define HAL_UART_ERROR_PE                0x00000001U   /*!< Parity error        */
#define HAL_UART_ERROR_NE                0x00000002U   /*!< Noise error         */
#define HAL_UART_ERROR_FE                0x00000004U   /*!< Frame error         */
#define HAL_UART_ERROR_ORE               0x00000008U   /*!< Overrun error       */
#define HAL_UART_ERROR_DMA               0x00000010U   /*!< DMA transfer error  */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
#define  HAL_UART_ERROR_INVALID_CALLBACK 0x00000020U   /*!< Invalid Callback error  */
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
/**
  * end of UART_Error_Code @}
  */

/** @defgroup UART_Word_Length UART Word Length
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define UART_WORDLENGTH_8B                  0x00000000U
#define UART_WORDLENGTH_9B                  (UART_CR1_M)
/**
  * end of UART Word Length @}
  */

/** @defgroup UART_Stop_Bits UART Stop Bits
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define UART_STOPBITS_1                     0x00000000U
#define UART_STOPBITS_2                     (UART_CR2_STOP_BIT1)
/**
  * end of UART_Stop_Bits @}
  */

/** @defgroup UART_Parity UART Parity
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define UART_PARITY_NONE                    0x00000000U
#define UART_PARITY_EVEN                    (UART_CR1_PCE)
#define UART_PARITY_ODD                     (UART_CR1_PCE | UART_CR1_PS)
/**
  * end of UART_Parity @}
  */

/** @defgroup UART_Hardware_Flow_Control UART Hardware Flow Control
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define UART_HWCONTROL_NONE                  0x00000000U
#define UART_HWCONTROL_RTS                   (UART_CR3_RTSE)
#define UART_HWCONTROL_CTS                   (UART_CR3_CTSE)
#define UART_HWCONTROL_RTS_CTS               (UART_CR3_RTSE | UART_CR3_CTSE)
/**
  * end of UART_Hardware_Flow_Control @}
  */

/** @defgroup UART_Mode UART Mode
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define UART_MODE_RX                        (UART_CR1_RE)
#define UART_MODE_TX                        (UART_CR1_TE)
#define UART_MODE_TX_RX                     (UART_CR1_TE | UART_CR1_RE)
/**
  * end of UART_Mode @}
  */

/** @defgroup UART_State UART State
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define UART_STATE_DISABLE                  0x00000000U
#define UART_STATE_ENABLE                   (UART_CR1_UE)
/**
  * end of UART_State @}
  */

/** @defgroup UART_LIN_Break_Detection_Length UART LIN_Break Detection Length
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define UART_LINBREAKDETECTLENGTH_10B      0x00000000U
#define UART_LINBREAKDETECTLENGTH_11B      (UART_CR2_LBDL)
/**
  * end of UART_LIN_Break_Detection_Length @}
  */

/** @defgroup UART_WakeUp_functions UART WakeUp functions
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define UART_WAKEUPMETHOD_IDLELINE                0x00000000U
#define UART_WAKEUPMETHOD_ADDRESSMARK             (UART_CR1_WAKE)
/**
  * end of UART_WakeUp_functions @}
  */

/** @defgroup UART_Flags UART Flags
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define UART_FLAG_CTS                       (UART_SR_CTS)
#define UART_FLAG_LBD                       (UART_SR_LBD)
#define UART_FLAG_TXE                       (UART_SR_TXE)
#define UART_FLAG_TC                        (UART_SR_TC)
#define UART_FLAG_RXNE                      (UART_SR_RXNE)
#define UART_FLAG_IDLE                      (UART_SR_IDLE)
#define UART_FLAG_ORE                       (UART_SR_ORE)
#define UART_FLAG_NE                        (UART_SR_NE)
#define UART_FLAG_FE                        (UART_SR_FE)
#define UART_FLAG_PE                        (UART_SR_PE)
/**
  * end of UART_Flags @}
  */

/** @defgroup UART_Interrupt_definition UART Interrupt definition
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define UART_IT_MASK                     0x0000FFFFU
#define UART_CR1_REG_INDEX               1U
#define UART_CR2_REG_INDEX               2U
#define UART_CR3_REG_INDEX               3U

#define UART_IT_PE                       (UART_CR1_REG_INDEX << 28U | UART_CR1_PEIE)
#define UART_IT_TXE                      (UART_CR1_REG_INDEX << 28U | UART_CR1_TXEIE)
#define UART_IT_TC                       (UART_CR1_REG_INDEX << 28U | UART_CR1_TCIE)
#define UART_IT_RXNE                     (UART_CR1_REG_INDEX << 28U | UART_CR1_RXNEIE)
#define UART_IT_IDLE                     (UART_CR1_REG_INDEX << 28U | UART_CR1_IDLEIE)
#define UART_IT_LBD                      (UART_CR2_REG_INDEX << 28U | UART_CR2_LBDIE)
#define UART_IT_CTS                      (UART_CR3_REG_INDEX << 28U | UART_CR3_CTSIE)
#define UART_IT_ERR                      (UART_CR3_REG_INDEX << 28U | UART_CR3_EIE)
/**
  * end of UART_Interrupt_definition @}
  */

/** @defgroup UART_Reception_Type_Values UART Reception Type Values
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define HAL_UART_RECEPTION_STANDARD          (0x00000000U)             /*!< Standard reception                       */
#define HAL_UART_RECEPTION_TOIDLE            (0x00000001U)             /*!< Reception till completion or IDLE event  */
/**
  * end of UART_Reception_Type_Values @}
  */

/** @defgroup UART_RxEvent_Type_Values UART RxEvent Type Values
  * @ingroup  UART_Parameter_Definitions
  * @{
  */
#define HAL_UART_RXEVENT_TC                  (0x00000000U)             /*!< RxEvent linked to Transfer Complete event */
#define HAL_UART_RXEVENT_HT                  (0x00000001U)             /*!< RxEvent linked to Half Transfer event     */
#define HAL_UART_RXEVENT_IDLE                (0x00000002U)
/**
  * end of UART_RxEvent_Type_Values @}
  */

/**
  * end of UART_Parameter_Definitions @}
  */


/******************************************************************************/
/*                                UART Macro                                 */
/******************************************************************************/

/** @defgroup UART_Macro_Definitions UART Macro Definitions
  * @{
  */

/** @brief Reset UART handle gstate & RxState
  * @param  __HANDLE__ specifies the UART Handle.
  *         UART Handle selects the UARTx or UARTy peripheral
  *         (UART,UART availability and x,y values depending on device).
  * @retval None
  */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
#define __HAL_UART_RESET_HANDLE_STATE(__HANDLE__)  do{                                                   \
                                                       (__HANDLE__)->gState = HAL_UART_STATE_RESET;      \
                                                       (__HANDLE__)->RxState = HAL_UART_STATE_RESET;     \
                                                       (__HANDLE__)->MspInitCallback = NULL;             \
                                                       (__HANDLE__)->MspDeInitCallback = NULL;           \
                                                     } while(0U)
#else
#define __HAL_UART_RESET_HANDLE_STATE(__HANDLE__)  do{                                                   \
                                                       (__HANDLE__)->gState = HAL_UART_STATE_RESET;      \
                                                       (__HANDLE__)->RxState = HAL_UART_STATE_RESET;     \
                                                     } while(0U)
#endif /*USE_HAL_UART_REGISTER_CALLBACKS */

/** @brief  Flushes the UART DR register
  * @param  __HANDLE__ specifies the UART Handle.
  */
#define __HAL_UART_FLUSH_DRREGISTER(__HANDLE__) ((__HANDLE__)->Instance->DR)

/** @brief  Checks whether the specified UART flag is set or not.
  * @param  __HANDLE__ specifies the UART Handle.
  * @param  __FLAG__ specifies the flag to check.
  *        This parameter can be one of the following values:
  *            @arg UART_FLAG_CTS:  CTS Change flag
  *            @arg UART_FLAG_LBD:  LIN Break detection flag
  *            @arg UART_FLAG_TXE:  Transmit data register empty flag
  *            @arg UART_FLAG_TC:   Transmission Complete flag
  *            @arg UART_FLAG_RXNE: Receive data register not empty flag
  *            @arg UART_FLAG_IDLE: Idle Line detection flag
  *            @arg UART_FLAG_ORE:  Overrun Error flag
  *            @arg UART_FLAG_NE:   Noise Error flag
  *            @arg UART_FLAG_FE:   Framing Error flag
  *            @arg UART_FLAG_PE:   Parity Error flag
  * @retval The new state of __FLAG__ (TRUE or FALSE).
  */
#define __HAL_UART_GET_FLAG(__HANDLE__, __FLAG__) (((__HANDLE__)->Instance->SR & (__FLAG__)) == (__FLAG__))

/** @brief  Clears the specified UART pending flag.
  * @param  __HANDLE__ specifies the UART Handle.
  *         UART Handle selects the UARTx or UARTy peripheral
  *         (UART,UART availability and x,y values depending on device).
  * @param  __FLAG__ specifies the flag to check.
  *          This parameter can be any combination of the following values:
  *            @arg UART_FLAG_CTS:  CTS Change flag (not available for UART4 and UART5).
  *            @arg UART_FLAG_LBD:  LIN Break detection flag.
  *            @arg UART_FLAG_TC:   Transmission Complete flag.
  *            @arg UART_FLAG_RXNE: Receive data register not empty flag.
  *
  * @note   PE (Parity error), FE (Framing error), NE (Noise error), ORE (Overrun
  *          error) and IDLE (Idle line detected) flags are cleared by software
  *          sequence: a read operation to UART_SR register followed by a read
  *          operation to UART_DR register.
  * @note   RXNE flag can be also cleared by a read to the UART_DR register.
  * @note   TC flag can be also cleared by software sequence: a read operation to
  *          UART_SR register followed by a write operation to UART_DR register.
  * @note   TXE flag is cleared only by a write to the UART_DR register.
  *
  * @retval None
  */
#define __HAL_UART_CLEAR_FLAG(__HANDLE__, __FLAG__) ((__HANDLE__)->Instance->SR = ~(__FLAG__))

/** @brief  Clears the UART PE pending flag.
  * @param  __HANDLE__ specifies the UART Handle.
  * @retval None
  */
#define __HAL_UART_CLEAR_PEFLAG(__HANDLE__)     \
  do{                                           \
    __IO uint32_t tmpreg = 0x00U;               \
    tmpreg = (__HANDLE__)->Instance->SR;        \
    tmpreg = (__HANDLE__)->Instance->DR;        \
    UNUSED(tmpreg);                             \
  } while(0U)

/** @brief  Clears the UART FE pending flag.
  * @param  __HANDLE__ specifies the UART Handle.
  * @retval None
  */
#define __HAL_UART_CLEAR_FEFLAG(__HANDLE__) __HAL_UART_CLEAR_PEFLAG(__HANDLE__)

/** @brief  Clears the UART NE pending flag.
  * @param  __HANDLE__ specifies the UART Handle.
  * @retval None
  */
#define __HAL_UART_CLEAR_NEFLAG(__HANDLE__) __HAL_UART_CLEAR_PEFLAG(__HANDLE__)

/** @brief  Clears the UART ORE pending flag.
  * @param  __HANDLE__ specifies the UART Handle.
  * @retval None
  */
#define __HAL_UART_CLEAR_OREFLAG(__HANDLE__) __HAL_UART_CLEAR_PEFLAG(__HANDLE__)

/** @brief  Clears the UART IDLE pending flag.
  * @param  __HANDLE__ specifies the UART Handle.
  * @retval None
  */
#define __HAL_UART_CLEAR_IDLEFLAG(__HANDLE__) __HAL_UART_CLEAR_PEFLAG(__HANDLE__)

/** @brief  Enable the specified UART interrupt.
  * @param  __HANDLE__ specifies the UART Handle.
  * @param  __INTERRUPT__ specifies the UART interrupt source to enable.
  *          This parameter can be one of the following values:
  *            @arg UART_IT_CTS:  CTS change interrupt
  *            @arg UART_IT_LBD:  LIN Break detection interrupt
  *            @arg UART_IT_TXE:  Transmit Data Register empty interrupt
  *            @arg UART_IT_TC:   Transmission complete interrupt
  *            @arg UART_IT_RXNE: Receive Data register not empty interrupt
  *            @arg UART_IT_IDLE: Idle line detection interrupt
  *            @arg UART_IT_PE:   Parity Error interrupt
  *            @arg UART_IT_ERR:  Error interrupt(Frame error, noise error, overrun error)
  * @retval None
  */
#define __HAL_UART_ENABLE_IT(__HANDLE__, __INTERRUPT__)   ((((__INTERRUPT__) >> 28U) == UART_CR1_REG_INDEX)? ((__HANDLE__)->Instance->CR1 |= ((__INTERRUPT__) & UART_IT_MASK)): \
                                                           (((__INTERRUPT__) >> 28U) == UART_CR2_REG_INDEX)? ((__HANDLE__)->Instance->CR2 |= ((__INTERRUPT__) & UART_IT_MASK)): \
                                                           ((__HANDLE__)->Instance->CR3 |= ((__INTERRUPT__) & UART_IT_MASK)))

/** @brief  Disable the specified UART interrupt.
  * @param  __HANDLE__ specifies the UART Handle.
  * @param  __INTERRUPT__ specifies the UART interrupt source to disable.
  *          This parameter can be one of the following values:
  *            @arg UART_IT_CTS:  CTS change interrupt
  *            @arg UART_IT_LBD:  LIN Break detection interrupt
  *            @arg UART_IT_TXE:  Transmit Data Register empty interrupt
  *            @arg UART_IT_TC:   Transmission complete interrupt
  *            @arg UART_IT_RXNE: Receive Data register not empty interrupt
  *            @arg UART_IT_IDLE: Idle line detection interrupt
  *            @arg UART_IT_PE:   Parity Error interrupt
  *            @arg UART_IT_ERR:  Error interrupt(Frame error, noise error, overrun error)
  * @retval None
  */
#define __HAL_UART_DISABLE_IT(__HANDLE__, __INTERRUPT__)  ((((__INTERRUPT__) >> 28U) == UART_CR1_REG_INDEX)? ((__HANDLE__)->Instance->CR1 &= ~((__INTERRUPT__) & UART_IT_MASK)): \
                                                           (((__INTERRUPT__) >> 28U) == UART_CR2_REG_INDEX)? ((__HANDLE__)->Instance->CR2 &= ~((__INTERRUPT__) & UART_IT_MASK)): \
                                                           ((__HANDLE__)->Instance->CR3 &= ~ ((__INTERRUPT__) & UART_IT_MASK)))

/** @brief  Checks whether the specified UART interrupt source is enabled or not.
  * @param  __HANDLE__ specifies the UART Handle.
  * @param  __IT__ specifies the UART interrupt source to check.
  *          This parameter can be one of the following values:
  *            @arg UART_IT_CTS: CTS change interrupt (not available for UART4 and UART5)
  *            @arg UART_IT_LBD: LIN Break detection interrupt
  *            @arg UART_IT_TXE: Transmit Data Register empty interrupt
  *            @arg UART_IT_TC:  Transmission complete interrupt
  *            @arg UART_IT_RXNE: Receive Data register not empty interrupt
  *            @arg UART_IT_IDLE: Idle line detection interrupt
  *            @arg UART_IT_ERR: Error interrupt
  * @retval The new state of __IT__ (TRUE or FALSE).
  */
#define __HAL_UART_GET_IT_SOURCE(__HANDLE__, __IT__) (((((__IT__) >> 28U) == UART_CR1_REG_INDEX)? (__HANDLE__)->Instance->CR1:(((((uint32_t)(__IT__)) >> 28U) == UART_CR2_REG_INDEX)? \
                                                       (__HANDLE__)->Instance->CR2 : (__HANDLE__)->Instance->CR3)) & (((uint32_t)(__IT__)) & UART_IT_MASK))

/** @brief  Enable CTS flow control
  * @note   This macro allows to enable CTS hardware flow control for a given UART instance,
  *         without need to call HAL_UART_Init() function.
  *         As involving direct access to UART registers, usage of this macro should be fully endorsed by user.
  * @note   As macro is expected to be used for modifying CTS Hw flow control feature activation, without need
  *         for UART instance Deinit/Init, following conditions for macro call should be fulfilled :
  *           - UART instance should have already been initialised (through call of HAL_UART_Init() )
  *           - macro could only be called when corresponding UART instance is disabled (i.e __HAL_UART_DISABLE(__HANDLE__))
  *             and should be followed by an Enable macro (i.e __HAL_UART_ENABLE(__HANDLE__)).
  * @param  __HANDLE__ specifies the UART Handle.
  *         The Handle Instance can be any UARTx (supporting the HW Flow control feature).
  *         It is used to select the UART peripheral (UART availability and x value depending on device).
  * @retval None
  */
#define __HAL_UART_HWCONTROL_CTS_ENABLE(__HANDLE__)        \
  do{                                                      \
    ATOMIC_SET_BIT((__HANDLE__)->Instance->CR3, UART_CR3_CTSE);  \
    (__HANDLE__)->Init.HwFlowCtl |= UART_CR3_CTSE;        \
  } while(0U)

/** @brief  Disable CTS flow control
  * @note   This macro allows to disable CTS hardware flow control for a given UART instance,
  *         without need to call HAL_UART_Init() function.
  *         As involving direct access to UART registers, usage of this macro should be fully endorsed by user.
  * @note   As macro is expected to be used for modifying CTS Hw flow control feature activation, without need
  *         for UART instance Deinit/Init, following conditions for macro call should be fulfilled :
  *           - UART instance should have already been initialised (through call of HAL_UART_Init() )
  *           - macro could only be called when corresponding UART instance is disabled (i.e __HAL_UART_DISABLE(__HANDLE__))
  *             and should be followed by an Enable macro (i.e __HAL_UART_ENABLE(__HANDLE__)).
  * @param  __HANDLE__ specifies the UART Handle.
  *         The Handle Instance can be any UARTx (supporting the HW Flow control feature).
  *         It is used to select the UART peripheral (UART availability and x value depending on device).
  * @retval None
  */
#define __HAL_UART_HWCONTROL_CTS_DISABLE(__HANDLE__)        \
  do{                                                       \
    ATOMIC_CLEAR_BIT((__HANDLE__)->Instance->CR3, UART_CR3_CTSE); \
    (__HANDLE__)->Init.HwFlowCtl &= ~(UART_CR3_CTSE);      \
  } while(0U)

/** @brief  Enable RTS flow control
  *         This macro allows to enable RTS hardware flow control for a given UART instance,
  *         without need to call HAL_UART_Init() function.
  *         As involving direct access to UART registers, usage of this macro should be fully endorsed by user.
  * @note   As macro is expected to be used for modifying RTS Hw flow control feature activation, without need
  *         for UART instance Deinit/Init, following conditions for macro call should be fulfilled :
  *           - UART instance should have already been initialised (through call of HAL_UART_Init() )
  *           - macro could only be called when corresponding UART instance is disabled (i.e __HAL_UART_DISABLE(__HANDLE__))
  *             and should be followed by an Enable macro (i.e __HAL_UART_ENABLE(__HANDLE__)).
  * @param  __HANDLE__ specifies the UART Handle.
  *         The Handle Instance can be any UARTx (supporting the HW Flow control feature).
  *         It is used to select the UART peripheral (UART availability and x value depending on device).
  * @retval None
  */
#define __HAL_UART_HWCONTROL_RTS_ENABLE(__HANDLE__)       \
  do{                                                     \
    ATOMIC_SET_BIT((__HANDLE__)->Instance->CR3, UART_CR3_RTSE); \
    (__HANDLE__)->Init.HwFlowCtl |= UART_CR3_RTSE;       \
  } while(0U)

/** @brief  Disable RTS flow control
  *         This macro allows to disable RTS hardware flow control for a given UART instance,
  *         without need to call HAL_UART_Init() function.
  *         As involving direct access to UART registers, usage of this macro should be fully endorsed by user.
  * @note   As macro is expected to be used for modifying RTS Hw flow control feature activation, without need
  *         for UART instance Deinit/Init, following conditions for macro call should be fulfilled :
  *           - UART instance should have already been initialised (through call of HAL_UART_Init() )
  *           - macro could only be called when corresponding UART instance is disabled (i.e __HAL_UART_DISABLE(__HANDLE__))
  *             and should be followed by an Enable macro (i.e __HAL_UART_ENABLE(__HANDLE__)).
  * @param  __HANDLE__ specifies the UART Handle.
  *         The Handle Instance can be any UARTx (supporting the HW Flow control feature).
  *         It is used to select the UART peripheral (UART availability and x value depending on device).
  * @retval None
  */
#define __HAL_UART_HWCONTROL_RTS_DISABLE(__HANDLE__)       \
  do{                                                      \
    ATOMIC_CLEAR_BIT((__HANDLE__)->Instance->CR3, UART_CR3_RTSE);\
    (__HANDLE__)->Init.HwFlowCtl &= ~(UART_CR3_RTSE);     \
  } while(0U)

/** @brief  Enable UART
  * @param  __HANDLE__ specifies the UART Handle.
  * @retval None
  */
#define __HAL_UART_ENABLE(__HANDLE__)               ((__HANDLE__)->Instance->CR1 |=  UART_CR1_UE)

/** @brief  Disable UART
  * @param  __HANDLE__ specifies the UART Handle.
  * @retval None
  */
#define __HAL_UART_DISABLE(__HANDLE__)              ((__HANDLE__)->Instance->CR1 &=  ~UART_CR1_UE)

/** @brief  Check if the parameter INSTANCE is valid
  * @param  INSTANCE
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_UART_INSTANCE(INSTANCE) (((INSTANCE) == UART1) || \
                                     ((INSTANCE) == UART2) || \
                                     ((INSTANCE) == UART3) || \
                                     ((INSTANCE) == UART4)  || \
                                     ((INSTANCE) == UART5))

/** @brief  Check if the parameter LENGTH is valid
  * @param  LENGTH
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_UART_WORD_LENGTH(LENGTH) (((LENGTH) == UART_WORDLENGTH_8B) || \
                                     ((LENGTH) == UART_WORDLENGTH_9B))

/** @brief  Check if the parameter LENGTH is valid
  * @param  LENGTH
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_UART_LIN_WORD_LENGTH(LENGTH) (((LENGTH) == UART_LINBREAKDETECTLENGTH_10B) || \
                                          ((LENGTH) == UART_LINBREAKDETECTLENGTH_11B))

/** @brief  Check if the parameter STOPBITS is valid
  * @param  STOPBITS
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_UART_STOPBITS(STOPBITS) (((STOPBITS) == UART_STOPBITS_1) || \
                                    ((STOPBITS) == UART_STOPBITS_2))

/** @brief  Check if the parameter PARITY is valid
  * @param  PARITY
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_UART_PARITY(PARITY) (((PARITY) == UART_PARITY_NONE) || \
                                ((PARITY) == UART_PARITY_EVEN) || \
                                ((PARITY) == UART_PARITY_ODD))

/** @brief  Check if the parameter CONTROL is valid
  * @param  CONTROL
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_UART_HARDWARE_FLOW_CONTROL(CONTROL)\
                              (((CONTROL) == UART_HWCONTROL_NONE) || \
                               ((CONTROL) == UART_HWCONTROL_RTS) || \
                               ((CONTROL) == UART_HWCONTROL_CTS) || \
                               ((CONTROL) == UART_HWCONTROL_RTS_CTS))

/** @brief  Check if the parameter MODE is valid
  * @param  MODE
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_UART_MODE(MODE)           ( ((MODE) == UART_MODE_RX) || \
                                        ((MODE) == UART_MODE_TX) || \
                                        ((MODE) == UART_MODE_TX_RX))

/** @brief  Check if the parameter STATE is valid
  * @param  STATE
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_UART_STATE(STATE) (((STATE) == UART_STATE_DISABLE) || \
                              ((STATE) == UART_STATE_ENABLE))

/** @brief  Check if the parameter LENGTH is valid
  * @param  LENGTH
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_UART_LIN_BREAK_DETECT_LENGTH(LENGTH) (((LENGTH) == UART_LINBREAKDETECTLENGTH_10B) || \
                                                 ((LENGTH) == UART_LINBREAKDETECTLENGTH_11B))

/** @brief  Check if the parameter WAKEUP is valid
  * @param  WAKEUP
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_UART_WAKEUPMETHOD(WAKEUP) (((WAKEUP) == UART_WAKEUPMETHOD_IDLELINE) || \
                                      ((WAKEUP) == UART_WAKEUPMETHOD_ADDRESSMARK))

/** @brief  Check if the parameter BAUDRATE is valid
  * @param  BAUDRATE
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_UART_BAUDRATE(BAUDRATE) ((BAUDRATE) <= 4500000U)

/** @brief  Check if the parameter ADDRESS is valid
  * @param  ADDRESS
  * @retval Result
  *         @arg SET(valid)
  *         @arg RESET (invalid)
  */
#define IS_UART_ADDRESS(ADDRESS) ((ADDRESS) <= 0x0FU)

/** @brief  Calculate the baudrate of UART with 16-bit transmission(_PCLK_<171000000)
  * @param  _PCLK_: APB clock freqency
  * @param  _BAUD_: expected baudrate
  * @retval baudrate DIV
  */
#define UART_DIV_SAMPLING16(_PCLK_, _BAUD_)            (((_PCLK_)*25U)/(4U*(_BAUD_)))

/** @brief  Calculate the baudrate of UART with 16-bit transmission
  * @param  _PCLK_: APB clock freqency
  * @param  _BAUD_: expected baudrate
  * @retval baudrate DIVMANT
  */
#define UART_DIVMANT_SAMPLING16(_PCLK_, _BAUD_)        (UART_DIV_SAMPLING16((_PCLK_), (_BAUD_))/100U)

/** @brief  Calculate the baudrate of UART with 16-bit transmission
  * @param  _PCLK_: APB clock freqency
  * @param  _BAUD_: expected baudrate
  * @retval baudrate DIVFRAQ
  */
#define UART_DIVFRAQ_SAMPLING16(_PCLK_, _BAUD_)        ((((UART_DIV_SAMPLING16((_PCLK_), (_BAUD_)) - (UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) * 100U)) * 16U)\
                                                         + 50U) / 100U)

/** @brief  Calculate the baudrate of UART with 16-bit transmission
  * @param  _PCLK_: APB clock freqency
  * @param  _BAUD_: expected baudrate
  * @retval baudrate
  * @note   UART BRR = mantissa + overflow + fraction
            = (UART DIVMANT << 4) + (UART DIVFRAQ & 0xF0) + (UART DIVFRAQ & 0x0FU)
  */
#define UART_BRR_SAMPLING16(_PCLK_, _BAUD_)            (((UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) << 4U) + \
                                                         (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0xF0U)) + \
                                                         (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0x0FU))
                                                         
/** @brief  Calculate the baudrate of UART with 16-bit transmission(_PCLK_>=171000000)
  * @param  _PCLK_: APB clock freqency
  * @param  _BAUD_: expected baudrate
  * @retval baudrate DIV
  */                                                         
#define UART_DIV_SAMPLING16_171(_PCLK_, _BAUD_)     ((((85899345)*25U)/(4U*(_BAUD_)))+(((_PCLK_-85899345)*25U)/(4U*(_BAUD_))))

/** @brief  Calculate the baudrate of UART with 16-bit transmission(_PCLK_>=171000000)
  * @param  _PCLK_: APB clock freqency
  * @param  _BAUD_: expected baudrate
  * @retval baudrate DIVMANT
  */
#define UART_DIVMANT_SAMPLING16_171(_PCLK_, _BAUD_)     (UART_DIV_SAMPLING16_171((_PCLK_), (_BAUD_))/100U)

/** @brief  Calculate the baudrate of UART with 16-bit transmission(_PCLK_>=171000000)
  * @param  _PCLK_: APB clock freqency
  * @param  _BAUD_: expected baudrate
  * @retval baudrate DIVFRAQ
  */
#define UART_DIVFRAQ_SAMPLING16_171(_PCLK_, _BAUD_)     ((((UART_DIV_SAMPLING16_171((_PCLK_), (_BAUD_)) - (UART_DIVMANT_SAMPLING16_171((_PCLK_), (_BAUD_)) * 100)) * 16U)\
                                                          + 50U) / 100U)

/** @brief  Calculate the baudrate of UART with 16-bit transmission(_PCLK_>=171000000)
  * @param  _PCLK_: APB clock freqency
  * @param  _BAUD_: expected baudrate
  * @retval baudrate
  * @note   UART BRR = mantissa + overflow + fraction
            = (UART DIVMANT << 4) + (UART DIVFRAQ & 0xF0) + (UART DIVFRAQ & 0x0FU)
  */
#define UART_BRR_SAMPLING16_171(_PCLK_, _BAUD_)            (((UART_DIVMANT_SAMPLING16_171((_PCLK_), (_BAUD_)) << 4U) + \
                                                             (UART_DIVFRAQ_SAMPLING16_171((_PCLK_), (_BAUD_)) & 0xF0U)) + \
                                                             (UART_DIVFRAQ_SAMPLING16_171((_PCLK_), (_BAUD_)) & 0x0FU))                                                         

/** @brief  Calculate the baudrate of UART with 8-bit transmission
  * @param  _PCLK_: APB clock freqency
  * @param  _BAUD_: expected baudrate
  * @retval baudrate DIV
  */
#define UART_DIV_SAMPLING8(_PCLK_, _BAUD_)             (((_PCLK_)*25U)/(2U*(_BAUD_)))

/** @brief  Calculate the baudrate of UART with 8-bit transmission
  * @param  _PCLK_: APB clock freqency
  * @param  _BAUD_: expected baudrate
  * @retval baudrate DIVMANT
  */
#define UART_DIVMANT_SAMPLING8(_PCLK_, _BAUD_)         (UART_DIV_SAMPLING8((_PCLK_), (_BAUD_))/100U)

/** @brief  Calculate the baudrate of UART with 8-bit transmission
  * @param  _PCLK_: APB clock freqency
  * @param  _BAUD_: expected baudrate
  * @retval baudrate DIVFRAQ
  */
#define UART_DIVFRAQ_SAMPLING8(_PCLK_, _BAUD_)         ((((UART_DIV_SAMPLING8((_PCLK_), (_BAUD_)) - (UART_DIVMANT_SAMPLING8((_PCLK_), (_BAUD_)) * 100U)) * 8U)\
                                                         + 50U) / 100U)

/** @brief  Calculate the baudrate of UART with 8-bit transmission
  * @param  _PCLK_: APB clock freqency
  * @param  _BAUD_: expected baudrate
  * @retval baudrate
  * @note   UART BRR = mantissa + overflow + fraction
            = (UART DIVMANT << 4) + ((UART DIVFRAQ & 0xF8) << 1) + (UART DIVFRAQ & 0x07U)
  */
#define UART_BRR_SAMPLING8(_PCLK_, _BAUD_)             (((UART_DIVMANT_SAMPLING8((_PCLK_), (_BAUD_)) << 4U) + \
                                                         ((UART_DIVFRAQ_SAMPLING8((_PCLK_), (_BAUD_)) & 0xF8U) << 1U)) + \
                                                          (UART_DIVFRAQ_SAMPLING8((_PCLK_), (_BAUD_)) & 0x07U))

/**
  * end of UART_Macro_Definitions @}
  */


/******************************************************************************/
/*                               UART Functions                              */
/******************************************************************************/

/** @defgroup UART_Function_Definitions UART Function Definitions
  * @{
  */

/* Initialization/de-initialization functions  **********************************/
HAL_StatusTypeDef               HAL_UART_Init(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_HalfDuplex_Init(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_LIN_Init(UART_HandleTypeDef *huart, uint32_t BreakDetectLength);
HAL_StatusTypeDef               HAL_MultiProcessor_Init(UART_HandleTypeDef *huart, uint8_t Address, uint32_t WakeUpMethod);
HAL_StatusTypeDef               HAL_UART_DeInit(UART_HandleTypeDef *huart);
void                            HAL_UART_MspInit(UART_HandleTypeDef *huart);
void                            HAL_UART_MspDeInit(UART_HandleTypeDef *huart);

/* Callbacks Register/UnRegister functions  ***********************************/
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
HAL_StatusTypeDef               HAL_UART_RegisterCallback(UART_HandleTypeDef *huart, HAL_UART_CallbackIDTypeDef CallbackID,
                                                            pUART_CallbackTypeDef pCallback);
HAL_StatusTypeDef               HAL_UART_UnRegisterCallback(UART_HandleTypeDef *huart, HAL_UART_CallbackIDTypeDef CallbackID);

HAL_StatusTypeDef               HAL_UART_RegisterRxEventCallback(UART_HandleTypeDef *huart, pUART_RxEventCallbackTypeDef pCallback);
HAL_StatusTypeDef               HAL_UART_UnRegisterRxEventCallback(UART_HandleTypeDef *huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

/* IO operation functions *******************************************************/
HAL_StatusTypeDef               HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef               HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef               HAL_UART_Transmit_IT(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef               HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef               HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef               HAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef               HAL_UART_DMAPause(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_UART_DMAResume(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_UART_DMAStop(UART_HandleTypeDef *huart);

HAL_StatusTypeDef               HAL_UARTEx_ReceiveToIdle(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint16_t *RxLen,
                                                        uint32_t Timeout);
HAL_StatusTypeDef               HAL_UARTEx_ReceiveToIdle_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef               HAL_UARTEx_ReceiveToIdle_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);

HAL_UART_RxEventTypeTypeDef    HAL_UARTEx_GetRxEventType(UART_HandleTypeDef *huart);

/* Transfer Abort functions */
HAL_StatusTypeDef               HAL_UART_Abort(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_UART_AbortTransmit(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_UART_AbortReceive(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_UART_Abort_IT(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_UART_AbortTransmit_IT(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_UART_AbortReceive_IT(UART_HandleTypeDef *huart);

void                            HAL_UART_IRQHandler(UART_HandleTypeDef *huart);
void                            HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void                            HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart);
void                            HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void                            HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);
void                            HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
void                            HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart);
void                            HAL_UART_AbortTransmitCpltCallback(UART_HandleTypeDef *huart);
void                            HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart);

void                            HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);

/* Peripheral Control functions  ************************************************/
HAL_StatusTypeDef               HAL_LIN_SendBreak(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_MultiProcessor_EnterMuteMode(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_MultiProcessor_ExitMuteMode(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_HalfDuplex_EnableTransmitter(UART_HandleTypeDef *huart);
HAL_StatusTypeDef               HAL_HalfDuplex_EnableReceiver(UART_HandleTypeDef *huart);

/* Peripheral State functions  **************************************************/
HAL_UART_StateTypeDef          HAL_UART_GetState(const UART_HandleTypeDef *huart);
uint32_t                        HAL_UART_GetError(const UART_HandleTypeDef *huart);
HAL_StatusTypeDef               UART_Start_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef               UART_Start_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);

/**
  * end of UART_Function_Definitions @}
  */


/**
  * end of UART @}
  */


#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_UART_H_ */
