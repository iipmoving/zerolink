#ifndef __API_GPIO_H__
#define __API_GPIO_H__


#include	"stdint.h"





enum{				//对外的IO口索引（需要软件操作的）外部访问时作为封装
	DebugA_pin=0,
	DebugB_pin,			//1
	DebugC_pin,			//2
	DebugD_pin,			//3

	Zero_pin,			//4		
	VOLTAGE_pin,		//5
	
	PWM1L_pin,			//6
	PWM1H_pin,			//7
	PWM2L_pin,			//8
	PWM2H_pin,			//9	
	PWM3L_pin,			//10
	PWM3H_pin,			//11
	PWM4L_pin,			//12
	PWM4H_pin,			//13	
	
	BK12_pin,			//14
	BK34_pin,			//15
	
	T1A_pin,			//16
	T2A_pin,			//17
	T3A_pin,			//18
	T4A_pin,			//19

	// PAN_pin,			//20	//检锅口
	PANSW1_pin,			//21	检锅切换口	
	PANSW2_pin,			//22
	PANSW3_pin,			//23
	PANSW4_pin,			//24


	BOTTOM1_pin,		//25
	BOTTOM2_pin,		//26	
	BOTTOM3_pin,		//27		
	BOTTOM4_pin,		//28
	
	IGBT1_pin,			//29
	IGBT2_pin,			//30	
	IGBT3_pin,			//29
	IGBT4_pin,			//30	
		
	
	FAN_pin,			//31
	FAN_AD_pin,			//32
	
	SCL_pin,			//33
	SDA_pin,			//34	
		
				
	
	CUR1_pin,			//35
	CUR2_pin,			//36
	CUR3_pin,			//37
	CUR4_pin,			//38
	
	// OP1N_pin,			//39
	// OP1O_pin,			//40
	// OP2N_pin,			//41	
	// OP2O_pin,			//42	
	// OP3N_pin,			//43
	// OP3O_pin,			//44	
	// OP4N_pin,			//45
	// OP4O_pin,			//46
	
	HRTIM_SYN_pin,		//47		HRTIM同步	
	TX_pin,
	RX_pin,
	HRTIM_TEST1_pin,
	SCR_pin,			//繼電器開合
	Ceil1_pin,
	Ceil2_pin,
	PIN_MAX,
};
	
typedef enum
{
			MODER_Input   =0,                              
			MODER_Output,  
			MODER_Af, 
			MODER_Analog,
	
}API_GPIO_MODE_ENUM;	
typedef enum
{
			PUPDR_Floating    =0,                              
			PUPDR_Pullup,  
			PUPDR_Pulldown, 

	
}API_GPIO_PUPDR_ENUM;	

void		API_GPIO_BaseInit(uint8_t pin);
void			API_GPIO_PORT_INIT(void);
void		API_GPIO_WritePin(uint8_t pin,	uint8_t  value);
uint8_t		API_GPIO_ReadPin(uint8_t pin);
void		API_GPIO_PinPull(uint8_t pin,API_GPIO_PUPDR_ENUM pull);

void		API_GPIO_Mode(uint8_t pin,API_GPIO_MODE_ENUM mode);

#endif
