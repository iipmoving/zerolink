/**
 ******************************************************************************
 * @file    sc32f1xxx_smartcard.c
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   Smart_Card function module
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
#if defined(SC32L14xx)
#include "sc32f1xxx_smartcard.h"

/** @defgroup Smart_Card_Exported_Functions_Group1 Configuration of the Smart_Card computation unit functions
 *  @brief   Configuration of the Smart_Card computation unit functions
 *
@verbatim
 ===============================================================================
                  ##### Smart_Card configuration functions #####
 ===============================================================================
@endverbatim
  * @{
  */

/**
 * @brief  DeInitializes the Smart_Card peripheral 
 * @param  SCx[out]: select the SCx peripheral.
 *                  - Smart_Card: Only Smart_Card can be select the SCx peripheral.
 * @retval None
 */
void Smart_Card_DeInit ( Smart_Card_TypeDef* SCx )
{
    /* Check the parameters */
    assert_param ( IS_SMARTCARD_ALL_PERIPH ( SCx ) );

		if ( SCx == Smart_Card )
    {
				RCC_APB1PeriphResetCmd ( RCC_APB1Periph_SC, ENABLE );
				RCC_APB1PeriphResetCmd ( RCC_APB1Periph_SC, DISABLE );
		}

}

/**
 * @brief  Initializes the SC peripheral
 * @param  SCx[out]: select the SCx peripheral.
 *                  - Smart_Card: Only Smart_Card can be select the SCx peripheral.
 * @param  SC_InitStruct[out]: Pointer to structure SC_InitTypeDef,
 *                              to be initialized.
 * @retval None
 */
void Smart_Card_Init ( Smart_Card_TypeDef* SCx, SmartCard_InitTypeDef* SC_InitStruct )
{
    uint32_t tmpreg;
    /* Check the parameters */
    assert_param ( IS_SMARTCARD_ALL_PERIPH ( SCx ) );
    assert_param ( IS_SMARTCARD_PCS ( SC_InitStruct->SC_Parity ) );
    assert_param ( IS_SMARTCARD_TRER ( SC_InitStruct->SC_TransReadError ) );
    assert_param ( IS_SMARTCARD_CONS ( SC_InitStruct->SC_EncodeMethod ) );
    assert_param ( IS_SMARTCARD_ERS ( SC_InitStruct->SC_ErrorETU ) );
		assert_param ( IS_SMARTCARD_TREN ( SC_InitStruct->SC_TransReceEn ) );
    assert_param ( IS_SMARTCARD_CKEN ( SC_InitStruct->SC_ClockOutEn ) );

    tmpreg = SCx->SC0_CON;
    tmpreg &= ~ ( SC_CON_PCS | SC_CON_TRE | SC_CON_CONS | SC_CON_ERS | SC_CON_TREN | SC_CON_CKEN );
    tmpreg |= ( SC_InitStruct->SC_Parity | SC_InitStruct->SC_TransReadError |
                SC_InitStruct->SC_EncodeMethod | SC_InitStruct->SC_ErrorETU |
								 SC_InitStruct->SC_TransReceEn | SC_InitStruct->SC_ClockOutEn );
    SCx->SC0_CON = tmpreg;
}

/**
  * @brief  Fills each SC_InitStruct member with its default value.
  * @param  SC_InitStruct[out]: Pointer to structure SC_InitTypeDef,
  *                              to be initialized.
  * @retval None
  */
void Smart_Card_StructInit ( SmartCard_InitTypeDef* SC_InitStruct )
{
    /* Set the default configuration */
    SC_InitStruct->SC_Parity  = SC_PCS_NoneParity;
    SC_InitStruct->SC_TransReadError = SC_TRER_SetITFlag;
    SC_InitStruct->SC_EncodeMethod = SC_CONS_Positive;
    SC_InitStruct->SC_ErrorETU  = SC_ERS_2ETU;
	  SC_InitStruct->SC_TransReceEn = SC_TREN_RXEN;
		SC_InitStruct->SC_ClockOutEn = SC_CKEN_Disable;
}

/**
 * @brief  Enables or disables the specified SC peripheral.
 * @param  SCx[out]: select the SCx peripheral.
 *                  - Smart_Card: Only Smart_Card can be select the SCx peripheral.
 * @param  NewState[in]: new state of the SCx peripheral.
 *                  - DISABLE:Function disable
 *                  - ENABLE:Function enable
 * @retval None
 */
void Smart_Card_Cmd ( Smart_Card_TypeDef* SCx, FunctionalState NewState )
{
    /* Check the parameters */
    assert_param ( IS_SMARTCARD_ALL_PERIPH ( SCx ) );
    assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );

    if ( NewState != DISABLE )
    {
        /* Enable the Smart_Card */
        SCx->SC0_CON |= SC_CON_SCEN;
    }
    else
    {
        /* Disable the Smart_Card */
        SCx->SC0_CON &= ( uint32_t ) ~SC_CON_SCEN;
    }
}

/**
 * @brief  Enables or disables the specified Smart_Card clock.
 * @param  SCx[out]: select the SCx peripheral.
 *                  - Smart_Card: Only Smart_Card can be select the SCx peripheral.
 * @param  NewState[in]: new state of the SCx peripheral.
 *                  - DISABLE:Function disable
 *                  - ENABLE:Function enable
 * @retval None
 */
void Smart_Card_CKEN_Cmd ( Smart_Card_TypeDef* SCx, FunctionalState NewState )
{
    /* Check the parameters */
    assert_param ( IS_SMARTCARD_ALL_PERIPH ( SCx ) );
    assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );

    if ( NewState != DISABLE )
    {
        /* Enable the Smart_Card clock  */
        SCx->SC0_CON |= SC_CON_CKEN;
    }
    else
    {
        /* Disable the Smart_Card clock */
        SCx->SC0_CON &= ( uint32_t ) ~SC_CON_CKEN;
    }
}

/**
 * @brief  Enables the specified Smart_Card writing or reading.
 * @param  SCx[out]: select the SCx peripheral.
 *                  - Smart_Card: Only Smart_Card can be select the SCx peripheral.
 * @param  NewState[in]: new state of the SCx peripheral.
 *                  - DISABLE:Function disable
 *                  - ENABLE:Function enable
 * @retval None
 */
void Smart_Card_TREN_Cmd ( Smart_Card_TypeDef* SCx, SmartCard_TREN_TypeDef NewState )
{
    /* Check the parameters */
    assert_param ( IS_SMARTCARD_ALL_PERIPH ( SCx ) );
    assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );

    if ( NewState == SC_TREN_TXEN )
    {
        /* Enable the Smart_Card writing */
        SCx->SC0_CON |= SC_CON_TREN;
    }
    else if( NewState == SC_TREN_RXEN )
    {
        /* Enable the Smart_Card reading */
        SCx->SC0_CON &= ( uint32_t ) ~SC_CON_TREN;
    }
}

/**
 * @brief  Configure the baud rate of the Smart_Card.
 * @param  SCx[out]: select the SCx peripheral.
 *                  - Smart_Card: Only Smart_Card can be select the SCx peripheral.
 * @param  SC_BaudInitStruct[out]: Pointer to structure SC_BaudInit_TypeDef,
 *                                  to be initialized.
 * @retval None
 */
void Smart_Card_BaudConfig ( Smart_Card_TypeDef* SCx, SmartCard_BaudInitTypeDef* SC_BaudInitStruct )
{
		uint32_t tmpreg;
    /* Check the parameters */
    assert_param ( IS_SMARTCARD_ALL_PERIPH ( SCx ) );
	
		tmpreg = SCx->SC0_BAUD;
		tmpreg &= ~ ( SC_BAUD_ETUCK | SC_BAUD_SCCK | SC_BAUD_EGT );
    tmpreg |= ( (SC_BaudInitStruct->SC_ExtraGuardTime << SC_BAUD_EGT_Pos) | (SC_BaudInitStruct->SC_ClockCycle <<  SC_BAUD_SCCK_Pos) |
                SC_BaudInitStruct->SC_ETUClockCycle );
    SCx->SC0_BAUD = tmpreg;
}

/**
  * @brief  Fills each SC_BaudInitStruct member with its default value.
  * @param  SC_BaudInitStruct[out]: Pointer to structure SC_BaudInit_TypeDef,
  *                                 to be initialized.
  * @retval None
  */
void Smart_Card_BaudStructInit ( SmartCard_BaudInitTypeDef* SC_BaudInitStruct )
{
    /* Set the default configuration */
    SC_BaudInitStruct->SC_ExtraGuardTime  = 0x05;
    SC_BaudInitStruct->SC_ClockCycle = 0x02;
    SC_BaudInitStruct->SC_ETUClockCycle = 0x174;
}

/**
 * @}
 */
/* End of SC_Group1.	*/

/** @defgroup SC_Group2 Data transfers functions
 *  @brief   Data transfers functions
 *
@verbatim
 ===============================================================================
                     ##### Data transfers functions #####
 ===============================================================================
@endverbatim
  * @{
  */

/**
 * @brief  Transmits single data through the SCx peripheral.
 * @param  SCx[out]: select the SCx peripheral.
 *                  - Smart_Card: Only Smart_Card can be select the SCx peripheral.
 * @param  Data[in]: the data to transmit.
 * @retval None
 */
void Smart_Card_SendData ( Smart_Card_TypeDef* SCx, uint8_t Data )
{
    /* Check the parameters */
    assert_param ( IS_SMARTCARD_ALL_PERIPH ( SCx ) );

    /* Transmit Data */
    SCx->SC0_DATA = Data;
}

/**
 * @brief  Returns the most recent received data by the SCx peripheral.
 * @param  SCx[out]: select the SCx peripheral.
 *                  - Smart_Card: Only Smart_Card can be select the SCx peripheral.
 * @retval The received data.
 */
uint8_t Smart_Card_ReceiveData ( Smart_Card_TypeDef* SCx )
{
    /* Check the parameters */
    assert_param ( IS_SMARTCARD_ALL_PERIPH ( SCx ) );

    /* Receive Data */
    return ( uint8_t ) ( SCx->SC0_DATA );
}

/**
 * @}
 */
/* End of SC_Group2.	*/

/** @defgroup SC_Group3 Pin remap management functions
 *  @brief  Pin remap management functions
 *
@verbatim
 ===============================================================================
                     ##### Pin remap management functions #####
 ===============================================================================
@endverbatim
  * @{
  */

/**
 * @brief  Configures the SCx Pin Remap
 * @param  SCx[out]: select the SCx peripheral.
 *                  - Smart_Card: Only Smart_Card can be select the SCx peripheral.
 * @param  SC_Remap[in]: specifies the SCx input remapping source.
 *                  - SC_PinRemap_Default:Smart_Card Pin Remap Disable
 *                  - SC_PinRemap_A :Smart_Card Pin Remap mode A
 * @retval None
 */
void Smart_Card_PinRemapConfig ( Smart_Card_TypeDef* SCx, SmartCard_PinRemap_TypeDef SC_Remap )
{
    uint32_t tmpreg;
		/* Check the parameters */
		assert_param ( IS_SMARTCARD_ALL_PERIPH ( SCx ) );
		assert_param ( IS_SMARTCARD_PINREMAP ( SC_Remap ) );
		
		tmpreg = SCx->SC0_CON;

		tmpreg &= ( uint32_t ) ( ~SC_CON_SPOS );

		tmpreg |= SC_Remap;

		SCx->SC0_CON = tmpreg;
}

/**
 * @}
 */
/* End of SC_Group3.	*/

/** @defgroup SC_Group4 Interrupts and flags management functions
 *  @brief    Interrupts and flags management functions
 *
@verbatim
 ===============================================================================
                     ##### Interrupts and flags management functions #####
 ===============================================================================
@endverbatim
  * @{
  */
/**
 * @brief  Enables or disables the specified SC interrupts.
 * @param  SCx[out]: select the SCx peripheral.
 *                  - Smart_Card: Only Smart_Card can be select the SCx peripheral.
 * @param  SC_IT[in]: specifies the Smart_Card interrupts sources to be enabled or disabled.
 *                  - SC_IT_INTEN:Smart_Card Interrupt
 *                  - SC_IT_RXIE:Smart_Card Receive Interrupt 
 *                  - SC_IT_TXIE:Smart_Card Transmission Interrupt
 *                  - SC_IT_ERRIE:Smart_Card Communication abnormal Interrupt
 * @param  NewState[in]: new state of the Smart_Card interrupts.
 *                  - DISABLE:Function disable
 *                  - ENABLE:Function enable
 * @retval None
 */
void Smart_Card_ITConfig ( Smart_Card_TypeDef* SCx, uint16_t SC_IT, FunctionalState NewState )
{
    /* Check the parameters */
    assert_param ( IS_SMARTCARD_ALL_PERIPH ( SCx ) );
    assert_param ( IS_SMARTCARD_IT ( SC_IT ) );
    assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );
    if ( NewState != DISABLE )
    {
        /* Enable the Interrupt sources */
        SCx->SC0_IDE |= SC_IT;
    }
    else
    {
        /* Disable the Interrupt sources */
        SCx->SC0_IDE &= ( uint32_t ) ~SC_IT;
    }
}

/**
 * @brief  Checks whether the specified SC flag is set or not.
 * @param  SCx[out]: select the SCx peripheral.
 *                  - Smart_Card: Only Smart_Card can be select the SCx peripheral.
 * @param  SC_FLAG[in]:specifies the flag to check.
 *                  - SC_Flag_RC :Smart_Card Receive completion Flag 
 *                  - SC_Flag_TC :Smart_Card Transmission completion Flag 
 *                  - SC_Flag_ROVF :Smart_Card Receive overflow Flag 
 *                  - SC_Flag_FER :Smart_Card Receive frame error Flag 
 *                  - SC_Flag_WTER :Smart_Card Wait timeout Flag
 *                  - SC_Flag_RPER :Smart_Card Receive data parity error Flag 
 *                  - SC_Flag_TPER :Smart_Card Transmit data parity error Flag 
 *                  - SC_Flag_RBUSY :Smart_Card Data receiving busy state 
 *                  - SC_Flag_TBUSY :Smart_Card Data transmitting busy state 
 *                  - SC_Flag_WTRT :Smart_Card Waiting for data retransmission state
 * @retval The new state of SC_FLAG (SET or RESET).
 *                  -  RESET:Flag reset
 *                  -  SET :Flag up
 */
FlagStatus Smart_Card_GetFlagStatus ( Smart_Card_TypeDef* SCx, uint32_t SC_FLAG )
{
    ITStatus bitstatus = RESET;
    /* Check the parameters */
    assert_param ( IS_SMARTCARD_ALL_PERIPH ( SCx ) );
		assert_param ( IS_SMARTCARD_FLAG ( SC_FLAG ) );

    if ( ( SCx->SC0_STS & SC_FLAG ) != ( uint16_t ) RESET )
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
 * @brief  Clears the SCx's pending flags.
 * @param  SCx[out]: select the SCx peripheral.
 *                  - Smart_Card: Only Smart_Card can be select the SCx peripheral.
 * @param  SC_FLAG[in]:specifies the flag bit to clear.
 *                  - SC_Flag_RC :Smart_Card Receive completion Flag 
 *                  - SC_Flag_TC :Smart_Card Transmission completion Flag 
 *                  - SC_Flag_ROVF :Smart_Card Receive overflow Flag 
 *                  - SC_Flag_FER :Smart_Card Receive frame error Flag 
 *                  - SC_Flag_WTER :Smart_Card Wait timeout Flag
 *                  - SC_Flag_RPER :Smart_Card Receive data parity error Flag 
 *                  - SC_Flag_TPER :Smart_Card Transmit data parity error Flag 
 * @retval None
 */
void Smart_Card_ClearFlag ( Smart_Card_TypeDef* SCx, uint32_t SC_FLAG )
{
    /* Check the parameters */
    assert_param ( IS_SMARTCARD_ALL_PERIPH ( SCx ) );
		assert_param ( IS_SMARTCARD_FLAG ( SC_FLAG ) );

    /* Clear the flags */
		SCx->SC0_STS |= ( uint32_t ) SC_FLAG;
}

/**
 * @}
 */
/* End of SC_Group4.	*/

#endif

/************************ (C) COPYRIGHT SOC Microelectronics *****END OF FILE****/
