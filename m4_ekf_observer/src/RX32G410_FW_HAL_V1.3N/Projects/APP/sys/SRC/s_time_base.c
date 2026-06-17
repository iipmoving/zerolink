/********************************************************************************
    FileName    :  s_time_base.c
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

#include	"string.h"
//#include "sys_mem.h"

#include	"data_type.h"
#include "s_time_base.h"

#include	"API_gpio.h"

/*********************************************局部变量申请*/

TIME_BASE_T  TimeBase;         /*时间相关变量*/
/*********************************************函数列表*/

/********************************************************************************
*name       : void Time_BaseInit(void)
*author     : rsl
*function   : 初始化时基变量
*para       : 无
*return     : NULL
*brief      :
********************************************************************************/
void Time_BaseInit(void)
{
    memset(&TimeBase, 0, sizeof(TIME_BASE_T));
}

/********************************************************************************
*name         : void Time_Base(void)
*author       : rsl
*function     : 时间标志设置函数
*para         : NULL
*return       : 0
*brief        : 时间标志在每个初始时间片运行一次，10ms
********************************************************************************/
void Time_Base(void)
{

    TimeBase.TimeFlg.byte &=0x0f;			//低四位不清
			API_GPIO_WritePin(DebugB_pin,0);	
    TimeBase.TickCnt++;                         /*1ms时间累加*/
    if((TimeBase.TickCnt % TIME_100MS_CNT) == 0)
    {
	
        TimeBase.TimeFlg.bit.Ms100 = _TRUE;      /*100MS时间标志*/
    }

    if((TimeBase.TickCnt % TIME_250MS_CNT) == 0)
    {
			
        TimeBase.Flash++;             /*产生控制数码管闪烁的时间参考*/
    }


    if((TimeBase.TickCnt % TIME_500MS_CNT) == 0)
    {
        TimeBase.TimeFlg.bit.HalfSec = _TRUE;        /*500MS时间标志*/
    }
    //if(TimeBase.TickCnt >= 5)
    if(TimeBase.TickCnt >= TIME_1S_CNT)
    {
			API_GPIO_WritePin(DebugB_pin,1);	
        TimeBase.TimeFlg.bit.Sec = _TRUE;            /*1S时间标志*/
        TimeBase.TickCnt = 0;                   /*10ms时间计数清零，重新计数1s*/
        TimeBase.SecCnt++;
        if(TimeBase.SecCnt >= TIME_MIN_CNT) /*1分钟时间到*/
        {
            TimeBase.TimeFlg.bit.Min = _TRUE;
            TimeBase.SecCnt = 0;
        }

    }

}




//**********************************end of file********************************

