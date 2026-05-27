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
#include	"app_power.h"
#include "adc_processing.h"	

/* Private typedef -----------------------------------------------------------*/
typedef	struct 
{
	uint8_t 	adc20msCount;		//20MS 100US 次数统计
	uint8_t 	flag20ms;				//20MS adc平均值计算完成标志
	uint8_t		adcSelect;			//ADC规则通道选择
	uint8_t		res;
	
	int32_t 	inputValue[AdcGroupMax];			//计算处理后的AD值 
	// uint32_t	adcConvertedData[AdcGroupMax];		//从M2M得到的数据
	int32_t 	adcAverage[AdcGroupMax];			//平均值 
	// uint32_t	adcSum[AdcGroupMax];				//求和ADC值 
	
}APP_ADC_DEF;

typedef	struct{

	uint32_t  	buff[TxA_ADC_GROUP_NUM];	

}ADC_DualChannel_Def;
	
typedef	struct{

	uint16_t  	buff[TxA_ADC_ALL_GROUP_NUM];	

}ADC_FourChannel_Def;


typedef	struct{

	uint32_t  	buff[VcIc_ADC_GROUP_NUM];	

}ADC_VcIcChannel_Def;


typedef	struct{

	uint16_t  	buff[VcIc_ADC_GROUP_NUM];	

}ADC_Tempe_Def;


typedef struct
{
	uint16_t getMax:	8;			//连续采集MAX的平均值（20次） 
	uint16_t awgT12A:	1;			//ADC1发生AWG过流保护（找当次采集那个通道触发）
	uint16_t awgT34A:	1;			//ADC1发生AWG过流保护（找当次采集那个通道触发）
}ADC_Buff_FlagDef;

typedef struct
{
	uint16_t t100ms:	1;			//100ms取数时间到
	uint16_t dmaStart:	1;			//100msDMA开始 在DMA回调中切换到DMAEND
	uint16_t dmaEnd:	1;			//100msDMA结束，在主循环中处理
		
	
}ADC_T1A_Buff_FlagDef;	



typedef struct
{
	uint32_t	SumT1A;
	uint32_t	SumT2A;
	uint32_t	SumT3A;
	uint32_t	SumT4A;

}ADC_Buff_SumDef;

	


typedef struct
{
	
	uint8_t start;							//DMA开始点（0，或HALF)
	uint8_t end;
	union{
	ADC_Buff_FlagDef	flag;
	uint16_t	halfWord;
	};										//标志点，切换是否需要采最大点
	uint32_t 		count;			//累加记数		
	uint32_t 		Voltage;		//当前组的电压值 



	uint32_t			T12A[TxA_ADC_AdcDMA_BUFF_NUM];					//DMA连续采集，用于过流检测
	uint32_t			T34A[TxA_ADC_AdcDMA_BUFF_NUM];
	ADC_Buff_SumDef		Sum;
	ADC_Buff_SumDef		Avg;													
	uint16_t			Vc[TxA_ADC_AdcDMA_BUFF_NUM];						//DMA连续采集，用于过流检测
	uint16_t			Hrtim[Hrtim_ADC_DMA_BUFF_NUM];					//DMA连续采集，用于过流检测
}APP_ADC_AdcTxADMA_BUFF_DEF;			//ADC dma 缓存区




typedef struct
{
	
	uint8_t ch;							//DMA开始点（0start ，1HALF)
	uint8_t count;							//100ms計數值
	union{
	ADC_T1A_Buff_FlagDef	flag;
	uint16_t	halfWord;
	};										//标志点，切换是否需要采最大点
	
	uint16_t  	Txa[4][TxA_ADC_TimDMA_BUFF_NUM];
	// uint16_t	T1A[TxA_ADC_TimDMA_BUFF_NUM];		//t1a數據DMA緩存
	// uint16_t	T2A[TxA_ADC_TimDMA_BUFF_NUM];
	// uint16_t	T3A[TxA_ADC_TimDMA_BUFF_NUM];	
	// uint16_t	T4A[TxA_ADC_TimDMA_BUFF_NUM];	
	uint16_t	Hrtim[Hrtim_ADC_DMA_BUFF_NUM];		//兩组缓存与T1A配合
	uint16_t	VC[TxA_ADC_TimDMA_BUFF_NUM];

}APP_ADC_TimTxADMA_BUFF_DEF;			//100us采集一组的集合

typedef struct
{
	uint32_t 							num;					//有效计数
	ADC_VcIcChannel_Def		VcIc[AdcAvageCount];		//adc2*8
	ADC_Tempe_Def 			Tempe;						//adc1
	ADC_VcIcChannel_Def		FanAd;						//adc2
	uint16_t 				Txa[4][TxaAvageCount];		//保存TXA 100us采集值
	ADC_VcIcChannel_Def 	res;	
	
}APP_ADC_VcIcDMA_BUFF_DEF;			//DMA循环采样



typedef struct
{
	uint8_t 							num;					//有效计数
	uint8_t 				count;				//100ms計數
	uint16_t 				start;				//有效读数位置
	ADC_VcIcChannel_Def		VcIc[AdcAvageCount];		//adc2*199
	uint16_t		Txa[4][TxaAvageCount];			//20平均	
	ADC_Tempe_Def 			Tempe;						//adc1
	ADC_VcIcChannel_Def		FanAd;						//adc2
	ADC_VcIcChannel_Def 	res;	
	
}APP_ADC_AVG_BUFF_DEF;			//20ms采集一组的数据集合




APP_ADC_AWD_DNTR_DEF	APP_ADC_Dntr[2]={{0,0,0,{0},},{0,2,0,{0}}};			//ch赋初值

#define		APP_ADC_DNTR_T12A	APP_ADC_Dntr[DNTR_T12A]
#define		APP_ADC_DNTR_T34A	APP_ADC_Dntr[DNTR_T34A]


#if 0			//实例化回调函数

//API_ADC
__weak	void	API_VcIc_EOC_IRQHandlerCallBack(void);			//ADC2 EOC中断
__weak	void	API_T34_EOC_IRQHandlerCallBack(void);				//ADC3 EOC中断
__weak	void	API_ADC_DMA_M2M_IRQHandlerCallBack(void);		//M2M DMA中断
__weak	void	API_ADC_DMA_TxA_IRQHandlerCallBack(void);
//API_HRTIM
__weak	void	API_HRTIM1_TEST_UPD_IRQHandlerCallback(void);		//HRTIM的UPD中断，开停ADC T34A EOC中断

#endif

/* Private define ------------------------------------------------------------*/

#define		ADC_MAX_AVG_COUNT		10			//最大值取数次数


/* Private macro -------------------------------------------------------------*/




/* Private variables ---------------------------------------------------------*/





volatile	APP_ADC_TimTxADMA_BUFF_DEF			TxA_ADC_TimDmaBuff,TxA_ADC_TimDma100msBuff;									//DMA连续采样的缓存区，DMA启动后不停止，应用程序需要根据CNDTR的值进行取数
volatile	APP_ADC_AdcTxADMA_BUFF_DEF			TxA_ADC_AdcDmaBuff;									//DMA连续采样的缓存区，DMA启动后不停止，应用程序需要根据CNDTR的值进行取数
volatile	APP_ADC_VcIcDMA_BUFF_DEF				VcIc_ADC_DmaBuff;				//	电压DMA通道是连续采8组EOC,需要在100US中断中取出到VCIC的	AdcFromApiDma20ms中		
int16_t 	Pan_ADC_AdcDmaBuff[Pan_ADC_DMA_BUFF_NUM];




#define	VcIc_ADC_Buff		VcIc_ADC_DmaBuff.VcIc
#define	FanAd_ADC_Buff	VcIc_ADC_DmaBuff.FanAd
#define	Tempe_ADC_Buff	VcIc_ADC_DmaBuff.Tempe
#define	res_ADC_Buff	VcIc_ADC_DmaBuff.res




//#define		AdcFromApiDma100us	TxA_ADC_TimDmaBuff			//从API DMA 每10us传过来的数据谐振
APP_ADC_AVG_BUFF_DEF		AdcFromApiDma20ms;				//从API DMA 每20ms传过来的数据
APP_ADC_DEF					AdcFunRam;




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


/* Private function prototypes -----------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/



/* Private functions ---------------------------------------------------------*/
void		APP_ADC_SELECT_GROUP(uint8_t ch);//选择对应的ADC规则通道 OP1 P1P2
void		APP_ADC_ZERO_IrqFun(void);				//过零点进行ADC求平均值 
void		getADCmax(void);

 __attribute__((weak))	void	APP_ADC_IRQ_PPGstepDecTxA(APP_ADC_AWD_DNTR_DEF* txaDntr,uint16_t* txaBuff)
{}
__attribute__((weak))	void	APP_ADC_IRQ_PPGstepChangeCallBack(void)	//HRTIM1 UPD更新PPG
{}	
__attribute__((weak))	void	APP_ADC_IRQ_PPGstepDecT12aCallBack(APP_ADC_AWD_DNTR_DEF* dntr)	//HRTIM1 CMP1 PPG减小限流操作
{}	
__attribute__((weak))	void	APP_ADC_IRQ_PPGstepDecT34aCallBack(APP_ADC_AWD_DNTR_DEF* dntr)	//HRTIM1 CMP1 PPG减小限流操作
{}		
	
/**
  * @brief  Initialize the GPIOx peripheral according to the specified parameters in the GPIO_Init.连带IO口号一起运算
  * @param  GPIOx: where x can be (A..F) to select the GPIO peripheral for RX32G4xx family
  * @param  GPIO_Init: pointer to a GPIO_InitTypeDef structure that contains
  *         the configuration information for the specified GPIO peripheral.
            @arg @ref GPIO_InitTypeDef
  * @retval None
  */
	
uint32_t 	APP_ADC_GetPowerTxa(uint8_t ch)
{
	uint32_t*  value=(uint32_t*)(&TxA_ADC_AdcDmaBuff.Avg);
	return 	value[ch];
}	

uint16_t*		APP_ADC_GetHrtimAddress(void)
{
		return	(uint16_t*)&(TxA_ADC_AdcDmaBuff.Hrtim);
}


int16_t  icvalue[AdcAvageCount];	
	
int16_t*		getIcValueAdress(void)
{
		return	icvalue;
}	


void	APP_ADC_TxaAvgSum(uint8_t ch)	
{
	
	uint16_t   	minTxa=0xffff;		//第一个数据保存最小值
	uint16_t 	txaValue;
	uint32_t 	newValue=0,oldValue;

	for(uint8_t num=1;num<TxaAvageCount;num++)
	{
		txaValue=AdcFromApiDma20ms.Txa[ch][num];

		newValue+=txaValue;	
		if(minTxa>txaValue)
		{
			minTxa=txaValue;
		}	
	}
	minTxa*=2;
	newValue+=minTxa;	
	newValue/=(TxaAvageCount+1);		//第0与21值为最小值

	if(AdcAverage20ms[AdcGroupT1A+ch]==0)
	{
		oldValue=newValue;//初始化
	}
	else
	{		//一阶滤波 Yn=Y(n-1)*(1-a)+a*x(n)
		oldValue=AdcAverage20ms[AdcGroupT1A+ch]*(DIV_BASE-DIV_VALUE);
		oldValue+=newValue*DIV_VALUE;	
		oldValue/=DIV_BASE;
	}

	AdcAverage20ms[AdcGroupT1A+ch]=oldValue;

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
//			AdcAverage20ms[AdcGroupCurrent1]+=	AdcFromApiDma20ms.VcIc[i].buff[Current1ADC2_Group];
//			AdcAverage20ms[AdcGroupCurrent2]+=	AdcFromApiDma20ms.VcIc[i].buff[Current2ADC2_Group];
//			AdcAverage20ms[AdcGroupCurrent3]+=	AdcFromApiDma20ms.VcIc[i].buff[Current3ADC2_Group];
//			AdcAverage20ms[AdcGroupCurrent4]+=	AdcFromApiDma20ms.VcIc[i].buff[Current4ADC2_Group];			
//			AdcAverage20ms[AdcGroupVoltage]	+=	AdcFromApiDma20ms.VcIc[i].buff[VoltageADC2_Group];
			AdcAverage20ms[AdcGroupCurrent1]+=	VcIc_ADC_DmaBuff.VcIc[i].buff[Current1ADC2_Group];
			AdcAverage20ms[AdcGroupCurrent2]+=	VcIc_ADC_DmaBuff.VcIc[i].buff[Current2ADC2_Group];

			AdcAverage20ms[AdcGroupCurrent3]+=	VcIc_ADC_DmaBuff.VcIc[i].buff[Current3ADC2_Group];
			AdcAverage20ms[AdcGroupCurrent4]+=	VcIc_ADC_DmaBuff.VcIc[i].buff[Current4ADC2_Group];		
		
			AdcAverage20ms[AdcGroupVoltage]	+=	VcIc_ADC_DmaBuff.VcIc[i].buff[VoltageADC2_Group];
			 
			icvalue[i]=VcIc_ADC_DmaBuff.VcIc[i].buff[Current1ADC2_Group];		//保存电流数据用于输出


		}
	
		for(uint8_t i=AdcGroupCurrent1;i<=AdcGroupVoltage;i++)
		{
//			AdcAverage20ms[i] = ME_UDIV(Adc20msSum[i], AdcAvageCount);
				AdcAverage20ms[i]/=AdcAvageCount;
				// Adc20msSum[i]=0;
			
		}
		
		
		


#if 1




			AdcFromApiDma20ms.Txa[0][0]=0xffff;//最小值初始化
			AdcFromApiDma20ms.Txa[1][0]=0xffff;
			AdcFromApiDma20ms.Txa[2][0]=0xffff;
			AdcFromApiDma20ms.Txa[3][0]=0xffff;

			AdcAverage20ms[AdcGroupT1A]=0;//求和值初始化
			AdcAverage20ms[AdcGroupT2A]=0;
			AdcAverage20ms[AdcGroupT3A]=0;
			AdcAverage20ms[AdcGroupT4A]=0;
										



			APP_ADC_TxaAvgSum(0);
			APP_ADC_TxaAvgSum(1);				
			APP_ADC_TxaAvgSum(2);
			APP_ADC_TxaAvgSum(3);


	
#endif				
			
			
			


//-----------------以下ADC不需要滤波---------------------------------------------------------------------						

			AdcAverage20ms[AdcGroupIgbt1]=(AdcFromApiDma20ms.Tempe.buff[Igbt1ADC1_GroupZero])&0XFFFF;
			AdcAverage20ms[AdcGroupIgbt2]=(AdcFromApiDma20ms.Tempe.buff[Igbt2ADC1_GroupZero]>>8)&0XFFFF;
			AdcAverage20ms[AdcGroupBottom1]=(AdcFromApiDma20ms.Tempe.buff[Bottom1ADC1_GroupZero])&0XFFFF;
			AdcAverage20ms[AdcGroupBottom2]=(AdcFromApiDma20ms.Tempe.buff[Bottom2ADC1_GroupZero])&0XFFFF;
			AdcAverage20ms[AdcGroupBottom3]=(AdcFromApiDma20ms.Tempe.buff[Bottom3ADC1_GroupZero])&0XFFFF;
			AdcAverage20ms[AdcGroupBottom4]=(AdcFromApiDma20ms.Tempe.buff[Bottom4ADC1_GroupZero])&0XFFFF;
						
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
		

	
						AdcInputValue[AdcGroupCurrent1]=getADCaverage20ms(AdcGroupCurrent1);				//通道1电流值 
						AdcInputValue[AdcGroupCurrent2]=getADCaverage20ms(AdcGroupCurrent2);				//通道2电流值 
						AdcInputValue[AdcGroupCurrent3]=getADCaverage20ms(AdcGroupCurrent3);				//通道1电流值 
						AdcInputValue[AdcGroupCurrent4]=getADCaverage20ms(AdcGroupCurrent4);				//通道2电流值 
					
		

						AdcInputValue[AdcGroupT1A]=getADCaverage20ms(AdcGroupT1A);				//通道1谐振电流值 
						AdcInputValue[AdcGroupT2A]=getADCaverage20ms(AdcGroupT2A);				//通道2谐振电流值 
						AdcInputValue[AdcGroupT3A]=getADCaverage20ms(AdcGroupT3A);				//通道1谐振电流值 
						AdcInputValue[AdcGroupT4A]=getADCaverage20ms(AdcGroupT4A);				//通道2谐振电流值 

	
						for(uint8_t i=AdcGroupBottom1;i<AdcGroupFan;i++)
						{
								value =getADCaverage20ms(i);									//热敏电阻ADC值求反
								value|=0xf000;
								AdcInputValue[i]=~value;

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

	
// 	AdcFromApiDma20ms.VcIc[Adc20msCount].buff[Current1ADC2_Group]=VcIc_ADC_DmaBuff.VcIc[ch].buff[Current1ADC2_Group];
// 	AdcFromApiDma20ms.VcIc[Adc20msCount].buff[Current2ADC2_Group]=VcIc_ADC_DmaBuff.VcIc[ch].buff[Current2ADC2_Group];
// 	AdcFromApiDma20ms.VcIc[Adc20msCount].buff[Current3ADC2_Group]=VcIc_ADC_DmaBuff.VcIc[ch].buff[Current3ADC2_Group];
// 	AdcFromApiDma20ms.VcIc[Adc20msCount].buff[Current4ADC2_Group]=VcIc_ADC_DmaBuff.VcIc[ch].buff[Current4ADC2_Group];
// 	AdcFromApiDma20ms.VcIc[Adc20msCount].buff[VoltageADC2_Group]=VcIc_ADC_DmaBuff.VcIc[ch].buff[VoltageADC2_Group];

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

	AdcFromApiDma20ms.Tempe.buff[Igbt1ADC1_GroupZero]=VcIc_ADC_DmaBuff.Tempe.buff[Igbt1ADC1_GroupZero];
	AdcFromApiDma20ms.Tempe.buff[Igbt2ADC1_GroupZero]=VcIc_ADC_DmaBuff.Tempe.buff[Igbt2ADC1_GroupZero];
	AdcFromApiDma20ms.Tempe.buff[Bottom1ADC1_GroupZero]=VcIc_ADC_DmaBuff.Tempe.buff[Bottom1ADC1_GroupZero];
	AdcFromApiDma20ms.Tempe.buff[Bottom2ADC1_GroupZero]=VcIc_ADC_DmaBuff.Tempe.buff[Bottom2ADC1_GroupZero];
	AdcFromApiDma20ms.Tempe.buff[Bottom3ADC1_GroupZero]=VcIc_ADC_DmaBuff.Tempe.buff[Bottom3ADC1_GroupZero];
	AdcFromApiDma20ms.Tempe.buff[Bottom4ADC1_GroupZero]=VcIc_ADC_DmaBuff.Tempe.buff[Bottom4ADC1_GroupZero];

}	

void	APP_ZERO_Adc100usCallBack(void)
//void	 	APP_ADC_Adc20msCountADD(void)
{
				//				API_ADC_GET100us_Buff(&AdcFromApiDma100us);
				if(Adc20msCount<AdcAvageCount)			//第199次切换VC与TEMPE
				{	
		
//					APP_ADC_GET_VcIcBuff();//得到VCIC的当前数据	
					
					API_ADC_VcIc_ENABLE();	//開啟VCIC ADC 等待 HRTIM CMP2觸發


				}

				if(	Adc20msCount==AdcAvageCount)
				{	
						AdcFromApiDma20ms.num=1;
						Adc20msCount++;


					
						APP_ADC_GET_TxaAvg();				//得到功率平均值
			
//						API_TIM_100US_STOP();
						API_HRTIM_DISABLE_IT_UPD();
					
						API_DMA_STOP(ChDmaPan);
					
						AdcSelect=ADC_Select_Tempe;
						APP_ADC_SELECT_GROUP(ADC_Select_Tempe);	
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
	API_DMA_STOP(ChDmaPan);
	AdcSelect=ADC_Select_VcIc;
	APP_ADC_SELECT_GROUP(ADC_Select_VcIc);	

	
	
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
			//API_ADC_GET20ms_Buff(&AdcFromApiDma20ms);				//得到TEMPE数据
	
//			AdcFromApiDma100us.end=0;
//			AdcFromApiDma100us.start=0;	

//		API_ADC_DMA_ONOFF(1);


			API_HRTIM_ENABLE_IT_UPD();				//开启HRTIM UPD中断 准备读取谐振电流

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
	//ADC vcic 累加
	

//	AdcFromApiDma20ms.start=API_DMA_GetDmaCndtr(ChDmaVcIc);
//	API_ADC_VcIc_DISABLE_IT_EOC();			//	这里去读取VCIC 
	

//	APP_ADC_Adc20msCountADD();


//		API_HRTIM_ENABLE_IT_UPD();				//开启HRTIM UPD中断 准备读取谐振电流
	
//		uint32_t  dmaCounter=TxA_ADC_DMA_BUFF_NUM;						
//		dmaCounter-=	API_ADC_GetDmaCndtr();	

//		AdcFromApiDma100us.end=AdcFromApiDma100us.start;
//		AdcFromApiDma100us.start=	dmaCounter;					//得到结束点						//计算START-END之间的数据，100US内采集的TXA数据进行滤波后求和处理

////		portPendvSet();					//马上去进行每100us 执行一次					
//	
//	
//		API_GPIO_WritePin(DebugB_pin,1);	
		API_ADC_VcIc_DISABLE();		//关闭VCIC ADC 等待下一个100us
		
		volatile	API_DMA_RecoverDef		recover;	
				

		recover.SrcAddress=(uint32_t)API_ADC_GetAddressDRx(ChAdc2_VcIc,Current1ADC2_Group);
		recover.DstAddress=(uint32_t)VcIc_ADC_DmaBuff.VcIc[Adc20msCount].buff;
		recover.DataLength=VcIc_ADC_GROUP_NUM;
		
		API_DMA_RECOVER(ChDmaM2M, (API_DMA_RecoverDef*)(&recover));
		

		if(Adc20msCount%10==0)
		{
			TxA_ADC_TimDmaBuff.flag.t100ms=1;
			API_TIM_TXA_RESET();				//开始触发TXA 1us
		}
		Adc20msCount++;	
		API_GPIO_WritePin(DebugB_pin,0);
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

void	API_HRTIM1_TEST_UPD_IRQHandlerCallback(void)		//HRTIM的UPD中断回调，开停ADC T34A EOC中断
{
//			AdcFromApiDma100us.start=API_DMA_GetDmaCndtr(ChDmaT34A);
//			portPendvSet();
	
//			API_ADC_T34A_ENABLE_IT_EOC();				//开启T34 UPD中断
//			API_HRTIM_ENABLE_IT_REST();
//			API_HRTIM_DISABLE_IT_UPD();
//			count=0;
	
//					APP_ADC_Dntr[0].num=2;
//					APP_ADC_Dntr[0].dntr[0]=15;
//					APP_ADC_Dntr[0].dntr[1]=8;	
//					APP_ADC_IRQ_PPGstepDecTxA_ASM();
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






//void	API_HRTIM1_TEST_CMP1_IRQHandlerCallback(void)		//HRTIM的UPD中断回调，开停ADC T34A EOC中断
//{
//}
#define		APP_ADC_TxaMax		0xa00				//谐振电流保护值 



//******************************************************************
//			step 4
// 函数名	：void	API_HRTIM1_TEST_CMP1_IRQHandlerCallback(void)				
// 作者		：
// 功能		：//API_HRTIM.c的CMP1中断函数，
// 参数		：
// 返回值	：
// 调用全局变量:				
// 修改全局变量:			
// 备注：		1\记录DMA传送结束位置
//				2\关闭	CMP1中断，结束谐振电流值采集	
//			
//												
//												
//*****************************************************************
//void	API_MCU_TXA_IRQHandler(void)
void	API_HRTIM1_TEST_CMP1_IRQHandlerCallback(void)		//HRTIM的UPD中断回调，开停ADC T34A EOC中断
{	
//		API_TIM_TXA_STOP();
	
		uint8_t 	ch;
	
		API_HRTIM_DISABLE_IT_REST();		


		if(APP_ADC_DNTR_T12A.num)
		{	
			
			APP_ADC_IRQ_PPGstepDecTxA(&APP_ADC_DNTR_T12A,(uint16_t*)TxA_ADC_AdcDmaBuff.T12A);	//传递DNTR数据供判断

		}

		
		
		if(APP_ADC_DNTR_T34A.num)
		{
			APP_ADC_IRQ_PPGstepDecTxA(&APP_ADC_DNTR_T34A,(uint16_t*)TxA_ADC_AdcDmaBuff.T34A);	//传递DNTR数据供判断
		}	
		APP_ADC_DNTR_T12A.num=0;
		APP_ADC_DNTR_T34A.num=0;

}		


//******************************************************************
//			
// 函数名	：API_T12_AWD_IRQHandlerCallBack(void);			
// 作者		：
// 功能		：//API_ADC.c的ADC1 AWD中断函数，
// 参数		：
// 返回值	：
// 调用全局变量:				
// 修改全局变量:			
// 备注：		1\记录DMA转换位置，供后面取数
//				2实际处理在HRTIM CMP1中执行，此时谐振电流以下降，
//				
//*****************************************************************



void	API_T12_AWD_IRQHandlerCallBack(void)
{
		volatile	uint32_t  value;

	
		value=API_DMA_GetDmaCndtr(ChDmaT12A);
	
//		API_TIM_TXA_RESET();
		API_HRTIM_ENABLE_IT_REST();		
		APP_ADC_DNTR_T12A.dntr[APP_ADC_DNTR_T12A.num]=value;
		
		if(APP_ADC_DNTR_T12A.num<APP_ADC_DNTR_BUFF_MAX-1)
		{	
				APP_ADC_DNTR_T12A.num++;
		}

	
}
void	API_T34_AWD_IRQHandlerCallBack(void)
{
		volatile	uint32_t	value;
		value=API_DMA_GetDmaCndtr(ChDmaT34A);
//		API_TIM_TXA_RESET();
		API_HRTIM_ENABLE_IT_REST();	
		APP_ADC_DNTR_T34A.dntr[APP_ADC_DNTR_T34A.num]=value;

		if(APP_ADC_DNTR_T34A.num<APP_ADC_DNTR_BUFF_MAX-1)
		{	
				APP_ADC_DNTR_T34A.num++;
		}

}


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
	API_TIM_100US_STOP();
	API_OPAMP1_Select(ch);
	API_ADC_SELECT(ch);
	APP_POWER_SetTxaAwdValue();	
	
}


enum
{
		SW_ON=0,
		SW_OFF,
};	


void		APP_ADC_PanSwChange(uint32_t ch)
{
		uint32_t pinCh=PANSW1_pin+ch;

			API_GPIO_Mode(PAN_pin ,MODER_Output);


		API_GPIO_Mode(PANSW1_pin ,MODER_Output);
		API_GPIO_Mode(PANSW2_pin ,MODER_Output);
		API_GPIO_Mode(PANSW3_pin ,MODER_Output);
		API_GPIO_Mode(PANSW4_pin ,MODER_Output);
	
		API_GPIO_WritePin(PANSW4_pin,SW_OFF);		//输出0清除状态	
		API_GPIO_WritePin(PANSW1_pin,SW_OFF);		//输出0清除状态	
		API_GPIO_WritePin(PANSW2_pin,SW_OFF);		//输出0清除状态	
		API_GPIO_WritePin(PANSW3_pin,SW_OFF);		//输出0清除状态	
	#ifdef	DEBUG_POWER_OUT				//PANSW4不清0 TIM1_CH4 用于FALULT消隐
	
		API_GPIO_Mode(PANSW4_pin ,MODER_Output);	//同时是TIM1_CH4	
		API_GPIO_WritePin(PANSW4_pin,0);		//输出0清除状态		
	#endif

		if(ch<10)						
		{	
			API_GPIO_WritePin(pinCh,SW_ON);
//			API_GPIO_Mode(pinCh ,MODER_Input);
//			API_GPIO_PinPull(pinCh,PUPDR_Pulldown);			//下拉电阻
		
			API_GPIO_BaseInit(PAN_pin);			//PAN切换回初始ADC状态
		}
}	




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

void	API_DMA_TxA_IRQHandlerCallBack(uint8_t	num)
{
#if 0	
//		AdcFromApiDma100us.start=num;
		if(num)
		{
			TxA_ADC_AdcDmaBuff.start=TxA_ADC_AdcDMA_BUFF_NUM/4;		//注意实际运算是32位，保存是16位，高16位T2A,低16位T1A
		}
		else
		{
			TxA_ADC_AdcDmaBuff.start=0;
		}	
#endif	
//		portPendvSet();
//		API_GPIO_WritePin(DebugB_pin,1);	
		
			// TxA_ADC_AdcDmaBuff.Voltage=(*((uint16_t*)API_ADC_GetAddressDRx(ChAdc2_VcIc,VoltageADC2_Group)));
			// APP_ADC_AvgTxA20us();				//不需要采最大值 
//	TxA_ADC_AdcDmaBuff.count++;
//		API_GPIO_WritePin(DebugB_pin,0);	
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
int32_t  	getADCinputValue(uint8_t ch)
{

	return		AdcFunRam.inputValue[ch];
}


		

int16_t* 	API_ADC_GetPanDmaBuffAddress(void)
{

		return	(int16_t*)&Pan_ADC_AdcDmaBuff;
}	



void	API_ADC_DMA_RecoverPan(void)				//恢复检锅DMA
{

	//每1ms后，等待T1A DMA中斷觸發后，開始保存HRTIM位置，保證能訊錄到一個完整的HRTIM周期
		volatile	API_DMA_RecoverDef		recover;
		recover.SrcAddress=API_ADC_GetAddressJDRx(ChAdc2_VcIc,PanJDRxADC2_Group);
//		recover.SrcAddress=API_ADC_GetAddressDR(ChAdc2_VcIc);
		recover.DstAddress=(uint32_t)Pan_ADC_AdcDmaBuff;
		recover.DataLength=Pan_ADC_DMA_BUFF_NUM;
			
		API_DMA_RECOVER(ChDmaPan, (API_DMA_RecoverDef*)(&recover));


}
	
void	API_ADC_DMA_RecoverTxaPan(void)				//恢复检锅DMA
{
		volatile	API_DMA_RecoverDef		recover;
		recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc1_T12A,T1A_0_ADC1Group);
//		recover.SrcAddress=API_ADC_GetAddressDR(ChAdc2_VcIc);
		recover.DstAddress=(uint32_t)Pan_ADC_AdcDmaBuff;
		recover.DataLength=Pan_ADC_DMA_BUFF_NUM;
			
		API_DMA_RECOVER(ChDmaPan, (API_DMA_RecoverDef*)(&recover));

}	


void	API_ADC_DMA_RecoverTxa(void)				//恢复检锅DMA
{
		volatile	API_DMA_RecoverDef		recover;		//T1A 每1us保存一次，總緩存區130*2，保證能記錄一個HRTIM周期AD值
		recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc1_T12A,T1A_0_ADC1Group);
		recover.DstAddress=(uint32_t)TxA_ADC_TimDmaBuff.Txa[chT1A];
		recover.DataLength=TxA_ADC_TimDMA_BUFF_NUM;
			
		API_DMA_RECOVER(ChDmaT1A, (API_DMA_RecoverDef*)(&recover));

//		recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc1_T12A,T2A_0_ADC1Group);
//		recover.DstAddress=(uint32_t)TxA_ADC_TimDmaBuff.Txa[chT2A];
//		recover.DataLength=TxA_ADC_TimDMA_BUFF_NUM;
//		API_DMA_RECOVER(ChDmaT2A, (API_DMA_RecoverDef*)(&recover));

		// recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc3_T34A,T3A_0_ADC3Group);
		// recover.DstAddress=(uint32_t)TxA_ADC_TimDmaBuff.T3A;
		// recover.DataLength=TxA_ADC_TimDMA_BUFF_NUM;
		// API_DMA_RECOVER(ChDmaT3A, (API_DMA_RecoverDef*)(&recover));

		// recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc3_T34A,T4A_0_ADC3Group);
		// recover.DstAddress=(uint32_t)TxA_ADC_TimDmaBuff.T4A;
		// recover.DataLength=TxA_ADC_TimDMA_BUFF_NUM;
		// API_DMA_RECOVER(ChDmaT4A, (API_DMA_RecoverDef*)(&recover));

		recover.SrcAddress=(uint32_t)API_DMA_GetDmaCndtrAddress(ChDmaT1A);//得到当前TXA DMA存的位置
		recover.DstAddress=(uint32_t)TxA_ADC_TimDmaBuff.Hrtim;
		recover.DataLength=Hrtim_ADC_DMA_BUFF_NUM;
			
		API_DMA_RECOVER(ChDmaHrtim, (API_DMA_RecoverDef*)(&recover));	


}
	


//void	API_ADC_DMA_RecoverCallBack(ADC_SELECT_ENUM ch)
void	API_ADC_DMA_RecoverFun(ADC_SELECT_ENUM ch)
{	
		volatile	API_DMA_RecoverDef		recover;


		if(ch==ADC_Select_VcIc)
		{	
//			recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc1_T12A,1);
//			recover.DstAddress=(uint32_t)TxA_ADC_TimDmaBuff.T1A;
//			recover.DataLength=TxA_ADC_TimBUFF_NUM;					//64*2
//			
//			API_DMA_RECOVER(ChDmaT1A, (API_DMA_RecoverDef*)(&recover));
//			
//			recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc1_T12A,2);
//			recover.DstAddress=(uint32_t)TxA_ADC_TimDmaBuff.T2A;
////			recover.DataLength=TxA_ADC_DMA_BUFF_NUM;
//			
//			API_DMA_RECOVER(ChDmaT2A, (API_DMA_RecoverDef*)(&recover));
//			
//			recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc3_T34A,1);
//			recover.DstAddress=(uint32_t)TxA_ADC_TimDmaBuff.T3A;
////			recover.DataLength=TxA_ADC_DMA_BUFF_NUM;
//			
//			API_DMA_RECOVER(ChDmaT3A, (API_DMA_RecoverDef*)(&recover));
//			
//			recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc3_T34A,2);
//			recover.DstAddress=(uint32_t)TxA_ADC_TimDmaBuff.T4A;
////			recover.DataLength=TxA_ADC_DMA_BUFF_NUM;
			
//			API_DMA_RECOVER(ChDmaT4A, (API_DMA_RecoverDef*)(&recover));

//-------------------DMA 获取ADC1同步的T12值----------------------------------------------------------------------
		
			recover.SrcAddress=API_ADC_GetAddressDR(ChAdc1_T12A);
			recover.DstAddress=(uint32_t)TxA_ADC_AdcDmaBuff.T12A;
			recover.DataLength=TxA_ADC_AdcDMA_BUFF_NUM;
			
			API_DMA_RECOVER(ChDmaT12A, (API_DMA_RecoverDef*)(&recover));
			
//-------------------DMA 获取ADC1同步的T34值----------------------------------------------------------------------
			
			recover.SrcAddress=API_ADC_GetAddressDR(ChAdc3_T34A);
			recover.DstAddress=(uint32_t)TxA_ADC_AdcDmaBuff.T34A;
//			recover.DataLength=TxA_ADC_AdcDMA_BUFF_NUM;
			
			API_DMA_RECOVER(ChDmaT34A, (API_DMA_RecoverDef*)(&recover));	
//-------------------DMA 获取ADC1同步的VC值----------------------------------------------------------------------
//			recover.SrcAddress=API_ADC_GetAddressDRx(ChAdc2_VcIc,VoltageADC2_Group);
//			recover.DstAddress=(uint32_t)TxA_ADC_AdcDmaBuff.Vc;
////			recover.DataLength=TxA_ADC_AdcDMA_BUFF_NUM;
//			
//			API_DMA_RECOVER(ChDmaVc, (API_DMA_RecoverDef*)(&recover));			
//-------------------DMA 获取HRTIM的COUNT----------------------------------------------------------------------

//-------------------DMA 获取VCIC----------------------------------------------------------------------

//			recover.SrcAddress=API_ADC_GetAddressDR(ChAdc2_VcIc);
//			recover.DstAddress=(uint32_t)VcIc_ADC_Buff;
//			recover.DataLength=VcIc_ADC_DMA_BUFF_NUM;
//			
//			API_DMA_RECOVER(ChDmaVcIc, (API_DMA_RecoverDef*)(&recover));
			
			API_ADC_DMA_RecoverTxa();




		}
		if(ch==ADC_Select_Tempe)
		{	
			recover.DataLength=0;
			API_DMA_RECOVER(ChDmaVc, (API_DMA_RecoverDef*)(&recover));	
//			API_DMA_RECOVER(ChDmaHtrim, (API_DMA_RecoverDef*)(&recover));	
//			API_DMA_RECOVER(ChDmaT1A, (API_DMA_RecoverDef*)(&recover));
//			API_DMA_RECOVER(ChDmaT2A, (API_DMA_RecoverDef*)(&recover));
//			API_DMA_RECOVER(ChDmaT3A, (API_DMA_RecoverDef*)(&recover));
//			API_DMA_RECOVER(ChDmaT4A, (API_DMA_RecoverDef*)(&recover));
			
			recover.SrcAddress=API_ADC_GetAddressDR(ChAdc1_T12A);
			recover.DstAddress=(uint32_t)&Tempe_ADC_Buff;
			recover.DataLength=VcIc_ADC_BUFF_NUM;						//8个通道
			
			API_DMA_RECOVER(ChDmaT12A,(API_DMA_RecoverDef*)(&recover));
			
			recover.SrcAddress=API_ADC_GetAddressDR(ChAdc3_T34A);
			recover.DstAddress=(uint32_t)&res_ADC_Buff;
//			recover.DataLength=TxA_ADC_DMA_BUFF_NUM;
			
			API_DMA_RECOVER(ChDmaT34A, (API_DMA_RecoverDef*)(&recover));
			
//			recover.SrcAddress=API_ADC_GetAddressDR(ChAdc2_VcIc);
//			recover.DstAddress=(uint32_t)&FanAd_ADC_Buff;
//			recover.DataLength=VcIc_ADC_BUFF_NUM;
//			
//			API_DMA_RECOVER(ChDmaVcIc, (API_DMA_RecoverDef*)(&recover));

//		__HAL_DMA_DISABLE_IT(&AdcT12A_dma, (DMA_IT_TC | DMA_IT_HT | DMA_IT_TE));				
		}		

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

void 	API_ADC_Instance_RecoverCallBack(ADC_SELECT_ENUM ch)			//保存Instance设置值 		在APP_ADC中实现
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

static   uint16_t  adcBuff[2][100];
static   uint32_t		adcSize,adcSum;

#define		DEBUGOUT 1

void 	APP_ADC_TimDmaEnd(void)
{	

	uint8_t  hrtimPoint[4];	
	uint16_t  value[4][60];
	uint32_t	sum[4]={0,0,0,0};
	int16_t  avgValue;
	int16_t  newValue;
	int16_t  absDiv;
	uint8_t  hrtimP;			//有效點計數	
	uint8_t  endPoint;		//緩存計數值(當前緩存)
	uint8_t		count=AdcFromApiDma20ms.count;
	uint16_t* 	buff;			//
	static		uint16_t  csvCount=0;

		if(TxA_ADC_TimDmaBuff.flag.dmaEnd)
		{
			API_GPIO_WritePin(DebugB_pin,1);
			TxA_ADC_TimDmaBuff.flag.dmaEnd=0;

			if(AdcFromApiDma20ms.count<20)
			{
			
			hrtimPoint[0]=TxA_ADC_TimDMA_BUFF_NUM-TxA_ADC_TimDma100msBuff.Hrtim[1];
			hrtimPoint[1]=hrtimPoint[0];
			hrtimP=2;	
			do	//循環直到50us完整多個HRTIM周期
			{
				endPoint=TxA_ADC_TimDMA_BUFF_NUM-TxA_ADC_TimDma100msBuff.Hrtim[hrtimP];				
				if((endPoint-hrtimPoint[0])>50||endPoint<hrtimPoint[1])
				{
					break;	
				}
				
				hrtimPoint[1]=endPoint;
			}while ((hrtimP++)<5);
		
			
			hrtimP=0;
	
			
			for(uint8_t i=hrtimPoint[0];i<hrtimPoint[1];i++)
			{
				for(uint8_t j=0;j<1;j++)				//4個通道循環
				{
					buff=(uint16_t *)&(TxA_ADC_TimDma100msBuff.Txa[j][0]);

					avgValue=(buff[i-1]+buff[i+1])/2;
					newValue=buff[i];
				
					if(newValue>avgValue)
					{
						absDiv=	newValue-avgValue;
					}
					else
					{
						absDiv=	avgValue-newValue;
					}			
				
					if(absDiv>300)			//取前后兩數為平均值，與當前值偏差太大認為噪聲
					{
						newValue	=	avgValue;				//噪聲太大以平均值取代
					}
					value[j][hrtimP]=newValue;
					sum[j]+=newValue;

				}	
				hrtimP++;
			}
			sum[0]/=	hrtimP;		
#if	DEBUGOUT			
			if(count==0)
			{
				sum[0]=10;
			}
#endif
			
			AdcFromApiDma20ms.Txa[0][count]=sum[0];
#if	DEBUGOUT					
			adcBuff[0][count]=sum[0];
			adcSum+=sum[0];
#endif						
			sum[1]/=	hrtimP;
			AdcFromApiDma20ms.Txa[1][count]=sum[1];
			sum[2]/=	hrtimP;
			AdcFromApiDma20ms.Txa[2][count]=sum[2];
			sum[3]/=	hrtimP;
			AdcFromApiDma20ms.Txa[3][count]=sum[3];		

			

#if	DEBUGOUT		
			if(AdcFromApiDma20ms.count==20)
			{
					adcSum/=20;
					adcSize=adcSum;
					adcSum=0;
					
			}	
#endif			
#if 0		
			buff=(uint16_t *)&(TxA_ADC_TimDma100msBuff.Txa[0][hrtimPoint[0]]);
			
			if(AdcFromApiDma20ms.count==0)
			{

					for(uint8_t i=0;i<hrtimP;i++)
					{
							adcBuff[0][i]=buff[i];
							adcBuff[1][i]=value[0][i];
							adcSize=hrtimP;
							adcSum=sum[0];
					}
						
				}
	
#endif
				AdcFromApiDma20ms.count++;

		}	
		API_GPIO_WritePin(DebugB_pin,0);

				
		}	


}

int16_t *  getAdcBuffAddress(uint8_t ch)
{

	return 	(int16_t*)&(adcBuff[ch][0]);
}
int16_t   getAdcBuffSize(void)
{
	return	adcSize;
}
int16_t   getAdcBuffSum(void)
{
	return	adcSum;
}



void	API_DMA_M2M_OverCallback(void)
{	
	if(TxA_ADC_TimDmaBuff.flag.dmaStart)
	{	
	
		TxA_ADC_TimDmaBuff.flag.dmaStart=0;
		TxA_ADC_TimDmaBuff.flag.dmaEnd=1;			//在主循环中处理

	}
}

void	API_DMA_T1A_IRQHandlerCallBack(uint8_t	num)
{

		API_DMA_RecoverDef	recover;

		
		
			API_GPIO_WritePin(DebugA_pin,1);	


		if(TxA_ADC_TimDmaBuff.flag.t100ms)
		{								//获取当前100ms 的完整周期T1A数据
			TxA_ADC_TimDmaBuff.flag.t100ms=0;
			TxA_ADC_TimDmaBuff.flag.dmaStart=1;//在DMA回调中处理数据	
			recover.SrcAddress=(uint32_t)&TxA_ADC_TimDmaBuff;		//保存完整的150us数据，前面几个数据可能无效
			recover.DstAddress=(uint32_t)&TxA_ADC_TimDma100msBuff;
			recover.DataLength=sizeof(TxA_ADC_TimDmaBuff)/sizeof(int);
			API_DMA_RECOVER(ChDmaM2M, (API_DMA_RecoverDef*)(&recover));

		}


		recover.SrcAddress=(uint32_t)API_DMA_GetDmaCndtrAddress(ChDmaT1A);//交替HRTIM位置缓存
		recover.DstAddress=(uint32_t)&TxA_ADC_TimDmaBuff.Hrtim;
		recover.DataLength=Hrtim_ADC_DMA_BUFF_NUM;
			
		API_DMA_RECOVER(ChDmaHrtim, (API_DMA_RecoverDef*)(&recover));	
		API_GPIO_WritePin(DebugA_pin,0);	
}




#if 1

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
