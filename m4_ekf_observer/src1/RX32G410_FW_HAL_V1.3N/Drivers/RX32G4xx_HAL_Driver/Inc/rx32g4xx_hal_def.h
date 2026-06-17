/*************************************************************************************************
* Copyright <2024> <Icore Technology (Nanjing) Co.,Ltd>
* All Rights Reserved,
*
* Redistribution and use in source and binary forms, with or without modification, are permitted
* provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this list of
*    conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of
*    conditions and the following disclaimer in the documentation and/or other materials provided
*    with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to
*    endorse or promote products derived from this software without specific prior written
*    permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************
@file
**************************************************************************************************
*                                              rx32g4xx
*                                          Library Function
*
*                                   Copyright 2024, RX Tech, Corp.
*                                        All Rights Reserved
*
*
*   Project : rx32g4xx
*   File    : rx32g4xx_hal_def.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef _RX32G4XX_HAL_DEF_H_
#define _RX32G4XX_HAL_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx.h"
#include <stddef.h>

/* Exported types ------------------------------------------------------------*/

/** @addtogroup HAL_Def
  * @{
  */

/******************************************************************************/
/*                            HAL_Def Structures                              */
/******************************************************************************/

/** @defgroup HAL_Def_Structure_Definitions HAL Def Structure Definitions
  * @{
  */

/** @defgroup HAL_Status_Structure_Definitions HAL Status Structure Definitions
  * @ingroup  HAL_Def_Structure_Definitions
  * @{
  */
/**
  * @brief  HAL Status structures definition
  */
typedef enum
{
  HAL_OK       = 0x00U,
  HAL_ERROR    = 0x01U,
  HAL_BUSY     = 0x02U,
  HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;
/**
  * end of HAL_Status_Structure_Definitions @}
  */

/** @defgroup HAL_Lock_Structure_Definitions HAL Lock Structure Definitions
  * @ingroup  HAL_Def_Structure_Definitions
  * @{
  */
/**
  * @brief  HAL Lock structures definition
  */
typedef enum
{
  HAL_UNLOCKED = 0x00U,
  HAL_LOCKED   = 0x01U
} HAL_LockTypeDef;
/**
  * end of HAL_Lock_Structure_Definitions @}
  */

/**
  * end of HAL_Def_Structure_Definitions @}
  */


/******************************************************************************/
/*                            HAL_Def Parameters                              */
/******************************************************************************/

/** @defgroup HAL_Def_Parameter_Definitions HAL Def Parameter Definitions
  * @{
  */

/**
  * end of HAL_Def_Parameter_Definitions @}
  */


/******************************************************************************/
/*                              HAL_Def Macro                                 */
/******************************************************************************/

/** @defgroup HAL_Def_Macro_Definitions HAL Def Macro Definitions
  * @{
  */

#define HAL_MAX_DELAY      0xFFFFFFFFU

#define HAL_IS_BIT_SET(REG, BIT)         (((REG) & (BIT)) == (BIT))
#define HAL_IS_BIT_CLR(REG, BIT)         (((REG) & (BIT)) == 0U)

#define __HAL_LINKDMA(__HANDLE__, __PPP_DMA_FIELD__, __DMA_HANDLE__) \
  do{                                                                \
    (__HANDLE__)->__PPP_DMA_FIELD__ = &(__DMA_HANDLE__);             \
    (__DMA_HANDLE__).Parent = (__HANDLE__);                          \
  } while(0)

#if !defined(UNUSED)
#define UNUSED(X) (void)X         /* To avoid gcc/g++ warnings */
#endif /* UNUSED */

/** @brief  Reset the Handle's State field.
  * @param  __HANDLE__: specifies the Peripheral Handle.
  * @retval None
  * @note   This macro can be used for the following purpose:
  *           - When the Handle is declared as local variable; before passing it as parameter
  *             to HAL_PPP_Init() for the first time, it is mandatory to use this macro
  *             to set to 0 the Handle's "State" field.
  *             Otherwise, "State" field may have any random value and the first time the function
  *             HAL_PPP_Init() is called, the low level hardware initialization will be missed
  *             (i.e. HAL_PPP_MspInit() will not be executed).
  *           - When there is a need to reconfigure the low level hardware: instead of calling
  *             HAL_PPP_DeInit() then HAL_PPP_Init(), user can make a call to this macro then HAL_PPP_Init().
  *             In this later function, when the Handle's "State" field is set to 0, it will execute the function
  *             HAL_PPP_MspInit() which will reconfigure the low level hardware.
  */
#define __HAL_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = 0)

#if (USE_RTOS == 1U)
/* Reserved for future use */
#error " USE_RTOS should be 0 in the current HAL release "
#else
#define __HAL_LOCK(__HANDLE__)             \
  do{                                      \
    if((__HANDLE__)->Lock == HAL_LOCKED)   \
    {                                      \
      return HAL_BUSY;                     \
    }                                      \
    else                                   \
    {                                      \
      (__HANDLE__)->Lock = HAL_LOCKED;     \
    }                                      \
  }while (0U)

#define __HAL_UNLOCK(__HANDLE__)           \
  do{                                      \
    (__HANDLE__)->Lock = HAL_UNLOCKED;     \
  }while (0U)
#endif /* USE_RTOS */

/**
  * end of HAL_Def_Macro_Definitions @}
  */


/******************************************************************************/
/*                             HAL_Def Functions                              */
/******************************************************************************/

/** @defgroup HAL_Def_Function_Definitions HAL Def Function Definitions
  * @{
  */

/**
  * end of HAL_Def_Function_Definitions @}
  */


/**
  * end of HAL_Def @}
  */


#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) /* ARM Compiler V6 */
  #ifndef __weak
  #define __weak    __attribute__((weak))
  #endif /* __weak */
  #ifndef __packed
  #define __packed  __attribute__((packed))
  #endif /* __packed */

#elif defined ( __GNUC__ ) && !defined (__CC_ARM) /* GNU Compiler */
  #ifndef __weak
  #define __weak    __attribute__((weak))
  #endif /* __weak */
  #ifndef __packed
  #define __packed  __attribute__((__packed__))
  #endif /* __packed */

#endif /* __GNUC__ */

/* Macro to get variable aligned on 4-bytes, for __ICCARM__ the directive "#pragma data_alignment=4" must be used instead */
#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) /* ARM Compiler V6 */
  #ifndef __ALIGN_BEGIN
  #define __ALIGN_BEGIN
  #endif /* __ALIGN_BEGIN */
  #ifndef __ALIGN_END
  #define __ALIGN_END      __attribute__((aligned (4)))
  #endif /* __ALIGN_END */

#elif defined ( __GNUC__ ) && !defined (__CC_ARM) /* GNU Compiler */
  #ifndef __ALIGN_BEGIN
  #define __ALIGN_BEGIN
  #endif /* __ALIGN_BEGIN */
  #ifndef __ALIGN_END
  #define __ALIGN_END      __attribute__((aligned (4U)))
  #endif /* __ALIGN_END */

#elif defined (__CC_ARM) /* ARM Compiler V5*/
  #ifndef __ALIGN_BEGIN
  #define __ALIGN_BEGIN    __align(4U)
  #endif /* __ALIGN_BEGIN */
  #ifndef __ALIGN_END
  #define __ALIGN_END
  #endif /* __ALIGN_END */

#else
  #ifndef __ALIGN_BEGIN
  #define __ALIGN_BEGIN
  #endif /* __ALIGN_BEGIN */
  #ifndef __ALIGN_END
  #define __ALIGN_END
  #endif /* __ALIGN_END */

#endif /* __GNUC__ */

/**
  * @brief  __RAM_FUNC definition
  */
#if defined ( __CC_ARM   ) || (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
/* ARM Compiler V4/V5 and V6
   --------------------------
   RAM functions are defined using the toolchain options.
   Functions that are executed in RAM should reside in a separate source module.
   Using the 'Options for File' dialog you can simply change the 'Code / Const'
   area of a module to a memory space in physical RAM.
   Available memory areas are declared in the 'Target' tab of the 'Options for Target'
   dialog.
*/
#define __RAM_FUNC

#elif defined ( __ICCARM__ )
/* ICCARM Compiler
   ---------------
   RAM functions are defined using a specific toolchain keyword "__ramfunc".
*/
#define __RAM_FUNC __ramfunc

#elif defined   (  __GNUC__  )
/* GNU Compiler
   ------------
  RAM functions are defined using a specific toolchain attribute
   "__attribute__((section(".RamFunc")))".
*/
#define __RAM_FUNC __attribute__((section(".RamFunc")))

#endif /* __CC_ARM */

/**
  * @brief  __NOINLINE definition
  */
#if defined ( __CC_ARM   ) || (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)) || defined   (  __GNUC__  )
/* ARM V4/V5 and V6 & GNU Compiler
   -------------------------------
*/
#define __NOINLINE __attribute__ ( (noinline) )

#elif defined ( __ICCARM__ )
/* ICCARM Compiler
   ---------------
*/
#define __NOINLINE _Pragma("optimize = no_inline")

#endif /* __CC_ARM || __GNUC__ */


#ifdef __cplusplus
}
#endif

#endif /* _RX32G4XX_HAL_DEF_H_ */
