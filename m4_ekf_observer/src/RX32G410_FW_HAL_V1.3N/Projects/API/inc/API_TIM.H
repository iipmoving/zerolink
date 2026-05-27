#ifndef __TIM_H__
#define __TIM_H__

#include	"stdint.h"

#define	TIM_BLANKING_TEST	0			//COMP消抖定时器输出测试
#define	TIM_BLANKING_PLUSE		20		//COMP 消抖时长  500ns

#define	TIM_TXA_PERIO_us	0.5            //比两个AD采样时间略长

#define	TIM_TXA_DIV_ADC		0.01

#define	TIM_TXA_CC1_us	    TIM_TXA_DIV_ADC+ TIM_TXA_PERIO_us/2            //比ADC 0.25us略长 注意读的是上一个AD的值，第一个可能空
#define	TIM_TXA_CC2_us	    TIM_TXA_DIV_ADC             //比ADC 0.25us略长 注意读的是上一个AD的值，第一个可能空
#define	TIM_TXA_CC3_us	    TIM_TXA_CC1_us+0.01            //第二个ADC 采样点位
#define	TIM_TXA_CC4_us	    TIM_TXA_CC2_us+0.01             //比ADC 0.25us略长 注意读的是上一个AD的值，第一个可能空


void		API_TIM_INIT(void);

void 	API_TIM_ZERO_Init(void);
void	API_TIM_100US_SET_50Hz(void);
void	API_TIM_100US_RESET(void);
void	API_TIM_100US_STOP(void);
void 	API_TIM_100US_Init(void);

void	API_TIM_PAN_RESET(void);
void	API_TIM_PAN_STOP(void);
void 	API_TIM_PAN_Init(void);

void	API_TIM_TXA_RESET(void);
void	API_TIM_TXA_STOP(void);



void 	API_TIM_TGO_PPG_SINGLE_Init(void);				//TIM7  触发PPG单脉冲
void	API_TIM_TGO_PPG_SINGLE_Start(void);


void	API_TIM_GCC_DELAY(uint32_t ns);							//ns延时
void	API_TIM_FanSetSpeed(uint8_t speed);					//风机转速取各炉头最大值
void	API_TIM_ScrSetSpeed(uint8_t speed);                 //继电器开关，  speed/100
#endif
