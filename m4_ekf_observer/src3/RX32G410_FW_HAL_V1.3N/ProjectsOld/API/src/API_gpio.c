
#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include "system_init.h"
#include "system_bsp.h"

#include	"stdint.h"


#include	"DRV_GPIO.H"
#include	"API_gpio.h"


#if 0

typedef struct		//与IO口索引对应的IO结构，
{
	GPIO_BaseInitTypeDef* debugA;			//0	//调试口
	GPIO_BaseInitTypeDef* debugB;			//1
	GPIO_BaseInitTypeDef* debugC;			//2
	GPIO_BaseInitTypeDef* debugD;			//3	
	
	GPIO_BaseInitTypeDef* zero;				//4	//过零口
	GPIO_BaseInitTypeDef* voltage;			//5	电压AD口
	
	GPIO_BaseInitTypeDef* PWM1L;			//6	ppg互补输出口	
	GPIO_BaseInitTypeDef* PWM1H;			//7
	GPIO_BaseInitTypeDef* PWM2L;			//8
	GPIO_BaseInitTypeDef* PWM2H;			//9
	GPIO_BaseInitTypeDef* PWM3L;			//10
	GPIO_BaseInitTypeDef* PWM3H;			//11
	GPIO_BaseInitTypeDef* PWM4L;			//12
	GPIO_BaseInitTypeDef* PWM4H;			//13

	GPIO_BaseInitTypeDef* BK12;				//14	IGBT C极过流保护
	GPIO_BaseInitTypeDef* BK34;				//15

	
	GPIO_BaseInitTypeDef* T1A;				//16	谐振电流 CMP输入口
	GPIO_BaseInitTypeDef* T2A;				//17
	GPIO_BaseInitTypeDef* T3A;				//18
	GPIO_BaseInitTypeDef* T4A;				//19


	


	GPIO_BaseInitTypeDef* PAN;				//20	检锅口
	GPIO_BaseInitTypeDef* PANSW1;			//21	检锅切换口	
	GPIO_BaseInitTypeDef* PANSW2;			//22
	GPIO_BaseInitTypeDef* PANSW3;			//23
	GPIO_BaseInitTypeDef* PANSW4;			//24

	GPIO_BaseInitTypeDef* BOTTOM1;			//25	炉面热敏电阻
	GPIO_BaseInitTypeDef* BOTTOM2;			//26	
	GPIO_BaseInitTypeDef* BOTTOM3;			//27
	GPIO_BaseInitTypeDef* BOTTOM4;			//28

	GPIO_BaseInitTypeDef* IGBT1;			//29	IGBT散热器温度
	GPIO_BaseInitTypeDef* IGBT2;			//30

	GPIO_BaseInitTypeDef* FAN;				//31	风机输出
	GPIO_BaseInitTypeDef* FAN_AD;			//32	风机电流检测
	
	GPIO_BaseInitTypeDef* SCL;				//33	I2C1 SCL
	GPIO_BaseInitTypeDef* SDA;				//34	I2C1 SDA

	GPIO_BaseInitTypeDef* CUR1;				//35	谐振电流 CMP输入口 OP正极
	GPIO_BaseInitTypeDef* CRU2;				//36
	GPIO_BaseInitTypeDef* CUR3;				//37
	GPIO_BaseInitTypeDef* CUR4;				//38


	GPIO_BaseInitTypeDef* OPA1N;			//39	OP负极
	GPIO_BaseInitTypeDef* OPA1O;			//40	OP输出	

	GPIO_BaseInitTypeDef* OPA2N;			//41	OP负极
	GPIO_BaseInitTypeDef* OPA2O;			//42	OP输出

	GPIO_BaseInitTypeDef* OPA3N;			//43	OP负极
	GPIO_BaseInitTypeDef* OPA3O;			//44	OP输出
	
	GPIO_BaseInitTypeDef* OPA4N;			//45	OP负极
	GPIO_BaseInitTypeDef* OPA4O;			//46	OP输出	

	GPIO_BaseInitTypeDef*	HRTIM_SYN,		//47		HRTIM同步	

	GPIO_BaseInitTypeDef* TX;			//48	UART TX
	GPIO_BaseInitTypeDef* RX;			//49	UART RX	

}GPIO_GROUP_DEF;	

#endif



GPIO_BaseInitTypeDef const DEBUG_A_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_3,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_OUTPUT_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PULLUP,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};	
#define		DEBUG_A		DEBUG_A_GPIO_InitStruct

GPIO_BaseInitTypeDef const DEBUG_B_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_5,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_OUTPUT_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PULLUP,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};	
#define		DEBUG_B		DEBUG_B_GPIO_InitStruct

GPIO_BaseInitTypeDef const DEBUG_C_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_4,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_OUTPUT_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PULLUP,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};	
#define		DEBUG_C		DEBUG_C_GPIO_InitStruct

GPIO_BaseInitTypeDef const	DEBUG_D_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_4,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_OUTPUT_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PULLUP,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};	
#define		DEBUG_D		DEBUG_D_GPIO_InitStruct

GPIO_BaseInitTypeDef const DEBUG_ZERO_GPIO_InitStruct={
        
        GPIOD,
        
        {
            GPIO_PIN_2,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PULLUP,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF6_TIM8,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		ZERO		DEBUG_ZERO_GPIO_InitStruct


GPIO_BaseInitTypeDef const VOLTAGE_AN1_9_GPIO_InitStruct={
        
        GPIOC,
        
        {
            GPIO_PIN_3,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		VOLTAGE		VOLTAGE_AN1_9_GPIO_InitStruct



//---------DEFINE PAN START-----------------------------------




GPIO_BaseInitTypeDef const PANSW1_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_15,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_OUTPUT_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		PANSW1		PANSW1_GPIO_InitStruct

GPIO_BaseInitTypeDef const PANSW2_GPIO_InitStruct={
        
        GPIOF,
        
        {
            GPIO_PIN_1,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_OUTPUT_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		PANSW2		PANSW2_GPIO_InitStruct

GPIO_BaseInitTypeDef const PANSW3_GPIO_InitStruct={
        
        GPIOF,
        
        {
            GPIO_PIN_0,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_OUTPUT_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		PANSW3		PANSW3_GPIO_InitStruct

GPIO_BaseInitTypeDef const PANSW4_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_11,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_OUTPUT_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		PANSW4		PANSW4_GPIO_InitStruct


//---------DEFINE PAN END-----------------------------------


//---------DEFINE CUR START-----------------------------------

GPIO_BaseInitTypeDef const T1A_CMP1_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_0,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		T1A		T1A_CMP1_GPIO_InitStruct


GPIO_BaseInitTypeDef const T2A_CMP2_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_1,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		T2A		T2A_CMP2_GPIO_InitStruct


GPIO_BaseInitTypeDef const T3A_CMP3_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_13,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		T3A		T3A_CMP3_GPIO_InitStruct

GPIO_BaseInitTypeDef const T4A_CMP4_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_14,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		T4A		T4A_CMP4_GPIO_InitStruct

//---------DEFINE CUR END-----------------------------------

//---------DEFINE PPG_OUT START-----------------------------------

GPIO_BaseInitTypeDef	const 	HRTIM_CHB1_GPIO_InitStruct={

        GPIOC,
        
        {
            GPIO_PIN_6,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF13,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};
#define		PWM1H		HRTIM_CHB1_GPIO_InitStruct

GPIO_BaseInitTypeDef	const 	HRTIM_CHB2_GPIO_InitStruct={

        GPIOC,
        
        {
            GPIO_PIN_7,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF13,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};
#define		PWM1L		HRTIM_CHB2_GPIO_InitStruct

GPIO_BaseInitTypeDef	const 	HRTIM_CHE1_GPIO_InitStruct={

        GPIOC,
        
        {
            GPIO_PIN_8,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF13,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};
#define		PWM2H		HRTIM_CHE1_GPIO_InitStruct

GPIO_BaseInitTypeDef	const 	HRTIM_CHE2_GPIO_InitStruct={

        GPIOC,
        
        {
            GPIO_PIN_9,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF13,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};
#define		PWM2L		HRTIM_CHE2_GPIO_InitStruct

GPIO_BaseInitTypeDef	const 	HRTIM_CHA1_GPIO_InitStruct={

        GPIOA,
        
        {
            GPIO_PIN_8,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF13,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};
#define		PWM3H		HRTIM_CHA1_GPIO_InitStruct

GPIO_BaseInitTypeDef	const 	HRTIM_CHA2_GPIO_InitStruct={

        GPIOA,
        
        {
            GPIO_PIN_9,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF13,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};
#define		PWM3L		HRTIM_CHA2_GPIO_InitStruct

GPIO_BaseInitTypeDef	const 	HRTIM_CHD1_GPIO_InitStruct={

        GPIOC,
        
        {
            GPIO_PIN_11,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF12,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};
#define		PWM4H		HRTIM_CHD1_GPIO_InitStruct

GPIO_BaseInitTypeDef	const 	HRTIM_CHD2_GPIO_InitStruct={

        GPIOC,
        
        {
            GPIO_PIN_12,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF12,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};
#define		PWM4L		HRTIM_CHD2_GPIO_InitStruct





GPIO_BaseInitTypeDef	const 	HRTIM_CHC1_GPIO_InitStruct={

        GPIOA,
        
        {
            GPIO_PIN_15,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF12,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};
#define		HRTIM_TEST1		HRTIM_CHC1_GPIO_InitStruct		// 输出公共周期






GPIO_BaseInitTypeDef	const 	HRTIM_FLT3_GPIO_InitStruct={

        GPIOE,
        
        {
            GPIO_PIN_6,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF13,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};
#define		BK12		HRTIM_FLT3_GPIO_InitStruct

GPIO_BaseInitTypeDef	const 	HRTIM_FLT6_GPIO_InitStruct={

        GPIOC,
        
        {
            GPIO_PIN_10,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF13,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};
#define		BK34		HRTIM_FLT6_GPIO_InitStruct


//---------DEFINE PPG_OUT END-----------------------------------

//---------DEFINE IGBT START-----------------------------------

GPIO_BaseInitTypeDef const IGBT1_AN2_1_GPIO_InitStruct={
        
        GPIOC,
        
        {
            GPIO_PIN_4,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		IGBT1	IGBT1_AN2_1_GPIO_InitStruct

GPIO_BaseInitTypeDef const IGBT2_AN2_2_GPIO_InitStruct={
        
        GPIOC,
        
        {
            GPIO_PIN_5,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		IGBT2	IGBT2_AN2_2_GPIO_InitStruct


GPIO_BaseInitTypeDef const IGBT3_AN2_8_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_0,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		IGBT3	IGBT3_AN2_8_GPIO_InitStruct


GPIO_BaseInitTypeDef const IGBT4_AN2_7_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_7,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		IGBT4	 IGBT4_AN2_7_GPIO_InitStruct


GPIO_BaseInitTypeDef const Ceil1_AN1_2_GPIO_InitStruct={
        
        GPIOC,
        
        {
            GPIO_PIN_15,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		Ceil1	Ceil1_AN1_2_GPIO_InitStruct



GPIO_BaseInitTypeDef const Ceil2_AN1_10_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_4,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		Ceil2	Ceil2_AN1_10_GPIO_InitStruct





GPIO_BaseInitTypeDef const PAN_AN2_2_GPIO_InitStruct={
        
        GPIOC,
        
        {
            GPIO_PIN_5,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		PAN	PAN_AN2_2_GPIO_InitStruct




//---------DEFINE IGBT END-----------------------------------
//---------DEFINE BOTTOM START-----------------------------------

GPIO_BaseInitTypeDef const BOTTOM1_AN1_13_GPIO_InitStruct={
        
        GPIOC,
        
        {
            GPIO_PIN_13,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		BOTTOM1	BOTTOM1_AN1_13_GPIO_InitStruct

GPIO_BaseInitTypeDef const BOTTOM2_AN1_1_GPIO_InitStruct={
        
        GPIOC,
        
        {
            GPIO_PIN_14,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		BOTTOM2	BOTTOM2_AN1_1_GPIO_InitStruct

GPIO_BaseInitTypeDef const BOTTOM3_AN1_7_GPIO_InitStruct={
        
        GPIOC,
        
        {
            GPIO_PIN_1,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		BOTTOM3	BOTTOM3_AN1_7_GPIO_InitStruct

GPIO_BaseInitTypeDef const BOTTOM4_AN1_8_GPIO_InitStruct={
        
        GPIOC,
        
        {
            GPIO_PIN_2,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		BOTTOM4	BOTTOM4_AN1_8_GPIO_InitStruct


//---------DEFINE BOTTOM END-----------------------------------


//---------DEFINE FAN START-----------------------------------

GPIO_BaseInitTypeDef const FAN_TIM2_4_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_10,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF10_TIM2,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		FAN	FAN_TIM2_4_GPIO_InitStruct



GPIO_BaseInitTypeDef const FAN_OP1P2_GPIO_InitStruct={
        
        GPIOC,
        
        {
            GPIO_PIN_0,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		FAN_AD	FAN_OP1P2_GPIO_InitStruct

//---------DEFINE FAN END-----------------------------------



//---------DEFINE I2C START-----------------------------------
//GPIO_AF5_I2C2	


#ifdef	I2C1_PORT


GPIO_BaseInitTypeDef	const 	I2C1_SCL_GPIO_InitStruct={

        GPIOB,
        
        {
            GPIO_PIN_8,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_OD,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pullup,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF4_I2C1,							/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};

GPIO_BaseInitTypeDef	const 	I2C1_SDA_GPIO_InitStruct={

        GPIOB,
        
        {
            GPIO_PIN_9,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_OD,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pullup,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF4_I2C1,							/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};

#define		I2Cx_SCL	I2C1_SCL_GPIO_InitStruct
#define		I2Cx_SDA	I2C1_SDA_GPIO_InitStruct

#else

GPIO_BaseInitTypeDef	const 	I2C2_SDA_GPIO_InitStruct={

        GPIOB,
        
        {
            GPIO_PIN_8,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_OD,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pullup,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF5_I2C2,							/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};

GPIO_BaseInitTypeDef	const 	I2C2_SCL_GPIO_InitStruct={

        GPIOB,
        
        {
            GPIO_PIN_9,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_OD,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pullup,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF5_I2C2,							/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};

#define		I2Cx_SCL	I2C2_SCL_GPIO_InitStruct
#define		I2Cx_SDA	I2C2_SDA_GPIO_InitStruct

#endif
GPIO_BaseInitTypeDef	const 	UART3_RX_SCL_GPIO_InitStruct={

        GPIOB,
        
        {
            GPIO_PIN_8,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_OD,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pullup,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF7_UART3,							/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};







GPIO_BaseInitTypeDef	const 	UART3_TX_SDA_GPIO_InitStruct={

        GPIOB,
        
        {
            GPIO_PIN_9,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_OD,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pullup,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF7_UART3,							/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }

};




//---------DEFINE I2C END-----------------------------------

//---------DEFINE CUR START-----------------------------------
GPIO_BaseInitTypeDef const CUR1_OP1P1_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_2,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		CUR1	CUR1_OP1P1_GPIO_InitStruct

GPIO_BaseInitTypeDef const CUR2_OP2P1_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_5,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		CUR2	CUR2_OP2P1_GPIO_InitStruct

GPIO_BaseInitTypeDef const CUR3_OP3P1_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_2,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		CUR3	CUR3_OP3P1_GPIO_InitStruct

GPIO_BaseInitTypeDef const CUR4_OP4P1_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_10,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		CUR4	CUR4_OP4P1_GPIO_InitStruct

GPIO_BaseInitTypeDef const OP1N1_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_3,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		OP1N1	OP1N1_GPIO_InitStruct

GPIO_BaseInitTypeDef const OP1O_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_4,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		OP1O	OP1O_GPIO_InitStruct


GPIO_BaseInitTypeDef const OP2N1_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_6,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		OP2N1	OP2N1_GPIO_InitStruct

GPIO_BaseInitTypeDef const OP2O_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_7,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		OP2O	OP2O_GPIO_InitStruct

GPIO_BaseInitTypeDef const OP3N1_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_1,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		OP3N1	OP3N1_GPIO_InitStruct

GPIO_BaseInitTypeDef const OP3O_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_0,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		OP3O	OP3O_GPIO_InitStruct

GPIO_BaseInitTypeDef const OP4N1_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_11,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		OP4N1	OP4N1_GPIO_InitStruct

GPIO_BaseInitTypeDef const OP4O_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_12,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_ANALOG,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Floating,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};




#define		OP4O	OP4O_GPIO_InitStruct

GPIO_BaseInitTypeDef const HRTIM_SYN_GPIO_InitStruct={
        
        GPIOB,
        
        {
            GPIO_PIN_6,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF12,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

#define		HRTIM_SYN	HRTIM_SYN_GPIO_InitStruct


GPIO_BaseInitTypeDef const TP1_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_15,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_OUTPUT_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};


#define		TP1	TP1_GPIO_InitStruct



GPIO_BaseInitTypeDef const SCR_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_6,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF1_TIM16,			/*uint32_t Alternate;  GPIO_AF2_TIM3!< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};


#define		SCR	SCR_GPIO_InitStruct



GPIO_BaseInitTypeDef const TP2_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_12,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_OUTPUT_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pulldown,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF0,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};


#define		TP2	TP2_GPIO_InitStruct


GPIO_BaseInitTypeDef const PA14_SDA1_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_14,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_OD,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pullup,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF4_I2C1,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};


GPIO_BaseInitTypeDef const PA13_SCL1_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_13,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_OD,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pullup,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF4_I2C1,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

GPIO_BaseInitTypeDef const UART2_RX_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_15,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pullup,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF7,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};

GPIO_BaseInitTypeDef const UART2_TX_GPIO_InitStruct={
        
        GPIOA,
        
        {
            GPIO_PIN_14,										/*uint32_t Pin;        !< Specifies the GPIO pins to be configured.
                                                                        This parameter can be any value of @ref GPIO_pins */
    
            GPIO_MODE_AF_PP,							/*uint32_t Mode;       !< Specifies the operating mode for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_mode */
    
            GPIO_PUPDR_Pullup,									/*uint32_t Pull;       !< Specifies the Pull-up or Pull-Down activation for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_pull */
    
            GPIO_SPEED_FREQ_VERY_HIGH,		/*uint32_t Speed;      !< Specifies the speed for the selected pins.
                                                                        This parameter can be a value of @ref GPIO_speed */
    
            GPIO_AF7,								/*uint32_t Alternate;  !< Peripheral to be connected to the selected pins
                                                                        This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
        }
};




#ifdef    COMM_UART

#define		TX_SEL		UART3_TX_SDA_GPIO_InitStruct            //信息口PB9
#define		RX_SEL		UART3_RX_SCL_GPIO_InitStruct            //[PB8]        
      


#define		SCL		PA13_SCL1_GPIO_InitStruct
#define		SDA		PA14_SDA1_GPIO_InitStruct

#else

#define		SCL		I2Cx_SCL                        //PB9
#define		SDA		I2Cx_SDA

#define		TX_SEL		UART2_TX_GPIO_InitStruct        //PA14            //信息口
#define		RX_SEL		UART2_RX_GPIO_InitStruct

#endif



//---------DEFINE CUR END-----------------------------------


static 	const  GPIO_BaseInitTypeDef* GPIO_PIN[]={

		&DEBUG_A,			//0
		&DEBUG_B,			//1
		&DEBUG_C,			//2
		&DEBUG_D,			//3
		&ZERO,				//4
		&VOLTAGE,			//5
		
		&PWM1L,				//6
		&PWM1H,				//7
		&PWM2L,				//8
		&PWM2H,				//9
		&PWM3L,				//10
		&PWM3H,				//11	
		&PWM4L,				//12
		&PWM4H,				//13	
	
		&BK12,				//14
		&BK34,				//15
	
		&T1A,				//16
		&T2A,				//17
		&T3A,				//18
		&T4A,				//19	

		// &PAN,				//20
		&PANSW1,			//21
		&PANSW2,			//22	
		&PANSW3,			//23
		&PANSW4,			//24

		&BOTTOM1,			//25
		&BOTTOM2,			//26
		&BOTTOM3,			//27
		&BOTTOM4,			//28	


		&IGBT1,				//29
		&IGBT2,				//30
		&IGBT3,				//29
		&IGBT4,				//30


		&FAN,				//31
		&FAN_AD,			//32	
		
		
		&SCL,				//33
		&SDA,				//34
		
		&CUR1,				//35
		&CUR2,				//36
		&CUR3,				//37
		&CUR4,				//38	
		
		// &OP1N1,				//39
		// &OP1O,				//40
		// &OP2N1,				//41	
		// &OP2O,				//42
		// &OP3N1,				//43
		// &OP3O,				//44	
		// &OP4N1,				//45
		// &OP4O,				//46

                &HRTIM_SYN,			//47
		&TX_SEL,						//48            //调试信息
		&RX_SEL,						//49
		&HRTIM_TEST1,
		&SCR,		
		&Ceil1,
		&Ceil2,
};





void		API_GPIO_PORT_INIT(void)
{	
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();	
	
	for(uint8_t i=0;i<PIN_MAX;i++)
	{
		DRV_GPIO_BaseInit((GPIO_BaseInitTypeDef*)GPIO_PIN[i]);
	}		
	

}

void		API_GPIO_BaseInit(uint8_t pin)
{
		DRV_GPIO_BaseInit((GPIO_BaseInitTypeDef*)GPIO_PIN[pin]);
}

 void	API_GPIO_WritePin(uint8_t pin,	uint8_t  value)
{
	DRV_GPIO_BaseWritePin((GPIO_BaseInitTypeDef*)GPIO_PIN[pin],value);
}	
uint8_t		API_GPIO_ReadPin(uint8_t pin)
{
	return	DRV_GPIO_BaseReadPin((GPIO_BaseInitTypeDef*)GPIO_PIN[pin]);
}

void		API_GPIO_PinPull(uint8_t pin,API_GPIO_PUPDR_ENUM pull)
{
	DRV_GPIO_PinPull((GPIO_BaseInitTypeDef*)GPIO_PIN[pin],pull);
}	
void		API_GPIO_Mode(uint8_t pin,API_GPIO_MODE_ENUM mode)
{	
		DRV_GPIO_Mode((GPIO_BaseInitTypeDef*)GPIO_PIN[pin],(uint32_t)mode);
}




