#include "API_adc.h"
#include	"API_gpio.h"
#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include "system_init.h"
#include "system_bsp.h"
 

//表格区索引
//TxA_ADC_REG_RankInitTypeDef		ADC规则通道设置
//ADC_InitTypeDef								ADC初始化	
//ADC_AnalogWDGConfTypeDef			ADC AWD初始化
//ADC_TOTAL_InitTypeDef					ADC+DMA组合设置
//ADC_DMA_InitTypeDef						ADC DMA设置
#define SET_FREE	1
#define	PAN_AWD_0	4096*4/33
#define	PAN_AWD_1	4096*5/33
#define	PAN_AWD_2	4096*6/33
#define	PAN_AWD_3	4096*7/33

uint16_t 	const 		PAN_AWD_GROUP[4]={PAN_AWD_0,PAN_AWD_1,PAN_AWD_2,PAN_AWD_3};

__weak	void 	API_ADC_Instance_RecoverCallBack(ADC_SELECT_ENUM		ch)			//恢复Instance设置值 在APP_ADC中实现
{


}


__weak	void 	API_ADC_Instance_SaveCallBack(ADC_SELECT_ENUM ch)			//保存Instance设置值 		在APP_ADC中实现
{

}
__weak	void	API_VcIc_EOC_IRQHandlerCallBack(void)		//ADC2 EOC中断
{

}
__weak	uint8_t	API_PAN_JEOC_IRQHandlerCallBack(void)		//ADC2 EOC中断
{
	return 0;
}


__weak	void	API_T34_EOC_IRQHandlerCallBack(void)			//ADC3 EOC中断
{
}



__weak	void	API_T12_EOC_IRQHandlerCallBack(void)
{
}

__weak	void	API_ADC_Current1AWD_IRQHandlerCallBack(void)			//T12A谐振电流过流
{}
__weak	void	API_ADC_Current2AWD_IRQHandlerCallBack(void)			//T34A谐振电流过流
{}

////__weak	void	API_ADC_DMA_RecoverCallBack(ADC_SELECT_ENUM ch)	
//{

//}

__weak	uint8_t API_ADC_PanWaitEdgeCallBack(void)				//PAN检锅信号低于设定值 
{
	return 0;	
}

void	API_ADC_DISABLE_IT_AWD(void);	
void	API_ADC_ENABLE_IT_AWD(void);	
void	API_ADC_SetConfing_DMA(ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length);
void	API_ADC_Start_DMA(ADC_HandleTypeDef* hadc);	



#define	DMA_Channel_HrtimCount	DMA1_Channel5

typedef struct
{
  DMA_Channel_TypeDef    *Instance;                                                  /*!< Register base address                */
  DMA_InitTypeDef       Init;                                                        /*!< DMA communication parameters         */
  void                  *Parent;                                                     /*!< Parent object state                  */

}ADC_DMA_InitTypeDef;




typedef struct				//检锅PAN	
{
  uint32_t Length;
  ADC_ChannelConfTypeDef Rank[TxA_ADC_GROUP_NUM];
} TxA_ADC_REG_RankInitTypeDef;

typedef struct
{
  uint32_t Length;
  ADC_ChannelConfTypeDef Rank[VcIc_ADC_GROUP_NUM];
}Tempe_ADC_REG_RankInitTypeDef;

typedef struct					//谐振电流DMA缓存
{
  uint32_t Length;
  ADC_ChannelConfTypeDef Rank[Current_ADC_GROUP_NUM];
}Current_ADC_REG_RankInitTypeDef;



//ADC_HandleTypeDef			AdcTempeHandle;
//ADC_InjectionConfTypeDef    AdcT12A_Injected;

//ADC_InjectionConfTypeDef    AdcT34A_Injected;


ADC_HandleTypeDef 			AdcT12aHandle;
ADC_HandleTypeDef 			AdcT34aHandle;
ADC_HandleTypeDef 			AdcCurrentHandle;


//ADC_HandleTypeDef 			AdcFanAdHandle;



ADC_HandleTypeDef*	API_ADC_Handle[]=
{
	&AdcT12aHandle,			//adc1
	&AdcCurrentHandle,			//adc2`	
	&AdcT34aHandle,			//adc3	

	
};


		

uint32_t			AdcBuff[8];

typedef struct
{
	ADC_TypeDef*										adc;
	ADC_InitTypeDef*               	Init;                  /*!< ADC required parameters @ref HAL_ADC_InitTypeDef */
//	DMA_HandleTypeDef*             	DMA_Handle;            /*!< Pointer DMA Handler @ref DMA_HandleTypeDef */
	ADC_AnalogWDGConfTypeDef*      	Awd;
	Tempe_ADC_REG_RankInitTypeDef*		AdcRank;
	
}ADC_TOTAL_InitTypeDef;	




#if 0
分组	功能		ADCH	通道	脚位	端口
	谐振电流	T1A	ADC1_IN4	CMP1P1	12	PA0
						T2A	ADC1_IN5	CMP2P1	13	PA1
						T3A	ADC3_IN5	CMP3P1	35	PB13
						T4A	ADC3_IN6	CMP4P1	36	PB14
	母线电流	CUR1	ADC2_IN9	OPA1P1	14	PA2
						CUR2	ADC2_IN10	OPA2P1	19	PA5
						CUR3	ADC2_IN11	OPA3P1	26	PB2
						CUR4	ADC2_IN12	OPA4P1	30	PB10
	电压			VOLTAGE	ADC2_IN1		11	PC3
						
	IGBT温度	IGBT1	ADC1_IN9_H		22	PC4
						IGBT2	ADC2_IN2		23	PC5
	底部温度	TOP1	ADC1_IN13		2	PC13
						TOP2	ADC1_IN1		3	PC14
						TOP3	ADC1_IN7_H		9	PC1
						TOP4	ADC1_IN8_H		10	PC2
	风机电流	FANAD	ADC2_IN9	OPAMP1_P2	8	PC0
	检锅共用口	PAN		ADC1_IN2		4 PC15
	
#endif

enum
{
	VoltageAd_CHANNEL			=		ADC_CHANNEL_9,//ADC_CHANNEL_9,			//ADC1
 	HalfVoltage_CHANNEL						=		ADC_CHANNEL_10,					//ADC1
   PAN_CHANNEL				=		HalfVoltage_CHANNEL,		//ADC1

  Igbt1Ad_CHANNEL			=		ADC_CHANNEL_1,					//ADC2
	Igbt2Ad_CHANNEL			=		ADC_CHANNEL_2,					//ADC2
  Igbt3Ad_CHANNEL			=		ADC_CHANNEL_7,					//ADC2
	Igbt4Ad_CHANNEL			=		ADC_CHANNEL_8,					//ADC2
	
	
  Bottom1Ad_CHANNEL 		=		ADC_CHANNEL_13,					//ADC1
  Bottom2Ad_CHANNEL 		=		ADC_CHANNEL_1,					//ADC1
  Bottom3Ad_CHANNEL 		=		ADC_CHANNEL_7,					//ADC1
  Bottom4Ad_CHANNEL 		=		ADC_CHANNEL_8,					//ADC1	
	Ceil1Ad_CHANNEL				=		ADC_CHANNEL_2,
	Ceil2Ad_CHANNEL				=		ADC_CHANNEL_10,
	
	
	FanAd_CHANNEL			=		ADC_CHANNEL_9,		//ADC2  op1p2
  Current1Ad_CHANNEL		=		ADC_CHANNEL_9,		//ADC2	op1p1
  Current2Ad_CHANNEL		=		ADC_CHANNEL_10,		//ADC2	



  Current3Ad_CHANNEL		=		ADC_CHANNEL_11,		//ADC3
  Current4Ad_CHANNEL		=		ADC_CHANNEL_12,		//ADC3	
	
	T3A_CHANNEL=ADC_CHANNEL_5,					//ADC3				//检锅
  T4A_CHANNEL=ADC_CHANNEL_6,					//ADC3

  Fast_CHANNEL=ADC_CHANNEL_11,



}AdcChannelDef;




enum
{	
//--------ADC1----------------------------------------------------------
		Adc1ChannelGroup0=VoltageAd_CHANNEL,
		Adc1ChannelGroup1=HalfVoltage_CHANNEL,
		// Adc1ChannelGroup2=VoltageAd_CHANNEL,
		// Adc1ChannelGroup3=VoltageAd_CHANNEL,
		// Adc1ChannelGroup4=VoltageAd_CHANNEL,
		// Adc1ChannelGroup5=VoltageAd_CHANNEL,
		// Adc1ChannelGroup6=VoltageAd_CHANNEL,
		// Adc1ChannelGroup7=VoltageAd_CHANNEL,






		Adc1ChannelGroup0_1=			Bottom1Ad_CHANNEL,
		Adc1ChannelGroup1_1=			Bottom2Ad_CHANNEL,		
		Adc1ChannelGroup2_1=			Bottom3Ad_CHANNEL,
		Adc1ChannelGroup3_1=			Bottom4Ad_CHANNEL,
	
		Adc1ChannelGroup4_1=			Ceil1Ad_CHANNEL,		//以下保留
		Adc1ChannelGroup5_1=			Ceil2Ad_CHANNEL,		
		Adc1ChannelGroup6_1=			Bottom3Ad_CHANNEL,
		Adc1ChannelGroup7_1=			Bottom4Ad_CHANNEL,	
	

//--------ADC3----------------------------------------------------------
	
		Adc3ChannelGroup0=Current3Ad_CHANNEL,
		Adc3ChannelGroup1=Current4Ad_CHANNEL,
		Adc3ChannelGroup2=Fast_CHANNEL,
		Adc3ChannelGroup3=Fast_CHANNEL,
		Adc3ChannelGroup4=T3A_CHANNEL,
		Adc3ChannelGroup5=T4A_CHANNEL,
		Adc3ChannelGroup6=T3A_CHANNEL,
		Adc3ChannelGroup7=T4A_CHANNEL,	
	
//--------ADC2----------------------------------------------------------	
	//vc  adc2	
		Adc2ChannelGroup0=				Current1Ad_CHANNEL,	
		Adc2ChannelGroup1=				Current2Ad_CHANNEL,
		// Adc2ChannelGroup2=				Current3Ad_CHANNEL,	
		// Adc2ChannelGroup3=				Current4Ad_CHANNEL,		
		
		
		
	//tempe adc

	
		Adc2ChannelGroup0_1=				Igbt1Ad_CHANNEL,
		Adc2ChannelGroup1_1=				Igbt2Ad_CHANNEL,	
		Adc2ChannelGroup2_1=				Igbt3Ad_CHANNEL,
		Adc2ChannelGroup3_1=				Igbt4Ad_CHANNEL,	
		Adc2ChannelGroup4_1=				FanAd_CHANNEL,
		
		Adc2ChannelGroup5_1=				Igbt1Ad_CHANNEL,	
		Adc2ChannelGroup6_1=				Igbt2Ad_CHANNEL,
		Adc2ChannelGroup7_1=				Igbt4Ad_CHANNEL,


		Adc2InjectionChannelGroup0=PAN_CHANNEL,				//注入通道
		
};	

//-----------ADC_REG_RankInit	START--adcclk 64M    采样时间+12.5周期---------------------------------------------------------
#define	ADC_SAMPLINGTIME_HIGH	ADC_SAMPLINGTIME_3CYCLES_5     //3.5+12.5=16  =16/64=250ns
#define	ADC_SAMPLINGTIME_MEIDM	ADC_SAMPLINGTIME_13CYCLES_5		//13.5+12.5=31  =26/64=406ns
#define	ADC_SAMPLINGTIME_SLOW	ADC_SAMPLINGTIME_28CYCLES_5		//28.5+12.5=31  =31/64=640ns




//电压与检锅2
Current_ADC_REG_RankInitTypeDef const VcHalf_RankInitTypeDef={
	Current_ADC_GROUP_NUM,			//注意RANK要对应1234
	{
		{
			Adc1ChannelGroup0,
			1,
			ADC_SAMPLINGTIME_HIGH,	
		},
		{
			Adc1ChannelGroup1,
			2,
			ADC_SAMPLINGTIME_HIGH,		
		},
		// {
		// 	Adc1ChannelGroup1,
		// 	3,
		// 	ADC_SAMPLINGTIME_HIGH,	
		// },
		// {
		// 	Adc1ChannelGroup0,
		// 	4,
		// 	ADC_SAMPLINGTIME_HIGH,		
		// },		
		
	},	

};
#define	VcFast_RankInitTypeDef		VcHalf_RankInitTypeDef





Current_ADC_REG_RankInitTypeDef  const	T34aFast_RankInitTypeDef={
	Current_ADC_GROUP_NUM,
	{

		{
			Adc3ChannelGroup0,
			1,
			ADC_SAMPLINGTIME_HIGH,	
		},
		{
			Adc3ChannelGroup1,
			2,
			ADC_SAMPLINGTIME_HIGH,		
		},
		// {
		// 	Adc3ChannelGroup2,
		// 	3,
		// 	ADC_SAMPLINGTIME_HIGH,	
		// },
		// {
		// 	Adc3ChannelGroup3,
		// 	4,
		// 	ADC_SAMPLINGTIME_HIGH,		
		// },		

	}
};



//检锅34
Current_ADC_REG_RankInitTypeDef  const	Current_RankInitTypeDef={
	Current_ADC_GROUP_NUM,
	{

		{
			Adc2ChannelGroup0,
			1,
			ADC_SAMPLINGTIME_HIGH,	
		},
		{
			Adc2ChannelGroup1,
			2,
			ADC_SAMPLINGTIME_HIGH,		
		},
		
		
		// {
		// 	Adc2ChannelGroup2,
		// 	3,
		// 	ADC_SAMPLINGTIME_HIGH,	
		// },
		// {
		// 	Adc2ChannelGroup3,
		// 	4,
		// 	ADC_SAMPLINGTIME_HIGH,		
		// },		
	
	}
};





Tempe_ADC_REG_RankInitTypeDef  const	TEMPE_ADC1_RankInitTypeDef={
	Tempe_ADC_GROUP_NUM,
	{
		{
			Adc1ChannelGroup0_1,
			1,
			ADC_SAMPLINGTIME_SLOW,	
		},
		{
			Adc1ChannelGroup1_1,
			2,
			ADC_SAMPLINGTIME_SLOW,		
		},
#if Tempe_ADC_GROUP_NUM==8		
	
		{
			Adc1ChannelGroup2_1,
			3,
			ADC_SAMPLINGTIME_SLOW,	
		},
		{
			Adc1ChannelGroup3_1,
			4,
			ADC_SAMPLINGTIME_SLOW,		
		},		
		{
			Adc1ChannelGroup4_1,
			5,
			ADC_SAMPLINGTIME_SLOW,	
		},
		{
			Adc1ChannelGroup5_1,
			6,
			ADC_SAMPLINGTIME_SLOW,		
		},
		{
			Adc1ChannelGroup6_1,
			7,
			ADC_SAMPLINGTIME_SLOW,	
		},
		{
			Adc1ChannelGroup7_1,
			8,
			ADC_SAMPLINGTIME_SLOW,		
		},	
#endif		
	}

};

Tempe_ADC_REG_RankInitTypeDef  const	TEMPE_ADC3_RankInitTypeDef={
	Tempe_ADC_GROUP_NUM,
	{

		{
			Adc3ChannelGroup0,
			1,
			ADC_SAMPLINGTIME_SLOW,	
		},
		{
			Adc3ChannelGroup1,
			2,
			ADC_SAMPLINGTIME_SLOW,		
		},
#if Tempe_ADC_GROUP_NUM==8			
		
		{
			Adc3ChannelGroup2,
			3,
			ADC_SAMPLINGTIME_SLOW,	
		},
		{
			Adc3ChannelGroup3,
			4,
			ADC_SAMPLINGTIME_SLOW,		
		},		
		{
			Adc3ChannelGroup4,
			5,
			ADC_SAMPLINGTIME_SLOW,	
		},
		{
			Adc3ChannelGroup5,
			6,
			ADC_SAMPLINGTIME_SLOW,		
		},
		{
			Adc3ChannelGroup6,
			7,
			ADC_SAMPLINGTIME_SLOW,	
		},
		{
			Adc3ChannelGroup7,
			8,
			ADC_SAMPLINGTIME_SLOW,		
		},	
#endif		
	}
};



Tempe_ADC_REG_RankInitTypeDef const TEMPE_ADC2_RankInitTypeDef={
	Tempe_ADC_GROUP_NUM,
	{
		{
		Adc2ChannelGroup0_1,				//FAN_AD
		1,
		ADC_SAMPLINGTIME_SLOW,	
		},
		{
		Adc2ChannelGroup1_1,				//IGBT2
		2,
		ADC_SAMPLINGTIME_SLOW,		
		},
		{
		Adc2ChannelGroup2_1,
		3,
		ADC_SAMPLINGTIME_SLOW,		
		},
		{
		Adc2ChannelGroup3_1,
		4,
		ADC_SAMPLINGTIME_SLOW,		
		},
		{
		Adc2ChannelGroup4_1,
		5,
		ADC_SAMPLINGTIME_SLOW,		
		},	
#if Tempe_ADC_GROUP_NUM==8		
		{
		Adc2ChannelGroup5_1,
		6,
		ADC_SAMPLINGTIME_SLOW,		
		},
		{
		Adc2ChannelGroup6_1,
		7,
		ADC_SAMPLINGTIME_SLOW,		
		},			
		{
		Adc2ChannelGroup7_1,
		8,
		ADC_SAMPLINGTIME_SLOW,		
		},			


		
#endif		
	},
};




//-----------ADC_REG_RankInit	START-----------------------------------------------------------


//-----------ADC_Init	START//检锅与电压-----------------------------------------------------------

ADC_InitTypeDef const ADC1_InitTypeDef={		

	
  ADC_DATAALIGN_RIGHT,				/*uint32_t DataAlign;                        !< Specifies ADC data alignment to right or to left.
                                                  This parameter can be a value of @ref ADC_Data_align */
  ADC_SCAN_ENABLE,					/*uint32_t ScanConvMode;                     !< Configures the sequencer of regular and injected groups.
                                                  If disabled: Conversion is performed in single mode.
                                                  If enabled:  Conversions are performed in sequence mode
                                                  This parameter can be a value of @ref ADC_Scan_mode */
  ENABLE,							/*FunctionalState ContinuousConvMode;         !< Specifies whether the conversion is performed in single mode (one conversion) or continuous mode for regular group,
                                                  after the selected trigger occurred (software start or external trigger).
                                                  This parameter can be set to ENABLE or DISABLE. */
  Current_ADC_GROUP_NUM,								/*uint32_t NbrOfConversion;                  !< Specifies the number of ranks that will be converted within the regular group sequencer.
                                                  To use regular group sequencer and convert several ranks, parameter 'ScanConvMode' must be enabled.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  DISABLE,							/*FunctionalState  DiscontinuousConvMode;    !< Specifies whether the conversions sequence of regular group is performed in Complete-sequence/Discontinuous-sequence (main sequence subdivided in successive parts).
                                                  Discontinuous mode is used only if sequencer is enabled (parameter 'ScanConvMode'). If sequencer is disabled, this parameter is discarded.
                                                  Discontinuous mode can be enabled only if continuous mode is disabled. If continuous mode is enabled, this parameter setting is discarded.
                                                  This parameter can be set to ENABLE or DISABLE. */
  1,								/*uint32_t NbrOfDiscConversion;              !< Specifies the number of discontinuous conversions in which the  main sequence of regular group (parameter NbrOfConversion) will be subdivided.
                                                  If parameter 'DiscontinuousConvMode' is disabled, this parameter is discarded.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  ADC1_CR2_EXTSEL_TIM3_TRGO,	/*uint32_t ExternalTrigConv;                 !< Selects the external event used to trigger the conversion start of regular group.
                                                  If set to ADC1_CR2_EXTSEL_SWSTART, external triggers are disabled.
                                                  If set to external trigger source, triggering is on event rising edge.
                                                  This parameter can be a value of @ref ADC_External_trigger_Regular */
	
};	



#define		VcT1A_ADC_InitTypeDef	ADC1_InitTypeDef		
#define		VcT2A_ADC_InitTypeDef	ADC1_InitTypeDef		

//检锅
ADC_InitTypeDef const ADC3_InitTypeDef={	

	
  ADC_DATAALIGN_RIGHT,				/*uint32_t DataAlign;                        !< Specifies ADC data alignment to right or to left.
                                                  This parameter can be a value of @ref ADC_Data_align */
  ADC_SCAN_ENABLE,					/*uint32_t ScanConvMode;                     !< Configures the sequencer of regular and injected groups.
                                                  If disabled: Conversion is performed in single mode.
                                                  If enabled:  Conversions are performed in sequence mode
                                                  This parameter can be a value of @ref ADC_Scan_mode */
  ENABLE,							/*FunctionalState ContinuousConvMode;         !< Specifies whether the conversion is performed in single mode (one conversion) or continuous mode for regular group,
                                                  after the selected trigger occurred (software start or external trigger).
                                                  This parameter can be set to ENABLE or DISABLE. */
  Current_ADC_GROUP_NUM,								/*uint32_t NbrOfConversion;                  !< Specifies the number of ranks that will be converted within the regular group sequencer.
                                                  To use regular group sequencer and convert several ranks, parameter 'ScanConvMode' must be enabled.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  DISABLE,							/*FunctionalState  DiscontinuousConvMode;    !< Specifies whether the conversions sequence of regular group is performed in Complete-sequence/Discontinuous-sequence (main sequence subdivided in successive parts).
                                                  Discontinuous mode is used only if sequencer is enabled (parameter 'ScanConvMode'). If sequencer is disabled, this parameter is discarded.
                                                  Discontinuous mode can be enabled only if continuous mode is disabled. If continuous mode is enabled, this parameter setting is discarded.
                                                  This parameter can be set to ENABLE or DISABLE. */
  1,								/*uint32_t NbrOfDiscConversion;              !< Specifies the number of discontinuous conversions in which the  main sequence of regular group (parameter NbrOfConversion) will be subdivided.
                                                  If parameter 'DiscontinuousConvMode' is disabled, this parameter is discarded.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
 ADC3_CR2_EXTSEL_TIM3_TRGO,	/*// ADC3_CR2_EXTSEL_hrtim_adc_trg5,	uint32_t ExternalTrigConv;                 !< Selects the external event used to trigger the conversion start of regular group.
                                                  If set to ADC1_CR2_EXTSEL_SWSTART, external triggers are disabled.
                                                  If set to external trigger source, triggering is on event rising edge.
                                                  This parameter can be a value of @ref ADC_External_trigger_Regular */
	
};	
	

#define		T34A_ADC_InitTypeDef	ADC3_InitTypeDef	



ADC_InitTypeDef const ADC2_InitTypeDef={

	
  ADC_DATAALIGN_RIGHT,				/*uint32_t DataAlign;                        !< Specifies ADC data alignment to right or to left.
                                                  This parameter can be a value of @ref ADC_Data_align */
  ADC_SCAN_ENABLE,					/*uint32_t ScanConvMode;                     !< Configures the sequencer of regular and injected groups.
                                                  If disabled: Conversion is performed in single mode.
                                                  If enabled:  Conversions are performed in sequence mode
                                                  This parameter can be a value of @ref ADC_Scan_mode */
  ENABLE,							/*FunctionalState ContinuousConvMode;         !< Specifies whether the conversion is performed in single mode (one conversion) or continuous mode for regular group,
                                                  after the selected trigger occurred (software start or external trigger).
                                                  This parameter can be set to ENABLE or DISABLE. */
  Current_ADC_GROUP_NUM,								/*uint32_t NbrOfConversion;                  !< Specifies the number of ranks that will be converted within the regular group sequencer.
                                                  To use regular group sequencer and convert several ranks, parameter 'ScanConvMode' must be enabled.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  DISABLE,							/*FunctionalState  DiscontinuousConvMode;    !< Specifies whether the conversions sequence of regular group is performed in Complete-sequence/Discontinuous-sequence (main sequence subdivided in successive parts).
                                                  Discontinuous mode is used only if sequencer is enabled (parameter 'ScanConvMode'). If sequencer is disabled, this parameter is discarded.
                                                  Discontinuous mode can be enabled only if continuous mode is disabled. If continuous mode is enabled, this parameter setting is discarded.
                                                  This parameter can be set to ENABLE or DISABLE. */
  1,								/*uint32_t NbrOfDiscConversion;              !< Specifies the number of discontinuous conversions in which the  main sequence of regular group (parameter NbrOfConversion) will be subdivided.
                                                  If parameter 'DiscontinuousConvMode' is disabled, this parameter is discarded.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
 ADC2_CR2_EXTSEL_TIM3_TRGO,/*ADC2_CR2_EXTSEL_TIM6_TRGO,	// ADC3_CR2_EXTSEL_hrtim_adc_trg5,	uint32_t ExternalTrigConv;                 !< Selects the external event used to trigger the conversion start of regular group.
                                                  If set to ADC1_CR2_EXTSEL_SWSTART, external triggers are disabled.
                                                  If set to external trigger source, triggering is on event rising edge.
                                                  This parameter can be a value of @ref ADC_External_trigger_Regular */
	
};	
	

#define		Current_ADC_InitTypeDef	ADC2_InitTypeDef	





ADC_InitTypeDef const ADC1_1_InitTypeDef={

	
  ADC_DATAALIGN_RIGHT,				/*uint32_t DataAlign;                        !< Specifies ADC data alignment to right or to left.
                                                  This parameter can be a value of @ref ADC_Data_align */
  ADC_SCAN_ENABLE,					/*uint32_t ScanConvMode;                     !< Configures the sequencer of regular and injected groups.
                                                  If disabled: Conversion is performed in single mode.
                                                  If enabled:  Conversions are performed in sequence mode
                                                  This parameter can be a value of @ref ADC_Scan_mode */
	ENABLE,							/*FunctionalState ContinuousConvMode;         !< Specifies whether the conversion is performed in single mode (one conversion) or continuous mode for regular group,
                                                  after the selected trigger occurred (software start or external trigger).
                                                  This parameter can be set to ENABLE or DISABLE. */
  Tempe_ADC_GROUP_NUM,								/*uint32_t NbrOfConversion;                  !< Specifies the number of ranks that will be converted within the regular group sequencer.
                                                  To use regular group sequencer and convert several ranks, parameter 'ScanConvMode' must be enabled.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  DISABLE,							/*FunctionalState  DiscontinuousConvMode;    !< Specifies whether the conversions sequence of regular group is performed in Complete-sequence/Discontinuous-sequence (main sequence subdivided in successive parts).
                                                  Discontinuous mode is used only if sequencer is enabled (parameter 'ScanConvMode'). If sequencer is disabled, this parameter is discarded.
                                                  Discontinuous mode can be enabled only if continuous mode is disabled. If continuous mode is enabled, this parameter setting is discarded.
                                                  This parameter can be set to ENABLE or DISABLE. */
  1,								/*uint32_t NbrOfDiscConversion;              !< Specifies the number of discontinuous conversions in which the  main sequence of regular group (parameter NbrOfConversion) will be subdivided.
                                                  If parameter 'DiscontinuousConvMode' is disabled, this parameter is discarded.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  ADC1_CR2_EXTSEL_TIM3_TRGO,	/*uint32_t ExternalTrigConv;                 !< Selects the external event used to trigger the conversion start of regular group.
                                                  If set to ADC1_CR2_EXTSEL_SWSTART, external triggers are disabled.
                                                  If set to external trigger source, triggering is on event rising edge.
                                                  This parameter can be a value of @ref ADC_External_trigger_Regular */
	
};	

#define		TEMPE_ADC1_InitTypeDef	ADC1_1_InitTypeDef		




ADC_InitTypeDef const ADC3_1_InitTypeDef={

	
  ADC_DATAALIGN_RIGHT,				/*uint32_t DataAlign;                        !< Specifies ADC data alignment to right or to left.
                                                  This parameter can be a value of @ref ADC_Data_align */
  ADC_SCAN_ENABLE,					/*uint32_t ScanConvMode;                     !< Configures the sequencer of regular and injected groups.
                                                  If disabled: Conversion is performed in single mode.
                                                  If enabled:  Conversions are performed in sequence mode
                                                  This parameter can be a value of @ref ADC_Scan_mode */
	DISABLE,							/*FunctionalState ContinuousConvMode;         !< Specifies whether the conversion is performed in single mode (one conversion) or continuous mode for regular group,
                                                  after the selected trigger occurred (software start or external trigger).
                                                  This parameter can be set to ENABLE or DISABLE. */
  Tempe_ADC_GROUP_NUM,								/*uint32_t NbrOfConversion;                  !< Specifies the number of ranks that will be converted within the regular group sequencer.
                                                  To use regular group sequencer and convert several ranks, parameter 'ScanConvMode' must be enabled.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  DISABLE,							/*FunctionalState  DiscontinuousConvMode;    !< Specifies whether the conversions sequence of regular group is performed in Complete-sequence/Discontinuous-sequence (main sequence subdivided in successive parts).
                                                  Discontinuous mode is used only if sequencer is enabled (parameter 'ScanConvMode'). If sequencer is disabled, this parameter is discarded.
                                                  Discontinuous mode can be enabled only if continuous mode is disabled. If continuous mode is enabled, this parameter setting is discarded.
                                                  This parameter can be set to ENABLE or DISABLE. */
  1,								/*uint32_t NbrOfDiscConversion;              !< Specifies the number of discontinuous conversions in which the  main sequence of regular group (parameter NbrOfConversion) will be subdivided.
                                                  If parameter 'DiscontinuousConvMode' is disabled, this parameter is discarded.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  ADC3_CR2_EXTSEL_TIM3_TRGO,	/*uint32_t ExternalTrigConv;                 !< Selects the external event used to trigger the conversion start of regular group.
                                                  If set to ADC1_CR2_EXTSEL_SWSTART, external triggers are disabled.
                                                  If set to external trigger source, triggering is on event rising edge.
                                                  This parameter can be a value of @ref ADC_External_trigger_Regular */
	
};	

#define		TEMPE_ADC3_InitTypeDef	ADC3_1_InitTypeDef		





ADC_InitTypeDef const ADC2_1_InitTypeDef={

	
  ADC_DATAALIGN_RIGHT,				/*uint32_t DataAlign;                        !< Specifies ADC data alignment to right or to left.
                                                  This parameter can be a value of @ref ADC_Data_align */
  ADC_SCAN_ENABLE,					/*uint32_t ScanConvMode;                     !< Configures the sequencer of regular and injected groups.
                                                  If disabled: Conversion is performed in single mode.
                                                  If enabled:  Conversions are performed in sequence mode
                                                  This parameter can be a value of @ref ADC_Scan_mode */
  DISABLE,							/*FunctionalState ContinuousConvMode;         !< Specifies whether the conversion is performed in single mode (one conversion) or continuous mode for regular group,
                                                  after the selected trigger occurred (software start or external trigger).
                                                  This parameter can be set to ENABLE or DISABLE. */
  Tempe_ADC_GROUP_NUM,								/*uint32_t NbrOfConversion;                  !< Specifies the number of ranks that will be converted within the regular group sequencer.
                                                  To use regular group sequencer and convert several ranks, parameter 'ScanConvMode' must be enabled.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  DISABLE,							/*FunctionalState  DiscontinuousConvMode;    !< Specifies whether the conversions sequence of regular group is performed in Complete-sequence/Discontinuous-sequence (main sequence subdivided in successive parts).
                                                  Discontinuous mode is used only if sequencer is enabled (parameter 'ScanConvMode'). If sequencer is disabled, this parameter is discarded.
                                                  Discontinuous mode can be enabled only if continuous mode is disabled. If continuous mode is enabled, this parameter setting is discarded.
                                                  This parameter can be set to ENABLE or DISABLE. */
  1,								/*uint32_t NbrOfDiscConversion;              !< Specifies the number of discontinuous conversions in which the  main sequence of regular group (parameter NbrOfConversion) will be subdivided.
                                                  If parameter 'DiscontinuousConvMode' is disabled, this parameter is discarded.
                                                  This parameter must be a number between Min_Data = 1 and Max_Data = 8. */
  ADC2_CR2_EXTSEL_TIM3_TRGO,	/*uint32_t ExternalTrigConv;                 !< Selects the external event used to trigger the conversion start of regular group.
                                                  If set to ADC1_CR2_EXTSEL_SWSTART, external triggers are disabled.
                                                  If set to external trigger source, triggering is on event rising edge.
                                                  This parameter can be a value of @ref ADC_External_trigger_Regular */
	
};	
#define	TEMPE_ADC2_InitTypeDef	ADC2_1_InitTypeDef
//-----------ADC_Init	END-----------------------------------------------------------

//-----------ADC_AnalogWDG	START-----------------------------------------------------------

ADC_AnalogWDGConfTypeDef	const Current_AnalogWDGConfTypeDef={
	
	ADC_ANALOGWATCHDOG_ALL_REG, 	/*uint32_t WatchdogMode;      !< Configures the ADC analog watchdog mode: single/all channels, regular/injected group.
                                   This parameter can be a value of @ref ADC_analog_watchdog_mode. */
	ADC_CHANNEL_1,				/*uint32_t Channel;           !< Selects which ADC channel to monitor by analog watchdog.
                                   This parameter has an effect only if watchdog mode is configured on single channel (parameter WatchdogMode)
                                   This parameter can be a value of @ref ADC_channels. */
	ENABLE,						/*FunctionalState  ITMode;    !< Specifies whether the analog watchdog is configured in interrupt or polling mode.
                                   This parameter can be set to ENABLE or DISABLE */
	0xFF0,				/*uint32_t HighThreshold;     !< Configures the ADC analog watchdog High threshold value.
                                   This parameter must be a number between Min_Data = 0x000 and Max_Data = 0xFFF. */
	(4096 * 0/8),					/*uint32_t LowThreshold;      !< Configures the ADC analog watchdog High threshold value.
                                   This parameter must be a number between Min_Data = 0x000 and Max_Data = 0xFFF. */
	ADC_AWD_FILTERING_NONE,		/*uint32_t WatchdogFilter;    !< Configures the ADC analog watchdog filter
                                   This parameter can a value of @ref ADC_analog_watchdog_filter.  */
 
};
#define	T34A_AnalogWDGConfTypeDef	Current_AnalogWDGConfTypeDef



ADC_AnalogWDGConfTypeDef	const TEMPE_AnalogWDGConfTypeDef={
	
	ADC_ANALOGWATCHDOG_NONE, 	/*uint32_t WatchdogMode;      !< Configures the ADC analog watchdog mode: single/all channels, regular/injected group.
                                   This parameter can be a value of @ref ADC_analog_watchdog_mode. */
	PAN_CHANNEL,				/*uint32_t Channel;           !< Selects which ADC channel to monitor by analog watchdog.
                                   This parameter has an effect only if watchdog mode is configured on single channel (parameter WatchdogMode)
                                   This parameter can be a value of @ref ADC_channels. */
	DISABLE,								/*FunctionalState  ITMode;    !< Specifies whether the analog watchdog is configured in interrupt or polling mode.
                                   This parameter can be set to ENABLE or DISABLE */
	0xf00,					/*uint32_t HighThreshold;     !< Configures the ADC analog watchdog High threshold value.
                                   This parameter must be a number between Min_Data = 0x000 and Max_Data = 0xFFF. */
	(4096 * 0/8),					/*uint32_t LowThreshold;      !< Configures the ADC analog watchdog High threshold value.
                                   This parameter must be a number between Min_Data = 0x000 and Max_Data = 0xFFF. */
	ADC_AWD_FILTERING_NONE,		/*uint32_t WatchdogFilter;    !< Configures the ADC analog watchdog filter
                                   This parameter can a value of @ref ADC_analog_watchdog_filter.  */
 
};


#define	VcT1A_AnalogWDGConfTypeDef	TEMPE_AnalogWDGConfTypeDef
#define	VcT2A_AnalogWDGConfTypeDef	TEMPE_AnalogWDGConfTypeDef







//-----------ADC_AnalogWDG	END-----------------------------------------------------------

//-----------ADC_TOTAL_InitTypeDef	START-----------------------------------------------------------


typedef struct
{
	ADC_TOTAL_InitTypeDef*	t12a;			//adc1
	ADC_TOTAL_InitTypeDef*	t34a;			//adc3
	ADC_TOTAL_InitTypeDef*	current;		//adc2	
}ADC_TOTAL_GROUP_DEF;

typedef struct
{
	uint32_t* t12a;
	uint32_t* t34a;	
	uint32_t* current;	
}ADC_TOTAL_BUFF_DEF;




ADC_TOTAL_InitTypeDef	VcHalf_ADC_TOTAL={

	ADC1,
	(ADC_InitTypeDef*)&VcT1A_ADC_InitTypeDef,
//	&AdcT12A_dma,
	0,//(ADC_AnalogWDGConfTypeDef*)&VcT1A_AnalogWDGConfTypeDef,//没有AWG
	(Tempe_ADC_REG_RankInitTypeDef*)&VcHalf_RankInitTypeDef,

};




ADC_TOTAL_InitTypeDef	T34A_ADC_TOTAL={

	ADC3,
	(ADC_InitTypeDef*)&T34A_ADC_InitTypeDef,
//	&AdcT34A_dma,
	(ADC_AnalogWDGConfTypeDef*)&T34A_AnalogWDGConfTypeDef,//有AWG
	(Tempe_ADC_REG_RankInitTypeDef*)&T34aFast_RankInitTypeDef,

};
ADC_TOTAL_InitTypeDef	Current_ADC_TOTAL={

	ADC2,
	(ADC_InitTypeDef*)&Current_ADC_InitTypeDef,
//	&AdcVcIc_dma,
	(ADC_AnalogWDGConfTypeDef*)&Current_AnalogWDGConfTypeDef,		//带AWG
	(Tempe_ADC_REG_RankInitTypeDef*)&Current_RankInitTypeDef,

};

ADC_TOTAL_InitTypeDef	TEMPE_ADC1_TOTAL={

	ADC1,
	(ADC_InitTypeDef*)&TEMPE_ADC1_InitTypeDef,							//单次采集
//	&AdcT12A_dma,
	0,//(ADC_AnalogWDGConfTypeDef*)&TEMPE_AnalogWDGConfTypeDef,	//关闭AWG
	(Tempe_ADC_REG_RankInitTypeDef*)&TEMPE_ADC1_RankInitTypeDef,		//切换成TEMPE通道设置 

};


ADC_TOTAL_InitTypeDef	TEMPE_ADC3_TOTAL={

	ADC3,
	(ADC_InitTypeDef*)&TEMPE_ADC3_InitTypeDef,							//单次采集
//	&AdcT34A_dma,
	0,//(ADC_AnalogWDGConfTypeDef*)&TEMPE_AnalogWDGConfTypeDef,	//关闭AWG
	(Tempe_ADC_REG_RankInitTypeDef*)&TEMPE_ADC3_RankInitTypeDef,		//还是T34,单只采一次

};

ADC_TOTAL_InitTypeDef	TEMPE_ADC2_TOTAL={

	ADC2,
	(ADC_InitTypeDef*)&TEMPE_ADC2_InitTypeDef,							//单次采集
//	&AdcVcIc_dma,
	0,//(ADC_AnalogWDGConfTypeDef*)&VcIc_AnalogWDGConfTypeDef,				//关闭AWG
	(Tempe_ADC_REG_RankInitTypeDef*)&TEMPE_ADC2_RankInitTypeDef,			//还是T34,单只采一次

};


ADC_TOTAL_GROUP_DEF	AdcTotalGroup[3]={
	{//正常电压采样，检锅为T1A
		&VcHalf_ADC_TOTAL,
		&T34A_ADC_TOTAL,
		&Current_ADC_TOTAL,
	},
	{//热敏电阻采样
		&TEMPE_ADC1_TOTAL,
		&TEMPE_ADC3_TOTAL,
		&TEMPE_ADC2_TOTAL,
	},	

	
};	
	
//-----------ADC_TOTAL_InitTypeDef	END-----------------------------------------------------------

//-----------ADC_DMA_InitTypeDef	START-----------------------------------------------------------






// ADC_DMA_InitTypeDef	ADC3_HrtimCount_DMA_InitType={
// 	DMA_Channel_HrtimCount,
// 	{
//   DMA_REQUEST_ADC3,/*uint32_t Request;  这里要验证  !< Specifies the request selected for the specified channel.
//                                            This parameter can be a value of @ref DMA_request */

//   DMA_PERIPH_TO_MEMORY,/*uint32_t Direction;                 !< Specifies if the data will be transferred from memory to peripheral,
//                                            from memory to memory or from peripheral to memory.
//                                            This parameter can be a value of @ref DMA_Data_transfer_direction */

//   DMA_PINC_DISABLE,/*uint32_t PeriphInc;                 !< Specifies whether the Peripheral address register should be incremented or not.
//                                            This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

//   DMA_MINC_ENABLE,/*uint32_t MemInc;                    !< Specifies whether the memory address register should be incremented or not.
//                                            This parameter can be a value of @ref DMA_Memory_incremented_mode */

//   DMA_PDATAALIGN_WORD,/*uint32_t PeriphDataAlignment;       !< Specifies the Peripheral data width.
//                                            This parameter can be a value of @ref DMA_Peripheral_data_size */

//   DMA_MDATAALIGN_WORD,/*uint32_t MemDataAlignment;          !< Specifies the Memory data width.
//                                            This parameter can be a value of @ref DMA_Memory_data_size */

//   DMA_CIRCULAR,/*uint32_t Mode;                     !< Specifies the operation mode of the DMAy Channelx.
//                                            This parameter can be a value of @ref DMA_mode
//                                            @note The circular buffer mode cannot be used if the memory-to-memory
//                                                  data transfer is configured on the selected Channel */

//   DMA_PRIORITY_LOW,/*uint32_t Priority;                  !< Specifies the software priority for the DMAy Channelx.*/
 	
	
	
// 	},
// 	(void*)0,

// };

// #define	HrtimCount_DMA_InitType	ADC3_HrtimCount_DMA_InitType




__weak void	Error_Handler(void)
{
	
}	

void HAL_ADC_MspPostInit(ADC_HandleTypeDef *hadc)
{
	
#if SET_FREE==0  	
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  if(hadc->Instance == ADC1)
  {
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_PUPDR_Floating;
    GPIO_InitStruct.Speed = GPIO_OSPEEDR_Very_High;
    GPIO_InitStruct.Alternate = GPIO_AF0;
    GPIO_InitStruct.Pin = GPIO_PIN_0;         //ADC1_IN4 -> PA0
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_1;         //ADC1_IN5 -> PA1
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
  else if(hadc->Instance == ADC2)
  {
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_PUPDR_Floating;
    GPIO_InitStruct.Speed = GPIO_OSPEEDR_Very_High;
    GPIO_InitStruct.Alternate = GPIO_AF0;
    GPIO_InitStruct.Pin = GPIO_PIN_2;         //ADC2_IN9 -> PA2
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_5;         //ADC2_IN10 -> PA5
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_2;         //ADC2_IN11 -> PB2
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_10;         //ADC2_IN12 -> PB10
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_4;         //ADC2_IN1 -> PC3
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  }
  else if(hadc->Instance == ADC3)
  {
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_PUPDR_Floating;
    GPIO_InitStruct.Speed = GPIO_OSPEEDR_Very_High;
    GPIO_InitStruct.Alternate = GPIO_AF0;
    GPIO_InitStruct.Pin = GPIO_PIN_13;         //ADC3_IN5 -> PB13
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_14;         //ADC3_IN6 -> PB14
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  }
	
	
#endif	
}

void	API_ADC_DMA_Init(DMA_HandleTypeDef* dmaStr,ADC_DMA_InitTypeDef* adcDmaConst)
{
		dmaStr->Instance=adcDmaConst->Instance;
		(dmaStr->Init)=(adcDmaConst->Init);
		dmaStr->Parent=adcDmaConst->Parent;

		HAL_DMA_Init(dmaStr);

}	


//-------------------------------------------------
//void	API_ADC_DMA_HrtimCount_Init(uint32_t* dr)//ADC3触发HRTIM CNT 读取DMA
//{
//	HAL_DMA_Start(&HrtimCount_dma,(uint32_t)dr,(uint32_t)&(TxA_ADC_DmaBuff.HRTIM),TxA_ADC_GROUP_NUM*2);
//}	


void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  
	
	
  
  HAL_ADC_MspPostInit(hadc);
  
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  
  if(hadc->Instance == ADC1)
  {
    __HAL_RCC_ADC1_CLK_ENABLE();
    
    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
 #if 0   
    adc1_dma.Instance                 = DMA1_Channel1;
    adc1_dma.Init.Request             = DMA_REQUEST_ADC1;
    adc1_dma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    adc1_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
    adc1_dma.Init.MemInc              = DMA_MINC_ENABLE;
    adc1_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    adc1_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    adc1_dma.Init.Mode                = DMA_CIRCULAR;
    adc1_dma.Init.Priority            = DMA_PRIORITY_LOW;
    adc1_dma.Parent                   = &adc1;
    
    HAL_DMA_Init(&adc1_dma);
#endif
    
//	API_ADC_DMA_Init(&AdcT12A_dma,&T12A_DMA_InitType);
		
		
    HAL_NVIC_SetPriority(ADC1_2_IRQn, IRQ_PRIORITY_ADC1_2, 1);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
  }
  else if(hadc->Instance == ADC2)
  {
    __HAL_RCC_ADC2_CLK_ENABLE();
    
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
#if 0    
    adc2_dma.Instance                 = DMA1_Channel3;
    adc2_dma.Init.Request             = DMA_REQUEST_ADC2;
    adc2_dma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    adc2_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
    adc2_dma.Init.MemInc              = DMA_MINC_ENABLE;
    adc2_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    adc2_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    adc2_dma.Init.Mode                = DMA_CIRCULAR;
    adc2_dma.Init.Priority            = DMA_PRIORITY_LOW;
    adc2_dma.Parent                   = &adc2;
    
    HAL_DMA_Init(&adc2_dma);
#endif		
//		API_ADC_DMA_Init(&AdcVcIc_dma,&VcIc_DMA_InitType);
		
  }
  else if(hadc->Instance == ADC3)
  {
    __HAL_RCC_ADC3_CLK_ENABLE();
    
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
#if SET_FREE==0    
    AdcT34A_dma.Instance                 = DMA1_Channel2;
    AdcT34A_dma.Init.Request             = DMA_REQUEST_ADC3;
    AdcT34A_dma.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    AdcT34A_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
    AdcT34A_dma.Init.MemInc              = DMA_MINC_ENABLE;
    AdcT34A_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    AdcT34A_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    AdcT34A_dma.Init.Mode                = DMA_CIRCULAR;
    AdcT34A_dma.Init.Priority            = DMA_PRIORITY_LOW;
    AdcT34A_dma.Parent                   = &AdcT34aHandle;
    
    HAL_DMA_Init(&AdcT34A_dma);
#else

//	API_ADC_DMA_Init(&AdcT34A_dma,&T34A_DMA_InitType);
#endif  	  
//	API_ADC_DMA_Init(&HrtimCount_dma,&HrtimCount_DMA_InitType);






  HAL_NVIC_SetPriority(ADC3_IRQn, IRQ_PRIORITY_ADC3, 1);
  HAL_NVIC_EnableIRQ(ADC3_IRQn);
  }
  
  
}

//HAL_StatusTypeDef       HAL_ADCEx_InjectedConfigChannel(ADC_HandleTypeDef* hadc,ADC_InjectionConfTypeDef* sConfigInjected);



ADC_InjectionConfTypeDef	const	Pan_InjectionConfTypeDef=
{

  Adc2InjectionChannelGroup0,/*uint32_t InjectedChannel;                       !< Selection of ADC channel to configure
                                                       This parameter can be a value of @ref ADC_channels */
  ADC_INJECTED_RANK_1,/*uint32_t InjectedRank;                          !< Rank in the injected group sequencer
                                                       This parameter must be a value of @ref ADCEx_injected_rank*/
  ADC_SAMPLINGTIME_HIGH,/*uint32_t InjectedSamplingTime;                  !< Sampling time value to be set for the selected channel.
                                                       Unit: ADC clock cycles
                                                       Conversion time is the addition of sampling time and processing time (12.5 ADC clock cycles at ADC resolution 12 bits).
                                                       This parameter can be a value of @ref ADC_sampling_times */
  0,/*uint32_t InjectedOffset;                        !< Defines the offset to be subtracted from the raw converted data (for channels set on injected group only).
                                                       this parameter must be a number between Min_Data = 0x000 and Max_Data = 0xFFF. */
  1,/*uint32_t InjectedNbrOfConversion;               !< Specifies the number of ranks that will be converted within the injected group sequencer.
                                                       To use the injected group sequencer and convert several ranks, parameter 'ScanConvMode' must be enabled.
                                                       This parameter must be a number between Min_Data = 1 and Max_Data = 4. */
  DISABLE,/*FunctionalState InjectedDiscontinuousConvMode;  !< Specifies whether the conversions sequence of injected group is performed in Complete-sequence/Discontinuous-sequence (main sequence subdivided in successive parts).
                                                       Discontinuous mode is used only if sequencer is enabled (parameter 'ScanConvMode'). If sequencer is disabled, this parameter is discarded.
                                                       Discontinuous mode can be enabled only if continuous mode is disabled. If continuous mode is enabled, this parameter setting is discarded.
                                                       This parameter can be set to ENABLE or DISABLE. */
  DISABLE,/*FunctionalState AutoInjectedConv;               !< Enables or disables the selected ADC automatic injected group conversion after regular one
                                                       This parameter can be set to ENABLE or DISABLE. */
  ADC2_CR2_JEXTSEL_TIM3_TRGO,/*uint32_t ExternalTrigInjecConv;                 !< Selects the external event used to trigger the conversion start of injected group.
                                                       If set to ADC1_CR2_JEXTSEL_JSWSTART, external triggers are disabled.
                                                       If set to external trigger source, triggering is on event rising edge.
                                                       This parameter can be a value of @ref ADCEx_External_trigger_source_Injected */




};



void	API_ADC_InjectedConfig(void)
{

	HAL_ADCEx_InjectedConfigChannel(&AdcCurrentHandle,(ADC_InjectionConfTypeDef*)&Pan_InjectionConfTypeDef);

//	__HAL_ADC_ENABLE_IT(&AdcCurrentHandle, ADC_IT_JEOC);
	HAL_ADCEx_InjectedStart(&AdcCurrentHandle);
}
	



/**
 * @brief  切换ADC1 Regular Group 序列
 *         panMode=1: SQ1=PAN, SQ2=PAN  (检锅模式)
 *         panMode=0: SQ1=VC,  SQ2=PAN  (正常模式)
 */
void API_ADC_SwitchPanSequence(uint8_t panMode)
{
    if (panMode) {
        /* SQ1 = PAN(CH10) */
        AdcT12aHandle.Instance->SQR3 &= ~ADC_SQR3_SQ1_Msk;
        AdcT12aHandle.Instance->SQR3 |= (PAN_CHANNEL << ADC_SQR3_SQ1_Pos);
    } else {
        /* SQ1 = VC(CH9) */
        AdcT12aHandle.Instance->SQR3 &= ~ADC_SQR3_SQ1_Msk;
        AdcT12aHandle.Instance->SQR3 |= (VoltageAd_CHANNEL << ADC_SQR3_SQ1_Pos);
    }
}








void	API_ADCx_Init(ADC_HandleTypeDef*	adcx,ADC_TOTAL_InitTypeDef* adcTotal)
{

	ADC_ChannelConfTypeDef   sConfig;
	ADC_AnalogWDGConfTypeDef AnalogWDGConfig;  
  
	
  /* ***************************************ADC1 Regular*************************************** */
  //ADC1 Regular, Resonance Current: T1A, T2A
	adcx->Instance = adcTotal->adc;
	

//	adcx->DMA_Handle = adcTotal->DMA_Handle;
  	
	__HAL_ADC_DISABLE(adcx);				//先关闭ADC
	
	if(adcTotal->Init)
	{
		adcx->Init=*(adcTotal->Init);
		if (HAL_ADC_Init(adcx) != HAL_OK)
		{
      /* ADC initialization error */
			Error_Handler();
		}
	}

  for(uint8_t i=0;i<adcTotal->AdcRank->Length;i++)
  {
	sConfig=adcTotal->AdcRank->Rank[i];
	  
	if(sConfig.Channel)
	{	
		if (HAL_ADC_ConfigChannel(adcx, &sConfig) != HAL_OK)
		{
			Error_Handler();
		}
	}	
  }
  

  /*ADC1, Analog watchdog 1 configuration */
//  AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_ALL_REG;

	if(adcTotal->Awd)
	{
		AnalogWDGConfig=*(adcTotal->Awd);
  
		if (HAL_ADC_AnalogWDGConfig(adcx, &AnalogWDGConfig) != HAL_OK)
		{
      /* Channel Configuration Error */
			Error_Handler();
		}
	}
	else			//不需要ADC AWG
	{
		__HAL_ADC_DISABLE_IT(adcx, ADC_IT_AWD);
	}	

	
}

typedef	struct 
{
	uint32_t cr1;
	uint32_t cr2;
	uint32_t cr3;	
	uint32_t sqr1;	
	uint32_t sqr2;
	uint32_t sqr3;
	uint32_t smpr1;
	uint32_t smpr2;
	uint32_t sr;	
	
}ADcInstanceRegSave_Def;

typedef	struct 
{
	ADC_TypeDef		AdcSave[ChAdc_Max];
	
}ADcInstanceSave_Def;


ADcInstanceSave_Def		AdcInstanceSave[ADC_Select_Max];		//ch0 vcic ch1 tempe






void	API_ADC_GROUP_INIT(uint8_t ch)							//CH0  采集T1234 CH1 采集TEMPE
{


	
		API_ADCx_Init(&AdcT12aHandle,AdcTotalGroup[ch].t12a);		//连续采集 关AWG，采集VC T12其中之一, 
		API_ADCx_Init(&AdcT34aHandle,AdcTotalGroup[ch].t34a);		//连续采集 开AWG，采集T34, T34不变
		API_ADCx_Init(&AdcCurrentHandle,AdcTotalGroup[ch].current);	//单次采集 关AWG	CUR VOLTAGE 		



			
		API_ADC_Start_DMA(&AdcT12aHandle);			//为了打开ADC DMAEN
		API_ADC_Start_DMA(&AdcT34aHandle);
		API_ADC_Start_DMA(&AdcCurrentHandle);


		API_ADC_Instance_SaveCallBack(ch);




}	

ADC_M2M_RecoverDef	API_ADC_GetAddressInstance(ADC_CH_ENUM ch)
{
	ADC_M2M_RecoverDef	xReturn;
	
	ADC_TypeDef*	point;
	xReturn.ch=ch;

	point=(API_ADC_Handle[ch]->Instance);
		xReturn.InstanceAddress=(uint32_t *)point;
//	for(uint8_t i=0;i<ADC_Select_Max;i++)
//	{
//				xReturn.SaveAddress[i]=(uint32_t *)&(AdcInstanceSave[i].AdcSave[ch]);
//	}
	xReturn.SaveAddress[0]=(uint32_t *)&(AdcInstanceSave[0].AdcSave[ch]);
	xReturn.SaveAddress[1]=(uint32_t *)&(AdcInstanceSave[1].AdcSave[ch]);


	xReturn.DataLength=sizeof(ADC_TypeDef)/sizeof(uint32_t);
	return 	xReturn;
}

void	API_ADC_StopAllAdc(void)
{
    __HAL_ADC_DISABLE(&AdcT12aHandle);
    __HAL_ADC_DISABLE(&AdcT34aHandle);
    __HAL_ADC_DISABLE(&AdcCurrentHandle);	
	
}	
void	API_ADC_StartAllAdc(void)
{
    __HAL_ADC_ENABLE(&AdcT12aHandle);
    __HAL_ADC_ENABLE(&AdcT34aHandle);
    __HAL_ADC_ENABLE(&AdcCurrentHandle);	
	
}	




void	API_ADC_SELECT(ADC_SELECT_ENUM ch)
{	
		

		HAL_ADC_Stop(&AdcT12aHandle);
		HAL_ADC_Stop(&AdcT34aHandle);
		HAL_ADC_Stop(&AdcCurrentHandle);	


		API_ADC_Instance_RecoverCallBack(ch);
		

	
}
void API_ADC_Init(void)
{
  ADC_ChannelConfTypeDef   sConfig;
  ADC_AnalogWDGConfTypeDef AnalogWDGConfig;
  

	

		
	

	API_ADC_GROUP_INIT(ADC_Select_Tempe);		//过零检测其它IGBT的设置
//	API_ADC_GROUP_INIT(ADC_Select_VcT2A);		//T2A需要检锅的设置 
	API_ADC_GROUP_INIT(ADC_Select_VcIc);		//正常加热的设置
	

	
	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, IRQ_PRIORITY_DMA1_Channel1, 0);		//m2m
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);	
		
		
}




void	API_ADC_Start_IT(ADC_HandleTypeDef* hadc)
{
	
	      __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_EOC);
      /* Enable ADC DMA mode */
      SET_BIT(hadc->Instance->CR2, ADC_CR2_DMA);
			HAL_ADC_Start_IT(hadc);	

}	
void	API_ADC_Start(ADC_HandleTypeDef* hadc)
{
	
	      __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_EOC);
      /* Enable ADC DMA mode */
      SET_BIT(hadc->Instance->CR2, ADC_CR2_DMA);
			HAL_ADC_Start(hadc);	

}	
	
void	API_ADC_Start_DMA(ADC_HandleTypeDef* hadc)		
{	
	HAL_ADC_Stop_DMA(hadc);
  if(HAL_ADC_Start_DMA(hadc, AdcBuff, 8) != HAL_OK )//DMA地址为虚拟，实际在RECOVER设置
  {
      Error_Handler();
  }

}





void API_ADC1_2_IRQHandler(void)
{


  
  uint32_t flag_it = AdcCurrentHandle.Instance->SR;
  uint32_t source_it = AdcCurrentHandle.Instance->CR1;
  
  
//----------ADC2--------------------------------------------------------------------------  
//  if(__HAL_ADC_GET_FLAG(&AdcT12aHandle, ADC_FLAG_AWD)&&__HAL_ADC_GET_IT_SOURCE(&AdcT12aHandle,ADC_IT_AWD))

	if((flag_it&ADC_FLAG_AWD)&&(source_it&ADC_IT_AWD))		//current过流处理
  {

    __HAL_ADC_CLEAR_FLAG(&AdcCurrentHandle, ADC_FLAG_AWD);
		API_ADC_Current1AWD_IRQHandlerCallBack();					//在APP ADC中处理  

  } 	
		
	
	
//   if(__HAL_ADC_GET_FLAG(&AdcT12aHandle, ADC_FLAG_EOC)&&__HAL_ADC_GET_IT_SOURCE(&AdcT12aHandle,ADC_IT_EOC))
		if((flag_it&ADC_FLAG_EOC)&&(source_it&ADC_IT_EOC))
		{

    __HAL_ADC_CLEAR_FLAG(&AdcCurrentHandle, ADC_FLAG_EOC);				//ADC2 中断后VCIC累加
		
	

  }


  

	
//----------ADC 1-----------------------------------------------------------------------------------------------
 flag_it = AdcT12aHandle.Instance->SR;
 source_it = AdcT12aHandle.Instance->CR1;

  if((flag_it&ADC_FLAG_EOC)&&(source_it&ADC_IT_EOC))
  {

    __HAL_ADC_CLEAR_FLAG(&AdcT12aHandle, ADC_FLAG_EOC);				//ADC2 中断后VCIC累加
		
//		API_VcIc_EOC_IRQHandlerCallBack();					//在APP ADC中处理
		API_T12_EOC_IRQHandlerCallBack();					//在APP ADC中处理
 	
  } 	
	


}	
void API_ADC3_IRQHandler(void)
{

	uint32_t flag_it = AdcT34aHandle.Instance->SR;
  uint32_t source_it = AdcT34aHandle.Instance->CR1;
	
//  if(__HAL_ADC_GET_FLAG(&AdcT34aHandle, ADC_FLAG_AWD)&&__HAL_ADC_GET_IT_SOURCE(&AdcT34aHandle,ADC_IT_AWD))
	if((flag_it&ADC_FLAG_AWD)&&(source_it&ADC_IT_AWD))  
  {
  
    __HAL_ADC_CLEAR_FLAG(&AdcT34aHandle, ADC_FLAG_AWD);
		API_ADC_Current2AWD_IRQHandlerCallBack();					//在APP ADC中处理   

  } 
//  if(__HAL_ADC_GET_FLAG(&AdcT34aHandle, ADC_FLAG_EOC)&&__HAL_ADC_GET_IT_SOURCE(&AdcT34aHandle,ADC_IT_EOC))	
	if((flag_it&ADC_FLAG_EOC)&&(source_it&ADC_IT_EOC))
	{

		__HAL_ADC_CLEAR_FLAG(&AdcT34aHandle, ADC_FLAG_EOC);
		// API_T34_EOC_IRQHandlerCallBack();

		
	}

}





 void 	API_ADC_T12A_ENABLE_IT_EOC(void)		//ADC1返回值代表当前EOC中断是否开启 1：开启 0：关闭
 {

 		__HAL_ADC_CLEAR_FLAG(&AdcT12aHandle, ADC_FLAG_EOC);
 		__HAL_ADC_ENABLE_IT(&AdcT12aHandle,ADC_IT_EOC);


 }
 void 	API_ADC_T12A_DISABLE_IT_EOC(void)			//返回值代表当前EOC中断是否开启 1：开启 0：关闭
 {
   	__HAL_ADC_DISABLE_IT(&AdcT12aHandle,ADC_IT_EOC);

 }



void 	API_ADC_T34A_ENABLE_IT_EOC(void)			//返回值代表当前EOC中断是否开启 1：开启 0：关闭
{

		__HAL_ADC_CLEAR_FLAG(&AdcT34aHandle, ADC_FLAG_EOC);
		__HAL_ADC_ENABLE_IT(&AdcT34aHandle,ADC_IT_EOC);

}	


//static 	uint16_t AdcDmaCnctrLast;				//上一次DMA位置 
//static 	uint16_t AdcDmaCnctrNew;				//当次DMA位置 

	

void 	API_ADC_T34A_DISABLE_IT_EOC(void)			//返回值代表当前EOC中断是否开启 1：开启 0：关闭
{
			__HAL_ADC_DISABLE_IT(&AdcT34aHandle,ADC_IT_EOC);
			__HAL_ADC_CLEAR_FLAG(&AdcT34aHandle, ADC_FLAG_EOC);

}	
uint8_t API_ADC_T34A_GET_IT_EOC(void)
{
	return __HAL_ADC_GET_IT_SOURCE(&AdcT34aHandle,ADC_IT_EOC);
}	




void API_ADC_SetAdcWatchIT(uint8_t onOff)	
{
	

	if(onOff)
	{
		SET_BIT(AdcCurrentHandle.Instance->CR1, ADC_CR1_AWDEN);
	}
	else
	{	
		CLEAR_BIT(AdcCurrentHandle.Instance->CR1, ADC_CR1_AWDEN);
	}
}

void	portPendvSet(void)
{
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}	
void	portPendvClear(void)
{
		SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk;
}	

void	API_TIM_PAN_IRQHandler(void)
{
//		API_GPIO_WritePin(DebugA_pin,1);
//		API_GPIO_WritePin(DebugA_pin,0);
}



uint32_t API_ADC_GetAddressDR(ADC_CH_ENUM ch)
{
	return((uint32_t)&(API_ADC_Handle[ch]->Instance->DR));
}
uint32_t API_ADC_GetAddressDRx(ADC_CH_ENUM ch,uint32_t channel)
{
	
	
	uint32_t *   drx=(uint32_t*)&(API_ADC_Handle[ch]->Instance->DR1);

	drx+=(channel);

	return((uint32_t)drx);
}

uint32_t API_ADC_GetAddressJDRx(ADC_CH_ENUM ch,uint32_t channel)
{
	
	
	uint32_t *   drx=(uint32_t*)&(API_ADC_Handle[ch]->Instance->JDR1);

	drx+=(channel);

	return((uint32_t)drx);
}

uint32_t API_ADC_GetDRx(ADC_CH_ENUM ch,uint32_t channel)
{
	
	
	uint32_t *   drx=(uint32_t*)&(API_ADC_Handle[ch]->Instance->DR1);

	drx+=(channel);

	return((uint32_t)*drx);
}


static uint32_t _awd_cached_value = 0xfff;

/* STRONG: API_ADC_TxaAwdValue — APP_ADC __weak → 此处 STRONG (Pair D)
 * 重写上次 API_ADC_TxaAwdValue 缓存的值到硬件寄存器
 * ADC 通道切换后硬件 AWD 寄存器被重置, 需重新写入 */
 void	API_ADC_TxaAwdValue(uint32_t value)
 {
 		_awd_cached_value = value;
 		/* Set the high threshold */
 		WRITE_REG(AdcCurrentHandle.Instance->HTR, value);		//等待超过阀值
		WRITE_REG(AdcT34aHandle.Instance->HTR, value);		//等待超过阀值
 		/* Set the low threshold */
 		WRITE_REG(AdcCurrentHandle.Instance->LTR, 0);		//等待超过阀值
		WRITE_REG(AdcT34aHandle.Instance->LTR, 0);		//等待超过阀值
 }




uint32_t API_ADC_GetHTR(void)
{
	return	AdcCurrentHandle.Instance->HTR;
}


