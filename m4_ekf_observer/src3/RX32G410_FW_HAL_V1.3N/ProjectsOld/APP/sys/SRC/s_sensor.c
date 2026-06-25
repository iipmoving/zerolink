/********************************************************************************
    FileName    :  s_sensor.c
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  各个传感器的检测与保护

    Date        :  2018-10-17
    Modify      :
                   2018-10-17 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/


/********************************************Head Files*/
#include	"string.h"
//#include "sys_mem.h"

#include "s_sensor.h"





code VOL_SENSOR_LIMIT VolLimit = 
{
    270 - 50,               /*高压停功率点*/
    260 - 50,               /*高压恢复点*/
    110 - 50,               /*电压低停功率点*/
    120 - 50,               /*低电压恢复点*/
    230 - 50,               /*高压低档升功率点，230Vad*/
    210 - 50,               /*低压线电流点，210Vad*/
};


//xdata NTC_SENSOR SensorIgbt;    //IGBT传感器
//xdata NTC_SENSOR SensorBot;     //底部传感器
//xdata VOL_SENSOR SensorVol;     //电压传感器
//xdata SENSOR_MAINBOARD SensorMainBoard;  //主板内部错误和无锅错误

NTC_SENSOR_STR		xdata 	Sensor_Temp;		//暂存传感器结构

#define	SensorT_Value		Sensor_Temp.Value
#define	SensorT_Level		Sensor_Temp.Level
#define	SensorT_LevelCnt	Sensor_Temp.LevelCnt
#define	SensorT_LevelAds	Sensor_Temp.Limit
#define	SensorT_TempCnt		Sensor_Temp.TempCnt
#define	SensorT_TempSum		Sensor_Temp.TempSum
#define	SensorT_TempDec		Sensor_Temp.Temp		//十进制温度值


/********************************************************************************
*name         : INT8U Sensor_GetVol(INT8U vol)
*author       : rsl
*function     : 获取电压数据
*para         : 无
*return       : 电压值，如果是0xff(255)则表明是>=255V
*brief        : 0xA0代表220V
                    220*ad           ad+ad+ad
            vol = ---------- = ad + --------
                     160               8
            系统电压可以瞬间变化，故不做数据滤波处理
********************************************************************************/

#if  DF_UNCALLED_CODE  == 0  //未使用函数 
INT8U Sensor_GetVol(INT8U vol)
{
    // #define VOL_220V_AD 0xA0
    INT16U sum;

    sum = ((INT16U)(vol)) * 151;
    sum = sum / 100;

    sum -= 50;  //减50，使电压范围能达到300V
//    sum -= 5;   //偏差了5V左右
//  if(Comm_GetMainSensor(COMM_SENSOR_POS_POWER) >= 400 / 25)
//  {
//      sum += 10;
//  }

    if(sum > 0xFF)
    {
        vol = 0xFF;
    }
    else
    {
        vol = sum;
    }

    return vol;
}
#endif





#define TEMP_VALUE_MAX_CNT 180
//二分查找温度值，表里面的数值已经转换成了10K上拉情况下读取的AD值
code INT8U   TempNtcAdTab[TEMP_VALUE_MAX_CNT] =                    // 温度值转换Ad值;Ad值越大,温度越低;降序;
{
    249,  249,   248,   248,   247,   247,   246,   246,   245,   245,  //0-9
    244,  244,   243,   242,   242,   241,   241,   240,   239,   238,  //10-19
    238,  237,   236,   235,   234,   233,   232,   231,   230,   229,  //20-29
    228,  227,   226,   225,   224,   222,   221,   220,   219,   217,  //30-39
    216,  215,   213,   212,   210,   209,   207,   206,   204,   202,  //40-49
    201,  199,   197,   196,   194,   192,   190,   188,   187,   185,  //50-59
    183,  181,   179,   177,   175,   173,   171,   169,   167,   165,  //60-69
    163,  161,   159,   157,   155,   153,   150,   148,   146,   144,  //70-79
    142,  140,   138,   136,   134,   132,   130,   128,   126,   124,  //80-89
    122,  120,   118,   116,   114,   112,   110,   108,   106,   104,  //99-99
    102,  101,   99,    97,    95,    93,    92,    90,    88,    87,   //100-109
    85,   83,    82,    80,    79,    77,    76,    74,    73,    71,   //110-119
    70,   69,    67,    66,    65,    63,    62,    61,    60,    58,   //120-129
    57,   56,    55,    54,    53,    52,    51,    50,    49,    48,   //130-139
    47,   46,    45,    44,    43,    42,    41,    40,    40,    39,   //140-149
    0xff-0xd9,0xff-0xd9,0xff-0xda,0xff-0xdb,0xff-0xdb,0xff-0xdc,0xff-0xdd,0xff-0xdd,0xff-0xde,0xff-0xdf,//150-159
    0xff-0xdf,0xff-0xe0,0xff-0xe1,0xff-0xe1,0xff-0xe2,0xff-0xe3,0xff-0xe3,0xff-0xe3,0xff-0xe4,0xff-0xe4,//160-169
    0xff-0xe5,0xff-0xe5,0xff-0xe6,0xff-0xe6,0xff-0xe7,0xff-0xe7,0xff-0xe8,0xff-0xe8,0xff-0xe9,0xff-0xe9,//170-179
};

/********************************************************************************
*name       : void Sensor_CalcTemp(INT8U ad)
*author     : rsl
*function   : 把AD转换成真实的温度值
*Para       : 无
*return     : 温度值，最后一位是小数位
*brief      : 二分查找，找出温度的整数值
********************************************************************************/
INT16U Sensor_CalcTemp(INT8U ad, INT8U qufan)
{
    INT16U st,ed,md;      //查找的头尾，中间
    INT8U i ;
    INT16U temp;

    if(qufan)
        ad = 0xff - ad;     //通讯获取的AD值要取反
    st = 0;
    ed = TEMP_VALUE_MAX_CNT-1 ;
    i = 0;

    if(ad >= TempNtcAdTab[st])
    {
        return st * 10;                     //ad超过表
    }
    else if(ad <= TempNtcAdTab[ed])
    {
        return ed * 10;                      //ad超过表
    }


    while(st < ed)
    {
        md = (st+ed)/2 ;

        if(ad == TempNtcAdTab[md])
        {
            break ;
        }
        if((ad < TempNtcAdTab[md]) && (ad > TempNtcAdTab[md+1]))
        {
            break ;
        }


        if(ad > TempNtcAdTab[md])
        {
            ed = md ;
        }
        else
        {
            st = md ;
        }

        if(i++ > TEMP_VALUE_MAX_CNT)
        {
            break ;
        }
    }

    if(st > ed )        //没找到
    {
        return 0 ;
    }

    //区分小数位，让每一个AD都能起作用
    if(ad == TempNtcAdTab[md])
    {
        temp = md*10;
    }
    else
    {
        temp = md*10+5;
    }

    return temp;
}


/********************************************************************************
*name         : void Sensor_OpenShortDet(NTC_SENSOR *sensor)
*author       : rsl
*function     : 传感器处理，得到传感器分区值，通过分区判断开短路，过热
*para         : sensor传感器机构体指针  , value传感器的值
*return       : 错误类型0
*brief        :
********************************************************************************/
void	SensorMemPush(void *source)
{

   memcpy(&Sensor_Temp,source,sizeof(NTC_SENSOR_STR));

}

void	SensorMemPop(void *dest)
{

   memcpy(dest,&Sensor_Temp,sizeof(NTC_SENSOR_STR));

}

/********************************************************************************
*name         : INT16U Sensor_GetTemp(INT8U pos, INT16U last)
*author       : rsl
*function     : 获取传感器温度值
*para         : pos 0-底部，1顶部  last 上一次的温度值
*return       : 无
*brief        : 数据做极值滤波和平滑滤波
               极值滤波，去掉极大值和极小值
               平滑滤波，去掉变化比较大的值
               去掉变化很大的值
********************************************************************************/
void Sensor_GetTemp(INT16U t_SensorInput)
{
#define SENSOR_TEMP_FILTER_CNT  4       //平滑滤波次数 8次即使温度值1555，也不会溢出
    INT16U temp;
        temp = t_SensorInput;  //计算温度


        if((temp == 0)||(temp > TEMP_VALUE_MAX))        //去掉极大值和极小值
        {
            temp = 0;//SensorT_TempDec;
        }

    if(SensorT_TempSum==0)   //赋一个初值
    {
      SensorT_TempSum=temp*SENSOR_TEMP_FILTER_CNT;
		SensorT_TempDec=temp;
		
    }

	SensorT_TempSum-=SensorT_TempSum/SENSOR_TEMP_FILTER_CNT;
	SensorT_TempSum+= temp;      //一阶滤波，减去上一次平均值，加上当前平均值

    SensorT_TempDec=SensorT_TempSum/SENSOR_TEMP_FILTER_CNT;

}





 

//========================================================================
// 函数:  
// 描述: 	
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2022年6月14日 10:59:36
// 备注: 
//========================================================================
INT8U	 Sensor_LevelDet(void)
{
    INT8U	i;
		INT8U  *p;
 	INT16U  IN_DATA;


		p=(INT8U*)SensorT_LevelAds;
//			{
//			指向这 4个 点
//				INT8U Open;            /*炉面传感器开路AD，小于这个AD认为开路*/ //低电压AD
//				INT8U Recover;        /*恢复加热的温度AD*/		//低电压恢复AD
//				INT8U Over;            /*超温保护温度AD*/	 //高电压恢复AD 
//				INT8U Short;           /*短路的温度AD*/		//高电压AD
//			
//			}
	
//			code NTC_SENSOR_LEVEL NtcLimtVol =        //电压
//		{
//    112,               /*低压*/
//    119,               /*低压恢复*/
//    173,               /*高压恢复*/
//    180,               /*高压*/
//		};



//enum
//{														//温度传感器区间定义
//		SENSOR_TEMP_NULL=0,			//初始化状态	
//		SENSOR_TEMP_OPEN,       //开路
//		SENSOR_TEMP_NORMAL,			//正常
//		SENSOR_TEMP_RECOVER,	  //过热恢复
//		SENSOR_TEMP_OVER,			 //过热
//		SENSOR_TEMP_SHORT,		 //短路
//		SENSOR_TEMP_LEVELMAX=SENSOR_TEMP_SHORT,
//};

//enum
//{
//		SENSOR_VC_NULL= 0,        //电压区间定义
//		SENSOR_VC_LOW,
//		SENSOR_VC_LOWRECOVER,
//		SENSOR_VC_NORMAL,
//		SENSOR_VC_HIGHRECOVER,
//		SENSOR_VC_HIGH,
//		SENSOR_VC_LEVELMAX=SENSOR_VC_HIGH,					//LEVE最大数 5个区间
//};

  
		for(i=0;i<SENSOR_TEMP_LEVELMAX-1;i++)   //小于4 
		{
	
			
				if(SensorT_Value<*p)  //当前输入 AD值
				{
					break;
				}
				p++;
		}
		
		//当前输入值
		
		i++;	//档位0为初始化值，实际档位往上加一位
		
		if(SensorT_Level==i)  //状态等级相等 维持不变
		{
			SensorT_LevelCnt=0;
		}
		else
		{
			SensorT_LevelCnt++;
		}

		if(SensorT_LevelCnt>50)   //更新新的 状态等级
		{
				SensorT_Level=i;
		}
		
	 
		IN_DATA=SensorT_TempDec=Sensor_CalcTemp(SensorT_Value,1);  //把AD转换成真实的温度值 ,（输入当前AD，1== AD取反 ==0 不变
		
		Sensor_GetTemp(IN_DATA);                                  //获取传感器温度值
		
		return	SensorT_Level;		
}






//**********************************end of file********************************
