/**
  ******************************************************************************
  * @file    main.c
  * @author  
  * @version V1.1
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2023 Shanghai Macrogiga Electronics</center></h2>
  *
  ******************************************************************************
  */

#include	"commClass.h"


#ifdef __clang__
	#define		__va_start	va_start;
#else	

#endif	



enum
{
		FatherNum=0,							//这四个是默认位置 
		AttributeNum,
		RxBuffNum,
		TxBuffNum,

};	

//---------------------------------------------------------------------------
void* NewClass(const void *fatherClass,...)										//返回子类地址，父类地址在第一位
{
	 const CommBaseDef * base=(CommBaseDef*)fatherClass;

	void* commSon=calloc(1,base->size);								//子类地址		 创建子类内存空间
//	assert_param(commSon);
	*(const CommBaseDef**)commSon=base;										//保存命令列表
	
	
	

	
	va_list ap;		
#ifdef __clang__
		va_start(ap,fatherClass);
#else	
	__va_start(ap,fatherClass);
#endif		

	
	int*		element=(int*)commSon;
	uint8_t 	strMax[4];	
	uint8_t		len=base->size/sizeof(int);									//首址是父类地址，其它是空间指针
#ifdef __clang__
	strMax[0]=va_arg(ap,int);															//缓存区大小
	strMax[1]=va_arg(ap,int);															//缓存区大小
	strMax[2]=va_arg(ap,int);															//缓存区大小
	strMax[3]=va_arg(ap,int);	
#else
	strMax[0]=__va_arg(ap,int);															//缓存区大小
	strMax[1]=__va_arg(ap,int);															//缓存区大小
	strMax[2]=__va_arg(ap,int);															//缓存区大小
	strMax[3]=__va_arg(ap,int);															//缓存区大小
#endif
	
	for(uint8_t i=1;i<len;i++)
	{
			
			element[i]=(int)MemCalloc(strMax[i-1]);								//指针强制存到结构体地址位中
	}
		
	
	if(base->Init)
	{
		commSon=		base->Init(commSon,&ap);											//创建子类
	}		
#ifdef __clang__
		va_end(ap);
#else	
		__va_end(ap);	
#endif	
		return commSon;																	//子类指针

}

void*	FreeClass(void* sonPoint)						
{
		int		element;							//元素个数
//		int 	len;									//缓存区大小
		int 	*	point=sonPoint;
		void*		structPoint;
		void*		freePoint;				//需要FREE的空间地址				

	CommBaseDef*	father=(CommBaseDef*)(*point);		//第一个字节是父类指针

	
	if(father->Free)																//调用父类方法						
	{
		(father->Free(sonPoint));						//析构函数,每个子类中实现
		
		element=father->size/sizeof(int);	
		*point=NULL;
		point++;
		for(uint8_t i=1;i<element;i++)
		{	
				structPoint=point;
				freePoint=(void*)*point++;
		
				free((void*)freePoint);
				*(int*)structPoint=NULL;								//指针清除

		}
		free(sonPoint);
		sonPoint=NULL;
	}		
	return sonPoint;
}	



void*	CloneClass(void* sonPoint)
{
	
		uint8_t		element;							//元素个数
		uint8_t 	len;									//缓存区大小
		int *	point=(int*)sonPoint;
		int* 	newPoint;		

		CommBaseDef*	father=(CommBaseDef*)(*point);	//第一个字节是父类指针
		
		len=father->size;
	
	
//		newPoint=malloc(0x14);			//声明空间
		newPoint=calloc(1,len);			//声明空间
		if(newPoint==NULL)
		{
				return newPoint;
		}	
		
		newPoint[0]=point[0];					//保存father地址
		element=len/sizeof(int);
		for(uint8_t i=1;i<element;i++)
		{	
//			newPoint[i]=point[i];				//重新分配空间	
			
			len=*((uint8_t*)point[i]);		//将POINT强制转换为8位指针，得到结构体长度
		
	
			newPoint[i]=(int)MemCalloc(len);
			memcpy((int*)newPoint[i],(int*)point[i],len+sizeof(int));				//复制数据
			
		}
		return newPoint;				//返回新对象地址
}	


void*	CopyClass(void* sonPoint,void* sourcePoint)
{
	
		int		element;							//元素个数
		uint8_t  	len,srcLen;									//缓存区大小

		int *	srcPoint=(int*)sourcePoint;		//源结构	
		int *	dscPoint=(int*)sonPoint;					//目标结构				
		CommBaseDef*	father=(CommBaseDef*)(*dscPoint);	//第一个字节是父类指针
		CommBaseDef*	srcfather=(CommBaseDef*)(*srcPoint);	//第一个字节是父类指针
			
		if(father->size==srcfather->size)									//相同结构类型才能复制
		{
//				assert(father->size==srcfather->size);
				srcPoint[0]=dscPoint[0];
				for(uint8_t i=1;i<element;i++)
				{
						len=*((uint8_t*)dscPoint[i]);		//将POINT强制转换为8位指针，得到结构体长度
						srcLen=*((uint8_t*)srcPoint[i]);
						if(len==srcLen)
						{
//							assert(len==srcLen);
							memcpy((int*)dscPoint[i],(int*)srcPoint[i],len+sizeof(int));				//复制数据
						}
				}		
		}

		return dscPoint;				//返回新对象地址	
}	







void*	ReadClass(void* sonPoint)						
{


	int *	point=(int*)sonPoint;
	void* xReturn=NULL;
	CommBaseDef*	father=(CommBaseDef*)(*point);	//第一个字节是父类指针
	
	
	if((father->size/sizeof(int))<RxBuffNum)									//没有定义RXbuff
	{
			return xReturn;
	}	
	
	UserStringDef*	sonStr=(UserStringDef*)point[RxBuffNum];		//第三个属性一定是TX
	xReturn=sonStr;
	
	
	if(father->Read)																//调用父类方法						
	{

		xReturn=father->Read(sonStr);											//运行函数,每个子类中实现
	
	}		
	return 	xReturn;
}	

void	SendClass(void* sonPoint,void* app)						
{
	int *	point=(int*)sonPoint;
	CommBaseDef*	father=(CommBaseDef*)(*point);	//第一个字节是父类指针
	
	if((father->size/sizeof(int))<TxBuffNum)									//没有定义TXbuff
	{
			return;
	}	
	
	UserStringDef* 	sourceStr=(UserStringDef*)app;			//需要发送的数据
	UserStringDef*	sonStr=(UserStringDef*)point[TxBuffNum];		//第四个属性一定是TX
	
	if(sonStr->num!=sourceStr->num)							//数据有更新
	{	
			sonStr->num=sourceStr->num;	
			
			if(sourceStr->len>0&&sourceStr->len<sonStr->max)
			{	
				sonStr->len=sourceStr->len;
				memcpy(&(sonStr->p),&(sourceStr->p),sourceStr->len);
			}
			
	}

	
	if(father->Send)																//调用父类方法						
	{

		father->Send(sonPoint,&app);									//运行函数,每个子类中实现

	}	
	
	
}	


CommBaseDef*  getFatherPoint(void* sonPoint)
{	

	int* point=(int*)(sonPoint);
	return (CommBaseDef*)(*point);		//第一个字节是父类指针
}



void* MemNew(void* str, unsigned char max)
{
		if(str==NULL&&max)
		{
				str=calloc(1,max+sizeof(int));
				(*(int*)str)=(int)max;
		}
		return str;
}	



void*	MemCalloc(unsigned char max)
{

	uint32_t*		memPoint;				// 分配空间的指针  

	if(max)
	{
			memPoint=(uint32_t*)calloc(1,max+sizeof(int));			//多一个INT，代表空间大小等		
//			memPoint=(uint32_t*)malloc(max+sizeof(int));			//多一个INT，代表空间大小等	
			if(memPoint==NULL)
			{
					return memPoint;
			}		
		
//			(*classUser)=(uint32_t)memPoint;					//在结构体（输入）中保存空间地址
			*memPoint=max;								//第一个字节为长度
	}	
	return 	memPoint;
}	

void*	MemSetInt(uint32_t* str, uint32_t value,uint32_t size)
{
	for(uint16_t i=0;i<size;i++)
	{
		str[i]=value;
	}
	return	str;
}	

uint8_t publicBuff[MaxPublicBuffSize];
uint8_t* publicPoint=publicBuff;


uint16_t getPublicBuffSize(void)
{
	return 	publicPoint-publicBuff;
}	

void* getPublicBuffAdr(uint32_t point)		//得到公共缓存
{
	return	publicBuff;
}	

void PublicBuffFreeAll(void)
{
		publicPoint=publicBuff;
}	
void*	PubicBuffCalloc(uint32_t size)
{	
		uint8_t* buffStart=publicPoint;
		if(publicPoint+size<&publicBuff[MaxPublicBuffSize])
		{
				publicPoint+=size;
				return	buffStart;
		}	
		return 0;
}	
void	PubicBuffFree(void* buff)
{
	uint8_t* freeBuff=(uint8_t *)buff;
	
	if(freeBuff>publicBuff&&freeBuff<&publicBuff[MaxPublicBuffSize])
	{
			publicPoint=buff;
	}	
	
}