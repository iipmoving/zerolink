/*********************************************************************************************
Copyright <2024> <Icore Technology (Nanjing)  Co.,Ltd>	moving
All Rights Reserved,
Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials provided
with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to
endorse or promote products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************/

/*
*********************************************************************************************************
*                                              rx32g4xx
*                                           Library Function
*
*                                   Copyright 2024, RX Tech, Corp.
*                                        All Rights Reserved
*
*
* Project      : rx32g4xx
* File         : DRV_MCU.c
* By           : MOVING
*********************************************************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include	"APP_ADC.H"
#include	"API_hrtim.h"
#include	"API_adc.h"
#include	"API_gpio.h"
#include	"API_TIM.h"
#include		"api_dma.h"
#define PotChWork PotCh1
#include "adc_processing.h"
#include "../include/app_adc_io.h"

/* === __weak stubs → app_power (Pair T/U/V) ====================== */
__attribute__((weak)) void APP_POWERR_SetTxaAwdValue(void) {}
__attribute__((weak)) void APP_POWER_SetTxaAwdValue(void) {}
__attribute__((weak)) int16_t* APP_POWER_GetPanDmaBuffAddress(void) { return 0; }
#include	"API_FMAC.H"

#include	"printMessage.h"
#include	"power_calculator.h"
#include	"wave_capture.h"
#define		FMAC_OFFSET		(FmacLeve/2+2)

#define 	fmacLeveNum		FmacLeve			//根据FMAC滤波系数大小，确定缓存前置空间


static  uint16_t arrayPoint[4];
/* Private typedef -----------------------------------------------------------*/
typedef	struct __attribute__((aligned(32)))
{
	uint8_t 	adc20msCount;		//20MS 100US 次数统计
	uint8_t 	flag20ms;				//20MS adc平均值计算完成标志
	uint8_t		adcSelect;			//ADC规则通道选择
	uint8_t		res;
	
	uint32_t 	inputValue[AdcGroupMax];			//计算处理后的AD值 
	// uint32_t	adcConvertedData[AdcGroupMax];		//从M2M得到的数据
	uint32_t 	adcAverage[AdcGroupMax];			//平均值 
	// uint32_t	adcSum[AdcGroupMax];				//求和ADC值 
	
}APP_ADC_DEF;

//typedef	struct{

//	uint32_t  	buff[TxA_ADC_GROUP_NUM];	

//}ADC_DualChannel_Def;
//	
//typedef	struct{

//	uint16_t  	buff[TxA_ADC_ALL_GROUP_NUM];	

//}ADC_FourChannel_Def;


typedef	struct{

	uint32_t  	buff[VcIc_ADC_GROUP_NUM];	

}ADC_VcIcChannel_Def;


typedef	struct{	__attribute__((aligned(1)))

	uint16_t  	buff[TempeGroupZeroMax];	

}ADC_Tempe_Def;


typedef	struct{	__attribute__((aligned(1)))

	uint16_t  	buff[Current_ADC_GROUP_NUM];	

}ADC_Current_Def;



typedef struct
{
	uint16_t getMax:	8;			//连续采集MAX的平均值（20次） 
	uint16_t awgT12A:	1;			//ADC1发生AWG过流保护（找当次采集那个通道触发）
	uint16_t awgT34A:	1;			//ADC1发生AWG过流保护（找当次采集那个通道触发）
}ADC_Buff_FlagDef;

typedef struct
{
	uint8_t t100ms:	1;			//100ms取数时间到
	uint8_t dmaStart:	1;			//100msDMA开始 在DMA回调中切换到DMAEND
	uint8_t dmaEnd:	1;			//100msDMA结束，在主循环中处理
		
	
}ADC_T1A_Buff_FlagDef;	



typedef struct
{
	uint32_t	SumT1A;
	uint32_t	SumT2A;
	uint32_t	SumT3A;
	uint32_t	SumT4A;

}ADC_Buff_SumDef;

	


typedef struct	__attribute__((aligned(32)))
{
	
	uint8_t 				ch;		//dma当前组数		//	start;							//DMA开始点（0，或HALF)
	uint8_t 				res;	//dma以缓存数据组数
	uint8_t 				step;
	union{
	ADC_T1A_Buff_FlagDef	flag;
	uint8_t					byte;
	};										//标志点，切换是否需要采最大点
	uint32_t 		count;			//累加记数		
	uint32_t 		Voltage;		//当前组的电压值 

	uint16_t 						HrtimSyncBuff[4];			//HRTIM差值

	ADC_Current_Def			CurrentAdc2[2][TxA_ADC_AdcBUFF_NUM];					//DMA连续采集，用于过流检测
	ADC_Current_Def			CurrentAdc3[2][TxA_ADC_AdcBUFF_NUM];					//DMA连续采集，用于过流检测
	ADC_Current_Def			VcAdc1[TxA_ADC_AdcBUFF_NUM];
	
	ADC_Buff_SumDef			Sum;
	ADC_Buff_SumDef			Avg;													
					//DMA连续采集，用于过流检测

//	uint16_t						HrtimPoint[10];		
	ADC_Current_Def			CurrentAdc2Save[TxA_ADC_AdcBUFF_NUM];					//DMA连续采集，用于过流检测
	ADC_Current_Def			CurrentAdc3Save[TxA_ADC_AdcBUFF_NUM];
	uint16_t 						HrtimSave[PotNum][TxA_ADC_AdcBUFF_NUM];					//DMA连续采集，用于过流检测


	//	ADC_Current_Def			HrtimSaveBase[TxA_ADC_AdcBUFF_NUM];					//DMA连续采集，用于过流检测

	
}APP_ADC_AdcTxADMA_BUFF_DEF;			//ADC dma 缓存区


enum
{
	TXA_StepStart=0,
	TXA_StepDmaRest,			//恢复DMA
	TXA_StepDmaStart,
	TXA_StepDmaEnd,
	TXA_StepFmacStart,
	TXA_StepFmacEnd,					//FMAC完成
	TXA_StepPrintPause,			//打印数据时暂停TXA DMA
	
};	

typedef struct __attribute__((aligned(1)))
{
	
	uint8_t ch;							//DMA开始点（0start ，1HALF)
	uint8_t count;							//100ms計數值
	uint8_t step;						//工作步数
	union{
	ADC_T1A_Buff_FlagDef	flags;
	uint8_t	byte;
	};										//标志点，切换是否需要采最大点
	
	uint16_t  	Txa[4][TxA_ADC_TimDMA_BUFF_NUM];
	uint16_t		Vc[TxA_ADC_TimDMA_BUFF_NUM];
	
	uint16_t		Hrtim1us[TxA_ADC_TimDMA_BUFF_NUM];
	
	uint8_t		Hrtim[8];		//兩组缓存与T1A配合

}APP_ADC_TimTxADMA_BUFF_DEF;			//100us采集一组的集合

typedef struct	__attribute__((aligned(1)))
{
	uint32_t 							num;					//有效计数
	uint16_t 		Vc[AdcAvageCount];		//adc2*8
	ADC_Tempe_Def 				TempeAdc1;						//adc1
	ADC_Tempe_Def				TempeAdc2; 

//	uint16_t 				Txa[4][TxaAvageCount];		//保存TXA 100us采集值

	
}APP_ADC_TempeDMA_BUFF_DEF;			//DMA循环采样



typedef struct	__attribute__((aligned(32)))
{
	uint8_t 							num;					//有效计数
	uint8_t 				count;				//100ms計數
	uint16_t 				start;				//有效读数位置
	ADC_VcIcChannel_Def		VcIc[AdcAvageCount];		//adc2*199

	uint16_t				Txa[4][TxaAvageCount];			//20平均谐振电流	
	uint16_t 				voltage[4][TxaAvageCount];	
	
	uint16_t 				phase[4][TxaAvageCount];		//相位值
	uint16_t 				phaseValue[4][TxaAvageCount];


	
	uint16_t				iPeak[4][TxaAvageCount];			//峰值电流	
	uint16_t 				ceilQ[4][TxaAvageCount];

	uint16_t 				esr[4][TxaAvageCount];				//等效电阻
	
	ADC_Tempe_Def 			Tempe;						//adc1
	ADC_VcIcChannel_Def		FanAd;						//adc2
	ADC_VcIcChannel_Def 	res;	
	
}APP_ADC_AVG_BUFF_DEF;			//20ms采集一组的数据集合




/* DNTR data moved to app_power.c (Pair L/M/N migration) */

PowerCalculatorInputDef inputArray[4];		//两组同步炉头，每组是同频率




/* Private define ------------------------------------------------------------*/

#define		ADC_MAX_AVG_COUNT		10			//最大值取数次数


/* Private macro -------------------------------------------------------------*/




/* Private variables ---------------------------------------------------------*/





//volatile	APP_ADC_TimTxADMA_BUFF_DEF			TxA_ADC_TimDmaBuff;//,TxA_ADC_TimDma100msBuff;									//DMA连续采样的缓存区，DMA启动后不停止，应用程序需要根据CNDTR的值进行取数

//#define	TxA_ADC_TimDma100msBuff	TxA_ADC_TimDmaBuff
volatile	APP_ADC_AdcTxADMA_BUFF_DEF			TxA_ADC_AdcDmaBuff;									//DMA连续采样的缓存区，DMA启动后不停止，应用程序需要根据CNDTR的值进行取数
volatile	APP_ADC_TempeDMA_BUFF_DEF				Tempe_ADC_DmaBuff;				//热敏电阻AD值， 每20ms采集一次
//int16_t 	Pan_ADC_AdcDmaBuff[Pan_ADC_DMA_BUFF_NUM];

uint16_t 	TxaBuffCh1[TxA_ADC_AdcBUFF_NUM];



#define	VcIc_ADC_Buff				Tempe_ADC_DmaBuff.VcIc
#define	FanAd_ADC_Buff				Tempe_ADC_DmaBuff.FanAd
#define	BottomTempe_ADC_Buff		Tempe_ADC_DmaBuff.TempeAdc1
#define	IgbtTempe_ADC_Buff			Tempe_ADC_DmaBuff.TempeAdc2




//#define		AdcFromApiDma100us	TxA_ADC_TimDmaBuff			//从API DMA 每10us传过来的数据谐振
APP_ADC_AVG_BUFF_DEF		AdcFromApiDma20ms;				//从API DMA 每20ms传过来的数据
APP_ADC_DEF					AdcFunRam;
static Adc_Output_t g_out;                                        /* v2.0 Data Switcher 输出槽 */




#define		AdcInputValue 	AdcFunRam.inputValue		//计算处理后的AD值 
// #define		uhADCxConvertedData 	AdcFunRam.adcConvertedData
#define	 	Adc20msCount		AdcFunRam.adc20msCount					//20ms 
// #define 	Adc20msSum			AdcFunRam.adcSum								//20ms求和
#define 	AdcAverage20ms	AdcFunRam.adcAverage							//AD采集平均值20ms
#define 	IcVc20msOccur		AdcFunRam.flag20ms		
#define  	AdcSelect				AdcFunRam.adcSelect	
	
//uint16_t  	adcCount;
//uint16_t   	buff[450];
//uint16_t   	buff1[450];

//uint16_t*   point=buff;
//uint16_t*		point1=buff1;

//uint16_t		max1;
//uint16_t		max2;

IH_ElecInputDef		powerResult20ms[4];			//保存给20ms参数计算



/* Private function prototypes -----------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/



/* Private functions ---------------------------------------------------------*/
void		APP_ADC_SELECT_GROUP(uint8_t ch);//选择对应的ADC规则通道 OP1 P1P2
void		APP_ADC_ZERO_IrqFun(void);				//过零点进行ADC求平均值 
void		getADCmax(void);
void	APP_ADC_DMA_RecoverHrtim(void);			//每1ms更新DMA,保证TIM3与ADC同步
uint8_t   		APP_ADC_MesageBuff(void);


 /* Pair N moved to app_power.c — 整个限流子系统已迁出 */
__attribute__((weak)) void APP_ADC_IRQ_PPGstepChangeCallBack(void) //HRTIM1 UPD更新PPG
{}	


	
__attribute__((weak)) void APP_ADC_DebugValueCallBack(uint8_t ch,uint8_t value) //HRTIM1 CMP1 PPG减小限流操作
{}		
	
	
	/* ====== __weak stub — 接线到 methodology ih_elec_params 算法集 ====== */
__attribute__((weak)) void IhElecParams_Calculate(void *in, void *out) {}





	/* ====== test: 20ms 电参数计算 (4炉头统一调用 methodology 算法) ====== */
	void APP_ADC_ComputeElecParams(void)
	{
		static uint8_t elec_buf[4][64];	/* 64B/head, IH_ElecResult=58B */
		for (uint8_t i = 0; i < PotNum; i++) {
			IhElecParams_Calculate(&powerResult20ms[i], elec_buf[i]);
		}
	}	
	
// __weak		uint8_t APP_ADC_Power_ZeroIrqFun(void)
// {return 0;}	
/**
  * @brief  Initialize the GPIOx peripheral according to the specified parameters in the GPIO_Init.连带IO口号一起运算
  * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral for RX32G4xx family
  * @param  GPIO_Init: pointer to a GPIO_InitTypeDef structure that contains
  *         the configuration information for the specified GPIO peripheral.
            @arg @ref GPIO_InitTypeDef
  * @retval None
  */
	
uint32_t 	Adc_GetPowerTxa(uint8_t ch)
{
	uint32_t*  value=(uint32_t*)(&TxA_ADC_AdcDmaBuff.Avg);
	return 	value[ch];
}	

// uint16_t*		APP_ADC_GetHrtimAddress(uint8_t ch)
// {
// 		return	(uint16_t*)&(TxA_ADC_AdcDmaBuff.HrtimPotch[0][ch]);
// }


int16_t  icvalue[AdcAvageCount];	
	
int16_t*		getIcValueAdress(void)
{
		return	icvalue;
}

/* Pair J: STRONG — app_power 拉取 T12A DMA 缓冲指针 */
uint16_t* Adc_GetCurrentAdc2Ptr(void)
{
	return (uint16_t*)TxA_ADC_AdcDmaBuff.CurrentAdc2;
}

/* Pair K: STRONG — app_power 拉取 T34A DMA 缓冲指针 */
uint16_t* Adc_GetCurrentAdc3Ptr(void)
{
	return (uint16_t*)TxA_ADC_AdcDmaBuff.CurrentAdc3;
}	


enum
{
	AVG_TXA=0,		//Txa平均
	AVG_POWER,		//功率平均	
	AVG_VOLTAGE,	//电压平均		
	AVG_Q,			//Q值平均
	AVG_PHASE,		//相位值平均	
	AVG_MAX,
};	


void	Adc_TxaAvgReset(uint8_t ch)
{
	AdcAverage20ms[AdcGroupT1A+ch]=0;
}	


void	APP_ADC_TxaAvgSum(uint8_t ch)	//每20ms统计一次功率值
{
	
	uint16_t   	minTxa=0xffff;		//第一个数据保存最小值
	uint32_t 	txaValue[AVG_MAX];				//
	uint8_t		minNum=0;
	uint32_t 	newValue[AVG_MAX]={0,0,0,0,0};
	uint32_t	oldValue[AVG_MAX];
	
	uint64_t		powerSum=0;
		
	
	for(uint8_t num=1;num<TxaAvageCount-1;num++)
	{
		uint8_t num10=num%10;
//		if(num10==0)
//		{
//			continue;						//0点不计算
//		}	
		txaValue[AVG_TXA]=AdcFromApiDma20ms.Txa[ch][num];
		newValue[AVG_TXA]+=txaValue[AVG_TXA];	

		
		
//		txaValue[AVG_POWER]=AdcFromApiDma20ms.Power[ch][num];
//		newValue[AVG_POWER]+=txaValue[AVG_POWER];	

		txaValue[AVG_VOLTAGE]=AdcFromApiDma20ms.voltage[ch][num];
		newValue[AVG_VOLTAGE]+=txaValue[AVG_VOLTAGE];	
		
		uint64_t newPower=txaValue[AVG_TXA]*txaValue[AVG_VOLTAGE];		//电压*电流
		powerSum+=newPower;
		
		if(num10>=4&&num10<=5)
		{			//只取高电压段
			txaValue[AVG_Q]=AdcFromApiDma20ms.ceilQ[ch][num];	
			newValue[AVG_Q]+=txaValue[AVG_Q];	

			txaValue[AVG_PHASE]=AdcFromApiDma20ms.phase[ch][num];	
			newValue[AVG_PHASE]+=txaValue[AVG_PHASE];	
			

		}

		
//		if(minTxa>txaValue[AVG_TXA])
//		{
//			minTxa=txaValue[AVG_TXA];
//			minNum=num;
//		}	
	}

	uint16_t  value;

//每20mS取平均	
	txaValue[AVG_TXA]=AdcFromApiDma20ms.Txa[ch][10];				//谐振电流零点为minTxa/2
	txaValue[AVG_TXA]<<=1;			//0~19两个点用最小点
	
	newValue[AVG_TXA]+=txaValue[AVG_TXA];

	
	newValue[AVG_TXA]/=(TxaAvageCount);		//第0与20值为最小值

	txaValue[AVG_VOLTAGE]=AdcFromApiDma20ms.voltage[ch][10];
	txaValue[AVG_VOLTAGE]<<=1;
	newValue[AVG_VOLTAGE]+=txaValue[AVG_VOLTAGE];
	newValue[AVG_VOLTAGE]/=(TxaAvageCount);		//第0与20值为最小值

	powerSum+=txaValue[AVG_TXA]*txaValue[AVG_VOLTAGE];
	powerSum/=20;
	
//每20mS取平均

	newValue[AVG_Q]/=(4);		//第0与10  20值为最小值
	newValue[AVG_PHASE]/=4;		//第0与10  20值为最小值


//	newValue[AVG_PHASE]+=AdcFromApiDma20ms.phase[ch][minNum];
//	newValue[AVG_PHASE]+=AdcFromApiDma20ms.phase[ch][minNum];
//	功率值滤波

	AdcAverage20ms[AdcGroupPower1+ch]=powerSum>>4;

//TXA 一阶滤波
	if(AdcAverage20ms[AdcGroupT1A+ch]==0)
	{
		oldValue[AVG_TXA]=newValue[AVG_TXA];//初始化
	}
	else
	{
				//一阶滤波 Yn=Y(n-1)*(1-a)+a*x(n)
		oldValue[AVG_TXA]=AdcAverage20ms[AdcGroupT1A+ch]*(DIV_BASE-DIV_VALUE);
		oldValue[AVG_TXA]+=newValue[AVG_TXA]*DIV_VALUE;	
		oldValue[AVG_TXA]/=DIV_BASE;
	
	}
	AdcAverage20ms[AdcGroupT1A+ch]=oldValue[AVG_TXA];

//相位值一阶平均	
	if(AdcAverage20ms[AdcGroupPhaseDown1+ch]==0)
	{
		oldValue[AVG_VOLTAGE]=newValue[AVG_VOLTAGE];
	}
	else
	{
	
		oldValue[AVG_VOLTAGE]=AdcAverage20ms[AdcGroupPhaseDown1+ch]*(DIV_BASE_POWER-DIV_VALUE_POWER);
		oldValue[AVG_VOLTAGE]+=newValue[AVG_VOLTAGE]*DIV_VALUE_POWER;	
		oldValue[AVG_VOLTAGE]/=DIV_BASE_POWER;
	
	}	
	AdcAverage20ms[AdcGroupPhaseDown1+ch]=oldValue[AVG_VOLTAGE];	
	
//CEILQ 一阶平均
	if(AdcAverage20ms[AdcGroupCeilQ1+ch]==0)
	{
		oldValue[AVG_Q]=newValue[AVG_Q];
	}
	else
	{
		oldValue[AVG_Q]=AdcAverage20ms[AdcGroupCeilQ1+ch]*(DIV_BASE_Q-DIV_VALUE_Q);
		oldValue[AVG_Q]+=newValue[AVG_Q]*DIV_VALUE_Q;	
		oldValue[AVG_Q]/=DIV_BASE_Q;	
	}	
	AdcAverage20ms[AdcGroupCeilQ1+ch]=oldValue[AVG_Q];		
	
//相位值一阶平均
	if(AdcAverage20ms[AdcGroupPhase1+ch]==0)
	{
		oldValue[AVG_PHASE]=newValue[AVG_PHASE];
	}
	else
	{
		oldValue[AVG_PHASE]=AdcAverage20ms[AdcGroupPhase1+ch]*(DIV_BASE_Q-DIV_VALUE_Q);
		oldValue[AVG_PHASE]+=newValue[AVG_PHASE]*DIV_VALUE_Q;	
		oldValue[AVG_PHASE]/=DIV_BASE_Q;	
	}	
	AdcAverage20ms[AdcGroupPhase1+ch]=oldValue[AVG_PHASE];		




}	



void	APP_ADC_AVG_Fun(void)
{
		if(AdcFromApiDma20ms.num==0)		//有无新的数据进入
		{
				return;
		}
		AdcFromApiDma20ms.num=0;
		for(uint8_t i=0;i<AdcAvageCount;i++)			//VcIc累加
		{

//			AdcAverage20ms[AdcGroupCurrent1]+=	Tempe_ADC_DmaBuff.VcIc[i].buff[Current1ADC2_Group];
//			AdcAverage20ms[AdcGroupCurrent2]+=	Tempe_ADC_DmaBuff.VcIc[i].buff[Current2ADC2_Group];
//			AdcAverage20ms[AdcGroupCurrent3]+=	Tempe_ADC_DmaBuff.VcIc[i].buff[Current3ADC2_Group];
//			AdcAverage20ms[AdcGroupCurrent4]+=	Tempe_ADC_DmaBuff.VcIc[i].buff[Current4ADC2_Group];		
	
			AdcAverage20ms[AdcGroupVoltage]	+=	Tempe_ADC_DmaBuff.Vc[i];
			 
//			icvalue[i]=Tempe_ADC_DmaBuff.VcIc[i].buff[Current1ADC2_Group];		//保存电流数据用于输出


		}

		AdcAverage20ms[AdcGroupVoltage]/=AdcAvageCount;
		
//		for(uint8_t i=AdcGroupCurrent1;i<=AdcGroupVoltage;i++)
//		{
////			AdcAverage20ms[i] = ME_UDIV(Adc20msSum[i], AdcAvageCount);
//				AdcAverage20ms[i]/=AdcAvageCount;
//				// Adc20msSum[i]=0;
//			
//		}

		APP_ADC_TxaAvgSum(0);		//计算TXA有效值
		APP_ADC_TxaAvgSum(1);				
		APP_ADC_TxaAvgSum(2);
		APP_ADC_TxaAvgSum(3);


	
			
			
			
			


//-----------------以下ADC不需要滤波---------------------------------------------------------------------						

			AdcAverage20ms[AdcGroupIgbt1]=(AdcFromApiDma20ms.Tempe.buff[Igbt1ADC2_GroupZero]);
			AdcAverage20ms[AdcGroupIgbt2]=(AdcFromApiDma20ms.Tempe.buff[Igbt2ADC2_GroupZero]);
			AdcAverage20ms[AdcGroupIgbt3]=(AdcFromApiDma20ms.Tempe.buff[Igbt3ADC2_GroupZero]);
			AdcAverage20ms[AdcGroupIgbt4]=(AdcFromApiDma20ms.Tempe.buff[Igbt4ADC2_GroupZero]);
			
			
			AdcAverage20ms[AdcGroupBottom1]=(AdcFromApiDma20ms.Tempe.buff[Bottom1ADC1_GroupZero]);
			AdcAverage20ms[AdcGroupBottom2]=(AdcFromApiDma20ms.Tempe.buff[Bottom2ADC1_GroupZero]);
			AdcAverage20ms[AdcGroupBottom3]=(AdcFromApiDma20ms.Tempe.buff[Bottom3ADC1_GroupZero]);
			AdcAverage20ms[AdcGroupBottom4]=(AdcFromApiDma20ms.Tempe.buff[Bottom4ADC1_GroupZero]);
						
			if(Adc20msCount>=AdcAvageCount)			//采样数量不对，此次数据无效
			{
				IcVc20msOccur=1;	
			}		


}	



//-----------------返回标志接口函数--start------------------------------------------------------------------						
 
uint8_t		getIcVc20msOccur(void)
{
		uint8_t 	xReturn=IcVc20msOccur;			//自清除，只调用一处
		return xReturn;
}	
void		clrIcVc20msOccur(void)
{
		 IcVc20msOccur=0;
}	

uint32_t  getADCaverage20ms(uint8_t ch)
{
		return 	AdcAverage20ms[ch];
}	


// uint16_t	getADCxConvertedData(uint8_t ch)				//返回相应通道ADC数据
// {
// 			return 	uhADCxConvertedData[ch];
// }	

//-----------------返回标志接口函数-----end----------------------------------------------------------------						




//******************************************************************
// 函数名	：void			AdcValueFun(void)							
// 作者		：
// 功能		：//从DRV_mcu中获取ADC数据转换到
// 参数		：
// 返回值	：
// 调用全局变量:				
// 修改全局变量:			
// 备注：
//*****************************************************************



uint8_t 		AdcValueFun(void)					//统一处理ADC值 
{

		
	uint16_t		temp;
	uint16_t 	value;
	uint8_t xReturn=0;
	
	if(getIcVc20msOccur())			//自清除
	{
						clrIcVc20msOccur();					//得到20ms过零信号，两个炉头都加上标志
//						PowerMem[0].staticReg->flag.bit.IcVcAdcOk=1;		//标志在后续执行才会清除m_ic_vc_adc_ok_flag
//						PowerMem[1].staticReg->flag.bit.IcVcAdcOk=1;

		
						value=getADCaverage20ms(AdcGroupVoltage);
						temp=value;															//电压值*1.25
						temp>>=2;			//temp/4
						value+=temp;
						AdcInputValue[AdcGroupVoltage]=value;
		

	
//						AdcInputValue[AdcGroupCurrent1]=getADCaverage20ms(AdcGroupCurrent1);				//通道1电流值 
//						AdcInputValue[AdcGroupCurrent2]=getADCaverage20ms(AdcGroupCurrent2);				//通道2电流值 
//						AdcInputValue[AdcGroupCurrent3]=getADCaverage20ms(AdcGroupCurrent3);				//通道1电流值 
//						AdcInputValue[AdcGroupCurrent4]=getADCaverage20ms(AdcGroupCurrent4);				//通道2电流值 
//					
		

						AdcInputValue[AdcGroupT1A]=getADCaverage20ms(AdcGroupT1A);				//通道1谐振电流值 
						AdcInputValue[AdcGroupT2A]=getADCaverage20ms(AdcGroupT2A);				//通道2谐振电流值 
						AdcInputValue[AdcGroupT3A]=getADCaverage20ms(AdcGroupT3A);				//通道3谐振电流值 
						AdcInputValue[AdcGroupT4A]=getADCaverage20ms(AdcGroupT4A);				//通道4谐振电流值 

						AdcInputValue[AdcGroupPower1]=getADCaverage20ms(AdcGroupPower1);				//通道1瞬时功率值 
						AdcInputValue[AdcGroupPower2]=getADCaverage20ms(AdcGroupPower2);				//通道2瞬时功率值  
						AdcInputValue[AdcGroupPower3]=getADCaverage20ms(AdcGroupPower3);				//通道3瞬时功率值 
						AdcInputValue[AdcGroupPower4]=getADCaverage20ms(AdcGroupPower4);				//通道4瞬时功率值 

						AdcInputValue[AdcGroupCeilQ1]=getADCaverage20ms(AdcGroupCeilQ1);				//通道1瞬时功率值 
						AdcInputValue[AdcGroupCeilQ2]=getADCaverage20ms(AdcGroupCeilQ2);				//通道2瞬时功率值  
						AdcInputValue[AdcGroupCeilQ3]=getADCaverage20ms(AdcGroupCeilQ3);				//通道3瞬时功率值 
						AdcInputValue[AdcGroupCeilQ4]=getADCaverage20ms(AdcGroupCeilQ4);				//通道4瞬时功率值 

						AdcInputValue[AdcGroupPhase1]=getADCaverage20ms(AdcGroupPhase1);				//通道1瞬时功率值 
						AdcInputValue[AdcGroupPhase2]=getADCaverage20ms(AdcGroupPhase2);				//通道2瞬时功率值  
						AdcInputValue[AdcGroupPhase3]=getADCaverage20ms(AdcGroupPhase3);				//通道3瞬时功率值 
						AdcInputValue[AdcGroupPhase4]=getADCaverage20ms(AdcGroupPhase4);				//通道4瞬时功率值 

						AdcInputValue[AdcGroupPhaseDown1]=getADCaverage20ms(AdcGroupPhaseDown1);				//通道1瞬时功率值 
						AdcInputValue[AdcGroupPhaseDown2]=getADCaverage20ms(AdcGroupPhaseDown2);				//通道2瞬时功率值  
						AdcInputValue[AdcGroupPhaseDown3]=getADCaverage20ms(AdcGroupPhaseDown3);				//通道3瞬时功率值 
						AdcInputValue[AdcGroupPhaseDown4]=getADCaverage20ms(AdcGroupPhaseDown4);				//通道4瞬时功率值 





						for(uint8_t i=AdcGroupBottom1;i<AdcGroupFan;i++)
						{
								value =~getADCaverage20ms(i);									//热敏电阻ADC值求反
								value&=0xfff;

								AdcInputValue[i]=value;

						}
						

						

						xReturn=1;
	}
	return	xReturn;
}


// void	APP_ADC_GET_VcIcBuff(void)
// {
// 	uint32_t ch=AdcFromApiDma20ms.start>>3;		//注意取数时超过范围
	
// 	if(ch==0)
// 	{
// 			ch=1;
// 	}	
// 	ch-=1;

	
// 	AdcFromApiDma20ms.VcIc[Adc20msCount].buff[Current1ADC2_Group]=Tempe_ADC_DmaBuff.VcIc[ch].buff[Current1ADC2_Group];
// 	AdcFromApiDma20ms.VcIc[Adc20msCount].buff[Current2ADC2_Group]=Tempe_ADC_DmaBuff.VcIc[ch].buff[Current2ADC2_Group];
// 	AdcFromApiDma20ms.VcIc[Adc20msCount].buff[Current3ADC2_Group]=Tempe_ADC_DmaBuff.VcIc[ch].buff[Current3ADC2_Group];
// 	AdcFromApiDma20ms.VcIc[Adc20msCount].buff[Current4ADC2_Group]=Tempe_ADC_DmaBuff.VcIc[ch].buff[Current4ADC2_Group];
// 	AdcFromApiDma20ms.VcIc[Adc20msCount].buff[VoltageADC2_Group]=Tempe_ADC_DmaBuff.VcIc[ch].buff[VoltageADC2_Group];

// }	



void	APP_ADC_GET_TxaAvg(void)
{
					TxA_ADC_AdcDmaBuff.Avg.SumT1A=TxA_ADC_AdcDmaBuff.Sum.SumT1A/TxA_ADC_AdcDmaBuff.count;
					TxA_ADC_AdcDmaBuff.Avg.SumT2A=TxA_ADC_AdcDmaBuff.Sum.SumT2A/TxA_ADC_AdcDmaBuff.count;
					TxA_ADC_AdcDmaBuff.Avg.SumT3A=TxA_ADC_AdcDmaBuff.Sum.SumT3A/TxA_ADC_AdcDmaBuff.count;
					TxA_ADC_AdcDmaBuff.Avg.SumT4A=TxA_ADC_AdcDmaBuff.Sum.SumT4A/TxA_ADC_AdcDmaBuff.count;
					TxA_ADC_AdcDmaBuff.count=0;				
					TxA_ADC_AdcDmaBuff.Sum.SumT1A=0;
					TxA_ADC_AdcDmaBuff.Sum.SumT2A=0;					
					TxA_ADC_AdcDmaBuff.Sum.SumT3A=0;					
					TxA_ADC_AdcDmaBuff.Sum.SumT4A=0;		
}

void	APP_ADC_GET_TEMPE(void)
{
#if 0
	AdcFromApiDma20ms.Tempe.buff[Igbt1ADC2_GroupZero]=Tempe_ADC_DmaBuff.TempeAdc2.buff[Igbt1ADC2_GroupZero];
	AdcFromApiDma20ms.Tempe.buff[Igbt2ADC2_GroupZero]=Tempe_ADC_DmaBuff.TempeAdc2.buff[Igbt2ADC2_GroupZero];
	AdcFromApiDma20ms.Tempe.buff[Igbt3ADC2_GroupZero]=Tempe_ADC_DmaBuff.TempeAdc2.buff[Igbt3ADC2_GroupZero];
	AdcFromApiDma20ms.Tempe.buff[Igbt4ADC2_GroupZero]=Tempe_ADC_DmaBuff.TempeAdc2.buff[Igbt4ADC2_GroupZero];


	AdcFromApiDma20ms.Tempe.buff[Bottom1ADC1_GroupZero]=Tempe_ADC_DmaBuff.TempeAdc1.buff[Bottom1ADC1_GroupZero];
	AdcFromApiDma20ms.Tempe.buff[Bottom2ADC1_GroupZero]=Tempe_ADC_DmaBuff.TempeAdc1.buff[Bottom2ADC1_GroupZero];
	AdcFromApiDma20ms.Tempe.buff[Bottom3ADC1_GroupZero]=Tempe_ADC_DmaBuff.TempeAdc1.buff[Bottom3ADC1_GroupZero];
	AdcFromApiDma20ms.Tempe.buff[Bottom4ADC1_GroupZero]=Tempe_ADC_DmaBuff.TempeAdc1.buff[Bottom4ADC1_GroupZero];
#else

	AdcFromApiDma20ms.Tempe.buff[Igbt1ADC2_GroupZero]=API_ADC_GetDRx(ChAdc2_Current,Igbt1ADC2_GroupCh);
	AdcFromApiDma20ms.Tempe.buff[Igbt2ADC2_GroupZero]=API_ADC_GetDRx(ChAdc2_Current,Igbt2ADC2_GroupCh);
	AdcFromApiDma20ms.Tempe.buff[Igbt3ADC2_GroupZero]=API_ADC_GetDRx(ChAdc2_Current,Igbt3ADC2_GroupCh);
	AdcFromApiDma20ms.Tempe.buff[Igbt4ADC2_GroupZero]=API_ADC_GetDRx(ChAdc2_Current,Igbt4ADC2_GroupCh);


	AdcFromApiDma20ms.Tempe.buff[Bottom1ADC1_GroupZero]=API_ADC_GetDRx(ChAdc1_Vc,Bottom1ADC1_GroupCh);
	AdcFromApiDma20ms.Tempe.buff[Bottom2ADC1_GroupZero]=API_ADC_GetDRx(ChAdc1_Vc,Bottom2ADC1_GroupCh);
	AdcFromApiDma20ms.Tempe.buff[Bottom3ADC1_GroupZero]=API_ADC_GetDRx(ChAdc1_Vc,Bottom3ADC1_GroupCh);
	AdcFromApiDma20ms.Tempe.buff[Bottom4ADC1_GroupZero]=API_ADC_GetDRx(ChAdc1_Vc,Bottom4ADC1_GroupCh);
	
#endif	
	
	
}	

void	Adc_ClearCeilQAvg(uint8_t ch)		//重置CEILQ的滤波，防止倍频切换时误判
{
	AdcAverage20ms[AdcGroupCeilQ1+ch]=0;
}

#include	"API_I2C.H"
void	APP_ZERO_Adc100usCallBack(void)
//void	 	APP_ADC_Adc20msCountADD(void)
{

				API_I2C_CheckBuffMax();
				//				API_ADC_GET100us_Buff(&AdcFromApiDma100us);
				if(Adc20msCount<AdcAvageCount)			//第199次切换VC与TEMPE
				{	
		
//					APP_ADC_GET_VcIcBuff();//得到VCIC的当前数据	
					
					// API_ADC_VcIc_ENABLE();	//開啟VCIC ADC 等待 HRTIM CMP2觸發
					Tempe_ADC_DmaBuff.Vc[Adc20msCount]=API_ADC_GetDRx(ChAdc1_Vc,VoltageADC1_Group);
					
//					if(Adc20msCount== 70)
//					{
//						APP_POWER_CycleChange();	

//					}
//					if(Adc20msCount==99)
//					{
//						APP_POWER_CycleReset();	
//					}

					Adc20msCount++;	

				}

				if(	Adc20msCount==AdcAvageCount)
				{	
//					API_GPIO_WritePin(DebugB_pin,1);
						TxA_ADC_AdcDmaBuff.step=TXA_StepStart;
						AdcFromApiDma20ms.num=1;
						Adc20msCount++;


					
						APP_ADC_GET_TxaAvg();				//得到功率平均值
			
//						API_TIM_100US_STOP();
//						API_HRTIM_DISABLE_IT_UPD();
					

					
						AdcSelect=ADC_Select_Tempe;
						APP_ADC_SELECT_GROUP(AdcSelect);	
						API_ADC_T12A_ENABLE_IT_EOC();
						API_TIM_100US_RESET();									//更新计数器马上触发	ADC
//					if(AdcSelect==ADC_Select_VcIc)
//					{

//				API_ADC_DMA_ONOFF(0);
//				API_ADC_VcIc_DISABLE_IT_EOC();
//				API_HRTIM_DISABLE_IT_REST();	

//					}
					APP_ADC_AVG_Fun();	//计算求和

				}
			}				





//void	API_ADC_DMA_M2M_IRQHandlerCallBack(void)

//---------M2M数据处理函数----START-------------------------------







//******************************************************************
// 函数名	：void	APP_ADC_GetFanAd_AdcValue(uint32_t* adc_buff,	uint8_t	ch)				
// 作者		：
// 功能		：//从M2M缓存中获取ADC数据转换到(VOLTAGE, FAN_AD,CURRENT2~4 IGBT2)
// 参数		：adc_buff  M2M缓存   ch 读取的数据长度
// 返回值	：
// 调用全局变量:				
// 修改全局变量:			
// 备注：主要是得到FAN_AD  它需要切换OP1P2 ADC2
//
//*****************************************************************


void	APP_ADC_GetFanAd_AdcValue(uint32_t* adc_buff,	uint8_t	ch)
{
//		uint16_t  t_adc;
//					t_adc=  adc_buff[Current1ADC2_Group];

//					uhADCxConvertedData[AdcGroupCurrent1]=t_adc;
//					Adc20msSum[AdcGroupCurrent1]+=t_adc;
}

//******************************************************************
// 函数名	：void	APP_ADC_GetTxAadcValue(uint32_t* adc_buff,	uint8_t	ch)				
// 作者		：
// 功能		：//从M2M缓存中获取ADC数据转换到(T1~4A 谐振电流)
// 参数		：
// 返回值	：
// 调用全局变量:				
// 修改全局变量:			
// 备注：T1、2 和T3\4分别一组， ADC1 ADC3
//
//*****************************************************************


void	APP_ADC_GetTxAadcValue(uint32_t* adc_buff,	uint8_t	ch)
{	
//			if(adcCount<100)
//			{

//						buff[adcCount]= adc_buff[0];
//						buff1[adcCount]= adc_buff[1];
//						adcCount++;
//							
//			
//						buff[adcCount]= adc_buff[2];
//						buff1[adcCount]= adc_buff[3];
//						adcCount++;		
//						buff[adcCount]= adc_buff[4];
//						buff1[adcCount]= adc_buff[5];
//			
//						adcCount++;
//						buff[adcCount]= adc_buff[6];
//						buff1[adcCount]= adc_buff[7];
//						adcCount++;

//			}
//			else
//			{
//				adcCount=0;

////				ADC_Disable_IT(ADC1,ADC_CR1_EOCIE);
//			}	
}
//******************************************************************
// 函数名	：void	APP_ADC_GeTempeAdcValue(uint32_t* adc_buff,	uint8_t	ch)				
// 作者		：
// 功能		：//从M2M缓存中获取ADC数据转换到(BOTTOM1~4 IGBT1)
// 参数		：
// 返回值	：
// 调用全局变量:				
// 修改全局变量:			
// 备注：ADC1 得到BOTTOM1~4 IGBT1  ADC3 还是T3`4
//
//*****************************************************************
void	APP_ADC_GeTempeAdcValue(uint32_t* adc_buff,	uint8_t	ch)
{




}

//---------M2M数据处理函数----START-------------------------------



//谐振电流采集一个HRTIM周期
//---------中断回调函数----START-------------------------------

//******************************************************************
//			step 1
// 函数名	：void	APP_ADC_ZERO_IrqFun(void)				
// 作者		：
// 功能		：//过零号的CMP1中断函数，
// 参数		：
// 返回值	：
// 调用全局变量:				
// 修改全局变量:			
// 备注：		1\	Adc20msCount=0;
//				2\	AdcSelect=ADC_Select_Tempe;;		//选择测试TEMPE
//				3\	API_ADC_DMA_ONOFF(0);DMA 关闭
//				4\	API_ADC_T12A_ENABLE_IT_EOC 开ADC1 EOC

//*****************************************************************




void	APP_ADC_ZERO_IrqFun(void)
{	

//	AdcSelect=ADC_Select_VcIc;			//恢复VCIC
	

//	API_HRTIM_DISABLE_IT_UPD();
	

	AdcFromApiDma20ms.count=0;
	Adc20msCount=0;	
	

	AdcSelect=ADC_Select_VcIc;
	
// 	if(APP_ADC_Power_ZeroIrqFun())			//是否是POT2
// 	{
// //			AdcSelect=ADC_Select_VcT2A;
// 	}	
	
	APP_ADC_SELECT_GROUP(AdcSelect);	
	APP_POWER_SetTxaAwdValue();	
//	API_GPIO_WritePin(DebugA_pin,1);		
//	API_GPIO_WritePin(DebugA_pin,0);	
	

	
	
//	API_ADC_VcIc_ENABLE_IT_EOC();
}
//******************************************************************
//			step 2
// 函数名	：void	API_T12_EOC_IRQHandlerCallBack(void)				
// 作者		：
// 功能		：//ADC1.c的EOC中断函数，
// 参数		：
// 返回值	：
// 调用全局变量:				
// 修改全局变量:			
// 备注：		1\关闭ADC1 EOC中断
//				2\得到ADC1 TEMPE  ADC2 FANAD 的ADC数据
//				3\切到ADC_Select_VcIc, 
//				4\开启DMA
//				5\开启ADC2 EOC
//*****************************************************************

void	API_T12_EOC_IRQHandlerCallBack(void)
{
	
					
			API_ADC_T12A_DISABLE_IT_EOC();
			APP_ADC_GET_TEMPE();						


			API_HRTIM_BASE_ENABLE_IT_CMP();				//开启HRTIM UPD中断 准备读取谐振电流


}







//******************************************************************
//			step 1
// 函数名	：void	API_VcIc_EOC_IRQHandlerCallBack(void)					
// 作者		：
// 功能		：//API_ADC.c的ADC2 EOC中断函数，
// 参数		：
// 返回值	：
// 调用全局变量:				
// 修改全局变量:			
// 备注：		1\ADC2 规则通道转换完成中断，数据保存在uint32_t VcIc_ADC_Buff[VcIc_ADC_GROUP_NUM];
//				2\API_ADC_DMA_TxaM2M_ONOFF(0);	恢复DMA谐振电流采集	M2M
//				2\开启HRTIM_UPD中断，准备时行谐振电流同步采样
//*****************************************************************
void	API_VcIc_EOC_IRQHandlerCallBack(void)	
{

}	

//******************************************************************
//			step 2
// 函数名	：void	API_HRTIM1_TEST_UPD_IRQHandlerCallback(void)				
// 作者		：
// 功能		：//API_HRTIM.c的UPD中断函数，
// 参数		：
// 返回值	：
// 调用全局变量:				
// 修改全局变量:			
// 备注：		1\打开  ADC3 EOC	准备记录DMA传送位置
//				2\打开	CMP1中断，准备结束谐振电流值采集	
//				2\关闭UPD IT   
//*****************************************************************
static 	uint8_t  count=0;

void	API_HRTIM1_TEST_UPD_IRQHandlerCallback(uint8_t sourse)		//HRTIM的UPD中断回调，PPG增减操作
{

	APP_ADC_IRQ_PPGstepChangeCallBack();

}

//******************************************************************
//			step 3
// 函数名	：void	API_T34_EOC_IRQHandlerCallBack(void)				
// 作者		：
// 功能		：//API_ADC.c的ADC3 EOC中断函数，
// 参数		：
// 返回值	：
// 调用全局变量:				
// 修改全局变量:			
// 备注：		1\记录DMA转换位置，供后面取数
//				2\关闭T34 EOC中断
//				
//*****************************************************************

void	API_T34_EOC_IRQHandlerCallBack(void)			//T34的谐振电流回调 将ADC DMA缓存，M2M到计算缓存
{
		
//		API_ADC_DMA_ONOFF(1);							//得到DMA传输位置 
//		API_ADC_T34A_DISABLE_IT_EOC();
	
}	
#define		APP_ADC_TxaMax		0xa00				//谐振电流保护值 
/* Pairs L/M/N (ISR callbacks) moved to app_power.c */

//---------中断回调函数-END----------------------------------




//******************************************************************
// 函数名	：APP_ADC_SELECT_GROUP		
// 作者		：
// 功能		：//切换ADC规则队列，OPAMP1 P1P2，
// 参数		：ADC_Select_VcIc 0 	ADC_Select_Tempe 1  
// 返回值	：
// 调用全局变量:				
// 修改全局变量:			
// 备注：	1\：ADC_Select_VcIc  采集VCIC  OP1P1
//				2\ADC_Select_Tempe  采集 BOTTOM OP1P2
//*****************************************************************
#include	"API_OPAMP.H"

void		APP_ADC_SELECT_GROUP(uint8_t ch)
{	
//	API_TIM_100US_STOP();
	
				API_GPIO_WritePin(DebugA_pin,1);
	API_TIM_TXA_STOP();				//暂停ADC触发
	API_OPAMP1_Select(ch);
	API_ADC_SELECT(ch);
	API_TIM_TXA_RESET();			//开启ADC触发
	APP_POWER_SetTxaAwdValue();	
				API_GPIO_WritePin(DebugA_pin,0);
}
enum
{
		SW_ON=0,
		SW_OFF,
};	


void		APP_ADC_PanSwChange(uint32_t ch)
{
		uint32_t pinCh=PANSW1_pin+ch;

//		API_GPIO_WritePin(PAN_pin,0);		//输出0清除状态	
//		API_GPIO_Mode(PAN_pin ,MODER_Output);



		API_GPIO_WritePin(PANSW1_pin,SW_OFF);		//输出0清除状态	
		API_GPIO_WritePin(PANSW2_pin,SW_OFF);		//输出0清除状态	
		API_GPIO_WritePin(PANSW3_pin,SW_OFF);		//输出0清除状态		
		API_GPIO_WritePin(PANSW4_pin,SW_OFF);		//输出0清除状态		
	
		API_GPIO_Mode(PANSW1_pin ,MODER_Output);
		API_GPIO_Mode(PANSW2_pin ,MODER_Output);
		API_GPIO_Mode(PANSW3_pin ,MODER_Output);
		API_GPIO_Mode(PANSW4_pin ,MODER_Output);
	

	#ifdef	DEBUG_POWER_OUT				//PANSW4不清0 TIM1_CH4 用于FALULT消隐
	
		API_GPIO_Mode(PANSW4_pin ,MODER_Output);	//同时是TIM1_CH4	
		API_GPIO_WritePin(PANSW4_pin,0);		//输出0清除状态		
	#endif

		if(ch<10)						
		{	
			API_GPIO_WritePin(pinCh,SW_ON);
//			API_GPIO_Mode(pinCh ,MODER_Input);
//			API_GPIO_PinPull(pinCh,PUPDR_Pulldown);			//下拉电阻
		
			// API_GPIO_BaseInit(PAN_pin);			//PAN切换回初始ADC状态
		}
}	






void	API_DMA_TxA_IRQHandlerCallBack(uint8_t	num)
{



		if(TxA_ADC_AdcDmaBuff.step==TXA_StepDmaStart)		//保存DMA数据
		{
			volatile	API_DMA_RecoverDef		recover;	
		API_GPIO_WritePin(DebugA_pin,1);	

			recover.DataLength=Current_ADC_AdcDMA_BUFF_NUM/4;
			
			recover.SrcAddress=(uint32_t)&TxA_ADC_AdcDmaBuff.CurrentAdc2[0];		//保存完整的150us数据，前面几个数据可能无效
			recover.DstAddress=(uint32_t)&TxA_ADC_AdcDmaBuff.CurrentAdc2Save;
			API_DMA_RECOVER(ChDmaM2M, (API_DMA_RecoverDef*)(&recover));

			for(uint16_t i=0;i<TxA_ADC_AdcBUFF_NUM;i++)			//手动保存数据
			{
					TxaBuffCh1[i]=TxA_ADC_AdcDmaBuff.CurrentAdc2[0][i].buff[0];
			}
			
			
			
			recover.SrcAddress=(uint32_t)&TxA_ADC_AdcDmaBuff.CurrentAdc3[0];		//保存完整的150us数据，前面几个数据可能无效
			recover.DstAddress=(uint32_t)&TxA_ADC_AdcDmaBuff.CurrentAdc3Save;
			API_DMA_RECOVER(ChDmaM2M, (API_DMA_RecoverDef*)(&recover));
				
//		recover.DataLength=TxA_ADC_AdcBUFF_NUM/2;
//		for(uint8_t i=0;i<PotNum;i++)
//		{
//			recover.SrcAddress=(uint32_t)&TxA_ADC_AdcDmaBuff.HrtimPotch[i][num];		//保存完整的150us数据，前面几个数据可能无效
//			recover.DstAddress=(uint32_t)&TxA_ADC_AdcDmaBuff.HrtimSave[i];
//			API_DMA_RECOVER(ChDmaM2M, (API_DMA_RecoverDef*)(&recover));
//		}

#if DMA_HRTIM		

		uint16_t 	div1=TxA_ADC_AdcDmaBuff.CurrentAdc2Save[1].buff[0]-TxA_ADC_AdcDmaBuff.HrtimSave[0][1];
		uint16_t 	div2=TxA_ADC_AdcDmaBuff.CurrentAdc2Save[1].buff[1]-TxA_ADC_AdcDmaBuff.HrtimSave[1][1];		//CC1,3会在ADC前读一个，所以要去掉一个
#endif

			API_GPIO_WritePin(DebugA_pin,0);	
			TxA_ADC_AdcDmaBuff.step=TXA_StepDmaEnd;
		}




}	

#if 0

__attribute__((section(".RAMCODE")))void 	APP_ADC_AvgTxA20us1(void)
{
	
	
	
	volatile 	uint32_t * buffT1A;	
	volatile	uint32_t * buffT2A;
	volatile 	uint32_t * buffT3A;	
	volatile	uint32_t * buffT4A;	
	volatile	uint32_t 	sum1=0;
	volatile	uint32_t 	sum2=0;
	volatile	uint32_t 	sum3=0;
	volatile	uint32_t 	sum4=0;
	uint32_t    ch12,ch34;

	uint8_t num=0;
	

				buffT1A=&(TxA_ADC_TimDmaBuff.T1A[num]);
				buffT3A=&(TxA_ADC_TimDmaBuff.T3A[num]);
				buffT2A=&(TxA_ADC_TimDmaBuff.T2A[num]);
				buffT4A=&(TxA_ADC_TimDmaBuff.T4A[num]);
				
//				ch12=(buffT1A[0]<<16)+buffT2A[0];
//				sum1+=ch12;
//				ch12=(buffT1A[1]<<16)+buffT2A[1];
//				sum1+=ch12;				
//				ch12=(buffT1A[2]<<16)+buffT2A[2];
//				sum1+=ch12;
//				ch12=(buffT1A[3]<<16)+buffT2A[3];
//				sum1+=ch12;				
//				ch12=(buffT1A[4]<<16)+buffT2A[4];
//				sum1+=ch12;
//				ch12=(buffT1A[5]<<16)+buffT2A[5];
//				sum1+=ch12;				
//				ch12=(buffT1A[6]<<16)+buffT2A[6];
//				sum1+=ch12;
//				ch12=(buffT1A[7]<<16)+buffT2A[7];
//				sum1+=ch12;	



//				ch34=(buffT3A[0]<<16)+buffT4A[0];				
//				sum2+=ch34;
//				
//				ch34=(buffT3A[1]<<16)+buffT4A[1];				
//				sum2+=ch34;				
//				
//				ch34=(buffT3A[2]<<16)+buffT4A[2];				
//				sum2+=ch34;		

//				ch34=(buffT3A[3]<<16)+buffT4A[3];				
//				sum2+=ch34;				
//				
//				ch34=(buffT3A[4]<<16)+buffT4A[4];				
//				sum2+=ch34;	
//				
//				ch34=(buffT3A[5]<<16)+buffT4A[5];				
//				sum2+=ch34;				
//				
//				ch34=(buffT3A[6]<<16)+buffT4A[6];				
//				sum2+=ch34;	
//				ch34=(buffT3A[7]<<16)+buffT4A[7];				
//				sum2+=ch34;					
				

				sum1=buffT1A[0]+buffT1A[1]+buffT1A[2]+buffT1A[3]+buffT1A[4]+buffT1A[5]+buffT1A[6]+buffT1A[7];
				sum2=buffT2A[0]+buffT2A[1]+buffT2A[2]+buffT2A[3]+buffT2A[4]+buffT2A[5]+buffT2A[6]+buffT2A[7];
				sum3=buffT3A[0]+buffT3A[1]+buffT3A[2]+buffT3A[3]+buffT3A[4]+buffT3A[5]+buffT3A[6]+buffT3A[7];
				sum4=buffT4A[0]+buffT4A[1]+buffT4A[2]+buffT4A[3]+buffT4A[4]+buffT4A[5]+buffT4A[6]+buffT4A[7];


}
				


//__attribute__((used, optimize("unroll-loops", "O3"))) 
__attribute__((section(".RAMCODE")))void 	APP_ADC_AvgTxA20us1(void)
{
    // 使用静态索引实现块循环
    static uint8_t block_idx = 0;
    
    // 直接指针访问避免多次计算偏移
    volatile register uint32_t *buffT12A = &TxA_ADC_AdcDmaBuff.T12A[block_idx << 3]; // *8
    volatile register uint32_t *buffT34A = &TxA_ADC_AdcDmaBuff.T34A[block_idx << 3];

    
    // 寄存器变量加速计算 (强制使用CPU寄存器)
    volatile register uint32_t sumT12A  = 0;
    volatile register uint32_t sumT34A  = 0;

    
    // 并行展开计算 (无依赖链)
	
	sumT12A+=buffT12A[0];sumT34A+=buffT34A[0];
	sumT12A+=buffT12A[1];sumT34A+=buffT34A[1];
	sumT12A+=buffT12A[2];sumT34A+=buffT34A[2];
	sumT12A+=buffT12A[3];sumT34A+=buffT34A[3];
	sumT12A+=buffT12A[4];sumT34A+=buffT34A[4];
	sumT12A+=buffT12A[5];sumT34A+=buffT34A[5];
	sumT12A+=buffT12A[6];sumT34A+=buffT34A[6];
	sumT12A+=buffT12A[7];sumT34A+=buffT34A[7];	
	sumT12A+=buffT12A[8];sumT34A+=buffT34A[8];
	sumT12A+=buffT12A[9];sumT34A+=buffT34A[9];
	sumT12A+=buffT12A[10];sumT34A+=buffT34A[10];
	sumT12A+=buffT12A[11];sumT34A+=buffT34A[11];
	sumT12A+=buffT12A[12];sumT34A+=buffT34A[12];
	sumT12A+=buffT12A[13];sumT34A+=buffT34A[13];
	sumT12A+=buffT12A[14];sumT34A+=buffT34A[14];
	sumT12A+=buffT12A[15];sumT34A+=buffT34A[15];

	

	


//    // 存储结果 (确保内存对齐)
//    TxA_ADC_TimDmaBuff.sumT1A[block_idx] = sum1;
//    TxA_ADC_TimDmaBuff.sumT2A[block_idx] = sum2;
//    TxA_ADC_TimDmaBuff.sumT3A[block_idx] = sum3;
//    TxA_ADC_TimDmaBuff.sumT4A[block_idx] = sum4;
//    
//    // 循环索引更新 (无分支)
//    block_idx = (block_idx + 1) & 0x03; // MOD 4

}

#endif




#if 0
__attribute__((section(".RAMCODE")))void	APP_ADC_AvgTxA20usMax(void)
{
	uint32_t * buffT12A;	
	uint32_t * buffT34A;
	uint8_t count=TxA_ADC_TimBUFF_NUM/2;
//	uint8_t num=AdcFromApiDma100us.end>>3;
//	uint8_t	end=AdcFromApiDma100us.start>>3;
//	
//	AdcFromApiDma100us.end=AdcFromApiDma100us.start;
	

	
//	if(start<=end)		//没有翻转
//	{
//		count=(end-start)/2+1;	//从end 往后数到一半			
//	}
//	else
//	{
//		count=(TxA_ADC_BUFF_NUM-start+end)/2;
//	}	
	
	do
	{	
				buffT12A=(uint32_t *)&(TxA_ADC_AdcDmaBuff.T12A[num].buff[0]);
				buffT34A=(uint32_t *)&(TxA_ADC_AdcDmaBuff.T34A[num].buff[0]);
								
				
//				for(uint8_t j=0;j<4;j++)
//				{

					Adc20msSum[AdcGroupT1A]+=*(buffT12A);	
							Adc20msSum[AdcGroupT2A]+=*(buffT12A+1);
					Adc20msSum[AdcGroupT1A]+=*(buffT12A+2);	
							Adc20msSum[AdcGroupT2A]+=*(buffT12A+3);
					Adc20msSum[AdcGroupT1A]+=*(buffT12A+4);	
							Adc20msSum[AdcGroupT2A]+=*(buffT12A+5);
					Adc20msSum[AdcGroupT1A]+=*(buffT12A+6);					
							Adc20msSum[AdcGroupT2A]+=*(buffT12A+7);
		

//					Adc20msSum[AdcGroupT2A]+=*(buffT12A+3);	
//					Adc20msSum[AdcGroupT2A]+=*(buffT12A+5);
//					Adc20msSum[AdcGroupT2A]+=*(buffT12A+7);	
					
					
					Adc20msSum[AdcGroupT3A]+=*(buffT34A);	
										Adc20msSum[AdcGroupT4A]+=*(buffT34A+1);	
					Adc20msSum[AdcGroupT3A]+=*(buffT34A+2);
								Adc20msSum[AdcGroupT4A]+=*(buffT34A+3);				
					Adc20msSum[AdcGroupT3A]+=*(buffT34A+4);	
							Adc20msSum[AdcGroupT4A]+=*(buffT34A+5);				
					Adc20msSum[AdcGroupT3A]+=*(buffT34A+6);	
						Adc20msSum[AdcGroupT4A]+=*(buffT34A+7);	
					
					
			

					
//				}
				
						

			
			num++;	
			if(num>TxA_ADC_TimBUFF_NUM)
			{
				num=0;							//循环队列复位
			}	
			
			

	}while(num!=end);							//循环到起始点





}
__attribute__((section(".RAMCODE"))) void	APP_ADC_AvgTxA20us_Max(void)
{

	

	


	uint32_t * buffT12A;	
	uint32_t * buffT34A;	

	uint32_t 	maxT1A=0;
	uint32_t 	maxT2A=0;	
	uint32_t 	maxT3A=0;
	uint32_t 	maxT4A=0;		
	uint32_t	buffSave;
	uint8_t count=TxA_ADC_TimBUFF_NUM/2;
	uint8_t num=AdcFromApiDma100us.start;
		

	
	

#if 1	

	do
	{		
			
		
				buffT12A=(uint32_t *)&(TxA_ADC_AdcDmaBuff.T12A[num].buff[0]);
				buffT34A=(uint32_t *)&(TxA_ADC_AdcDmaBuff.T34A[num].buff[0]);
				

		
		
					buffSave=*(buffT12A+0);	
					Adc20msSum[AdcGroupT1A]+=buffSave;	
					if(maxT1A<buffSave)
					{
						maxT1A=buffSave;
					}
					buffSave=*(buffT12A+2);	
					Adc20msSum[AdcGroupT1A]+=buffSave;	
					if(maxT1A<buffSave)
					{
						maxT1A=buffSave;
					}
					buffSave=*(buffT12A+4);	
					Adc20msSum[AdcGroupT1A]+=buffSave;	
					if(maxT1A<buffSave)
					{
						maxT1A=buffSave;
					}
					buffSave=*(buffT12A+6);	
					Adc20msSum[AdcGroupT1A]+=buffSave;	
					if(maxT1A<buffSave)
					{
						maxT1A=buffSave;
					}
				

					buffSave=*(buffT12A+1);	
					Adc20msSum[AdcGroupT2A]+=buffSave;	
					if(maxT2A<buffSave)
					{
						maxT2A=buffSave;
					}
					buffSave=*(buffT12A+3);	
					Adc20msSum[AdcGroupT2A]+=buffSave;	
					if(maxT2A<buffSave)
					{
						maxT2A=buffSave;
					}
					buffSave=*(buffT12A+5);	
					Adc20msSum[AdcGroupT2A]+=buffSave;	
					if(maxT2A<buffSave)
					{
						maxT2A=buffSave;
					}
					buffSave=*(buffT12A+7);	
					Adc20msSum[AdcGroupT2A]+=buffSave;	
					if(maxT2A<buffSave)
					{
						maxT2A=buffSave;
					}




					
					
					buffSave=*(buffT34A+0);	
					Adc20msSum[AdcGroupT3A]+=buffSave;	
					if(maxT3A<buffSave)
					{
						maxT3A=buffSave;
					}
					
					buffSave=*(buffT34A+2);	
					Adc20msSum[AdcGroupT3A]+=buffSave;	
					if(maxT3A<buffSave)
					{
						maxT3A=buffSave;
					}
										
					buffSave=*(buffT34A+4);	
					Adc20msSum[AdcGroupT3A]+=buffSave;	
					if(maxT3A<buffSave)
					{
						maxT3A=buffSave;
					}
						
					buffSave=*(buffT34A+6);	
					Adc20msSum[AdcGroupT3A]+=buffSave;	
					if(maxT3A<buffSave)
					{
						maxT3A=buffSave;
					}
						
					
					buffSave=*(buffT34A+1);	
					Adc20msSum[AdcGroupT4A]+=buffSave;	
					if(maxT4A<buffSave)
					{
						maxT4A=buffSave;
					}
					
					buffSave=*(buffT34A+3);	
					Adc20msSum[AdcGroupT4A]+=buffSave;	
					if(maxT4A<buffSave)
					{
						maxT4A=buffSave;
					}
										
					buffSave=*(buffT34A+5);	
					Adc20msSum[AdcGroupT4A]+=buffSave;	
					if(maxT4A<buffSave)
					{
						maxT4A=buffSave;
					}
						
					buffSave=*(buffT34A+7);	
					Adc20msSum[AdcGroupT4A]+=buffSave;	
					if(maxT4A<buffSave)
					{
						maxT4A=buffSave;
					}
		
				

			
			num++;	
//			if(num>TxA_ADC_BUFF_NUM)
//			{
//				num=0;							//循环队列复位
//			}	
//			
			

	}while(count--);							//循环到起始点

	


#endif

}	
#endif

#if 0

void		getADCmax(void)
{

	uint32_t	adcCount=10;//TxA_ADC_BUFF_NUM;
	uint32_t 	maxT1A=0;
	uint32_t 	maxT2A=0;	
	uint32_t 	maxT3A=0;
	uint32_t 	maxT4A=0;		

	
	uint32_t	start,end,count;

	uint32_t * buffT12A;	
	uint32_t * buffT34A;	

//	start=(TxA_ADC_DMA_BUFF_NUM-AdcFromApiDma100us.hrtim_start)>>3;	
//	end=(TxA_ADC_TimDMA_BUFF_NUM-AdcFromApiDma100us.hrtim_end)>>3;
	
//	if(start<=end)		//没有翻转
//	{
//		count=(end-start)/2+1;	//从end 往后数到一半			
//	}
//	else
//	{
//		count=(TxA_ADC_BUFF_NUM-start+end)/2;
//	}	
	count=5;
	uint32_t num=end;					//TxA_ 组号，每组8个数据			
	uint32_t buffSave;



		do	
		{
				
				buffT12A=(uint32_t *)&(TxA_ADC_AdcDmaBuff.T12A[num]);
				buffT34A=(uint32_t *)&(TxA_ADC_AdcDmaBuff.T34A[num]);
				
				for(uint8_t j=0;j<4;j++)
				{
					buffSave=	buffT12A[T1A_0_ADC1Group+j*2];
					if(maxT1A<buffSave)
					{
						maxT1A=buffSave;
					}			
				
				
					buffSave=	buffT12A[T2A_0_ADC1Group+j*2];			
					if(maxT2A<buffSave)
					{
						maxT2A=buffSave;
					}	
				
					buffSave=	buffT34A[T3A_0_ADC3Group+j*2];
					if(maxT3A<buffSave)
					{
						maxT3A=buffSave;
					}		
				
					buffSave=	buffT34A[T4A_0_ADC3Group+j*2];
					if(maxT4A<buffSave)
					{
						maxT4A=buffSave;
					}				

				
						
			}



			
			if(num==0)
			{
				num=TxA_ADC_TimBUFF_NUM;
			}else
			{
				num--;		
			}		
			
		
		}while(count--);
		
			uhADCxConvertedData[AdcGroupT1A]=maxT1A;
			uhADCxConvertedData[AdcGroupT2A]=maxT2A;
			uhADCxConvertedData[AdcGroupT3A]=maxT3A;
			uhADCxConvertedData[AdcGroupT4A]=maxT4A;		
		
		
#endif			
	


		
		
	//将数据发送到串口	
#if	0
//#ifdef	DebugOutPc		

		
static 	uint32_t  delayCount;		
		
		
		delayCount++;
		
		
		
			#include	"simulative_uart.h"
	if(delayCount>5000)								//串口有没有发送完
	{	
		
		delayCount=0;
		
		setDebugOutBuff(adcCount,0);
		setDebugOutBuff(uhADCxConvertedData[AdcGroupVoltage],1);
		for(uint8_t i=2;i<DEBUG_OUT_CH_NUM;i++)
		{
			setDebugOutBuff(buff[(i-2)*2],i);
		}	
	 
		UARTx_SendValueClass(Debug_OutToPc());
	}
		



}	
#endif		
/* getADCinputValue — 已删除, APP_ADC 数据通过 Pair O PUSH 传递 */


		




#if 0
void	API_ADC_DMA_RecoverPan(uint8_t ch)				//恢复检锅DMA
{

	//每1ms后，等待T1A DMA中斷觸發后，開始保存HRTIM位置，保證能訊錄到一個完整的HRTIM周期
		volatile	API_DMA_RecoverDef		recover;
	
		switch(ch)			//不同通道用不同的检锅源
		{
			case PotCh1:
					recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc2_Current,Current1ADC2_Group);				
					break;			
			case PotCh2:	
					recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc2_Current,Current2ADC2_Group);				
					break;
			case	PotCh3:
					recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc2_Current,Current3ADC2_Group);				
					break;		
			case	PotCh4:
					recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc2_Current,Current4ADC2_Group);				
					break;	

		}	

		
		
		recover.DstAddress=(uint32_t)Pan_ADC_AdcDmaBuff;		//+COEFF_VECTOR_B_SIZE;
		recover.DataLength=Pan_ADC_DMA_BUFF_NUM;
			
		API_DMA_RECOVER(ChDmaPan, (API_DMA_RecoverDef*)(&recover));


}
#else	
	
void	APP_ADC_DMA_RecoverPan(uint8_t ch)				//恢复检锅DMA
{


		volatile	API_DMA_RecoverDef		recover;
	
		recover.SrcAddress=	API_ADC_GetAddressDRx(ChAdc1_Vc,PanADC1_ADC1Group);	
		recover.DstAddress=(uint32_t)APP_POWER_GetPanDmaBuffAddress();		//+COEFF_VECTOR_B_SIZE;
		recover.DataLength=Pan_ADC_DMA_BUFF_NUM;
			
		API_DMA_RECOVER(ChDmaPan, (API_DMA_RecoverDef*)(&recover));
}
#endif
	

uint8_t 	Adc_IsTxaDmaStart(void)			//DMA TXA时不调整PPG
{
	if(TxA_ADC_AdcDmaBuff.step==TXA_StepDmaStart)
	{
				return 1;
	}
	
	return 0;
}


void	API_ADC_DMA_TimStart(void)
{
		if(TxA_ADC_AdcDmaBuff.step==TXA_StepStart)//TXA DMA恢复后执行
		{	

//								API_GPIO_WritePin(DebugA_pin,1);
								//API_TIM_TXA_RESET();//开始1us定时器
								TxA_ADC_AdcDmaBuff.step=TXA_StepDmaRest;
//								API_GPIO_WritePin(DebugA_pin,0);
			

	

			if(TxA_ADC_AdcDmaBuff.step==TXA_StepDmaRest)
			{
				TxA_ADC_AdcDmaBuff.step=TXA_StepDmaStart;	
								API_GPIO_WritePin(DebugA_pin,1);
				APP_ADC_DMA_RecoverHrtim();		//开启新的一组HRTIM point
								API_GPIO_WritePin(DebugA_pin,0);
			}

			
		}	

}	


void	API_ADC_DMA_RecoverTxa(void)				//恢复检锅DMA
{

}


//void	API_ADC_DMA_RecoverCallBack(ADC_SELECT_ENUM ch)
void	API_ADC_DMA_RecoverFun(ADC_SELECT_ENUM ch)
{	
		volatile	API_DMA_RecoverDef		recover;
	


		if(ch==ADC_Select_Tempe)
		{	

			recover.DataLength=0;
			API_DMA_RECOVER(ChDmaVcAdc1, (API_DMA_RecoverDef*)(&recover));	
			API_DMA_RECOVER(ChDmaCurrentAdc2, (API_DMA_RecoverDef*)(&recover));	
			API_DMA_RECOVER(ChDmaCurrentAdc3, (API_DMA_RecoverDef*)(&recover));	
			API_DMA_RECOVER(ChDmaHrtimPotCh1, (API_DMA_RecoverDef*)(&recover));	
			API_DMA_RECOVER(ChDmaHrtimPotCh2, (API_DMA_RecoverDef*)(&recover));	
			API_DMA_RECOVER(ChDmaHrtimPotCh3, (API_DMA_RecoverDef*)(&recover));	
			API_DMA_RECOVER(ChDmaHrtimPotCh4, (API_DMA_RecoverDef*)(&recover));	
																

		}		
		else
		{
//-------------------DMA 获取ADC2同步的T12值--用于谐振电流过流检测--------------------------------------------------------------------





			recover.DataLength=Current_ADC_AdcDMA_BUFF_NUM;	//双缓存 双数组
#if		DMA_HRTIM	
			recover.SrcAddress=API_HRTIM_GetAddressTxaCnt(PotCh1);		//	
#else
			recover.SrcAddress=API_ADC_GetAddressDR(ChAdc2_Current);
#endif			


			recover.DstAddress=(uint32_t)TxA_ADC_AdcDmaBuff.CurrentAdc2;
	
			API_DMA_RECOVER(ChDmaCurrentAdc2, (API_DMA_RecoverDef*)(&recover));

#if		DMA_HRTIM	
			recover.SrcAddress=API_HRTIM_GetAddressTxaCnt(PotCh1);		//	
#else
			recover.SrcAddress=API_ADC_GetAddressDR(ChAdc3_Current);
#endif							
			recover.DstAddress=(uint32_t)TxA_ADC_AdcDmaBuff.CurrentAdc3;

			
			API_DMA_RECOVER(ChDmaCurrentAdc3, (API_DMA_RecoverDef*)(&recover));

			recover.DataLength=TxA_ADC_AdcBUFF_NUM;		//单缓存，单数组
			recover.SrcAddress=API_HRTIM_GetAddressTxaCnt(PotCh1);				
			recover.DstAddress=(uint32_t)TxA_ADC_AdcDmaBuff.HrtimSave[PotCh1];
			API_DMA_RECOVER(ChDmaHrtimPotCh1, (API_DMA_RecoverDef*)(&recover));
			recover.SrcAddress=API_HRTIM_GetAddressTxaCnt(PotCh2);				
			recover.DstAddress=(uint32_t)TxA_ADC_AdcDmaBuff.HrtimSave[PotCh2];
			API_DMA_RECOVER(ChDmaHrtimPotCh2, (API_DMA_RecoverDef*)(&recover));
			
			recover.SrcAddress=API_HRTIM_GetAddressTxaCnt(PotCh3);				
			recover.DstAddress=(uint32_t)TxA_ADC_AdcDmaBuff.HrtimSave[PotCh3];
			API_DMA_RECOVER(ChDmaHrtimPotCh3, (API_DMA_RecoverDef*)(&recover));
			
			recover.SrcAddress=API_HRTIM_GetAddressTxaCnt(PotCh4);				
			recover.DstAddress=(uint32_t)TxA_ADC_AdcDmaBuff.HrtimSave[PotCh4];
			API_DMA_RECOVER(ChDmaHrtimPotCh4, (API_DMA_RecoverDef*)(&recover));
			
			recover.DataLength=TxA_ADC_AdcBUFF_NUM*2;		//单缓存			双数组
			recover.SrcAddress=	API_ADC_GetAddressDRx(ChAdc1_Vc,VoltageADC1_Group);	
			// recover.SrcAddress=API_HRTIM_GetAddressTxaCnt(PotCh1);		//	
			recover.DstAddress=(uint32_t)TxA_ADC_AdcDmaBuff.VcAdc1;

			
			API_DMA_RECOVER(ChDmaVcAdc1, (API_DMA_RecoverDef*)(&recover));			

		}		
		
		


	}


	
void	APP_ADC_DMA_RecoverHrtim(void)				//恢复检锅DMA
{

		API_ADC_StopAllAdc();
		API_TIM_TXA_STOP();				//暂停ADC触发
//		API_ADC_DMA_RecoverFun(ADC_Select_VcIc);			//重新更新TXA DMA
	

		API_DMA_REST(ChDmaCurrentAdc2,Current_ADC_AdcDMA_BUFF_NUM);	
		API_DMA_REST(ChDmaCurrentAdc3,Current_ADC_AdcDMA_BUFF_NUM);	
	
		API_DMA_REST(ChDmaHrtimPotCh1,TxA_ADC_AdcBUFF_NUM);	
		API_DMA_REST(ChDmaHrtimPotCh2,TxA_ADC_AdcBUFF_NUM);	
		API_DMA_REST(ChDmaHrtimPotCh3,TxA_ADC_AdcBUFF_NUM);	
		API_DMA_REST(ChDmaHrtimPotCh4,TxA_ADC_AdcBUFF_NUM);	
	
		API_DMA_REST(ChDmaVcAdc1,TxA_ADC_AdcBUFF_NUM*2);
	
		API_ADC_StartAllAdc();
//		if(API_DMA_GetDmaCndtr(ChDmaCurrentAdc2)!=Current_ADC_AdcDMA_BUFF_NUM)
//		{
//				API_GPIO_WritePin(DebugB_pin,1);
//				API_GPIO_WritePin(DebugB_pin,0);		
//		}	
	
		API_TIM_TXA_RESET();			//开启ADC触发
		
	
	
}	
	
	
	
void 	API_ADC_Instance_SaveCallBack(ADC_SELECT_ENUM ch)			//保存Instance设置值 		在APP_ADC中实现
{
		volatile	ADC_M2M_RecoverDef		adcM2m;
		volatile	API_DMA_RecoverDef		recover;	
		uint32_t*   src,dest;
		uint32_t   length;
		API_ADC_DMA_RecoverFun(ch);
		
		for(uint8_t i=0;i<ChAdc_Max;i++)
		{

#if 1
			
			adcM2m=API_ADC_GetAddressInstance(i);
			recover.SrcAddress=(uint32_t)adcM2m.InstanceAddress;
			recover.DstAddress=(uint32_t)adcM2m.SaveAddress[ch];
			recover.DataLength=adcM2m.DataLength;
			
			
			API_DMA_RECOVER(ChDmaM2M, (API_DMA_RecoverDef*)(&recover));

			
#else
			src=adcM2m.InstanceAddress;
			dest=adcM2m.SaveAddress[ch]


#endif			
			
		}
}

void 	API_ADC_Instance_RecoverCallBack(ADC_SELECT_ENUM ch)			//恢复Instance设置值 		在APP_ADC中实现
{
		volatile	ADC_M2M_RecoverDef		adcM2m;
		volatile	API_DMA_RecoverDef		recover;	

		
		for(uint8_t i=0;i<ChAdc_Max;i++)
		{
			
			adcM2m=API_ADC_GetAddressInstance(i);
			recover.SrcAddress=(uint32_t)adcM2m.SaveAddress[ch];
			recover.DstAddress=(uint32_t)adcM2m.InstanceAddress;
			recover.DataLength=adcM2m.DataLength;
			
			API_DMA_RECOVER(ChDmaM2M, (API_DMA_RecoverDef*)(&recover));
		}
		API_ADC_DMA_RecoverFun(ch);		
		
}
	
	

void API_ADC_PENDV_IRQHandler(void)
{
	
//	DebugBoutput(1);
	
	
	portPendvClear();

	
}	



#include	"API_UART.H"

//static   uint32_t  adcBuff[4][100];
//static   uint32_t		adcSize,adcSum[4];

#define		DEBUGOUT 0






typedef	struct APP_ADC
{
		uint8_t		num;
		uint8_t  	potch;
		uint8_t 	pluseType;			//波形检测状态 0: 上升拐点, 1:下降拐点	
		uint8_t		res;	

		uint16_t  	newValue;
		uint16_t 	hrtimPointEnd;		//上管关断

		uint16_t 	hrtimPointOn;		//下管开启
		uint16_t 	hrtimPointOff;		//下管关断
		
		uint16_t	phaseUp;			//相位相对于1us的值
		int16_t		phaseUpValue;		//相位相对于HRTIM value的值
		uint16_t	phaseDown;
		int16_t		phaseDownValue;		

		uint16_t	TxaMax;				//电流最大值 
		uint16_t 	TxaMaxNum;
		uint16_t	upPoint;			//上管开通时间
		int16_t		res16;
	
		int16_t		phaseUpHrtim;			//相位值相对于HRTIM value的值
		int16_t		res16_1;			//相位值相对于HRTIM value的值

		uint32_t	downPhaseValue;
		uint32_t	upHalfValue;	//上升半波积分	
		uint32_t	upAllValue;	//上升半波积分	

		uint32_t 	lastAdjValue;
		uint32_t	sumHighUp;			//上管充电积分		以PPG HIGH关断为结束计算
		uint32_t 	sumDown;			//下降段积分		
		uint32_t 	sumUp;				//上升段积分		以PPG  LOW开通为结束
		uint32_t	lowSum;				//下管积分
		uint32_t	powerSum;
		uint32_t	phaseSum;			/*相位积分*/
		uint32_t	sumStart;			//开始三个点的累加值，用于判断相位	
		
		PPGpointDef ppgPoint;	//PPG CNT值 
}APP_ADC_TxaPointDef;



uint16_t 			CurrentSave[4][100];					//DMA连续采集，用于过流检测
uint16_t			sumin;
enum
{
	PLUSE_START=0,
	PLUSE_UP,
	PLUSE_DOWN,

};
typedef struct
{
	uint16_t  start;		// 一个完整周期的开始点
	uint16_t  end;		//	完整周期点数
	

}TxaHrtimPointDef;


typedef	struct
{
TxaHrtimPointDef		pointSave;
uint16_t 		zeroValue;	
uint16_t 		zeroPoint;
uint16_t*		hrtimBuff;
uint16_t 		firstValue;	
	
}SaveDataDef;	


SaveDataDef		savePoint[4];



uint16_t* TxaAdcBuff[4];						//谐振电流缓存指针
uint16_t* TxaHrtimBuff[4];					//谐振电流对应HRTIM缓存指针
uint16_t* TxaVcBuff[4];					//电压值缓存指针

uint16_t* TxaHrtimSaveBuff;					//谐振电流对应HRTIM缓存指针(保存用于打印输出）


uint16_t*	TxaFmacBuff[4];




TxaHrtimPointDef	APP_ADC_GetTxaStartPoint(uint8_t potCh)		//从DMA中提取一周期完整数据的起点与终点
{																														//group对应两组同步炉头	
	uint16_t startPoint,endPoint,temp;
		TxaHrtimPointDef  xRet={0};
	
	uint16_t* buffHrtim=(uint16_t*)&TxA_ADC_AdcDmaBuff.HrtimSave[potCh][0];		//第一个是错误数据

	savePoint[potCh].hrtimBuff=buffHrtim;
	savePoint[potCh].firstValue=buffHrtim[0];	
		
	startPoint=buffHrtim[FMAC_OFFSET+HRTIM_OFFSET];
	
	endPoint=API_PPG_GetPeroid(potCh);	
	
	temp=FRE_PER_ADC;
	
	if(startPoint<endPoint)
	{
		temp=endPoint-startPoint;	
		temp/=FRE_PER_ADC;				//得到周期最大值的大概位置 
	}
				
	if(temp>Pan_ADC_DMA_BUFF_NUM)
	{
		return	xRet;
	}	
	

		
	while(buffHrtim[temp]>FRE_PER_ADC)
	{
		temp++;	
	}
	savePoint[potCh].zeroPoint=temp;
	savePoint[potCh].zeroValue=buffHrtim[temp];
	
	uint16_t lastPointHrtim=endPoint-FRE_PER_ADC*2;//上一个值最后一个点
	
	if(buffHrtim[temp-1]<lastPointHrtim)
	{
			return xRet;
	}
	
	endPoint/=FRE_PER_ADC;
	
	if(temp<FMAC_OFFSET+HRTIM_OFFSET)			//前面补两个
	{
		startPoint=0;
		
	}
	else
	{
		temp-=HRTIM_OFFSET;
		startPoint=temp-(FMAC_OFFSET);
	}	
	

//						uint16_t startP=buffHrtim[temp];
//						if(startP>2000)			//不在起始HRTIM点
//						{
//								API_GPIO_WritePin(DebugB_pin,1);
//								API_GPIO_WritePin(DebugB_pin,0);	

//						}	
		
	
	endPoint+=temp;				//一个周期结束点	
	endPoint+=FMAC_OFFSET;					//结束点也加4个, 用于检查相位			
	
	


	xRet.end=endPoint;
	xRet.start=startPoint;
	return xRet;
	
}







void	APP_ADC_TxaBuffChange(uint8_t ch,TxaHrtimPointDef* point)//将4个一组的数据转换为连续保存
{
	uint16_t num=0; 

	uint16_t  start=point->start;
	uint16_t	end=point->end;
	ADC_Current_Def* currentAdr;
	if(ch<2)
	{
		currentAdr=(ADC_Current_Def*)&TxA_ADC_AdcDmaBuff.CurrentAdc2Save[0];

	}
	else
	{
		currentAdr=(ADC_Current_Def*)&TxA_ADC_AdcDmaBuff.CurrentAdc3Save[0];
	}


	
	uint8_t index=ch&0x1;//对应BUFF[0,1]

	for(uint16_t i=start;i<end;i++)
	{
	#ifdef		DEBUG_POWER_OUT_CONST//AS		//TXA取样
				TxaAdcBuff[ch][num]=Power_Calculator_GetTxaBuffAddress(0)[num];
				TxaVcBuff[ch][num]=Power_Calculator_GetVoltageBuffAddress(0)[num];		
	#else	
				TxaAdcBuff[ch][num]=currentAdr[i].buff[index];
				TxaVcBuff[ch][num]=TxA_ADC_AdcDmaBuff.VcAdc1[i].buff[1];
	#endif


			
			num++;
	}

	

}	


void	APP_ADC_TxaMessageOut(PowerCalculatorInputDef* input)
{
	
	MessageDef	message;
	PowerCalculatorInputDef*	inputCh=input+PotChWork;
	
//		if(AdcFromApiDma20ms.count==5)
//			if(AdcFromApiDma20ms.ceilQ[PotChWork][count]>0xa0)
//			if(AdcFromApiDma20ms.phase[PotChWork][count]>70)
	if(1)	
	{
	uint8_t count=AdcFromApiDma20ms.count-1;

//					API_GPIO_WritePin(DebugB_pin,1);

			uint16_t start=inputCh->start;
			uint16_t end=inputCh->end;
			uint16_t size=(end-start);
			message.array[0].size=size;		
			message.array[1].size=size;				
			message.array[2].size=size;	
			message.array[3].size=0;	

			message.array[0].buff=TxaHrtimBuff[PotChWork];
			message.array[1].buff=TxaVcBuff[PotChWork];
//			message.array[2].buff=TxaAdcBuff[PotChWork];		
			message.array[2].buff=TxaFmacBuff[PotChWork]+fmacLeveNum/2;
		


#if 0	
		
		
		
			size=20;
			message.array[4].size=size;		
			message.array[5].size=size;				
			message.array[6].size=size;	
			message.array[7].size=size;			

			message.array[4].buff=AdcFromApiDma20ms.voltage[PotChWork];
			message.array[5].buff=AdcFromApiDma20ms.phase[PotChWork];
			message.array[6].buff=AdcFromApiDma20ms.Power[PotChWork];
			message.array[7].buff=AdcFromApiDma20ms.Txa[PotChWork];
#else

			size =0;
			message.array[4].size=size;		
			message.array[5].size=size;				
			message.array[6].size=size;	
			message.array[7].size=size;	


#endif

			// uint32_t* 	buff2=(uint32_t *)&(AdcFromApiDma20ms.ceilQ[PotChWork][0]);
			

			
#if 0
				uint16_t para[4];
				para[0]=AdcFromApiDma20ms.ceilQ[PotChWork][count];						//相位值
				para[1]=AdcFromApiDma20ms.phase[PotChWork][count];	
				para[2]=AdcFromApiDma20ms.phaseValue[PotChWork][count];
				para[3]=arrayPoint[PotChWork];//AdcFromApiDma20ms.Power[PotChWork][count];
#else				
				uint16_t para[5];
				para[0]=inputCh->highOn;						//相位值
				para[1]=inputCh->highOff;	
				para[2]=inputCh->lowOn;
				para[3]=inputCh->lowOff;
				para[4]=count;
#endif


			message.paraArray.buff=para;	
			message.paraArray.size=5;
			message.num=TXA_MESSAGE;
			
			
#if 0			
			if(PrintMessagePush(message)==0)
			{//打印数据缓存成功
					printf(" Message txa Printf Fail");
			}
#else
#include	"wave_capture.h"
			WaveCapture_PushMessage((void*)&message);
#endif


//					API_GPIO_WritePin(DebugB_pin,0);
					
	}
}



//void	APP_ADC_WaitTxaCalOver(void)		//等待TXA DMA采集完成，防止与检锅冲突
//{
//	while(TxA_ADC_AdcDmaBuff.step!=TXA_StepStart);
//}


void 	APP_ADC_GetTxaPeiodPoint(void)			//从DMA缓存到TXA数组转换
{

	for(uint8_t ch=0;ch<PotNum;ch++)
	{ 

		inputArray[ch].perAdc=FRE_PER_ADC;
		TxaHrtimPointDef* txaHrtimPoint=(TxaHrtimPointDef*)&inputArray[ch].start;		//输入参数地址
				
		*txaHrtimPoint=APP_ADC_GetTxaStartPoint(ch);	//这是在完整缓存的位置
										
		if(txaHrtimPoint->end==0||txaHrtimPoint->end>180*4)
		{
			txaHrtimPoint->end=0;			//没有数据
			arrayPoint[ch]=0;		//绝对点
		}	
		else
		{
	
			if(ch==0&&AdcFromApiDma20ms.count==2)//20ms  同步一次用于检查多炉头是否HRTIM同步 在过零点修正
			{
					uint16_t num=txaHrtimPoint->start+5;		//保证同步点在翻转点后
					TxA_ADC_AdcDmaBuff.HrtimSyncBuff[0]=TxA_ADC_AdcDmaBuff.HrtimSave[0][num];
					TxA_ADC_AdcDmaBuff.HrtimSyncBuff[1]=TxA_ADC_AdcDmaBuff.HrtimSave[1][num];				
					TxA_ADC_AdcDmaBuff.HrtimSyncBuff[2]=TxA_ADC_AdcDmaBuff.HrtimSave[2][num];
					TxA_ADC_AdcDmaBuff.HrtimSyncBuff[3]=TxA_ADC_AdcDmaBuff.HrtimSave[3][num];				
			}	

			
			arrayPoint[ch]=txaHrtimPoint->start;
		
		}
		
		PPGpointDef	*ppgValue;
				
		ppgValue=(PPGpointDef*)&inputArray[ch].highOn;
		*ppgValue=API_PPG_getValueDeadTime(ch);
		
		if(ppgValue->highOff==0)
		{
			txaHrtimPoint->end=0;	
		}	
		else
		{
#ifdef	DEBUG_POWER_OUT_CONST//AS			//输入参数固定		
			inputArray[ch]=*Power_Calculator_GetInputArrayAddress(testCh);
#endif		//DEBUG_POWER_OUT
		}
	}

	
}				



void	APP_ADC_FmacSetTxa(uint16_t size)
{
		API_FMAC_MEMDEF* fmacMem=API_FMAC_GetMemAddress();
	
		if(fmacMem->type==FmacStop)		//没有FMAC在执行 在完成回调中清除
		{	
			fmacMem->type=FmacTxa;		//正在諧振電流濾波

			
		fmacMem->X1=(int16_t *)TxaAdcBuff[0]  ;
		fmacMem->Y=(int16_t *)TxaFmacBuff[0];
		fmacMem->outSize=size;

			API_FMAC_Rest();
		}	
}	


				

void		  API_FMAC_AppAdcOverCallBack(API_FMAC_MEMDEF*	FmacMem)    //谐振电流处理
{
		TxA_ADC_AdcDmaBuff.step=TXA_StepFmacEnd;
}


static	uint16_t MessageCnt=0;

void 	APP_ADC_TimDmaEnd(void)			//每1ms 计算一次TXA有效值
{	


	uint16_t		count=AdcFromApiDma20ms.count;
	uint16_t		count10=count%10;			//转换为10ms

	
	if(TxA_ADC_AdcDmaBuff.step==TXA_StepDmaEnd)
	{
//			API_GPIO_WritePin(DebugA_pin,1);	//txaDmaEnd

			TxA_ADC_AdcDmaBuff.step=TXA_StepStart;	//有时间片不需要处理
		
		
			AdcFromApiDma20ms.count++;	
//			if(count10==0)			//交流点不计算，APP_ADC_TxaAvgSum()中用最小值替代
//			{
//				
//				AdcFromApiDma20ms.Txa[0][count]		=0;
//				AdcFromApiDma20ms.Txa[1][count]		=0;
//				AdcFromApiDma20ms.Txa[2][count]		=0;
//				AdcFromApiDma20ms.Txa[3][count]		=0;

//				return;
//			}	



		if(AdcFromApiDma20ms.count<=20&&AdcFromApiDma20ms.count>1)
		{
//-----------找到起始点----------------------------------	
			

//			uint16_t delay=1000;
//			while(delay--);
			
			API_GPIO_WritePin(DebugA_pin,1);
			
startLoop:

			APP_ADC_GetTxaPeiodPoint();	//找到四个通道的起始点


			
	
			
			if(APP_ADC_MesageBuff())			//将DMA数据转存到数组
			{

					
			}
			API_GPIO_WritePin(DebugA_pin,0);
			
		}
		else
		{
				
		}	
		
	}	
}	
		


void	APP_ADC_CalculatePower(void)
{		
		
	
	if(TxA_ADC_AdcDmaBuff.step==TXA_StepFmacEnd)
	{	


			TxA_ADC_AdcDmaBuff.step=TXA_StepStart;
			uint8_t count=AdcFromApiDma20ms.count-1;
		
			for(uint8_t potCh=0;potCh<PotNum;potCh++)
			{	
//				if(TxA_ADC_AdcDmaBuff.Txa[potCh].adcFmac[0])

				if(inputArray[potCh].end>0&&inputArray[potCh].highOff>0)
				{

//				TxA_ADC_AdcDmaBuff.Txa[potCh].adcFmac[0]=0;//fmac完成后才有数值，作为FAMC完成的标志

				#ifdef	TxaFmac


				uint16_t* currentAdr=(uint16_t*)TxaFmacBuff[potCh];//计算fMAC计算后的数据首址
				currentAdr+=fmacLeveNum/2;

				#else		//txaFmac
				uint16_t* currentAdr=(uint16_t*)TxaAdcBuff[potCh];

				#endif		//txaFmac
				uint16_t* hrtimAdr=(uint16_t*)TxaHrtimBuff[potCh];
				uint16_t*	voltageAdr=(uint16_t*)TxaVcBuff[potCh];


				
					PowerResult	result;
					
					result=CalculatePower(currentAdr,hrtimAdr,voltageAdr,&inputArray[potCh]);		//计算谐振电流
				
					if(result.zero_cross_high)
					{	
						
						memcpy(&powerResult20ms[potCh].cycle[count],&result,sizeof(IH_CycleDataDef));//保存数据给20ms参数计算
						
						uint8_t value=result.zero_cross_high;
						APP_ADC_DebugValueCallBack(potCh,value);
						

						
						AdcFromApiDma20ms.Txa[potCh][count]	=result.active_current;//电流值
						AdcFromApiDma20ms.voltage[potCh][count]=result.voltage;//即时电压		
						AdcFromApiDma20ms.phase[potCh][count]=result.phase_angleUp;//相位值（角度）;	//这个不要变
						AdcFromApiDma20ms.iPeak[potCh][count]	=result.peak_current;//功率值		


						
						AdcFromApiDma20ms.ceilQ[potCh][count]=inputArray[potCh].highOff;//上管关断HRTIM值;
						AdcFromApiDma20ms.esr[potCh][count]=result.esr;//即时功率值
						AdcFromApiDma20ms.phaseValue[potCh][count]=result.zero_cross_high/(FRE_PER_ADC/4);//ppgValue->lowOn;//上管关断HRTIM值;
				
//					if(potCh==PotChWork&&	txaCount==TxaCount)
				

						
						if(potCh==PotChWork)
						{
							if(WaveCapture_IsReady()==0)
							{	
//							TxaHrtimPointDef* txaHrtimPoint=(TxaHrtimPointDef*)&inputArray[potCh].start;
							APP_ADC_TxaMessageOut(inputArray);//调试信息内存赋值
							}	
						}											
						
						
						
						
					}


	


					
					powerResult20ms[potCh].count = AdcFromApiDma20ms.count;
				}//if(inputArray[potCh].end>0&&inputArray[potCh].highOff>0)

			}


		}



#if 0		
		/* ---- 20ms 电参数计算 (4炉头统一) ---- */
		
		{
			uint16_t* cb[4] = {0};
			uint16_t* hb[4] = {0};
			uint16_t* vb[4] = {0};
			PowerCalculatorInputDef* ib[4] = {0};
			ElecParamsDef elec[4];
			uint8_t nh = (PotNum < 4) ? PotNum : 4;

			for (uint8_t h = 0; h < nh; h++) {
				if (inputArray[h].end > 0 && inputArray[h].highOff > 0) {
#ifdef TxaFmac
					cb[h] = (uint16_t*)TxaFmacBuff[h] + fmacLeveNum/2;
#else
					cb[h] = (uint16_t*)TxaAdcBuff[h];
#endif
					hb[h] = (uint16_t*)TxaHrtimBuff[h];
					vb[h] = (uint16_t*)TxaVcBuff[h];
					ib[h] = &inputArray[h];
				}
			}
			CalculateElecParams_20ms(cb, hb, vb, ib, elec);
			/* elec[h].L_kalman_uH / .Q_factor / .anomaly 供后续使用 */
		}
#endif
	if(PrintMessageOut())			//输出打印信息
	{

	}
}
#define		TXA_MAX_BUFF		120			//18K 最大数据缓存



void		APP_ADC_TxaPublicBuffInit(void)
{
			PublicBuffFreeAll();
			for(uint8_t i=0;i<PotNum;i++)		//分配VCBUFF
			{
					TxaVcBuff[i]=PubicBuffCalloc(TXA_MAX_BUFF*sizeof(uint16_t));

			}		
	

}	


uint16_t* Adc_GetHrtimSyncBuffAdr(void)
{	

	return (uint16_t *)&TxA_ADC_AdcDmaBuff.HrtimSyncBuff[0];

}

uint8_t   		APP_ADC_MesageBuff(void)
{	


	
				PublicBuffFreeAll();

				uint16_t buffSize[PotNum];		//四个缓存区大小
			
				for(uint8_t i=0;i<PotNum;i++)		//分配VCBUFF
				{
					TxaVcBuff[i]=0;				//没有工作的炉头不分配
					buffSize[i]=0;
					if(inputArray[i].end>inputArray[i].start)
					{	
						buffSize[i]=	inputArray[i].end-inputArray[i].start;
					}	
					TxaVcBuff[i]=PubicBuffCalloc(buffSize[i]*sizeof(uint16_t));			//电压缓存

				}

	

				for(uint8_t i=0;i<PotNum;i++)
				{
					TxaHrtimPointDef* txaHrtimPoint=(TxaHrtimPointDef*)&inputArray[i].start;
					TxaHrtimBuff[i]=0;			
					{

						uint16_t size=buffSize[i]+fmacLeveNum;
						uint16_t	hrtimStart=txaHrtimPoint->start;			//每个起点可能不一样
						hrtimStart+=HRTIM_OFFSET;
						TxaAdcBuff[i]=PubicBuffCalloc(size*sizeof(uint16_t));		//电流缓存
						
						
						#ifdef	DEBUG_POWER_OUT_CONST//AS				//HRTIM DATA 固定
						
						TxaHrtimBuff[i]=(uint16_t*)Power_Calculator_GetHrtimBuffAddress(0);
						#else		//DEBUG_POWER_OUT
						TxaHrtimBuff[i]=(uint16_t*)&TxA_ADC_AdcDmaBuff.HrtimSave[i][hrtimStart];

						
						#endif	//DEBUG_POWER_OUT
						
						uint16_t startP=TxaHrtimBuff[i][FMAC_OFFSET+2];
						if(startP>2000)			//不在起始HRTIM点
						{

								txaHrtimPoint->end=0;		
						}	
						

						{
							APP_ADC_TxaBuffChange(i,txaHrtimPoint);	
							//从绝对地址转换为相对地址
							txaHrtimPoint->start=0;			//数据区保留的是一组完整数据
							txaHrtimPoint->end=buffSize[i];//这个没有加FMACleve间隔
						}

					}

					
				}
				
		
				
#ifdef	TxaFmac			

	
				uint16_t 	sizeAll=0;

				for(uint8_t i=0;i<PotNum;i++)		//分配VCBUFF
				{
					TxaFmacBuff[i]=0;

					uint16_t size=buffSize[i]+fmacLeveNum;
					sizeAll+=size;
					
					TxaFmacBuff[i]=PubicBuffCalloc(size*sizeof(uint16_t));	//后面空间都给FMAC,这个100不是实际大小	

				}

				if(sizeAll)
				{
					TxA_ADC_AdcDmaBuff.step=TXA_StepFmacStart;		//需要FMAC
					APP_ADC_FmacSetTxa(sizeAll);
				}	

#else
	
				TxA_ADC_AdcDmaBuff.step=TXA_StepFmacEnd;
#endif

				return 1;
}


		
	



//uint32_t *  getAdcBuffAddress(uint8_t ch)
//{

//	return 	(uint32_t*)&(adcBuff[ch][0]);
//}
//uint32_t   getAdcBuffSize(void)
//{
//	return	adcSize;
//}
//uint32_t   getAdcBuffSum(uint8_t ch)
//{
//	return	adcSum[ch];
//}



void	API_DMA_M2M_OverCallback(void)
{	

}

// void	API_DMA_T1A_IRQHandlerCallBack(uint8_t	num)
// {

// 		API_DMA_RecoverDef	recover;

		
		
// //	API_GPIO_WritePin(DebugA_pin,1);	


// //		if(TxA_ADC_TimDmaBuff.flag.t100ms)
// 		{								//获取当前100ms 的完整周期T1A数据

// //			API_GPIO_WritePin(DebugA_pin,1);
// //			API_TIM_TXA_STOP();

// //			TxA_ADC_AdcDmaBuff.step=TXA_StepDmaEnd;//在DMA回调中处理数据	

// //			API_GPIO_WritePin(DebugA_pin,0);
			
// 			//recover.SrcAddress=(uint32_t)&TxA_ADC_TimDmaBuff;		//保存完整的150us数据，前面几个数据可能无效
// //			recover.DstAddress=(uint32_t)&TxA_ADC_TimDma100msBuff;
// //			recover.DataLength=sizeof(TxA_ADC_TimDmaBuff)/sizeof(int)-2;
// //			API_DMA_RECOVER(ChDmaM2M, (API_DMA_RecoverDef*)(&recover));
// 		}

// //--------得到HRTIM在TXA队列中的位置-------------------------------------


// //		recover.SrcAddress=(uint32_t)API_DMA_GetDmaCndtrAddress(ChDmaCurrent1);//交替HRTIM位置缓存
// //		recover.DstAddress=(uint32_t)&TxA_ADC_TimDmaBuff.Hrtim;
// //		recover.DataLength=Hrtim_ADC_DMA_BUFF_NUM;
// //			
// //		API_DMA_RECOVER(ChDmaHrtim, (API_DMA_RecoverDef*)(&recover));	
// //		API_GPIO_WritePin(DebugA_pin,0);	
// }


#if 0

#include <stdio.h>
#include <stdlib.h>

void find_main_pulse_edges(int data[], int len) {
    // 步骤1: 动态计算噪声阈值（取最大值的10%作为阈值）
    int max_val = 0;
    for (int i = 0; i < len; i++) {
        if (data[i] > max_val) max_val = data[i];
    }
    int noise_threshold = max_val * 0.1; // 主脉冲幅度10%以下视为干扰

    // 步骤2: 状态机跟踪脉冲边界
    int rise_start = -1;  // 上升起点索引
    int fall_end = -1;    // 下降终点索引
    int in_pulse = 0;     // 是否处于脉冲区间 (0:否, 1:是)

    // 步骤3: 遍历数据识别主脉冲边界
    for (int i = 1; i < len - 1; i++) {
        // 检测上升起点 (低→高 且突破阈值)
        if (!in_pulse && data[i] > noise_threshold && 
            data[i] > data[i-1] && data[i] >= data[i+1]) {
            rise_start = i;
            in_pulse = 1; // 进入脉冲区间
        }
        // 检测下降终点 (高→低 且跌破阈值)
        else if (in_pulse && data[i] > noise_threshold && 
                data[i] < data[i-1] && data[i] <= data[i+1]) {
            fall_end = i;
            in_pulse = 0; // 退出脉冲区间
            break;        // 找到主脉冲后立即退出
        }
    }

    // 步骤4: 输出结果
    printf("主脉冲边界检测结果：\n");
    printf("----------------------\n");
    if (rise_start != -1) {
        printf("上升起点\t%d\t%d\n", rise_start, data[rise_start]);
    }
    if (fall_end != -1) {
        printf("下降终点\t%d\t%d\n", fall_end, data[fall_end]);
    }
    if (rise_start == -1 || fall_end == -1) {
        printf("警告：未检测到完整脉冲边界！\n");
    }
}

int main() {
    // 输入数据（含干扰和小脉冲）
    int data[] = {
        12, 0, 3, 170, 455, 717, 948, 1142, 1349, 1408,
        1500, 1550, 1565, 1564, 1512, 1423, 1317, 1182,
        1022, 873, 684, 328, 75, 0, 8, 6, 7, 4, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };
    int len = sizeof(data) / sizeof(data);
    
    // 执行检测
    find_main_pulse_edges(data, len);
    return 0;
}



// 声明链接器生成的符号（用于拷贝）
extern uint32_t Load$$RAMCODE$$Base;    // Flash 中的加载地址
extern uint32_t Image$$RAMCODE$$Base;    // RAM 中的执行地址
extern uint32_t Image$$RAMCODE$$Length;  // 段长度



//// 将 ISR 从 Flash 拷贝到 RAM
//void CopyISRToRAM(void) {
//    uint32_t *src = (uint32_t*)Load$$RAMCODE$$Base;
//    uint32_t *dst = (uint32_t*)Image$$RAMCODE$$Base;;
//    uint32_t size = (uint32_t)0x1000;

//    for (uint32_t i = 0; i < size / 4; i++) {
//        dst[i] = src[i];  // 逐字拷贝
//    }

//    // 如果使用 Cache（如 STM32H7），需清理 Cache
//    // SCB_CleanDCache();
//}

#endif


/* === v2.0 Data Switcher interface ================================= */
void Adc_GetIO(Adc_Output_t **ppOut)
{
    *ppOut = &g_out;
}

void Adc_DoWork(void)
{
    g_out.status &= ~0x02;               /* 每帧先清就绪标志 */
    if (AdcValueFun()) {
        memcpy(g_out.inputValue, AdcFunRam.inputValue, sizeof(g_out.inputValue));
        g_out.status |= 0x02;            /* 有新产出时置位 */
    }
}


#if 0

#include <stdio.h>
#include <math.h>

#define DATA_SIZE 34
#define WINDOW_SIZE 5
#define RISE_THRESHOLD 50
#define FALL_THRESHOLD -30
#define MIN_PULSE_WIDTH 8

// 输入数据序列
int raw_data[DATA_SIZE] = {
    1, 131, 22, 6, 5, 0, 274, 199, 262, 422, 
    500, 568, 554, 492, 431, 347, 292, 220, 24, 64, 
    25, 3, 7, 7, 6, 4, 0, 0, 1, 7, 
    6, 6, 4, 1, 0
};

// 移动平均滤波
void moving_average_filter(int *input, double *output, int size) {
    for (int i = 0; i < size; i++) {
        double sum = 0;
        int count = 0;
        
        // 处理边界情况
        int start = (i - WINDOW_SIZE/2) > 0 ? (i - WINDOW_SIZE/2) : 0;
        int end = (i + WINDOW_SIZE/2) < size ? (i + WINDOW_SIZE/2) : size-1;
        
        for (int j = start; j <= end; j++) {
            sum += input[j];
            count++;
        }
        output[i] = sum / count;
    }
}

// 计算一阶导数（差分）
void calculate_derivative(double *data, double *derivative, int size) {
    for (int i = 1; i < size; i++) {
        derivative[i] = data[i] - data[i-1];
    }
    derivative = derivative; // 边界处理
}

// 检测主要脉冲的上升拐点
int detect_rise_point(double *derivative, int size) {
    int candidate = -1;
    double max_slope = 0;
    
    // 在数据前半部分寻找最大正斜率点
    for (int i = 2; i < size/2; i++) {
        // 使用3点平均斜率减少噪声影响
        double slope = (derivative[i-1] + derivative[i] + derivative[i+1]) / 3.0;
        
        if (slope > RISE_THRESHOLD && slope > max_slope) {
            max_slope = slope;
            candidate = i;
        }
    }
    
    return candidate;
}

// 检测主要脉冲的下降拐点
int detect_fall_point(double *data, double *derivative, int size, int rise_point) {
    int candidate = -1;
    double min_slope = 0;
    
    // 在上升点之后寻找最显著的负斜率点
    for (int i = rise_point + MIN_PULSE_WIDTH; i < size-2; i++) {
        // 使用3点平均斜率
        double slope = (derivative[i-1] + derivative[i] + derivative[i+1]) / 3.0;
        
        if (slope < FALL_THRESHOLD && slope < min_slope) {
            min_slope = slope;
            candidate = i;
        }
    }
    
    return candidate;
}

int main() {
    double filtered_data[DATA_SIZE];
    double derivative[DATA_SIZE];
    
    printf("=== 波形拐点检测程序 ===\n");
    printf("数据点数: %d\n", DATA_SIZE);
    
    // 1. 数据预处理 - 滤波降噪
    moving_average_filter(raw_data, filtered_data, DATA_SIZE);
    printf("数据滤波完成\n");
    
    // 2. 计算导数（变化率）
    calculate_derivative(filtered_data, derivative, DATA_SIZE);
    printf("导数计算完成\n");
    
    // 3. 检测上升拐点
    int rise_point = detect_rise_point(derivative, DATA_SIZE);
    
    // 4. 检测下降拐点
    int fall_point = detect_fall_point(filtered_data, derivative, DATA_SIZE, rise_point);
    
    // 5. 输出结果
    printf("\n=== 检测结果 ===\n");
    
    if (rise_point != -1) {
        printf("? 上升拐点: 索引 %d, 数值 %d\n", 
               rise_point, raw_data[rise_point]);
        printf("   前后趋势: %d → %d → %d\n", 
               raw_data[rise_point-1], raw_data[rise_point], raw_data[rise_point+1]);
    } else {
        printf("? 未检测到明显的上升拐点\n");
    }
    
    if (fall_point != -1) {
        printf("? 下降拐点: 索引 %d, 数值 %d\n", 
               fall_point, raw_data[fall_point]);
        printf("   前后趋势: %d → %d → %d\n", 
               raw_data[fall_point-1], raw_data[fall_point], raw_data[fall_point+1]);
    } else {
        printf("? 未检测到明显的下降拐点\n");
    }
    
    // 6. 脉冲特征分析
    if (rise_point != -1 && fall_point != -1) {
        int pulse_width = fall_point - rise_point;
        int peak_value = 0;
        int peak_index = rise_point;
        
        // 寻找峰值
        for (int i = rise_point; i <= fall_point; i++) {
            if (raw_data[i] > peak_value) {
                peak_value = raw_data[i];
                peak_index = i;
            }
        }
        
        printf("\n=== 脉冲特征分析 ===\n");
        printf("脉冲宽度: %d 个采样点\n", pulse_width);
        printf("峰值位置: 索引 %d, 峰值 %d\n", peak_index, peak_value);
        printf("上升幅度: %d\n", peak_value - raw_data[rise_point]);
        printf("下降幅度: %d\n", peak_value - raw_data[fall_point]);
    }
    
    return 0;
}

#endif