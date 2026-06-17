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
*   File    : system_init.c
*   By      : RX_DV_Team
**************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "rx32g4xx_hal.h"
#include "rx32g4xx_config_def.h"
#include "system_init.h"

__weak	void	SysTemInit_Uart_fputc_Callback(uint8_t ch, uint8_t* string,uint8_t size);		

const sysclk_param UsePLLConfig[] =
{
    { .flash_latency=FLASH_LATENCY_4, .sysclk_src=RCC_SYSCLKSOURCE_PLLCLK, .pll_src=RCC_PLLSOURCE_HSI , .pll_mul=24 , .pllr_div=RCC_PLLR_DIV2 },  // SYSCLK_192M
    { .flash_latency=FLASH_LATENCY_4, .sysclk_src=RCC_SYSCLKSOURCE_PLLCLK, .pll_src=RCC_PLLSOURCE_HSI , .pll_mul=21 , .pllr_div=RCC_PLLR_DIV2 },  // SYSCLK_168M
    { .flash_latency=FLASH_LATENCY_3, .sysclk_src=RCC_SYSCLKSOURCE_PLLCLK, .pll_src=RCC_PLLSOURCE_HSI , .pll_mul=18 , .pllr_div=RCC_PLLR_DIV2 },  // SYSCLK_144M
    { .flash_latency=FLASH_LATENCY_2, .sysclk_src=RCC_SYSCLKSOURCE_PLLCLK, .pll_src=RCC_PLLSOURCE_HSI , .pll_mul=12 , .pllr_div=RCC_PLLR_DIV2 },  // SYSCLK_96M
    { .flash_latency=FLASH_LATENCY_2, .sysclk_src=RCC_SYSCLKSOURCE_PLLCLK, .pll_src=RCC_PLLSOURCE_HSI , .pll_mul=20 , .pllr_div=RCC_PLLR_DIV4 },  // SYSCLK_80M
    { .flash_latency=FLASH_LATENCY_1, .sysclk_src=RCC_SYSCLKSOURCE_PLLCLK, .pll_src=RCC_PLLSOURCE_HSI , .pll_mul=18 , .pllr_div=RCC_PLLR_DIV4 },  // SYSCLK_72M
    { .flash_latency=FLASH_LATENCY_1, .sysclk_src=RCC_SYSCLKSOURCE_PLLCLK, .pll_src=RCC_PLLSOURCE_HSI , .pll_mul=16 , .pllr_div=RCC_PLLR_DIV4 },  // SYSCLK_64M
    { .flash_latency=FLASH_LATENCY_1, .sysclk_src=RCC_SYSCLKSOURCE_PLLCLK, .pll_src=RCC_PLLSOURCE_HSI , .pll_mul=14 , .pllr_div=RCC_PLLR_DIV4 },  // SYSCLK_56M
    { .flash_latency=FLASH_LATENCY_1, .sysclk_src=RCC_SYSCLKSOURCE_PLLCLK, .pll_src=RCC_PLLSOURCE_HSI , .pll_mul=10 , .pllr_div=RCC_PLLR_DIV4 },  // SYSCLK_40M
    { .flash_latency=FLASH_LATENCY_1, .sysclk_src=RCC_SYSCLKSOURCE_PLLCLK, .pll_src=RCC_PLLSOURCE_HSI , .pll_mul=8  , .pllr_div=RCC_PLLR_DIV4 },  // SYSCLK_32M
    { .flash_latency=FLASH_LATENCY_1, .sysclk_src=RCC_SYSCLKSOURCE_PLLCLK, .pll_src=RCC_PLLSOURCE_HSI , .pll_mul=8  , .pllr_div=RCC_PLLR_DIV8 },  // SYSCLK_16M
};

/**
  * @brief  Set SYSCLK.
  * @param  SYSCLK
  *         @arg @ref  SYSCLK_USE_192M
  *         @arg @ref  SYSCLK_USE_168M
  *         @arg @ref  SYSCLK_USE_144M
  *         @arg @ref  SYSCLK_USE_96M
  *         @arg @ref  SYSCLK_USE_80M
  *         @arg @ref  SYSCLK_USE_72M
  *         @arg @ref  SYSCLK_USE_64M
  *         @arg @ref  SYSCLK_USE_56M
  *         @arg @ref  SYSCLK_USE_40M
  *         @arg @ref  SYSCLK_USE_32M
  *         @arg @ref  SYSCLK_USE_16M
  */
void HAL_SystemClocks_Config(uint32_t sysclk_freq)
{
    const sysclk_param *clk_config;
    uint32_t u32_FlashLatency, u32_SysclkSource, u32_PLLSource, u32_PLLMUL, u32_PLLRDIV;
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Get SYSCLK Config parameter */
    clk_config = &UsePLLConfig[sysclk_freq];

    u32_FlashLatency = clk_config->flash_latency;
    u32_SysclkSource = clk_config->sysclk_src;
    u32_PLLSource    = clk_config->pll_src;
    u32_PLLMUL       = clk_config->pll_mul;
    u32_PLLRDIV      = clk_config->pllr_div;

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = u32_PLLSource;
    RCC_OscInitStruct.PLL.PLLN            = u32_PLLMUL;
    RCC_OscInitStruct.PLL.PLLP            = ADC_CLOCK_DIV;    // ADC Clock divider ( This paramter can be modified in rx32g4xx_config_def.h)
    RCC_OscInitStruct.PLL.PLLQ            = USB_CLOCK_DIV;    // USB_Clock ( The default setting will be DIV_6 if this parameter is left unchanged )
    RCC_OscInitStruct.PLL.PLLR            = u32_PLLRDIV;      // System Clock divider
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        while(1);
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = u32_SysclkSource;
    RCC_ClkInitStruct.AHBCLKDivider  = HCLK_DIV;               // AHB  Clock divider ( This paramter can be modified in rx32g4xx_config_def.h )
    RCC_ClkInitStruct.APB1CLKDivider = PCLK1_DIV;              // APB1 Clock divider ( This paramter can be modified in rx32g4xx_config_def.h )
    RCC_ClkInitStruct.APB2CLKDivider = PCLK2_DIV;              // APB2 Clock divider ( This paramter can be modified in rx32g4xx_config_def.h )

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, u32_FlashLatency) != HAL_OK)
    {
        while(1);
    }
}

void HAL_UART_Config(void)
{
    /*!< ============= GPIO Pin Select ============= */
    /*     UART_SEL   GPIO_TX_pin   GPIO_RX_pin     *
     *     UART1      PA9           PA10            *
     *     UART2      PB3           PB4      JREST  *
     *     UART3      PB9           PB8             *
     *     UART4       PC10          PC11            *
     *     UART5       PC12          PD2             */
    /*< ============================================ */

#ifdef UART_SEL   // UART_SEL can be modified in rx32g4xx_config_def.h

    GPIO_InitTypeDef      GPIO_Init_Struct = {0};
    UART_InitTypeDef     UART_Init_Struct = {0};
    UART_HandleTypeDef   huart;

    GPIO_Init_Struct.Mode = GPIO_MODE_AF_PP;
    GPIO_Init_Struct.Speed = GPIO_SPEED_FREQ_LOW;

    if(UART_SEL == UART1)
    {
        __HAL_RCC_UART1_CLK_ENABLE();
//        __HAL_RCC_GPIOA_CLK_ENABLE();

//        GPIO_Init_Struct.Alternate = GPIO_AF7_UART1;
//        GPIO_Init_Struct.Pin = GPIO_PIN_9;
//        GPIO_Init_Struct.Pull = GPIO_PULLUP;
//        HAL_GPIO_Init(GPIOA, &GPIO_Init_Struct);

//        GPIO_Init_Struct.Pin = GPIO_PIN_10;
//        GPIO_Init_Struct.Pull = GPIO_NOPULL;
//        HAL_GPIO_Init(GPIOA, &GPIO_Init_Struct);

	       __HAL_RCC_GPIOB_CLK_ENABLE();		
        GPIO_Init_Struct.Alternate = GPIO_AF7_UART1;
        GPIO_Init_Struct.Pin = GPIO_PIN_6;
        GPIO_Init_Struct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);

        GPIO_Init_Struct.Pin = GPIO_PIN_7;
        GPIO_Init_Struct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);			
						
			
			
    }
    else if(UART_SEL == UART2)
    {
        __HAL_RCC_UART2_CLK_ENABLE();
//        __HAL_RCC_GPIOB_CLK_ENABLE();

//        GPIO_Init_Struct.Alternate = GPIO_AF7_UART2;
//        GPIO_Init_Struct.Pin = GPIO_PIN_3;
//        GPIO_Init_Struct.Pull = GPIO_PULLUP;
//        HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);

//        GPIO_Init_Struct.Pin = GPIO_PIN_4;
//        GPIO_Init_Struct.Pull = GPIO_NOPULL;
//        HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);
	        __HAL_RCC_GPIOA_CLK_ENABLE();		
        GPIO_Init_Struct.Alternate = GPIO_AF7_UART2;
        GPIO_Init_Struct.Pin = GPIO_PIN_13;
        GPIO_Init_Struct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOA, &GPIO_Init_Struct);

        GPIO_Init_Struct.Pin = GPIO_PIN_14;
        GPIO_Init_Struct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_Init_Struct);			
			
			
    }
    else if(UART_SEL == UART3)
    {
        __HAL_RCC_UART3_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        GPIO_Init_Struct.Alternate = GPIO_AF7_UART3;
        GPIO_Init_Struct.Pin = GPIO_PIN_9;
        GPIO_Init_Struct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);

        GPIO_Init_Struct.Pin = GPIO_PIN_8;
        GPIO_Init_Struct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);
    }
    else if(UART_SEL == UART4)
    {
        __HAL_RCC_UART4_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();

//        GPIO_Init_Struct.Alternate = GPIO_AF5_UART4;
//        GPIO_Init_Struct.Pin = GPIO_PIN_10;
//        GPIO_Init_Struct.Pull = GPIO_PULLUP;
//        HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);

//        GPIO_Init_Struct.Pin = GPIO_PIN_11;
//        GPIO_Init_Struct.Pull = GPIO_NOPULL;
//        HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);
    }
    else if(UART_SEL == UART5)
    {
        __HAL_RCC_UART5_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();

//        GPIO_Init_Struct.Alternate = GPIO_AF5_UART5;
//        GPIO_Init_Struct.Pin = GPIO_PIN_12;
//        GPIO_Init_Struct.Pull = GPIO_PULLUP;
//        HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);

//        GPIO_Init_Struct.Pin = GPIO_PIN_2;
//        GPIO_Init_Struct.Pull = GPIO_NOPULL;
//        HAL_GPIO_Init(GPIOD, &GPIO_Init_Struct);
    }
    else
    {
        while(1);  // Wrong definition for UART
    }

    UART_Init_Struct.BaudRate   = UART_BAUDRATE;
    UART_Init_Struct.Mode       = UART_MODE_TX_RX;
    UART_Init_Struct.HwFlowCtl  = UART_HWCONTROL_NONE;
    UART_Init_Struct.Parity     = UART_PARITY_NONE;
    UART_Init_Struct.StopBits   = UART_STOPBITS_1;
    UART_Init_Struct.WordLength = UART_WORDLENGTH_8B;

    huart.Instance = UART_SEL;
    huart.Init     = UART_Init_Struct;

    HAL_UART_Init(&huart);
    HAL_HalfDuplex_EnableTransmitter(&huart);

#endif
}

#if defined(UART_SEL)

int fputc(int ch, FILE *fp)
{
	
#ifdef PrintMessage	
    if(IS_UART_INSTANCE(UART_SEL))
    {

#if 1
				UART_SEL->DR = ch & 0x1FFU;
        while((UART_SEL->SR & UART_SR_TXE) == 0);
#else

			
			SysTemInit_Uart_fputc_Callback(ch, *fp,8);
			
#endif			
			}
#endif
    return ch;
		
		
}
#endif
