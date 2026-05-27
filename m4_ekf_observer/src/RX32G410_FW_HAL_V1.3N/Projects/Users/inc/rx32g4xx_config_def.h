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
* File         : rx32g4xx_config_def.h
* By           : RX_DV_Team
*********************************************************************************************************
*/

#ifndef __RX32G4XX_CONFIG_DEF_H__
#define __RX32G4XX_CONFIG_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/

// Set HSE value (the externl oscillator frequency)
// Note: This values is depended on the board level design
#define HSE_USER_DEF_VALUE   8000000U

// Set SYSCLK value
#define FW_SYSCLK            SYSCLK_USE_192M

// Set ADC Clock(PLLP) value
// ADC Clock(PLLP) = PLL * PLLN / PLLPDIV
// USB Clock(PLLQ) = PLL * PLLN / PLLQDIV
// About PLL * PLLN
//      SYSCLK 192M : 384 = 16 * 24
//      SYSCLK 144M : 288 = 16 * 18
//      SYSCLK  96M : 192 = 16 * 12
//      SYSCLK  80M : 320 = 16 * 20
//      SYSCLK  72M : 288 = 16 * 18
//      SYSCLK  64M : 256 = 16 * 16
//      SYSCLK  56M : 224 = 16 * 14
//      SYSCLK  40M : 160 = 16 * 10
//      SYSCLK  32M : 128 = 16 * 8
//      SYSCLK  16M : 128 = 16 * 8
#define ADC_CLOCK_DIV        RCC_PLLP_DIV6     // 384MHz / 6 = 64MHz (Max PLLP Clock freq)
#define USB_CLOCK_DIV        RCC_PLLQ_DIV8     // 384MHz / 8 = 48MHz (Max PLLQ Clock freq)

// Set all Periphrel prescaler
#define HCLK_DIV             RCC_SYSCLK_DIV1
#define PCLK1_DIV            RCC_HCLK_DIV1
#define PCLK2_DIV            RCC_HCLK_DIV2

#define HSI_VALUE            16000000U         // Value of the Internel oscillator in Hz
#define LSI_VALUE            32000U            // Value of the Internel oscillator in Hz
#define HSE_VALUE            16000000U         // Value of the Externel oscillator in Hz

// Select the output uart for printf (USE : UART1/ UART2/ UART3/ UART4/ UART5)
#ifdef  COMM_UART
    #define UART_SEL            UART3       //调试接口
#else
    #define UART_SEL            UART2
#endif


// Set the UART baudrate
#define UART_BAUDRATE       115200

// System tick interrupt (0: Disable system tick interrupt, 1: Enable system tick interrupt)
#define SYSTICK_INTERRUPT_ENABLE  1

// System BSP the LED configuration
#define SYS_BSP_LED_EN                      1                   // 0: Disable  1: Enable
#define SYS_BSP_LED_GPIO_PORT               GPIOA
#define SYS_BSP_LED_PIN                     GPIO_PIN_5

// System BSP the button configuration
#define SYS_BSP_BUTTON_EN                   0                   // 0: Disable  1: Enable
#define SYS_BSP_BUTTON_GPIO_PORT            GPIOC
#define SYS_BSP_BUTTON_PIN                  GPIO_PIN_1
#define SYS_BSP_BUTTON_EXTI_IRQn            EXTI1_IRQn
#define SYS_BSP_BUTTON_EXTI_IRQ_PRIORITY    IRQ_PRIORITY_EXTI1
#define SYS_BSP_BUTTON_EXTI_IRQ_HANDLER     EXTI1_IRQHandler

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* __RX32G4XX_CONFIG_DEF_H__ */
