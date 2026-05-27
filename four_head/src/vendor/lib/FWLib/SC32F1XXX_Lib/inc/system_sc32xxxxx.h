/**
******************************************************************************
 * @file    system_sc32xxxxx.h
 * @author  SOC SA Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   CMSIS Cortex-M0+ Device System Source File for sc32xxxxx devices.
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

/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup sc32xxxxx_system
  * @{
  */

/**
  * @brief Define to prevent recursive inclusion
  */
#ifndef SYSTEM_SOC0003_H
#define SYSTEM_SOC0003_H

#ifdef __cplusplus
 extern "C" {
#endif

/** @addtogroup sc32xxxxx_System_Includes
  * @{
  */

/**
  * @}
  */


/** @addtogroup sc32xxxxx_System_Exported_types
  * @{
  */
extern uint32_t SystemCoreClock;         /*!< System Clock Frequency (Core Clock) */

extern const uint32_t AHBPrescTable[16];  /*!<  AHB prescalers table values */
extern const uint32_t APBPrescTable[8];   /*!< APB prescalers table values */

/**
  * @}
  */

/** @addtogroup sc32xxxxx_System_Exported_Constants
  * @{
  */

/**
  * @}
  */

/** @addtogroup sc32xxxxx_System_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @addtogroup sc32xxxxx_System_Exported_Functions
  * @{
  */

extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /*SYSTEM_SC32XXXXX_H */

/**
  * @}
  */

/**
  * @}
  */
/************************ (C) COPYRIGHT SOC Microelectronics *****END OF FILE****/
