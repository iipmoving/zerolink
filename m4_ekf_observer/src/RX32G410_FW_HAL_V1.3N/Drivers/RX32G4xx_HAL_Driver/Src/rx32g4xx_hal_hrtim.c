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
*   File    : rx32g4xx_hal_hrtim.c
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"

#ifdef HAL_HRTIM_MODULE_ENABLED

/** @defgroup HRTIM HRTIM
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/** @defgroup HRTIM_Private_Defines HRTIM Private Define
  * @{
  */
#define HRTIM_FLTR_FLTxEN            (HRTIM_FLTR_FLT1EN |\
                                      HRTIM_FLTR_FLT2EN |\
                                      HRTIM_FLTR_FLT3EN |\
                                      HRTIM_FLTR_FLT4EN | \
                                      HRTIM_FLTR_FLT5EN | \
                                      HRTIM_FLTR_FLT6EN)

#define HRTIM_TIMCR_TIMUPDATETRIGGER (HRTIM_TIMUPDATETRIGGER_MASTER  |\
                                      HRTIM_TIMUPDATETRIGGER_TIMER_A |\
                                      HRTIM_TIMUPDATETRIGGER_TIMER_B |\
                                      HRTIM_TIMUPDATETRIGGER_TIMER_C |\
                                      HRTIM_TIMUPDATETRIGGER_TIMER_D |\
                                      HRTIM_TIMUPDATETRIGGER_TIMER_E |\
                                      HRTIM_TIMUPDATETRIGGER_TIMER_F)

#define HRTIM_FLTINR1_FLTxLCK        ((HRTIM_FAULTLOCK_READONLY)        | \
                                      (HRTIM_FAULTLOCK_READONLY << 8U)  | \
                                      (HRTIM_FAULTLOCK_READONLY << 16U) | \
                                      (HRTIM_FAULTLOCK_READONLY << 24U))

#define HRTIM_FLTINR2_FLTxLCK        ((HRTIM_FAULTLOCK_READONLY)        | \
                                      (HRTIM_FAULTLOCK_READONLY << 8U))
/**
  * end of HRTIM_Private_Defines @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/** @defgroup HRTIM_Private_Variables HRTIM Private Variables
  * @{
  */
static uint32_t TimerIdxToTimerId[] =
{
  HRTIM_TIMERID_TIMER_A,
  HRTIM_TIMERID_TIMER_B,
  HRTIM_TIMERID_TIMER_C,
  HRTIM_TIMERID_TIMER_D,
  HRTIM_TIMERID_TIMER_E,
  HRTIM_TIMERID_TIMER_F,
  HRTIM_TIMERID_MASTER,
};

/**
  * end of HRTIM_Private_Variables @}
  */

/* Private function prototypes -----------------------------------------------*/
/** @defgroup HRTIM_Private_Functions HRTIM Private Functions
  * @{
  */
static void HRTIM_MasterBase_Config(HRTIM_HandleTypeDef * hhrtim,
                                    const HRTIM_TimeBaseCfgTypeDef * pTimeBaseCfg);

static void HRTIM_TimingUnitBase_Config(HRTIM_HandleTypeDef * hhrtim,
                                        uint32_t TimerIdx,
                                        const HRTIM_TimeBaseCfgTypeDef * pTimeBaseCfg);

static void HRTIM_MasterWaveform_Config(HRTIM_HandleTypeDef * hhrtim,
                                        const HRTIM_TimerCfgTypeDef * pTimerCfg);

static void HRTIM_TimingUnitWaveform_Config(HRTIM_HandleTypeDef * hhrtim,
                                            uint32_t TimerIdx,
                                            const HRTIM_TimerCfgTypeDef * pTimerCfg);

static void HRTIM_TimingUnitWaveform_Control(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t TimerIdx,
                                             const HRTIM_TimerCtlTypeDef * pTimerCtl);

static void  HRTIM_TimingUnitRollOver_Config(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t pRollOverMode);


static void HRTIM_CaptureUnitConfig(HRTIM_HandleTypeDef * hhrtim,
                                    uint32_t TimerIdx,
                                    uint32_t CaptureUnit,
                                    uint32_t Event);

static void HRTIM_OutputConfig(HRTIM_HandleTypeDef * hhrtim,
                                uint32_t TimerIdx,
                                uint32_t Output,
                                const HRTIM_OutputCfgTypeDef * pOutputCfg);

static void HRTIM_EventConfig(HRTIM_HandleTypeDef * hhrtim,
                              uint32_t Event,
                              const HRTIM_EventCfgTypeDef * pEventCfg);

static void HRTIM_TIM_ResetConfig(HRTIM_HandleTypeDef * hhrtim,
                                  uint32_t TimerIdx,
                                  uint32_t Event);

static uint32_t HRTIM_GetITFromOCMode(const HRTIM_HandleTypeDef * hhrtim,
                                      uint32_t TimerIdx,
                                      uint32_t OCChannel);

static uint32_t HRTIM_GetDMAFromOCMode(const HRTIM_HandleTypeDef * hhrtim,
                                       uint32_t TimerIdx,
                                       uint32_t OCChannel);

static DMA_HandleTypeDef * HRTIM_GetDMAHandleFromTimerIdx(const HRTIM_HandleTypeDef * hhrtim,
                                                          uint32_t TimerIdx);

static uint32_t GetTimerIdxFromDMAHandle(const HRTIM_HandleTypeDef * hhrtim,
                                         const DMA_HandleTypeDef * hdma);

static void HRTIM_ForceRegistersUpdate(HRTIM_HandleTypeDef * hhrtim,
                                      uint32_t TimerIdx);

static void HRTIM_HRTIM_ISR(HRTIM_HandleTypeDef * hhrtim);

static void HRTIM_Master_ISR(HRTIM_HandleTypeDef * hhrtim);

static void HRTIM_Timer_ISR(HRTIM_HandleTypeDef * hhrtim,
                                       uint32_t TimerIdx);

static void HRTIM_DMAMasterCplt(DMA_HandleTypeDef *hdma);

static void HRTIM_DMATimerxCplt(DMA_HandleTypeDef *hdma);

static void HRTIM_DMAError(DMA_HandleTypeDef *hdma);

static void HRTIM_BurstDMACplt(DMA_HandleTypeDef *hdma);

/**
  * end of HRTIM_Private_Functions @}
  */

/* Exported functions ---------------------------------------------------------*/

/** @addtogroup HRTIM_Function_Definitions HRTIM Function Definitions
  * @{
  */

/**
  * @brief  Initialize a HRTIM instance
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_Init(HRTIM_HandleTypeDef * hhrtim)
{
  uint8_t timer_idx;
  uint32_t hrtim_mcr;

  /* Check the HRTIM handle allocation */
  if(hhrtim == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_HRTIM_ALL_INSTANCE(hhrtim->Instance));
  assert_param(IS_HRTIM_IT(hhrtim->Init.HRTIMInterruptResquests));

#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
  if (hhrtim->State == HAL_HRTIM_STATE_RESET)
  {
    /* Initialize callback function pointers to their default values */
    hhrtim->Fault1Callback               = HAL_HRTIM_Fault1Callback;
    hhrtim->Fault2Callback               = HAL_HRTIM_Fault2Callback;
    hhrtim->Fault3Callback               = HAL_HRTIM_Fault3Callback;
    hhrtim->Fault4Callback               = HAL_HRTIM_Fault4Callback;
    hhrtim->Fault5Callback               = HAL_HRTIM_Fault5Callback;
    hhrtim->Fault6Callback               = HAL_HRTIM_Fault6Callback;
    hhrtim->SystemFaultCallback          = HAL_HRTIM_SystemFaultCallback;
    hhrtim->DLLCalibrationReadyCallback  = HAL_HRTIM_DLLCalibrationReadyCallback;
    hhrtim->BurstModePeriodCallback      = HAL_HRTIM_BurstModePeriodCallback;
    hhrtim->SynchronizationEventCallback = HAL_HRTIM_SynchronizationEventCallback;
    hhrtim->ErrorCallback                = HAL_HRTIM_ErrorCallback;
    hhrtim->RegistersUpdateCallback      = HAL_HRTIM_RegistersUpdateCallback;
    hhrtim->RepetitionEventCallback      = HAL_HRTIM_RepetitionEventCallback;
    hhrtim->Compare1EventCallback        = HAL_HRTIM_Compare1EventCallback;
    hhrtim->Compare2EventCallback        = HAL_HRTIM_Compare2EventCallback;
    hhrtim->Compare3EventCallback        = HAL_HRTIM_Compare3EventCallback;
    hhrtim->Compare4EventCallback        = HAL_HRTIM_Compare4EventCallback;
    hhrtim->Capture1EventCallback        = HAL_HRTIM_Capture1EventCallback;
    hhrtim->Capture2EventCallback        = HAL_HRTIM_Capture2EventCallback;
    hhrtim->DelayedProtectionCallback    = HAL_HRTIM_DelayedProtectionCallback;
    hhrtim->CounterResetCallback         = HAL_HRTIM_CounterResetCallback;
    hhrtim->Output1SetCallback           = HAL_HRTIM_Output1SetCallback;
    hhrtim->Output1ResetCallback         = HAL_HRTIM_Output1ResetCallback;
    hhrtim->Output2SetCallback           = HAL_HRTIM_Output2SetCallback;
    hhrtim->Output2ResetCallback         = HAL_HRTIM_Output2ResetCallback;
    hhrtim->BurstDMATransferCallback     = HAL_HRTIM_BurstDMATransferCallback;

    if (hhrtim->MspInitCallback == NULL)
    {
      hhrtim->MspInitCallback = HAL_HRTIM_MspInit;
    }
  }
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */

  /* Set the HRTIM state */
  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Initialize the DMA handles */
  hhrtim->hdmaMaster = (DMA_HandleTypeDef *)NULL;
  hhrtim->hdmaTimerA = (DMA_HandleTypeDef *)NULL;
  hhrtim->hdmaTimerB = (DMA_HandleTypeDef *)NULL;
  hhrtim->hdmaTimerC = (DMA_HandleTypeDef *)NULL;
  hhrtim->hdmaTimerD = (DMA_HandleTypeDef *)NULL;
  hhrtim->hdmaTimerE = (DMA_HandleTypeDef *)NULL;
  hhrtim->hdmaTimerF = (DMA_HandleTypeDef *)NULL;

  /* HRTIM output synchronization configuration (if required) */
  if ((hhrtim->Init.SyncOptions & HRTIM_SYNCOPTION_MASTER) != (uint32_t)RESET)
  {
    /* Check parameters */
    assert_param(IS_HRTIM_SYNCOUTPUTSOURCE(hhrtim->Init.SyncOutputSource));
    assert_param(IS_HRTIM_SYNCOUTPUTPOLARITY(hhrtim->Init.SyncOutputPolarity));

    /* The synchronization output initialization procedure must be done prior
       to the configuration of the MCU outputs (done within HAL_HRTIM_MspInit)
    */
    if (hhrtim->Instance == HRTIM1)
    {
      /* Enable the HRTIM peripheral clock */
      __HAL_RCC_HRTIM1_CLK_ENABLE();
    }

    hrtim_mcr = hhrtim->Instance->sMasterRegs.MCR;

    /* Set the event to be sent on the synchronization output */
    hrtim_mcr &= ~(HRTIM_MCR_SYNC_SRC);
    hrtim_mcr |= (hhrtim->Init.SyncOutputSource & HRTIM_MCR_SYNC_SRC);

    /* Set the polarity of the synchronization output */
    hrtim_mcr &= ~(HRTIM_MCR_SYNC_OUT);
    hrtim_mcr |= (hhrtim->Init.SyncOutputPolarity & HRTIM_MCR_SYNC_OUT);

    /* Update the HRTIM registers */
    hhrtim->Instance->sMasterRegs.MCR = hrtim_mcr;
  }

  /* Init the low level hardware : GPIO, CLOCK, NVIC and DMA */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
  hhrtim->MspInitCallback(hhrtim);
#else
  HAL_HRTIM_MspInit(hhrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */

  /* HRTIM input synchronization configuration (if required) */
  if ((hhrtim->Init.SyncOptions & HRTIM_SYNCOPTION_SLAVE) != (uint32_t)RESET)
  {
    /* Check parameters */
    assert_param(IS_HRTIM_SYNCINPUTSOURCE(hhrtim->Init.SyncInputSource));

    hrtim_mcr = hhrtim->Instance->sMasterRegs.MCR;

    /* Set the synchronization input source */
    hrtim_mcr &= ~(HRTIM_MCR_SYNC_IN);
    hrtim_mcr |= (hhrtim->Init.SyncInputSource & HRTIM_MCR_SYNC_IN);

    /* Update the HRTIM registers */
    hhrtim->Instance->sMasterRegs.MCR = hrtim_mcr;
  }

  /* Initialize the HRTIM state*/
  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Initialize the lock status of the HRTIM HAL API */
  __HAL_UNLOCK(hhrtim);

  /* Initialize timer related parameters */
  for (timer_idx = HRTIM_TIMERINDEX_TIMER_A ;
       timer_idx <= HRTIM_TIMERINDEX_MASTER ;
       timer_idx++)
  {
    hhrtim->TimerParam[timer_idx].CaptureTrigger1 = HRTIM_CAPTURETRIGGER_NONE;
    hhrtim->TimerParam[timer_idx].CaptureTrigger2 = HRTIM_CAPTURETRIGGER_NONE;
    hhrtim->TimerParam[timer_idx].InterruptRequests = HRTIM_IT_NONE;
    hhrtim->TimerParam[timer_idx].DMARequests = HRTIM_IT_NONE;
    hhrtim->TimerParam[timer_idx].DMASrcAddress = 0U;
    hhrtim->TimerParam[timer_idx].DMASize = 0U;
  }

  return HAL_OK;
}

/**
  * @brief  De-initialize a HRTIM instance
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_DeInit (HRTIM_HandleTypeDef * hhrtim)
{
  /* Check the HRTIM handle allocation */
  if(hhrtim == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_HRTIM_ALL_INSTANCE(hhrtim->Instance));

  /* Set the HRTIM state */
  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* DeInit the low level hardware */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
  if (hhrtim->MspDeInitCallback == NULL)
  {
    hhrtim->MspDeInitCallback = HAL_HRTIM_MspDeInit;
  }

  hhrtim->MspDeInitCallback(hhrtim);
#else
  HAL_HRTIM_MspDeInit(hhrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */

  hhrtim->State = HAL_HRTIM_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  MSP initialization for a HRTIM instance
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
__weak void HAL_HRTIM_MspInit(HRTIM_HandleTypeDef * hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_HRTIM_MspInit could be implemented in the user file
   */
}

/**
  * @brief  MSP de-initialization of a HRTIM instance
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
__weak void HAL_HRTIM_MspDeInit(HRTIM_HandleTypeDef * hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_HRTIM_MspDeInit could be implemented in the user file
   */
}

/**
  * @brief  Start the DLL calibration
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  CalibrationRate DLL calibration period
  *                    This parameter can be one of the following values:
  *                    @arg HRTIM_SINGLE_CALIBRATION: One shot DLL calibration
  *                    @arg HRTIM_CALIBRATIONRATE_0: Periodic DLL calibration. T=6.168 ms
  *                    @arg HRTIM_CALIBRATIONRATE_1: Periodic DLL calibration. T=0.771 ms
  *                    @arg HRTIM_CALIBRATIONRATE_2: Periodic DLL calibration. T=0.096 ms
  *                    @arg HRTIM_CALIBRATIONRATE_3: Periodic DLL calibration. T=0.012 ms
  * @retval HAL Status
  * @note This function locks the HRTIM instance. HRTIM instance is unlocked
  *       within the HAL_HRTIM_PollForDLLCalibration function, just before
  *       exiting the function.
  */
HAL_StatusTypeDef HAL_HRTIM_DLLCalibrationStart(HRTIM_HandleTypeDef * hhrtim,
                                                uint32_t CalibrationRate)
{
  __HAL_HRTIM_SETCLOCKPRESCALER(hhrtim, HRTIM_TIMERINDEX_MASTER, HRTIM_PRESCALERRATIO_DIV1);
  __HAL_HRTIM_SETCLOCKPRESCALER(hhrtim, HRTIM_TIMERINDEX_TIMER_A, HRTIM_PRESCALERRATIO_DIV1);
  __HAL_HRTIM_SETCLOCKPRESCALER(hhrtim, HRTIM_TIMERINDEX_TIMER_B, HRTIM_PRESCALERRATIO_DIV1);
  __HAL_HRTIM_SETCLOCKPRESCALER(hhrtim, HRTIM_TIMERINDEX_TIMER_C, HRTIM_PRESCALERRATIO_DIV1);
  __HAL_HRTIM_SETCLOCKPRESCALER(hhrtim, HRTIM_TIMERINDEX_TIMER_D, HRTIM_PRESCALERRATIO_DIV1);
  __HAL_HRTIM_SETCLOCKPRESCALER(hhrtim, HRTIM_TIMERINDEX_TIMER_E, HRTIM_PRESCALERRATIO_DIV1);
  __HAL_HRTIM_SETCLOCKPRESCALER(hhrtim, HRTIM_TIMERINDEX_TIMER_F, HRTIM_PRESCALERRATIO_DIV1);
  
  MODIFY_REG(hhrtim->Instance->sCommonRegs.OPT,  HRTIM_OPT_DLLBYPASS, HRTIM_OPT_DLLBYPASS);
  
  MODIFY_REG(hhrtim->Instance->sCommonRegs.DLLCR,  HRTIM_DLLCR_CKS, HRTIM_DLLCR_CKS);
  
  /* Check the parameters */
  assert_param(IS_HRTIM_CALIBRATIONRATE(CalibrationRate));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  if (CalibrationRate == HRTIM_SINGLE_CALIBRATION)
  {
    /* One shot DLL calibration */
    CLEAR_BIT(hhrtim->Instance->sCommonRegs.DLLCR, HRTIM_DLLCR_CALEN);
    SET_BIT(hhrtim->Instance->sCommonRegs.DLLCR, HRTIM_DLLCR_CAL);
  }
  else
  {
    /* Periodic DLL calibration */
    SET_BIT(hhrtim->Instance->sCommonRegs.DLLCR, HRTIM_DLLCR_CALEN);
    MODIFY_REG(hhrtim->Instance->sCommonRegs.DLLCR, HRTIM_DLLCR_CALRTE, CalibrationRate);
    SET_BIT(hhrtim->Instance->sCommonRegs.DLLCR, HRTIM_DLLCR_CAL);
  }

  /* Set HRTIM state */
  hhrtim->State = HAL_HRTIM_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Start the DLL calibration.
  *         DLL ready interrupt is enabled
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  CalibrationRate DLL calibration period
  *         This parameter can be one of the following values:
  *           @arg HRTIM_SINGLE_CALIBRATION: One shot DLL calibration
  *           @arg HRTIM_CALIBRATIONRATE_0: Periodic DLL calibration. T=6.168 ms
  *           @arg HRTIM_CALIBRATIONRATE_1: Periodic DLL calibration. T=0.771 ms
  *           @arg HRTIM_CALIBRATIONRATE_2: Periodic DLL calibration. T=0.096 ms
  *           @arg HRTIM_CALIBRATIONRATE_3: Periodic DLL calibration. T=0.012 ms
  * @retval HAL Status
  * @note This function locks the HRTIM instance. HRTIM instance is unlocked
  *       within the IRQ processing function when processing the DLL ready
  *       interrupt.
  * @note If this function is called for periodic calibration, the DLLRDY
  *       interrupt is generated every time the calibration completes which
  *       will significantly increases the overall interrupt rate.
  */
HAL_StatusTypeDef HAL_HRTIM_DLLCalibrationStart_IT(HRTIM_HandleTypeDef * hhrtim,
                                                   uint32_t CalibrationRate)
{
  /* Check the parameters */
  assert_param(IS_HRTIM_CALIBRATIONRATE(CalibrationRate));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable DLL Ready interrupt flag */
  __HAL_HRTIM_ENABLE_IT(hhrtim, HRTIM_IT_DLLRDY);

  if (CalibrationRate == HRTIM_SINGLE_CALIBRATION)
  {
    /* One shot DLL calibration */
    CLEAR_BIT(hhrtim->Instance->sCommonRegs.DLLCR, HRTIM_DLLCR_CALEN);
    SET_BIT(hhrtim->Instance->sCommonRegs.DLLCR, HRTIM_DLLCR_CAL);
  }
  else
  {
    /* Periodic DLL calibration */
    SET_BIT(hhrtim->Instance->sCommonRegs.DLLCR, HRTIM_DLLCR_CALEN);
    MODIFY_REG(hhrtim->Instance->sCommonRegs.DLLCR, HRTIM_DLLCR_CALRTE, CalibrationRate);
    SET_BIT(hhrtim->Instance->sCommonRegs.DLLCR, HRTIM_DLLCR_CAL);
  }

  /* Set HRTIM state */
  hhrtim->State = HAL_HRTIM_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Poll the DLL calibration ready flag and returns when the flag is
  *         set (DLL calibration completed) or upon timeout expiration.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Timeout Timeout duration in millisecond
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_PollForDLLCalibration(HRTIM_HandleTypeDef * hhrtim,
                                                  uint32_t Timeout)
{
  uint32_t tickstart;

  tickstart = HAL_GetTick();

  /* Check End of conversion flag */
  while(__HAL_HRTIM_GET_FLAG(hhrtim, HRTIM_IT_DLLRDY) == (uint32_t)RESET)
  {
    if (Timeout != HAL_MAX_DELAY)
    {
      if(((HAL_GetTick()-tickstart) > Timeout) || (Timeout == 0U))
      {
        hhrtim->State = HAL_HRTIM_STATE_ERROR;
        return HAL_TIMEOUT;
      }
    }
  }

  /* Set HRTIM State */
  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the time base unit of a timer
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_MASTER for master timer
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  pTimeBaseCfg pointer to the time base configuration structure
  * @note This function must be called prior starting the timer
  * @note   The time-base unit initialization parameters specify:
  *           The timer counter operating mode (continuous, one shot),
  *           The timer clock prescaler,
  *           The timer period,
  *           The timer repetition counter.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_TimeBaseConfig(HRTIM_HandleTypeDef *hhrtim,
                                           uint32_t TimerIdx,
                                           const HRTIM_TimeBaseCfgTypeDef * pTimeBaseCfg)
{
  /* Check the parameters */
  assert_param(IS_HRTIM_TIMERINDEX(TimerIdx));
  assert_param(IS_HRTIM_PRESCALERRATIO(pTimeBaseCfg->PrescalerRatio));
  assert_param(IS_HRTIM_MODE(pTimeBaseCfg->Mode));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Set the HRTIM state */
  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  if (TimerIdx == HRTIM_TIMERINDEX_MASTER)
  {
    /* Configure master timer time base unit */
    HRTIM_MasterBase_Config(hhrtim, pTimeBaseCfg);
  }
  else
  {
    /* Configure timing unit time base unit */
    HRTIM_TimingUnitBase_Config(hhrtim, TimerIdx, pTimeBaseCfg);
  }

  /* Set HRTIM state */
  hhrtim->State = HAL_HRTIM_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Start the counter of a timer operating in simple time base mode.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index.
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleBaseStart(HRTIM_HandleTypeDef * hhrtim,
                                           uint32_t TimerIdx)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMERINDEX(TimerIdx));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Stop the counter of a timer operating in simple time base mode.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index.
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleBaseStop(HRTIM_HandleTypeDef * hhrtim,
                                          uint32_t TimerIdx)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMERINDEX(TimerIdx));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Disable the timer counter */
  __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Start the counter of a timer operating in simple time base mode
  *         (Timer repetition interrupt is enabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index.
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleBaseStart_IT(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMERINDEX(TimerIdx));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable the repetition interrupt */
  if (TimerIdx == HRTIM_TIMERINDEX_MASTER)
  {
    __HAL_HRTIM_MASTER_ENABLE_IT(hhrtim, HRTIM_MASTER_IT_MREP);
  }
  else
  {
    __HAL_HRTIM_TIMER_ENABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_REP);
  }

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Stop the counter of a timer operating in simple time base mode
  *         (Timer repetition interrupt is disabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index.
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleBaseStop_IT(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t TimerIdx)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMERINDEX(TimerIdx));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Disable the repetition interrupt */
  if (TimerIdx == HRTIM_TIMERINDEX_MASTER)
  {
    __HAL_HRTIM_MASTER_DISABLE_IT(hhrtim, HRTIM_MASTER_IT_MREP);
  }
  else
  {
    __HAL_HRTIM_TIMER_DISABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_REP);
  }

  /* Disable the timer counter */
  __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Start the counter of a timer operating in simple time base mode
  *         (Timer repetition DMA request is enabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index.
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  SrcAddr DMA transfer source address
  * @param  DestAddr DMA transfer destination address
  * @param  Length The length of data items (data size) to be transferred
  *                     from source to destination
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleBaseStart_DMA(HRTIM_HandleTypeDef * hhrtim,
                                               uint32_t TimerIdx,
                                               uint32_t SrcAddr,
                                               uint32_t DestAddr,
                                               uint32_t Length)
{
  DMA_HandleTypeDef * hdma;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMERINDEX(TimerIdx));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }
  if(hhrtim->State == HAL_HRTIM_STATE_READY)
  {
    if((SrcAddr == 0U ) || (DestAddr == 0U ) || (Length == 0U))
    {
      return HAL_ERROR;
    }
    else
    {
      hhrtim->State = HAL_HRTIM_STATE_BUSY;
    }
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  /* Get the timer DMA handler */
  hdma = HRTIM_GetDMAHandleFromTimerIdx(hhrtim, TimerIdx);

  if (hdma == NULL)
  {
   hhrtim->State = HAL_HRTIM_STATE_ERROR;

   /* Process Unlocked */
   __HAL_UNLOCK(hhrtim);

   return HAL_ERROR;
  }

  /* Set the DMA transfer completed callback */
  if (TimerIdx == HRTIM_TIMERINDEX_MASTER)
  {
    hdma->XferCpltCallback = HRTIM_DMAMasterCplt;
  }
  else
  {
    hdma->XferCpltCallback = HRTIM_DMATimerxCplt;
  }

  /* Set the DMA error callback */
  hdma->XferErrorCallback = HRTIM_DMAError ;

  /* Enable the DMA channel */
  if (HAL_DMA_Start_IT(hdma, SrcAddr, DestAddr, Length) != HAL_OK)
    {
        hhrtim->State = HAL_HRTIM_STATE_ERROR;

        /* Process Unlocked */
        __HAL_UNLOCK(hhrtim);

        return HAL_ERROR;
    }

  /* Enable the timer repetition DMA request */
  if (TimerIdx == HRTIM_TIMERINDEX_MASTER)
  {
    __HAL_HRTIM_MASTER_ENABLE_DMA(hhrtim, HRTIM_MASTER_DMA_MREP);
  }
  else
  {
    __HAL_HRTIM_TIMER_ENABLE_DMA(hhrtim, TimerIdx, HRTIM_TIM_DMA_REP);
  }

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Stop the counter of a timer operating in simple time base mode
  *         (Timer repetition DMA request is disabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index.
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleBaseStop_DMA(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  DMA_HandleTypeDef * hdma;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMERINDEX(TimerIdx));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  if (TimerIdx == HRTIM_TIMERINDEX_MASTER)
  {
    hhrtim->State = HAL_HRTIM_STATE_READY;

    /* Disable the DMA */
    if (HAL_DMA_Abort(hhrtim->hdmaMaster) != HAL_OK)
    {
        hhrtim->State = HAL_HRTIM_STATE_ERROR;
    }
    /* Disable the timer repetition DMA request */
    __HAL_HRTIM_MASTER_DISABLE_DMA(hhrtim, HRTIM_MASTER_DMA_MREP);
  }
  else
  {
    /* Get the timer DMA handler */
    hdma = HRTIM_GetDMAHandleFromTimerIdx(hhrtim, TimerIdx);

    if (hdma == NULL)
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;
    }
    else
    {
      hhrtim->State = HAL_HRTIM_STATE_READY;

      /* Disable the DMA */
      if (HAL_DMA_Abort(hdma) != HAL_OK)
      {
         hhrtim->State = HAL_HRTIM_STATE_ERROR;
      }

      /* Disable the timer repetition DMA request */
      __HAL_HRTIM_TIMER_DISABLE_DMA(hhrtim, TimerIdx, HRTIM_TIM_DMA_REP);
     }
  }

  /* Disable the timer counter */
  __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  if (hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
      return HAL_ERROR;
  }
  else
  {
      return HAL_OK;
  }
}

/**
  * @brief  Configure an output in simple output compare mode
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  OCChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @param  pSimpleOCChannelCfg pointer to the simple output compare output configuration structure
  * @note When the timer operates in simple output compare mode:
  *         Output 1 is implicitly controlled by the compare unit 1
  *         Output 2 is implicitly controlled by the compare unit 2
  *       Output Set/Reset crossbar is set according to the selected output compare mode:
  *         Toggle: SETxyR = RSTxyR = CMPy
  *         Active: SETxyR = CMPy, RSTxyR = 0
  *         Inactive: SETxy =0, RSTxy = CMPy
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleOCChannelConfig(HRTIM_HandleTypeDef * hhrtim,
                                                 uint32_t TimerIdx,
                                                 uint32_t OCChannel,
                                                 const HRTIM_SimpleOCChannelCfgTypeDef* pSimpleOCChannelCfg)
{
  uint32_t CompareUnit = (uint32_t)RESET;
  HRTIM_OutputCfgTypeDef OutputCfg;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, OCChannel));
  assert_param(IS_HRTIM_BASICOCMODE(pSimpleOCChannelCfg->Mode));
  assert_param(IS_HRTIM_OUTPUTPULSE(pSimpleOCChannelCfg->Pulse));
  assert_param(IS_HRTIM_OUTPUTPOLARITY(pSimpleOCChannelCfg->Polarity));
  assert_param(IS_HRTIM_OUTPUTIDLELEVEL(pSimpleOCChannelCfg->IdleLevel));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  /* Set HRTIM state */
  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Configure timer compare unit */
  switch (OCChannel)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      CompareUnit = HRTIM_COMPAREUNIT_1;
      hhrtim->Instance->sTimerxRegs[TimerIdx].CMP1xR = pSimpleOCChannelCfg->Pulse;
      break;
    }
  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      CompareUnit = HRTIM_COMPAREUNIT_2;
      hhrtim->Instance->sTimerxRegs[TimerIdx].CMP2xR = pSimpleOCChannelCfg->Pulse;
      break;
    }
  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Configure timer output */
  OutputCfg.Polarity = (pSimpleOCChannelCfg->Polarity & HRTIM_OUTR_POL1);
  OutputCfg.IdleLevel = (pSimpleOCChannelCfg->IdleLevel & HRTIM_OUTR_IDLES1);
  OutputCfg.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_NONE;
  OutputCfg.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;
  OutputCfg.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
  OutputCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;

  switch (pSimpleOCChannelCfg->Mode)
  {
  case HRTIM_BASICOCMODE_TOGGLE:
    {
      if (CompareUnit == HRTIM_COMPAREUNIT_1)
      {
        OutputCfg.SetSource = HRTIM_OUTPUTSET_TIMCMP1;
      }
      else
      {
        OutputCfg.SetSource = HRTIM_OUTPUTSET_TIMCMP2;
      }
      OutputCfg.ResetSource = OutputCfg.SetSource;
      break;
    }

  case HRTIM_BASICOCMODE_ACTIVE:
    {
      if (CompareUnit == HRTIM_COMPAREUNIT_1)
      {
        OutputCfg.SetSource = HRTIM_OUTPUTSET_TIMCMP1;
      }
      else
      {
        OutputCfg.SetSource = HRTIM_OUTPUTSET_TIMCMP2;
      }
      OutputCfg.ResetSource = HRTIM_OUTPUTRESET_NONE;
      break;
    }

  case HRTIM_BASICOCMODE_INACTIVE:
    {
      if (CompareUnit == HRTIM_COMPAREUNIT_1)
      {
        OutputCfg.ResetSource = HRTIM_OUTPUTRESET_TIMCMP1;
      }
      else
      {
        OutputCfg.ResetSource = HRTIM_OUTPUTRESET_TIMCMP2;
      }
      OutputCfg.SetSource = HRTIM_OUTPUTSET_NONE;
      break;
    }

  default:
    {
      OutputCfg.SetSource = HRTIM_OUTPUTSET_NONE;
      OutputCfg.ResetSource = HRTIM_OUTPUTRESET_NONE;

      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  HRTIM_OutputConfig(hhrtim,
                     TimerIdx,
                     OCChannel,
                     &OutputCfg);

  /* Set HRTIM state */
  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Start the output compare signal generation on the designed timer output
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  OCChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleOCStart(HRTIM_HandleTypeDef * hhrtim,
                                         uint32_t TimerIdx,
                                         uint32_t OCChannel)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, OCChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable the timer output */
  hhrtim->Instance->sCommonRegs.OENR |= OCChannel;

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Stop the output compare signal generation on the designed timer output
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  OCChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleOCStop(HRTIM_HandleTypeDef * hhrtim,
                                        uint32_t TimerIdx,
                                        uint32_t OCChannel)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, OCChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Disable the timer output */
  hhrtim->Instance->sCommonRegs.ODISR |= OCChannel;

  /* Disable the timer counter */
  __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Start the output compare signal generation on the designed timer output
  *         (Interrupt is enabled (see note note below)).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  OCChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @note Interrupt enabling depends on the chosen output compare mode
  *          Output toggle: compare match interrupt is enabled
  *          Output set active:  output set interrupt is enabled
  *          Output set inactive:  output reset interrupt is enabled
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleOCStart_IT(HRTIM_HandleTypeDef * hhrtim,
                                            uint32_t TimerIdx,
                                            uint32_t OCChannel)
{
  uint32_t interrupt;

   /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, OCChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Get the interrupt to enable (depends on the output compare mode) */
  interrupt = HRTIM_GetITFromOCMode(hhrtim, TimerIdx, OCChannel);

  /* Enable the timer output */
  hhrtim->Instance->sCommonRegs.OENR |= OCChannel;

  /* Enable the timer interrupt (depends on the output compare mode) */
  __HAL_HRTIM_TIMER_ENABLE_IT(hhrtim, TimerIdx, interrupt);

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Stop the output compare signal generation on the designed timer output
  *         (Interrupt is disabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  OCChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleOCStop_IT(HRTIM_HandleTypeDef * hhrtim,
                                           uint32_t TimerIdx,
                                           uint32_t OCChannel)
{
  uint32_t interrupt;

   /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, OCChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Disable the timer output */
  hhrtim->Instance->sCommonRegs.ODISR |= OCChannel;

  /* Get the interrupt to disable (depends on the output compare mode) */
  interrupt = HRTIM_GetITFromOCMode(hhrtim, TimerIdx, OCChannel);

  /* Disable the timer interrupt */
  __HAL_HRTIM_TIMER_DISABLE_IT(hhrtim, TimerIdx, interrupt);

  /* Disable the timer counter */
  __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Start the output compare signal generation on the designed timer output
  *         (DMA request is enabled (see note below)).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  OCChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @param  SrcAddr DMA transfer source address
  * @param  DestAddr DMA transfer destination address
  * @param  Length The length of data items (data size) to be transferred
  *                     from source to destination
  * @note  DMA request enabling depends on the chosen output compare mode
  *          Output toggle: compare match DMA request is enabled
  *          Output set active:  output set DMA request is enabled
  *          Output set inactive:  output reset DMA request is enabled
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleOCStart_DMA(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t OCChannel,
                                             uint32_t SrcAddr,
                                             uint32_t DestAddr,
                                             uint32_t Length)
{
  DMA_HandleTypeDef * hdma;
  uint32_t dma_request;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, OCChannel));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }
  if(hhrtim->State == HAL_HRTIM_STATE_READY)
  {
    if((SrcAddr == 0U ) || (DestAddr == 0U ) || (Length == 0U))
    {
      return HAL_ERROR;
    }
    else
    {
      hhrtim->State = HAL_HRTIM_STATE_BUSY;
    }
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

   /* Enable the timer output */
  hhrtim->Instance->sCommonRegs.OENR |= OCChannel;

  /* Get the DMA request to enable */
  dma_request = HRTIM_GetDMAFromOCMode(hhrtim, TimerIdx, OCChannel);

  /* Get the timer DMA handler */
  hdma = HRTIM_GetDMAHandleFromTimerIdx(hhrtim, TimerIdx);

  if (hdma == NULL)
  {
   hhrtim->State = HAL_HRTIM_STATE_ERROR;

   /* Process Unlocked */
   __HAL_UNLOCK(hhrtim);

   return HAL_ERROR;
  }

  /* Set the DMA error callback */
  hdma->XferErrorCallback = HRTIM_DMAError ;

  /* Set the DMA transfer completed callback */
  hdma->XferCpltCallback = HRTIM_DMATimerxCplt;

  /* Enable the DMA channel */
  if (HAL_DMA_Start_IT(hdma, SrcAddr, DestAddr, Length) != HAL_OK)
    {
        hhrtim->State = HAL_HRTIM_STATE_ERROR;

        /* Process Unlocked */
        __HAL_UNLOCK(hhrtim);

        return HAL_ERROR;
    }

  /* Enable the timer DMA request */
  __HAL_HRTIM_TIMER_ENABLE_DMA(hhrtim, TimerIdx, dma_request);

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Stop the output compare signal generation on the designed timer output
  *         (DMA request is disabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  OCChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleOCStop_DMA(HRTIM_HandleTypeDef * hhrtim,
                                            uint32_t TimerIdx,
                                            uint32_t OCChannel)
{
  uint32_t dma_request;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, OCChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Disable the timer output */
  hhrtim->Instance->sCommonRegs.ODISR |= OCChannel;

  /* Get the timer DMA handler */
  /* Disable the DMA */
  if (HAL_DMA_Abort(HRTIM_GetDMAHandleFromTimerIdx(hhrtim, TimerIdx)) != HAL_OK)
  {
    hhrtim->State = HAL_HRTIM_STATE_ERROR;

    /* Process Unlocked */
    __HAL_UNLOCK(hhrtim);

    return HAL_ERROR;
  }

  /* Get the DMA request to disable */
  dma_request = HRTIM_GetDMAFromOCMode(hhrtim, TimerIdx, OCChannel);

  /* Disable the timer DMA request */
  __HAL_HRTIM_TIMER_DISABLE_DMA(hhrtim, TimerIdx, dma_request);

  /* Disable the timer counter */
  __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure an output in simple PWM mode
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  PWMChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @param  pSimplePWMChannelCfg pointer to the simple PWM output configuration structure
  * @note When the timer operates in simple PWM output mode:
  *         Output 1 is implicitly controlled by the compare unit 1
  *         Output 2 is implicitly controlled by the compare unit 2
  *       Output Set/Reset crossbar is set as follows:
  *         Output 1: SETx1R = CMP1, RSTx1R = PER
  *         Output 2: SETx2R = CMP2, RST2R = PER
  * @note When Simple PWM mode is used the registers preload mechanism is
  *       enabled (otherwise the behavior is not guaranteed).
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimplePWMChannelConfig(HRTIM_HandleTypeDef * hhrtim,
                                                  uint32_t TimerIdx,
                                                  uint32_t PWMChannel,
                                                  const HRTIM_SimplePWMChannelCfgTypeDef* pSimplePWMChannelCfg)
{
  HRTIM_OutputCfgTypeDef OutputCfg;
  uint32_t hrtim_timcr;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, PWMChannel));
  assert_param(IS_HRTIM_OUTPUTPOLARITY(pSimplePWMChannelCfg->Polarity));
  assert_param(IS_HRTIM_OUTPUTPULSE(pSimplePWMChannelCfg->Pulse));
  assert_param(IS_HRTIM_OUTPUTIDLELEVEL(pSimplePWMChannelCfg->IdleLevel));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Configure timer compare unit */
  switch (PWMChannel)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].CMP1xR = pSimplePWMChannelCfg->Pulse;
      OutputCfg.SetSource = HRTIM_OUTPUTSET_TIMCMP1;
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].CMP2xR = pSimplePWMChannelCfg->Pulse;
      OutputCfg.SetSource = HRTIM_OUTPUTSET_TIMCMP2;
      break;
    }
  default:
    {
      OutputCfg.SetSource = HRTIM_OUTPUTSET_NONE;
      OutputCfg.ResetSource = HRTIM_OUTPUTRESET_NONE;

      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Configure timer output */
  OutputCfg.Polarity = (pSimplePWMChannelCfg->Polarity & HRTIM_OUTR_POL1);
  OutputCfg.IdleLevel = (pSimplePWMChannelCfg->IdleLevel& HRTIM_OUTR_IDLES1);
  OutputCfg.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_NONE;
  OutputCfg.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;
  OutputCfg.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
  OutputCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;
  OutputCfg.ResetSource = HRTIM_OUTPUTRESET_TIMPER;

  HRTIM_OutputConfig(hhrtim,
                     TimerIdx,
                     PWMChannel,
                     &OutputCfg);

  /* Enable the registers preload mechanism */
  hrtim_timcr = hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR;
  hrtim_timcr |= HRTIM_TIMCR_PREEN;
  hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR = hrtim_timcr;

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Start the PWM output signal generation on the designed timer output
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  PWMChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimplePWMStart(HRTIM_HandleTypeDef * hhrtim,
                                          uint32_t TimerIdx,
                                          uint32_t PWMChannel)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, PWMChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable the timer output */
  hhrtim->Instance->sCommonRegs.OENR |= PWMChannel;

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Stop the PWM output signal generation on the designed timer output
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  PWMChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimplePWMStop(HRTIM_HandleTypeDef * hhrtim,
                                         uint32_t TimerIdx,
                                         uint32_t PWMChannel)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, PWMChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Disable the timer output */
  hhrtim->Instance->sCommonRegs.ODISR |= PWMChannel;

  /* Disable the timer counter */
  __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Start the PWM output signal generation on the designed timer output
  *         (The compare interrupt is enabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  PWMChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimplePWMStart_IT(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t PWMChannel)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, PWMChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable the timer output */
  hhrtim->Instance->sCommonRegs.OENR |= PWMChannel;

  /* Enable the timer interrupt (depends on the PWM output) */
  switch (PWMChannel)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      __HAL_HRTIM_TIMER_ENABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CMP1);
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      __HAL_HRTIM_TIMER_ENABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CMP2);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Stop the PWM output signal generation on the designed timer output
  *         (The compare interrupt is disabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  PWMChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimplePWMStop_IT(HRTIM_HandleTypeDef * hhrtim,
                                            uint32_t TimerIdx,
                                            uint32_t PWMChannel)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, PWMChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Disable the timer output */
  hhrtim->Instance->sCommonRegs.ODISR |= PWMChannel;

  /* Disable the timer interrupt (depends on the PWM output) */
  switch (PWMChannel)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      __HAL_HRTIM_TIMER_DISABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CMP1);
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      __HAL_HRTIM_TIMER_DISABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CMP2);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Disable the timer counter */
  __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Start the PWM output signal generation on the designed timer output
  *         (The compare DMA request is enabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  PWMChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @param  SrcAddr DMA transfer source address
  * @param  DestAddr DMA transfer destination address
  * @param  Length The length of data items (data size) to be transferred
  *                     from source to destination
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimplePWMStart_DMA(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx,
                                              uint32_t PWMChannel,
                                              uint32_t SrcAddr,
                                              uint32_t DestAddr,
                                              uint32_t Length)
{
  DMA_HandleTypeDef * hdma;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, PWMChannel));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }
  if(hhrtim->State == HAL_HRTIM_STATE_READY)
  {
    if((SrcAddr == 0U ) || (DestAddr == 0U ) || (Length == 0U))
    {
      return HAL_ERROR;
    }
    else
    {
      hhrtim->State = HAL_HRTIM_STATE_BUSY;
    }
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  /* Enable the timer output */
  hhrtim->Instance->sCommonRegs.OENR |= PWMChannel;

  /* Get the timer DMA handler */
  hdma = HRTIM_GetDMAHandleFromTimerIdx(hhrtim, TimerIdx);

  if (hdma == NULL)
  {
    hhrtim->State = HAL_HRTIM_STATE_ERROR;

    /* Process Unlocked */
    __HAL_UNLOCK(hhrtim);

    return HAL_ERROR;
  }

  /* Set the DMA error callback */
  hdma->XferErrorCallback = HRTIM_DMAError ;

  /* Set the DMA transfer completed callback */
  hdma->XferCpltCallback = HRTIM_DMATimerxCplt;

  /* Enable the DMA channel */
  if (HAL_DMA_Start_IT(hdma, SrcAddr, DestAddr, Length) != HAL_OK)
    {
        hhrtim->State = HAL_HRTIM_STATE_ERROR;

        /* Process Unlocked */
        __HAL_UNLOCK(hhrtim);

        return HAL_ERROR;
    }

  /* Enable the timer DMA request */
  switch (PWMChannel)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      __HAL_HRTIM_TIMER_ENABLE_DMA(hhrtim, TimerIdx, HRTIM_TIM_DMA_CMP1);
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      __HAL_HRTIM_TIMER_ENABLE_DMA(hhrtim, TimerIdx, HRTIM_TIM_DMA_CMP2);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Stop the PWM output signal generation on the designed timer output
  *         (The compare DMA request is disabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  PWMChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimplePWMStop_DMA(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t PWMChannel)
{
  /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, PWMChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Disable the timer output */
  hhrtim->Instance->sCommonRegs.ODISR |= PWMChannel;

  /* Get the timer DMA handler */
  /* Disable the DMA */
  if (HAL_DMA_Abort(HRTIM_GetDMAHandleFromTimerIdx(hhrtim, TimerIdx)) != HAL_OK)
  {
    hhrtim->State = HAL_HRTIM_STATE_ERROR;

    /* Process Unlocked */
    __HAL_UNLOCK(hhrtim);

    return HAL_ERROR;
  }

  /* Disable the timer DMA request */
  switch (PWMChannel)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      __HAL_HRTIM_TIMER_DISABLE_DMA(hhrtim, TimerIdx, HRTIM_TIM_DMA_CMP1);
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      __HAL_HRTIM_TIMER_DISABLE_DMA(hhrtim, TimerIdx, HRTIM_TIM_DMA_CMP2);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Disable the timer counter */
  __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure a simple capture
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CaptureChannel Capture unit
  *         This parameter can be one of the following values:
  *           @arg HRTIM_CAPTUREUNIT_1: Capture unit 1
  *           @arg HRTIM_CAPTUREUNIT_2: Capture unit 2
  * @param  pSimpleCaptureChannelCfg pointer to the simple capture configuration structure
  * @note When the timer operates in simple capture mode the capture is triggered
  *       by the designated external event and GPIO input is implicitly used as event source.
  *       The cature can be triggered by a rising edge, a falling edge or both
  *       edges on event channel.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureChannelConfig(HRTIM_HandleTypeDef * hhrtim,
                                                      uint32_t TimerIdx,
                                                      uint32_t CaptureChannel,
                                                      const HRTIM_SimpleCaptureChannelCfgTypeDef* pSimpleCaptureChannelCfg)
{
  HRTIM_EventCfgTypeDef EventCfg;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_CAPTUREUNIT(CaptureChannel));
  assert_param(IS_HRTIM_EVENT(pSimpleCaptureChannelCfg->Event));
  assert_param(IS_HRTIM_EVENTPOLARITY(pSimpleCaptureChannelCfg->EventSensitivity,
                                      pSimpleCaptureChannelCfg->EventPolarity));
  assert_param(IS_HRTIM_EVENTSENSITIVITY(pSimpleCaptureChannelCfg->EventSensitivity));
  assert_param(IS_HRTIM_EVENTFILTER(pSimpleCaptureChannelCfg->Event,
                                    pSimpleCaptureChannelCfg->EventFilter));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Configure external event channel */
  EventCfg.FastMode = HRTIM_EVENTFASTMODE_DISABLE;
  EventCfg.Filter = (pSimpleCaptureChannelCfg->EventFilter & HRTIM_EECR3_EE6F);
  EventCfg.Polarity = (pSimpleCaptureChannelCfg->EventPolarity & HRTIM_EECR1_EE1POL);
  EventCfg.Sensitivity = (pSimpleCaptureChannelCfg->EventSensitivity & HRTIM_EECR1_EE1SNS);
  EventCfg.Source = HRTIM_EEV1SRC_GPIO; /* source 1 for External Event */

  HRTIM_EventConfig(hhrtim,
                    pSimpleCaptureChannelCfg->Event,
                    &EventCfg);

  /* Memorize capture trigger (will be configured when the capture is started */
  HRTIM_CaptureUnitConfig(hhrtim,
                          TimerIdx,
                          CaptureChannel,
                          pSimpleCaptureChannelCfg->Event);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Enable a simple capture on the designed capture unit
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CaptureChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_CAPTUREUNIT_1: Capture unit 1
  *           @arg HRTIM_CAPTUREUNIT_2: Capture unit 2
  * @retval HAL Status
  * @note  The external event triggering the capture is available for all timing
  *        units. It can be used directly and is active as soon as the timing
  *        unit counter is enabled.
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureStart(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx,
                                              uint32_t CaptureChannel)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_CAPTUREUNIT(CaptureChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Set the capture unit trigger */
  switch (CaptureChannel)
  {
  case HRTIM_CAPTUREUNIT_1:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xCR = hhrtim->TimerParam[TimerIdx].CaptureTrigger1;
      break;
    }

  case HRTIM_CAPTUREUNIT_2:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xCR = hhrtim->TimerParam[TimerIdx].CaptureTrigger2;
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Disable a simple capture on the designed capture unit
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CaptureChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_CAPTUREUNIT_1: Capture unit 1
  *           @arg HRTIM_CAPTUREUNIT_2: Capture unit 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureStop(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t CaptureChannel)
{
  uint32_t hrtim_cpt1cr;
  uint32_t hrtim_cpt2cr;

   /* Check the parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_CAPTUREUNIT(CaptureChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Set the capture unit trigger */
  switch (CaptureChannel)
  {
  case HRTIM_CAPTUREUNIT_1:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xCR = HRTIM_CAPTURETRIGGER_NONE;
      break;
    }

  case HRTIM_CAPTUREUNIT_2:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xCR = HRTIM_CAPTURETRIGGER_NONE;
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  hrtim_cpt1cr = hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xCR;
  hrtim_cpt2cr = hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xCR;

  /* Disable the timer counter */
  if ((hrtim_cpt1cr == HRTIM_CAPTURETRIGGER_NONE) &&
      (hrtim_cpt2cr == HRTIM_CAPTURETRIGGER_NONE))
  {
    __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Enable a simple capture on the designed capture unit
  *         (Capture interrupt is enabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CaptureChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_CAPTUREUNIT_1: Capture unit 1
  *           @arg HRTIM_CAPTUREUNIT_2: Capture unit 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureStart_IT(HRTIM_HandleTypeDef * hhrtim,
                                                 uint32_t TimerIdx,
                                                 uint32_t CaptureChannel)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_CAPTUREUNIT(CaptureChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Set the capture unit trigger */
  switch (CaptureChannel)
  {
  case HRTIM_CAPTUREUNIT_1:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xCR = hhrtim->TimerParam[TimerIdx].CaptureTrigger1;

      /* Enable the capture unit 1 interrupt */
      __HAL_HRTIM_TIMER_ENABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CPT1);
      break;
    }

  case HRTIM_CAPTUREUNIT_2:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xCR = hhrtim->TimerParam[TimerIdx].CaptureTrigger2;

      /* Enable the capture unit 2 interrupt */
      __HAL_HRTIM_TIMER_ENABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CPT2);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Disable a simple capture on the designed capture unit
  *         (Capture interrupt is disabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CaptureChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_CAPTUREUNIT_1: Capture unit 1
  *           @arg HRTIM_CAPTUREUNIT_2: Capture unit 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureStop_IT(HRTIM_HandleTypeDef * hhrtim,
                                                uint32_t TimerIdx,
                                                uint32_t CaptureChannel)
{

  uint32_t hrtim_cpt1cr;
  uint32_t hrtim_cpt2cr;

   /* Check the parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_CAPTUREUNIT(CaptureChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Set the capture unit trigger */
  switch (CaptureChannel)
  {
  case HRTIM_CAPTUREUNIT_1:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xCR = HRTIM_CAPTURETRIGGER_NONE;

      /* Disable the capture unit 1 interrupt */
      __HAL_HRTIM_TIMER_DISABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CPT1);
      break;
    }

  case HRTIM_CAPTUREUNIT_2:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xCR = HRTIM_CAPTURETRIGGER_NONE;

      /* Disable the capture unit 2 interrupt */
      __HAL_HRTIM_TIMER_DISABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CPT2);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  hrtim_cpt1cr = hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xCR;
  hrtim_cpt2cr = hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xCR;

  /* Disable the timer counter */
  if ((hrtim_cpt1cr == HRTIM_CAPTURETRIGGER_NONE) &&
      (hrtim_cpt2cr == HRTIM_CAPTURETRIGGER_NONE))
  {
    __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Enable a simple capture on the designed capture unit
  *         (Capture DMA request is enabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CaptureChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_CAPTUREUNIT_1: Capture unit 1
  *           @arg HRTIM_CAPTUREUNIT_2: Capture unit 2
  * @param  SrcAddr DMA transfer source address
  * @param  DestAddr DMA transfer destination address
  * @param  Length The length of data items (data size) to be transferred
  *                     from source to destination
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureStart_DMA(HRTIM_HandleTypeDef * hhrtim,
                                                  uint32_t TimerIdx,
                                                  uint32_t CaptureChannel,
                                                  uint32_t SrcAddr,
                                                  uint32_t DestAddr,
                                                  uint32_t Length)
{
  DMA_HandleTypeDef * hdma;

   /* Check the parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_CAPTUREUNIT(CaptureChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Get the timer DMA handler */
  hdma = HRTIM_GetDMAHandleFromTimerIdx(hhrtim, TimerIdx);

  if (hdma == NULL)
  {
   hhrtim->State = HAL_HRTIM_STATE_ERROR;

   /* Process Unlocked */
   __HAL_UNLOCK(hhrtim);

   return HAL_ERROR;
  }

  /* Set the DMA error callback */
  hdma->XferErrorCallback = HRTIM_DMAError ;

  /* Set the DMA transfer completed callback */
  hdma->XferCpltCallback = HRTIM_DMATimerxCplt;

  /* Enable the DMA channel */
  if (HAL_DMA_Start_IT(hdma, SrcAddr, DestAddr, Length) != HAL_OK)
    {
        hhrtim->State = HAL_HRTIM_STATE_ERROR;

        /* Process Unlocked */
        __HAL_UNLOCK(hhrtim);

        return HAL_ERROR;
    }

  switch (CaptureChannel)
  {
  case HRTIM_CAPTUREUNIT_1:
    {
      /* Set the capture unit trigger */
      hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xCR = hhrtim->TimerParam[TimerIdx].CaptureTrigger1;

      __HAL_HRTIM_TIMER_ENABLE_DMA(hhrtim, TimerIdx, HRTIM_TIM_DMA_CPT1);
      break;
    }

  case HRTIM_CAPTUREUNIT_2:
    {
      /* Set the capture unit trigger */
      hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xCR = hhrtim->TimerParam[TimerIdx].CaptureTrigger2;

      /* Enable the timer DMA request */
      __HAL_HRTIM_TIMER_ENABLE_DMA(hhrtim, TimerIdx, HRTIM_TIM_DMA_CPT2);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
 }

 if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Disable a simple capture on the designed capture unit
  *         (Capture DMA request is disabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CaptureChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_CAPTUREUNIT_1: Capture unit 1
  *           @arg HRTIM_CAPTUREUNIT_2: Capture unit 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleCaptureStop_DMA(HRTIM_HandleTypeDef * hhrtim,
                                                 uint32_t TimerIdx,
                                                 uint32_t CaptureChannel)
{

  uint32_t hrtim_cpt1cr;
  uint32_t hrtim_cpt2cr;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_CAPTUREUNIT(CaptureChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Get the timer DMA handler */
  /* Disable the DMA */
  if (HAL_DMA_Abort(HRTIM_GetDMAHandleFromTimerIdx(hhrtim, TimerIdx)) != HAL_OK)
  {
        hhrtim->State = HAL_HRTIM_STATE_ERROR;

        /* Process Unlocked */
        __HAL_UNLOCK(hhrtim);

        return HAL_ERROR;
  }

  switch (CaptureChannel)
  {
  case HRTIM_CAPTUREUNIT_1:
    {
      /* Reset the capture unit trigger */
      hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xCR = HRTIM_CAPTURETRIGGER_NONE;

      /* Disable the capture unit 1 DMA request */
      __HAL_HRTIM_TIMER_DISABLE_DMA(hhrtim, TimerIdx, HRTIM_TIM_DMA_CPT1);
      break;
    }

  case HRTIM_CAPTUREUNIT_2:
    {
      /* Reset the capture unit trigger */
      hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xCR = HRTIM_CAPTURETRIGGER_NONE;

      /* Disable the capture unit 2 DMA request */
      __HAL_HRTIM_TIMER_DISABLE_DMA(hhrtim, TimerIdx, HRTIM_TIM_DMA_CPT2);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  hrtim_cpt1cr = hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xCR;
  hrtim_cpt2cr = hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xCR;

  /* Disable the timer counter */
  if ((hrtim_cpt1cr == HRTIM_CAPTURETRIGGER_NONE) &&
      (hrtim_cpt2cr == HRTIM_CAPTURETRIGGER_NONE))
  {
    __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure an output simple one pulse mode
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  OnePulseChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @param  pSimpleOnePulseChannelCfg pointer to the simple one pulse output configuration structure
  * @note When the timer operates in simple one pulse mode:
  *         the timer counter is implicitly started by the reset event,
  *         the reset of the timer counter is triggered by the designated external event
  *         GPIO input is implicitly used as event source,
  *         Output 1 is implicitly controlled by the compare unit 1,
  *         Output 2 is implicitly controlled by the compare unit 2.
  *       Output Set/Reset crossbar is set as follows:
  *         Output 1: SETx1R = CMP1, RSTx1R = PER
  *         Output 2: SETx2R = CMP2, RST2R = PER
  * @retval HAL Status
  * @note If HAL_HRTIM_SimpleOnePulseChannelConfig is called for both timer
  *       outputs, the reset event related configuration data provided in the
  *       second call will override the reset event related configuration data
  *       provided in the first call.
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleOnePulseChannelConfig(HRTIM_HandleTypeDef * hhrtim,
                                                       uint32_t TimerIdx,
                                                       uint32_t OnePulseChannel,
                                                       const HRTIM_SimpleOnePulseChannelCfgTypeDef* pSimpleOnePulseChannelCfg)
{
  HRTIM_OutputCfgTypeDef OutputCfg;
  HRTIM_EventCfgTypeDef EventCfg;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, OnePulseChannel));
  assert_param(IS_HRTIM_OUTPUTPULSE(pSimpleOnePulseChannelCfg->Pulse));
  assert_param(IS_HRTIM_OUTPUTPOLARITY(pSimpleOnePulseChannelCfg->OutputPolarity));
  assert_param(IS_HRTIM_OUTPUTIDLELEVEL(pSimpleOnePulseChannelCfg->OutputIdleLevel));
  assert_param(IS_HRTIM_EVENT(pSimpleOnePulseChannelCfg->Event));
  assert_param(IS_HRTIM_EVENTPOLARITY(pSimpleOnePulseChannelCfg->EventSensitivity,
                                      pSimpleOnePulseChannelCfg->EventPolarity));
  assert_param(IS_HRTIM_EVENTSENSITIVITY(pSimpleOnePulseChannelCfg->EventSensitivity));
  assert_param(IS_HRTIM_EVENTFILTER(pSimpleOnePulseChannelCfg->Event,
                                    pSimpleOnePulseChannelCfg->EventFilter));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Configure timer compare unit */
  switch (OnePulseChannel)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].CMP1xR = pSimpleOnePulseChannelCfg->Pulse;
      OutputCfg.SetSource = HRTIM_OUTPUTSET_TIMCMP1;
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].CMP2xR = pSimpleOnePulseChannelCfg->Pulse;
      OutputCfg.SetSource = HRTIM_OUTPUTSET_TIMCMP2;
      break;
    }

  default:
    {
      OutputCfg.SetSource = HRTIM_OUTPUTSET_NONE;
      OutputCfg.ResetSource = HRTIM_OUTPUTRESET_NONE;

      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Configure timer output */
  OutputCfg.Polarity =  (pSimpleOnePulseChannelCfg->OutputPolarity & HRTIM_OUTR_POL1);
  OutputCfg.IdleLevel = (pSimpleOnePulseChannelCfg->OutputIdleLevel & HRTIM_OUTR_IDLES1);
  OutputCfg.FaultLevel = HRTIM_OUTPUTFAULTLEVEL_NONE;
  OutputCfg.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;
  OutputCfg.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
  OutputCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;
  OutputCfg.ResetSource = HRTIM_OUTPUTRESET_TIMPER;

  HRTIM_OutputConfig(hhrtim,
                     TimerIdx,
                     OnePulseChannel,
                     &OutputCfg);

  /* Configure external event channel */
  EventCfg.FastMode = HRTIM_EVENTFASTMODE_DISABLE;
  EventCfg.Filter = (pSimpleOnePulseChannelCfg->EventFilter & HRTIM_EECR3_EE6F);
  EventCfg.Polarity = (pSimpleOnePulseChannelCfg->EventPolarity & HRTIM_OUTR_POL1);
  EventCfg.Sensitivity = (pSimpleOnePulseChannelCfg->EventSensitivity &HRTIM_EECR1_EE1SNS);
  EventCfg.Source = HRTIM_EEV1SRC_GPIO; /* source 1 for External Event */

  HRTIM_EventConfig(hhrtim,
                    pSimpleOnePulseChannelCfg->Event,
                    &EventCfg);

  /* Configure the timer reset register */
  HRTIM_TIM_ResetConfig(hhrtim,
                        TimerIdx,
                        pSimpleOnePulseChannelCfg->Event);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Enable the simple one pulse signal generation on the designed output
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  OnePulseChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleOnePulseStart(HRTIM_HandleTypeDef * hhrtim,
                                                uint32_t TimerIdx,
                                                uint32_t OnePulseChannel)
{
  /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, OnePulseChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable the timer output */
  hhrtim->Instance->sCommonRegs.OENR |= OnePulseChannel;

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Disable the simple one pulse signal generation on the designed output
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  OnePulseChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleOnePulseStop(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx,
                                              uint32_t OnePulseChannel)
{
  /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, OnePulseChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Disable the timer output */
  hhrtim->Instance->sCommonRegs.ODISR |= OnePulseChannel;

  /* Disable the timer counter */
  __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Enable the simple one pulse signal generation on the designed output
  *         (The compare interrupt is enabled (pulse start)).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  OnePulseChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleOnePulseStart_IT(HRTIM_HandleTypeDef * hhrtim,
                                                  uint32_t TimerIdx,
                                                  uint32_t OnePulseChannel)
{
  /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, OnePulseChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable the timer output */
  hhrtim->Instance->sCommonRegs.OENR |= OnePulseChannel;

  /* Enable the timer interrupt (depends on the OnePulse output) */
  switch (OnePulseChannel)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      __HAL_HRTIM_TIMER_ENABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CMP1);
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      __HAL_HRTIM_TIMER_ENABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CMP2);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Disable the simple one pulse signal generation on the designed output
  *         (The compare interrupt is disabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  OnePulseChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_SimpleOnePulseStop_IT(HRTIM_HandleTypeDef * hhrtim,
                                                 uint32_t TimerIdx,
                                                 uint32_t OnePulseChannel)
{
  /* Check the parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, OnePulseChannel));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Disable the timer output */
  hhrtim->Instance->sCommonRegs.ODISR |= OnePulseChannel;

  /* Disable the timer interrupt (depends on the OnePulse output) */
  switch (OnePulseChannel)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      __HAL_HRTIM_TIMER_DISABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CMP1);
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      __HAL_HRTIM_TIMER_DISABLE_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CMP2);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Disable the timer counter */
  __HAL_HRTIM_DISABLE(hhrtim, TimerIdxToTimerId[TimerIdx]);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the burst mode feature of the HRTIM
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  pBurstModeCfg pointer to the burst mode configuration structure
  * @retval HAL Status
  * @note This function must be called before starting the burst mode
  *       controller
  */
HAL_StatusTypeDef HAL_HRTIM_BurstModeConfig(HRTIM_HandleTypeDef * hhrtim,
                                            const HRTIM_BurstModeCfgTypeDef* pBurstModeCfg)
{
  uint32_t hrtim_bmcr;

  /* Check parameters */
  assert_param(IS_HRTIM_BURSTMODE(pBurstModeCfg->Mode));
  assert_param(IS_HRTIM_BURSTMODECLOCKSOURCE(pBurstModeCfg->ClockSource));
  assert_param(IS_HRTIM_HRTIM_BURSTMODEPRESCALER(pBurstModeCfg->Prescaler));
  assert_param(IS_HRTIM_BURSTMODEPRELOAD(pBurstModeCfg->PreloadEnable));
  assert_param(IS_HRTIM_BURSTMODETRIGGER(pBurstModeCfg->Trigger));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  hrtim_bmcr = hhrtim->Instance->sCommonRegs.BMCR;

  /* Set the burst mode operating mode */
  hrtim_bmcr &= ~(HRTIM_BMCR_BMOM);
  hrtim_bmcr |= (pBurstModeCfg->Mode & HRTIM_BMCR_BMOM);

  /* Set the burst mode clock source */
  hrtim_bmcr &= ~(HRTIM_BMCR_BMCLK);
  hrtim_bmcr |= (pBurstModeCfg->ClockSource & HRTIM_BMCR_BMCLK);

  /* Set the burst mode prescaler */
  hrtim_bmcr &= ~(HRTIM_BMCR_BMPRSC);
  hrtim_bmcr |= pBurstModeCfg->Prescaler;

  /* Enable/disable burst mode registers preload */
  hrtim_bmcr &= ~(HRTIM_BMCR_BMPREN);
  hrtim_bmcr |= (pBurstModeCfg->PreloadEnable & HRTIM_BMCR_BMPREN);

  /* Set the burst mode trigger */
  hhrtim->Instance->sCommonRegs.BMTRGR = pBurstModeCfg->Trigger;

  /* Set the burst mode compare value */
  hhrtim->Instance->sCommonRegs.BMCMPR = pBurstModeCfg->IdleDuration;

  /* Set the burst mode period */
  hhrtim->Instance->sCommonRegs.BMPER = pBurstModeCfg->Period;

  /* Update the HRTIM registers */
  hhrtim->Instance->sCommonRegs.BMCR = hrtim_bmcr;

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the conditioning of an external event
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Event external event to configure
  *         This parameter can be one of the following values:
  *           @arg HRTIM_EVENT_NONE: no external Event
  *           @arg HRTIM_EVENT_1: External event 1
  *           @arg HRTIM_EVENT_2: External event 2
  *           @arg HRTIM_EVENT_3: External event 3
  *           @arg HRTIM_EVENT_4: External event 4
  *           @arg HRTIM_EVENT_5: External event 5
  *           @arg HRTIM_EVENT_6: External event 6
  *           @arg HRTIM_EVENT_7: External event 7
  *           @arg HRTIM_EVENT_8: External event 8
  *           @arg HRTIM_EVENT_9: External event 9
  *           @arg HRTIM_EVENT_10: External event 10
  * @param  pEventCfg pointer to the event conditioning configuration structure
  * @note This function must be called before starting the timer
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_EventConfig(HRTIM_HandleTypeDef * hhrtim,
                                        uint32_t Event,
                                        const HRTIM_EventCfgTypeDef* pEventCfg)
{
  /* Check parameters */
  assert_param(IS_HRTIM_EVENT(Event));
  assert_param(IS_HRTIM_EVENTSRC(Event, pEventCfg->Source));
  assert_param(IS_HRTIM_EVENTPOLARITY(pEventCfg->Sensitivity, pEventCfg->Polarity));
  assert_param(IS_HRTIM_EVENTSENSITIVITY(pEventCfg->Sensitivity));
  assert_param(IS_HRTIM_EVENTFASTMODE(Event, pEventCfg->FastMode));
  assert_param(IS_HRTIM_EVENTFILTER(Event, pEventCfg->Filter));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Configure the event channel */
  HRTIM_EventConfig(hhrtim, Event, pEventCfg);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the external event conditioning block prescaler
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Prescaler Prescaler value
  *         This parameter can be one of the following values:
  *           @arg HRTIM_EVENTPRESCALER_DIV1: fEEVS=fHRTIM
  *           @arg HRTIM_EVENTPRESCALER_DIV2: fEEVS=fHRTIM / 2
  *           @arg HRTIM_EVENTPRESCALER_DIV4: fEEVS=fHRTIM / 4
  *           @arg HRTIM_EVENTPRESCALER_DIV8: fEEVS=fHRTIM / 8
  * @note This function must be called before starting the timer
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_EventPrescalerConfig(HRTIM_HandleTypeDef * hhrtim,
                                                 uint32_t Prescaler)
{
  /* Check parameters */
  assert_param(IS_HRTIM_EVENTPRESCALER(Prescaler));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Set the external event prescaler */
  MODIFY_REG(hhrtim->Instance->sCommonRegs.EECR3, HRTIM_EECR3_EEVSD, (Prescaler & HRTIM_EECR3_EEVSD));

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the conditioning of fault input
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Fault fault input to configure
  *         This parameter can be one of the following values:
  *           @arg HRTIM_FAULT_1: Fault input 1
  *           @arg HRTIM_FAULT_2: Fault input 2
  *           @arg HRTIM_FAULT_3: Fault input 3
  *           @arg HRTIM_FAULT_4: Fault input 4
  *           @arg HRTIM_FAULT_5: Fault input 5
  *           @arg HRTIM_FAULT_6: Fault input 6
  * @param  pFaultCfg pointer to the fault conditioning configuration structure
  * @note This function must be called before starting the timer and before
  *       enabling faults inputs
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_FaultConfig(HRTIM_HandleTypeDef * hhrtim,
                                        uint32_t Fault,
                                        const HRTIM_FaultCfgTypeDef* pFaultCfg)
{
  uint32_t hrtim_fltinr1;
  uint32_t hrtim_fltinr2;
  uint32_t source0,source1;

  /* Check parameters */
  assert_param(IS_HRTIM_FAULT(Fault));
  assert_param(IS_HRTIM_FAULTSOURCE(pFaultCfg->Source));
  assert_param(IS_HRTIM_FAULTPOLARITY(pFaultCfg->Polarity));
  assert_param(IS_HRTIM_FAULTFILTER(pFaultCfg->Filter));
  assert_param(IS_HRTIM_FAULTLOCK(pFaultCfg->Lock));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Configure fault channel */
  hrtim_fltinr1 = hhrtim->Instance->sCommonRegs.FLTINR1;
  hrtim_fltinr2 = hhrtim->Instance->sCommonRegs.FLTINR2;

  source0 =  (pFaultCfg->Source & 1U);
  source1 = ((pFaultCfg->Source & 2U) >> 1);

  switch (Fault)
  {
  case HRTIM_FAULT_1:
    {
      hrtim_fltinr1 &= ~(HRTIM_FLTINR1_FLT1P | HRTIM_FLTINR1_FLT1SRC_0 | HRTIM_FLTINR1_FLT1F | HRTIM_FLTINR1_FLT1LCK);
      hrtim_fltinr1 |= (pFaultCfg->Polarity & HRTIM_FLTINR1_FLT1P);
      hrtim_fltinr1 |= (source0 << HRTIM_FLTINR1_FLT1SRC_0_Pos);
      hrtim_fltinr2 &= ~(HRTIM_FLTINR2_FLT1SRC_1);
      hrtim_fltinr2 |= (source1 << HRTIM_FLTINR2_FLT1SRC_1_Pos);
      hrtim_fltinr1 |= (pFaultCfg->Filter & HRTIM_FLTINR1_FLT1F);
      hrtim_fltinr1 |= (pFaultCfg->Lock & HRTIM_FLTINR1_FLT1LCK);
      break;
    }

  case HRTIM_FAULT_2:
    {
      hrtim_fltinr1 &= ~(HRTIM_FLTINR1_FLT2P | HRTIM_FLTINR1_FLT2SRC_0 | HRTIM_FLTINR1_FLT2F | HRTIM_FLTINR1_FLT2LCK);
      hrtim_fltinr1 |= ((pFaultCfg->Polarity << 8U) & HRTIM_FLTINR1_FLT2P);
      hrtim_fltinr1 |= (source0 << HRTIM_FLTINR1_FLT2SRC_0_Pos);
      hrtim_fltinr2 &= ~(HRTIM_FLTINR2_FLT2SRC_1);
      hrtim_fltinr2 |= (source1 << HRTIM_FLTINR2_FLT2SRC_1_Pos);
      hrtim_fltinr1 |= ((pFaultCfg->Filter << 8U) & HRTIM_FLTINR1_FLT2F);
      hrtim_fltinr1 |= ((pFaultCfg->Lock << 8U) & HRTIM_FLTINR1_FLT2LCK);
      break;
    }

  case HRTIM_FAULT_3:
    {
      hrtim_fltinr1 &= ~(HRTIM_FLTINR1_FLT3P | HRTIM_FLTINR1_FLT3SRC_0 | HRTIM_FLTINR1_FLT3F | HRTIM_FLTINR1_FLT3LCK);
      hrtim_fltinr1 |= ((pFaultCfg->Polarity << 16U) & HRTIM_FLTINR1_FLT3P);
      hrtim_fltinr1 |= (source0 << HRTIM_FLTINR1_FLT3SRC_0_Pos);
      hrtim_fltinr2 &= ~(HRTIM_FLTINR2_FLT3SRC_1);
      hrtim_fltinr2 |= (source1 << HRTIM_FLTINR2_FLT3SRC_1_Pos);
      hrtim_fltinr1 |= ((pFaultCfg->Filter << 16U) & HRTIM_FLTINR1_FLT3F);
      hrtim_fltinr1 |= ((pFaultCfg->Lock << 16U) & HRTIM_FLTINR1_FLT3LCK);
      break;
     }

  case HRTIM_FAULT_4:
    {
      hrtim_fltinr1 &= ~(HRTIM_FLTINR1_FLT4P | HRTIM_FLTINR1_FLT4SRC_0 | HRTIM_FLTINR1_FLT4F | HRTIM_FLTINR1_FLT4LCK);
      hrtim_fltinr1 |= ((pFaultCfg->Polarity << 24U) & HRTIM_FLTINR1_FLT4P);
      hrtim_fltinr1 |= (source0 << HRTIM_FLTINR1_FLT4SRC_0_Pos);
      hrtim_fltinr2 &= ~(HRTIM_FLTINR2_FLT4SRC_1);
      hrtim_fltinr2 |= (source1 << HRTIM_FLTINR2_FLT4SRC_1_Pos);
      hrtim_fltinr1 |= ((pFaultCfg->Filter << 24U) & HRTIM_FLTINR1_FLT4F);
      hrtim_fltinr1 |= ((pFaultCfg->Lock << 24U) & HRTIM_FLTINR1_FLT4LCK);
      break;
    }

  case HRTIM_FAULT_5:
    {
      hrtim_fltinr2 &= ~(HRTIM_FLTINR2_FLT5P | HRTIM_FLTINR2_FLT5SRC_0 | HRTIM_FLTINR2_FLT5F | HRTIM_FLTINR2_FLT5LCK);
      hrtim_fltinr2 |= (pFaultCfg->Polarity & HRTIM_FLTINR2_FLT5P);
      hrtim_fltinr2 |= (source0 << HRTIM_FLTINR2_FLT5SRC_0_Pos);
      hrtim_fltinr2 &= ~(HRTIM_FLTINR2_FLT5SRC_1);
      hrtim_fltinr2 |= (source1 << HRTIM_FLTINR2_FLT5SRC_1_Pos);
      hrtim_fltinr2 |= (pFaultCfg->Filter & HRTIM_FLTINR2_FLT5F);
      hrtim_fltinr2 |= (pFaultCfg->Lock & HRTIM_FLTINR2_FLT5LCK);
      break;
    }

  case HRTIM_FAULT_6:
    {
      hrtim_fltinr2 &= ~(HRTIM_FLTINR2_FLT6P | HRTIM_FLTINR2_FLT6SRC_0 | HRTIM_FLTINR2_FLT6F | HRTIM_FLTINR2_FLT6LCK);
      hrtim_fltinr2 |= ((pFaultCfg->Polarity << 8U) & HRTIM_FLTINR2_FLT6P);
      hrtim_fltinr2 |= (source0 << HRTIM_FLTINR2_FLT6SRC_0_Pos);
      hrtim_fltinr2 &= ~(HRTIM_FLTINR2_FLT6SRC_1);
      hrtim_fltinr2 |= (source1 << HRTIM_FLTINR2_FLT6SRC_1_Pos);
      hrtim_fltinr2 |= ((pFaultCfg->Filter << 8U) & HRTIM_FLTINR2_FLT6F);
      hrtim_fltinr2 |= ((pFaultCfg->Lock << 8U) & HRTIM_FLTINR2_FLT6LCK);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Update the HRTIM registers except LOCK bit */
  hhrtim->Instance->sCommonRegs.FLTINR1 = (hrtim_fltinr1 & (~(HRTIM_FLTINR1_FLTxLCK)));
  hhrtim->Instance->sCommonRegs.FLTINR2 = (hrtim_fltinr2 & (~(HRTIM_FLTINR2_FLTxLCK)));

  /* Update the HRTIM registers LOCK bit */
  SET_BIT(hhrtim->Instance->sCommonRegs.FLTINR1,(hrtim_fltinr1 & HRTIM_FLTINR1_FLTxLCK));
  SET_BIT(hhrtim->Instance->sCommonRegs.FLTINR2,(hrtim_fltinr2 & HRTIM_FLTINR2_FLTxLCK));

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the fault conditioning block prescaler
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Prescaler Prescaler value
  *         This parameter can be one of the following values:
  *           @arg HRTIM_FAULTPRESCALER_DIV1: fFLTS=fHRTIM
  *           @arg HRTIM_FAULTPRESCALER_DIV2: fFLTS=fHRTIM / 2
  *           @arg HRTIM_FAULTPRESCALER_DIV4: fFLTS=fHRTIM / 4
  *           @arg HRTIM_FAULTPRESCALER_DIV8: fFLTS=fHRTIM / 8
  * @retval HAL Status
  * @note This function must be called before starting the timer and before
  *       enabling faults inputs
  */
HAL_StatusTypeDef HAL_HRTIM_FaultPrescalerConfig(HRTIM_HandleTypeDef * hhrtim,
                                                 uint32_t Prescaler)
{
  /* Check parameters */
  assert_param(IS_HRTIM_FAULTPRESCALER(Prescaler));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Set the external event prescaler */
  MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR2, HRTIM_FLTINR2_FLTSD, (Prescaler & HRTIM_FLTINR2_FLTSD));

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure and Enable the blanking source of a Fault input
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Fault fault input to configure
  *         This parameter can be one of the following values:
  *           @arg HRTIM_FAULT_1: Fault input 1
  *           @arg HRTIM_FAULT_2: Fault input 2
  *           @arg HRTIM_FAULT_3: Fault input 3
  *           @arg HRTIM_FAULT_4: Fault input 4
  *           @arg HRTIM_FAULT_5: Fault input 5
  *           @arg HRTIM_FAULT_6: Fault input 6
  * @param  pFaultBlkCfg: pointer to the fault conditioning configuration structure
  * @note This function automatically enables the Blanking on Fault
  * @note This function must be called when fault is not enabled
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_FaultBlankingConfigAndEnable(HRTIM_HandleTypeDef * hhrtim,
                                                uint32_t Fault,
                                                const HRTIM_FaultBlankingCfgTypeDef* pFaultBlkCfg)
{
  /* Check parameters */
  assert_param(IS_HRTIM_FAULT(Fault));
  assert_param(IS_HRTIM_FAULTBLANKNGMODE(pFaultBlkCfg->BlankingSource));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  switch (Fault)
  {
   case HRTIM_FAULT_1:
    {
       MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR3,
                  (HRTIM_FLTINR3_FLT1BLKS | HRTIM_FLTINR3_FLT1BLKE),
                  ((pFaultBlkCfg->BlankingSource << HRTIM_FLTINR3_FLT1BLKS_Pos) |
                  HRTIM_FLTINR3_FLT1BLKE));
       break;
    }
   case HRTIM_FAULT_2:
    {
       MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR3,
                  (HRTIM_FLTINR3_FLT2BLKS | HRTIM_FLTINR3_FLT2BLKE),
                  ((pFaultBlkCfg->BlankingSource << HRTIM_FLTINR3_FLT2BLKS_Pos) |
                  HRTIM_FLTINR3_FLT2BLKE));
       break;
    }
   case HRTIM_FAULT_3:
    {
       MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR3,
                  (HRTIM_FLTINR3_FLT3BLKS | HRTIM_FLTINR3_FLT3BLKE),
                  ((pFaultBlkCfg->BlankingSource << HRTIM_FLTINR3_FLT3BLKS_Pos) |
                  HRTIM_FLTINR3_FLT3BLKE));
       break;
    }
   case HRTIM_FAULT_4:
    {
       MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR3,
                  (HRTIM_FLTINR3_FLT4BLKS | HRTIM_FLTINR3_FLT4BLKE),
                  ((pFaultBlkCfg->BlankingSource << HRTIM_FLTINR3_FLT4BLKS_Pos) |
                  HRTIM_FLTINR3_FLT4BLKE));
       break;
    }
   case HRTIM_FAULT_5:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR4,
                  (HRTIM_FLTINR4_FLT5BLKS | HRTIM_FLTINR4_FLT5BLKE),
                  ((pFaultBlkCfg->BlankingSource << HRTIM_FLTINR4_FLT5BLKS_Pos) |
                  HRTIM_FLTINR4_FLT5BLKE));
      break;
    }
   case HRTIM_FAULT_6:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR4,
                  (HRTIM_FLTINR4_FLT6BLKS | HRTIM_FLTINR4_FLT6BLKE),
                  ((pFaultBlkCfg->BlankingSource << HRTIM_FLTINR4_FLT6BLKS_Pos) |
                  HRTIM_FLTINR4_FLT6BLKE));
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the Fault Counter (Threshold and Reset Mode)
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Fault fault input to configure
  *         This parameter can be one of the following values:
  *           @arg HRTIM_FAULT_1: Fault input 1
  *           @arg HRTIM_FAULT_2: Fault input 2
  *           @arg HRTIM_FAULT_3: Fault input 3
  *           @arg HRTIM_FAULT_4: Fault input 4
  *           @arg HRTIM_FAULT_5: Fault input 5
  *           @arg HRTIM_FAULT_6: Fault input 6
  * @param  pFaultBlkCfg: pointer to the fault conditioning configuration structure
  * @retval HAL Status
  * @note  A fault is considered valid when the number of
  *        events is equal to the (FLTxCNT[3:0]+1) value
  *
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_FaultCounterConfig(HRTIM_HandleTypeDef * hhrtim,
                                               uint32_t Fault,
                                               const HRTIM_FaultBlankingCfgTypeDef* pFaultBlkCfg)
{
  /* Check parameters */
  assert_param(IS_HRTIM_FAULT(Fault));
  assert_param(IS_HRTIM_FAULTCOUNTER(pFaultBlkCfg->Threshold));
  assert_param(IS_HRTIM_FAULTCOUNTERRST(pFaultBlkCfg->ResetMode));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  switch (Fault)
  {
   case HRTIM_FAULT_1:
    {
       MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR3,
                  (HRTIM_FLTINR3_FLT1RSTM | HRTIM_FLTINR3_FLT1CNT),
                  (pFaultBlkCfg->Threshold << HRTIM_FLTINR3_FLT1CNT_Pos) |
                  (pFaultBlkCfg->ResetMode << HRTIM_FLTINR3_FLT1RSTM_Pos));
       break;
    }
   case HRTIM_FAULT_2:
    {
       MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR3,
                  (HRTIM_FLTINR3_FLT2RSTM | HRTIM_FLTINR3_FLT2CNT),
                  (pFaultBlkCfg->Threshold << HRTIM_FLTINR3_FLT2CNT_Pos) |
                  (pFaultBlkCfg->ResetMode << HRTIM_FLTINR3_FLT2RSTM_Pos));
       break;
    }
   case HRTIM_FAULT_3:
    {
       MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR3,
                  (HRTIM_FLTINR3_FLT3RSTM | HRTIM_FLTINR3_FLT3CNT),
                  (pFaultBlkCfg->Threshold << HRTIM_FLTINR3_FLT3CNT_Pos) |
                  (pFaultBlkCfg->ResetMode << HRTIM_FLTINR3_FLT3RSTM_Pos));
       break;
    }
   case HRTIM_FAULT_4:
    {
       MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR3,
                  (HRTIM_FLTINR3_FLT4RSTM | HRTIM_FLTINR3_FLT4CNT),
                  (pFaultBlkCfg->Threshold << HRTIM_FLTINR3_FLT4CNT_Pos) |
                  (pFaultBlkCfg->ResetMode << HRTIM_FLTINR3_FLT4RSTM_Pos));
       break;
    }
   case HRTIM_FAULT_5:
    {
       MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR4,
                  (HRTIM_FLTINR4_FLT5RSTM | HRTIM_FLTINR4_FLT5CNT),
                  (pFaultBlkCfg->Threshold << HRTIM_FLTINR4_FLT5CNT_Pos) |
                  (pFaultBlkCfg->ResetMode << HRTIM_FLTINR4_FLT5RSTM_Pos));
       break;
    }
   case HRTIM_FAULT_6:
    {
       MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR4,
                  (HRTIM_FLTINR4_FLT6RSTM | HRTIM_FLTINR4_FLT6CNT),
                  (pFaultBlkCfg->Threshold << HRTIM_FLTINR4_FLT6CNT_Pos) |
                  (pFaultBlkCfg->ResetMode << HRTIM_FLTINR4_FLT6RSTM_Pos));
       break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Reset the fault Counter Reset
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Fault fault input to reset
  *         This parameter can be one of the following values:
  *           @arg HRTIM_FAULT_1: Fault input 1
  *           @arg HRTIM_FAULT_2: Fault input 2
  *           @arg HRTIM_FAULT_3: Fault input 3
  *           @arg HRTIM_FAULT_4: Fault input 4
  *           @arg HRTIM_FAULT_5: Fault input 5
  *           @arg HRTIM_FAULT_6: Fault input 6
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_FaultCounterReset(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t Fault)
{
  /* Check parameters */
  assert_param(IS_HRTIM_FAULT(Fault));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  switch (Fault)
  {
   case HRTIM_FAULT_1:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR3, HRTIM_FLTINR3_FLT1CRES, HRTIM_FLTINR3_FLT1CRES) ;
      break;
    }
   case HRTIM_FAULT_2:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR3, HRTIM_FLTINR3_FLT2CRES, HRTIM_FLTINR3_FLT2CRES) ;
      break;
    }
   case HRTIM_FAULT_3:
    {
       MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR3, HRTIM_FLTINR3_FLT3CRES, HRTIM_FLTINR3_FLT3CRES) ;
       break;
    }
   case HRTIM_FAULT_4:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR3, HRTIM_FLTINR3_FLT4CRES, HRTIM_FLTINR3_FLT4CRES) ;
      break;
    }
   case HRTIM_FAULT_5:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR4, HRTIM_FLTINR4_FLT5CRES, HRTIM_FLTINR4_FLT5CRES) ;
      break;
    }
   case HRTIM_FAULT_6:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR4, HRTIM_FLTINR4_FLT6CRES, HRTIM_FLTINR4_FLT6CRES) ;
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Enable or disables the HRTIMx Fault mode.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Fault fault input to reset
  *         This parameter can be one of the following values:
  *           @arg HRTIM_FAULT_1: Fault input 1
  *           @arg HRTIM_FAULT_2: Fault input 2
  *           @arg HRTIM_FAULT_3: Fault input 3
  *           @arg HRTIM_FAULT_4: Fault input 4
  *           @arg HRTIM_FAULT_5: Fault input 5
  *           @arg HRTIM_FAULT_6: Fault input 6
  * @param  Enable Fault(s) enabling
  *         This parameter can be one of the following values:
  *           @arg HRTIM_FAULTMODECTL_ENABLED: Fault(s) enabled
  *           @arg HRTIM_FAULTMODECTL_DISABLED: Fault(s) disabled
  * @retval None
  */
void HAL_HRTIM_FaultModeCtl(HRTIM_HandleTypeDef * hhrtim,
                        uint32_t Faults,
                        uint32_t Enable)
{
  /* Check parameters */
  assert_param(IS_HRTIM_FAULT(Faults));
  assert_param(IS_HRTIM_FAULTMODECTL(Enable));

  if ((Faults & HRTIM_FAULT_1) != (uint32_t)RESET)
  {
    MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR1, HRTIM_FLTINR1_FLT1E, (Enable & HRTIM_FLTINR1_FLT1E));
  }
  if ((Faults & HRTIM_FAULT_2) != (uint32_t)RESET)
  {
    MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR1, HRTIM_FLTINR1_FLT2E, ((Enable << 8U) & HRTIM_FLTINR1_FLT2E));
  }
  if ((Faults & HRTIM_FAULT_3) != (uint32_t)RESET)
  {
    MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR1, HRTIM_FLTINR1_FLT3E, ((Enable << 16U) & HRTIM_FLTINR1_FLT3E));
  }
  if ((Faults & HRTIM_FAULT_4) != (uint32_t)RESET)
  {
    MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR1, HRTIM_FLTINR1_FLT4E, ((Enable << 24U) & HRTIM_FLTINR1_FLT4E));
  }
  if ((Faults & HRTIM_FAULT_5) != (uint32_t)RESET)
  {
    MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR2, HRTIM_FLTINR2_FLT5E, ((Enable) & HRTIM_FLTINR2_FLT5E));
  }
  if ((Faults & HRTIM_FAULT_6) != (uint32_t)RESET)
  {
    MODIFY_REG(hhrtim->Instance->sCommonRegs.FLTINR2, HRTIM_FLTINR2_FLT6E, ((Enable << 8U) & HRTIM_FLTINR2_FLT6E));
  }
}

/**
  * @brief  Configure both the ADC trigger register update source and the ADC
  *         trigger source.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  ADCTrigger ADC trigger to configure
  *         This parameter can be one of the following values:
  *           @arg HRTIM_ADCTRIGGER_1: ADC trigger 1
  *           @arg HRTIM_ADCTRIGGER_2: ADC trigger 2
  *           @arg HRTIM_ADCTRIGGER_3: ADC trigger 3
  *           @arg HRTIM_ADCTRIGGER_4: ADC trigger 4
  *           @arg HRTIM_ADCTRIGGER_5: ADC trigger 5
  *           @arg HRTIM_ADCTRIGGER_6: ADC trigger 6
  *           @arg HRTIM_ADCTRIGGER_7: ADC trigger 7
  *           @arg HRTIM_ADCTRIGGER_8: ADC trigger 8
  *           @arg HRTIM_ADCTRIGGER_9: ADC trigger 9
  *           @arg HRTIM_ADCTRIGGER_10: ADC trigger 10
  * @param  pADCTriggerCfg pointer to the ADC trigger configuration structure
  *                        for Trigger nb (1..4): pADCTriggerCfg->Trigger parameter
  *                        can be a combination of the following values
  *           @arg HRTIM_ADCTRIGGEREVENT13_...
  *           @arg HRTIM_ADCTRIGGEREVENT24_...
  *                        for Trigger nb (5..10): pADCTriggerCfg->Trigger parameter
  *                        can be one of the following values
  *           @arg HRTIM_ADCTRIGGEREVENT579_...
  *           @arg HRTIM_ADCTRIGGEREVENT6810_...
  * @retval HAL Status
  * @note This function must be called before starting the timer
  */
HAL_StatusTypeDef HAL_HRTIM_ADCTriggerConfig(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t ADCTrigger,
                                             const HRTIM_ADCTriggerCfgTypeDef* pADCTriggerCfg)
{
  uint32_t hrtim_cr1;
  uint32_t hrtim_adcur;

  /* Check parameters */
  assert_param(IS_HRTIM_ADCTRIGGER(ADCTrigger));
  assert_param(IS_HRTIM_ADCTRIGGERUPDATE(pADCTriggerCfg->UpdateSource));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Set the ADC trigger update source */
  hrtim_cr1 = hhrtim->Instance->sCommonRegs.CR1;
  hrtim_adcur = hhrtim->Instance->sCommonRegs.ADCUR;

  switch (ADCTrigger)
  {
  case HRTIM_ADCTRIGGER_1:
    {
      hrtim_cr1 &= ~(HRTIM_CR1_ADC1USRC);
      hrtim_cr1 |= (pADCTriggerCfg->UpdateSource & HRTIM_CR1_ADC1USRC);

      /* Set the ADC trigger 1 source */
      hhrtim->Instance->sCommonRegs.ADC1R = pADCTriggerCfg->Trigger;
      break;
    }

  case HRTIM_ADCTRIGGER_2:
    {
      hrtim_cr1 &= ~(HRTIM_CR1_ADC2USRC);
      hrtim_cr1 |= ((pADCTriggerCfg->UpdateSource << 3U) & HRTIM_CR1_ADC2USRC);

      /* Set the ADC trigger 2 source */
      hhrtim->Instance->sCommonRegs.ADC2R = pADCTriggerCfg->Trigger;
      break;
    }

  case HRTIM_ADCTRIGGER_3:
    {
      hrtim_cr1 &= ~(HRTIM_CR1_ADC3USRC);
      hrtim_cr1 |= ((pADCTriggerCfg->UpdateSource << 6U) & HRTIM_CR1_ADC3USRC);

      /* Set the ADC trigger 3 source */
      hhrtim->Instance->sCommonRegs.ADC3R = pADCTriggerCfg->Trigger;
      break;
    }

  case HRTIM_ADCTRIGGER_4:
    {
      hrtim_cr1 &= ~(HRTIM_CR1_ADC4USRC);
      hrtim_cr1 |= ((pADCTriggerCfg->UpdateSource << 9U) & HRTIM_CR1_ADC4USRC);

      /* Set the ADC trigger 4 source */
      hhrtim->Instance->sCommonRegs.ADC4R = pADCTriggerCfg->Trigger;
      break;
    }

  case HRTIM_ADCTRIGGER_5:
    {
      hrtim_adcur &= ~(HRTIM_ADCUR_AD5USRC);
      hrtim_adcur |= ((pADCTriggerCfg->UpdateSource >> 16U) & HRTIM_ADCUR_AD5USRC);

      /* Set the ADC trigger 5 source */
      hhrtim->Instance->sCommonRegs.ADCER &= ~(HRTIM_ADCER_AD5TRG);
      hhrtim->Instance->sCommonRegs.ADCER |= ((pADCTriggerCfg->Trigger << HRTIM_ADCER_AD5TRG_Pos) & HRTIM_ADCER_AD5TRG);
      break;
    }

  case HRTIM_ADCTRIGGER_6:
    {
      hrtim_adcur &= ~(HRTIM_ADCUR_AD6USRC);
      hrtim_adcur |= ((pADCTriggerCfg->UpdateSource >> 12U) & HRTIM_ADCUR_AD6USRC);

      /* Set the ADC trigger 6 source */
      hhrtim->Instance->sCommonRegs.ADCER &= ~(HRTIM_ADCER_AD6TRG);
      hhrtim->Instance->sCommonRegs.ADCER |= ((pADCTriggerCfg->Trigger << HRTIM_ADCER_AD6TRG_Pos) & HRTIM_ADCER_AD6TRG);
      break;
    }

  case HRTIM_ADCTRIGGER_7:
    {
      hrtim_adcur &= ~(HRTIM_ADCUR_AD7USRC);
      hrtim_adcur |= ((pADCTriggerCfg->UpdateSource >> 8U) & HRTIM_ADCUR_AD7USRC);

      /* Set the ADC trigger 7 source */
      hhrtim->Instance->sCommonRegs.ADCER &= ~(HRTIM_ADCER_AD7TRG);
      hhrtim->Instance->sCommonRegs.ADCER |= ((pADCTriggerCfg->Trigger << HRTIM_ADCER_AD7TRG_Pos) & HRTIM_ADCER_AD7TRG);
      break;
    }

  case HRTIM_ADCTRIGGER_8:
    {
      hrtim_adcur &= ~(HRTIM_ADCUR_AD8USRC);
      hrtim_adcur |= ((pADCTriggerCfg->UpdateSource >> 4U) & HRTIM_ADCUR_AD8USRC);

      /* Set the ADC trigger 8 source */
      hhrtim->Instance->sCommonRegs.ADCER &= ~(HRTIM_ADCER_AD8TRG);
      hhrtim->Instance->sCommonRegs.ADCER |= ((pADCTriggerCfg->Trigger << HRTIM_ADCER_AD8TRG_Pos) & HRTIM_ADCER_AD8TRG);
      break;
    }

  case HRTIM_ADCTRIGGER_9:
    {
      hrtim_adcur &= ~(HRTIM_ADCUR_AD9USRC);
      hrtim_adcur |= ((pADCTriggerCfg->UpdateSource) & HRTIM_ADCUR_AD9USRC);

      /* Set the ADC trigger 9 source */
      hhrtim->Instance->sCommonRegs.ADCER &= ~(HRTIM_ADCER_AD9TRG);
      hhrtim->Instance->sCommonRegs.ADCER |= ((pADCTriggerCfg->Trigger << HRTIM_ADCER_AD9TRG_Pos) & HRTIM_ADCER_AD9TRG);
      break;
    }

  case HRTIM_ADCTRIGGER_10:
    {
      hrtim_adcur &= ~(HRTIM_ADCUR_AD10USRC);
      hrtim_adcur |= ((pADCTriggerCfg->UpdateSource << 4U) & HRTIM_ADCUR_AD10USRC);

      /* Set the ADC trigger 10 source */
      hhrtim->Instance->sCommonRegs.ADCER &= ~(HRTIM_ADCER_AD10TRG);
      hhrtim->Instance->sCommonRegs.ADCER |= ((pADCTriggerCfg->Trigger << HRTIM_ADCER_AD10TRG_Pos) & HRTIM_ADCER_AD10TRG);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  /* Update the HRTIM registers */
  if (ADCTrigger < HRTIM_ADCTRIGGER_5)
  {
   hhrtim->Instance->sCommonRegs.CR1 = hrtim_cr1;
  }
  else
  {
   hhrtim->Instance->sCommonRegs.ADCUR = hrtim_adcur;
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the ADC trigger postscaler register of the ADC
  *         trigger source.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  ADCTrigger ADC trigger to configure
  *         This parameter can be one of the following values:
  *           @arg HRTIM_ADCTRIGGER_1: ADC trigger 1
  *           @arg HRTIM_ADCTRIGGER_2: ADC trigger 2
  *           @arg HRTIM_ADCTRIGGER_3: ADC trigger 3
  *           @arg HRTIM_ADCTRIGGER_4: ADC trigger 4
  *           @arg HRTIM_ADCTRIGGER_5: ADC trigger 5
  *           @arg HRTIM_ADCTRIGGER_6: ADC trigger 6
  *           @arg HRTIM_ADCTRIGGER_7: ADC trigger 7
  *           @arg HRTIM_ADCTRIGGER_8: ADC trigger 8
  *           @arg HRTIM_ADCTRIGGER_9: ADC trigger 9
  *           @arg HRTIM_ADCTRIGGER_10: ADC trigger 10
  * @param  Postscaler  value 0..1F
  * @retval HAL Status
  * @note This function must be called before starting the timer
  */
HAL_StatusTypeDef HAL_HRTIM_ADCPostScalerConfig(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t ADCTrigger,
                                             uint32_t Postscaler)
{
  /* Check parameters */
  assert_param(IS_HRTIM_ADCTRIGGER(ADCTrigger));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  switch (ADCTrigger)
  {
  case HRTIM_ADCTRIGGER_1:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.ADCPS1, HRTIM_ADCPS1_AD1PSC, (Postscaler & HRTIM_ADCPS1_AD1PSC));
      break;
    }

  case HRTIM_ADCTRIGGER_2:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.ADCPS1, HRTIM_ADCPS1_AD2PSC, ((Postscaler << HRTIM_ADCPS1_AD2PSC_Pos) & HRTIM_ADCPS1_AD2PSC));
      break;
    }

  case HRTIM_ADCTRIGGER_3:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.ADCPS1, HRTIM_ADCPS1_AD3PSC, ((Postscaler << HRTIM_ADCPS1_AD3PSC_Pos) & HRTIM_ADCPS1_AD3PSC));
      break;
    }

  case HRTIM_ADCTRIGGER_4:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.ADCPS1, HRTIM_ADCPS1_AD4PSC, ((Postscaler << HRTIM_ADCPS1_AD4PSC_Pos) & HRTIM_ADCPS1_AD4PSC));
      break;
    }

  case HRTIM_ADCTRIGGER_5:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.ADCPS1, HRTIM_ADCPS1_AD5PSC, ((Postscaler << HRTIM_ADCPS1_AD5PSC_Pos) & HRTIM_ADCPS1_AD5PSC));
      break;
    }

  case HRTIM_ADCTRIGGER_6:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.ADCPS2, HRTIM_ADCPS2_AD6PSC, ((Postscaler << HRTIM_ADCPS2_AD6PSC_Pos) & HRTIM_ADCPS2_AD6PSC));
      break;
    }

  case HRTIM_ADCTRIGGER_7:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.ADCPS2, HRTIM_ADCPS2_AD7PSC, ((Postscaler << HRTIM_ADCPS2_AD7PSC_Pos) & HRTIM_ADCPS2_AD7PSC));
      break;
    }

  case HRTIM_ADCTRIGGER_8:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.ADCPS2, HRTIM_ADCPS2_AD8PSC, ((Postscaler << HRTIM_ADCPS2_AD8PSC_Pos) & HRTIM_ADCPS2_AD8PSC));
      break;
    }

  case HRTIM_ADCTRIGGER_9:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.ADCPS2, HRTIM_ADCPS2_AD9PSC, ((Postscaler << HRTIM_ADCPS2_AD9PSC_Pos) & HRTIM_ADCPS2_AD9PSC));
      break;
    }

  case HRTIM_ADCTRIGGER_10:
    {
      MODIFY_REG(hhrtim->Instance->sCommonRegs.ADCPS2, HRTIM_ADCPS2_AD10PSC, ((Postscaler << HRTIM_ADCPS2_AD10PSC_Pos) & HRTIM_ADCPS2_AD10PSC));
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the ADC Roll-Over mode of the ADC
  *         trigger source.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  RollOverCfg This parameter can be a combination of all the following values:
  *           @arg HRTIM_TIM_FEROM_BOTH  or HRTIM_TIM_FEROM_CREST  or HRTIM_TIM_FEROM_VALLEY
  *           @arg HRTIM_TIM_BMROM_BOTH  or HRTIM_TIM_BMROM_CREST  or HRTIM_TIM_BMROM_VALLEY
  *           @arg HRTIM_TIM_ADROM_BOTH  or HRTIM_TIM_ADROM_CREST  or HRTIM_TIM_ADROM_VALLEY
  *           @arg HRTIM_TIM_OUTROM_BOTH or HRTIM_TIM_OUTROM_CREST or HRTIM_TIM_OUTROM_VALLEY
  *           @arg HRTIM_TIM_ROM_BOTH    or HRTIM_TIM_ROM_CREST    or HRTIM_TIM_ROM_VALLEY
  * @note This function must be called before starting the timer
  */
HAL_StatusTypeDef HAL_HRTIM_RollOverModeConfig(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t RollOverCfg)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_ROLLOVERMODE(RollOverCfg));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

   HRTIM_TimingUnitRollOver_Config(hhrtim,TimerIdx,RollOverCfg);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the general behavior of a timer operating in waveform mode
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  pTimerCfg pointer to the timer configuration structure
  * @note When the timer operates in waveform mode, all the features supported by
  *       the HRTIM are available without any limitation.
  * @retval HAL Status
  * @note This function must be called before starting the timer
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformTimerConfig(HRTIM_HandleTypeDef * hhrtim,
                                                uint32_t TimerIdx,
                                                const HRTIM_TimerCfgTypeDef * pTimerCfg)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMERINDEX(TimerIdx));

  /* Relevant for all HRTIM timers, including the master */
  assert_param(IS_HRTIM_HALFMODE(pTimerCfg->HalfModeEnable));
  assert_param(IS_HRTIM_INTERLEAVEDMODE(pTimerCfg->InterleavedMode));
  assert_param(IS_HRTIM_SYNCSTART(pTimerCfg->StartOnSync));
  assert_param(IS_HRTIM_SYNCRESET(pTimerCfg->ResetOnSync));
  assert_param(IS_HRTIM_DACSYNC(pTimerCfg->DACSynchro));
  assert_param(IS_HRTIM_PRELOAD(pTimerCfg->PreloadEnable));
  assert_param(IS_HRTIM_TIMERBURSTMODE(pTimerCfg->BurstMode));
  assert_param(IS_HRTIM_UPDATEONREPETITION(pTimerCfg->RepetitionUpdate));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  if (TimerIdx == HRTIM_TIMERINDEX_MASTER)
  {
    /* Check parameters */
    assert_param(IS_HRTIM_UPDATEGATING_MASTER(pTimerCfg->UpdateGating));
    assert_param(IS_HRTIM_MASTER_IT(pTimerCfg->InterruptRequests));
    assert_param(IS_HRTIM_MASTER_DMA(pTimerCfg->DMARequests));

    /* Configure master timer */
    HRTIM_MasterWaveform_Config(hhrtim, pTimerCfg);
  }
  else
  {
    /* Check parameters */
    assert_param(IS_HRTIM_UPDATEGATING_TIM(pTimerCfg->UpdateGating));
    assert_param(IS_HRTIM_TIM_IT(pTimerCfg->InterruptRequests));
    assert_param(IS_HRTIM_TIM_DMA(pTimerCfg->DMARequests));
    assert_param(IS_HRTIM_TIMPUSHPULLMODE(pTimerCfg->PushPull));
    assert_param(IS_HRTIM_TIMFAULTENABLE(pTimerCfg->FaultEnable));
    assert_param(IS_HRTIM_TIMFAULTLOCK(pTimerCfg->FaultLock));
    assert_param(IS_HRTIM_TIMDEADTIMEINSERTION(pTimerCfg->PushPull,
                                               pTimerCfg->DeadTimeInsertion));
    assert_param(IS_HRTIM_TIMDELAYEDPROTECTION(pTimerCfg->PushPull,
                                               pTimerCfg->DelayedProtectionMode));
    assert_param(IS_HRTIM_OUTPUTBALANCEDIDLE(pTimerCfg->BalancedIdleAutomaticResume));
    assert_param(IS_HRTIM_TIMUPDATETRIGGER(pTimerCfg->UpdateTrigger));
    assert_param(IS_HRTIM_TIMRESETTRIGGER(pTimerCfg->ResetTrigger));
    assert_param(IS_HRTIM_TIMUPDATEONRESET(pTimerCfg->ResetUpdate));
    assert_param(IS_HRTIM_TIMSYNCUPDATE(pTimerCfg->ReSyncUpdate));

    /* Configure timing unit */
    HRTIM_TimingUnitWaveform_Config(hhrtim, TimerIdx, pTimerCfg);
  }

  /* Update timer parameters */
  hhrtim->TimerParam[TimerIdx].InterruptRequests = pTimerCfg->InterruptRequests;
  hhrtim->TimerParam[TimerIdx].DMARequests = pTimerCfg->DMARequests;
  hhrtim->TimerParam[TimerIdx].DMASrcAddress = pTimerCfg->DMASrcAddress;
  hhrtim->TimerParam[TimerIdx].DMADstAddress = pTimerCfg->DMADstAddress;
  hhrtim->TimerParam[TimerIdx].DMASize = pTimerCfg->DMASize;

  /* Force a software update */
  HRTIM_ForceRegistersUpdate(hhrtim, TimerIdx);

  /* Configure slave timer update re-synchronization */
  if ((TimerIdx != HRTIM_TIMERINDEX_MASTER)
   && (pTimerCfg->UpdateGating == HRTIM_UPDATEGATING_INDEPENDENT))
  {
    MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR,
               HRTIM_TIMCR_RSYNCU_Msk,
               pTimerCfg->ReSyncUpdate << HRTIM_TIMCR_RSYNCU_Pos);
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the general behavior of a timer operating in waveform mode
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  pTimerCtl pointer to the timer configuration structure
  * @note When the timer operates in waveform mode, all the features supported by
  *       the HRTIM are available without any limitation.
  * @retval HAL Status
  * @note This function must be called before starting the timer
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformTimerControl(HRTIM_HandleTypeDef * hhrtim,
                                                uint32_t TimerIdx,
                                                const HRTIM_TimerCtlTypeDef * pTimerCtl)
{
    /* Check parameters */
    assert_param(IS_HRTIM_TIMERINDEX(TimerIdx));
    /* Relevant for all A..F HRTIM timers */
    assert_param(IS_HRTIM_TIMERUPDOWNMODE(pTimerCtl->UpDownMode));
    assert_param(IS_HRTIM_TIMERTRGHLFMODE(pTimerCtl->TrigHalf));
    assert_param(IS_HRTIM_TIMERGTCMP3(pTimerCtl->GreaterCMP3));
    assert_param(IS_HRTIM_TIMERGTCMP1(pTimerCtl->GreaterCMP1));
    assert_param(IS_HRTIM_DUALDAC_RESET(pTimerCtl->DualChannelDacReset));
    assert_param(IS_HRTIM_DUALDAC_STEP(pTimerCtl->DualChannelDacStep));
    assert_param(IS_HRTIM_DUALDAC_ENABLE(pTimerCtl->DualChannelDacEnable));

    if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
    {
       return HAL_BUSY;
    }

    /* Process Locked */
    __HAL_LOCK(hhrtim);

    hhrtim->State = HAL_HRTIM_STATE_BUSY;

    /* Configure timing unit */
    HRTIM_TimingUnitWaveform_Control(hhrtim, TimerIdx, pTimerCtl);

    /* Force a software update */
    HRTIM_ForceRegistersUpdate(hhrtim, TimerIdx);

    hhrtim->State = HAL_HRTIM_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(hhrtim);

    return HAL_OK;
}

/**
  * @brief  Configure the Dual Channel Dac behavior of a timer operating in waveform mode
  * @param  hhrtim: pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  pTimerCtl pointer to the timer DualChannel Dac configuration structure
  * @note When the timer operates in waveform mode, all the features supported by
  *       the HRTIM are available without any limitation.
  * @retval HAL Status
  * @note This function must be called before starting the timer
  */
HAL_StatusTypeDef HAL_HRTIM_TimerDualChannelDacConfig(HRTIM_HandleTypeDef * hhrtim,
                                                uint32_t TimerIdx,
                                                const HRTIM_TimerCtlTypeDef * pTimerCtl)
{
    assert_param(IS_HRTIM_DUALDAC_RESET(pTimerCtl->DualChannelDacReset));
    assert_param(IS_HRTIM_DUALDAC_STEP(pTimerCtl->DualChannelDacStep));
    assert_param(IS_HRTIM_DUALDAC_ENABLE(pTimerCtl->DualChannelDacEnable));

    if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
    {
     return HAL_BUSY;
    }

    /* Process Locked */
    __HAL_LOCK(hhrtim);

    hhrtim->State = HAL_HRTIM_STATE_BUSY;
    /* clear DCDS,DCDR,DCDE bits */
    CLEAR_BIT(hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR2,
                   (HRTIM_TIMER_DCDE_ENABLED |
                    HRTIM_TIMER_DCDS_OUT1RST |
                    HRTIM_TIMER_DCDR_OUT1SET) );

    MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR2 ,
                   (HRTIM_TIMER_DCDE_ENABLED |
                    HRTIM_TIMER_DCDS_OUT1RST |
                    HRTIM_TIMER_DCDR_OUT1SET),
                   (pTimerCtl->DualChannelDacReset |
                    pTimerCtl->DualChannelDacStep |
                    pTimerCtl->DualChannelDacEnable));

    hhrtim->State = HAL_HRTIM_STATE_READY;

     /* Process Unlocked */
     __HAL_UNLOCK(hhrtim);

     return HAL_OK;
}

/**
  * @brief  Configure the event filtering capabilities of a timer (blanking, windowing)
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  Event external event for which timer event filtering must be configured
  *         This parameter can be one of the following values:
  *           @arg HRTIM_EVENT_1: External event 1
  *           @arg HRTIM_EVENT_2: External event 2
  *           @arg HRTIM_EVENT_3: External event 3
  *           @arg HRTIM_EVENT_4: External event 4
  *           @arg HRTIM_EVENT_5: External event 5
  *           @arg HRTIM_EVENT_6: External event 6
  *           @arg HRTIM_EVENT_7: External event 7
  *           @arg HRTIM_EVENT_8: External event 8
  *           @arg HRTIM_EVENT_9: External event 9
  *           @arg HRTIM_EVENT_10: External event 10
  * @param  pTimerEventFilteringCfg pointer to the timer event filtering configuration structure
  * @note This function must be called before starting the timer
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_TimerEventFilteringConfig(HRTIM_HandleTypeDef * hhrtim,
                                                      uint32_t TimerIdx,
                                                      uint32_t Event,
                                                      const HRTIM_TimerEventFilteringCfgTypeDef* pTimerEventFilteringCfg)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_EVENT(Event));
  assert_param(IS_HRTIM_TIMEVENTFILTER(TimerIdx,pTimerEventFilteringCfg->Filter));

  assert_param(IS_HRTIM_TIMEVENTLATCH(pTimerEventFilteringCfg->Latch));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Configure timer event filtering capabilities */
  switch (Event)
  {
  case HRTIM_EVENT_NONE:
    {
      CLEAR_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR1);
      CLEAR_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR2);
      break;
    }

  case HRTIM_EVENT_1:
    {
      MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR1, (HRTIM_EEFR1_EE1FLTR | HRTIM_EEFR1_EE1LTCH), (pTimerEventFilteringCfg->Filter | pTimerEventFilteringCfg->Latch));
      break;
    }

  case HRTIM_EVENT_2:
    {
      MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR1, (HRTIM_EEFR1_EE2FLTR | HRTIM_EEFR1_EE2LTCH), ((pTimerEventFilteringCfg->Filter | pTimerEventFilteringCfg->Latch) << 6U) );
      break;
    }

  case HRTIM_EVENT_3:
    {
      MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR1, (HRTIM_EEFR1_EE3FLTR | HRTIM_EEFR1_EE3LTCH), ((pTimerEventFilteringCfg->Filter | pTimerEventFilteringCfg->Latch) << 12U) );
      break;
    }

  case HRTIM_EVENT_4:
    {
      MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR1, (HRTIM_EEFR1_EE4FLTR | HRTIM_EEFR1_EE4LTCH), ((pTimerEventFilteringCfg->Filter | pTimerEventFilteringCfg->Latch) << 18U) );
      break;
    }

  case HRTIM_EVENT_5:
    {
      MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR1, (HRTIM_EEFR1_EE5FLTR | HRTIM_EEFR1_EE5LTCH), ((pTimerEventFilteringCfg->Filter | pTimerEventFilteringCfg->Latch) << 24U) );
      break;
    }

  case HRTIM_EVENT_6:
    {
      MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR2, (HRTIM_EEFR2_EE6FLTR | HRTIM_EEFR2_EE6LTCH), (pTimerEventFilteringCfg->Filter | pTimerEventFilteringCfg->Latch) );
      break;
    }

  case HRTIM_EVENT_7:
    {
      MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR2, (HRTIM_EEFR2_EE7FLTR | HRTIM_EEFR2_EE7LTCH), ((pTimerEventFilteringCfg->Filter | pTimerEventFilteringCfg->Latch) << 6U) );
      break;
    }

  case HRTIM_EVENT_8:
    {
      MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR2, (HRTIM_EEFR2_EE8FLTR | HRTIM_EEFR2_EE8LTCH), ((pTimerEventFilteringCfg->Filter | pTimerEventFilteringCfg->Latch) << 12U) );
      break;
    }

  case HRTIM_EVENT_9:
    {
      MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR2, (HRTIM_EEFR2_EE9FLTR | HRTIM_EEFR2_EE9LTCH), ((pTimerEventFilteringCfg->Filter | pTimerEventFilteringCfg->Latch) << 18U) );
      break;
    }

  case HRTIM_EVENT_10:
    {
      MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR2, (HRTIM_EEFR2_EE10FLTR | HRTIM_EEFR2_EE10LTCH), ((pTimerEventFilteringCfg->Filter | pTimerEventFilteringCfg->Latch) << 24U) );
      break;
    }

  default:
   {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the external Event Counter A or B of a timer (source, threshold, reset mode)
  *         but does not enable : call HAL_HRTIM_ExternalEventCounterEnable afterwards
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  EventCounter external event Counter A for which timer event must be configured
  *         This parameter can be one of the following values:
  *           @arg HRTIM_EVENTCOUNTER_A
  * @param  pTimerExternalEventCfg: pointer to the timer external event configuration structure
  * @note This function must be called before starting the timer
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_ExtEventCounterConfig(HRTIM_HandleTypeDef * hhrtim,
                                                      uint32_t TimerIdx,
                                                      uint32_t EventCounter,
                                                      const HRTIM_ExternalEventCfgTypeDef* pTimerExternalEventCfg)
{
  uint32_t hrtim_eefr3;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_TIMEEVENT(EventCounter));
  assert_param(IS_HRTIM_TIMEEVENT_RESETMODE(pTimerExternalEventCfg->ResetMode));
  assert_param(IS_HRTIM_TIMEEVENT_COUNTER(pTimerExternalEventCfg->Counter));
  assert_param(IS_HRTIM_EVENT(pTimerExternalEventCfg->Source));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  if ((EventCounter & HRTIM_EVENTCOUNTER_A) != 0U)
  {
   if (pTimerExternalEventCfg->Source == HRTIM_EVENT_NONE)
   { /* reset External EventCounter A */
     WRITE_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR3, 0xFFFF0000U);
   }
   else
   {
     /* Set timer External EventCounter A configuration */
     hrtim_eefr3  = (pTimerExternalEventCfg->ResetMode) << HRTIM_EEFR3_EEVARSTM_Pos;
     hrtim_eefr3 |= ((pTimerExternalEventCfg->Source - 1U)) << HRTIM_EEFR3_EEVASEL_Pos;
     hrtim_eefr3 |= (pTimerExternalEventCfg->Counter) << HRTIM_EEFR3_EEVACNT_Pos;
     /* do not enable, use HAL_HRTIM_TimerExternalEventEnable function */

     MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR3, (HRTIM_EEFR3_EEVARSTM | HRTIM_EEFR3_EEVASEL | HRTIM_EEFR3_EEVACNT) , hrtim_eefr3 );
   }
  }
  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Enable the external event Counter A or B of a timer
  * @param  hhrtim: pointer to HAL HRTIM handle
  * @param  TimerIdx: Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  EventCounter external Event Counter A for which timer event must be configured
  *         This parameter can be a one of the following values:
  *           @arg HRTIM_EVENTCOUNTER_A
  * @note This function must be called before starting the timer
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_ExtEventCounterEnable(HRTIM_HandleTypeDef * hhrtim,
                                                      uint32_t TimerIdx,
                                                      uint32_t EventCounter)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_TIMEEVENT(EventCounter));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  if ((EventCounter & HRTIM_EVENTCOUNTER_A) != 0U)
  {
    SET_BIT(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR3, HRTIM_EEFR3_EEVACE);
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Disable the external event Counter A or B of a timer
  * @param  hhrtim: pointer to HAL HRTIM handle
  * @param  TimerIdx: Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  EventCounter external Event Counter A for which timer event must be configured
  *         This parameter can be a one of the following values:
  *           @arg HRTIM_EVENTCOUNTER_A
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_ExtEventCounterDisable(HRTIM_HandleTypeDef * hhrtim,
                                                      uint32_t TimerIdx,
                                                      uint32_t EventCounter)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_TIMEEVENT(EventCounter));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  if ((EventCounter & HRTIM_EVENTCOUNTER_A) != 0U)
  {
    CLEAR_BIT(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR3, HRTIM_EEFR3_EEVACE);
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Reset the external event Counter A or B of a timer
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx: Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  EventCounter external Event Counter A for which timer event must be configured
  *         This parameter can be a one of the following values:
  *           @arg HRTIM_EVENTCOUNTER_A
  * @note This function must be called before starting the timer
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_ExtEventCounterReset(HRTIM_HandleTypeDef * hhrtim,
                                                      uint32_t TimerIdx,
                                                      uint32_t EventCounter)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_TIMEEVENT(EventCounter));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  if ((EventCounter & HRTIM_EVENTCOUNTER_A) != 0U)
  {
    SET_BIT(hhrtim->Instance->sTimerxRegs[TimerIdx].EEFxR3, HRTIM_EEFR3_EEVACRES);
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the dead-time insertion feature for a timer
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx: Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  pDeadTimeCfg pointer to the deadtime insertion configuration structure
  * @retval HAL Status
  * @note This function must be called before starting the timer
  */
HAL_StatusTypeDef HAL_HRTIM_DeadTimeConfig(HRTIM_HandleTypeDef * hhrtim,
                                           uint32_t TimerIdx,
                                           const HRTIM_DeadTimeCfgTypeDef* pDeadTimeCfg)
{
  uint32_t hrtim_dtr;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_TIMDEADTIME_PRESCALERRATIO(pDeadTimeCfg->Prescaler));
  assert_param(IS_HRTIM_TIMDEADTIME_RISINGSIGN(pDeadTimeCfg->RisingSign));
  assert_param(IS_HRTIM_TIMDEADTIME_RISINGLOCK(pDeadTimeCfg->RisingLock));
  assert_param(IS_HRTIM_TIMDEADTIME_RISINGSIGNLOCK(pDeadTimeCfg->RisingSignLock));
  assert_param(IS_HRTIM_TIMDEADTIME_FALLINGSIGN(pDeadTimeCfg->FallingSign));
  assert_param(IS_HRTIM_TIMDEADTIME_FALLINGLOCK(pDeadTimeCfg->FallingLock));
  assert_param(IS_HRTIM_TIMDEADTIME_FALLINGSIGNLOCK(pDeadTimeCfg->FallingSignLock));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Set timer deadtime configuration */
  hrtim_dtr  = (pDeadTimeCfg->Prescaler & HRTIM_DTR_DTPRSC);
  hrtim_dtr |= (pDeadTimeCfg->RisingValue & HRTIM_DTR_DTR);
  hrtim_dtr |= (pDeadTimeCfg->RisingSign & HRTIM_DTR_SDTR);
  hrtim_dtr |= (pDeadTimeCfg->RisingSignLock & HRTIM_DTR_DTRSLK);
  hrtim_dtr |= (pDeadTimeCfg->RisingLock & HRTIM_DTR_DTRLK);
  hrtim_dtr |= ((pDeadTimeCfg->FallingValue << 16U) & HRTIM_DTR_DTF);
  hrtim_dtr |= (pDeadTimeCfg->FallingSign & HRTIM_DTR_SDTF);
  hrtim_dtr |= (pDeadTimeCfg->FallingSignLock & HRTIM_DTR_DTFSLK);
  hrtim_dtr |= (pDeadTimeCfg->FallingLock & HRTIM_DTR_DTFLK);

  /* Update the HRTIM registers */
  MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].DTxR, (
                 HRTIM_DTR_DTR | HRTIM_DTR_SDTR | HRTIM_DTR_DTPRSC |
                 HRTIM_DTR_DTRSLK | HRTIM_DTR_DTRLK | HRTIM_DTR_DTF |
                 HRTIM_DTR_SDTF | HRTIM_DTR_DTFSLK | HRTIM_DTR_DTFLK), hrtim_dtr);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the chopper mode feature for a timer
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx: Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  pChopperModeCfg pointer to the chopper mode configuration structure
  * @retval HAL Status
  * @note This function must be called before configuring the timer output(s)
  */
HAL_StatusTypeDef HAL_HRTIM_ChopperModeConfig(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx,
                                              const HRTIM_ChopperModeCfgTypeDef* pChopperModeCfg)
{
  uint32_t hrtim_chpr;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_CHOPPER_PRESCALERRATIO(pChopperModeCfg->CarrierFreq));
  assert_param(IS_HRTIM_CHOPPER_DUTYCYCLE(pChopperModeCfg->DutyCycle));
  assert_param(IS_HRTIM_CHOPPER_PULSEWIDTH(pChopperModeCfg->StartPulse));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Set timer choppe mode configuration */
  hrtim_chpr  = (pChopperModeCfg->CarrierFreq & HRTIM_CHPR_CARFRQ);
  hrtim_chpr |= (pChopperModeCfg->DutyCycle & HRTIM_CHPR_CARDTY);
  hrtim_chpr |= (pChopperModeCfg->StartPulse & HRTIM_CHPR_STRPW);

  /* Update the HRTIM registers */
  MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].CHPxR,
             (HRTIM_CHPR_CARFRQ | HRTIM_CHPR_CARDTY | HRTIM_CHPR_STRPW),
             hrtim_chpr);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the burst DMA controller for a timer
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx: Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  RegistersToUpdate registers to be written by DMA
  *         This parameter can be any combination of the following values:
  *           @arg HRTIM_BURSTDMA_CR: HRTIM_MCR or HRTIM_TIMxCR
  *           @arg HRTIM_BURSTDMA_ICR: HRTIM_MICR or HRTIM_TIMxICR
  *           @arg HRTIM_BURSTDMA_DIER: HRTIM_MDIER or HRTIM_TIMxDIER
  *           @arg HRTIM_BURSTDMA_CNT: HRTIM_MCNT or HRTIM_TIMxCNT
  *           @arg HRTIM_BURSTDMA_PER: HRTIM_MPER or HRTIM_TIMxPER
  *           @arg HRTIM_BURSTDMA_REP: HRTIM_MREP or HRTIM_TIMxREP
  *           @arg HRTIM_BURSTDMA_CMP1: HRTIM_MCMP1 or HRTIM_TIMxCMP1
  *           @arg HRTIM_BURSTDMA_CMP2: HRTIM_MCMP2 or HRTIM_TIMxCMP2
  *           @arg HRTIM_BURSTDMA_CMP3: HRTIM_MCMP3 or HRTIM_TIMxCMP3
  *           @arg HRTIM_BURSTDMA_CMP4: HRTIM_MCMP4 or HRTIM_TIMxCMP4
  *           @arg HRTIM_BURSTDMA_DTR: HRTIM_TIMxDTR
  *           @arg HRTIM_BURSTDMA_SET1R: HRTIM_TIMxSET1R
  *           @arg HRTIM_BURSTDMA_RST1R: HRTIM_TIMxRST1R
  *           @arg HRTIM_BURSTDMA_SET2R: HRTIM_TIMxSET2R
  *           @arg HRTIM_BURSTDMA_RST2R: HRTIM_TIMxRST2R
  *           @arg HRTIM_BURSTDMA_EEFR1: HRTIM_TIMxEEFR1
  *           @arg HRTIM_BURSTDMA_EEFR2: HRTIM_TIMxEEFR2
  *           @arg HRTIM_BURSTDMA_RSTR: HRTIM_TIMxRSTR
  *           @arg HRTIM_BURSTDMA_CHPR: HRTIM_TIMxCHPR
  *           @arg HRTIM_BURSTDMA_OUTR: HRTIM_TIMxOUTR
  *           @arg HRTIM_BURSTDMA_FLTR: HRTIM_TIMxFLTR
  * @retval HAL Status
  * @note This function must be called before starting the timer
  */
HAL_StatusTypeDef HAL_HRTIM_BurstDMAConfig(HRTIM_HandleTypeDef * hhrtim,
                                           uint32_t TimerIdx,
                                           uint32_t RegistersToUpdate)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMER_BURSTDMA(TimerIdx, RegistersToUpdate));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Set the burst DMA timer update register */
  switch (TimerIdx)
  {
  case HRTIM_TIMERINDEX_TIMER_A:
    {
      hhrtim->Instance->sCommonRegs.BDTAUPR = RegistersToUpdate;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_B:
    {
      hhrtim->Instance->sCommonRegs.BDTBUPR = RegistersToUpdate;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_C:
    {
      hhrtim->Instance->sCommonRegs.BDTCUPR = RegistersToUpdate;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_D:
    {
      hhrtim->Instance->sCommonRegs.BDTDUPR = RegistersToUpdate;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_E:
    {
      hhrtim->Instance->sCommonRegs.BDTEUPR = RegistersToUpdate;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_F:
    {
      hhrtim->Instance->sCommonRegs.BDTFUPR = RegistersToUpdate;
      break;
    }

  case HRTIM_TIMERINDEX_MASTER:
    {
      hhrtim->Instance->sCommonRegs.BDMUPR = RegistersToUpdate;
      break;
    }

  default:
   {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the compare unit of a timer operating in waveform mode
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx: Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CompareUnit Compare unit to configure
  *         This parameter can be one of the following values:
  *           @arg HRTIM_COMPAREUNIT_1: Compare unit 1
  *           @arg HRTIM_COMPAREUNIT_2: Compare unit 2
  *           @arg HRTIM_COMPAREUNIT_3: Compare unit 3
  *           @arg HRTIM_COMPAREUNIT_4: Compare unit 4
  * @param  pCompareCfg pointer to the compare unit configuration structure
  * @note When auto delayed mode is required for compare unit 2 or compare unit 4,
  *       application has to configure separately the capture unit. Capture unit
  *       to configure in that case depends on the compare unit auto delayed mode
  *       is applied to (see below):
  *         Auto delayed on output compare 2: capture unit 1 must be configured
  *         Auto delayed on output compare 4: capture unit 2 must be configured
  * @retval HAL Status
  * @note This function must be called before starting the timer
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformCompareConfig(HRTIM_HandleTypeDef * hhrtim,
                                                  uint32_t TimerIdx,
                                                  uint32_t CompareUnit,
                                                  const HRTIM_CompareCfgTypeDef* pCompareCfg)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMERINDEX(TimerIdx));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Configure the compare unit */
  if (TimerIdx == HRTIM_TIMERINDEX_MASTER)
  {
    switch (CompareUnit)
    {
      case HRTIM_COMPAREUNIT_1:
        {
        hhrtim->Instance->sMasterRegs.MCMP1R = pCompareCfg->CompareValue;
        break;
        }

      case HRTIM_COMPAREUNIT_2:
        {
        hhrtim->Instance->sMasterRegs.MCMP2R = pCompareCfg->CompareValue;
        break;
        }

      case HRTIM_COMPAREUNIT_3:
        {
        hhrtim->Instance->sMasterRegs.MCMP3R = pCompareCfg->CompareValue;
        break;
        }

      case HRTIM_COMPAREUNIT_4:
        {
        hhrtim->Instance->sMasterRegs.MCMP4R = pCompareCfg->CompareValue;
        break;
        }

      default:
        {
        hhrtim->State = HAL_HRTIM_STATE_ERROR;

        /* Process Unlocked */
        __HAL_UNLOCK(hhrtim);

        break;
        }
    }

    if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
    {
     return HAL_ERROR;
    }

  }
  else
  {
    switch (CompareUnit)
    {
    case HRTIM_COMPAREUNIT_1:
      {
        /* Set the compare value */
        hhrtim->Instance->sTimerxRegs[TimerIdx].CMP1xR = pCompareCfg->CompareValue;
        break;
      }

    case HRTIM_COMPAREUNIT_2:
      {
        /* Check parameters */
        assert_param(IS_HRTIM_COMPAREUNIT_AUTODELAYEDMODE(CompareUnit, pCompareCfg->AutoDelayedMode));

        /* Set the compare value */
        hhrtim->Instance->sTimerxRegs[TimerIdx].CMP2xR = pCompareCfg->CompareValue;

        if (pCompareCfg->AutoDelayedMode != HRTIM_AUTODELAYEDMODE_REGULAR)
        {
          /* Configure auto-delayed mode */
          /* DELCMP2 bitfield must be reset when reprogrammed from one value */
          /* to the other to reinitialize properly the auto-delayed mechanism */
          hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR &= ~HRTIM_TIMCR_DELCMP2;
          hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR |= pCompareCfg->AutoDelayedMode;

          /* Set the compare value for timeout compare unit (if any) */
          if (pCompareCfg->AutoDelayedMode == HRTIM_AUTODELAYEDMODE_AUTODELAYED_TIMEOUTCMP1)
          {
            hhrtim->Instance->sTimerxRegs[TimerIdx].CMP1xR = pCompareCfg->AutoDelayedTimeout;
          }
          else if (pCompareCfg->AutoDelayedMode == HRTIM_AUTODELAYEDMODE_AUTODELAYED_TIMEOUTCMP3)
          {
            hhrtim->Instance->sTimerxRegs[TimerIdx].CMP3xR = pCompareCfg->AutoDelayedTimeout;
          }
          else
          {
            /* nothing to do */
          }
        }
        else
        {
          /* Clear HRTIM_TIMxCR.DELCMP2 bitfield */
          MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR, HRTIM_TIMCR_DELCMP2, 0U);
        }
         break;
      }

    case HRTIM_COMPAREUNIT_3:
      {
        /* Set the compare value */
        hhrtim->Instance->sTimerxRegs[TimerIdx].CMP3xR = pCompareCfg->CompareValue;
        break;
      }

    case HRTIM_COMPAREUNIT_4:
      {
        /* Check parameters */
        assert_param(IS_HRTIM_COMPAREUNIT_AUTODELAYEDMODE(CompareUnit, pCompareCfg->AutoDelayedMode));

        /* Set the compare value */
        hhrtim->Instance->sTimerxRegs[TimerIdx].CMP4xR = pCompareCfg->CompareValue;

        if (pCompareCfg->AutoDelayedMode != HRTIM_AUTODELAYEDMODE_REGULAR)
        {
          /* Configure auto-delayed mode */
          /* DELCMP4 bitfield must be reset when reprogrammed from one value */
          /* to the other to reinitialize properly the auto-delayed mechanism */
          hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR &= ~HRTIM_TIMCR_DELCMP4;
          hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR |= (pCompareCfg->AutoDelayedMode << 2U);

          /* Set the compare value for timeout compare unit (if any) */
          if (pCompareCfg->AutoDelayedMode == HRTIM_AUTODELAYEDMODE_AUTODELAYED_TIMEOUTCMP1)
          {
            hhrtim->Instance->sTimerxRegs[TimerIdx].CMP1xR = pCompareCfg->AutoDelayedTimeout;
          }
          else if (pCompareCfg->AutoDelayedMode == HRTIM_AUTODELAYEDMODE_AUTODELAYED_TIMEOUTCMP3)
          {
            hhrtim->Instance->sTimerxRegs[TimerIdx].CMP3xR = pCompareCfg->AutoDelayedTimeout;
          }
          else
          {
            /* nothing to do */
          }
        }
        else
        {
          /* Clear HRTIM_TIMxCR.DELCMP4 bitfield */
          MODIFY_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR, HRTIM_TIMCR_DELCMP4, 0U);
        }
         break;
      }

  default:
     {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
     }
   }

   if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
   {
     return HAL_ERROR;
   }

  }
  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the capture unit of a timer operating in waveform mode
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx: Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CaptureUnit Capture unit to configure
  *         This parameter can be one of the following values:
  *           @arg HRTIM_CAPTUREUNIT_1: Capture unit 1
  *           @arg HRTIM_CAPTUREUNIT_2: Capture unit 2
  * @param  pCaptureCfg pointer to the compare unit configuration structure
  * @retval HAL Status
  * @note This function must be called before starting the timer
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformCaptureConfig(HRTIM_HandleTypeDef * hhrtim,
                                                  uint32_t TimerIdx,
                                                  uint32_t CaptureUnit,
                                                  const HRTIM_CaptureCfgTypeDef* pCaptureCfg)
{
  uint32_t Trigger;
  uint32_t TimerF_Trigger = (uint32_t)(pCaptureCfg->Trigger >> 32);

  /* Check parameters */
  assert_param(IS_HRTIM_TIMER_CAPTURETRIGGER(TimerIdx, (uint32_t)(pCaptureCfg->Trigger)));
  assert_param(IS_HRTIM_TIMER_CAPTUREFTRIGGER(TimerIdx, TimerF_Trigger));
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* TimerF_Trigger is valid for setting other Timers than Timer F */
  if (TimerIdx == HRTIM_TIMERINDEX_TIMER_A)
    { Trigger = ((uint32_t)(pCaptureCfg->Trigger) & 0xFFFF0FFFU) | ( (TimerF_Trigger ) << HRTIM_CPT1CR_TA1SET_Pos ); }
  else if (TimerIdx == HRTIM_TIMERINDEX_TIMER_B)
    { Trigger = ((uint32_t)(pCaptureCfg->Trigger) & 0xFFF0FFFFU) | ( (TimerF_Trigger ) << HRTIM_CPT1CR_TB1SET_Pos ); }
  else if (TimerIdx == HRTIM_TIMERINDEX_TIMER_C)
    { Trigger = ((uint32_t)(pCaptureCfg->Trigger) & 0xFF0FFFFFU) | ( (TimerF_Trigger ) << HRTIM_CPT1CR_TC1SET_Pos ); }
  else if (TimerIdx == HRTIM_TIMERINDEX_TIMER_D)
    { Trigger = ((uint32_t)(pCaptureCfg->Trigger) & 0xF0FFFFFFU) | ( (TimerF_Trigger ) << HRTIM_CPT1CR_TD1SET_Pos ); }
  else if (TimerIdx == HRTIM_TIMERINDEX_TIMER_E)
    { Trigger = ((uint32_t)(pCaptureCfg->Trigger) & 0x0FFFFFFFU) | ( (TimerF_Trigger ) << HRTIM_CPT1CR_TE1SET_Pos ); }
  else
    { Trigger = ((uint32_t)(pCaptureCfg->Trigger) & 0xFFFFFFFFU); }
  /* for setting source capture on Timer F, use Trigger only (all bits are valid then) */

  /* Configure the capture unit */

  switch (CaptureUnit)
  {
  case HRTIM_CAPTUREUNIT_1:
    {
      WRITE_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xCR, Trigger);
      break;
    }

  case HRTIM_CAPTUREUNIT_2:
    {
      WRITE_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xCR, Trigger);
      break;
    }

  default:
   {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }


  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Configure the output of a timer operating in waveform mode
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx: Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  Output Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @param  pOutputCfg pointer to the timer output configuration structure
  * @retval HAL Status
  * @note This function must be called before configuring the timer and after
  *       configuring the deadtime insertion feature (if required).
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformOutputConfig(HRTIM_HandleTypeDef * hhrtim,
                                                uint32_t TimerIdx,
                                                uint32_t Output,
                                                const HRTIM_OutputCfgTypeDef * pOutputCfg)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, Output));
  assert_param(IS_HRTIM_OUTPUTPOLARITY(pOutputCfg->Polarity));
  assert_param(IS_HRTIM_OUTPUTIDLELEVEL(pOutputCfg->IdleLevel));
  assert_param(IS_HRTIM_OUTPUTIDLEMODE(pOutputCfg->IdleMode));
  assert_param(IS_HRTIM_OUTPUTFAULTLEVEL(pOutputCfg->FaultLevel));
  assert_param(IS_HRTIM_OUTPUTCHOPPERMODE(pOutputCfg->ChopperModeEnable));
  assert_param(IS_HRTIM_OUTPUTBURSTMODEENTRY(pOutputCfg->BurstModeEntryDelayed));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Configure the timer output */
  HRTIM_OutputConfig(hhrtim,
                     TimerIdx,
                     Output,
                     pOutputCfg);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Force the timer output to its active or inactive state
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx: Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  Output Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @param OutputLevel indicates whether the output is forced to its active or inactive level
  *        This parameter can be one of the following values:
  *          @arg HRTIM_OUTPUTLEVEL_ACTIVE: output is forced to its active level
  *          @arg HRTIM_OUTPUTLEVEL_INACTIVE: output is forced to its inactive level
  * @retval HAL Status
  * @note The 'software set/reset trigger' bit in the output set/reset registers
  *       is automatically reset by hardware
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformSetOutputLevel(HRTIM_HandleTypeDef * hhrtim,
                                                   uint32_t TimerIdx,
                                                   uint32_t Output,
                                                   uint32_t OutputLevel)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, Output));
  assert_param(IS_HRTIM_OUTPUTLEVEL(OutputLevel));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Force timer output level */
  switch (Output)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      if (OutputLevel == HRTIM_OUTPUTLEVEL_ACTIVE)
      {
        /* Force output to its active state */
        SET_BIT(hhrtim->Instance->sTimerxRegs[TimerIdx].SETx1R,HRTIM_SET1R_SST);
      }
      else
      {
        /* Force output to its inactive state */
        SET_BIT(hhrtim->Instance->sTimerxRegs[TimerIdx].RSTx1R, HRTIM_RST1R_SRT);
      }
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      if (OutputLevel == HRTIM_OUTPUTLEVEL_ACTIVE)
      {
        /* Force output to its active state */
        SET_BIT(hhrtim->Instance->sTimerxRegs[TimerIdx].SETx2R, HRTIM_SET2R_SST);
      }
      else
      {
        /* Force output to its inactive state */
        SET_BIT(hhrtim->Instance->sTimerxRegs[TimerIdx].RSTx2R, HRTIM_RST2R_SRT);
      }
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

      break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Enable the generation of the waveform signal on the designated output(s)
  *         Outputs can be combined (ORed) to allow for simultaneous output enabling.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  OutputsToStart Timer output(s) to enable
  *         This parameter can be any combination of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformOutputStart(HRTIM_HandleTypeDef * hhrtim,
                                                uint32_t OutputsToStart)
{
  CLEAR_BIT(hhrtim->Instance->sCommonRegs.OPT,  HRTIM_OPT_DLLBYPASS);
  
   /* Check the parameters */
  assert_param(IS_HRTIM_OUTPUT(OutputsToStart));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable the HRTIM outputs */
  hhrtim->Instance->sCommonRegs.OENR |= (OutputsToStart);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Disable the generation of the waveform signal on the designated output(s)
  *         Outputs can be combined (ORed) to allow for simultaneous output disabling.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  OutputsToStop Timer output(s) to disable
  *         This parameter can be any combination of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformOutputStop(HRTIM_HandleTypeDef * hhrtim,
                                               uint32_t OutputsToStop)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_OUTPUT(OutputsToStop));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable the HRTIM outputs */
  hhrtim->Instance->sCommonRegs.ODISR |= (OutputsToStop);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Start the counter of the designated timer(s) operating in waveform mode
  *         Timers can be combined (ORed) to allow for simultaneous counter start.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Timers Timer counter(s) to start
  *         This parameter can be any combination of the following values:
  *           @arg HRTIM_TIMERID_MASTER
  *           @arg HRTIM_TIMERID_TIMER_A
  *           @arg HRTIM_TIMERID_TIMER_B
  *           @arg HRTIM_TIMERID_TIMER_C
  *           @arg HRTIM_TIMERID_TIMER_D
  *           @arg HRTIM_TIMERID_TIMER_E
  *           @arg HRTIM_TIMERID_TIMER_F
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformCountStart(HRTIM_HandleTypeDef * hhrtim,
                                                 uint32_t Timers)
{
  /* Check the parameters */
  assert_param(IS_HRTIM_TIMERID(Timers));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable timer(s) counter */
  hhrtim->Instance->sMasterRegs.MCR |= (Timers);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Stop the counter of the designated timer(s) operating in waveform mode
  *         Timers can be combined (ORed) to allow for simultaneous counter stop.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Timers Timer counter(s) to stop
  *         This parameter can be any combination of the following values:
  *           @arg HRTIM_TIMERID_MASTER
  *           @arg HRTIM_TIMERID_TIMER_A
  *           @arg HRTIM_TIMERID_TIMER_B
  *           @arg HRTIM_TIMERID_TIMER_C
  *           @arg HRTIM_TIMERID_TIMER_D
  *           @arg HRTIM_TIMERID_TIMER_E
  *           @arg HRTIM_TIMERID_TIMER_F
  * @retval HAL Status
  * @note The counter of a timer is stopped only if all timer outputs are disabled
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformCountStop(HRTIM_HandleTypeDef * hhrtim,
                                                uint32_t Timers)
{
  /* Check the parameters */
  assert_param(IS_HRTIM_TIMERID(Timers));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Disable timer(s) counter */
  hhrtim->Instance->sMasterRegs.MCR &= ~(Timers);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Start the counter of the designated timer(s) operating in waveform mode
  *         Timers can be combined (ORed) to allow for simultaneous counter start.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Timers Timer counter(s) to start
  *         This parameter can be any combination of the following values:
  *           @arg HRTIM_TIMERID_MASTER
  *           @arg HRTIM_TIMERID_TIMER_A
  *           @arg HRTIM_TIMERID_TIMER_B
  *           @arg HRTIM_TIMERID_TIMER_C
  *           @arg HRTIM_TIMERID_TIMER_D
  *           @arg HRTIM_TIMERID_TIMER_E
  *           @arg HRTIM_TIMERID_TIMER_F
  * @note HRTIM interrupts (e.g. faults interrupts) and interrupts related
  *       to the timers to start are enabled within this function.
  *       Interrupts to enable are selected through HAL_HRTIM_WaveformTimerConfig
  *       function.
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformCountStart_IT(HRTIM_HandleTypeDef * hhrtim,
                                                    uint32_t Timers)
{
  uint8_t timer_idx;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMERID(Timers));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable HRTIM interrupts (if required) */
  __HAL_HRTIM_ENABLE_IT(hhrtim, hhrtim->Init.HRTIMInterruptResquests);

  /* Enable master timer related interrupts (if required) */
  if ((Timers & HRTIM_TIMERID_MASTER) != 0U)
  {
    __HAL_HRTIM_MASTER_ENABLE_IT(hhrtim,
                                 hhrtim->TimerParam[HRTIM_TIMERINDEX_MASTER].InterruptRequests);
  }

  /* Enable timing unit related interrupts (if required) */
  for (timer_idx = HRTIM_TIMERINDEX_TIMER_A ;
       timer_idx < HRTIM_TIMERINDEX_MASTER ;
       timer_idx++)
  {
    if ((Timers & TimerIdxToTimerId[timer_idx]) != 0U)
    {
      __HAL_HRTIM_TIMER_ENABLE_IT(hhrtim,
                                  timer_idx,
                                  hhrtim->TimerParam[timer_idx].InterruptRequests);
    }
  }

  /* Enable timer(s) counter */
  hhrtim->Instance->sMasterRegs.MCR |= (Timers);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;}

/**
  * @brief  Stop the counter of the designated timer(s) operating in waveform mode
  *         Timers can be combined (ORed) to allow for simultaneous counter stop.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Timers Timer counter(s) to stop
  *         This parameter can be any combination of the following values:
  *           @arg HRTIM_TIMERID_MASTER
  *           @arg HRTIM_TIMERID_TIMER_A
  *           @arg HRTIM_TIMERID_TIMER_B
  *           @arg HRTIM_TIMERID_TIMER_C
  *           @arg HRTIM_TIMERID_TIMER_D
  *           @arg HRTIM_TIMERID_TIMER_E
  *           @arg HRTIM_TIMERID_TIMER_F
  * @retval HAL Status
  * @note The counter of a timer is stopped only if all timer outputs are disabled
  * @note All enabled timer related interrupts are disabled.
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformCountStop_IT(HRTIM_HandleTypeDef * hhrtim,
                                                   uint32_t Timers)
{
  /* ++ WA */
  __IO uint32_t delai = (uint32_t)(0x17FU);
  /* -- WA */

  uint8_t timer_idx;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMERID(Timers));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Disable HRTIM interrupts (if required) */
  __HAL_HRTIM_DISABLE_IT(hhrtim, hhrtim->Init.HRTIMInterruptResquests);

  /* Disable master timer related interrupts (if required) */
  if ((Timers & HRTIM_TIMERID_MASTER) != 0U)
  {
    /* Interrupts enable flag must be cleared one by one */
    __HAL_HRTIM_MASTER_DISABLE_IT(hhrtim, hhrtim->TimerParam[HRTIM_TIMERINDEX_MASTER].InterruptRequests);
  }

  /* Disable timing unit related interrupts (if required) */
  for (timer_idx = HRTIM_TIMERINDEX_TIMER_A ;
       timer_idx < HRTIM_TIMERINDEX_MASTER ;
       timer_idx++)
  {
    if ((Timers & TimerIdxToTimerId[timer_idx]) != 0U)
    {
      __HAL_HRTIM_TIMER_DISABLE_IT(hhrtim, timer_idx, hhrtim->TimerParam[timer_idx].InterruptRequests);
    }
  }

  /* ++ WA */
  do { delai--; } while (delai != 0U);
  /* -- WA */

  /* Disable timer(s) counter */
  hhrtim->Instance->sMasterRegs.MCR &= ~(Timers);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Start the counter of the designated timer(s) operating in waveform mode
  *         Timers can be combined (ORed) to allow for simultaneous counter start.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Timers Timer counter(s) to start
  *         This parameter can be any combination of the following values:
  *           @arg HRTIM_TIMERID_MASTER
  *           @arg HRTIM_TIMERID_TIMER_A
  *           @arg HRTIM_TIMERID_TIMER_B
  *           @arg HRTIM_TIMERID_TIMER_C
  *           @arg HRTIM_TIMERID_TIMER_D
  *           @arg HRTIM_TIMERID_TIMER_E
  *           @arg HRTIM_TIMERID_TIMER_F
  * @retval HAL Status
  * @note This function enables the dma request(s) mentioned in the timer
  *       configuration data structure for every timers to start.
  * @note The source memory address, the destination memory address and the
  *       size of each DMA transfer are specified at timer configuration time
  *       (see HAL_HRTIM_WaveformTimerConfig)
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformCountStart_DMA(HRTIM_HandleTypeDef * hhrtim,
                                                     uint32_t Timers)
{
  uint8_t timer_idx;
  DMA_HandleTypeDef * hdma;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMERID(Timers));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  if (((Timers & HRTIM_TIMERID_MASTER) != (uint32_t)RESET) &&
      (hhrtim->TimerParam[HRTIM_TIMERINDEX_MASTER].DMARequests != 0U))
  {
      /* Set the DMA error callback */
      hhrtim->hdmaMaster->XferErrorCallback = HRTIM_DMAError ;

      /* Set the DMA transfer completed callback */
      hhrtim->hdmaMaster->XferCpltCallback = HRTIM_DMAMasterCplt;

      /* Enable the DMA channel */
      if (HAL_DMA_Start_IT(hhrtim->hdmaMaster,
                       hhrtim->TimerParam[HRTIM_TIMERINDEX_MASTER].DMASrcAddress,
                       hhrtim->TimerParam[HRTIM_TIMERINDEX_MASTER].DMADstAddress,
                       hhrtim->TimerParam[HRTIM_TIMERINDEX_MASTER].DMASize) != HAL_OK)
    {
            hhrtim->State = HAL_HRTIM_STATE_ERROR;

            /* Process Unlocked */
            __HAL_UNLOCK(hhrtim);

            return HAL_ERROR;
        }

      /* Enable the timer DMA request */
      __HAL_HRTIM_MASTER_ENABLE_DMA(hhrtim,
                                   hhrtim->TimerParam[HRTIM_TIMERINDEX_MASTER].DMARequests);
  }

  for (timer_idx = HRTIM_TIMERINDEX_TIMER_A ;
       timer_idx < HRTIM_TIMERINDEX_MASTER ;
       timer_idx++)
  {
    if (((Timers & TimerIdxToTimerId[timer_idx]) != (uint32_t)RESET) &&
         (hhrtim->TimerParam[timer_idx].DMARequests != 0U))
    {
      /* Get the timer DMA handler */
      hdma = HRTIM_GetDMAHandleFromTimerIdx(hhrtim, timer_idx);

      if (hdma == NULL)
      {
        hhrtim->State = HAL_HRTIM_STATE_ERROR;

        /* Process Unlocked */
        __HAL_UNLOCK(hhrtim);

        return HAL_ERROR;
      }

       /* Set the DMA error callback */
      hdma->XferErrorCallback = HRTIM_DMAError ;

      /* Set the DMA transfer completed callback */
      hdma->XferCpltCallback = HRTIM_DMATimerxCplt;

      /* Enable the DMA channel */
      if (HAL_DMA_Start_IT(hdma,
                       hhrtim->TimerParam[timer_idx].DMASrcAddress,
                       hhrtim->TimerParam[timer_idx].DMADstAddress,
                       hhrtim->TimerParam[timer_idx].DMASize) != HAL_OK)
    {
              hhrtim->State = HAL_HRTIM_STATE_ERROR;

              /* Process Unlocked */
              __HAL_UNLOCK(hhrtim);

              return HAL_ERROR;
        }

      /* Enable the timer DMA request */
      __HAL_HRTIM_TIMER_ENABLE_DMA(hhrtim,
                                   timer_idx,
                                   hhrtim->TimerParam[timer_idx].DMARequests);
    }
  }

  /* Enable the timer counter */
  __HAL_HRTIM_ENABLE(hhrtim, Timers);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Stop the counter of the designated timer(s) operating in waveform mode
  *         Timers can be combined (ORed) to allow for simultaneous counter stop.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Timers Timer counter(s) to stop
  *         This parameter can be any combination of the following values:
  *           @arg HRTIM_TIMERID_MASTER
  *           @arg HRTIM_TIMERID_TIMER_A
  *           @arg HRTIM_TIMERID_TIMER_B
  *           @arg HRTIM_TIMERID_TIMER_C
  *           @arg HRTIM_TIMERID_TIMER_D
  *           @arg HRTIM_TIMERID_TIMER_E
  *           @arg HRTIM_TIMERID_TIMER_F
  * @retval HAL Status
  * @note  The counter of a timer is stopped only if all timer outputs are disabled
  * @note  All enabled timer related DMA requests are disabled.
  */
HAL_StatusTypeDef HAL_HRTIM_WaveformCountStop_DMA(HRTIM_HandleTypeDef * hhrtim,
                                                    uint32_t Timers)
{
  uint8_t timer_idx;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMERID(Timers));

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  if (((Timers & HRTIM_TIMERID_MASTER) != 0U) &&
      (hhrtim->TimerParam[HRTIM_TIMERINDEX_MASTER].DMARequests != 0U))
  {
    /* Disable the DMA */
    if (HAL_DMA_Abort(hhrtim->hdmaMaster) != HAL_OK)
    {
          hhrtim->State = HAL_HRTIM_STATE_ERROR;
    }
    else
    {
          hhrtim->State = HAL_HRTIM_STATE_READY;
          /* Disable the DMA request(s) */
          __HAL_HRTIM_MASTER_DISABLE_DMA(hhrtim,
                                         hhrtim->TimerParam[HRTIM_TIMERINDEX_MASTER].DMARequests);
    }
  }

  for (timer_idx = HRTIM_TIMERINDEX_TIMER_A ;
       timer_idx < HRTIM_TIMERINDEX_MASTER ;
       timer_idx++)
  {
    if (((Timers & TimerIdxToTimerId[timer_idx]) != 0U) &&
        (hhrtim->TimerParam[timer_idx].DMARequests != 0U))
    {
      /* Get the timer DMA handler */
      /* Disable the DMA */
      if (HAL_DMA_Abort(HRTIM_GetDMAHandleFromTimerIdx(hhrtim, timer_idx)) != HAL_OK)
      {
        hhrtim->State = HAL_HRTIM_STATE_ERROR;
      }
      else
      {
        hhrtim->State = HAL_HRTIM_STATE_READY;

        /* Disable the DMA request(s) */
        __HAL_HRTIM_TIMER_DISABLE_DMA(hhrtim,
                                      timer_idx,
                                      hhrtim->TimerParam[timer_idx].DMARequests);
      }
    }
  }

  /* Disable the timer counter */
  __HAL_HRTIM_DISABLE(hhrtim, Timers);

  if (hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
      return HAL_ERROR;
  }
  else
  {
      return HAL_OK;
  }
}

/**
  * @brief  Enable or disables the HRTIM burst mode controller.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Enable Burst mode controller enabling
  *         This parameter can be one of the following values:
  *           @arg HRTIM_BURSTMODECTL_ENABLED: Burst mode enabled
  *           @arg HRTIM_BURSTMODECTL_DISABLED: Burst mode disabled
  * @retval HAL Status
  * @note This function must be called after starting the timer(s)
  */
HAL_StatusTypeDef HAL_HRTIM_BurstModeCtl(HRTIM_HandleTypeDef * hhrtim,
                                         uint32_t Enable)
{
  /* Check parameters */
  assert_param(IS_HRTIM_BURSTMODECTL(Enable));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable/Disable the burst mode controller */
  MODIFY_REG(hhrtim->Instance->sCommonRegs.BMCR, HRTIM_BMCR_BME, Enable);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Trig the burst mode operation.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_BurstModeSoftwareTrigger(HRTIM_HandleTypeDef *hhrtim)
{
  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Software trigger of the burst mode controller */
  SET_BIT(hhrtim->Instance->sCommonRegs.BMTRGR, HRTIM_BMTRGR_SW);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Trig a software capture on the designed capture unit
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CaptureUnit Capture unit to trig
  *         This parameter can be one of the following values:
  *           @arg HRTIM_CAPTUREUNIT_1: Capture unit 1
  *           @arg HRTIM_CAPTUREUNIT_2: Capture unit 2
  * @retval HAL Status
  * @note The 'software capture' bit in the capure configuration register is
  *       automatically reset by hardware
  */
HAL_StatusTypeDef HAL_HRTIM_SoftwareCapture(HRTIM_HandleTypeDef * hhrtim,
                                            uint32_t TimerIdx,
                                            uint32_t CaptureUnit)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_CAPTUREUNIT(CaptureUnit));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Force a software capture on concerned capture unit */
  switch (CaptureUnit)
  {
  case HRTIM_CAPTUREUNIT_1:
    {
      SET_BIT(hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xCR, HRTIM_CPT1CR_SWCPT);
      break;
    }

  case HRTIM_CAPTUREUNIT_2:
    {
      SET_BIT(hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xCR, HRTIM_CPT2CR_SWCPT);
      break;
    }

  default:
    {
      hhrtim->State = HAL_HRTIM_STATE_ERROR;

      /* Process Unlocked */
      __HAL_UNLOCK(hhrtim);

    break;
    }
  }

  if(hhrtim->State == HAL_HRTIM_STATE_ERROR)
  {
     return HAL_ERROR;
  }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Trig the update of the registers of one or several timers
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Timers timers concerned with the software register update
  *         This parameter can be any combination of the following values:
  *           @arg HRTIM_TIMERUPDATE_MASTER
  *           @arg HRTIM_TIMERUPDATE_A     
  *           @arg HRTIM_TIMERUPDATE_B     
  *           @arg HRTIM_TIMERUPDATE_C     
  *           @arg HRTIM_TIMERUPDATE_D     
  *           @arg HRTIM_TIMERUPDATE_E     
  *           @arg HRTIM_TIMERUPDATE_F     
  * @retval HAL Status
  * @note The 'software update' bits in the HRTIM control register 2 register are
  *       automatically reset by hardware
  */
HAL_StatusTypeDef HAL_HRTIM_SoftwareUpdate(HRTIM_HandleTypeDef * hhrtim,
                                           uint32_t Timers)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMERUPDATE(Timers));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Force timer(s) registers update */
  hhrtim->Instance->sCommonRegs.CR2 |= Timers;

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Swap the Timer  outputs
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Timers timers concerned with the software register update
  *         This parameter can be any combination of the following values:
  *                   @arg HRTIM_TIMERSWAP_A
  *                   @arg HRTIM_TIMERSWAP_B
  *                   @arg HRTIM_TIMERSWAP_C
  *                   @arg HRTIM_TIMERSWAP_D
  *                   @arg HRTIM_TIMERSWAP_E
  *                   @arg HRTIM_TIMERSWAP_F
  * @retval HAL status
  * @note The function is not significant when the Push-pull mode is enabled (PSHPLL = 1)
  */
HAL_StatusTypeDef HAL_HRTIM_SwapTimerOutput(HRTIM_HandleTypeDef * hhrtim,
                                           uint32_t Timers)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMERSWAP(Timers));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Force timer(s) registers update */
  hhrtim->Instance->sCommonRegs.CR2 |= Timers;

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Trig the reset of one or several timers
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Timers timers concerned with the software counter reset
  *         This parameter can be any combination of the following values:
  *                   @arg HRTIM_TIMERRESET_MASTER
  *                   @arg HRTIM_TIMERRESET_TIMER_A
  *                   @arg HRTIM_TIMERRESET_TIMER_B
  *                   @arg HRTIM_TIMERRESET_TIMER_C
  *                   @arg HRTIM_TIMERRESET_TIMER_D
  *                   @arg HRTIM_TIMERRESET_TIMER_E
  *                   @arg HRTIM_TIMERRESET_TIMER_F
  * @retval HAL status
  * @note The 'software reset' bits in the HRTIM control register 2  are
  *       automatically reset by hardware
  */
HAL_StatusTypeDef HAL_HRTIM_SoftwareReset(HRTIM_HandleTypeDef * hhrtim,
                                          uint32_t Timers)
{
  /* Check parameters */
  assert_param(IS_HRTIM_TIMERRESET(Timers));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Force timer(s) registers reset */
  hhrtim->Instance->sCommonRegs.CR2 = Timers;

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief Swap the output of one or several timers
  * @param hhrtim: pointer to HAL HRTIM handle
  * @param Timers: timers concerned with the software register update
  *        This parameter can be any combination of the following values:
  *          @arg HRTIM_TIMERSWAP_A
  *          @arg HRTIM_TIMERSWAP_B
  *          @arg HRTIM_TIMERSWAP_C
  *          @arg HRTIM_TIMERSWAP_D
  *          @arg HRTIM_TIMERSWAP_E
  *          @arg HRTIM_TIMERSWAP_F
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_OutputSwapEnable(HRTIM_HandleTypeDef * hhrtim,
                                          uint32_t Timers)
{
   /* Check parameters */
   assert_param(IS_HRTIM_TIMERSWAP(Timers));

   if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
   {
     return HAL_BUSY;
   }

   /* Process Locked */
   __HAL_LOCK(hhrtim);

   hhrtim->State = HAL_HRTIM_STATE_BUSY;

   /* Force timer(s) registers update */
   hhrtim->Instance->sCommonRegs.CR2 |= Timers;

   hhrtim->State = HAL_HRTIM_STATE_READY;

   /* Process Unlocked */
   __HAL_UNLOCK(hhrtim);

   return HAL_OK;
}

/**
  * @brief Un-swap the output of one or several timers
  * @param hhrtim: pointer to HAL HRTIM handle
  * @param Timers: timers concerned with the software register update
  *        This parameter can be any combination of the following values:
  *          @arg HRTIM_TIMERSWAP_A
  *          @arg HRTIM_TIMERSWAP_B
  *          @arg HRTIM_TIMERSWAP_C
  *          @arg HRTIM_TIMERSWAP_D
  *          @arg HRTIM_TIMERSWAP_E
  *          @arg HRTIM_TIMERSWAP_F
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_OutputSwapDisable(HRTIM_HandleTypeDef * hhrtim,
                                          uint32_t Timers)
{
   /* Check parameters */
   assert_param(IS_HRTIM_TIMERSWAP(Timers));

   if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
   {
     return HAL_BUSY;
   }

   /* Process Locked */
   __HAL_LOCK(hhrtim);

   hhrtim->State = HAL_HRTIM_STATE_BUSY;

   /* Force timer(s) registers update */
   hhrtim->Instance->sCommonRegs.CR2 &= ~(Timers);

   hhrtim->State = HAL_HRTIM_STATE_READY;

   /* Process Unlocked */
   __HAL_UNLOCK(hhrtim);

   return HAL_OK;
}

/**
  * @brief  Start a burst DMA operation to update HRTIM control registers content
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  BurstBufferAddress address of the buffer the HRTIM control registers
  *                             content will be updated from.
  * @param  BurstBufferLength size (in WORDS) of the burst buffer.
  * @retval HAL Status
  * @note The TimerIdx parameter determines the dma channel to be used by the
  *       DMA burst controller (see below)
  *       HRTIM_TIMERINDEX_MASTER: DMA channel 2 is used by the DMA burst controller
  *       HRTIM_TIMERINDEX_TIMER_A: DMA channel 3 is used by the DMA burst controller
  *       HRTIM_TIMERINDEX_TIMER_B: DMA channel 4 is used by the DMA burst controller
  *       HRTIM_TIMERINDEX_TIMER_C: DMA channel 5 is used by the DMA burst controller
  *       HRTIM_TIMERINDEX_TIMER_D: DMA channel 6 is used by the DMA burst controller
  *       HRTIM_TIMERINDEX_TIMER_E: DMA channel 7 is used by the DMA burst controller
  *       HRTIM_TIMERINDEX_TIMER_F: DMA channel 8 is used by the DMA burst controller
  */
HAL_StatusTypeDef HAL_HRTIM_BurstDMATransfer(HRTIM_HandleTypeDef *hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t BurstBufferAddress,
                                             uint32_t BurstBufferLength)
{
  DMA_HandleTypeDef * hdma;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMERINDEX(TimerIdx));

  if(hhrtim->State == HAL_HRTIM_STATE_BUSY)
  {
     return HAL_BUSY;
  }
  if(hhrtim->State == HAL_HRTIM_STATE_READY)
  {
    if((BurstBufferAddress == 0U ) || (BurstBufferLength == 0U))
    {
      return HAL_ERROR;
    }
    else
    {
      hhrtim->State = HAL_HRTIM_STATE_BUSY;
    }
  }

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  /* Get the timer DMA handler */
  hdma = HRTIM_GetDMAHandleFromTimerIdx(hhrtim, TimerIdx);

  if (hdma == NULL)
  {
    hhrtim->State = HAL_HRTIM_STATE_ERROR;

    /* Process Unlocked */
    __HAL_UNLOCK(hhrtim);

    return HAL_ERROR;
  }

  /* Set the DMA transfer completed callback */
  hdma->XferCpltCallback = HRTIM_BurstDMACplt;

  /* Set the DMA error callback */
  hdma->XferErrorCallback = HRTIM_DMAError ;

  /* Enable the DMA channel */
  if (HAL_DMA_Start_IT(hdma,
                   BurstBufferAddress,
                   (uint32_t)&(hhrtim->Instance->sCommonRegs.BDMADR),
                   BurstBufferLength) != HAL_OK)
    {
           hhrtim->State = HAL_HRTIM_STATE_ERROR;

           /* Process Unlocked */
           __HAL_UNLOCK(hhrtim);

           return HAL_ERROR;
        }

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
}

/**
  * @brief  Enable the transfer from preload to active registers for one
  *         or several timing units (including master timer).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Timers Timer(s) concerned by the register preload enabling command
  *         This parameter can be any combination of the following values:
  *           @arg HRTIM_TIMERUPDATE_MASTER
  *           @arg HRTIM_TIMERUPDATE_A
  *           @arg HRTIM_TIMERUPDATE_B
  *           @arg HRTIM_TIMERUPDATE_C
  *           @arg HRTIM_TIMERUPDATE_D
  *           @arg HRTIM_TIMERUPDATE_E
  *           @arg HRTIM_TIMERUPDATE_E
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_UpdateEnable(HRTIM_HandleTypeDef *hhrtim,
                                          uint32_t Timers)
{
   /* Check the parameters */
  assert_param(IS_HRTIM_TIMERUPDATE(Timers));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable timer(s) registers update */
  hhrtim->Instance->sCommonRegs.CR1 &= ~(Timers);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
  }

/**
  * @brief  Disable the transfer from preload to active registers for one
  *         or several timing units (including master timer).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Timers Timer(s) concerned by the register preload disabling command
  *         This parameter can be any combination of the following values:
  *           @arg HRTIM_TIMERUPDATE_MASTER
  *           @arg HRTIM_TIMERUPDATE_A
  *           @arg HRTIM_TIMERUPDATE_B
  *           @arg HRTIM_TIMERUPDATE_C
  *           @arg HRTIM_TIMERUPDATE_D
  *           @arg HRTIM_TIMERUPDATE_E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_UpdateDisable(HRTIM_HandleTypeDef *hhrtim,
                                          uint32_t Timers)
{
  /* Check the parameters */
  assert_param(IS_HRTIM_TIMERUPDATE(Timers));

  /* Process Locked */
  __HAL_LOCK(hhrtim);

  hhrtim->State = HAL_HRTIM_STATE_BUSY;

  /* Enable timer(s) registers update */
  hhrtim->Instance->sCommonRegs.CR1 |= (Timers);

  hhrtim->State = HAL_HRTIM_STATE_READY;

  /* Process Unlocked */
  __HAL_UNLOCK(hhrtim);

  return HAL_OK;
  }

/**
  * @brief  Return the HRTIM HAL state
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval HAL state
  */
HAL_HRTIM_StateTypeDef HAL_HRTIM_GetState(const HRTIM_HandleTypeDef* hhrtim)
{
  /* Return HRTIM state */
  return hhrtim->State;
}

/**
  * @brief  Return actual value of the capture register of the designated capture unit
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CaptureUnit Capture unit to trig
  *         This parameter can be one of the following values:
  *           @arg HRTIM_CAPTUREUNIT_1: Capture unit 1
  *           @arg HRTIM_CAPTUREUNIT_2: Capture unit 2
  * @retval Captured value
  */
uint32_t HAL_HRTIM_GetCapturedValue(const HRTIM_HandleTypeDef * hhrtim,
                                    uint32_t TimerIdx,
                                    uint32_t CaptureUnit)
{
  uint32_t captured_value;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_CAPTUREUNIT(CaptureUnit));

  /* Read captured value */
  switch (CaptureUnit)
  {
  case HRTIM_CAPTUREUNIT_1:
    {
      captured_value = hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xR & 0x0000FFFFU;
      break;
    }

  case HRTIM_CAPTUREUNIT_2:
    {
      captured_value = hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xR & 0x0000FFFFU;
      break;
    }

  default:
   {
       captured_value = 0xFFFFFFFFUL;
      break;
    }

  }

  return captured_value;
}

/**
  * @brief  Return actual value and direction of the capture register of the designated capture unit
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CaptureUnit Capture unit to trig
  *         This parameter can be one of the following values:
  *           @arg HRTIM_CAPTUREUNIT_1: Capture unit 1
  *           @arg HRTIM_CAPTUREUNIT_2: Capture unit 2
  * @retval captured value and direction structure
  */
HRTIM_CaptureValueTypeDef HAL_HRTIM_GetCaptured(const HRTIM_HandleTypeDef * hhrtim,
                                    uint32_t TimerIdx,
                                    uint32_t CaptureUnit)
{
  uint32_t tmp;
  HRTIM_CaptureValueTypeDef captured;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_CAPTUREUNIT(CaptureUnit));

  /* Read captured value */
  switch (CaptureUnit)
  {
  case HRTIM_CAPTUREUNIT_1:
      tmp = hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xR;
      captured.Value = tmp & HRTIM_CPT1R_CPT1R & 0x0000FFFFU;
      captured.Dir = (((tmp & HRTIM_CPT1R_DIR) == HRTIM_CPT1R_DIR)?1U:0U);
    break;
  case HRTIM_CAPTUREUNIT_2:
      tmp = hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xR;
      captured.Value = tmp & HRTIM_CPT2R_CPT2R & 0x0000FFFFU;
      captured.Dir = (((tmp & HRTIM_CPT2R_DIR ) == HRTIM_CPT2R_DIR)?1U:0U);
    break;
  default:
      captured.Value = 0xFFFFFFFFUL;
      captured.Dir = 0xFFFFFFFFUL;
    break;
   }

  return captured;
}

/**
  * @brief  Return actual direction of the capture register of the designated capture unit
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  CaptureUnit Capture unit to trig
  *         This parameter can be one of the following values:
  *           @arg HRTIM_CAPTUREUNIT_1: Capture unit 1
  *           @arg HRTIM_CAPTUREUNIT_2: Capture unit 2
  * @retval captured direction
  *           @arg This parameter is one HRTIM_Timer_UpDown_Mode :
  *           @arg HRTIM_TIMERUPDOWNMODE_UP
  *           @arg HRTIM_TIMERUPDOWNMODE_UPDOWN
  */
uint32_t HAL_HRTIM_GetCapturedDir(const HRTIM_HandleTypeDef * hhrtim,
                                    uint32_t TimerIdx,
                                    uint32_t CaptureUnit)
{
  uint32_t tmp;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));
  assert_param(IS_HRTIM_CAPTUREUNIT(CaptureUnit));

  /* Read captured value */
  switch (CaptureUnit)
  {
  case HRTIM_CAPTUREUNIT_1:
      tmp = ((hhrtim->Instance->sTimerxRegs[TimerIdx].CPT1xR & HRTIM_CPT1R_DIR ) >> HRTIM_CPT1R_DIR_Pos);
    break;
  case HRTIM_CAPTUREUNIT_2:
      tmp = ((hhrtim->Instance->sTimerxRegs[TimerIdx].CPT2xR & HRTIM_CPT2R_DIR ) >> HRTIM_CPT2R_DIR_Pos);
    break;
  default:
    tmp = 0xFFFFFFFFU;
    break;
  }

  return tmp;
}


/**
  * @brief  Return actual level (active or inactive) of the designated output
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  Output Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval Output level
  * @note Returned output level is taken before the output stage (chopper,
  *        polarity).
  */
uint32_t HAL_HRTIM_WaveformGetOutputLevel(const HRTIM_HandleTypeDef * hhrtim,
                                          uint32_t TimerIdx,
                                          uint32_t Output)
{
  uint32_t output_level;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, Output));

  /* Read the output level */
  switch (Output)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      if ((hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxISR & HRTIM_TIMISR_O1CPY) != (uint32_t)RESET)
      {
        output_level = HRTIM_OUTPUTLEVEL_ACTIVE;
      }
      else
      {
        output_level = HRTIM_OUTPUTLEVEL_INACTIVE;
      }
     break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      if ((hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxISR & HRTIM_TIMISR_O2CPY) != (uint32_t)RESET)
      {
        output_level = HRTIM_OUTPUTLEVEL_ACTIVE;
      }
      else
      {
        output_level = HRTIM_OUTPUTLEVEL_INACTIVE;
      }
      break;
    }

  default:
    {
      output_level = 0xFFFFFFFFUL;
      break;
    }
  }

  return output_level;
}

/**
  * @brief  Return actual state (RUN, IDLE, FAULT) of the designated output
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  Output Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval Output state
  */
uint32_t HAL_HRTIM_WaveformGetOutputState(const HRTIM_HandleTypeDef * hhrtim,
                                          uint32_t TimerIdx,
                                          uint32_t Output)
{
  uint32_t output_bit;
  uint32_t output_state;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, Output));

  /* Prevent unused argument(s) compilation warning */
  UNUSED(TimerIdx);

  /* Set output state according to output control status and output disable status */
  switch (Output)
  {
  case HRTIM_OUTPUT_TA1:
    {
      output_bit = HRTIM_OENR_TA1OEN;
      break;
    }

  case HRTIM_OUTPUT_TA2:
    {
      output_bit = HRTIM_OENR_TA2OEN;
      break;
    }

  case HRTIM_OUTPUT_TB1:
    {
      output_bit = HRTIM_OENR_TB1OEN;
      break;
    }

  case HRTIM_OUTPUT_TB2:
    {
      output_bit = HRTIM_OENR_TB2OEN;
      break;
    }

  case HRTIM_OUTPUT_TC1:
    {
      output_bit = HRTIM_OENR_TC1OEN;
      break;
    }

  case HRTIM_OUTPUT_TC2:
    {
      output_bit = HRTIM_OENR_TC2OEN;
      break;
    }

  case HRTIM_OUTPUT_TD1:
    {
      output_bit = HRTIM_OENR_TD1OEN;
      break;
    }

  case HRTIM_OUTPUT_TD2:
    {
      output_bit = HRTIM_OENR_TD2OEN;
      break;
    }

  case HRTIM_OUTPUT_TE1:
    {
      output_bit = HRTIM_OENR_TE1OEN;
      break;
    }

  case HRTIM_OUTPUT_TE2:
    {
      output_bit = HRTIM_OENR_TE2OEN;
      break;
    }

  case HRTIM_OUTPUT_TF1:
    {
      output_bit = HRTIM_OENR_TF1OEN;
      break;
    }

  case HRTIM_OUTPUT_TF2:
    {
      output_bit = HRTIM_OENR_TF2OEN;
      break;
    }

  default:
    {
      output_bit = 0UL;
      break;
    }
  }

  if ((hhrtim->Instance->sCommonRegs.OENR & output_bit) != (uint32_t)RESET)
  {
    /* Output is enabled: output in RUN state (whatever output disable status is)*/
    output_state = HRTIM_OUTPUTSTATE_RUN;
  }
  else
  {
    if ((hhrtim->Instance->sCommonRegs.ODSR & output_bit) != (uint32_t)RESET)
    {
      /* Output is disabled: output in FAULT state */
      output_state = HRTIM_OUTPUTSTATE_FAULT;
    }
    else
    {
      /* Output is disabled: output in IDLE state */
      output_state = HRTIM_OUTPUTSTATE_IDLE;
    }
  }

  return(output_state);
}

/**
  * @brief  Return the level (active or inactive) of the designated output
  *         when the delayed protection was triggered.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @param  Output Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval Delayed protection status
  */
uint32_t HAL_HRTIM_GetDelayedProtectionStatus(const HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx,
                                              uint32_t Output)
{
  uint32_t delayed_protection_status;

  /* Check parameters */
  assert_param(IS_HRTIM_TIMER_OUTPUT(TimerIdx, Output));

  /* Read the delayed protection status */
  switch (Output)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      if ((hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxISR & HRTIM_TIMISR_O1STAT) != (uint32_t)RESET)
      {
        /* Output 1 was active when the delayed idle protection was triggered */
        delayed_protection_status = HRTIM_OUTPUTLEVEL_ACTIVE;
      }
      else
      {
        /* Output 1 was inactive when the delayed idle protection was triggered */
        delayed_protection_status = HRTIM_OUTPUTLEVEL_INACTIVE;
      }
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      if ((hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxISR & HRTIM_TIMISR_O2STAT) != (uint32_t)RESET)
      {
        /* Output 2 was active when the delayed idle protection was triggered */
        delayed_protection_status = HRTIM_OUTPUTLEVEL_ACTIVE;
      }
      else
      {
        /* Output 2 was inactive when the delayed idle protection was triggered */
        delayed_protection_status = HRTIM_OUTPUTLEVEL_INACTIVE;
      }
      break;
    }

  default:
    {
      delayed_protection_status = 0xFFFFFFFFUL;
      break;
    }
  }

  return delayed_protection_status;
}

/**
  * @brief  Return the actual status (active or inactive) of the burst mode controller
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval Burst mode controller status
  */
uint32_t HAL_HRTIM_GetBurstStatus(const HRTIM_HandleTypeDef * hhrtim)
{
  uint32_t burst_mode_status;

  /* Read burst mode status */
  burst_mode_status = (hhrtim->Instance->sCommonRegs.BMCR & HRTIM_BMCR_BMSTAT);

  return burst_mode_status;
}

/**
  * @brief  Indicate on which output the signal is currently active (when the
  *         push pull mode is enabled).
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval Burst mode controller status
  */
uint32_t HAL_HRTIM_GetCurrentPushPullStatus(const HRTIM_HandleTypeDef * hhrtim,
                                            uint32_t TimerIdx)
{
  uint32_t current_pushpull_status;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));

  /* Read current push pull status */
  current_pushpull_status = (hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxISR & HRTIM_TIMISR_CPPSTAT);

  return current_pushpull_status;
}


/**
  * @brief  Indicate on which output the signal was applied, in push-pull mode,
            balanced fault mode or delayed idle mode, when the protection was triggered.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval Idle Push Pull Status
  */
uint32_t HAL_HRTIM_GetIdlePushPullStatus(const HRTIM_HandleTypeDef * hhrtim,
                                         uint32_t TimerIdx)
{
  uint32_t idle_pushpull_status;

  /* Check the parameters */
  assert_param(IS_HRTIM_TIMING_UNIT(TimerIdx));

  /* Read current push pull status */
  idle_pushpull_status = (hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxISR & HRTIM_TIMISR_IPPSTAT);

  return idle_pushpull_status;
}

/**
  * @brief  This function handles HRTIM interrupt request.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be any value of HRTIM_Timer_Index
  * @retval None
  */
void HAL_HRTIM_IRQHandler(HRTIM_HandleTypeDef * hhrtim,
                          uint32_t TimerIdx)
{
  /* HRTIM interrupts handling */
  if (TimerIdx == HRTIM_TIMERINDEX_COMMON)
  {
    HRTIM_HRTIM_ISR(hhrtim);
  }
  else if (TimerIdx == HRTIM_TIMERINDEX_MASTER)
  {
    /* Master related interrupts handling */
    HRTIM_Master_ISR(hhrtim);
  }
  else
  {
    /* Timing unit related interrupts handling */
    HRTIM_Timer_ISR(hhrtim, TimerIdx);
  }

}

/**
  * @brief  Callback function invoked when a fault 1 interrupt occurred
  * @param  hhrtim pointer to HAL HRTIM handle  * @retval None
  * @retval None
  */
__weak void HAL_HRTIM_Fault1Callback(HRTIM_HandleTypeDef * hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Fault1Callback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when a fault 2 interrupt occurred
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
__weak void HAL_HRTIM_Fault2Callback(HRTIM_HandleTypeDef * hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Fault2Callback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when a fault 3 interrupt occurred
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
__weak void HAL_HRTIM_Fault3Callback(HRTIM_HandleTypeDef * hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Fault3Callback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when a fault 4 interrupt occurred
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
__weak void HAL_HRTIM_Fault4Callback(HRTIM_HandleTypeDef * hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Fault4Callback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when a fault 5 interrupt occurred
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
__weak void HAL_HRTIM_Fault5Callback(HRTIM_HandleTypeDef * hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Fault5Callback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when a fault 6 interrupt occurred
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
__weak void HAL_HRTIM_Fault6Callback(HRTIM_HandleTypeDef * hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Fault6Callback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when a system fault interrupt occurred
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
__weak void HAL_HRTIM_SystemFaultCallback(HRTIM_HandleTypeDef * hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_SystemFaultCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the DLL calibration is completed
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
__weak void HAL_HRTIM_DLLCalibrationReadyCallback(HRTIM_HandleTypeDef * hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_DLLCalibrationCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the end of the burst mode period is reached
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
__weak void HAL_HRTIM_BurstModePeriodCallback(HRTIM_HandleTypeDef * hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_BurstModeCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when a synchronization input event is received
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
__weak void HAL_HRTIM_SynchronizationEventCallback(HRTIM_HandleTypeDef * hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_SynchronizationEventCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when timer registers are updated
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_RegistersUpdateCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Master_RegistersUpdateCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when timer repetition period has elapsed
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_RepetitionEventCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Master_RepetitionEventCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the timer counter matches the value
  *         programmed in the compare 1 register
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_Compare1EventCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Master_Compare1EventCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the timer counter matches the value
  *         programmed in the compare 2 register
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  */
__weak void HAL_HRTIM_Compare2EventCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Master_Compare2EventCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the timer counter matches the value
  *         programmed in the compare 3 register
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_Compare3EventCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Master_Compare3EventCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the timer counter matches the value
  *         programmed in the compare 4 register.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_Compare4EventCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Master_Compare4EventCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the timer x capture 1 event occurs
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_Capture1EventCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Timer_Capture1EventCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the timer x capture 2 event occurs
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_Capture2EventCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Timer_Capture2EventCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the delayed idle or balanced idle mode is
  *         entered.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_DelayedProtectionCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Timer_DelayedProtectionCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the timer x counter reset/roll-over
  *         event occurs.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_CounterResetCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Timer_CounterResetCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the timer x output 1 is set
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_Output1SetCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Timer_Output1SetCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the timer x output 1 is reset
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_Output1ResetCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Timer_Output1ResetCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the timer x output 2 is set
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_Output2SetCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Timer_Output2SetCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when the timer x output 2 is reset
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_Output2ResetCallback(HRTIM_HandleTypeDef * hhrtim,
                                              uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_Timer_Output2ResetCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when a DMA burst transfer is completed
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_MASTER  for master timer
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
  */
__weak void HAL_HRTIM_BurstDMATransferCallback(HRTIM_HandleTypeDef * hhrtim,
                                               uint32_t TimerIdx)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);
  UNUSED(TimerIdx);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_BurstDMATransferCallback could be implemented in the user file
   */
}

/**
  * @brief  Callback function invoked when a DMA error occurs
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
__weak void HAL_HRTIM_ErrorCallback(HRTIM_HandleTypeDef *hhrtim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(hhrtim);

  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_HRTIM_ErrorCallback could be implemented in the user file
   */
}

#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
/**
  * @brief  HRTIM callback function registration
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  CallbackID ID of the HRTIM callback function to register
  *         This parameter can be one of the following values:
  *           @arg HAL_HRTIM_FAULT1CALLBACK_CB_ID
  *           @arg HAL_HRTIM_FAULT2CALLBACK_CB_ID
  *           @arg HAL_HRTIM_FAULT3CALLBACK_CB_ID
  *           @arg HAL_HRTIM_FAULT4CALLBACK_CB_ID
  *           @arg HAL_HRTIM_FAULT5CALLBACK_CB_ID
  *           @arg HAL_HRTIM_FAULT6CALLBACK_CB_ID
  *           @arg HAL_HRTIM_SYSTEMFAULTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_DLLCALBRATIONREADYCALLBACK_CB_ID
  *           @arg HAL_HRTIM_BURSTMODEPERIODCALLBACK_CB_ID
  *           @arg HAL_HRTIM_SYNCHRONIZATIONEVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_ERRORCALLBACK_CB_ID
  *           @arg HAL_HRTIM_MSPINIT_CB_ID
  *           @arg HAL_HRTIM_MSPDEINIT_CB_ID
  * @param  pCallback Callback function pointer
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_RegisterCallback(HRTIM_HandleTypeDef *       hhrtim,
                                             HAL_HRTIM_CallbackIDTypeDef CallbackID,
                                             pHRTIM_CallbackTypeDef      pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* Update the state */
    hhrtim->State = HAL_HRTIM_STATE_INVALID_CALLBACK;

    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hhrtim);

  if (HAL_HRTIM_STATE_READY == hhrtim->State)
  {
    switch (CallbackID)
    {
      case HAL_HRTIM_FAULT1CALLBACK_CB_ID :
        hhrtim->Fault1Callback = pCallback;
        break;

      case HAL_HRTIM_FAULT2CALLBACK_CB_ID :
        hhrtim->Fault2Callback = pCallback;
        break;

      case HAL_HRTIM_FAULT3CALLBACK_CB_ID :
        hhrtim->Fault3Callback = pCallback;
        break;

      case HAL_HRTIM_FAULT4CALLBACK_CB_ID :
        hhrtim->Fault4Callback = pCallback;
        break;

      case HAL_HRTIM_FAULT5CALLBACK_CB_ID :
        hhrtim->Fault5Callback = pCallback;
        break;

      case HAL_HRTIM_FAULT6CALLBACK_CB_ID :
        hhrtim->Fault6Callback = pCallback;
        break;

      case HAL_HRTIM_SYSTEMFAULTCALLBACK_CB_ID :
        hhrtim->SystemFaultCallback = pCallback;
        break;

      case HAL_HRTIM_DLLCALBRATIONREADYCALLBACK_CB_ID :
        hhrtim->DLLCalibrationReadyCallback = pCallback;
        break;

      case HAL_HRTIM_BURSTMODEPERIODCALLBACK_CB_ID :
        hhrtim->BurstModePeriodCallback = pCallback;
        break;

      case HAL_HRTIM_SYNCHRONIZATIONEVENTCALLBACK_CB_ID :
        hhrtim->SynchronizationEventCallback = pCallback;
        break;

      case HAL_HRTIM_ERRORCALLBACK_CB_ID :
        hhrtim->ErrorCallback = pCallback;
        break;

      case HAL_HRTIM_MSPINIT_CB_ID :
        hhrtim->MspInitCallback = pCallback;
        break;

      case HAL_HRTIM_MSPDEINIT_CB_ID :
        hhrtim->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the state */
        hhrtim->State = HAL_HRTIM_STATE_INVALID_CALLBACK;

        /* Return error status */
        status = HAL_ERROR;
        break;
    }
  }
  else if (HAL_HRTIM_STATE_RESET == hhrtim->State)
  {
    switch (CallbackID)
    {
      case HAL_HRTIM_MSPINIT_CB_ID :
        hhrtim->MspInitCallback = pCallback;
        break;

      case HAL_HRTIM_MSPDEINIT_CB_ID :
        hhrtim->MspDeInitCallback = pCallback;
        break;

      default :
        /* Update the state */
        hhrtim->State = HAL_HRTIM_STATE_INVALID_CALLBACK;

        /* Return error status */
        status = HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the state */
    hhrtim->State = HAL_HRTIM_STATE_INVALID_CALLBACK;

    /* Return error status */
    status = HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hhrtim);

  return status;
}

/**
  * @brief  HRTIM callback function un-registration
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  CallbackID ID of the HRTIM callback function to unregister
  *         This parameter can be one of the following values:
  *           @arg HAL_HRTIM_FAULT1CALLBACK_CB_ID
  *           @arg HAL_HRTIM_FAULT2CALLBACK_CB_ID
  *           @arg HAL_HRTIM_FAULT3CALLBACK_CB_ID
  *           @arg HAL_HRTIM_FAULT4CALLBACK_CB_ID
  *           @arg HAL_HRTIM_FAULT5CALLBACK_CB_ID
  *           @arg HAL_HRTIM_FAULT6CALLBACK_CB_ID
  *           @arg HAL_HRTIM_SYSTEMFAULTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_DLLCALBRATIONREADYCALLBACK_CB_ID
  *           @arg HAL_HRTIM_BURSTMODEPERIODCALLBACK_CB_ID
  *           @arg HAL_HRTIM_SYNCHRONIZATIONEVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_ERRORCALLBACK_CB_ID
  *           @arg HAL_HRTIM_MSPINIT_CB_ID
  *           @arg HAL_HRTIM_MSPDEINIT_CB_ID
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_UnRegisterCallback(HRTIM_HandleTypeDef * hhrtim,
                                               HAL_HRTIM_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hhrtim);

  if (HAL_HRTIM_STATE_READY == hhrtim->State)
  {
    switch (CallbackID)
    {
    case HAL_HRTIM_FAULT1CALLBACK_CB_ID :
      hhrtim->Fault1Callback = HAL_HRTIM_Fault1Callback;
      break;

    case HAL_HRTIM_FAULT2CALLBACK_CB_ID :
      hhrtim->Fault2Callback = HAL_HRTIM_Fault2Callback;
      break;

    case HAL_HRTIM_FAULT3CALLBACK_CB_ID :
      hhrtim->Fault3Callback = HAL_HRTIM_Fault3Callback;
      break;

    case HAL_HRTIM_FAULT4CALLBACK_CB_ID :
      hhrtim->Fault4Callback = HAL_HRTIM_Fault4Callback;
      break;

    case HAL_HRTIM_FAULT5CALLBACK_CB_ID :
      hhrtim->Fault5Callback = HAL_HRTIM_Fault5Callback;
      break;

    case HAL_HRTIM_FAULT6CALLBACK_CB_ID :
      hhrtim->Fault6Callback = HAL_HRTIM_Fault6Callback;
      break;

    case HAL_HRTIM_SYSTEMFAULTCALLBACK_CB_ID :
      hhrtim->SystemFaultCallback = HAL_HRTIM_SystemFaultCallback;
      break;

    case HAL_HRTIM_DLLCALBRATIONREADYCALLBACK_CB_ID :
      hhrtim->DLLCalibrationReadyCallback = HAL_HRTIM_DLLCalibrationReadyCallback;
      break;

    case HAL_HRTIM_BURSTMODEPERIODCALLBACK_CB_ID :
      hhrtim->BurstModePeriodCallback = HAL_HRTIM_BurstModePeriodCallback;
      break;

    case HAL_HRTIM_SYNCHRONIZATIONEVENTCALLBACK_CB_ID :
      hhrtim->SynchronizationEventCallback = HAL_HRTIM_SynchronizationEventCallback;
      break;

    case HAL_HRTIM_ERRORCALLBACK_CB_ID :
      hhrtim->ErrorCallback = HAL_HRTIM_ErrorCallback;
      break;

    case HAL_HRTIM_MSPINIT_CB_ID :
      hhrtim->MspInitCallback = HAL_HRTIM_MspInit;
      break;

    case HAL_HRTIM_MSPDEINIT_CB_ID :
      hhrtim->MspDeInitCallback = HAL_HRTIM_MspDeInit;
      break;

    default :
    /* Update the state */
    hhrtim->State = HAL_HRTIM_STATE_INVALID_CALLBACK;

    /* Return error status */
    status = HAL_ERROR;
      break;
    }
  }
  else if (HAL_HRTIM_STATE_RESET == hhrtim->State)
  {
    switch (CallbackID)
    {
    case HAL_HRTIM_MSPINIT_CB_ID :
      hhrtim->MspInitCallback = HAL_HRTIM_MspInit;
      break;

    case HAL_HRTIM_MSPDEINIT_CB_ID :
      hhrtim->MspDeInitCallback = HAL_HRTIM_MspDeInit;
      break;

    default :
    /* Update the state */
    hhrtim->State = HAL_HRTIM_STATE_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
      break;
    }
  }
  else
  {
    /* Update the state */
    hhrtim->State = HAL_HRTIM_STATE_INVALID_CALLBACK;

    /* Return error status */
    status =  HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hhrtim);

  return status;
}

/**
  * @brief  HRTIM Timer x callback function registration
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  CallbackID ID of the HRTIM Timer x callback function to register
  *         This parameter can be one of the following values:
  *           @arg HAL_HRTIM_REGISTERSUPDATECALLBACK_CB_ID
  *           @arg HAL_HRTIM_REPETITIONEVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_COMPARE1EVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_COMPARE2EVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_COMPARE3EVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_COMPARE4EVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_CAPTURE1EVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_CAPTURE2EVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_DELAYEDPROTECTIONCALLBACK_CB_ID
  *           @arg HAL_HRTIM_COUNTERRESETCALLBACK_CB_ID
  *           @arg HAL_HRTIM_OUTPUT1SETCALLBACK_CB_ID
  *           @arg HAL_HRTIM_OUTPUT1RESETCALLBACK_CB_ID
  *           @arg HAL_HRTIM_OUTPUT2SETCALLBACK_CB_ID
  *           @arg HAL_HRTIM_OUTPUT2RESETCALLBACK_CB_ID
  *           @arg HAL_HRTIM_BURSTDMATRANSFERCALLBACK_CB_ID
  * @param  pCallback Callback function pointer
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_TIMxRegisterCallback(HRTIM_HandleTypeDef * hhrtim,
                                                 HAL_HRTIM_CallbackIDTypeDef CallbackID,
                                                 pHRTIM_TIMxCallbackTypeDef pCallback)
{
  HAL_StatusTypeDef status = HAL_OK;

  if (pCallback == NULL)
  {
    /* Update the state */
    hhrtim->State = HAL_HRTIM_STATE_INVALID_CALLBACK;

    return HAL_ERROR;
  }

  /* Process locked */
  __HAL_LOCK(hhrtim);

  if (HAL_HRTIM_STATE_READY == hhrtim->State)
  {
    switch (CallbackID)
    {
      case HAL_HRTIM_REGISTERSUPDATECALLBACK_CB_ID :
        hhrtim->RegistersUpdateCallback = pCallback;
        break;

      case HAL_HRTIM_REPETITIONEVENTCALLBACK_CB_ID :
        hhrtim->RepetitionEventCallback = pCallback;
        break;

      case HAL_HRTIM_COMPARE1EVENTCALLBACK_CB_ID :
        hhrtim->Compare1EventCallback = pCallback;
        break;

      case HAL_HRTIM_COMPARE2EVENTCALLBACK_CB_ID :
        hhrtim->Compare2EventCallback = pCallback;
        break;

      case HAL_HRTIM_COMPARE3EVENTCALLBACK_CB_ID :
        hhrtim->Compare3EventCallback = pCallback;
        break;

      case HAL_HRTIM_COMPARE4EVENTCALLBACK_CB_ID :
        hhrtim->Compare4EventCallback = pCallback;
        break;

      case HAL_HRTIM_CAPTURE1EVENTCALLBACK_CB_ID :
        hhrtim->Capture1EventCallback = pCallback;
        break;

      case HAL_HRTIM_CAPTURE2EVENTCALLBACK_CB_ID :
        hhrtim->Capture2EventCallback = pCallback;
        break;

      case HAL_HRTIM_DELAYEDPROTECTIONCALLBACK_CB_ID :
        hhrtim->DelayedProtectionCallback = pCallback;
        break;

      case HAL_HRTIM_COUNTERRESETCALLBACK_CB_ID :
        hhrtim->CounterResetCallback = pCallback;
        break;

      case HAL_HRTIM_OUTPUT1SETCALLBACK_CB_ID :
        hhrtim->Output1SetCallback = pCallback;
        break;

      case HAL_HRTIM_OUTPUT1RESETCALLBACK_CB_ID :
        hhrtim->Output1ResetCallback = pCallback;
        break;

      case HAL_HRTIM_OUTPUT2SETCALLBACK_CB_ID :
        hhrtim->Output2SetCallback = pCallback;
        break;

      case HAL_HRTIM_OUTPUT2RESETCALLBACK_CB_ID :
        hhrtim->Output2ResetCallback = pCallback;
        break;

      case HAL_HRTIM_BURSTDMATRANSFERCALLBACK_CB_ID :
        hhrtim->BurstDMATransferCallback = pCallback;
        break;

    default :
        /* Update the state */
        hhrtim->State = HAL_HRTIM_STATE_INVALID_CALLBACK;

        /* Return error status */
        status = HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the state */
    hhrtim->State = HAL_HRTIM_STATE_INVALID_CALLBACK;

    /* Return error status */
    status = HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hhrtim);

  return status;
}

/**
  * @brief  HRTIM Timer x callback function un-registration
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  CallbackID ID of the HRTIM Timer x callback function to register
  *         This parameter can be one of the following values:
  *           @arg HAL_HRTIM_REGISTERSUPDATECALLBACK_CB_ID
  *           @arg HAL_HRTIM_REPETITIONEVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_COMPARE1EVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_COMPARE2EVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_COMPARE3EVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_COMPARE4EVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_CAPTURE1EVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_CAPTURE2EVENTCALLBACK_CB_ID
  *           @arg HAL_HRTIM_DELAYEDPROTECTIONCALLBACK_CB_ID
  *           @arg HAL_HRTIM_COUNTERRESETCALLBACK_CB_ID
  *           @arg HAL_HRTIM_OUTPUT1SETCALLBACK_CB_ID
  *           @arg HAL_HRTIM_OUTPUT1RESETCALLBACK_CB_ID
  *           @arg HAL_HRTIM_OUTPUT2SETCALLBACK_CB_ID
  *           @arg HAL_HRTIM_OUTPUT2RESETCALLBACK_CB_ID
  *           @arg HAL_HRTIM_BURSTDMATRANSFERCALLBACK_CB_ID
  * @retval HAL Status
  */
HAL_StatusTypeDef HAL_HRTIM_TIMxUnRegisterCallback(HRTIM_HandleTypeDef * hhrtim,
                                                   HAL_HRTIM_CallbackIDTypeDef CallbackID)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Process locked */
  __HAL_LOCK(hhrtim);

  if (HAL_HRTIM_STATE_READY == hhrtim->State)
  {
    switch (CallbackID)
    {
      case HAL_HRTIM_REGISTERSUPDATECALLBACK_CB_ID :
        hhrtim->RegistersUpdateCallback = HAL_HRTIM_RegistersUpdateCallback;
        break;

      case HAL_HRTIM_REPETITIONEVENTCALLBACK_CB_ID :
        hhrtim->RepetitionEventCallback = HAL_HRTIM_RepetitionEventCallback;
        break;

      case HAL_HRTIM_COMPARE1EVENTCALLBACK_CB_ID :
        hhrtim->Compare1EventCallback = HAL_HRTIM_Compare1EventCallback;
        break;

      case HAL_HRTIM_COMPARE2EVENTCALLBACK_CB_ID :
        hhrtim->Compare2EventCallback = HAL_HRTIM_Compare2EventCallback;
        break;

      case HAL_HRTIM_COMPARE3EVENTCALLBACK_CB_ID :
        hhrtim->Compare3EventCallback = HAL_HRTIM_Compare3EventCallback;
        break;

      case HAL_HRTIM_COMPARE4EVENTCALLBACK_CB_ID :
        hhrtim->Compare4EventCallback = HAL_HRTIM_Compare4EventCallback;
        break;

      case HAL_HRTIM_CAPTURE1EVENTCALLBACK_CB_ID :
        hhrtim->Capture1EventCallback = HAL_HRTIM_Capture1EventCallback;
        break;

      case HAL_HRTIM_CAPTURE2EVENTCALLBACK_CB_ID :
        hhrtim->Capture2EventCallback = HAL_HRTIM_Capture2EventCallback;
        break;

      case HAL_HRTIM_DELAYEDPROTECTIONCALLBACK_CB_ID :
        hhrtim->DelayedProtectionCallback = HAL_HRTIM_DelayedProtectionCallback;
        break;

      case HAL_HRTIM_COUNTERRESETCALLBACK_CB_ID :
        hhrtim->CounterResetCallback = HAL_HRTIM_CounterResetCallback;
        break;

      case HAL_HRTIM_OUTPUT1SETCALLBACK_CB_ID :
        hhrtim->Output1SetCallback = HAL_HRTIM_Output1SetCallback;
        break;

      case HAL_HRTIM_OUTPUT1RESETCALLBACK_CB_ID :
        hhrtim->Output1ResetCallback = HAL_HRTIM_Output1ResetCallback;
        break;

      case HAL_HRTIM_OUTPUT2SETCALLBACK_CB_ID :
        hhrtim->Output2SetCallback = HAL_HRTIM_Output2SetCallback;
        break;

      case HAL_HRTIM_OUTPUT2RESETCALLBACK_CB_ID :
        hhrtim->Output2ResetCallback = HAL_HRTIM_Output2ResetCallback;
        break;

      case HAL_HRTIM_BURSTDMATRANSFERCALLBACK_CB_ID :
        hhrtim->BurstDMATransferCallback = HAL_HRTIM_BurstDMATransferCallback;
        break;

    default :
        /* Update the state */
        hhrtim->State = HAL_HRTIM_STATE_INVALID_CALLBACK;

        /* Return error status */
        status = HAL_ERROR;
        break;
    }
  }
  else
  {
    /* Update the state */
    hhrtim->State = HAL_HRTIM_STATE_INVALID_CALLBACK;

    /* Return error status */
    status = HAL_ERROR;
  }

  /* Release Lock */
  __HAL_UNLOCK(hhrtim);

  return status;
}
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */

/**
  * @brief  Configure the master timer time base
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  pTimeBaseCfg pointer to the time base configuration structure
  * @retval None
  */
static void HRTIM_MasterBase_Config(HRTIM_HandleTypeDef * hhrtim,
                                    const HRTIM_TimeBaseCfgTypeDef * pTimeBaseCfg)
{
  uint32_t hrtim_mcr;

  /* Configure master timer */
  hrtim_mcr = hhrtim->Instance->sMasterRegs.MCR;

  /* Set the prescaler ratio */
  hrtim_mcr &= (uint32_t) ~(HRTIM_MCR_CK_PSC);
  hrtim_mcr |= (uint32_t)pTimeBaseCfg->PrescalerRatio;

  /* Set the operating mode */
  hrtim_mcr &= (uint32_t) ~(HRTIM_MCR_CONT | HRTIM_MCR_RETRIG);
  hrtim_mcr |= (uint32_t)pTimeBaseCfg->Mode;

  /* Update the HRTIM registers */
  hhrtim->Instance->sMasterRegs.MCR = hrtim_mcr;
  hhrtim->Instance->sMasterRegs.MPER = pTimeBaseCfg->Period;
  hhrtim->Instance->sMasterRegs.MREP = pTimeBaseCfg->RepetitionCounter;
}

/**
  * @brief  Configure timing unit (Timer A to Timer F) time base
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  * @param  pTimeBaseCfg pointer to the time base configuration structure
  * @retval None
  */
static void HRTIM_TimingUnitBase_Config(HRTIM_HandleTypeDef * hhrtim,
                                        uint32_t TimerIdx ,
                                        const HRTIM_TimeBaseCfgTypeDef * pTimeBaseCfg)
{
  uint32_t hrtim_timcr;

  /* Configure master timing unit */
  hrtim_timcr = hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR;

  /* Set the prescaler ratio */
  hrtim_timcr &= (uint32_t) ~(HRTIM_TIMCR_CK_PSC);
  hrtim_timcr |= (uint32_t)pTimeBaseCfg->PrescalerRatio;

  /* Set the operating mode */
  hrtim_timcr &= (uint32_t) ~(HRTIM_TIMCR_CONT | HRTIM_TIMCR_RETRIG);
  hrtim_timcr |= (uint32_t)pTimeBaseCfg->Mode;

  /* Update the HRTIM registers */
  hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR = hrtim_timcr;
  hhrtim->Instance->sTimerxRegs[TimerIdx].PERxR = pTimeBaseCfg->Period;
  hhrtim->Instance->sTimerxRegs[TimerIdx].REPxR = pTimeBaseCfg->RepetitionCounter;
}

/**
  * @brief  Configure the master timer in waveform mode
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  pTimerCfg pointer to the timer configuration data structure
  * @retval None
  */
static void HRTIM_MasterWaveform_Config(HRTIM_HandleTypeDef * hhrtim,
                                        const HRTIM_TimerCfgTypeDef * pTimerCfg)
{
  uint32_t hrtim_mcr;
  uint32_t hrtim_bmcr;

  /* Configure master timer */
  hrtim_mcr = hhrtim->Instance->sMasterRegs.MCR;
  hrtim_bmcr = hhrtim->Instance->sCommonRegs.BMCR;

  /* Enable/Disable the half mode */
  hrtim_mcr &= ~(HRTIM_MCR_HALF);
  hrtim_mcr |= pTimerCfg->HalfModeEnable;

  /* INTLVD bits are set to 00 */
  hrtim_mcr &= ~(HRTIM_MCR_INTLVD);
  if ((pTimerCfg->HalfModeEnable == HRTIM_HALFMODE_ENABLED) || (pTimerCfg->InterleavedMode == HRTIM_INTERLEAVED_MODE_DUAL))
  {
    /* INTLVD bits set to 00 */
    hrtim_mcr &= ~(HRTIM_MCR_INTLVD);
    hrtim_mcr |= (HRTIM_MCR_HALF);
  }
  else if ( pTimerCfg->InterleavedMode == HRTIM_INTERLEAVED_MODE_TRIPLE)
  {
        hrtim_mcr |= (HRTIM_MCR_INTLVD_BIT0);
        hrtim_mcr &= ~(HRTIM_MCR_INTLVD_BIT1);
  }
  else if ( pTimerCfg->InterleavedMode == HRTIM_INTERLEAVED_MODE_QUAD)
  {
        hrtim_mcr |= (HRTIM_MCR_INTLVD_BIT1);
        hrtim_mcr &= ~(HRTIM_MCR_INTLVD_BIT0);
  }
  else
  {
        hrtim_mcr &= ~(HRTIM_MCR_HALF);
        hrtim_mcr &= ~(HRTIM_MCR_INTLVD);
  }

  /* Enable/Disable the timer start upon synchronization event reception */
  hrtim_mcr &= ~(HRTIM_MCR_SYNCSTRTM);
  hrtim_mcr |= pTimerCfg->StartOnSync;

  /* Enable/Disable the timer reset upon synchronization event reception */
  hrtim_mcr &= ~(HRTIM_MCR_SYNCRSTM);
  hrtim_mcr |= pTimerCfg->ResetOnSync;

  /* Enable/Disable the DAC synchronization event generation */
  hrtim_mcr &= ~(HRTIM_MCR_DACSYNC);
  hrtim_mcr |= pTimerCfg->DACSynchro;

  /* Enable/Disable preload mechanism for timer registers */
  hrtim_mcr &= ~(HRTIM_MCR_PREEN);
  hrtim_mcr |= pTimerCfg->PreloadEnable;

  /* Master timer registers update handling */
  hrtim_mcr &= ~(HRTIM_MCR_BRSTDMA);
  hrtim_mcr |= (pTimerCfg->UpdateGating << 2U);

  /* Enable/Disable registers update on repetition */
  hrtim_mcr &= ~(HRTIM_MCR_MREPU);
  hrtim_mcr |= pTimerCfg->RepetitionUpdate;

  /* Set the timer burst mode */
  hrtim_bmcr &= ~(HRTIM_BMCR_MTBM);
  hrtim_bmcr |= pTimerCfg->BurstMode;

  /* Update the HRTIM registers */
  hhrtim->Instance->sMasterRegs.MCR = hrtim_mcr;
  hhrtim->Instance->sCommonRegs.BMCR = hrtim_bmcr;
}

/**
  * @brief  Configure timing unit (Timer A to Timer F) in waveform mode
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  * @param  pTimerCfg pointer to the timer configuration data structure
  * @retval None
  */
static void  HRTIM_TimingUnitWaveform_Config(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t TimerIdx,
                                             const HRTIM_TimerCfgTypeDef * pTimerCfg)
{
  uint32_t hrtim_timcr;
  uint32_t hrtim_timfltr;
  uint32_t hrtim_timoutr;
  uint32_t hrtim_timrstr;
  uint32_t hrtim_bmcr;

  /* UPDGAT bitfield must be reset before programming a new value */
  hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR &= ~(HRTIM_TIMCR_UPDGAT);

  /* Configure timing unit (Timer A to Timer F) */
  hrtim_timcr = hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR;
  hrtim_timfltr = hhrtim->Instance->sTimerxRegs[TimerIdx].FLTxR;
  hrtim_timoutr = hhrtim->Instance->sTimerxRegs[TimerIdx].OUTxR;
  hrtim_bmcr = hhrtim->Instance->sCommonRegs.BMCR;

  /* Enable/Disable the half mode */
  hrtim_timcr &= ~(HRTIM_TIMCR_HALF);
  hrtim_timcr |= pTimerCfg->HalfModeEnable;

  if ((pTimerCfg->HalfModeEnable == HRTIM_HALFMODE_ENABLED) || (pTimerCfg->InterleavedMode == HRTIM_INTERLEAVED_MODE_DUAL))
  {
    /* INTLVD bits set to 00 */
    hrtim_timcr &= ~(HRTIM_TIMCR_INTLVD);
    hrtim_timcr |= (HRTIM_TIMCR_HALF);
  }
  else if ( pTimerCfg->InterleavedMode == HRTIM_INTERLEAVED_MODE_TRIPLE)
  {
        hrtim_timcr |= (HRTIM_TIMCR_INTLVD_BIT0);
        hrtim_timcr &= ~(HRTIM_TIMCR_INTLVD_BIT1);
  }
  else if ( pTimerCfg->InterleavedMode == HRTIM_INTERLEAVED_MODE_QUAD)
  {
        hrtim_timcr |= (HRTIM_TIMCR_INTLVD_BIT1);
        hrtim_timcr &= ~(HRTIM_TIMCR_INTLVD_BIT0);
  }
  else
  {
        hrtim_timcr &= ~(HRTIM_TIMCR_HALF);
        hrtim_timcr &= ~(HRTIM_TIMCR_INTLVD);
  }

  /* Enable/Disable the timer start upon synchronization event reception */
  hrtim_timcr &= ~(HRTIM_TIMCR_SYNCSTRT);
  hrtim_timcr |= pTimerCfg->StartOnSync;

  /* Enable/Disable the timer reset upon synchronization event reception */
  hrtim_timcr &= ~(HRTIM_TIMCR_SYNCRST);
  hrtim_timcr |= pTimerCfg->ResetOnSync;

  /* Enable/Disable the DAC synchronization event generation */
  hrtim_timcr &= ~(HRTIM_TIMCR_DACSYNC);
  hrtim_timcr |= pTimerCfg->DACSynchro;

  /* Enable/Disable preload mechanism for timer registers */
  hrtim_timcr &= ~(HRTIM_TIMCR_PREEN);
  hrtim_timcr |= pTimerCfg->PreloadEnable;

  /* Timing unit registers update handling */
  hrtim_timcr &= ~(HRTIM_TIMCR_UPDGAT);
  hrtim_timcr |= pTimerCfg->UpdateGating;

  /* Enable/Disable registers update on repetition */
  hrtim_timcr &= ~(HRTIM_TIMCR_TREPU);
  if (pTimerCfg->RepetitionUpdate == HRTIM_UPDATEONREPETITION_ENABLED)
  {
    hrtim_timcr |= HRTIM_TIMCR_TREPU;
  }

  /* Set the push-pull mode */
  hrtim_timcr &= ~(HRTIM_TIMCR_PSHPLL);
  hrtim_timcr |= pTimerCfg->PushPull;

  /* Enable/Disable registers update on timer counter reset */
  hrtim_timcr &= ~(HRTIM_TIMCR_TRSTU);
  hrtim_timcr |= pTimerCfg->ResetUpdate;

  /* Set the timer update trigger */
  hrtim_timcr &= ~(HRTIM_TIMCR_TIMUPDATETRIGGER);
  hrtim_timcr |= pTimerCfg->UpdateTrigger;

  /* Enable/Disable the fault channel at timer level */
  hrtim_timfltr &= ~(HRTIM_FLTR_FLTxEN);
  hrtim_timfltr |= (pTimerCfg->FaultEnable & HRTIM_FLTR_FLTxEN);

  /* Lock/Unlock fault sources at timer level */
  hrtim_timfltr &= ~(HRTIM_FLTR_FLTLCK);
  hrtim_timfltr |= pTimerCfg->FaultLock;

    /* Enable/Disable dead time insertion at timer level */
    hrtim_timoutr &= ~(HRTIM_OUTR_DTEN);
    hrtim_timoutr |= pTimerCfg->DeadTimeInsertion;

  /* Enable/Disable delayed protection at timer level
     Delayed Idle is available whatever the timer operating mode (regular, push-pull)
     Balanced Idle is only available in push-pull mode
  */
  if ( ((pTimerCfg->DelayedProtectionMode != HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_BALANCED_EEV6)
       && (pTimerCfg->DelayedProtectionMode != HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_BALANCED_EEV7))
       || (pTimerCfg->PushPull == HRTIM_TIMPUSHPULLMODE_ENABLED))
  {
    hrtim_timoutr &= ~(HRTIM_OUTR_DLYPRT| HRTIM_OUTR_DLYPRTEN);
    hrtim_timoutr |= pTimerCfg->DelayedProtectionMode;
  }

  /* Set the BIAR mode : one bit for both outputs */
  hrtim_timoutr &= ~(HRTIM_OUTR_BIAR);
  hrtim_timoutr |= (pTimerCfg->BalancedIdleAutomaticResume);

  /* Set the timer counter reset trigger */
  hrtim_timrstr = pTimerCfg->ResetTrigger;

  /* Set the timer burst mode */
  switch (TimerIdx)
  {
  case HRTIM_TIMERINDEX_TIMER_A:
    {
      hrtim_bmcr &= ~(HRTIM_BMCR_TABM);
      hrtim_bmcr |= ( pTimerCfg->BurstMode << 1U);
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_B:
    {
      hrtim_bmcr &= ~(HRTIM_BMCR_TBBM);
      hrtim_bmcr |= ( pTimerCfg->BurstMode << 2U);
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_C:
    {
      hrtim_bmcr &= ~(HRTIM_BMCR_TCBM);
      hrtim_bmcr |= ( pTimerCfg->BurstMode << 3U);
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_D:
    {
      hrtim_bmcr &= ~(HRTIM_BMCR_TDBM);
      hrtim_bmcr |= ( pTimerCfg->BurstMode << 4U);
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_E:
    {
      hrtim_bmcr &= ~(HRTIM_BMCR_TEBM);
      hrtim_bmcr |= ( pTimerCfg->BurstMode << 5U);
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_F:
    {
      hrtim_bmcr &= ~(HRTIM_BMCR_TFBM);
      hrtim_bmcr |= ( pTimerCfg->BurstMode << 6U);
      break;
    }

  default:
    break;
  }

  /* Update the HRTIM registers */
  hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR = hrtim_timcr;
  hhrtim->Instance->sTimerxRegs[TimerIdx].FLTxR = hrtim_timfltr;
  hhrtim->Instance->sTimerxRegs[TimerIdx].OUTxR = hrtim_timoutr;
  hhrtim->Instance->sTimerxRegs[TimerIdx].RSTxR = hrtim_timrstr;
  hhrtim->Instance->sCommonRegs.BMCR = hrtim_bmcr;
}

/**
  * @brief  Control timing unit (timer A to timer F) in waveform mode
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  * @param  pTimerCtl pointer to the timer configuration data structure
  * @retval None
  */
static void HRTIM_TimingUnitWaveform_Control(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t TimerIdx,
                                             const HRTIM_TimerCtlTypeDef * pTimerCtl)
{
   uint32_t hrtim_timcr2;

   /* Configure timing unit (Timer A to Timer F) */
   hrtim_timcr2 = hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR2;

   /* Set the UpDown counting Mode */
   hrtim_timcr2 &= ~(HRTIM_TIMCR2_UDM);
   hrtim_timcr2 |= (pTimerCtl->UpDownMode << HRTIM_TIMCR2_UDM_Pos) ;

   /* Set the TrigHalf Mode : requires the counter to be disabled */
   hrtim_timcr2 &= ~(HRTIM_TIMCR2_TRGHLF);
   hrtim_timcr2 |= pTimerCtl->TrigHalf;

   /* define the compare event operating mode */
   hrtim_timcr2 &= ~(HRTIM_TIMCR2_GTCMP1);
   hrtim_timcr2 |= pTimerCtl->GreaterCMP1;

   /* define the compare event operating mode */
   hrtim_timcr2 &= ~(HRTIM_TIMCR2_GTCMP3);
   hrtim_timcr2 |= pTimerCtl->GreaterCMP3;

   if (pTimerCtl->DualChannelDacEnable == HRTIM_TIMER_DCDE_ENABLED)
   {
      /* Set the DualChannel DAC Reset trigger : requires DCDE enabled */
      hrtim_timcr2 &= ~(HRTIM_TIMCR2_DCDR);
      hrtim_timcr2 |= pTimerCtl->DualChannelDacReset;

      /* Set the DualChannel DAC Step trigger : requires DCDE enabled */
      hrtim_timcr2 &= ~(HRTIM_TIMCR2_DCDS);
      hrtim_timcr2 |= pTimerCtl->DualChannelDacStep;

      /* Enable the DualChannel DAC trigger */
      hrtim_timcr2 &= ~(HRTIM_TIMCR2_DCDE);
      hrtim_timcr2 |= pTimerCtl->DualChannelDacEnable;
   }
   /* Update the HRTIM registers */
   hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR2  = hrtim_timcr2;

}

/**
  * @brief  Configure timing RollOver Mode (Timer A to Timer F)
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  * @param  pRollOverMode: a combination of the timer RollOver Mode configuration
  * @retval None
  */
static void  HRTIM_TimingUnitRollOver_Config(HRTIM_HandleTypeDef * hhrtim,
                                             uint32_t TimerIdx,
                                             uint32_t pRollOverMode)
{
  uint32_t hrtim_timcr2;

  /* Configure timing unit (Timer A to Timer F) */
  hrtim_timcr2 = hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR2;

  if ((hrtim_timcr2 & HRTIM_TIMCR2_UDM) != 0U)
  {
       /* xxROM bitfield must be reset before programming a new value */
       hrtim_timcr2 &= ~(HRTIM_TIMCR2_ROM | HRTIM_TIMCR2_OUTROM |
                         HRTIM_TIMCR2_ADROM | HRTIM_TIMCR2_BMROM | HRTIM_TIMCR2_FEROM);

       /* Update the HRTIM TIMxCR2 register */
       hrtim_timcr2 |= pRollOverMode & (HRTIM_TIMCR2_ROM | HRTIM_TIMCR2_OUTROM |
                                        HRTIM_TIMCR2_ADROM | HRTIM_TIMCR2_BMROM | HRTIM_TIMCR2_FEROM);

       hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxCR2 = hrtim_timcr2;
  }
}

/**
  * @brief  Configure a capture unit
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  * @param  CaptureUnit Capture unit identifier
  * @param  Event Event reference
  * @retval None
  */
static void HRTIM_CaptureUnitConfig(HRTIM_HandleTypeDef * hhrtim,
                                    uint32_t TimerIdx,
                                    uint32_t CaptureUnit,
                                    uint32_t Event)
{
  uint32_t CaptureTrigger = 0xFFFFFFFFU;

  switch (Event)
  {
  case HRTIM_EVENT_1:
    {
      CaptureTrigger = HRTIM_CAPTURETRIGGER_EEV_1;
      break;
    }

  case HRTIM_EVENT_2:
    {
      CaptureTrigger = HRTIM_CAPTURETRIGGER_EEV_2;
      break;
    }

  case HRTIM_EVENT_3:
    {
      CaptureTrigger = HRTIM_CAPTURETRIGGER_EEV_3;
      break;
    }

  case HRTIM_EVENT_4:
    {
      CaptureTrigger = HRTIM_CAPTURETRIGGER_EEV_4;
      break;
    }

  case HRTIM_EVENT_5:
    {
      CaptureTrigger = HRTIM_CAPTURETRIGGER_EEV_5;
      break;
    }

  case HRTIM_EVENT_6:
    {
      CaptureTrigger = HRTIM_CAPTURETRIGGER_EEV_6;
      break;
    }

  case HRTIM_EVENT_7:
    {
      CaptureTrigger = HRTIM_CAPTURETRIGGER_EEV_7;
      break;
    }

  case HRTIM_EVENT_8:
    {
      CaptureTrigger = HRTIM_CAPTURETRIGGER_EEV_8;
      break;
    }

  case HRTIM_EVENT_9:
    {
      CaptureTrigger = HRTIM_CAPTURETRIGGER_EEV_9;
      break;
    }

  case HRTIM_EVENT_10:
    {
      CaptureTrigger = HRTIM_CAPTURETRIGGER_EEV_10;
      break;
    }

  default:
    break;
  }

  switch (CaptureUnit)
  {
  case HRTIM_CAPTUREUNIT_1:
    {
      hhrtim->TimerParam[TimerIdx].CaptureTrigger1 = CaptureTrigger;
      break;
    }

  case HRTIM_CAPTUREUNIT_2:
    {
      hhrtim->TimerParam[TimerIdx].CaptureTrigger2 = CaptureTrigger;
      break;
    }

  default:
    break;
  }
}

/**
  * @brief  Configure the output of a timing unit
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  * @param  Output timing unit output identifier
  * @param  pOutputCfg pointer to the output configuration data structure
  * @retval None
  */
static void  HRTIM_OutputConfig(HRTIM_HandleTypeDef * hhrtim,
                                uint32_t TimerIdx,
                                uint32_t Output,
                                const HRTIM_OutputCfgTypeDef * pOutputCfg)
{
  uint32_t hrtim_outr;
  uint32_t hrtim_dtr;

  uint32_t shift = 0U;

  hrtim_outr = hhrtim->Instance->sTimerxRegs[TimerIdx].OUTxR;
  hrtim_dtr = hhrtim->Instance->sTimerxRegs[TimerIdx].DTxR;

  switch (Output)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      /* Set the output set/reset crossbar */
      hhrtim->Instance->sTimerxRegs[TimerIdx].SETx1R = pOutputCfg->SetSource;
      hhrtim->Instance->sTimerxRegs[TimerIdx].RSTx1R = pOutputCfg->ResetSource;
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      /* Set the output set/reset crossbar */
      hhrtim->Instance->sTimerxRegs[TimerIdx].SETx2R = pOutputCfg->SetSource;
      hhrtim->Instance->sTimerxRegs[TimerIdx].RSTx2R = pOutputCfg->ResetSource;
      shift = 16U;
      break;
    }

  default:
    break;
  }

  /* Clear output config */
  hrtim_outr &= ~((HRTIM_OUTR_POL1 |
                   HRTIM_OUTR_IDLM1 |
                   HRTIM_OUTR_IDLES1|
                   HRTIM_OUTR_FAULT1|
                   HRTIM_OUTR_CHP1 |
                   HRTIM_OUTR_DIDL1) << shift);

  /* Set the polarity */
  hrtim_outr |= (pOutputCfg->Polarity << shift);

  /* Set the IDLE mode */
  hrtim_outr |= (pOutputCfg->IdleMode << shift);

  /* Set the IDLE state */
  hrtim_outr |= (pOutputCfg->IdleLevel << shift);

  /* Set the FAULT state */
  hrtim_outr |= (pOutputCfg->FaultLevel << shift);

  /* Set the chopper mode */
  hrtim_outr |= (pOutputCfg->ChopperModeEnable << shift);

  /* Set the burst mode entry mode : deadtime insertion when entering the idle
     state during a burst mode operation is allowed only under the following
     conditions:
     - the outputs is active during the burst mode (IDLES=1U)
     - positive deadtimes (SDTR/SDTF set to 0U)
  */
  if ((pOutputCfg->IdleLevel == HRTIM_OUTPUTIDLELEVEL_ACTIVE) &&
      ((hrtim_dtr & HRTIM_DTR_SDTR) == (uint32_t)RESET) &&
      ((hrtim_dtr & HRTIM_DTR_SDTF) == (uint32_t)RESET))
  {
    hrtim_outr |= (pOutputCfg->BurstModeEntryDelayed << shift);
  }

  /* Update HRTIM register */
  hhrtim->Instance->sTimerxRegs[TimerIdx].OUTxR = hrtim_outr;
}

/**
  * @brief  Configure an external event channel
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  Event Event channel identifier
  * @param  pEventCfg pointer to the event channel configuration data structure
  * @retval None
  */
static void HRTIM_EventConfig(HRTIM_HandleTypeDef * hhrtim,
                              uint32_t Event,
                              const HRTIM_EventCfgTypeDef *pEventCfg)
{
  uint32_t hrtim_eecr1;
  uint32_t hrtim_eecr2;
  uint32_t hrtim_eecr3;

  /* Configure external event channel */
  hrtim_eecr1 = hhrtim->Instance->sCommonRegs.EECR1;
  hrtim_eecr2 = hhrtim->Instance->sCommonRegs.EECR2;
  hrtim_eecr3 = hhrtim->Instance->sCommonRegs.EECR3;

  switch (Event)
  {
  case HRTIM_EVENT_NONE:
    {
      /* Update the HRTIM registers */
      hhrtim->Instance->sCommonRegs.EECR1 = 0U;
      hhrtim->Instance->sCommonRegs.EECR2 = 0U;
      hhrtim->Instance->sCommonRegs.EECR3 = 0U;
      break;
    }

  case HRTIM_EVENT_1:
    {
      hrtim_eecr1 &= ~(HRTIM_EECR1_EE1SRC | HRTIM_EECR1_EE1POL | HRTIM_EECR1_EE1SNS | HRTIM_EECR1_EE1FAST);
      hrtim_eecr1 |= (pEventCfg->Source & HRTIM_EECR1_EE1SRC);
      hrtim_eecr1 |= (pEventCfg->Polarity & HRTIM_EECR1_EE1POL);
      hrtim_eecr1 |= (pEventCfg->Sensitivity & HRTIM_EECR1_EE1SNS);
      /* Update the HRTIM registers (all bitfields but EE1FAST bit) */
      hhrtim->Instance->sCommonRegs.EECR1 = hrtim_eecr1;
      /* Update the HRTIM registers (EE1FAST bit) */
      hrtim_eecr1 |= (pEventCfg->FastMode  & HRTIM_EECR1_EE1FAST);
      hhrtim->Instance->sCommonRegs.EECR1 = hrtim_eecr1;
      break;
    }

  case HRTIM_EVENT_2:
    {
      hrtim_eecr1 &= ~(HRTIM_EECR1_EE2SRC | HRTIM_EECR1_EE2POL | HRTIM_EECR1_EE2SNS | HRTIM_EECR1_EE2FAST);
      hrtim_eecr1 |= ((pEventCfg->Source << 6U) & HRTIM_EECR1_EE2SRC);
      hrtim_eecr1 |= ((pEventCfg->Polarity << 6U) & HRTIM_EECR1_EE2POL);
      hrtim_eecr1 |= ((pEventCfg->Sensitivity << 6U) & HRTIM_EECR1_EE2SNS);
      /* Update the HRTIM registers (all bitfields but EE2FAST bit) */
      hhrtim->Instance->sCommonRegs.EECR1 = hrtim_eecr1;
      /* Update the HRTIM registers (EE2FAST bit) */
      hrtim_eecr1 |= ((pEventCfg->FastMode << 6U) & HRTIM_EECR1_EE2FAST);
      hhrtim->Instance->sCommonRegs.EECR1 = hrtim_eecr1;
      break;
    }

  case HRTIM_EVENT_3:
    {
      hrtim_eecr1 &= ~(HRTIM_EECR1_EE3SRC | HRTIM_EECR1_EE3POL | HRTIM_EECR1_EE3SNS | HRTIM_EECR1_EE3FAST);
      hrtim_eecr1 |= ((pEventCfg->Source << 12U) & HRTIM_EECR1_EE3SRC);
      hrtim_eecr1 |= ((pEventCfg->Polarity << 12U) & HRTIM_EECR1_EE3POL);
      hrtim_eecr1 |= ((pEventCfg->Sensitivity << 12U) & HRTIM_EECR1_EE3SNS);
      /* Update the HRTIM registers (all bitfields but EE3FAST bit) */
      hhrtim->Instance->sCommonRegs.EECR1 = hrtim_eecr1;
      /* Update the HRTIM registers (EE3FAST bit) */
      hrtim_eecr1 |= ((pEventCfg->FastMode << 12U) & HRTIM_EECR1_EE3FAST);
      hhrtim->Instance->sCommonRegs.EECR1 = hrtim_eecr1;
      break;
    }

  case HRTIM_EVENT_4:
    {
      hrtim_eecr1 &= ~(HRTIM_EECR1_EE4SRC | HRTIM_EECR1_EE4POL | HRTIM_EECR1_EE4SNS | HRTIM_EECR1_EE4FAST);
      hrtim_eecr1 |= ((pEventCfg->Source << 18U) & HRTIM_EECR1_EE4SRC);
      hrtim_eecr1 |= ((pEventCfg->Polarity << 18U) & HRTIM_EECR1_EE4POL);
      hrtim_eecr1 |= ((pEventCfg->Sensitivity << 18U) & HRTIM_EECR1_EE4SNS);
      /* Update the HRTIM registers (all bitfields but EE4FAST bit) */
      hhrtim->Instance->sCommonRegs.EECR1 = hrtim_eecr1;
      /* Update the HRTIM registers (EE4FAST bit) */
      hrtim_eecr1 |= ((pEventCfg->FastMode << 18U) & HRTIM_EECR1_EE4FAST);
      hhrtim->Instance->sCommonRegs.EECR1 = hrtim_eecr1;
      break;
    }

  case HRTIM_EVENT_5:
    {
      hrtim_eecr1 &= ~(HRTIM_EECR1_EE5SRC | HRTIM_EECR1_EE5POL | HRTIM_EECR1_EE5SNS | HRTIM_EECR1_EE5FAST);
      hrtim_eecr1 |= ((pEventCfg->Source << 24U) & HRTIM_EECR1_EE5SRC);
      hrtim_eecr1 |= ((pEventCfg->Polarity << 24U) & HRTIM_EECR1_EE5POL);
      hrtim_eecr1 |= ((pEventCfg->Sensitivity << 24U) & HRTIM_EECR1_EE5SNS);
      /* Update the HRTIM registers (all bitfields but EE5FAST bit) */
      hhrtim->Instance->sCommonRegs.EECR1 = hrtim_eecr1;
      /* Update the HRTIM registers (EE5FAST bit) */
      hrtim_eecr1 |= ((pEventCfg->FastMode << 24U) & HRTIM_EECR1_EE5FAST);
      hhrtim->Instance->sCommonRegs.EECR1 = hrtim_eecr1;
      break;
    }

  case HRTIM_EVENT_6:
    {
      hrtim_eecr2 &= ~(HRTIM_EECR2_EE6SRC | HRTIM_EECR2_EE6POL | HRTIM_EECR2_EE6SNS);
      hrtim_eecr2 |= (pEventCfg->Source & HRTIM_EECR2_EE6SRC);
      hrtim_eecr2 |= (pEventCfg->Polarity & HRTIM_EECR2_EE6POL);
      hrtim_eecr2 |= (pEventCfg->Sensitivity & HRTIM_EECR2_EE6SNS);
      hrtim_eecr3 &= ~(HRTIM_EECR3_EE6F);
      hrtim_eecr3 |= (pEventCfg->Filter & HRTIM_EECR3_EE6F);
      /* Update the HRTIM registers */
      hhrtim->Instance->sCommonRegs.EECR2 = hrtim_eecr2;
      hhrtim->Instance->sCommonRegs.EECR3 = hrtim_eecr3;
      break;
    }

  case HRTIM_EVENT_7:
    {
      hrtim_eecr2 &= ~(HRTIM_EECR2_EE7SRC | HRTIM_EECR2_EE7POL | HRTIM_EECR2_EE7SNS);
      hrtim_eecr2 |= ((pEventCfg->Source << 6U) & HRTIM_EECR2_EE7SRC);
      hrtim_eecr2 |= ((pEventCfg->Polarity << 6U) & HRTIM_EECR2_EE7POL);
      hrtim_eecr2 |= ((pEventCfg->Sensitivity << 6U) & HRTIM_EECR2_EE7SNS);
      hrtim_eecr3 &= ~(HRTIM_EECR3_EE7F);
      hrtim_eecr3 |= ((pEventCfg->Filter << 6U) & HRTIM_EECR3_EE7F);
      /* Update the HRTIM registers */
      hhrtim->Instance->sCommonRegs.EECR2 = hrtim_eecr2;
      hhrtim->Instance->sCommonRegs.EECR3 = hrtim_eecr3;
      break;
    }

  case HRTIM_EVENT_8:
    {
      hrtim_eecr2 &= ~(HRTIM_EECR2_EE8SRC | HRTIM_EECR2_EE8POL | HRTIM_EECR2_EE8SNS);
      hrtim_eecr2 |= ((pEventCfg->Source << 12U) & HRTIM_EECR2_EE8SRC);
      hrtim_eecr2 |= ((pEventCfg->Polarity << 12U) & HRTIM_EECR2_EE8POL);
      hrtim_eecr2 |= ((pEventCfg->Sensitivity << 12U) & HRTIM_EECR2_EE8SNS);
      hrtim_eecr3 &= ~(HRTIM_EECR3_EE8F);
      hrtim_eecr3 |= ((pEventCfg->Filter << 12U) & HRTIM_EECR3_EE8F );
      /* Update the HRTIM registers */
      hhrtim->Instance->sCommonRegs.EECR2 = hrtim_eecr2;
      hhrtim->Instance->sCommonRegs.EECR3 = hrtim_eecr3;
      break;
    }

  case HRTIM_EVENT_9:
    {
      hrtim_eecr2 &= ~(HRTIM_EECR2_EE9SRC | HRTIM_EECR2_EE9POL | HRTIM_EECR2_EE9SNS);
      hrtim_eecr2 |= ((pEventCfg->Source << 18U) & HRTIM_EECR2_EE9SRC);
      hrtim_eecr2 |= ((pEventCfg->Polarity << 18U) & HRTIM_EECR2_EE9POL);
      hrtim_eecr2 |= ((pEventCfg->Sensitivity << 18U) & HRTIM_EECR2_EE9SNS);
      hrtim_eecr3 &= ~(HRTIM_EECR3_EE9F);
      hrtim_eecr3 |= ((pEventCfg->Filter << 18U) & HRTIM_EECR3_EE9F);
      /* Update the HRTIM registers */
      hhrtim->Instance->sCommonRegs.EECR2 = hrtim_eecr2;
      hhrtim->Instance->sCommonRegs.EECR3 = hrtim_eecr3;
      break;
    }

  case HRTIM_EVENT_10:
    {
      hrtim_eecr2 &= ~(HRTIM_EECR2_EE10SRC | HRTIM_EECR2_EE10POL | HRTIM_EECR2_EE10SNS);
      hrtim_eecr2 |= ((pEventCfg->Source << 24U) & HRTIM_EECR2_EE10SRC);
      hrtim_eecr2 |= ((pEventCfg->Polarity << 24U) & HRTIM_EECR2_EE10POL);
      hrtim_eecr2 |= ((pEventCfg->Sensitivity << 24U) & HRTIM_EECR2_EE10SNS);
      hrtim_eecr3 &= ~(HRTIM_EECR3_EE10F);
      hrtim_eecr3 |= ((pEventCfg->Filter << 24U) & HRTIM_EECR3_EE10F);
      /* Update the HRTIM registers */
      hhrtim->Instance->sCommonRegs.EECR2 = hrtim_eecr2;
      hhrtim->Instance->sCommonRegs.EECR3 = hrtim_eecr3;
      break;
    }

  default:
    break;
  }
}

/**
  * @brief  Configure the timer counter reset
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  * @param  Event Event channel identifier
  * @retval None
  */
static void HRTIM_TIM_ResetConfig(HRTIM_HandleTypeDef * hhrtim,
                                  uint32_t TimerIdx,
                                  uint32_t Event)
{
  switch (Event)
  {
  case HRTIM_EVENT_1:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].RSTxR = HRTIM_TIMRESETTRIGGER_EEV_1;
      break;
    }

  case HRTIM_EVENT_2:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].RSTxR = HRTIM_TIMRESETTRIGGER_EEV_2;
      break;
    }

  case HRTIM_EVENT_3:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].RSTxR = HRTIM_TIMRESETTRIGGER_EEV_3;
      break;
    }

  case HRTIM_EVENT_4:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].RSTxR = HRTIM_TIMRESETTRIGGER_EEV_4;
      break;
    }

  case HRTIM_EVENT_5:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].RSTxR = HRTIM_TIMRESETTRIGGER_EEV_5;
      break;
    }

  case HRTIM_EVENT_6:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].RSTxR = HRTIM_TIMRESETTRIGGER_EEV_6;
      break;
    }

  case HRTIM_EVENT_7:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].RSTxR = HRTIM_TIMRESETTRIGGER_EEV_7;
      break;
    }

  case HRTIM_EVENT_8:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].RSTxR = HRTIM_TIMRESETTRIGGER_EEV_8;
      break;
    }

  case HRTIM_EVENT_9:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].RSTxR = HRTIM_TIMRESETTRIGGER_EEV_9;
      break;
    }

  case HRTIM_EVENT_10:
    {
      hhrtim->Instance->sTimerxRegs[TimerIdx].RSTxR = HRTIM_TIMRESETTRIGGER_EEV_10;
      break;
    }

  default:
    break;
  }
}

/**
  * @brief  Return the interrupt to enable or disable according to the
  *         OC mode.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  * @param  OCChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval Interrupt to enable or disable
  */
static uint32_t HRTIM_GetITFromOCMode(const HRTIM_HandleTypeDef * hhrtim,
                                      uint32_t TimerIdx,
                                      uint32_t OCChannel)
{
  uint32_t hrtim_set;
  uint32_t hrtim_reset;
  uint32_t interrupt = 0U;

  switch (OCChannel)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      /* Retreives actual OC mode and set interrupt accordingly */
      hrtim_set = hhrtim->Instance->sTimerxRegs[TimerIdx].SETx1R;
      hrtim_reset = hhrtim->Instance->sTimerxRegs[TimerIdx].RSTx1R;

      if (((hrtim_set & HRTIM_OUTPUTSET_TIMCMP1) == HRTIM_OUTPUTSET_TIMCMP1) &&
          ((hrtim_reset & HRTIM_OUTPUTRESET_TIMCMP1) == HRTIM_OUTPUTRESET_TIMCMP1))
      {
        /* OC mode: HRTIM_BASICOCMODE_TOGGLE */
        interrupt = HRTIM_TIM_IT_CMP1;
      }
      else if (((hrtim_set & HRTIM_OUTPUTSET_TIMCMP1) == HRTIM_OUTPUTSET_TIMCMP1) &&
               (hrtim_reset == 0U))
      {
         /* OC mode: HRTIM_BASICOCMODE_ACTIVE */
        interrupt = HRTIM_TIM_IT_SET1;
      }
      else if ((hrtim_set == 0U) &&
               ((hrtim_reset & HRTIM_OUTPUTRESET_TIMCMP1) == HRTIM_OUTPUTRESET_TIMCMP1))
      {
         /* OC mode: HRTIM_BASICOCMODE_INACTIVE */
        interrupt = HRTIM_TIM_IT_RST1;
      }
      else
      {
       /* nothing to do */
      }
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      /* Retreives actual OC mode and set interrupt accordingly */
      hrtim_set = hhrtim->Instance->sTimerxRegs[TimerIdx].SETx2R;
      hrtim_reset = hhrtim->Instance->sTimerxRegs[TimerIdx].RSTx2R;

      if (((hrtim_set & HRTIM_OUTPUTSET_TIMCMP2) == HRTIM_OUTPUTSET_TIMCMP2) &&
          ((hrtim_reset & HRTIM_OUTPUTRESET_TIMCMP2) == HRTIM_OUTPUTRESET_TIMCMP2))
      {
        /* OC mode: HRTIM_BASICOCMODE_TOGGLE */
        interrupt = HRTIM_TIM_IT_CMP2;
      }
      else if (((hrtim_set & HRTIM_OUTPUTSET_TIMCMP2) == HRTIM_OUTPUTSET_TIMCMP2) &&
               (hrtim_reset == 0U))
      {
         /* OC mode: HRTIM_BASICOCMODE_ACTIVE */
        interrupt = HRTIM_TIM_IT_SET2;
      }
      else if ((hrtim_set == 0U) &&
               ((hrtim_reset & HRTIM_OUTPUTRESET_TIMCMP2) == HRTIM_OUTPUTRESET_TIMCMP2))
      {
         /* OC mode: HRTIM_BASICOCMODE_INACTIVE */
        interrupt = HRTIM_TIM_IT_RST2;
      }
      else
      {
       /* nothing to do */
      }
      break;
    }

  default:
    break;
  }

  return interrupt;
}

/**
  * @brief  Return the DMA request to enable or disable according to the
  *         OC mode.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  * @param  OCChannel Timer output
  *         This parameter can be one of the following values:
  *           @arg HRTIM_OUTPUT_TA1: Timer A - Output 1
  *           @arg HRTIM_OUTPUT_TA2: Timer A - Output 2
  *           @arg HRTIM_OUTPUT_TB1: Timer B - Output 1
  *           @arg HRTIM_OUTPUT_TB2: Timer B - Output 2
  *           @arg HRTIM_OUTPUT_TC1: Timer C - Output 1
  *           @arg HRTIM_OUTPUT_TC2: Timer C - Output 2
  *           @arg HRTIM_OUTPUT_TD1: Timer D - Output 1
  *           @arg HRTIM_OUTPUT_TD2: Timer D - Output 2
  *           @arg HRTIM_OUTPUT_TE1: Timer E - Output 1
  *           @arg HRTIM_OUTPUT_TE2: Timer E - Output 2
  *           @arg HRTIM_OUTPUT_TF1: Timer F - Output 1
  *           @arg HRTIM_OUTPUT_TF2: Timer F - Output 2
  * @retval DMA request to enable or disable
  */
static uint32_t HRTIM_GetDMAFromOCMode(const HRTIM_HandleTypeDef * hhrtim,
                                       uint32_t TimerIdx,
                                       uint32_t OCChannel)
{
  uint32_t hrtim_set;
  uint32_t hrtim_reset;
  uint32_t dma_request = 0U;

  switch (OCChannel)
  {
  case HRTIM_OUTPUT_TA1:
  case HRTIM_OUTPUT_TB1:
  case HRTIM_OUTPUT_TC1:
  case HRTIM_OUTPUT_TD1:
  case HRTIM_OUTPUT_TE1:
  case HRTIM_OUTPUT_TF1:
    {
      /* Retreives actual OC mode and set dma_request accordingly */
      hrtim_set = hhrtim->Instance->sTimerxRegs[TimerIdx].SETx1R;
      hrtim_reset = hhrtim->Instance->sTimerxRegs[TimerIdx].RSTx1R;

      if (((hrtim_set & HRTIM_OUTPUTSET_TIMCMP1) == HRTIM_OUTPUTSET_TIMCMP1) &&
          ((hrtim_reset & HRTIM_OUTPUTRESET_TIMCMP1) == HRTIM_OUTPUTRESET_TIMCMP1))
      {
        /* OC mode: HRTIM_BASICOCMODE_TOGGLE */
        dma_request = HRTIM_TIM_DMA_CMP1;
      }
      else if (((hrtim_set & HRTIM_OUTPUTSET_TIMCMP1) == HRTIM_OUTPUTSET_TIMCMP1) &&
               (hrtim_reset == 0U))
      {
         /* OC mode: HRTIM_BASICOCMODE_ACTIVE */
        dma_request = HRTIM_TIM_DMA_SET1;
      }
      else if ((hrtim_set == 0U) &&
               ((hrtim_reset & HRTIM_OUTPUTRESET_TIMCMP1) == HRTIM_OUTPUTRESET_TIMCMP1))
      {
         /* OC mode: HRTIM_BASICOCMODE_INACTIVE */
        dma_request = HRTIM_TIM_DMA_RST1;
      }
      else
      {
    /* nothing to do */
      }
      break;
    }

  case HRTIM_OUTPUT_TA2:
  case HRTIM_OUTPUT_TB2:
  case HRTIM_OUTPUT_TC2:
  case HRTIM_OUTPUT_TD2:
  case HRTIM_OUTPUT_TE2:
  case HRTIM_OUTPUT_TF2:
    {
      /* Retreives actual OC mode and set dma_request accordingly */
      hrtim_set = hhrtim->Instance->sTimerxRegs[TimerIdx].SETx2R;
      hrtim_reset = hhrtim->Instance->sTimerxRegs[TimerIdx].RSTx2R;

      if (((hrtim_set & HRTIM_OUTPUTSET_TIMCMP2) == HRTIM_OUTPUTSET_TIMCMP2) &&
          ((hrtim_reset & HRTIM_OUTPUTRESET_TIMCMP2) == HRTIM_OUTPUTRESET_TIMCMP2))
      {
        /* OC mode: HRTIM_BASICOCMODE_TOGGLE */
        dma_request = HRTIM_TIM_DMA_CMP2;
      }
      else if (((hrtim_set & HRTIM_OUTPUTSET_TIMCMP2) == HRTIM_OUTPUTSET_TIMCMP2) &&
               (hrtim_reset == 0U))
      {
         /* OC mode: HRTIM_BASICOCMODE_ACTIVE */
        dma_request = HRTIM_TIM_DMA_SET2;
      }
      else if ((hrtim_set == 0U) &&
               ((hrtim_reset & HRTIM_OUTPUTRESET_TIMCMP2) == HRTIM_OUTPUTRESET_TIMCMP2))
      {
         /* OC mode: HRTIM_BASICOCMODE_INACTIVE */
        dma_request = HRTIM_TIM_DMA_RST2;
      }
      else
      {
    /* nothing to do */
      }
      break;
    }

  default:
    break;
  }

  return dma_request;
}

static DMA_HandleTypeDef * HRTIM_GetDMAHandleFromTimerIdx(const HRTIM_HandleTypeDef * hhrtim,
                                                          uint32_t TimerIdx)
{
  DMA_HandleTypeDef * hdma = (DMA_HandleTypeDef *)NULL;

  switch (TimerIdx)
  {
  case HRTIM_TIMERINDEX_MASTER:
    {
      hdma = hhrtim->hdmaMaster;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_A:
    {
      hdma = hhrtim->hdmaTimerA;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_B:
    {
      hdma = hhrtim->hdmaTimerB;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_C:
    {
      hdma = hhrtim->hdmaTimerC;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_D:
    {
      hdma = hhrtim->hdmaTimerD;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_E:
    {
      hdma = hhrtim->hdmaTimerE;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_F:
    {
      hdma = hhrtim->hdmaTimerF;
      break;
    }

  default:
    break;
  }

  return hdma;
}

static uint32_t GetTimerIdxFromDMAHandle(const HRTIM_HandleTypeDef * hhrtim,
                                         const DMA_HandleTypeDef * hdma)
{
  uint32_t timed_idx = 0xFFFFFFFFU;

  if (hdma == hhrtim->hdmaMaster)
  {
    timed_idx = HRTIM_TIMERINDEX_MASTER;
  }
  else if (hdma == hhrtim->hdmaTimerA)
  {
    timed_idx = HRTIM_TIMERINDEX_TIMER_A;
  }
  else if (hdma == hhrtim->hdmaTimerB)
  {
    timed_idx = HRTIM_TIMERINDEX_TIMER_B;
  }
  else if (hdma == hhrtim->hdmaTimerC)
  {
    timed_idx = HRTIM_TIMERINDEX_TIMER_C;
  }
  else if (hdma == hhrtim->hdmaTimerD)
  {
    timed_idx = HRTIM_TIMERINDEX_TIMER_D;
  }
  else if (hdma == hhrtim->hdmaTimerE)
  {
    timed_idx = HRTIM_TIMERINDEX_TIMER_E;
  }
  else if (hdma == hhrtim->hdmaTimerF)
  {
    timed_idx = HRTIM_TIMERINDEX_TIMER_F;
  }
  else
  {
    /* nothing to do */
  }
  return timed_idx;
}

/**
  * @brief  Force an immediate transfer from the preload to the active
  *         registers.
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  * @retval None
  */
static void HRTIM_ForceRegistersUpdate(HRTIM_HandleTypeDef * hhrtim,
                                       uint32_t TimerIdx)
{
  switch (TimerIdx)
  {
  case HRTIM_TIMERINDEX_MASTER:
    {
      hhrtim->Instance->sCommonRegs.CR2 |= HRTIM_CR2_MSWU;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_A:
    {
      hhrtim->Instance->sCommonRegs.CR2 |= HRTIM_CR2_TASWU;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_B:
    {
      hhrtim->Instance->sCommonRegs.CR2 |= HRTIM_CR2_TBSWU;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_C:
    {
      hhrtim->Instance->sCommonRegs.CR2 |= HRTIM_CR2_TCSWU;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_D:
    {
      hhrtim->Instance->sCommonRegs.CR2 |= HRTIM_CR2_TDSWU;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_E:
    {
      hhrtim->Instance->sCommonRegs.CR2 |= HRTIM_CR2_TESWU;
      break;
    }

  case HRTIM_TIMERINDEX_TIMER_F:
    {
      hhrtim->Instance->sCommonRegs.CR2 |= HRTIM_CR2_TFSWU;
      break;
    }

  default:
    break;
  }
}


/**
  * @brief  HRTIM interrupts service routine
  * @param  hhrtim pointer to HAL HRTIM handle
  * @retval None
  */
static void HRTIM_HRTIM_ISR(HRTIM_HandleTypeDef * hhrtim)
{
  uint32_t isrflags = READ_REG(hhrtim->Instance->sCommonRegs.ISR);
  uint32_t ierits   = READ_REG(hhrtim->Instance->sCommonRegs.IER);

  /* Fault 1 event */
  if((uint32_t)(isrflags & HRTIM_FLAG_FLT1) != (uint32_t)RESET)
  {
    if((uint32_t)(ierits & HRTIM_IT_FLT1) != (uint32_t)RESET)
    {
      __HAL_HRTIM_CLEAR_IT(hhrtim, HRTIM_IT_FLT1);

      /* Invoke Fault 1 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Fault1Callback(hhrtim);
#else
      HAL_HRTIM_Fault1Callback(hhrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Fault 2 event */
  if((uint32_t)(isrflags & HRTIM_FLAG_FLT2) != (uint32_t)RESET)
  {
    if((uint32_t)(ierits & HRTIM_IT_FLT2) != (uint32_t)RESET)
    {
      __HAL_HRTIM_CLEAR_IT(hhrtim, HRTIM_IT_FLT2);

      /* Invoke Fault 2 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Fault2Callback(hhrtim);
#else
      HAL_HRTIM_Fault2Callback(hhrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Fault 3 event */
  if((uint32_t)(isrflags & HRTIM_FLAG_FLT3) != (uint32_t)RESET)
  {
    if((uint32_t)(ierits & HRTIM_IT_FLT3) != (uint32_t)RESET)
    {
      __HAL_HRTIM_CLEAR_IT(hhrtim, HRTIM_IT_FLT3);

      /* Invoke Fault 3 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Fault3Callback(hhrtim);
#else
      HAL_HRTIM_Fault3Callback(hhrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Fault 4 event */
  if((uint32_t)(isrflags & HRTIM_FLAG_FLT4) != (uint32_t)RESET)
  {
    if((uint32_t)(ierits & HRTIM_IT_FLT4) != (uint32_t)RESET)
    {
      __HAL_HRTIM_CLEAR_IT(hhrtim, HRTIM_IT_FLT4);

      /* Invoke Fault 4 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Fault4Callback(hhrtim);
#else
      HAL_HRTIM_Fault4Callback(hhrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Fault 5 event */
  if((uint32_t)(isrflags & HRTIM_FLAG_FLT5) != (uint32_t)RESET)
  {
    if((uint32_t)(ierits & HRTIM_IT_FLT5) != (uint32_t)RESET)
    {
      __HAL_HRTIM_CLEAR_IT(hhrtim, HRTIM_IT_FLT5);

      /* Invoke Fault 5 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Fault5Callback(hhrtim);
#else
      HAL_HRTIM_Fault5Callback(hhrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Fault 6 event */
  if((uint32_t)(isrflags & HRTIM_FLAG_FLT6) != (uint32_t)RESET)
  {
    if((uint32_t)(ierits & HRTIM_IT_FLT6) != (uint32_t)RESET)
    {
      __HAL_HRTIM_CLEAR_IT(hhrtim, HRTIM_IT_FLT6);

      /* Invoke Fault 6 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Fault6Callback(hhrtim);
#else
      HAL_HRTIM_Fault6Callback(hhrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* System fault event */
  if((uint32_t)(isrflags & HRTIM_FLAG_SYSFLT) != (uint32_t)RESET)
  {
    if((uint32_t)(ierits & HRTIM_IT_SYSFLT) != (uint32_t)RESET)
    {
      __HAL_HRTIM_CLEAR_IT(hhrtim, HRTIM_IT_SYSFLT);

      /* Invoke System fault event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->SystemFaultCallback(hhrtim);
#else
      HAL_HRTIM_SystemFaultCallback(hhrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }
}

/**
* @brief  Master timer interrupts service routine
* @param  hhrtim pointer to HAL HRTIM handle
* @retval None
*/
static void HRTIM_Master_ISR(HRTIM_HandleTypeDef * hhrtim)
{
  uint32_t isrflags  = READ_REG(hhrtim->Instance->sCommonRegs.ISR);
  uint32_t ierits    = READ_REG(hhrtim->Instance->sCommonRegs.IER);
  uint32_t misrflags = READ_REG(hhrtim->Instance->sMasterRegs.MISR);
  uint32_t mdierits  = READ_REG(hhrtim->Instance->sMasterRegs.MDIER);

  /* DLL calibration ready event */
  if((uint32_t)(isrflags & HRTIM_FLAG_DLLRDY) != (uint32_t)RESET)
  {
    if((uint32_t)(ierits & HRTIM_IT_DLLRDY) != (uint32_t)RESET)
    {
      __HAL_HRTIM_CLEAR_IT(hhrtim, HRTIM_IT_DLLRDY);

      /* Set HRTIM State */
      hhrtim->State = HAL_HRTIM_STATE_READY;

      /* Process unlocked */
      __HAL_UNLOCK(hhrtim);

      /* Invoke System fault event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->DLLCalibrationReadyCallback(hhrtim);
#else
      HAL_HRTIM_DLLCalibrationReadyCallback(hhrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Burst mode period event */
  if((uint32_t)(isrflags & HRTIM_FLAG_BMPER) != (uint32_t)RESET)
  {
    if((uint32_t)(ierits & HRTIM_IT_BMPER) != (uint32_t)RESET)
    {
      __HAL_HRTIM_CLEAR_IT(hhrtim, HRTIM_IT_BMPER);

      /* Invoke Burst mode period event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->BurstModePeriodCallback(hhrtim);
#else
      HAL_HRTIM_BurstModePeriodCallback(hhrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Master timer compare 1 event */
  if((uint32_t)(misrflags & HRTIM_MASTER_FLAG_MCMP1) != (uint32_t)RESET)
  {
    if((uint32_t)(mdierits & HRTIM_MASTER_IT_MCMP1) != (uint32_t)RESET)
    {
      __HAL_HRTIM_MASTER_CLEAR_IT(hhrtim, HRTIM_MASTER_IT_MCMP1);

      /* Invoke compare 1 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Compare1EventCallback(hhrtim, HRTIM_TIMERINDEX_MASTER);
#else
      HAL_HRTIM_Compare1EventCallback(hhrtim, HRTIM_TIMERINDEX_MASTER);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Master timer compare 2 event */
  if((uint32_t)(misrflags & HRTIM_MASTER_FLAG_MCMP2) != (uint32_t)RESET)
  {
    if((uint32_t)(mdierits & HRTIM_MASTER_IT_MCMP2) != (uint32_t)RESET)
    {
      __HAL_HRTIM_MASTER_CLEAR_IT(hhrtim, HRTIM_MASTER_IT_MCMP2);

      /* Invoke compare 2 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Compare2EventCallback(hhrtim, HRTIM_TIMERINDEX_MASTER);
#else
      HAL_HRTIM_Compare2EventCallback(hhrtim, HRTIM_TIMERINDEX_MASTER);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Master timer compare 3 event */
  if((uint32_t)(misrflags & HRTIM_MASTER_FLAG_MCMP3) != (uint32_t)RESET)
  {
    if((uint32_t)(mdierits & HRTIM_MASTER_IT_MCMP3) != (uint32_t)RESET)
    {
      __HAL_HRTIM_MASTER_CLEAR_IT(hhrtim, HRTIM_MASTER_IT_MCMP3);

      /* Invoke compare 3 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Compare3EventCallback(hhrtim, HRTIM_TIMERINDEX_MASTER);
#else
      HAL_HRTIM_Compare3EventCallback(hhrtim, HRTIM_TIMERINDEX_MASTER);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Master timer compare 4 event */
  if((uint32_t)(misrflags & HRTIM_MASTER_FLAG_MCMP4) != (uint32_t)RESET)
  {
    if((uint32_t)(mdierits & HRTIM_MASTER_IT_MCMP4) != (uint32_t)RESET)
    {
      __HAL_HRTIM_MASTER_CLEAR_IT(hhrtim, HRTIM_MASTER_IT_MCMP4);

      /* Invoke compare 4 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Compare4EventCallback(hhrtim, HRTIM_TIMERINDEX_MASTER);
#else
      HAL_HRTIM_Compare4EventCallback(hhrtim, HRTIM_TIMERINDEX_MASTER);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Master timer repetition event */
  if((uint32_t)(misrflags & HRTIM_MASTER_FLAG_MREP) != (uint32_t)RESET)
  {
    if((uint32_t)(mdierits & HRTIM_MASTER_IT_MREP) != (uint32_t)RESET)
    {
      __HAL_HRTIM_MASTER_CLEAR_IT(hhrtim, HRTIM_MASTER_IT_MREP);

      /* Invoke repetition event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->RepetitionEventCallback(hhrtim, HRTIM_TIMERINDEX_MASTER);
#else
      HAL_HRTIM_RepetitionEventCallback(hhrtim, HRTIM_TIMERINDEX_MASTER);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Synchronization input event */
  if((uint32_t)(misrflags & HRTIM_MASTER_FLAG_SYNC) != (uint32_t)RESET)
  {
    if((uint32_t)(mdierits & HRTIM_MASTER_IT_SYNC) != (uint32_t)RESET)
    {
      __HAL_HRTIM_MASTER_CLEAR_IT(hhrtim, HRTIM_MASTER_IT_SYNC);

      /* Invoke synchronization event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->SynchronizationEventCallback(hhrtim);
#else
      HAL_HRTIM_SynchronizationEventCallback(hhrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Master timer registers update event */
  if((uint32_t)(misrflags & HRTIM_MASTER_FLAG_MUPD) != (uint32_t)RESET)
  {
    if((uint32_t)(mdierits & HRTIM_MASTER_IT_MUPD) != (uint32_t)RESET)
    {
      __HAL_HRTIM_MASTER_CLEAR_IT(hhrtim, HRTIM_MASTER_IT_MUPD);

      /* Invoke registers update event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->RegistersUpdateCallback(hhrtim, HRTIM_TIMERINDEX_MASTER);
#else
      HAL_HRTIM_RegistersUpdateCallback(hhrtim, HRTIM_TIMERINDEX_MASTER);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }
}

/**
  * @brief  Timer interrupts service routine
  * @param  hhrtim pointer to HAL HRTIM handle
  * @param  TimerIdx Timer index
  *         This parameter can be one of the following values:
  *           @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *           @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *           @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *           @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *           @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *           @arg HRTIM_TIMERINDEX_TIMER_F for timer F
  * @retval None
*/
static void HRTIM_Timer_ISR(HRTIM_HandleTypeDef * hhrtim,
                     uint32_t TimerIdx)
{
  uint32_t tisrflags = READ_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxISR);
  uint32_t tdierits  = READ_REG(hhrtim->Instance->sTimerxRegs[TimerIdx].TIMxDIER);

  /* Timer compare 1 event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_CMP1) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_CMP1) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CMP1);

      /* Invoke compare 1 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Compare1EventCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_Compare1EventCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Timer compare 2 event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_CMP2) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_CMP2) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CMP2);

      /* Invoke compare 2 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Compare2EventCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_Compare2EventCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Timer compare 3 event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_CMP3) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_CMP3) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CMP3);

      /* Invoke compare 3 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Compare3EventCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_Compare3EventCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Timer compare 4 event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_CMP4) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_CMP4) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CMP4);

      /* Invoke compare 4 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Compare4EventCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_Compare4EventCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Timer repetition event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_REP) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_REP) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_REP);

      /* Invoke repetition event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->RepetitionEventCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_RepetitionEventCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Timer registers update event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_UPD) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_UPD) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_UPD);

      /* Invoke registers update event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->RegistersUpdateCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_RegistersUpdateCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Timer capture 1 event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_CPT1) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_CPT1) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CPT1);

      /* Invoke capture 1 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Capture1EventCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_Capture1EventCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Timer capture 2 event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_CPT2) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_CPT2) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_CPT2);

      /* Invoke capture 2 event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Capture2EventCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_Capture2EventCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Timer output 1 set event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_SET1) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_SET1) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_SET1);

      /* Invoke output 1 set event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Output1SetCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_Output1SetCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Timer output 1 reset event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_RST1) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_RST1) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_RST1);

      /* Invoke output 1 reset event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Output1ResetCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_Output1ResetCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Timer output 2 set event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_SET2) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_SET2) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_SET2);

      /* Invoke output 2 set event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Output2SetCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_Output2SetCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Timer output 2 reset event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_RST2) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_RST2) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_RST2);

      /* Invoke output 2 reset event callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->Output2ResetCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_Output2ResetCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Timer reset event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_RST) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_RST) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_RST);

      /* Invoke timer reset callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->CounterResetCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_CounterResetCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }

  /* Delayed protection event */
  if((uint32_t)(tisrflags & HRTIM_TIM_FLAG_DLYPRT) != (uint32_t)RESET)
  {
    if((uint32_t)(tdierits & HRTIM_TIM_IT_DLYPRT) != (uint32_t)RESET)
    {
      __HAL_HRTIM_TIMER_CLEAR_IT(hhrtim, TimerIdx, HRTIM_TIM_IT_DLYPRT);

      /* Invoke delayed protection callback */
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
      hhrtim->DelayedProtectionCallback(hhrtim, TimerIdx);
#else
      HAL_HRTIM_DelayedProtectionCallback(hhrtim, TimerIdx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
    }
  }
}

/**
  * @brief  DMA callback invoked upon master timer related DMA request completion
  * @param  hdma pointer to DMA handle.
  * @retval None
  */
static void HRTIM_DMAMasterCplt(DMA_HandleTypeDef *hdma)
{
  HRTIM_HandleTypeDef * hrtim = (HRTIM_HandleTypeDef *)((DMA_HandleTypeDef* )hdma)->Parent;

  if ((hrtim->Instance->sMasterRegs.MDIER & HRTIM_MASTER_DMA_MCMP1) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Compare1EventCallback(hrtim, HRTIM_TIMERINDEX_MASTER);
#else
    HAL_HRTIM_Compare1EventCallback(hrtim, HRTIM_TIMERINDEX_MASTER);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sMasterRegs.MDIER & HRTIM_MASTER_DMA_MCMP2) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Compare2EventCallback(hrtim, HRTIM_TIMERINDEX_MASTER);
#else
    HAL_HRTIM_Compare2EventCallback(hrtim, HRTIM_TIMERINDEX_MASTER);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sMasterRegs.MDIER & HRTIM_MASTER_DMA_MCMP3) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Compare3EventCallback(hrtim, HRTIM_TIMERINDEX_MASTER);
#else
    HAL_HRTIM_Compare3EventCallback(hrtim, HRTIM_TIMERINDEX_MASTER);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sMasterRegs.MDIER & HRTIM_MASTER_DMA_MCMP4) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Compare4EventCallback(hrtim, HRTIM_TIMERINDEX_MASTER);
#else
    HAL_HRTIM_Compare4EventCallback(hrtim, HRTIM_TIMERINDEX_MASTER);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sMasterRegs.MDIER & HRTIM_MASTER_DMA_SYNC) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->SynchronizationEventCallback(hrtim);
#else
    HAL_HRTIM_SynchronizationEventCallback(hrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sMasterRegs.MDIER & HRTIM_MASTER_DMA_MUPD) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->RegistersUpdateCallback(hrtim, HRTIM_TIMERINDEX_MASTER);
#else
    HAL_HRTIM_RegistersUpdateCallback(hrtim, HRTIM_TIMERINDEX_MASTER);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sMasterRegs.MDIER & HRTIM_MASTER_DMA_MREP) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->RepetitionEventCallback(hrtim, HRTIM_TIMERINDEX_MASTER);
#else
    HAL_HRTIM_RepetitionEventCallback(hrtim, HRTIM_TIMERINDEX_MASTER);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else
  {
    /* nothing to do */
  }
}

/**
  * @brief  DMA callback invoked upon timer A..F related DMA request completion
  * @param  hdma pointer to DMA handle.
  * @retval None
  */
static void HRTIM_DMATimerxCplt(DMA_HandleTypeDef *hdma)
{
  uint8_t timer_idx;

  HRTIM_HandleTypeDef * hrtim = (HRTIM_HandleTypeDef *)((DMA_HandleTypeDef* )hdma)->Parent;

  timer_idx = (uint8_t)GetTimerIdxFromDMAHandle(hrtim, hdma);

  if ( !IS_HRTIM_TIMING_UNIT(timer_idx) ) {return;}

  if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_CMP1) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Compare1EventCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_Compare1EventCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_CMP2) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Compare2EventCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_Compare2EventCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_CMP3) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Compare3EventCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_Compare3EventCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_CMP4) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Compare4EventCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_Compare4EventCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_UPD) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->RegistersUpdateCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_RegistersUpdateCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_CPT1) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Capture1EventCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_Capture1EventCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_CPT2) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Capture2EventCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_Capture2EventCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_SET1) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Output1SetCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_Output1SetCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_RST1) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Output1ResetCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_Output1ResetCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_SET2) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Output2SetCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_Output2SetCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_RST2) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->Output2ResetCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_Output2ResetCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_RST) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->CounterResetCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_CounterResetCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_DLYPRT) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->DelayedProtectionCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_DelayedProtectionCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else if ((hrtim->Instance->sTimerxRegs[timer_idx].TIMxDIER & HRTIM_TIM_DMA_REP) != (uint32_t)RESET)
  {
#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->RepetitionEventCallback(hrtim, timer_idx);
#else
    HAL_HRTIM_RepetitionEventCallback(hrtim, timer_idx);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
  }
  else
  {
    /* nothing to do */
  }
}

/**
* @brief  DMA error callback
* @param  hdma pointer to DMA handle.
* @retval None
*/
static void HRTIM_DMAError(DMA_HandleTypeDef *hdma)
{
  HRTIM_HandleTypeDef * hrtim = (HRTIM_HandleTypeDef *)((DMA_HandleTypeDef* )hdma)->Parent;

#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->ErrorCallback(hrtim);
#else
  HAL_HRTIM_ErrorCallback(hrtim);
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
}

/**
  * @brief  DMA callback invoked upon burst DMA transfer completion
  * @param  hdma pointer to DMA handle.
  * @retval None
  */
static void HRTIM_BurstDMACplt(DMA_HandleTypeDef *hdma)
{
  HRTIM_HandleTypeDef * hrtim = (HRTIM_HandleTypeDef *)((DMA_HandleTypeDef* )hdma)->Parent;

#if (USE_HAL_HRTIM_REGISTER_CALLBACKS == 1)
    hrtim->BurstDMATransferCallback(hrtim, GetTimerIdxFromDMAHandle(hrtim, hdma));
#else
  HAL_HRTIM_BurstDMATransferCallback(hrtim, GetTimerIdxFromDMAHandle(hrtim, hdma));
#endif /* USE_HAL_HRTIM_REGISTER_CALLBACKS */
}

/**
  * end of HRTIM_Function_Definitions @}
  */

#endif /* HAL_HRTIM_MODULE_ENABLED */

/**
  * end of HRTIM @}
  */
