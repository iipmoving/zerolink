/********************************************************************************
    FileName    :  app_task.c
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  应用任务函数文件，所有应用函数的最终调用都在这里

    Date        :  2018-09-14
    Modify      :
                   2018-09-14 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/


/********************************************Head Files*/



#include 	"sys_task.h"
#include	"s_time_base.h"

#include	"app_task.h"
#include	"app_power.h"
#include	"APP_ZERO.H"
#include	"APP_ADC.H"

#include	"API_DMA.H"
#include	"API_gpio.h"
#include 	"API_hrtim.h"
#include 	"API_TIM.h"
#include 	"API_ADC.h"
#include 	"API_COMP.h"
#include	"API_I2C.H"
#include	"API_OPAMP.H"
#include	"API_DAC.H"
#include	"proto_i2c.h"
#include	"commClass.h"
#include	"adc_processing.h"
#include	"API_UART.H"
#include "Modbus_Lib_Init_An_Analysis.h" 
#include	"wave_capture.h"
void Task_Sys(void);
/*********************************************变量申请*/

/*********************************************函数列表*/



//=================================================任务函数列表
/********************************************************************************
*name       : void AppTaskRoute(void)
*author     : rsl
*function   : 用户时间片调用
*Para       : 无
*return     : NULL
*brief      : 看门狗，时间基准，蜂鸣器，电机软启动，继电器控制等底层驱动占用时间比较少的任务
            全部放在一起。
********************************************************************************/


void	AppTaskRoute(void)
{


	AppTask_Init();					//先把指针设置完成
	SystemInitial();				//
	
    while (1)
    {	
//			WatchDogFeed();
			Sys_TaskService();
//---------这里可以增加需要多次查询的程序段，但执行完时间片任务会挂起------------------------------
	}


}	


//=================================================任务函数列表
/********************************************************************************
*name       : void Task_TimeChip1(void)
*author     : rsl
*function   : 时间片任务1
*Para       : 无
*return     : NULL
*brief      : 看门狗，时间基准，蜂鸣器，电机软启动，继电器控制等底层驱动占用时间比较少的任务
            全部放在一起。
********************************************************************************/
void Task_TimeChip0(void)
{
//    Bsp_WTDFeed();      //看门狗
//	APP_POWERR_SetTxaAwdValue(TxaAwdHigh);		//设置AWD值 谐振电流保护值 ch=0 正常高值  ch=1 低电压移锅值

	Time_Base();
//	APP_ADC_AVG_Fun();				//得到上20MS得到的ADC数据

}

/**
 * @brief Switcher模块Slot1时间片运行函数（弱定义）
 * 
 * 该函数为Switcher模块的时间片Slot1提供默认的空实现。
 * 使用__attribute__((weak))属性声明为弱符号，允许用户在其他源文件中
 * 提供同名函数的强定义来覆盖此默认实现。
 * 
 * Slot1是时间片序号之一，系统每1ms执行一个SLOT，各Slot按序轮转执行，
 * 实现周期性任务调度机制。
 * 
 * @note 如果用户未提供自定义实现，系统将使用此空函数作为默认行为。
 */
__attribute__((weak)) void Switcher_Run_Slot1(void) {} 
/** @brief Switcher模块初始化函数（弱定义）
 * 
 * 该函数为Switcher模块提供默认的空初始化实现。
 * 使用__attribute__((weak))属性声明为弱符号，允许用户在其他源文件中
 * 提供同名函数的强定义来覆盖此默认实现。
 * 
 * @note 如果用户未提供自定义实现，系统将使用此空函数作为默认行为。
 */
__attribute__((weak)) void Switcher_Init(void) {}
/**
 * @brief Switcher模块Tick任务入口函数（弱定义）
 * 
 * 该函数为Switcher模块的Tick任务提供默认的空实现，是每1ms空闲时间段的程序段入口。
 * 使用__attribute__((weak))属性声明为弱符号，允许用户在其他源文件中
 * 提供同名函数的强定义来覆盖此默认实现。
 * 
 * 该函数在时间片Slot执行之后调度，适用于需要低优先级处理的周期性任务，
 * 
 * 
 * @note 如果用户未提供自定义实现，系统将使用此空函数作为默认行为。
 */
__attribute__((weak)) void Switcher_Run_TK(void) {}
void Task_TimeChip1(void)
{
		Time_Base();
//	API_TIM_TGO_PPG_SINGLE_Start();
		APP_POWER_CompSetValue();


}
void Task_TimeChip2(void)
{
//       API_GPIO_WritePin(DebugB_pin,1);

 		Switcher_Run_Slot1();			// v2.0 Data Switcher: ADC → Power 路由
//////		 API_GPIO_WritePin(DebugB_pin,0);

}

void Task_TimeChip3(void)
{
}

void Task_TimeChip4(void)
{
 	APP_POWER_PanStartPluse();		//产生起振脉冲，和功率控制分离
}
void Task_TimeChip5(void)
{

}
void Task_TimeChip6(void)
{
		API_POWER_PanFmac();				//开始PAN FMAC滤波

}
void Task_TimeChip7(void)
{

	


}

void Task_TimeChip8(void)
{
	APP_POWER_DutyLess50();		//检查占空比调整炉头，靠后，近量使功率调整完成
	APP_POWER_PotTypeCheck();	//锅具类型修正
// 	APP_POWER_CycleChange();
//	APP_POWERR_SetTxaAwdValue();		//设置AWD值 谐振电流保护值 ch=0 正常高值  ch=1 低电压移锅值

}
void Task_TimeChip9(void)
{

//	printf("T9\n");
}

void Task_Tk(void)      //触摸扫描
{

#ifdef	COMM_UART	
	Modbus_Protocol_Analysis_Main();
#endif
	
	
	iic_bus_updata();
	APP_ADC_TimDmaEnd();
	APP_ADC_CalculatePower();
	Switcher_Run_TK();
}


//extern 	void	Voltage_change_check(void);		//检查电压突升


void Task_Ms(void)
{
//	Voltage_change_check();				//检查电压突升
//	adc_ic_vc_fun();							//每一MS采集一次电流电压

 	API_ADC_DMA_TimStart();

//	API_POWER_ScrOutput(SYS_GetTskId());			//过零开通


	
	
}	

/********************************************************************************
*name       : void Task_Sys(void)
*author     : rsl
*function   : 系统任务，产生时基以及切换任务
*Para       : 无
*return     : NULL
*brief      : 1ms运行一次
********************************************************************************/
void Task_Sys(void)
{
	SYS_TaskRotate();
}

void Power_OffInit(void)
{



}


TSK_FUN const TaskFun[SYS_TSK_NUM] = //任务函数数组
{
    //把各个任务函数赋值给任务函数列表，函数名称可以改变
    Task_TimeChip0,              //时间片0
    Task_TimeChip1,              //时间片1
    Task_TimeChip2,              //时间片2
    Task_TimeChip3,              //时间片3
    Task_TimeChip4,              //时间片4
    Task_TimeChip5,              //时间片5
    Task_TimeChip6,              //时间片6
    Task_TimeChip7,              //时间片7
    Task_TimeChip8,              //时间片8
    Task_TimeChip9,              //时间片9
    Task_Tk,              	 			//空闲执行任务
		Task_Ms,                  	 //1ms任务
    Task_Null,              	 //常规任务2
    Task_Null,              	 //常规任务3
    Task_Null,              	 //常规任务4
    Task_Sys                	 //系统任务，不可改变
};


/********************************************************************************
*name         : void SystemInitial(void)
*author       : rsl
*function     : 系统初始化调用 
*para         : 无
*return       : 无
*brief        : 在TASK使能后开始调用
********************************************************************************/

void Error_Handler(void)
{
}	

uint8_t string[10]="ABCDE12345";
uint8_t rxBuff[25];



void SystemInitial(void)
{
	

	


  /* Initialize all configured peripherals */
	
	API_SystemClocks_Init();  
//	CopyISRToRAM();
	
	WaveCapture_Init();
	API_GPIO_PORT_INIT();
	API_DMA_Init();	


	API_UART_Init(UARTX,API_DMA_GetDmaHandle(ChDmaRx),API_DMA_GetDmaHandle(ChDmaTx));		//控制口采用DMA


	API_UART_DMA_SendValue(UARTX,string,10);
	
//	API_UART_DMA_ReadValue(UARTX,MB_Uart_Rx_Data,DF_MB_Uart_Rx_LONG);	
	
	


	API_ADC_Init();

	API_OPAMP_Init();
	
	API_DAC_Init();			

	API_COMP_Init();


//	API_HRTIM_SetDmaHandle();
	API_HRTIM1_Init();
	API_HRTIM_MasterSync_InitMaster(MAX_FRE_PWM*2);      // Master 周期 = 基频
	
#if 0	
#include "API_hrtim_fullbridge.h"	
	// ==================== 全桥1: 调频模式 ====================
	// 50kHz，90度移相(phaseShift = period/4 = 7680)
	API_FB_OutputFreqModulation(PotCh1, 50000, 7680);

	// ==================== 运行时调整 ====================
	API_FB_SetFrequency(PotCh1, 60000);       // 改频率到60kHz（自动保持50%占空比）
	API_FB_SetPhaseShift(PotCh1, 3840);       // 改移相到45度

	// ==================== 全桥2: 调功模式 ====================
	uint16_t period = (192000000U * 4U) / 30000;   // 30kHz
	uint16_t cmpVal = period * 40 / 100;            // 40%占空比
	API_FB_OutputPowerModulation(PotCh2, 30000, cmpVal);

	// 调整功率
	API_FB_SetCmpValue(PotCh2, period * 60 / 100);  // 调到60%


uint16_t basePer = FRE_25K_PWM * 2;           // 25kHz 基频
API_HRTIM_MasterSync_InitMaster(basePer);      // Master 周期 = 基频
API_HRTIM_MasterSync_ConfigSlave(PotCh1, basePer);       // 25kHz
API_HRTIM_MasterSync_ConfigSlave(PotCh2, basePer / 2);   // 50kHz (2倍频)
API_HRTIM_MasterSync_StartAll();               // 同一条指令全部启动


// ==================== 停止 ====================
API_FB_Stop(PotCh1);
API_FB_Stop(PotCh2);
	
#endif	
	
	PPGvalueDef		value;
	value.duty=PAN_FRE_PWM;
	value.prioed=PAN_FRE_PWM*2;
	
	API_HRTIM_MasterSync_SetPeriod(value.prioed);
	API_PPG_setValue(PotCh1,value);
	API_PPG_setValue(PotCh2,value);
	API_PPG_setValue(PotCh3,value);	
	API_PPG_setValue(PotCh4,value);		
	API_PPG_setValue(PotChTest1,value);	
	API_PPG_setValue(PotChBase,value);		
  /* Enable HRTIM's outputs TD1 and start Timer D */
//	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pulldown);
	
	API_I2C_Init();
	API_TIM_INIT();
	

	
	API_GPIO_PinPull(HRTIM_SYN_pin,PUPDR_Pullup);
//	APP_ADC_PanSwChange(0x20);

	printf("system init over\n");
}




/********************************************************************************
*name         : void App_Init(void)
*author       : rsl
*function     : 应用层初始化调用 
*para         : 无
*return       : 无
*brief        : 在TASK使能后开始调用
********************************************************************************/


void AppTask_Init(void)
{
	
	Sys_SetTskFunAddress((TSK_FUN*)TaskFun);
	Switcher_Init();                 /* v2.0 Data Switcher 初始化 */
//	SetDrvCallbackFun(TimIrqHandlePPGstepChange,TIM6_PPGstepChangeFun);					//调整PPG值增减
//	SetDrvCallbackFun(Tim8IrqHandlePPGstepChange,TIM8_PPGstepChangeFun);				//设置PPG到寄存器	
//	
//	SetDrvCallbackFun(AdcIrqHandlePPGstepChangeCh1,ADC_PPGstepDecFunCh1);				//限电流减PPGCH1
//	SetDrvCallbackFun(AdcIrqHandlePPGstepChangeCh2,ADC_PPGstepDecFunCh2);				//限电流减PPGCH2

//	SetDrvCallbackFun(AdcIrqHandleWatchDogLock,ADC_WatchDogLock);				//限电流减PPGCH2

//	
//	SetDrvCallbackFun(TimIrqHandleBkCh1,TIM_BkFunCh1);	
//	SetDrvCallbackFun(TimIrqHandleBkCh2,TIM_BkFunCh2);		
	
	I2cSlaveInit();
	u_power_init();

	Sys_Init();



	
}





//**********************************end of file********************************

