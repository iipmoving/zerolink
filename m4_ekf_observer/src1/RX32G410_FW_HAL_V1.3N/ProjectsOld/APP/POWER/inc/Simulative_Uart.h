#ifndef __Simulative_Uart_H_
#define __Simulative_Uart_H_

#include	"commclass.h"



		//  滑条款 没有 IO口 用不了
//#if  TYPE_NUM == TYPE_SIMPLE
 
//#define	 DF_Use_Simulative_UART    //是否 使用模拟串口
//  
//#endif  

 
extern unsigned int  Sur_TAtest[18];

extern void Simulative_Uart_TX(void);//125us 调用   顺序1
extern void Simulative_UR_TxDispose(void);//125us 调用 顺序2
extern void Simulative_Uart_Init(void);//模拟串口 初始化

UserStringDef* Debug_OutToPc(void);		//将需要发送的数据转换成串口发送的数据格式
void	setDebugOutBuff(int16_t value,uint8_t ch);				//向测试缓存区写入数据
 
#define Debug_SetCh(ch, dat) Sur_TAtest[ch] = dat
#define DEBUG_OUT_CH_NUM    16


#endif




