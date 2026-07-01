/**
 ******************************************************************************
 * @file    sc32f1xxx_aes.c
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   AES function module
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
#include "sc32f1xxx_aes.h"

/** @defgroup AES_Group1 Initialization and Configuration
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
 * @brief  Reset the peripheral AES register to its default value.
 * @param  None
 * @retval None
 */	
void AES_DeInit ( void )
{
	  /* Enable AES reset state */
    RCC_AHBPeriphResetCmd(RCC_AHBPeriph_AES,ENABLE);
	  /* Disable AES reset state */
	  RCC_AHBPeriphResetCmd(RCC_AHBPeriph_AES,DISABLE);
}

/**
 * @}
 */
/* End of AES_Group1.	*/


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
  * @brief  Enables or disables the AES interrupt.
  * @param  AES_IT[in]:specifies the AES interrupts sources to be enabled or disabled.
  *                  - AES_IT_INTEN: AES Interrupt
  * @param  NewState[in]:new state of the AES interrupts.
  *                   - DISABLE:Function disable
  *                   - ENABLE:Function enable
  * @retval None
  */
void AES_ITConfig ( uint16_t AES_IT, FunctionalState NewState )
{
    /* Check the parameters */
    assert_param ( IS_AES_IT ( AES_IT ) );
	
    if ( NewState != DISABLE )
    {
       AES->AES_CONFIG |= AES_IT;
    }
    else
    {
       AES->AES_CONFIG &= ( uint32_t ) ~ ( ( uint32_t ) AES_IT );
    }
}

/**
 * @brief  Checks whether the specified AES flag is set or not.
 * @param  AES_FLAG[in]:specifies the flag to check.
 *                  - AES_Flag_BUSY :In the process of calculation
 *                  - AES_Flag_CCFIF:Calculation completed
 * @retval The new state of AES_FLAG (SET or RESET).
 *                  -  RESET:Flag reset
 *                  -  SET :Flag up
 */
FlagStatus AES_GetFlagStatus ( AES_Flag_TypeDef AES_FLAG )
{
    FlagStatus bitstatus = RESET;
    /* Check the parameters */
    assert_param ( IS_GET_AES_FLAG ( AES_FLAG ) );

    if ( ( AES->TRNG_STS & AES_FLAG ) != ( uint16_t ) RESET )
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
 * @brief  Clears the AES pending flags.
 * @param  AES_FLAG[in]:specifies the flag to check.
 *                  - AES_Flag_CCFIF:Calculation completed
 * @retval None
 */
void AES_ClearFlag ( uint16_t AES_FLAG )
{
    /* Check the parameters */
    assert_param ( IS_GET_AES_FLAG ( AES_FLAG ) );

    /* Clear the flags */
    AES->TRNG_STS = ( uint32_t ) AES_FLAG;
}


/**
 * @}
 */
/* End of AES_Group2.	*/

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
