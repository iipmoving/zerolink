#include "SC_TK_Scan.h"
#include ".\TKDriverNew\TKDriver.h"






//==========================================================
//函数名称：void  Sys_Scan(void) 
//函数功能：扫描TK和显示
//入口参数：void
//出口参数：void  
//==========================================================
unsigned char Bit_SYS_Read_Key=0; //按键是否 有效 
unsigned long Sys_Scan(void)
{      				
		unsigned long  exKeyValueFlag=0;
	Bit_SYS_Read_Key=0;
	if(TK_TouchKeyStatus&0x80)	    //重要步骤2:  触摸键扫描一轮标志，是否调用TouchKeyScan()一定要根据此标志位置起后
	 {	   																	
		TK_TouchKeyStatus &= 0x7f;	//重要步骤3: 清除标志位， 需要外部清除。													    
		exKeyValueFlag = TK_TouchKeyScan();//按键数据处理函数    
		TK_Restart();				//启动下一轮转换																												 			
		Bit_SYS_Read_Key=1;
	 }		  	
	
	return    exKeyValueFlag;
}





