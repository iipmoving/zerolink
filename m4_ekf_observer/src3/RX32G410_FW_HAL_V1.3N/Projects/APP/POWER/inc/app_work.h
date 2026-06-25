/********************************************************************************
    FileName    :  app_work.h
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  工作过程中的按键，显示，状态切换，运行等调用实现

    Date        :  2018-09-14
    Modify      :
                   2018-09-14 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
#ifndef APP_WORK_H
#define APP_WORK_H

#include "data_type.h"
#include "app_cook.h"
//=======================================工作状态处理
#define POWER_TO_IDLE_TIME				50			//上电1秒后转到待机状态
#define WAIT_TO_RUN_TIME	   			3//30//30			//3秒后转到运行状态
#define SHUTTLE_WAIT_TO_RUN_TIME		50			//3秒后从班车选择状态转到运行状态
#define IDLE_TO_OFF_TIME				600			//待机无操作待机转关机时间1分钟
#define FACTORY_ENTERM_TIME	 			3			//上电15S后不能进入工厂模式
#define SET_RESERVE_TO_RESERVE_TIME		30//30//2//50 //50

#define LEVEL_FLASH_TIME 30
#define SET_RESERVE_FLASH_TIME 1//10


#define DSP_SET_TIMER_TIME 100
#define DSP_SET_TIME 500

#define RESERVE_TIME_MAX   (10*60*60UL)     //最大预约时间
#define RESERVE_TIME_MIN   (60*60UL)        //最小预约时间
#define RESERVE_TIME_DEFAULT (60*60UL)    //预约默认时间
#define RESERVE_TIME_STEP  (60*60UL)        //预约调整步长
#define TIME_ERR_REPORT    (11)             //报错时间
#define TIME_ERR_STOP  (60)           //保温模式错误自动关机时间3分钟

#define INTO_SET_RESERVE_TIME (10)


#define KEY_GROUP_DRYPOT	0x01					//干锅
#define KEY_COOKNODDLE		0x02					//煮面条
#define KEY_DRYPOT			0x04					//发海参
#define KEY_DRYFISH			0x08					//干烧鱼
#define KEY_BIGPORRIDGE		0x200					//大米粥
#define KEY_SOUP			0x400					//煲靓汤
#define KEY_PHEASANT		0x800					//黄焖鸡
#define KEY_STEPORK			0x100					//炖排骨
#define KEY_CLAMMEAT		0x10					//焖酥肉
#define KEY_BRAISEPORK		0x1000					//红烧肉

#define KEY_REDUCE			0x2000					//减
#define KEY_PLUS			0x20					//加
#define KEY_1_MIN			0x4000					//1分钟
#define KEY_10_MIN			0x40					//10分钟
#define KEY_1_HOUR			0x80					//1小时

#define KEY_POWER			0x8000					//电源键

#define KEY_CANCEL			KEY_1_HOUR|KEY_1_MIN	//取消







enum            //cm设备工作状态
{
    WORK_STA_OFF=0 ,				//0.关机状态
    WORK_STA_POWER,					//1.上电状态
    WORK_STA_IDLE,					//2.待机状态
	WORK_STA_SET_RESERVE,			//3.设置预约时间
	WORK_STA_SET_TIMER,				//4.设置定时器
    WORK_STA_RESERVE,				//5.预约状态
	WORK_STA_WAIT,					//6.等待确认状态
	WORK_STA_WAIT_SHUTTLE,			//7.等待班车确认状态
    WORK_STA_RUN,					//8.工作状态
};
enum
{
    WORK_DSP_MENU_TIME = 0,
    WORK_DSP_RESERVE_TIME,
};


typedef struct
{
	
	uint8_t isDoubleClick:1;
	uint8_t isSetReserveTime:1;
	uint8_t isFirstTurn:1;
	uint8_t isShuttle:1;
	uint8_t isSetUsrTimer:1;	
	uint8_t isTFT_Lock:1;
	uint8_t isTFT_Lock_KEY:1;//通过按键 进入的 自锁

	
}WorkFlagDef;	


//union 
//{
//	WorkFlagDef	flag;
//	uint8_t 	byte;

//}CookWorkFlagDef;	


typedef struct cooking_machine
{
    //工作状态变量
    INT8U WorkSta;      	//工作状态标志
    INT16U SwitchTime;    	//切换时间，待机无操作30s进入关机状态 上电3s转到待机状态
    INT8U DoubleClick;  	//双击，800ms内，按第二次算双击
    INT16U DoubleClick2;
	
	WorkFlagDef	WorkFlag;
	

	INT8U ReserveTime_h;
	INT8U ReserveTime_10m;
	INT8U ReserveTime_m;
    INT32U ReserveTime;    //预约时间，S计数，最大计数12小时
    INT32U LastReserveTime;
    INT32U MenuTime;    //菜单时间，应用于可调时间的菜单
    INT8U MenuIdx;      //菜单序号
    INT8U FactoryTime;  //进入工厂模式时间，开机20s后不能再进入
    INT8U ErrReportTime;    //报错持续时间，如果时间超了，只显示，不响

    INT8U LastIdx;
	
	INT32U TimerTime;
	INT8U TimerTime_h;
	INT8U TimerTime_10m;
	INT8U TimerTime_m;

    INT16U DspSetTimerTime;         //显示"设置倒计时"倒计时
    INT16U DspSetTime;              //显示"设置"倒计时
    INT8U DspSetType;


    INT32U DspUsrMenuTime;          //用户设定的倒计时
    INT32U UsrMenuTime;             //用户设定的倒计时

    INT16U SetReserveCnt;
	

	INT16U ShuttleAralmFlag;
	INT16U ShuttleAralmFlag_Copy;
	


    MENU_ATTR *Attr;
} COOK_MAC_STR;




void Cook_ChangeSta(INT8U sta);
void Cook_SetMenu(INT8U idx);
INT8U Cook_GetSta(void);
INT8U Cook_GetMenu(void);
INT32U Cook_GetReserveTime(void);
void Cook_WorkStaService(void);
void Cook_InitMenu(void);
void Cook_KeyService(void);

void Cook_DspService(void);

void	AppWorkInit(void);

#endif

//**********************************end of file********************************
















