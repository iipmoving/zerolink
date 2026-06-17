/**
  ******************************************************************************
  * @file    commclass.h
  * @author  
  * @version V1.1
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2022 Shanghai Macrogiga Electronics</center></h2>
  *
  ******************************************************************************
*/



#ifndef _PLUSE_H
#define _PLUSE_H


// #include	"data_type.h"
// #include	<stdarg.h>
// #include 	<stdlib.h>
// #include 	<string.h>
// ====================== 配置参数 ======================
#define WINDOW_SIZE       30      // 滑动窗口大小
#define PEAK_QUEUE_SIZE    20      // 峰值队列大小

#define BASELINE_SHIFT     8      // 定点数精度 (1<<8 = 256级)
#define ALPHA              251    // 平滑系数α=0.98 (251/256)
#define INVERSE_ALPHA      5      // (1-α)=0.02 (5/256)
#define BELOW_THRESHOLD    5      // 连续低于基线次数阈值
#define PEAK_THRESHOLD     20   // 峰值相对阈值 (baseline * 1.25)
#define MAX_SAMPLE_JUMP    300   // 最大允许采样跳变值

// =====================================================
uint8_t    pulse_check_process(uint16_t* buff,uint16_t size);//返回检锅数
void    pulse_check(void);

int16_t * pluse_getIntervalsAddress(uint8_t ch);		//测试用数据首址
uint16_t 	pluse_getIntervalsSize(uint8_t ch);				//测试用数据大小
#endif





