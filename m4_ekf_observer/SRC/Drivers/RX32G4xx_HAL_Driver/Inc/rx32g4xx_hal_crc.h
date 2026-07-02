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
*   File    : rx32g4xx_hal_crc.h
*   By      : RX_DV_Team
**************************************************************************************************
*/



#ifndef _RX32G4XX_HAL_CRC_H_
#define _RX32G4XX_HAL_CRC_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_common.h"
#include "rx32g4xx_hal_def.h"
#include "rx32g4xx_hal_conf.h"

/* Exported types ------------------------------------------------------------*/
  
/** @addtogroup CRC
  * @{
  */

/******************************************************************************/
/*                               CRC Structures                               */
/******************************************************************************/
  
/** @defgroup CRC_Structure_Definitions CRC Structure Definitions
  * @{
  */
  
/** @defgroup HAL_CRC_StateTypeDef HAL CRC StateTypeDef
  * @ingroup  CRC_Structure_Definitions 
  * @{
  */  
/**
  * @brief  CRC HAL State Structure definition
  */
typedef enum
{
  HAL_CRC_STATE_RESET     = 0x00U,  /*!< CRC not yet initialized or disabled */
  HAL_CRC_STATE_READY     = 0x01U,  /*!< CRC initialized and ready for use   */
  HAL_CRC_STATE_BUSY      = 0x02U,  /*!< CRC internal process is ongoing     */
  HAL_CRC_STATE_TIMEOUT   = 0x03U,  /*!< CRC timeout state                   */
  HAL_CRC_STATE_ERROR     = 0x04U   /*!< CRC error state                     */
} HAL_CRC_StateTypeDef; 
/** 
  * end of HAL_CRC_StateTypeDef @}  
  */
  
/** @defgroup CRC_HandleTypeDef CRC HandleTypeDef
  * @ingroup  CRC_Structure_Definitions 
  * @{
  */
/**
  * @brief  CRC Handle Structure definition
  */
typedef struct
{
  CRC_TypeDef                 *Instance;   /*!< Register base address        */

  HAL_LockTypeDef             Lock;        /*!< CRC Locking object           */

  __IO HAL_CRC_StateTypeDef   State;       /*!< CRC communication state      */
  
} CRC_HandleTypeDef;
  
/** 
  * end of CRC_HandleTypeDef @}  
  */  
/** 
  * end of CRC_Structure_Definitions @}  
  */

/******************************************************************************/
/*                              CRC Parameters                                */
/******************************************************************************/
  
/** @defgroup CRC_Parameter_Definitions CRC Parameter Definitions
  * @{
  */
  
/** @defgroup CRC_Default_Polynomial_Value CRC Default Polynomial Value
  * @ingroup  CRC_Parameter_Definitions
  * @{
  */
/**
  * @brief  CRC default polynomial
  */
#define DEFAULT_CRC32_POLY      0x04C11DB7U  /*!<  X^32 + X^26 + X^23 + X^22 + X^16 + X^12 + X^11 + X^10 +X^8 + X^7 + X^5 + X^4 + X^2+ X +1 */  
/** 
  * end of CRC_Default_Polynomial_Value @}  
  */
  
/** @defgroup CRC_Default_InitValue    Default CRC computation initialization value
  * @ingroup  CRC_Parameter_Definitions
  * @{
  */
/**
  * @brief  CRC init value
  */
#define DEFAULT_CRC_INITVALUE   0xFFFFFFFFU  /*!< Initial CRC default value */
/** 
  * end of CRC_Default_InitValue @}  
  */  
/** 
  * end of CRC_Parameter_Definitions @}  
  */


/******************************************************************************/
/*                                  CRC Macro                                 */
/******************************************************************************/
/** @defgroup CRC_Macro_Definitions CRC Macro Definitions
  * @{
  */

/** @brief Reset CRC handle state.
  * @param  __HANDLE__ CRC handle.
  * @retval None
  */
#define __HAL_CRC_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_CRC_STATE_RESET)

/**
  * @brief  Reset CRC Data Register.
  * @param  __HANDLE__ CRC handle
  * @retval None
  */
#define __HAL_CRC_DR_RESET(__HANDLE__) ((__HANDLE__)->Instance->CR |= CRC_CR_RESET)

/**
  * @brief Store data in the Independent Data (ID) register.
  * @param __HANDLE__ CRC handle
  * @param __VALUE__  Value to be stored in the ID register
  * @note  Refer to the Reference Manual to get the authorized __VALUE__ length in bits
  * @retval None
  */
#define __HAL_CRC_SET_IDR(__HANDLE__, __VALUE__) (WRITE_REG((__HANDLE__)->Instance->IDR, (__VALUE__)))

/**
  * @brief Return the data stored in the Independent Data (ID) register.
  * @param __HANDLE__ CRC handle
  * @note  Refer to the Reference Manual to get the authorized __VALUE__ length in bits
  * @retval Value of the ID register
  */
#define __HAL_CRC_GET_IDR(__HANDLE__) (((__HANDLE__)->Instance->IDR) & CRC_IDR_IDR)
 
/** 
  * end of CRC_Macro_Definitions @}  
  */
  
/******************************************************************************/
/*                                CRC Functions                               */
/******************************************************************************/

/** @defgroup CRC_Function_Definitions CRC Function Definitions
  * @{
  */
HAL_StatusTypeDef                   HAL_CRC_Init(CRC_HandleTypeDef *hcrc);
HAL_StatusTypeDef                   HAL_CRC_DeInit(CRC_HandleTypeDef *hcrc);
void                                HAL_CRC_MspInit(CRC_HandleTypeDef *hcrc);
void                                HAL_CRC_MspDeInit(CRC_HandleTypeDef *hcrc);
uint32_t                            HAL_CRC_Accumulate(CRC_HandleTypeDef *hcrc, uint32_t pBuffer[], uint32_t BufferLength);
HAL_CRC_StateTypeDef                HAL_CRC_GetState(const CRC_HandleTypeDef *hcrc); 
/** 
  * end of CRC_Function_Definitions @}  
  */

/** 
  * end of CRC @}  
  */

#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_CRC_H_ */

