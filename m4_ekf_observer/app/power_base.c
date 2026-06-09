/**
 * @file    power_base.c
 * @brief   PowerBase 婴儿模块 — v2.2 MODULE_SKELETON 范式
 * @layer   app
 *
 * 遵循 std_module.h PULL 范式。
 * 输入: PowerBase_Input_t (AdcBlock_t* + PowerCalc_Output_t* + MODBUS commands)
 * 输出: PowerBase_Output_t (ppg_delta per head)
 *
 * ProcessInput 每 1ms 调用一次:
 *   ① 从 g_input.para 读指针，拷贝 ADC 数据到 PowerMem[ch]
 *   ② 对每个炉头: PowerControlFun -> surge -> OVP -> panCheck -> power -> PID -> ppg_delta
 *   ③ 写入 g_output.para -> ST_OUT
 *
 * 硬件访问 (PPG/HRTIM/PanDetect) 保留在 app_power.c 中，
 * 通过函数指针表 PowerControl->funAdr-> 间接调用。
 *
 * 文件结构:
 *   1. 模块骨架 + 数据实体定义
 *   2. 纯算法函数 (Pure Algorithm) — 无硬件依赖
 *   3. 硬件控制函数 (HAL_HALF) — 直接调用 HAL API
 *   4. 中断回调函数 (ISR Callbacks) — 中断入口与硬件交互
 *   5. API层函数 (API Layer) — 业务逻辑与状态管理
 *   6. 模块导出
 */

#include "std_module.h"
#include "../include/app_adc_io.h"
#include "../include/app_power_io.h"
#include "../include/app_power_calc_io.h"
#include "../include/app_power_defs.h"

#include "data_type.h"
#include "API_HRTIM.h"
#include "S_PID.h"
#include "s_data_stack.h"
#include "pluse.H"
#include "phase.h"
#include "Simulative_Uart.h"
#include "printMessage.h"

/* ================================================================
 * 外部函数声明 — 实现在 app_power.c (硬件访问层)
 * ================================================================ */

/* PPG 硬件操作 (每炉头独立函数) */
void PPGinit(void);
void PPGdeadTimeCh1(uint8_t downDts, uint8_t upDts);
void PPGdeadTimeCh2(uint8_t downDts, uint8_t upDts);
void PPGdeadTimeCh3(uint8_t downDts, uint8_t upDts);
void PPGdeadTimeCh4(uint8_t downDts, uint8_t upDts);
void PPGsetValueCh1(PPGvalueDef input);
void PPGsetValueCh2(PPGvalueDef input);
void PPGsetValueCh3(PPGvalueDef input);
void PPGsetValueCh4(PPGvalueDef input);
void PPGsetDutyCh1(uint16_t duty);
void PPGsetDutyCh2(uint16_t duty);
void PPGsetDutyCh3(uint16_t duty);
void PPGsetDutyCh4(uint16_t duty);
PPGvalueDef PPGgetValueCh1(void);
PPGvalueDef PPGgetValueCh2(void);
PPGvalueDef PPGgetValueCh3(void);
PPGvalueDef PPGgetValueCh4(void);
void PPGonOffCh1(uint8_t onOff);
void PPGonOffCh2(uint8_t onOff);
void PPGonOffCh3(uint8_t onOff);
void PPGonOffCh4(uint8_t onOff);
void PPGgetAdcValueCh1(void);
void PPGgetAdcValueCh2(void);
void PPGgetAdcValueCh3(void);
void PPGgetAdcValueCh4(void);
void PPGgetAdcValue_ch1(void);

/* 检锅脉冲计数 (每炉头独立) */
void APP_POWER_PanCountInitCh1(void);
void APP_POWER_PanCountInitCh2(void);
void APP_POWER_PanCountInitCh3(void);
void APP_POWER_PanCountInitCh4(void);
uint8_t APP_POWER_PanCountGetValue(void);
void APP_POWER_PanCountSetValue(uint8_t onOff);

/* BK 标志 (每炉头独立) */
uint8_t API_PPG_BkFlag_Pot1(void);
uint8_t API_PPG_BkFlag_Pot2(void);
uint8_t API_PPG_BkFlag_Pot3(void);
uint8_t API_PPG_BkFlag_Pot4(void);

/* I2C 通讯函数 */
uint8_t I2cSuccessCount(void);
void AdcIrqHandleWatchDogLock(void);

/* ADC 接口 (Pair F) */
uint32_t Adc_GetPowerTxa(uint8_t ch);
extern void Adc_TxaAvgReset(uint8_t ch);

/* 功率控制硬件接口 (实现在 app_power.c / power_calc_half.c) */
uint8_t check_pot_in(void);
INT8U getMaxPowerDiv(void);

/* 时间接口 */
extern uint8_t Time_GetMs100Flg(void);
extern uint8_t Time_GetSecFlg(void);

/* PPG 起振/检锅硬件接口 */
void PPGsetHalf(uint8_t ch, uint16_t fre);
void API_HRTIM_CHECK_PAN_PLUSE(uint8_t ch);
void API_HRTIM_PanOffCallBack(void);

/* 脉冲检测共享状态 (app_power.c DMA/FMAC 硬件写入，power_base 读取) */
typedef struct {
    uint8_t  ch;
    uint8_t  res;
    uint8_t  pulse_count;
    uint16_t res16;
} RealTimePulseDetector __attribute__((aligned(32)));
extern RealTimePulseDetector PanPluse;

/* ================================================================
 * 枚举 / 常量
 * ================================================================ */

/* ---- PowerStautsEnumDef: 功率状态枚举 ---- */
enum {
    PowerOffStatus      = 0,
    PowerCheckPanStatus,
    PowerCheckFreqStatus,
    PowerPotInStatus,
    PowerOnFreqStatus,
    PowerOnDutyStatus,
};

#define POTNUM  4
#define MasterCh 0
#define SlaveCh  1
#define MasterRCh 2
#define SlaveRCh  3

enum {
    POWER_CHANGE_PAUSE = 0,
    POWER_CHANGE_ZERO,
    POWER_CHANGE_DUTY,
    POWER_CHANGE_CYCLE,
    POWER_CHANGE_CYCLE_CHANGE,
    POWER_CHANGE_CYCLE_RESET
};

#define PPG_MAX_RANGE   200
#define C_KEEP_TIME     5
#define PPG_ON          1
#define PPG_OFF         0
#define C_MAIN_ERR_TIME 4

enum {
    C_POT_IN   = 0,
    C_POT_MAY  = 1,
    C_POT_ERR  = 2,
    C_MAIN_ERR = 4,
    SWITCH_OFF         = 0,
    SWITCH_OFF_NO_POT  = 1,
    SWITCH_OFF_POT     = 2,
    SWITCH_PAUSE       = 3,
};

#define C_POWERUP_SUDDENLY   0x012
#define C_POWERDOWN_SUDDENLY 0x12
#define C_PEN_CHECK_TIME     0x00
#define C_PEN_MOVE_TIME      3
#define C_POWER_DATA_RACE    0x04

#define C_CHECKPAN_REQUESTOFF 0x0
#define C_CHECKPAN_PPG_ON     3
#define C_CHECKPAN_PPG_OFF    0x2
#define C_CHECKPAN_GET_PLUSE  0xE0

#define IRON_POWER_H    (1000/25)
#define MIN_BASE_COUNT  50

/* 调试开关 */
/* #define DEBUG_POWER_OUT */
/* #define DEBUG_PAN_IN    */

/* 相位枚举 */
enum {
    PhaseProtectValue = 5,
    PhaseLimitValue   = 10,
    PhaseResumeValue  = 20,
};
enum {
    phaseCancle     = 0x1f,
    PhaseProtect    = 1,
    PhaseLimit      = 2,
    PhaseProtectDiv = 0x11,
    PhaseLimitDiv   = 0x12,
};

/* 标志位 */
#define F_POWER_FAST_UP    1
#define F_POWER_FAST_DOWN  2
#define F_POWER_OVER       3
#define F_SURGE_OVER       4
#define F_POT_ERR          5

#define B_PPGMAX   _BIT0
#define B_VCOUT    _BIT1
#define B_LOWV     _BIT2
#define B_PWDEAD   _BIT3

#define B_INIT_SUC_FLAG   _BIT7
#define B_POW_STB_FLAG    _BIT6
#define B_POW_ARRIVE_FLAG _BIT5
#define B_PAN_ADJ_FLAG    _BIT4

/* ================================================================
 * 数据实体 — 全部 static，零 extern
 * ================================================================ */

AppPowerDef             PowerMem[POTNUM];
PowerInputDef           PowerInput[POTNUM];
AppPowerStaticDef       PowerStaticReg[POTNUM];
AppPowerKeepDef         PowerKeepReg[POTNUM];
AppPowerDef*            PowerControl;
int                     PidReturn[4];
static INT8U            s_ppg_encoded[POTNUM];  /* s_ppg_fun 编码输出 per channel */

AppPowerAllDef           PowerAll;

/* ---- v2.2 PULL 私有 I/O ---- */
static PowerBase_Input_t   s_in;
static PowerBase_Output_t  s_out;

/* ================================================================
 * 宏 — 通过 PowerControl 间接访问当前炉头数据
 * ================================================================ */

#define PowerPotReset       PowerAll.flag.bit.rest
#define PowerPotCheckEnd    PowerAll.flag.bit.potCheckEnd
#define PowerOvpValueAll    PowerAll.ovp
#define PowerChangeStatus   PowerAll.changeStatus
#define PowerScrCnt         PowerAll.scrCnt
#define PowerPotNum         PowerAll.PotNum
#define PowerMinFre         PowerAll.minFre
#define PowerCycle          PowerAll.pCycle
#define PowerStack          PowerAll.stack

#define g_surge_power       PowerControl->staticReg->SurgePower
#define s_leave_time        PowerControl->staticReg->LeaveTime
#define vcout_delay         PowerControl->staticReg->VcoutDelay
#define PowerOvpValue       PowerControl->staticReg->OvpValue
#define g_surge_delay       PowerControl->staticReg->SurgeDelay
#define power_half_adj      PowerControl->staticReg->power_half_adj
#define s_limit_max_count   PowerControl->staticReg->LimitMaxCount
#define s_limit_voltage     PowerControl->staticReg->LimitVoltage
#define s_limit_Qvalue      PowerControl->staticReg->limitQ
#define s_limit_Qsum        PowerControl->staticReg->limitQSum
#define s_ppg_limit_power   PowerControl->staticReg->ppgLimitPower
#define CheckPanStep        PowerControl->staticReg->checkPanStep
#define MaxPowerDiv         PowerControl->staticReg->DivMaxPowerRes
#define PowerHalfCnt        PowerControl->staticReg->powerHalfCnt

#define CurrentValue16      PowerControl->staticReg->current16
#define PhaseSumValue       PowerControl->staticReg->phaseSumValue
#define power_arrive        PowerControl->staticReg->PowerArrive
#define s_power_surge       PowerControl->staticReg->PowerSurge
#define PpgValue            PowerControl->staticReg->ppgValue
#define g_power_adc_fact    PowerControl->staticReg->PowerAdcFact
#define powerAdcFactTxa     PowerControl->staticReg->PowerTxaFact
#define powerPhase          PowerControl->staticReg->phaseValue

#define s_ppg_limit         PowerControl->staticReg->PpgLimit
#define s_ppg_limit_max     PowerControl->staticReg->ppgLimitMax
#define s_ppg_power_adj     PowerControl->staticReg->ppgPowerAdj

#define g_power_adc_trig    PowerControl->staticReg->PowerAdcTrig
#define g_power_duty        PowerControl->staticReg->PowerDuty
#define g_duty_actual       PowerControl->staticReg->PPGdutyActual
#define g_power_cycle       PowerControl->staticReg->powerCycle

#define PowerCycleDoubleOn    PowerControl->staticReg->cycleChange.doubleOn
#define PowerCycleDoubleCnt   PowerControl->staticReg->cycleChange.doubleCount
#define PowerCycleBaseCnt     PowerControl->staticReg->cycleChange.baseCount
#define PowerCycleDoubleDuty  PowerControl->staticReg->cycleChange.doubleDuty
#define PowerCycleBaseDuty    PowerControl->staticReg->cycleChange.baseDuty
#define PowerCycleDoublePower PowerControl->staticReg->cycleChange.doublePower
#define PowerCycleBasePower   PowerControl->staticReg->cycleChange.basePower
#define PowerDeadCnt          PowerControl->staticReg->DeadCnt

#define PowerPpgSave        PowerControl->keepReg->ppgSave
#define g_valtage_210_buf   PowerControl->keepReg->Valtage210
#define g_p25_ad            PowerControl->keepReg->Power25wAd
#define Power_channel       PowerControl->keepReg->channel
#define PowerPid            PowerControl->keepReg->PowerPIDstr
#define MaxPowerSet         PowerControl->keepReg->SetMaxPower
#define g_panc_time         PowerControl->keepReg->PancTime

#define g_off_flag          PowerControl->staticReg->PowerOffFlag
#define g_power_limit_flag  PowerControl->staticReg->PowerLimitFlag

#define m_power_duty50      PowerControl->staticReg->flag.bit.PowerDuty50
#define m_power_hold_max    PowerControl->staticReg->flag.bit.PowerHoldMax
#define m_power_hold_min    PowerControl->staticReg->flag.bit.PowerHoldMin
#define m_power_pause_flag  PowerControl->staticReg->flag.bit.PowerPause
#define m_load_check_pan    PowerControl->staticReg->flag.bit.LoadCheckPan
#define m_ic_vc_adc_ok_flag PowerControl->staticReg->flag.bit.IcVcAdcOk
#define m_power_resume_flag PowerControl->staticReg->flag.bit.powerResume
#define m_ppg_on            PowerControl->staticReg->flag.bit.ppgOn
#define m_power_cycle_flag  PowerControl->staticReg->flag.bit.PowerCycleType
#define m_pot_type          PowerControl->staticReg->flag.bit.PotType
#define m_check_pan_flag    PowerControl->staticReg->flag.bit.CheckPan
#define m_ppg_add_flag      PowerControl->staticReg->flag.bit.ppgAdd
#define m_dis_voltage_flag  PowerControl->staticReg->flag.bit.DisVoltage
#define m_power_off_flag    PowerControl->staticReg->flag.bit.PowerOff
#define m_ppg_work_flag     PowerControl->staticReg->flag.bit.ppgWork
#define m_vcout_flag        PowerControl->staticReg->flag.bit.Vcout
#define m_ppg_lock_flag     PowerControl->staticReg->flag.bit.ppgLock

#define FunPPGsetDuty       PowerControl->funAdr->_PPGsetDuty
#define FunPPGgetValue      PowerControl->funAdr->_PPGgetValue
#define FunDeadTimeSetValue PowerControl->funAdr->_PPGdeadTime
#define FunPPGonOff         PowerControl->funAdr->_PPGonOff
#define FunPanCountInit     PowerControl->funAdr->_PanCountInit
#define FunPanCountReset()  PowerControl->funAdr->_PanCountSetValue(1)
#define FunPanCountGetValue PowerControl->funAdr->_PanCountGetValue
#define FunPanCountSetValue PowerControl->funAdr->_PanCountSetValue
#define FunPPGgetAdcValue   PowerControl->funAdr->_PPGgetAdcValue
#define FunTimBkFlag        PowerControl->funAdr->_TimBkFlag

#define IHStatus            PowerControl->input->status.ihStatus

#define VoltageValue    PowerControl->input->status.voltageAd
#define CurrentValue    PowerControl->input->status.currentAd
#define IGBTValue       PowerControl->input->status.igbtAd
#define BOTTOMValue     PowerControl->input->status.bottomAd
#define TOPValue        PowerControl->input->status.topAd
#define ActualPower     PowerControl->input->status.actualPowerDiv25
#define TargetPower     PowerControl->input->status.targetPowerDiv25
#define ActualPPG       PowerControl->input->status.actualPPG
#define PowerStatus     PowerControl->input->status.powerStatus
#define LoadValue       PowerControl->input->status.loadValue
#define VCNTValue       PowerControl->input->status.vcountValue
#define EquivalentRes   PowerControl->input->status.equivalentResistance
#define PWMValue_L      PowerControl->input->status.equivalentResistance
#define PWMValue_H      PowerControl->input->status.res2
#define CURRENT_OFFSET  PowerControl->input->status.powerP25
#define VersionValue    PowerControl->input->status.checkSum

#define LoadTest        PowerControl->input->init.loadTest
#define OvpShort        PowerControl->input->init.ovpShort
#define LoadLeave       PowerControl->input->init.loadLeave
#define VC_LIMIT_MAX    PowerControl->input->init.vcLimitMax
#define LoadLeavePhase  PowerControl->input->init.loadLeavePhase
#define MinPhase        PowerControl->input->init.minPhase
#define PotPowerM       PowerControl->input->init.potPowerM
#define MaxPowerM       PowerControl->input->init.maxPowerM

#define power_control_set PowerControl->input->control.powerControlSet
#define power_switch      PowerControl->input->control.powerSwitch
#define power_setm        PowerControl->input->control.powerSetm
#define fan_speed_in      PowerControl->input->control.fanSpeed
#define k_value_in        PowerControl->input->control.kValue

/* ================================================================
 * ADC 通道枚举 (本地副本，索引 pAdc->inputValue[])
 * ================================================================ */
enum {
    AdcGroupT1A = 0,
    AdcGroupT2A = 1,
    AdcGroupT3A = 2,
    AdcGroupT4A = 3,
    AdcGroupPower1 = 4,
    AdcGroupPower2 = 5,
    AdcGroupPower3 = 6,
    AdcGroupPower4 = 7,
    AdcGroupVoltage = 8,
    AdcGroupIgbt1 = 9,
    AdcGroupIgbt2 = 10,
    AdcGroupBottom1 = 11,
    AdcGroupBottom2 = 12,
    AdcGroupBottom3 = 13,
    AdcGroupBottom4 = 14,
    AdcGroupCeilQ1 = 15,
    AdcGroupCeilQ2 = 16,
    AdcGroupCeilQ3 = 17,
    AdcGroupCeilQ4 = 18,
    AdcGroupPhase1 = 19,
    AdcGroupPhase2 = 20,
    AdcGroupPhase3 = 21,
    AdcGroupPhase4 = 22,
};

/* ================================================================
 * 函数指针表 — 指向 app_power.c 中的硬件实现
 * ================================================================ */
static const AppPowerFunDef Power1FunTable = {
    PPGinit, PPGdeadTimeCh1, PPGsetDutyCh1, PPGgetValueCh1,
    PPGonOffCh1, PPGgetAdcValueCh1,
    APP_POWER_PanCountInitCh1, APP_POWER_PanCountGetValue,
    APP_POWER_PanCountSetValue, API_PPG_BkFlag_Pot1,
};
static const AppPowerFunDef Power2FunTable = {
    PPGinit, PPGdeadTimeCh2, PPGsetDutyCh2, PPGgetValueCh2,
    PPGonOffCh2, PPGgetAdcValueCh2,
    APP_POWER_PanCountInitCh2, APP_POWER_PanCountGetValue,
    APP_POWER_PanCountSetValue, API_PPG_BkFlag_Pot2,
};
static const AppPowerFunDef Power3FunTable = {
    PPGinit, PPGdeadTimeCh3, PPGsetDutyCh3, PPGgetValueCh3,
    PPGonOffCh3, PPGgetAdcValueCh3,
    APP_POWER_PanCountInitCh3, APP_POWER_PanCountGetValue,
    APP_POWER_PanCountSetValue, API_PPG_BkFlag_Pot3,
};
static const AppPowerFunDef Power4FunTable = {
    PPGinit, PPGdeadTimeCh4, PPGsetDutyCh4, PPGgetValueCh4,
    PPGonOffCh4, PPGgetAdcValueCh4,
    APP_POWER_PanCountInitCh4, APP_POWER_PanCountGetValue,
    APP_POWER_PanCountSetValue, API_PPG_BkFlag_Pot4,
};

/* ================================================================
 * MODULE_SKELETON — 必须在所有使用 g_input/g_output 的函数之前
 * ================================================================ */
MODULE_SKELETON(PowerBase);

/* ================================================================
 * 前置声明
 * ================================================================ */
static void Init(void);
static void ProcessInput(void);
static void PowerMemInit(void);

/* -------------------------- 纯算法函数 -------------------------- */
static void PowerTypeFun(void);
static INT8U PowerControlFun(uint8_t chn);
static void surge_Processing(void);
static void getOvpValueAdj(void);
static void PanStatusCheck(void);
static INT16U power_con_fun(void);
static INT8U s_ppg_fun(void);
static void APP_POWER_PhaseHalfTypeSet(void);
static void APP_POWER_PpgHalfTypeSet(int t_ppgchange);
static void getMemSetValue(void);
static void get_MAXMIN_PPG(void);
static INT16U s_power_convert_bass(INT8U triger_powerm);
static void reset_ppg_limit(void);
static void reset_ppg_limit_ch(uint8_t i);
static void APP_POWER_PotCheckRest(void);
AppPowerDef* getPowerCHN(uint8_t chn);
static uint8_t APP_POWER_GetResumeFlag(void);
static INT8U s_power_equ_control(INT16U t_power_adc_trig);
static INT8U s_check_pan_leave(void);
static INT8U s_pan_err_adj(INT8U lens);
static INT8U s_pan_pot_check(INT8U temp);
static INT16U s_low_voltage_limit_current(INT16U trig_power_adc);

/* -------------------------- 硬件控制函数 -------------------------- */
static void s_pwm_off(INT8U off_num);
static void s_pwm_on(void);
static void s_pan_check_fun(void);
static void StartPPG(void);
void i_ppg_set_limit(INT16U t_corrent_ppg);

/* -------------------------- 中断回调函数 -------------------------- */
void API_HRTIM_PanOffCallBack(void);

/* -------------------------- API层函数 -------------------------- */

/* ---- 外部函数 (app_power.c HAL 层) ---- */
extern void APP_POWER_PotTypeCheckConfirm(void);
extern void APP_POWER_PotTypeCheck_FRE(uint8_t ch);
extern void APP_POWER_PotTypeDebugMsgOut(void);

/* ================================================================
 * 初始化参数表 (本地定义，零 extern)
 * ================================================================ */
static const unsigned char g_core_para_init[8] = {
    0x17,       /* 检锅 */
    0x90,       /* maxppg */
    600 / 25,   /* 锅具有效功率 */
    0x60,       /* 反压限制幅度 */
    0x10,       /* 检锅电流 */
    0x8f,       /* 电流修正系数 */
    400 / 25,   /* 最小功率 */
    3300 / 25   /* 最大功率 */
};

/* ================================================================
 * ========== 纯算法函数 (Pure Algorithm) ==========
 *
 * 特征：
 *   - 仅操作内存数据，无硬件API调用
 *   - 纯数学计算/数据处理逻辑
 *   - 可独立单元测试，与硬件解耦
 * ================================================================ */

/**
 * @brief 获取指定炉头的控制结构指针
 * @param chn 炉头索引 (0-3)
 * @return 炉头控制结构指针
 */
AppPowerDef* getPowerCHN(uint8_t chn)
{
    switch (chn) {
    case 0: PowerControl = &PowerMem[0]; break;
    case 1: PowerControl = &PowerMem[1]; break;
    case 2: PowerControl = &PowerMem[2]; break;
    case 3: PowerControl = &PowerMem[3]; break;
    default: PowerControl = NULL; break;
    }
    return PowerControl;
}

/**
 * @brief 从Flash读取参数设置
 */
static void getMemSetValue(void)
{
    if (PowerControl->input->flash != NULL) {
        CURRENT_OFFSET = PowerControl->input->flash->power25;
        g_p25_ad = CURRENT_OFFSET * 2;
    }
}

/**
 * @brief 获取最大最小PPG频率（桩函数）
 */
static void get_MAXMIN_PPG(void)
{
    /* stub — 频率限制计算，当前为空 */
}

/**
 * @brief 功率明码转换为16位数据
 * @param triger_powerm 功率明码
 * @return 转换后的16位功率数据
 */
static INT16U s_power_convert_bass(INT8U triger_powerm)
{
    INT16U t_convert_p;
    if (triger_powerm == 0) {
        triger_powerm = MaxPowerM;
    }
    t_convert_p = g_p25_ad;
    t_convert_p *= triger_powerm;
    return t_convert_p;
}

/**
 * @brief 低压限流计算
 * @param trig_power_adc 目标功率ADC值
 * @return 限制后的功率值
 */
static INT16U s_low_voltage_limit_current(INT16U trig_power_adc)
{
    INT16U t_tem_power;
    INT16U t_tem_power_max;
    INT8U  t_voltage_dalta;

    g_power_limit_flag &= ~B_LOWV;

    t_tem_power_max = trig_power_adc;

    if (VoltageValue < g_valtage_210_buf) {
        t_voltage_dalta = g_valtage_210_buf - VoltageValue;

        t_tem_power = t_tem_power_max;
        t_tem_power = t_tem_power / 64;
        t_tem_power = t_voltage_dalta * t_tem_power;

        g_power_limit_flag |= B_LOWV;

        if (t_tem_power_max > t_tem_power) {
            t_tem_power_max = t_tem_power_max - t_tem_power;
            if (t_tem_power_max < s_power_convert_bass(LoadLeave)) {
                t_tem_power_max = s_power_convert_bass(LoadLeave);
            }
            if (t_tem_power_max >= trig_power_adc) {
                t_tem_power_max = trig_power_adc;
            }
        } else {
            t_tem_power_max = s_power_convert_bass(LoadLeave);
        }
    }

    return t_tem_power_max;
}

/**
 * @brief 恒功率控制（保留桩函数）
 * @param t_power_adc_trig 目标功率ADC值
 * @return 控制结果
 */
static INT8U s_power_equ_control(INT16U t_power_adc_trig)
{
    /* 原实现位于 app_power_backup.c L3273-L3383。
     * 此函数调用链已在原代码中被注释（s_ppg_fun 改用 FixedPID_Compute），
     * 保留桩以备将来参考。 */
    (void)t_power_adc_trig;
    return 0;
}

/**
 * @brief 移锅检测算法
 * @return 锅具状态标志
 */
static INT8U s_check_pan_leave(void)
{
    INT8U   t_pan_flag;
    INT16U  t_pan_curr;
    INT16U  t_fre_25_per;
    INT16U  t_voltage_div;
    INT8U   t_load_leave;

    t_fre_25_per = STEEL_LOAD_FRE_PWM;
    if (m_pot_type == PotIron) {
        t_fre_25_per = IRON_LOAD_FRE_PWM;
    }

    t_voltage_div = 0;
    if (VoltageValue < g_valtage_210_buf) {
        t_voltage_div = g_valtage_210_buf - VoltageValue;
        t_pan_curr = CurrentValue;
        t_pan_curr *= g_valtage_210_buf;
    } else {
        t_pan_curr = ActualPower;
    }

    t_voltage_div *= 16;
    t_fre_25_per += t_voltage_div;

    if (t_fre_25_per > s_ppg_limit) {
        t_fre_25_per = s_ppg_limit;
    }

    t_pan_flag = C_POT_MAY;

    if (m_ic_vc_adc_ok_flag == 0) {
        return t_pan_flag;
    }

    if (g_surge_delay > APP_POWER_PAN_DELAY) {
        t_load_leave = LoadLeave;

        {
            uint8_t minReson = (MinPhase & 0x0F) * 4;
            minReson = 2;
            uint8_t t_panLeave = 0;
            if (minReson) {
                if (powerPhase > 75) {
                    t_panLeave = 1;
                }
            }

            if (t_panLeave) {
                /* 相位判移锅 — 当前禁用 */
            }
        }

        if (ActualPower > t_load_leave
            || (t_pan_curr > s_power_convert_bass(LoadLeave))) {
            t_pan_flag = C_POT_IN;
            s_leave_time = 0;
        } else {
            if (FunPPGgetValue().duty >= t_fre_25_per || m_vcout_flag) {
                t_pan_flag = s_pan_err_adj(C_PEN_MOVE_TIME);
            }
        }
    } else {
        m_vcout_flag = 0;
    }

    return t_pan_flag;
}

/**
 * @brief 无锅检测确认计数
 * @param lens 确认次数阈值
 * @return 锅具状态
 */
static INT8U s_pan_err_adj(INT8U lens)
{
    INT8U t_tem;
    t_tem = C_POT_MAY;
    if (s_leave_time >= lens) {
        t_tem = C_POT_ERR;
        g_surge_power = 0;
        if (LoadValue == 0x0f) {
            t_tem = C_MAIN_ERR;
        }
    } else {
        s_leave_time++;
    }
    return t_tem;
}

/**
 * @brief 锅具检测（保留桩函数）
 * @param temp 温度参数
 * @return 检测结果
 */
static INT8U s_pan_pot_check(INT8U temp)
{
    /* 原声明存在于 app_power_backup.c L942，但无实现体。
     * 保留桩以备将来使用。 */
    (void)temp;
    return 0;
}

/* ================================================================
 * ========== 硬件控制函数 (HAL_HALF) ==========
 *
 * 特征：
 *   - 直接调用 HAL API
 *   - 操作硬件寄存器/外设
 *   - 未来将迁移到 HAL_P 层
 * ================================================================ */

/**
 * @brief 启动检锅PPG起振
 */
static void StartPPG(void)
{
    FunPanCountInit();
    FunDeadTimeSetValue(DTS4US, DTS4US);
    PPGsetHalf(Power_channel, PAN_FRE_PWM);

    m_check_pan_flag = 1;
    CheckPanStep     = C_CHECKPAN_GET_PLUSE;
    g_panc_time      = 0;

    API_HRTIM_CHECK_PAN_PLUSE(Power_channel);
}

/* ================================================================
 * ========== 中断回调函数 (ISR Callbacks) ==========
 *
 * 特征：
 *   - 作为中断服务函数的回调入口
 *   - 执行时间敏感，应快速执行
 *   - 可能混合硬件操作和简单数据处理
 * ================================================================ */

/**
 * @brief HRTIM检锅结束回调
 */
void API_HRTIM_PanOffCallBack(void)
{
    FunPanCountReset();
}

/* ================================================================
 * ========== API层函数 (API Layer) ==========
 *
 * 特征：
 *   - 业务逻辑和状态管理
 *   - 协调硬件操作和算法调用
 *   - 提供对外接口
 * ================================================================ */

/**
 * @brief 检锅脉冲处理状态机
 */
static void check_pot_pluse(void)
{
    INT8U temp, temp1;

    if (Time_GetSecFlg()) {
        CheckPanStep = 0;
    }

    if (m_ppg_on) {
        return;
    }

    temp  = g_power_adc_trig / 256;
    temp1 = g_power_adc_trig;

    if (temp1 == 0x03) {
        return;
    }

    if ((temp) || (temp1 == 0x02)) {
        if (Time_GetSecFlg()) {
            g_panc_time++;
            CheckPanStep = 0;
        }

        if ((g_panc_time >= 0x0f) || (g_panc_time == ((LoadTest >> 4) & 0x0f))) {
            uint8_t checkPanDelay = C_CHECKPAN_PPG_ON;
            checkPanDelay += (Power_channel * 8);

            if (CheckPanStep < checkPanDelay) {
                CheckPanStep++;
            }

            if (CheckPanStep >= checkPanDelay) {
                StartPPG();
            }
        }
    } else {
        s_pwm_off(PowerOffCheckPan);

        if ((g_power_adc_trig & 0x0f) == 0x00) {
            g_panc_time = 0x0e;
        }
    }
}

/**
 * @brief 锅具检测判断
 * @return 锅具状态标志
 */
uint8_t check_pot_in(void)
{
    INT8U t_pan_flag, t_pan_cnt;

    t_pan_flag = C_POT_MAY;

    if (m_ppg_on) {
        return t_pan_flag;
    }

    if (CheckPanStep == C_CHECKPAN_GET_PLUSE && PanPluse.res == PanPluseEnd) {
        PanPluse.res  = PanCheckRest;
        CheckPanStep  = C_CHECKPAN_GET_PLUSE + 1;
        t_pan_cnt     = FunPanCountGetValue();

        if (t_pan_cnt >= 16) {
            t_pan_cnt = 15;
        }

        t_pan_cnt = ~t_pan_cnt;
        t_pan_cnt &= 0x0f;

        LoadValue &= 0xf0;
        LoadValue |= t_pan_cnt;

#ifdef DEBUG_POWER_OUT
        t_pan_cnt = 11;
#endif

        if ((t_pan_cnt > (LoadTest & 0x0f)) && (t_pan_cnt != 0x0f)) {
            g_surge_delay      = 0;
            t_pan_flag         = C_POT_IN;
            g_power_limit_flag = 0;
            s_leave_time       = 0;
        } else {
            t_pan_flag = s_pan_err_adj(C_PEN_CHECK_TIME);
        }
    }

    if ((LoadTest & 0x0f) != 0x0f) {
        if ((t_pan_flag == C_POT_IN) && (g_power_adc_trig & 0xfff0)) {
            s_pwm_on();
        }
    }

    return t_pan_flag;
}

/**
 * @brief 计算钢锅限制功率（纯算法）
 * @return 功率限制值
 */
INT8U getMaxPowerDiv(void)
{
    INT8U t_power_div = 0;
    t_power_div *= 4;
    if (MaxPowerM > t_power_div) {
        t_power_div = MaxPowerM - t_power_div;
    } else {
        t_power_div = MaxPowerM;
    }
    return t_power_div;
}

/**
 * @brief 关闭PPG输出（硬件控制）
 * @param off_num 关断模式
 */
static void s_pwm_off(INT8U off_num)
{
    if (off_num < 0x10 && off_num > 0) {
        CURRENT_OFFSET = off_num;
    }

    if (m_power_pause_flag) {
        PowerPpgSave = g_power_duty;
    } else {
        PowerPpgSave = 0;
    }

    if (m_ppg_on) {
        MemSetInt((uint32_t*)(PowerControl->staticReg), 0,
                  sizeof(AppPowerStaticDef) / sizeof(int));
    }

    m_ppg_on      = 0;
    FunTimBkFlag();
    g_duty_actual = OFF_FRE_PWM;
    m_power_off_flag = 1;
    MaxPowerSet   = getMaxPowerDiv();
}

/**
 * @brief 开启PPG输出（硬件控制）
 */
static void s_pwm_on(void)
{
    if (m_ppg_on == 0) {
        if (PowerPpgSave) {
            g_power_duty = PowerPpgSave;
        } else {
            g_power_duty = START_FRE_PWM;
        }

        m_ppg_on           = 1;
        m_dis_voltage_flag = 1;
        MaxPowerSet        = getMaxPowerDiv();
        FunTimBkFlag();
        g_surge_delay      = 0;
        vcout_delay        = 0;
    }
}

/**
 * @brief 获取恢复标志处理
 * @return 恢复标志状态
 */
static uint8_t APP_POWER_GetResumeFlag(void)
{
    uint8_t xReturn = 0;
    for (uint8_t i = 0; i < POTNUM; i++) {
        if (PowerMem[i].staticReg->flag.bit.powerResume) {
            PowerMem[i].staticReg->flag.bit.powerResume = 0;
            xReturn++;
        }
    }
    if (DataStruct_GetStackTop(&PowerStack) <= 1) {
        xReturn = 0;
    }
    return xReturn;
}

/* ================================================================
 * Init — 模块初始化
 * ================================================================ */
static void PowerMemInit(void)
{
    MemSetInt((uint32_t*)(PowerControl->staticReg), 0,
              sizeof(AppPowerStaticDef) / sizeof(int));

    MaxPowerSet = 3500 / 25;
    g_valtage_210_buf = C_VOLTAGE_210V;
    g_p25_ad = C_P25W * 2;

    memcpy(&(PowerControl->input->init), g_core_para_init, 8);

    m_ppg_on = 0;
    FixedPID_Init(&PowerPid, 0.1, 0.01, 0.00, 1000);
    g_power_cycle = PowerCycle;
}

static void Init(void)
{
    uint8_t ch;

    /* 绑定 per-head 数据指针 */
    PowerMem[0].keepReg   = &PowerKeepReg[0];
    PowerMem[0].staticReg = &PowerStaticReg[0];
    PowerMem[0].funAdr    = (AppPowerFunDef*)&Power1FunTable;
    PowerMem[0].input     = &PowerInput[0];

    PowerMem[1].keepReg   = &PowerKeepReg[1];
    PowerMem[1].staticReg = &PowerStaticReg[1];
    PowerMem[1].funAdr    = (AppPowerFunDef*)&Power2FunTable;
    PowerMem[1].input     = &PowerInput[1];

    PowerMem[2].keepReg   = &PowerKeepReg[2];
    PowerMem[2].staticReg = &PowerStaticReg[2];
    PowerMem[2].funAdr    = (AppPowerFunDef*)&Power3FunTable;
    PowerMem[2].input     = &PowerInput[2];

    PowerMem[3].keepReg   = &PowerKeepReg[3];
    PowerMem[3].staticReg = &PowerStaticReg[3];
    PowerMem[3].funAdr    = (AppPowerFunDef*)&Power4FunTable;
    PowerMem[3].input     = &PowerInput[3];

    /* 逐炉头初始化 */
    ch = PotCh1;
    PowerControl  = &PowerMem[ch];
    Power_channel = ch;
    PowerMemInit();

    ch = PotCh2;
    PowerControl  = &PowerMem[ch];
    Power_channel = ch;
    PowerMemInit();

    ch = PotCh3;
    PowerControl  = &PowerMem[ch];
    Power_channel = ch;
    PowerMemInit();

    ch = PotCh4;
    PowerControl  = &PowerMem[ch];
    Power_channel = ch;
    PowerMemInit();

    MemSetInt((uint32_t*)(&PowerAll), 0, sizeof(AppPowerAllDef) / sizeof(int));

    PowerCycle  = PAN_FRE_PWM * 2;
    PowerScrCnt = 0;
    DataStruct_StackInit(&PowerStack);

    /* v2.2 PULL: 绑定 I/O 缓冲区 */
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

/* ================================================================
 * reset_ppg_limit — 功率变化后清限制
 * ================================================================ */
static void reset_ppg_limit_ch(uint8_t i)
{
    uint16_t minFre = PowerMinFre;
    if (i < POTNUM) {
        PowerMem[i].staticReg->LimitMaxCount = 0;
        PowerMem[i].staticReg->PpgLimit      = minFre;
        PowerMem[i].staticReg->ppgLimitMax   = minFre;
        PowerMem[i].staticReg->PowerAdcTrig  = 0;
        PowerMem[i].staticReg->ppgLimitPower = 0;
        PowerMem[i].staticReg->ppgPowerAdj   = minFre;
        PowerMem[i].staticReg->LimitVoltage   = 0;
#pragma push_macro("power_half_adj")
#undef power_half_adj
        PowerStaticReg[i].power_half_adj = 0;
#pragma pop_macro("power_half_adj")
        PowerMem[i].staticReg->LimitMaxCount  = 0;
        PowerMem[i].staticReg->limitQ         = 0;
        PowerMem[i].staticReg->PowerLimitFlag = 0;
        PowerMem[i].staticReg->DeadCnt        = 0;
        PowerMem[i].staticReg->cycleChange.baseDuty   = START_FRE_PWM;
        PowerMem[i].staticReg->cycleChange.doubleDuty = START_FRE_PWM;
    }
}

static void reset_ppg_limit(void)
{
    s_ppg_limit_power = TargetPower;
    s_limit_max_count = 0;
    PowerMinFre       = MIN_FRE_PWM;
    s_ppg_limit       = PowerMinFre;
    s_ppg_limit_max   = PowerMinFre;
    s_limit_voltage   = VoltageValue;
    s_limit_Qvalue    = 0;
    g_power_limit_flag = 0;
    PowerDeadCnt       = 0;
    PowerCycleBaseDuty   = START_FRE_PWM;
    PowerCycleDoubleDuty = START_FRE_PWM;
    PhaseController_Init(&PowerControl->staticReg->potPhase);
}

static void APP_POWER_PotCheckRest(void)
{
    PowerMinFre     = MIN_FRE_PWM;
    PowerPotCheckEnd = 0;

    for (uint8_t i = 0; i < POTNUM; i++) {
        reset_ppg_limit_ch(i);
        PowerMem[i].staticReg->SurgeDelay = 0;
        PowerMem[i].staticReg->flag.bit.PowerCycleType = 0;
        PowerMem[i].staticReg->flag.bit.PotType = PotSteel;
        PowerMem[i].staticReg->PPGdutyActual = START_FRE_PWM;
        PowerMem[i].staticReg->cycleChange.baseDuty   = START_FRE_PWM;
        PowerMem[i].staticReg->cycleChange.doubleDuty = START_FRE_PWM;
        PowerMem[i].staticReg->potPowerSave[0].count = 0;
        PowerMem[i].staticReg->potPowerSave[1].count = 0;
        PowerMem[i].staticReg->potPowerSave[2].count = 0;
        Adc_TxaAvgReset(i);
    }
}

/* ================================================================
 * ProcessInput — 主入口，由 DoWork 每 1ms 调用
 * ================================================================ */
static void ProcessInput(void)
{
    uint8_t ch;

    /* ====== 输入段: 检查 InputCallback 是否注入了新数据 ====== */
    if (!(g_input.info.status & ST_NEW))
        return;

    {
        PowerBase_Input_t *in  = (PowerBase_Input_t *)g_input.para;
        PowerBase_Output_t *out = (PowerBase_Output_t *)g_output.para;
        AdcBlock_t *pAdc = in->pAdc;

        /* ---- 分发 ADC 数据到 PowerMem[ch] ---- */
        for (ch = 0; ch < POTNUM; ch++) {
            PowerControl = &PowerMem[ch];

#ifdef CurrentFromTxa
            PowerControl->staticReg->current16 = pAdc->inputValue[AdcGroupT1A + ch];
            PowerControl->input->status.currentAd = PowerControl->staticReg->current16 >> 2;
#else
            PowerControl->staticReg->current16 = pAdc->inputValue[AdcGroupPower1 + ch];
            PowerControl->input->status.currentAd = PowerControl->staticReg->current16 >> 4;
#endif
            PowerControl->input->status.voltageAd = pAdc->inputValue[AdcGroupVoltage] >> 4;
            PowerControl->staticReg->PowerTxaFact = Adc_GetPowerTxa(PotCh1 + ch);
            PowerControl->staticReg->phaseValue   = pAdc->inputValue[AdcGroupPhase1 + ch];
            PowerControl->staticReg->limitQSum    = pAdc->inputValue[AdcGroupCeilQ1 + ch];
            PowerControl->staticReg->PowerTxaFact >>= 6;

            /* ---- MODBUS 命令: 从 head[ch] 填入 ---- */
            PowerControl->input->control.powerSetm  = in->head[ch].target_power;
            PowerControl->input->control.powerSwitch = in->head[ch].power_on ? 0x03 : 0x00;
        }

        g_input.info.status &= ~ST_NEW;

        /* ====== 计算段: 按炉头调用控制链 ====== */
        PowerTypeFun();

        /* ====== 输出段: 读取编码 PPG delta → PowerBase_Output_t ====== */
        for (ch = 0; ch < POTNUM; ch++) {
            out->head[ch].ppg_delta   = s_ppg_encoded[ch];
            out->head[ch].delta_valid = (s_ppg_encoded[ch] != 0xff) ? 1 : 0;
            out->head[ch].power_state = (uint8_t)PowerMem[ch].staticReg->flag.bit.ppgOn;
        }
        out->any_active = (PowerPotNum > 0) ? 1 : 0;

        g_output.info.status |= ST_OUT;
    }
}

/* ================================================================
 * PowerTypeFun — 多炉头调度
 * ================================================================ */
static void PowerTypeFun(void)
{
    if (I2cSuccessCount()) {
        AdcIrqHandleWatchDogLock();
    }

    for (uint8_t i = 0; i < POTNUM; i++) {
        if (getPowerCHN(i) == NULL) {
            break;
        }
        s_ppg_encoded[i] = PowerControlFun(i);
    }

#ifdef DebugOutPc
    {
        static uint32_t delayCount;
        delayCount++;
        if (delayCount > 100) {
            delayCount = 0;
            uint8_t j = 0;
            setDebugOutBuff(PowerMem[0].input->status.voltageAd, j++);
            setDebugOutBuff(PowerMem[0].input->status.currentAd, j++);
            setDebugOutBuff(PowerMem[0].input->status.topAd, j++);
            setDebugOutBuff(PowerMem[0].input->status.actualPowerDiv25, j++);
            setDebugOutBuff(PowerMem[0].input->status.targetPowerDiv25, j++);
            setDebugOutBuff(PowerMem[0].input->status.powerStatus, j++);
            setDebugOutBuff(PowerMem[0].input->status.equivalentResistance, j++);
            setDebugOutBuff(0x2, j++);
            setDebugOutBuff(PowerMem[1].input->status.currentAd, j++);
            setDebugOutBuff(PowerMem[1].input->status.topAd, j++);
            setDebugOutBuff(PowerMem[1].input->status.actualPowerDiv25, j++);
            setDebugOutBuff(PowerMem[1].input->status.targetPowerDiv25, j++);
            setDebugOutBuff(PowerMem[1].input->status.powerStatus, j++);
            setDebugOutBuff(PowerMem[1].input->status.equivalentResistance, j++);
            UARTx_SendValueClass(Debug_OutToPc());
        }
    }
#endif
}

/* ================================================================
 * PowerControlFun — 单炉头功率控制链
 *
 * 调用顺序: surge → OVP → panCheck → panRemoval → powerMgmt → PID → PPG apply
 * ================================================================ */
static INT8U PowerControlFun(uint8_t chn)
{
    if (getPowerCHN(chn) == NULL) {
        return 0xff;
    }

#ifdef DEBUG_POWER_OUT
    CurrentValue = 0x20;
    if (g_surge_delay > 10) {
        if (Power_channel == PotChWork) {
            CurrentValue = 0x40;
        }
    }
    VoltageValue = 0x9c;
#endif

    surge_Processing();
    getOvpValueAdj();

    /* ---- 检锅 ---- */
    PanStatusCheck();
    s_pan_check_fun();

    /* ---- 功率管理 + PID ---- */
    power_con_fun();
    {
        INT8U ppg_delta = s_ppg_fun();
        m_ic_vc_adc_ok_flag = 0;
        return ppg_delta;
    }
}

/* ================================================================
 * surge_Processing — 浪涌保护
 * ================================================================ */
static void surge_Processing(void)
{
    if (FunTimBkFlag()) {
        s_power_surge = 0x10;
        if (m_ppg_on) {
            s_pwm_off(PowerOffSurge);
            g_panc_time = 0x0d;
            g_off_flag  = F_SURGE_OVER;
        }
    }
}

/* ================================================================
 * getOvpValueAdj — 设置谐振电流过流 CMP 保护值
 * ================================================================ */
static void getOvpValueAdj(void)
{
    uint16_t valueVc = VC_LIMIT_MAX;
    PowerOvpValue = valueVc;
}

/* ================================================================
 * PanStatusCheck — 检锅状态检查
 * ================================================================ */
static void PanStatusCheck(void)
{
    static uint8_t s_main_err_cnt;

    switch (check_pot_in()) {
    case C_MAIN_ERR:
        s_main_err_cnt++;
        if (s_main_err_cnt > C_MAIN_ERR_TIME) {
            IHStatus = (IHStatus & 0xe0) + C_ERR_MAIN;
            IHStatus &= (~B_PAN_ADJ_FLAG);
        }
        break;

    case C_POT_IN:
        IHStatus &= (~B_PAN_ADJ_FLAG);
        s_main_err_cnt = 0;
        if ((IHStatus & 0x0f) == C_ERR_MAIN) {
            IHStatus &= 0xe0;
        }
        break;

    case C_POT_MAY:
        break;

    case C_POT_ERR:
        IHStatus |= B_PAN_ADJ_FLAG;
        s_main_err_cnt = 0;
        if ((IHStatus & 0x0f) == C_ERR_MAIN) {
            IHStatus &= 0xf0;
        }
        break;

    default:
        break;
    }
}

/* ================================================================
 * s_pan_check_fun — 移锅检测 + 锅具类型识别
 * ================================================================ */
static void s_pan_check_fun(void)
{
    INT8U t_pan_flag, temp;

    t_pan_flag = C_POT_MAY;

    temp = g_power_adc_trig >> 4;
    if (temp == 0) {
        temp = g_power_adc_trig;
        if (temp != SWITCH_PAUSE) {
            m_ppg_lock_flag = 0;
            g_surge_power   = 0;
            s_pwm_off(PowerOffZero);
            g_off_flag  = 0x0e;
            g_panc_time = 0x0e;
        }
    }

    if (m_ppg_on) {
        if (Time_GetMs100Flg()) {
            if (g_surge_delay < 200) {
                g_surge_delay++;
            }
        }

        if (g_surge_delay == POT_TYPE_DELAY1) {
            APP_POWER_PotTypeCheck_FRE(0);
        }

        if (g_surge_delay == POT_TYPE_DELAY1 + 1) {
            g_power_duty = POTTYPE2_FRE_PWM;
        }

        if (g_surge_delay == POT_TYPE_DELAY2) {
            APP_POWER_PotTypeCheck_FRE(1);
        }

        if (g_surge_delay > POT_TYPE_DELAY2 + 2) {
            if (ActualPower > IRON_POWER_H
                || g_power_limit_flag & (B_PPGMAX | B_PWDEAD)) {
                APP_POWER_PotTypeCheck_FRE(2);
            }
            APP_POWER_PotTypeCheckConfirm();
        }

        t_pan_flag = s_check_pan_leave();

#if DEBUG_PAN_IN
        t_pan_flag = C_POT_IN;
#endif
        t_pan_flag = C_POT_IN;

        if (t_pan_flag >= C_POT_ERR) {
            s_pwm_off(PowerOffNoPan);
            g_panc_time    = 0xd;
            m_ppg_lock_flag = 0;
            g_off_flag     = F_POT_ERR;
        }
        t_pan_flag = C_POT_MAY;
    } else {
        g_surge_delay = 0;
    }

    /* check_pot_pluse() is called per-head; original code called it from
     * the main loop context. Kept as-is for now. */
}

/* ================================================================
 * power_con_fun — 根据通讯数据计算目标功率
 * ================================================================ */
static INT16U power_con_fun(void)
{
    INT8U  temp;
    INT8U  t_power_switch, t_power_setm;
    uint8_t switchTemp;

    getMemSetValue();
    get_MAXMIN_PPG();

    if (power_setm > MaxPowerSet) {
        power_setm = MaxPowerSet;
    }

    t_power_setm   = power_setm;
    t_power_switch = power_switch;

    switchTemp = (t_power_switch & 0x0f);

    if (t_power_setm) {
        g_power_adc_trig = s_power_convert_bass(t_power_setm);
        if (t_power_switch & 0xf0) {
            switchTemp = 0x2;
        }
    } else {
        g_power_adc_trig = 0;
        if (t_power_switch == 0) {
            IHStatus &= (~B_PAN_ADJ_FLAG);
        }
    }

    temp = t_power_setm;

    if (TargetPower != temp) {
        IHStatus &= (~B_POW_STB_FLAG);
        power_half_adj = 0;
        FixedPIDclearIntegral(&PowerPid);
        if (TargetPower < temp && temp > 1500 / 25) {
            power_half_adj = TargetPower + (temp - TargetPower) / 2;
            PowerHalfCnt = 0;
        }
        m_power_resume_flag = 1;
        TargetPower = temp;
        reset_ppg_limit();

        if (TargetPower == 0) {
            DataStruct_StackDelet(&PowerStack, Power_channel + 1);
        } else {
            DataStruct_StackPush(&PowerStack, Power_channel + 1);
        }
    }

    if (power_half_adj) {
        g_power_adc_trig = s_power_convert_bass(power_half_adj);
    }

    g_power_adc_trig &= 0xfff0;
    g_power_adc_trig += switchTemp;

    return g_power_adc_trig;
}

/* ================================================================
 * s_ppg_fun — PID 计算 + 相位限制 + 同频/倍频切换
 *
 * 返回: ppg 调整量 (bit7=方向, bit6..0=变化量), 0xff=不调整
 * ================================================================ */
static INT8U s_ppg_fun(void)
{
    INT16U t_power_adc_trig;
    INT16U t_tem_power_new;
    INT16U t_tem_power_dalta;
    INT8U  t_tem_ppg_add = 0;
    INT8U  t_ic_value, t_vc_value;
    int    t_pidReturn;

    if (m_ic_vc_adc_ok_flag == 0) {
        return 0xff;
    }

    t_power_adc_trig = g_power_adc_trig;

#if 1
    t_ic_value      = CurrentValue;
    t_vc_value      = VoltageValue;
    t_tem_power_new = t_ic_value;
    t_tem_power_new *= t_vc_value;
#else
    t_tem_power_new = powerAdcFactTxa;
#endif

    t_tem_power_dalta = t_tem_power_new / g_p25_ad;

    if (t_tem_power_dalta > ActualPower) {
        if (ActualPower > MinPowerM) {
            /* s_power_dalta used elsewhere if needed */
        }
    }

    ActualPower      = t_tem_power_dalta;
    g_power_adc_fact = t_tem_power_new;

    if (!m_ppg_on) {
        return 0xff;
    }

    /* 过功率保护 */
    if (ActualPower > MaxPowerM + 8) {
        g_off_flag = F_POWER_OVER;
        return 0x96;
    }

    /* PID 计算 */
    t_pidReturn = FixedPID_Compute(&PowerPid, t_power_adc_trig,
                                   g_power_adc_fact, g_p25_ad);
    PidReturn[Power_channel] = t_pidReturn;
    t_tem_ppg_add = abs(t_pidReturn);

    /* 相位限制 */
    APP_POWER_PhaseHalfTypeSet();

    /* 死区判稳 */
    {
        uint16_t t_powerDead = abs(TargetPower - ActualPower);
        if (t_powerDead <= 1 && t_tem_ppg_add < 1) {
            if (power_half_adj == 0) {
                PowerDeadCnt++;
                if (PowerDeadCnt > 5) {
                    PowerDeadCnt = 5;
                    g_power_limit_flag |= B_PWDEAD;
                }
            }
        } else {
            if (PowerDeadCnt >= 5) {
                PowerDeadCnt = 3;
            } else {
                PowerDeadCnt = 0;
            }
            g_power_limit_flag &= ~B_PWDEAD;

            /* 倍频/同频切换 — 仅在非死区时调用 */
            APP_POWER_PpgHalfTypeSet((int)t_pidReturn);
        }
    }

    /* 半功率逼近: pidReturn < 2 时逐步清除 power_half_adj */
    if (t_pidReturn < 2) {
        if (power_half_adj) {
            PowerHalfCnt++;
            if (PowerHalfCnt > 50) {
                PowerHalfCnt  = 0;
                power_half_adj = 0;
                g_surge_delay  = APP_POWER_PAN_DELAY - 10;
            }
        }
    }

    /* 编码方向 + 锁升/锁降保护 */
    if (t_pidReturn < 0) {
        t_tem_ppg_add |= 0x80;
        if (m_power_hold_min) {
            t_tem_ppg_add = 0;
        }
    } else {
        if (m_power_hold_max) {
            t_tem_ppg_add = 0;
        }
    }

    return t_tem_ppg_add;
}

/* ================================================================
 * APP_POWER_PhaseHalfTypeSet — 相位限制 → 锁升/锁降
 * ================================================================ */
static void APP_POWER_PhaseHalfTypeSet(void)
{
    uint8_t  phaseType = 0;
    uint16_t duty, peroid;

#ifdef DEBUG_POWER_OUT
    powerPhase = 20;
#endif
    uint16_t t_phase = powerPhase / 10;
    t_phase = 20;
    if (t_phase <= PhaseProtectValue) {
        phaseType = PhaseProtect;
    } else if (t_phase > PhaseResumeValue) {
        phaseType = phaseCancle;
    }

    if (PhaseSumValue > 0xA0) {
        phaseType = PhaseLimit;
    }

    duty   = FunPPGgetValue().duty * 2 + 10;
    peroid = FunPPGgetValue().prioed;

    if (duty < peroid) {
        phaseType |= 0x10;
    }

    switch (phaseType) {
    case PhaseProtect:
    case PhaseLimit:
        m_power_hold_max = 1;
        m_power_hold_min = 0;
        break;
    case PhaseProtectDiv:
    case PhaseLimitDiv:
        m_power_hold_min = 1;
        m_power_hold_max = 0;
        break;
    default:
        m_power_hold_min = 0;
        m_power_hold_max = 0;
        break;
    }
}

/* ================================================================
 * APP_POWER_PpgHalfTypeSet — 倍频/同频切换策略
 * ================================================================ */
static void APP_POWER_PpgHalfTypeSet(int t_ppgchange)
{
    PPGvalueDef ppgValue = FunPPGgetValue();

    if (m_power_cycle_flag) {
        PowerCycleDoubleDuty  = FunPPGgetValue().duty;
        PowerCycleDoublePower = ActualPower;
    } else {
        PowerCycleBaseDuty  = FunPPGgetValue().duty;
        PowerCycleBasePower = ActualPower;
    }

    /* 钢锅强制倍频 */
    if (m_pot_type == PotSteel) {
        if (PowerMinFre == MID_FRE_PWM) {
            if (PowerCycle > MIN_FRE_PWM * 2) {
                m_power_cycle_flag = 1;
                return;
            } else {
                m_power_cycle_flag = 0;
            }
        }
    }

    if (t_ppgchange > 0) {
        /* 需要增加 PPG */
        if (m_power_cycle_flag) {
            if (ppgValue.duty + t_ppgchange >= ppgValue.prioed / 2) {
                PowerCycleDoubleCnt++;
                if (PowerCycleDoubleCnt > PowerCycleDoubleOn) {
                    PowerCycleDoubleCnt = 0;
                    m_power_cycle_flag  = 0;
                    PowerCycleBaseCnt   = 0;
                    PowerCycleDoubleDuty  = FunPPGgetValue().duty;
                    PowerCycleDoublePower = ActualPower;
                }
            }
        }
    } else {
        /* 需要减小 PPG */
        if (m_power_cycle_flag == 0) {
            if (PowerCycle >= FRE_30K_PWM * 2) {
                if (ppgValue.duty <= MAX_FRE_PWM || m_power_hold_min) {
                    PowerCycleBaseCnt++;
                    if (PowerCycleBaseCnt > MIN_BASE_COUNT) {
                        PowerCycleBaseCnt = 0;
                        m_power_cycle_flag  = 1;
                        PowerCycleDoubleCnt = 0;
                        PowerCycleBaseDuty  = FunPPGgetValue().duty;
                        PowerCycleBasePower = ActualPower;

                        uint32_t doubleOnTime = 50;
                        if (TargetPower > 200 / 25 && ActualPower > TargetPower) {
                            doubleOnTime = ActualPower - TargetPower;
                            doubleOnTime *= MIN_BASE_COUNT;
                            doubleOnTime /= TargetPower - 200 / 25;
                        }
                        PowerCycleDoubleOn = doubleOnTime;
                    }
                }
            }
        }
    }
}

/* ================================================================
 * i_ppg_set_limit — PPG 稳定后动态收紧/放宽上限
 * ================================================================ */
void i_ppg_set_limit(INT16U t_corrent_ppg)
{
    if ((g_power_limit_flag & B_PWDEAD)) {
        if (TargetPower == s_ppg_limit_power && power_half_adj == 0) {
            if (g_surge_delay > C_KEEP_TIME + POT_TYPE_DELAY2) {
                s_limit_max_count++;
                if (s_limit_max_count > 10) {
                    s_limit_max_count = 0;
                    s_ppg_limit_max = t_corrent_ppg + PPG_MAX_RANGE;
                    if (s_ppg_limit_max < FRE_30K_PWM) {
                        s_ppg_limit_max = FRE_30K_PWM;
                    }
                }

                if ((t_corrent_ppg < s_ppg_limit)) {
                    IHStatus |= B_POW_ARRIVE_FLAG;
                    s_ppg_limit = t_corrent_ppg + (PPG_MAX_RANGE / 5);
                    if (s_ppg_limit < FRE_40K_PWM) {
                        s_ppg_limit = FRE_40K_PWM;
                    }
                    {
                        uint16_t cycle = PowerCycle / 4 + 100;
                        if (s_ppg_limit < cycle) {
                            s_ppg_limit = cycle;
                        }
                    }
                    m_ppg_lock_flag = 1;
                    s_limit_voltage = VoltageValue;
                }
            } else {
                s_limit_max_count = 0;
            }
        }
    }

    if (s_ppg_limit > s_ppg_limit_max) {
        s_ppg_limit = s_ppg_limit_max;
    }
}

/* ================================================================
 * HAL_HALF: 外部函数声明 — 实现在 app_power.c
 *
 * 这些函数直接操作硬件寄存器 (HRTIM/PPG/DMA),
 * 遵循 HAL_HALF 模式保留在源文件中。
 * 此处声明确保编译通过。
 * ================================================================ */

/* ---- 外部函数: app_power.c (ISR + 硬件控制) ---- */
extern void powerZeroChange(void);
extern void APP_ADC_IRQ_PPGstepChangeCallBack(void);
extern void API_POWER_ScrOutput(uint8_t TskId);
extern void PowerStepDec(AppPowerDef* powerCh);
extern uint16_t PowerStepChange(AppPowerDef* powerCh);
extern void APP_POWER_SetTxaAwdValue(void);
extern void APP_POWER_PpgSetMinAll(void);
extern void APP_POWER_CycleChange(void);
extern void APP_POWER_CycleReset(void);
extern uint8_t APP_POWER_DutyLess50(void);
extern void APP_POWER_PotTypeCheck(void);
extern uint8_t APP_POWER_IsBitPggOn(uint8_t ch);
extern uint8_t check_pot_in(void);

/* ---- MODULE_EXPORT — 必须在最后 ---- */
MODULE_EXPORT(PowerBase);
