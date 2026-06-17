/********************************************************************************
    FileName    :  s_sensor.h
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  各个传感器的检测与保护

    Date        :  2018-10-17
    Modify      :
                   2018-10-17 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
#ifndef   S_SENSOR_H
#define   S_SENSOR_H



/********************************************Head Files*/
#include "data_type.h"



typedef struct ntc_sensor_limit_         //ntc传感器极限参数
{
    INT8U OpenValue;                /*炉面传感器开路AD，小于这个AD认为开路*/
    INT8U ShortValue;               /*炉面传感器短路AD，大于这个AD认为短路*/

    INT8U ProtectValue;            /*超温保护温度点温度*/
    INT8U RecoveryValue;           /*恢复加热的温度*/
} NTC_SENSOR_LIMIT;

typedef struct ntc_sensor_      //NTC热敏电阻传感器结构体
{
    
	INT8U ShortCnt;             //短路确认次数
    INT8U OpenCnt;              //开路确认次数
    INT8U OverCnt;              //超温确认次数
	INT8U IsOver;
    INT8U NormalCnt;            //正常确认次数
	NTC_SENSOR_LIMIT code *Limit;
    INT8U Temp;                //实际温度 度

} NTC_SENSOR;




enum
{														//温度传感器区间定义
		SENSOR_TEMP_NULL=0,			//初始化状态	
		SENSOR_TEMP_OPEN,       //开路
		SENSOR_TEMP_NORMAL,			//正常
		SENSOR_TEMP_RECOVER,	  //过热恢复
		SENSOR_TEMP_OVER,			 //过热
		SENSOR_TEMP_SHORT,		 //短路
		SENSOR_TEMP_LEVELMAX=SENSOR_TEMP_SHORT,
};

enum
{
		SENSOR_VC_NULL= 0,        //电压区间定义
		SENSOR_VC_LOW,
		SENSOR_VC_LOWRECOVER,
		SENSOR_VC_NORMAL,
		SENSOR_VC_HIGHRECOVER,
		SENSOR_VC_HIGH,
		SENSOR_VC_LEVELMAX=SENSOR_VC_HIGH,					//LEVE最大数 5个区间
};



typedef struct ntc_sensor_level_         //ntc传感器极限参数
{
    INT8U Open;            /*炉面传感器开路AD，小于这个AD认为开路*/ //低电压AD
    INT8U Recover;        /*恢复加热的温度AD*/		//低电压恢复AD
    INT8U Over;            /*超温保护温度AD*/	 //高电压恢复AD 
    INT8U Short;           /*短路的温度AD*/		//高电压AD
} NTC_SENSOR_LEVEL;





typedef struct ntc_sensor_str_      //NTC热敏电阻传感器结构体
{
		INT8U 	Value;								//当前输入值
		INT8U 	Level;             		//温度区间值
		INT8U 	LevelCnt;             //当前区间确认次数
		INT8U	TempCnt;							//滤波计数值
		INT16U 	TempSum;							//滤波累加和
		INT16U 	Temp;								//滤波后温度值	
		NTC_SENSOR_LEVEL code *Limit;
		
}NTC_SENSOR_STR;



typedef struct vol_sensor_limit_     //电压传感器保护结构体
{
    INT8U HighVol;              //高压停功率点
    INT8U HighNorVol;           //高压恢复点
    INT8U LowVol;               //低压停功率点
    INT8U LowNorVol;            //电压恢复点
    INT8U HightIncVol;          //高压低档升功率点，小功率的情况下需要改成大功率间歇
    INT8U LowDecVol;            //低压限电流点，降功率

} VOL_SENSOR_LIMIT;


typedef struct vol_sensor_
{
  INT8U LimitCnt;              //超过极限电压确认次数
	INT8U LimitCnt2;
  INT8U NormalCnt;             //电压恢复正常确认次数
	INT8U NormalCnt2;
	INT8U Vol;									 //电压值
} VOL_SENSOR;


typedef struct main_board_err
{
    INT8U InnerErrCnt;      //内部错误确认次数
    INT8U NoPanCnt;         //无锅错误确认次数
} SENSOR_MAINBOARD;


typedef struct	sensor_str
{
 INT8U				FurnaceIdx;				 //索引值	
 NTC_SENSOR_STR 	SensorIgbt;    //IGBT传感器
 NTC_SENSOR_STR 	SensorBot;     //底部传感器
 NTC_SENSOR_STR 	SensorVol;     //电压传感器
 SENSOR_MAINBOARD 	SensorMainBoard;  //主板内部错误和无锅错误
 INT8U				VolErrTab;
 INT8U  			BOT_AD_Temp;
 INT16U				ErrTbl;				//错误代码
INT16U 				ERR_Delay_Timer;   //故障 延迟 报错   
}SENSOR_STR;




#define SENSOR_DET_CNT      35   //23   //传感器检测确认次数
#define SENSOR_DET_CNT1     50      //传感器检测确认次数
#define SENSOR_DET_CNT2     50     //传感器检测确认次数
#define SENSOR_NOPAN_DET_CNT 1
#define TEMP_VALUE_MAX      2000     //温度最高值

#define SENSOR_RES_NORMAL 0x00  //回复传感器正常
#define SENSOR_RES_SHORT 0xfe   //回复传感器短路
#define SENSOR_RES_OPEN  0xfd   //回复传感器开路
#define SENSOR_RES_OVER  0xFc   //回复传感器高超限
#define SENSOR_RES_LOW   0xFb   //回复传感器低超限

enum
{
    SENSOR_TEMP_IGBT = 0,        //各个温度传感器的序号
    SENSOR_TEMP_BOT,
    SENSOR_TEMP_TOP,
		SENSOR_TEMP_SIDE,
		SENSOR_TEMP_BOT_MAX,
    SENSOR_TEMP_MAX = SENSOR_TEMP_BOT_MAX,
};

//extern xdata NTC_SENSOR SensorIgbt[4];    //IGBT传感器
//extern xdata NTC_SENSOR SensorBot[4];     //底部传感器
//extern xdata VOL_SENSOR SensorVol[4];     //电压传感器
//extern xdata SENSOR_MAINBOARD SensorMainBoard[4];

//extern xdata NTC_SENSOR SensorIgbt;    //IGBT传感器
//extern xdata NTC_SENSOR SensorBot;     //底部传感器
//extern xdata VOL_SENSOR SensorVol;     //电压传感器
//extern xdata SENSOR_MAINBOARD SensorMainBoard;



INT16U Sensor_CalcTemp(INT8U ad, INT8U qufan);
void Sensor_GetTemp(INT16U t_SensorInput);
void Sensor_ParaInit();
//INT8U Sensor_OpenShortDet(NTC_SENSOR *sensor, INT8U value);
INT8U	 Sensor_LevelDet(void);

INT8U Sensor_VolProtect();
INT8U Sensor_GetVol(INT8U vol);
void Sensor_ErrMainBoard();
INT8U  Sensor_TempProtect(NTC_SENSOR *sensor);
void TotalVolCheck(void);
void	VolErrTabClear(void);
void Clear_SensorErr();

void	SensorMemPush(void *source);
void	SensorMemPop(void *dest);


#endif
//**********************************end of file********************************
