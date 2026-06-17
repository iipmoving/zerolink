#ifndef __API_ADC_H__
#define __API_ADC_H__

#include	<stdint.h>




#define		AdcAvageCount		199 			//每个交流周期AD采样数量	
#define		TxaAvageCount		20 			//每个交流周期AD采样数量	
	
#define	TxA_ADC_GROUP_NUM		2						//检锅ADC通道数
#define	Current_ADC_GROUP_NUM		2				//谐振电流單ADC规则通道数
#define	TxA_ADC_ALL_GROUP_NUM		4				//谐振电流全ADC规则通道数

#define	Tempe_ADC_GROUP_NUM		8				//
#define	VcIc_ADC_GROUP_NUM		8			//电压电流规则通道数	


#define	TxA_ADC_AdcBUFF_NUM		250			//adc触发 DMA 获取DR值的缓存组数，防止取数时被DMA覆盖	8位ADC规则通道数量	
#define	TxA_ADC_TimBUFF_NUM		8				//TIM触发 DMA 获取DR值的缓存组数，防止取数时被DMA覆盖	8位ADC规则通道数量	
#define	VcIc_ADC_BUFF_NUM		8				//8位ADC规则通道数量	


#define	TxA_ADC_AdcDMA_BUFF_NUM	TxA_ADC_AdcBUFF_NUM*TxA_ADC_GROUP_NUM     	// 	
#define	Current_ADC_AdcDMA_BUFF_NUM	TxA_ADC_AdcBUFF_NUM*Current_ADC_GROUP_NUM*2     	//双缓存区 	

#define	TxA_ADC_TimDMA_BUFF_NUM	40*4		//DMA接收数量 保证能得到一个HRTIM周期的数据
#define	VcIc_ADC_DMA_BUFF_NUM	VcIc_ADC_GROUP_NUM*AdcAvageCount		//DMA接收数量 8*
#define	Pan_ADC_DMA_BUFF_NUM	300											//DMA接收数量 8*
#define	Hrtim_ADC_DMA_BUFF_NUM	4											//DMA接收数量 8*


enum
{
	VoltageADC1_Group=0,	
	PanADC1_ADC1Group,



	
};				//TXA 通道顺序
enum
{
	T3A_0_ADC3Group=0,
	T4A_0_ADC3Group,
	T3A_1_ADC3Group,
	T4A_1_ADC3Group,

	T3A_2_ADC3Group,
	T4A_2_ADC3Group,
	T3A_3_ADC3Group,
	T4A_3_ADC3Group,
	
};				//TXA 通道顺序

enum
{

	Bottom1ADC1_GroupCh=0,
	Bottom2ADC1_GroupCh,
	Bottom3ADC1_GroupCh,
	Bottom4ADC1_GroupCh,
	
};				//ADC1 TEMPE 通道顺序


enum
{

	Igbt1ADC2_GroupCh=0,
	Igbt2ADC2_GroupCh,
	Igbt3ADC2_GroupCh,
	Igbt4ADC2_GroupCh,	
	FanADC2_GroupCh,		
	
};				//ADC2	TEMPE 通道顺序




enum
{

		Current1ADC2_Group=0,	
		Current2ADC2_Group,		
		Current3ADC2_Group,		
		Current4ADC2_Group,	
	

//		PanADC2_Group,	
//	
//		PanJDRxADC2_Group=0,
		
};			//CURRENT ADC2 通道顺序



enum
{

	Igbt1ADC2_GroupZero=0,
	Igbt2ADC2_GroupZero,
	Igbt3ADC2_GroupZero,
	Igbt4ADC2_GroupZero,	
	Bottom1ADC1_GroupZero,
	Bottom2ADC1_GroupZero,
	Bottom3ADC1_GroupZero,
	Bottom4ADC1_GroupZero,	
	FanADC2_GroupZero,
	TempeGroupZeroMax,	
};				//ADC2	TEMPE 通道顺序






void API_ADC_Init(void);

void API_ADC_SetAdcWatchIT(uint8_t onOff);	

void	API_ADC_T34A_ENABLE_IT_EOC(void);	//ADC3	返回值代表当前EOC中断是否开启 1：开启 0：关闭
void	API_ADC_T34A_DISABLE_IT_EOC(void);	//返回值代表当前EOC中断是否开启 1：开启 0：关闭
uint8_t API_ADC_T34A_GET_IT_EOC(void);

void 	API_ADC_VcIc_ENABLE_IT_EOC(void);		//ADC2返回值代表当前EOC中断是否开启 1：开启 0：关闭
void 	API_ADC_VcIc_DISABLE_IT_EOC(void);			//返回值代表当前EOC中断是否开启 1：开启 0：关闭

void 	API_ADC_T12A_ENABLE_IT_EOC(void);		//ADC1返回值代表当前EOC中断是否开启 1：开启 0：关闭
void 	API_ADC_T12A_DISABLE_IT_EOC(void);			//返回值代表当前EOC中断是否开启 1：开启 0：关闭


uint8_t		API_DMA_M2M_FROM_T34A(uint32_t*	aDST_Buffer,uint32_t buff_size);		//读取ADC1 ADC3的缓存到DST
uint8_t		API_DMA_M2M_FROM_VcIc(uint32_t*	aDST_Buffer,uint32_t buff_size);		//读取ADC2的缓存到DST
void 		API_ADC_SetM2Mstatus(uint8_t m2m_sta);		//返回M2M STAUTS

void	API_ADC_DMA_ONOFF(uint8_t onOff);
typedef enum
{
	ADC_Select_VcIc=0,				//前199 采集主通道ADC
	ADC_Select_Tempe,					//最后1次 采副通道
//	ADC_Select_VcT2A,					//第二通道需要检锅了
	ADC_Select_Max,
}ADC_SELECT_ENUM;//ADCtemp





typedef struct 
{
	uint32_t* InstanceAddress; 		//ADC设置寄存器地址
	uint32_t* SaveAddress[3];					//设置保存地址

  	uint32_t DataLength;					//数据长度	
	uint32_t ch; 									//通道号
}ADC_M2M_RecoverDef;//ADCtemp




typedef enum
{
	ChAdc1_Vc=0,				//对应API ADC内部的HANDLE
	ChAdc2_Current,
	ChAdc3_Current,
	ChAdc_Max,
//	ChAdc1_Tempe,
//	ChAdc2_FanAd,
//	ADC_Select_WaitZero,						//等过零信号
}ADC_CH_ENUM;//ADCtemp

uint32_t API_ADC_GetAddressDR(ADC_CH_ENUM ch);	//得到对应通道的ADC DR值 
uint32_t API_ADC_GetAddressDRx(ADC_CH_ENUM ch,uint32_t channel);	//得到对应通道的ADC DRx值 
uint32_t API_ADC_GetAddressJDRx(ADC_CH_ENUM ch,uint32_t channel);
uint32_t API_ADC_GetDRx(ADC_CH_ENUM ch,uint32_t channel);	//得到对应通道的ADC DRx值 
ADC_M2M_RecoverDef	API_ADC_GetAddressInstance(ADC_CH_ENUM ch);


//void	API_ADC_AwdValue(ADC_CH_ENUM ch,uint16_t valueH,uint16_t valueL);	//设置ADC AWD值
void	API_ADC_TxaAwdValue(uint32_t value);									//同时对T12 T34设置高限值
//void API_ADC_PanAwdValue(uint32_t value);										//设置PAN AWD低限值


void	API_ADC_StopAllAdc(void);
void	API_ADC_StartAllAdc(void);
//enum{
//	M2M_FREE=0,			//M2M状态
//	M2M_BUSY,
//	M2M_VCIC_START,			//正在转换 VCIC
//	M2M_VCIC_END,
//	M2M_TxA_START,			//正在转换 TXA
//	M2M_TxA_END,
//	M2M_FAN_AD_START,		//正在转换 FAN_AD(VCIC)
//	M2M_FAN_AD_END,		//正在转换 FAN_AD(VCIC)	
//	M2M_TEMPE_START,			//正在转换 BOTTOM,IGBT1	
//	M2M_TEMPE_END,			//正在转换 BOTTOM,IGBT1		
//	M2M_ERR,
//};							//M2M转换状态	



void	API_ADC_DMA_HrtimCount_Init(uint32_t* dr);	//ADC3触发HRTIM CNT 读取DMA

void	API_ADC_SELECT(ADC_SELECT_ENUM ch);									//CH0  采集T1234 CH1 采集TEMPE


typedef enum
{
	CNDTR_START=0,
	CNDTR_END,

}CNDTR_TYPE_ENUM;




void	portPendvSet(void);
void	portPendvClear(void);


//void				API_ADC_PanCountInit(uint8_t ppgCh);							//检锅脉冲计数初始化
//uint8_t				API_ADC_PanCountGetValue(uint8_t ppgCh);							//得到检锅脉冲计数值
//void	API_ADC_DISABLE_IT_AWD(void);
//void	API_ADC_ENABLE_IT_AWD(void);
//void	API_ADC_ENABLE_IT_JEOC(void);							//PAN检锅JEOC中断
//void	API_ADC_DISABLE_IT_JEOC(void);							//PAN检锅JEOC中断

void		API_ADC_Pan_ConfigChannel(uint8_t ch);		//CH0  VCIC单次触发，CH1 PAN连续触发


void	API_ADC_ClearDrxValue(void);

void	API_ADC_VcIc_ENABLE(void);

void	API_ADC_VcIc_DISABLE(void);

uint32_t API_ADC_GetHTR(void);	

#endif
