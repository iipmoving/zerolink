/**
 ******************************************************************************
 * @file    sc32f1xxx_lpd.h
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   Header file of LPD module.
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
#ifndef __sc32f1xxx_LPD_H
#define __sc32f1xxx_LPD_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "sc32_conf.h"
#include "sc32.h"
#include "sc32f1xxx_rcc.h"

/** @addtogroup sc32f1xxx_StdPeriph_Driver
 * @{
 */

/** @addtogroup LPD
 * @{
 */

/* Exported enumerations ------------------------------------------------------------*/
/** @defgroup LPD_Exported_Enumerations LPD Exported Enumerations
 * @{
 */

/** @brief LPD_IS LPD threshold voltage level selection
 * @{
 */
typedef enum
{
    VLPD_1_85V_typ    = ( uint16_t ) ( 0x00 << LPD_CON_LPDIS_Pos ), /*!<The LPD threshold voltage is 1.85V */
    VLPD_2_05V_typ    = ( uint16_t ) ( 0x01 << LPD_CON_LPDIS_Pos ), /*!<The LPD threshold voltage is 2.05V */
		VLPD_2_25V_typ    = ( uint16_t ) ( 0x02 << LPD_CON_LPDIS_Pos ), /*!<The LPD threshold voltage is 2.25V */
		VLPD_2_45V_typ    = ( uint16_t ) ( 0x03 << LPD_CON_LPDIS_Pos ), /*!<The LPD threshold voltage is 2.45V */
    VLPD_2_65V_typ    = ( uint16_t ) ( 0x04 << LPD_CON_LPDIS_Pos ), /*!<The LPD threshold voltage is 2.65V */
		VLPD_2_85V_typ    = ( uint16_t ) ( 0x05 << LPD_CON_LPDIS_Pos ), /*!<The LPD threshold voltage is 2.85V */
		VLPD_3_05V_typ    = ( uint16_t ) ( 0x06 << LPD_CON_LPDIS_Pos ), /*!<The LPD threshold voltage is 3.05V */
		VLPD_3_25V_typ    = ( uint16_t ) ( 0x07 << LPD_CON_LPDIS_Pos ), /*!<The LPD threshold voltage is 3.25V */
} LPD_IS_TypeDef;

#define IS_LPD_VLPD(VLPD) (((VLPD) == VLPD_1_85V_typ) ||  \
													 ((VLPD) == VLPD_2_05V_typ) ||  \
													 ((VLPD) == VLPD_2_25V_typ) ||  \
													 ((VLPD) == VLPD_2_45V_typ) ||  \
												   ((VLPD) == VLPD_2_65V_typ) ||  \
													 ((VLPD) == VLPD_2_85V_typ) ||  \
											     ((VLPD) == VLPD_3_05V_typ) ||  \
													 ((VLPD) == VLPD_3_25V_typ))
/**
 * @}
 */

/** @brief LPD_IT LPD Interrupt
 * @{
 */
typedef enum
{
    LPD_IT_INTEN = ( uint16_t ) LPD_IDE_INTEN,	/*!< LPD Interrupt: LPD  Interrupt */
} LPD_IT_TypeDef;

#define IS_LPD_IT(IT) (((IT) == LPD_IT_INTEN))
/**
 * @}
 */

/** @brief LPD_FLAG LPD Flag
 * @{
 */
typedef enum
{
    LPD_Flag_LPDOF    = ( uint32_t )LPD_CON_LPDOF,	/*!< LPD status Flag */
    LPD_Flag_LPDIF    = ( uint32_t )LPD_CON_LPDIF,  /*!< LPD interrupt request Flag */
} LPD_FLAG_TypeDef;
#define IS_LPD_FLAG(FLAG) ((((FLAG) & (uint16_t)0xFFFC) == 0x00) && ((FLAG) != (uint16_t)0x0000))

/**
 * @}
 */

/**
 * @}
 */
/* End of enumerations -----------------------------------------------------*/


#define IS_LPD_ALL_PERIPH(PERIPH) ((PERIPH) == LPD)

/**
 * @}
 */
/* End of constants -----------------------------------------------------*/

/** @defgroup LPD_Exported_Struct LPD Exported Struct
 * @{
 */


/**
 * @}
 */
/* End of struct -----------------------------------------------------*/

/** @addtogroup LPD_Exported_Functions LPD Exported Functions
 * @{
 */

/* LPD Base functions ********************************************************/
void LPD_DeInit ( LPD_TypeDef* LPDx );
void LPD_Cmd ( LPD_TypeDef *LPDx, FunctionalState NewState );
void LPD_VLPDConfig ( LPD_TypeDef* LPDx, LPD_IS_TypeDef LPD_VLPD );

/* Interrupts and flags  ***********************************************/
void LPD_ITConfig ( LPD_TypeDef* LPDx, uint16_t LPD_IT, FunctionalState NewState );
FlagStatus LPD_GetFlagStatus ( LPD_TypeDef* LPDx, uint32_t LPD_FLAG );
void LPD_ClearFlag ( LPD_TypeDef* LPDx, uint32_t LPD_FLAG );
/**
 * @}
 */
/* End of exported functions --------------------------------------------------*/

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
