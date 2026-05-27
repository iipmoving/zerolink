/**
 ******************************************************************************
 * @file    sc32f1xxx_lpd.c
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   LPD function module
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
#include "sc32f1xxx_lpd.h"

/** @defgroup LPD_Exported_Functions_Group1 Configuration of the LPD computation unit functions
 *  @brief   Configuration of the LPD computation unit functions
 *
@verbatim
 ===============================================================================
                     ##### LPD configuration functions #####
 ===============================================================================
@endverbatim
  * @{
  */

/**
 * @brief  DeInitializes the LPD peripheral
 * @param  LPDx[out]: select the LPDx peripheral.
 *                  - LPD: Only LPD can be select the LPDx peripheral.
 * @retval None
 */
void LPD_DeInit ( LPD_TypeDef* LPDx )
{
    /* Check the parameters */
    assert_param ( IS_LPD_ALL_PERIPH ( LPDx ) );

    LPDx->LPD_CON = ( uint32_t ) 0x00000000U;
    LPDx->LPD_IDE = ( uint32_t ) 0x00000000U;
}

/**
 * @brief  Enables or disables the specified LPD peripheral.
 * @param  LPDx[out]: select the LPDx peripheral.
 *                  - LPD: Only LPD can be select the LPDx peripheral.
 * @param  NewState[in]: new state of the LPDx peripheral.
 *                  - DISABLE:Function disable
 *                  - ENABLE:Function enable
 * @retval None
 */
void LPD_Cmd ( LPD_TypeDef* LPDx, FunctionalState NewState )
{
    /* Check the parameters */
    assert_param ( IS_LPD_ALL_PERIPH ( LPDx ) );
    assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );

    if ( NewState != DISABLE )
    {
        /* Enable the LPD */
        LPDx->LPD_CON |= LPD_CON_LPDEN;
    }
    else
    {
        /* Disable the LPD */
        LPDx->LPD_CON &= ( uint32_t ) ~LPD_CON_LPDEN;
    }
}

/**
 * @brief  Configure the conversion mode of the LPD.
 * @param  LPDx[out]: select the LPDx peripheral.
 *                  - LPD: Only LPD can be select the LPDx peripheral.
 * @param  LPD_VLPD[in]: LPD threshold voltage level selection.VLPD represents the LPD threshold voltage threshold value.
 *                  - VLPD_1_85V_typ:The LPD threshold voltage is 1.85V.
 *                  - VLPD_2_05V_typ:The LPD threshold voltage is 2.05V.
 *                  - VLPD_2_25V_typ:The LPD threshold voltage is 2.25V.
 *                  - VLPD_2_45V_typ:The LPD threshold voltage is 2.45V.
 *                  - VLPD_2_65V_typ:The LPD threshold voltage is 2.65V.
 *                  - VLPD_2_85V_typ:The LPD threshold voltage is 2.85V.
 *                  - VLPD_3_05V_typ:The LPD threshold voltage is 3.05V.
 *                  - VLPD_3_25V_typ:The LPD threshold voltage is 3.25V.
 * @retval None
 */
void LPD_VLPDConfig ( LPD_TypeDef* LPDx, LPD_IS_TypeDef LPD_VLPD )
{
    uint32_t tmpreg;
    /* Check the parameters */
    assert_param ( IS_LPD_ALL_PERIPH ( LPDx ) );
	  assert_param ( IS_LPD_VLPD ( LPD_VLPD ) );

    tmpreg = LPDx->LPD_CON;
    tmpreg &= ( uint32_t ) ~ ( LPD_CON_LPDIS );

    /* Set the LPD channel type */
    tmpreg |= LPD_VLPD;

    LPDx->LPD_CON = tmpreg;
}

/**
 * @}
 */
/* End of LPD_Group1.	*/

/** @defgroup LPD_Group2 Interrupts and flags management functions
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
 * @brief  Enables or disables the specified LPD interrupts.
 * @param  LPDx[out]: select the LPDx peripheral.
 *                  - LPD: Only LPD can be select the LPDx peripheral.
 * @param  LPD_IT[in]: specifies the LPD interrupts sources to be enabled or disabled.
 *                  - LPD_IT_INTEN:LPD Interrupt: LPD Interrupt
 * @param  NewState[in]: new state of the LPD interrupts.
 *                  - DISABLE:Function disable
 *                  - ENABLE:Function enable
 * @retval None
 */
void LPD_ITConfig ( LPD_TypeDef* LPDx, uint16_t LPD_IT, FunctionalState NewState )
{
    /* Check the parameters */
    assert_param ( IS_LPD_ALL_PERIPH ( LPDx ) );
    assert_param ( IS_LPD_IT ( LPD_IT ) );
    assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );
    if ( NewState != DISABLE )
    {
        /* Enable the Interrupt sources */
        LPDx->LPD_IDE |= LPD_IT;
    }
    else
    {
        /* Disable the Interrupt sources */
        LPDx->LPD_IDE &= ( uint32_t ) ~LPD_IT;
    }
}

/**
 * @brief  Checks whether the specified LPD flag is set or not.
 * @param  LPDx[out]: select the LPDx peripheral.
 *                  - LPD: Only LPD can be select the LPDx peripheral.
 * @param  LPD_FLAG[in]:specifies the flag to check.
 *                  - LPD_Flag_LPDOF: LPD status Flag
 *                  - LPD_Flag_LPDIF: LPD Interrupt request Flag 
 * @retval The new state of LPD_FLAG (SET or RESET).
 *                  -  RESET:Flag reset
 *                  -  SET :Flag up
 */
FlagStatus LPD_GetFlagStatus ( LPD_TypeDef* LPDx, uint32_t LPD_FLAG )
{
    ITStatus bitstatus = RESET;
    /* Check the parameters */
    assert_param ( IS_LPD_ALL_PERIPH ( LPDx ) );

    if ( ( LPDx->LPD_CON & LPD_FLAG ) != RESET )
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
 * @brief  Clears the LPDx's pending flags.
 * @param  LPDx[out]: select the LPDx peripheral.
 *                  - LPD: Only LPD can be select the LPDx peripheral.
 * @param  LPD_FLAG[in]:specifies the flag bit to clear.
 *                  - LPD_Flag_LPDIF :LPD interrupt request Flag
 * @retval None
 */
void LPD_ClearFlag ( LPD_TypeDef* LPDx, uint32_t LPD_FLAG )
{
    /* Check the parameters */
    assert_param ( IS_LPD_ALL_PERIPH ( LPDx ) );

    /* Clear the flags */
    LPDx->LPD_CON |= ( uint32_t ) LPD_FLAG;
}

/**
 * @}
 */
/* End of LPD_Group2.	*/

#endif

/************************ (C) COPYRIGHT SOC Microelectronics *****END OF FILE****/
