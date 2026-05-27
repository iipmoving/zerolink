/**
 ******************************************************************************
 * @file    sc32f1xxx_aes.h
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   Header file of AES module.
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
#ifndef __sc32f1xxx_aes_H
#define __sc32f1xxx_aes_H

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

/** @addtogroup AES
 * @{
 */

/** @defgroup AES_Enumerations AES Enumerations
 * @{
 */
 
/** @brief AES_Mode AES work mode
 * @{
 */ 
typedef enum
{
    AES_MODE_ENCRYPT = (uint16_t)0x0000, /*!< AES Work Mode: Encrypt  */
    AES_MODE_DECRYPT = (uint16_t)AES_CFG_MODE, /*!< AES Work Mode: Decrypt  */
} AES_MODE_TypeDef;

#define IS_AES_MODE(MODE)	(((MODE) == AES_MODE_ENCRYPT) ||  \
												  ((MODE) == AES_MODE_DECRYPT))
/**
 * @}
 */

/** @brief AES_KEYSIZE AES Key Size
 * @{
 */ 
typedef enum
{
    AES_KEYSIZE_128B = (uint16_t)0x0080, /*!< AES KEY Size: 128 bit  */
    AES_KEYSIZE_192B = (uint16_t)0x00c0, /*!< AES KEY Size: 192 bit  */
    AES_KEYSIZE_256B = (uint16_t)0x0100, /*!< AES KEY Size: 256 bit  */
} AES_KEYSIZE_TypeDef;

#define IS_AES_KEYSIZE(KEYSIZE)	(((KEYSIZE) == AES_KEYSIZE_128B) ||  \
												   ((KEYSIZE) == AES_KEYSIZE_192B) ||  \
                          ((KEYSIZE) == AES_KEYSIZE_256B))
/**
 * @}
 */

/** @brief AES_IT AES Interrupt
 * @{
 */
typedef enum
{
    AES_IT_INTEN = ( uint16_t ) AES_CFG_CCFIE,	/*!< AES Interrupt: AES Interrupt */
} AES_IT_TypeDef;

#define IS_AES_IT(IT) ((IT) == AES_IT_INTEN)
/**
 * @}
 */

/** @brief AES_Flag AES Flag
 * @{
 */
typedef enum
{
    AES_Flag_BUSY  = ( uint16_t ) ASE_STS_BUSY, /*!< AES Flag: In the process of calculation */
    AES_Flag_CCFIF = ( uint16_t ) ASE_STS_CCFIF, /*!< AES Flag: Calculation completed */
} AES_Flag_TypeDef;

#define IS_GET_AES_FLAG(FLAG) (((FLAG) == AES_Flag_BUSY) ||  \
                               ((FLAG) == AES_Flag_CCFIF)) 
/**
 * @}
 */

/**
 * @}
 */
/* End of enumerations -----------------------------------------------------*/

/** @addtogroup AES_Functions AES Functions
 * @{
 */

/* AES Base functions ********************************************************/
void AES_DeInit(void);

/* AES Encryption and decryption of functions ********************************************************/
ErrorStatus  AES_ECBModeConfig(AES_MODE_TypeDef AES_Mode,uint8_t *key,AES_KEYSIZE_TypeDef keysize,uint8_t *input, uint32_t length, uint8_t  *output);
ErrorStatus  AES_CBCModeConfig ( AES_MODE_TypeDef AES_Mode, uint8_t *key, AES_KEYSIZE_TypeDef keysize,uint8_t* Vector,uint8_t *input, uint32_t length, uint8_t  *output );
ErrorStatus  AES_CTRModeConfig ( AES_MODE_TypeDef AES_Mode, uint8_t *key, AES_KEYSIZE_TypeDef keysize,uint8_t* Vector,uint8_t *input, uint32_t length, uint8_t  *output );

/* AES Interrupts functions ********************************************************/
void AES_ITConfig ( uint16_t AES_IT, FunctionalState NewState );
FlagStatus AES_GetFlagStatus ( AES_Flag_TypeDef AES_FLAG );
void AES_ClearFlag ( uint16_t AES_FLAG );
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
