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
* File         : DRV_GPIO.h
* By           : MOVING
*********************************************************************************************************
*/

#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include	"rx32g4xx_hal_gpio.h"
	
/* Exported typedef ----------------------------------------------------------*/
typedef struct
{
	GPIO_TypeDef*	base;							//IO口  ：GPIO_							
	GPIO_InitTypeDef	initStr;			//初始化参数


	
}GPIO_BaseInitTypeDef;


typedef struct
{	
	GPIO_BaseInitTypeDef	gpio;
	
//	void(* Init)	 (GPIO_BaseInitTypeDef* gpio);
	GPIO_PinState    (*ReadPin)(GPIO_BaseInitTypeDef *gpio);
	void             (* WritePin)(GPIO_BaseInitTypeDef *gpio, GPIO_PinState PinState);	
	
}GPIO_ClaseDef;

typedef struct                              //IO口设置集合
{
        uint8_t    num;                     //对象大小
        uint8_t    max;                     //完整空间大小（用于声明）
        uint8_t    res1;                    //保留
        uint8_t    res2;                    //保留

		GPIO_BaseInitTypeDef*	gpio1;      //元素指针
        GPIO_BaseInitTypeDef*	gpio2;      //元素指针
		GPIO_BaseInitTypeDef*	gpio3;      //元素指针
        GPIO_BaseInitTypeDef*	gpio4;      //元素指针
	
}GPIOsetGroupDef;



/* Exported define -----------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

/* 连带GPIO基址直接运算 *****************************************************/
void 			  DRV_GPIO_BaseInit(GPIO_BaseInitTypeDef* gpio);
GPIO_PinState     DRV_GPIO_BaseReadPin(GPIO_BaseInitTypeDef *gpio);
void              DRV_GPIO_BaseWritePin(GPIO_BaseInitTypeDef *gpio, GPIO_PinState PinState);
void              DRV_GPIO_BaseTogglePin(GPIO_BaseInitTypeDef *gpio);
void 				DRV_GPIO_PinPull(GPIO_BaseInitTypeDef *gpio, uint32_t Pull);




void DRV_GPIO_Mode(GPIO_BaseInitTypeDef *gpio, uint32_t inOrOut);

#ifdef __cplusplus
}
#endif

#endif /* __DRV_GPIO_H__ */
