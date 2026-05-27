/********************************************************************************
    FileName    :  s_time_base.h
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  基本时间函数
                可以产生100ms，500ms，1s, 1min的时间标志，可以记录系统时间

    Date        :  2018-09-06
    Modify      :
                   2018-09-06 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/


#ifndef TIMER_BASE_H
#define TIMER_BASE_H

#include "data_type.h"

//#define TIME_DEBUG            /*时间调试模式，分钟比较快*/

/*********************************************常量声明*/


#define TIME_100MS_CNT  10  /*100ms包含的时间片个数*/
#define TIME_250MS_CNT  25   /*250ms包含的时间片个数*/
#define TIME_500MS_CNT  50  /*500ms包含的时间片个数*/
#define TIME_1S_CNT     100 /*1s包含的时间片个数*/


#ifdef TIME_DEBUG
#define TIME_MIN_CNT        5 /*1分钟内秒的次数*/
#else
#define TIME_MIN_CNT        60 /*1分钟内秒的次数*/
#endif

/*时间标志结构体，时间节点标志等的定义*/

typedef struct time_flag
{
		INT8U Ms     		:1;    /*1ms时间节点标志*/
		INT8U	Zero			:1;			/*过零复位信号*/
		INT8U res1     	:1;    /*10ms时间节点标志*/
		INT8U	res2			:1;			/*保留*/

		INT8U Ms100     :1;    /*100ms时间节点标志*/
    INT8U HalfSec   :1;    /*500ms时间节点标志*/
    INT8U Sec       :1;    /*1s时间节点标志*/
    INT8U Min       :1;     /*1分钟时间节点标志*/

} TIME_FLG_T;

typedef struct timer_base
{
    INT8U TickCnt;          /*10ms滴答的计数，用于确定100ms，500ms等时间节点*/
    INT8U SecCnt;           /*秒计数，用于分钟计数*/
		INT8U Flash;     				/*闪烁频率控制，每500ms计数一次*/
	
		union	
		{
			INT8U	byte;		
			TIME_FLG_T bit;     /*时间标志*/
		}	TimeFlg;
		
} TIME_BASE_T;

extern TIME_BASE_T TimeBase;          /*时间相关变量*/

void Time_BaseInit(void);
void Time_Base(void);

#define Time_SetZeroFlg() TimeBase.TimeFlg.bit.Zero=1;
#define Time_ClrZeroFlg() TimeBase.TimeFlg.bit.Zero=0;
#define Time_GetZeroFlg() TimeBase.TimeFlg.bit.Ms
#define Time_SetMsFlg() TimeBase.TimeFlg.bit.Ms=1;
#define Time_ClrMsFlg() TimeBase.TimeFlg.bit.Ms=0;
#define Time_GetMsFlg() TimeBase.TimeFlg.bit.Ms
#define Time_GetMs100Flg() TimeBase.TimeFlg.bit.Ms100
#define Time_GetHalfSecFlg() TimeBase.TimeFlg.bit.HalfSec
#define Time_GetSecFlg() TimeBase.TimeFlg.bit.Sec
#define Time_GetMinFlg() TimeBase.TimeFlg.bit.Min
#define Time_ClrSecCnt() TimeBase.SecCnt = 0
#define Time_GetFlash() TimeBase.Flash
#endif

//**********************************end of file********************************


