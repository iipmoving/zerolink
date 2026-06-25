/********************************************************************************
    FileName    :  sys_queue.c
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  环形队列

    Date        :  2018-09-06
    Modify      :
                   2018-09-06 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/

/********************************************Head Files*/

/*********************************************常量声明*/

/*********************************************局部变量申请*/



#include "sys_queue.h"
#include <stdio.h>
#include <stdlib.h>



///*print the QueueDef*/
//void PrintQueue(QueueDef q) {
//	int i, j;
//	for (i = 0, j = q.front; i < QueueLength(q); i++, j = (j + 1) % MAXQSIZE) {
//		printf("%d\n",q.base[j]);
//	}
//}



/*********************************************函数列表*/

/********************************************************************************
*name         : void InitQueue(QueueDef *q) 
*author       : rsl
*function     : 初始化队列函数
*para         : 
*return       : 
*brief        :
********************************************************************************/
void InitQueue(QueueDef *q) 
{
	q->base = (Item*)malloc(MAXQSIZE * sizeof(Item));
	if (q->base == NULL)
	{	
		return;
	}
	q->front = 0;
	q->rear = 0;
}

/********************************************************************************
*name         : unsigned int QueueLength(QueueDef q) 
*author       : rsl
*function     : 队列内数据长度
*para         : 
*return       : 
*brief        : 
********************************************************************************/
/*return the length of the QueueDef*/
unsigned int QueueLength(QueueDef q) 
{
	return (q.rear - q.front + MAXQSIZE) % MAXQSIZE;
}
/********************************************************************************
*name         : void DestroyQueue(QueueDef *q)
*author       : rsl
*function     : 删除队列
*para         : 
*return       :
*brief        :
********************************************************************************/
/*Destroy the QueueDef*/
void DestroyQueue(QueueDef *q) {
	q->base = NULL;
	q->rear = 0;
	q->front = 0;
	free(q->base);
}

/********************************************************************************
*name         : bool IsEmpty(QueueDef q)
*author       : rsl
*function     : 查空
*para         : 
*return       : 
*brief        : 
********************************************************************************/
/*determine if the QueueDef is empty*/
bool IsEmpty(QueueDef q) {
	return q.rear == q.front;
}
/********************************************************************************
*name         : bool IsFull(QueueDef q)
*author       : rsl
*function     : 查满
*para         : 
*return       : 
*brief        : 
********************************************************************************/
bool IsFull(QueueDef q) {
	return (q.rear + 1) % MAXQSIZE == q.front;
}


/********************************************************************************
*name         : Item Top(QueueDef q) 
*author       : rsl
*function     : 队列队首数据
*para         : 
*return       : 
*brief        : 
********************************************************************************/
/*return the head elem of the QueueDef*/
Item Top(QueueDef q) {
	return q.base[q.front];
}

/********************************************************************************
*name         : Item Back(QueueDef q)
*author       : rsl
*function     : 队尾数据
*para         : 
*return       : 
*brief        : 
********************************************************************************/
/*return the back elem of the QueueDef*/
Item Back(QueueDef q) {
	return q.base[(q.rear - 1 + MAXQSIZE) % MAXQSIZE];
}
/********************************************************************************
*name         : bool EnQueue(QueueDef *q, Item e) 
*author       : rsl
*function     : 增加一个数据
*para         : 
*return       : 
*brief        : 
********************************************************************************/
/*enqueue, insert the rear*/
bool EnQueue(QueueDef *q, Item e) {
	if (IsFull(*q))
		return ERROR;
	q->base[q->rear] = e;
	q->rear = (q->rear + 1) % MAXQSIZE;
	
	return OK;
}
/********************************************************************************
*name         : bool DeQueue(QueueDef *q)
*author       : rsl
*function     : 读取一个数据
*return       : 
*brief        :
********************************************************************************/
/*dequeue, pop the front*/
bool DeQueue(QueueDef *q,Item* buff) {
	if(IsEmpty(*q))
	{	
		return ERROR;
	}
	*buff=q->base[q->front];
	
	q->front = (q->front + 1) % MAXQSIZE;
	return OK;
}


//**********************************end of file********************************
