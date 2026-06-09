/**
 * @file    power_calc_half.c
 * @brief   PowerCalc_HalfBridge 婴儿模块 — v2.2 MODULE_SKELETON 范式
 * @layer   app
 *
 * 遵循 std_module.h PULL 范式 — 半桥功率计算子类。
 * 输入: AdcBlock_t* (来自 AppAdc) + PowerBase_Output_t* (ppg_delta)
 * 输出: PowerCalc_Output_t (窗口积分: active_power + peak_current)
 *
 * ProcessInput 三段式:
 *   ① 从 pAdc 取 DMA 原始指针 + pBase 取 ppg_delta
 *   ② 窗口积分 (FindZeroCrossing → CalculateAuctalCurrent → CalculatePower)
 *   ③ 输出映射 (i_ppg_control → 写 HRTIM)
 */

#include "std_module.h"
#include "../include/app_adc_io.h"
#include "../include/app_power_io.h"
#include "../include/app_power_calc_io.h"
#include "../include/app_power_defs.h"

#include "API_HRTIM.h"
#include "../base_class/inc/power_calculator.h"

/* ---- POTNUM: 与 power_base.c 保持一致 ---- */
#define POTNUM  4

/* ---- extern: 函数来自 app_power.c / power_base.c ---- */
extern void     i_ppg_set_limit(INT16U t_corrent_ppg);
extern uint8_t  Time_GetMs100Flg(void);

/* ---- 常量 ---- */
#define PPG_MAX_RANGE      200
#define C_KEEP_TIME        5

/* ---- 标志位 ---- */
#define F_POWER_FAST_UP    1
#define F_POWER_FAST_DOWN  2
#define F_POWER_OVER       3
#define F_SURGE_OVER       4
#define F_POT_ERR          5

/* ---- 功率极限标志 (来自原 app_power.c) ---- */
#define B_PPGMAX           _BIT0
#define B_POW_ARRIVE_FLAG  _BIT5

/* ---- PowerControl 宏 (与 power_base.c 保持一致, 独立编译单元) ---- */
#define PowerMinFre             PowerAll.minFre
#define PowerCycle              PowerAll.pCycle

#define vcout_delay             PowerControl->staticReg->VcoutDelay
#define g_surge_delay           PowerControl->staticReg->SurgeDelay
#define power_half_adj          PowerControl->staticReg->power_half_adj
#define s_limit_max_count       PowerControl->staticReg->LimitMaxCount
#define s_limit_voltage         PowerControl->staticReg->LimitVoltage
#define s_limit_Qvalue          PowerControl->staticReg->limitQ
#define s_limit_Qsum            PowerControl->staticReg->limitQSum
#define s_ppg_limit_power       PowerControl->staticReg->ppgLimitPower

#define s_ppg_limit             PowerControl->staticReg->PpgLimit
#define s_ppg_limit_max         PowerControl->staticReg->ppgLimitMax
#define g_power_adc_trig        PowerControl->staticReg->PowerAdcTrig
#define g_power_duty            PowerControl->staticReg->PowerDuty
#define g_duty_actual           PowerControl->staticReg->PPGdutyActual

#define g_off_flag              PowerControl->staticReg->PowerOffFlag
#define g_power_limit_flag      PowerControl->staticReg->PowerLimitFlag

#define m_ppg_on                PowerControl->staticReg->flag.bit.ppgOn
#define m_power_cycle_flag      PowerControl->staticReg->flag.bit.PowerCycleType
#define m_ppg_add_flag          PowerControl->staticReg->flag.bit.ppgAdd
#define m_vcout_flag            PowerControl->staticReg->flag.bit.Vcout
#define m_ppg_lock_flag         PowerControl->staticReg->flag.bit.ppgLock

#define IHStatus                PowerControl->input->status.ihStatus
#define VoltageValue            PowerControl->input->status.voltageAd
#define CurrentValue            PowerControl->input->status.currentAd
#define BOTTOMValue             PowerControl->input->status.bottomAd
#define TOPValue                PowerControl->input->status.topAd
#define ActualPower             PowerControl->input->status.actualPowerDiv25
#define TargetPower             PowerControl->input->status.targetPowerDiv25
#define ActualPPG               PowerControl->input->status.actualPPG
#define PowerStatus             PowerControl->input->status.powerStatus

#define LoadTest                PowerControl->input->init.loadTest
#define VC_LIMIT_MAX            PowerControl->input->init.vcLimitMax
#define MaxPowerM               PowerControl->input->init.maxPowerM

/* ====== 骨架 (声明 g_input/g_output — 必须在所有使用它们之前) ====== */
MODULE_SKELETON(PowerCalc);

/* ================================================================
 * 调试/测试数据数组 (来自 power_calculator.c)
 * 实际使用时这些会被 DMA buffer 指针替换
 * ================================================================ */

uint16_t voltage_data[] = {
    255,247,258,254,255,262,251,228,235,264,266,260,262,255,268,255,
    266,260,260,268,259,270,259,272,263,267,270,263,276,263,274,270,
    269,276,267,280,263,274,270,270,276,269,328,295,292,286,281,284,
    275,286,275,286,279,281,286,277,285,279,286,279,283,286,281,292,
    279,292,286,288,287,283,296,287,236,263,281,308,293,
};

uint16_t hrtim_data[] = {
    23164,23548,23932,24316,24700,25084,264,648,1032,1416,1800,2184,
    2568,2952,3336,3720,4104,4488,4872,5256,5640,6024,6408,6792,
    7176,7560,7944,8328,8712,9096,9480,9864,10248,10632,11016,11400,
    11784,12168,12552,12920,13328,13704,14072,14480,14856,15216,15600,
    15984,16368,16784,17160,17544,17928,18312,18704,19080,19464,19848,
    20240,20616,21000,21392,21768,22152,22536,22920,23304,23688,24080,
    24456,24840,20,404,788,1172,1556,1940,
};

uint16_t current_data[] = {
    44,47,50,55,55,52,59,63,67,72,71,55,52,41,40,31,24,23,6,
    12,5,8,7,4,12,1,14,15,24,27,30,38,37,44,47,52,55,58,62,
    62,63,65,63,59,56,38,39,31,27,24,15,8,10,4,24,5,8,7,4,
    14,13,24,23,29,31,24,46,44,52,51,57,59,62,63,68,70,57,
};

uint16_t hrtim_data1[] = {
    11556,12324,292,1060,1828,2596,3364,4132,4900,5668,6444,7204,
    7972,8740,9508,10276,11044,11812,
};

uint16_t current_data1[] = {
    28,76,122,192,199,139,78,28,29,58,117,149,175,142,100,41,18,23,
};

const uint16_t CURRENT_DATA_SIZE1 = sizeof(current_data1) / sizeof(uint16_t);

PowerCalculatorInputDef inputArrayConst1 = {
    0,
    CURRENT_DATA_SIZE1,
    1572,               /* highOn     */
    6527,               /* highOff    */
    6527 + 1572,        /* lowOn      */
    6527 * 2,           /* lowOff = period */
    0,
    0,
    0x180,              /* perAdc     */
    0,
};

const uint16_t CURRENT_DATA_SIZE = sizeof(current_data) / sizeof(uint16_t);

PowerCalculatorInputDef inputArrayConst = {
    0,
    CURRENT_DATA_SIZE,
    572,                /* highOn     */
    12602,              /* highOff    */
    12602 + 572,        /* lowOn      */
    12602 * 2,          /* lowOff = period */
    0,
    0,
    0x180,              /* perAdc     */
    0,
};

/* ================================================================
 * 数据访问函数
 * ================================================================ */

int16_t * Power_Calculator_GetHrtimBuffAddress(uint8_t ch)
{
    if (ch) return (int16_t *)hrtim_data1;
    else    return (int16_t *)hrtim_data;
}

int16_t * Power_Calculator_GetVoltageBuffAddress(uint8_t ch)
{
    (void)ch;
    return (int16_t *)voltage_data;
}

int16_t Power_Calculator_GetTxaBuffSize(uint8_t ch)
{
    if (ch) return CURRENT_DATA_SIZE1;
    else    return CURRENT_DATA_SIZE;
}

int16_t * Power_Calculator_GetTxaBuffAddress(uint8_t ch)
{
    if (ch) return (int16_t *)current_data1;
    else    return (int16_t *)current_data;
}

PowerCalculatorInputDef * Power_Calculator_GetInputArrayAddress(uint8_t ch)
{
    if (ch) return &inputArrayConst1;
    else    return &inputArrayConst;
}

/* ================================================================
 * FindZeroCrossing — 在指定区间内寻找过零点
 * ================================================================ */
static uint16_t FindZeroCrossing(const uint16_t *current,
                                 uint16_t start, uint16_t end)
{
    uint16_t candidate  = 0;
    uint16_t direction  = 0xffff;
    uint16_t preValue, newValue;
    uint32_t mivZ = 0xffffffff;

    if (current == 0)       return 0;
    if (start > 0xff00)      return 0;

    preValue = current[start];

    for (uint16_t i = start + 1; i < end; i++) {
        newValue = current[i];
        int16_t delat = newValue - preValue;

        direction <<= 1;
        if (delat > 0)  direction |= 0x1;
        else            direction &= ~0x1;

        preValue = newValue;

        if ((direction & 0x3) == 0x1) {
            uint16_t phasePoint = i - 1;
            uint32_t zeroRange  = current[phasePoint - 1]
                                + current[phasePoint + 1];

            if (mivZ > zeroRange) {
                mivZ = zeroRange;
                zeroRange >>= 1;

                if (current[phasePoint] <= zeroRange) {
                    candidate = phasePoint;
                }
            }
        }
    }
    return candidate;
}

/* ================================================================
 * CalculateAuctalCurrent — 窗口积分计算有效电流
 * ================================================================ */
static CalculatorResultDef CalculateAuctalCurrent(
    uint16_t *resonant_current,
    uint16_t *hrtim_values,
    uint16_t *voltage_data,
    uint16_t start,
    PowerCalculatorInputDef *input)
{
    uint16_t num = start;
    CalculatorResultDef result = {0};
    uint16_t hrtim_per_adc = input->perAdc;
    uint16_t end  = input->highOff;
    uint16_t dead = input->lowOn;
    uint16_t currentMax    = 0;
    uint16_t currentMaxNum;
    uint16_t currentType   = 0;   /* pluseStart */
    uint32_t currentMaxSum;
    uint32_t currentSumPower;
    uint16_t vcCnt = 1;

    if (hrtim_values[start] > end) {
        end  = input->lowOff;
        dead = input->highOn;
    }

    uint32_t currentSum = resonant_current[num];
    uint32_t vcSum      = voltage_data[num];
    num++;

    do {
        uint16_t newValue = resonant_current[num];
        currentSum += newValue;

        if (currentMax < newValue) {
            currentMax    = newValue;
            currentMaxSum = currentSum;
            currentMaxNum = num;
        }

        uint16_t hrtim1us = hrtim_values[num];
        int32_t lastPoint;

        switch (currentType) {
        case 0: /* pluseStart */
            vcSum += voltage_data[num];
            vcCnt++;
            lastPoint = (int32_t)end - hrtim1us;
            if (lastPoint < hrtim_per_adc) {
                if (1) {
                    if (lastPoint > 0) {
                        lastPoint *= newValue;
                        currentSumPower  = currentSum;
                        currentSumPower *= hrtim_per_adc;
                        currentSumPower += lastPoint;
                        currentType = 2;   /* pluseLowOn */
                    }
                } else {
                    currentType = 1;   /* pluseHighOff */
                }
            }
            break;

        case 1: /* pluseHighOff */
            if ((dead - hrtim1us) < hrtim_per_adc) {
                currentSumPower  = currentMaxSum;
                currentSumPower *= hrtim_per_adc;
                currentType = 2;   /* pluseLowOn */
            }
            break;
        }

        num++;
        if (num > input->end) {
            return result;
        }

    } while (currentType < 2);  /* pluseLowOn */

    vcSum /= vcCnt;

    result.current = currentSumPower;
    result.voltage = vcSum;

    return result;
}

/* ================================================================
 * CalculatePower — 主功率计算
 * ================================================================ */
PowerResult CalculatePower(
    uint16_t *resonant_current,
    uint16_t *hrtim_values,
    uint16_t *voltage_values,
    PowerCalculatorInputDef *input)
{
    PowerResult xReturn;
    xReturn.zero_cross_high = 0;
    xReturn.zero_cross_low  = 0;

    uint16_t hrtim_per_adc = input->perAdc;
    uint16_t potNumStart   = input->start + 4;
    uint16_t zeroStart, zeroEnd;
    uint16_t highOffNum    = input->highOff / hrtim_per_adc;

    highOffNum += potNumStart;

    /* Step 1: 寻找过零点 */
    zeroStart = potNumStart;
    zeroEnd   = highOffNum + 2;

    uint16_t phaseUp = FindZeroCrossing(resonant_current, zeroStart, zeroEnd);

    zeroStart = highOffNum;
    zeroEnd   = input->end;

    uint16_t phaseDown = FindZeroCrossing(resonant_current, zeroStart, zeroEnd);

    if (phaseUp && phaseDown) {
        CalculatorResultDef sumUp = CalculateAuctalCurrent(
            resonant_current, hrtim_values, voltage_values,
            phaseUp, input);

        if (sumUp.current == 0) {
            return xReturn;
        }

        uint64_t sumAll;
        uint32_t sumCurrent;

        if ((input->highOff * 2 + 10) < input->lowOff) {
            /* 非对称输出 */
            CalculatorResultDef sumDown = CalculateAuctalCurrent(
                resonant_current, hrtim_values, voltage_values,
                phaseDown, input);
            sumAll    = sumUp.current * sumUp.voltage
                      + sumDown.current * sumDown.voltage;
            sumCurrent = sumUp.current + sumDown.current;
        } else {
            /* 对称输出 */
            sumAll     = sumUp.current * sumUp.voltage * 2;
            sumCurrent = sumUp.current * 2;
        }

        sumAll    /= input->lowOff;
        sumCurrent /= input->lowOff;

        xReturn.voltage        = sumUp.voltage;
        xReturn.active_power   = sumAll >> 4;
        xReturn.active_current = sumCurrent;
        {
            uint32_t esrValue = sumUp.voltage / sumCurrent;
            xReturn.esr = esrValue;
        }

        /* 相位角计算 */
        if (hrtim_values[phaseUp] > input->highOn) {
            uint32_t angle = hrtim_values[phaseUp] - input->highOn;
            xReturn.zero_cross_high = angle / (FRE_PER_ADC / 4);
            angle *= 180;
            angle /= input->highOff - input->highOn;
            xReturn.phase_angleUp = angle;
        }
        if (hrtim_values[phaseDown] > input->lowOn) {
            uint32_t angle = hrtim_values[phaseDown] - input->lowOn;
            xReturn.zero_cross_low = angle / (FRE_PER_ADC / 4);
            angle *= 180;
            angle /= input->lowOff - input->lowOn;
            xReturn.phase_angleDown = angle;
        }
    }

    return xReturn;
}

/* ================================================================
 * i_ppg_control — PPG 增减控制 (原 app_power.c:4152)
 *
 * 由 Switcher Slot 2 (PowerCalc.DoWork → ProcessInput) 调用。
 * 每炉头独立运行 — 调用前需 getPowerCHN(ch) 设置 PowerControl 上下文。
 * ================================================================ */
INT16U i_ppg_control(INT8U t_pan_cur_change)
{
    INT16U t_corrent_ppg;
    INT8U  t_ppg_dict;
    INT8U  t_ppg_change_value;
    INT16U t_ppg_limit;

    t_corrent_ppg = g_duty_actual;

    if (g_surge_delay < POT_TYPE_DELAY2 + 2) {
        m_vcout_flag = 0;
        vcout_delay  = 0;
        return g_power_duty;
    }

    if (m_vcout_flag) {
        vcout_delay  = 50;
        m_vcout_flag = 0;
    }

    if (vcout_delay) {
        if (Time_GetMs100Flg()) {
            vcout_delay--;
        }
    }

    if (t_pan_cur_change != 0xff) {
        g_power_limit_flag &= ~B_PPGMAX;

        if (t_pan_cur_change) {
            t_ppg_dict        = t_pan_cur_change & 0x80;
            t_ppg_change_value = t_pan_cur_change & 0x7f;

            if (t_ppg_dict) {
                m_ppg_add_flag = 0;
                if (t_corrent_ppg > t_ppg_change_value) {
                    t_corrent_ppg -= t_ppg_change_value;
                }
            } else {
                if (vcout_delay) {
                    t_ppg_change_value = 1;
                    if (vcout_delay > 40) {
                        t_ppg_change_value = 0;
                    }
                }
                BOTTOMValue       = t_ppg_change_value;
                t_corrent_ppg    += t_ppg_change_value;
                m_ppg_add_flag    = 1;
            }
        }

        i_ppg_set_limit(t_corrent_ppg);

#if 1
        t_ppg_limit = s_ppg_limit;

        if (t_corrent_ppg >= t_ppg_limit) {
            if (vcout_delay == 0) {
                if (s_ppg_limit < s_ppg_limit_max) {
                    s_ppg_limit++;
                }
            }
            t_corrent_ppg = t_ppg_limit;
            g_power_limit_flag |= B_PPGMAX;
            IHStatus |= B_POW_ARRIVE_FLAG;
        } else {
            g_power_limit_flag &= ~B_PPGMAX;
        }
#endif
    }

    PowerStatus = ((g_off_flag & 0x0f) << 4) + (g_power_limit_flag & 0x0f);

    if (m_ppg_on) {
        if (t_corrent_ppg < MAX_FRE_PWM) {
            t_corrent_ppg = MAX_FRE_PWM;
        }
    }
    if (t_corrent_ppg > PowerMinFre) {
        t_corrent_ppg = PowerMinFre;
    }

    g_power_duty = t_corrent_ppg;

    TOPValue  = s_ppg_limit >> 8;
    ActualPPG = g_power_duty >> 8;

    return t_corrent_ppg;
}

/* ================================================================
 * 模块私有数据
 * ================================================================ */
static PowerCalc_Input_t   s_in;
static PowerCalc_Output_t  s_out;

/* ================================================================
 * Init — 首次运行前的构造
 * ================================================================ */
static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

/* ================================================================
 * ProcessInput — 三段式: 窗口积分 → HRTIM 输出
 *
 * InputCallback (data_switcher.c 强符号) 在上游 DoWork 后
 * 自动填充 pIn->pAdc 和 pIn->pBase。
 * ================================================================ */
static void ProcessInput(void)
{
    if (!(g_input.info.status & ST_NEW))
        return;

    {
        PowerCalc_Input_t  *pIn   = (PowerCalc_Input_t *)g_input.para;
        PowerCalc_Output_t *pOut  = (PowerCalc_Output_t *)g_output.para;
        PowerBase_Output_t *pBase = pIn->pBase;

        g_input.info.status &= ~ST_NEW;

        /* ---- 每炉头: 功率计算 + PPG 输出 ---- */
        for (uint8_t ch = 0; ch < POTNUM; ch++) {
            if (getPowerCHN(ch) == NULL) {
                break;
            }

            /* ① 功率计算 (窗口积分) */
            {
                int16_t *cur_buf = Power_Calculator_GetTxaBuffAddress(ch);
                int16_t *hrt_buf = Power_Calculator_GetHrtimBuffAddress(ch);
                int16_t *v_buf   = Power_Calculator_GetVoltageBuffAddress(ch);
                PowerCalculatorInputDef *inp = Power_Calculator_GetInputArrayAddress(ch);

                PowerResult res = CalculatePower(
                    (uint16_t *)cur_buf,
                    (uint16_t *)hrt_buf,
                    (uint16_t *)v_buf,
                    inp);

                pOut->head[ch].active_power   = res.active_power;
                pOut->head[ch].active_current = res.active_current;
                pOut->head[ch].peak_current   = res.peak_current;
                pOut->head[ch].voltage        = res.voltage;
            }

            /* ② PPG 输出: 应用 PowerBase 传来的 ppg_delta 到 HRTIM */
            if (pBase) {
                i_ppg_control(pBase->head[ch].ppg_delta);
            }
        }

        pOut->valid = (POTNUM > 0) ? 1 : 0;
        g_output.info.status |= ST_OUT;
    }
}

/* ====== 导出 ====== */
MODULE_EXPORT(PowerCalc);
