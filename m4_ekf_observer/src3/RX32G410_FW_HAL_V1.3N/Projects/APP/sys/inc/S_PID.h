/********************************************************************************
    FileName    :  sys_PID.h
    Author      :  rsl
    Version     :  V1.0.1
    Brief       :  增量式PID算法，用于功率的快速逼近

    Date        :  2018-09-06
    Modify      :
                   2018-09-06 创建

    Copyright (c)    Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/
#ifndef SYS_PID_H
#define SYS_PID_H


#include	<stdint.h>
//常量定义
#define SYS_TSK_NUM 16   //任务数量

//结构体定义
typedef  signed	int fixed_point_t;  
  
// PID结构体，存储定点数PID参数和状态  
typedef struct {  
    fixed_point_t kp;       // 比例系数（定点数）  
    fixed_point_t ki;       // 积分系数（定点数）  
    fixed_point_t kd;       // 微分系数（定点数）  
    fixed_point_t prev_error;   // 上一次误差（定点数） 
    fixed_point_t old_error;    // 上两次误差（定点数） 
     
    fixed_point_t integral; // 积分项（定点数）  
    fixed_point_t factor;   // 定点数比例因子（用于转换浮点数到定点数）  
	fixed_point_t output;	//上一次的输出值 
} FixedPIDController;  


//函数定义
void FixedPID_Init(FixedPIDController *pid, double kp, double ki, double kd, fixed_point_t factor);
fixed_point_t FixedPID_Compute(FixedPIDController *pid, \
																fixed_point_t setpoint, \
																fixed_point_t measured_value, \
																unsigned int dt);  

void	 FixedPIDclearIntegral(FixedPIDController *pid)	;	

#endif

//**********************************end of file********************************

