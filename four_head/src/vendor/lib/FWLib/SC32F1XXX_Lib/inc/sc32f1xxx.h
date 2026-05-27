/**
 ******************************************************************************
 * @file    SC32F1XXX.h
 * @author  SOC SA Team
 * @version V1.9-BetaV6
 * @date    2026-04-03
 * @brief   CMSIS SC32F1xxx Device Peripheral Access Layer Header File.
 *          The file is the unique include file that the application programmer
 *          is using in the C source code, usually in main.c. This file contains:
 *           - Configuration section that allows to select:
 *           - The SC32F1xxx device used in the target application
 *
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


/** @addtogroup CMSIS_Device
  * @{
  */

/** @addtogroup sc32f1xxx
  * @{
  */

#ifndef SC32F1XXX_H
#define SC32F1XXX_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @addtogroup Library_configuration_section
  * @{
  */

#if !defined  (SC32F1)
#define SC32F1
#endif /* SC32F1 */

/* Programming Port UART Optimization Switch */
#define UARTOptSwitch 1  

#if defined(SC32f10xx)
  #include "sc32f10xx.h"
#elif defined(SC32R803)
  #include "sc32r803.h"
#elif defined(SC32f12xx)
  #include "sc32f12xx.h"
#elif defined(SC32f15xx)
  #include "sc32f15Gx.h"
#elif defined(SC32R601)
  #include "sc32r601.h"
#elif defined(SC32R805)
  #include "sc32r805.h"
#elif defined(SC32R806)
  #include "sc32r806.h"
#elif defined(SC32L14xx)
  #include "sc32L14xx.h"
#elif defined(SC32R807)
  #include "sc32r807.h"
#else
 #error "Please select first the target SC32F1xxx device used in your application (in SC32F1xxx.h file)"
#endif



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SC32F1xxx_H */

/**
  * @}
  */

/**
* @}
*/

