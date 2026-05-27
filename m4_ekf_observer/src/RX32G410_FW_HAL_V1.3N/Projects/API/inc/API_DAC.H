#ifndef __API_DAC_H__
#define __API_DAC_H__


#include	"stdint.h"





//enum{				//对外的IO口索引（需要软件操作的）外部访问时作为封装

//};
void API_DAC_Init(void);	
void	API_DAC_COMP_SetValue(uint8_t ch, uint8_t value);		//设置单个比较器比较电压
void	API_DAC_COMP_SetValueAll(uint8_t value);		//设置所有比较器比较电压

#endif
