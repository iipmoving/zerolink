/********************************************************************************
    FileName    :  s_time_base.c
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  基本时间函数
                可以产生100ms，500ms，1s, 1min的时间标志，可以记录系统时间

    Date        :  2018-09-06
    Modify      :
                   2018-09-06 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/


/********************************************Head Files*/



#include	"s_pid.h"

// PID结构体，存储PID参数和状态  

/*********************************************局部变量申请*/

/*********************************************函数列表*/





  
// 定义定点数类型，这里使用32位整数，并假设小数点后有16位（即精度为1/65536）  



//int main() {  
//    // 初始化定点数PID控制器参数  
//    FixedPIDController pid;  
//    FixedPID_Init(&pid, 10, 0.1, 0.01, 65536); // 假设我们使用1/65536的比例因子来表示小数  
//  
//    // 设定值和测量值（都转换为定点数）  
//    fixed_point_t setpoint = 100 * 65536; // 设定值为100（定点数表示）  
//    fixed_point_t measured_value = 90 * 65536; // 初始测量值为90（定点数表示）  
//  
//    // 控制周期（这里假设为1个单位时间，实际应用中应该根据采样时间进行缩放）  
//    uint32_t dt = 1;  
//  
//    // 模拟控制循环  
//    for (int i = 0; i < 100; i++) {  
//        fixed_point_t delta_output = FixedPID_Compute(&pid, setpoint, measured_value, dt);  
//  
//        // 假设系统输出是上一次输出加上PID增量输出（这里需要系统模型，但为简化起见，直接相加）  
//        // 注意：这里的相加操作可能需要根据实际系统的输出范围进行限制或缩放  
//        fixed_point_t output = measured_value + delta_output; // 示例：直接相加作为输出（可能不合适，仅用于说明）  
//  
//        // 更新测量值（这里为了简单起见，假设测量值逐步接近设定值，实际应用中应通过传感器或系统模型获取）  
//        measured_value += (delta_output / 10); // 示例：逐步接近设定值（这里的除以10是为了模拟一个较慢的接近过程）  
//  
//        // 打印输出（为了说明，这里将定点数转换回浮点数进行打印，实际应用中应直接处理定点数）  
//        printf("Time: %d, Measured Value: %.2f, PID Output Increment: %.2f, Output: %.2f\n",  
//               i, (double)measured_value / pid->factor / 100.0, (double)delta_output / pid->factor, (double)output / pid->factor / 100.0);  
//    }  
//  
//    return 0;  
//} 






/********************************************************************************
*name       : void PID_Init(PIDController *pid, double kp, double ki, double kd)
*author     : rsl
*function   : 初始化PID
*para       : 比例系数采用1000的倍数， 0.1==100/1000，
*return     : NULL
*brief      :
********************************************************************************/
// 初始化PID控制器  
  
// 初始化定点数PID控制器  
void FixedPID_Init(FixedPIDController *pid, double kp, double ki, double kd, fixed_point_t factor) {  
    pid->kp = (fixed_point_t)(kp * factor);  
    pid->ki = (fixed_point_t)(ki * factor);  
    pid->kd = (fixed_point_t)(kd * factor);  
    pid->prev_error = 0;  
    pid->integral = 0;  
    pid->factor = factor;  
	pid->output=0;		
}  

/*
清除PID积分的影响
*/
void	 FixedPIDclearIntegral(FixedPIDController *pid)		
{
    pid->prev_error = 0;  
    pid->integral = 0;  
	pid->output=0;		
}	


/********************************************************************************
*name         : double PID_Compute(PIDController *pid, double setpoint, double measured_value, double dt) 
*author       : rsl
*function     : 增量式PID处理
*para         : NULL
*return       : 0
*brief        : 时间标志在每个初始时间片运行一次，10ms
********************************************************************************/
#include	<stdlib.h>
// 计算增量式PID输出  
// 计算定点数PID输出  
fixed_point_t FixedPID_Compute_old(FixedPIDController *pid, fixed_point_t setpoint, fixed_point_t measured_value, uint32_t dt) {  
    // 将定点数转换为对应的浮点数表示（为了说明，这里实际上没有真正进行浮点运算）  
    // 在实际应用中，应该直接使用定点数进行计算  
    // 但为了简化说明，这里假设有一个函数可以将定点数转换为浮点数（实际上并不存在这样的标准函数）  
    // double f_setpoint = (double)setpoint / pid->factor;  
    // double f_measured_value = (double)measured_value / pid->factor;  
      
    // 由于我们直接使用定点数进行计算，所以不需要上面的转换  
    fixed_point_t	error  = setpoint - measured_value;  
		uint16_t div;
		div=abs(error)/dt;				//得到功率明码值（25W)
	
		if(div<50/25)				//50W以内为死区 主要以微分项
		{	
			pid->kp=0;
			pid->ki=0;
			pid->kd=50;
			pid->integral=0;
		}
		else if(div<300/25)				//200w以内
		{
			pid->kp=5;						
			pid->ki=5;
			pid->kd=30;
//			pid->integral=0;		
	
		}
		else
		{
			pid->kp=10;
			pid->ki=5;
			pid->kd=0;	

		}	
		


			pid->integral += error ; // 注意：这里dt应该是以某种单位时间表示的定点数  积分
	
			if(pid->integral>5000)
			{
				pid->integral=5000;
			}		
			if(pid->integral<-5000)
			{
				pid->integral=-5000;
			}			
		
		
    // 为了简化，这里假设dt为1（即每个控制周期都视为1个单位时间）  
    // 在实际应用中，dt应该是根据实际的采样时间进行缩放的定点数  
    fixed_point_t derivative = (error - pid->prev_error); // 微分项（这里省略了除以dt，因为dt为1） //这里代表上一次输入值变化了多少输出量 
	
	
    // 计算PID输出增量  
    // fixed_point_t delta_output = (pid->kp * error) - (0 * pid->prev_error) + (pid->ki * pid->integral) + (pid->kd * derivative);  
    fixed_point_t delta_output = (pid->kp * error) - (pid->kp * pid->prev_error) + (pid->ki * pid->integral) + (pid->kd * derivative);  

	// 注意：上面的计算中，由于kp在乘以error和prev_error时都出现了，所以可以进行合并优化  
    // 但为了保持与浮点数PID算法的直观对应，这里保留了原始形式  
  
//		delta_output=ME_SDIV(delta_output,pid->factor);
			
			
		delta_output/=pid->factor;
		if(delta_output>100)
		{
				delta_output=100;
		}
		else if(delta_output<-100)
		{
				delta_output=-100;
		}		
			
    // 更新上一次误差  
    pid->prev_error = error;  
	pid->output=delta_output;
    // 返回PID输出增量（注意：这个增量可能需要根据实际系统进行缩放或限制）  
		

#ifdef	DebugOutPc	

#include	"simulative_uart.h"
#include	"drv_mcu.h"
	{	
		setDebugOutBuff(setpoint*25/10/dt,0);
		setDebugOutBuff(measured_value*25/10/dt,1);
		setDebugOutBuff((pid->integral),	2);
		setDebugOutBuff(delta_output,3);
//		setDebugOutBuff(i_ppg_getppg()/50,4);
		UARTx_SendValueClass(Debug_OutToPc());
	}
	
#endif	
		
    return delta_output;  
}  
 
//增量式PID

fixed_point_t FixedPID_Compute(FixedPIDController *pid, fixed_point_t setpoint, fixed_point_t measured_value, uint32_t dt) {  
    // 将定点数转换为对应的浮点数表示（为了说明，这里实际上没有真正进行浮点运算）  
    // 在实际应用中，应该直接使用定点数进行计算  
    // 但为了简化说明，这里假设有一个函数可以将定点数转换为浮点数（实际上并不存在这样的标准函数）  
    // double f_setpoint = (double)setpoint / pid->factor;  
    // double f_measured_value = (double)measured_value / pid->factor;  
      
    // 由于我们直接使用定点数进行计算，所以不需要上面的转换  
    fixed_point_t error  = setpoint - measured_value;  
		uint16_t div;
		div=abs(error)/dt;				//得到功率明码值（25W)
	
		if(div<50/25)				//50W以内为死区 主要以微分项
		{	
			pid->kp=5;
			pid->ki=5;
			pid->kd=10;
			// pid->integral=0;
		}
		else if(div<300/25&&measured_value>500/25)				//200w以内
		{
			pid->kp=5;						
			pid->ki=5;
			pid->kd=5;
//			pid->integral=0;		
	
		}
		else 
		{
			pid->kp=5;
			pid->ki=15;
			pid->kd=0;	

		}	
		


		pid->integral = error ; // 注意：这里dt应该是以某种单位时间表示的定点数  积分

		
    // 为了简化，这里假设dt为1（即每个控制周期都视为1个单位时间）  
    // 在实际应用中，dt应该是根据实际的采样时间进行缩放的定点数  
	//	derivative=	(error-2*prev_error+old_error)
    fixed_point_t derivative = error; // 微分项（这里省略了除以dt，因为dt为1） //这里代表上一次输入值变化了多少输出量 
					derivative	+=pid->old_error;
					derivative-= pid->prev_error*2;
	
    // 计算PID输出增量  
    // Kp*(error-prev_error)+Ki*error+Kd*(error-2*prev_error+old_error);  
    fixed_point_t delta_output = (pid->kp * error) - (pid->kp * pid->prev_error) + (pid->ki * pid->integral) + (pid->kd * derivative);  

	// 注意：上面的计算中，由于kp在乘以error和prev_error时都出现了，所以可以进行合并优化  
    // 但为了保持与浮点数PID算法的直观对应，这里保留了原始形式  
  
//		delta_output=ME_SDIV(delta_output,pid->factor);
			
			
		delta_output/=pid->factor;
		if(delta_output>100)
		{
				delta_output=100;
		}
		else if(delta_output<-100)
		{
				delta_output=-100;
		}		
			
    // 更新上一次误差  
	pid->old_error=pid->prev_error;
    pid->prev_error = error;  
	pid->output=delta_output;
    // 返回PID输出增量（注意：这个增量可能需要根据实际系统进行缩放或限制）  
		

#ifdef	DebugOutPc	

#include	"simulative_uart.h"
#include	"drv_mcu.h"
	{	
		setDebugOutBuff(setpoint*25/10/dt,0);
		setDebugOutBuff(measured_value*25/10/dt,1);
		setDebugOutBuff((pid->integral),	2);
		setDebugOutBuff(delta_output,3);
//		setDebugOutBuff(i_ppg_getppg()/50,4);
		UARTx_SendValueClass(Debug_OutToPc());
	}
	
#endif	
		
    return delta_output;  
}  
  


//**********************************end of file********************************

