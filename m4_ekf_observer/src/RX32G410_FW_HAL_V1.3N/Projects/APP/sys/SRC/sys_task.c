/********************************************************************************
    FileName    :  SYS_TASK.c
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  系统任务处理文件，系统任务的安排和切换
                一共创建5个时间片任务，每个任务占用固定的时间片，每个时间片2ms
                一共创建3个其他时任务，在每个时间片之后都能运行

    Date        :  2018-09-06
    Modify      :
                   2018-09-06 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
#include "sys_task.h"
#include	"s_time_base.h"
#include	"APp_zero.h"

//************************外部引用

//************************内部变量声明
SYS_TSK_STR  SysTask;
TSK_FUN*  	TskFun=0;				//任务指针

//************************************************函数列表




/********************************************************************************
*name       : void Sys_SetTskFunAddress(TSK_FUN* point)
*author     : moving
*function   : 任务指针赋值
*Para       : us
*return     : 无
*brief      : 
********************************************************************************/
void Sys_SetTskFunAddress(TSK_FUN* point)
{
	TskFun=point;
}	

/********************************************************************************
*name       : void Sys_DlyUs(INT16U us)
*author     : rsl
*function   : 延时us时间
*Para       : us
*return     : 无
*brief      : 并不是准确的1us时间，只是相对短一点的延时
********************************************************************************/
void Sys_DlyUs(INT16U us)
{
    while(us--)
    {
        ;
    }
}
/********************************************************************************
*name       : void Sys_DlyMs(INT16U ms)
*author     : rsl
*function   : 延时ms时间
*Para       : ms
*return     : 无
*brief      : 并不是准确的1ms时间，只是相对长一点的延时
********************************************************************************/
void Sys_DlyMs(INT16U ms)
{


//		while(ms--)
//		{
//			for(i=0; i<250; i++);
//			Bsp_WTDFeed();      //看门狗
//		}
//		vTaskDelay(1);
	
}

void Task_Null(void)        //空任务
{

}

/********************************************************************************
*name       : void SYS_RunTask(INT16U tsk_id)
*author     : rsl
*function   : 运行某个任务
*Para       : tsk_id 任务id
*return     : NULL
*brief      : 在任务列表里置位，等下一次任务切换的时候开始执行任务
********************************************************************************/
void Sys_RunTask(INT16U tsk_id)
{
    if(tsk_id < SYS_TSK_NUM)
    {
        SysTask.TskTabl |= (1<<tsk_id);
    }
}
/********************************************************************************
*name       : void SYS_StopTask(INT16U tsk_id)
*author     : rsl
*function   : 停止任务
*Para       : 无
*return     : NULL
*brief      : 把任务列表清空
********************************************************************************/
void Sys_StopTask(INT16U tsk_id)
{
    if(tsk_id < SYS_TSK_NUM)
    {
        SysTask.TskTabl &= ~(1<<tsk_id);
    }
}

/********************************************************************************
*name       : void SYS_TaskService(void)
*author     : rsl
*function   : 系统任务执行函数
*Para       : 无
*return     : NULL
*brief      :
********************************************************************************/
extern	void	s_sum_ic_vc(void);		//IC VC 20ms为周期求平均值
void Sys_TaskService(void)
{

		if(TimeBase.TimeFlg.bit.Zero)
		{	

				TimeBase.TimeFlg.bit.Zero=0;
				SysTask.TskId=0;							//20ms强制重新复位
//					s_sum_ic_vc();
		}	
	
		(*TskFun[SysTask.TskId%SYS_TIMCHIP_NUM])();       //运行任务函数 统计了20次   
	


		while(Time_GetMsFlg()==0)
		{	
				(*TskFun[SYS_TIMCHIP_NUM])();			//每次运行后会执行的进程, 尽可能短
		}	

		Time_ClrMsFlg();
				

		
		

//		SysTask.TskId%=SYS_TIMCHIP_NUM;
		if(SysTask.TskId<(SYS_TIMCHIP_NUM*2-1))
		{
				SysTask.TskId++;
		}
		
		(*TskFun[SYS_TIMCHIP_NUM+1])();				//每一ms执行一次的任务

	
	
}


uint8_t    SYS_GetTskId(void)
{
    return SysTask.TskId;
}

void	SYS_TaskRotate(void)
{	
    Sys_RunTask(SysTask.TimeChip);            //切换时间片任务 ======不可修改
		Sys_RunTask(SYS_TK_TSKID);
    SysTask.TimeChip++;                        //时间片轮转 ======不可修改
    SysTask.TimeChip %= SYS_TIMCHIP_NUM;
}





/********************************************************************************
*name       : void Sys_Init(void)
*author     : rsl
*function   : 系统任务初始化函数
*Para       : 无
*return     : NULL
*brief      :
********************************************************************************/
void Sys_Init(void)
{
    SysTask.TskTabl = 0;
    SysTask.TimeChip = 0;
    SysTask.TskId = 0;

}

//**********************************end of file********************************

