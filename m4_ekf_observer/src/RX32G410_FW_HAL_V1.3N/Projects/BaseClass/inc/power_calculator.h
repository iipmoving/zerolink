#ifndef POWER_CALCULATOR_H
#define POWER_CALCULATOR_H

#include <stdint.h>
#include <stdbool.h>

// 工作周期定义
typedef struct {
    uint16_t start;         //HRTIM开始点
    uint16_t end;            //HRTIM结束点   

	
    uint16_t highOn;		    //死区后高端开通HRTIM值
		uint16_t highOff;			//高端关闭，也就是DUTY PPG占空比 
	
    uint16_t lowOn;		        //死区后低端开通
		uint16_t lowOff;			//低端关闭，也就是prioed	PPG周期
   
    uint16_t zero_cross_high; // 高端过零点索引
    uint16_t zero_cross_low;  // 低端过零点索引
	
		uint16_t perAdc;				//每个ADC对应的HRTIM值
		uint16_t lagDuty;  // 婊炲悗鑷傚崰绌烘瘮CMP鍊硷紙鍏ㄦˉ锛�
}PowerCalculatorInputDef;	        //谐振电流计算输入参数

// 峰值电流信息
typedef struct {
    uint16_t index;   // 峰值索引
    uint16_t value;   // 峰值电流值
}PeakInfo;


typedef struct {
    uint32_t voltage;   // 峰值索引
    uint32_t current;   // 峰值电流值
}CalculatorResultDef;



// 功率计算结果
typedef struct {
    int32_t active_power;     // 有功功率
    int32_t active_current;   // 无功功率

		uint16_t peak_current;    	// 峰值电流
    uint16_t zero_current;    	// 过零点电流值   
	
		int16_t phase_angleUp;      // 相位角（度*100）
    int16_t phase_angleDown;

    uint16_t zero_cross_high; // 高端过零点索引
    uint16_t zero_cross_low;  // 低端过零点索引

    uint16_t esr;               //等效电阻
    uint16_t voltage; 

}PowerResult;

/**
 * @brief 计算功率及相关参数
 * @param resonant_current 谐振电流数组
 * @param hrtim_values HRTIM时间戳数组
 * @param voltage_data 电压数据（可选）
 * @param count 数据点数量
 * @param ppg 工作周期定义
 * @return PowerResult 计算结果结构体
 */
PowerResult CalculatePower(
    uint16_t* resonant_current,
    uint16_t* hrtim_values,
    uint16_t* voltage_data,
    PowerCalculatorInputDef* input
);
int16_t * Power_Calculator_GetVoltageBuffAddress(uint8_t ch);		//得到电压调试数据
int16_t * Power_Calculator_GetHrtimBuffAddress(uint8_t ch);       //得到调试HRTIM地址
int16_t * Power_Calculator_GetTxaBuffAddress(uint8_t ch);       //得到调试数据地址
int16_t Power_Calculator_GetTxaBuffSize(uint8_t ch);			//得到调试数据大小
PowerCalculatorInputDef*	Power_Calculator_GetInputArrayAddress(uint8_t ch);


#define     ZERO_WINDOW_MIN     8           //过零最小检测窗口，+- 2* ZERO_WINDOW_MIN+1 个点

#define testCh	0
#define		PHASE_DEG_BASE		1800   //180.0搴�
#endif // POWER_CALCULATOR_H


