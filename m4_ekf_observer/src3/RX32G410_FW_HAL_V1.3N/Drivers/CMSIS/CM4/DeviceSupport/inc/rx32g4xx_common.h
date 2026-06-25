/*********************************************************************************************
Copyright <2024> <Icore Technology (Nanjing)  Co.,Ltd>
All Rights Reserved,
Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials provided
with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to
endorse or promote products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************/

/*
*********************************************************************************************************
*                                              rx32g4xx
*                                           Library Function
*
*                                   Copyright 2024, RX Tech, Corp.
*                                        All Rights Reserved
*
*
* Project      : rx32g4xx
* File         : rx32g4xx_common.h
* By           : RX_DV_Team
*********************************************************************************************************
*/


#ifndef __RX32G4XX_COMMON_H__
#define __RX32G4XX_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "rx32g4xx_config_def.h"

/* Exported typedef ----------------------------------------------------------*/
typedef enum {DISABLE = 0, ENABLE = !DISABLE }  FunctionalState;
typedef enum {RESET   = 0, SET    = !RESET   }  FlagStatus, ITStatus;
typedef enum {FALSE   = 0, TRUE   = !FALSE   }  Bool;
typedef enum {SUCCESS = 0, ERROR  = !SUCCESS }  ErrorStatus;

/**
  * @brief  Status structures definition
  */
typedef enum
{
  STATUS_OK       = 0x00U,
  STATUS_ERROR    = 0x01U,
  STATUS_BUSY     = 0x02U,
  STATUS_TIMEOUT  = 0x03U
} StatusTypeDef;


/* Exported define -----------------------------------------------------------*/
#define BIT0            ((uint32_t)0x00000001)
#define BIT1            ((uint32_t)0x00000002)
#define BIT2            ((uint32_t)0x00000004)
#define BIT3            ((uint32_t)0x00000008)
#define BIT4            ((uint32_t)0x00000010)
#define BIT5            ((uint32_t)0x00000020)
#define BIT6            ((uint32_t)0x00000040)
#define BIT7            ((uint32_t)0x00000080)
#define BIT8            ((uint32_t)0x00000100)
#define BIT9            ((uint32_t)0x00000200)
#define BIT10           ((uint32_t)0x00000400)
#define BIT11           ((uint32_t)0x00000800)
#define BIT12           ((uint32_t)0x00001000)
#define BIT13           ((uint32_t)0x00002000)
#define BIT14           ((uint32_t)0x00004000)
#define BIT15           ((uint32_t)0x00008000)
#define BIT16           ((uint32_t)0x00010000)
#define BIT17           ((uint32_t)0x00020000)
#define BIT18           ((uint32_t)0x00040000)
#define BIT19           ((uint32_t)0x00080000)
#define BIT20           ((uint32_t)0x00100000)
#define BIT21           ((uint32_t)0x00200000)
#define BIT22           ((uint32_t)0x00400000)
#define BIT23           ((uint32_t)0x00800000)
#define BIT24           ((uint32_t)0x01000000)
#define BIT25           ((uint32_t)0x02000000)
#define BIT26           ((uint32_t)0x04000000)
#define BIT27           ((uint32_t)0x08000000)
#define BIT28           ((uint32_t)0x10000000)
#define BIT29           ((uint32_t)0x20000000)
#define BIT30           ((uint32_t)0x40000000)
#define BIT31           ((uint32_t)0x80000000)


/* Exported macro ------------------------------------------------------------*/
#define SET(n,c)                ((n)|=(c))
#define CLR(n,c)                ((n)&=~(c))

#define IS_BIT_SET(REG, BIT)    (((REG) & (BIT)) != 0U)
#define IS_BIT_CLR(REG, BIT)    (((REG) & (BIT)) == 0U)

#define SET_BIT(REG, BIT)       ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)     ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)      ((REG) & (BIT))

#define CLEAR_REG(REG)          ((REG) = (0x0))
#define WRITE_REG(REG, VAL)     ((REG) = (VAL))
#define READ_REG(REG)           ((REG))

#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

#define POSITION_VAL(VAL)       (__CLZ(__RBIT(VAL)))

#define M8(adr)                 (*((volatile uint8_t *)  (adr)))
#define M16(adr)                (*((volatile uint16_t *) (adr)))
#define M32(adr)                (*((volatile uint32_t *) (adr)))

#define _UNUSED(X)              (void)X

#define ATOMIC_SET_BIT(REG, BIT)                             \
  do {                                                       \
    uint32_t val;                                            \
    do {                                                     \
      val = __LDREXW((__IO uint32_t *)&(REG)) | (BIT);       \
    } while ((__STREXW(val,(__IO uint32_t *)&(REG))) != 0U); \
  } while(0)

/* Atomic 32-bit register access macro to clear one or several bits */
#define ATOMIC_CLEAR_BIT(REG, BIT)                           \
  do {                                                       \
    uint32_t val;                                            \
    do {                                                     \
      val = __LDREXW((__IO uint32_t *)&(REG)) & ~(BIT);      \
    } while ((__STREXW(val,(__IO uint32_t *)&(REG))) != 0U); \
  } while(0)

/* Atomic 32-bit register access macro to clear and set one or several bits */
#define ATOMIC_MODIFY_REG(REG, CLEARMSK, SETMASK)                          \
  do {                                                                     \
    uint32_t val;                                                          \
    do {                                                                   \
      val = (__LDREXW((__IO uint32_t *)&(REG)) & ~(CLEARMSK)) | (SETMASK); \
    } while ((__STREXW(val,(__IO uint32_t *)&(REG))) != 0U);               \
  } while(0)

// enable / disable the output of printf
#if defined(UART_SEL) || defined(MCUCMD)
#else
#define printf(args...)
#endif


/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* __RX32G4XX_COMMON_H__ */
