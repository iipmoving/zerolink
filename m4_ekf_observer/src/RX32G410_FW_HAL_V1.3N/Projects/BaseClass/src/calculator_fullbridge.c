/**
 * @file    calculator_fullbridge.c
 * @brief   Full-Bridge Power Calculator Module (v2.2 PULL Architecture)
 * @layer   base_class
 */
#include "calculator_fullbridge.h"
#include <string.h>

/* ---- 硬件标定常数 ---- */
#define V_SCALE         0.10606f    // V/count  (Vref/4096 / 分压比)
#define I_SCALE         0.02523f    // A/count  (Vref/4096 / I分压比)
#define FRE_PER_ADC     384         // 频率每ADC
#define PHASE_DEG_BASE  1800        // 180.0 度定标

/* ---- 中间结果 ---- */
typedef struct {
    uint32_t voltage;
    uint32_t current;
    uint16_t peak_current;
    uint16_t peak_num;
} FB_CalculatorResultDef;

/* ---- 窗口判断状态机定义 ---- */
enum {
    WINDOW_START = 0,
    WINDOW_ACTIVE,
    WINDOW_EXIT,
};

/* ---- 数据实体（模块私有）---- */
static FB_Calculator_Input_t   s_inPara;    // 输入参数缓冲区
static FB_Calculator_Output_t  s_outPara;   // 输出参数缓冲区

/* ---- v2.2 框架层骨架（使用标准宏）---- */
MODULE_SKELETON(FB_Calculator);

/* ---- 内部函数声明 ---- */
static uint16_t FindZeroCrossing(const uint16_t* current, uint16_t start, uint16_t end);
static FB_CalculatorResultDef FB_CalculateAuctalCurrent(uint16_t* resonant_current,
    uint16_t* hrtim_lead, uint16_t* hrtim_lag, uint16_t* voltage_data,
    uint16_t start, FB_Calculator_InputParams_t* input);
static void CalculatePhaseAngle(FB_Calculator_OutHead_t* head_out, uint16_t* hrtim_lead,
    uint16_t phaseUp, uint16_t phaseDown, FB_Calculator_InputParams_t* input);
static void FillHRTIMStatus(FB_Calculator_OutHead_t* head_out, FB_Calculator_InputParams_t* input);
static void ProcessSingleHead(FB_Calculator_InHead_t* head_in, FB_Calculator_OutHead_t* head_out);

/* ---- 处理逻辑入口（每帧被调）---- */
static void ProcessInput(void)
{
    if (g_input.info.status & ST_NEW) {
        FB_Calculator_Input_t* in = (FB_Calculator_Input_t*)g_input.para;
        FB_Calculator_Output_t* out = (FB_Calculator_Output_t*)g_output.para;

        for (uint8_t h = 0; h < FB_CALC_POTMAX; h++) {
            ProcessSingleHead(&in->head[h], &out->head[h]);
        }

        g_input.info.status &= ~ST_NEW;
        g_output.info.status |= ST_NEW;
    }
}

/* ---- 处理单个通道数据 ---- */
static void ProcessSingleHead(FB_Calculator_InHead_t* head_in, FB_Calculator_OutHead_t* head_out)
{
    /* 输入有效性检查 */
    if (!head_in->resonant_current || 
        !head_in->hrtim_lead || 
        !head_in->hrtim_lag ||
        !head_in->voltage_data ||
        head_in->params.end == 0 || 
        head_in->params.highOff == 0) {
        memset(head_out, 0, sizeof(FB_Calculator_OutHead_t));
        return;
    }

    /* 初始化输出 */
    memset(head_out, 0, sizeof(FB_Calculator_OutHead_t));

    uint16_t* resonant_current = head_in->resonant_current;
    uint16_t* hrtim_lead = head_in->hrtim_lead;
    uint16_t* hrtim_lag = head_in->hrtim_lag;
    uint16_t* voltage_data = head_in->voltage_data;
    FB_Calculator_InputParams_t* input = &head_in->params;

    uint16_t hrtim_per_adc = input->perAdc;
    if (hrtim_per_adc == 0) hrtim_per_adc = 1;

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
        // 统计正向窗口(Q1+Q4)有效电流
        FB_CalculatorResultDef sumUp = FB_CalculateAuctalCurrent(
            resonant_current, hrtim_lead, hrtim_lag,
            voltage_data, phaseUp, input);

        if (sumUp.current == 0) {
            return;
        }

        uint64_t sumAll;
        uint32_t sumCurrent;

        if ((input->highOff * 2 + 10) < input->lowOff) {
            // 非对称输出 → 统计反向窗口(Q2+Q3)
            FB_CalculatorResultDef sumDown = FB_CalculateAuctalCurrent(
                resonant_current, hrtim_lead, hrtim_lag,
                voltage_data, phaseDown, input);
            sumAll = (uint64_t)sumUp.current * sumUp.voltage + 
                     (uint64_t)sumDown.current * sumDown.voltage;
            sumCurrent = sumUp.current + sumDown.current;
        } else {
            // 对称输出
            sumAll = (uint64_t)sumUp.current * sumUp.voltage * 2;
            sumCurrent = sumUp.current * 2;
        }

        sumAll /= input->lowOff;
        sumCurrent /= input->lowOff;

        /* 填充基本结果 */
        head_out->voltage = sumUp.voltage;
        head_out->active_power = (int32_t)(sumAll >> 4);
        head_out->active_current = sumCurrent;

        /* 等效电阻计算 */
        if (sumCurrent > 0) {
            head_out->esr = (uint16_t)(sumUp.voltage / sumCurrent);
        }

        /* Step 3: 计算相位角 */
        CalculatePhaseAngle(head_out, hrtim_lead, phaseUp, phaseDown, input);

        /* Step 4: 填充 HRTIM 状态 */
        FillHRTIMStatus(head_out, input);
    }
}

/* ---- 辅助函数：找过零点 ---- */
static uint16_t FindZeroCrossing(const uint16_t* current, uint16_t start, uint16_t end)
{
    if (current == 0) return 0;
    if (start > 0xff00) return 0;

    uint16_t candidate = 0;
    uint16_t direction = 0xffff;
    uint16_t preValue = current[start];
    uint32_t mivZ = 0xffffffff;

    for (uint16_t i = start + 1; i < end; i++) {
        uint16_t newValue = current[i];
        int16_t delat = newValue - preValue;
        direction <<= 1;
        if (delat > 0)
            direction |= 0x1;
        else
            direction &= ~0x1;
        preValue = newValue;

        if ((direction & 0x3) == 0x1) {
            uint16_t phasePoint = i - 1;
            uint32_t zeroRange = current[phasePoint - 1] + current[phasePoint + 1];
            if (mivZ > zeroRange) {
                mivZ = zeroRange;
                zeroRange >>= 1;
                if (current[phasePoint] <= zeroRange)
                    candidate = phasePoint;
            }
        }
    }
    return candidate;
}

/* ---- FB_CalculateAuctalCurrent — 双HRTIM窗口电流积分 ---- */
/*
 * 【电流采集窗口说明】
 *
 *   移相全桥由四个MOSFET构成两组对角管：
 *     超前臂(TimerB):  Q1=TB1(上管)  Q2=TB2(下管)  互补输出
 *     滞后臂(TimerE):  Q3=TE1(上管)  Q4=TE2(下管)  互补输出
 *
 *   HRTIM计数器 up-counting 模式，CMP1为占空比分界点：
 *     lead <  highOff   → Q1=ON, Q2=OFF  (超前臂上管导通)
 *     lead >= highOff   → Q1=OFF,Q2=ON   (超前臂下管导通)
 *     lag  <  lagDuty   → Q3=ON, Q4=OFF  (滞后臂上管导通)
 *     lag  >= lagDuty   → Q3=OFF,Q4=ON   (滞后臂下管导通)
 *
 *   能量传输只发生在对角管同时导通期间，分为两个窗口：
 *
 *   ┌─────────────────────────────────────────────────────────┐
 *   │  正向窗口 (FORWARD)                                      │
 *   │  Q1(lead < highOff) && Q4(lag >= lagDuty)               │
 *   │  电流路径: Vbus+ → Q1 → 谐振网络 → Q4 → GND             │
 *   │  谐振电流方向 → 正向，直接累加                           │
 *   │  窗口结束: Q1关断(lead↑触达highOff) 时                   │
 *   ├─────────────────────────────────────────────────────────┤
 *   │  反向窗口 (REVERSE)                                      │
 *   │  Q2(lead >= highOff) && Q3(lag < lagDuty)               │
 *   │  电流路径: Vbus+ → Q3 → 谐振网络 → Q2 → GND             │
 *   │  谐振电流方向 → 反向，取绝对值后累加                     │
 *   │  窗口结束: Q3关断(lag↑触达lagDuty) 时                    │
 *   └─────────────────────────────────────────────────────────┘
 *
 *   每个采样点提供一组数据：
 *     resonant_current[num]   ← 1路电流ADC值（两窗口共用）
 *     voltage_data[num]       ← 1路电压ADC值（两窗口共用）
 *     hrtim_lead[num]         ← 超前臂TimerB计数器
 *     hrtim_lag[num]          ← 滞后臂TimerE计数器
 *
 *   由 DMA 分别从 TimerB.CNTxR 和 TimerE.CNTxR 搬运。
 */
static FB_CalculatorResultDef FB_CalculateAuctalCurrent(
    uint16_t* resonant_current,   // 谐振电流ADC数组 [1路，两窗口共用]
    uint16_t* hrtim_lead,         // 超前臂TimerB计数器数组
    uint16_t* hrtim_lag,          // 滞后臂TimerE计数器数组
    uint16_t* voltage_data,       // 电压ADC数组 [1路，两窗口共用]
    uint16_t start,               // 零交叉点数组索引
    FB_Calculator_InputParams_t* input)
{
    uint16_t num = start;
    FB_CalculatorResultDef result = {0};
    uint16_t hrtim_per_adc = input->perAdc;     // 每个ADC采样点对应的HRTIM计数值
    uint16_t endLead = input->highOff;          // 超前臂CMP1 → Q1→Q2切换阈值
    uint16_t endLag  = input->lagDuty;          // 滞后臂CMP1 → Q3→Q4切换阈值
    uint16_t currentType = WINDOW_START;
    uint16_t currentMax = 0;
    uint32_t currentMaxSum;
    uint32_t currentSumPower;
    uint16_t vcCnt = 1;

    // ---- 步骤1: 根据起始点双通道HRTIM状态判定窗口方向 ----
    bool isForward;
    if (hrtim_lead[start] < endLead && hrtim_lag[start] >= endLag)
        isForward = true;     // Q1(lead<endLead) + Q4(lag>=endLag) → 正向
    else if (hrtim_lead[start] >= endLead && hrtim_lag[start] < endLag)
        isForward = false;    // Q2(lead>=endLead) + Q3(lag<endLag) → 反向
    else
        return result;        // 不在任何对角管导通窗口

    // 反向窗口: 电流过零点作为绝对值参考零点
    uint16_t zeroRef = resonant_current[start];

    // ---- 步骤2: 初始化首点累加 ----
    uint32_t currentSum = resonant_current[num];
    uint32_t vcSum = voltage_data[num];
    num++;

    // ---- 步骤3: 遍历采样点，窗口内累加 ----
    do {
        uint16_t rawValue = resonant_current[num];  // 1路电流ADC值
        uint16_t leadVal  = hrtim_lead[num];        // 超前臂计数器
        uint16_t lagVal   = hrtim_lag[num];         // 滞后臂计数器

        // ---- 步骤3a: 对角管导通窗口判断 ----
        bool inWindow;
        if (isForward) {
            // 正向窗口: Q1=ON(lead<endLead) && Q4=ON(lag>=endLag)
            inWindow = (leadVal < endLead) && (lagVal >= endLag);
        } else {
            // 反向窗口: Q2=ON(lead>=endLead) && Q3=ON(lag<endLag)
            inWindow = (leadVal >= endLead) && (lagVal < endLag);
            // 反向电流取绝对值
            if (inWindow)
                rawValue = (rawValue >= zeroRef) ? (rawValue - zeroRef)
                                                 : (zeroRef - rawValue);
        }

        // ---- 步骤3b: 窗口内累加电流积分和电压平均值 ----
        if (inWindow) {
            currentSum += rawValue;
            vcSum += voltage_data[num];
            vcCnt++;

            if (currentMax < rawValue) {
                currentMax = rawValue;
                currentMaxSum = currentSum;
            }
        }

        // ---- 步骤3c: 窗口退出点检测 + 末端插值修正 ----
        switch (currentType) {
        case WINDOW_START: {
            int32_t toLeadOff, toLagOff;
            if (isForward) {
                toLeadOff = (int32_t)endLead - leadVal;  // Q1关断
                toLagOff  = (int32_t)lagVal - endLag;    // Q4关断
            } else {
                toLeadOff = (int32_t)leadVal - endLead;  // Q2关断
                toLagOff  = (int32_t)endLag - lagVal;    // Q3关断
            }

            int32_t nearOff = 0x7FFFFFFF;
            if (toLeadOff >= 0 && toLeadOff < nearOff) nearOff = toLeadOff;
            if (toLagOff  >= 0 && toLagOff  < nearOff) nearOff = toLagOff;

            if (nearOff < (int32_t)hrtim_per_adc) {
                if (nearOff > 0) {
                    currentSumPower = currentSum;
                    currentSumPower *= hrtim_per_adc;
                    currentSumPower += nearOff * rawValue;
                } else {
                    currentSumPower = currentSum;
                    currentSumPower *= hrtim_per_adc;
                }
                currentType = WINDOW_EXIT;
            }
            break;
        }
        case WINDOW_ACTIVE:
            break;
        }

        num++;
        if (num > input->end)
            return result;

    } while (currentType < WINDOW_EXIT);

    // ---- 步骤4: 输出电流积分值和电压平均值 ----
    vcSum /= vcCnt;
    result.current = currentSumPower;
    result.voltage = vcSum;
    return result;
}

/* ---- 计算相位角 ---- */
static void CalculatePhaseAngle(FB_Calculator_OutHead_t* head_out, uint16_t* hrtim_lead,
    uint16_t phaseUp, uint16_t phaseDown, FB_Calculator_InputParams_t* input)
{
    if (hrtim_lead[phaseUp] > input->highOn) {
        uint32_t angle = hrtim_lead[phaseUp] - input->highOn;
        head_out->zero_cross_high = angle / (FRE_PER_ADC / 4);
        angle *= PHASE_DEG_BASE;
        angle /= input->highOff - input->highOn;
        head_out->phase_angleUp = (int16_t)angle;
    }
    if (hrtim_lead[phaseDown] > input->lowOn) {
        uint32_t angle = hrtim_lead[phaseDown] - input->lowOn;
        head_out->zero_cross_low = angle / (FRE_PER_ADC / 4);
        angle *= 180;
        angle /= (input->lowOff - input->lowOn);
        head_out->phase_angleDown = (int16_t)angle;
    }
}

/* ---- 填充 HRTIM 状态 ---- */
static void FillHRTIMStatus(FB_Calculator_OutHead_t* head_out, FB_Calculator_InputParams_t* input)
{
    head_out->hrtim.highOn = input->highOn;
    head_out->hrtim.highOff = input->highOff;
    head_out->hrtim.lowOn = input->lowOn;
    head_out->hrtim.lowOff = input->lowOff;
    head_out->hrtim.lagDuty = input->lagDuty;
}

/* ---- 初始化 ---- */
static void Init(void)
{
    memset(&s_inPara,  0, sizeof(s_inPara));
    memset(&s_outPara, 0, sizeof(s_outPara));
    g_input.para   = &s_inPara;
    g_output.para  = &s_outPara;
}

/* ---- 导出 ---- */
MODULE_EXPORT(FB_Calculator);