//************************************************************
// Copyright (c) 深圳市鑫汇科电子有限公司
// 文件名称	:	data_type.h
// 作者		:	何志军
// 最后更正日期:	2010-06-08
// 版本		:	V1.0
// 更改记录	:	
//*************************************************************

#ifndef DATA_TYPE_C_HEADER
#define DATA_TYPE_C_HEADER

//typedef bit 			BOOL; 		// 布尔变量（位变量）
typedef unsigned char	CHAR;		// 字符变量
typedef unsigned char 	INT8U; 		// 无符号8位整型变量  
typedef signed char 	INT8S; 		// 有符号8位整型变量  
typedef unsigned int 	INT16U; 	// 无符号16位整型变量 
typedef signed int 		INT16S; 	// 有符号16位整型变量
typedef unsigned long 	INT32U; 	// 无符号32位整型变量
typedef signed long 	INT32S; 	// 有符号32位整型变量 
typedef float 			FP32; 		// 单精度浮点数(32位长度) 
typedef double 			FP64; 		// 双精度浮点数(64位长度)



typedef  unsigned int   uint32_t;
typedef  unsigned short uint16_t;
typedef  unsigned char  uint8_t;

typedef  signed int     int32_t;
typedef  signed short   int16_t;
typedef  signed char    int8_t;
typedef	 unsigned char 	__Bool;



#define 	true		1
#define 	false		0
#define 	enable		1
#define 	disable		0
#define		_TRUE		1
#define		_FALSE		0
//#define		ENABLE		1
//#define		DISABLE		0

#define		_BIT0	0x01
#define		_BIT1	0x02
#define		_BIT2	0x04
#define		_BIT3	0x08
#define		_BIT4	0x10
#define		_BIT5	0x20
#define		_BIT6	0x40
#define		_BIT7	0x80

#define		xdata 	 
#define		code	const	 






#endif
