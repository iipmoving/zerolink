/**
  ******************************************************************************
  * @file    commclass.h
  * @author  
  * @version V1.1
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2022 Shanghai Macrogiga Electronics</center></h2>
  *
  ******************************************************************************
*/



#ifndef _COMM_CLASS_H
#define _COMM_CLASS_H


#include	"data_type.h"
#include	<stdarg.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<stdint.h>
typedef	struct
{
	size_t 		size;					//声明空间大小
	void* 		commSet;				//硬件设置参数
	void*		(*Init)(const void* self,va_list *app);		//初始化
	void*		(*Free)(void* self);
	void*		(*Read)(void* self);									//通用读取接口
	void		(*Send)(void* self,void *app);				//通用发送接口
	
}CommBaseDef;									//通讯父类定义	



typedef	struct
{
	unsigned char	max;		//当前数据长度	
	unsigned char	num;		//新数据累加，每发送一次成功数据加一	
	unsigned char	len;		//申请的空间长度	
	unsigned char res;		//保留
	int				p;					//空间地址(只是占用一个地址需要这个绝对地址	实际后面是一片缓存区
}UserStringDef;			//	


typedef	struct
{
	unsigned char	max;		//当前数据长度	
	unsigned char	num;		//新数据累加，每发送一次成功数据加一	
	unsigned char	len;		//申请的空间长度	
	unsigned char res;		//保留
	uint8_t*	p;					//存放空间地址
}UserGroupDef;			//			






//---------------这三个函数是基于父类对象的通用函数，函数中提供接口，-----------------------------
//----------------其实现方法在子类模块中实现，接口指向子类实现函数--------------------------------------
void* 	NewClass(const void *newClass,...);			//新建对象	返回子类对象指针
void*	FreeClass(void* item);										//释放对象
void*	ReadClass(void* sonPoint);								//从对象中接收数据	
void	SendClass(void* sonPoint,void* ap);						//向对象发送数据		
void*	CloneClass(void* sonPoint);	//拷贝一个对象，输入原对象指针，对象结构大小，输出新对象指针

CommBaseDef*  getFatherPoint(void* sonPoint);
//void*	UserStringInit(UserStringDef *uString,unsigned char max);

void*	MemCalloc(unsigned char max);								//不判断初始化，直接生成
void* MemNew(void* str, unsigned char max);				//判断是否初始化

#endif





