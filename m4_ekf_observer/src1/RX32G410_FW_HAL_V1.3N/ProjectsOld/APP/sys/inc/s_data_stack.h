/********************************************************************************
    FileName    :  S_DATA_STACK_H
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  数据栈处理，数据压入或提取从栈中，有效值为栈顶

    Date        :  2018-09-06
    Modify      :
                   2018-09-06 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
#ifndef S_DATA_STACK_H
#define S_DATA_STACK_H


#include	<stdint.h>
//常量定义

#define     stackSize       4 
// PID结构体，存储定点数PID参数和状态  


//结构体定义
typedef  unsigned char     stack_point_t;          //栈结构类型 
  
// PID结构体，存储定点数PID参数和状态  
typedef struct {  
    stack_point_t top;                    // 栈顶位置
    stack_point_t stackBuff[stackSize+1];       //栈数据
}StackStructDef;   


//函数定义
void 					DataStruct_StackInit(StackStructDef* stack);
void 					DataStruct_StackDelet(StackStructDef* stack,stack_point_t value);     //从栈中去除一个数据 
void 					DataStruct_StackPush(StackStructDef* stack,stack_point_t value);   //从栈中压入一个数据
stack_point_t DataStruct_GetStackValue(StackStructDef* stack,uint8_t point);   //得到栈顶数据
uint8_t  			DataStruct_GetStackTop(StackStructDef* stack);   //得到栈顶位置  有几个炉头在加热
#endif

//**********************************end of file********************************

