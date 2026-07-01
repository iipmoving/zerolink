/**
 ******************************************************************************
 * @file    sc32f1xxx_lpcounter.h
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   Header file of LPC module.
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
#ifndef __sc32f1xxx_LPC_H
#define __sc32f1xxx_LPC_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "sc32f1xxx.h"
#include "sc32.h"

/** @addtogroup sc32f1xxx_StdPeriph_Driver
 * @{
 */

/** @addtogroup LPC
 * @{
 */

/** @defgroup LPC_Enumerations LPC Enumerations
 * @{
 */
 
/** @brief LPC_INTSEL LPC INTSEL
 * @{
 */
typedef enum
{
    LPC_INTSEL_INTA = 0x00U,   /*!< LPC Select: INTA    */
    LPC_INTSEL_INTB = 0x01U,   /*!< LPC Select: INTB    */
} LPC_INTSEL_Typedef;

#define IS_LPC_INTSEL(INTSEL) (((INTSEL) == LPC_INTSEL_A) || \
                               ((INTSEL) == LPC_INTSEL_B))
/**
 * @}
 */

/** @brief LPC_Trigger LPC Trigger
 * @{
 */
typedef enum
{
    LPC_Trigger_Rising         = ( uint8_t ) 0x00,  /*!< LPC Interrupt: Rising edge capture */
    LPC_Trigger_Falling        = ( uint8_t ) 0x01,	/*!< LPC Interrupt: Falling edge capture */
} LPC_Trigger_TypeDef;

#define IS_LPC_TRIGGER(TRIGGER)	(((TRIGGER) == LPC_Trigger_Rising) || \
																 ((TRIGGER) == LPC_Trigger_Falling))
/**
 * @}
 */

/** @brief LPC_InitDirection LPC init direction
 * @{
 */
typedef enum
{
    LPC_INITDIR_Forward  = ( uint8_t ) 0x00, /*!< LPC Direction: Initialization direction is forward rotation */
    LPC_INITDIR_Reversal = ( uint8_t ) 0x01, /*!< LPC Direction: Initialization direction is reversed */	
} LPC_InitDirection_TypeDef;

#define IS_LPC_INITDIR(INITDIR)	(((INITDIR) == LPC_INITDIR_Forward) || \
																 ((INITDIR) == LPC_INITDIR_Reversal))
/**
 * @}
 */

/** @brief LPC_IT LPC Interrupt
 * @{
 */
typedef enum
{
    LPC_IT_INTEN = ( uint8_t ) LPC_IDE_INTEN, /*!< LPC Interrupt: LPC interrupt enable */			
    LPC_IT_DIRIE = ( uint8_t ) LPC_IDE_DIRIE,	/*!< LPC Interrupt: Direction jump interrupt enable */					
	  LPC_IT_CAIE = ( uint8_t ) LPC_IDE_CAIE,   /*!< LPC Interrupt: LPCA_CNT Count Overflow Interrupt Enable */			
    LPC_IT_CBIE = ( uint8_t ) LPC_IDE_CBIE,	  /*!< LPC Interrupt: LPCB_CNT Count Overflow Interrupt Enable */			
    LPC_IT_RFAIE = ( uint8_t ) LPC_IDE_RFAIE,	/*!< LPC Interrupt: INTA input valid edge interrupt enable */			
    LPC_IT_RFBIE = ( uint8_t ) LPC_IDE_RFBIE,	/*!< LPC Interrupt: INTB input valid edge interrupt enable */			
} LPC_IT_TypeDef;

#define IS_LPC_IT(IT) (((IT) == LPC_IT_INTEN) ||  \
                       ((IT) == LPC_IT_DIRIE) ||   \
                       ((IT) == LPC_IT_CAIE) ||   \
                       ((IT) == LPC_IT_CBIE) ||   \
                       ((IT) == LPC_IT_RFAIE) ||   \
                       ((IT) == LPC_IT_RFBIE)) 
/**
 * @}
 */

/** @brief LPC_Flag  LPC Flag
 * @{
 */
typedef enum
{
    LPC_FLAG_FLIPIF = ( uint8_t ) LPC_STS_FLIPIF,	/*!< LPC Flag: Direction Flip Flag */		
    LPC_FLAG_DIRIF = ( uint8_t ) LPC_STS_DIRIF,		/*!< LPC Flag: Waveform direction indication */			
	  LPC_FLAG_CAIF = ( uint8_t ) LPC_STS_CAIF,     /*!< LPC Flag: LPCA count overflow flag */		
    LPC_FLAG_CBIF = ( uint8_t ) LPC_STS_CBIF,	    /*!< LPC Flag: LPCB count overflow flag */		
    LPC_FLAG_RFAIF = ( uint8_t ) LPC_STS_RFAIF,	  /*!< LPC Flag: INTA input valid along the detected flag */		
    LPC_FLAG_RFBIF = ( uint8_t ) LPC_STS_RFBIF,		/*!< LPC Flag: INTB input valid along the detected flag */		
} LPC_Flag_TypeDef;

#define IS_GET_LPC_FLAG(FLAG) (((FLAG) == LPC_FLAG_FLIPIF) ||  \
                               ((FLAG) == LPC_FLAG_DIRIF)  ||  \
                               ((FLAG) == LPC_FLAG_CAIF)   ||  \
                               ((FLAG) == LPC_FLAG_CBIF)   ||  \
                               ((FLAG) == LPC_FLAG_RFAIF)  ||  \
                               ((FLAG) == LPC_FLAG_RFBIF)) 
/**
 * @}
 */

/**
 * @}
 */
/* End of constants -----------------------------------------------------*/


/** @defgroup LPC_Struct LPC Struct
 * @{
 */

/** @brief LPC_InitTypeDef LPC Init Functions definition
 * @{
 */
typedef struct
{
	 uint8_t Padding[3];  /*!< Add 3 bytes of padding to avoid the -Wpadded warning. */
	
   uint8_t  LPC_INTATrigger;/*!<  Specifies the INTA trigger edge.
                                      This parameter can be a value of @ref LPC_Trigger_TypeDef */
	 uint8_t  LPC_INTBTrigger;/*!<  Specifies the INTB trigger edge.
                                      This parameter can be a value of @ref LPC_Trigger_TypeDef */
	 uint8_t  LPC_INTAOverValue;/*!<  Specifies the INTA Trigger value of overflow event.
                                      This parameter can be a number between Min_Data = 0x00 and Max_Data = 0xFF. */
	 uint8_t  LPC_INTBOverValue;/*!<  Specifies the INTB Trigger value of overflow event.
                                      This parameter can be a number between Min_Data = 0x00 and Max_Data = 0xFF. */
	 uint8_t  LPC_InitDir;/*!<  Specifies the LPC Initial direction.
                                      This parameter can be a value of @ref LPC_InitDirection_TypeDef. */
	 
	 FunctionalState LPC_DisDirEN;/*!<  Is the initial direction counting function enabled.
                                      This parameter can be a value of @ref FunctionalState. */
}LPC_InitTypeDef;

/**
 * @}
 */

/**
 * @}
 */
/* End of struct -----------------------------------------------------*/

/** @addtogroup LPC_Functions LPC Functions
 * @{
 */

/* LPC Base functions ********************************************************/
void LPC_DeInit ( LPC_TypeDef* LPCx );
void LPC_Init(LPC_TypeDef* LPCx,LPC_InitTypeDef* LPC_InitStruct );
void LPC_StructInit ( LPC_InitTypeDef* LPC_InitStruct );
void LPC_Cmd(LPC_TypeDef* LPCx,FunctionalState NewState);
void LPC_TriggerMode (  LPC_TypeDef* LPCx, LPC_INTSEL_Typedef LPC_INTSEL, LPC_Trigger_TypeDef Trigger_Mode);
void LPC_SetOverflowCounter ( LPC_TypeDef* LPCx, LPC_INTSEL_Typedef LPC_INTSEL,uint8_t Counter );
uint8_t  LPC_GetOverflowCounter ( LPC_TypeDef* LPCx, LPC_INTSEL_Typedef LPC_INTSEL );
void LPC_SetCounter ( LPC_TypeDef* LPCx, LPC_INTSEL_Typedef LPC_INTSEL,uint8_t Counter );
uint8_t  LPC_GetCounter ( LPC_TypeDef* LPCx, LPC_INTSEL_Typedef LPC_INTSEL );
/* LPC Interrupts functions ********************************************************/
void LPC_ITConfig(LPC_TypeDef* LPCx,uint8_t LPC_IT,FunctionalState NewState);
FlagStatus LPC_GetFlagStatus(LPC_TypeDef* LPCx,LPC_Flag_TypeDef LPC_FLAG);
void LPC_ClearFlag(LPC_TypeDef* LPCx,uint8_t LPC_FLAG);

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

