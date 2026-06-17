#include "power_calculator_fullbridge.h"
#include "api_hrtim.h"
#include <stdlib.h>

/*============================================================================
 *                        测试数据（双通道HRTIM）
 *============================================================================*/

// 通道0 — 超前臂(TimerB) HRTIM数据
uint16_t hrtim_lead_data0[] = {
    23164, 23548, 23932, 24316, 24700, 25084, 264, 648,
    1032, 1416, 1800, 2184, 2568, 2952, 3336, 3720,
    4104, 4488, 4872, 5256, 5640, 6024, 6408, 6792,
    7176, 7560, 7944, 8328, 8712, 9096, 9480, 9864,
    10248, 10632, 11016, 11400, 11784, 12168, 12552, 12920,
    13328, 13704, 14072, 14480, 14856, 15216, 15600, 15984,
    16368, 16784, 17160, 17544, 17928, 18312, 18704, 19080,
    19464, 19848, 20240, 20616, 21000, 21392, 21768, 22152,
    22536, 22920, 23304, 23688, 24080, 24456, 24840, 20,
    404, 788, 1172, 1556, 1940,
};

// 通道0 — 滞后臂(TimerE) HRTIM数据（与lead有相位偏移）
uint16_t hrtim_lag_data0[] = {
    11556, 12324, 292, 1060, 1828, 2596, 3364, 4132,
    4900, 5668, 6444, 7204, 7972, 8740, 9508, 10276,
    11044, 11812, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
};

// 通道1 — 超前臂(TimerB) HRTIM数据
uint16_t hrtim_lead_data1[] = {
    11556, 12324, 292, 1060, 1828, 2596, 3364, 4132,
    4900, 5668, 6444, 7204, 7972, 8740, 9508, 10276,
    11044, 11812,
};

// 通道1 — 滞后臂(TimerE) HRTIM数据
uint16_t hrtim_lag_data1[] = {
    23164, 23548, 23932, 24316, 24700, 25084, 264, 648,
    1032, 1416, 1800, 2184, 2568, 2952, 3336, 3720,
    4104, 4488,
};

// 电流数据
uint16_t current_data_fb[] = {
    44, 47, 50, 55, 55, 52, 59, 63,
    67, 72, 71, 55, 52, 41, 40, 31,
    24, 23, 6, 12, 5, 8, 7, 4,
    12, 1, 14, 15, 24, 27, 30, 38,
    37, 44, 47, 52, 55, 58, 62, 62,
    63, 65, 63, 59, 56, 38, 39, 31,
    27, 24, 15, 8, 10, 4, 24, 5,
    8, 7, 4, 14, 13, 24, 23, 29,
    31, 24, 46, 44, 52, 51, 57, 59,
    62, 63, 68, 70, 57,
};

uint16_t current_data_fb1[] = {
    28, 76, 122, 192, 199, 139, 78, 28,
    29, 58, 117, 149, 175, 142, 100, 41,
    18, 23,
};

// 电压数据
uint16_t voltage_data_fb[] = {
    255, 247, 258, 254, 255, 262, 251, 228,
    235, 264, 266, 260, 262, 255, 268, 255,
    266, 260, 260, 268, 259, 270, 259, 272,
    263, 267, 270, 263, 276, 263, 274, 270,
    269, 276, 267, 280, 263, 274, 270, 270,
    276, 269, 328, 295, 292, 286, 281, 284,
    275, 286, 275, 286, 279, 281, 286, 277,
    285, 279, 286, 279, 283, 286, 281, 292,
    279, 292, 286, 288, 287, 283, 296, 287,
    236, 263, 281, 308, 293,
};

const uint16_t CURRENT_DATA_SIZE_FB0 = sizeof(current_data_fb) / sizeof(uint16_t);
const uint16_t CURRENT_DATA_SIZE_FB1 = sizeof(current_data_fb1) / sizeof(uint16_t);

FB_PowerCalculatorInputDef inputArrayConst_fb0 = {
    0,                      // start
    CURRENT_DATA_SIZE_FB0,  // end
    572,                    // highOn (lead dead time end)
    12602,                  // highOff (lead duty CMP1)
    12602 + 572,            // lowOn (lead highOff + dead time fall)
    12602 * 2,              // lowOff (period)
    0,                      // zero_cross_high
    0,                      // zero_cross_low
    0x180,                  // perAdc
    6527,                   // lagDuty (lag arm duty CMP1)
};

FB_PowerCalculatorInputDef inputArrayConst_fb1 = {
    0,
    CURRENT_DATA_SIZE_FB1,
    1572,                   // highOn
    6527,                   // highOff (lead duty)
    6527 + 1572,            // lowOn
    6527 * 2,               // lowOff (period)
    0,
    0,
    0x180,                  // perAdc
    12602,                  // lagDuty (lag arm duty)
};

/*============================================================================
 *                        辅助函数（复用半桥逻辑）
 *============================================================================*/

// 32位整数平方根
static uint32_t sqrt32(uint32_t n) {
    uint32_t root = 0;
    uint32_t bit = 1UL << 30;
    while (bit > n) bit >>= 2;
    while (bit != 0) {
        if (n >= root + bit) {
            n -= root + bit;
            root = (root >> 1) + bit;
        } else {
            root >>= 1;
        }
        bit >>= 2;
    }
    return root;
}

// 在指定区间内寻找过零点
static uint16_t FindZeroCrossing(const uint16_t* current, uint16_t start, uint16_t end) {
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

/*============================================================================
 *                    窗口判断状态机定义
 *============================================================================*/

enum {
    WINDOW_START = 0,
    WINDOW_ACTIVE,
    WINDOW_EXIT,
};

/*============================================================================
 *              FB_CalculateAuctalCurrent — 双HRTIM窗口电流积分
 *
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
 *============================================================================*/

CalculatorResultDef FB_CalculateAuctalCurrent(
    uint16_t* resonant_current,   // 谐振电流ADC数组 [1路，两窗口共用]
    uint16_t* hrtim_lead,         // 超前臂TimerB计数器数组
    uint16_t* hrtim_lag,          // 滞后臂TimerE计数器数组
    uint16_t* voltage_data,       // 电压ADC数组 [1路，两窗口共用]
    uint16_t start,               // 零交叉点数组索引
    PowerCalculatorInputDef* input)
{
    uint16_t num = start;
    CalculatorResultDef result = {0};
    uint16_t hrtim_per_adc = input->perAdc;     // 每个ADC采样点对应的HRTIM计数值
    uint16_t endLead = input->highOff;          // 超前臂CMP1 → Q1→Q2切换阈值
    uint16_t endLag  = input->lagDuty;          // 滞后臂CMP1 → Q3→Q4切换阈值
    uint16_t currentType = WINDOW_START;
    uint16_t currentMax = 0;
    uint32_t currentMaxSum;
    uint32_t currentSumPower;
    uint16_t vcCnt = 1;

    // ---- 步骤1: 根据起始点双通道HRTIM状态判定窗口方向 ----
    // 零交叉点必然落在对角管导通窗口内，据此判断正向/反向
    bool isForward;
    if (hrtim_lead[start] < endLead && hrtim_lag[start] >= endLag)
        isForward = true;     // Q1(lead<endLead) + Q4(lag>=endLag) → 正向
    else if (hrtim_lead[start] >= endLead && hrtim_lag[start] < endLag)
        isForward = false;    // Q2(lead>=endLead) + Q3(lag<endLag) → 反向
    else
        return result;        // 不在任何对角管导通窗口（两上管或两下管同时导通）

    // 反向窗口: 电流过零点作为绝对值参考零点
    uint16_t zeroRef = resonant_current[start];

    // ---- 步骤2: 初始化首点累加 ----
    uint32_t currentSum = resonant_current[num];
    uint32_t vcSum = voltage_data[num];
    num++;

    // ---- 步骤3: 遍历采样点，窗口内累加 ----
    do {
        uint16_t rawValue = resonant_current[num];  // 1路电流ADC值
        uint16_t leadVal  = hrtim_lead[num];        // 超前臂计数器 → 判断Q1/Q2
        uint16_t lagVal   = hrtim_lag[num];         // 滞后臂计数器 → 判断Q3/Q4

        // ---- 步骤3a: 对角管导通窗口判断 ----
        bool inWindow;
        if (isForward) {
            // 正向窗口: Q1=ON(lead<endLead) && Q4=ON(lag>=endLag)
            inWindow = (leadVal < endLead) && (lagVal >= endLag);
        } else {
            // 反向窗口: Q2=ON(lead>=endLead) && Q3=ON(lag<endLag)
            inWindow = (leadVal >= endLead) && (lagVal < endLag);
            // 反向电流取绝对值，以零交叉点电流为参考零点
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
            // 计算各管到关断点的HRTIM计数值距离（正值=尚未关断）
            int32_t toLeadOff, toLagOff;
            if (isForward) {
                toLeadOff = (int32_t)endLead - leadVal;  // Q1关断: lead↑逼近endLead
                toLagOff  = (int32_t)lagVal - endLag;    // Q4关断: lag↓逼近endLag(逆行检测)
            } else {
                toLeadOff = (int32_t)leadVal - endLead;  // Q2关断: lead↓逼近endLead(逆行检测)
                toLagOff  = (int32_t)endLag - lagVal;    // Q3关断: lag↑逼近endLag
            }

            // 取最先关断的正值距离
            int32_t nearOff = 0x7FFFFFFF;
            if (toLeadOff >= 0 && toLeadOff < nearOff) nearOff = toLeadOff;
            if (toLagOff  >= 0 && toLagOff  < nearOff) nearOff = toLagOff;

            // 距离 < 一个采样周期 → 窗口即将关闭，插值修正末端不完整采样点
            if (nearOff < (int32_t)hrtim_per_adc) {
                if (nearOff > 0) {
                    currentSumPower = currentSum;
                    currentSumPower *= hrtim_per_adc;
                    currentSumPower += nearOff * rawValue;   // 末端点按比例计入
                } else {
                    currentSumPower = currentSum;
                    currentSumPower *= hrtim_per_adc;
                }
                currentType = WINDOW_EXIT;
            }
            break;
        }
        case WINDOW_ACTIVE:
            // 死区延伸（预留，与半桥pluseHighOff对应）
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

/*============================================================================
 *                FB_CalculatePower — 移相全桥功率计算主函数
 *============================================================================*/

PowerResult FB_CalculatePower(
    uint16_t* resonant_current,
    uint16_t* hrtim_lead,
    uint16_t* hrtim_lag,
    uint16_t* voltage_values,
    PowerCalculatorInputDef* input)
{
    PowerResult xReturn;
    xReturn.zero_cross_high = 0;
    xReturn.zero_cross_low = 0;
    xReturn.active_power = 0;
    xReturn.active_current = 0;
    xReturn.voltage = 0;
    xReturn.esr = 0;

    uint16_t hrtim_per_adc = input->perAdc;
    uint16_t potNumStart = input->start + 4;
    uint16_t zeroStart, zeroEnd;
    uint16_t highOffNum = input->highOff / hrtim_per_adc;

    highOffNum += potNumStart;  // 上管关断序号点

    // Step 1: 寻找过零点（沿用半桥逻辑，基于电流波形）
    zeroStart = potNumStart;
    zeroEnd = highOffNum + 2;
    uint16_t phaseUp = FindZeroCrossing(resonant_current, zeroStart, zeroEnd);

    zeroStart = highOffNum;
    zeroEnd = input->end;
    uint16_t phaseDown = FindZeroCrossing(resonant_current, zeroStart, zeroEnd);

    if (phaseUp && phaseDown) {
        // 统计正向窗口(Q1+Q4)有效电流
        CalculatorResultDef sumUp = FB_CalculateAuctalCurrent(
            resonant_current, hrtim_lead, hrtim_lag,
            voltage_values, phaseUp, input);

        if (sumUp.current == 0)
            return xReturn;

        uint64_t sumAll;
        uint32_t sumCurrent;

        if ((input->highOff * 2 + 10) < input->lowOff) {
            // 非对称输出 → 统计反向窗口(Q2+Q3)
            CalculatorResultDef sumDown = FB_CalculateAuctalCurrent(
                resonant_current, hrtim_lead, hrtim_lag,
                voltage_values, phaseDown, input);
            sumAll = sumUp.current * sumUp.voltage + sumDown.current * sumDown.voltage;
            sumCurrent = sumUp.current + sumDown.current;
        } else {
            // 对称输出
            sumAll = sumUp.current * sumUp.voltage * 2;
            sumCurrent = sumUp.current * 2;
        }

        sumAll /= input->lowOff;
        sumCurrent /= input->lowOff;

        xReturn.voltage = sumUp.voltage;
        xReturn.active_power = sumAll >> 4;
        xReturn.active_current = sumCurrent;

        uint32_t esrValue = sumUp.voltage / sumCurrent;
        xReturn.esr = esrValue;

        // 相位角（使用超前臂HRTIM值，对应半桥逻辑）
        if (hrtim_lead[phaseUp] > input->highOn) {
            uint32_t angle = hrtim_lead[phaseUp] - input->highOn;
            xReturn.zero_cross_high = angle / (FRE_PER_ADC / 4);
            angle *=PHASE_DEG_BASE;
            angle /= input->highOff - input->highOn;
            xReturn.phase_angleUp = angle;
        }
        if (hrtim_lead[phaseDown] > input->lowOn) {
            uint32_t angle = hrtim_lead[phaseDown] - input->lowOn;
            xReturn.zero_cross_low = angle / (FRE_PER_ADC / 4);
            angle *= 180;
            angle /= (input->lowOff - input->lowOn);
            xReturn.phase_angleDown = angle;
        }
    }

    return xReturn;
}

/*============================================================================
 *                    数据缓冲区访问函数
 *============================================================================*/

int16_t* FB_Power_Calculator_GetHrtimLeadBuffAddress(uint8_t ch)
{
    return ch ? (int16_t*)hrtim_lead_data1 : (int16_t*)hrtim_lead_data0;
}

int16_t* FB_Power_Calculator_GetHrtimLagBuffAddress(uint8_t ch)
{
    return ch ? (int16_t*)hrtim_lag_data1 : (int16_t*)hrtim_lag_data0;
}

int16_t* FB_Power_Calculator_GetVoltageBuffAddress(uint8_t ch)
{
    (void)ch;
    return (int16_t*)voltage_data_fb;
}

int16_t* FB_Power_Calculator_GetTxaBuffAddress(uint8_t ch)
{
    return ch ? (int16_t*)current_data_fb1 : (int16_t*)current_data_fb;
}

int16_t FB_Power_Calculator_GetTxaBuffSize(uint8_t ch)
{
    return ch ? CURRENT_DATA_SIZE_FB1 : CURRENT_DATA_SIZE_FB0;
}

PowerCalculatorInputDef* FB_Power_Calculator_GetInputArrayAddress(uint8_t ch)
{
    return ch ? &inputArrayConst_fb1 : &inputArrayConst_fb0;
}
