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
*   File    : rx32g4xx_hal_pwr_ex.h
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _RX32G4xx_HAL_PWR_EX_H_
#define _RX32G4xx_HAL_PWR_EX_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_def.h"

/** @addtogroup PWR
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** @addtogroup PWR_Structure_Definitions PWR Structure Definitions
  * @{
  */

/** @defgroup PWR_PVMTypeDef PWR PVMTypeDef
  * @ingroup  PWR_Structure_Definitions
  * @{
  */

/**
  * @brief  PWR PVM configuration structure definition
  */
typedef struct
{
  uint32_t PVMType;   /*!< PVMType: Specifies which voltage is monitored and against which threshold.
                           This parameter can be a value of @ref PWREx_PVM_Type. */
  uint32_t Mode;      /*!< Mode: Specifies the operating mode for the selected pins.
                           This parameter can be a value of @ref PWREx_PVM_Mode. */
}PWR_PVMTypeDef;

/**
  * end of PWR_PVMTypeDef @}
  */

/**
  * end of PWR_Structure_Definitions @}
  */

/* Exported constants --------------------------------------------------------*/

/** @defgroup PWR_Parameter_Definitions PWR Parameter Definitions
  * @{
  */

/** @defgroup PWR_WUP_Polarity PWR WUP Polarity
  * @ingroup  PWR_Parameter_Definitions
  * @{
  */

/**
  * @brief  Shift to apply to retrieve polarity information from PWR_WAKEUP_PINy_xxx constants
  */

#define PWR_WUP_POLARITY_SHIFT                  0x05U   /*!< Internal constant used to retrieve wakeup pin polariry */
/**
  * end of PWR_WUP_Polarity @}
  */

/** @defgroup PWR_WakeUp_Pins  PWR WakeUp Pins
  * @ingroup  PWR_Parameter_Definitions
  * @{
  */

#define PWR_WAKEUP_PIN1                 PWR_CR3_EWUP1    /*!< Wakeup pin 1 (with high level polarity) */
#define PWR_WAKEUP_PIN2                 PWR_CR3_EWUP2    /*!< Wakeup pin 2 (with high level polarity) */
#define PWR_WAKEUP_PIN3                 PWR_CR3_EWUP3    /*!< Wakeup pin 3 (with high level polarity) */
#define PWR_WAKEUP_PIN4                 PWR_CR3_EWUP4    /*!< Wakeup pin 4 (with high level polarity) */

#define PWR_WAKEUP_PIN1_HIGH            PWR_WAKEUP_PIN1  /*!< Wakeup pin 1 (with high level polarity) */
#define PWR_WAKEUP_PIN2_HIGH            PWR_WAKEUP_PIN2  /*!< Wakeup pin 2 (with high level polarity) */
#define PWR_WAKEUP_PIN3_HIGH            PWR_WAKEUP_PIN3  /*!< Wakeup pin 3 (with high level polarity) */
#define PWR_WAKEUP_PIN4_HIGH            PWR_WAKEUP_PIN4  /*!< Wakeup pin 4 (with high level polarity) */

#define PWR_WAKEUP_PIN1_LOW             (uint32_t)((PWR_CR4_WP1<<PWR_WUP_POLARITY_SHIFT) | PWR_CR3_EWUP1) /*!< Wakeup pin 1 (with low level polarity) */
#define PWR_WAKEUP_PIN2_LOW             (uint32_t)((PWR_CR4_WP2<<PWR_WUP_POLARITY_SHIFT) | PWR_CR3_EWUP2) /*!< Wakeup pin 2 (with low level polarity) */
#define PWR_WAKEUP_PIN3_LOW             (uint32_t)((PWR_CR4_WP3<<PWR_WUP_POLARITY_SHIFT) | PWR_CR3_EWUP3) /*!< Wakeup pin 3 (with low level polarity) */
#define PWR_WAKEUP_PIN4_LOW             (uint32_t)((PWR_CR4_WP4<<PWR_WUP_POLARITY_SHIFT) | PWR_CR3_EWUP4) /*!< Wakeup pin 4 (with low level polarity) */
/**
  * end of PWR_WakeUp_Pins @}
  */

/** @defgroup PWR_Status_Flags PWR Status Flags
  * @ingroup  PWR_Parameter_Definitions
  * @{
  */

/**
  * @brief  PWR Status Flags
  *         Elements values convention: 0000 0000 0XXY YYYYb
  *           - Y YYYY  : Flag position in the XX register (5 bits)
  *           - XX  : Status register (2 bits)
  *                 - 01: SR1 register
  *                 - 10: SR2 register
  *        The only exception is PWR_FLAG_WU, encompassing all
  *        wake-up flags and set to PWR_SR1_WUF.
  */

#define PWR_FLAG_WUF1                       PWR_SR_WUF1          /*!< Wakeup event on wakeup pin 1 */
#define PWR_FLAG_WUF2                       PWR_SR_WUF2          /*!< Wakeup event on wakeup pin 2 */
#define PWR_FLAG_WUF3                       PWR_SR_WUF3          /*!< Wakeup event on wakeup pin 3 */
#define PWR_FLAG_WUF4                       PWR_SR_WUF4          /*!< Wakeup event on wakeup pin 4 */
#define PWR_FLAG_WU                         PWR_SR_WUF           /*!< Encompass wakeup event on all wakeup pins */
#define PWR_FLAG_SB                         PWR_SR_SBF           /*!< Standby flag */
#define PWR_FLAG_WUFI                       PWR_SR_WUFI          /*!< Wakeup on internal wakeup line */

#define PWR_FLAG_PVDO                       PWR_SR2_PVDO         /*!< Power Voltage Detector output flag */

/**
  * end of PWR_Status_Flags @}
  */

/**
  * end of PWR_Parameter_Definitions @}
  */

/* Exported macros -----------------------------------------------------------*/
/** @defgroup PWR_Macro_Definitions PWR Macro Definitions
 * @{
 */

/* Private macros --------------------------------------------------------*/

#define IS_PWR_WAKEUP_PIN(PIN) (((PIN) == PWR_WAKEUP_PIN1)      || \
                                ((PIN) == PWR_WAKEUP_PIN2)      || \
                                ((PIN) == PWR_WAKEUP_PIN3)      || \
                                ((PIN) == PWR_WAKEUP_PIN4)      || \
                                ((PIN) == PWR_WAKEUP_PIN1_HIGH) || \
                                ((PIN) == PWR_WAKEUP_PIN2_HIGH) || \
                                ((PIN) == PWR_WAKEUP_PIN3_HIGH) || \
                                ((PIN) == PWR_WAKEUP_PIN4_HIGH) || \
                                ((PIN) == PWR_WAKEUP_PIN1_LOW)  || \
                                ((PIN) == PWR_WAKEUP_PIN2_LOW)  || \
                                ((PIN) == PWR_WAKEUP_PIN3_LOW)  || \
                                ((PIN) == PWR_WAKEUP_PIN4_LOW))
/**
  * end of PWR_Macro_Definitions @}
  */

/** @defgroup PWR_Function_Definitions PWR Function Definitions
  * @{
  */

/* Low Power modes configuration functions ************************************/
void HAL_PWREx_EnterSTOP0Mode(uint8_t STOPEntry);
void HAL_PWREx_EnterSTOP1Mode(uint8_t STOPEntry);

void HAL_PWREx_PVD_PVM_IRQHandler(void);

/**
  * end of PWR_Function_Definitions @}
  */

/**
  * end of PWR @}
  */

#ifdef __cplusplus
}
#endif


#endif /* _RX32G4xx_HAL_PWR_EX_H_ */

