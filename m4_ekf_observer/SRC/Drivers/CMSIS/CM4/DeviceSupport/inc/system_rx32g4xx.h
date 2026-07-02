/**************************************************************************//**
 * @file     system_rx32g4xx.h
 * @brief    CMSIS Device System Header File for
 *           rx32g4xx Device
 * @version  V5.3.2
 * @date     15. November 2019
 ******************************************************************************/
/*
 * Copyright (c) 2009-2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _SYSTEM_RX32G4XX_H_
#define _SYSTEM_RX32G4XX_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief Exception / Interrupt Handler Function Prototype
  */
typedef void(*VECTOR_TABLE_Type)(void);

/**
  * @brief System Clock Frequency (Core Clock)
  */
extern uint32_t SystemCoreClock;

/**
  * @brief Setup the microcontroller system.
  *        Initialize the System and update the SystemCoreClock variable.
  */
extern void SystemInit (void);


/**
  * @brief  Update SystemCoreClock variable.
  *         Updates the SystemCoreClock with current core Clock retrieved from cpu registers.
  */
extern void SystemCoreClockUpdate (void);

#ifdef __cplusplus
}
#endif

#endif /* _SYSTEM_RX32G4XX_H_ */
