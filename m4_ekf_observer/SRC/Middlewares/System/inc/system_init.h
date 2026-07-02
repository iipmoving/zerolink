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
*   File    : rx32g4xx_hal_Template.h
*   By      : RX_DV_Team
**************************************************************************************************
*/


#ifndef __rx32g4xx_SYSINIT_H__
#define __rx32g4xx_SYSINIT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"

typedef struct
{
    uint32_t  flash_latency;
    uint32_t  sysclk_src;
    uint32_t  pll_src;
    uint32_t  pll_mul;
    uint32_t  pllr_div;
} sysclk_param;

#define SYSCLK_USE_192M               0
#define SYSCLK_USE_168M               1
#define SYSCLK_USE_144M               2
#define SYSCLK_USE_96M                3
#define SYSCLK_USE_80M                4
#define SYSCLK_USE_72M                5
#define SYSCLK_USE_64M                6
#define SYSCLK_USE_56M                7
#define SYSCLK_USE_40M                8
#define SYSCLK_USE_32M                9
#define SYSCLK_USE_16M                10


void HAL_SystemClocks_Config(uint32_t sysclk_freq);
void HAL_UART_Config(void);
void SystemTick_Config(void);


#ifdef __cplusplus
}
#endif

#endif /* __rx32g4xx_SYSINIT_H__ */
