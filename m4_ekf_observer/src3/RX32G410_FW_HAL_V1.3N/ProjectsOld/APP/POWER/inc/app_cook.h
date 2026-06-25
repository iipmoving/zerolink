/********************************************************************************
    FileName    :  app_cook.h
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  按照菜单设定烹饪过程

    Date        :  2018-09-16
    Modify      :
                   2018-09-16 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
#ifndef APP_COOK_H
#define APP_COOK_H

#include "data_type.h"

typedef struct menu_attr_flg    //菜单属性标志
{
    INT8U MenuType  :4;     //菜单类型
    INT8U TimeAdj   :1;     //时间是否可调 0不，1可
    INT8U ReserveEn :1;     //是否可预约
    INT8U StewJuice :3;     //收汁
} MENU_ATTR_FLG;
typedef struct menu_attr        //菜单属性描述
{
    INT32U TimeMin;     //最短可调时间
    INT32U TimeMax;     //最长可调时间分钟
    INT32U TimeDefault; //默认时间
    INT32U TimeStep;    //调节步长
    MENU_ATTR_FLG  Flg;        //标志
} MENU_ATTR;
typedef  struct menu_step_flg       //烹饪步骤标志字
{
    INT8U TempNext      :1;     /*温度到了自动转下一步*/
    INT8U TimeAdj       :1;     /*时间修正*/
    INT8U Alarm         :1;     /*提醒*/
    INT8U warmFlag      :5;     /*保温模式*/
} MENU_STEP_FLG;

typedef struct
{
    INT16U LOW;
    INT16U HIGH;
} WARM_TEMP;
typedef struct
{
    INT16U First;
    INT16U Second;
} AUTO_CHANGE_TEMP;
typedef struct cook_menu_step        //烹饪菜单步骤设置
{
    INT8U Step;         //步骤，高4位总步骤，低4位当前步骤
    INT8U HeatMode;     //加热模式，最高位1，表明是煮粥，汤
    INT8U Power;        //加热功率,功率档位
    INT16U Temp;         //目标温度
    INT32U Time;        //步骤时间
    MENU_STEP_FLG Flg;  //步骤标志
} COOK_MENU_SET;


typedef  struct        //烹饪步骤标志字
{

    INT8U NoWaterProtect:1	;		//无水保护
	INT8U NoWaterAlarm	:1	;		//干烧报警
	INT8U ShuttleAlarm	:1	;	
    INT8U isOverflowConfirm:1;     //溢出确认标志
    INT8U isOverflow:1;            //溢出标志	
    INT8U isChangePower:1;
    INT8U isBoil:1;               //沸腾标志	
	
	
}MenuStatusFlagDef;				//菜单程序位标志

typedef struct cook_menu_def_
{
    INT8U MenuIdx;      //菜单编号
	INT8U LastIdx;
	
	MenuStatusFlagDef flag;		//菜单函数标志位
	
    INT8U StepAll;     //菜单总步骤数
    INT8U StepCur;      //当前步骤

    INT32U TimeAll;     //总时间，单位分钟
    INT32U TimeCur;     //当前走时，单位分钟

	INT32U	UsrMenuTime;	//用户菜单时间
    //当前步骤变量
    INT8U Power;        //加热功率
	INT8U ProteectPower;
    INT16U TempTarget;   //目标温度
    INT32U TimeStepAll;  //步骤总时间
    INT32U TimeStepCur; //步骤已用时间

    //温度上升斜率计算
    INT16U TempBotHead;
    INT16U TempBotTail;  //
	INT16U TempSideHead;
    INT16U TempSideTail;  //
	
	
    INT8U 	SlopCaclCnt;   // 斜率判断时间
    INT16S 	TempBotSlop;     //斜率
	INT16S 	TempSideSlop;    //斜率
    INT8U 	NoWaterConfirm;  //无水干烧判断次数


    //沸点相关
    INT16U 	TempBoil;            //沸点
    INT16U 	BoilConfirm;         //沸腾判断次数
    INT16U 	BoilCnt;             //沸腾计时
	

    INT8U 	TempSwitchDly;        //温度到切换次数确认
    INT8U 	LateStepAdj;          //煮饭模式最后时间可调

    INT8U 	overflowDetCnt;       //溢出计数
    INT16U 	overflowTimeoutCnt;  //溢出停功率计时
    INT16U 	overflowContinueCnt; //溢出持续计数
    INT8U 	noOverflowCnt;        //无溢出计数

    INT8U HitCnt;               //加热计数
    INT8U StopHitCnt;           //停止加热计数
    INT8U StewJuiceConfim;      //收汁确认计数
    INT8U StewJuiceSta;         //收汁状态
    INT8U StewJuiceCnt;         //收汁次数

    INT8U TempCheckCnt;         //点检计数
    INT8U TempCheckConfim;

    INT16U 	CycleHeatCnt;        //循环加热计时
    INT8U 	CycleHeatSta;         //循环加热状态
    INT8U 	CycleHeatTimes;       //循环加热计数

    INT8U 	SoakSta;              //浸泡状态

    INT8U 	LongBeep;
    INT16U 	LongBeepCnt;

    INT8U 	PorridgeBoilCnt;


    COOK_MENU_SET  *	MenuSet;    //标准菜单项
    MENU_ATTR *		Attr;
}COOK_MENU_DEF;

enum    //加热模式
{
    HEAT_MODE_STOP = 0,         /*停功率，不加热*/
    HEAT_MODE_POWER,            //指定功率加热
	HEAT_MODE_POWER_TEMP,            //指定功率加热 & 指定温度恒温	
    HEAT_MODE_POWER_AUTO_CHANGE,   //指定功率加热,溢出自动转功率
    HEAT_MODE_TOBOIL,           //加热到沸腾，从当前温度加热到沸腾
    HEAT_MODE_TOBOIL_PORRIDGE,	//粥类烹饪，到沸腾
    HEAT_MODE_TEMP,             //指定温度保温
    HEAT_MODE_STEW_JUICE,       //焖汁
    HEAT_MODE_TEMP_CHECK,       //点检
    HEAT_MODE_SOAK,             //浸泡
    HEAT_MODE_CYCLE,            //循环加热
    HEAT_MODE_ADJUST,
		HEAT_MODE_POWER_AUTO_CHANGE_2,//温度到转功率
};

//====保温温度判断值
#define TEMP_ERR_CLOSE_VALUE    9       //温度接近值，目前8bit adc只能分辨0.5°，这里用0.9和1区分
#define TEMP_ERR_SMALL_VALUE    20      //温度小偏差值
#define TEMP_ERR_LARGE_VALUE    30      //温度大偏差值

#define NO_WATER_TEMP_SLOP      160     //干烧判断斜率
#define NO_WATER_BOT_TEMP       1750     //干烧底部温度
#define NO_WATER_CONFIRM_CNT    5       //连续20次，认为无水干烧

//===沸腾判断
#define TEMP_BOIL_CONFIRM_CNT   5       //沸腾判断次数


enum _rice_amount
{
    RICE_AMOUNT_UNKNOWN = 0,    /*未知米量*/
    RICE_AMOUNT_XS,      //小小米量
    RICE_AMOUNT_S,      //小米量
    RICE_AMOUNT_M,      //中米量
    RICE_AMOUNT_L,      //大米量
    RICE_AMOUNT_XL,
    RICE_AMOUNT_MAX = RICE_AMOUNT_XL
};
enum menu_type          //菜单类型
{
    MENU_TYPE_RICE = 0, //煮饭
    MENU_TYPE_DRYPOT,   //干锅
    MENU_TYPE_WARM,     //保温
    MENU_TYPE_BRAISED,  //焖
    MENU_TYPE_SOUP,     //炖，汤
    MENU_TYPE_HOTPOT,   //火锅
    MENU_TYPE_PORRIDGE, //煮粥
    MENU_TYPE_STEW,     //收汁
	MENU_TYPE_CHICKEN,	//鸡	
	MENU_TYPE_SHUTTLE,	//班车模式
    MENU_TYPE_BUUBLE,   //泡
	MENU_TYPE_BaoChao,   //爆炒
	

};


enum 
{
  NoneIdx,                       	//无菜单情况
	DryPotIdx,            			//1干锅
	CookNoodleIdx,							//2煮面条
	CookTrepangIdx,							//3发海参
	DryFishIdx,            			//4干烧鱼
  BigPorridgeIdx,           	//5大米粥
  SoupIdx,	                	//6煲靓汤
  PheasantIdx,              	//7黄焖鸡
  StePorkIdx,               	//8炖排骨
	ClamMeatIdx,              	//9焖酥肉
  BraisedPorkIdx,           	//10红烧肉
  HotPotIdx,                	//11火锅
	
	
  KeepWarmIdx,              	//12保温
  KeepWarm_STEW_Idx,        	//13保温(炖煮,粥)
  StewJuice1Idx,            	//14收汁1
  StewJuice21Idx,           	//15收汁2
	StewJuice31Idx,				//16收汁3
	StewJuice41Idx,				//17收汁4
	
	ColaChickenCingsIdx,				//可乐鸡翅			//18		
	SweetSourPorkRibsIdx,			//糖醋排骨				//19
	HandGraspingMuttonIdx,			//手抓羊肉			//20
	FermentedBeanCurdHoofIdx,	//腐乳蹄					//21
	SauceBraisedBeefIdx,				//酱焖牛				//22


	PickledFishIdx,						//酸菜鱼					//23
	SeafoodFoupIdx,						//海鲜汤24
	StewedChickenIdx,					//清炖鸡25

	StewLambIdx,						//炖羊肉26
	TalkOldChickenIdx,					//煲老鸡27
	ScallionChickenIdx,					//葱油鸡28
//BoiledChickenTable,					//白斩鸡28

	ChickenJuiceCabbageIdx,		//鸡汁白菜31
	NortheastStewIdx,					//东北乱炖	30		
	BoiledDumplingsIdx,				//煮饺子31

	BoiledEggIdx,								//煮鸡蛋32
	StandardRiceIdx,							//焖米饭33	

	DF_stir_fried_greens,
	DF_stir_fried_greens2,
	DF_stir_fried_greens3,
//ShreddedPorkCeleryTable,			//芹菜肉丝(以下直接干锅模式）
//BraisedPorkTable,						//鱼香肉丝
//BeanCurdTable,								//麻婆豆腐
//BraisedBambooShootsTable,		//油焖笋
//BraisedShrimpTable,					//油焖虾					//24
//GingerBakedCrabTable,				//姜葱焗蟹				//25
//PastaTable,									//意面	
	
	StopHeatIdx,					//18停止加热10分钟
	


	
	
	
    //==============老化
    FactoryIdx,


};







#define MENU_IDX_CookNoodle 				2
#define MENU_IDX_DRYFISH       				4
#define MENU_IDX_Pheasant					7
#define MENU_IDX_ClamMeat   				9
#define MENU_IDX_BraisedPork				10
#define MENU_IDX_HOTPOT						11		//火锅

#define MENU_IDX_AVAILABLE  				11

#define MENU_IDX_MAX        				StopHeatIdx		//菜单编号的最大值
#define MENU_IDX_WARM       				12		//保温
#define MENU_IDX_WARM_STEW  				13		//保温(炖煮)
#define MENU_IDX_STEW1      				14		//收汁1
#define MENU_IDX_STEW2      				15		//收汁2
#define MENU_IDX_STEW3      				16		//收汁3
#define MENU_IDX_STEW4      				17		//收汁4
#define MENU_IDX_STOPHEAT      			StopHeatIdx		//停止加热10分钟
#define MENU_IDX_FACTORY    				FactoryIdx




void Menu_Init(INT8U idx, INT32U time);

void Menu_CookService(void);

//MENU_ATTR *Menu_GetMenuAttr(INT8U idx);


enum
{
	BUZZ_SINGLE=1,
	BUZZ_LONG,
	BUZZ_OVER,
	BUZZ_NEXT,
};	
typedef struct        //烹饪菜单步骤设置
{
	INT32U 	MenuTime;		//用户定义时间
	INT16U	SideTempe;
	INT16U	BottomTempe;
}CookMenuInputDef;

typedef struct        //烹饪菜单步骤设置
{
	INT8U	BuzzType;	//蜂鸣器类型
}CookMenuOutputDef;

CookMenuInputDef*	GetCookMenuInputAddress(void);		//通过指针传递输入参量(函数调用前执行)
CookMenuOutputDef*	GetCookMenuOutputAddress(void);		//通过指针传递输出参量(函数调用后执行)


#endif

//**********************************end of file********************************

