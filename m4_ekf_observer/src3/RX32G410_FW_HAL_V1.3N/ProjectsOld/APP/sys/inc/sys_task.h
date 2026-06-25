/********************************************************************************
    FileName    :  sys_task.h
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
#ifndef SYS_TASK_H
#define SYS_TASK_H

#include "data_type.h"

#define SYS_TSK_NUM 16   //任务数量
#define SYS_TIMCHIP_NUM 10   //时间片数量

typedef void (*TSK_FUN)(void);         //定义任务函数类型

typedef struct
{
    INT16U TskTabl;          //任务列表
    INT8U TimeChip;         //时间片
    INT8U TskId;            //任务ID

} SYS_TSK_STR;


//任务编号

#define SYS_IIC_TSKID 14        //IIC通讯任务
#define SYS_LED_TSKID 10        //LED显示任务
#define SYS_TK_TSKID  11        //TK触摸扫描任务


//extern SYS_TSK_STR xdata SysTask;

void Sys_Init(void);

void Sys_SetTskFunAddress(TSK_FUN* point);		//设置任务指针地址，初始化时调用

void Sys_TaskService(void);

void Sys_RunTask(INT16U tsk_id);
void	SYS_TaskRotate(void);				//时间片轮转
void Sys_DlyUs(INT16U us);
void Sys_DlyMs(INT16U ms);
void Task_Null(void);        //空任务

uint8_t    SYS_GetTskId(void);

#endif

//**********************************end of file********************************

