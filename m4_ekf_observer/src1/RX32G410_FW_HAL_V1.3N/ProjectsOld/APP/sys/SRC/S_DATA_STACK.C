/********************************************************************************
    FileName    :  s_data_stack.c
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  基本时间函数
                可以产生100ms，500ms，1s, 1min的时间标志，可以记录系统时间

    Date        :  2018-09-06
    Modify      :
                   2018-09-06 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/


/********************************************Head Files*/



#include	"s_data_stack.h"

// PID结构体，存储PID参数和状态  

/*********************************************局部变量申请*/

/*********************************************函数列表*/





  
// 定义定点数类型，这里使用32位整数，并假设小数点后有16位（即精度为1/65536）  








/********************************************************************************
*name       : void DataStruct_StackInit()
*author     : rsl
*function   : 初始化栈
*para       : 
*return     : NULL
*brief      :
********************************************************************************/
// 初始化PID控制器  
  
// 初始化定点数PID控制器  
void DataStruct_StackInit(StackStructDef* stack)
{  
	stack->top=0;
  for(uint8_t i=0;i<stackSize;i++)
  {
        stack->stackBuff[i]=0;       //0x为无效数据
  }
}  

stack_point_t DataStruct_GetStackValueTop(StackStructDef* stack)   //得到栈顶数据
{
    stack_point_t   xReturn =0;      //无效数据
    if(stack->top>0&&stack->top<=stackSize)
    {
        xReturn=stack->stackBuff[stack->top-1];
    }
    return  xReturn;
}

stack_point_t DataStruct_GetStackValue(StackStructDef* stack,uint8_t point)   //得到栈顶数据
{
    stack_point_t   xReturn =0;      //无效数据
    if(stack->top>0&&point<=stack->top)
    {
        xReturn=stack->stackBuff[point-1];
    }
    return  xReturn;
}


uint8_t  DataStruct_GetStackTop(StackStructDef* stack)   //得到栈顶位置
{

    if(stack->top>stackSize)
    {
       stack->top=0;
    }
    return  stack->top;
}



void DataStruct_StackDelet(StackStructDef* stack,stack_point_t value) {  

 
    if(value>stackSize||value==0)      
    {
                      //ch无效
       return;
    }

    if(DataStruct_GetStackValue(stack,stack->top)==0)        //检查队列是否为空
    {
        return;
    }

    stack_point_t ch=0;

    for(uint8_t i=0;i<stack->top;i++)           //查找炉头号对应的栈号
    {

        if(ch==0)
        {
            if(stack->stackBuff[i]==value)
            {
                ch=i+1;
            }
        }
        if(ch>0&&i<stack->top)
        {
           stack->stackBuff[i]=stack->stackBuff[i+1];     
        }

    }
    stack->top-=1;


}  
void DataStruct_StackPush(StackStructDef* stack,stack_point_t value) {
 

    for(uint8_t i=0;i<stack->top;i++)           //先查找栈中有无对应炉头，有则先去除这一记录
    {
        if(stack->stackBuff[i]==value)
        {
            DataStruct_StackDelet(stack,value);
            break;
        }
    }

    if(stack->top<stackSize)
    {   
        stack->stackBuff[stack->top]=value;
        stack->top++;  
    }
    else
    {
        stack->top=0;       //所有数据无效
    }
}    


