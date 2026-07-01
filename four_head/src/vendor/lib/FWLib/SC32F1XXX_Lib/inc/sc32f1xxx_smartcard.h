/**
 ******************************************************************************
 * @file    sc32f1xxx_smartcard.h
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   Header file of Smart_Card module.
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
 *  COPYRIGHT 202 SinOne Microelectronics
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __sc32f1xxx_smartcard_H
#define __sc32f1xxx_smartcard_H

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

/** @addtogroup Smart_Card
 * @{
 */

/* Exported enumerations ------------------------------------------------------------*/
/** @defgroup Smart_Card_Exported_Enumerations Smart_Card Exported Enumerations
 * @{
 */

/** @brief SmartCard_PCS Smart_Card Parity Selection
 * @{
 */
typedef enum
{
    SC_PCS_NoneParity    = ( uint16_t ) ( 0x00 << SC_CON_PCS_Pos ), /*!< Parity selection: No parity (checking) */
    SC_PCS_EvenParity    = ( uint16_t ) ( 0x01 << SC_CON_PCS_Pos ), /*!< Parity selection: Even parity */
} SmartCard_PCS_TypeDef;
#define IS_SMARTCARD_PCS(PCS) (((PCS) == SC_PCS_NoneParity) ||  \
															 ((PCS) == SC_PCS_EvenParity))
/**
 * @}
 */

/** @brief SmartCard_TRER Control for retransmission due to data transmission/reception parity check error
 * @{
 */
typedef enum
{
    SC_TRER_SetITFlag    = ( uint32_t ) ( 0x00 << SC_CON_TRE_Pos ), /*!< Set interrupt flag directly upon parity check error */
    SC_TRER_LowLevelACK  = ( uint32_t ) ( 0x01 << SC_CON_TRE_Pos ), /*!< An error in the received data parity check will trigger the transmission of a low-level acknowledgment. 
																																				 If a low-level acknowledgment is received after sending data, the data will be resent */
} SmartCard_TRER_TypeDef;
#define IS_SMARTCARD_TRER(TRER) (((TRER) == SC_TRER_SetITFlag) ||  \
																 ((TRER) == SC_TRER_LowLevelACK))
/**
 * @}
 */

/** @brief SmartCard_CONS Smart_Card coding mode control
 * @{
 */
typedef enum
{
    SC_CONS_Positive  = ( uint32_t ) ( 0x00 << SC_CON_CONS_Pos ), /*!< Positive convention, LSB (Least Significant Bit) transmission, positive logic level */
    SC_CONS_Negative  = ( uint32_t ) ( 0x01 << SC_CON_CONS_Pos ), /*!< Reverse convention, MSB (Most Significant Bit) transmission, inverted logic level */
} SmartCard_CONS_TypeDef;
#define IS_SMARTCARD_CONS(CONS) (((CONS) == SC_CONS_Positive) ||  \
												       	 ((CONS) == SC_CONS_Negative))
/**
 * @}
 */

/** @brief SmartCard_ERS Smart_Card selection of stop and error signal duration
 * @{
 */
typedef enum
{
    SC_ERS_2ETU     = ( uint32_t ) ( 0x00 << SC_CON_ERS_Pos ), /*!< Both the Stop and Error Signal durations are 2 ETUs */
    SC_ERS_2ETU_    = ( uint32_t ) ( 0x01 << SC_CON_ERS_Pos ), /*!< Both the Stop and Error Signal durations are 2 ETUs */
		SC_ERS_1_5ETU   = ( uint32_t ) ( 0x02 << SC_CON_ERS_Pos ), /*!< Both the Stop and Error Signal durations are 1.5 ETUs */
    SC_ERS_1ETU     = ( uint32_t ) ( 0x03 << SC_CON_ERS_Pos ), /*!< Both the Stop and Error Signal durations are 1 ETUs */
} SmartCard_ERS_TypeDef;
#define IS_SMARTCARD_ERS(ERS) (((ERS) == SC_ERS_2ETU) ||  \
												       ((ERS) == SC_ERS_2ETU_) ||  \
												       ((ERS) == SC_ERS_1_5ETU) ||  \
												       ((ERS) == SC_ERS_1ETU))
/**
 * @}
 */

/** @brief SmartCard_TREN Smart_Card transmit/receive enable
 * @{
 */
typedef enum
{
    SC_TREN_RXEN     = ( uint32_t ) ( 0x00 << SC_CON_TREN_Pos ), /*!< Receive enabled, transmit disabled */
    SC_TREN_TXEN     = ( uint32_t ) ( 0x01 << SC_CON_TREN_Pos ), /*!< Transmit enabled, receive disabled. After sending a complete frame of data, the interface 
																																			will release SC_DAT and begin detecting the Error Signal on the stop bit */
} SmartCard_TREN_TypeDef;
#define IS_SMARTCARD_TREN(TREN) (((TREN) == SC_TREN_RXEN) ||  \
												         ((TREN) == SC_TREN_TXEN))
/**
 * @}
 */

/** @brief SmartCard_CKEN Smart_Card clock output enable
 * @{
 */
typedef enum
{
    SC_CKEN_Disable     = ( uint32_t ) ( 0x00 << SC_CON_CKEN_Pos ), /*!< Disable clock output */
    SC_CKEN_Enable      = ( uint32_t ) ( 0x01 << SC_CON_CKEN_Pos ), /*!< Enable clock output */
} SmartCard_CKEN_TypeDef;
#define IS_SMARTCARD_CKEN(CKEN) (((CKEN) == SC_CKEN_Disable) ||  \
												         ((CKEN) == SC_CKEN_Enable))
/**
 * @}
 */

/** @brief Smart_Card_PinRemap Smart_Card Pin Remap
 * @{
 */
typedef enum
{
    SC_PinRemap_Default = ( uint32_t ) ( 0x00 << SC_CON_SPOS_Pos ), /*!< SC Pin Remap: Disable */
    SC_PinRemap_A       = ( uint32_t ) ( 0x01 << SC_CON_SPOS_Pos ), /*!< SC Pin Remap: Remap mode A */
} SmartCard_PinRemap_TypeDef;

#define IS_SMARTCARD_PINREMAP(PINREMAP) (((PINREMAP) == SC_PinRemap_Default) ||  \
																         ((PINREMAP) == SC_PinRemap_A))
/**
 * @}
 */

/** @brief Smart_Card_IT Smart_Card Interrupt
 * @{
 */
typedef enum
{
    SC_IT_INTEN   = ( uint16_t )SC_IDE_INTEN,	/*!< Smart_Card Interrupt: Smart_Card interrupt */
    SC_IT_RXIE    = ( uint16_t )SC_IDE_RXIE,	/*!< Smart_Card Receive interrupt enable control */
    SC_IT_TXIE    = ( uint16_t )SC_IDE_TXIE,	/*!< Smart_Card Transmit interrupt enable control */
    SC_IT_ERRIE   = ( uint16_t )SC_IDE_ERRIE,	/*!< Smart_Card Communication exception interrupt enable */
} SmartCard_IT_TypeDef;
#define IS_SMARTCARD_IT(IT) ((((IT) & (uint16_t)0xFFF0) == 0x00) && ((IT) != (uint16_t)0x0000))
/**
 * @}
 */

/** @brief Smart_Card_FLAG Smart_Card Flag
 * @{
 */
typedef enum
{
    SC_Flag_RC      = ( uint16_t )SC_STS_RC,	  /*!< Smart_Card receive completion Flag */
    SC_Flag_TC      = ( uint16_t )SC_STS_TC,    /*!< Smart_Card transmission completion Flag */
    SC_Flag_ROVF    = ( uint16_t )SC_STS_ROVF,	/*!< Smart_Card receive overflow Flag */
    SC_Flag_FER     = ( uint16_t )SC_STS_FER,   /*!< Smart_Card receive frame error Flag */
    SC_Flag_WTER    = ( uint16_t )SC_STS_WTER,	/*!< Smart_Card wait timeout Flag */
    SC_Flag_RPER    = ( uint16_t )SC_STS_RPER,	/*!< Smart_Card receive data parity error Flag */	
		SC_Flag_TPER    = ( uint16_t )SC_STS_TPER,	/*!< Smart_Card transmit data parity error Flag */
    SC_Flag_RBUSY   = ( uint16_t )SC_STS_RBUSY,	/*!< Smart_Card data reception busy status */	
		SC_Flag_TBUSY   = ( uint16_t )SC_STS_TBUSY,	/*!< Smart_Card data transmission busy status */
    SC_Flag_WTRT    = ( uint16_t )SC_STS_WTRT,	/*!< Smart_Card waiting for data retransmission status */	
} SmartCard_FLAG_TypeDef;
#define IS_SMARTCARD_FLAG(FLAG) ((((FLAG) & (uint16_t)0xF880) == 0x00) && ((FLAG) != (uint16_t)0x0000))
/**
 * @}
 */

/**
 * @}
 */
/* End of enumerations -----------------------------------------------------*/


#define IS_SMARTCARD_ALL_PERIPH(PERIPH) ((PERIPH) == Smart_Card)

/**
 * @}
 */
/* End of constants -----------------------------------------------------*/

/** @defgroup Smart_Card_Exported_Struct Smart_Card Exported Struct
 * @{
 */

/** @brief Smart_Card Initialization Configuration Structure definition
 * @{
 */
typedef struct
{
    uint32_t SC_Parity; /*!< Parity check selection of Smart_Card.
																  This parameter can be a value of @ref SmartCard_PCS_TypeDef */

    uint32_t SC_TransReadError; /*!< Control for data transmission/reception error checking and retransmission of Smart_Card.
																	This parameter can be a value of @ref SmartCard_TRER_TypeDef */

    uint32_t SC_EncodeMethod; /*!< Control of encoding method for Smart_Card.
																	This parameter can be a value of @ref SmartCard_CONS_TypeDef */

    uint32_t SC_ErrorETU; /*!< Selection of Stop and Error Signal Duration.
																	This parameter can be a value of @ref SmartCard_ERS_TypeDef */

		uint32_t SC_TransReceEn; /*!< Transmit/Receive Enable for Smart_Card.
																	This parameter can be a value of @ref SmartCard_TREN_TypeDef */
	
		uint32_t SC_ClockOutEn; /*!< Clock Output Enable for Smart_Card.
																	This parameter can be a value of @ref SmartCard_CKEN_TypeDef */
	
} SmartCard_InitTypeDef;
/**
 * @}
 */

/** @brief SC Time base Configuration Structure definition
 * @{
 */
typedef struct
{
    uint32_t SC_ExtraGuardTime; /*!< Extended Protection time of Smart_Card.
																  This parameter can be a value of 0x00~0xFF */

    uint32_t SC_ClockCycle; /*!< Clock cycle of Smart_Card.
																	This parameter can be a value of 0x00~0x1F */

    uint32_t SC_ETUClockCycle; /*!< ETU clock cycle of Smart_Card.
																	This parameter can be a value of 0x00~0xFFF */
	
} SmartCard_BaudInitTypeDef;
/**
 * @}
 */

/**
 * @}
 */
/* End of struct -----------------------------------------------------*/

/** @addtogroup Smart_Card_Exported_Functions Smart_Card Exported Functions
 * @{
 */

/* Smart_Card Base functions *************************************************************************/
void Smart_Card_DeInit ( Smart_Card_TypeDef* SCx );
void Smart_Card_Init ( Smart_Card_TypeDef* SCx, SmartCard_InitTypeDef* SC_InitStruct );
void Smart_Card_StructInit ( SmartCard_InitTypeDef* SC_InitStruct );
void Smart_Card_Cmd ( Smart_Card_TypeDef *SCx, FunctionalState NewState );
void Smart_Card_CKEN_Cmd ( Smart_Card_TypeDef* SCx, FunctionalState NewState );
void Smart_Card_TREN_Cmd ( Smart_Card_TypeDef* SCx, SmartCard_TREN_TypeDef NewState );
void Smart_Card_BaudConfig ( Smart_Card_TypeDef* SCx, SmartCard_BaudInitTypeDef* SC_BaudInitStruct );
void Smart_Card_BaudStructInit ( SmartCard_BaudInitTypeDef* SC_BaudInitStruct );

/* Data transfers functions ******************************************************************/
void Smart_Card_SendData ( Smart_Card_TypeDef* SCx, uint8_t Data );
uint8_t Smart_Card_ReceiveData ( Smart_Card_TypeDef* SCx );

/* Pin remap management functions  ***********************************************************/
void Smart_Card_PinRemapConfig ( Smart_Card_TypeDef* SCx, SmartCard_PinRemap_TypeDef SC_Remap );

/* Interrupts and flags management functions  ************************************************/
void Smart_Card_ITConfig ( Smart_Card_TypeDef* SCx, uint16_t SC_IT, FunctionalState NewState );
FlagStatus Smart_Card_GetFlagStatus ( Smart_Card_TypeDef* SCx, uint32_t SC_FLAG );
void Smart_Card_ClearFlag ( Smart_Card_TypeDef* SCx, uint32_t SC_FLAG );


/**
 * @}
 */
/* End of exported functions --------------------------------------------------*/

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif
