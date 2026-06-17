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
*   File    : rx32g4xx_hal_dma_ex.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_DMA_EX_H_
#define _RX32G4XX_HAL_DMA_EX_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"


/* Exported types ------------------------------------------------------------*/

/** @addtogroup DMAEx
  * @{
  */

/******************************************************************************/
/*                            DMAEx Structures                                */
/******************************************************************************/

/** @defgroup DMAEx_Structure_Definitions DMAEx Structure Definitions
  * @{
  */

/** @defgroup HAL_DMA_MuxSyncConfigTypeDef HAL DMA MuxSyncConfigTypeDef
  * @ingroup  DMAEx_Structure_Definitions
  * @{
  */
/**
  * @brief  HAL DMAMUX Synchronization configuration structure definition
  */
typedef struct
{
  uint32_t SyncSignalID;            /*!< Specifies the synchronization signal gating the DMA request in periodic mode.
                                         This parameter can be a value of @ref DMAEx_DMAMUX_SyncSignalID_selection */

  uint32_t SyncPolarity;            /*!< Specifies the polarity of the signal on which the DMA request is synchronized.
                                         This parameter can be a value of @ref DMAEx_DMAMUX_SyncPolarity_selection */

  FunctionalState SyncEnable;       /*!< Specifies if the synchronization shall be enabled or disabled
                                         This parameter can take the value ENABLE or DISABLE*/


  FunctionalState EventEnable;      /*!< Specifies if an event shall be generated once the RequestNumber is reached.
                                         This parameter can take the value ENABLE or DISABLE */

  uint32_t RequestNumber;           /*!< Specifies the number of DMA request that will be authorized after a sync event
                                         This parameter must be a number between Min_Data = 1 and Max_Data = 32 */


} HAL_DMA_MuxSyncConfigTypeDef;
/**
  * end of HAL_DMA_MuxSyncConfigTypeDef @}
  */

/** @defgroup HAL_DMA_MuxRequestGeneratorConfigTypeDef HAL DMA MuxRequestGeneratorConfigTypeDef
  * @ingroup  DMAEx_Structure_Definitions
  * @{
  */
/**
  * @brief  HAL DMAMUX request generator parameters structure definition
  */
typedef struct
{
  uint32_t SignalID;                /*!< Specifies the ID of the signal used for DMAMUX request generator
                                         This parameter can be a value of @ref DMAEx_DMAMUX_SignalGeneratorID_selection */

  uint32_t Polarity;                /*!< Specifies the polarity of the signal on which the request is generated.
                                         This parameter can be a value of @ref DMAEx_DMAMUX_RequestGeneneratorPolarity_selection */

  uint32_t RequestNumber;           /*!< Specifies the number of DMA request that will be generated after a signal event
                                         This parameter must be a number between Min_Data = 1 and Max_Data = 32 */

} HAL_DMA_MuxRequestGeneratorConfigTypeDef;
/**
  * end of HAL_DMA_MuxRequestGeneratorConfigTypeDef @}
  */

/**
  * end of DMAEx_Structure_Definitions @}
  */


/******************************************************************************/
/*                            DMAEx Parameters                                */
/******************************************************************************/

/** @defgroup DMAEx_Parameter_Definitions DMAEx Parameter Definitions
  * @{
  */

/** @defgroup DMAEx_DMAMUX_SyncSignalID_selection DMAEx DMAMUX SyncSignalID selection
  * @ingroup  DMAEx_Parameter_Definitions
  * @{
  */
#define HAL_DMAMUX1_SYNC_EXTI0                      0U     /*!<  Synchronization Signal is EXTI0  IT   */
#define HAL_DMAMUX1_SYNC_EXTI1                      1U     /*!<  Synchronization Signal is EXTI1  IT   */
#define HAL_DMAMUX1_SYNC_EXTI2                      2U     /*!<  Synchronization Signal is EXTI2  IT   */
#define HAL_DMAMUX1_SYNC_EXTI3                      3U     /*!<  Synchronization Signal is EXTI3  IT   */
#define HAL_DMAMUX1_SYNC_EXTI4                      4U     /*!<  Synchronization Signal is EXTI4  IT   */
#define HAL_DMAMUX1_SYNC_EXTI5                      5U     /*!<  Synchronization Signal is EXTI5  IT   */
#define HAL_DMAMUX1_SYNC_EXTI6                      6U     /*!<  Synchronization Signal is EXTI6  IT   */
#define HAL_DMAMUX1_SYNC_EXTI7                      7U     /*!<  Synchronization Signal is EXTI7  IT   */
#define HAL_DMAMUX1_SYNC_EXTI8                      8U     /*!<  Synchronization Signal is EXTI8  IT   */
#define HAL_DMAMUX1_SYNC_EXTI9                      9U     /*!<  Synchronization Signal is EXTI9  IT   */
#define HAL_DMAMUX1_SYNC_EXTI10                    10U     /*!<  Synchronization Signal is EXTI10 IT   */
#define HAL_DMAMUX1_SYNC_EXTI11                    11U     /*!<  Synchronization Signal is EXTI11 IT   */
#define HAL_DMAMUX1_SYNC_EXTI12                    12U     /*!<  Synchronization Signal is EXTI12 IT   */
#define HAL_DMAMUX1_SYNC_EXTI13                    13U     /*!<  Synchronization Signal is EXTI13 IT   */
#define HAL_DMAMUX1_SYNC_EXTI14                    14U     /*!<  Synchronization Signal is EXTI14 IT   */
#define HAL_DMAMUX1_SYNC_EXTI15                    15U     /*!<  Synchronization Signal is EXTI15 IT   */
#define HAL_DMAMUX1_SYNC_DMAMUX1_CH0_EVT           16U     /*!<  Synchronization Signal is DMAMUX1 Channel0 Event  */
#define HAL_DMAMUX1_SYNC_DMAMUX1_CH1_EVT           17U     /*!<  Synchronization Signal is DMAMUX1 Channel1 Event  */
#define HAL_DMAMUX1_SYNC_DMAMUX1_CH2_EVT           18U     /*!<  Synchronization Signal is DMAMUX1 Channel2 Event  */
#define HAL_DMAMUX1_SYNC_DMAMUX1_CH3_EVT           19U     /*!<  Synchronization Signal is DMAMUX1 Channel3 Event  */
/**
  * end of DMAEx_DMAMUX_SyncSignalID_selection @}
  */

/** @defgroup DMAEx_DMAMUX_SyncPolarity_selection DMAEx DMAMUX SyncPolarity selection
  * @ingroup  DMAEx_Parameter_Definitions
  * @{
  */
#define HAL_DMAMUX_SYNC_NO_EVENT            0x00000000U                         /*!< block synchronization events */
#define HAL_DMAMUX_SYNC_RISING              ((uint32_t)DMAMUX_CxCR_SPOL_BIT0)   /*!< synchronize with rising edge events */
#define HAL_DMAMUX_SYNC_FALLING             ((uint32_t)DMAMUX_CxCR_SPOL_BIT1)   /*!< synchronize with falling edge events */
#define HAL_DMAMUX_SYNC_RISING_FALLING      ((uint32_t)DMAMUX_CxCR_SPOL)        /*!< synchronize with rising and falling edge events */
/**
  * end of DMAEx_DMAMUX_SyncPolarity_selection @}
  */

/** @defgroup DMAEx_DMAMUX_SignalGeneratorID_selection DMAEx DMAMUX SignalGeneratorID selection
  * @ingroup  DMAEx_Parameter_Definitions
  * @{
  */
#define HAL_DMAMUX1_REQ_GEN_EXTI0                0U        /*!< Request generator Signal is EXTI0 IT    */
#define HAL_DMAMUX1_REQ_GEN_EXTI1                1U        /*!< Request generator Signal is EXTI1 IT    */
#define HAL_DMAMUX1_REQ_GEN_EXTI2                2U        /*!< Request generator Signal is EXTI2 IT    */
#define HAL_DMAMUX1_REQ_GEN_EXTI3                3U        /*!< Request generator Signal is EXTI3 IT    */
#define HAL_DMAMUX1_REQ_GEN_EXTI4                4U        /*!< Request generator Signal is EXTI4 IT    */
#define HAL_DMAMUX1_REQ_GEN_EXTI5                5U        /*!< Request generator Signal is EXTI5 IT    */
#define HAL_DMAMUX1_REQ_GEN_EXTI6                6U        /*!< Request generator Signal is EXTI6 IT    */
#define HAL_DMAMUX1_REQ_GEN_EXTI7                7U        /*!< Request generator Signal is EXTI7 IT    */
#define HAL_DMAMUX1_REQ_GEN_EXTI8                8U        /*!< Request generator Signal is EXTI8 IT    */
#define HAL_DMAMUX1_REQ_GEN_EXTI9                9U        /*!< Request generator Signal is EXTI9 IT    */
#define HAL_DMAMUX1_REQ_GEN_EXTI10              10U        /*!< Request generator Signal is EXTI10 IT   */
#define HAL_DMAMUX1_REQ_GEN_EXTI11              11U        /*!< Request generator Signal is EXTI11 IT   */
#define HAL_DMAMUX1_REQ_GEN_EXTI12              12U        /*!< Request generator Signal is EXTI12 IT   */
#define HAL_DMAMUX1_REQ_GEN_EXTI13              13U        /*!< Request generator Signal is EXTI13 IT   */
#define HAL_DMAMUX1_REQ_GEN_EXTI14              14U        /*!< Request generator Signal is EXTI14 IT   */
#define HAL_DMAMUX1_REQ_GEN_EXTI15              15U        /*!< Request generator Signal is EXTI15 IT   */
#define HAL_DMAMUX1_REQ_GEN_DMAMUX1_CH0_EVT     16U        /*!< Request generator Signal is DMAMUX1 Channel0 Event */
#define HAL_DMAMUX1_REQ_GEN_DMAMUX1_CH1_EVT     17U        /*!< Request generator Signal is DMAMUX1 Channel1 Event */
#define HAL_DMAMUX1_REQ_GEN_DMAMUX1_CH2_EVT     18U        /*!< Request generator Signal is DMAMUX1 Channel2 Event */
#define HAL_DMAMUX1_REQ_GEN_DMAMUX1_CH3_EVT     19U        /*!< Request generator Signal is DMAMUX1 Channel3 Event */
/**
  * end of DMAEx_DMAMUX_SignalGeneratorID_selection @}
  */

/** @defgroup DMAEx_DMAMUX_RequestGeneneratorPolarity_selection DMAEx DMAMUX RequestGeneneratorPolarity selection
  * @ingroup  DMAEx_Parameter_Definitions
  * @{
  */
#define HAL_DMAMUX_REQ_GEN_NO_EVENT         0x00000000U             /*!< block request generator events */
#define HAL_DMAMUX_REQ_GEN_RISING           DMAMUX_RGxCR_GPOL_BIT0  /*!< generate request on rising edge events */
#define HAL_DMAMUX_REQ_GEN_FALLING          DMAMUX_RGxCR_GPOL_BIT1  /*!< generate request on falling edge events */
#define HAL_DMAMUX_REQ_GEN_RISING_FALLING   DMAMUX_RGxCR_GPOL       /*!< generate request on rising and falling edge events */
/**
  * end of DMAEx_DMAMUX_RequestGeneneratorPolarity_selection @}
  */

/**
  * end of DMAEx_Parameter_Definitions @}
  */


/******************************************************************************/
/*                              DMAEx Macro                                   */
/******************************************************************************/

/** @defgroup DMAEx_Macro_Definitions DMAEx Macro Definitions
  * @{
  */

#define IS_DMAMUX_SYNC_SIGNAL_ID(SIGNAL_ID) ((SIGNAL_ID) <= HAL_DMAMUX1_SYNC_DMAMUX1_CH3_EVT)

#define IS_DMAMUX_SYNC_REQUEST_NUMBER(REQUEST_NUMBER) (((REQUEST_NUMBER) > 0U) && ((REQUEST_NUMBER) <= 32U))

#define IS_DMAMUX_SYNC_POLARITY(POLARITY) (((POLARITY) == HAL_DMAMUX_SYNC_NO_EVENT)    || \
                                           ((POLARITY) == HAL_DMAMUX_SYNC_RISING)   || \
                                           ((POLARITY) == HAL_DMAMUX_SYNC_FALLING)  || \
                                           ((POLARITY) == HAL_DMAMUX_SYNC_RISING_FALLING))

#define IS_DMAMUX_SYNC_STATE(SYNC) (((SYNC) == DISABLE)   || ((SYNC) == ENABLE))

#define IS_DMAMUX_SYNC_EVENT(EVENT) (((EVENT) == DISABLE)   || \
                                     ((EVENT) == ENABLE))

#define IS_DMAMUX_REQUEST_GEN_SIGNAL_ID(SIGNAL_ID) ((SIGNAL_ID) <= HAL_DMAMUX1_REQ_GEN_DMAMUX1_CH3_EVT)

#define IS_DMAMUX_REQUEST_GEN_REQUEST_NUMBER(REQUEST_NUMBER) (((REQUEST_NUMBER) > 0U) && ((REQUEST_NUMBER) <= 32U))

#define IS_DMAMUX_REQUEST_GEN_POLARITY(POLARITY) (((POLARITY) == HAL_DMAMUX_REQ_GEN_NO_EVENT)   || \
                                                  ((POLARITY) == HAL_DMAMUX_REQ_GEN_RISING)  || \
                                                  ((POLARITY) == HAL_DMAMUX_REQ_GEN_FALLING) || \
                                                  ((POLARITY) == HAL_DMAMUX_REQ_GEN_RISING_FALLING))

/**
  * end of DMAEx_Macro_Definitions @}
  */


/******************************************************************************/
/*                             DMAEx Functions                                */
/******************************************************************************/

/** @defgroup DMAEx_Function_Definitions DMAEx Function Definitions
  * @{
  */

/* ------------------------- REQUEST -----------------------------------------*/
HAL_StatusTypeDef HAL_DMAEx_ConfigMuxRequestGenerator(DMA_HandleTypeDef *hdma,
                                                      HAL_DMA_MuxRequestGeneratorConfigTypeDef *pRequestGeneratorConfig);
HAL_StatusTypeDef HAL_DMAEx_EnableMuxRequestGenerator(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef HAL_DMAEx_DisableMuxRequestGenerator(DMA_HandleTypeDef *hdma);
/* -------------------------------------------------------------------------- */

/* ------------------------- SYNCHRO -----------------------------------------*/
HAL_StatusTypeDef HAL_DMAEx_ConfigMuxSync(DMA_HandleTypeDef *hdma, HAL_DMA_MuxSyncConfigTypeDef *pSyncConfig);
/* -------------------------------------------------------------------------- */

void HAL_DMAEx_MUX_IRQHandler(DMA_HandleTypeDef *hdma);

/**
  * end of DMAEx_Function_Definitions @}
  */


/**
  * end of DMAEx @}
  */


#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_DMA_EX_H_ */
