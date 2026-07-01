/**
 ******************************************************************************
 * @file    sc32f1xxx_trng.h
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   Header file of TRNG module.
 ******************************************************************************
 * @attention
 *
 *1.This software is supplied by SinOne Microelectronics Co.,Ltd. and is only 
 *intended for use with SinOne products. No other uses are authorized. This 
 *software is owned by SinOne Microelectronics Co.,Ltd. and is protected under 
 *all applicable laws, including copyright laws. 
 *2.The software which is for guidance only aims at providing customers with 
 *coding information regarding their products in order for them to save time. 
 *As a result, SinOne shall not be held liable for any direct, indirect or 
 *consequential damages with respect to any claims arising from the content of 
 *such software and/or the use made by customers of the coding information 
 *contained herein in connection with their products.
 *
 *  COPYRIGHT 2026 SinOne Microelectronics
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __sc32f1xxx_trng_H
#define __sc32f1xxx_trng_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "sc32f1xxx.h"
#include "sc32.h"
#include "sc32f1xxx_rcc.h"

/** @addtogroup sc32f1xxx_StdPeriph_Driver
 * @{
 */

/** @addtogroup TRNG
 * @{
 */

/** @defgroup TRNG_Enumerations TRNG Enumerations
 * @{
 */

/** @brief TRNG_IT TRNG Interrupt
 * @{
 */
typedef enum
{
    TRNG_IT_INTEN = (uint16_t)TRNG_CFG_TINTEN,	/*!< TRNG Interrupt: TRNG Interrupt */
    TRNG_IT_DRDYIE = (uint16_t)TRNG_CFG_DRDYIE,	/*!< TRNG Interrupt: TRNG Data generation completed Interrupt */
} TRNG_IT_TypeDef;

#define IS_TRNG_IT(IT) (((IT) == TRNG_CFG_TINTEN) ||  \
                         ((IT) == TRNG_CFG_DRDYIE)) 
/**
 * @}
 */

/** @brief TRNG_Flag  TRNG Flag
 * @{
 */
typedef enum
{
    TRNG_FLAG_SEIF = (uint16_t)TRNG_STS_SEIF,/*!< TRNG Flag: Data generation completed */
    TRNG_FLAG_DRDYIF = (uint16_t)TRNG_STS_DRDYIF,/*!< TRNG Flag: Seed error interrupt */		
} TRNG_Flag_TypeDef;

#define IS_GET_TRNG_FLAG(FLAG) (((FLAG) == TRNG_FLAG_SEIF) ||  \
                               ((FLAG) == TRNG_FLAG_DRDYIE)) 
/**
 * @}
 */

/**
 * @}
 */
/* End of enumerations -----------------------------------------------------*/

/** @addtogroup TRNG_Functions TRNG Functions
 * @{
 */

/* TRNG Base functions ********************************************************/
void TRNG_DeInit ( void );
void TRNG_Cmd(FunctionalState NewState);
void TRNG_TLSELConfig(uint8_t TRNG_TLSEL);
void TRNG_TLDIVConfig(uint8_t TRNG_TLDIV);
uint32_t TRNG_GetRandomNumber(void);
/* TRNG Interrupts functions ********************************************************/
void TRNG_ITConfig(uint16_t TRNG_IT,FunctionalState NewState);
FlagStatus TRNG_GetFlagStatus(TRNG_Flag_TypeDef TRNG_FLAG);
void TRNG_ClearFlag(uint8_t TRNG_FLAG);

/**
 * @}
 */
/* End of functions --------------------------------------------------*/

/**
 * @}
 */

/**
 * @}
 */
 
#ifdef __cplusplus
}
#endif

#endif
