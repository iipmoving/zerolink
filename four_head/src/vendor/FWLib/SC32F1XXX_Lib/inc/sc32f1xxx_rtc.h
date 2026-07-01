/**
 ******************************************************************************
 * @file    sc32f1xxx_RTC.h
 * @author  SOC AE Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   Header file of RTC module.
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
#ifndef __sc32f1xxx_RTC_H
#define __sc32f1xxx_RTC_H

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

/** @addtogroup RTC
 * @{
 */

/* Exported enumerations ------------------------------------------------------------*/
/** @defgroup RTC_Exported_Enumerations RTC Exported Enumerations
 * @{
 */

/** @brief RTC_HourFormat RTC HourFormat
 * @{
 */
typedef enum
{
    RTC_HourFormat12  = ( uint32_t ) ( 0x00 << RTC_CON_AMPM_Pos ), /*!< 12-hour system */
    RTC_HourFormat24  = ( uint32_t ) ( 0x01 << RTC_CON_AMPM_Pos ), /*!< 24-hour system */
} RTC_HourFormat_TypeDef;
#define IS_RTC_HOURFORMAT(HOURFORMAT) (((HOURFORMAT) == RTC_HourFormat12) ||  \
																		   ((HOURFORMAT) == RTC_HourFormat24))
/**
 * @}
 */

/** @brief RTC_ClockSource RTC clock source
 * @{
 */

typedef enum
{
    RTC_ClockSource_NON      = ( uint32_t ) (0x00 << RTC_CON_RTCSEL_Pos),		 /*!< No clock (RTC clock source disabled) */
    RTC_ClockSource_LXT      = ( uint32_t ) (0x01 << RTC_CON_RTCSEL_Pos),		 /*!< Select LXT as the RTC clock source */
} RTC_ClockSource_Typedef;
#define IS_RTC_CLOCKSOURCE(CLOCKSOURCE) ((CLOCKSOURCE) == RTC_ClockSource_NON  ||\
                                         (CLOCKSOURCE) == RTC_ClockSource_LXT)

/**
 * @}
 */

/** @defgroup RTC_Week RTC Week  
 * @{
 */
typedef enum
{
    RTC_Alarm_Sunday      = ( uint32_t ) (0x01 << RTC_ALARM_WW_Pos),		 /*!< Set to Sunday */
    RTC_Alarm_Monday      = ( uint32_t ) (0x02 << RTC_ALARM_WW_Pos),		 /*!< Set to Monday */
    RTC_Alarm_Tuesday     = ( uint32_t ) (0x04 << RTC_ALARM_WW_Pos),		 /*!< Set to Tuesday */
    RTC_Alarm_Wednesday   = ( uint32_t ) (0x08 << RTC_ALARM_WW_Pos),		 /*!< Set to Wednesday */
		RTC_Alarm_Thursday    = ( uint32_t ) (0x10 << RTC_ALARM_WW_Pos),		 /*!< Set to Thursday */
    RTC_Alarm_Friday      = ( uint32_t ) (0x20 << RTC_ALARM_WW_Pos),		 /*!< Set to Friday */
    RTC_Alarm_Saturday    = ( uint32_t ) (0x40 << RTC_ALARM_WW_Pos),		 /*!< Set to Saturday */
} RTC_Week_Typedef;
#define IS_RTC_WEEK(WEEK) ((WEEK) == RTC_Alarm_Sunday  ||\
                           (WEEK) == RTC_Alarm_Monday ||\
                           (WEEK) == RTC_Alarm_Tuesday ||\
											     (WEEK) == RTC_Alarm_Wednesday  ||\
                           (WEEK) == RTC_Alarm_Thursday ||\
                           (WEEK) == RTC_Alarm_Friday ||\
													 (WEEK) == RTC_Alarm_Saturday)

/**
 * @}
 */

/** @brief RTC_IT RTC Interrupt
 * @{
 */
typedef enum
{
    RTC_IT_WALIE    = ( uint16_t )RTC_CON_WALIE,	    /*!< RTC alarm tnterrupt enable */
    RTC_IT_INTEN    = ( uint16_t )RTC_CON_INTEN,	    /*!< RTC interrupt enable */
} RTC_IT_TypeDef;
#define IS_RTC_IT(IT) ((((IT) & (uint16_t)0xFCFF) == 0x00) && ((IT) != (uint8_t)0x00))
/**
 * @}
 */

/** @brief RTC_IT RTC Interrupt
 * @{
 */
typedef enum
{
    RTC_CT_None         = ( uint16_t )0x00,	/*!< Do not use fixed-period interrupt function */
    RTC_CT_0_5Second    = ( uint16_t )0x01,	/*!< Once every 0.5 seconds (synchronized with second accumulation) */
		RTC_CT_1Second      = ( uint16_t )0x02,	/*!< Once every 1 second (simultaneous with second accumulation) */
		RTC_CT_1Minute      = ( uint16_t )0x03,	/*!< Once every 1 minute (at the 00-second mark of each minute) */
		RTC_CT_1Hour        = ( uint16_t )0x04,	/*!< Once every 1 hour (at the 00-minute 00-second mark of each hour) */
		RTC_CT_1Day         = ( uint16_t )0x05,	/*!< Once every 1 day (at 00:00:00 daily in 24-hour format, or 12:00:00 AM daily in 12-hour format) */
		RTC_CT_1Month       = ( uint16_t )0x06,	/*!< Once every 1 month (at 00:00:00 AM on the 1st day of each month in 24-hour format, 
	                                               or 12:00:00 AM on the 1st day of each month in 12-hour format)*/
} RTC_CT_TypeDef;
#define IS_RTC_CT(CT) ((CT) == RTC_CT_None  ||\
                       (CT) == RTC_CT_0_5Second  ||\
											 (CT) == RTC_CT_1Second  ||\
										   (CT) == RTC_CT_1Minute  ||\
											 (CT) == RTC_CT_1Hour  ||\
										   (CT) == RTC_CT_1Day  ||\
											 (CT) == RTC_CT_1Month)
/**
 * @}
 */

/** @brief RTC_FLAG RTC Flag
 * @{
 */
typedef enum
{
    RTC_Flag_RTCCTIF  = ( uint8_t ) RTC_STS_RTCCTIF,	/*!< Fixed-Period interrupt Flag */
    RTC_Flag_WALIF    = ( uint8_t ) RTC_STS_WALIF,    /*!< Alarm detection Flag */
} RTC_FLAG_TypeDef;
#define IS_RTC_FLAG(FLAG) ((((FLAG) & (uint8_t)0xFC) == 0x00) && ((FLAG) != (uint8_t)0x00))

/**
 * @}
 */

/**
 * @}
 */
/* End of enumerations -----------------------------------------------------*/


#define IS_RTC_ALL_PERIPH(PERIPH) ((PERIPH) == RTC)

/**
 * @}
 */
/* End of constants -----------------------------------------------------*/

/** @defgroup RTC_Exported_Struct RTC Exported Struct
 * @{
 */

/** @brief RTC_InitTypeDef RTC Configuration Structure definition
 * @{
 */
typedef struct
{
    uint32_t RTC_AMPM; /*!< Hour System Setting.
																  This parameter can be a value of @ref RTC_HourFormat_TypeDef */

    uint32_t RTC_RTCSEL; /*!< RTC Clock Source Selection.
																	This parameter can be a value of @ref RTC_ClockSource_Typedef */

} RTC_InitTypeDef;
/**
 * @}
 */

/** @brief RTC_DateTypeDef RTC Date base Configuration Structure definition
 * @{
 */
typedef struct
{
    uint32_t RTC_YEAR;   /*!< Year counter value setting. */

    uint32_t RTC_MONTH;  /*!< Month counter value setting. */
		
		uint32_t RTC_WEEK;   /*!< week counter value setting. */
	
		uint32_t RTC_DAY;    /*!< day counter value setting. */

} RTC_DateTypeDef;
/**
 * @}
 */

/** @brief RTC_TimeTypeDef RTC Time base Configuration Structure definition
 * @{
 */
typedef struct
{
    uint32_t RTC_HOUR;   /*!< Hour counter value setting. */

    uint32_t RTC_MIN;    /*!< Minute counter value setting. */
		
		uint32_t RTC_SEC;    /*!< Second counter value setting. */
	
} RTC_TimeTypeDef;
/**
 * @}
 */

/** @brief RTC_AlarmTypeDef RTC Alarm base Configuration Structure definition
 * @{
 */
typedef struct
{
    uint32_t RTC_Alarm_WW; /*!<  Specifies sampling time of RTC. */

    uint32_t RTC_Alarm_WH; /*!< Specifies Input channel of RTC. */
		
		uint32_t RTC_Alarm_WM; /*!< Specifies Input channel of RTC. */
	
} RTC_AlarmTypeDef;
/**
 * @}
 */


/**
 * @}
 */
/* End of struct -----------------------------------------------------*/

/** @addtogroup RTC_Exported_Functions RTC Exported Functions
 * @{
 */

/* RTC Base functions ********************************************************/
void RTC_DeInit ( RTC_TypeDef* RTCx );
void RTC_Init ( RTC_TypeDef* RTCx, RTC_InitTypeDef* RTC_InitStruct );
void RTC_StructInit ( RTC_InitTypeDef* RTC_InitStruct );
void RTC_SetDate ( RTC_TypeDef* RTCx, RTC_DateTypeDef* RTC_Date );
void RTC_SetTime ( RTC_TypeDef* RTCx, RTC_TimeTypeDef* RTC_Time );
void RTC_SetAlarm ( RTC_TypeDef* RTCx, RTC_AlarmTypeDef* RTC_Alarm );

/* Date and Time acquisition functions  *************************************/
void RTC_GetDate ( RTC_TypeDef* RTCx, RTC_DateTypeDef* RTC_Date );
void RTC_GetTime ( RTC_TypeDef* RTCx , RTC_TimeTypeDef* RTC_Time );

/* Interrupts and flags  ****************************************************/
void RTC_ITConfig ( RTC_TypeDef* RTCx, uint16_t RTC_IT, FunctionalState NewState );
void RTC_SetCT ( RTC_TypeDef* RTCx, RTC_CT_TypeDef RTC_CT );
FlagStatus RTC_GetFlagStatus ( RTC_TypeDef* RTCx, RTC_FLAG_TypeDef RTC_FLAG );
void RTC_ClearFlag ( RTC_TypeDef* RTCx, uint32_t RTC_FLAG );

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
