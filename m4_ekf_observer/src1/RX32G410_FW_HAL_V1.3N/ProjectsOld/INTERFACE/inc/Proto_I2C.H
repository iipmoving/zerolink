/**
  ******************************************************************************
  * @file    APP_ROUTE.h
  * @author  
  * @version V1.1
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2022 Shanghai Macrogiga Electronics</center></h2>
  *
  ******************************************************************************
*/



#ifndef _PROTO_I2C_H
#define _PROTO_I2C_H




#include "data_type.h"


typedef	struct
{
	unsigned char	negBit		:1		;					//求反标志
	unsigned char	initBit		:1		;					//初始化完成标志
	
}I2cFlagDef;	




typedef	struct
{
	unsigned char	comByte;		//当前读取的数据长度	
	unsigned char I2cSuccess;		//通讯成功计数，长时间不恢复复位总线
	unsigned char I2cFail;		//通讯总线重置次数统计
	unsigned char	comByteSave;		//保存回读数据长度	
	
	

	unsigned char 	i2cChn;		//当前执行的I2C 子地址
	unsigned char 	comAddressSave;		//保留
	unsigned char		res1;	
	unsigned char		res2;		
	
	
	unsigned char* 	comBuffNow;				//新数据指针（当前读到的数据）
	unsigned char*	comBuffLast;			//缓存的数据指针（上一次读到的数据）
	

}I2cStructDef;			//公共运行静态变量


#if 0

#define C_POT_IN	0		//确认有锅
#define C_POT_MAY	1		//检锅过程中
#define C_POT_ERR	2		//确认无锅
#define C_MAIN_ERR	4		//电路故障





#define MinPowerM	500/25	//g_sys_para_init[6]	//最小连续功率设定





#define		SWITCH_OFF	0		//关机
#define		SWITCH_OFF_NO_POT	1	// 关功率不检锅
#define		SWITCH_OFF_POT	2		// 关功率检锅
#define		SWITCH_PAUSE	3		//暂停加热，不清PPG_BUFF

#endif

//extern INT8U	g_ppg_limit_type;			//PWM限制类型
//#define	MIN_PHASE_LIMIT		1				//最小相位限制
//#define	VC_LIMIT			2				//谐振电流限制
//#define	POT_TYPE_LIMIT		3				//锅具类型限制
//#define PER_25W_LIMIT		4				//每25W PPG增加值，计算最大PPG
//#define PER_PASHE_LIMIT		5				//根据相位值确定最大PPG
//#define	MAXPPG_LIMIT		6				//最大PPG限制


//改为参数设置
#define	CURRENTADDRESS	1				//电流位是否增加地址
             
void	iic_bus_updata(void);
uint8_t	I2cSuccessCount(void);
void	I2cSlaveInit(void);

void	SetI2cRxControlCallBackFun(void(* TaskAddrss)(uint8_t *buff, uint8_t len));   //I2C控制数据回调函数
void	SetI2cRxInitCallBackFun(void(* TaskAddrss)(uint8_t *buff, uint8_t len));   //I2C初始化数据回调函数
void	SetI2cSetTxValueCallBackFun(uint8_t* (* TaskAddrss)(uint8_t chn, uint8_t len));		//得到需要发送的数据

void*	getFlashValue(INT8U start_addr);				//得到FLASH值 

uint8_t 	getI2cChn(uint8_t ch);						//转换I2C通道号

#endif
