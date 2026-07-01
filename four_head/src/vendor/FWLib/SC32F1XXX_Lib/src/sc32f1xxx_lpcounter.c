/**
 ******************************************************************************
 * @file    sc32f1xxx_lpcounter.c
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   LPC function module
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
#if defined(SC32L14xx)||defined(SC32R807)
#include "sc32f1xxx_lpcounter.h"

/** @defgroup LPC_Group1 Initialization and Configuration
 *  @brief   Initialization and Configuration
 *
@verbatim
 ===============================================================================
                    ##### Initialization and Configuration #####
 ===============================================================================

@endverbatim
  * @{
  */
	
/**
 * @brief  Reset the peripheral LPCx register to its default value.
 * @param  LPCx[out]: select the LPCx peripheral.
 *                  - LPC: Only LPC can be select the LPCx peripheral.
 * @retval None
 */	
void LPC_DeInit ( LPC_TypeDef* LPCx )
{
	  LPCx->LPC_CON = 0x00000000;
	  LPCx->LPC_IDE = 0x00000000;
	  LPCx->LPCA_CNT = 0x00000000;
	  LPCx->LPCB_CNT = 0x00000000;
	  LPCx->LPCA_MAX = 0x00000000;
	  LPCx->LPCB_MAX = 0X0000003F;
}
	
/**
 * @brief  Initializes the ADC peripheral
 * @param  LPCx[out]: select the LPCx peripheral.
 *                  - LPC: Only LPC can be select the LPCx peripheral.
 * @param  LPC_InitStruct[out]: Pointer to structure LPC_InitStruct,
 *                              to be initialized.
 * @retval None
 */
void  LPC_Init(LPC_TypeDef* LPCx,LPC_InitTypeDef* LPC_InitStruct )
{
	uint32_t tmpreg;
	
  /* Check the parameters */
  assert_param ( IS_LPC_ALL_PERIPH ( LPCx ) );
  assert_param ( IS_LPC_TRIGGER ( LPC_InitStruct->LPC_INTATrigger ) );
  assert_param ( IS_LPC_TRIGGER ( LPC_InitStruct->LPC_INTBTrigger ) );	
  assert_param ( IS_LPC_INITDIR ( LPC_InitStruct->LPC_InitDir ) );	
	
	tmpreg = LPCx->LPC_CON;
	tmpreg &= ~((uint32_t)(LPC_CON_INITDIR | LPC_CON_DISDIR | LPC_CON_RFSELA | LPC_CON_RFSELB));
	if(LPC_InitStruct->LPC_InitDir == LPC_INITDIR_Reversal)
	{
		  tmpreg |= LPC_CON_INITDIR;
	}
  if(LPC_InitStruct->LPC_DisDirEN != DISABLE)
	{
		  tmpreg |= LPC_CON_DISDIR;
	}
	if(LPC_InitStruct->LPC_INTATrigger == LPC_Trigger_Falling)
	{
		  tmpreg |= LPC_CON_RFSELA;
	}
	if(LPC_InitStruct->LPC_INTBTrigger == LPC_Trigger_Falling)
	{
		  tmpreg |= LPC_CON_RFSELB;
	}
  LPCx->LPC_CON = tmpreg;
	
  LPCx->LPCA_MAX  = LPC_InitStruct->LPC_INTAOverValue;
  LPCx->LPCB_MAX  = LPC_InitStruct->LPC_INTBOverValue;	
}

/**
  * @brief  Fills each LPC_InitStruct member with its default value.
  * @param  LPC_InitStruct[out]: Pointer to structure LPC_InitStruct,
  *                              to be initialized.
  * @retval None
  */
void LPC_StructInit ( LPC_InitTypeDef* LPC_InitStruct )
{
    /* Set the default configuration */
    LPC_InitStruct->LPC_InitDir = LPC_INITDIR_Forward;
	  LPC_InitStruct->LPC_DisDirEN = DISABLE;
	  LPC_InitStruct->LPC_INTATrigger = LPC_Trigger_Rising;
		LPC_InitStruct->LPC_INTBTrigger = LPC_Trigger_Rising;
    LPC_InitStruct->LPC_INTAOverValue = 0XFF;
    LPC_InitStruct->LPC_INTBOverValue = 0XFF;		
}

/**
 * @brief  Enables or disables the specified LPC peripheral.
 * @param  LPCx[out]: select the LPCx peripheral.
 *                  - LPC: Only LPC can be select the LPCx peripheral.
 * @param  NewState[in]: new state of the LPCx peripheral.
 *                  - DISABLE:Function disable
 *                  - ENABLE:Function enable
 * @retval None
 */
void LPC_Cmd(LPC_TypeDef* LPCx,FunctionalState NewState)
{
    /* Check the parameters */
    assert_param ( IS_LPC_ALL_PERIPH ( LPCx ) );
		assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );

    if ( NewState != DISABLE )
    {
			  /* Enable the LPCx Counter */
        LPCx->LPC_CON |= LPC_CON_LPCTEN;
    }
    else
    {
			  /* Disable the LPCx Counter */
        LPCx->LPC_CON &= ( uint32_t ) ~LPC_CON_LPCTEN;
    }
}

/**
 * @brief  LPCx Trigger Mode Select
 * @param  LPCx[out]: select the LPCx peripheral.
 *                  - LPC: Only LPC can be select the LPCx peripheral.
 * @param  LPC_INTSEL[in]: select the LPCx active edge.
 *                  - LPC_INTSEL_INTA: Select the INTA.
 *                  - LPC_INTSEL_INTB: Select the INTB.
 * @param  Trigger_Mode[in]: Select triggering mode.
 *                  - LPC_Trigger_Rising: Rising edge capture
 *                  - LPC_Trigger_Falling: Falling edge capture
 * @retval None
 */
void LPC_TriggerMode (  LPC_TypeDef* LPCx, LPC_INTSEL_Typedef LPC_INTSEL, LPC_Trigger_TypeDef Trigger_Mode)
{
    /* Check the parameters */
	  assert_param ( IS_LPC_ALL_PERIPH ( LPCx ) );
    assert_param ( LPC_INTSEL_Typedef ( LPC_INTSEL ) );	
    assert_param ( IS_LPC_TRIGGER ( Trigger_Mode ) );
	
	  if(LPC_INTSEL == LPC_INTSEL_INTA)
		{
			  if(Trigger_Mode == LPC_Trigger_Rising)
				{
			     LPCx->LPC_CON &= ( uint32_t ) ~LPC_CON_RFSELA;
				}
				else
				{
					LPCx->LPC_CON |= ( uint32_t ) LPC_CON_RFSELA;
				}
		}
		else
		{
			  if(Trigger_Mode == LPC_Trigger_Rising)
				{
			     LPCx->LPC_CON &= ( uint32_t ) ~LPC_CON_RFSELB;
				}
				else
				{
					LPCx->LPC_CON |= ( uint32_t ) LPC_CON_RFSELB;
				}			 
		}	  
}

/**
 * @brief  Set the Register value of LPCx Overflow Count.
 * @param  LPCx[out]: select the LPCx peripheral.
 *                  - LPC: Only LPC can be select the LPCx peripheral.
 * @param  LPC_INTSEL[in]: select the LPCx active edge.
 *                  - LPC_INTSEL_INTA: Select the INTA.
 *                  - LPC_INTSEL_INTB: Select the INTB.
 * @param  Counter[in]: specifies the MAX Counter register new value.
 * @retval None
 */
void LPC_SetOverflowCounter ( LPC_TypeDef* LPCx, LPC_INTSEL_Typedef LPC_INTSEL,uint8_t Counter )
{
    /* Check the parameters */
	  assert_param ( IS_LPC_ALL_PERIPH ( LPCx ) );
    assert_param ( LPC_INTSEL_Typedef ( LPC_INTSEL ) );	
	
	  if(LPC_INTSEL == LPC_INTSEL_INTA)
		{
			  LPCx->LPCA_MAX = ( uint32_t )Counter;
		}
		else
		{
			  LPCx->LPCB_MAX = ( uint32_t )Counter;
		}
}

/**
 * @brief  Set the Register value of LPCx Overflow Count.
 * @param  LPCx[out]: select the LPCx peripheral.
 *                  - LPC: Only LPC can be select the LPCx peripheral.
 * @param  LPC_INTSEL[in]: select the LPCx active edge.
 *                  - LPC_INTSEL_INTA: Select the INTA.
 *                  - LPC_INTSEL_INTB: Select the INTB.
 * @retval MAX Counter register value
 */
uint8_t  LPC_GetOverflowCounter ( LPC_TypeDef* LPCx, LPC_INTSEL_Typedef LPC_INTSEL )
{
	  uint8_t  tempData;
    /* Check the parameters */
	  assert_param ( IS_LPC_ALL_PERIPH ( LPCx ) );
    assert_param ( LPC_INTSEL_Typedef ( LPC_INTSEL ) );	
	
	  if(LPC_INTSEL == LPC_INTSEL_INTA)
		{
			  tempData = ( uint8_t )LPCx->LPCA_MAX;
		}
		else
		{
			  tempData = ( uint8_t )LPCx->LPCB_MAX;
		}
		
		return tempData;
}

/**
 * @brief  Set the Register value of LPCx Count.
 * @param  LPCx[out]: select the LPCx peripheral.
 *                  - LPC: Only LPC can be select the LPCx peripheral.
 * @param  LPC_INTSEL[in]: select the LPCx active edge.
 *                  - LPC_INTSEL_INTA: Select the INTA.
 *                  - LPC_INTSEL_INTB: Select the INTB.
 * @param  Counter[in]: specifies the Counter register new value.
 * @retval None
 */
void LPC_SetCounter ( LPC_TypeDef* LPCx, LPC_INTSEL_Typedef LPC_INTSEL,uint8_t Counter )
{
    /* Check the parameters */
	  assert_param ( IS_LPC_ALL_PERIPH ( LPCx ) );
    assert_param ( LPC_INTSEL_Typedef ( LPC_INTSEL ) );	
	
	  if(LPC_INTSEL == LPC_INTSEL_INTA)
		{
			  LPCx->LPCA_CNT = ( uint32_t )Counter;
		}
		else
		{
			  LPCx->LPCB_CNT = ( uint32_t )Counter;
		}
}

/**
 * @brief  Set the Register value of LPCx Overflow Count.
 * @param  LPCx[out]: select the LPCx peripheral.
 *                  - LPC: Only LPC can be select the LPCx peripheral.
 * @param  LPC_INTSEL[in]: select the LPCx active edge.
 *                  - LPC_INTSEL_INTA: Select the INTA.
 *                  - LPC_INTSEL_INTB: Select the INTB.
 * @retval Counter register value
 */
uint8_t  LPC_GetCounter ( LPC_TypeDef* LPCx, LPC_INTSEL_Typedef LPC_INTSEL )
{
	  uint8_t  tempData;
    /* Check the parameters */
	  assert_param ( IS_LPC_ALL_PERIPH ( LPCx ) );
    assert_param ( LPC_INTSEL_Typedef ( LPC_INTSEL ) );	
	
	  if(LPC_INTSEL == LPC_INTSEL_INTA)
		{
			  tempData = ( uint8_t )LPCx->LPCA_CNT;
		}
		else
		{
			  tempData = ( uint8_t )LPCx->LPCB_CNT;
		}
		
		return tempData;
}
/**
 * @}
 */
/* End of LPC_Group1.	*/

/** @defgroup Interrupts and flags management functions
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
  * @brief  Enables or disables the LPCx interrupt.
  * @param  LPCx[out]: select the LPCx peripheral.
  *                  - LPC: Only LPC can be select the LPCx peripheral.
  * @param  LPC_IT[in]:specifies the LPCx interrupts sources to be enabled or disabled.
  *                  - LPC_IT_INTEN: LPC interrupt enable.
  *                  - LPC_IT_DIRIE: Direction jump interrupt enable.
	*                  - LPC_IT_CAIE: LPCA_CNT Count Overflow Interrupt Enable.
  *                  - LPC_IT_CBIE: LPCB_CNT Count Overflow Interrupt Enable.
  *                  - LPC_IT_RFAIE: INTA input valid edge interrupt enable.
  *                  - LPC_IT_RFBIE: INTB input valid edge interrupt enable.
  * @param  NewState[in]:new state of the LPCx interrupts.
  *                   - DISABLE:Function disable
  *                   - ENABLE:Function enable
  * @retval None
  */
void LPC_ITConfig(LPC_TypeDef* LPCx,uint8_t LPC_IT,FunctionalState NewState)
{
  /* Check the parameters */
	assert_param ( IS_LPC_ALL_PERIPH ( LPCx ) );
  assert_param ( IS_LPC_IT ( LPC_IT ) );
	assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );
		
  if (NewState != DISABLE)
  {
     LPCx->LPC_IDE |= LPC_IT;
  }
  else
  {
     LPCx->LPC_IDE &= ( uint32_t ) ~ ( ( uint32_t ) LPC_IT );		
  }	
}

/**
 * @brief  Checks whether the specified LPCx flag is set or not.
 * @param  LPCx[out]: select the LPCx peripheral.
 *                  - LPC: Only LPC can be select the LPCx peripheral.
 * @param  LPC_FLAG[in]:specifies the flag to check.
 *                  - LPC_FLAG_FLIPIF : Direction Flip Flag.
 *                  - LPC_FLAG_DIRIF: Waveform direction indication.
 *                  - LPC_FLAG_CAIF : LPCA count overflow flag.
 *                  - LPC_FLAG_CBIF: LPCB count overflow flag.
 *                  - LPC_FLAG_RFAIF: INTA input valid along the detected flag.
 *                  - LPC_FLAG_RFBIF: INTB input valid along the detected flag.
 * @retval The new state of LPC_FLAG (SET or RESET).
 *                  -  RESET:Flag reset
 *                  -  SET :Flag up
 */
FlagStatus LPC_GetFlagStatus(LPC_TypeDef* LPCx,LPC_Flag_TypeDef LPC_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
	assert_param ( IS_LPC_ALL_PERIPH ( LPCx ) );
  assert_param ( IS_GET_LPC_FLAG ( LPC_FLAG ) );
	
  if (LPCx->LPC_STS & LPC_FLAG)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }

  return  bitstatus;
}

/**
 * @brief  Clears the LPCx pending flags.
 * @param  LPCx[out]: select the LPCx peripheral.
 *                  - LPC: Only LPC can be select the LPCx peripheral.
 * @param  LPC_FLAG[in]:specifies the flag to check.
 *                  - LPC_FLAG_FLIPIF : Direction Flip Flag.
 *                  - LPC_FLAG_CAIF : LPCA count overflow flag.
 *                  - LPC_FLAG_CBIF: LPCB count overflow flag.
 *                  - LPC_FLAG_RFAIF: INTA input valid along the detected flag.
 *                  - LPC_FLAG_RFBIF: INTB input valid along the detected flag.
 * @retval None
 */
void LPC_ClearFlag(LPC_TypeDef* LPCx,uint8_t LPC_FLAG)
{
	/* Clear the flags */
  LPCx->LPC_STS = ((uint32_t)LPC_FLAG); 
}

/**
 * @}
 */
/* End of LPC_Group2.	*/

#endif

/************************ (C) COPYRIGHT SOC Microelectronics *****END OF FILE****/
