#include "S_TOUCHKEYCFG.H"
#include "TKDriver.h"
#include "sc32f1xxx.h"
#include "sc32f1xxx_rcc.h"

#ifdef	SC32f10xx

#define ScanTimeSet 4000

#elif	SC32f12xx
//待填写
#define ScanTimeSet 4000
#elif	SC32R803
//待填写
#define ScanTimeSet 4000

#elif	SC32L14xx
//待填写
#define ScanTimeSet 3000

#endif



//全局变量声明：该区域不可修改
unsigned short int  TK_ScanTime = ScanTimeSet;
unsigned short int 	UpdateBaseLNum; 	    // 单键长按计数
unsigned short int 	MultipleLNum;			// 多按键干扰计数

unsigned    char	bMultiple;	     		//多按键标志	
unsigned    char	ConfirmTouchCnt;
unsigned    char	MultipleDealTpye = 0; 

unsigned    char    SingleCurrentChannelMax;
char				SetNoiseThreshold;


unsigned    char			TK_TouchKeyStatus;

unsigned 	long long 		KeyFlag,KeyMark;
unsigned    char			bNeedUpdate;
volatile unsigned    char   CurrentIndex = 0;			//sensor当前通道 
unsigned 	short int     	ScanTime;				//用于换算TKTM

unsigned    char 			IsInNosicValue;  			//确定一轮中数据是否都在NOSIC 之内
unsigned    char			MultipleDealIndex = 0; 		//多基线更新复位计数
unsigned    char			FlowFingerRoundIndex; 
unsigned 	short int    	FlowFingerRoundCnt; 		//考虑数据浮在NOSIC 与FINGER 之间

volatile unsigned char		WakeUp_Flag = 0;
volatile unsigned char  	TK_WakeUp_Flag = 0;
volatile unsigned char  	LowPowerScan_Flag = 0;          //低功耗扫描标志
/*******************************隔空库独有************************************/
unsigned 	char 	BigNosicCount=0; 				//大数据干扰计数
unsigned 	char 	StrongNosicCount=0; 		    //强干扰计数
unsigned 	char 	MultipleKeyNosicCount=0; 		//多键干扰计数
unsigned 	char 	WeakNosicCount=0; 				//弱干扰计数
unsigned 	char	MultipleKeyInhibitionCount=0; 	//多键抑制计数
unsigned 	char  	RemoveTopCount=0; 			    //消顶计数
/****************************************************************************/

unsigned char		CurrentChannel[SOCAPI_SET_TOUCHKEY_TOTAL];
TK_ParameterSaveBuffer_StructDef SingleParameterBufferSet[SOCAPI_SET_TOUCHKEY_TOTAL] ;
TK_BasicDataUpdatePar_StructDef SingleChannelsBaseLineUpdatePar[SOCAPI_SET_TOUCHKEY_TOTAL] ;

#if (TK_LowPowerMode == 1)
unsigned long long 	CombineKeyFlag,CombineKeyMark;
unsigned    char    CombineCurrentChannelMax;
unsigned long long	CombineCurrentChannel[SOCAPI_COMBINE_TOUCHKEY_TOTAL];//记录多少组并联通道组 	
TK_ParameterSaveBuffer_StructDef CombineParameterBufferSet[SOCAPI_COMBINE_TOUCHKEY_TOTAL] ;
TK_BasicDataUpdatePar_StructDef CombineChannelsBaseLineUpdatePar[SOCAPI_COMBINE_TOUCHKEY_TOTAL] ;

unsigned char	  	BaseLineAdjusttmp_Low[SOCAPI_SET_TOUCHKEY_TOTAL];
unsigned  int  		ScanTimeTemp_Low[SOCAPI_SET_TOUCHKEY_TOTAL];
short int			SetFingerThresholdtmp_Low[SOCAPI_SET_TOUCHKEY_TOTAL];

unsigned char	  	CombineBaseLineAdjusttmp_Low[SOCAPI_COMBINE_TOUCHKEY_TOTAL];
unsigned  int  		CombineScanTimeTemp_Low[SOCAPI_COMBINE_TOUCHKEY_TOTAL];
short int			CombineSetFingerThresholdtmp_Low[SOCAPI_COMBINE_TOUCHKEY_TOTAL];

#endif


/*******************************库函数基础设置，一般不需要更改******************/
#define		SOCAPI_CFG_CYCLE_CNT					    32 		//取值范围10-255，触控初始化数据滤出次数，以及修正值初始化修正次数，默认32，取值越大触控初始化时间越长，触控数据越稳定  
#define		SOCAPI_CFG_OVERLOW_MAX_COUNT			    10		//低基线更新速度设置，代表发生低基线后，多少轮更新一次基线，更新步进noise/2，值越小基线更新速度越快
#define		SOCAPI_CFG_RESET_BASELINE_CNT			    10  	//低基线更新多少次后，如果还处于低基线状态，直接强制更新基线，值越小基线更新速度越快

																//SOCAPI_CFG_OVERLOW_MAX_COUNT和SOCAPI_CFG_RESET_BASELINE_CNT的取值乘积不能大于255
const unsigned 	char 		CFG_OVERLOW_MAX_COUNT = SOCAPI_CFG_OVERLOW_MAX_COUNT;
const unsigned 	char 		CFG_RESET_BASELINE_CNT = SOCAPI_CFG_RESET_BASELINE_CNT;	
const unsigned 	char 		CFG_CYCLE_CNT = SOCAPI_CFG_CYCLE_CNT;

#define		TK_InitBaseLineUpRangeSet					    6 		//取值范围6-8，默认6
#define		TK_InitBaseLineLowRangeSet			    		4		//取值范围2-4，默认4

#define		TK_CurrentBaseLineUpRangeSet					7 		//取值范围6-8，默认7
#define		TK_CurrentBaseLineLowRangeSet			    	3		//取值范围2-4，默认3

const unsigned 	char     	TK_InitBaseLineUpRange = TK_InitBaseLineUpRangeSet;
const unsigned 	char     	TK_InitBaseLineLowRange = TK_InitBaseLineLowRangeSet;
const unsigned 	char     	TK_CurrentBaseLineUpRange = TK_CurrentBaseLineUpRangeSet;
const unsigned 	char     	TK_CurrentBaseLineLowRange = TK_CurrentBaseLineLowRangeSet;

/****************************************************************************/


/*******************************弹簧库独有************************************/
#define		SOCAPI_SET_CS_FUNCTION						1		//0:表示不进行CS测试,1: 表示进行CS测试,默认0
#define		SOCAPI_INHIBITION_ZONE					    8		//抑制区间%，设置范围5-10，默认7,即（7*10）%=70% ，连水时加大该参数,对讲机设置小
#define		SOCAPI_MAX_KEY_MUTIPLE						300		//多少次干扰更新基线，默认300*5=1500
#define		SOCAPI_MAX_KEY_NUM_INVALID					3		//强制更新基线按键限制个数，默认3
/****************************************************************************/


/****************************************************************************/

#define	  	AppType			     			0
#define	  	IsDoubleKey			 			1
#define	  	AirSeparationDistance	 	    2
#define   	CONFIRMTOUCHCNT        	        3
#define   	INIT_AUTO_UPDATE_TIME	 	    4
#define   	SET_KEY_CONTI_TIME     	        5  
#define   	SET_SYNC_UPDATE 		 		6
#define   	SET_UPDATE_SPEED 		 		7	
#define   	AUTO_UPDATE_TIME	     	    8
#define	  	FilteredKValue		 			9
#define	  	SET_ANTIJAM		     			10
#define	  	BAUD		             		11
#define	  	DwellTime		         		12
#define   	SaveTime					    13
#define   	NOISE                  	        16


#define   	SET_TOUCH_FREQ		 			0
#define	  	SET_RESOLUTION_FACTOR		 	1
#define   	SET_RESOLUTION			 		2
#define   	SET_GAIN_CFG				 	3
#define   	SCANTIME				 		4
#define   	SET_ICHA_FACTOR	 		    	5
#define   	SET_ICHA	 		    		6
#define		FINGER_THRESHOLD_H				8
#define		FINGER_THRESHOLD_L				9


//**********************************************************************************	
// 								不同应用下参数差异							      //
//**********************************************************************************
#if (TK_LowPowerMode == 1)
#define ScanTimeCon		3
#endif
#if (TK_LowPowerMode == 0)
#define ScanTimeCon		0
#endif
volatile unsigned char  TK_ShiftScanTime = ScanTimeCon;

#if (Slider_key == 1)
unsigned char UsingTKSlideModuleNumber;//通道组数
#endif



//**********************************************************************************	
// 								滑动模块设置							              //
//**********************************************************************************
#if (Slider_key == 1)

#define      USING_TKSlideModule_Number 		 USING_TKSlideModule_Number_Set	

TKSlideModuleDeal TKSlideModuleDealArray[USING_TKSlideModule_Number] = 
{
#if (USING_TKSlideModule_Number >= 1) && (USING_TKSlideModule_Number <= 4)
	{
		TypeFlag0,			//(需修改)		滑动块类型,bar:为滑条，Circle:为滑轮
		{TKChannel0},	//(需修改)		按照滑动顺序，依次将使用到的滑条按键通道存入数组6->5->4->3滑动值逐渐变大
		UsingTKChannelNumber0,			//(需修改)		当前滑动模块使用的TK通道数量
		SideLevel0,			//(需修改)		滑动输出的档位最大级数（从1算起） 
		
		
		20,			//(可修改)		DDiffer值<SUB_DATA该Differ值无效，会设置为0，屏蔽干扰（修改灵敏度）  该值建议默认
		LIBData0,		//(可修改)		Differ值>LIB_DATA表示有键（修改灵敏度）该值建议设置为阈值的四分之一到三分之一
		
		
		{0},		//(无需修改)		存放内部TK通道排序都的index
		1,			//(无需修改)		扫描多少轮无滑条按键即更新基线，默认10   可修改
		0,			//(无需修改)		记录上一次输出的值
		0,			//(无需修改)		滑动输出值，通过该值获取输出结果
		0,			//(无需修改)		基线更新计数
		150,		//(无需修改)		可采用150默认值，耦合参数范围(100~200)，参数过小会出现出值缺失,例如滑动出值OutValue为1->2->3->5->6->7,参数过大会出现出值重叠,例如滑动出值OutValue为1->2->3->4->3->4->5->6->7
		0,			//(无需修改)		耦合参数，DeBug值,用于调试，调试时输出该值，滑动滑动模块取输出的最小值，根据最小值调节CouplingValue耦合参数
		0			//(无需修改)		滑动模块触发计数标志

	}, 
#endif
#if (USING_TKSlideModule_Number >= 2)&&  (USING_TKSlideModule_Number <= 4)
	{
		TypeFlag1,		//(需修改)		滑动块类型,bar:为滑条，Circle:为滑轮
		{TKChannel1},	//(需修改)		按照滑动顺序，依次将使用到的滑条按键通道存入数组6->5->4->3滑动值逐渐变大
		UsingTKChannelNumber1,			//(需修改)		当前滑动模块使用的TK通道数量
		SideLevel1,			//(需修改)		滑动输出的档位最大级数（从1算起） 

		
		20,			//(可修改)		Differ值<SUB_DATA表示无键（修改灵敏度） 
		LIBData1,		//(可修改)		Differ值>LIB_DATA表示有键（修改灵敏度）

		
		{0},		//(无需修改)		存放内部TK通道排序都的index
		1,			//(无需修改)		扫描多少轮无滑条按键即更新基线，默认10   可修改
		0,			//(无需修改)		记录上一次输出的值
		0,			//(无需修改)		滑动输出值，通过该值获取输出结果
		0,			//(无需修改)		基线更新计数
		150,		//(无需修改)		可采用150默认值，耦合参数范围(100~200)，参数过小会出现出值缺失,例如滑动出值OutValue为1->2->3->5->6->7,参数过大会出现出值重叠,例如滑动出值OutValue为1->2->3->4->3->4->5->6->7
		0,			//(无需修改)		耦合参数，DeBug值,用于调试，调试时输出该值，滑动滑动模块取输出的最小值，根据最小值调节CouplingValue耦合参数
		0			//(无需修改)		滑动模块触发计数标志

	},
#endif
#if (USING_TKSlideModule_Number >= 3)&& (USING_TKSlideModule_Number <= 4) 
	{
		TypeFlag2,		//(需修改)		滑动块类型,bar:为滑条，Circle:为滑轮
		{TKChannel2},	//(需修改)		按照滑动顺序，依次将使用到的滑条按键通道存入数组6->5->4->3滑动值逐渐变大
		UsingTKChannelNumber2,			//(需修改)		当前滑动模块使用的TK通道数量
		SideLevel2,			//(需修改)		滑动输出的档位最大级数（从1算起） 

		
		20,			//(可修改)		Differ值<SUB_DATA表示无键（修改灵敏度） 
		LIBData2,		//(可修改)		Differ值>LIB_DATA表示有键（修改灵敏度）

		
		{0},		//(无需修改)		存放内部TK通道排序都的index
		1,			//(无需修改)		扫描多少轮无滑条按键即更新基线，默认10   可修改
		0,			//(无需修改)		记录上一次输出的值
		0,			//(无需修改)		滑动输出值，通过该值获取输出结果
		0,			//(无需修改)		基线更新计数
		150,		//(无需修改)		可采用150默认值，耦合参数范围(100~200)，参数过小会出现出值缺失,例如滑动出值OutValue为1->2->3->5->6->7,参数过大会出现出值重叠,例如滑动出值OutValue为1->2->3->4->3->4->5->6->7
		0,			//(无需修改)		耦合参数，DeBug值,用于调试，调试时输出该值，滑动滑动模块取输出的最小值，根据最小值调节CouplingValue耦合参数
		0			//(无需修改)		滑动模块触发计数标志

	},
#endif
#if (USING_TKSlideModule_Number == 4)   	
	{
		TypeFlag3,		//(需修改)		滑动块类型,bar:为滑条，Circle:为滑轮
		{TKChannel3},	//(需修改)		按照滑动顺序，依次将使用到的滑条按键通道存入数组6->5->4->3滑动值逐渐变大
		UsingTKChannelNumber3,			//(需修改)		当前滑动模块使用的TK通道数量
		SideLevel3,			//(需修改)		滑动输出的档位最大级数（从1算起） 

		
		20,			//(可修改)		Differ值<SUB_DATA表示无键（修改灵敏度） 
		LIBData3,		//(可修改)		Differ值>LIB_DATA表示有键（修改灵敏度）

		
		{0},		//(无需修改)		存放内部TK通道排序都的index
		1,			//(无需修改)		扫描多少轮无滑条按键即更新基线，默认10   可修改
		0,			//(无需修改)		记录上一次输出的值
		0,			//(无需修改)		滑动输出值，通过该值获取输出结果
		0,			//(无需修改)		基线更新计数
		150,		//(无需修改)		可采用150默认值，耦合参数范围(100~200)，参数过小会出现出值缺失,例如滑动出值OutValue为1->2->3->5->6->7,参数过大会出现出值重叠,例如滑动出值OutValue为1->2->3->4->3->4->5->6->7
		0,			//(无需修改)		耦合参数，DeBug值,用于调试，调试时输出该值，滑动滑动模块取输出的最小值，根据最小值调节CouplingValue耦合参数
		0			//(无需修改)		滑动模块触发计数标志

	},	
#endif 
};

#endif


//**********************************************************************************	
// 								低功耗设置							              //
//**********************************************************************************               
#if (TK_LowPowerMode == 1)

#define		TK_GET_TKIF	 (TK->TK_CON & TK_CON_TKIF)

#define		BTM_TIMEBASE_15625US     0X00U		//低频时钟中断时间为15.6MS
#define		BTM_TIMEBASE_31250US     0X01U		//低频时钟中断时间为31.3MS
#define		BTM_TIMEBASE_62500US     0X02U		//低频时钟中断时间为62.5MS
#define		BTM_TIMEBASE_125MS  	 0X03U		//低频时钟中断时间为125MS
#define		BTM_TIMEBASE_250MS       0X04U		//低频时钟中断时间为250MS
#define		BTM_TIMEBASE_500MS       0X05U		//低频时钟中断时间为500MS
#define		BTM_TIMEBASE_1S          0X06U		//低频时钟中断时间为1S
#define		BTM_TIMEBASE_2S          0X07U		//低频时钟中断时间为2S
#define		BTM_TIMEBASE_4S          0X08U		//低频时钟中断时间为4S


#define      WakeUpKeyNum                      WakeUpKeyNum_Set                	//低功耗模式下扫描按键个数     
#define      WakeUpKeyChannel1                 WakeUpKeyChannel_Set           	//低功耗下扫描按键的对应通道
#define      WakeUpKeyChannel2                 WakeUpKeyChannel2_Set           //低功耗下扫描按键的对应通道高32bit
#define      TK_SeepTimervSetting              TK_SeepTimervSetting_Set   		//低功耗下按键之间的扫描间隔
#define      TK_WakeUpConfirmTouchCnt          TK_WakeUpConfirmTouchCnt_Set 	//低功耗下确认按键次数

#if	TK_SeepTimervSetting == BTM_TIMEBASE_4S							//低功耗下基线更新间隔定义
	#define  BaselineUpdateCnt  1										
#elif	TK_SeepTimervSetting == BTM_TIMEBASE_2S							
	#define  BaselineUpdateCnt  3
#elif	TK_SeepTimervSetting == BTM_TIMEBASE_1S							
	#define  BaselineUpdateCnt  6
#elif	TK_SeepTimervSetting == BTM_TIMEBASE_500MS					
	#define  BaselineUpdateCnt  12
#elif TK_SeepTimervSetting == BTM_TIMEBASE_250MS					 
	#define  BaselineUpdateCnt  24
#elif TK_SeepTimervSetting == BTM_TIMEBASE_125MS
	#define  BaselineUpdateCnt  48
#elif TK_SeepTimervSetting == BTM_TIMEBASE_62500US
	#define  BaselineUpdateCnt  96
#elif TK_SeepTimervSetting == BTM_TIMEBASE_31300US
	#define  BaselineUpdateCnt  192
#elif TK_SeepTimervSetting == BTM_TIMEBASE_15600US
	#define  BaselineUpdateCnt  384									
#endif

unsigned char 	OffHandCount = 0;
unsigned int  	NumCount = 0;
unsigned int 	CumulateCount = 0;

volatile unsigned char  	Touch_WakeUpFlag=0;				//按键唤醒标志位
volatile unsigned char  	BTM_WakeUpFlag =0;				//BTM唤醒标志位
unsigned char 				LowPowerEnter_Flag = 0;

unsigned char  	SingleKeyFastScan_Flag = 0;                 //单按键快速扫描标志
unsigned char   CombineKeyFastScan_Flag = 0;                //并联按键快速扫描标志

unsigned char  	WakeUpKey_List[WakeUpKeyNum];
unsigned char	WakeUpThenScanCount = 0; 
unsigned char  	WakeUpKeyValue;								//==WakeUpKey_List[WakeUpKey_Index]
unsigned  int 	ScanTimeTemp0;							//==ScanTime>>4
unsigned short int WakeUpNum;								//唤醒次数计数--用于低功耗下更新基线

extern void 	Customer_IntoLowPowerMode_Init(void);
extern void 	Customer_QuitLowPowerMode_Init(void);
extern void 	Customer_BTM_Dispose(void);

/**************************************************
*函数名称：void BtmInit(void) 
*函数功能：
*入口参数：void 
*出口参数：void
**************************************************/
void BTM_Init(void)
{
	RCC_Unlock(0xFF);
	RCC_LIRCCmd(ENABLE);//使能LIRC
	
	/* 使能BTM时钟 */
	RCC_APB2Cmd(ENABLE);
	
	BTM->BTM_CON = (BTM->BTM_CON & (uint32_t)~BTM_CON_BTMFS ) | TK_SeepTimervSetting; //BTM定时时间设置
	BTM->BTM_CON = (BTM->BTM_CON  | (uint32_t)BTM_CON_INTEN | (uint32_t)BTM_CON_BTMEN); //使能BTM中断及启动BTM
	
	NVIC_EnableIRQ(BTM_IRQn);
}
/**************************************************
*函数名称：void BTM_IRQHandler(void)
*函数功能：Btm中断服务函数
*入口参数：void 
*出口参数：void
**************************************************/
void BTM_IRQHandler(void) 
{
	BTM->BTM_STS  |= 0x01; //清零BTM中断标志位
	BTM_WakeUpFlag = 1;
}

#endif

/**************************************************
*函数名称：void CLR_TK_WDT(void) 
*函数功能：喂狗函数
*入口参数：void 
*出口参数：void
**************************************************/
void CLR_TK_WDT(void)
{
	WDT->WDT_CON |= WDT_CON_CLRWDT;	//喂狗 WDT_CON_CLRWDT
}					

/**************************************************
*函数名称：void TKCLK_Init(void) 
*函数功能：TK模块时钟开启
*入口参数：void 
*出口参数：void
**************************************************/
void TKCLK_Init(void)
{
	RCC_APB2Cmd(ENABLE);  //APB2时钟开启	
	NVIC_EnableIRQ(TK_IRQn); //开NVIC TK中断
}

/**************************************************
*函数名称：void TK_TRIG(void) 
*函数功能：Touchkey的触发开关，完成一次按键扫描
*入口参数：void 
*出口参数：void
**************************************************/
void TK_TRIG(void)
{
	TK->TK_CON |= TK_CON_TRIG;	
}

/**************************************************
*函数名称：void TK_CLR_TKIF(void) 
*函数功能：清除TK中断标志位
*入口参数：void 
*出口参数：void
**************************************************/
void TK_CLR_TKIF(void)
{
	TK->TK_CON |= TK_CON_TKIF;	
}

/**************************************************
*函数名称：unsigned short int TK_CNTGet(void) 
*函数功能：获取TK数据
*入口参数：void 
*出口参数：void
**************************************************/
unsigned short int TK_CNTGet(void)
{
	return TK->TK_CNT;	
}

/**************************************************
*函数名称：void TK_MOD_SET(unsigned char i) 
*函数功能：TK模式设置
*入口参数：unsigned char i 
*出口参数：void
**************************************************/
void TK_MOD_SET(unsigned char i)
{

	if(i)
	{
		TK->TK_CON |= TK_CON_TKMOD;
		
	}
	else
	{
		TK->TK_CON &= ~(TK_CON_TKMOD);
	}
	
}

/**************************************************
*函数名称：void TK_ENTKS_SET(unsigned char i) 
*函数功能：TK电源设置
*入口参数：unsigned char i 
*出口参数：void
**************************************************/
void TK_ENTKS_SET(unsigned char i)
{

	if(i)
	{
		TK->TK_CON |= TK_CON_ENTKS;
		
	}
	else
	{
		TK->TK_CON &= ~(TK_CON_ENTKS);
	}
	
}

/**************************************************
*函数名称：void TK_TKIS_SET(unsigned char i) 
*函数功能：TK单通道设置
*入口参数：unsigned char i 
*出口参数：void
**************************************************/
void TK_TKIS_SET(unsigned char i)
{
#ifdef	SC32f10xx
	TK->TK_CHN = (1 << i);
#elif	SC32R803

	TK->TK_CHN = (1 << i);
	
#elif	SC32f12xx
	if(i <= 31)
    {
        TK->TK_CHN0 = (1UL << i);
        TK->TK_CHN1 = 0;
    }
    else if((i <= 63) && (i > 31))
    {
        TK->TK_CHN0 = 0;
        TK->TK_CHN1 = (1UL << (i-32));
    }
	
#elif	SC32L14xx
	TK->TK_CHN = (1 << i);
	
#endif
	
}

#if (TK_LowPowerMode == 1)
/**************************************************
*函数名称：void TK_TKIS_CombineSET(uint64_t SET) 
*函数功能：单按键扫描函数
*入口参数：uint64_t SET
*出口参数：void
*备注	 ：
**************************************************/		 		
void TK_TKIS_CombineSET(uint64_t SET) 
{
#ifdef	SC32f10xx 
	
    TK->TK_CHN = (uint32_t)SET;
	
#elif	SC32R803

    TK->TK_CHN = (uint32_t)SET;
	
#elif	SC32f12xx
	
	TK->TK_CHN0 = (uint32_t)SET;
	TK->TK_CHN1 = (uint32_t)(SET >> 32);

#elif	SC32L14xx
	
	TK->TK_CHN = (uint32_t)SET;
	
#endif
}
#endif

/**************************************************
*函数名称：void TK_VREF_SET(unsigned char i) 
*函数功能：TK电压设置
*入口参数：unsigned char i 
*出口参数：void
**************************************************/
void TK_VREF_SET(unsigned char i)
{

	TK->TK_CFG = (TK->TK_CFG & ~(TK_CFG_VREF)) | ((i & 0X0FL)<<24);
	
}

/**************************************************
*函数名称：void TK_CTIME_SET(unsigned char i) 
*函数功能：TK频率设置
*入口参数：unsigned char i 
*出口参数：void
**************************************************/
void TK_CTIME_SET(unsigned char i)
{

	TK->TK_CON = (TK->TK_CON & ~(TK_CON_CTIME)) | ((i & 0X0FL)<<12);
	
}


/**************************************************
*函数名称：void TK_PRS_SET(unsigned char i) 
*函数功能：TK PRS设置
*入口参数：unsigned char i 
*出口参数：void
**************************************************/
void TK_PRS_SET(unsigned char i)
{

	if(i)
	{
		TK->TK_CON |= TK_CON_PRS;
		
	}
	else
	{
		TK->TK_CON &= ~(TK_CON_PRS);
	}
	
}

/**************************************************
*函数名称：void TK_PRSC_SET(unsigned char i) 
*函数功能：TK PRSC设置
*入口参数：unsigned char i 
*出口参数：void
**************************************************/
void TK_PRSC_SET(unsigned char i)
{

	if(i)
	{
		TK->TK_CON |= TK_CON_PRSC;
		
	}
	else
	{
		TK->TK_CON &= ~(TK_CON_PRSC);
	}
	
}

/**************************************************
*函数名称：void TK_ICHAC_SET(unsigned char i) 
*函数功能：TK ICHAC设置
*入口参数：unsigned char i 
*出口参数：void
**************************************************/
void TK_ICHAC_SET(unsigned char i)
{

	TK->TK_CFG = (TK->TK_CFG & ~(TK_CFG_ICHAC))| ((uint32_t)(i & 0X03UL)<<30);
	
}

/**************************************************
*函数名称：void TK_ICHGC_SET(unsigned char i) 
*函数功能：TK ICHGC设置
*入口参数：unsigned char i 
*出口参数：void
**************************************************/
void TK_ICHGC_SET(unsigned char i)
{

	TK->TK_CFG = (TK->TK_CFG & ~(TK_CFG_ICHGC))| ((i & 0X03L)<<28);
	
}

/**************************************************
*函数名称：void TK_ICHA_SET(unsigned char i) 
*函数功能：TK ICHA设置
*入口参数：unsigned char i 
*出口参数：void
**************************************************/
void TK_ICHA_SET(unsigned char i)
{

	TK->TK_CFG = (TK->TK_CFG & ~(TK_CFG_ICHA))| ((i & 0XFFL)<<8);
	
}

/**************************************************
*函数名称：void TK_ICHG_SET(unsigned char i) 
*函数功能：TK ICHG设置
*入口参数：unsigned char i 
*出口参数：void
**************************************************/
void TK_ICHG_SET(unsigned char i)
{

	TK->TK_CFG = (TK->TK_CFG & ~(TK_CFG_ICHG))| ((i & 0XFFL)<<0);
	
}

/**************************************************
*函数名称：void TK_TKTM_SET(unsigned int i) 
*函数功能：TK TKTM设置
*入口参数：unsigned int i 
*出口参数：void
**************************************************/
void TK_TKTM_SET(unsigned int i)
{

	TK->TK_TM = (i & TK_TK_TM);
	
}

/**************************************************
*函数名称：void TK_TKINT_SET(unsigned char i) 
*函数功能：TK 中断使能设置
*入口参数：unsigned char i 
*出口参数：void
**************************************************/
void TK_TKINT_SET(unsigned char i)
{

	if(i)
	{
		TK->TK_CON |= TK_CON_INTEN;
		
	}
	else
	{
		TK->TK_CON &= ~(TK_CON_INTEN);
	}
	
}

/*****************************************************************************
*函数名称：unsigned short int TK_SetOneKeyPushResetTime(void) 
*函数功能：按键最长的输出时间
*入口参数：void
*出口参数：unsigned short int TKCFG[SET_KEY_CONTI_TIME]
*备注	 ：最长输出时间计算：
是根据有多长时间启动TouchKeyRestart（）一次*TKCFG[SET_KEY_CONTI_TIME]
例如10ms启动一次,超过该时间TKCFG[SET_KEY_CONTI_TIME]*10ms后此按键无效
*****************************************************************************/
unsigned short int TK_SetOneKeyPushResetTime(void)   
{	  
	return  TKCFG[SET_KEY_CONTI_TIME];
}

/****************************************************************************
*函数名称：unsigned short int  TK_GetBaselineUpdateThreshold(void)
*函数功能：更新速度 
*入口参数：void
*出口参数：unsigned short int
*备注	 ：
*****************************************************************************/
unsigned short int TK_GetBaselineUpdateThreshold(void)
{
	return TKCFG[SET_UPDATE_SPEED]; 
}

/****************************************************************************
*函数名称：unsigned char TK_GetInitAutoUpdateTime(void)
*函数功能：初始化自动校准次数
*入口参数：void
*出口参数：unsigned char
*备注	 ：
*****************************************************************************/
unsigned char TK_GetInitAutoUpdateTime(void)
{
	return  TKCFG[INIT_AUTO_UPDATE_TIME];
}



/****************************************************************************
*函数名称：unsigned char TK_IsDoubleKeyOrSlideKey(void)
*函数功能：检测是否是弹簧滑条或者双键
*入口参数：void
*出口参数：unsigned char
*备注	 ：
*****************************************************************************/
unsigned char TK_IsDoubleKeyOrSlideKey(void)
{
    return TKCFG[IsDoubleKey];
}

/****************************************************************************
*函数名称：unsigned char TK_GetCsFunction(void)
*函数功能：进行CS 测试
*入口参数：void
*出口参数：unsigned char SET_CS_FUNCTION
*备注	 ：
*****************************************************************************/
unsigned char TK_GetCsFunction(void)
{
	return SOCAPI_SET_CS_FUNCTION; 
}

/****************************************************************************
*函数名称：unsigned char TK_GetTKYzCnt(void)
*函数功能：获取按键抑制确认次数
*入口参数：void
*出口参数：返回抑制次数 
*备注	 ：
*****************************************************************************/
unsigned char TK_GetTKYzCnt(void)
{
	return (ConfirmTouchCnt/3);
}

/****************************************************************************
*函数名称：unsigned char TK_GetUpConfirmCnt(void)
*函数功能：检测按键弹起次数
*入口参数：void
*出口参数：返回按键弹起确认次数 
*备注	 ：
*****************************************************************************/
unsigned char TK_GetUpConfirmCnt(void)
{
	return ConfirmTouchCnt>>1;
}


/****************************************************************************
*函数名称：unsigned short int  TK_GetCurrFingerValue(unsigned char i)
*函数功能：获取当前的finger 值
*入口参数：unsigned char
*出口参数：unsigned short int 
*备注	 ：
*****************************************************************************/
unsigned short int TK_GetCurrFingerValue(unsigned char i)
{ 
	return	TKChannelCfg[i][FINGER_THRESHOLD_H]*256 + TKChannelCfg[i][FINGER_THRESHOLD_L] ;
}

/****************************************************************************
*函数名称：unsigned char TK_GetScanTimeValue(unsigned char i)
*函数功能：获取当前通道的扫描时间
*入口参数：unsigned char
*出口参数：unsigned char 
*备注	 ：
*****************************************************************************/
unsigned char TK_GetScanTimeValue(unsigned char i)
{ 
	return TKChannelCfg[i][SCANTIME];
}

/****************************************************************************
*函数名称：unsigned char TK_GetBaseLineAdjustValue(unsigned char i)
*函数功能：获取当前通道的基线调整
j
*入口参数：unsigned char
*出口参数：unsigned char 
*备注	 ：
*****************************************************************************/
unsigned char TK_GetBaseLineAdjustValue(unsigned char i)
{	
     return SingleParameterBufferSet[i].BaseLineAdjusttmp; 
}

/****************************************************************************
*函数名称：unsigned char  TK_GetCfgMsg(unsigned char i,unsigned char j)
*函数功能：获取Touch KEY 配置信息
*入口参数：void
*出口参数： 
*备注	 ：
*****************************************************************************/
unsigned char  TK_GetCfgMsg(unsigned char i,unsigned char j)
{
	switch(j)
	{ 
		case 0:  return TKChannelCfg[i][SET_TOUCH_FREQ]; 
		case 1:  return TKChannelCfg[i][SET_RESOLUTION];
		case 2:  return TKChannelCfg[i][SET_GAIN_CFG];
		case 3:  return TK_GetBaseLineAdjustValue(i);
		case 4:  return TKCFG[SET_ANTIJAM];
		case 5:  return TKChannelCfg[i][SET_RESOLUTION_FACTOR];
		case 6:  return TKChannelCfg[i][SET_ICHA_FACTOR];
		
		default:return 0; 	 	
	}
}

/****************************************************************************
*函数名称： short int TK_GetTKYzThreshold(unsigned char i)
*函数功能：获取按键抑制区间
*入口参数：unsigned char i
*出口参数：返回抑制区间
*备注	 ：
*****************************************************************************/
short int TK_GetTKYzThreshold(unsigned char i)
{	
	unsigned short int SetFingerThresholdtmp; 
	
	SetFingerThresholdtmp = TK_GetCurrFingerValue(i); 
    SetFingerThresholdtmp = SetFingerThresholdtmp*SOCAPI_INHIBITION_ZONE/10;

	return SetFingerThresholdtmp;
}


#if (TK_LowPowerMode == 1)
/****************************************************************************
*函数名称：unsigned short int  TK_GetCombineCurrFingerValue(unsigned char i)
*函数功能：获取当前的finger 值
*入口参数：unsigned char
*出口参数：unsigned short int 
*备注	 ：
*****************************************************************************/
unsigned short int TK_GetCombineCurrFingerValue(unsigned char i)
{ 
	return	TKCombineChannelCfg[i][FINGER_THRESHOLD_H]*256 + TKCombineChannelCfg[i][FINGER_THRESHOLD_L];
}

/****************************************************************************
*函数名称：unsigned char TK_GetCombineScanTimeValue(unsigned char i)
*函数功能：获取当前通道的扫描时间
*入口参数：unsigned char
*出口参数：unsigned char 
*备注	 ：
*****************************************************************************/
unsigned char TK_GetCombineScanTimeValue(unsigned char i)
{ 
	return TKCombineChannelCfg[i][SCANTIME];
}

/****************************************************************************
*函数名称：unsigned char TK_GetCombineBaseLineAdjustValue(unsigned char i)
*函数功能：获取当前通道的基线调整
j
*入口参数：unsigned char
*出口参数：unsigned char 
*备注	 ：
*****************************************************************************/
unsigned char TK_GetCombineBaseLineAdjustValue(unsigned char i)
{	
     return CombineParameterBufferSet[i].BaseLineAdjusttmp; 
}

/****************************************************************************
*函数名称：unsigned char  TK_GetCombineCfgMsg(unsigned char i,unsigned char j)
*函数功能：获取Touch KEY 配置信息
*入口参数：void
*出口参数： 
*备注	 ：
*****************************************************************************/
unsigned char  TK_GetCombineCfgMsg(unsigned char i,unsigned char j)
{
	switch(j)
	{ 
		case 0:  return TKCombineChannelCfg[i][SET_TOUCH_FREQ]; 
		case 1:  return TKCombineChannelCfg[i][SET_RESOLUTION];
		case 2:  return TKCombineChannelCfg[i][SET_GAIN_CFG];
		case 3:  return TK_GetCombineBaseLineAdjustValue(i);
		case 4:  return TKCFG[SET_ANTIJAM];
		case 5:  return TKCombineChannelCfg[i][SET_RESOLUTION_FACTOR];
		case 6:  return TKCombineChannelCfg[i][SET_ICHA_FACTOR];
		
		default:return 0; 	 	
	}
}

/****************************************************************************
*函数名称：short int TK_GetCombineTKYzThreshold(unsigned char i)
*函数功能：获取按键抑制区间
*入口参数：unsigned char i
*出口参数：返回抑制区间
*备注	 ：
*****************************************************************************/
short int TK_GetCombineTKYzThreshold(unsigned char i)
{	
	unsigned short int SetFingerThresholdtmp; 
	
	SetFingerThresholdtmp = TK_GetCombineCurrFingerValue(i); 
    SetFingerThresholdtmp = SetFingerThresholdtmp*SOCAPI_INHIBITION_ZONE/10;

	return SetFingerThresholdtmp;
}

#endif


/**************************************************
*函数名称：void TK_FilterDataDeal(unsigned char  i)
*函数功能：滤波函数
*入口参数：void
*出口参数：void
*备注	 ：
**************************************************/
void TK_FilterDataDeal(unsigned char  i)
{
	#if (T1_key ==1) 
	
	FilterDataDeal_T1(i);
	
	#elif (T2_key == 1)
	
	FilterDataDeal_T2(i);
	
	#elif (T2_key == 0) && (T1_key == 0) && (Slider_key == 1)
	
	FilterDataDeal_T1(i);
	
	#endif
}

/**************************************************
*函数名称：void TK_FilterCommCode(void)
*函数功能：
*入口参数：void
*出口参数：void
*备注	 ：
**************************************************/
void TK_FilterCommCode(void)
{
	#if (T1_key ==1) 
	
	TK_FilterCommCodeT1();
	
	#elif (T2_key == 1)
	
	TK_FilterCommCodeT2();
	
	#elif (T2_key == 0) && (T1_key == 0) && (Slider_key == 1)
	
	TK_FilterCommCodeT1();
	
	#endif
	
}

/**************************************************
*函数名称：void  TK_FilterAvergeDeal(void)
*函数功能：均值滑动滤波
*入口参数：void
*出口参数：void
*备注	 ：
**************************************************/
void TK_FilterAvergeDeal(void)
{
#if (TK_LowPowerMode == 0)	
	TK_FilterAvergeDeal_Normal();
#endif
#if (TK_LowPowerMode == 1)	
	TK_FilterAvergeDeal_Combine();
#endif
}

/****************************************************************************
*函数名称：void TK_SingleCurrentSensorChoose(void)
*函数功能：当前单通道选择
*入口参数：void
*出口参数：void
*备注	 ：
*****************************************************************************/
void TK_SingleCurrentSensorChoose(void) //根据不同应用类型设置  区分通道序列号
{
	unsigned char  i = 0;
	unsigned char  Ctk_Channel_mark = 0;
	unsigned int   CurrentSensorKey;
	unsigned int   CurrentSensorKey2; 	
	
#if (TK_LowPowerMode == 1)
	unsigned char  WakeUpKey_Channel_mark = 0;
	unsigned long long  CombineCurrentSensorKey;  	
#endif

#if (Slider_key == 1)
	unsigned char  j = 0,k = 0;
#endif
	
	CurrentSensorKey = SOCAPI_SET_TOUCHKEY_CHANNEL;
	CurrentSensorKey2 = SOCAPI_SET_TOUCHKEY_CHANNEL2; 	
	
	//通道号换算为序列号
	for(i=0;i<32;i++)
	{
		if((0x01UL << i) & CurrentSensorKey)
		{
			CurrentChannel[Ctk_Channel_mark] = i;						//选择触摸当前的通道
			
            #if (TK_LowPowerMode == 1)
			if(WakeUpKey_Channel_mark<WakeUpKeyNum)
            {
                if((WakeUpKeyChannel1&(0x01UL<<i)))
                {
                    WakeUpKey_List[WakeUpKey_Channel_mark++] = Ctk_Channel_mark;
                }
            }
            #endif
					
			Ctk_Channel_mark++;			
		}		
	}
	
	for(i=0;i<32;i++)
	{
		if((0x01UL << i) & CurrentSensorKey2)
		{
			CurrentChannel[Ctk_Channel_mark] = (i+32);					//选择触摸当前的通道
			
			#if (TK_LowPowerMode == 1)
			//挑选出属于低功耗唤醒通道的序列号
			if(WakeUpKey_Channel_mark<WakeUpKeyNum)
            {
                if((WakeUpKeyChannel2&(0x01UL<<i)))
                {
                    WakeUpKey_List[WakeUpKey_Channel_mark++] = Ctk_Channel_mark;
                }
            }
            #endif
						
			Ctk_Channel_mark++;
			if(Ctk_Channel_mark >= SOCAPI_SET_TOUCHKEY_TOTAL)
				break;
		}				
	}

#if (TK_LowPowerMode == 1)
	#if (SOCAPI_COMBINE_TOUCHKEY_TOTAL >= 1)	
	for(i=0;i<SOCAPI_COMBINE_TOUCHKEY_TOTAL;i++)
	{
		CombineCurrentSensorKey = (uint64_t)TKCombineChannels[i][0] + (((uint64_t)TKCombineChannels[i][1])<<8) + (((uint64_t)TKCombineChannels[i][2])<<16) + (((uint64_t)TKCombineChannels[i][3])<<24) + (((uint64_t)TKCombineChannels[i][4])<<32) + (((uint64_t)TKCombineChannels[i][4])<<40);
		CombineCurrentChannel[i] = CombineCurrentSensorKey;//取出对应并联的各通道号，用于后续并联通道确认某个并联有数据后，拆分单个通道扫描需要
	}
	
	#endif
	CombineCurrentChannelMax = SOCAPI_COMBINE_TOUCHKEY_TOTAL;
#endif
	
#if (Slider_key == 1)
		//挑选出属于滑轮滑条通道的序列号
		for(k = 0; k < UsingTKSlideModuleNumber; k ++)
		{
			for(i = 0;i<TKSlideModuleDealArray[k].UsingTKChannelNumber;i++)						//在CurrentChannel[]中取出滑条TK通道排列序号 
			{
				for(j=0;j<Ctk_Channel_mark;j++)
				{
					if(TKSlideModuleDealArray[k].TKChannel[i] == CurrentChannel[j])	//取序号
					{
						TKSlideModuleDealArray[k].TKOrderChannel[i]=j;	
					}
				}	
			}
		}
#endif
	
	SingleCurrentChannelMax = Ctk_Channel_mark;
	
}

/**************************************************
*函数名称：void TK_InitializeSensorFlag(void)
*函数功能：初始化变量
*入口参数：void
*出口参数：void
*备注	 ：
**************************************************/
void TK_InitializeSensorFlag(void)
{
	unsigned char  i;
	
	for(i =0; i< SingleCurrentChannelMax; i++)			
	{
		SingleParameterBufferSet[i].TouchCnt = 0;
	 	SingleParameterBufferSet[i].RestAreaCnt = 0;																
     	SingleParameterBufferSet[i].NoTouchCnt = 0;																
     	SingleParameterBufferSet[i].DifferAccum = 0;
		SingleParameterBufferSet[i].LowFingerDataCnt = 0;	 			
	}

#if (TK_LowPowerMode == 1)	
	for(i=0; i< CombineCurrentChannelMax; i++)
	{
		CombineParameterBufferSet[i].TouchCnt = 0;
	 	CombineParameterBufferSet[i].RestAreaCnt = 0;																
     	CombineParameterBufferSet[i].NoTouchCnt = 0;																
     	CombineParameterBufferSet[i].DifferAccum = 0;
		CombineParameterBufferSet[i].LowFingerDataCnt = 0;
		
	}
#endif
	
	KeyFlag =0;	
	FlowFingerRoundIndex = 0xff;
	FlowFingerRoundCnt = 0;
	bNeedUpdate = 0; 
	
}

/**************************************************
*函数名称：void TK_InitializeBaselines(void)
*函数功能：基线初始化
*入口参数：void
*出口参数：void
*备注	 ：
**************************************************/
void TK_InitializeBaselines(void)
{
	unsigned char  i;
	for(i=0;i< SingleCurrentChannelMax;i++)
	{
		SingleChannelsBaseLineUpdatePar[i].Baseline = SingleChannelsBaseLineUpdatePar[i].Rawdata; 		
	}
#if (TK_LowPowerMode == 1)	
	for(i=0; i< CombineCurrentChannelMax; i++)  
	{
		CombineChannelsBaseLineUpdatePar[i].Baseline = CombineChannelsBaseLineUpdatePar[i].Rawdata;
	}
#endif	
}

/**************************************************
*函数名称：void TK_FilterIIRInit(void)
*函数功能：初始化滤波
*入口参数：void
*出口参数：void
*备注	 ：
**************************************************/
void TK_FilterIIRInit(void)
{
	unsigned char  i;
	for(i=0; i< SingleCurrentChannelMax; i++)  
	{
		SingleChannelsBaseLineUpdatePar[i].FilterData = SingleChannelsBaseLineUpdatePar[i].Rawdata;
	}
#if (TK_LowPowerMode == 1)		
	for(i=0; i< CombineCurrentChannelMax; i++)  //初始化Filter滤波
	{
		CombineChannelsBaseLineUpdatePar[i].FilterData = CombineChannelsBaseLineUpdatePar[i].Rawdata;
	}
#endif	
}

/****************************************************************************
*函数名称：void TK_TouchKeyCFGInit(void)
*函数功能：初始化TK寄存器
*入口参数：void
*出口参数：void
*备注	 ：
*****************************************************************************/
void TK_TouchKeyCFGInit(void)
{
	unsigned char i;

#if (Slider_key == 1)
	unsigned char	k;
	UsingTKSlideModuleNumber = USING_TKSlideModule_Number;
#endif		
	
	ConfirmTouchCnt = TKCFG[CONFIRMTOUCHCNT];
	SetNoiseThreshold = TKCFG[NOISE];
	TK_SingleCurrentSensorChoose(); 
	for(i=0;i< SingleCurrentChannelMax;i++)
	{
		SingleParameterBufferSet[i].BaseLineAdjusttmp = TKChannelCfg[i][SET_ICHA];
	} 
	UpdateBaseLNum = 0; 
	MultipleLNum = 0;

#if (Slider_key == 1)
	for(k = 0; k < UsingTKSlideModuleNumber; k ++)
	{
		TKSlideModuleDealArray[k].UpdateBaseLineNumber = 0;
	}
#endif	
	
#if  (TK_LowPowerMode == 1)
	for(i=0;i<SOCAPI_COMBINE_TOUCHKEY_TOTAL;i++)
	{
		CombineParameterBufferSet[i].BaseLineAdjusttmp =TKCombineChannelCfg[i][SET_ICHA];	
	}
	
    BTM_Init();
#endif
	
}

#if (Slider_key == 1)
/****************************************************************************
*函数名称：char TKSlideModuleCHIs(unsigned char ch)
*函数功能：区分不同应用按键序列号
*入口参数：void
*出口参数：char
*备注	 ：
*****************************************************************************/
char TKSlideModuleCHIs(unsigned char ch)
{
	unsigned char i,k; 

	for(k = 0; k < UsingTKSlideModuleNumber; k++)
	{
		for(i=0;i<TKSlideModuleDealArray[k].UsingTKChannelNumber;i++)
		{	
			if(ch == TKSlideModuleDealArray[k].TKOrderChannel[i])	  //序号
			{	
				break;	
			}
		}
		if(i != TKSlideModuleDealArray[k].UsingTKChannelNumber)
			break;
	}
	
	if(k != UsingTKSlideModuleNumber)
	{
		return 1;
	}
	
	ch = ch;
	return 0;
}
#endif

/**************************************************
*函数名称：unsigned  long int TK_SensorKeyFlag(void)
*函数功能：出键处理函数
*入口参数：void
*出口参数：unsigned  int KeyFlag
*备注	 ：用于区分按键/滑轮滑条运行不同的处理函数
**************************************************/	
unsigned long long TK_SensorKeyFlag() //
{
	unsigned char i = 0;
	
	unsigned char SlideCount = 0;
	
	IsInNosicValue = 0; 
	BigNosicCount=0;
	
	//T1处理逻辑
	for(i=0;i<SingleCurrentChannelMax;i++)
	{
		SlideCount = 1;

#if (Slider_key == 1)	
		if(TKSlideModuleCHIs(i))
		{
			SlideCount=0;
		}
#endif
		
#if (T1_key == 1) 		
		FilterDataDeal_T1(i);
		
		if(SlideCount == 1)
		{			
			KeyT1_Analysis(i);
		}
		
#elif  (T2_key == 0) && (T1_key == 0) && (Slider_key == 1)		
		FilterDataDeal_T1(i);
		
		
#elif (T2_key == 1)
		FilterDataDeal_T2(i);
#endif
	}

	
#if (TK_LowPowerMode == 1)	
	for(i=0;i<CombineCurrentChannelMax;i++)
	{  			
		TK_CombineFilterDataDeal(i);			//IIR滤波
		TK_CombineAnalysis(i);	
	}
#endif
	
	
#if (T2_key == 1)	
	//T2处理逻辑
	if(SingleCurrentChannelMax>3)
	{
		if(BigNosicCount<=2)
		{
			KeyT2_Analysis();//T2是一轮扫描处理按键，需要在函数内判断序列号是否为常规按键
		}
	}
	else
	{
		KeyT2_Analysis();//T2是一轮扫描处理按键，需要在函数内判断序列号是否为常规按键
	}
	
	//消顶处理
	
	if(BigNosicCount!=0)
	{
	 	RemoveTopCount++;
		if(RemoveTopCount>(ConfirmTouchCnt*4))
		{
		    bNeedUpdate = 1; 	//强制基线复位
		}
	}
	else
	{
	  	RemoveTopCount=0;
	}
		
#endif
	
#if (Slider_key == 1)
	
	for(i=0;i<UsingTKSlideModuleNumber;i++)
	{
		TKSlideModuleSensorRenovate(&TKSlideModuleDealArray[i]);
	}	
	
#endif	
	
	return KeyFlag;	
}

/****************************************************************************
*函数名称：unsigned long int TK_TouchKeyScan(void)
*函数功能：检测按键接口
*入口参数：void
*出口参数：按键通道， 返回的是一个uint64_t , 通道数8个字节0x0000000000000000
*备注	 ：1,  调用触控库检测函数SensorKeyFlag()
		   2,  分析得出，哪个通道有按下，按下bit 位设置为1，否则为0
		   3,  检测是否需要立即更新baseline:  大于MAX_KEY_RESET_BASELINE 个按键按下时立即更新baseline
		   4,  双键或者单键按下时， 时间大于SetOneKeyPushResetTime()结果时更新baseline 
*****************************************************************************/
unsigned long long TK_TouchKeyScan(void)
{
#if (Slider_key == 1)
	unsigned char	k;
#endif	
	
	unsigned char   	t;
    unsigned char   	MultipleCnt = 0;//按键计数
	unsigned long long	Keyvalue; 
	unsigned long long	KeyData = 0; 

#if (TK_LowPowerMode == 1)	
	short int WakeupDiffData = 0; 
	short int WakeupSetFingerThresholdtmp;
#endif
	
	if(WakeUp_Flag == 0)
	{
		if(TK_GetIsNeedUpdateBaseline() == 0)
		{
			Keyvalue = TK_SensorKeyFlag();
			
			//区分T1/T2/滑轮滑条按键处理逻辑
			for(t=0;t< SingleCurrentChannelMax;t++)
			{
				if(((unsigned long long)0x01 << t) & Keyvalue)
				{
					KeyData |= ((unsigned long long)0x01 << (CurrentChannel[t]));
					
					#if (T1_key == 1)			
					MultipleCnt++;	
					
					#elif (T2_key == 0) && (T1_key == 0) && (Slider_key == 1)		
					MultipleCnt++;	
					
					#elif (T2_key == 1)
					break;		
					#endif
					
				}
			}                

			#if (T1_key == 1)			
			
			if(MultipleCnt >= 2) 	 									//进入多按键处理
			{			
				bMultiple = 1;			
				if(MultipleCnt >= SOCAPI_MAX_KEY_NUM_INVALID)
				{
					TK_SetNeedUpdateBaseline(); 							// 立即更新baseline ,例如亚克力板盖上去
				}
				else
				{					
					if(TK_IsDoubleKeyOrSlideKey())
					{
						bMultiple = 0;
					} 				 
				}			
			}			

			if(bMultiple == 0)							//进入按键判断
			{		
				if(KeyData != 0x0)					    //单个按键达到多长时间就update baseline ,松手检测
				{			
					UpdateBaseLNum++; 
				}
				else	
				{
					UpdateBaseLNum = 0; 	
				} 
			}	
			else
			{   
				//考虑基线更新		
				MultipleLNum++; 
				KeyData = 0x00;
			}

			
					
			if(MultipleLNum >SOCAPI_MAX_KEY_MUTIPLE)		  //干扰计数大于最大计数更新基线
			{
				TK_SetNeedUpdateBaseline(); 
				MultipleDealTpye = 1; 
				MultipleLNum = 0;
			}		
			
			#elif (T2_key == 1)		
			if(KeyData !=0x0)					    //单个按键达到多长时间就update baseline ,松手检测
			{
				UpdateBaseLNum++; 
			}
			else	
			{
				UpdateBaseLNum = 0; 	
			}
			#endif		
			
			if(UpdateBaseLNum >= TK_SetOneKeyPushResetTime())	  //按键超出最长输出时间更新基线
			{
				TK_SetNeedUpdateBaseline(); 
				UpdateBaseLNum = 0;
			}
			
			
			#if (Slider_key == 1)
		
			for(k = 0; k < UsingTKSlideModuleNumber; k ++)
			{
				if(TKSlideModuleDealArray[k].OutValue != 0x00)					        //单个按键达到多长时间就update baseline ,松手检测
				{	
					if(TKSlideModuleDealArray[k].OutValue == TKSlideModuleDealArray[k].LastOutValue)
					{		
					   TKSlideModuleDealArray[k].UpdateBaseLineNumber++; 
					}
					else
					{
					   TKSlideModuleDealArray[k].LastOutValue = TKSlideModuleDealArray[k].OutValue;
					   TKSlideModuleDealArray[k].UpdateBaseLineNumber = 0; 
					}
				}
				else	
				{
					TKSlideModuleDealArray[k].UpdateBaseLineNumber = 0; 	
				}

				if(TKSlideModuleDealArray[k].UpdateBaseLineNumber >= TK_SetOneKeyPushResetTime())	  //按键超出最长输出时间更新基线
				{
					TK_SetNeedUpdateBaseline(); 
					TKSlideModuleDealArray[k].UpdateBaseLineNumber = 0;
				}
			}
			#endif
			
			
		}
		else
		{
			
			#if (T1_key == 1) 
			TK_MultipleDeal(TKCFG[AUTO_UPDATE_TIME],1);  	//基线复位处理
			#elif (T2_key == 1) 	
			TK_MultipleDeal(TKCFG[AUTO_UPDATE_TIME],0);  	//基线复位处理
			#elif (T2_key == 0) && (T1_key == 0) && (Slider_key == 1)	
			TK_MultipleDeal(TKCFG[AUTO_UPDATE_TIME],1);  	//基线复位处理
			#endif
			
		}		
		
	}
	else
	{
		#if (TK_LowPowerMode == 1)
		
		if(Touch_WakeUpFlag==1)
		{
			CumulateCount ++;
			
			KeyData |= ((unsigned long long)0x01 << (CurrentChannel[WakeUpKeyValue]));

			WakeupDiffData = SingleChannelsBaseLineUpdatePar[WakeUpKeyValue].Rawdata - SingleChannelsBaseLineUpdatePar[WakeUpKeyValue].Baseline;
			WakeupSetFingerThresholdtmp = TK_GetCurrFingerValue(WakeUpKeyValue);			

			if(WakeupDiffData <= (WakeupSetFingerThresholdtmp-((WakeupSetFingerThresholdtmp)>>2)))
			{	
				NumCount=0;
				if(++OffHandCount>5)
				{
					OffHandCount = 0;
					WakeUp_Flag = 0;
					Touch_WakeUpFlag=0;
					KeyData = 0;
					
				}
				for(t=0;t<SingleCurrentChannelMax;t++)
				{
					TK_FilterDataDeal(t);
					if(!WakeUp_Flag)
					{
						if(WakeUpKeyValue == t)
						continue;
						SingleChannelsBaseLineUpdatePar[t].Baseline = SingleChannelsBaseLineUpdatePar[t].Rawdata;	
					}
				}
					
			}
			else
			{
				OffHandCount=0;
				for(t=0;t<SingleCurrentChannelMax;t++)
				{
					TK_FilterDataDeal(t);	
				}
				if(++NumCount >= TK_SetOneKeyPushResetTime())	//按键超出最长输出时间更新基线
				{
		 			TK_SetNeedUpdateBaseline(); 
					NumCount = 0;
					WakeUp_Flag = 0;
					Touch_WakeUpFlag=0;
					KeyData = 0;
				}		
			}
			
			if(CumulateCount >= TK_SetOneKeyPushResetTime())
			{
				TK_SetNeedUpdateBaseline(); 
				NumCount = 0;
				OffHandCount = 0;
				CumulateCount = 0;
				WakeUp_Flag = 0;
				Touch_WakeUpFlag=0;
				KeyData = 0;
			}
			
		}
		else
		{
			if(++WakeUpThenScanCount>5)
			{
				WakeUpThenScanCount = 0;
				WakeUp_Flag = 0;
			}
			for(t=0;t<SingleCurrentChannelMax;t++)
			{
				TK_FilterDataDeal(t);
				if(!WakeUp_Flag)
				{
					
					SingleChannelsBaseLineUpdatePar[t].Baseline = SingleChannelsBaseLineUpdatePar[t].Rawdata;	
				}	
			}
			
		}
		
		#endif
	}
	
	return KeyData;
}	

/****************************************************************************
*函数名称：void TK_IRQHandler(void) 
*函数功能：触摸中断服务函数
*入口参数：void
*出口参数：void
*备注	 ：
*****************************************************************************/
void TK_IRQHandler(void)
{
	if(LowPowerScan_Flag == 0)
	{
		#if (T1_key == 1)  && (TK_LowPowerMode == 0)	
		TouchKey_Service(1); //中断服务函数
		
		#elif (T2_key == 1) && (TK_LowPowerMode == 0)	
		TouchKey_Service(0); //中断服务函数
		
		#elif (T2_key == 0) && (T1_key == 0) && (Slider_key == 1) && (TK_LowPowerMode == 0)	
		TouchKey_Service(1); //中断服务函数
		#endif

		#if (T1_key == 1)  && (TK_LowPowerMode == 1)	
		TouchKeyCombine_Service(1);
		
		#elif (T2_key == 1) && (TK_LowPowerMode == 1)		
		TouchKeyCombine_Service(0);
		
		#elif (T2_key == 0) && (T1_key == 0) && (Slider_key == 1) && (TK_LowPowerMode == 1)
		TouchKeyCombine_Service(1);	
		#endif
	}
	else
	{
		TK_CLR_TKIF();
		TK_WakeUp_Flag = 1;
	}
}


#if (TK_LowPowerMode ==1)

//**************************************************
//*函数名称：unsigned char GetLowPowerScanFlag(void)
//*函数功能获取低功耗模式
//*入口参数：void
//*出口参数：void  
//**************************************************/
unsigned char GetLowPowerScanFlag(void) //获取低功耗模式
{	
    return LowPowerScan_Flag;
}


/**************************************************
*函数名称：void TouchKey_LowPower_Init(void)
*函数功能低功耗初始化
*入口参数：void
*出口参数：void  
**************************************************/
void TouchKey_LowPower_Init(void) //低功耗初始化
{
	unsigned char i;

	ScanTimeTemp0 = (TK_ScanTime/(TK_GetCfgMsg(0,0)+1)); 
	
    for(i=0;i<WakeUpKeyNum;i++)
    {
        WakeUpKeyValue = WakeUpKey_List[i];
		
        ScanTimeTemp_Low[WakeUpKeyValue] = ScanTimeTemp0*TKChannelCfg[WakeUpKeyValue][SCANTIME]>> ScanTimeCon;
        BaseLineAdjusttmp_Low[WakeUpKeyValue] = SingleParameterBufferSet[WakeUpKeyValue].BaseLineAdjusttmp;

        SetFingerThresholdtmp_Low[WakeUpKeyValue] = TKChannelCfg[WakeUpKeyValue][FINGER_THRESHOLD_H]*256+TKChannelCfg[WakeUpKeyValue][FINGER_THRESHOLD_L];
    }
	
	TK_TKTM_SET(ScanTimeTemp_Low[WakeUpKey_List[0]]);//TKTM设置	
	TK_VREF_SET(TK_GetCfgMsg(WakeUpKey_List[0],2)); //VREF设置
	TK_CTIME_SET(TK_GetCfgMsg(WakeUpKey_List[0],0));//Ctime设置
	TK_ICHAC_SET(TK_GetCfgMsg(WakeUpKey_List[0],6)); //ICHAC设置
	TK_ICHA_SET(TK_GetCfgMsg(WakeUpKey_List[0],3));  //ICHA设置
	TK_ICHGC_SET(TK_GetCfgMsg(WakeUpKey_List[0],5)); //ICHGC设置
	TK_ICHG_SET(TK_GetCfgMsg(WakeUpKey_List[0],1)); //ICHG设置
	
	TK_TKIS_SET(CurrentChannel[WakeUpKey_List[0]]); //配置当前通道CurrentIndex
}

/**************************************************
*函数名称：void TouchKey_LowPower_CombineScan(void)
*函数功能进入低功耗模式下并联扫描
*入口参数：void
*出口参数：void  
**************************************************/
void TouchKey_LowPower_CombineScan(void)
{
	TK_WakeUp_Flag = 0;
	
	CombineSetFingerThresholdtmp_Low[0] = TKCombineChannelCfg[0][FINGER_THRESHOLD_H]*256+TKCombineChannelCfg[0][FINGER_THRESHOLD_L]; 
	
	ScanTimeTemp0 = (TK_ScanTime/(TK_GetCombineCfgMsg(0,0)+1)); 
	
	CombineScanTimeTemp_Low[0] = (ScanTimeTemp0*TKCombineChannelCfg[0][SCANTIME])>>ScanTimeCon;
	
	TK_TKTM_SET(CombineScanTimeTemp_Low[0]);//TKTM设置
	
	TK_VREF_SET(TK_GetCombineCfgMsg(0,2)); //VREF设置

	TK_CTIME_SET(TK_GetCombineCfgMsg(0,0));//Ctime设置

	TK_ICHAC_SET(TK_GetCombineCfgMsg(0,6)); //ICHAC设置
	TK_ICHA_SET(TK_GetCombineCfgMsg(0,3)); //ICHA设置
	
	TK_ICHGC_SET(TK_GetCombineCfgMsg(0,5)); //ICHGC设置
	TK_ICHG_SET(TK_GetCombineCfgMsg(0,1)); //ICHG设置
	
	TK_TKIS_CombineSET(CombineCurrentChannel[0]);
	
}

/**************************************************
*函数名称：void TouchKey_IntoLowPowerMode(void)
*函数功能进入低功耗模式
*入口参数：void
*出口参数：void  
**************************************************/
void TouchKey_IntoLowPowerMode(void)
{	
	
	unsigned char t;
	
	if(TK_GetIsNeedUpdateBaseline() == 0)
	{		
		LowPowerScan_Flag = 1;
		
		LowPowerEnter_Flag = 1;			
		
		TouchKey_LowPower_CombineScan();		
	   
		for(t=0;t<SingleCurrentChannelMax;t++)
		{
		   SingleParameterBufferSet[t].TouchCnt = 0;
		}
		for(t=0;t<CombineCurrentChannelMax;t++)
		{
		   CombineParameterBufferSet[t].TouchCnt=0;		
		}

	}
	else
	{
		TK_TRIG();	
	}

}

/**************************************************
*函数名称：void TouchKey_QuitLowPowerMode(void)
*函数功能退出低功耗模式
*入口参数：void
*出口参数：void  
**************************************************/
void TouchKey_QuitLowPowerMode(void)
{
	LowPowerScan_Flag = 0;
	WakeUp_Flag = 1;
	
	ScanTimeTemp0 = (TK_ScanTime/(TK_GetCfgMsg(0,0)+1)); 

	TK_TKTM_SET(ScanTimeTemp0*TK_GetScanTimeValue(0)>> ScanTimeCon);//TKTM设置
	
	TK_VREF_SET(TK_GetCfgMsg(0,2)); //VREF设置

	TK_CTIME_SET(TK_GetCfgMsg(0,0));//Ctime设置

	TK_ICHAC_SET(TK_GetCfgMsg(0,6)); //ICHAC设置
	TK_ICHA_SET(TK_GetCfgMsg(0,3));  //ICHA设置

	TK_ICHGC_SET(TK_GetCfgMsg(0,5)); //ICHGC设置
	TK_ICHG_SET(TK_GetCfgMsg(0,1)); //ICHG设置

	TK_TKIS_SET(CurrentChannel[0]); //配置当前通道CurrentIndex
	 
	NVIC_DisableIRQ(TK_IRQn); //关闭NVIC TK中断
	TK_TKINT_SET(0);//关闭TK中断请求
    
	TK_TRIG();
	while(TK_GET_TKIF == 0);
	
	TK_CLR_TKIF();
	
	NVIC_EnableIRQ(TK_IRQn); //开NVIC TK中断
	TK_TKINT_SET(1);//使能TK中断请求
	
	TK_TRIG();
 
    Customer_QuitLowPowerMode_Init();
	
}

/**************************************************
*函数名称：void  TKSleepMode(void)
*函数功能进入STOP模式
*入口参数：void
*出口参数：void  
**************************************************/
void  TKSleepMode(void)
{   
	SCB->SCR |= (unsigned int)0x01<<2;//进入STOP模式
	
	__WFI();
	
	SCB->SCR &= ~((unsigned int)0x01<<2);
}

/**************************************************
*函数名称：void TouchKey_LowPower_Dispose(void)
*函数功能低功耗扫描数据处理
*入口参数：void
*出口参数：void  
**************************************************/
void TouchKey_LowPower_Dispose(void)
{
	short int  differData; 
	unsigned char WakeUpKey_Index;
	unsigned char i;
	
	BTM_WakeUpFlag = 0;
	TK_WakeUp_Flag = 0;
	
	TK_TRIG(); //单次扫描
    TKSleepMode(); 

	while(TK_WakeUp_Flag == 0);
	TK_WakeUp_Flag = 0;
	
	for(i=0;i<10;i++){}

    TK_TRIG(); //单次扫描
    TKSleepMode(); 
	
	while(TK_WakeUp_Flag == 0);
	TK_WakeUp_Flag = 0;
	
	for(i=0;i<10;i++){}
	
	for(WakeUpKey_Index=0; WakeUpKey_Index<WakeUpKeyNum; WakeUpKey_Index++)	//循环扫描按键
	{
        WakeUpKeyValue = WakeUpKey_List[WakeUpKey_Index];
		
		TK_TKTM_SET(ScanTimeTemp_Low[WakeUpKeyValue]);//TKTM设置
		TK_ICHAC_SET(TK_GetCfgMsg(WakeUpKeyValue,6)); //ICHAC设置
		TK_ICHA_SET(BaseLineAdjusttmp_Low[WakeUpKeyValue]);  //ICHA设置

		TK_TKIS_SET(CurrentChannel[WakeUpKeyValue]); //配置当前通道CurrentIndex
		
		TK_TRIG(); //单次扫描
		TKSleepMode(); 		

		while(TK_WakeUp_Flag == 0);
		TK_WakeUp_Flag = 0;

		for(i=0;i<10;i++){}
		
		SingleChannelsBaseLineUpdatePar[WakeUpKeyValue].Rawdata = ((TK->TK_CNT)<<ScanTimeCon);   
		
		TK_FilterDataDeal(WakeUpKeyValue);
		differData = SingleChannelsBaseLineUpdatePar[WakeUpKeyValue].Rawdata - SingleChannelsBaseLineUpdatePar[WakeUpKeyValue].Baseline ; 
		if(differData >= SetFingerThresholdtmp_Low[WakeUpKeyValue])
		{
			 SingleKeyFastScan_Flag = 1;
			 break;       
		} 
		else if(differData > (-(SetFingerThresholdtmp_Low[WakeUpKeyValue]>>1)))        
		{
			if(WakeUpNum==WakeUpKey_Index%BaselineUpdateCnt)
				SingleChannelsBaseLineUpdatePar[WakeUpKeyValue].Baseline += differData>>4;            
		}
		else
		{
			SingleChannelsBaseLineUpdatePar[WakeUpKeyValue].Baseline += differData>>2; 
		}
		          
	}
	
	if(SingleKeyFastScan_Flag == 0)
	{
		TouchKey_LowPower_CombineScan();
	}

	if(++WakeUpNum>=BaselineUpdateCnt)
	{
		WakeUpNum = 0;
	}
}

/**************************************************
*函数名称：void SingleKeyFastScan(void)
*函数功能单按键快速扫描模式
*入口参数：void
*出口参数：void  
**************************************************/
void SingleKeyFastScan(void)
{
	unsigned char i;
    short int differData; 
    short int SetFingerThresholdtmp;
	
	SingleKeyFastScan_Flag = 0;
	TK_WakeUp_Flag = 0;

	TK_TKINT_SET(1);//使能TK中断请求
	
	TK_TKTM_SET(ScanTimeTemp_Low[WakeUpKeyValue]);//TKTM设置
	TK_ICHAC_SET(TK_GetCfgMsg(WakeUpKeyValue,6)); //ICHAC设置
	TK_ICHA_SET(BaseLineAdjusttmp_Low[WakeUpKeyValue]);  //ICHA设置
	
	TK_TKIS_SET(CurrentChannel[WakeUpKeyValue]); //配置当前通道CurrentIndex
			
	for(i=0;i<TK_WakeUpConfirmTouchCnt;i++)
    {        
		TK_TRIG(); //单次扫描
		TKSleepMode(); 
		while(TK_WakeUp_Flag == 0);
		TK_WakeUp_Flag = 0;
		
		SingleChannelsBaseLineUpdatePar[WakeUpKeyValue].Rawdata = (TK->TK_CNT)<<ScanTimeCon;   
		
		TK_FilterDataDeal(WakeUpKeyValue);
		differData = SingleChannelsBaseLineUpdatePar[WakeUpKeyValue].Rawdata - SingleChannelsBaseLineUpdatePar[WakeUpKeyValue].Baseline ;
		
		SetFingerThresholdtmp = TK_GetCurrFingerValue(WakeUpKeyValue);

		 if(differData >= SetFingerThresholdtmp )
         {   
            SingleParameterBufferSet[WakeUpKeyValue].TouchCnt++;  
         }             
         else
		 {
			TouchKey_LowPower_CombineScan();
			break;
		 }
    }
	
    if(SingleParameterBufferSet[WakeUpKeyValue].TouchCnt>=TK_WakeUpConfirmTouchCnt)
    {    
		 TouchKey_QuitLowPowerMode();       
		 Touch_WakeUpFlag= 1; 
		 CumulateCount = 0;
	}
    else
    {
        SingleParameterBufferSet[WakeUpKeyValue].TouchCnt = 0;
    }
	
}


/**************************************************
*函数名称：void LowPower_Touchkey_Scan(void)
*函数功能低功耗模式TK扫描
*入口参数：void
*出口参数：void  
**************************************************/
void LowPower_Touchkey_Scan(void)
{	
	int16_t  CombinedifferData;
	if(LowPowerEnter_Flag == 1) //进低功耗标志判断，固定写法，用户不能删除，
	{
		LowPowerEnter_Flag = 0; //低功耗标志清零，固定写法，用户不能删除
		
		Customer_IntoLowPowerMode_Init();
	}
	
    TK_ENTKS_SET(0); 	//关闭TK电源
	
	TKSleepMode(); 
    
	TK_ENTKS_SET(1);	//打开TK电源
	
	//进行按键处理
	if(BTM_WakeUpFlag)
	{
		BTM_WakeUpFlag = 0;
		TK_WakeUp_Flag = 0;		
		
		TK_TRIG(); //单次扫描
		TKSleepMode(); 

		while(TK_WakeUp_Flag == 0);
		TK_WakeUp_Flag = 0;
		
		TK_TRIG(); //单次扫描
		TKSleepMode(); 
		while(TK_WakeUp_Flag == 0);
		TK_WakeUp_Flag = 0;	
		
		TK_TRIG(); //单次扫描
		TKSleepMode(); 
		while(TK_WakeUp_Flag == 0);
		TK_WakeUp_Flag = 0;
		
		CombineChannelsBaseLineUpdatePar[0].Rawdata = TK->TK_CNT << ScanTimeCon;
		TK_CombineFilterDataDeal(0);
		
		CombinedifferData = CombineChannelsBaseLineUpdatePar[0].Rawdata - CombineChannelsBaseLineUpdatePar[0].Baseline;
		
		if(CombinedifferData >= CombineSetFingerThresholdtmp_Low[0])
		{				
			TouchKey_LowPower_Init(); 
			TouchKey_LowPower_Dispose();  	   //检测按键	
		}
		else if(CombinedifferData > (-(CombineSetFingerThresholdtmp_Low[0]>>1)))        
		{
			CombineChannelsBaseLineUpdatePar[0].Baseline += CombinedifferData>>4;            
		}
		else
		{
			CombineChannelsBaseLineUpdatePar[0].Baseline += CombinedifferData>>2; 
		}	
		
		if( SingleKeyFastScan_Flag == 1)
	    {								   
			SingleKeyFastScan();		   //若有按键信息进入单按键快速多次扫描确定按键是否真实信号。
	    }
	
		// 用户BTM唤醒后的处理函数
		Customer_BTM_Dispose();
    }
	//其他唤醒请在以下分支进行判断并调用退出低功耗函数TouchKey_QuitLowPowerMode()
	//例如：if(外部中断唤醒标志==1)
	//		{
	//			TouchKey_QuitLowPowerMode();
	//			//业务层唤醒后实现逻辑
	//		}
	//
	//
	//
	{
			//非BTM唤醒，用户根据需要自行增添处理程序
	}
   
}

#endif
















