/**
 ******************************************************************************
 * @file    sc32f1xxx_rtc.c
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   RTC function module
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
#include "sc32f1xxx_rtc.h"

/** @defgroup RTC_Exported_Functions_Group1 Configuration of the RTC computation unit functions
 *  @brief   Configuration of the RTC computation unit functions
 *
@verbatim
 ===============================================================================
                     ##### RTC configuration functions #####
 ===============================================================================
@endverbatim
  * @{
  */

/**
 * @brief  DeInitializes the RTC peripheral
 * @param  RTCx[out]: select the RTCx peripheral.
 *                  - RTC: Only RTC can be select the RTCx peripheral.
 * @retval None
 */
void RTC_DeInit ( RTC_TypeDef* RTCx )
{
    /* Check the parameters */
    assert_param ( IS_RTC_ALL_PERIPH ( RTCx ) );

    RTCx->RTC_CON = ( uint32_t ) 0x00000000U;
    RTCx->RTC_CFG0 = ( uint32_t ) 0x00000000U;
    RTCx->RTC_CFG1 = ( uint32_t ) 0x00000000U;
    RTCx->RTC_ALARM = ( uint32_t ) 0x00000000U;
		RTCx->RTC_STS = ( uint32_t ) 0x00000001U;
}

/**
 * @brief  Initializes the RTC peripheral.
 * @param  RTCx[out]: select the RTCx peripheral.
 *                  - RTC: Only RTC can be select the RTCx peripheral.
 * @param  RTC_InitStruct[out]: Pointer to structure RTC_InitTypeDef,
 *                              to be initialized.
 * @retval None
 */
void RTC_Init ( RTC_TypeDef* RTCx, RTC_InitTypeDef* RTC_InitStruct )
{
		uint32_t tmpreg;
		/* Check the parameters */
		assert_param(IS_RTC_ALL_PERIPH(RTCx));
		assert_param(IS_RTC_CONVMODE(RTC_InitStruct->RTC_AMPM));
		assert_param(IS_RTC_PRESCALER(RTC_InitStruct->RTC_RTCSEL));
		
		tmpreg = RTCx->RTC_CON;
		tmpreg &= ~( RTC_CON_RTCSEL | RTC_CON_AMPM );
		tmpreg |= ( RTC_InitStruct->RTC_AMPM |
								RTC_InitStruct->RTC_RTCSEL);
		RTCx->RTC_CON = tmpreg;
}

/**
  * @brief  Fills each RTC_InitStruct member with its default value.
  * @param  RTC_InitStruct[out]: Pointer to structure RTC_InitTypeDef,
  *                              to be initialized.
  * @retval None
  */
void RTC_StructInit ( RTC_InitTypeDef* RTC_InitStruct )
{
    /* Set the default configuration */
    RTC_InitStruct->RTC_AMPM = RTC_HourFormat12;
    RTC_InitStruct->RTC_RTCSEL = RTC_ClockSource_NON;
}


/**
 * @brief  Configure the conversion mode of the RTC.
 * @param  RTCx[out]: select the RTCx peripheral.
 *                  - RTC: Only RTC can be select the RTCx peripheral.
 * @param  RTC_Date[out]: Pointer to structure RTC_DateTypeDef,
 *                        to be initialized.
 * @retval None
 */
void RTC_SetDate ( RTC_TypeDef* RTCx, RTC_DateTypeDef* RTC_Date )
{
		uint32_t tmpreg;
    /* Check the parameters */
    assert_param ( IS_RTC_ALL_PERIPH ( RTCx ) );

		tmpreg = RTCx->RTC_CFG0;
		tmpreg &= ~(uint32_t)0xFF1F033F;
		tmpreg |= (((uint32_t)(RTC_Date->RTC_YEAR / 10 << 4 | RTC_Date->RTC_YEAR % 10) << RTC_CFG0_YEAR_Pos) | \
							((uint32_t)(RTC_Date->RTC_MONTH / 10 << 4 | RTC_Date->RTC_MONTH % 10) << RTC_CFG0_MONTH_Pos) | \
							((uint32_t)RTC_Date->RTC_WEEK << RTC_CFG0_WEEK_Pos) | \
							((uint32_t)(RTC_Date->RTC_DAY / 10 << 4 | RTC_Date->RTC_DAY % 10)));
		
		while(!(RTCx->RTC_CFG1 & 0x80)){}  //Writable register starting from the half-second position.	
		
		RTCx->RTC_CFG0 = (uint32_t)tmpreg;
}

/**
 * @brief  Configure the RTC input channel
 * @param  RTCx[out]: select the RTCx peripheral.
 *                  - RTC: Only RTC can be select the RTCx peripheral.
 * @param  RTC_Time[out]: Pointer to structure RTC_TimeTypeDef,
 *                        to be initialized.
 * @retval None
 */
void RTC_SetTime ( RTC_TypeDef* RTCx, RTC_TimeTypeDef* RTC_Time )
{
    uint32_t tmpreg;
    /* Check the parameters */
    assert_param ( IS_RTC_ALL_PERIPH ( RTCx ) );
	
		tmpreg = RTCx->RTC_CFG1;
		tmpreg &= ~(uint32_t)0x003F7FFF;		
		tmpreg |= (((uint32_t)(RTC_Time->RTC_HOUR / 10 << 4 | RTC_Time->RTC_HOUR % 10) << RTC_CFG1_HOUR_Pos) | \
               ((uint32_t)(RTC_Time->RTC_MIN / 10 << 4 | RTC_Time->RTC_MIN % 10) << RTC_CFG1_MIN_Pos) | \
               ((uint32_t)(RTC_Time->RTC_SEC / 10 << 4 | RTC_Time->RTC_SEC % 10)));
	
		while(!(RTCx->RTC_CFG1 & 0x80)){}  //Writable register starting from the half-second position.	
		RTCx->RTC_CFG1 = (uint32_t)tmpreg;
}

/**
 * @brief  Gets the RTC input channel.
 * @param  RTCx[out]: select the RTCx peripheral.
 *                  - RTC: Only RTC can be select the RTCx peripheral.
 * @param  RTC_Alarm[out]: Pointer to structure RTC_AlarmTypeDef,
 *                         to be initialized.
 * @retval None
 */
void RTC_SetAlarm ( RTC_TypeDef* RTCx, RTC_AlarmTypeDef* RTC_Alarm )
{
		uint32_t tmpreg;
		/* Check the parameters */
		assert_param(IS_RTC_ALL_PERIPH(RTCx));

		tmpreg = RTCx->RTC_ALARM;
	  tmpreg &= ~(uint32_t)0x007F3F7F;
		tmpreg |= (((uint32_t)RTC_Alarm->RTC_Alarm_WW) | \
							((uint32_t)(RTC_Alarm->RTC_Alarm_WH / 10<<4 | RTC_Alarm->RTC_Alarm_WH % 10) << RTC_ALARM_WH_Pos) | \
							((uint32_t)(RTC_Alarm->RTC_Alarm_WM / 10<<4 | RTC_Alarm->RTC_Alarm_WM % 10)));
		
		RTCx->RTC_ALARM = (uint32_t)tmpreg;
}

/**
 * @}
 */
/* End of RTC_Group1.	*/

/** @defgroup RTC_Group2 Date and Time acquisition functions
 *  @brief   Date and Time acquisition functions
 *
@verbatim
 ===============================================================================
                 ##### Date and Time acquisition functions #####
 ===============================================================================
@endverbatim
  * @{
  */

/**
 * @brief  Read RTC Date.
 * @param  RTCx[out]: select the RTCx peripheral.
 *                  - RTC: Only RTC can be select the RTCx peripheral.
 * @param  RTC_Date[out]: Pointer to structure RTC_DateTypeDef,
 *                        to be initialized.
 * @retval None
 */
void RTC_GetDate ( RTC_TypeDef* RTCx, RTC_DateTypeDef* RTC_Date )
{
		uint32_t tmpreg;
		uint8_t year, month, date, week;
    /* Check the parameters */
    assert_param ( IS_RTC_ALL_PERIPH ( RTCx ) );

    while(!(RTCx->RTC_CFG1 & 0x80)){}  //Writable register starting from the half-second position.
		tmpreg = RTCx->RTC_CFG0;
	
		year  = (uint8_t)((tmpreg & ((uint32_t)RTC_CFG0_YEAR)) >> RTC_CFG0_YEAR_Pos);
		month = (uint8_t)((tmpreg & ((uint32_t)RTC_CFG0_MONTH)) >> RTC_CFG0_MONTH_Pos);
		week  = (uint8_t)((tmpreg & ((uint32_t)RTC_CFG0_WEEK)) >> RTC_CFG0_WEEK_Pos);
		date  = (uint8_t)(tmpreg & RTC_CFG0_DAY);
	
		RTC_Date->RTC_YEAR = year/16 * 10 + year%16;
		RTC_Date->RTC_MONTH = month/16 * 10 + month%16;
		RTC_Date->RTC_WEEK = week;
		RTC_Date->RTC_DAY = date/16 * 10 + date%16;
}

/**
 * @brief  Read RTC Time.
 * @param  RTCx[out]: select the RTCx peripheral.
 *                  - RTC: Only RTC can be select the RTCx peripheral.
 * @param  RTC_Time[out]: Pointer to structure RTC_TimeTypeDef,
 *                        to be initialized.
 * @retval None
 */
void RTC_GetTime ( RTC_TypeDef* RTCx , RTC_TimeTypeDef* RTC_Time )
{
		uint32_t tmpreg;
		uint8_t hours, minutes, seconds;
    /* Check the parameters */
    assert_param ( IS_RTC_ALL_PERIPH ( RTCx ) );
	
		while(!(RTCx->RTC_CFG1 & 0x80)){}  //Writable register starting from the half-second position.
		tmpreg = RTCx->RTC_CFG1;

    hours   = (uint8_t)((tmpreg & ((uint32_t)RTC_CFG1_HOUR)) >> RTC_CFG1_HOUR_Pos);
		minutes = (uint8_t)((tmpreg & ((uint32_t)RTC_CFG1_MIN)) >> RTC_CFG1_MIN_Pos);
		seconds = (uint8_t)(tmpreg & RTC_CFG1_SEC);

		RTC_Time->RTC_HOUR  = hours/16 * 10 + hours%16;
		RTC_Time->RTC_MIN   = minutes/16 * 10 + minutes%16;
		RTC_Time->RTC_SEC   = seconds/16 * 10 + seconds%16;
}

/**
 * @}
 */
/* End of RTC_Group2.	*/

/** @defgroup RTC_Group3 Interrupts and flags management functions
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
 * @brief  Enables or disables the specified RTC interrupts.
 * @param  RTCx[out]: select the RTCx peripheral.
 *                  - RTC: Only RTC can be select the RTCx peripheral.
 * @param  RTC_IT[in]: specifies the RTC interrupts sources to be enabled or disabled.
 *                  - RTC_IT_WALIE:RTC alarm tnterrupt enable
 *                  - RTC_IT_INTEN:RTC interrupt enable
 * @param  NewState[in]: new state of the RTC interrupts.
 *                  - DISABLE:Function disable
 *                  - ENABLE:Function enable
 * @retval None
 */
void RTC_ITConfig ( RTC_TypeDef* RTCx, uint16_t RTC_IT, FunctionalState NewState )
{
    /* Check the parameters */
    assert_param ( IS_RTC_ALL_PERIPH ( RTCx ) );
    assert_param ( IS_RTC_IT ( RTC_IT ) );
    assert_param ( IS_FUNCTIONAL_STATE ( NewState ) );
    if ( NewState != DISABLE )
    {
        /* Enable the Interrupt sources */
        RTCx->RTC_CON |= RTC_IT;
    }
    else
    {
        /* Disable the Interrupt sources */
        RTCx->RTC_CON &= ( uint32_t ) ~RTC_IT;
    }
}

/**
 * @brief  Fixed-Period interrupt time setting.
 * @param  RTCx[out]: select the RTCx peripheral.
 *                  - RTC: Only RTC can be select the RTCx peripheral.
 * @param  RTC_CT[in]: Fixed-Period interrupt time selection.
 *                  - RTC_CT_None :Do not use fixed-period interrupt function
 *                  - RTC_CT_0_5Second :Once every 0.5 seconds (synchronized with second accumulation)
 *                  - RTC_CT_1Second :Once every 1 second (simultaneous with second accumulation)
 *                  - RTC_CT_1Minute :Once every 1 minute (at the 00-second mark of each minute)
 *                  - RTC_CT_1Hour :Once every 1 hour (at the 00-minute 00-second mark of each hour)
 *                  - RTC_CT_1Day :Once every 1 day (at 00:00:00 daily in 24-hour format, or 12:00:00 AM daily in 12-hour format)
 *                  - RTC_CT_1Month :Once every 1 month (at 00:00:00 AM on the 1st day of each month in 24-hour format, 
 *                                   or 12:00:00 AM on the 1st day of each month in 12-hour format)
 * @retval None
 */
void RTC_SetCT ( RTC_TypeDef* RTCx, RTC_CT_TypeDef RTC_CT )
{
    /* Check the parameters */
    assert_param ( IS_RTC_ALL_PERIPH ( RTCx ) );
    assert_param ( IS_RTC_CT ( RTC_CT ) );

		RTCx->RTC_CON &= ( uint32_t ) ~RTC_CON_CT;
		RTCx->RTC_CON |= RTC_CT;
}

/**
 * @brief  Checks whether the specified RTC flag is set or not.
 * @param  RTCx[out]: select the RTCx peripheral.
 *                  - RTC: Only RTC can be select the RTCx peripheral.
 * @param  RTC_FLAG[in]:specifies the flag to check.
 *                  - RTC_Flag_RTCCTIF :Fixed-Period interrupt Flag
 *                  - RTC_Flag_WALIF :Alarm detection Flag
 * @retval The new state of RTC_FLAG (SET or RESET).
 *                  -  RESET:Flag reset
 *                  -  SET :Flag up
 */
FlagStatus RTC_GetFlagStatus ( RTC_TypeDef* RTCx, RTC_FLAG_TypeDef RTC_FLAG )
{
    ITStatus bitstatus = RESET;
    /* Check the parameters */
    assert_param ( IS_RTC_ALL_PERIPH ( RTCx ) );
		assert_param ( IS_RTC_FLAG ( RTC_FLAG ) );

    if ( ( RTCx->RTC_STS & RTC_FLAG ) != ( uint16_t ) RESET )
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
 * @brief  Clears the RTCx's pending flags.
 * @param  RTCx[out]: select the RTCx peripheral.
 *                  - RTC: Only RTC can be select the RTCx peripheral.
 * @param  RTC_FLAG[in]:specifies the flag bit to clear.
 *                  - RTC_Flag_RTCCTIF :Fixed-Period interrupt Flag
 *                  - RTC_Flag_WALIF :Alarm detection Flag
 * @retval None
 */
void RTC_ClearFlag ( RTC_TypeDef* RTCx, uint32_t RTC_FLAG )
{
    /* Check the parameters */
    assert_param ( IS_RTC_ALL_PERIPH ( RTCx ) );
		assert_param ( IS_RTC_FLAG ( RTC_FLAG ) );

    /* Clear the flags */
		RTCx->RTC_STS |= ( uint32_t ) RTC_FLAG;
}

/**
 * @}
 */
/* End of RTC_Group3.	*/

#endif




/************************ (C) COPYRIGHT SOC Microelectronics *****END OF FILE****/
