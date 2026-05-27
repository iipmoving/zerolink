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
*   File    : rx32g4xx_hal.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_H_
#define _RX32G4XX_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal_conf.h"


/* Exported variables ---------------------------------------------------------*/
extern __IO uint32_t uwTick;
extern uint32_t uwTickPrio;
extern uint32_t uwTickFreq;


/* Exported types ------------------------------------------------------------*/

/** @addtogroup HAL
  * @{
  */

/******************************************************************************/
/*                            HAL Structures                                  */
/******************************************************************************/

/** @defgroup HAL_Structure_Definitions HAL Structure Definitions
  * @{
  */

/**
  * end of HAL_Structure_Definitions @}
  */


/******************************************************************************/
/*                            HAL Parameters                                  */
/******************************************************************************/

/** @defgroup HAL_Parameter_Definitions HAL Parameter Definitions
  * @{
  */

/** @defgroup HAL_TICK_FREQ HAL TICK FREQ
  * @ingroup  HAL_Parameter_Definitions
  * @{
  */
#define HAL_TICK_FREQ_10HZ         100U
#define HAL_TICK_FREQ_100HZ        10U
#define HAL_TICK_FREQ_1KHZ         1U
#define HAL_TICK_FREQ_DEFAULT      HAL_TICK_FREQ_1KHZ
/**
  * end of HAL_TICK_FREQ @}
  */

/** @defgroup SYSCFG_VREFBUF_VoltageScale VREFBUF Voltage Scale
  * @ingroup  HAL_Parameter_Definitions
  * @{
  */
#define SYSCFG_VREFBUF_VOLTAGE_SCALE0  0x00000000U              /*!< Voltage reference scale 0 (VREFBUF_OUT = 2.0V) */
#define SYSCFG_VREFBUF_VOLTAGE_SCALE1  VREFBUF_CSR_VRS_BIT0     /*!< Voltage reference scale 1 (VREFBUF_OUT = 2.5V) */
#define SYSCFG_VREFBUF_VOLTAGE_SCALE2  VREFBUF_CSR_VRS_BIT1     /*!< Voltage reference scale 2 (VREFBUF_OUT = 2.8V) */
/**
  * end of SYSCFG_VREFBUF_VoltageScale @}
  */

/** @defgroup SYSCFG_VREFBUF_HighImpedance VREFBUF High Impedance
  * @ingroup  HAL_Parameter_Definitions
  * @{
  */
#define SYSCFG_VREFBUF_HIGH_IMPEDANCE_DISABLE  0x00000000U      /*!< VREF_plus pin is internally connected to Voltage reference buffer output */
#define SYSCFG_VREFBUF_HIGH_IMPEDANCE_ENABLE   VREFBUF_CSR_HIZ  /*!< VREF_plus pin is high impedance */
/**
  * end of SYSCFG_VREFBUF_HighImpedance @}
  */

/** @defgroup SYSCFG_flags_definition SYSCFG flags definition
  * @ingroup  HAL_Parameter_Definitions
  * @{
  */
#define SYSCFG_FLAG_SRAM_PE             SYSCFG_CFGR2_SPF        /*!< SRAM parity error (first 32kB of SRAM1 + CCM SRAM) */
#define SYSCFG_FLAG_SRAM_PE2            SYSCFG_CFGR2_SPF2       /*!< SRAM parity error (first 32kB of SRAM1 + CCM SRAM) */
#define SYSCFG_FLAG_CCMSRAM_BUSY        SYSCFG_SCSR_CCMBSY      /*!< CCMSRAM busy by erase operation */
/**
  * end of SYSCFG_flags_definition @}
  */

/**
  * end of HAL_Parameter_Definitions @}
  */


/******************************************************************************/
/*                              HAL Macro                                     */
/******************************************************************************/

/** @defgroup HAL_Macro_Definitions HAL Macro Definitions
  * @{
  */

/** @brief  SYSCFG Break ECC lock.
  *         Enable and lock the connection of Flash ECC error connection to TIM1/8/15/16/17 Break input.
  * @note   The selected configuration is locked and can be unlocked only by system reset.
  */
#define __HAL_SYSCFG_BREAK_ECC_LOCK()        SET_BIT(SYSCFG->CFGR2, SYSCFG_CFGR2_ECCL)

/** @brief  SYSCFG Break Cortex-M4 Lockup lock.
  *         Enable and lock the connection of Cortex-M4 LOCKUP (Hardfault) output to TIM1/8/15/16/17 Break input.
  * @note   The selected configuration is locked and can be unlocked only by system reset.
  */
#define __HAL_SYSCFG_BREAK_LOCKUP_LOCK()     SET_BIT(SYSCFG->CFGR2, SYSCFG_CFGR2_CLL)

/** @brief  SYSCFG Break PVD lock.
  *         Enable and lock the PVD connection to Timer1/8/15/16/17 Break input, as well as the PVDE and PLS[2:0] in the PWR_CR2 register.
  * @note   The selected configuration is locked and can be unlocked only by system reset.
  */
#define __HAL_SYSCFG_BREAK_PVD_LOCK()        SET_BIT(SYSCFG->CFGR2, SYSCFG_CFGR2_PVDL)

/** @brief  SYSCFG Break SRAM parity lock.
  *         Enable and lock the SRAM parity error (first 32kB of SRAM1 + CCM SRAM) signal connection to TIM1/8/15/16/17 Break input.
  * @note   The selected configuration is locked and can be unlocked by system reset.
  */
#define __HAL_SYSCFG_BREAK_SRAMPARITY_LOCK() SET_BIT(SYSCFG->CFGR2, SYSCFG_CFGR2_SPL)

/** @brief  Check SYSCFG flag is set or not.
  * @param  __FLAG__: specifies the flag to check.
  *         This parameter can be one of the following values:
  *         @arg @ref SYSCFG_FLAG_SRAM_PE : SRAM Parity Error Flag
  *         @arg @ref SYSCFG_FLAG_CCMSRAM_BUSY : CCMSRAM Erase Ongoing
  * @retval The new state of __FLAG__ (TRUE or FALSE).
  */
#define __HAL_SYSCFG_GET_FLAG(__FLAG__)      ((((((__FLAG__) == SYSCFG_SCSR_CCMBSY)? SYSCFG->SCSR : SYSCFG->CFGR2)\
                                                & (__FLAG__))!= 0U) ? 1U : 0U)

/** @brief  Set the SPF bit to clear the SRAM Parity Error Flag.
  */
#define __HAL_SYSCFG_CLEAR_FLAG()            SET_BIT(SYSCFG->CFGR2, SYSCFG_CFGR2_SPF)

/** @brief  Set the SPF2 bit to clear the SRAM Parity Error Flag.
  */
#define __HAL_SYSCFG_CLEAR_FLAG2()           SET_BIT(SYSCFG->CFGR2, SYSCFG_CFGR2_SPF2)

#define IS_SYSCFG_VREFBUF_VOLTAGE_SCALE(__SCALE__)  (((__SCALE__) == SYSCFG_VREFBUF_VOLTAGE_SCALE0) || \
                                                     ((__SCALE__) == SYSCFG_VREFBUF_VOLTAGE_SCALE1) || \
                                                     ((__SCALE__) == SYSCFG_VREFBUF_VOLTAGE_SCALE2))

#define IS_SYSCFG_VREFBUF_HIGH_IMPEDANCE(__VALUE__)  (((__VALUE__) == SYSCFG_VREFBUF_HIGH_IMPEDANCE_DISABLE) || \
                                                      ((__VALUE__) == SYSCFG_VREFBUF_HIGH_IMPEDANCE_ENABLE))

#define IS_SYSCFG_VREFBUF_TRIMMING(__VALUE__)  ((__VALUE__) <= VREFBUF_CCR_TRIM)

#define IS_TICKFREQ(FREQ) (((FREQ) == HAL_TICK_FREQ_10HZ)  || \
                           ((FREQ) == HAL_TICK_FREQ_100HZ) || \
                           ((FREQ) == HAL_TICK_FREQ_1KHZ))

/**
  * end of HAL_Macro_Definitions @}
  */


/******************************************************************************/
/*                             HAL Functions                                  */
/******************************************************************************/

/** @defgroup HAL_Function_Definitions HAL Function Definitions
  * @{
  */

/* Initialization and Configuration functions  ******************************/
HAL_StatusTypeDef HAL_Init(void);
HAL_StatusTypeDef HAL_DeInit(void);
void HAL_MspInit(void);
void HAL_MspDeInit(void);
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority);

/* Peripheral Control functions  ************************************************/
void HAL_IncTick(void);
uint32_t HAL_GetTick(void);
uint32_t HAL_GetTickPrio(void);
HAL_StatusTypeDef HAL_SetTickFreq(uint32_t Freq);
uint32_t HAL_GetTickFreq(void);
uint32_t HAL_Tick_Diff(uint32_t u32Base);
void HAL_Delay(uint32_t Delay);
void HAL_SuspendTick(void);
void HAL_ResumeTick(void);

/* SYSCFG Control functions  ****************************************************/
void HAL_SYSCFG_CCMSRAMErase(void);

void HAL_SYSCFG_VREFBUF_VoltageScalingConfig(uint32_t VoltageScaling);
void HAL_SYSCFG_VREFBUF_HighImpedanceConfig(uint32_t Mode);
void HAL_SYSCFG_VREFBUF_TrimmingConfig(uint32_t TrimmingValue);
HAL_StatusTypeDef HAL_SYSCFG_EnableVREFBUF(void);
void HAL_SYSCFG_DisableVREFBUF(void);

/**
  * end of HAL_Function_Definitions @}
  */


/**
  * end of HAL @}
  */


#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_H_ */
