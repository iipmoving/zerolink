#ifndef __HRTIM_H__
#define __HRTIM_H__

//#include "rx32g4xx_config_def.h"
//#include "rx32g4xx_hal.h"
//#include "system_init.h"
//#include "system_bsp.h"

#include	<stdint.h>

#define HRTIM_INPUT_CLOCK           192000000
#define TIMD_PWM_FREQ               100000
#define TIMD_DUTY_CYCLE             0.5

//extern HRTIM_HandleTypeDef hhrtim1;


enum{
				//炉头序号，作为外部索引，内部指向对应HRTIM OP  CMP BK
	PotCh1=0,	//	Pot1_TimerIndex=HRTIM_TIMERINDEX_TIMER_B,
	PotCh2,		//Pot2_TimerIndex=HRTIM_TIMERINDEX_TIMER_E,
	PotCh3,		//Pot3_TimerIndex=HRTIM_TIMERINDEX_TIMER_A,
	PotCh4,		//Pot4_TimerIndex=HRTIM_TIMERINDEX_TIMER_D,
	PotChTest1,
	PotChBase,
//	PotMaster,		//potmaster是最后一个
	PotMax,	
	PotAll=0xff,
	PotNum=PotChTest1,
};	



typedef struct {
	uint16_t prioed;		//PPG周期
	uint16_t duty;			//PPG占空比 
	
}PPGvalueDef;	

typedef struct {

	uint16_t highOn;		//死区后高端开通
	uint16_t highOff;			//高端关闭，也就是DUTY PPG占空比 
	uint16_t lowOn;		//死区后低端开通
	uint16_t lowOff;			//低端关闭，也就是prioed	PPG周期

}PPGpointDef;	

//---------------常量设置 ---------------------------------------------------

enum
{
	PAN_INIT=1,
	PAN_START_PPG_COUNT=3,	
	PAN_END=0XFD,

};



#define d_PWM_F				25000	//initialization PWM Freq 
#define	PAN_START_PPG_COUNT			3			//起振脉冲个数 最小1  

#define DEAD0_TIME			2000 	/* PWM0 Dead Time set ns */
#define DEAD1_TIME			2000 	/* PWM1 Dead Time set ns */
#define DEAD0_1_TIME		2000 	/* PWM0 and PWM1 Dead Time set ns */

#define DEC_PWM_Period  	200	
#define	MIN_PHASE			2500	/* minimum phase width 3000ns */
#define	PHASE0				0		/* Select Phase 0 to detect */
#define	PHASE1				1		/* Select Phase 1 to detect */

#define fPWM_div1           1
#define fPWM_div2           2
#define fPWM_div4           4
#define fPWM_div8           8
#define fPWM_m16           16	

#define	PWM_DIV	fPWM_div4
#define	SysPWM_Frequency	(192*1000*1000)			//主频时钟频率
#define	PWM_SF	SysPWM_Frequency*PWM_DIV/2	//PWM分频后频率 -半为DUTY

#if 0
#define	LARGE_FRE			20000	//4b00 minimum PWM frequency HZ ns //最大频率（BOAST档）*/
#define	MID_FRE				22000	//442e minimum PWM frequency HZ ns //中间级最大频率（铁锅最大频率）*/
#define	MIN_FRE				29500	/*/32d8 minimum PWM frequency HZ ns //第一级最大频率（钢锅最大频率）*/
#else
#define	LARGE_FRE			21500	//4b00 minimum PWM frequency HZ ns //最大频率（BOAST档）*/
#define	MID_FRE				22500	//442e minimum PWM frequency HZ ns //中间级最大频率（铁锅最大频率）*/
#define	MIN_FRE				29500	/*/32d8 minimum PWM frequency HZ ns //第一级最大频率（钢锅最大频率）*/
#endif


#define	MAX_FRE				60000	//maximum PWM frequency					
#define	LOAD_FRE_IRON		25000	//移锅判断 PWM frequency
#define	LOAD_FRE_STEEL		30000	//铁锅移锅
#define	FRE_20K				20000	//移锅判断 PWM frequency					
#define	FRE_22K				22000	//移锅判断 PWM frequency					
#define	FRE_25K				25000	//移锅判断 PWM frequency					
#define	FRE_27K				27000	//移锅判断 PWM frequency					
#define	FRE_28K				28000	//移锅判断 PWM frequency					
#define	FRE_29K				29500	//移锅判断 PWM frequency					

#define	FRE_30K				30000	//移锅判断 PWM frequency					
#define	FRE_35K				35000	//移锅判断 PWM frequency					
#define	FRE_38K				38000	//移锅判断 PWM frequency					

#define	FRE_40K				40000
#define	FRE_42K				42000
#define	FRE_45K				45000
#define	FRE_50K				50000
#define	FRE_55K				55000
#define	FRE_500K			500000			//1us对应PWM值（500k)
#define	FRE_1000K			1000000			//0.5us对应PWM值（500k)
#define	FRE_2000K			2000000			//0.25us对应PWM值（500k)

#define	FRE_SOFTSTART			80000			//100K软启动


#define	FRE_PAN				60000	



#define	IRON_LOAD_FRE_PWM		(PWM_SF/LOAD_FRE_IRON)			//minimum PWM frequency 对应PWM值
#define	STEEL_LOAD_FRE_PWM		(PWM_SF/LOAD_FRE_STEEL)			//minimum PWM frequency 对应PWM值

#define	FRE_20K_PWM			(PWM_SF/FRE_20K)			//移锅判断 PWM frequency					
#define	FRE_22K_PWM			(PWM_SF/FRE_22K)			//移锅判断 PWM frequency					
#define	FRE_25K_PWM			(PWM_SF/FRE_25K)			//移锅判断 PWM frequency					
#define	FRE_27K_PWM			(PWM_SF/FRE_27K)			//移锅判断 PWM frequency					
#define	FRE_28K_PWM			(PWM_SF/FRE_28K)			//移锅判断 PWM frequency					
#define	FRE_29K_PWM			(PWM_SF/FRE_29K)			//移锅判断 PWM frequency					

#define	FRE_30K_PWM			(PWM_SF/FRE_30K)			//移锅判断 PWM frequency					
#define	FRE_35K_PWM			(PWM_SF/FRE_35K)			//移锅判断 PWM frequency					
#define	FRE_38K_PWM			(PWM_SF/FRE_38K)			//移锅判断 PWM frequency					
#define	FRE_40K_PWM			(PWM_SF/FRE_40K)			//移锅判断 PWM frequency					
#define	FRE_42K_PWM			(PWM_SF/FRE_42K)			//移锅判断 PWM frequency					
#define	FRE_45K_PWM			(PWM_SF/FRE_45K)			//移锅判断 PWM frequency					

#define	FRE_50K_PWM			(PWM_SF/FRE_50K)			//移锅判断 PWM frequency					
#define	FRE_55K_PWM			(PWM_SF/FRE_55K)			//移锅判断 PWM frequency
#define	FRE_500K_PWM		(PWM_SF/FRE_500K)			//移锅判断 PWM frequency	
#define	FRE_1000K_PWM		(PWM_SF/FRE_1000K)			//移锅判断 PWM frequency					
#define	FRE_2000K_PWM		(PWM_SF/FRE_2000K)			//移锅判断 PWM frequency					

#define	FRE_SOFTSTART_PWM	(PWM_SF/FRE_SOFTSTART)			//移锅判断 PWM frequency					


#define	LARGE_FRE_PWM			(PWM_SF/LARGE_FRE)	//minimum PWM frequency 对应PWM值
#define	MID_FRE_PWM			(PWM_SF/MID_FRE)	//minimum PWM frequency 对应PWM值
#define	MIN_FRE_PWM			(PWM_SF/MIN_FRE)	//minimum PWM frequency 对应PWM值
#define	FRE_CYCLE_PWM		MIN_FRE_PWM


#define	MAX_FRE_PWM			(PWM_SF/MAX_FRE)	//maximum PWM frequency	对应PWM值	 			
#define	PAN_FRE_PWM			(PWM_SF/FRE_PAN)			//移锅判断 PWM frequency					
#define	OFF_FRE_PWM			0

#define	START_FRE_PWM				FRE_40K_PWM			//移锅判断 PWM frequency					
#define	POTTYPE2_FRE_PWM		FRE_38K_PWM			//移锅判断 PWM frequency					

#define	MAX_FRE_PERIOD		MAX_FRE_PWM*2

#define TIMD_PERIOD 	MIN_FRE_PWM*2	//((uint16_t)((((uint64_t)HRTIM_INPUT_CLOCK) * 32) / TIMD_PWM_FREQ))


//#define		DTS2US				DTS1US*2		//1.6US		//0XC0+2000/DTS_PER_0xC0-32 
#if 0//DEBUG_POWER_OUT
#define		DTS1US				0x2
#define		DTS4US		 		0x2		//0XE0+4500/DTS_PER_0xE0-32		//DT=（32+DTG[4:0]）× Tdtg，其中 Tdtg = 16 × TDTS；
#define		DTS3US				0x2		
#define		DTS6US				0x2		
#else

#define		DTS1					48
#define		DTS1US				DTS1
#define		DTS2US				DTS1*2
#define		DTS3US		 		DTS1*3//DTS1US*4		//0XE0+4500/DTS_PER_0xE0-32		//DT=（32+DTG[4:0]）× Tdtg，其中 Tdtg = 16 × TDTS；
#define		DTS4US		 		DTS1*4//DTS1US*4		//0XE0+4500/DTS_PER_0xE0-32		//DT=（32+DTG[4:0]）× Tdtg，其中 Tdtg = 16 × TDTS；

#define		DTS6US				0xff
#define		DTSMAX				0XFF
#endif

#define		HRTIM1_PAN_IRQHandler					HRTIM1_TIMA_IRQHandler
#define		HRTIM1_PAN_IRQn								HRTIM1_TIMA_IRQn
#define		IRQ_PRIORITY_HRTIM_PAN				IRQ_PRIORITY_HRTIM_TIMA





#define		HRTIM1_CH4_IRQHandler					HRTIM1_TIMD_IRQHandler
#define		HRTIM1_CH4_IRQn								HRTIM1_TIMD_IRQn
#define		IRQ_PRIORITY_HRTIM_CH4				IRQ_PRIORITY_HRTIM_TIMD



#define		HRTIM1_TEST1_IRQHandler					HRTIM1_TIMC_IRQHandler
#define		HRTIM1_TEST1_IRQn								HRTIM1_TIMC_IRQn
#define		IRQ_PRIORITY_HRTIM_TEST1				IRQ_PRIORITY_HRTIM_TIMC


#define		HRTIM1_TEST2_IRQHandler					HRTIM1_TIMF_IRQHandler
#define		HRTIM1_TEST2_IRQn						HRTIM1_TIMF_IRQn
#define		IRQ_PRIORITY_HRTIM_TEST2				IRQ_PRIORITY_HRTIM_TIMF

#if 1

#define		OUTPUTRESETCHN				HRTIM_OUTPUTRESET_TIMCMP4
#define		OUTPUTSETCHN				HRTIM_OUTPUTSET_TIMPER

//#define		OUTPUTRESETCHN				HRTIM_OUTPUTSET_TIMPER
//#define		OUTPUTSETCHN				HRTIM_OUTPUTRESET_TIMCMP4

#define		COMPAREUNIT_REST			HRTIM_COMPAREUNIT_4			//互补恢复 下管开通时间
#define		COMPAREUNIT_SYNC			HRTIM_COMPAREUNIT_1			//与COMP同步，用于UPD沿的消隐，利用比较器的遮罩  现在用UPD
#define		COMPAREUNIT_BLKS_START		HRTIM_COMPAREUNIT_4			//关断沿与复位沿同步 REST 与START 一起
#define		COMPAREUNIT_BLKS_END		HRTIM_COMPAREUNIT_3	

#define		COMPAREUNIT_PAN							HRTIM_COMPAREUNIT_2
#define		HRTIM1_PAN_ICR							HRTIM_TIMICR_CMP2C
#define		HRTIM1_PAN_ISR							HRTIM_TIMISR_CMP2
#define		HRTIM1_PAN_IT								HRTIM_TIM_IT_CMP2

#define		HRTIM1_TEST1_ICR						HRTIM_TIMICR_UPDC			//限流中断
#define		HRTIM1_TEST1_IT							HRTIM_TIM_IT_UPD

#define		HRTIM1_TEST2_ICR						HRTIM_TIMICR_CMP4C		//PPG更新中断
#define		HRTIM1_TEST2_IT							HRTIM_TIM_IT_CMP4





#else



#define		OUTPUTRESETCHN				HRTIM_OUTPUTRESET_TIMCMP1
#define		COMPAREUNIT_REST			HRTIM_COMPAREUNIT_1			//互补恢复 下管开通时间
#define		COMPAREUNIT_SYNC			HRTIM_COMPAREUNIT_2			//与COMP同步，用于UPD沿的消隐，利用比较器的遮罩
#define		COMPAREUNIT_BLKS_START		HRTIM_COMPAREUNIT_4	
#define		COMPAREUNIT_BLKS_END		HRTIM_COMPAREUNIT_3	

//#define		HRTIM1_TEST1_ICR						HRTIM_TIMICR_CMP1C
//#define		HRTIM1_TEST1_IT							HRTIM_TIM_IT_CMP1

#define		COMPAREUNIT_PAN			HRTIM_COMPAREUNIT_2
#define		HRTIM1_PAN_ICR							HRTIM_TIMICR_CMP2C
#define		HRTIM1_PAN_ISR							HRTIM_TIMISR_CMP2


//enum{				//POT炉头对应的HRTIM INDEX 

//	Pot1_TimerIndex=HRTIM_TIMERINDEX_TIMER_E,
//	Pot2_TimerIndex=HRTIM_TIMERINDEX_TIMER_B,
//	Pot3_TimerIndex=HRTIM_TIMERINDEX_TIMER_A,
//	Pot4_TimerIndex=HRTIM_TIMERINDEX_TIMER_D,
//	Test1_TimerIndex=HRTIM_TIMERINDEX_TIMER_C,		//始终保持占空比为50%的调试通道
//	Test2_TimerIndex=HRTIM_TIMERINDEX_TIMER_F,		//始终保持占空比为50%的调试通道	
//};



//HRTIM_FALUT_DEF const 	HRTIM_FAULT_NUM[6]=
//{
//	{	HRTIM_FAULT_1,HRTIM_FAULTSOURCE_INTERNAL,		HRTIM_FAULTPOLARITY_HIGH	,	},		//comp2
//	{	HRTIM_FAULT_4,HRTIM_FAULTSOURCE_INTERNAL,		HRTIM_FAULTPOLARITY_HIGH	,	},		//comp1
//	{	HRTIM_FAULT_5,HRTIM_FAULTSOURCE_INTERNAL,		HRTIM_FAULTPOLARITY_HIGH	,	},		//COMP3
//	{	HRTIM_FAULT_2,HRTIM_FAULTSOURCE_INTERNAL,		HRTIM_FAULTPOLARITY_HIGH	,	},		//COMP4
//	{	HRTIM_FAULT_6,HRTIM_FAULTSOURCE_DIGITALINPUT,	HRTIM_FAULTPOLARITY_HIGH,	},		//pc10
//	{	HRTIM_FAULT_3,HRTIM_FAULTSOURCE_DIGITALINPUT,	HRTIM_FAULTPOLARITY_HIGH,	},		//pe6


//};	




#endif

/*
hrtim_in_flt1[4:1] PA12 COMP2 EEV1 N/A 
hrtim_in_flt2[4:1] PA15 COMP4 EEV2 N/A 
hrtim_in_flt3[4:1] PE6 - EEV3 N/A 
hrtim_in_flt4[4:1] PB11 COMP1 EEV4 N/A 
hrtim_in_flt5[4:1] PA11/PC7 COMP3 EEV5 N/A 
hrtim_in_flt6[4:1] PC10 - EEV6 N/A */

#define		HRTIM_FAULTFILTER_VALUE	HRTIM_FAULTFILTER_4		//FAULT消抖次数
#define		BLKS_DIV			FRE_500K_PWM*1				//消隐的时间偏移量  FRE_50K_PWM=10us/5=2us


#define		HRTIM_FAULT_POT1		HRTIM_FAULT_4
#define		HRTIM_ISR_POT1			HRTIM_ISR_FLT4			//HRTIM_FAULT_4
#define		HRTIM_ICR_POT1			HRTIM_ICR_FLT4C

#define		HRTIM_FAULT_POT2		HRTIM_FAULT_1
#define		HRTIM_ISR_POT2			HRTIM_ISR_FLT1			//HRTIM_FAULT_1
#define		HRTIM_ICR_POT2			HRTIM_ICR_FLT1C


#define		HRTIM_FAULT_POT3		HRTIM_FAULT_5
#define		HRTIM_ISR_POT3			HRTIM_ISR_FLT5			//HRTIM_FAULT_5	
#define		HRTIM_ICR_POT3			HRTIM_ICR_FLT5C

#define		HRTIM_FAULT_POT4		HRTIM_FAULT_2
#define		HRTIM_ISR_POT4			HRTIM_ISR_FLT2			//HRTIM_FAULT_2
#define		HRTIM_ICR_POT4			HRTIM_ICR_FLT2C



#define		HRTIM_ADJ_1000nS		FRE_500K_PWM
#define		HRTIM_ADJ_500nS			FRE_500K_PWM/2
#define		HRTIM_ADJ_1300nS		1040			//1.3us对应的HRTIM值

#define		HRTIM_ADJ			HRTIM_ADJ_500nS			//两个ADC一组 每个0.25us
	

#define		FRE_PER_ADC		HRTIM_ADJ			//0.5us




void API_HRTIM1_Init(void);
void API_SystemClocks_Init(void);


//---------PPG API函数----------------------------------------

void	API_PPG_SET_CONTINUOUS(uint8_t ch);			//对应通道PPG设置为半脉冲输出或连续输出
void	API_PPG_SET_SINGLE(uint8_t ch);


void		API_PPG_setPluse(uint8_t ppgCh ,uint16_t value);	
void		API_PPG_setPeriodChx(uint8_t ppgCh ,uint16_t value);	

void		API_PPG_setValueChx(uint8_t ppgCh ,uint16_t period,uint16_t duty);
void		API_PPG_setPeriod(uint16_t value);			//设置PWM输出周期


void				API_PPG_DeadTime(uint8_t ppgCh,uint8_t upDts,uint8_t downDts);		//死区时间设置 
void				API_PPG_setValue(uint8_t ppgCh,PPGvalueDef value);			//设置PWM输出周期
PPGvalueDef			API_PPG_getValue(uint8_t ppgCh);					//PWM周期返回
PPGpointDef			API_PPG_getValueDeadTime(uint8_t ppgCh);		//减去死区的DUTY	
uint32_t    		API_PPG_GetPeroid(uint8_t ppgCh);

uint32_t			API_PPG_getDeadTime(uint8_t ppgCh);				//得到死区值


void				API_PPG_OnOff(uint8_t ppgCh,uint8_t flag);						//PWM输出开始
// uint8_t				API_PPG_BkFlag(uint8_t ppgCh);						//得到BKFLAG



uint8_t		API_PPG_BkFlag_Pot1(void);						//得到BKFLAG
uint8_t		API_PPG_BkFlag_Pot2(void);						//得到BKFLAG
uint8_t		API_PPG_BkFlag_Pot3(void);						//得到BKFLAG
uint8_t		API_PPG_BkFlag_Pot4(void);						//得到BKFLAG	
//void			API_PanCountInit(uint8_t ppgCh);						//检锅脉冲计数初始化
//uint8_t 		API_PanCountGetValue(uint8_t ppgCh);					//得到检锅数
//void  			API_PanCountSetValue(uint8_t ppgCh);					//开启计数器



void			API_Comp_SetSelPanCheck(uint8_t ppgCh);
void			API_Comp_SetSelPowerOn(uint8_t ppgCh);
void 			API_HRTIM_DISABLE_IT_REST(void);		
void 			API_HRTIM_ENABLE_IT_REST(void);

void 			API_HRTIM_BASE_DISABLE_IT_UPD(void);		//HRTIMF 基准中断	
void 			API_HRTIM_BASE_ENABLE_IT_UPD(void);
void 			API_HRTIM_BASE_DISABLE_IT_CMP(void);		
void 			API_HRTIM_BASE_ENABLE_IT_CMP(void);


uint8_t	 		API_HRTIM_GET_IT_UPD(uint8_t ch);		
void	 		API_HRTIM_CLEAR_IT_UPD(uint8_t ch);	

uint32_t 		API_HRTIM_GetAddressTestCnt(void);			//得到HRTIM 基准通道CNFT
uint32_t 		API_HRTIM_GetAddressTxaCnt(uint8_t ch);			//得到HRTIM 基准通道CNFT地址
uint32_t 		API_HRTIM_GetTxaCnt(uint8_t ch);			//得到HRTIM 基准通道CNFT

void			API_HRTIM_CHECK_PAN_PLUSE(uint8_t ch );
void 			TIMsynchronous(void);			//HRTIM同步   ch 当前新增加的
void 	TIMsynchronousPower(void);			//HRTIM同步
void	API_HRTIM_PAN_CLEAR_FLAG(uint32_t ch);			//清除检锅起振HRTIM CMP2中断

void		API_HRTIM_SetDmaHandle(uint32_t* addr);


//=============================================================================
// 【新增】Master 同步模式 API (半桥通道内部同步，替代过零 IO 同步)
//=============================================================================
// 使用方式:
//   API_HRTIM1_Init();                                  // 原初始化(IO同步), 保留不动
//   API_HRTIM_MasterSync_InitMaster(basePeriod);         // 配置Master定时器
//   API_HRTIM_MasterSync_ConfigSlave(PotCh1, period1);   // 通道1 → Master同步
//   API_HRTIM_MasterSync_ConfigSlave(PotCh2, period2);   // 通道2 → Master同步(可不同频率)
//   API_HRTIM_MasterSync_StartAll();                     // Master+全部Slave同时启动
//
// 倍频示例: PotCh1=25kHz, PotCh2=50kHz, 同起始点
//   uint16_t basePer = FRE_25K_PWM * 2;
//   API_HRTIM_MasterSync_InitMaster(basePer);
//   API_HRTIM_MasterSync_ConfigSlave(PotCh1, basePer);       // 25kHz
//   API_HRTIM_MasterSync_ConfigSlave(PotCh2, basePer / 2);   // 50kHz
//   API_HRTIM_MasterSync_StartAll();
//   → 每个Master周期末尾全部归零，倍频通道自动对齐

void API_HRTIM_MasterSync_InitMaster(uint16_t masterPeriod);
void API_HRTIM_MasterSync_ConfigSlave(uint8_t ch, uint16_t slavePeriod);
void API_HRTIM_MasterSync_StartAll(void);
void API_HRTIM_MasterSync_StopAll(void);


// 寄存器地址定义（需根据实际硬件手册补充）
#define HTRIM_TEST1_CR    (0x40016900)  // 控制寄存器基地址（示例值）
#define HTRIM_TEST1_ICR   (0x88)        // 中断清除寄存器偏移
#define HTRIM_TEST1_IER   (0x8c)        // 中断使能寄存器偏移

// 宏1：清除定时器标志位（对应第一段汇编）
#define CLEAR_HRTIM_FLAG(base,reg, mask) 	


// 宏2：使能定时器中断（对应第二段汇编）
#define ENABLE_HRTIM_IT(base,reg, mask) \
    __asm volatile( \
        "MOV R0, %0\n\t" \
        "LDR R1, [R0, %1]\n\t" \
        "ORR R1, R1, %2\n\t" \
        "STR R1, [R0, %1]\n\t" \
    : : "r" (base),     /* 使用寄存器约束 */ \
        "i" (reg), \
        "i" (mask) \
    : "r0", "r1", "memory" \
    )




#endif
