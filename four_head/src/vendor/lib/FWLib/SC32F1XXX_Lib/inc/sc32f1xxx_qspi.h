/**
 ******************************************************************************
 * @file    SC32f1xxx_qspi.h
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   Header file of QSPI module.
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
#ifndef __sc32r803_QSPI_H
#define __sc32r803_QSPI_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "sc32r803.h"
#include "sc32.h"
#include "sc32f1xxx_rcc.h"


/** @defgroup QSPI_SShift_TypeDef QSPI SShift
 * @{
 */
typedef enum
{
    QSPI_SShift_OFF	= ( uint32_t ) ( 0x00U << TWI_QSPIx_CON_SSHIFT_Pos ), /*!Sampling shift off<*/
    QSPI_SShift_HalfClock	= ( uint32_t ) ( 0x01U << TWI_QSPIx_CON_SSHIFT_Pos ), /*!HalfClock Sampling shift open<*/
} QSPI_SShift_TypeDef;	
#define IS_QSPI_SShift(SShift) (((SShift) == QSPI_SShift_OFF) || \
			                         ((SShift) == QSPI_SShift_HalfClock))	
/**
 * @}
 */
/** @defgroup QSPI_Prescaler_TypeDef QSPI Prescaler
 * @{
 */
typedef enum
{
	  QSPI_Prescaler_1 = 0x00U << TWI_QSPIx_CON_QTWCK_Pos,    /*!< Clock division: Fsource/1    */
    QSPI_Prescaler_2 = 0x01U << TWI_QSPIx_CON_QTWCK_Pos,    /*!< Clock division: Fsource/2    */
    QSPI_Prescaler_4 = 0x02U << TWI_QSPIx_CON_QTWCK_Pos,    /*!< Clock division: Fsource/4    */
    QSPI_Prescaler_8 = 0x03U << TWI_QSPIx_CON_QTWCK_Pos,    /*!< Clock division: Fsource/8    */
    QSPI_Prescaler_16 = 0x04U << TWI_QSPIx_CON_QTWCK_Pos,   /*!< Clock division: Fsource/16    */
    QSPI_Prescaler_32 = 0x05U << TWI_QSPIx_CON_QTWCK_Pos,   /*!< Clock division: Fsource/32    */
    QSPI_Prescaler_64 = 0x06U << TWI_QSPIx_CON_QTWCK_Pos,   /*!< Clock division: Fsource/64    */
    QSPI_Prescaler_128 = 0x07U << TWI_QSPIx_CON_QTWCK_Pos,  /*!< Clock division: Fsource/128    */
    QSPI_Prescaler_256 = 0x08U << TWI_QSPIx_CON_QTWCK_Pos,  /*!< Clock division: Fsource/256    */
    QSPI_Prescaler_512 = 0x09U << TWI_QSPIx_CON_QTWCK_Pos,  /*!< Clock division: Fsource/512    */
    QSPI_Prescaler_1024 = 0x0AU << TWI_QSPIx_CON_QTWCK_Pos, /*!< Clock division: Fsource/1024    */
    QSPI_Prescaler_2048 = (uint16_t)(0x0BU << TWI_QSPIx_CON_QTWCK_Pos), /*!< Clock division: Fsource/2048   */
    QSPI_Prescaler_4096 = (uint16_t)(0x0CU << TWI_QSPIx_CON_QTWCK_Pos), /*!< Clock division: Fsource/2048   */
} QSPI_Prescaler_TypeDef;	
#define IS_QSPI_PRESCALER(PRESCALER) (((PRESCALER) == QSPI_Prescaler_1) || \
                                     ((PRESCALER) == QSPI_Prescaler_2) || \
                                     ((PRESCALER) == QSPI_Prescaler_4) || \
                                     ((PRESCALER) == QSPI_Prescaler_8) || \
                                     ((PRESCALER) == QSPI_Prescaler_16) || \
                                     ((PRESCALER) == QSPI_Prescaler_32) || \
                                     ((PRESCALER) == QSPI_Prescaler_64) || \
                                     ((PRESCALER) == QSPI_Prescaler_128) || \
                                     ((PRESCALER) == QSPI_Prescaler_256) || \
                                     ((PRESCALER) == QSPI_Prescaler_512) || \
                                     ((PRESCALER) == QSPI_Prescaler_1024)|| \
                                     ((PRESCALER) == QSPI_Prescaler_2048) || \
                                     ((PRESCALER) == QSPI_Prescaler_4096) )
/**
 * @}
 */
/** @defgroup QSPI_DWidth_TypeDef QSPI DWidth
 * @{
 */
typedef enum
{
  	QSPI_DWidth_8bit	= ( uint32_t ) ( 0x00U << TWI_QSPIx_CON_DWIDTH_Pos ), /*!Transmitted data width:8 bit<*/
  	QSPI_DWidth_16bit	= ( uint32_t ) ( 0x01U << TWI_QSPIx_CON_DWIDTH_Pos ), /*!Transmitted data width:16 bit<*/
  	QSPI_DWidth_24bit	= ( uint32_t ) ( 0x02U << TWI_QSPIx_CON_DWIDTH_Pos ), /*!Transmitted data width:24 bit<*/
  	QSPI_DWidth_32bit	= ( uint32_t ) ( 0x03U << TWI_QSPIx_CON_DWIDTH_Pos ), /*!Transmitted data width:32 bit<*/
} QSPI_DWidth_TypeDef;	
#define IS_QSPI_DWidth(DWidth) (((DWidth) == QSPI_DWidth_8bit) || \
                                ((DWidth) == QSPI_DWidth_16bit) || \
                                ((DWidth) == QSPI_DWidth_24bit) || \
			                         ((DWidth) == QSPI_DWidth_32bit))
/**
 * @}
 */
/** @defgroup QSPI_LMode_TypeDef QSPI LMode
 * @{
 */
#define  QSPI_LMode_0Line	 ( uint32_t )( 0x00U << TWI_QSPIx_CON_LMODE_Pos )   /*!Transmitted line :No data<*/
#define  QSPI_LMode_1Line	 ( uint32_t )( 0x01U << TWI_QSPIx_CON_LMODE_Pos )   /*!Transmitted line :1 line<*/
#define  QSPI_LMode_2Line	 ( uint32_t )( 0x02UL << TWI_QSPIx_CON_LMODE_Pos )  /*!Transmitted line :2 line<*/
#define  QSPI_LMode_4Line	 ( uint32_t )( 0x03UL << TWI_QSPIx_CON_LMODE_Pos )  /*!Transmitted line :4 line<*/
#define IS_QSPI_LMode(LMode) (((LMode) == QSPI_LMode_0Line) || \
                              ((LMode) == QSPI_LMode_1Line) || \
                              ((LMode) == QSPI_LMode_2Line) || \
			                        ((LMode) == QSPI_LMode_4Line))
/**
 * @}
 */
/** @defgroup QSPI_Mode_TypeDef QSPI Mode
 * @{
 */
typedef enum
{
  	QSPI_Mode_SPI	= ( uint32_t ) ( 0x01U << TWI_QSPIx_CON_MODE_Pos ), /*!Work mode:SPI <*/
  	QSPI_Mode_QSPI	= ( uint32_t ) ( 0x02U << TWI_QSPIx_CON_MODE_Pos ), /*!Work mode:QSPI <*/
  	QSPI_Mode_QSPISTRT	= ( uint32_t ) (( 0x03U << TWI_QSPIx_CON_MODE_Pos )|TWI_QSPIx_CON_STRTEN|TWI_QSPIx_CON_STRDIR), /*!Work mode:QSPISTRT <*/
  	QSPI_Mode_QSPISTRR	= ( uint32_t ) (( 0x03U << TWI_QSPIx_CON_MODE_Pos )|TWI_QSPIx_CON_STRREN|TWI_QSPIx_CON_STRDIR), /*!Work mode:QSPISTRR <*/
} QSPI_Mode_TypeDef;	
#define IS_QSPI_Mode(Mode) (((LMode) == QSPI_Mode_SPI) || \
                                ((LMode) == QSPI_Mode_QSPI) || \
                                ((LMode) == QSPI_Mode_QSPISTRT) || \
			                         ((LMode) == QSPI_Mode_QSPISTRR))
/**
 * @}
 */
/** @defgroup QSPI_CPMode_TypeDef QSPI CPMode
 * @{
 */
typedef enum
{
    QSPI_CPMode_Low	= ( uint32_t ) (( 0x00U << TWI_QSPIx_CON_CPOL_Pos )|( 0x00U << TWI_QSPIx_CON_CPHA_Pos )), /*!Idle clock low, sampling along the first edge sampling <*/
  	QSPI_CPMode_High	= ( uint32_t ) (( 0x01U << TWI_QSPIx_CON_CPOL_Pos )|( 0x01U << TWI_QSPIx_CON_CPHA_Pos )), /*!Idle clock high, sampling along the second edge<*/

} QSPI_CPMode_TypeDef;	
#define IS_QSPI_CPMode(CPMode) (((CPMode) == QSPI_CPMode_Low) || \
                                ((CPMode) == QSPI_CPMode_High))

/**
 * @}
 */
/** @defgroup QSPI_RW_TypeDef QSPI RW
 * @{
 */
typedef enum
{
    QSPI_RW_Write	= ( uint32_t ) (( 0x00U <<TWI_QSPIx_CON_RW_Pos)), /*!QSPI write requirements<*/
  	QSPI_RW_Read	= ( uint32_t ) (( 0x01U <<TWI_QSPIx_CON_RW_Pos)), /*!QSPI read requirements<*/

} QSPI_RW_TypeDef;	
#define IS_QSPI_RW(RW) (((RW) == QSPI_RW_Write) || \
                        ((RW) == QSPI_RW_Read))
/**
 * @}
 */
/** @defgroup QSPI_CLKONLY_TypeDef QSPI CLKONLY
 * @{
 */
typedef enum
{
    QSPI_CLKONLY_OFF	= ( uint32_t ) (( 0x00U <<TWI_QSPIx_CON_CLKONLY_Pos)), /*!Only send the clock OFF<*/
  	QSPI_CLKONLY_ON	= ( uint32_t ) (( 0x01U <<TWI_QSPIx_CON_CLKONLY_Pos)), /*!Only send the clock ON<*/

} QSPI_CLKONLY_TypeDef;	
#define IS_QSPI_CLKONLY(CLKONLY) (((CLKONLY) == QSPI_CLKONLY_OFF) || \
                        ((CLKONLY) == QSPI_CLKONLY_ON))
/**
 * @}
 */
/** @defgroup QSPI_IT_TypeDef QSPI IT
 * @{
 */
typedef enum
{
  QSPI_IT_INTEN = (uint32_t)TWI_SPIx_IDE_INTEN,	/*!< QSPI Interrupt: QSPI total interruption */
  QSPI_IT_RXNE  = (uint32_t)TWI_SPIx_IDE_RXNEIE,	/*!< QSPI Interrupt: QSPI RX buffer not empty Interrupt */
  QSPI_IT_TB    = (uint32_t)TWI_SPIx_IDE_TBIE,  		/*!< QSPI Interrupt: QSPI TX buffer is empty Interrupt */
  QSPI_IT_RX    = (uint32_t)TWI_SPIx_IDE_RXIE,  		/*!< QSPI Interrupt: Receiving FIFO overflow Interrupt */
  QSPI_IT_RXH   = (uint32_t)TWI_SPIx_IDE_RXHIE,  	/*!< QSPI Interrupt: Receive FIFO over half Interrupt */
  QSPI_IT_TXH   = (uint32_t)TWI_SPIx_IDE_TXHIE,  	/*!< QSPI Interrupt: Transmit FIFO under half Interrupt */
  QSPI_IT_QTWIE = (uint32_t)TWI_SPIx_IDE_QTWIE,	/*!< QSPI Interrupt: SPI total interruption */	
} QSPI_IT_TypeDef;

#define IS_QSPI_IT(IT) (((IT) == QSPI_IT_INTEN) || \
                       ((IT) == QSPI_IT_RXNE) || \
                       ((IT) == QSPI_IT_TB) || \
                       ((IT) == QSPI_IT_RX) || \
											 ((IT) == QSPI_IT_RXH)|| \
                       ((IT) == QSPI_IT_TXH)|| \
                       ((IT) == QSPI_IT_QTWIE))
/**
 * @}
 */
/** @defgroup QSPI_FLAG_TypeDef QSPI FLAG
 * @{
 */
typedef enum
{
  QSPI_Flag_RINEIF = (uint32_t)TWI_SPIx_STS_RXNEIF, /*!< QSPI Flag: QSPI RX buffer not empty flag */
  QSPI_Flag_TXEIF  = (uint32_t)TWI_SPIx_STS_TXEIF, /*!< QSPI Flag: TX buffer is empty flag */
  QSPI_Flag_RXFIF  = (uint32_t)TWI_SPIx_STS_RXFIF, /*!< QSPI Flag: Receiving FIFO overflow flag */
  QSPI_Flag_RXHIF  = (uint32_t)TWI_SPIx_STS_RXHIF, /*!< QSPI Flag: Receive FIFO over half flag */
  QSPI_Flag_TXHIF  = (uint32_t)TWI_SPIx_STS_TXHIF, /*!< QSPI Flag: Transmit FIFO under half flag */
	QSPI_Flag_DLUFIF = (uint32_t)TWI_SPIx_STS_DLUFIF,
  QSPI_Flag_WCOL   = (uint32_t)TWI_SPIx_STS_WCOL, /*!< QSPI Flag: Write conflict flag */
	QSPI_FLAG_QTWIF = (uint8_t)TWI_SPIx_STS_QTWIF, /*!< QSPI1 Flag: Interrupt flag */
	QSPI_Flag_BUSY = (uint32_t)TWI_SPIx_STS_BUSY,
} QSPI_FLAG_TypeDef;

#define IS_QSPI_FLAG(FLAG) (((FLAG) == QSPI_Flag_RINEIF) || \
                           ((FLAG) == QSPI_Flag_TXEIF) || \
                           ((FLAG) == QSPI_Flag_RXFIF) || \
                           ((FLAG) == QSPI_Flag_RXHIF) || \
                           ((FLAG) == QSPI_Flag_TXHIF) || \
                           ((FLAG) == QSPI_Flag_WCOL) || \
                           ((FLAG) == QSPI_FLAG_QTWIF)|| \
                            ((FLAG) == QSPI_Flag_BUSY) )
/**
 * @}
 */
/** @defgroup QSPI_DMAReq_TypeDef QSPI DMAReq
 * @{
 */
typedef enum
{
  QSPI_DMAReq_RX = (uint32_t)TWI_SPIx_IDE_RXDMAEN, /*!< QSPI DMA Request: Receive */
  QSPI_DMAReq_TX = (uint32_t)TWI_SPIx_IDE_TXDMAEN, /*!< QSPI DMA Request: Transmit */
} QSPI_DMAReq_TypeDef;

#define IS_QSPI_DMAREQ(DMAREQ) (((DMAREQ) == QSPI_DMAReq_RX) || \
                               ((DMAREQ) == QSPI_DMAReq_TX))
/**
 * @}
 */
#define IS_QSPI_ALL_PERIPH(PERIPH) (((PERIPH) == QSPI0) || \
																	((PERIPH) == QSPI1))
/* End of constants -----------------------------------------------------*/

/** @defgroup QSPI_Struct QSPI Struct
 * @{
 */

typedef struct
{
			
		uint32_t QSPI_SShift;    /*!< This member configures QSPI.
                                              This parameter can be a value of @ref QSPI_SShift_TypeDef. */
		uint32_t QSPI_Prescaler; /*!< This member configures QSPI.
                                              This parameter can be a value of @ref QSPI_Prescaler_TypeDef. */
		uint32_t QSPI_DWidth;     /*!< This member configures QSPI.
                                              This parameter can be a value of @ref QSPI_DWidth_TypeDef. */
		uint32_t QSP_LMode;        /*!< This member configures QSPI.
                                              This parameter can be a value of @ref QSPI_LMode_TypeDef. */
		uint32_t QSPI_Mode; /*!< This member configures QSPI.
                                              This parameter can be a value of @ref QSPI_Mode_TypeDef. */
	
		uint32_t QSPI_CPMode;    /*!< This member configures QSPI.
                                              This parameter can be a value of @ref QSPI_CPMode_TypeDef. */
		uint32_t QSPI_RW;  /*!< This member configures QSPI.
                                              This parameter can be a value of @ref QSPI_RW_TypeDef. */
		uint32_t QSPI_CLKONLY;  /*!< This member configures QSPI.
                                              This parameter can be a value of @ref QSPI_CLKONLY_TypeDef. */


} QSPI_InitTypeDef;

/**
 * @}
 */



/* QSPI Base functions ********************************************************/
void QSPI_DeInit(QSPI_TypeDef* QSPIx);
void QSPI_StructInit(QSPI_InitTypeDef* QSPI_InitStruct);
void QSPI_Init(QSPI_TypeDef* QSPIx,QSPI_InitTypeDef* QSPI_InitStruct);
void QSPI_Cmd(QSPI_TypeDef* QSPIx, FunctionalState NewState);
///* Calibration functions ******************************************************/
void QSPI_SetDataLength(QSPI_TypeDef *QSPIx,QSPI_DWidth_TypeDef QSPI_DWidth);
void QSPI_LModeSelect(QSPI_TypeDef *QSPIx,uint32_t QSPI_LMode);
void QSPI_ModeSelect(QSPI_TypeDef *QSPIx,QSPI_Mode_TypeDef QSPI_Mode);
void QSPI_SetDataLength(QSPI_TypeDef *QSPIx,QSPI_DWidth_TypeDef QSPI_DWidth);
void QSPI_Write_ComSet( QSPI_TypeDef *QSPIx, uint32_t LMODE, uint32_t DWIDTH,uint32_t CLKONLY);
void QSPI_SendData8(QSPI_TypeDef *QSPIx,uint8_t Data);
void QSPI_SendData16(QSPI_TypeDef *QSPIx,uint16_t Data);
void QSPI_SendData32(QSPI_TypeDef *QSPIx,uint32_t Data);
void QSPI_SendDataFIFO (QSPI_TypeDef *QSPIx, uint32_t* Data, uint16_t length );
void QSPI_SendMultipleData(QSPI_TypeDef *QSPIx, uint32_t *buf, uint32_t length);
void QSPI_Read_ComSet(QSPI_TypeDef *QSPIx, uint32_t LMODE, uint32_t DWIDTH, uint32_t length);	
void QSPI_CLKONLYSet(QSPI_TypeDef *QSPIx, uint32_t CLKONLY);
uint8_t QSPI_ReceiveData8(QSPI_TypeDef *QSPIx);
uint16_t QSPI_ReceiveData16(QSPI_TypeDef *QSPIx);
uint32_t QSPI_ReceiveData32(QSPI_TypeDef *QSPIx);
void QSPI_Receivelen(QSPI_TypeDef *QSPIx,uint32_t datalen);
void QSPI_ReceiveDataFIFO (QSPI_TypeDef *QSPIx, uint32_t* Data, uint16_t length );
void QSPI_ReceiveMultipleData(QSPI_TypeDef *QSPIx,uint32_t *buf, uint32_t length);
/* Interrupts and flags management functions  **********************************************/
void QSPI_ITConfig(QSPI_TypeDef* QSPIx,uint32_t QSPI_IT, FunctionalState NewState);
FlagStatus QSPI_GetFlagStatus ( QSPI_TypeDef* QSPIx, QSPI_FLAG_TypeDef QSPI_FLAG );
void QSPI_ClearFlag (QSPI_TypeDef* QSPIx, uint32_t QSPI_FLAG );
void QSPI_DMACmd ( QSPI_TypeDef* QSPIx, uint16_t QSPI_DMAReq, FunctionalState NewState );
/**
 * @}
 */
/* End of exported functions --------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif
