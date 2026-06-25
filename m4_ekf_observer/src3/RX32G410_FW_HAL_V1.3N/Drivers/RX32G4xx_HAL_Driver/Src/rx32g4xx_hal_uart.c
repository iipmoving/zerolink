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
*   File    : rx32g4xx_hal_uart.c
*   By      : RX_DV_Team
**************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"


/** @defgroup UART UART
  * @{
  */

#ifdef HAL_UART_MODULE_ENABLED

/* Exported types ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/** @defgroup UART_Private_Functions  UART Private Functions
  * @{
  */

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
void                UART_InitCallbacksToDefault(UART_HandleTypeDef *huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
static void         UART_EndTxTransfer(UART_HandleTypeDef *huart);
static void         UART_EndRxTransfer(UART_HandleTypeDef *huart);
static void         UART_DMATransmitCplt(DMA_HandleTypeDef *hdma);
static void         UART_DMAReceiveCplt(DMA_HandleTypeDef *hdma);
static void         UART_DMATxHalfCplt(DMA_HandleTypeDef *hdma);
static void         UART_DMARxHalfCplt(DMA_HandleTypeDef *hdma);
static void         UART_DMAError(DMA_HandleTypeDef *hdma);
static void         UART_DMAAbortOnError(DMA_HandleTypeDef *hdma);
static void         UART_DMATxAbortCallback(DMA_HandleTypeDef *hdma);
static void         UART_DMARxAbortCallback(DMA_HandleTypeDef *hdma);
static void         UART_DMATxOnlyAbortCallback(DMA_HandleTypeDef *hdma);
static void         UART_DMARxOnlyAbortCallback(DMA_HandleTypeDef *hdma);
static              HAL_StatusTypeDef UART_Transmit_IT(UART_HandleTypeDef *huart);
static              HAL_StatusTypeDef UART_EndTransmit_IT(UART_HandleTypeDef *huart);
static              HAL_StatusTypeDef UART_Receive_IT(UART_HandleTypeDef *huart);
static              HAL_StatusTypeDef UART_WaitOnFlagUntilTimeout(UART_HandleTypeDef *huart, uint32_t Flag, FlagStatus Status,
                                                     uint32_t Tickstart, uint32_t Timeout);
static void         UART_SetConfig(UART_HandleTypeDef *huart);

/**
  * end of UART_Private_Functions @}
  */

/* Exported functions --------------------------------------------------------*/

/** @addtogroup UART_Function_Definitions UART Function Definitions
  * @{
  */

/**
  * @brief  Initializes the UART mode according to the specified parameters in
  *         the UART_InitTypeDef and create the associated handle.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart)
{
    /* Check the UART handle allocation */
    if (huart == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    if (huart->Init.HwFlowCtl != UART_HWCONTROL_NONE)
    {
        /* The hardware flow control is available only for UART1, UART2 and UART3 */
        assert_param(IS_UART_HWFLOW_INSTANCE(huart->Instance));
        assert_param(IS_UART_HARDWARE_FLOW_CONTROL(huart->Init.HwFlowCtl));
    }
    else
    {
        assert_param(IS_UART_INSTANCE(huart->Instance));
    }
    assert_param(IS_UART_WORD_LENGTH(huart->Init.WordLength));

    if (huart->gState == HAL_UART_STATE_RESET)
    {
        /* Allocate lock resource and initialize it */
        huart->Lock = HAL_UNLOCKED;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        UART_InitCallbacksToDefault(huart);

        if (huart->MspInitCallback == NULL)
        {
            huart->MspInitCallback = HAL_UART_MspInit;
        }

        /* Init the low level hardware */
        huart->MspInitCallback(huart);
#else
        /* Init the low level hardware : GPIO, CLOCK */
        HAL_UART_MspInit(huart);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS) */
    }

    huart->gState = HAL_UART_STATE_BUSY;

    /* Disable the peripheral */
    __HAL_UART_DISABLE(huart);

    /* Set the UART Communication parameters */
    UART_SetConfig(huart);

    /* In asynchronous mode, the following bits must be kept cleared:
        - LINEN and CLKEN bits in the UART_CR2 register,
        - SCEN, HDSEL and IREN  bits in the UART_CR3 register.*/
    CLEAR_BIT(huart->Instance->CR2, UART_CR2_LINEN);
    CLEAR_BIT(huart->Instance->CR3, UART_CR3_HDSEL);

    /* Enable the peripheral */
    __HAL_UART_ENABLE(huart);

    /* Initialize the UART state */
    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_READY;
    huart->RxState = HAL_UART_STATE_READY;
    huart->RxEventType = HAL_UART_RXEVENT_TC;

    return HAL_OK;
}

/**
  * @brief  Initializes the half-duplex mode according to the specified
  *         parameters in the UART_InitTypeDef and create the associated handle.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_HalfDuplex_Init(UART_HandleTypeDef *huart)
{
    /* Check the UART handle allocation */
    if (huart == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_UART_HALFDUPLEX_INSTANCE(huart->Instance));
    assert_param(IS_UART_WORD_LENGTH(huart->Init.WordLength));

    if (huart->gState == HAL_UART_STATE_RESET)
    {
        /* Allocate lock resource and initialize it */
        huart->Lock = HAL_UNLOCKED;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        UART_InitCallbacksToDefault(huart);

        if (huart->MspInitCallback == NULL)
        {
            huart->MspInitCallback = HAL_UART_MspInit;
        }

        /* Init the low level hardware */
        huart->MspInitCallback(huart);
#else
        /* Init the low level hardware : GPIO, CLOCK */
        HAL_UART_MspInit(huart);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS) */
    }

    huart->gState = HAL_UART_STATE_BUSY;

    /* Disable the peripheral */
    __HAL_UART_DISABLE(huart);

    /* Set the UART Communication parameters */
    UART_SetConfig(huart);

    /* In half-duplex mode, the following bits must be kept cleared:
        - LINEN and CLKEN bits in the UART_CR2 register,
        - SCEN and IREN bits in the UART_CR3 register.*/
    CLEAR_BIT(huart->Instance->CR2, UART_CR2_LINEN);

    /* Enable the Half-Duplex mode by setting the HDSEL bit in the CR3 register */
    SET_BIT(huart->Instance->CR3, UART_CR3_HDSEL);

    /* Enable the peripheral */
    __HAL_UART_ENABLE(huart);

    /* Initialize the UART state*/
    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_READY;
    huart->RxState = HAL_UART_STATE_READY;
    huart->RxEventType = HAL_UART_RXEVENT_TC;

    return HAL_OK;
}

/**
  * @brief  Initializes the LIN mode according to the specified
  *         parameters in the UART_InitTypeDef and create the associated handle.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @param  BreakDetectLength Specifies the LIN break detection length.
  *         This parameter can be one of the following values:
  *            @arg UART_LINBREAKDETECTLENGTH_10B: 10-bit break detection
  *            @arg UART_LINBREAKDETECTLENGTH_11B: 11-bit break detection
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LIN_Init(UART_HandleTypeDef *huart, uint32_t BreakDetectLength)
{
    /* Check the UART handle allocation */
    if (huart == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the LIN UART instance */
    assert_param(IS_UART_LIN_INSTANCE(huart->Instance));

    /* Check the Break detection length parameter */
    assert_param(IS_UART_LIN_BREAK_DETECT_LENGTH(BreakDetectLength));
    assert_param(IS_UART_LIN_WORD_LENGTH(huart->Init.WordLength));

    if (huart->gState == HAL_UART_STATE_RESET)
    {
        /* Allocate lock resource and initialize it */
        huart->Lock = HAL_UNLOCKED;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        UART_InitCallbacksToDefault(huart);

        if (huart->MspInitCallback == NULL)
        {
            huart->MspInitCallback = HAL_UART_MspInit;
        }

        /* Init the low level hardware */
        huart->MspInitCallback(huart);
#else
        /* Init the low level hardware : GPIO, CLOCK */
        HAL_UART_MspInit(huart);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS) */
    }

    huart->gState = HAL_UART_STATE_BUSY;

    /* Disable the peripheral */
    __HAL_UART_DISABLE(huart);

    /* Set the UART Communication parameters */
    UART_SetConfig(huart);

    /* Enable the LIN mode by setting the LINEN bit in the CR2 register */
    SET_BIT(huart->Instance->CR2, UART_CR2_LINEN);

    /* Set the UART LIN Break detection length. */
    CLEAR_BIT(huart->Instance->CR2, UART_CR2_LBDL);
    SET_BIT(huart->Instance->CR2, BreakDetectLength);

    /* Enable the peripheral */
    __HAL_UART_ENABLE(huart);

    /* Initialize the UART state*/
    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_READY;
    huart->RxState = HAL_UART_STATE_READY;
    huart->RxEventType = HAL_UART_RXEVENT_TC;

    return HAL_OK;
}

/**
  * @brief  Initializes the Multi-Processor mode according to the specified
  *         parameters in the UART_InitTypeDef and create the associated handle.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @param  Address UART address
  * @param  WakeUpMethod specifies the UART wake-up method.
  *         This parameter can be one of the following values:
  *            @arg UART_WAKEUPMETHOD_IDLELINE: Wake-up by an idle line detection
  *            @arg UART_WAKEUPMETHOD_ADDRESSMARK: Wake-up by an address mark
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_MultiProcessor_Init(UART_HandleTypeDef *huart, uint8_t Address, uint32_t WakeUpMethod)
{
    /* Check the UART handle allocation */
    if (huart == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_UART_INSTANCE(huart->Instance));

    /* Check the Address & wake up method parameters */
    assert_param(IS_UART_WAKEUPMETHOD(WakeUpMethod));
    assert_param(IS_UART_ADDRESS(Address));
    assert_param(IS_UART_WORD_LENGTH(huart->Init.WordLength));

    if (huart->gState == HAL_UART_STATE_RESET)
    {
        /* Allocate lock resource and initialize it */
        huart->Lock = HAL_UNLOCKED;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        UART_InitCallbacksToDefault(huart);

        if (huart->MspInitCallback == NULL)
        {
            huart->MspInitCallback = HAL_UART_MspInit;
        }

        /* Init the low level hardware */
        huart->MspInitCallback(huart);
#else
        /* Init the low level hardware : GPIO, CLOCK */
        HAL_UART_MspInit(huart);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS) */
    }

    huart->gState = HAL_UART_STATE_BUSY;

    /* Disable the peripheral */
    __HAL_UART_DISABLE(huart);

    /* Set the UART Communication parameters */
    UART_SetConfig(huart);

    /* In Multi-Processor mode, the following bits must be kept cleared:
        - LINEN and CLKEN bits in the UART_CR2 register,
        - SCEN, HDSEL and IREN  bits in the UART_CR3 register */
    CLEAR_BIT(huart->Instance->CR2, UART_CR2_LINEN);
    CLEAR_BIT(huart->Instance->CR3, (UART_CR3_HDSEL));

    /* Set the UART address node */
    CLEAR_BIT(huart->Instance->CR2, UART_CR2_ADD);
    SET_BIT(huart->Instance->CR2, Address);

    /* Set the wake up method by setting the WAKE bit in the CR1 register */
    CLEAR_BIT(huart->Instance->CR1, UART_CR1_WAKE);
    SET_BIT(huart->Instance->CR1, WakeUpMethod);

    /* Enable the peripheral */
    __HAL_UART_ENABLE(huart);

    /* Initialize the UART state */
    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_READY;
    huart->RxState = HAL_UART_STATE_READY;
    huart->RxEventType = HAL_UART_RXEVENT_TC;

    return HAL_OK;
}

/**
  * @brief  DeInitializes the UART peripheral.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef *huart)
{
    /* Check the UART handle allocation */
    if (huart == NULL)
    {
        return HAL_ERROR;
    }

    /* Check the parameters */
    assert_param(IS_UART_INSTANCE(huart->Instance));

    huart->gState = HAL_UART_STATE_BUSY;

    /* Disable the Peripheral */
    __HAL_UART_DISABLE(huart);

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    if (huart->MspDeInitCallback == NULL)
    {
        huart->MspDeInitCallback = HAL_UART_MspDeInit;
    }
    /* DeInit the low level hardware */
    huart->MspDeInitCallback(huart);
#else
    /* DeInit the low level hardware */
    HAL_UART_MspDeInit(huart);
#endif /* (USE_HAL_UART_REGISTER_CALLBACKS) */

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_RESET;
    huart->RxState = HAL_UART_STATE_RESET;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
    huart->RxEventType = HAL_UART_RXEVENT_TC;

    /* Process Unlock */
    __HAL_UNLOCK(huart);

    return HAL_OK;
}

/**
  * @brief  UART MSP Init.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
__weak void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    /* NOTE: This function should not be modified, when the callback is needed,
            the HAL_UART_MspInit could be implemented in the user file
    */
}

/**
  * @brief  UART MSP DeInit.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
__weak void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    /* NOTE: This function should not be modified, when the callback is needed,
            the HAL_UART_MspDeInit could be implemented in the user file
    */
}

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
/**
  * @brief  Register a User UART Callback
  *         To be used instead of the weak predefined callback
  * @note   The HAL_UART_RegisterCallback() may be called before HAL_UART_Init(), HAL_HalfDuplex_Init(), HAL_LIN_Init(),
  *         HAL_MultiProcessor_Init() to register callbacks for HAL_UART_MSPINIT_CB_ID and HAL_UART_MSPDEINIT_CB_ID
  * @param  huart uart handle
  * @param  CallbackID ID of the callback to be registered
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_UART_TX_HALFCOMPLETE_CB_ID Tx Half Complete Callback ID
  *           @arg @ref HAL_UART_TX_COMPLETE_CB_ID Tx Complete Callback ID
  *           @arg @ref HAL_UART_RX_HALFCOMPLETE_CB_ID Rx Half Complete Callback ID
  *           @arg @ref HAL_UART_RX_COMPLETE_CB_ID Rx Complete Callback ID
  *           @arg @ref HAL_UART_ERROR_CB_ID Error Callback ID
  *           @arg @ref HAL_UART_ABORT_COMPLETE_CB_ID Abort Complete Callback ID
  *           @arg @ref HAL_UART_ABORT_TRANSMIT_COMPLETE_CB_ID Abort Transmit Complete Callback ID
  *           @arg @ref HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID Abort Receive Complete Callback ID
  *           @arg @ref HAL_UART_MSPINIT_CB_ID MspInit Callback ID
  *           @arg @ref HAL_UART_MSPDEINIT_CB_ID MspDeInit Callback ID
  * @param  pCallback pointer to the Callback function
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_RegisterCallback(UART_HandleTypeDef *huart, HAL_UART_CallbackIDTypeDef CallbackID,
                                            pUART_CallbackTypeDef pCallback)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (pCallback == NULL)
    {
        /* Update the error code */
        huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

        return HAL_ERROR;
    }

    if (huart->gState == HAL_UART_STATE_READY)
    {
        switch (CallbackID)
        {
            case HAL_UART_TX_HALFCOMPLETE_CB_ID :
                huart->TxHalfCpltCallback = pCallback;
                break;

            case HAL_UART_TX_COMPLETE_CB_ID :
                huart->TxCpltCallback = pCallback;
                break;

            case HAL_UART_RX_HALFCOMPLETE_CB_ID :
                huart->RxHalfCpltCallback = pCallback;
                break;

            case HAL_UART_RX_COMPLETE_CB_ID :
                huart->RxCpltCallback = pCallback;
                break;

            case HAL_UART_ERROR_CB_ID :
                huart->ErrorCallback = pCallback;
                break;

            case HAL_UART_ABORT_COMPLETE_CB_ID :
                huart->AbortCpltCallback = pCallback;
                break;

            case HAL_UART_ABORT_TRANSMIT_COMPLETE_CB_ID :
                huart->AbortTransmitCpltCallback = pCallback;
                break;

            case HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID :
                huart->AbortReceiveCpltCallback = pCallback;
                break;

            case HAL_UART_MSPINIT_CB_ID :
                huart->MspInitCallback = pCallback;
                break;

            case HAL_UART_MSPDEINIT_CB_ID :
                huart->MspDeInitCallback = pCallback;
                break;

            default :
                /* Update the error code */
                huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

                /* Return error status */
                status =  HAL_ERROR;
                break;
        }
    }
    else if (huart->gState == HAL_UART_STATE_RESET)
    {
        switch (CallbackID)
        {
            case HAL_UART_MSPINIT_CB_ID :
                huart->MspInitCallback = pCallback;
                break;

            case HAL_UART_MSPDEINIT_CB_ID :
                huart->MspDeInitCallback = pCallback;
                break;

            default :
                /* Update the error code */
                huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

                /* Return error status */
                status =  HAL_ERROR;
                break;
        }
    }
    else
    {
        /* Update the error code */
        huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
    }

    return status;
}

/**
  * @brief  Unregister an UART Callback
  *         UART callaback is redirected to the weak predefined callback
  * @note   The HAL_UART_UnRegisterCallback() may be called before HAL_UART_Init(), HAL_HalfDuplex_Init(),
  *         HAL_LIN_Init(), HAL_MultiProcessor_Init() to un-register callbacks for HAL_UART_MSPINIT_CB_ID
  *         and HAL_UART_MSPDEINIT_CB_ID
  * @param  huart uart handle
  * @param  CallbackID ID of the callback to be unregistered
  *         This parameter can be one of the following values:
  *           @arg @ref HAL_UART_TX_HALFCOMPLETE_CB_ID Tx Half Complete Callback ID
  *           @arg @ref HAL_UART_TX_COMPLETE_CB_ID Tx Complete Callback ID
  *           @arg @ref HAL_UART_RX_HALFCOMPLETE_CB_ID Rx Half Complete Callback ID
  *           @arg @ref HAL_UART_RX_COMPLETE_CB_ID Rx Complete Callback ID
  *           @arg @ref HAL_UART_ERROR_CB_ID Error Callback ID
  *           @arg @ref HAL_UART_ABORT_COMPLETE_CB_ID Abort Complete Callback ID
  *           @arg @ref HAL_UART_ABORT_TRANSMIT_COMPLETE_CB_ID Abort Transmit Complete Callback ID
  *           @arg @ref HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID Abort Receive Complete Callback ID
  *           @arg @ref HAL_UART_MSPINIT_CB_ID MspInit Callback ID
  *           @arg @ref HAL_UART_MSPDEINIT_CB_ID MspDeInit Callback ID
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_UnRegisterCallback(UART_HandleTypeDef *huart, HAL_UART_CallbackIDTypeDef CallbackID)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (HAL_UART_STATE_READY == huart->gState)
    {
        switch (CallbackID)
        {
            case HAL_UART_TX_HALFCOMPLETE_CB_ID :
                huart->TxHalfCpltCallback = HAL_UART_TxHalfCpltCallback;               /* Legacy weak  TxHalfCpltCallback       */
                break;

            case HAL_UART_TX_COMPLETE_CB_ID :
                huart->TxCpltCallback = HAL_UART_TxCpltCallback;                       /* Legacy weak TxCpltCallback            */
                break;

            case HAL_UART_RX_HALFCOMPLETE_CB_ID :
                huart->RxHalfCpltCallback = HAL_UART_RxHalfCpltCallback;               /* Legacy weak RxHalfCpltCallback        */
                break;

            case HAL_UART_RX_COMPLETE_CB_ID :
                huart->RxCpltCallback = HAL_UART_RxCpltCallback;                       /* Legacy weak RxCpltCallback            */
                break;

            case HAL_UART_ERROR_CB_ID :
                huart->ErrorCallback = HAL_UART_ErrorCallback;                         /* Legacy weak ErrorCallback             */
                break;

            case HAL_UART_ABORT_COMPLETE_CB_ID :
                huart->AbortCpltCallback = HAL_UART_AbortCpltCallback;                 /* Legacy weak AbortCpltCallback         */
                break;

            case HAL_UART_ABORT_TRANSMIT_COMPLETE_CB_ID :
                huart->AbortTransmitCpltCallback = HAL_UART_AbortTransmitCpltCallback; /* Legacy weak AbortTransmitCpltCallback */
                break;

            case HAL_UART_ABORT_RECEIVE_COMPLETE_CB_ID :
                huart->AbortReceiveCpltCallback = HAL_UART_AbortReceiveCpltCallback;   /* Legacy weak AbortReceiveCpltCallback  */
                break;

            case HAL_UART_MSPINIT_CB_ID :
                huart->MspInitCallback = HAL_UART_MspInit;                             /* Legacy weak MspInitCallback           */
                break;

            case HAL_UART_MSPDEINIT_CB_ID :
                huart->MspDeInitCallback = HAL_UART_MspDeInit;                         /* Legacy weak MspDeInitCallback         */
                break;

            default :
                /* Update the error code */
                huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

                /* Return error status */
                status =  HAL_ERROR;
                break;
        }
    }
    else if (HAL_UART_STATE_RESET == huart->gState)
    {
        switch (CallbackID)
        {
            case HAL_UART_MSPINIT_CB_ID :
                huart->MspInitCallback = HAL_UART_MspInit;
                break;

            case HAL_UART_MSPDEINIT_CB_ID :
                huart->MspDeInitCallback = HAL_UART_MspDeInit;
                break;

            default :
                /* Update the error code */
                huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

                /* Return error status */
                status =  HAL_ERROR;
                break;
        }
    }
    else
    {
        /* Update the error code */
        huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

        /* Return error status */
        status =  HAL_ERROR;
    }

    return status;
}

/**
  * @brief  Register a User UART Rx Event Callback
  *         To be used instead of the weak predefined callback
  * @param  huart     Uart handle
  * @param  pCallback Pointer to the Rx Event Callback function
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_RegisterRxEventCallback(UART_HandleTypeDef *huart, pUART_RxEventCallbackTypeDef pCallback)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (pCallback == NULL)
    {
        huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

        return HAL_ERROR;
    }

    /* Process locked */
    __HAL_LOCK(huart);

    if (huart->gState == HAL_UART_STATE_READY)
    {
        huart->RxEventCallback = pCallback;
    }
    else
    {
        huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

        status =  HAL_ERROR;
    }

    /* Release Lock */
    __HAL_UNLOCK(huart);

    return status;
}

/**
  * @brief  UnRegister the UART Rx Event Callback
  *         UART Rx Event Callback is redirected to the weak HAL_UARTEx_RxEventCallback() predefined callback
  * @param  huart     Uart handle
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_UnRegisterRxEventCallback(UART_HandleTypeDef *huart)
{
    HAL_StatusTypeDef status = HAL_OK;

    /* Process locked */
    __HAL_LOCK(huart);

    if (huart->gState == HAL_UART_STATE_READY)
    {
        huart->RxEventCallback = HAL_UARTEx_RxEventCallback; /* Legacy weak UART Rx Event Callback  */
    }
    else
    {
        huart->ErrorCode |= HAL_UART_ERROR_INVALID_CALLBACK;

        status =  HAL_ERROR;
    }

    /* Release Lock */
    __HAL_UNLOCK(huart);
    return status;
}
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

/**
  * @brief  Sends an amount of data in blocking mode.
  * @note   When UART parity is not enabled (PCE = 0), and Word Length is configured to 9 bits (M1-M0 = 01),
  *         the sent data is handled as a set of u16. In this case, Size must indicate the number
  *         of u16 provided through pData.
  * @param  huart Pointer to a UART_HandleTypeDef structure that contains
  *               the configuration information for the specified UART module.
  * @param  pData Pointer to data buffer (u8 or u16 data elements).
  * @param  Size  Amount of data elements (u8 or u16) to be sent
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    const uint8_t  *pdata8bits;
    const uint16_t *pdata16bits;
    uint32_t tickstart = 0U;

    /* Check that a Tx process is not already ongoing */
    if (huart->gState == HAL_UART_STATE_READY)
    {
        if ((pData == NULL) || (Size == 0U))
        {
            return  HAL_ERROR;
        }

        huart->ErrorCode = HAL_UART_ERROR_NONE;
        huart->gState = HAL_UART_STATE_BUSY_TX;

        /* Init tickstart for timeout management */
        tickstart = HAL_GetTick();

        huart->TxXferSize = Size;
        huart->TxXferCount = Size;

        /* In case of 9bits/No Parity transfer, pData needs to be handled as a uint16_t pointer */
        if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
        {
            pdata8bits  = NULL;
            pdata16bits = (const uint16_t *) pData;
        }
        else
        {
            pdata8bits  = pData;
            pdata16bits = NULL;
        }

        while (huart->TxXferCount > 0U)
        {
            if (UART_WaitOnFlagUntilTimeout(huart, UART_FLAG_TXE, RESET, tickstart, Timeout) != HAL_OK)
            {
                return HAL_TIMEOUT;
            }
            if (pdata8bits == NULL)
            {
                huart->Instance->DR = (uint16_t)(*pdata16bits & 0x01FFU);
                pdata16bits++;
            }
            else
            {
                huart->Instance->DR = (uint8_t)(*pdata8bits & 0xFFU);
                pdata8bits++;
            }
            huart->TxXferCount--;
        }

        if (UART_WaitOnFlagUntilTimeout(huart, UART_FLAG_TC, RESET, tickstart, Timeout) != HAL_OK)
        {
            return HAL_TIMEOUT;
        }

        /* At end of Tx process, restore huart->gState to Ready */
        huart->gState = HAL_UART_STATE_READY;

        return HAL_OK;
    }
    else
    {
        return HAL_BUSY;
    }
}

/**
  * @brief  Receives an amount of data in blocking mode.
  * @note   When UART parity is not enabled (PCE = 0), and Word Length is configured to 9 bits (M1-M0 = 01),
  *         the received data is handled as a set of u16. In this case, Size must indicate the number
  *         of u16 available through pData.
  * @param  huart Pointer to a UART_HandleTypeDef structure that contains
  *               the configuration information for the specified UART module.
  * @param  pData Pointer to data buffer (u8 or u16 data elements).
  * @param  Size  Amount of data elements (u8 or u16) to be received.
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    uint8_t  *pdata8bits;
    uint16_t *pdata16bits;
    uint32_t tickstart = 0U;

    /* Check that a Rx process is not already ongoing */
    if (huart->RxState == HAL_UART_STATE_READY)
    {
        if ((pData == NULL) || (Size == 0U))
        {
            return  HAL_ERROR;
        }

        huart->ErrorCode = HAL_UART_ERROR_NONE;
        huart->RxState = HAL_UART_STATE_BUSY_RX;
        huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

        /* Init tickstart for timeout management */
        tickstart = HAL_GetTick();

        huart->RxXferSize = Size;
        huart->RxXferCount = Size;

        /* In case of 9bits/No Parity transfer, pRxData needs to be handled as a uint16_t pointer */
        if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
        {
            pdata8bits  = NULL;
            pdata16bits = (uint16_t *) pData;
        }
        else
        {
            pdata8bits  = pData;
            pdata16bits = NULL;
        }

        /* Check the remain data to be received */
        while (huart->RxXferCount > 0U)
        {
            if (UART_WaitOnFlagUntilTimeout(huart, UART_FLAG_RXNE, RESET, tickstart, Timeout) != HAL_OK)
            {
                return HAL_TIMEOUT;
            }
            if (pdata8bits == NULL)
            {
                *pdata16bits = (uint16_t)(huart->Instance->DR & 0x01FF);
                pdata16bits++;
            }
            else
            {
                if ((huart->Init.WordLength == UART_WORDLENGTH_9B) || ((huart->Init.WordLength == UART_WORDLENGTH_8B) && (huart->Init.Parity == UART_PARITY_NONE)))
                {
                    *pdata8bits = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
                }
                else
                {
                    *pdata8bits = (uint8_t)(huart->Instance->DR & (uint8_t)0x007F);
                }
                pdata8bits++;
            }
            huart->RxXferCount--;
        }

        /* At end of Rx process, restore huart->RxState to Ready */
        huart->RxState = HAL_UART_STATE_READY;

        return HAL_OK;
    }
    else
    {
        return HAL_BUSY;
    }
}

/**
  * @brief  Sends an amount of data in non blocking mode.
  * @note   When UART parity is not enabled (PCE = 0), and Word Length is configured to 9 bits (M1-M0 = 01),
  *         the sent data is handled as a set of u16. In this case, Size must indicate the number
  *         of u16 provided through pData.
  * @param  huart Pointer to a UART_HandleTypeDef structure that contains
  *               the configuration information for the specified UART module.
  * @param  pData Pointer to data buffer (u8 or u16 data elements).
  * @param  Size  Amount of data elements (u8 or u16) to be sent
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size)
{
    /* Check that a Tx process is not already ongoing */
    if (huart->gState == HAL_UART_STATE_READY)
    {
        if ((pData == NULL) || (Size == 0U))
        {
            return HAL_ERROR;
        }

        huart->pTxBuffPtr = pData;
        huart->TxXferSize = Size;
        huart->TxXferCount = Size;

        huart->ErrorCode = HAL_UART_ERROR_NONE;
        huart->gState = HAL_UART_STATE_BUSY_TX;

        /* Enable the UART Transmit data register empty Interrupt */
        __HAL_UART_ENABLE_IT(huart, UART_IT_TXE);

        return HAL_OK;
    }
    else
    {
        return HAL_BUSY;
    }
}

/**
  * @brief  Receives an amount of data in non blocking mode.
  * @note   When UART parity is not enabled (PCE = 0), and Word Length is configured to 9 bits (M1-M0 = 01),
  *         the received data is handled as a set of u16. In this case, Size must indicate the number
  *         of u16 available through pData.
  * @param  huart Pointer to a UART_HandleTypeDef structure that contains
  *               the configuration information for the specified UART module.
  * @param  pData Pointer to data buffer (u8 or u16 data elements).
  * @param  Size  Amount of data elements (u8 or u16) to be received.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    /* Check that a Rx process is not already ongoing */
    if (huart->RxState == HAL_UART_STATE_READY)
    {
        if ((pData == NULL) || (Size == 0U))
        {
            return HAL_ERROR;
        }

        /* Set Reception type to Standard reception */
        huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

        return (UART_Start_Receive_IT(huart, pData, Size));
    }
    else
    {
        return HAL_BUSY;
    }
}

/**
  * @brief  Sends an amount of data in DMA mode.
  * @note   When UART parity is not enabled (PCE = 0), and Word Length is configured to 9 bits (M1-M0 = 01),
  *         the sent data is handled as a set of u16. In this case, Size must indicate the number
  *         of u16 provided through pData.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @param  pData Pointer to data buffer (u8 or u16 data elements).
  * @param  Size  Amount of data elements (u8 or u16) to be sent
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, const uint8_t *pData, uint16_t Size)
{
    const uint32_t *tmp;

    /* Check that a Tx process is not already ongoing */
    if (huart->gState == HAL_UART_STATE_READY)
    {
        if ((pData == NULL) || (Size == 0U))
        {
            return HAL_ERROR;
        }

        huart->pTxBuffPtr = pData;
        huart->TxXferSize = Size;
        huart->TxXferCount = Size;

        huart->ErrorCode = HAL_UART_ERROR_NONE;
        huart->gState = HAL_UART_STATE_BUSY_TX;

        /* Set the UART DMA transfer complete callback */
        huart->hdmatx->XferCpltCallback = UART_DMATransmitCplt;

        /* Set the UART DMA Half transfer complete callback */
        huart->hdmatx->XferHalfCpltCallback = UART_DMATxHalfCplt;

        /* Set the DMA error callback */
        huart->hdmatx->XferErrorCallback = UART_DMAError;

        /* Set the DMA abort callback */
        huart->hdmatx->XferAbortCallback = NULL;

        /* Enable the UART transmit DMA channel */
        tmp = (const uint32_t *)&pData;
        HAL_DMA_Start_IT(huart->hdmatx, *(const uint32_t *)tmp, (uint32_t)&huart->Instance->DR, Size);

        /* Clear the TC flag in the SR register by writing 0 to it */
        __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_TC);

        /* Enable the DMA transfer for transmit request by setting the DMAT bit
        in the UART CR3 register */
        SET_BIT(huart->Instance->CR3, UART_CR3_DMAT);

        return HAL_OK;
    }
    else
    {
        return HAL_BUSY;
    }
}

/**
  * @brief  Receives an amount of data in DMA mode.
  * @note   When UART parity is not enabled (PCE = 0), and Word Length is configured to 9 bits (M1-M0 = 01),
  *         the received data is handled as a set of u16. In this case, Size must indicate the number
  *         of u16 available through pData.
  * @param  huart Pointer to a UART_HandleTypeDef structure that contains
  *               the configuration information for the specified UART module.
  * @param  pData Pointer to data buffer (u8 or u16 data elements).
  * @param  Size  Amount of data elements (u8 or u16) to be received.
  * @note   When the UART parity is enabled (PCE = 1) the received data contains the parity bit.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    /* Check that a Rx process is not already ongoing */
    if (huart->RxState == HAL_UART_STATE_READY)
    {
        if ((pData == NULL) || (Size == 0U))
        {
            return HAL_ERROR;
        }

        /* Set Reception type to Standard reception */
        huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

        return (UART_Start_Receive_DMA(huart, pData, Size));
    }
    else
    {
        return HAL_BUSY;
    }
}

/**
  * @brief Pauses the DMA Transfer.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_DMAPause(UART_HandleTypeDef *huart)
{
    uint32_t dmarequest = 0x00U;

    dmarequest = HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAT);
    if ((huart->gState == HAL_UART_STATE_BUSY_TX) && dmarequest)
    {
        /* Disable the UART DMA Tx request */
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAT);
    }

    dmarequest = HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAR);
    if ((huart->RxState == HAL_UART_STATE_BUSY_RX) && dmarequest)
    {
        /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
        CLEAR_BIT(huart->Instance->CR1, UART_CR1_PEIE);
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_EIE);

        /* Disable the UART DMA Rx request */
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAR);
    }

    return HAL_OK;
}

/**
  * @brief Resumes the DMA Transfer.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_DMAResume(UART_HandleTypeDef *huart)
{

    if (huart->gState == HAL_UART_STATE_BUSY_TX)
    {
        /* Enable the UART DMA Tx request */
        SET_BIT(huart->Instance->CR3, UART_CR3_DMAT);
    }

    if (huart->RxState == HAL_UART_STATE_BUSY_RX)
    {
        /* Clear the Overrun flag before resuming the Rx transfer*/
        __HAL_UART_CLEAR_OREFLAG(huart);

        /* Re-enable PE and ERR (Frame error, noise error, overrun error) interrupts */
        if (huart->Init.Parity != UART_PARITY_NONE)
        {
            SET_BIT(huart->Instance->CR1, UART_CR1_PEIE);
        }
        SET_BIT(huart->Instance->CR3, UART_CR3_EIE);

        /* Enable the UART DMA Rx request */
        SET_BIT(huart->Instance->CR3, UART_CR3_DMAR);
    }

    return HAL_OK;
}

/**
  * @brief Stops the DMA Transfer.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_DMAStop(UART_HandleTypeDef *huart)
{
    uint32_t dmarequest = 0x00U;
    /* The Lock is not implemented on this API to allow the user application
        to call the HAL UART API under callbacks HAL_UART_TxCpltCallback() / HAL_UART_RxCpltCallback():
        when calling HAL_DMA_Abort() API the DMA TX/RX Transfer complete interrupt is generated
        and the correspond call back is executed HAL_UART_TxCpltCallback() / HAL_UART_RxCpltCallback()
        */

    /* Stop UART DMA Tx request if ongoing */
    dmarequest = HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAT);
    if ((huart->gState == HAL_UART_STATE_BUSY_TX) && dmarequest)
    {
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAT);

        /* Abort the UART DMA Tx channel */
        if (huart->hdmatx != NULL)
        {
            HAL_DMA_Abort(huart->hdmatx);
        }
        UART_EndTxTransfer(huart);
    }

    /* Stop UART DMA Rx request if ongoing */
    dmarequest = HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAR);
    if ((huart->RxState == HAL_UART_STATE_BUSY_RX) && dmarequest)
    {
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAR);

        /* Abort the UART DMA Rx channel */
        if (huart->hdmarx != NULL)
        {
            HAL_DMA_Abort(huart->hdmarx);
        }
        UART_EndRxTransfer(huart);
    }

    return HAL_OK;
}

/**
  * @brief Receive an amount of data in blocking mode till either the expected number of data is received or an IDLE event occurs.
  * @note   HAL_OK is returned if reception is completed (expected number of data has been received)
  *         or if reception is stopped after IDLE event (less than the expected number of data has been received)
  *         In this case, RxLen output parameter indicates number of data available in reception buffer.
  * @note   When UART parity is not enabled (PCE = 0), and Word Length is configured to 9 bits (M = 01),
  *         the received data is handled as a set of uint16_t. In this case, Size must indicate the number
  *         of uint16_t available through pData.
  * @param huart   UART handle.
  * @param pData   Pointer to data buffer (uint8_t or uint16_t data elements).
  * @param Size    Amount of data elements (uint8_t or uint16_t) to be received.
  * @param RxLen   Number of data elements finally received (could be lower than Size, in case reception ends on IDLE event)
  * @param Timeout Timeout duration expressed in ms (covers the whole reception sequence).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint16_t *RxLen,
                                           uint32_t Timeout)
{
    uint8_t  *pdata8bits;
    uint16_t *pdata16bits;
    uint32_t tickstart;

    /* Check that a Rx process is not already ongoing */
    if (huart->RxState == HAL_UART_STATE_READY)
    {
        if ((pData == NULL) || (Size == 0U))
        {
            return  HAL_ERROR;
        }

        huart->ErrorCode = HAL_UART_ERROR_NONE;
        huart->RxState = HAL_UART_STATE_BUSY_RX;
        huart->ReceptionType = HAL_UART_RECEPTION_TOIDLE;
        huart->RxEventType = HAL_UART_RXEVENT_TC;

        /* Init tickstart for timeout management */
        tickstart = HAL_GetTick();

        huart->RxXferSize  = Size;
        huart->RxXferCount = Size;

        /* In case of 9bits/No Parity transfer, pRxData needs to be handled as a uint16_t pointer */
        if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
        {
            pdata8bits  = NULL;
            pdata16bits = (uint16_t *) pData;
        }
        else
        {
            pdata8bits  = pData;
            pdata16bits = NULL;
        }

        /* Initialize output number of received elements */
        *RxLen = 0U;

        /* as long as data have to be received */
        while (huart->RxXferCount > 0U)
        {
            /* Check if IDLE flag is set */
            if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
            {
                /* Clear IDLE flag in ISR */
                __HAL_UART_CLEAR_IDLEFLAG(huart);

                /* If Set, but no data ever received, clear flag without exiting loop */
                /* If Set, and data has already been received, this means Idle Event is valid : End reception */
                if (*RxLen > 0U)
                {
                    huart->RxEventType = HAL_UART_RXEVENT_IDLE;
                    huart->RxState = HAL_UART_STATE_READY;

                    return HAL_OK;
                }
            }

            /* Check if RXNE flag is set */
            if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE))
            {
                if (pdata8bits == NULL)
                {
                    *pdata16bits = (uint16_t)(huart->Instance->DR & (uint16_t)0x01FF);
                    pdata16bits++;
                }
                else
                {
                    if ((huart->Init.WordLength == UART_WORDLENGTH_9B) || ((huart->Init.WordLength == UART_WORDLENGTH_8B) && (huart->Init.Parity == UART_PARITY_NONE)))
                    {
                        *pdata8bits = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
                    }
                    else
                    {
                        *pdata8bits = (uint8_t)(huart->Instance->DR & (uint8_t)0x007F);
                    }

                    pdata8bits++;
                }
                /* Increment number of received elements */
                *RxLen += 1U;
                huart->RxXferCount--;
            }

            /* Check for the Timeout */
            if (Timeout != HAL_MAX_DELAY)
            {
                if (((HAL_GetTick() - tickstart) > Timeout) || (Timeout == 0U))
                {
                    huart->RxState = HAL_UART_STATE_READY;

                    return HAL_TIMEOUT;
                }
            }
        }

        /* Set number of received elements in output parameter : RxLen */
        *RxLen = huart->RxXferSize - huart->RxXferCount;
        /* At end of Rx process, restore huart->RxState to Ready */
        huart->RxState = HAL_UART_STATE_READY;

        return HAL_OK;
    }
    else
    {
        return HAL_BUSY;
    }
}

/**
  * @brief Receive an amount of data in interrupt mode till either the expected number of data is received or an IDLE event occurs.
  * @note   Reception is initiated by this function call. Further progress of reception is achieved thanks
  *         to UART interrupts raised by RXNE and IDLE events. Callback is called at end of reception indicating
  *         number of received data elements.
  * @note   When UART parity is not enabled (PCE = 0), and Word Length is configured to 9 bits (M = 01),
  *         the received data is handled as a set of uint16_t. In this case, Size must indicate the number
  *         of uint16_t available through pData.
  * @param huart UART handle.
  * @param pData Pointer to data buffer (uint8_t or uint16_t data elements).
  * @param Size  Amount of data elements (uint8_t or uint16_t) to be received.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    HAL_StatusTypeDef status;

    /* Check that a Rx process is not already ongoing */
    if (huart->RxState == HAL_UART_STATE_READY)
    {
        if ((pData == NULL) || (Size == 0U))
        {
            return HAL_ERROR;
        }

        /* Set Reception type to reception till IDLE Event*/
        huart->ReceptionType = HAL_UART_RECEPTION_TOIDLE;
        huart->RxEventType = HAL_UART_RXEVENT_TC;

        status =  UART_Start_Receive_IT(huart, pData, Size);

        /* Check Rx process has been successfully started */
        if (status == HAL_OK)
        {
            if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
            {
                __HAL_UART_CLEAR_IDLEFLAG(huart);
                SET_BIT(huart->Instance->CR1, UART_CR1_IDLEIE);
            }
            else
            {
                /* In case of errors already pending when reception is started,
                Interrupts may have already been raised and lead to reception abortion.
                (Overrun error for instance).
                In such case Reception Type has been reset to HAL_UART_RECEPTION_STANDARD. */
                status = HAL_ERROR;
            }
        }

        return status;
    }
    else
    {
        return HAL_BUSY;
    }
}

/**
  * @brief Receive an amount of data in DMA mode till either the expected number of data is received or an IDLE event occurs.
  * @note   Reception is initiated by this function call. Further progress of reception is achieved thanks
  *         to DMA services, transferring automatically received data elements in user reception buffer and
  *         calling registered callbacks at half/end of reception. UART IDLE events are also used to consider
  *         reception phase as ended. In all cases, callback execution will indicate number of received data elements.
  * @note   When the UART parity is enabled (PCE = 1), the received data contain
  *         the parity bit (MSB position).
  * @note   When UART parity is not enabled (PCE = 0), and Word Length is configured to 9 bits (M = 01),
  *         the received data is handled as a set of uint16_t. In this case, Size must indicate the number
  *         of uint16_t available through pData.
  * @param huart UART handle.
  * @param pData Pointer to data buffer (uint8_t or uint16_t data elements).
  * @param Size  Amount of data elements (uint8_t or uint16_t) to be received.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    HAL_StatusTypeDef status;

    /* Check that a Rx process is not already ongoing */
    if (huart->RxState == HAL_UART_STATE_READY)
    {
        if ((pData == NULL) || (Size == 0U))
        {
            return HAL_ERROR;
        }

        /* Set Reception type to reception till IDLE Event*/
        huart->ReceptionType = HAL_UART_RECEPTION_TOIDLE;
        huart->RxEventType = HAL_UART_RXEVENT_TC;

        status =  UART_Start_Receive_DMA(huart, pData, Size);

        /* Check Rx process has been successfully started */
        if (status == HAL_OK)
        {
            if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
            {
                __HAL_UART_CLEAR_IDLEFLAG(huart);
                SET_BIT(huart->Instance->CR1, UART_CR1_IDLEIE);
            }
            else
            {
                /* In case of errors already pending when reception is started,
                Interrupts may have already been raised and lead to reception abortion.
                (Overrun error for instance).
                In such case Reception Type has been reset to HAL_UART_RECEPTION_STANDARD. */
                status = HAL_ERROR;
            }
        }

        return status;
    }
    else
    {
        return HAL_BUSY;
    }
}

/**
  * @brief Provide Rx Event type that has lead to RxEvent callback execution.
  * @note  When HAL_UARTEx_ReceiveToIdle_IT() or HAL_UARTEx_ReceiveToIdle_DMA() API are called, progress
  *        of reception process is provided to application through calls of Rx Event callback (either default one
  *        HAL_UARTEx_RxEventCallback() or user registered one). As several types of events could occur (IDLE event,
  *        Half Transfer, or Transfer Complete), this function allows to retrieve the Rx Event type that has lead
  *        to Rx Event callback execution.
  * @note  This function is expected to be called within the user implementation of Rx Event Callback,
  *        in order to provide the accurate value :
  *        In Interrupt Mode :
  *           - HAL_UART_RXEVENT_TC : when Reception has been completed (expected nb of data has been received)
  *           - HAL_UART_RXEVENT_IDLE : when Idle event occurred prior reception has been completed (nb of
  *             received data is lower than expected one)
  *        In DMA Mode :
  *           - HAL_UART_RXEVENT_TC : when Reception has been completed (expected nb of data has been received)
  *           - HAL_UART_RXEVENT_HT : when half of expected nb of data has been received
  *           - HAL_UART_RXEVENT_IDLE : when Idle event occurred prior reception has been completed (nb of
  *             received data is lower than expected one).
  *        In DMA mode, RxEvent callback could be called several times;
  *        When DMA is configured in Normal Mode, HT event does not stop Reception process;
  *        When DMA is configured in Circular Mode, HT, TC or IDLE events don't stop Reception process;
  * @param  huart UART handle.
  * @retval Rx Event Type (returned value will be a value of @ref UART_RxEvent_Type_Values)
  */
HAL_UART_RxEventTypeTypeDef HAL_UARTEx_GetRxEventType(UART_HandleTypeDef *huart)
{
  /* Return Rx Event type value, as stored in UART handle */
  return(huart->RxEventType);
}

/**
  * @brief  Abort ongoing transfers (blocking mode).
  * @param  huart UART handle.
  * @note   This procedure could be used for aborting any ongoing transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable UART Interrupts (Tx and Rx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort (in case of transfer in DMA mode)
  *           - Set handle State to READY
  * @note   This procedure is executed in blocking mode : when exiting function, Abort is considered as completed.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Abort(UART_HandleTypeDef *huart)
{
    /* Disable TXEIE, TCIE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
    CLEAR_BIT(huart->Instance->CR1, (UART_CR1_RXNEIE | UART_CR1_PEIE | UART_CR1_TXEIE | UART_CR1_TCIE));
    CLEAR_BIT(huart->Instance->CR3, UART_CR3_EIE);

    /* If Reception till IDLE event was ongoing, disable IDLEIE interrupt */
    if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
    {
        CLEAR_BIT(huart->Instance->CR1, (UART_CR1_IDLEIE));
    }

    /* Disable the UART DMA Tx request if enabled */
    if (HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAT))
    {
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAT);

        /* Abort the UART DMA Tx channel: use blocking DMA Abort API (no callback) */
        if (huart->hdmatx != NULL)
        {
            /* Set the UART DMA Abort callback to Null.
                No call back execution at end of DMA abort procedure */
            huart->hdmatx->XferAbortCallback = NULL;

            if (HAL_DMA_Abort(huart->hdmatx) != HAL_OK)
            {
                if (HAL_DMA_GetError(huart->hdmatx) == HAL_DMA_ERROR_TIMEOUT)
                {
                    /* Set error code to DMA */
                    huart->ErrorCode = HAL_UART_ERROR_DMA;

                    return HAL_TIMEOUT;
                }
            }
        }
    }

    /* Disable the UART DMA Rx request if enabled */
    if (HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAR))
    {
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAR);

        /* Abort the UART DMA Rx channel: use blocking DMA Abort API (no callback) */
        if (huart->hdmarx != NULL)
        {
            /* Set the UART DMA Abort callback to Null.
                No call back execution at end of DMA abort procedure */
            huart->hdmarx->XferAbortCallback = NULL;

            if (HAL_DMA_Abort(huart->hdmarx) != HAL_OK)
            {
                if (HAL_DMA_GetError(huart->hdmarx) == HAL_DMA_ERROR_TIMEOUT)
                {
                    /* Set error code to DMA */
                    huart->ErrorCode = HAL_UART_ERROR_DMA;

                    return HAL_TIMEOUT;
                }
            }
        }
    }

    /* Reset Tx and Rx transfer counters */
    huart->TxXferCount = 0x00U;
    huart->RxXferCount = 0x00U;

    /* Reset ErrorCode */
    huart->ErrorCode = HAL_UART_ERROR_NONE;

    /* Restore huart->RxState and huart->gState to Ready */
    huart->RxState = HAL_UART_STATE_READY;
    huart->gState = HAL_UART_STATE_READY;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

    return HAL_OK;
}

/**
  * @brief  Abort ongoing Transmit transfer (blocking mode).
  * @param  huart UART handle.
  * @note   This procedure could be used for aborting any ongoing Tx transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable UART Interrupts (Tx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort (in case of transfer in DMA mode)
  *           - Set handle State to READY
  * @note   This procedure is executed in blocking mode : when exiting function, Abort is considered as completed.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_AbortTransmit(UART_HandleTypeDef *huart)
{
    /* Disable TXEIE and TCIE interrupts */
    CLEAR_BIT(huart->Instance->CR1, (UART_CR1_TXEIE | UART_CR1_TCIE));

    /* Disable the UART DMA Tx request if enabled */
    if (HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAT))
    {
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAT);

        /* Abort the UART DMA Tx channel : use blocking DMA Abort API (no callback) */
        if (huart->hdmatx != NULL)
        {
            /* Set the UART DMA Abort callback to Null.
                No call back execution at end of DMA abort procedure */
            huart->hdmatx->XferAbortCallback = NULL;

            if (HAL_DMA_Abort(huart->hdmatx) != HAL_OK)
            {
                if (HAL_DMA_GetError(huart->hdmatx) == HAL_DMA_ERROR_TIMEOUT)
                {
                    /* Set error code to DMA */
                    huart->ErrorCode = HAL_UART_ERROR_DMA;

                    return HAL_TIMEOUT;
                }
            }
        }
    }

    /* Reset Tx transfer counter */
    huart->TxXferCount = 0x00U;

    /* Restore huart->gState to Ready */
    huart->gState = HAL_UART_STATE_READY;

    return HAL_OK;
}

/**
  * @brief  Abort ongoing Receive transfer (blocking mode).
  * @param  huart UART handle.
  * @note   This procedure could be used for aborting any ongoing Rx transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable UART Interrupts (Rx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort (in case of transfer in DMA mode)
  *           - Set handle State to READY
  * @note   This procedure is executed in blocking mode : when exiting function, Abort is considered as completed.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef *huart)
{
    /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
    CLEAR_BIT(huart->Instance->CR1, (UART_CR1_RXNEIE | UART_CR1_PEIE));
    CLEAR_BIT(huart->Instance->CR3, UART_CR3_EIE);

    /* If Reception till IDLE event was ongoing, disable IDLEIE interrupt */
    if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
    {
        CLEAR_BIT(huart->Instance->CR1, (UART_CR1_IDLEIE));
    }

    /* Disable the UART DMA Rx request if enabled */
    if (HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAR))
    {
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAR);

        /* Abort the UART DMA Rx channel : use blocking DMA Abort API (no callback) */
        if (huart->hdmarx != NULL)
        {
            /* Set the UART DMA Abort callback to Null.
                No call back execution at end of DMA abort procedure */
            huart->hdmarx->XferAbortCallback = NULL;

            if (HAL_DMA_Abort(huart->hdmarx) != HAL_OK)
            {
                if (HAL_DMA_GetError(huart->hdmarx) == HAL_DMA_ERROR_TIMEOUT)
                {
                    /* Set error code to DMA */
                    huart->ErrorCode = HAL_UART_ERROR_DMA;

                    return HAL_TIMEOUT;
                }
            }
        }
    }

    /* Reset Rx transfer counter */
    huart->RxXferCount = 0x00U;

    /* Restore huart->RxState to Ready */
    huart->RxState = HAL_UART_STATE_READY;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

    return HAL_OK;
}

/**
  * @brief  Abort ongoing transfers (Interrupt mode).
  * @param  huart UART handle.
  * @note   This procedure could be used for aborting any ongoing transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable UART Interrupts (Tx and Rx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort_IT (in case of transfer in DMA mode)
  *           - Set handle State to READY
  *           - At abort completion, call user abort complete callback
  * @note   This procedure is executed in Interrupt mode, meaning that abort procedure could be
  *         considered as completed only when user abort complete callback is executed (not when exiting function).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_Abort_IT(UART_HandleTypeDef *huart)
{
    uint32_t AbortCplt = 0x01U;

    /* Disable TXEIE, TCIE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
    CLEAR_BIT(huart->Instance->CR1, (UART_CR1_RXNEIE | UART_CR1_PEIE | UART_CR1_TXEIE | UART_CR1_TCIE));
    CLEAR_BIT(huart->Instance->CR3, UART_CR3_EIE);

    /* If Reception till IDLE event was ongoing, disable IDLEIE interrupt */
    if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
    {
        CLEAR_BIT(huart->Instance->CR1, (UART_CR1_IDLEIE));
    }

    /* If DMA Tx and/or DMA Rx Handles are associated to UART Handle, DMA Abort complete callbacks should be initialised
        before any call to DMA Abort functions */
    /* DMA Tx Handle is valid */
    if (huart->hdmatx != NULL)
    {
        /* Set DMA Abort Complete callback if UART DMA Tx request if enabled.
        Otherwise, set it to NULL */
        if (HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAT))
        {
            huart->hdmatx->XferAbortCallback = UART_DMATxAbortCallback;
        }
        else
        {
            huart->hdmatx->XferAbortCallback = NULL;
        }
    }
    /* DMA Rx Handle is valid */
    if (huart->hdmarx != NULL)
    {
        /* Set DMA Abort Complete callback if UART DMA Rx request if enabled.
        Otherwise, set it to NULL */
        if (HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAR))
        {
            huart->hdmarx->XferAbortCallback = UART_DMARxAbortCallback;
        }
        else
        {
            huart->hdmarx->XferAbortCallback = NULL;
        }
    }

    /* Disable the UART DMA Tx request if enabled */
    if (HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAT))
    {
        /* Disable DMA Tx at UART level */
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAT);

        /* Abort the UART DMA Tx channel : use non blocking DMA Abort API (callback) */
        if (huart->hdmatx != NULL)
        {
            /* UART Tx DMA Abort callback has already been initialised :
                will lead to call HAL_UART_AbortCpltCallback() at end of DMA abort procedure */

            /* Abort DMA TX */
            if (HAL_DMA_Abort_IT(huart->hdmatx) != HAL_OK)
            {
                huart->hdmatx->XferAbortCallback = NULL;
            }
            else
            {
                AbortCplt = 0x00U;
            }
        }
    }

    /* Disable the UART DMA Rx request if enabled */
    if (HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAR))
    {
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAR);

        /* Abort the UART DMA Rx channel : use non blocking DMA Abort API (callback) */
        if (huart->hdmarx != NULL)
        {
            /* UART Rx DMA Abort callback has already been initialised :
                will lead to call HAL_UART_AbortCpltCallback() at end of DMA abort procedure */

            /* Abort DMA RX */
            if (HAL_DMA_Abort_IT(huart->hdmarx) != HAL_OK)
            {
                huart->hdmarx->XferAbortCallback = NULL;
                AbortCplt = 0x01U;
            }
            else
            {
                AbortCplt = 0x00U;
            }
        }
    }

    /* if no DMA abort complete callback execution is required => call user Abort Complete callback */
    if (AbortCplt == 0x01U)
    {
        /* Reset Tx and Rx transfer counters */
        huart->TxXferCount = 0x00U;
        huart->RxXferCount = 0x00U;

        /* Reset ErrorCode */
        huart->ErrorCode = HAL_UART_ERROR_NONE;

        /* Restore huart->gState and huart->RxState to Ready */
        huart->gState  = HAL_UART_STATE_READY;
        huart->RxState = HAL_UART_STATE_READY;
        huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

        /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /* Call registered Abort complete callback */
        huart->AbortCpltCallback(huart);
#else
        /* Call legacy weak Abort complete callback */
        HAL_UART_AbortCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    }

    return HAL_OK;
}

/**
  * @brief  Abort ongoing Transmit transfer (Interrupt mode).
  * @param  huart UART handle.
  * @note   This procedure could be used for aborting any ongoing Tx transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable UART Interrupts (Tx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort_IT (in case of transfer in DMA mode)
  *           - Set handle State to READY
  *           - At abort completion, call user abort complete callback
  * @note   This procedure is executed in Interrupt mode, meaning that abort procedure could be
  *         considered as completed only when user abort complete callback is executed (not when exiting function).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_AbortTransmit_IT(UART_HandleTypeDef *huart)
{
    /* Disable TXEIE and TCIE interrupts */
    CLEAR_BIT(huart->Instance->CR1, (UART_CR1_TXEIE | UART_CR1_TCIE));

    /* Disable the UART DMA Tx request if enabled */
    if (HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAT))
    {
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAT);

        /* Abort the UART DMA Tx channel : use blocking DMA Abort API (no callback) */
        if (huart->hdmatx != NULL)
        {
            /* Set the UART DMA Abort callback :
                will lead to call HAL_UART_AbortCpltCallback() at end of DMA abort procedure */
            huart->hdmatx->XferAbortCallback = UART_DMATxOnlyAbortCallback;

            /* Abort DMA TX */
            if (HAL_DMA_Abort_IT(huart->hdmatx) != HAL_OK)
            {
                /* Call Directly huart->hdmatx->XferAbortCallback function in case of error */
                huart->hdmatx->XferAbortCallback(huart->hdmatx);
            }
        }
        else
        {
            /* Reset Tx transfer counter */
            huart->TxXferCount = 0x00U;

            /* Restore huart->gState to Ready */
            huart->gState = HAL_UART_STATE_READY;

            /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
            /* Call registered Abort Transmit Complete Callback */
            huart->AbortTransmitCpltCallback(huart);
#else
            /* Call legacy weak Abort Transmit Complete Callback */
            HAL_UART_AbortTransmitCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
        }
    }
    else
    {
        /* Reset Tx transfer counter */
        huart->TxXferCount = 0x00U;

        /* Restore huart->gState to Ready */
        huart->gState = HAL_UART_STATE_READY;

        /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /* Call registered Abort Transmit Complete Callback */
        huart->AbortTransmitCpltCallback(huart);
#else
        /* Call legacy weak Abort Transmit Complete Callback */
        HAL_UART_AbortTransmitCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    }

    return HAL_OK;
}

/**
  * @brief  Abort ongoing Receive transfer (Interrupt mode).
  * @param  huart UART handle.
  * @note   This procedure could be used for aborting any ongoing Rx transfer started in Interrupt or DMA mode.
  *         This procedure performs following operations :
  *           - Disable UART Interrupts (Rx)
  *           - Disable the DMA transfer in the peripheral register (if enabled)
  *           - Abort DMA transfer by calling HAL_DMA_Abort_IT (in case of transfer in DMA mode)
  *           - Set handle State to READY
  *           - At abort completion, call user abort complete callback
  * @note   This procedure is executed in Interrupt mode, meaning that abort procedure could be
  *         considered as completed only when user abort complete callback is executed (not when exiting function).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_UART_AbortReceive_IT(UART_HandleTypeDef *huart)
{
    /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
    CLEAR_BIT(huart->Instance->CR1, (UART_CR1_RXNEIE | UART_CR1_PEIE));
    CLEAR_BIT(huart->Instance->CR3, UART_CR3_EIE);

    /* If Reception till IDLE event was ongoing, disable IDLEIE interrupt */
    if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
    {
        CLEAR_BIT(huart->Instance->CR1, (UART_CR1_IDLEIE));
    }

    /* Disable the UART DMA Rx request if enabled */
    if (HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAR))
    {
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAR);

        /* Abort the UART DMA Rx channel : use blocking DMA Abort API (no callback) */
        if (huart->hdmarx != NULL)
        {
            /* Set the UART DMA Abort callback :
                will lead to call HAL_UART_AbortCpltCallback() at end of DMA abort procedure */
            huart->hdmarx->XferAbortCallback = UART_DMARxOnlyAbortCallback;

            /* Abort DMA RX */
            if (HAL_DMA_Abort_IT(huart->hdmarx) != HAL_OK)
            {
                /* Call Directly huart->hdmarx->XferAbortCallback function in case of error */
                huart->hdmarx->XferAbortCallback(huart->hdmarx);
            }
        }
        else
        {
            /* Reset Rx transfer counter */
            huart->RxXferCount = 0x00U;

            /* Restore huart->RxState to Ready */
            huart->RxState = HAL_UART_STATE_READY;
            huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

            /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
            /* Call registered Abort Receive Complete Callback */
            huart->AbortReceiveCpltCallback(huart);
#else
            /* Call legacy weak Abort Receive Complete Callback */
            HAL_UART_AbortReceiveCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
        }
    }
    else
    {
        /* Reset Rx transfer counter */
        huart->RxXferCount = 0x00U;

        /* Restore huart->RxState to Ready */
        huart->RxState = HAL_UART_STATE_READY;
        huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

        /* As no DMA to be aborted, call directly user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /* Call registered Abort Receive Complete Callback */
        huart->AbortReceiveCpltCallback(huart);
#else
        /* Call legacy weak Abort Receive Complete Callback */
        HAL_UART_AbortReceiveCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    }

    return HAL_OK;
}

/**
  * @brief  This function handles UART interrupt request.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_IRQHandler(UART_HandleTypeDef *huart)
{
    uint32_t isrflags   = READ_REG(huart->Instance->SR);
    uint32_t cr1its     = READ_REG(huart->Instance->CR1);
    uint32_t cr3its     = READ_REG(huart->Instance->CR3);
    uint32_t errorflags = 0x00U;
    uint32_t dmarequest = 0x00U;

    /* If no error occurs */
    errorflags = (isrflags & (uint32_t)(UART_SR_PE | UART_SR_FE | UART_SR_ORE | UART_SR_NE));
    if (errorflags == RESET)
    {
        /* UART in mode Receiver -------------------------------------------------*/
        if (((isrflags & UART_SR_RXNE) != RESET) && ((cr1its & UART_CR1_RXNEIE) != RESET))
        {
            UART_Receive_IT(huart);
            return;
        }
    }

    /* If some errors occur */
    if ((errorflags != RESET) && (((cr3its & UART_CR3_EIE) != RESET)
                                    || ((cr1its & (UART_CR1_RXNEIE | UART_CR1_PEIE)) != RESET)))
    {
        /* UART parity error interrupt occurred ----------------------------------*/
        if (((isrflags & UART_SR_PE) != RESET) && ((cr1its & UART_CR1_PEIE) != RESET))
        {
            huart->ErrorCode |= HAL_UART_ERROR_PE;
        }

        /* UART noise error interrupt occurred -----------------------------------*/
        if (((isrflags & UART_SR_NE) != RESET) && ((cr3its & UART_CR3_EIE) != RESET))
        {
            huart->ErrorCode |= HAL_UART_ERROR_NE;
        }

        /* UART frame error interrupt occurred -----------------------------------*/
        if (((isrflags & UART_SR_FE) != RESET) && ((cr3its & UART_CR3_EIE) != RESET))
        {
            huart->ErrorCode |= HAL_UART_ERROR_FE;
        }

        /* UART Over-Run interrupt occurred --------------------------------------*/
        if (((isrflags & UART_SR_ORE) != RESET) && (((cr1its & UART_CR1_RXNEIE) != RESET)
                                                    || ((cr3its & UART_CR3_EIE) != RESET)))
        {
            huart->ErrorCode |= HAL_UART_ERROR_ORE;
        }

        /* Call UART Error Call back function if need be --------------------------*/
        if (huart->ErrorCode != HAL_UART_ERROR_NONE)
        {
            /* UART in mode Receiver -----------------------------------------------*/
            if (((isrflags & UART_SR_RXNE) != RESET) && ((cr1its & UART_CR1_RXNEIE) != RESET))
            {
                UART_Receive_IT(huart);
            }

            /* If Overrun error occurs, or if any error occurs in DMA mode reception,
                consider error as blocking */
            dmarequest = HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAR);
            if (((huart->ErrorCode & HAL_UART_ERROR_ORE) != RESET) || dmarequest)
            {
                /* Blocking error : transfer is aborted
                Set the UART state ready to be able to start again the process,
                Disable Rx Interrupts, and disable Rx DMA request, if ongoing */
                UART_EndRxTransfer(huart);

                /* Disable the UART DMA Rx request if enabled */
                if (HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAR))
                {
                    CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAR);

                    /* Abort the UART DMA Rx channel */
                    if (huart->hdmarx != NULL)
                    {
                        /* Set the UART DMA Abort callback :
                        will lead to call HAL_UART_ErrorCallback() at end of DMA abort procedure */
                        huart->hdmarx->XferAbortCallback = UART_DMAAbortOnError;
                        if (HAL_DMA_Abort_IT(huart->hdmarx) != HAL_OK)
                        {
                            /* Call Directly XferAbortCallback function in case of error */
                            huart->hdmarx->XferAbortCallback(huart->hdmarx);
                        }
                    }
                    else
                    {
                        /* Call user error callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
                        /*Call registered error callback*/
                        huart->ErrorCallback(huart);
#else
                        /*Call legacy weak error callback*/
                        HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
                    }
                }
                else
                {
                    /* Call user error callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
                    /*Call registered error callback*/
                    huart->ErrorCallback(huart);
#else
                    /*Call legacy weak error callback*/
                    HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
                }
            }
            else
            {
                /* Non Blocking error : transfer could go on.
                    Error is notified to user through user error callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
                /*Call registered error callback*/
                huart->ErrorCallback(huart);
#else
                /*Call legacy weak error callback*/
                HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

                huart->ErrorCode = HAL_UART_ERROR_NONE;
            }
        }
        return;
    } /* End if some error occurs */

    /* Check current reception Mode :
        If Reception till IDLE event has been selected : */
    if ((huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
        && ((isrflags & UART_SR_IDLE) != 0U)
        && ((cr1its & UART_SR_IDLE) != 0U))
    {
        __HAL_UART_CLEAR_IDLEFLAG(huart);

        /* Check if DMA mode is enabled in UART */
        if (HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAR))
        {
            /* DMA mode enabled */
            /* Check received length : If all expected data are received, do nothing,
                (DMA cplt callback will be called).
                Otherwise, if at least one data has already been received, IDLE event is to be notified to user */
            uint16_t nb_remaining_rx_data = (uint16_t) __HAL_DMA_GET_COUNTER(huart->hdmarx);
            if ((nb_remaining_rx_data > 0U)
                && (nb_remaining_rx_data < huart->RxXferSize))
            {
                /* Reception is not complete */
                huart->RxXferCount = nb_remaining_rx_data;

                /* In Normal mode, end DMA xfer and HAL UART Rx process*/
                if (huart->hdmarx->Init.Mode != DMA_CIRCULAR)
                {
                    /* Disable PE and ERR (Frame error, noise error, overrun error) interrupts */
                    CLEAR_BIT(huart->Instance->CR1, UART_CR1_PEIE);
                    CLEAR_BIT(huart->Instance->CR3, UART_CR3_EIE);

                    /* Disable the DMA transfer for the receiver request by resetting the DMAR bit
                        in the UART CR3 register */
                    CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAR);

                    /* At end of Rx process, restore huart->RxState to Ready */
                    huart->RxState = HAL_UART_STATE_READY;
                    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

                    CLEAR_BIT(huart->Instance->CR1, UART_CR1_IDLEIE);

                    /* Last bytes received, so no need as the abort is immediate */
                    (void)HAL_DMA_Abort(huart->hdmarx);
                }

                /* Initialize type of RxEvent that correspond to RxEvent callback execution;
                In this case, Rx Event type is Idle Event */
                huart->RxEventType = HAL_UART_RXEVENT_IDLE;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
                /*Call registered Rx Event callback*/
                huart->RxEventCallback(huart, (huart->RxXferSize - huart->RxXferCount));
#else
                /*Call legacy weak Rx Event callback*/
                HAL_UARTEx_RxEventCallback(huart, (huart->RxXferSize - huart->RxXferCount));
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
            }
            return;
        }
        else
        {
            /* DMA mode not enabled */
            /* Check received length : If all expected data are received, do nothing.
                Otherwise, if at least one data has already been received, IDLE event is to be notified to user */
            uint16_t nb_rx_data = huart->RxXferSize - huart->RxXferCount;
            if ((huart->RxXferCount > 0U)
                && (nb_rx_data > 0U))
            {
                /* Disable the UART Parity Error Interrupt and RXNE interrupts */
                CLEAR_BIT(huart->Instance->CR1, (UART_CR1_RXNEIE | UART_CR1_PEIE));

                /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
                CLEAR_BIT(huart->Instance->CR3, UART_CR3_EIE);

                /* Rx process is completed, restore huart->RxState to Ready */
                huart->RxState = HAL_UART_STATE_READY;
                huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

                CLEAR_BIT(huart->Instance->CR1, UART_CR1_IDLEIE);

                /* Initialize type of RxEvent that correspond to RxEvent callback execution;
                In this case, Rx Event type is Idle Event */
                huart->RxEventType = HAL_UART_RXEVENT_IDLE;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
                /*Call registered Rx complete callback*/
                huart->RxEventCallback(huart, nb_rx_data);
#else
                /*Call legacy weak Rx Event callback*/
                HAL_UARTEx_RxEventCallback(huart, nb_rx_data);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
            }
            return;
        }
    }

    /* UART in mode Transmitter ------------------------------------------------*/
    if (((isrflags & UART_SR_TXE) != RESET) && ((cr1its & UART_CR1_TXEIE) != RESET))
    {
        UART_Transmit_IT(huart);
        return;
    }

    /* UART in mode Transmitter end --------------------------------------------*/
    if (((isrflags & UART_SR_TC) != RESET) && ((cr1its & UART_CR1_TCIE) != RESET))
    {
        UART_EndTransmit_IT(huart);
        return;
    }
}

/**
  * @brief  Tx Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
__weak void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    /* NOTE: This function should not be modified, when the callback is needed,
            the HAL_UART_TxCpltCallback could be implemented in the user file
    */
}

/**
  * @brief  Tx Half Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
__weak void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    /* NOTE: This function should not be modified, when the callback is needed,
            the HAL_UART_TxHalfCpltCallback could be implemented in the user file
    */
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
__weak void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    /* NOTE: This function should not be modified, when the callback is needed,
            the HAL_UART_RxCpltCallback could be implemented in the user file
    */
}

/**
  * @brief  Rx Half Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
__weak void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    /* NOTE: This function should not be modified, when the callback is needed,
            the HAL_UART_RxHalfCpltCallback could be implemented in the user file
    */
}

/**
  * @brief  UART error callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
__weak void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    /* NOTE: This function should not be modified, when the callback is needed,
            the HAL_UART_ErrorCallback could be implemented in the user file
    */
}

/**
  * @brief  UART Abort Complete callback.
  * @param  huart UART handle.
  * @retval None
  */
__weak void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);

    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_UART_AbortCpltCallback can be implemented in the user file.
    */
}

/**
  * @brief  UART Abort Complete callback.
  * @param  huart UART handle.
  * @retval None
  */
__weak void HAL_UART_AbortTransmitCpltCallback(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);

    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_UART_AbortTransmitCpltCallback can be implemented in the user file.
    */
}

/**
  * @brief  UART Abort Receive Complete callback.
  * @param  huart UART handle.
  * @retval None
  */
__weak void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);

    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_UART_AbortReceiveCpltCallback can be implemented in the user file.
    */
}

/**
  * @brief  Reception Event Callback (Rx event notification called after use of advanced reception service).
  * @param  huart UART handle
  * @param  Size  Number of data available in application reception buffer (indicates a position in
  *               reception buffer until which, data are available)
  * @retval None
  */
__weak void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(huart);
    UNUSED(Size);

    /* NOTE : This function should not be modified, when the callback is needed,
                the HAL_UARTEx_RxEventCallback can be implemented in the user file.
    */
}

/**
  * @brief  Transmits break characters.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_LIN_SendBreak(UART_HandleTypeDef *huart)
{
    /* Check the parameters */
    assert_param(IS_UART_INSTANCE(huart->Instance));

    /* Process Locked */
    __HAL_LOCK(huart);

    huart->gState = HAL_UART_STATE_BUSY;

    /* Send break characters */
    SET_BIT(huart->Instance->CR1, UART_CR1_SBK);

    huart->gState = HAL_UART_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(huart);

    return HAL_OK;
}

/**
  * @brief  Enters the UART in mute mode.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_MultiProcessor_EnterMuteMode(UART_HandleTypeDef *huart)
{
    /* Check the parameters */
    assert_param(IS_UART_INSTANCE(huart->Instance));

    /* Process Locked */
    __HAL_LOCK(huart);

    huart->gState = HAL_UART_STATE_BUSY;

    /* Enable the UART mute mode  by setting the RWU bit in the CR1 register */
    SET_BIT(huart->Instance->CR1, UART_CR1_RWU);

    huart->gState = HAL_UART_STATE_READY;
    huart->RxEventType = HAL_UART_RXEVENT_TC;

    /* Process Unlocked */
    __HAL_UNLOCK(huart);

    return HAL_OK;
}

/**
  * @brief  Exits the UART mute mode: wake up software.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_MultiProcessor_ExitMuteMode(UART_HandleTypeDef *huart)
{
    /* Check the parameters */
    assert_param(IS_UART_INSTANCE(huart->Instance));

    /* Process Locked */
    __HAL_LOCK(huart);

    huart->gState = HAL_UART_STATE_BUSY;

    /* Disable the UART mute mode by clearing the RWU bit in the CR1 register */
    CLEAR_BIT(huart->Instance->CR1, UART_CR1_RWU);

    huart->gState = HAL_UART_STATE_READY;
    huart->RxEventType = HAL_UART_RXEVENT_TC;

    /* Process Unlocked */
    __HAL_UNLOCK(huart);

    return HAL_OK;
}

/**
  * @brief  Enables the UART transmitter and disables the UART receiver.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_HalfDuplex_EnableTransmitter(UART_HandleTypeDef *huart)
{
    uint32_t tmpreg = 0x00U;

    /* Process Locked */
    __HAL_LOCK(huart);

    huart->gState = HAL_UART_STATE_BUSY;

    /*-------------------------- UART CR1 Configuration -----------------------*/
    tmpreg = huart->Instance->CR1;

    /* Clear TE and RE bits */
    tmpreg &= (uint32_t)~((uint32_t)(UART_CR1_TE | UART_CR1_RE));

    /* Enable the UART's transmit interface by setting the TE bit in the UART CR1 register */
    tmpreg |= (uint32_t)UART_CR1_TE;

    /* Write to UART CR1 */
    WRITE_REG(huart->Instance->CR1, (uint32_t)tmpreg);

    huart->gState = HAL_UART_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(huart);

    return HAL_OK;
}

/**
  * @brief  Enables the UART receiver and disables the UART transmitter.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_HalfDuplex_EnableReceiver(UART_HandleTypeDef *huart)
{
    uint32_t tmpreg = 0x00U;

    /* Process Locked */
    __HAL_LOCK(huart);

    huart->gState = HAL_UART_STATE_BUSY;

    /*-------------------------- UART CR1 Configuration -----------------------*/
    tmpreg = huart->Instance->CR1;

    /* Clear TE and RE bits */
    tmpreg &= (uint32_t)~((uint32_t)(UART_CR1_TE | UART_CR1_RE));

    /* Enable the UART's receive interface by setting the RE bit in the UART CR1 register */
    tmpreg |= (uint32_t)UART_CR1_RE;

    /* Write to UART CR1 */
    WRITE_REG(huart->Instance->CR1, (uint32_t)tmpreg);

    huart->gState = HAL_UART_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(huart);

    return HAL_OK;
}

/**
  * @brief  Returns the UART state.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL state
  */
HAL_UART_StateTypeDef HAL_UART_GetState(const UART_HandleTypeDef *huart)
{
    uint32_t temp1 = 0x00U, temp2 = 0x00U;
    temp1 = huart->gState;
    temp2 = huart->RxState;

    return (HAL_UART_StateTypeDef)(temp1 | temp2);
}

/**
  * @brief  Return the UART error code
  * @param  huart Pointer to a UART_HandleTypeDef structure that contains
  *               the configuration information for the specified UART.
  * @retval UART Error Code
  */
uint32_t HAL_UART_GetError(const UART_HandleTypeDef *huart)
{
    return huart->ErrorCode;
}


/**
  * @brief  Initialize the callbacks to their default values.
  * @param  huart UART handle.
  * @retval none
  */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
void UART_InitCallbacksToDefault(UART_HandleTypeDef *huart)
{
    /* Init the UART Callback settings */
    huart->TxHalfCpltCallback        = HAL_UART_TxHalfCpltCallback;        /* Legacy weak TxHalfCpltCallback        */
    huart->TxCpltCallback            = HAL_UART_TxCpltCallback;            /* Legacy weak TxCpltCallback            */
    huart->RxHalfCpltCallback        = HAL_UART_RxHalfCpltCallback;        /* Legacy weak RxHalfCpltCallback        */
    huart->RxCpltCallback            = HAL_UART_RxCpltCallback;            /* Legacy weak RxCpltCallback            */
    huart->ErrorCallback             = HAL_UART_ErrorCallback;             /* Legacy weak ErrorCallback             */
    huart->AbortCpltCallback         = HAL_UART_AbortCpltCallback;         /* Legacy weak AbortCpltCallback         */
    huart->AbortTransmitCpltCallback = HAL_UART_AbortTransmitCpltCallback; /* Legacy weak AbortTransmitCpltCallback */
    huart->AbortReceiveCpltCallback  = HAL_UART_AbortReceiveCpltCallback;  /* Legacy weak AbortReceiveCpltCallback  */
    huart->RxEventCallback           = HAL_UARTEx_RxEventCallback;         /* Legacy weak RxEventCallback           */

}
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

/**
  * @brief  DMA UART transmit process complete callback.
  * @param  hdma  Pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA module.
  * @retval None
  */
static void UART_DMATransmitCplt(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
    /* DMA Normal mode*/
    if ((hdma->Instance->CCR & DMA_CCR_CIRC) == 0U)
    {
        huart->TxXferCount = 0x00U;

        /* Disable the DMA transfer for transmit request by setting the DMAT bit
        in the UART CR3 register */
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAT);

        /* Enable the UART Transmit Complete Interrupt */
        SET_BIT(huart->Instance->CR1, UART_CR1_TCIE);

				huart->gState = HAL_UART_STATE_READY;
    }
    /* DMA Circular mode */
    else
    {
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered Tx complete callback*/
        huart->TxCpltCallback(huart);
#else
        /*Call legacy weak Tx complete callback*/
        HAL_UART_TxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
  }
}

/**
  * @brief DMA UART transmit process half complete callback
  * @param  hdma  Pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA module.
  * @retval None
  */
static void UART_DMATxHalfCplt(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /*Call registered Tx complete callback*/
    huart->TxHalfCpltCallback(huart);
#else
    /*Call legacy weak Tx complete callback*/
    HAL_UART_TxHalfCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA UART receive process complete callback.
  * @param  hdma  Pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA module.
  * @retval None
  */
static void UART_DMAReceiveCplt(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

    /* DMA Normal mode*/
    if ((hdma->Instance->CCR & DMA_CCR_CIRC) == 0U)
    {
        huart->RxXferCount = 0U;

        /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
        CLEAR_BIT(huart->Instance->CR1, UART_CR1_PEIE);
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_EIE);

        /* Disable the DMA transfer for the receiver request by setting the DMAR bit
        in the UART CR3 register */
        CLEAR_BIT(huart->Instance->CR3, UART_CR3_DMAR);

        /* At end of Rx process, restore huart->RxState to Ready */
        huart->RxState = HAL_UART_STATE_READY;

        /* If Reception till IDLE event has been selected, Disable IDLE Interrupt */
        if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
        {
            CLEAR_BIT(huart->Instance->CR1, UART_CR1_IDLEIE);
        }
    }

    /* Initialize type of RxEvent that correspond to RxEvent callback execution;
    In this case, Rx Event type is Transfer Complete */
    huart->RxEventType = HAL_UART_RXEVENT_TC;

    /* Check current reception Mode :
        If Reception till IDLE event has been selected : use Rx Event callback */
    if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
    {
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered Rx Event callback*/
        huart->RxEventCallback(huart, huart->RxXferSize);
#else
        /*Call legacy weak Rx Event callback*/
        HAL_UARTEx_RxEventCallback(huart, huart->RxXferSize);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    }
    else
    {
        /* In other cases : use Rx Complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered Rx complete callback*/
        huart->RxCpltCallback(huart);
#else
        /*Call legacy weak Rx complete callback*/
        HAL_UART_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    }
}

/**
  * @brief DMA UART receive process half complete callback
  * @param  hdma  Pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA module.
  * @retval None
  */
static void UART_DMARxHalfCplt(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

    /* Initialize type of RxEvent that correspond to RxEvent callback execution;
        In this case, Rx Event type is Half Transfer */
    huart->RxEventType = HAL_UART_RXEVENT_HT;

    /* Check current reception Mode :
        If Reception till IDLE event has been selected : use Rx Event callback */
    if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
    {
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered Rx Event callback*/
        huart->RxEventCallback(huart, huart->RxXferSize / 2U);
#else
        /*Call legacy weak Rx Event callback*/
        HAL_UARTEx_RxEventCallback(huart, huart->RxXferSize / 2U);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    }
    else
    {
        /* In other cases : use Rx Half Complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
        /*Call registered Rx Half complete callback*/
        huart->RxHalfCpltCallback(huart);
#else
        /*Call legacy weak Rx Half complete callback*/
        HAL_UART_RxHalfCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    }
}

/**
  * @brief  DMA UART communication error callback.
  * @param  hdma  Pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA module.
  * @retval None
  */
static void UART_DMAError(DMA_HandleTypeDef *hdma)
{
    uint32_t dmarequest = 0x00U;
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

    /* Stop UART DMA Tx request if ongoing */
    dmarequest = HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAT);
    if ((huart->gState == HAL_UART_STATE_BUSY_TX) && dmarequest)
    {
        huart->TxXferCount = 0x00U;
        UART_EndTxTransfer(huart);
    }

    /* Stop UART DMA Rx request if ongoing */
    dmarequest = HAL_IS_BIT_SET(huart->Instance->CR3, UART_CR3_DMAR);
    if ((huart->RxState == HAL_UART_STATE_BUSY_RX) && dmarequest)
    {
        huart->RxXferCount = 0x00U;
        UART_EndRxTransfer(huart);
    }

    huart->ErrorCode |= HAL_UART_ERROR_DMA;
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /*Call registered error callback*/
    huart->ErrorCallback(huart);
#else
    /*Call legacy weak error callback*/
    HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief  This function handles UART Communication Timeout. It waits
  *         until a flag is no longer in the specified status.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @param  Flag specifies the UART flag to check.
  * @param  Status The actual Flag status (SET or RESET).
  * @param  Tickstart Tick start value
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
static HAL_StatusTypeDef UART_WaitOnFlagUntilTimeout(UART_HandleTypeDef *huart, uint32_t Flag, FlagStatus Status,
                                                     uint32_t Tickstart, uint32_t Timeout)
{
    /* Wait until flag is set */
    while ((__HAL_UART_GET_FLAG(huart, Flag) ? SET : RESET) == Status)
    {
        /* Check for the Timeout */
        if (Timeout != HAL_MAX_DELAY)
        {
            if ((Timeout == 0U) || ((HAL_GetTick() - Tickstart) > Timeout))
            {
                /* Disable TXE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts for the interrupt process */
                CLEAR_BIT(huart->Instance->CR1, (UART_CR1_RXNEIE | UART_CR1_PEIE | UART_CR1_TXEIE));
                CLEAR_BIT(huart->Instance->CR3, UART_CR3_EIE);

                huart->gState  = HAL_UART_STATE_READY;
                huart->RxState = HAL_UART_STATE_READY;

                /* Process Unlocked */
                __HAL_UNLOCK(huart);

                return HAL_TIMEOUT;
            }
        }
    }
    return HAL_OK;
}

/**
  * @brief  Start Receive operation in interrupt mode.
  * @note   This function could be called by all HAL UART API providing reception in Interrupt mode.
  * @note   When calling this function, parameters validity is considered as already checked,
  *         i.e. Rx State, buffer address, ...
  *         UART Handle is assumed as Locked.
  * @param  huart UART handle.
  * @param  pData Pointer to data buffer (u8 or u16 data elements).
  * @param  Size  Amount of data elements (u8 or u16) to be received.
  * @retval HAL status
  */
HAL_StatusTypeDef UART_Start_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    huart->pRxBuffPtr = pData;
    huart->RxXferSize = Size;
    huart->RxXferCount = Size;

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_BUSY_RX;

    if (huart->Init.Parity != UART_PARITY_NONE)
    {
        /* Enable the UART Parity Error Interrupt */
        __HAL_UART_ENABLE_IT(huart, UART_IT_PE);
    }

    /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
    __HAL_UART_ENABLE_IT(huart, UART_IT_ERR);

    /* Enable the UART Data Register not empty Interrupt */
    __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);

    return HAL_OK;
}

/**
  * @brief  Start Receive operation in DMA mode.
  * @note   This function could be called by all HAL UART API providing reception in DMA mode.
  * @note   When calling this function, parameters validity is considered as already checked,
  *         i.e. Rx State, buffer address, ...
  *         UART Handle is assumed as Locked.
  * @param  huart UART handle.
  * @param  pData Pointer to data buffer (u8 or u16 data elements).
  * @param  Size  Amount of data elements (u8 or u16) to be received.
  * @retval HAL status
  */
HAL_StatusTypeDef UART_Start_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    uint32_t *tmp;

    huart->pRxBuffPtr = pData;
    huart->RxXferSize = Size;

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_BUSY_RX;

    /* Set the UART DMA transfer complete callback */
    huart->hdmarx->XferCpltCallback = UART_DMAReceiveCplt;

    /* Set the UART DMA Half transfer complete callback */
    huart->hdmarx->XferHalfCpltCallback = UART_DMARxHalfCplt;

    /* Set the DMA error callback */
    huart->hdmarx->XferErrorCallback = UART_DMAError;

    /* Set the DMA abort callback */
    huart->hdmarx->XferAbortCallback = NULL;

    /* Enable the DMA stream */
    tmp = (uint32_t *)&pData;
    HAL_DMA_Start_IT(huart->hdmarx, (uint32_t)&huart->Instance->DR, *(uint32_t *)tmp, Size);

    /* Clear the Overrun flag just before enabling the DMA Rx request: can be mandatory for the second transfer */
    __HAL_UART_CLEAR_OREFLAG(huart);

    if (huart->Init.Parity != UART_PARITY_NONE)
    {
        /* Enable the UART Parity Error Interrupt */
        SET_BIT(huart->Instance->CR1, UART_CR1_PEIE);
    }

    /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
    SET_BIT(huart->Instance->CR3, UART_CR3_EIE);

    /* Enable the DMA transfer for the receiver request by setting the DMAR bit
    in the UART CR3 register */
    SET_BIT(huart->Instance->CR3, UART_CR3_DMAR);

    return HAL_OK;
}

/**
  * @brief  End ongoing Tx transfer on UART peripheral (following error detection or Transmit completion).
  * @param  huart UART handle.
  * @retval None
  */
static void UART_EndTxTransfer(UART_HandleTypeDef *huart)
{
    /* Disable TXEIE and TCIE interrupts */
    CLEAR_BIT(huart->Instance->CR1, (UART_CR1_TXEIE | UART_CR1_TCIE));

    /* At end of Tx process, restore huart->gState to Ready */
    huart->gState = HAL_UART_STATE_READY;
}

/**
  * @brief  End ongoing Rx transfer on UART peripheral (following error detection or Reception completion).
  * @param  huart UART handle.
  * @retval None
  */
static void UART_EndRxTransfer(UART_HandleTypeDef *huart)
{
    /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
    CLEAR_BIT(huart->Instance->CR1, (UART_CR1_RXNEIE | UART_CR1_PEIE));
    CLEAR_BIT(huart->Instance->CR3, UART_CR3_EIE);

    /* In case of reception waiting for IDLE event, disable also the IDLE IE interrupt source */
    if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
    {
        CLEAR_BIT(huart->Instance->CR1, UART_CR1_IDLEIE);
    }

    /* At end of Rx process, restore huart->RxState to Ready */
    huart->RxState = HAL_UART_STATE_READY;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
}

/**
  * @brief  DMA UART communication abort callback, when initiated by HAL services on Error
  *         (To be called at end of DMA Abort procedure following error occurrence).
  * @param  hdma  Pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA module.
  * @retval None
  */
static void UART_DMAAbortOnError(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
    huart->RxXferCount = 0x00U;
    huart->TxXferCount = 0x00U;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /*Call registered error callback*/
    huart->ErrorCallback(huart);
#else
    /*Call legacy weak error callback*/
    HAL_UART_ErrorCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA UART Tx communication abort callback, when initiated by user
  *         (To be called at end of DMA Tx Abort procedure following user abort request).
  * @note   When this callback is executed, User Abort complete call back is called only if no
  *         Abort still ongoing for Rx DMA Handle.
  * @param  hdma  Pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA module.
  * @retval None
  */
static void UART_DMATxAbortCallback(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

    huart->hdmatx->XferAbortCallback = NULL;

    /* Check if an Abort process is still ongoing */
    if (huart->hdmarx != NULL)
    {
        if (huart->hdmarx->XferAbortCallback != NULL)
        {
            return;
        }
    }

    /* No Abort process still ongoing : All DMA channels are aborted, call user Abort Complete callback */
    huart->TxXferCount = 0x00U;
    huart->RxXferCount = 0x00U;

    /* Reset ErrorCode */
    huart->ErrorCode = HAL_UART_ERROR_NONE;

    /* Restore huart->gState and huart->RxState to Ready */
    huart->gState  = HAL_UART_STATE_READY;
    huart->RxState = HAL_UART_STATE_READY;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

    /* Call user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /* Call registered Abort complete callback */
    huart->AbortCpltCallback(huart);
#else
    /* Call legacy weak Abort complete callback */
    HAL_UART_AbortCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA UART Rx communication abort callback, when initiated by user
  *         (To be called at end of DMA Rx Abort procedure following user abort request).
  * @note   When this callback is executed, User Abort complete call back is called only if no
  *         Abort still ongoing for Tx DMA Handle.
  * @param  hdma  Pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA module.
  * @retval None
  */
static void UART_DMARxAbortCallback(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

    huart->hdmarx->XferAbortCallback = NULL;

    /* Check if an Abort process is still ongoing */
    if (huart->hdmatx != NULL)
    {
        if (huart->hdmatx->XferAbortCallback != NULL)
        {
            return;
        }
    }

    /* No Abort process still ongoing : All DMA channels are aborted, call user Abort Complete callback */
    huart->TxXferCount = 0x00U;
    huart->RxXferCount = 0x00U;

    /* Reset ErrorCode */
    huart->ErrorCode = HAL_UART_ERROR_NONE;

    /* Restore huart->gState and huart->RxState to Ready */
    huart->gState  = HAL_UART_STATE_READY;
    huart->RxState = HAL_UART_STATE_READY;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

    /* Call user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /* Call registered Abort complete callback */
    huart->AbortCpltCallback(huart);
#else
    /* Call legacy weak Abort complete callback */
    HAL_UART_AbortCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA UART Tx communication abort callback, when initiated by user by a call to
  *         HAL_UART_AbortTransmit_IT API (Abort only Tx transfer)
  *         (This callback is executed at end of DMA Tx Abort procedure following user abort request,
  *         and leads to user Tx Abort Complete callback execution).
  * @param  hdma  Pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA module.
  * @retval None
  */
static void UART_DMATxOnlyAbortCallback(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

    huart->TxXferCount = 0x00U;

    /* Restore huart->gState to Ready */
    huart->gState = HAL_UART_STATE_READY;

    /* Call user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /* Call registered Abort Transmit Complete Callback */
    huart->AbortTransmitCpltCallback(huart);
#else
    /* Call legacy weak Abort Transmit Complete Callback */
    HAL_UART_AbortTransmitCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA UART Rx communication abort callback, when initiated by user by a call to
  *         HAL_UART_AbortReceive_IT API (Abort only Rx transfer)
  *         (This callback is executed at end of DMA Rx Abort procedure following user abort request,
  *         and leads to user Rx Abort Complete callback execution).
  * @param  hdma  Pointer to a DMA_HandleTypeDef structure that contains
  *               the configuration information for the specified DMA module.
  * @retval None
  */
static void UART_DMARxOnlyAbortCallback(DMA_HandleTypeDef *hdma)
{
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

    huart->RxXferCount = 0x00U;

    /* Restore huart->RxState to Ready */
    huart->RxState = HAL_UART_STATE_READY;
    huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

    /* Call user Abort complete callback */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /* Call registered Abort Receive Complete Callback */
    huart->AbortReceiveCpltCallback(huart);
#else
    /* Call legacy weak Abort Receive Complete Callback */
    HAL_UART_AbortReceiveCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
}

/**
  * @brief  Sends an amount of data in non blocking mode.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
static HAL_StatusTypeDef UART_Transmit_IT(UART_HandleTypeDef *huart)
{
    const uint16_t *tmp;

    /* Check that a Tx process is ongoing */
    if (huart->gState == HAL_UART_STATE_BUSY_TX)
    {
        if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
        {
            tmp = (const uint16_t *) huart->pTxBuffPtr;
            huart->Instance->DR = (uint16_t)(*tmp & (uint16_t)0x01FF);
            huart->pTxBuffPtr += 2U;
        }
        else
        {
            huart->Instance->DR = (uint8_t)(*huart->pTxBuffPtr++ & (uint8_t)0x00FF);
        }

        if (--huart->TxXferCount == 0U)
        {
            /* Disable the UART Transmit Data Register Empty Interrupt */
            __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);

            /* Enable the UART Transmit Complete Interrupt */
            __HAL_UART_ENABLE_IT(huart, UART_IT_TC);
        }
        return HAL_OK;
    }
    else
    {
        return HAL_BUSY;
    }
}

/**
  * @brief  Wraps up transmission in non blocking mode.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
static HAL_StatusTypeDef UART_EndTransmit_IT(UART_HandleTypeDef *huart)
{
    /* Disable the UART Transmit Complete Interrupt */
    __HAL_UART_DISABLE_IT(huart, UART_IT_TC);

    /* Tx process is ended, restore huart->gState to Ready */
    huart->gState = HAL_UART_STATE_READY;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
    /*Call registered Tx complete callback*/
    huart->TxCpltCallback(huart);
#else
    /*Call legacy weak Tx complete callback*/
    HAL_UART_TxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */

    return HAL_OK;
}

/**
  * @brief  Receives an amount of data in non blocking mode
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval HAL status
  */
static HAL_StatusTypeDef UART_Receive_IT(UART_HandleTypeDef *huart)
{
    uint8_t  *pdata8bits;
    uint16_t *pdata16bits;

    /* Check that a Rx process is ongoing */
    if (huart->RxState == HAL_UART_STATE_BUSY_RX)
    {
        if ((huart->Init.WordLength == UART_WORDLENGTH_9B) && (huart->Init.Parity == UART_PARITY_NONE))
        {
            pdata8bits  = NULL;
            pdata16bits = (uint16_t *) huart->pRxBuffPtr;
            *pdata16bits = (uint16_t)(huart->Instance->DR & (uint16_t)0x01FF);
            huart->pRxBuffPtr += 2U;
         }
         else
         {
            pdata8bits = (uint8_t *) huart->pRxBuffPtr;
            pdata16bits  = NULL;

            if ((huart->Init.WordLength == UART_WORDLENGTH_9B) || ((huart->Init.WordLength == UART_WORDLENGTH_8B) && (huart->Init.Parity == UART_PARITY_NONE)))
            {
                *pdata8bits = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
            }
            else
            {
                *pdata8bits = (uint8_t)(huart->Instance->DR & (uint8_t)0x007F);
            }
            huart->pRxBuffPtr += 1U;
        }

        if (--huart->RxXferCount == 0U)
        {
            /* Disable the UART Data Register not empty Interrupt */
            __HAL_UART_DISABLE_IT(huart, UART_IT_RXNE);

            /* Disable the UART Parity Error Interrupt */
            __HAL_UART_DISABLE_IT(huart, UART_IT_PE);

            /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
            __HAL_UART_DISABLE_IT(huart, UART_IT_ERR);

            /* Rx process is completed, restore huart->RxState to Ready */
            huart->RxState = HAL_UART_STATE_READY;

            /* Initialize type of RxEvent to Transfer Complete */
            huart->RxEventType = HAL_UART_RXEVENT_TC;

            /* Check current reception Mode :
            If Reception till IDLE event has been selected : */
            if (huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE)
            {
                /* Set reception type to Standard */
                huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

                /* Disable IDLE interrupt */
                CLEAR_BIT(huart->Instance->CR1, UART_CR1_IDLEIE);

                /* Check if IDLE flag is set */
                if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
                {
                    /* Clear IDLE flag in ISR */
                    __HAL_UART_CLEAR_IDLEFLAG(huart);
                }

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
                /*Call registered Rx Event callback*/
                huart->RxEventCallback(huart, huart->RxXferSize);
#else
                /*Call legacy weak Rx Event callback*/
                HAL_UARTEx_RxEventCallback(huart, huart->RxXferSize);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
            }
            else
            {
                /* Standard reception API called */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
                /*Call registered Rx complete callback*/
                huart->RxCpltCallback(huart);
#else
                /*Call legacy weak Rx complete callback*/
                HAL_UART_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
            }

            return HAL_OK;
        }
        return HAL_OK;
    }
    else
    {
        return HAL_BUSY;
    }
}

/**
  * @brief  Configures the UART peripheral.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
static void UART_SetConfig(UART_HandleTypeDef *huart)
{
    uint32_t tmpreg;
    uint32_t pclk;

    /* Check the parameters */
    assert_param(IS_UART_BAUDRATE(huart->Init.BaudRate));
    assert_param(IS_UART_STOPBITS(huart->Init.StopBits));
    assert_param(IS_UART_PARITY(huart->Init.Parity));
    assert_param(IS_UART_MODE(huart->Init.Mode));

    /*-------------------------- UART CR2 Configuration -----------------------*/
    /* Configure the UART Stop Bits: Set STOP[13:12] bits
        according to huart->Init.StopBits value */
    MODIFY_REG(huart->Instance->CR2, UART_CR2_STOP, huart->Init.StopBits);

    /*-------------------------- UART CR1 Configuration -----------------------*/
    /* Configure the UART Word Length, Parity and mode:
        Set the M bits according to huart->Init.WordLength value
        Set PCE and PS bits according to huart->Init.Parity value
        Set TE and RE bits according to huart->Init.Mode value
        Set OVER8 bit according to huart->Init.OverSampling value */

    tmpreg = (uint32_t)huart->Init.WordLength | huart->Init.Parity | huart->Init.Mode;
    MODIFY_REG(huart->Instance->CR1,
                (uint32_t)(UART_CR1_M | UART_CR1_PCE | UART_CR1_PS | UART_CR1_TE | UART_CR1_RE),
                tmpreg);

    /*-------------------------- UART CR3 Configuration -----------------------*/
    /* Configure the UART HFC: Set CTSE and RTSE bits according to huart->Init.HwFlowCtl value */
    MODIFY_REG(huart->Instance->CR3, (UART_CR3_RTSE | UART_CR3_CTSE), huart->Init.HwFlowCtl);


    if(huart->Instance == UART1)
    {
        pclk = HAL_RCC_GetPCLK2Freq();
    }
    else
    {
        pclk = HAL_RCC_GetPCLK1Freq();
    }

    /*-------------------------- UART BRR Configuration ---------------------*/

    if(pclk<171798691)
      huart->Instance->BRR = UART_BRR_SAMPLING16(pclk, huart->Init.BaudRate);
    else
      huart->Instance->BRR = UART_BRR_SAMPLING16_171(pclk, huart->Init.BaudRate);
    
}

/**
  * end of UART_Function_Definitions @}
  */

#endif /* HAL_UART_MODULE_ENABLED */

/**
  * end of UART @}
  */
