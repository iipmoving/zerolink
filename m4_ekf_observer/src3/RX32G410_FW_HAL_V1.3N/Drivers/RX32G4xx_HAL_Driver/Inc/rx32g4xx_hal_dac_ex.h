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
*   File    : rx32g4xx_hal_dac_ex.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_DAC_EX_H_
#define _RX32G4XX_HAL_DAC_EX_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"
#include "rx32g4xx_hal_dac.h"
#include "rx32g4xx_hal_dma.h"


/* Exported types ------------------------------------------------------------*/

/** @addtogroup DAC
  * @{
  */
  
/******************************************************************************/
/*                                 DAC Functions                              */
/******************************************************************************/

/** @addtogroup DAC_Function_Definitions DAC Function Definitions
  * @{
  */
  
HAL_StatusTypeDef           HAL_DACEx_WaveGenerate(DAC_HandleTypeDef *hdac, uint32_t Channel);
HAL_StatusTypeDef HAL_DACEx_SawtoothWaveGenerate(DAC_HandleTypeDef *hdac, uint32_t Channel, uint32_t Polarity,
                                                 uint32_t ResetData, uint32_t StepData);
HAL_StatusTypeDef           HAL_DACEx_SawtoothWaveDataReset(DAC_HandleTypeDef *hdac, uint32_t Channel);
HAL_StatusTypeDef           HAL_DACEx_SawtoothWaveDataStep(DAC_HandleTypeDef *hdac, uint32_t Channel);
            
HAL_StatusTypeDef           HAL_DACEx_DualStart(DAC_HandleTypeDef *hdac);
HAL_StatusTypeDef           HAL_DACEx_DualStop(DAC_HandleTypeDef *hdac);
HAL_StatusTypeDef           HAL_DACEx_DualStart_DMA(DAC_HandleTypeDef *hdac, uint32_t Channel,
                                                    const uint32_t *pData, uint32_t Length, uint32_t Alignment);
HAL_StatusTypeDef           HAL_DACEx_DualStop_DMA(DAC_HandleTypeDef *hdac, uint32_t Channel);
HAL_StatusTypeDef           HAL_DACEx_DualSetValue(DAC_HandleTypeDef *hdac, uint32_t Alignment, uint32_t Data1, uint32_t Data2);
uint32_t                    HAL_DACEx_DualGetValue(const DAC_HandleTypeDef *hdac);

void                        HAL_DACEx_ConvCpltCallbackCh2(DAC_HandleTypeDef *hdac);
void                        HAL_DACEx_ConvHalfCpltCallbackCh2(DAC_HandleTypeDef *hdac);
void                        HAL_DACEx_ErrorCallbackCh2(DAC_HandleTypeDef *hdac);
void                        HAL_DACEx_DMAUnderrunCallbackCh2(DAC_HandleTypeDef *hdac);


HAL_StatusTypeDef           HAL_DACEx_SelfCalibrate(DAC_HandleTypeDef *hdac, DAC_ChannelConfTypeDef *sConfig, uint32_t Channel);
HAL_StatusTypeDef           HAL_DACEx_SetUserTrimming(DAC_HandleTypeDef *hdac, DAC_ChannelConfTypeDef *sConfig, uint32_t Channel, uint32_t NewTrimmingValue);
uint32_t                    HAL_DACEx_GetTrimOffset(const DAC_HandleTypeDef *hdac, uint32_t Channel);

void                        DAC_DMAConvCpltCh2(DMA_HandleTypeDef *hdma);
void                        DAC_DMAErrorCh2(DMA_HandleTypeDef *hdma);
void                        DAC_DMAHalfConvCpltCh2(DMA_HandleTypeDef *hdma);
  
/** 
  * end of DAC_Function_Definitions @}  
  */
/** 
  * end of DAC @}  
  */


#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_DAC_EX_H_ */

