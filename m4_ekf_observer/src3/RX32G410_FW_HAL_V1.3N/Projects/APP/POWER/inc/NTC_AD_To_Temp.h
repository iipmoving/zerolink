#ifndef __NTC_AD_To_Temp_H__
#define __NTC_AD_To_Temp_H__


enum AD_TO_Temp_Type
{ 
	 DF_SET_10K_1K5=0,
	 DF_SET_100K_10K,       //选择 转换 表
 	 DF_SET10_100K_10K,     //10位AD 选择 转换 表
 
};


//INT16U Sensor_CalcTemp(INT8U ad, INT8U qufan);

WORD AD_Transition_Temp(BYTE Check_Temp,WORD AdDat);



#endif

