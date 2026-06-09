/********************************************************************************
    FileName    :  app_power.h
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  功率设置，调整功能实现

    Date        :  2018-10-20
    Modify      :
                   2018-10-20创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/


/********************************************Head Files*/
#ifndef APP_POWER_H
#define APP_POWER_H


/********************************************Head Files*/
#include "data_type.h"


/* === v2.0 Data Switcher: 跨模块结构体已迁移至 include/app_power_io.h ===
 * 原 _LINK 双向维护区 (S6/S7/S8) 已删除.
 * app_power 通过 Power_DoWork() 从 g_in (Power_Input_t) 消费 Switcher 填入的数据.
 * ==================================================================== */



#define		FRE_10			1			//微調步進，單位Hz
#define		FRE_20			2			//微調步進，單位Hz
#define		FRE_30			3			//微調步進，單位Hz
#define		FRE_40			4			//微調步進，單位Hz
#define		FRE_50			5			//微調步進，單位Hz
#define		FRE_60			6			//微調步進，單位Hz
#define		FRE_60			6			//微調步進，單位Hz
#define		FRE_70			7			//微調步進，單位Hz
#define		FRE_80			8			//微調步進，單位Hz
#define		FRE_90			9			//微調步進，單位Hz
#define		FRE_100			10			//微調步進，單位Hz
#define		FRE_120			12			//細調步進，單位Hz
#define		FRE_150			15			//細調步進，單位Hz
#define		FRE_200			20			//粗調步進，單位Hz
#define		FRE_300			30			//粗調步進，單位Hz
#define		FRE_400			40			//粗調步進，單位Hz
#define		FRE_500			50			//粗調步進，單位Hz
#define		FRE_600			60			//粗調步進，單位Hz
#define		FRE_700			70			//粗調步進，單位Hz
#define		FRE_800			80			//粗調步進，單位Hz
#define		FRE_900			90			//粗調步進，單位Hz
#define		FRE_1000		100			//粗調步進，單位Hz


#define		POWER_400		400			//低功率門限，
#define		POWER_500		500			//低功率門限，
#define		POWER_600		600			//低功率門限，
#define		POWER_700		700			//低功率門限，
#define		POWER_1000		1000		//低功率門限，
#define		POWER_DIFF_50	50
#define		POWER_DIFF_100	100
#define		POWER_DIFF_200	200
#define		POWER_DIFF_500	500
#define		POWER_DIFF_1000 1000		//量測功率與設定功率差門限，超過此門限使用粗調


//改为参数设置


#define C_VOLTAGE_210V 		0x8D		//210V电压ADC值

#ifdef CurrentFromTxa

	#define C_P25W 				95		//25W功率值  (电流*电压/(c_p25w*2))*25 = 输出功率,   如电压0X9C 电流0X86  输出功率2500W则 C_P25W=
//C_P25W= 电流*电压/(功率/25)/2
#else //CurrentFromTxa

	#define C_P25W 				0x47		//25W功率值

#endif //CurrentFromTxa

#define		PPG_PER_25W		2		//每25W PPG增加值，计算最大PPG	后面改成配置值
#define		PHASE_PER_ADJ	12		//每一相位偏差值，PPG增加的系数	
#define		PHASE_POT_UP	0x8a	//抬锅相位值
#define		PHASE_POT_LEAVE	0x90	//相位移锅值

#define		DEC_PWM_SURGE	16			//浪涌PPG减小值 

#define MinPowerM	500/25	//g_sys_para_init[6]	//最小连续功率设定


#define		OvpDiv			(4096*20/330)				//谐振电流限制毛刺干扰突升值 （100mv	

enum	
{
		PowerOffSurge=0x1,				//浪涌关机
		PowerOffCommLost	=0x2,		//无通讯关机
		PowerOffNoPan=0x11,				//无锅关机	
		PowerOffZero=0x12,				//无功率关机
		PowerOffCheckPan=0x13,		//检锅中间关功率	



};	



typedef struct
{
		uint8_t ihStatus;						//IH控制器状态寄存器：包含错误信息与工作状态
		uint8_t voltageAd;					//读取电压A/D采样值
		uint8_t currentAd;					//读取电流A/D采样值
		uint8_t igbtAd;							//读取IGBT温度传感器的A/D采样值
		uint8_t bottomAd;
		uint8_t	topAd;						//读取顶部温度传感器的A/D采样值
		uint8_t	actualPowerDiv25;		//读取实际功率值/25
		uint8_t	targetPowerDiv25;		//	//读取目标功率值/25
		uint8_t	actualPPG;					//读取实际加热PPG值
		uint8_t	powerStatus;				//读取功率限制状态（低4位）
		uint8_t	loadValue;					//读取负载检测脉冲数（低4位）和电压浪涌标志（高4位）
		uint8_t	vcountValue;				//反压计数器
		uint8_t	equivalentResistance;								//等效电阻 母线电流与谐振电流的平方
		uint8_t	res2;								//频率计数器  
		uint8_t	powerP25;						//25W修正值 
		uint8_t	checkSum;						//校验和

}PowerStatusDef;	


//#define	IHStatus	g_sys_para[0]	//IH控制器状态寄存器：包含错误信息与工作状态

//	#define B_INIT_SUC_FLAG		_BIT7	//初始化成功标志 0 不成功
//	#define B_POW_STB_FLAG		_BIT6	//功率稳定标志	0 不稳定
//	#define B_POW_ARRIVE_FLAG	_BIT5	//功率以经达到平衡 启动阶段
//	#define B_PAN_ADJ_FLAG		_BIT4	//有锅无锅标志  0 有锅
//	//错误代码定义

//	#define C_WORK_NOMAL		0
//	#define	C_ERR_MAIN			2				//;电路故障


typedef struct
{
	uint8_t 		loadTest;			//检锅强度设定
	uint8_t 		ovpShort;			//短路保护（高4位）
	uint8_t 		loadLeave;			//检锅功率设定
	uint8_t 		vcLimitMax;			//谐振电流保护值（高4位） 

	uint8_t 		loadLeavePhase;		//移锅相位值（高4位）
	uint8_t 		minPhase;					//最小相位设定(高6位）低电压区间电流限制值 ，低2位检锅信号下限值
	uint8_t 		potPowerM;				//钢锅铁锅修正，高4位 与最小相位的差值=H4+8 23~8  低4位 钢锅与铁锅的限制值差值=L4*100  1500~0
	uint8_t 		maxPowerM;	//最大连续功率设定

}PowerInitDef;						//初始化结构体

typedef struct
{
	uint8_t 		powerControlSet;		//检锅强度设定
	uint8_t 		powerSwitch;				//短路保护（高4位）
	uint8_t 		powerSetm;					//检锅功率设定
	uint8_t 		fanSpeed;						//风机调速

	uint8_t 		kValue;							//谐振电流保护值（高4位） 
	uint8_t			res1;
	uint8_t 		res2;
	uint8_t    	res3;
	
}PowerControlDef;						//初始化结构体


typedef struct 
{
		INT8U protect;    						/*写保护开关*/
		INT8U	len;										/*长度*/
		INT8U power25;     	    /*电压值210V*/
		INT8U	voltage210;						/*25w修正值 */
	
		INT8U	checksum;						/*校验和 */
		
}FlashValueDef;					//FLASH保存的数据

typedef struct 
{
	uint8_t 	potOff;
	uint8_t 	potCheck;
	uint8_t 	potResume;
	uint8_t 	potCycle;


}APP_POWER_ZeroReturn;			//过零状态


void u_power_init(void);				//初始化

void	PowerControlFun(uint8_t chn);		//功率大小控制




void	PowerPanCheckFun(uint8_t chn);		//锅具检测功能


//void		AdcValueFun(void);					//统一处理ADC值 
void		AdcGroupValueFun(void);			//主循环处理谐振电流



//void	s_pwm_off(INT8U	off_num);





#define	PluseUpCnt			2					//脉冲宽度次数阀值
#define	PluseUpValue			80				//脉冲最大值阀值
#define	FalseCnt			20				//脉冲错误次数阀值，超过退出检测


#define	POT_TYPE_DELAY1	10				//第一判锅点
#define	POT_TYPE_DELAY2	15				//第二判锅点

#define	PanUpValue			20			//脉冲速率增加值，小于此值认为没有增加
#define	PanUpMax			50				//脉冲统计最大值，可提前结束，减少运算时间
#define	PanUpMinLine		1600			//脉冲最小值， 小于认为=0
#define	APP_POWER_PAN_DELAY		30			//检锅延时参数

#define	PotChWork			PotCh1	//PotAll				//当等于0XFF时代表多炉头工作，1~4CH代表单炉头调试

#define	PotChWorkAll		0	// 1  多炉头 此时PotChWork无效  0：单炉头PotChWork工作
#define	TxaCount				5

#define	RESONANCE1_SURGE	1		//谐振电流超限
#define	RESONANCE2_SURGE	2
#define	VOLTAGE_SURGE		4		//电压浪涌	
#define	ERSG_SURGE			3		//错误
#define	PHASE_SURGE			5		//相位检测	

uint8_t	power_zero_adjust(uint8_t ch);				//返回值 1： 有新启动功率炉头

void 	TimIrqHandlePPGstepChange(void);
void 	Tim8IrqHandlePPGstepChange(void);

void 	AdcIrqHandleWatchDogLock(void);		//ADC_ WATCHDOG被锁定，关功率后锁定

void 	AdcIrqHandlePPGstepChangeCh1(void);		//这个要单独处理，两个炉头
void 	AdcIrqHandlePPGstepChangeCh2(void);		//这个要单独处理，两个炉头

void	TimIrqHandleBkCh1(void);						//过流BK中断回调
void	TimIrqHandleBkCh2(void);

void	PowerTypeFun(void);							//功率模式选择	
void	powerZeroChange(void);							//零点开关PPG


enum{
	TxaAwdHigh=0,
	TxaAwdLow,
};

enum{
	PanCheckRest=0,
	// PanSwitch,			//下一个过零点切换T1A T2A
	// PanStartPpg,		//准备起振
	PanDmaEnd,			//DMA读取完成
	PanFmacEnd,			//FMAC滤波完成
	PanPluseEnd,		//PAN脉冲检测完成	
	TxaDmaEnd,			//谐振电流DMA读取完成
};


	
void	APP_POWERR_SetTxaAwdValue(void);		//设置AWD值 谐振电流保护值 ch=0 正常高值  ch=1 低电压移锅值
void	API_POWER_PanFmac(void);					//PAN ADC FIR低通
void		API_POWER_PanCheckPluse(void);	//PAN ADC PLUSE检测

void	APP_POWER_TxaStrart(void);
void	APP_POWER_TxaDmaEnd(void);

void	APP_POWER_SetTxaAwdValue(void);		//watchDOG 值会变化，需要人为更新
void	APP_POWER_CompSetValue(void);			//设置CMP 比较电平
void	APP_POWER_PanStartPluse(void);		//产生起振脉冲，和功率控制分离
void	APP_POWER_PpgSetMinAll(void);			//所有炉头设置为最小功率

void	API_POWER_ScrOutput(uint8_t TskId);				//过零开通SCR
void	APP_POWER_CycleChange(void);				//中途切换同频与倍频
void	APP_POWER_CycleReset(void);			//恢复倍频
void	APP_POWER_DutyLess50(void);				//检查主频率炉头有没有发生切换，有可能需要更新限制值
void	APP_POWER_PotTypeCheck(void);					//检查锅具类型，确定最大PPG
void	APP_POWER_PotCheckRest(void);				//重新炉头检查


int16_t* 	APP_POWER_GetPanDmaBuffAddress(void);		//返回PAN  ADC缓存
int16_t* 	APP_POWER_GetPanFmacBuffAddress(void);		//返回PAN  fMAC缓存


#if 0		//通讯接口实现

void	API_POWER_RxControlCallback(uint8_t chn, uint8_t* buff,uint8_t len);
void API_POWER_RxInitCallback(uint8_t chn,int8_t *buff, uint8_t len);
uint8_t* API_POWER_TxStatusCallback(uint8_t chn, uint8_t len)	
uint8_t* API_POWER_TxInitCallback(uint8_t chn, uint8_t len);	
#endif 

#endif

//**********************************end of file********************************
