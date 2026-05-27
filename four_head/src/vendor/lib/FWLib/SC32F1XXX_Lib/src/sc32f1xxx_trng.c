/**
 ******************************************************************************
 * @file    sc32f1xxx_trng.c
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   TRNG function module
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
#include "sc32f1xxx_trng.h"

/** @defgroup TRNG_Group1 Initialization and Configuration
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
 * @brief  Reset the peripheral TRNG register to its default value.
 * @param  None
 * @retval None
 */	
void TRNG_DeInit ( void )
{
	  /* Enable TRNG reset state */
    RCC_AHBPeriphResetCmd(RCC_AHBPeriph_AES,ENABLE);
	  /* Disable TRNG reset state */
	  RCC_AHBPeriphResetCmd(RCC_AHBPeriph_AES,DISABLE);
}	
 
/**
 * @brief  Enables or disables the specified TRNG peripheral.
 * @param  NewState[in]:new state of the TRNG peripheral.
 *                  - DISABLE:Function disable
 *                  - ENABLE:Function enable
 * @retval None
 */ 
void TRNG_Cmd(FunctionalState NewState)
{
    if ( NewState != DISABLE )
    {
			  /* Enable the TRNG Function */
        AES->AES_CONFIG |= TRNG_CFG_TRNGEN;
    }
    else
    {
			  /* Disable the TWI Function */
        AES->AES_CONFIG &= ( uint32_t ) ~TRNG_CFG_TRNGEN;
    }
}

/**
 * @brief  Selection of TRNG random number generation method.
 * @param  TRNG_TLSEL[in]:TRNG random number generation method.It is recommended that users set it to 0x00.
 * @retval None
 */ 
void TRNG_TLSELConfig(uint8_t TRNG_TLSEL)
{
	  AES->AES_CONFIG &= ~((uint32_t)(0x07<<4)); 
	  AES->AES_CONFIG |= TRNG_TLSEL;
}

/**
 * @brief  Configures the TRNG clock source
 * @param  TRNG_TLDIV[in]:TRNG clock source.It is recommended that users set it to 0x05.
 * @retval None
 */ 
void TRNG_TLDIVConfig(uint8_t TRNG_TLDIV)
{
	  AES->AES_CONFIG &= ~((uint32_t)(0x07<<1)); 
	  AES->AES_CONFIG |= TRNG_TLDIV;
}

/**
 * @brief  Returns a 32-bit random number. 
 * @param  None
 * @retval 32-bit random number.
 */
uint32_t TRNG_GetRandomNumber(void)
{
   return AES->TRNG_DATA;
}

/**
 * @}
 */
/* End of TRNG_Group1.	*/

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
  * @brief  Enables or disables the TRNG interrupt.
  * @param  TRNG_IT[in]:specifies the TRNG interrupts sources to be enabled or disabled.
  *                  - TRNG_IT_INTEN: TRNG Interrupt
  *                  - TRNG_IT_DRDYIE: TRNG Data generation completed Interrupt
  * @param  NewState[in]:new state of the TRNG interrupts.
  *                   - DISABLE:Function disable
  *                   - ENABLE:Function enable
  * @retval None
  */
void TRNG_ITConfig(uint16_t TRNG_IT,FunctionalState NewState)
{
  /* Check the parameters */
  assert_param ( IS_TRNG_IT ( TRNG_IT ) );
	
  if (NewState != DISABLE)
  {
     AES->AES_CONFIG |= TRNG_IT;
  }
  else
  {
     AES->AES_CONFIG &= ( uint32_t ) ~ ( ( uint32_t ) TRNG_IT );		
  }
}

/**
 * @brief  Checks whether the specified TRNG flag is set or not.
 * @param  TRNG_FLAG[in]:specifies the flag to check.
 *                  - TRNG_FLAG_SEIF :Seed error interrupt.
 *                  - TRNG_FLAG_DRDYIF:Data generation completed.
 * @retval The new state of TRNG_FLAG (SET or RESET).
 *                  -  RESET:Flag reset
 *                  -  SET :Flag up
 */
FlagStatus TRNG_GetFlagStatus(TRNG_Flag_TypeDef TRNG_FLAG)
{
   FlagStatus bitstatus = RESET;
   /* Check the parameters */
   assert_param ( IS_GET_TRNG_FLAG ( TRNG_FLAG ) );

  if ( ( AES->TRNG_STS & TRNG_FLAG ) != ( uint16_t ) RESET )		
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
 * @brief  Clears the TRNG pending flags.
 * @param  TRNG_FLAG[in]:specifies the flag to check.
 *                  - TRNG_FLAG_SEIF :Seed error interrupt.
 * @retval None
 */
void TRNG_ClearFlag(uint8_t TRNG_FLAG)
{
	 /* Clear the flags */
  AES->TRNG_STS = ((uint32_t)TRNG_FLAG); 
}

/**
 * @}
 */
/* End of TRNG_Group2.	*/

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
