// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
#include "../../include_io/calculator_io.h"

MODULE_SKELETON(Calculator);


/* ---- 管道 SEQ 有效性跟踪 (替代 ST_NEW) ---- */
static uint8_t s_last_seq_AppAdc;  /* AppAdc → Calculator */

/* ---- 数据实体（模块私有）---- */
static MODULE_INPUT(Calculator)*   s_inPara;    // 输入参数实体在ADC， 这里只调用不修改
static MODULE_OUTPUT(Calculator)  s_outPara;   // 输出参数缓冲区

/* ---- 内部 OUTPUT_LINK 实例 (指针直穿目标) ---- */
static MODULE_OUTPUT_LINK(Calculator, ElecParams)  s_calcToElecLink;
static MODULE_OUTPUT_LINK(Calculator, AppAdc)      s_calcToAppAdcLink;

static void user_Process(MODULE_INPUT(Calculator) *in, MODULE_OUTPUT(Calculator) *out);

static void ProcessInput(void)
{
    MODULE_INPUT(Calculator) *in  = (MODULE_INPUT(Calculator)*)g_input.para;
    MODULE_OUTPUT(Calculator) *out = (MODULE_OUTPUT(Calculator)*)g_output.para;

    /* === 用户业务 (内部自行做 seq 有效性检测) === */

    if (in->AppAdc_params->seq == s_last_seq_AppAdc)
    { 
       return;
    }
    s_last_seq_AppAdc = in->AppAdc_params->seq; 

    user_Process(in, out);
}

MODULE_EXPORT(Calculator);

// ===== [END AI GENERATED] =====

/**
 * @file    calculator.c
 * @brief   Power Calculator Module (v2.2 PULL Architecture)
 * @layer   base_class
 */

//#include "../include/calculator_io.h"

#include <string.h>
#include <math.h>

#define	CALC_POTMAX 4

/* ---- 硬件标定常数 ---- */
#define V_SCALE         0.10606f    // V/count  (Vref/4096 / 分压比)
#define I_SCALE         0.02523f    // A/count  (Vref/4096 / I分压比)
#define VDC_SCALE       0.10606f    // V/count  (同 V 分压网络)
#define HRTIM_CLK_HZ    144000000   // HRTIM 时钟 144MHz
#define FRE_PER_ADC     384         // 频率每ADC
#define PHASE_DEG_BASE  1800        // 180.0 度定标

/* ---- 状态枚举 ---- */
typedef enum {
    pluseStart = 0,
    pluseHighOff,
    pluseLowOn
} PluseState_t;

/* ---- 中间结果 ---- */
typedef struct {
    uint32_t voltage;
    uint32_t current;
    uint16_t peak_current;
    uint16_t peak_num;
    uint16_t voltage_count;
} CalculatorResultDef;

/* ---- Calculator → PowerBase 直接参数累加器 ---- */
typedef struct {
    uint32_t resonant_current;   // peak_current 累加和
    uint32_t voltage;            // voltage_sum 累加和
    int32_t  phase;              // 相位角累加和 (0.01° × count)
} PowerDirectAcc_t;
//static PowerDirectAcc_t s_power_acc[CALC_POTMAX];



/* ---- 内部函数声明 ---- */
static uint16_t FindZeroCrossing(const uint16_t* current, uint16_t start, uint16_t end);
static CalculatorResultDef CalculateAuctalCurrent(uint16_t* resonant_current,
    uint16_t* hrtim_values, uint16_t* voltage_data, uint16_t start,
    Calculator_InputParams_t* input);
// static void CalculatePhaseAngle(Calculator_OutHead_t* head_out, uint16_t* hrtim_values,
    // uint16_t phaseUp, uint16_t phaseDown, Calculator_InputParams_t* input);
// static void FillHRTIMStatus(Calculator_OutHead_t* head_out, Calculator_InputParams_t* input);
static void ProcessAllHead(MODULE_INPUT(Calculator)* head_in, MODULE_OUTPUT(Calculator)* head_out);
static void CalculatePower(uint16_t* resonant_current,uint16_t* hrtim_values,uint16_t* voltage_data,Calculator_InputParams_t* input,MODULE_OUTPUT_PARAMS(Calculator, ElecParams) *outPut);
/* ---- 处理逻辑入口（每帧被调）---- */
static void user_Process(MODULE_INPUT(Calculator) *in, MODULE_OUTPUT(Calculator) *out)
{
//    MODULE_INPUT(Calculator)*   pIn = g_input.para;
//    MODULE_OUTPUT(Calculator)*  pOut = g_output.para;



                if(in->AppAdc_params->count)
				{	
					ProcessAllHead(in, out);			//对每1ms数据进行计算


				}
				else
				{
						// 完成 Calculator→PowerBase 直接参数平均值 // 每20ms计算一次
					
					// if((out->AppAdc_params->status&ST_NEW)==0)			//以经执行就不再执行这个时间片，等ADC把数据取走
					{		
						

			//ELEC 模块数据有效，这里可能不需要了
					    out->ElecParams_params->seq++;  //elec 模块数据有效
						out->Telemetry_params=out->ElecParams_params;	//数据指向向ELEC输出的缓存
						
			//power 模块数据有效，这里可能不需要了
                         out->AppAdc_params->seq++;      // 输出有效
				
					}
				}
				

}

/* ---- 处理单个通道数据 ---- */
static void ProcessAllHead(MODULE_INPUT(Calculator)* head_in, MODULE_OUTPUT(Calculator)* head_out)
{
    uint8_t cycle_idx = head_in->AppAdc_params->count;  // 当前周期索引 1..19

    /* 输入有效性检查 */
    // if (!head_in->resonant_current || 
    //     !head_in->hrtim_values || 
    //     !head_in->voltage_data ||
    //     head_in->params.end == 0 || 
    //     head_in->params.highOff == 0) {
    //     memset(head_out, 0, sizeof(Calculator_OutHead_t));
    //     return;
    // }

		uint16_t* voltage_point = (uint16_t*)(head_in->AppAdc_params->params->voltage_data)[0];
		voltage_point = (uint16_t*)(head_in->AppAdc_params->params->voltage_data)[1];
	
	

   for (int i = 0; i < CALC_POTMAX; i++) { 
			
	      uint16_t* voltage_point = (uint16_t*)(head_in->AppAdc_params->params->voltage_data)[i];
        uint16_t* hrtim_point = (uint16_t*)(head_in->AppAdc_params->params->hrtim_values)[i];
        uint16_t* current_point = (uint16_t*)head_in->AppAdc_params->params->resonant_current[i];

			Calculator_InputParams_t* params_point =(head_in->AppAdc_params->params->input); //这里存的是缓存的地址（4个）
			params_point+=i;
		 MODULE_OUTPUT_PARAMS(Calculator, ElecParams) *elec_params = &s_outPara.ElecParams_params->params[i][cycle_idx];

		CalculatePower(current_point,hrtim_point,voltage_point,params_point,elec_params);

		// 累加 Calculator → PowerBase 直接参数
		if (elec_params->peak_current > 0) {

			
			//-----------------------------------------------------------				


		uint64_t		sumCurrent=elec_params->active_current_sum_high+elec_params->active_current_sum_low;
				sumCurrent/=elec_params->hrtim_lowOff;      //电流值

//瞬时电压			
			
				
			
				head_out->AppAdc_params->params[i][cycle_idx].voltage=elec_params->voltage_sum/elec_params->voltage_count;
				
        head_out->AppAdc_params->params[i][cycle_idx].resonant_current=sumCurrent;

				

            uint32_t angle=elec_params->zero_cross_high-elec_params->hrtim_highOn;
						

					
               angle*=PHASE_DEG_BASE;		//相位角（180度为单位）
               angle/=elec_params->hrtim_highOff-elec_params->hrtim_highOn;
             head_out->AppAdc_params->params[i][cycle_idx].phase_angle=angle;


				}	



    }
}
//----------------------------------------------------------------			

			
			

static void  CalculatePower(uint16_t* resonant_current,uint16_t* hrtim_values,uint16_t* voltage_data,Calculator_InputParams_t* input,MODULE_OUTPUT_PARAMS(Calculator, ElecParams) *outPut)
{

    uint16_t hrtim_per_adc = input->perAdc;
    uint16_t potNumStart = input->start + 4;
    uint16_t zeroStart, zeroEnd;
    uint16_t highOffNum = input->highOff / hrtim_per_adc;
    highOffNum += potNumStart;

    /* Step 1: 寻找过零点 */
    zeroStart = potNumStart;
    zeroEnd = highOffNum + 2;
    uint16_t phaseUp = FindZeroCrossing(resonant_current, zeroStart, zeroEnd);

    zeroStart = highOffNum;
    zeroEnd = input->end;
    uint16_t phaseDown = FindZeroCrossing(resonant_current, zeroStart, zeroEnd);

    /* Step 2: 过零点确认后计算有效功率 */
    if (phaseUp && phaseDown) {
        CalculatorResultDef sumUp = CalculateAuctalCurrent(
            resonant_current, hrtim_values, voltage_data, phaseUp, input);

        if (sumUp.current == 0) {
            return  ;
        }

        uint32_t sumHigh, sumLow;

        /* 判断是否对称输出 */
        if ((input->highOff * 2 + 10) < input->lowOff) {
            /* 非对称输出 */
            CalculatorResultDef sumDown = CalculateAuctalCurrent(
                resonant_current, hrtim_values, voltage_data, phaseDown, input);
            sumHigh = sumUp.current;
            sumLow  = sumDown.current;
        } else {
            /* 对称输出 */
            sumHigh = sumUp.current;
            sumLow  = sumUp.current;
        }


	
        /* 填充基本结果 — 原始积分值，ElecParams 浮点除 */
        outPut->active_current_sum_high = sumHigh;
        outPut->active_current_sum_low  = sumLow;
        outPut->voltage_sum = sumUp.voltage;
        outPut->voltage_count = sumUp.voltage_count;
        outPut->peak_current = sumUp.peak_current;
        outPut->peak_point = hrtim_values[sumUp.peak_num];

        /* Step 3: 计算相位差 */
        outPut->zero_cross_high=hrtim_values[phaseUp]; 
        outPut->zero_cross_low=hrtim_values[phaseDown]; 

         // /* Step 4: 填充 HRTIM 状态 */
        outPut->hrtim_highOff=input->highOff;
        outPut->hrtim_lowOff=input->lowOff;
        outPut->hrtim_highOn=input->highOn;
        outPut->hrtim_lowOn=input->lowOn;





    }

}
#if 0
/* ---- 计算相位角 ---- */
static void CalculatePhaseAngle(Calculator_OutHead_t* head_out, uint16_t* hrtim_values,
    uint16_t phaseUp, uint16_t phaseDown, Calculator_InputParams_t* input)
{
    if (hrtim_values[phaseUp] > input->highOn) {
        uint32_t angle = hrtim_values[phaseUp] - input->highOn;
        head_out->zero_cross_high = hrtim_values[phaseUp];
        angle *= PHASE_DEG_BASE;
        angle /= (input->highOff - input->highOn);
        head_out->phase_angleUp = angle;
    }

    if (hrtim_values[phaseDown] > input->lowOn) {
        uint32_t angle = hrtim_values[phaseDown] - input->lowOn;
        head_out->zero_cross_low = hrtim_values[phaseDown];
        angle *= 180;
        angle /= (input->lowOff - input->lowOn);
        head_out->phase_angleDown = angle;
    }
}

/* ---- 填充 HRTIM 状态 ---- */
static void FillHRTIMStatus(Calculator_OutHead_t* head_out, Calculator_InputParams_t* input)
{
    head_out->hrtim.highOn = input->highOn;
    head_out->hrtim.highOff = input->highOff;
    head_out->hrtim.lowOn = input->lowOn;
    head_out->hrtim.lowOff = input->lowOff;
}
#endif
/* ---- 辅助函数：找过零点 ---- */
static uint16_t FindZeroCrossing(const uint16_t* current, uint16_t start, uint16_t end)
{
    if (current == 0 || start > 0xff00) {
        return 0;
    }

    uint16_t candidate = 0;
    uint16_t direction = 0xffff;
    uint16_t preValue, newValue;
    preValue = current[start];
    uint32_t minZ = 0xffffffff;

    for (uint16_t i = start + 1; i < end; i++) {
        newValue = current[i];
        int16_t delta = newValue - preValue;

        direction <<= 1;
        if (delta > 0) {
            direction |= 0x1;
        } else {
            direction &= ~0x1;
        }

        preValue = newValue;

        /* 前两次方向，拐点位置 */
        if ((direction & 0x3) == 0x1) {
            uint16_t phasePoint = i - 1;  // 得到相位点

            /* 过零点前后二个点的积最小，为过零点 */
            uint32_t zeroRange = current[phasePoint - 1] + current[phasePoint + 1];

            if (minZ > zeroRange) {
                minZ = zeroRange;
                zeroRange >>= 1;  // 两个点的平均值

                /* 判断最低点值是否够小 */
                if (current[phasePoint] <= zeroRange) {
                    candidate = phasePoint;
                }
            }
        }
    }

    return candidate;
}

/* ---- 辅助函数：计算有效电流 ---- */
static CalculatorResultDef CalculateAuctalCurrent(uint16_t* resonant_current,
    uint16_t* hrtim_values, uint16_t* voltage_data, uint16_t start,
    Calculator_InputParams_t* input)
{
    uint16_t num = start;
    CalculatorResultDef result = {0};
    uint16_t hrtim_per_adc = input->perAdc;
    if (hrtim_per_adc == 0) hrtim_per_adc = 1;

    uint16_t end = input->highOff;
    uint16_t dead = input->lowOn;  // 死区时间
    uint16_t currentMax = 0;
    uint16_t currentMaxNum;
    uint16_t currentType = pluseStart;  // 当前统计区间
    uint32_t currentMaxSum;
    uint32_t currentSumPower;
    uint16_t vcCnt = 1;

    if (hrtim_values[start] > end) {
        end = input->lowOff;
        dead = input->highOn;
    }

    uint32_t currentSum = resonant_current[num];
    uint32_t vcSum = voltage_data[num];
    num++;

    do {
        uint16_t newValue = resonant_current[num];
        currentSum += newValue;

        /* 峰值检测 */
        if (currentMax < newValue) {
            int16_t d1, d2, d3, avg;
            d1 = newValue - resonant_current[num - 1];  /* 最后一步 (正数=上升) */
            d2 = resonant_current[num - 1] - resonant_current[num - 2];
            d3 = resonant_current[num - 2] - resonant_current[num - 3];

            if (d2 < 0) d2 = 0;
            if (d3 < 0) d3 = 0;

            avg = (d2 + d3) / 2;

            if (d1 > avg) {
                newValue = resonant_current[num - 1] + avg;  /* 限幅到平均值 */
            }

            currentMax = newValue;
            currentMaxSum = currentSum;
            currentMaxNum = num;
        }

        uint16_t hrtim1us = hrtim_values[num];
        int32_t lastPoint;

        switch (currentType) {
            case pluseStart:
                vcSum += voltage_data[num];  // 统计电压平均值
                vcCnt++;
                lastPoint = (int32_t)end - hrtim1us;

                if (lastPoint < hrtim_per_adc) {  // 关断点
                    if (1) {  // 最大值在关断点前出现，以关断点为结束
                        /* 关断点插值修正 */
                        if (lastPoint > 0) {
                            lastPoint *= newValue;  // 最后一个点不完整，加上修正值
                            currentSumPower = currentSum;
                            currentSumPower *= hrtim_per_adc;
                            currentSumPower += lastPoint;
                            currentType = pluseLowOn;
                        }
                    } else {  // 最大值在关断点前没出现，继续统计到对管开通（死区计入电流积分）
                        currentType = pluseHighOff;
                    }
                }
                break;

            case pluseHighOff:  // 开启点，用于等谐振电流最大值
                if ((dead - hrtim1us) < hrtim_per_adc) {
                    currentSumPower = currentMaxSum;
                    currentSumPower *= hrtim_per_adc;
                    currentType = pluseLowOn;  // 结束循环
                }
                break;
        }

        num++;
        if (num > input->end) {
            return result;
        }

    } while (currentType < pluseLowOn);

    result.current = currentSumPower;
    result.voltage = vcSum;             // 原始累加，ElecParams 浮点除
    result.peak_current = currentMax;
    result.peak_num = currentMaxNum;
    result.voltage_count = vcCnt;

    return result;
}

/* ---- 初始化 ---- */
static void Init(void)
{
//    memset(&s_inPara,  0, sizeof(s_inPara));
		s_inPara=NULL;
    memset(&s_outPara, 0, sizeof(s_outPara));

	/* 绑定 OUTPUT_LINK 指钺 */
	s_outPara.ElecParams_params = &s_calcToElecLink;
	s_outPara.AppAdc_params     = &s_calcToAppAdcLink;

		s_outPara.ElecParams_params->res[0]=3;				//CONSET_OUT: CALC_TO_ELEC
		s_outPara.AppAdc_params->res[0]=2;							//CONSET_OUT :CALC_TO_APPPOWER
	
		
    g_input.para   = &s_inPara;		//在输入回调里初始化
		g_input.info.status =0;
    g_output.para  = &s_outPara;

}

/* ---- 导出 ---- */

