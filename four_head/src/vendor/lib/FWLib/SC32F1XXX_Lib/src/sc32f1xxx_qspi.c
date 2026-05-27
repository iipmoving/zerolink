/**
 ******************************************************************************
 * @file    sc32f1xxx_qspi.c
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   QSPI function module
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
/* Includes ------------------------------------------------------------------*/
#if defined(SC32R803)
#include "sc32f1xxx_qspi.h"
/** @defgroup QSPI_Exported_Functions_Group1 Configuration of the QSPI computation unit functions
 *  @brief   Configuration of the QSPI computation unit functions
 *
@verbatim
 ===============================================================================
                     ##### QSPI configuration functions #####
 ===============================================================================
@endverbatim
  * @{
  */

/**
 * @brief  DeInitialize the QSPI peripheral registers to their default reset values.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @retval None
 */
void QSPI_DeInit(QSPI_TypeDef* QSPIx)
{
    assert_param ( IS_QSPIx_ALL_PERIPH ( QSPIx ) );

    if ( QSPIx == QSPI0 )
    {
			/* Enable QSPI reset state */
        RCC_APB0PeriphResetCmd ( RCC_APB0Periph_QTWI0, ENABLE );
			/* DISABLE QSPI reset state */
        RCC_APB0PeriphResetCmd ( RCC_APB0Periph_QTWI0, DISABLE );
    }
    else if ( QSPIx == QSPI1 )
    {
			/* Enable QSPI reset state */
        RCC_APB1PeriphResetCmd ( RCC_APB1Periph_QTWI1, ENABLE );
			/* DISABLE QSPI reset state */
        RCC_APB1PeriphResetCmd ( RCC_APB1Periph_QTWI1, DISABLE );
    }	
}
/**
  * @brief  Fills each QSPI_InitTypeDef member with its default value.
  * @param  QSPI_InitStruct[out]:Pointer to structure QSPI_InitTypeDef, to be initialized.
  * @retval None
  */
void QSPI_StructInit(QSPI_InitTypeDef* QSPI_InitStruct)
{
			/* Set the default configuration */
			QSPI_InitStruct->QSPI_CLKONLY = QSPI_CLKONLY_OFF;
			QSPI_InitStruct->QSPI_CPMode = QSPI_CPMode_Low;
			QSPI_InitStruct->QSPI_DWidth = QSPI_DWidth_8bit;
			QSPI_InitStruct->QSPI_Mode =  QSPI_Mode_SPI;
			QSPI_InitStruct->QSPI_Prescaler = QSPI_Prescaler_1;
			QSPI_InitStruct->QSPI_RW = QSPI_RW_Write;
			QSPI_InitStruct->QSPI_SShift = QSPI_SShift_OFF;
			QSPI_InitStruct->QSP_LMode = QSPI_LMode_0Line;
	
}
/**
 * @brief  Initializes the QSPI peripheral
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  QSPI_InitStruct[out]:Pointer to structure QSPI_InitTypeDef, to be initialized.
 * @retval None
 */
void QSPI_Init(QSPI_TypeDef* QSPIx,QSPI_InitTypeDef* QSPI_InitStruct)
{
	uint32_t tmpreg;
	assert_param (IS_QSPI_SShift(SShift));
	assert_param (IS_QSPI_PRESCALER(PRESCALER));
	assert_param (IS_QSPI_DWidth(DWidth));
  assert_param (IS_QSPI_LMode(LMode) );
	assert_param (IS_QSPI_Mode(Mode));
	assert_param (IS_QSPI_CPMode(CPMode));
	 /*---------------------------- TWI_QSPIx_CON Configuration ------------------------*/
	 /* Get the TWI_QSPIx_CON value */
	tmpreg = QSPIx->QSPI_CON;
	 /* Clear SSHIFT, QTWCK ,DWIDTH , LMODE , MDOE , STRDIR , STRTEN  , QTWEN And so on SPR bits */
	tmpreg &= ~(TWI_QSPIx_CON_SSHIFT|TWI_QSPIx_CON_QTWCK|TWI_QSPIx_CON_DWIDTH|TWI_QSPIx_CON_LMODE|\
	            TWI_QSPIx_CON_MODE|TWI_QSPIx_CON_STRDIR|TWI_QSPIx_CON_STRTEN|TWI_QSPIx_CON_STRREN|\
	            TWI_QSPIx_CON_RW|TWI_QSPIx_CON_CPOL|TWI_QSPIx_CON_CPHA|TWI_QSPIx_CON_DORD|TWI_QSPIx_CON_CLKONLY|\
	            TWI_QSPIx_CON_SMSTR|TWI_QSPIx_CON_SPOS|TWI_QSPIx_CON_QTWEN);
	  /* Configure QSPI: SSHIFT, QTWCK ,DWIDTH , LMODE , MDOE , STRDIR , STRTEN  , QTWEN And so on SPR bits */
	tmpreg |= (QSPI_InitStruct->QSPI_CLKONLY|QSPI_InitStruct->QSPI_CPMode|QSPI_InitStruct->QSPI_DWidth|\
	           QSPI_InitStruct->QSPI_Mode|QSPI_InitStruct->QSPI_Prescaler|QSPI_InitStruct->QSPI_RW|\
	           QSPI_InitStruct->QSPI_SShift|QSPI_InitStruct->QSP_LMode);
	tmpreg |=TWI_QSPIx_CON_SMSTR;
	 /* Write to TWI_QSPIx_CON */
	QSPIx->QSPI_CON = tmpreg;
	
}
/**
 * @brief  Enables or disables the specified QSPI peripheral.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  NewState[in]: new state of the QSPI peripheral.
 *                  - DISABLE:Function disable
 *                  - ENABLE:Function enable
 * @retval None
 */
void QSPI_Cmd(QSPI_TypeDef* QSPIx, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );
		if (NewState != DISABLE)
		{
			/* Enable the selected QSPI interrupt */
			QSPIx->QSPI_CON |= TWI_QSPIx_CON_QTWEN;
		}
		else
		{
			/* Disable the selected QSPI interrupt */
			QSPIx->QSPI_CON  &= ~TWI_QSPIx_CON_QTWEN;
		}
}
/**
 * @brief  Set the QSPI transfer data length.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  DWIDTH[in]: QSPI Work Mode.
 *                  - QSPI_DWidth_8bit:Transmitted data width:8 bit
 *                  - QSPI_DWidth_16bit:Transmitted data width:16 bit
 *                  - QSPI_DWidth_24bit:Transmitted data width:24 bit
 *                  - QSPI_DWidth_32bit:Transmitted data width:32 bit

 * @retval None
 */
void QSPI_SetDataLength(QSPI_TypeDef *QSPIx,QSPI_DWidth_TypeDef QSPI_DWidth)
{
	assert_param (IS_QSPI_DWidth(QSPI_DWidth));
	
	 uint32_t tmpreg;
	/* Get the TWI_QSPIx_CON value */
	tmpreg = QSPIx->QSPI_CON;
	 /* Clear DWIDTH SPR bits */
	tmpreg &= ~(TWI_QSPIx_CON_DWIDTH);
	/* Configure QSPI: LMODE, DWIDTH */
	tmpreg |=   QSPI_DWidth;
	 /* Write to TWI_QSPIx_CON */
	QSPIx->QSPI_CON = tmpreg;
}
/**
 * @brief  Configure the QSPI transfer mode
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  LMODE[in]: QSPI Transmitted line.
 *                  - QSP_LMode_0Line:Transmitted line :0 line
 *                  - QSP_LMode_1Line:Transmitted line :1 line
 *                  - QSP_LMode_2Line:Transmitted line :2 line
 *                  - QSP_LMode_4Line:Transmitted line :4 line
 * @retval None
 */
void QSPI_LModeSelect(QSPI_TypeDef *QSPIx,uint32_t QSPI_LMode)
{
		assert_param (IS_QSPI_DWidth(QSPI_LMode));
	
	 uint32_t tmpreg;
	/* Get the TWI_QSPIx_CON value */
	tmpreg = QSPIx->QSPI_CON;
	 /* Clear LMODE SPR bits */
	tmpreg &= ~(TWI_QSPIx_CON_LMODE);
	/* Configure QSPI: LMODE*/
	tmpreg |=   QSPI_LMode;
	 /* Write to TWI_QSPIx_CON */
	QSPIx->QSPI_CON = tmpreg;
}
/**
 * @brief  Configure the TWI_QSPI  mode
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  LMODE[in]: Work mode.
 *               - QSPI_Mode_SPI:Work mode:SPI 
 *               - QSPI_Mode_QSPI:Work mode:QSPI
 *               - QSPI_Mode_QSPISTRT:Work mode:QSPISTRT
 *               - QSPI_Mode_QSPISTRR:Work mode:QSPISTRR 
 * @retval None
 */
void QSPI_ModeSelect(QSPI_TypeDef *QSPIx,QSPI_Mode_TypeDef QSPI_Mode)
{
		assert_param (IS_QSPI_DWidth(QSPI_Mode));
	
	 uint32_t tmpreg;
	/* Get the TWI_QSPIx_CON value */
	tmpreg = QSPIx->QSPI_CON;
	 /* Clear MODE SPR bits */
	tmpreg &= ~(TWI_QSPIx_CON_MODE);
	/* Configure QSPI: LMODE*/
	tmpreg |=   QSPI_Mode;
	 /* Write to TWI_QSPIx_CON */
	QSPIx->QSPI_CON = tmpreg;
}
/**
 * @brief  Configure QSPI write conditions.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  LMODE[in]: QSPI Transmitted line.
 *                  - QSP_LMode_0Line:Transmitted line :0 line
 *                  - QSP_LMode_1Line:Transmitted line :1 line
 *                  - QSP_LMode_2Line:Transmitted line :2 line
 *                  - QSP_LMode_4Line:Transmitted line :4 line
 * @param  DWIDTH[in]: QSPI Work Mode.
 *                  - QSPI_DWidth_8bit:Transmitted data width:8 bit
 *                  - QSPI_DWidth_16bit:Transmitted data width:16 bit
 *                  - QSPI_DWidth_24bit:Transmitted data width:24 bit
 *                  - QSPI_DWidth_32bit:Transmitted data width:32 bit
 * @param  CLKONLY[in]: Enable or disable QSPI CLKONLY.
 *                  - QSPI_CLKONLY_OFF  Only send the clock OFF
 *                  - QSPI_CLKONLY_ON  Only send the clock ON
 * @retval None
 */
void QSPI_Write_ComSet( QSPI_TypeDef *QSPIx, uint32_t LMODE, uint32_t DWIDTH,uint32_t CLKONLY)
{
  uint32_t tmpreg;
	assert_param (IS_QSPI_SShift(LMODE));
	assert_param (IS_QSPI_PRESCALER(DWIDTH));
	assert_param (IS_QSPI_DWidth(CLKONLY));
	/* Get the TWI_QSPIx_CON value */
	tmpreg = QSPIx->QSPI_CON;
	 /* Clear SSHIFT, QTWCK ,DWIDTH , LMODE , MDOE , RW SPR bits */
	tmpreg &= ~(TWI_QSPIx_CON_DWIDTH|TWI_QSPIx_CON_LMODE|TWI_QSPIx_CON_RW);
	/* Configure QSPI: LMODE, DWIDTH */
	tmpreg |=   LMODE|DWIDTH;
	 /* Write to TWI_QSPIx_CON */
	QSPIx->QSPI_CON = tmpreg;
	/* Configure QSPI: QSPI_CLKONLY */
	if(CLKONLY == QSPI_CLKONLY_ON)                     //  ж ֻ  ʱ   Ƿ  ֹ
	{
		QSPIx->QSPI_CON  |= QSPI_CLKONLY_ON;
	}
	else
	{
		QSPIx->QSPI_CON &= ~(uint32_t)QSPI_CLKONLY_ON;
	}
}

/**
 * @brief  Configure QSPI write conditions.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  Data[in]: QSPI Transmitted Data.
 * @retval None
 */
void QSPI_SendData8(QSPI_TypeDef *QSPIx,uint8_t Data)
{
	 QSPIx->QSPI_DATA = (uint8_t) Data;
}
/**
 * @brief  Configure QSPI write conditions.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  Data[in]: QSPI Transmitted Data.
 * @retval None
 */
void QSPI_SendData16(QSPI_TypeDef *QSPIx,uint16_t Data)
{
	 QSPIx->QSPI_DATA = (uint16_t) Data;
}
/**
 * @brief  Configure QSPI write conditions.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  Data[in]: QSPI Transmitted Data.
 * @retval None
 */
void QSPI_SendData32(QSPI_TypeDef *QSPIx,uint32_t Data)
{
	 QSPIx->QSPI_DATA = (uint32_t) Data;
}
/**
 * @brief  Transmits multiple data through the SPIx peripheral FIFO.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  Data[in]: the datas to transmit.
 * @param  length[in]: the data length to tramsmit.
 * @retval None
 */
void QSPI_SendDataFIFO (QSPI_TypeDef *QSPIx, uint32_t* Data, uint16_t length )
{

    uint16_t tmpNum;
    /* Check the parameters */
    assert_param ( IS_SPI_ALL_PERIPH ( QSPIx ) );
    /* Transmit 24Bit Data */
		if ( ( QSPIx->QSPI_CON & TWI_QSPIx_CON_DWIDTH)>=QSPI_DWidth_24bit)
		{
				for ( tmpNum = 0; tmpNum < length; tmpNum++ )
				{
						QSPIx->QSPI_DATA = ( uint32_t ) Data[tmpNum];
				}
		}
		/* Transmit 16 Bit Data */
		else if( ( QSPIx->QSPI_CON & TWI_QSPIx_CON_DWIDTH)==QSPI_DWidth_16bit)
		{
				for ( tmpNum = 0; tmpNum < length; tmpNum++ )
				{
						QSPIx->QSPI_DATA  = ( uint32_t ) ( * ( ( uint16_t* ) Data + tmpNum ) );
				}
		}
				/* Transmit 8 Bit Data */
		else if( ( QSPIx->QSPI_CON & TWI_QSPIx_CON_DWIDTH)==QSPI_DWidth_8bit)
		{
				for ( tmpNum = 0; tmpNum < length; tmpNum++ )
				{
						QSPIx->QSPI_DATA  = ( uint32_t ) ( * ( ( uint8_t* ) Data + tmpNum ) );
				}
		}


}
/**
 * @brief  Transmits multiple data through the QSPIx peripheral .
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI can be select the QSPI1 peripheral.
 * @param  buf[in]: the datas to transmit.
 * @param  len[in]: the data length to tramsmit.
 * @retval None
 */
void QSPI_SendMultipleData(QSPI_TypeDef *QSPIx, uint32_t *buf, uint32_t length)
{
    uint32_t tmpNum;
    /* Check the parameters */
    assert_param ( IS_SPI_ALL_PERIPH ( QSPIx ) );
    /* Transmit 16Bit Data */
		if ( ( QSPIx->QSPI_CON & TWI_QSPIx_CON_DWIDTH)>=QSPI_DWidth_24bit)
		{
				for ( tmpNum = 0; tmpNum < length; tmpNum++ )
				{
						QSPIx->QSPI_DATA = ( uint32_t ) buf[tmpNum];
					  while(QSPI_GetFlagStatus(QSPI0,QSPI_Flag_BUSY)==0); // ȴ BUSY    
				}
		}
		/* Transmit 16 Bit Data */
		else if( ( QSPIx->QSPI_CON & TWI_QSPIx_CON_DWIDTH)==QSPI_DWidth_16bit)
		{
				for ( tmpNum = 0; tmpNum < length; tmpNum++ )
				{
						QSPIx->QSPI_DATA  = ( uint32_t ) ( * ( ( uint16_t* ) buf + tmpNum ) );
					while(QSPI_GetFlagStatus(QSPI0,QSPI_Flag_BUSY)); // ȴ BUSY    
				}
		}
				/* Transmit 8 Bit Data */
		else if( ( QSPIx->QSPI_CON & TWI_QSPIx_CON_DWIDTH)==QSPI_DWidth_8bit)
		{
				for ( tmpNum = 0; tmpNum < length; tmpNum++ )
				{
						QSPIx->QSPI_DATA  = ( uint32_t ) ( * ( ( uint8_t* ) buf + tmpNum ) );
					 while(QSPI_GetFlagStatus(QSPI0,QSPI_Flag_BUSY)); // ȴ BUSY    
				}
		}	
	
		
		  
		                              
}
/**
 * @brief  Configure QSPI write conditions.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  LMODE[in]: QSPI Transmitted line.
 *                  - QSP_LMode_0Line:Transmitted line :0 line
 *                  - QSP_LMode_1Line:Transmitted line :1 line
 *                  - QSP_LMode_2Line:Transmitted line :2 line
 *                  - QSP_LMode_4Line:Transmitted line :4 line
 * @param  DWIDTH[in]: QSPI Work Mode.
 *                  - QSPI_Mode_SPI:Work mode:SPI 
 *                  -QSPI_Mode_QSPI:Work mode:QSPI
 *                  -QSPI_Mode_QSPISTRT:Work mode:QSPISTRT 
 *                  -QSPI_Mode_QSPISTRR:Work mode:QSPISTRR 
 * @param  CLKONLY[in]: Enable or disable QSPI CLKONLY.
 *                  - QSPI_CLKONLY_OFF  Only send the clock OFF
 *                  - QSPI_CLKONLY_ON  Only send the clock ON
 * @retval None
 */
void QSPI_Read_ComSet(QSPI_TypeDef *QSPIx, uint32_t LMODE, uint32_t DWIDTH, uint32_t length)
{
			uint32_t tmpreg;
			assert_param (IS_QSPI_SShift(QSPIx));
			assert_param (IS_QSPI_PRESCALER(LMODE));
			assert_param (IS_QSPI_DWidth(DWIDTH));
			assert_param (IS_QSPI_LMode(length) );
			/* Get the TWI_QSPIx_CON value */
			tmpreg = QSPIx->QSPI_CON;
			 /* Clear SSHIFT, QTWCK ,DWIDTH , LMODE , MDOE , RW SPR bits */
			tmpreg &= ~(TWI_QSPIx_CON_DWIDTH|TWI_QSPIx_CON_LMODE|TWI_QSPIx_CON_RW);
			/* Configure QSPI: LMODE, DWIDTH */
			tmpreg |=   LMODE|DWIDTH;
			 /* Write to TWI_QSPIx_CON */
    	
	
			QSPIx->QSPI_CON = tmpreg;
	
		   QSPIx->QSPI_CON |= TWI_QSPIx_CON_RW;
			 /* Clear WI_QSPIx_DL */
			QSPIx->QSPI_DL = 0x00;
			
			QSPIx->QSPI_DL = length;
	
}
/**
 * @brief  Configure QSPI Clock Open.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  CLKONLY[in]: Enable or disable QSPI CLKONLY.
 *                  - QSPI_CLKONLY_OFF  Only send the clock OFF
 *                  - QSPI_CLKONLY_ON  Only send the clock ON
 * @retval None
 */
void QSPI_CLKONLYSet(QSPI_TypeDef *QSPIx, uint32_t CLKONLY)
{
		/* Configure QSPI: QSPI_CLKONLY */
			if(CLKONLY == QSPI_CLKONLY_ON)                     
			{
				QSPIx->QSPI_CON  |= QSPI_CLKONLY_ON;
			}
			else
			{
				QSPIx->QSPI_CON &= ~(uint32_t)QSPI_CLKONLY_ON;
			}	
}

/**
 * @brief  Configure QSPI write conditions.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  Data[in]: QSPI Transmitted Data.
 * @retval None
 */
uint8_t QSPI_ReceiveData8(QSPI_TypeDef *QSPIx)
{
    /* Check the parameters */
    assert_param ( IS_SPI_ALL_PERIPH ( QSPIx ) );
    return ( uint8_t ) QSPIx->QSPI_DATA;
}
/**
 * @brief  Configure QSPI write conditions.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  Data[in]: QSPI Transmitted Data.
 * @retval None
 */
uint16_t QSPI_ReceiveData16(QSPI_TypeDef *QSPIx)
{
    /* Check the parameters */
    assert_param ( IS_SPI_ALL_PERIPH ( QSPIx ) );
    return ( uint16_t ) QSPIx->QSPI_DATA;
}
/**
 * @brief  Configure QSPI write conditions.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  Data[in]: QSPI Transmitted Data.
 * @retval None
 */
uint32_t QSPI_ReceiveData32(QSPI_TypeDef *QSPIx)
{
    /* Check the parameters */
    assert_param ( IS_SPI_ALL_PERIPH ( QSPIx ) );
    return ( uint32_t ) QSPIx->QSPI_DATA;
}
/**
 * @brief  Configure QSPI Receive length.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  datalen[in]: QSPI Receive length
 * @retval None
 */
void QSPI_Receivelen(QSPI_TypeDef *QSPIx,uint32_t datalen)
{
    /* Check the parameters */
    assert_param ( IS_SPI_ALL_PERIPH ( QSPIx ) );
  	QSPIx->QSPI_DL = datalen;
    
}
/**
 * @brief  Receive multiple data through the QSPIx peripheral .
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI can be select the QSPI1 peripheral.
 * @param  buf[in]: the datas to transmit.
 * @param  length[in]: the data length to tramsmit.
 * @retval None
**/
void QSPI_ReceiveMultipleData(QSPI_TypeDef *QSPIx,uint32_t *buf, uint32_t length)
{
    uint32_t tmpNum;

    /* Check the parameters */
    assert_param ( IS_SPI_ALL_PERIPH ( QSPIx ) );
    /* Transmit 24 Bit Data */
		if ( ( QSPIx->QSPI_CON & TWI_QSPIx_CON_DWIDTH)>=QSPI_DWidth_24bit)
		{ 
			
				for ( tmpNum = 0; tmpNum < length; tmpNum++ )
				{
				 QSPIx->QSPI_DL = 1;
				 QSPIx->QSPI_CON |=	TWI_QSPIx_CON_CLKONLY;
				 while(QSPI_GetFlagStatus(QSPI0,QSPI_Flag_BUSY)==0){} // ȴ BUSY    
					buf[tmpNum]	= QSPIx->QSPI_DATA  ;
					
				}
		}
		/* Transmit 16 Bit Data */
		else if( ( QSPIx->QSPI_CON & TWI_QSPIx_CON_DWIDTH)==QSPI_DWidth_16bit)
		{ 
				for ( tmpNum = 0; tmpNum < length; tmpNum++ )
				{
					QSPIx->QSPI_DL = 1;
          QSPIx->QSPI_CON |=	TWI_QSPIx_CON_CLKONLY;
					while(QSPI_GetFlagStatus(QSPI0,QSPI_Flag_BUSY)){} // ȴ BUSY    
				 ( * ( ( uint16_t* ) buf + tmpNum ) ) = (uint16_t)QSPIx->QSPI_DATA ;
					
				}
		}
				/* Transmit 8 Bit Data */
		else if( ( QSPIx->QSPI_CON & TWI_QSPIx_CON_DWIDTH)==QSPI_DWidth_8bit)
		{  
				for ( tmpNum = 0; tmpNum < length; tmpNum++ )
				{
					QSPIx->QSPI_DL = 1;	
          QSPIx->QSPI_CON |=	TWI_QSPIx_CON_CLKONLY;
				while(QSPI_GetFlagStatus(QSPI0,QSPI_Flag_BUSY)){} // ȴ BUSY    
				 ( * ( ( uint8_t* ) buf + tmpNum ) ) = (uint8_t)QSPIx->QSPI_DATA  ;
					
				}
		}	

}
/**
 * @brief  QSPIx receives data through FIFO.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  Data[in]: the datas to receive.
 * @param  length[in]: the data length to receive.
 * @retval The received data.
 */
void QSPI_ReceiveDataFIFO (QSPI_TypeDef *QSPIx, uint32_t* Data, uint16_t length )
{

    uint16_t tmpNum;
    /* Check the parameters */
    assert_param ( IS_SPI_ALL_PERIPH ( QSPIx ) );
    /* Transmit 24 Bit Data */
		if ( ( QSPIx->QSPI_CON & TWI_QSPIx_CON_DWIDTH)>=QSPI_DWidth_24bit)
		{
				for ( tmpNum = 0; tmpNum < length; tmpNum++ )
				{
					   Data[tmpNum]	 = QSPIx->QSPI_DATA;
				}
		}
		/* Transmit 16 Bit Data */
		else if( ( QSPIx->QSPI_CON & TWI_QSPIx_CON_DWIDTH)==QSPI_DWidth_16bit)
		{
				for ( tmpNum = 0; tmpNum < length; tmpNum++ )
				{
					 ( * ( ( uint16_t* ) Data + tmpNum )  = (uint16_t)QSPIx->QSPI_DATA );
				}
		}
				/* Transmit 8 Bit Data */
		else if( ( QSPIx->QSPI_CON & TWI_QSPIx_CON_DWIDTH)==QSPI_DWidth_8bit)
		{
				for ( tmpNum = 0; tmpNum < length; tmpNum++ )
				{
				 ( * ( ( uint8_t* ) Data + tmpNum ) = (uint8_t)QSPIx->QSPI_DATA  );
				}
		}
    

}
/** @defgroup QSPI_Group Interrupts, DMA and flags management functions
 *  @brief   Interrupts, DMA and flags management functions
 *
@verbatim
 ===============================================================================
            ##### Interrupts, DMA and flags management functions #####
 ===============================================================================
@endverbatim
  * @{
  */
/**
 * @brief  Enables or disables the specified SPI interrupts.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  QSPI_IT[in]:specifies the QSPI interrupts sources to be enabled or disabled.
 *               - QSPI_IT_INTEN:SPI total interruption
 *               - QSPI_IT_RXNE:SPI RX buffer not empty Interrupt
 *               - QSPI_IT_TB:SPI TX buffer is empty Interrupt
 *               - QSPI_IT_RX:Receiving FIFO overflow Interrupt
 *               - QSPI_IT_RXH:Receive FIFO over half Interrupt
 *               - QSPI_IT_TXH:Transmit FIFO under half Interrupt
 *               - QSPI_IT_TBIE  SPI Interrupt: Transmit TBIE under half Interrupt 
 * @param  NewState[in]: new state of the SPI interrupts.
 *               - DISABLE:Function disable
 *               - ENABLE:Function enable
 * @retval None
 */
void QSPI_ITConfig(QSPI_TypeDef* QSPIx,uint32_t QSPI_IT, FunctionalState NewState)
{
    /* Check the parameters */
    assert_param ( IS_QSPI_ALL_PERIPH ( QSPIx ) );
    assert_param ( IS_QSPI_IT ( QSPI_IT ) );
    assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );
    if ( NewState != DISABLE )
    {
        /* Enable the Interrupt sources */
        QSPIx->QSPI_IDE |= QSPI_IT;
    }
    else
    {
        /* Disable the Interrupt sources */
        QSPIx->QSPI_IDE &= ( uint16_t ) ~QSPI_IT;
    }
	
}
/**
 * @brief  Checks whether the specified SPI flag is set or not.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  QSPI_FLAG[in]: specifies the flag to check.
 *         - QSPI_Flag_SPIF:SPI transmit and receive flag
 *         - QSPI_Flag_RINEIF:SPI RX buffer not empty flag
 *         - QSPI_Flag_TXEIF:TX buffer is empty flag
 *         - QSPI_Flag_RXFIF:Receiving FIFO overflow flag
 *         - QSPI_Flag_RXHIF:Receive FIFO over half flag
 *         - QSPI_Flag_TXHIF:Transmit FIFO under half flag
 *         - QSPI_Flag_WCOL:Write conflict flag
 *         - QSPI_FLAG_QTWIF = (uint8_t)SPI1_TWI1_STS_QTWIF  
 * @retval The new state of INT_Flag (SET or RESET).
 *                  -  RESET:Flag reset
 *                  -  SET :Flag up
 */
FlagStatus QSPI_GetFlagStatus ( QSPI_TypeDef* QSPIx, QSPI_FLAG_TypeDef QSPI_FLAG )
{

    ITStatus bitstatus = RESET;
    /* Check the parameters */
    assert_param ( IS_SPI_ALL_PERIPH ( QSPIx ) );
    assert_param ( IS_SPI_FLAG ( QSPI_FLAG ) );

    if ( ( QSPIx->QSPI_STS & QSPI_FLAG ) != ( uint16_t ) RESET )
    {
        bitstatus = SET;
    }
    else
    {
        bitstatus = RESET;
    }
    return bitstatus;

}
/**
 * @brief  Clears the SPIx's pending flags.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  QSPI_FLAG[in]:specifies the flag bit to clear.
 *         - QSPI_Flag_SPIF:SPI transmit and receive flag
 *         - QSPI_Flag_RINEIF:SPI RX buffer not empty flag
 *         - QSPI_Flag_TXEIF:TX buffer is empty flag
 *         - QSPI_Flag_RXFIF:Receiving FIFO overflow flag
 *         - QSPI_Flag_RXHIF:Receive FIFO over half flag
 *         - QSPI_Flag_TXHIF:Transmit FIFO under half flag
 *         - QSPI_Flag_WCOL:Write conflict flag
 * @retval None
 */

void QSPI_ClearFlag (QSPI_TypeDef* QSPIx, uint32_t QSPI_FLAG )
{
    /* Check the parameters */
    assert_param ( IS_QSPI_ALL_PERIPH ( QSPI_FLAG ) );
    /* Clear the flags */
    QSPIx->QSPI_STS |= ( uint16_t ) QSPI_FLAG;

}
/**
 * @brief  Enables or disables the SPI's DMA interface.
 * @param  QSPIx[out]:  can be select the QSPIx peripheral.
 *               - QSPI0: QSPI0 can be select the QSPI0 peripheral.
 *               - QSPI1: QSPI0 can be select the QSPI1 peripheral.
 * @param  QSPI_DMAReq[in]: specifies the DMA request.
 *           - QSPI_DMAReq_RX:SPI DMA Request Receive
 *           - QSPI_DMAReq_TX:SPI DMA Request Transmit
 * @param  NewState[in]:new state of the DMA Request sources.
 *           - DISABLE:Function disable
 *           - ENABLE:Function enable
 * @retval None
 */

void QSPI_DMACmd ( QSPI_TypeDef* QSPIx, uint16_t QSPI_DMAReq, FunctionalState NewState )
{
    /* Check the parameters */
    assert_param ( IS_QSPI_ALL_PERIPH ( QSPIx ) );
    assert_param ( IS_QSPI_DMAREQ ( QSPI_DMAReq ) );
    assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );
    if ( NewState != DISABLE )
    {
        /* Enable the selected SPI DMA requests */
        QSPIx->QSPI_IDE |= QSPI_DMAReq;
    }
    else
    {
        /* Disable the selected SPI DMA requests */
         QSPIx->QSPI_IDE &= ( uint16_t ) ~QSPI_DMAReq;
    }
}


/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
#endif
/************************ (C) COPYRIGHT SOC Microelectronics *****END OF FILE****/

