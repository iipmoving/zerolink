#ifndef __Simulative_Uart_H_
#define __Simulative_Uart_H_





		//  滑条款 没有 IO口 用不了
//#if  TYPE_NUM == TYPE_SIMPLE
 
#define	 DF_Use_Simulative_UART    //是否 使用模拟串口
  
//#endif  

 


extern void Simulative_Uart_TX(void);//125us 调用   顺序1
extern void Simulative_UR_TxDispose(void);//125us 调用 顺序2
extern void Simulative_Uart_Init(void);//模拟串口 初始化

 
#define Debug_SetCh(ch, dat) Sur_TAtest[ch] = dat


#endif




