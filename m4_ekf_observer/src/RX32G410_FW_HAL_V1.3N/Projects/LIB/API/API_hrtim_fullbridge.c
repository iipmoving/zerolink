/**
 ******************************************************************************
 * @file    API_hrtim_fullbridge.c
 * @brief   全桥HRTIM驱动API实现
 * @note    基于现有半桥API (API_hrtim.c) 扩展，不修改原有代码
 *          - 使用 PotCh1(TimerB:TB1/TB2) + PotCh2(TimerE:TE1/TE2) 组成全桥1
 *          - 使用 PotCh3(TimerA:TA1/TA2) + PotCh4(TimerD:TD1/TD2) 组成全桥2
 *          - ★ 所有HAL层依赖完全封装在.c内部，APP层不可见 ★
 *          - ★ 复用原 hhrtim1 句柄，不创建新的HRTIM句柄 ★
 *
 * 硬件通道映射:
 *   全桥CH1 (fbCh=PotCh1):
 *     超前臂(Lead):  PotCh1 -> TimerB -> TB1(Q1上管), TB2(Q2下管, 互补+死区)
 *     滞后臂(Lag):   PotCh2 -> TimerE -> TE1(Q3上管), TE2(Q4下管, 互补+死区)
 *   全桥CH2 (fbCh=PotCh2):
 *     超前臂(Lead):  PotCh3 -> TimerA -> TA1(Q1上管), TA2(Q2下管, 互补+死区)
 *     滞后臂(Lag):   PotCh4 -> TimerD -> TD1(Q3上管), TD2(Q4下管, 互补+死区)
 *
 * 同步机制 (Master同步模式):
 *   - Master定时器提供统一周期基准 (MPER)
 *   - 超前臂由 Master PER 事件复位 (与Master同频同相)
 *   - FB1滞后臂由 Master CMP1 事件复位 (MCMP1 控制移相)
 *   - FB2滞后臂由 Master CMP2 事件复位 (MCMP2 控制移相)
 *   - 所有从定时器 UpdateTrigger = MASTER
 *
 * 工作模式:
 *   - 调频模式: 前后桥臂各输出50%互补PWM, 频率和相位差独立调节
 *               (输出功率由相位差控制 = 移相全桥)
 *   - 调功模式: 前后桥臂同相位, 统一CMP值调节占空比
 *               (输出功率由占空比控制)
 ******************************************************************************
 */

/*============================================================================
 *                    HAL层头文件 (仅.c内部使用，APP不可见)
 *============================================================================*/
#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include "system_init.h"
#include "system_bsp.h"

/* API层头文件 */
#include "API_hrtim_fullbridge.h"
#include "API_hrtim.h"
#include "API_tim.h"
#include "DRV_GPIO.H"
#include "API_gpio.h"

/*============================================================================
 *                    【新增】外部引用 (来自原半桥代码)
 *============================================================================*/

extern HRTIM_HandleTypeDef hhrtim1;          /* 原半桥HRTIM句柄, 全桥复用 */
extern void Error_Handler(void);

/*============================================================================
 *          【新增】内部常量: 全桥硬件资源映射表 (HAL常量全部封装在此)
 *============================================================================*/

#define FB_MAX_CH   2    /* 最大全桥通道数: fbCh=PotCh1(0), fbCh=PotCh2(1) */

/**
 * @brief 全桥硬件资源映射 (每个全桥通道由超前臂+滞后臂组成)
 * @note  全部使用HAL层常量，仅.c内部可见
 */
typedef struct {
    uint32_t leadTimerIndex;        /* 超前臂定时器索引 (HRTIM_TIMERINDEX_TIMER_x) */
    uint32_t lagTimerIndex;         /* 滞后臂定时器索引 */
    uint32_t leadTimerID;           /* 超前臂定时器ID (HRTIM_TIMERID_TIMER_x, 用于启停) */
    uint32_t lagTimerID;            /* 滞后臂定时器ID */
    uint32_t leadOutPin;            /* 超前臂输出引脚 (HRTIM_OUTPUT_Tx1|HRTIM_OUTPUT_Tx2) */
    uint32_t lagOutPin;             /* 滞后臂输出引脚 */
    uint32_t masterCmpResetTrigger; /* 滞后臂复位触发源 (Master CMP事件) */
    uint32_t masterCmpUnit;         /* 对应的Master比较单元 (HRTIM_COMPAREUNIT_x) */
} FB_HWMapTypeDef;

/** @brief 全桥硬件映射查找表 */
static const FB_HWMapTypeDef FB_HW_MAP[FB_MAX_CH] = {
    {
        /* fbCh=PotCh1(0): 超前臂PotCh1(TimerB) + 滞后臂PotCh2(TimerE) */
        .leadTimerIndex       = HRTIM_TIMERINDEX_TIMER_B,
        .lagTimerIndex        = HRTIM_TIMERINDEX_TIMER_E,
        .leadTimerID          = HRTIM_TIMERID_TIMER_B,
        .lagTimerID           = HRTIM_TIMERID_TIMER_E,
        .leadOutPin           = HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2,
        .lagOutPin            = HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2,
        .masterCmpResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_CMP1,
        .masterCmpUnit         = HRTIM_COMPAREUNIT_1,     /* MCMP1 控制FB1移相 */
    },
    {
        /* fbCh=PotCh2(1): 超前臂PotCh3(TimerA) + 滞后臂PotCh4(TimerD) */
        .leadTimerIndex       = HRTIM_TIMERINDEX_TIMER_A,
        .lagTimerIndex        = HRTIM_TIMERINDEX_TIMER_D,
        .leadTimerID          = HRTIM_TIMERID_TIMER_A,
        .lagTimerID           = HRTIM_TIMERID_TIMER_D,
        .leadOutPin           = HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2,
        .lagOutPin            = HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2,
        .masterCmpResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_CMP2,
        .masterCmpUnit         = HRTIM_COMPAREUNIT_2,     /* MCMP2 控制FB2移相 */
    },
};

/*============================================================================
 *          【新增】内部常量: 全桥时基/比较/输出配置模板
 *============================================================================*/

/** @brief 默认周期值 (25kHz @ 192MHz MUL4) */
#define FB_DEFAULT_PERIOD           (((uint32_t)192000000U * 4U) / FB_DEFAULT_FREQ_HZ)

/** @brief 全桥Slave定时器使用的比较单元 (CMP2 = 占空比控制) */
#define FB_COMPAREUNIT_DUTY         HRTIM_COMPAREUNIT_2

/** @brief 独立模式时基配置 (各通道独立周期) */
static const HRTIM_TimeBaseCfgTypeDef FB_TimeBaseCfg_Independent = {
    FB_DEFAULT_PERIOD,                  /* Period (运行时覆盖) */
    0,                                  /* RepetitionCounter */
    HRTIM_PRESCALERRATIO_MUL4,          /* PrescalerRatio: fHRTIM = 768MHz */
    HRTIM_MODE_CONTINUOUS,              /* Mode: 连续模式 */
};

/** @brief Master同步模式时基配置 */
static const HRTIM_TimeBaseCfgTypeDef FB_TimeBaseCfg_MasterSync = {
    FB_DEFAULT_PERIOD,                  /* Period (运行时覆盖) */
    0,
    HRTIM_PRESCALERRATIO_MUL4,
    HRTIM_MODE_CONTINUOUS,
};

/** @brief Master定时器时基配置 (仅同步模式使用) */
static const HRTIM_TimeBaseCfgTypeDef FB_MasterTimeBaseCfg = {
    FB_DEFAULT_PERIOD,                  /* Period (运行时覆盖) */
    0,
    HRTIM_PRESCALERRATIO_MUL4,          /* 必须与Slave预分频一致 */
    HRTIM_MODE_CONTINUOUS,
};

/** @brief 定时器波形控制配置 (共用) */
static const HRTIM_TimerCtlTypeDef FB_TimerCtl = {
    HRTIM_TIMERUPDOWNMODE_UP,           /* UpDownMode: 向上计数 */
    HRTIM_TIMERTRIGHALF_DISABLED,       /* TrigHalf */
    HRTIM_TIMERGTCMP3_EQUAL,            /* GreaterCMP3 */
    HRTIM_TIMERGTCMP1_EQUAL,            /* GreaterCMP1 */
    HRTIM_TIMER_DCDR_COUNTER,           /* DualChannelDacReset */
    HRTIM_TIMER_DCDS_CMP2,              /* DualChannelDacStep */
    HRTIM_TIMER_DCDE_DISABLED,          /* DualChannelDacEnable */
};

/** @brief 死区基础配置模板 (具体值由fbConfig覆盖) */
static const HRTIM_DeadTimeCfgTypeDef FB_DeadTimeCfg_Base = {
    HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV4,   /* Prescaler */
    48,                                       /* RisingValue (运行时覆盖) */
    HRTIM_TIMDEADTIME_RISINGSIGN_POSITIVE,    /* RisingSign */
    HRTIM_TIMDEADTIME_RISINGLOCK_WRITE,       /* RisingLock */
    HRTIM_TIMDEADTIME_RISINGSIGNLOCK_WRITE,   /* RisingSignLock */
    48,                                       /* FallingValue (运行时覆盖) */
    HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE,   /* FallingSign */
    HRTIM_TIMDEADTIME_FALLINGLOCK_WRITE,      /* FallingLock */
    HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_WRITE,  /* FallingSignLock */
};

/*============================================================================
 *          【新增】内部辅助函数声明
 *============================================================================*/

static HAL_StatusTypeDef FB_ConfigMasterTimeBase(const FB_HWMapTypeDef *hw, uint16_t period);
static HAL_StatusTypeDef FB_ConfigMasterCompare(const FB_HWMapTypeDef *hw, uint16_t phaseShift);
static HAL_StatusTypeDef FB_ConfigTimerBase_Independent(const FB_HWMapTypeDef *hw, uint16_t period);
static HAL_StatusTypeDef FB_ConfigTimerFull_MasterSync(const FB_HWMapTypeDef *hw, uint8_t isLeadLeg, uint16_t period);
static HAL_StatusTypeDef FB_ConfigOutput_FreqMod(uint32_t timerIndex, uint32_t outPin1, uint32_t outPin2, uint16_t period);
static HAL_StatusTypeDef FB_ConfigOutput_PowerMod(uint32_t timerIndex, uint32_t outPin1, uint32_t outPin2, uint16_t cmpValue);
static HAL_StatusTypeDef FB_ConfigDeadTime_HAL(uint32_t timerIndex, uint16_t risingVal, uint16_t fallingVal);
static uint16_t FB_CalcDeadTimeCount(uint32_t deadtimeNs);
static const FB_HWMapTypeDef* FB_GetHWMap(uint8_t fbCh);

/*============================================================================
 *                    【新增】全桥配置存储 (静态, 内部访问)
 *============================================================================*/

/** @brief 默认全桥通道1配置 (fbCh=PotCh1) */
static FB_ConfigTypeDef g_fbConfig_Ch1 = {
    .syncMode    = FB_SYNC_MODE_MASTER,
    .bridgeMode  = FB_BRIDGE_MODE_FREQ_MOD,
    .deadTime = {
        .risingValue     = FB_DEFAULT_DEADTIME_RISING,
        .fallingValue    = FB_DEFAULT_DEADTIME_FALLING,
        .risingValueLag  = FB_DEFAULT_DEADTIME_RISING,
        .fallingValueLag = FB_DEFAULT_DEADTIME_FALLING,
    },
    .value = {
        .period     = 0,     /* 初始化时根据频率计算 */
        .cmpValue   = 0,
        .phaseShift = 0,
    },
    .isRunning = 0,
};

/** @brief 默认全桥通道2配置 (fbCh=PotCh2, 预留) */
static FB_ConfigTypeDef g_fbConfig_Ch2 = {
    .syncMode    = FB_SYNC_MODE_MASTER,
    .bridgeMode  = FB_BRIDGE_MODE_FREQ_MOD,
    .deadTime = {
        .risingValue     = FB_DEFAULT_DEADTIME_RISING,
        .fallingValue    = FB_DEFAULT_DEADTIME_FALLING,
        .risingValueLag  = FB_DEFAULT_DEADTIME_RISING,
        .fallingValueLag = FB_DEFAULT_DEADTIME_FALLING,
    },
    .value = {
        .period     = 0,
        .cmpValue   = 0,
        .phaseShift = 0,
    },
    .isRunning = 0,
};

/** @brief 全桥配置指针数组 (按fbCh索引) */
static FB_ConfigTypeDef* const g_fbConfigPtrs[FB_MAX_CH] = {
    &g_fbConfig_Ch1,    /* fbCh=PotCh1=0 */
    &g_fbConfig_Ch2,    /* fbCh=PotCh2=1 */
};

/*============================================================================
 *                    【新增】内部辅助函数: 硬件映射获取
 *============================================================================*/

/**
 * @brief  根据fbCh获取硬件映射指针
 * @param  fbCh: 全桥通道号 (PotCh1=0, PotCh2=1)
 * @return 硬件映射指针，无效时返回NULL
 */
static const FB_HWMapTypeDef* FB_GetHWMap(uint8_t fbCh)
{
    if (fbCh >= FB_MAX_CH) return NULL;
    return &FB_HW_MAP[fbCh];
}

/*============================================================================
 *                    【新增】内部辅助函数: 死区时间计算
 *============================================================================*/

/**
 * @brief  将死区时间(ns)转换为HRTIM死区计数值
 * @param  deadtimeNs: 死区时间 (纳秒)
 * @return HRTIM死区寄存器计数值 (最大0x1FF)
 * @note   DIV4预分频下: Tdtg = 16 × (1/192MHz) × 4 = 333.3ns/count
 *         计数值 ≈ deadtimeNs / 333 ≈ deadtimeNs * 3 / 1000
 */
static uint16_t FB_CalcDeadTimeCount(uint32_t deadtimeNs)
{
    uint32_t count = (deadtimeNs * 3U) / 1000U;
    if (count > 0x1FFU) count = 0x1FFU;
    return (uint16_t)count;
}

/*============================================================================
 *                    【新增】内部辅助函数: 配置Master定时器
 *============================================================================*/

/**
 * @brief  配置Master定时器时基 (Master同步模式使用)
 * @param  hw: 硬件映射指针
 * @param  period: 周期计数值
 * @return HAL状态
 */
static HAL_StatusTypeDef FB_ConfigMasterTimeBase(const FB_HWMapTypeDef *hw, uint16_t period)
{
    HRTIM_TimeBaseCfgTypeDef cfg = FB_MasterTimeBaseCfg;
    cfg.Period = period;
    (void)hw;
    return HAL_HRTIM_TimeBaseConfig(&hhrtim1, HRTIM_TIMERINDEX_MASTER, &cfg);
}

/**
 * @brief  配置Master比较寄存器 (用于滞后臂移相控制)
 * @param  hw: 硬件映射指针
 * @param  phaseShift: 相位差计数值
 * @return HAL状态
 * @note   使用HAL比较器配置写入MCMPxR
 */
static HAL_StatusTypeDef FB_ConfigMasterCompare(const FB_HWMapTypeDef *hw, uint16_t phaseShift)
{
    HRTIM_CompareCfgTypeDef cmpCfg;
    cmpCfg.CompareValue     = phaseShift;
    cmpCfg.AutoDelayedMode   = HRTIM_AUTODELAYEDMODE_REGULAR;
    cmpCfg.AutoDelayedTimeout = 0;

    return HAL_HRTIM_WaveformCompareConfig(&hhrtim1,
                                            HRTIM_TIMERINDEX_MASTER,
                                            hw->masterCmpUnit,
                                            &cmpCfg);
}

/*============================================================================
 *                    【新增】内部辅助函数: 配置Slave定时器
 *============================================================================*/

/**
 * @brief  配置单个Slave定时器基本参数 (独立模式)
 * @param  hw: 硬件映射指针
 * @param  period: 周期计数值
 * @return HAL状态
 */
static HAL_StatusTypeDef FB_ConfigTimerBase_Independent(const FB_HWMapTypeDef *hw, uint16_t period)
{
    HAL_StatusTypeDef status;
    HRTIM_TimeBaseCfgTypeDef timeBaseCfg = FB_TimeBaseCfg_Independent;
    timeBaseCfg.Period = period;

    status = HAL_HRTIM_TimeBaseConfig(&hhrtim1, hw->leadTimerIndex, &timeBaseCfg);
    if (status != HAL_OK) return status;

    status = HAL_HRTIM_TimeBaseConfig(&hhrtim1, hw->lagTimerIndex, &timeBaseCfg);
    if (status != HAL_OK) return status;

    /* 波形控制 (共用) */
    status = HAL_HRTIM_WaveformTimerControl(&hhrtim1, hw->leadTimerIndex,
                                              (HRTIM_TimerCtlTypeDef*)&FB_TimerCtl);
    if (status != HAL_OK) return status;

    status = HAL_HRTIM_WaveformTimerControl(&hhrtim1, hw->lagTimerIndex,
                                              (HRTIM_TimerCtlTypeDef*)&FB_TimerCtl);
    return status;
}

/**
 * @brief  配置单个Slave定时器完整参数 (Master同步模式)
 * @param  hw: 硬件映射指针
 * @param  isLeadLeg: 1=超前臂(由Master PER复位), 0=滞后臂(由Master CMP复位)
 * @param  period: 周期计数值
 * @return HAL状态
 */
static HAL_StatusTypeDef FB_ConfigTimerFull_MasterSync(const FB_HWMapTypeDef *hw,
                                                        uint8_t isLeadLeg,
                                                        uint16_t period)
{
    HAL_StatusTypeDef status;
    HRTIM_TimerCfgTypeDef timerCfg = {0};
    uint32_t timerIndex = isLeadLeg ? hw->leadTimerIndex : hw->lagTimerIndex;

    /* ---- 时基配置 ---- */
    HRTIM_TimeBaseCfgTypeDef timeBaseCfg = FB_TimeBaseCfg_MasterSync;
    timeBaseCfg.Period = period;

    status = HAL_HRTIM_TimeBaseConfig(&hhrtim1, timerIndex, &timeBaseCfg);
    if (status != HAL_OK) return status;

    /* ---- 定时器完整参数配置 ---- */
    timerCfg.InterruptRequests        = HRTIM_TIM_IT_NONE;
    timerCfg.DMARequests              = HRTIM_TIM_DMA_NONE;
    timerCfg.DMASrcAddress            = 0x400200c;
    timerCfg.DMADstAddress            = 0x2003df0;
    timerCfg.DMASize                  = 20;
    timerCfg.HalfModeEnable           = HRTIM_HALFMODE_DISABLED;
    timerCfg.InterleavedMode          = HRTIM_INTERLEAVED_MODE_DISABLED;
    timerCfg.StartOnSync              = HRTIM_SYNCSTART_DISABLED;  /* Master-Slave内部同步, 相位由ResetTrigger保证 */
    timerCfg.ResetOnSync              = HRTIM_SYNCRESET_DISABLED;
    timerCfg.DACSynchro               = HRTIM_DACSYNC_NONE;
    timerCfg.PreloadEnable            = HRTIM_PRELOAD_ENABLED;
    timerCfg.UpdateGating             = HRTIM_UPDATEGATING_INDEPENDENT;
    timerCfg.BurstMode                = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;
    timerCfg.RepetitionUpdate         = HRTIM_UPDATEONREPETITION_ENABLED;
    timerCfg.PushPull                 = HRTIM_TIMPUSHPULLMODE_DISABLED;
    timerCfg.FaultEnable              = HRTIM_TIMFAULTENABLE_NONE;
    timerCfg.FaultLock                = HRTIM_TIMFAULTLOCK_READWRITE;
    timerCfg.DeadTimeInsertion        = HRTIM_TIMDEADTIMEINSERTION_ENABLED;
    timerCfg.DelayedProtectionMode    = HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED;
    timerCfg.BalancedIdleAutomaticResume = HRTIM_OUTPUTBIAR_DISABLED;

    /* ★ 关键: 更新触发来自Master */
    timerCfg.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_MASTER;

    /* ★ 关键: 复位触发来源 */
    if (isLeadLeg) {
        /* 超前臂: 由Master周期事件复位 (与Master同步同相) */
        timerCfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_PER;
    } else {
        /* 滞后臂: 由Master CMPx事件复位 (实现移相) */
        timerCfg.ResetTrigger = hw->masterCmpResetTrigger;
    }
    timerCfg.ResetUpdate   = HRTIM_TIMUPDATEONRESET_DISABLED;
    timerCfg.ReSyncUpdate  = HRTIM_TIMERESYNC_UPDATE_UNCONDITIONAL;

    status = HAL_HRTIM_WaveformTimerConfig(&hhrtim1, timerIndex, &timerCfg);
    return status;
}

/*============================================================================
 *                    【新增】内部辅助函数: 输出和比较器配置
 *============================================================================*/

/**
 * @brief  配置定时器输出+比较器 (调频模式: 50%占空比互补输出)
 * @param  timerIndex: 定时器索引
 * @param  outPin1: 输出1 (上管, 奇数引脚)
 * @param  outPin2: 输出2 (下管, 偶数引脚, 互补)
 * @param  period: 周期值 (CMP2 = period/2 实现50%占空比)
 * @return HAL状态
 */
static HAL_StatusTypeDef FB_ConfigOutput_FreqMod(uint32_t timerIndex,
                                                  uint32_t outPin1,
                                                  uint32_t outPin2,
                                                  uint16_t period)
{
    HAL_StatusTypeDef status;
    HRTIM_CompareCfgTypeDef cmpCfg;
    HRTIM_OutputCfgTypeDef  outCfg;

    /* ---- CMP2: 50%占空比 = period/2 ---- */
    cmpCfg.CompareValue      = period / 2U;
    cmpCfg.AutoDelayedMode   = HRTIM_AUTODELAYEDMODE_REGULAR;
    cmpCfg.AutoDelayedTimeout = 0;

    status = HAL_HRTIM_WaveformCompareConfig(&hhrtim1, timerIndex,
                                              FB_COMPAREUNIT_DUTY, &cmpCfg);
    if (status != HAL_OK) return status;

    /* ---- 输出1 (上管): PER置位 → CMP2复位 → 50%高电平 ---- */
    outCfg.Polarity              = HRTIM_OUTPUTPOLARITY_HIGH;
    outCfg.SetSource             = HRTIM_OUTPUTSET_TIMPER;
    outCfg.ResetSource           = HRTIM_OUTPUTRESET_TIMCMP2;
    outCfg.IdleMode              = HRTIM_OUTPUTIDLEMODE_NONE;
    outCfg.IdleLevel             = HRTIM_OUTPUTIDLELEVEL_INACTIVE;
    outCfg.FaultLevel            = HRTIM_OUTPUTFAULTLEVEL_INACTIVE;
    outCfg.ChopperModeEnable     = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
    outCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;

    if (outPin1) {
        status = HAL_HRTIM_WaveformOutputConfig(&hhrtim1, timerIndex, outPin1, &outCfg);
        if (status != HAL_OK) return status;
    }

    /* ---- 输出2 (下管，互补): CMP2置位 → PER复位 (硬件死区自动反相) ---- */
    outCfg.SetSource   = HRTIM_OUTPUTSET_TIMCMP2;
    outCfg.ResetSource = HRTIM_OUTPUTRESET_TIMPER;

    if (outPin2) {
        status = HAL_HRTIM_WaveformOutputConfig(&hhrtim1, timerIndex, outPin2, &outCfg);
        if (status != HAL_OK) return status;
    }

    return HAL_OK;
}

/**
 * @brief  配置定时器输出+比较器 (调功模式: 可调占空比互补输出)
 * @param  timerIndex: 定时器索引
 * @param  outPin1: 输出1 (上管)
 * @param  outPin2: 输出2 (下管, 互补)
 * @param  cmpValue: CMP2比较值 (控制占空比)
 * @return HAL状态
 * @note   上管: PER置位 → CMP2复位
 *         下管: CMP2置位 → PER复位 (互补+死区)
 *         改CMP2值即同时改变两路占空比
 */
static HAL_StatusTypeDef FB_ConfigOutput_PowerMod(uint32_t timerIndex,
                                                   uint32_t outPin1,
                                                   uint32_t outPin2,
                                                   uint16_t cmpValue)
{
    HAL_StatusTypeDef status;
    HRTIM_CompareCfgTypeDef cmpCfg;
    HRTIM_OutputCfgTypeDef  outCfg;

    /* ---- CMP2: 统一占空比控制 ---- */
    cmpCfg.CompareValue      = cmpValue;
    cmpCfg.AutoDelayedMode   = HRTIM_AUTODELAYEDMODE_REGULAR;
    cmpCfg.AutoDelayedTimeout = 0;

    status = HAL_HRTIM_WaveformCompareConfig(&hhrtim1, timerIndex,
                                              FB_COMPAREUNIT_DUTY, &cmpCfg);
    if (status != HAL_OK) return status;

    /* ---- 输出1 (上管): PER置位 → CMP2复位 ---- */
    outCfg.Polarity              = HRTIM_OUTPUTPOLARITY_HIGH;
    outCfg.SetSource             = HRTIM_OUTPUTSET_TIMPER;
    outCfg.ResetSource           = HRTIM_OUTPUTRESET_TIMCMP2;
    outCfg.IdleMode              = HRTIM_OUTPUTIDLEMODE_NONE;
    outCfg.IdleLevel             = HRTIM_OUTPUTIDLELEVEL_INACTIVE;
    outCfg.FaultLevel            = HRTIM_OUTPUTFAULTLEVEL_INACTIVE;
    outCfg.ChopperModeEnable     = HRTIM_OUTPUTCHOPPERMODE_DISABLED;
    outCfg.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;

    if (outPin1) {
        status = HAL_HRTIM_WaveformOutputConfig(&hhrtim1, timerIndex, outPin1, &outCfg);
        if (status != HAL_OK) return status;
    }

    /* ---- 输出2 (下管，互补): CMP2置位 → PER复位 ---- */
    outCfg.SetSource   = HRTIM_OUTPUTSET_TIMCMP2;
    outCfg.ResetSource = HRTIM_OUTPUTRESET_TIMPER;

    if (outPin2) {
        status = HAL_HRTIM_WaveformOutputConfig(&hhrtim1, timerIndex, outPin2, &outCfg);
        if (status != HAL_OK) return status;
    }

    return HAL_OK;
}

/**
 * @brief  配置单个定时器的死区参数
 * @param  timerIndex: 定时器索引
 * @param  risingVal: 上升沿死区值
 * @param  fallingVal: 下降沿死区值
 * @return HAL状态
 */
static HAL_StatusTypeDef FB_ConfigDeadTime_HAL(uint32_t timerIndex,
                                                uint16_t risingVal,
                                                uint16_t fallingVal)
{
    HRTIM_DeadTimeCfgTypeDef dtCfg = FB_DeadTimeCfg_Base;
    dtCfg.RisingValue  = risingVal;
    dtCfg.FallingValue = fallingVal;
    return HAL_HRTIM_DeadTimeConfig(&hhrtim1, timerIndex, &dtCfg);
}

/*============================================================================
 *                    【新增】输出引脚分离辅助函数
 *============================================================================*/

/**
 * @brief  从输出引脚掩码中提取奇数位 (OUT1)
 * @param  outPinMask: 输出引脚掩码 (如 HRTIM_OUTPUT_TB1|HRTIM_OUTPUT_TB2)
 * @return 奇数位掩码 (如 HRTIM_OUTPUT_TB1)
 */
static inline uint32_t FB_GetOutPin1(uint32_t outPinMask)
{
    return outPinMask & 0x55555555U;
}

/**
 * @brief  从输出引脚掩码中提取偶数位 (OUT2)
 * @param  outPinMask: 输出引脚掩码
 * @return 偶数位掩码 (如 HRTIM_OUTPUT_TB2)
 */
static inline uint32_t FB_GetOutPin2(uint32_t outPinMask)
{
    return outPinMask & 0xAAAAAAAAU;
}

/*============================================================================
 *                    【新增】全桥定时器索引 -> 定时器ID 转换
 *============================================================================*/

/**
 * @brief  将HRTIM_TIMERINDEX_TIMER_x 转换为 HRTIM_TIMERID_TIMER_x
 * @note   通过查FB_HW_MAP来完成，如果需动态转换则用此函数
 */
static uint32_t FB_TimerIndexToID(uint32_t timerIndex)
{
    switch (timerIndex) {
        case HRTIM_TIMERINDEX_TIMER_A: return HRTIM_TIMERID_TIMER_A;
        case HRTIM_TIMERINDEX_TIMER_B: return HRTIM_TIMERID_TIMER_B;
        case HRTIM_TIMERINDEX_TIMER_C: return HRTIM_TIMERID_TIMER_C;
        case HRTIM_TIMERINDEX_TIMER_D: return HRTIM_TIMERID_TIMER_D;
        case HRTIM_TIMERINDEX_TIMER_E: return HRTIM_TIMERID_TIMER_E;
        case HRTIM_TIMERINDEX_TIMER_F: return HRTIM_TIMERID_TIMER_F;
        default: return 0;
    }
}

/*============================================================================
 *                    【新增】全桥初始化 - 独立同步模式
 *============================================================================*/

/**
 * @brief  全桥初始化 - 独立同步模式
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @note   各通道保留独立周期，不依赖Master定时器
 *         超前臂和滞后臂独立计数
 */
void API_FB_Init_Independent(uint8_t fbCh)
{
    const FB_HWMapTypeDef *hw = FB_GetHWMap(fbCh);
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];

    if (hw == NULL || cfg == NULL) return;

    /* 使用已设置的周期，若未设置(为0)则使用默认频率计算 */
    uint16_t period = cfg->value.period;
    if (period == 0) {
        period = (uint16_t)(((uint32_t)192000000U * 4U) / FB_DEFAULT_FREQ_HZ);
        cfg->value.period = period;
    }
    cfg->syncMode = FB_SYNC_MODE_INDEPENDENT;

    /* 步骤1: 配置超前臂和滞后臂时基 (独立模式) */
    if (FB_ConfigTimerBase_Independent(hw, period) != HAL_OK) {
        Error_Handler(); return;
    }

    /* 步骤2: 根据全桥模式配置输出和比较器 */
    if (cfg->bridgeMode == FB_BRIDGE_MODE_FREQ_MOD) {
        /* 调频模式: 50%互补输出 */
        if (FB_ConfigOutput_FreqMod(hw->leadTimerIndex,
                                     FB_GetOutPin1(hw->leadOutPin),
                                     FB_GetOutPin2(hw->leadOutPin),
                                     period) != HAL_OK) {
            Error_Handler(); return;
        }
        if (FB_ConfigOutput_FreqMod(hw->lagTimerIndex,
                                     FB_GetOutPin1(hw->lagOutPin),
                                     FB_GetOutPin2(hw->lagOutPin),
                                     period) != HAL_OK) {
            Error_Handler(); return;
        }
        cfg->value.cmpValue = period / 2U;
    } else {
        /* 调功模式: 可调占空比互补输出 */
        uint16_t cmpVal = period * FB_DEFAULT_DUTY_PERCENT / 100U;
        cfg->value.cmpValue = cmpVal;
        if (FB_ConfigOutput_PowerMod(hw->leadTimerIndex,
                                      FB_GetOutPin1(hw->leadOutPin),
                                      FB_GetOutPin2(hw->leadOutPin),
                                      cmpVal) != HAL_OK) {
            Error_Handler(); return;
        }
        if (FB_ConfigOutput_PowerMod(hw->lagTimerIndex,
                                      FB_GetOutPin1(hw->lagOutPin),
                                      FB_GetOutPin2(hw->lagOutPin),
                                      cmpVal) != HAL_OK) {
            Error_Handler(); return;
        }
    }

    /* 步骤3: 配置死区 */
    if (FB_ConfigDeadTime_HAL(hw->leadTimerIndex,
                               cfg->deadTime.risingValue,
                               cfg->deadTime.fallingValue) != HAL_OK) {
        Error_Handler(); return;
    }
    if (FB_ConfigDeadTime_HAL(hw->lagTimerIndex,
                               cfg->deadTime.risingValueLag,
                               cfg->deadTime.fallingValueLag) != HAL_OK) {
        Error_Handler(); return;
    }

    /* 步骤4: 软件更新 */
    HAL_HRTIM_SoftwareUpdate(&hhrtim1, hw->leadTimerIndex);
    HAL_HRTIM_SoftwareUpdate(&hhrtim1, hw->lagTimerIndex);

    cfg->isRunning = 0;
}

/*============================================================================
 *                    【新增】全桥初始化 - Master同步模式
 *============================================================================*/

/**
 * @brief  全桥初始化 - Master同步模式
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @note   核心同步机制:
 *         - Master定时器提供统一周期基准 (MPER)
 *         - 超前臂由Master PER事件复位 (与Master同频同相)
 *         - 滞后臂由Master CMPx事件复位 (MCMP1控制FB1移相, MCMP2控制FB2移相)
 *         - 所有从定时器预分频与Master一致 (MUL4)
 *         - 通过改变MCMPx值调节移相角
 *         - 复用已有hhrtim1句柄，不重新Init/DLL校准
 */
void API_FB_Init_MasterSync(uint8_t fbCh)
{
    const FB_HWMapTypeDef *hw = FB_GetHWMap(fbCh);
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];

    if (hw == NULL || cfg == NULL) return;

    /* 使用已设置的周期，若未设置(为0)则使用默认频率计算 */
    uint16_t period = cfg->value.period;
    if (period == 0) {
        period = (uint16_t)(((uint32_t)192000000U * 4U) / FB_DEFAULT_FREQ_HZ);
        cfg->value.period = period;
    }

    uint16_t phaseShift = cfg->value.phaseShift;

    cfg->syncMode = FB_SYNC_MODE_MASTER;

    /* ---- 步骤1: 配置Master定时器时基 (必须在Slave之前) ---- */
    if (FB_ConfigMasterTimeBase(hw, period) != HAL_OK) {
        Error_Handler(); return;
    }

    /* ---- 步骤2: 配置Master比较寄存器 (MCMPx用于此FB的滞后臂移相) ---- */
    if (FB_ConfigMasterCompare(hw, phaseShift) != HAL_OK) {
        Error_Handler(); return;
    }

    /* ---- 步骤3: 配置超前臂 (由Master PER复位, 与Master同步) ---- */
    if (FB_ConfigTimerFull_MasterSync(hw, 1, period) != HAL_OK) {
        Error_Handler(); return;
    }

    /* ---- 步骤4: 配置滞后臂 (由Master CMPx复位, 实现移相) ---- */
    if (FB_ConfigTimerFull_MasterSync(hw, 0, period) != HAL_OK) {
        Error_Handler(); return;
    }

    /* ---- 步骤5: 根据全桥模式配置输出和比较器 ---- */
    if (cfg->bridgeMode == FB_BRIDGE_MODE_FREQ_MOD) {
        /* 调频模式: 50%互补输出 */
        if (FB_ConfigOutput_FreqMod(hw->leadTimerIndex,
                                     FB_GetOutPin1(hw->leadOutPin),
                                     FB_GetOutPin2(hw->leadOutPin),
                                     period) != HAL_OK) {
            Error_Handler(); return;
        }
        if (FB_ConfigOutput_FreqMod(hw->lagTimerIndex,
                                     FB_GetOutPin1(hw->lagOutPin),
                                     FB_GetOutPin2(hw->lagOutPin),
                                     period) != HAL_OK) {
            Error_Handler(); return;
        }
        cfg->value.cmpValue = period / 2U;
    } else {
        /* 调功模式: 统一CMP值控制占空比 */
        uint16_t cmpVal = period * FB_DEFAULT_DUTY_PERCENT / 100U;
        cfg->value.cmpValue = cmpVal;
        if (FB_ConfigOutput_PowerMod(hw->leadTimerIndex,
                                      FB_GetOutPin1(hw->leadOutPin),
                                      FB_GetOutPin2(hw->leadOutPin),
                                      cmpVal) != HAL_OK) {
            Error_Handler(); return;
        }
        if (FB_ConfigOutput_PowerMod(hw->lagTimerIndex,
                                      FB_GetOutPin1(hw->lagOutPin),
                                      FB_GetOutPin2(hw->lagOutPin),
                                      cmpVal) != HAL_OK) {
            Error_Handler(); return;
        }
    }

    /* ---- 步骤6: 配置死区 ---- */
    if (FB_ConfigDeadTime_HAL(hw->leadTimerIndex,
                               cfg->deadTime.risingValue,
                               cfg->deadTime.fallingValue) != HAL_OK) {
        Error_Handler(); return;
    }
    if (FB_ConfigDeadTime_HAL(hw->lagTimerIndex,
                               cfg->deadTime.risingValueLag,
                               cfg->deadTime.fallingValueLag) != HAL_OK) {
        Error_Handler(); return;
    }

    /* ---- 步骤7: 软件更新所有相关定时器 ---- */
    HAL_HRTIM_SoftwareUpdate(&hhrtim1, HRTIM_TIMERINDEX_MASTER);
    HAL_HRTIM_SoftwareUpdate(&hhrtim1, hw->leadTimerIndex);
    HAL_HRTIM_SoftwareUpdate(&hhrtim1, hw->lagTimerIndex);

    cfg->isRunning = 0;
}

/*============================================================================
 *                    【新增】全桥频率设置
 *============================================================================*/

/**
 * @brief  设置全桥开关频率
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  freqHz: 目标频率 (Hz)
 * @note   period = (192MHz * 4) / freqHz
 *         同步模式: 同时更新Master + 超前臂 + 滞后臂周期
 *         独立模式: 仅更新超前臂和滞后臂周期
 *         调频模式: 自动保持CMP2=PER/2 (50%占空比)
 *         调功模式: 按比例缩放CMP值保持相同占空比百分比
 */
void API_FB_SetFrequency(uint8_t fbCh, uint32_t freqHz)
{
    const FB_HWMapTypeDef *hw = FB_GetHWMap(fbCh);
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];

    if (hw == NULL || cfg == NULL || freqHz == 0) return;

    uint16_t oldPeriod = cfg->value.period;
    uint16_t period = (uint16_t)(((uint32_t)192000000U * 4U) / freqHz);

    cfg->value.period = period;

    /* ---- Master同步模式: 更新Master ---- */
    if (cfg->syncMode == FB_SYNC_MODE_MASTER) {
        __HAL_HRTIM_SETPERIOD(&hhrtim1, HRTIM_TIMERINDEX_MASTER, period);

        /* 按比例缩放MCMP值 (保持相同移相角) */
        if (oldPeriod > 0) {
            cfg->value.phaseShift = (uint32_t)cfg->value.phaseShift * period / oldPeriod;
        }
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER,
                                hw->masterCmpUnit, cfg->value.phaseShift);
    }

    /* ---- 更新超前臂和滞后臂周期 ---- */
    __HAL_HRTIM_SETPERIOD(&hhrtim1, hw->leadTimerIndex, period);
    __HAL_HRTIM_SETPERIOD(&hhrtim1, hw->lagTimerIndex, period);

    /* ---- 更新CMP值 ---- */
    if (cfg->bridgeMode == FB_BRIDGE_MODE_FREQ_MOD) {
        /* 调频模式: 保持50%占空比 */
        uint16_t halfPeriod = period / 2U;
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, hw->leadTimerIndex,
                                FB_COMPAREUNIT_DUTY, halfPeriod);
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, hw->lagTimerIndex,
                                FB_COMPAREUNIT_DUTY, halfPeriod);
        cfg->value.cmpValue = halfPeriod;
    } else {
        /* 调功模式: 保持相同占空比比例 */
        if (oldPeriod > 0) {
            cfg->value.cmpValue = (uint32_t)cfg->value.cmpValue * period / oldPeriod;
        }
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, hw->leadTimerIndex,
                                FB_COMPAREUNIT_DUTY, cfg->value.cmpValue);
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, hw->lagTimerIndex,
                                FB_COMPAREUNIT_DUTY, cfg->value.cmpValue);
    }
}

/*============================================================================
 *                    【新增】全桥相位差设置
 *============================================================================*/

/**
 * @brief  设置全桥相位差
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  phaseShiftCount: 相位差计数值 (0 ~ period-1)
 * @note   同步模式: 更新对应的Master CMPx值来改变滞后臂复位点
 *         独立模式: 通过设置滞后臂CMP1偏移实现 (能力有限, 建议用Master同步)
 */
void API_FB_SetPhaseShift(uint8_t fbCh, uint16_t phaseShiftCount)
{
    const FB_HWMapTypeDef *hw = FB_GetHWMap(fbCh);
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];

    if (hw == NULL || cfg == NULL) return;

    cfg->value.phaseShift = phaseShiftCount;

    if (cfg->syncMode == FB_SYNC_MODE_MASTER) {
        /* Master同步模式: 更新对应的MCMPx */
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_MASTER,
                                hw->masterCmpUnit, phaseShiftCount);
    } else {
        /* 独立模式: 设置滞后臂CMP1偏移 */
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, hw->lagTimerIndex,
                                HRTIM_COMPAREUNIT_1, phaseShiftCount);
    }
}

/*============================================================================
 *                    【新增】统一CMP值设置 (调功模式核心)
 *============================================================================*/

/**
 * @brief  设置统一CMP值 (调功模式核心接口)
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  cmpValue: CMP比较值 (0 ~ period-1)
 * @note   一次调用同时设置超前臂和滞后臂的CMP2值
 *         死区由硬件自动插入, 无需软件处理
 */
void API_FB_SetCmpValue(uint8_t fbCh, uint16_t cmpValue)
{
    const FB_HWMapTypeDef *hw = FB_GetHWMap(fbCh);
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];

    if (hw == NULL || cfg == NULL) return;

    cfg->value.cmpValue = cmpValue;

    /* 同时更新超前臂和滞后臂的CMP2 (占空比控制) */
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, hw->leadTimerIndex,
                            FB_COMPAREUNIT_DUTY, cmpValue);
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, hw->lagTimerIndex,
                            FB_COMPAREUNIT_DUTY, cmpValue);
}

/*============================================================================
 *                    【新增】全桥启动/停止
 *============================================================================*/

/**
 * @brief  全桥启动
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @note   启动Master计数器(同步模式)和各通道计数器，使能PWM输出
 */
void API_FB_Start(uint8_t fbCh)
{
    const FB_HWMapTypeDef *hw = FB_GetHWMap(fbCh);
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];

    if (hw == NULL || cfg == NULL) return;

    /* 先停止以保证干净启动 */
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, hw->leadOutPin | hw->lagOutPin);

    /* 计数器清零 (Master + 超前臂 + 滞后臂全部清零) */
    __HAL_HRTIM_SETCOUNTER(&hhrtim1, hw->leadTimerIndex, 0);
    __HAL_HRTIM_SETCOUNTER(&hhrtim1, hw->lagTimerIndex, 0);

    /* Master和Slave同时启动 (与原始TIMsynchronous风格一致) */
    uint32_t timerIds = hw->leadTimerID | hw->lagTimerID;
    if (cfg->syncMode == FB_SYNC_MODE_MASTER) {
        __HAL_HRTIM_SETCOUNTER(&hhrtim1, HRTIM_TIMERINDEX_MASTER, 0);
        timerIds |= HRTIM_TIMERID_MASTER;
    }
    HAL_HRTIM_WaveformCountStart(&hhrtim1, timerIds);

    /* 使能输出引脚 */
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, hw->leadOutPin | hw->lagOutPin);

    cfg->isRunning = 1;
}

/**
 * @brief  全桥停止
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @note   停止PWM输出和计数器
 */
void API_FB_Stop(uint8_t fbCh)
{
    const FB_HWMapTypeDef *hw = FB_GetHWMap(fbCh);
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];

    if (hw == NULL || cfg == NULL) return;

    /* 先停止输出 */
    HAL_HRTIM_WaveformOutputStop(&hhrtim1, hw->leadOutPin | hw->lagOutPin);

    /* 停止计数器 */
    uint32_t timerIds = hw->leadTimerID | hw->lagTimerID;
    if (cfg->syncMode == FB_SYNC_MODE_MASTER) {
        timerIds |= HRTIM_TIMERID_MASTER;
    }
    HAL_HRTIM_WaveformCountStop(&hhrtim1, timerIds);

    cfg->isRunning = 0;
}

/*============================================================================
 *                    【新增】同步模式切换
 *============================================================================*/

/**
 * @brief  同步模式切换
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  newMode: 目标同步模式
 * @note   先停止当前输出，切换后重新初始化定时器
 *         切换是破坏性的，会清除当前PWM配置为默认值
 */
void API_FB_SwitchSyncMode(uint8_t fbCh, FB_SyncModeTypeDef newMode)
{
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];

    if (cfg == NULL) return;
    if (cfg->syncMode == newMode) return;

    /* 先停止 */
    if (cfg->isRunning) {
        API_FB_Stop(fbCh);
    }

    /* 根据目标模式重新初始化 */
    if (newMode == FB_SYNC_MODE_MASTER) {
        API_FB_Init_MasterSync(fbCh);
    } else {
        API_FB_Init_Independent(fbCh);
    }
}

/**
 * @brief  全桥模式切换 (调频/调功)
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  newMode: 目标全桥模式
 * @note   需先停止输出再切换
 */
void API_FB_SwitchBridgeMode(uint8_t fbCh, FB_BridgeModeTypeDef newMode)
{
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];

    if (cfg == NULL) return;
    if (cfg->bridgeMode == newMode) return;

    /* 必须停止后才能切换 */
    if (cfg->isRunning) {
        API_FB_Stop(fbCh);
    }

    cfg->bridgeMode = newMode;

    /* 重新初始化 (根据当前同步模式) */
    if (cfg->syncMode == FB_SYNC_MODE_MASTER) {
        API_FB_Init_MasterSync(fbCh);
    } else {
        API_FB_Init_Independent(fbCh);
    }
}

/*============================================================================
 *                    【新增】死区设置
 *============================================================================*/

/**
 * @brief  设置全桥死区时间
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  risingNs: 上升沿死区 (ns)
 * @param  fallingNs: 下降沿死区 (ns)
 * @note   同时设置超前臂和滞后臂的死区时间
 */
void API_FB_SetDeadTime(uint8_t fbCh, uint16_t risingNs, uint16_t fallingNs)
{
    const FB_HWMapTypeDef *hw = FB_GetHWMap(fbCh);
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];

    if (hw == NULL || cfg == NULL) return;

    uint16_t risingCount  = FB_CalcDeadTimeCount(risingNs);
    uint16_t fallingCount = FB_CalcDeadTimeCount(fallingNs);

    cfg->deadTime.risingValue     = risingCount;
    cfg->deadTime.fallingValue    = fallingCount;
    cfg->deadTime.risingValueLag  = risingCount;
    cfg->deadTime.fallingValueLag = fallingCount;

    FB_ConfigDeadTime_HAL(hw->leadTimerIndex, risingCount, fallingCount);
    FB_ConfigDeadTime_HAL(hw->lagTimerIndex, risingCount, fallingCount);
}

/*============================================================================
 *                    【新增】状态获取
 *============================================================================*/

/**
 * @brief  获取全桥当前周期值
 */
uint32_t API_FB_GetPeriod(uint8_t fbCh)
{
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];
    if (cfg == NULL) return 0;
    return cfg->value.period;
}

/**
 * @brief  获取全桥当前频率 (Hz)
 */
uint32_t API_FB_GetFrequency(uint8_t fbCh)
{
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];
    if (cfg == NULL || cfg->value.period == 0) return 0;
    return ((uint32_t)192000000U * 4U) / cfg->value.period;
}

/**
 * @brief  获取当前全桥参数
 */
FB_ValueTypeDef API_FB_GetValue(uint8_t fbCh)
{
    FB_ValueTypeDef val = {0, 0, 0};
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];
    if (cfg != NULL) {
        val = cfg->value;
    }
    return val;
}

/**
 * @brief  获取全桥配置结构体指针 (用于批量读写配置)
 */
FB_ConfigTypeDef* API_FB_GetConfig(uint8_t fbCh)
{
    if (fbCh >= FB_MAX_CH) return NULL;
    return g_fbConfigPtrs[fbCh];
}

/*============================================================================
 *          【新增】全桥特有 - 调频波形输出 (一站式初始化+启动)
 *============================================================================*/

/**
 * @brief  输出调频波形 (频率调制模式)
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  freqHz: 初始频率 (Hz)
 * @param  phaseShiftCount: 初始相位差计数值
 * @note   两桥臂各输出50%占空比互补PWM:
 *         - 超前臂: OUT1(上管) / OUT2(下管,互补+死区)
 *         - 滞后臂: OUT1(上管) / OUT2(下管,互补+死区)
 *         - 滞后臂相对超前臂有phaseShiftCount的相位偏移
 *         - 改变频率 = 改变周期; 改变相位差 = 改变输出功率
 *
 *         波形时序 (Master同步模式下):
 *         ┌─ Master周期 ─┐
 *         超前臂上管: ────┐      ┌────────  (50%占空)
 *         超前臂下管:      └──────┘         (互补+死区)
 *                     ← 相位差 →
 *         滞后臂上管: ────────┐      ┌────  (50%占空)
 *         滞后臂下管:          └──────┘     (互补+死区)
 */
void API_FB_OutputFreqModulation(uint8_t fbCh,
                                  uint32_t freqHz,
                                  uint16_t phaseShiftCount)
{
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];
    if (cfg == NULL) return;

    /* 设置为调频模式 */
    cfg->bridgeMode = FB_BRIDGE_MODE_FREQ_MOD;

    /* 计算频率对应的周期值 */
    uint16_t period = (uint16_t)(((uint32_t)192000000U * 4U) / freqHz);
    cfg->value.period = period;
    cfg->value.phaseShift = phaseShiftCount;
    cfg->value.cmpValue = period / 2U;  /* 50%占空比 */

    /* 使用Master同步模式初始化 (全桥移相需要Master提供统一时基) */
    /* 先预填参数再调Init, Init内部会使用cfg->value中的值 */
    API_FB_Init_MasterSync(fbCh);

    /* 启动输出 */
    API_FB_Start(fbCh);
}

/**
 * @brief  输出调功波形 (功率调制模式)
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  freqHz: 初始频率 (Hz)
 * @param  cmpValue: 初始CMP比较值
 * @note   两桥臂同相输出，统一CMP值控制占空比:
 *         - 前后桥臂使用相同的CMP2值, 一次设值同时影响两臂
 *         - 改变CMP值 = 调节输出功率
 *
 *         波形时序:
 *         ┌─ 周期 ─┐
 *         超前臂上管: ──┐    ┌────────  (CMP控制关断)
 *         超前臂下管:    └────┘         (互补+死区)
 *         滞后臂上管: ──┐    ┌────────  (同一CMP值)
 *         滞后臂下管:    └────┘         (互补+死区)
 */
void API_FB_OutputPowerModulation(uint8_t fbCh,
                                   uint32_t freqHz,
                                   uint16_t cmpValue)
{
    FB_ConfigTypeDef *cfg = g_fbConfigPtrs[fbCh];
    if (cfg == NULL) return;

    /* 设置为调功模式 */
    cfg->bridgeMode = FB_BRIDGE_MODE_POWER_MOD;

    /* 计算频率对应的周期值 */
    uint16_t period = (uint16_t)(((uint32_t)192000000U * 4U) / freqHz);
    cfg->value.period = period;
    cfg->value.cmpValue = cmpValue;
    cfg->value.phaseShift = 0;  /* 调功模式相位差为0 */

    /* 使用Master同步模式初始化 (统一时基保证前后桥臂同步) */
    API_FB_Init_MasterSync(fbCh);

    /* 启动输出 */
    API_FB_Start(fbCh);
}

/**
 * @brief  全桥HRTIM中断回调 (弱定义，用户可重写)
 * @param  fbCh: 全桥通道号
 * @param  interruptSource: 中断来源 (0=超前臂, 1=滞后臂, 2=Master)
 */
__weak void API_FB_IRQHandlerCallback(uint8_t fbCh, uint8_t interruptSource)
{
    (void)fbCh;
    (void)interruptSource;
}
