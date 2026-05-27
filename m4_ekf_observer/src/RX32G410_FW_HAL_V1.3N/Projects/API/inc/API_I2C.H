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
* File         : drv_tim.h
* By           : moving
*********************************************************************************************************
*/

#ifndef __DRV_I2C_H__
#define __DRV_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
//#include <stdio.h>
//#include <stdint.h>
//#include <stdbool.h>
//#include "rx32g4xx.h"


//#include "rx32g4xx_hal_gpio.h"
#include	"commClass.h"


#define	I2C_BuffMax	20

typedef struct
{
	uint8_t 	max;
	uint8_t 	len;
	uint8_t 	num;
	uint8_t 	res;
	
	uint8_t 	buff[I2C_BuffMax];
}I2C_BUFF_DEF;

/* Exported typedef ----------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/

/* User can use this section to tailor I2Cx/I2Cx instance used and associated
   resources */
/* Definition for I2Cx clock resources */
#define I2C1_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2C2_CLK_ENABLE()               __HAL_RCC_I2C2_CLK_ENABLE()
#define I2Cx_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOB_CLK_ENABLE()

#define I2C1_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define I2C1_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()
#define I2C2_FORCE_RESET()              __HAL_RCC_I2C2_FORCE_RESET()
#define I2C2_RELEASE_RESET()            __HAL_RCC_I2C2_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define I2C1_SCL_PIN                    GPIO_PIN_8
#define I2C1_SDA_PIN                    GPIO_PIN_9
#define I2C1_GPIO_AF                    GPIO_AF4
#define I2C1_GPIO_PORT                  GPIOB

#define I2C2_SCL_PIN                    GPIO_PIN_9
#define I2C2_SDA_PIN                    GPIO_PIN_8
#define I2C2_GPIO_AF                    GPIO_AF5
#define I2C2_GPIO_PORT                  GPIOB

/* Size of Transmission buffer */
#define TXBUFFERSIZE                    (COUNTOF(aTxBuffer) - 1)
/* Size of Reception buffer */
#define RXBUFFERSIZE                    TXBUFFERSIZE

/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)             (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
    
/* Exported variables --------------------------------------------------------*/
/* Exported function prototypes ----------------------------------------------*/

void API_I2C_Init (void);
void  API_I2C_CheckBuffMax(void);

UserStringDef*	getI2cRxBuffAdr(void);
UserStringDef*	getI2cTxBuffAdr(void);


#ifdef __cplusplus
}
#endif

#endif /* __DRV_I2C_H__ */
