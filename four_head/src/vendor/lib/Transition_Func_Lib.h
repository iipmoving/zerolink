#ifndef __Transition_Func_Lib_H_
#define __Transition_Func_Lib_H_


typedef enum   //故障检测库 返回代码   
{ 
	 DF_TF_NoErr=0, //无故障(TF == Transition Func
 	 
	 DF_TF_Func_Err,		//10函数配置异常
						// 1.程序无法进行计算 检查配置
						// 2.配置超出范围 
						// 3.模块调用时 未进行 初始化
  
}Transition_Func_Lib_EER_CODE;
//========================================================================
// 函数:  
// 描述:通用功能模块 时钟，该函数需要外部调用，
// 参数: TimerCont 调用速度/uS
// 返回: DF_OC_Func_Err || DF_OC_NoErr
// 版本: VER1.0
// 日期: 2025年7月12日
// 备注: 设置最大时间 为1us--1秒
//========================================================================
unsigned char Transition_Func_Lib_SYS_Clock(unsigned long TimerCont);

//======================================================================================================================================================================================================
// 
// 			
// 
//									↓↓↓↓↓↓↓		AD 温度公式法
// 
// 
// 
//======================================================================================================================================================================================================
//  NTC转温度公式法
// 定义电压配置结构体
typedef struct _AD_Temp_Conversion_RAM 
{
	unsigned long _Rx;   // //分压电阻
    unsigned long _Rp;   // //热敏电阻 25度  100K 
    unsigned long _Bx;  // //热敏 电阻参数B值
     float 		  _Vol;  // AD 分辨率 电压， 5V@ 10bit 0.00489  5V@ 8bit  0.0195
 
	
	
} AD_Temp_Conversion_RAM_;

//========================================================================
// 函数:  
// 描述: NTC AD 转换实际温度 公式法
// 参数: const 配置，	输入 AD
// 返回: 实际温度 250 == 25.0 ，精确到小数点 后一位	
// 版本: VER2.0
// 日期: 2025年3月11日
// 备注: 	
//========================================================================
int Temp_Conversion(const AD_Temp_Conversion_RAM_ *Cof_SET, unsigned short In_Ad);


////配置A,  档 参数不一致时 申请新的 配置
//code AD_Temp_Conversion_RAM_   NTC_3950_100K_20K_10BitAD =        
//{
// 
//    20000,	  // //分压电阻
//	100000,	  // //热敏电阻 25度  100K 
//	3950,	 // //热敏 电阻参数B值
// 	0.00489,	  // AD 分辨率 电压， 5V@ 10bit 0.00489  5V@ 8bit  0.0195
//	 
//};



//======================================================================================================================================================================================================
// 
// 			
// 
//									↓↓↓↓↓↓↓		AD 转换实际电压  区间线性平分法
// 
// 
// 
//======================================================================================================================================================================================================
 
// 定义电压配置结构体
typedef struct _VolAD_To_Vol_RAM 
{
	unsigned short _65V_AD;   // 65V 实测AD(无法测试填写 估算值
    unsigned short _90V_AD;   // 90V 实测AD
    unsigned short _120V_AD;  // 120V 实测AD
    unsigned short _150V_AD;  // 150V 实测AD
    unsigned short _180V_AD;  // 180V 实测AD
    unsigned short _220V_AD;  // 220V 实测AD
    unsigned short _240V_AD;  // 240V 实测AD
    unsigned short _250V_AD;  // 250V 实测AD
    unsigned short _270V_AD;  // 270V 实测AD
	unsigned short _300V_AD;  // 300V 实测AD(无法测试填写 估算值
	
	
} VolAD_To_Vol_RAM;

 
// 运行时内存中的电压配置
extern VolAD_To_Vol_RAM	 Vol_AD_ARM;
//========================================================================
// 函数:  
// 描述: 电压AD 转换实际电压
// 参数:	
// 返回: 	
// 版本: VER2.0
// 日期: 2025年3月11日
// 备注: 	
//========================================================================
unsigned short ad_to_voltage(unsigned short ad_value );
	









//======================================================================================================================================================================================================
// 
// 			
// 
//									↓↓↓↓↓↓↓				时间控制模块，定时 预约 无人看管  
// 
// 
// 
//======================================================================================================================================================================================================
typedef struct _Time_Control_Module_STATE 
{
	unsigned long DingShi_Timer;  //定时时间/S
	unsigned long YuYue_Timer;	  //预约时间/S
	unsigned long Unattended_Timer;//无人看管时间/S
	
	unsigned char Work_STA;		  //运行 状态 ||工作 状态
	unsigned char Tips_STA;		  //提示 状态
	unsigned char SET_STA;		  //设置 状态

	
	unsigned char Tips_Cnt;		  //提示 计数器
	unsigned char Tips_Timer;	  //提示 时间间隔
	unsigned char Bit_1S;   	  // 一秒时间标志
	
	
} Time_Control_Module_STATE_;


typedef struct _Time_Control_Module_RAM	//时间控制 配置
{
	unsigned short   DS_Tips;  	   //（秒） 倒计时 剩余多久 提示？(DS == 定时
	unsigned char  	 DS_Tips_JG;  //间隔多少秒提示 一次？
	unsigned char  	 DS_Tips_Cnt;  //总共提示几次 ？

	unsigned short   YY_Tips;  	   //（秒） 预约 剩余多久 提示？(YY == 预约
	unsigned char  	 YY_Tips_JG;  //间隔多少秒提示 一次？
	unsigned char  	 YY_Tips_Cnt;  //总共提示几次 ？
	
	unsigned char  	 Time_SET_Operating_Mode;  //时间设置操作模式 "_Transition_Func_Time_SET_Operating_Mode" 里面选
												//==0 同时设置	小时   & 	分钟	最后按下 无响应
												//==1 先设置 	小时 再设置 分钟	循环设置
												//==2 先设置 	分钟 再设置 小时	循环设置

												//==3 同时设置	小时   & 	分钟	立即确认时间
												//==4 先设置 	小时 再设置 分钟	立即确认时间
												//==5 先设置 	分钟 再设置 小时	立即确认时间
 
	unsigned char  	 Time_SET_Sec_Min; //时间 是否 设定为整数分钟，应用场景，计时器是 按照 秒来计算的，
									   //假设 设置 3分钟 的定时，当2分30秒（数码管显示3分钟）时增加了 一分钟 则是 3分30秒
									   //此时 数码管显示 4分钟，实际只运行 3分30秒， 需用默认 为0的 配置则 会自动校准为 4分钟 
									   //==1 设定为 整分钟
									   //==0 不修改 又输入决定

	
		

} Time_Control_Module_RAM_;




enum _Transition_Func_Time_SET_Operating_Mode  // 时间设置操作模式
{ 
	DF_TC_SET_HM_No=0,		//==0 同时设置	小时   & 	分钟	最后按下 无响应
	DF_TC_SET_H_M_No,		//==1 先设置 	小时 再设置 分钟	循环设置
	DF_TC_SET_M_H_No,		//==2 先设置 	分钟 再设置 小时	循环设置
	
	DF_TC_SET_HM_OK,		//==3 同时设置	小时   & 	分钟	立即确认时间
	DF_TC_SET_H_M_OK,		//==4 先设置 	小时 再设置 分钟	立即确认时间
	DF_TC_SET_M_H_OK,		//==5 先设置 	分钟 再设置 小时	立即确认时间
	
};





//========================================================================
// 函数:  
// 描述: 定时 预约，无人看管 时钟 该函数需要外部调用，
// 参数: TimerCont
// 返回: 
// 版本: VER1.0
// 日期: 2025年3月26日 
// 备注: 设置最大时间 为1us--1秒(调用的时间 约精准 ，定时 预约的 就越准
//========================================================================
unsigned char Time_Control_Module_SYS_Clock(unsigned long TimerCont);

//========================================================================
// 函数:  
// 描述: 时间控制模块，定时 预约 无人看管 初始化
// 参数: 
// 返回: 配置故障状态
// 版本: 
// 日期: 2025年5月4日 17:09:21
// 备注: 
//========================================================================  
unsigned char Time_Control_Module_Init(Time_Control_Module_STATE_ *Time_Buffer, Time_Control_Module_RAM_ *TC_ARM, unsigned char Timer_Count);

//========================================================================
// 函数:  
// 描述: 时间控制模块，定时 预约 无人看管 控制时间 计时任务
// 参数: 
// 返回: DF_TF_Func_Err 、 DF_TF_NoErr
// 版本: 
// 日期: 2025年5月4日 17:09:21
// 备注: 每间隔 100ms 调用就行
//========================================================================  
unsigned char Time_Control_Module(void);




// 属于设置项目 可以根据情况 多次调用 ↓↓↓↓↓↓↓↓↓↓↓↓




enum _Transition_Func_Sta_GET  //状态 获取
{ 
	DF_TC_Tips_Sta=1, 	//获取 提示状态
	DF_TC_Work_Sta,		//获取 工作状态
	DF_TC_SET_Sta, 		//获取 设置状态
	DF_TC_DS_Timer, 		//获取 定时 时间
	DF_TC_YY_Timer, 		//获取 预约 时间
	DF_TC_WR_Timer, 		//获取 无人 时间
	//DF_TC_GET_Sta_ERR=0xFFFFFFFF,	//获取 状态  错误 !! 由于c51 不支持 常量 FFFFFFFF 实际出 故障 返回FF！
};

 
enum _Transition_Func_Timer_SET_Sta  //时间控制模块 设置 状态  || 显示状态 （返回状态）
{ 
	//!! 禁止调整顺序 ,只能增加！
	DF_TC_SET_No=0, //没有设置 ... 
	DF_TC_SET_H_M, 	//正在设置...同时显示 小时 & 分钟
	DF_TC_SET_H, 	//正在设置...显示 小时
	DF_TC_SET_M, 	//正在设置...显示 分钟
	
};

enum _Transition_Func_Timer_Tips_Sta  //时间控制模块   提示 状态	（返回状态）
{ 
	DF_TC_Tips_OFF=0, 	//没有提示状态状态
	DF_TC_DS_TipsA,		//定时提示状态A（有些客户 需要在定时最后x分钟 提示几声）,
	//DF_TC_Tips_Dispose, //已处理  提示状态状态
	DF_TC_YY_TipsA,		//预约提示状态A（有些客户 需要在定时最后x分钟 提示几声） 
};


unsigned long Get_Time_Control_Module_Sta(unsigned char SET_STA,unsigned char Idx);// 描述: 时间控制模块，状态获取 



//========================================================================
// 函数:  
// 描述: 时间控制模块，状态设置
// 参数: DF_TC_DS_Run、DF_TC_DS_SET、DF_TC_YY_SET、DF_TC_YY_Run、DF_TC_WR_Run 、DF_TC_NoFun
//		
// 返回: DF_TF_NoErr 、DF_TF_Func_Err
// 版本: 
// 日期: 2025年5月4日 17:09:21
// 备注: 
//========================================================================  
enum _Transition_Func_Timer_Control_Sta  //时间控制模块 状态 （模块 工作状态）
{ 
	DF_TC_NoFun=0, 	//空闲状态(该模块 没有 在进行任何 任务  （一般 菜单有流程控制 || 有时间 控制时 避免冲突 
  
	DF_TC_DS_Run, 		//1定时进行时 || 时间 设置完成
	DF_TC_DS_SET,		//2设置定时 状态 || 设置时间
	DF_TC_DS_Cancel,	//3取消定时 !！默认转 无人看管
	DF_TC_DS_End,		//4定时 结束(不可用于设置 函数！！
 
	DF_TC_YY_Run, 		//5预约进行时
	DF_TC_YY_SET,		//6设置预约 状态 || 设置时间
	DF_TC_YY_Cancel,	//7取消预约  !！默认转 无人看管
	DF_TC_YY_End,		//8预约  结束（不可用于设置 函数！！
 
	DF_TC_WR_Run,		//9  无人看管进行时  
	DF_TC_WR_End,		//10 无人看管进行时 	 (不可用于设置 函数！！
	DF_TC_WR_SET,		//11 无人看管 ，  设置时间
  
	DF_TC_Tips_CLR,		//12 清除提示状态，表示已处理提示任务
};
unsigned char Time_Control_Module_SET(unsigned char SET_STA,unsigned char Idx);

//========================================================================
// 函数:  
// 描述: 时间控制模块，时间 设置
// 参数: SET_Data 输入 时间,  DF_TC_WR_SET  DF_TC_YY_SET    DF_TC_DS_SET
// 返回: DF_TF_NoErr 、DF_TF_Func_Err
// 版本: 
// 日期: 2025年6月2日
// 备注: 需要 状态正确 才允许 设置时间，例如：想要修改定时 时间 必须在 “DF_TC_DS_SET”状态下 才能实现！！
//		 无人看管 不需要 ，任何状态 均可 修改时间
//========================================================================  
unsigned char Time_Control_SET(unsigned char SET_STA,unsigned long SET_Data,unsigned char Idx);




//======================================================================================================================================================================================================
// 
// 			
// 
//									↓↓↓↓↓↓↓				增量式 PID算法
// 
// 
// 
//======================================================================================================================================================================================================
typedef struct
{
	float setpoint; //目标温度 
	float kp;		//P
	float ki;		//I
	float kd;		//D
	float lasterror;
	float preerror;
	float result;	//返回输出值 
//	float deadband;
}PID_Incremental;

//========================================================================
// 函数: 
// 描述: pv  实际值 (实际温度 /转速/
// 参数: 无  
// 返回: 无
// 版本: VER1.0
// 日期: 二〇二三年五月二十五日 16:07:54
// 备注:    
//========================================================================
void PIDIncrementalCalc(PID_Incremental *vPID, float pv);

// 调用 方法
//1.  PID_Incremental   temperaturePid;//申请执行 内存
//2.  PID初始化 
//void PID_INT()
//{
//	
//	temperaturePid.kp=15.00;
//	temperaturePid.ki=0.020;
//	temperaturePid.kd=0;
//	temperaturePid.lasterror=0;
//	temperaturePid.setpoint=0;
//	temperaturePid.preerror=0;
//	temperaturePid.result=0;

//}
//3. 可以根据应用  动态调节 PID 参数
//				temperaturePid.kp=150.0;//50.0; //0.9000;  调整的是 每一度的差值
//				temperaturePid.ki=0.0250;//0.0040;
//				temperaturePid.kd=0.0180;//0.0035;	
//4.更新目标温度 /转速 ..
//				temperaturePid.setpoint = Heat_Step_Temp;	//目标温度 
//5.输入 实际温度 /转速 
//				PIDIncrementalCalc(&temperaturePid,Global_Cook.Bot_Temp_THig);	//输入实际温度
//6.根据实际 返回结果 做调整
//				temperaturePid.result;	//返回加热值

//======================================================================================================================================================================================================
// 
// 			
// 
//									↓↓↓↓↓↓↓				斜率计算，
// 
// 
// 
//======================================================================================================================================================================================================
typedef struct _Slope_Calculate_STATE_	//高斜率停止加热 运行缓存
{
	signed 	 short	TempSlop;		//斜率结果 
	unsigned short	SlopCaclCnt;	//斜率 周期累计/秒
	unsigned long	TempHead;		//头部累计
	unsigned long	TempTail; 		//尾部累计 
	unsigned short	Check_Speed;	//采集速度累计

	 
	
//-------- 以下参数 需要在 初始化的 时候 配置好！！！！！
	
	unsigned short  HEAD_CNT;     	//斜率 前部时间	(根据特性 设置 前后时间 坡度
	unsigned short 	TAIL_CNT;		//斜率 后部时间 一般来说 该值是前部时间的2倍，  (根据特性 设置 前后时间 坡度
	unsigned short 	Speed_SET;		//采集速度 设置（一般来说是按照秒为单位，如果参数变化非常快快则可以缩短 时间
									//该值和 函数调用速度挂钩，比如 函数每100ms调用一次 则设置 10，10ms调用一次 则设置100.

	signed short  Scope_MAX;     	//斜率 无效范围 设置 ，MAX
	signed short  Scope_MIX;     	//斜率 无效范围 设置 , MIX

	
} Slope_Calculate_STATE_;

//========================================================================
// 函数:  
// 描述: 通用 斜率计算  初始化
// 参数: 
// 返回: 配置故障状态
// 版本: 
// 日期: 2025年6月7日
// 备注: 
//========================================================================  
unsigned char Coom_Slop_Calculate_Init(Slope_Calculate_STATE_ *SL_ARM, unsigned char SL_Count);

//========================================================================
// 函数:  
// 描述: 通用斜率计算
// 参数: 
// 返回: -
// 版本: 
// 日期: 2025年6月6日
// 备注: 
//========================================================================  
signed short General_Slop_Calculate(unsigned short In_Temp ,unsigned char Idx);






//======================================================================================================================================================================================================
// 
// 			
// 
//									↓↓↓↓↓↓↓				高温提示 
// 
// 
// 
//======================================================================================================================================================================================================
typedef struct _Temp_HighTips_STATE 
{
	unsigned char 	HOT_Tips;		//高温 提示
	unsigned short 	HOT_Delay;	//高温 提示 延迟熄灭
	
	
	unsigned short Fictitious_Timer;	//假显示 	时间
	unsigned short Fictitious_MIX;		//生效时间 	限制时间 （对应功能为 “Max_Fictitious”）
	
	
	unsigned char  Bit_1S;   		  // 一秒时间标志
} Temp_HighTips_STATE_;
 


typedef struct _Temp_HighTips_RAM		//NTC 配置
{
    unsigned char  	Mode;     			//高温提示 模式 ==0 依赖温度控制 ，==1 假显示，依靠时间 预估 
    unsigned short 	ON_Temp;			//高温提示 开始 >此温度 提示高温
	unsigned short 	OFF_Temp;			//高温提示 消失 <此温度 不提示高温 
//	unsigned short  _;					//高温 提示最长 时间是 <此温度时有效（相当于 第二层保险，例如有些人放了一锅油 高温肯定不能在限制的时间内熄灭。 所以要小于某个问题 才能触发！！
//	unsigned short 	Tips_Tiemp_MAX;		//高温 提示最长 时间是 多久？ 防止 一直显示不熄灭
    unsigned short 	Tips_OFF_Delay;		//（该配置仅模式0有！！）高温 消失后 再延迟多久？ 消除传感器偏差 保证一定不烫了  
	
	
	unsigned short 	Tips_Fictitious;	//假显示 最大显示 时间
 	unsigned char 	UP_Fictitious;		//每工作 一秒 假显示时间 增加多少秒?
 	unsigned short 	Max_Fictitious;		//假显示工作多少秒后开始计算？ （防止显得太假，一加热就显示高温） 
	

} Temp_HighTips_RAM_;

extern Temp_HighTips_RAM_       Temp_High_ARM;		//高温提示 配置
//========================================================================
// 函数:  
// 描述: 获取高温提示状态
// 参数: 
// 返回: 0 || 0xFF 
// 版本: VER1.0
// 日期: 2025年7月14日
// 备注: 
//========================================================================
unsigned char  Get_Temp_High_Tips_STA(unsigned char Idx);
//========================================================================
// 函数:  
// 描述: 高温提示功能 || 假高温提示功能 
// 参数: 监测开关、AD||转换温度、监测序号
// 返回: 故障代码
// 版本: VER2.0
// 日期: 2025年7月12日
// 备注: 
//========================================================================
Transition_Func_Lib_EER_CODE   Temp_High_Tips_Check(unsigned char ONOFF,  unsigned short IN_Temp_AD,unsigned char IDx);
//========================================================================
// 函数:  
// 描述: 高温提示功能 || 假高温提示功能
// 参数: User_Buffer 指向用户配置的 内存数量 High_Count 设置 检测数量
// 返回: 故障代码 DF_TF_NoErr; // 成功  DF_TF_Func_Err; // 无效参数
// 版本: VER1.0
// 日期: 2025年7月12日
// 备注: 
//========================================================================
Transition_Func_Lib_EER_CODE   Temp_HighTips_Init(Temp_HighTips_STATE_ *User_Buffer, unsigned char High_Count);
#endif
































