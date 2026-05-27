//========================================================================
// 函数: 
// 描述: 实现modbus 库配置与初始化 ，以及数据 解析 与回传 同步
// 参数: 	
// 返回: none.
// 版本: VER1.0
// 日期: 2025年12月2日
// 备注: 
//========================================================================
//#include  "func_def.h"
//#include "sys_mem.h"
#include 	<string.h>
#include "Modbus_Analysis_Lib.h"
#include "Modbus_Lib_Init_An_Analysis.h"
#include	"API_UART.h"
#include "../../../../../app/ekf/modbus_ekf_regs.h"

//#include "rx32g4xx_config_def.h"
//#include "rx32g4xx_hal.h"

//#include "rx32g4xx_hal_uart.h"
//#include "rx32g4xx_hal_def.h"
//#include "Uart_TX_RX_Dispose.h"

//------------ 反馈数据 || 数据更改 引用到的--- ↓↓↓↓
//#include "AD_To_Actual_value.h" 
//Get_AD_TO_Temp(DF_Get_BOT_Temp1);
//#include "s_comm.h"
//#include "Key_dispose.h"
//#include "Err_Check.h"
//------------ 反馈数据 || 数据更改 引用到的--- ↑↑↑↑
#define		DF_Stove_Quantity			4
#define 	DF_Versions  				01	// 没有显示板 该版本号已 放置在通讯
#define		DF_MB_Uart_Rx_LONG			50  //接收 缓存


void Get_IHPower_Main_Init_DATA(void); // 备注: 上电初始化时 执行一次

unsigned char MB_Uart_Rx_Data[DF_MB_Uart_Rx_LONG]={0}; //
unsigned char MB_Uart_Rx_Long; //


__attribute__((weak))	void API_UART_RxControlCallback(uint8_t ch,int8_t *buff, uint8_t len)	
{
}
__attribute__((weak))	void API_UART_RxInitCallback(uint8_t ch,int8_t *buff, uint8_t len)	
{
}
__attribute__((weak))	uint8_t* API_UART_TxStatusCallback(uint8_t ch, uint8_t len)	
{
	return 0;
}

__attribute__((weak))	uint8_t* API_UART_TxInitCallback(uint8_t chn, uint8_t len)
{
	return 0;
}

//typedef struct
//{ 	 
//	unsigned short  Heat_Power;  		//主机 请求 加热的 功率 
// 

//}Modbus_Main_Cof_STA; 		// Modbus_控制 状态


//Modbus_Main_Cof_STA			Modbus_Main_STA[DF_Stove_Quantity];


typedef enum {
	
	DF_Modbus_Slave_01=0,	
	DF_Modbus_Slave_02,
	DF_Modbus_Slave_03,
	DF_Modbus_Slave_04,
	
} _Modbus_Slave_Number; 				// Modbus 从机 号

//#include "Uart_Dispose.h" 

//#include "Uart_Dbug_Dispose.h" 
//#pragma pack(1)

//#pragma pack(push, 1)  // push current alignment, set to 1 byte boundary
typedef struct
{
    // --- I2C 读状态顺序 (0x10-0x1F 映射到 0x1000-0x100F) ---
    unsigned short  SYS_Sta;                //0x1000 IHStatus: 系统状态 (BIT3-0=炉头号)
    unsigned short  Vol_AD;                 //0x1001 VoltageValue: 电压AD
    unsigned short  Current_AD;             //0x1002 CurrentValue: 电流AD
    unsigned short  IGBT_AD;                //0x1003 Sensor1Value: IGBT温度AD
    unsigned short  Bot_AD;                 //0x1004 Sensor2Value: 炉面温度AD(TOP)
    unsigned short  Top_AD;                 //0x1005 Sensor3Value: 顶部温度AD(预留)
    unsigned short  Practical_Power;        //0x1006 ActualPower: 实际功率
    unsigned short  target_Power;           //0x1007 TargetPower: 目标功率(回读)
    unsigned short  Practical_PPG;          //0x1008 ActualPPG: 实际加热PPG
    unsigned short  P_limited_STA;          //0x1009 PowerStatus: 功率限制状态
    unsigned short  Pan_pulsating;          //0x100A LoadValue: 检锅脉冲(低4位)+浪涌(高4位)
    unsigned short  HVol_Cnt;               //0x100B VCNTValue: 反压计数器
    unsigned short  PWMValue_L;             //0x100C PWMValue_L: 频率限制值低位(预留)
    unsigned short  PWMValue_H;             //0x100D PWMValue_H: 频率限制值高位(预留)
    unsigned short  PowerAdjust;            //0x100E PowerAdjust: PPG修正值(预留)
    unsigned short  Version_Number;         //0x100F Version: 版本号
    // --- MODBUS 扩展寄存器 ---
    unsigned short  Fan_AD;                 //0x1010 风扇AD
    unsigned short  ERROR;                  //0x1011 故障码
    unsigned short  interior_ERR;           //0x1012 内部故障
    unsigned short  HZ_Cnt;                 //0x1013 频率计数器
    unsigned short  Discard_Cnt;            //0x1014 丢波计数器

}IH_STA_READ; 		//IH 状态 只读
 
typedef struct
{ 

	unsigned short  Check_Pan_LV;			//0x2000	检锅强度设定
	unsigned short  PPG_Max;				//0x2001	最大PPG限制
	unsigned short  Pan_Power;				//0x2002	移锅功率
	unsigned short  HVol_Limited;			//0x2003	反压限制
	unsigned short  Load_Current;			//0x2004	负载有效电流
	unsigned short  Current_calibration;	//0x2005	电流修正系数
	unsigned short  Power_MIX;				//0x2006	最小连续功率
	unsigned short  Power_MAX;				//0x2007	最大连续功率
												
	unsigned short  wrong_Pan;				//0x2008	恶略锅具保护功率
	unsigned short  syntony_Current;		//0x2009	谐振电流保护值
	unsigned short  phase_Pan;				//0x200A	移锅相位
	unsigned short  phase_Mix;				//0x200B	最小相位
	unsigned short  steel_calibration;		//0x200C	钢铁锅修正
	unsigned short  N_Pan_syntony_C;		//0x200D	移锅谐振电流限制
												
	unsigned short  Work_STA;				//0x200E	工作状态
	unsigned short  FAN_Speed;				//0x200F	风扇转速
	unsigned short  target_Power;			//0x2010	目标功率
	unsigned short  intermittent_Heat;		//0x2011	间断加热
	unsigned short  jitter_frequency;		//0x2012	抖频参数
	unsigned short  BuzzCof;				//0x2013	蜂鸣器控制
	unsigned short  syntony_Current_Short;	//0x2014	短路保护 谐振电流 ovp Short

	 
}IH_STA_READ_WRITE; 		//IH 状态 可读可写

typedef struct
{ 
	unsigned short  Power_Calibration;  // 功率校准值 
	unsigned short  Slave_Addr;			//从机地址设置
	unsigned short  Baud_rate_SET; 		//波特率设置 
	unsigned short  Save_order;			//保存命令

}IH_STA_READ_WRITE_SYS_SET; 			//IH 状态 可读可写

//#pragma pack(pop)  // restore original alignment



//========================================================================
// 函数: 
// 描述:  防止 设置漏 ，少设置等 低级错误，一定要使用 宏设置.！！！！
// 参数: 	
// 返回: none.
// 版本: VER1.0
// 日期:  
// 备注:  	
#define 	DF_Modbus_US_Num		4	//	modbus 从机总数量

#define 	DF_Modbus_ARM1_Num		4	//	modbus 从机1	内存片 数量 (+EKF)
#define 	DF_Modbus_S1_TX_Len		DF_MB_Uart_Rx_LONG	//	modbus 从机1	发送缓冲 长度

#define 	DF_Modbus_ARM2_Num		4	//	modbus 从机2	内存片 数量 (+EKF)
#define 	DF_Modbus_S2_TX_Len		DF_MB_Uart_Rx_LONG	//	modbus 从机1	发送缓冲 长度

#define 	DF_Modbus_ARM3_Num		4	//	modbus 从机1	内存片 数量 (+EKF)
#define 	DF_Modbus_S3_TX_Len		DF_MB_Uart_Rx_LONG	//	modbus 从机1	发送缓冲 长度

#define 	DF_Modbus_ARM4_Num		4	//	modbus 从机2	内存片 数量 (+EKF)
#define 	DF_Modbus_S4_TX_Len		DF_MB_Uart_Rx_LONG	//	modbus 从机1	发送缓冲 长度


//========================================================================

// Modbus 从机总配置数组
 PDUData_TypeDef Modbus_Cofg[DF_Modbus_US_Num]; // modbus 从机总数量

// --- 静态变量定义与初始化 ---
unsigned char Modbus_TX_BUFF[DF_Modbus_S1_TX_Len];

// --- 从机 1 (索引 0) 的配置 ---
// 内存区域配置数组
 Register_Area_t Modbus_ARM1_Addr_SET[DF_Modbus_ARM1_Num]; // modbus 从机1 内容片 数量
// 实际数据结构实例 (只读)
IH_STA_READ 					Modbus_ARM1_Addr_0x1000;

// 实际数据结构实例 (可读可写)
IH_STA_READ_WRITE 				Modbus_ARM1_Addr_0x2000;
// 实际数据结构实例(存储  主机写入的数据)
IH_STA_READ_WRITE 				Modbus_ARM1_Addr_0x2000_EEP;


// 实际数据结构实例 (可读可写)
IH_STA_READ_WRITE_SYS_SET		Modbus_ARM1_Addr_0x3000;
// 实际数据结构实例 (存储  主机写入的数据)
IH_STA_READ_WRITE_SYS_SET		Modbus_ARM1_Addr_0x3000_EEP;





// --- 从机 2 (索引 1) 的配置 ---
unsigned char Modbus_TX_BUFF2[DF_Modbus_S2_TX_Len];
// 内存区域配置数组
 Register_Area_t 				Modbus_ARM2_Addr_SET[DF_Modbus_ARM2_Num]; // modbus 从机2 内容片 数量
// 实际数据结构实例 (只读)
IH_STA_READ 					Modbus_ARM2_Addr_0x1000;


// 实际数据结构实例 (可读可写)
IH_STA_READ_WRITE 				Modbus_ARM2_Addr_0x2000;
// 实际数据结构实例 (存储  主机写入的数据)
IH_STA_READ_WRITE 				Modbus_ARM2_Addr_0x2000_EEP;


// 实际数据结构实例 (可读可写，系统设置)
IH_STA_READ_WRITE_SYS_SET 		Modbus_ARM2_Addr_0x3000;
// 实际数据结构实例(存储  主机写入的数据)
IH_STA_READ_WRITE_SYS_SET		Modbus_ARM2_Addr_0x3000_EEP;





// --- 从机 3 (索引 2) 的配置 ---
unsigned char Modbus_TX_BUFF3[DF_Modbus_S3_TX_Len];
// 内存区域配置数组
 Register_Area_t 				Modbus_ARM3_Addr_SET[DF_Modbus_ARM3_Num]; // modbus 从机2 内容片 数量
// 实际数据结构实例 (只读)
IH_STA_READ 					Modbus_ARM3_Addr_0x1000;


// 实际数据结构实例 (可读可写)
IH_STA_READ_WRITE 				Modbus_ARM3_Addr_0x2000;
// 实际数据结构实例 (存储  主机写入的数据)
IH_STA_READ_WRITE 				Modbus_ARM3_Addr_0x2000_EEP;


// 实际数据结构实例 (可读可写，系统设置)
IH_STA_READ_WRITE_SYS_SET 		Modbus_ARM3_Addr_0x3000;
// 实际数据结构实例(存储  主机写入的数据)
IH_STA_READ_WRITE_SYS_SET		Modbus_ARM3_Addr_0x3000_EEP;




// --- 从机 4 (索引 3) 的配置 ---
unsigned char Modbus_TX_BUFF4[DF_Modbus_S4_TX_Len];
// 内存区域配置数组
 Register_Area_t 				Modbus_ARM4_Addr_SET[DF_Modbus_ARM4_Num]; // modbus 从机2 内容片 数量
// 实际数据结构实例 (只读)
IH_STA_READ 					Modbus_ARM4_Addr_0x1000;


// 实际数据结构实例 (可读可写)
IH_STA_READ_WRITE 				Modbus_ARM4_Addr_0x2000;
// 实际数据结构实例 (存储  主机写入的数据)
IH_STA_READ_WRITE 				Modbus_ARM4_Addr_0x2000_EEP;


// 实际数据结构实例 (可读可写，系统设置)
IH_STA_READ_WRITE_SYS_SET 		Modbus_ARM4_Addr_0x3000;
// 实际数据结构实例(存储  主机写入的数据)
IH_STA_READ_WRITE_SYS_SET		Modbus_ARM4_Addr_0x3000_EEP;



//========================================================================
// 函数: 
// 描述: 从机1、数据写入 检查
// 参数: 	
// 返回: none.
// 版本: VER1.0
// 日期: 2025年12月2日
// 备注: 用户只需要完成 写入数据的 合法性 检查 并反馈即可
//		 当数据 不合法时 存储结构体 “Modbus_ARM2_Addr_0x3000_EEP” 是不会更新 最新值的
//========================================================================
unsigned char S1_Check_Write_Data_0x2000(void)
{
//	if(Modbus_ARM1_Addr_0x2000.target_Power>2800 || (Modbus_ARM1_Addr_0x2000.target_Power>=1 && Modbus_ARM1_Addr_0x2000.target_Power<200 )  ) //目标功率限制
//	{
//		return 0;
//	}
/*
	if(Modbus_ARM1_Addr_0x2000.Work_STA &0x8000) //申请进入功率校准 模式
	{
		Modbus_ARM1_Addr_0x3000.Power_Calibration		=Comm_GetMainSensor(COMM_SENSOR_POS_ADJ,0); 	//主板校准信息(主板存储的校准值
		Modbus_ARM1_Addr_0x3000_EEP.Power_Calibration	=Comm_GetMainSensor(COMM_SENSOR_POS_ADJ,0); 	//主板校准信息(主板存储的校准值
		
	}
*/

	return 1;
}
unsigned char S1_Check_Write_Data_0x3000(void)
{
	if(Modbus_ARM1_Addr_0x3000.Power_Calibration < 36 || Modbus_ARM1_Addr_0x3000.Power_Calibration > 96   ) //目标功率限制
	{
		return 0;
	}
	return 1;

}





unsigned char S2_Check_Write_Data_0x2000(void)
{
//	if(Modbus_ARM2_Addr_0x2000.target_Power>2800 || (Modbus_ARM2_Addr_0x2000.target_Power>=1 && Modbus_ARM2_Addr_0x2000.target_Power<200 )  ) //目标功率限制
//	{
//		return 0;
//	}
	/*
	if(Modbus_ARM2_Addr_0x2000.Work_STA &0x8000) //申请进入功率校准 模式
	{
		Modbus_ARM2_Addr_0x3000.Power_Calibration		=Comm_GetMainSensor(COMM_SENSOR_POS_ADJ,1); 	//主板校准信息(主板存储的校准值
		Modbus_ARM2_Addr_0x3000_EEP.Power_Calibration	=Comm_GetMainSensor(COMM_SENSOR_POS_ADJ,1); 	//主板校准信息(主板存储的校准值
		
	}
	*/
	return 1;

}
unsigned char S2_Check_Write_Data_0x3000(void)
{
//	if(Modbus_ARM2_Addr_0x3000.Power_Calibration < 36 || Modbus_ARM2_Addr_0x3000.Power_Calibration > 96   ) //目标功率限制
//	{
//		return 0;
//	}
	
	return 1;

}










unsigned char S3_Check_Write_Data_0x2000(void)
{
//	if(Modbus_ARM3_Addr_0x2000.target_Power>2800 || (Modbus_ARM3_Addr_0x2000.target_Power>=1 && Modbus_ARM3_Addr_0x2000.target_Power<200 )  ) //目标功率限制
//	{
//		return 0;
//	}
	/*
	if(Modbus_ARM2_Addr_0x2000.Work_STA &0x8000) //申请进入功率校准 模式
	{
		Modbus_ARM2_Addr_0x3000.Power_Calibration		=Comm_GetMainSensor(COMM_SENSOR_POS_ADJ,1); 	//主板校准信息(主板存储的校准值
		Modbus_ARM2_Addr_0x3000_EEP.Power_Calibration	=Comm_GetMainSensor(COMM_SENSOR_POS_ADJ,1); 	//主板校准信息(主板存储的校准值
		
	}
	*/
	return 1;

}
unsigned char S3_Check_Write_Data_0x3000(void)
{
//	if(Modbus_ARM3_Addr_0x3000.Power_Calibration < 36 || Modbus_ARM3_Addr_0x3000.Power_Calibration > 96   ) //目标功率限制
//	{
//		return 0;
//	}
	
	return 1;

}







unsigned char S4_Check_Write_Data_0x2000(void)
{
//	if(Modbus_ARM4_Addr_0x2000.target_Power>2800 || (Modbus_ARM4_Addr_0x2000.target_Power>=1 && Modbus_ARM4_Addr_0x2000.target_Power<200 )  ) //目标功率限制
//	{
//		return 0;
//	}
	/*
	if(Modbus_ARM2_Addr_0x2000.Work_STA &0x8000) //申请进入功率校准 模式
	{
		Modbus_ARM2_Addr_0x3000.Power_Calibration		=Comm_GetMainSensor(COMM_SENSOR_POS_ADJ,1); 	//主板校准信息(主板存储的校准值
		Modbus_ARM2_Addr_0x3000_EEP.Power_Calibration	=Comm_GetMainSensor(COMM_SENSOR_POS_ADJ,1); 	//主板校准信息(主板存储的校准值
		
	}
	*/
	return 1;

}
unsigned char S4_Check_Write_Data_0x3000(void)
{
	if(Modbus_ARM4_Addr_0x3000.Power_Calibration < 36 || Modbus_ARM4_Addr_0x3000.Power_Calibration > 96   ) //目标功率限制
	{
		return 0;
	}
	
	return 1;

}








//========================================================================
// 函数: 
// 描述:  初始化可读寄存器，( 从eeprom 读取可读写寄存器的 值
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2025年12月2日
// 备注: 
//========================================================================
void init_SR_RW_DATA(void)
{
	static unsigned char Modbus_DATA_Init=0;
	if(Modbus_DATA_Init) return;
	Modbus_DATA_Init=0xAA;
//memcpy
	memset ( &Modbus_ARM1_Addr_0x1000, 		0x00, sizeof ( IH_STA_READ ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM1_Addr_0x2000, 		0x00, sizeof ( IH_STA_READ_WRITE ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM1_Addr_0x2000_EEP, 	0x00, sizeof ( IH_STA_READ_WRITE ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM1_Addr_0x3000, 		0x00, sizeof ( IH_STA_READ_WRITE_SYS_SET ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM1_Addr_0x3000_EEP, 	0x00, sizeof ( IH_STA_READ_WRITE_SYS_SET ) ); //结构体成员 初始化为 FF	  	

	
	memset ( &Modbus_ARM2_Addr_0x1000, 		0x00, sizeof ( IH_STA_READ ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM2_Addr_0x2000, 		0x00, sizeof ( IH_STA_READ_WRITE ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM2_Addr_0x2000_EEP, 	0x00, sizeof ( IH_STA_READ_WRITE ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM2_Addr_0x3000, 		0x00, sizeof ( IH_STA_READ_WRITE_SYS_SET ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM2_Addr_0x3000_EEP, 	0x00, sizeof ( IH_STA_READ_WRITE_SYS_SET ) ); //结构体成员 初始化为 FF	  	

	memset ( &Modbus_ARM3_Addr_0x1000, 		0x00, sizeof ( IH_STA_READ ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM3_Addr_0x2000, 		0x00, sizeof ( IH_STA_READ_WRITE ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM3_Addr_0x2000_EEP, 	0x00, sizeof ( IH_STA_READ_WRITE ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM3_Addr_0x3000, 		0x00, sizeof ( IH_STA_READ_WRITE_SYS_SET ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM3_Addr_0x3000_EEP, 	0x00, sizeof ( IH_STA_READ_WRITE_SYS_SET ) ); //结构体成员 初始化为 FF	  	



	memset ( &Modbus_ARM4_Addr_0x1000, 		0x00, sizeof ( IH_STA_READ ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM4_Addr_0x2000, 		0x00, sizeof ( IH_STA_READ_WRITE ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM4_Addr_0x2000_EEP, 	0x00, sizeof ( IH_STA_READ_WRITE ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM4_Addr_0x3000, 		0x00, sizeof ( IH_STA_READ_WRITE_SYS_SET ) ); //结构体成员 初始化为 FF	  	
	memset ( &Modbus_ARM4_Addr_0x3000_EEP, 	0x00, sizeof ( IH_STA_READ_WRITE_SYS_SET ) ); //结构体成员 初始化为 FF	  	



 Get_IHPower_Main_Init_DATA();// 备注: 上电初始化时 执行一次
	/*
	//只读
	Modbus_ARM1_Addr_0x1000.Practical_Power=0;//实际加热 功率
	Modbus_ARM1_Addr_0x1000.Bot_AD=25; //底部传感器AD
	Modbus_ARM1_Addr_0x1000.Version_Number=DF_Versions;//版本号
	Modbus_ARM1_Addr_0x1000.ERROR=0;			//0x100F 故障码
	Modbus_ARM1_Addr_0x1000.interior_ERR=0;		//0x1010 内部故障	
	
	//可读可写
	Modbus_ARM1_Addr_0x2000.target_Power=0;		//目标功率
	Modbus_ARM1_Addr_0x2000_EEP.target_Power=0;	//目标功率
	
	Modbus_ARM1_Addr_0x2000.Work_STA=0;	//工作状态
	Modbus_ARM1_Addr_0x2000_EEP.Work_STA=0;
	
	
	
	//只读
	Modbus_ARM2_Addr_0x1000.Practical_Power=0;//实际加热 功率
	Modbus_ARM2_Addr_0x1000.Bot_AD=30; //底部传感器AD
	Modbus_ARM2_Addr_0x1000.Version_Number=DF_Versions;//版本号
	Modbus_ARM2_Addr_0x1000.ERROR=0;			//0x100F 故障码
	Modbus_ARM2_Addr_0x1000.interior_ERR=0;		//0x1010 内部故障	
	//可读可写
	Modbus_ARM2_Addr_0x2000.target_Power=0;		//目标功率
	Modbus_ARM2_Addr_0x2000_EEP.target_Power=0;	//目标功率
	
	Modbus_ARM2_Addr_0x2000.Work_STA=0;	//工作状态
	Modbus_ARM2_Addr_0x2000_EEP.Work_STA=0;
	
	
	
	
	
	//只读
	Modbus_ARM3_Addr_0x1000.Practical_Power=0;//实际加热 功率
	Modbus_ARM3_Addr_0x1000.Bot_AD=35; //底部传感器AD
	Modbus_ARM3_Addr_0x1000.Version_Number=DF_Versions;//版本号
	Modbus_ARM3_Addr_0x1000.ERROR=0;			//0x100F 故障码
	Modbus_ARM3_Addr_0x1000.interior_ERR=0;		//0x1010 内部故障	
	//可读可写
	Modbus_ARM3_Addr_0x2000.target_Power=0;		//目标功率
	Modbus_ARM3_Addr_0x2000_EEP.target_Power=0;	//目标功率
	
	Modbus_ARM3_Addr_0x2000.Work_STA=0;	//工作状态
	Modbus_ARM3_Addr_0x2000_EEP.Work_STA=0;
	
	
	
	//只读
	Modbus_ARM4_Addr_0x1000.Practical_Power=0;//实际加热 功率
	Modbus_ARM4_Addr_0x1000.Bot_AD=40; //底部传感器AD
	Modbus_ARM4_Addr_0x1000.Version_Number=DF_Versions;//版本号
	Modbus_ARM4_Addr_0x1000.ERROR=0;			//0x100F 故障码
	Modbus_ARM4_Addr_0x1000.interior_ERR=0;		//0x1010 内部故障	
	//可读可写
	Modbus_ARM4_Addr_0x2000.target_Power=0;		//目标功率
	Modbus_ARM4_Addr_0x2000_EEP.target_Power=0;	//目标功率
	
	Modbus_ARM4_Addr_0x2000.Work_STA=0;	//工作状态
	Modbus_ARM4_Addr_0x2000_EEP.Work_STA=0;
	*/
	
}	

//========================================================================
// 函数: 
// 描述: 对Modbus 协议解析库 进行初始 配置
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2025年12月2日
// 备注: 
//========================================================================
void Modbus_Cofg_Init_SET(void)
{
	static unsigned char Modbus_Init=0;
	if(Modbus_Init) return;
	Modbus_Init=0xAA;
		API_UART_DMA_ReadValue(UARTX,MB_Uart_Rx_Data,DF_MB_Uart_Rx_LONG);	//对 MDA 进行一次内存 初始化
	
    // --- 初始化 从机 1 (索引 0) ---
    // 配置内存区域 1 (示例: 地址 0x1000 开始的只读区域)
    Modbus_ARM1_Addr_SET[0].Start_Address   = 0x1000;
    Modbus_ARM1_Addr_SET[0].End_Address     = 0x1000 + (sizeof(IH_STA_READ)/sizeof(unsigned short)); 
    Modbus_ARM1_Addr_SET[0].Data_ptr        = (void*)&Modbus_ARM1_Addr_0x1000;
    Modbus_ARM1_Addr_SET[0].Data_ptr_EEPROM = NULL; // 只读区域，EEPROM指针为空
	Modbus_ARM1_Addr_SET[0].Check_Write_Data = NULL; // 只读区域，允许写入检查为空
    Modbus_ARM1_Addr_SET[0].Data_Size       = sizeof(unsigned short); // 假设寄存器是 16 位
    Modbus_ARM1_Addr_SET[0].Data_Pyte       = 0; // 0 表示只读
	
    // 配置内存区域 2 (示例: 地址 0x2000 开始的可读可写区域)
    Modbus_ARM1_Addr_SET[1].Start_Address   = 0x2000;
    Modbus_ARM1_Addr_SET[1].End_Address     = 0x2000 + (sizeof(IH_STA_READ_WRITE)/sizeof(unsigned short)); 
    Modbus_ARM1_Addr_SET[1].Data_ptr        = (void*)&Modbus_ARM1_Addr_0x2000;
    Modbus_ARM1_Addr_SET[1].Data_ptr_EEPROM = (void*)&Modbus_ARM1_Addr_0x2000_EEP; // 存储写入数据
	Modbus_ARM1_Addr_SET[1].Check_Write_Data = S1_Check_Write_Data_0x2000; 		//配置 写入参数检查 函数
    Modbus_ARM1_Addr_SET[1].Data_Size       = sizeof(unsigned short); 			// 假设寄存器是 16 位
    Modbus_ARM1_Addr_SET[1].Data_Pyte       = 1; // 1 表示可读可写

	
    // 配置内存区域 3 (示例: 地址 0x3000 开始的可读可写系统设置区域)
    Modbus_ARM1_Addr_SET[2].Start_Address   = 0x3000;
    Modbus_ARM1_Addr_SET[2].End_Address     = 0x3000 + (sizeof(IH_STA_READ_WRITE_SYS_SET)/sizeof(unsigned short)); // 0x3000
    Modbus_ARM1_Addr_SET[2].Data_ptr        = (void*)&Modbus_ARM1_Addr_0x3000;
    Modbus_ARM1_Addr_SET[2].Data_ptr_EEPROM = (void*)&Modbus_ARM1_Addr_0x3000_EEP; 	// 指向EEPROM备份
	Modbus_ARM1_Addr_SET[2].Check_Write_Data = S1_Check_Write_Data_0x3000; 			//配置 写入参数检查 函数
    Modbus_ARM1_Addr_SET[2].Data_Size       = sizeof(unsigned short);
    Modbus_ARM1_Addr_SET[2].Data_Pyte       = 1; // 可读可写
	
	
    // 设置从机 1 的基本信息
    // EKF 遥测: 0x1020 只读
    Modbus_ARM1_Addr_SET[3].Start_Address   = EKF_REG_BASE;
    Modbus_ARM1_Addr_SET[3].End_Address     = EKF_REG_BASE + EKF_REG_COUNT;
    Modbus_ARM1_Addr_SET[3].Data_ptr        = EKF_Regs_GetDataPtr(0);
    Modbus_ARM1_Addr_SET[3].Data_ptr_EEPROM = NULL;
    Modbus_ARM1_Addr_SET[3].Check_Write_Data = NULL;
    Modbus_ARM1_Addr_SET[3].Data_Size       = sizeof(unsigned short);
    Modbus_ARM1_Addr_SET[3].Data_Pyte       = 0;


    Modbus_Cofg[0].Us_Cof_ARM_Num       = Modbus_ARM1_Addr_SET; // 关联内存区域配置
    Modbus_Cofg[0].ARM_Count            = DF_Modbus_ARM1_Num; 	// 每个协议里面 有多少组协议
    Modbus_Cofg[0].Slave_Hardware_Addr  = 0x05; 				// 从机硬件地址设置 (注意前面的 0x)
	Modbus_Cofg[0].Slave_Hardware_Addr_ERR_RET=0;				// 从机硬件地址设置 错误回复什么？ ==0 保持沉默 不响应	 ==1 反馈错误代码
    Modbus_Cofg[0].Tx_Buf				=Modbus_TX_BUFF;		// 指向 发送缓存
	Modbus_Cofg[0].Comm_Tx_Len_Max      = DF_Modbus_S1_TX_Len;  // 用户的设备 支持多长的 数据发送？ 
	Modbus_Cofg[0].CRC_Order			=0;						// CRC高低字节 对调 ==0 使用默认先发低字节 再发高字节，==1 对调







    // --- 初始化 从机 2 (索引 1) ---
    // 配置内存区域 1 (示例: 地址 0x1000 开始的只读区域)
    Modbus_ARM2_Addr_SET[0].Start_Address   = 0x1000;
    Modbus_ARM2_Addr_SET[0].End_Address     = 0x1000 + (sizeof(IH_STA_READ)/sizeof(unsigned short)); // 0x1003
    Modbus_ARM2_Addr_SET[0].Data_ptr        = (void*)&Modbus_ARM2_Addr_0x1000;
    Modbus_ARM2_Addr_SET[0].Data_ptr_EEPROM = NULL; // 只读区域，EEPROM指针为空
	Modbus_ARM2_Addr_SET[0].Check_Write_Data = NULL; // 只读区域，允许写入检查为空
    Modbus_ARM2_Addr_SET[0].Data_Size       = sizeof(unsigned short);
    Modbus_ARM2_Addr_SET[0].Data_Pyte       = 0; // 只读

    // 配置内存区域 2 (示例: 地址 0x2000 开始的可读可写区域)
    Modbus_ARM2_Addr_SET[1].Start_Address   = 0x2000;
    Modbus_ARM2_Addr_SET[1].End_Address     = 0x2000 + (sizeof(IH_STA_READ_WRITE)/sizeof(unsigned short)); // 0x2003
    Modbus_ARM2_Addr_SET[1].Data_ptr        = (void*)&Modbus_ARM2_Addr_0x2000;
    Modbus_ARM2_Addr_SET[1].Data_ptr_EEPROM = (void*)&Modbus_ARM2_Addr_0x2000_EEP; // 指向EEPROM备份
	Modbus_ARM2_Addr_SET[1].Check_Write_Data = S2_Check_Write_Data_0x2000; //配置 写入参数检查 函数
    Modbus_ARM2_Addr_SET[1].Data_Size       = sizeof(unsigned short);
    Modbus_ARM2_Addr_SET[1].Data_Pyte       = 1; // 可读可写

    // 配置内存区域 3 (示例: 地址 0x3000 开始的可读可写系统设置区域)
    Modbus_ARM2_Addr_SET[2].Start_Address   = 0x3000;
    Modbus_ARM2_Addr_SET[2].End_Address     = 0x3000 + (sizeof(IH_STA_READ_WRITE_SYS_SET)/sizeof(unsigned short)); // 0x3000
    Modbus_ARM2_Addr_SET[2].Data_ptr        = (void*)&Modbus_ARM2_Addr_0x3000;
    Modbus_ARM2_Addr_SET[2].Data_ptr_EEPROM = (void*)&Modbus_ARM2_Addr_0x3000_EEP; // 指向EEPROM备份
	Modbus_ARM2_Addr_SET[2].Check_Write_Data = S2_Check_Write_Data_0x3000; //配置 写入参数检查 函数
    Modbus_ARM2_Addr_SET[2].Data_Size       = sizeof(unsigned short);
    Modbus_ARM2_Addr_SET[2].Data_Pyte       = 1; // 可读可写

    // 设置从机 2 的基本信息
    // EKF 遥测: 0x1020 只读
    Modbus_ARM2_Addr_SET[3].Start_Address   = EKF_REG_BASE;
    Modbus_ARM2_Addr_SET[3].End_Address     = EKF_REG_BASE + EKF_REG_COUNT;
    Modbus_ARM2_Addr_SET[3].Data_ptr        = EKF_Regs_GetDataPtr(1);
    Modbus_ARM2_Addr_SET[3].Data_ptr_EEPROM = NULL;
    Modbus_ARM2_Addr_SET[3].Check_Write_Data = NULL;
    Modbus_ARM2_Addr_SET[3].Data_Size       = sizeof(unsigned short);
    Modbus_ARM2_Addr_SET[3].Data_Pyte       = 0;


    Modbus_Cofg[1].Us_Cof_ARM_Num       = Modbus_ARM2_Addr_SET;		// 关联内存区域配置
    Modbus_Cofg[1].ARM_Count            = DF_Modbus_ARM2_Num; 		// 每个协议里面 有多少组协议
    Modbus_Cofg[1].Slave_Hardware_Addr  = 10;					 	// 示例硬件地址
	Modbus_Cofg[1].Slave_Hardware_Addr_ERR_RET=0;					//从机硬件地址设置 错误回复什么？ ==0 保持沉默 不响应	 ==1 反馈错误代码
    Modbus_Cofg[1].Tx_Buf			=Modbus_TX_BUFF2;				//指向 发送缓存
	Modbus_Cofg[1].Comm_Tx_Len_Max      = DF_Modbus_S2_TX_Len;  	// 示例最大发送长度
	Modbus_Cofg[1].CRC_Order			=0;							//CRC高低字节 对调 ==0 使用默认先发低字节 再发高字节(大端)， ==1（小端） 对调









    // --- 初始化 从机 3 (索引 2) ---
    // 配置内存区域 1 (示例: 地址 0x1000 开始的只读区域)
    Modbus_ARM3_Addr_SET[0].Start_Address   = 0x1000;
    Modbus_ARM3_Addr_SET[0].End_Address     = 0x1000 + (sizeof(IH_STA_READ)/sizeof(unsigned short)); // 0x1003
    Modbus_ARM3_Addr_SET[0].Data_ptr        = (void*)&Modbus_ARM3_Addr_0x1000;
    Modbus_ARM3_Addr_SET[0].Data_ptr_EEPROM = NULL; // 只读区域，EEPROM指针为空
	Modbus_ARM3_Addr_SET[0].Check_Write_Data = NULL; // 只读区域，允许写入检查为空
    Modbus_ARM3_Addr_SET[0].Data_Size       = sizeof(unsigned short);
    Modbus_ARM3_Addr_SET[0].Data_Pyte       = 0; // 只读

    // 配置内存区域 2 (示例: 地址 0x2000 开始的可读可写区域)
    Modbus_ARM3_Addr_SET[1].Start_Address   = 0x2000;
    Modbus_ARM3_Addr_SET[1].End_Address     = 0x2000 + (sizeof(IH_STA_READ_WRITE)/sizeof(unsigned short)); // 0x2003
    Modbus_ARM3_Addr_SET[1].Data_ptr        = (void*)&Modbus_ARM3_Addr_0x2000;
    Modbus_ARM3_Addr_SET[1].Data_ptr_EEPROM = (void*)&Modbus_ARM3_Addr_0x2000_EEP; // 指向EEPROM备份
	Modbus_ARM3_Addr_SET[1].Check_Write_Data = S3_Check_Write_Data_0x2000; //配置 写入参数检查 函数
    Modbus_ARM3_Addr_SET[1].Data_Size       = sizeof(unsigned short);
    Modbus_ARM3_Addr_SET[1].Data_Pyte       = 1; // 可读可写

    // 配置内存区域 3 (示例: 地址 0x3000 开始的可读可写系统设置区域)
    Modbus_ARM3_Addr_SET[2].Start_Address   = 0x3000;
    Modbus_ARM3_Addr_SET[2].End_Address     = 0x3000 + (sizeof(IH_STA_READ_WRITE_SYS_SET)/sizeof(unsigned short)); // 0x3000
    Modbus_ARM3_Addr_SET[2].Data_ptr        = (void*)&Modbus_ARM3_Addr_0x3000;
    Modbus_ARM3_Addr_SET[2].Data_ptr_EEPROM = (void*)&Modbus_ARM3_Addr_0x3000_EEP; // 指向EEPROM备份
	Modbus_ARM3_Addr_SET[2].Check_Write_Data = S3_Check_Write_Data_0x3000; //配置 写入参数检查 函数
    Modbus_ARM3_Addr_SET[2].Data_Size       = sizeof(unsigned short);
    Modbus_ARM3_Addr_SET[2].Data_Pyte       = 1; // 可读可写

    // 设置从机 2 的基本信息
    // EKF 遥测: 0x1020 只读
    Modbus_ARM3_Addr_SET[3].Start_Address   = EKF_REG_BASE;
    Modbus_ARM3_Addr_SET[3].End_Address     = EKF_REG_BASE + EKF_REG_COUNT;
    Modbus_ARM3_Addr_SET[3].Data_ptr        = EKF_Regs_GetDataPtr(2);
    Modbus_ARM3_Addr_SET[3].Data_ptr_EEPROM = NULL;
    Modbus_ARM3_Addr_SET[3].Check_Write_Data = NULL;
    Modbus_ARM3_Addr_SET[3].Data_Size       = sizeof(unsigned short);
    Modbus_ARM3_Addr_SET[3].Data_Pyte       = 0;


    Modbus_Cofg[2].Us_Cof_ARM_Num       = Modbus_ARM3_Addr_SET;		// 关联内存区域配置
    Modbus_Cofg[2].ARM_Count            = DF_Modbus_ARM3_Num; 		// 每个协议里面 有多少组协议
    Modbus_Cofg[2].Slave_Hardware_Addr  = 15;					 	// 示例硬件地址
	Modbus_Cofg[2].Slave_Hardware_Addr_ERR_RET=0;					//从机硬件地址设置 错误回复什么？ ==0 保持沉默 不响应	 ==1 反馈错误代码
    Modbus_Cofg[2].Tx_Buf			=Modbus_TX_BUFF3;				//指向 发送缓存
	Modbus_Cofg[2].Comm_Tx_Len_Max      = DF_Modbus_S3_TX_Len;  	// 示例最大发送长度
	Modbus_Cofg[2].CRC_Order			=0;							//CRC高低字节 对调 ==0 使用默认先发低字节 再发高字节(大端)， ==1（小端） 对调








    // --- 初始化 从机 3 (索引 2) ---
    // 配置内存区域 1 (示例: 地址 0x1000 开始的只读区域)
    Modbus_ARM4_Addr_SET[0].Start_Address   = 0x1000;
    Modbus_ARM4_Addr_SET[0].End_Address     = 0x1000 + (sizeof(IH_STA_READ)/sizeof(unsigned short)); // 0x1003
    Modbus_ARM4_Addr_SET[0].Data_ptr        = (void*)&Modbus_ARM4_Addr_0x1000;
    Modbus_ARM4_Addr_SET[0].Data_ptr_EEPROM = NULL; // 只读区域，EEPROM指针为空
	Modbus_ARM4_Addr_SET[0].Check_Write_Data = NULL; // 只读区域，允许写入检查为空
    Modbus_ARM4_Addr_SET[0].Data_Size       = sizeof(unsigned short);
    Modbus_ARM4_Addr_SET[0].Data_Pyte       = 0; // 只读

    // 配置内存区域 2 (示例: 地址 0x2000 开始的可读可写区域)
    Modbus_ARM4_Addr_SET[1].Start_Address   = 0x2000;
    Modbus_ARM4_Addr_SET[1].End_Address     = 0x2000 + (sizeof(IH_STA_READ_WRITE)/sizeof(unsigned short)); // 0x2003
    Modbus_ARM4_Addr_SET[1].Data_ptr        = (void*)&Modbus_ARM4_Addr_0x2000;
    Modbus_ARM4_Addr_SET[1].Data_ptr_EEPROM = (void*)&Modbus_ARM4_Addr_0x2000_EEP; // 指向EEPROM备份
	Modbus_ARM4_Addr_SET[1].Check_Write_Data = S4_Check_Write_Data_0x2000; //配置 写入参数检查 函数
    Modbus_ARM4_Addr_SET[1].Data_Size       = sizeof(unsigned short);
    Modbus_ARM4_Addr_SET[1].Data_Pyte       = 1; // 可读可写

    // 配置内存区域 3 (示例: 地址 0x3000 开始的可读可写系统设置区域)
    Modbus_ARM4_Addr_SET[2].Start_Address   = 0x3000;
    Modbus_ARM4_Addr_SET[2].End_Address     = 0x3000 + (sizeof(IH_STA_READ_WRITE_SYS_SET)/sizeof(unsigned short)); // 0x3000
    Modbus_ARM4_Addr_SET[2].Data_ptr        = (void*)&Modbus_ARM4_Addr_0x3000;
    Modbus_ARM4_Addr_SET[2].Data_ptr_EEPROM = (void*)&Modbus_ARM4_Addr_0x3000_EEP; // 指向EEPROM备份
	Modbus_ARM4_Addr_SET[2].Check_Write_Data = S4_Check_Write_Data_0x3000; //配置 写入参数检查 函数
    Modbus_ARM4_Addr_SET[2].Data_Size       = sizeof(unsigned short);
    Modbus_ARM4_Addr_SET[2].Data_Pyte       = 1; // 可读可写

    // 设置从机 2 的基本信息
    // EKF 遥测: 0x1020 只读
    Modbus_ARM4_Addr_SET[3].Start_Address   = EKF_REG_BASE;
    Modbus_ARM4_Addr_SET[3].End_Address     = EKF_REG_BASE + EKF_REG_COUNT;
    Modbus_ARM4_Addr_SET[3].Data_ptr        = EKF_Regs_GetDataPtr(3);
    Modbus_ARM4_Addr_SET[3].Data_ptr_EEPROM = NULL;
    Modbus_ARM4_Addr_SET[3].Check_Write_Data = NULL;
    Modbus_ARM4_Addr_SET[3].Data_Size       = sizeof(unsigned short);
    Modbus_ARM4_Addr_SET[3].Data_Pyte       = 0;


    Modbus_Cofg[3].Us_Cof_ARM_Num       = Modbus_ARM4_Addr_SET;		// 关联内存区域配置
    Modbus_Cofg[3].ARM_Count            = DF_Modbus_ARM4_Num; 		// 每个协议里面 有多少组协议
    Modbus_Cofg[3].Slave_Hardware_Addr  = 20;					 	// 示例硬件地址
	Modbus_Cofg[3].Slave_Hardware_Addr_ERR_RET=0;					//从机硬件地址设置 错误回复什么？ ==0 保持沉默 不响应	 ==1 反馈错误代码
    Modbus_Cofg[3].Tx_Buf				=Modbus_TX_BUFF4;				//指向 发送缓存
	Modbus_Cofg[3].Comm_Tx_Len_Max      = DF_Modbus_S4_TX_Len;  	// 示例最大发送长度
	Modbus_Cofg[3].CRC_Order			=0;							//CRC高低字节 对调 ==0 使用默认先发低字节 再发高字节(大端)， ==1（小端） 对调








	Modbus_Init_Lib(Modbus_Cofg, DF_Modbus_US_Num); // 申请 配置
 
}


//========================================================================
// 函数: 
// 描述: 获取 硬件从机地址
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2025年12月2日
// 备注: 
//========================================================================
unsigned char Get_Slave_Hardware_Addr(unsigned char Slave_ID)
{

	if(Slave_ID>=DF_Modbus_US_Num)
	{
		return 0;
	}	
	return Modbus_Cofg[Slave_ID].Slave_Hardware_Addr;

}


//========================================================================
// 函数: 
// 描述: Modbus 协议 转I2c 原有的 数据结构
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期:  
// 备注:  
//========================================================================
typedef struct	comm_run_   /*主板运行状态信息*/
{
		uint8_t ihStatus;					//IH控制器状态寄存器：包含错误信息与工作状态
		uint8_t voltageAd;					//读取电压A/D采样值
		uint8_t currentAd;					//读取电流A/D采样值
		uint8_t igbtAd;						//读取IGBT温度传感器的A/D采样值
		uint8_t bottomAd;
		uint8_t	topAd;						//读取顶部温度传感器的A/D采样值
		uint8_t	actualPowerDiv25;			//读取实际功率值/25
		uint8_t	targetPowerDiv25;			//读取目标功率值/25
		uint8_t	actualPPG;					//读取实际加热PPG值
		uint8_t	powerStatus;				//读取功率限制状态（低4位）
		uint8_t	loadValue;					//读取负载检测脉冲数（低4位）和电压浪涌标志（高4位）
		uint8_t	vcountValue;				//反压计数器
		uint8_t	equivalentResistance;		//等效电阻 母线电流与谐振电流的平方
		uint8_t	res2;						//频率计数器  
		uint8_t	powerP25;					//25W修正值 

}COMM_RUN;	
 

typedef struct  IH_run_Init
{
	uint8_t 		loadTest;			//检锅强度设定
	uint8_t 		ovpShort;			//短路保护（高4位）
	uint8_t 		loadLeave;			//检锅功率设定
	uint8_t 		vcLimitMax;			//谐振电流保护值（高4位） 
	uint8_t 		loadLeavePhase;		//移锅相位值（高4位）
	uint8_t 		minPhase;					//最小相位设定(高6位）低电压区间电流限制值 ，低2位检锅信号下限值
	uint8_t 		potPowerM;				//钢锅铁锅修正，高4位 与最小相位的差值=H4+8 23~8  低4位 钢锅与铁锅的限制值差值=L4*100  1500~0
	uint8_t 		maxPowerM;	//最大连续功率设定

}IH_RunInit_t;						//初始化结构体

typedef struct
{
	uint8_t 		powerControlSet;	//控制字符（注意高低字节取反问题）
	uint8_t 		powerSwitch;		//抖频参数
	uint8_t 		powerSetm;			//功率设定
	uint8_t 		fanSpeed;			//风机调速
	uint8_t 		kValue;				//原来的K值


}PowerControlDef;						//初始化结构体

		
COMM_RUN			Read_IH_STA[DF_Stove_Quantity];
IH_RunInit_t		IH_Work_Init[DF_Stove_Quantity];
PowerControlDef		IH_Work_SET[DF_Stove_Quantity];
//========================================================================
// 函数: 
// 描述: Modbus 协议 转I2c 原有的 数据结构
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2025年12月2日
// 备注: 在数据 解析后使用
//========================================================================
void Modbus_I2c_Data_Main(unsigned char Idx)
{
	uint8_t* pStatus;
	IH_STA_READ_WRITE*			_0x2000 = NULL;
    IH_STA_READ_WRITE_SYS_SET* 	_0x3000 = NULL;
    if (Idx>DF_Modbus_Slave_04) 
	{
		return;
	}
 
    // 根据ARM设备索引选择对应的数据结构 
	 switch (Idx)
	 {
	 	case DF_Modbus_Slave_01:
				 _0x2000 = &Modbus_ARM1_Addr_0x2000_EEP;
		 		 _0x3000 = &Modbus_ARM1_Addr_0x3000_EEP;
				break;
	 	case DF_Modbus_Slave_02:
				 _0x2000 = &Modbus_ARM2_Addr_0x2000_EEP;
		 		 _0x3000 = &Modbus_ARM2_Addr_0x3000_EEP;
		break;
	 	case DF_Modbus_Slave_03:
				 _0x2000 = &Modbus_ARM3_Addr_0x2000_EEP;
		 		 _0x3000 = &Modbus_ARM3_Addr_0x3000_EEP;
		break;
	 	case DF_Modbus_Slave_04:
				 _0x2000 = &Modbus_ARM4_Addr_0x2000_EEP;
		 		 _0x3000 = &Modbus_ARM4_Addr_0x3000_EEP;
		break;
	 default:
 
		break;
	 }
		
	IH_Work_Init[Idx].loadTest		=_0x2000->Check_Pan_LV;			//高四位检锅间隔，低四位检锅强度
	IH_Work_Init[Idx].ovpShort		=_0x2000->syntony_Current_Short;			//短路保护（高4位）			 //原来m02 高四位最大PPG，低四位最小PPG
	IH_Work_Init[Idx].loadLeave		=_0x2000->Pan_Power;		//检锅功率设定				 //负载移出检测功率明码设置值
	IH_Work_Init[Idx].vcLimitMax	=_0x2000->syntony_Current;		//谐振电流保护值（高4位）  	 // 反压限制设置值
	IH_Work_Init[Idx].loadLeavePhase=_0x2000->phase_Pan;	//移锅相位值（高4位）		 // 负载有效电流
	IH_Work_Init[Idx].minPhase		=_0x2000->phase_Mix;		//最小相位设定(高6位）低电压区间电流限制值 ，低2位检锅信号下限值    // 电流修正系数
	IH_Work_Init[Idx].potPowerM		=_0x2000->Power_MIX;		//钢锅铁锅修正，高4位 与最小相位的差值=H4+8 23~8  低4位 钢锅与铁锅的限制值差值=L4*100  1500~0
	IH_Work_Init[Idx].maxPowerM		=_0x2000->Power_MAX;		//最大连续功率设定

	//工作字符
	IH_Work_SET[Idx].powerControlSet=_0x2000->Work_STA;	//控制字符（注意高低字节取反问题）
	IH_Work_SET[Idx].powerSwitch	=_0x2000->jitter_frequency;		//抖频参数
	IH_Work_SET[Idx].powerSetm		=_0x2000->target_Power;			//功率设定
	IH_Work_SET[Idx].fanSpeed		=_0x2000->FAN_Speed;			//风机调速
	IH_Work_SET[Idx].kValue			=0x00;					//原来的K值
 
	API_UART_RxInitCallback(Idx, (int8_t*)&IH_Work_Init[Idx],  sizeof(IH_RunInit_t) ); //初始化 数据 
	API_UART_RxControlCallback(Idx, (int8_t*)&IH_Work_SET[Idx],  sizeof(PowerControlDef) );//工作数据 

 
}




//========================================================================
// 函数: 
// 描述: 获取主板 默认的初始化值 ，并同步到 协议 存储好
// 参数: 	
// 返回: none.
// 版本: VER1.0
// 日期: 2024年8月10日
// 备注: 上电初始化时 执行一次
//========================================================================
void Get_IHPower_Main_Init_DATA(void) 
{
	uint8_t* pStatus; 
	IH_RunInit_t				IH_initSta;//主板 底层 实际初始化 结果
	IH_STA_READ_WRITE*			_0x2000 = NULL;
	unsigned char Idx;
	for(Idx=0;Idx<DF_Stove_Quantity;Idx++)
	{
	
		pStatus = API_UART_TxInitCallback(Idx, sizeof(IH_RunInit_t)); //读取底层初始化值
		 
		if (pStatus != NULL) {
			memcpy(&IH_initSta, pStatus, sizeof(IH_RunInit_t));
		}	
	
		 switch (Idx)
		 {
			case DF_Modbus_Slave_01:
					// _0x1000 = &Modbus_ARM1_Addr_0x1000;
					 _0x2000 = &Modbus_ARM1_Addr_0x2000_EEP;
					// _0x3000 = &Modbus_ARM1_Addr_0x3000_EEP;
					break;
			case DF_Modbus_Slave_02:
					// _0x1000 = &Modbus_ARM2_Addr_0x1000;
					 _0x2000 = &Modbus_ARM2_Addr_0x2000_EEP;
					// _0x3000 = &Modbus_ARM2_Addr_0x3000;
			break;
			case DF_Modbus_Slave_03: 
					// _0x1000 = &Modbus_ARM3_Addr_0x1000;
					 _0x2000 = &Modbus_ARM3_Addr_0x2000_EEP;
					 //_0x3000 = &Modbus_ARM3_Addr_0x3000;
			break;
			case DF_Modbus_Slave_04:
					 //_0x1000 = &Modbus_ARM4_Addr_0x1000;
					 _0x2000 = &Modbus_ARM4_Addr_0x2000_EEP;
					 //_0x3000 = &Modbus_ARM4_Addr_0x3000;
			break;
		 default:
	 
			break;
		 }	
	
		_0x2000->Check_Pan_LV=IH_initSta.loadTest;			//检锅强度设定
		_0x2000->syntony_Current_Short=IH_initSta.ovpShort;			//短路保护（高4位）
		_0x2000->Pan_Power=IH_initSta.loadLeave;			//检锅功率设定
		_0x2000->syntony_Current=IH_initSta.vcLimitMax;			//谐振电流保护值（高4位） 
		_0x2000->phase_Pan=IH_initSta.loadLeavePhase;		//移锅相位值（高4位）
		_0x2000->phase_Mix=IH_initSta.minPhase;					//最小相位设定(高6位）低电压区间电流限制值 ，低2位检锅信号下限值
		_0x2000->Power_MAX=IH_initSta.potPowerM;				//钢锅铁锅修正，高4位 与最小相位的差值=H4+8 23~8  低4位 钢锅与铁锅的限制值差值=L4*100  1500~0
		_0x2000->Power_MIX=IH_initSta.maxPowerM;	//最大连续功率设定	
	
	 }
 }
//========================================================================
// 函数: 
// 描述: 刷新 静态寄存器的数值,（读取数据时 保证数值是当前最新的
// 参数: 	
// 返回: none.
// 版本: VER1.0
// 日期: 2024年8月10日
// 备注: 	
//========================================================================
void Update_Static_Register_DATA(unsigned char Idx)
{
	
	uint8_t* pStatus; 
	IH_STA_READ*				_0x1000 = NULL;
	IH_STA_READ_WRITE*			_0x2000 = NULL;
    IH_STA_READ_WRITE_SYS_SET* 	_0x3000 = NULL;    
	IH_RunInit_t				IH_initSta;//主板 底层 实际初始化 结果
    if (Idx>DF_Modbus_Slave_04) 
	{
		return;
	}

    pStatus = API_UART_TxStatusCallback(Idx, sizeof(COMM_RUN)); //读取 加热底层的状态
    if (pStatus != NULL) {
        memcpy(&Read_IH_STA[Idx], pStatus, sizeof(COMM_RUN));
    }
	
	 pStatus = API_UART_TxInitCallback(Idx, sizeof(IH_RunInit_t)); //读取底层初始化值
	 
    if (pStatus != NULL) {
        memcpy(&IH_initSta, pStatus, sizeof(IH_RunInit_t));
    }
	
	
 	//只刷新对应的 从机变量 减少数据搬运时间
    // 根据ARM设备索引选择对应的数据结构 
	 switch (Idx)
	 {
	 	case DF_Modbus_Slave_01:
				 _0x1000 = &Modbus_ARM1_Addr_0x1000;
				 _0x2000 = &Modbus_ARM1_Addr_0x2000;
		 		 _0x3000 = &Modbus_ARM1_Addr_0x3000;
				break;
	 	case DF_Modbus_Slave_02:
			 	 _0x1000 = &Modbus_ARM2_Addr_0x1000;
				 _0x2000 = &Modbus_ARM2_Addr_0x2000;
		 		 _0x3000 = &Modbus_ARM2_Addr_0x3000;
		break;
	 	case DF_Modbus_Slave_03: 
		 		 _0x1000 = &Modbus_ARM3_Addr_0x1000;
				 _0x2000 = &Modbus_ARM3_Addr_0x2000;
		 		 _0x3000 = &Modbus_ARM3_Addr_0x3000;
		break;
	 	case DF_Modbus_Slave_04:
		 		 _0x1000 = &Modbus_ARM4_Addr_0x1000;
				 _0x2000 = &Modbus_ARM4_Addr_0x2000;
		 		 _0x3000 = &Modbus_ARM4_Addr_0x3000;
		break;
	 default:
 
		break;
	 }
   
	_0x2000->Check_Pan_LV=IH_initSta.loadTest;			//检锅强度设定
	_0x2000->syntony_Current_Short=IH_initSta.ovpShort;			//短路保护（高4位）
	_0x2000->Pan_Power=IH_initSta.loadLeave;			//检锅功率设定
	_0x2000->syntony_Current=IH_initSta.vcLimitMax;			//谐振电流保护值（高4位） 
	_0x2000->phase_Pan=IH_initSta.loadLeavePhase;		//移锅相位值（高4位）
	_0x2000->phase_Mix=IH_initSta.minPhase;					//最小相位设定(高6位）低电压区间电流限制值 ，低2位检锅信号下限值
	_0x2000->Power_MAX=IH_initSta.potPowerM;				//钢锅铁锅修正，高4位 与最小相位的差值=H4+8 23~8  低4位 钢锅与铁锅的限制值差值=L4*100  1500~0
	_0x2000->Power_MIX=IH_initSta.maxPowerM;	//最大连续功率设定
	   
	// (removed) _0x2000->target_Power 是写寄存器(0x2010), 由主机下发, 不应被覆盖. 读回值在 _0x1000->target_Power(0x1007)

	 
	 
	 
 	// I2C 映射区 (0x1000-0x100F): 严格按 I2C 读状态顺序填充
	_0x1000->SYS_Sta         = (Read_IH_STA[Idx].ihStatus & 0xF0) | (Idx + 1); //0x1000 IHStatus: BIT7-4=状态, BIT3-0=炉头号
	_0x1000->Vol_AD          = Read_IH_STA[Idx].voltageAd;                     //0x1001 VoltageValue
	_0x1000->Current_AD      = Read_IH_STA[Idx].currentAd;                     //0x1002 CurrentValue
	_0x1000->IGBT_AD         = Read_IH_STA[Idx].igbtAd;                        //0x1003 Sensor1Value
	_0x1000->Bot_AD          = Read_IH_STA[Idx].bottomAd;                      //0x1004 Sensor2Value
	_0x1000->Top_AD          = Read_IH_STA[Idx].topAd;                         //0x1005 Sensor3Value (预留)
	_0x1000->Practical_Power = Read_IH_STA[Idx].actualPowerDiv25;         //0x1006 ActualPower
	_0x1000->target_Power    = Read_IH_STA[Idx].targetPowerDiv25;              //0x1007 TargetPower (回读)
	_0x1000->Practical_PPG   = Read_IH_STA[Idx].actualPPG;                     //0x1008 ActualPPG
	_0x1000->P_limited_STA   = Read_IH_STA[Idx].powerStatus;                   //0x1009 PowerStatus
	_0x1000->Pan_pulsating   = Read_IH_STA[Idx].loadValue & 0x0F;              //0x100A LoadValue (低4位=检锅脉冲)
	_0x1000->HVol_Cnt        = Read_IH_STA[Idx].vcountValue;                   //0x100B VCNTValue
	_0x1000->PWMValue_L      = 0x00;                                           //0x100C PWMValue_L (预留)
	_0x1000->PWMValue_H      = 0x00;                                           //0x100D PWMValue_H (预留)
	_0x1000->PowerAdjust     = 0x00;                                           //0x100E PowerAdjust (预留)
	_0x1000->Version_Number  = DF_Versions;                                    //0x100F Version
	// MODBUS 扩展区 (0x1010-0x1014)
	_0x1000->Fan_AD          = 0x00;                                           //0x1010 风扇AD
	_0x1000->ERROR           = Read_IH_STA[Idx].ihStatus & 0x0F;               //0x1011 故障码 (ihStatus低4位)
	_0x1000->interior_ERR    = (Read_IH_STA[Idx].loadValue >> 4) & 0x0F;       //0x1012 内部故障 (浪涌标志)
	_0x1000->HZ_Cnt          = Read_IH_STA[Idx].res2;                          //0x1013 频率计数器
	_0x1000->Discard_Cnt     = 0x00;                                           //0x1014 丢波计数器

	EKF_Regs_Update(Idx);  // 刷新 EKF 遥测寄存器 (0x1020-0x1029)
}



//========================================================================
// 函数: 
// 描述: 串口API接口  负责把接收到的 数据 转移到 modbus的 协议 缓冲里面
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2025年12月2日
// 备注: 底层会 自动调用 此函数
//========================================================================
void API_UART_RxEventCallback(uint16_t Size)
{
	MB_Uart_Rx_Long=Size;
	API_UART_DMA_ReadValue(UARTX,MB_Uart_Rx_Data,DF_MB_Uart_Rx_LONG);
}


//========================================================================
// 函数: 
// 描述: Modbus 协议 解析
// 参数: 
// 返回: none.
// 版本: VER1.0
// 日期: 2025年12月2日
// 备注: 主函数调用 
//========================================================================
void Modbus_Protocol_Analysis_Main(void)
{
//	unsigned char ls_buff[50]={0}; //
	unsigned short RLong;
	unsigned char ls_a;
	
	Modbus_Cofg_Init_SET();	// 描述: 对Modbus 协议解析库 进行初始 配置
	init_SR_RW_DATA();		//初始化可读寄存器，( 从eeprom 读取可读写寄存器的 值
 
	RLong =MB_Uart_Rx_Long;//Check_Reception_Data_Frames_Lib(Modbus_Uart_COMM);  //查询是否有完整帧可读
	if( RLong  > 0 ) 
	{ 
		
		
		
		//Get_Data_Reception_Processing_Lib(Modbus_Uart_COMM, ls_buff );//读出帧数据
		   
		if(MB_Uart_Rx_Data[0] == Get_Slave_Hardware_Addr(DF_Modbus_Slave_01) ) //从机 1 地址匹配
		{
			Update_Static_Register_DATA(DF_Modbus_Slave_01);// 描述: 刷新 静态寄存器的数值,（读取数据时 保证数值是当前最新的
			
			processModbusRequest(DF_Modbus_Slave_01,	MB_Uart_Rx_Data,	RLong); //对接收到的 协议进行 解析
			
			Get_Mobus_Response_Data_Lib(DF_Modbus_Slave_01, MB_Uart_Rx_Data, &RLong);//对解析数据 做出回应 
			
			if(RLong)
			{
				//Uart_general_SendData(Modbus_Uart_COMM,ls_buff ,RLong,20000); //启动发送
				API_UART_DMA_SendValue(UARTX,MB_Uart_Rx_Data,RLong);
			}		

			Modbus_I2c_Data_Main(DF_Modbus_Slave_01);	 //刷新数据 给 底层加热	
		}
		
		else if(MB_Uart_Rx_Data[0] == Get_Slave_Hardware_Addr(DF_Modbus_Slave_02) ) //从机 2 地址匹配
		{
			Update_Static_Register_DATA(DF_Modbus_Slave_02);// 描述: 刷新 静态寄存器的数值,（读取数据时 保证数值是当前最新的
			
			processModbusRequest(DF_Modbus_Slave_02,	MB_Uart_Rx_Data,	RLong); //对接收到的 协议进行 解析
			
			Get_Mobus_Response_Data_Lib(DF_Modbus_Slave_02, MB_Uart_Rx_Data, &RLong);//对解析数据 做出回应 
			
			if(RLong)
			{
				//Uart_general_SendData(Modbus_Uart_COMM,ls_buff ,RLong,20000); //启动发送
				API_UART_DMA_SendValue(UARTX,MB_Uart_Rx_Data,RLong);
			}	
			
			Modbus_I2c_Data_Main(DF_Modbus_Slave_02);	 //刷新数据 给 底层加热	
		}	
		else if(MB_Uart_Rx_Data[0] == Get_Slave_Hardware_Addr(DF_Modbus_Slave_03) ) //从机 2 地址匹配
		{
			Update_Static_Register_DATA(DF_Modbus_Slave_03);// 描述: 刷新 静态寄存器的数值,（读取数据时 保证数值是当前最新的
			
			processModbusRequest(DF_Modbus_Slave_03,	MB_Uart_Rx_Data,	RLong); //对接收到的 协议进行 解析
			
			Get_Mobus_Response_Data_Lib(DF_Modbus_Slave_03, MB_Uart_Rx_Data, &RLong);//对解析数据 做出回应 
			
			if(RLong)
			{
				//Uart_general_SendData(Modbus_Uart_COMM,ls_buff ,RLong,20000); //启动发送
				API_UART_DMA_SendValue(UARTX,MB_Uart_Rx_Data,RLong);
			}	
			
			Modbus_I2c_Data_Main(DF_Modbus_Slave_03);	 //刷新数据 给 底层加热	
		}	
		else if(MB_Uart_Rx_Data[0] == Get_Slave_Hardware_Addr(DF_Modbus_Slave_04) ) //从机 2 地址匹配
		{
			Update_Static_Register_DATA(DF_Modbus_Slave_04);// 描述: 刷新 静态寄存器的数值,（读取数据时 保证数值是当前最新的
			
			processModbusRequest(DF_Modbus_Slave_04,	MB_Uart_Rx_Data,	RLong); //对接收到的 协议进行 解析
			
			Get_Mobus_Response_Data_Lib(DF_Modbus_Slave_04, MB_Uart_Rx_Data, &RLong);//对解析数据 做出回应 
			
			if(RLong)
			{
				//Uart_general_SendData(Modbus_Uart_COMM,ls_buff ,RLong,20000); //启动发送
				API_UART_DMA_SendValue(UARTX,MB_Uart_Rx_Data,RLong);
			}	
			
			Modbus_I2c_Data_Main(DF_Modbus_Slave_04);	 //刷新数据 给 底层加热	
		}	
		
		 
		 MB_Uart_Rx_Long=0;
	}
  

		
	   

}

 

// 
//unsigned short Get_0x2000_ADDR_DATA(unsigned char Idx, unsigned char ARM_idx)
//{
//    IH_STA_READ_WRITE* data_ptr = NULL;
//    
//    // 选择正确的数据结构
//    switch(ARM_idx)
//    {
//        case 0: data_ptr = &Modbus_ARM1_Addr_0x2000_EEP; break;
//        case 1: data_ptr = &Modbus_ARM2_Addr_0x2000_EEP; break;
//        default: return 0xFFFF;
//    }
//    
//    // 参数检查
//    if(Idx >= sizeof(IH_STA_READ_WRITE) / sizeof(unsigned short))
//        return 0xFFFF;
//    
//    // 直接通过偏移访问
//    return ((unsigned short*)data_ptr)[Idx];
//}



//unsigned short Get_0x3000_ADDR_DATA(unsigned char Idx, unsigned char ARM_idx)
//{
//    IH_STA_READ_WRITE* data_ptr = NULL;
//    
//    // 选择正确的数据结构
//    switch(ARM_idx)
//    {
//        case 0: data_ptr = &Modbus_ARM1_Addr_0x3000_EEP; break;
//        case 1: data_ptr = &Modbus_ARM2_Addr_0x3000_EEP; break;
//        default: return 0xFFFF;
//    }
//    
//    // 参数检查
//    if(Idx >= sizeof(IH_STA_READ_WRITE) / sizeof(unsigned short))
//        return 0xFFFF;
//    
//    // 直接通过偏移访问
//    return ((unsigned short*)data_ptr)[Idx];
//}















// 

///**
// * @brief 获取0x2000区域参数数据
// * @param member_idx 参数索引，使用 IH_0x2000_MEMBER_IDX 枚举值
// * @param arm_idx ARM设备索引，使用 ARM_DEVICE_IDX 枚举值
// * @return 参数值，如果索引无效返回 0xFFFF
// * @note 对应地址范围：0x2000-0x2013
// */
//unsigned short Get_0x2000_ADDR_DATA(IH_0x2000_MEMBER_IDX member_idx, ARM_DEVICE_IDX arm_idx)
//{
//    IH_STA_READ_WRITE* data_ptr = NULL;
//    
//    // 根据ARM设备索引选择对应的数据结构
//    switch(arm_idx)
//    {
//        case ARM_DEVICE_1:
//            data_ptr = &Modbus_ARM1_Addr_0x2000_EEP;
//            break;
//            
//        case ARM_DEVICE_2:
//            data_ptr = &Modbus_ARM2_Addr_0x2000_EEP;
//            break;
//            
//		case ARM_DEVICE_3:
//			data_ptr = &Modbus_ARM3_Addr_0x2000_EEP;
//			break;
//		case ARM_DEVICE_4:
//			data_ptr = &Modbus_ARM4_Addr_0x2000_EEP;
//			break;
//		
//		
//        default:
//            return 0;  ///< 无效的ARM设备索引
//    }
//    
//    // 检查参数索引是否有效
//    if(member_idx >= MEMBER_COUNT_0x2000)
//    {
//        return 0;  ///< 无效的参数索引
//    }
//    
//	
////	if( Get_Uart_Online_Status_Lib(Modbus_Uart_COMM) == OFFLINE ) //获取串口在线状态
////	{
////		  return 0;  ///< 无效的参数索引
////	}	
//	
//    // 通过指针偏移访问对应的成员
//    return ((unsigned short*)data_ptr)[member_idx];
//}

///**
// * @brief 获取0x3000区域系统设置参数数据
// * @param member_idx 参数索引，使用 IH_0x3000_MEMBER_IDX 枚举值
// * @param arm_idx ARM设备索引，使用 ARM_DEVICE_IDX 枚举值
// * @return 参数值，如果索引无效返回 0xFFFF
// * @note 对应地址范围：0x3000-0x3003
// */
//unsigned short Get_0x3000_ADDR_DATA(IH_0x3000_MEMBER_IDX member_idx, ARM_DEVICE_IDX arm_idx)
//{
//    IH_STA_READ_WRITE_SYS_SET* data_ptr = NULL;
//    
//    // 根据ARM设备索引选择对应的数据结构
//    switch(arm_idx)
//    {
//        case ARM_DEVICE_1:
//            data_ptr = &Modbus_ARM1_Addr_0x3000_EEP;
//            break;
//            
//        case ARM_DEVICE_2:
//            data_ptr = &Modbus_ARM2_Addr_0x3000_EEP;
//            break;
//		case ARM_DEVICE_3:
//			data_ptr = &Modbus_ARM3_Addr_0x3000_EEP;
//		 break;
//		case ARM_DEVICE_4:
//			data_ptr = &Modbus_ARM4_Addr_0x3000_EEP;
//		 break;
//		
//        default:
//            return 0;  ///< 无效的ARM设备索引
//    }
//    
//    // 检查参数索引是否有效
//    if(member_idx >= MEMBER_COUNT_0x3000)
//    {
//        return 0;  ///< 无效的参数索引
//    }
//    
//    // 通过指针偏移访问对应的成员
//    return ((unsigned short*)data_ptr)[member_idx];
//}











