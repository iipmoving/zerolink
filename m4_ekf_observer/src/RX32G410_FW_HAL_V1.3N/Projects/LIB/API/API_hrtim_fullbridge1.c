/**
 ******************************************************************************
 * @file    API_hrtim_fullbridge.c
 * @brief   移相全桥HRTIM驱动实现（第一阶段：核心功能）
 * @note    基于RX32G410 MCU的HRTIM外设
 *          - 使用MASTER定时器作为统一时钟源（消除相位偏移）
 *          - TimerB输出超前臂Q1/Q2互补PWM
 *          - TimerE输出滞后臂Q3/Q4互补PWM
 *          - 通过MASTER CMP1控制移相角
 *          
 * 参考文档：
 * - RX32G410_Reference_Manual_v0p7.pdf 第22章 HRTIM
 * - RX32G410_HRTIM_MASTER统一时钟源配置.md
 ******************************************************************************
 */

#include "rx32g4xx_config_def.h"
#include "rx32g4xx_hal.h"
#include "system_init.h"
#include "system_bsp.h"

#include "API_hrtim_fullbridge.h"
#include "API_tim.h"
#include "DRV_GPIO.H"
#include "API_gpio.h"

// 外部函数声明
extern void Error_Handler(void);

/*============================================================================
 *                        内部宏定义
 *============================================================================*/

// 默认工作频率25kHz对应的周期值
#define FB_DEFAULT_PERIOD     ((FB_HRTIM_INPUT_CLOCK * 4) / FB_DEFAULT_FREQ)  // 30720

// 默认死区时间2000ns对应的计数值（使用DIV4预分频）
#define FB_DEFAULT_DEADTIME   FB_DEADTIME_COUNT(FB_DEADTIME_NS)  // 384

// 全桥炉头通道定义（与半桥POT通道对应）
#define FB_BRIDGE_CH1_LEAD_TIMER    HRTIM_TIMERINDEX_TIMER_B  // 1号炉头超前臂
#define FB_BRIDGE_CH1_LAG_TIMER     HRTIM_TIMERINDEX_TIMER_E  // 1号炉头滞后臂
#define FB_BRIDGE_CH2_LEAD_TIMER    HRTIM_TIMERINDEX_TIMER_A  // 2号炉头超前臂
#define FB_BRIDGE_CH2_LAG_TIMER     HRTIM_TIMERINDEX_TIMER_D  // 2号炉头滞后臂

// POT通道到全桥炉头的映射
#define IS_FULLBRIDGE_CH1(ch)       ((ch) == PotCh1)
#define IS_FULLBRIDGE_CH2(ch)       ((ch) == PotCh2)

/*============================================================================
 *                      HRTIM句柄和全局变量
 *============================================================================*/

HRTIM_HandleTypeDef hhrtim1_fb;  // 移相全桥HRTIM句柄

// 当前运行状态
FullBridgeStatusTypeDef fb_status = {0};

/*============================================================================
 *                    时基和配置常量（从半桥代码继承）
 *============================================================================*/

// 连续模式时基配置
static const HRTIM_TimeBaseCfgTypeDef FB_PPGTimeBaseCfg = {
    FB_DEFAULT_PERIOD,                    // Period: 周期值
    0,                                    // RepetitionCounter
    FB_HRTIM_PRESCALER_RATIO,             // PrescalerRatio: MUL4
    HRTIM_MODE_CONTINUOUS,                // Mode: 连续模式
};

// 定时器控制配置
static const HRTIM_TimerCtlTypeDef FB_PPGTimerCtl = {
    HRTIM_TIMERUPDOWNMODE_UP,             // UpDownMode: 递增计数
    HRTIM_TIMERTRIGHALF_DISABLED,         // TrigHalf
    HRTIM_TIMERGTCMP3_EQUAL,              // GreaterCMP3
    HRTIM_TIMERGTCMP1_EQUAL,              // GreaterCMP1
    HRTIM_TIMER_DCDR_COUNTER,             // DualChannelDacReset
    HRTIM_TIMER_DCDS_CMP2,                // DualChannelDacStep
    HRTIM_TIMER_DCDE_DISABLED,            // DualChannelDacEnable
};

// 比较配置（默认30%占空比）
static const HRTIM_CompareCfgTypeDef FB_PPGCompareCfg = {
    FB_DEFAULT_PERIOD * 30 / 100,         // CompareValue
    HRTIM_AUTODELAYEDMODE_REGULAR,        // AutoDelayedMode
    0,                                    // AutoDelayedTimeout
};

// 输出配置（对称互补PWM）
static const HRTIM_OutputCfgTypeDef FB_PPGOutputCfg = {
    HRTIM_OUTPUTPOLARITY_HIGH,            // Polarity
    HRTIM_OUTPUTSET_TIMPER,               // SetSource: 周期置位
    HRTIM_OUTPUTRESET_TIMCMP2,            // ResetSource: CMP2复位
    HRTIM_OUTPUTIDLEMODE_NONE,            // IdleMode
    HRTIM_OUTPUTIDLELEVEL_INACTIVE,       // IdleLevel
    HRTIM_OUTPUTFAULTLEVEL_INACTIVE,      // FaultLevel
    HRTIM_OUTPUTCHOPPERMODE_DISABLED,     // ChopperModeEnable
    HRTIM_OUTPUTBURSTMODEENTRY_REGULAR,   // BurstModeEntryDelayed
};

// 死区配置
static const HRTIM_DeadTimeCfgTypeDef FB_PPGDeadTimeCfg = {
    FB_DEADTIME_PRESCALER,                // Prescaler: DIV4
    FB_DEFAULT_DEADTIME,                  // RisingValue
    HRTIM_TIMDEADTIME_RISINGSIGN_POSITIVE,// RisingSign
    HRTIM_TIMDEADTIME_RISINGLOCK_WRITE,   // RisingLock
    HRTIM_TIMDEADTIME_RISINGSIGNLOCK_WRITE,// RisingSignLock
    FB_DEFAULT_DEADTIME,                  // FallingValue
    HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE,// FallingSign
    HRTIM_TIMDEADTIME_FALLINGLOCK_WRITE,  // FallingLock
    HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_WRITE,// FallingSignLock
};

/*============================================================================
 *                      MSP初始化函数
 *============================================================================*/

void HAL_HRTIM_MspInit(HRTIM_HandleTypeDef* hhrtim)
{
    if(hhrtim->Instance == HRTIM1) {
        __HAL_RCC_HRTIM1_CLK_ENABLE();
    }
}

void HAL_HRTIM_MspPostInit(HRTIM_HandleTypeDef* hhrtim)
{
    // GPIO配置在外部完成
}

/*============================================================================
 *                    通道映射辅助函数（私有）
 *============================================================================*/

/**
 * @brief 根据POT通道号获取超前臂定时器索引
 */
static uint8_t GetLeadTimerIndex(uint8_t pot_ch)
{
    if(IS_FULLBRIDGE_CH1(pot_ch)) {
        return FB_BRIDGE_CH1_LEAD_TIMER;  // TimerB
    } else if(IS_FULLBRIDGE_CH2(pot_ch)) {
        return FB_BRIDGE_CH2_LEAD_TIMER;  // TimerA
    }
    return FB_BRIDGE_CH1_LEAD_TIMER;  // 默认返回TimerB
}

/**
 * @brief 根据POT通道号获取滞后臂定时器索引
 */
static uint8_t GetLagTimerIndex(uint8_t pot_ch)
{
    if(IS_FULLBRIDGE_CH1(pot_ch)) {
        return FB_BRIDGE_CH1_LAG_TIMER;  // TimerE
    } else if(IS_FULLBRIDGE_CH2(pot_ch)) {
        return FB_BRIDGE_CH2_LAG_TIMER;  // TimerD
    }
    return FB_BRIDGE_CH1_LAG_TIMER;  // 默认返回TimerE
}

/**
 * @brief 根据POT通道号获取超前臂输出引脚
 */
static uint32_t GetLeadOutputPin(uint8_t pot_ch)
{
    if(IS_FULLBRIDGE_CH1(pot_ch)) {
        return HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2;
    } else if(IS_FULLBRIDGE_CH2(pot_ch)) {
        return HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2;
    }
    return HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2;
}

/**
 * @brief 根据POT通道号获取滞后臂输出引脚
 */
static uint32_t GetLagOutputPin(uint8_t pot_ch)
{
    if(IS_FULLBRIDGE_CH1(pot_ch)) {
        return HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2;
    } else if(IS_FULLBRIDGE_CH2(pot_ch)) {
        return HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2;
    }
    return HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2;
}

/*============================================================================
 *                    核心配置函数（私有）
 *============================================================================*/

/**
 * @brief 配置MASTER定时器（统一时钟源）
 */
static HAL_StatusTypeDef Configure_MasterTimer(void)
{
    HAL_StatusTypeDef status;
    
    // 配置MASTER时基
    status = HAL_HRTIM_TimeBaseConfig(&hhrtim1_fb, 
                                     HRTIM_TIMERINDEX_MASTER, 
                                     (HRTIM_TimeBaseCfgTypeDef*)&FB_PPGTimeBaseCfg);
    if(status != HAL_OK) return status;
    
    // 配置MASTER比较寄存器
    // MCMP1用于TimerE复位（移相控制），默认90度
    HRTIM_CompareCfgTypeDef MasterCompareCfg = FB_PPGCompareCfg;
    MasterCompareCfg.CompareValue = FB_DEFAULT_PERIOD / 4;  // 90度移相
    
    status = HAL_HRTIM_WaveformCompareConfig(&hhrtim1_fb,
                                            HRTIM_TIMERINDEX_MASTER,
                                            HRTIM_COMPAREUNIT_1,
                                            &MasterCompareCfg);
    if(status != HAL_OK) return status;
    
    // MCMP2预留
    MasterCompareCfg.CompareValue = FB_DEFAULT_PERIOD / 2;
    status = HAL_HRTIM_WaveformCompareConfig(&hhrtim1_fb,
                                            HRTIM_TIMERINDEX_MASTER,
                                            HRTIM_COMPAREUNIT_2,
                                            &MasterCompareCfg);
    
    return status;
}

/**
 * @brief 配置TimerB（超前臂Q1/Q2）
 */
static HAL_StatusTypeDef Configure_TimerB(void)
{
    HAL_StatusTypeDef status;
    
    // 配置TimerB时基（预分频器必须与MASTER相同！）
    status = HAL_HRTIM_TimeBaseConfig(&hhrtim1_fb,
                                     HRTIM_TIMERINDEX_TIMER_B,
                                     (HRTIM_TimeBaseCfgTypeDef*)&FB_PPGTimeBaseCfg);
    if(status != HAL_OK) return status;
    
    // 配置TimerB定时器参数
    HRTIM_TimerCfgTypeDef TimerBCfg = {0};
    TimerBCfg.InterruptRequests = HRTIM_TIM_IT_NONE;
    TimerBCfg.DMARequests = HRTIM_TIM_DMA_NONE;
    TimerBCfg.HalfModeEnable = HRTIM_HALFMODE_DISABLED;
    TimerBCfg.InterleavedMode = HRTIM_INTERLEAVED_MODE_DISABLED;
    TimerBCfg.StartOnSync = HRTIM_SYNCSTART_ENABLED;
    TimerBCfg.ResetOnSync = HRTIM_SYNCRESET_DISABLED;
    TimerBCfg.PreloadEnable = HRTIM_PRELOAD_ENABLED;
    TimerBCfg.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;
    TimerBCfg.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;
    TimerBCfg.RepetitionUpdate = HRTIM_UPDATEONREPETITION_ENABLED;
    TimerBCfg.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;
    TimerBCfg.FaultEnable = HRTIM_TIMFAULTENABLE_FAULT1 | HRTIM_TIMFAULTENABLE_FAULT2;
    TimerBCfg.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;
    TimerBCfg.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_ENABLED;
    TimerBCfg.DelayedProtectionMode = HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED;
    TimerBCfg.BalancedIdleAutomaticResume = HRTIM_OUTPUTBIAR_DISABLED;
    
    // ★ 关键：使用MASTER更新触发
    TimerBCfg.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_MASTER;
    
    // ★ 关键：TimerB由MASTER周期复位
    TimerBCfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_PER;
    TimerBCfg.ResetUpdate = HRTIM_TIMUPDATEONRESET_DISABLED;
    TimerBCfg.ReSyncUpdate = HRTIM_TIMERESYNC_UPDATE_UNCONDITIONAL;
    
    status = HAL_HRTIM_WaveformTimerConfig(&hhrtim1_fb,
                                          HRTIM_TIMERINDEX_TIMER_B,
                                          &TimerBCfg);
    if(status != HAL_OK) return status;
    
    // 配置TimerB比较寄存器
    status = HAL_HRTIM_WaveformCompareConfig(&hhrtim1_fb,
                                            HRTIM_TIMERINDEX_TIMER_B,
                                            HRTIM_COMPAREUNIT_2,
                                            (HRTIM_CompareCfgTypeDef*)&FB_PPGCompareCfg);
    if(status != HAL_OK) return status;
    
    // 配置TB1输出（Q1）
    HRTIM_OutputCfgTypeDef Output1Cfg = FB_PPGOutputCfg;
    Output1Cfg.SetSource = HRTIM_OUTPUTSET_TIMPER;
    Output1Cfg.ResetSource = HRTIM_OUTPUTRESET_TIMCMP2;
    
    status = HAL_HRTIM_WaveformOutputConfig(&hhrtim1_fb,
                                           HRTIM_TIMERINDEX_TIMER_B,
                                           HRTIM_OUTPUT_TB1,
                                           &Output1Cfg);
    if(status != HAL_OK) return status;
    
    // 配置TB2输出（Q2，互补）
    HRTIM_OutputCfgTypeDef Output2Cfg = FB_PPGOutputCfg;
    Output2Cfg.SetSource = HRTIM_OUTPUTSET_TIMCMP2;
    Output2Cfg.ResetSource = HRTIM_OUTPUTRESET_TIMPER;
    
    status = HAL_HRTIM_WaveformOutputConfig(&hhrtim1_fb,
                                           HRTIM_TIMERINDEX_TIMER_B,
                                           HRTIM_OUTPUT_TB2,
                                           &Output2Cfg);
    if(status != HAL_OK) return status;
    
    // 配置TimerB死区
    status = HAL_HRTIM_DeadTimeConfig(&hhrtim1_fb,
                                     HRTIM_TIMERINDEX_TIMER_B,
                                     (HRTIM_DeadTimeCfgTypeDef*)&FB_PPGDeadTimeCfg);
    
    return status;
}

/**
 * @brief 配置TimerE（滞后臂Q3/Q4）
 */
static HAL_StatusTypeDef Configure_TimerE(void)
{
    HAL_StatusTypeDef status;
    
    // 配置TimerE时基（预分频器必须与MASTER相同！）
    status = HAL_HRTIM_TimeBaseConfig(&hhrtim1_fb,
                                     HRTIM_TIMERINDEX_TIMER_E,
                                     (HRTIM_TimeBaseCfgTypeDef*)&FB_PPGTimeBaseCfg);
    if(status != HAL_OK) return status;
    
    // 配置TimerE定时器参数
    HRTIM_TimerCfgTypeDef TimerECfg = {0};
    TimerECfg.InterruptRequests = HRTIM_TIM_IT_NONE;
    TimerECfg.DMARequests = HRTIM_TIM_DMA_NONE;
    TimerECfg.HalfModeEnable = HRTIM_HALFMODE_DISABLED;
    TimerECfg.InterleavedMode = HRTIM_INTERLEAVED_MODE_DISABLED;
    TimerECfg.StartOnSync = HRTIM_SYNCSTART_ENABLED;
    TimerECfg.ResetOnSync = HRTIM_SYNCRESET_DISABLED;
    TimerECfg.PreloadEnable = HRTIM_PRELOAD_ENABLED;
    TimerECfg.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;
    TimerECfg.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;
    TimerECfg.RepetitionUpdate = HRTIM_UPDATEONREPETITION_ENABLED;
    TimerECfg.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;
    TimerECfg.FaultEnable = HRTIM_TIMFAULTENABLE_FAULT3 | HRTIM_TIMFAULTENABLE_FAULT4;
    TimerECfg.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;
    TimerECfg.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_ENABLED;
    TimerECfg.DelayedProtectionMode = HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED;
    TimerECfg.BalancedIdleAutomaticResume = HRTIM_OUTPUTBIAR_DISABLED;
    
    // ★ 关键：使用MASTER更新触发
    TimerECfg.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_MASTER;
    
    // ★ 关键：TimerE由MASTER CMP1复位（移相控制）
    TimerECfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_CMP1;
    TimerECfg.ResetUpdate = HRTIM_TIMUPDATEONRESET_DISABLED;
    TimerECfg.ReSyncUpdate = HRTIM_TIMERESYNC_UPDATE_UNCONDITIONAL;
    
    status = HAL_HRTIM_WaveformTimerConfig(&hhrtim1_fb,
                                          HRTIM_TIMERINDEX_TIMER_E,
                                          &TimerECfg);
    if(status != HAL_OK) return status;
    
    // 配置TimerE比较寄存器
    status = HAL_HRTIM_WaveformCompareConfig(&hhrtim1_fb,
                                            HRTIM_TIMERINDEX_TIMER_E,
                                            HRTIM_COMPAREUNIT_2,
                                            (HRTIM_CompareCfgTypeDef*)&FB_PPGCompareCfg);
    if(status != HAL_OK) return status;
    
    // 配置TE1输出（Q3）
    HRTIM_OutputCfgTypeDef Output1Cfg = FB_PPGOutputCfg;
    Output1Cfg.SetSource = HRTIM_OUTPUTSET_TIMPER;
    Output1Cfg.ResetSource = HRTIM_OUTPUTRESET_TIMCMP2;
    
    status = HAL_HRTIM_WaveformOutputConfig(&hhrtim1_fb,
                                           HRTIM_TIMERINDEX_TIMER_E,
                                           HRTIM_OUTPUT_TE1,
                                           &Output1Cfg);
    if(status != HAL_OK) return status;
    
    // 配置TE2输出（Q4，互补）
    HRTIM_OutputCfgTypeDef Output2Cfg = FB_PPGOutputCfg;
    Output2Cfg.SetSource = HRTIM_OUTPUTSET_TIMCMP2;
    Output2Cfg.ResetSource = HRTIM_OUTPUTRESET_TIMPER;
    
    status = HAL_HRTIM_WaveformOutputConfig(&hhrtim1_fb,
                                           HRTIM_TIMERINDEX_TIMER_E,
                                           HRTIM_OUTPUT_TE2,
                                           &Output2Cfg);
    if(status != HAL_OK) return status;
    
    // 配置TimerE死区
    status = HAL_HRTIM_DeadTimeConfig(&hhrtim1_fb,
                                     HRTIM_TIMERINDEX_TIMER_E,
                                     (HRTIM_DeadTimeCfgTypeDef*)&FB_PPGDeadTimeCfg);
    
    return status;
}

/**
 * @brief 配置TimerA（2号炉头超前臂Q1/Q2）
 */
static HAL_StatusTypeDef Configure_TimerA_Ch2(void)
{
    HAL_StatusTypeDef status;
    
    // 配置TimerA时基（预分频器必须与MASTER相同！）
    status = HAL_HRTIM_TimeBaseConfig(&hhrtim1_fb,
                                     HRTIM_TIMERINDEX_TIMER_A,
                                     (HRTIM_TimeBaseCfgTypeDef*)&FB_PPGTimeBaseCfg);
    if(status != HAL_OK) return status;
    
    // 配置TimerA定时器参数
    HRTIM_TimerCfgTypeDef TimerACfg = {0};
    TimerACfg.InterruptRequests = HRTIM_TIM_IT_NONE;
    TimerACfg.DMARequests = HRTIM_TIM_DMA_NONE;
    TimerACfg.HalfModeEnable = HRTIM_HALFMODE_DISABLED;
    TimerACfg.InterleavedMode = HRTIM_INTERLEAVED_MODE_DISABLED;
    TimerACfg.StartOnSync = HRTIM_SYNCSTART_ENABLED;
    TimerACfg.ResetOnSync = HRTIM_SYNCRESET_DISABLED;
    TimerACfg.PreloadEnable = HRTIM_PRELOAD_ENABLED;
    TimerACfg.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;
    TimerACfg.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;
    TimerACfg.RepetitionUpdate = HRTIM_UPDATEONREPETITION_ENABLED;
    TimerACfg.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;
    TimerACfg.FaultEnable = HRTIM_TIMFAULTENABLE_FAULT5 | HRTIM_TIMFAULTENABLE_FAULT6;
    TimerACfg.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;
    TimerACfg.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_ENABLED;
    TimerACfg.DelayedProtectionMode = HRTIM_TIMER_A_B_C_DELAYEDPROTECTION_DISABLED;
    TimerACfg.BalancedIdleAutomaticResume = HRTIM_OUTPUTBIAR_DISABLED;
    
    // ★ 关键：使用MASTER更新触发
    TimerACfg.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_MASTER;
    
    // ★ 关键：TimerA由MASTER周期复位
    TimerACfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_PER;
    TimerACfg.ResetUpdate = HRTIM_TIMUPDATEONRESET_DISABLED;
    TimerACfg.ReSyncUpdate = HRTIM_TIMERESYNC_UPDATE_UNCONDITIONAL;
    
    status = HAL_HRTIM_WaveformTimerConfig(&hhrtim1_fb,
                                          HRTIM_TIMERINDEX_TIMER_A,
                                          &TimerACfg);
    if(status != HAL_OK) return status;
    
    // 配置TimerA比较寄存器
    status = HAL_HRTIM_WaveformCompareConfig(&hhrtim1_fb,
                                            HRTIM_TIMERINDEX_TIMER_A,
                                            HRTIM_COMPAREUNIT_2,
                                            (HRTIM_CompareCfgTypeDef*)&FB_PPGCompareCfg);
    if(status != HAL_OK) return status;
    
    // 配置TA1输出（Q1）
    HRTIM_OutputCfgTypeDef Output1Cfg = FB_PPGOutputCfg;
    Output1Cfg.SetSource = HRTIM_OUTPUTSET_TIMPER;
    Output1Cfg.ResetSource = HRTIM_OUTPUTRESET_TIMCMP2;
    
    status = HAL_HRTIM_WaveformOutputConfig(&hhrtim1_fb,
                                           HRTIM_TIMERINDEX_TIMER_A,
                                           HRTIM_OUTPUT_TA1,
                                           &Output1Cfg);
    if(status != HAL_OK) return status;
    
    // 配置TA2输出（Q2，互补）
    HRTIM_OutputCfgTypeDef Output2Cfg = FB_PPGOutputCfg;
    Output2Cfg.SetSource = HRTIM_OUTPUTSET_TIMCMP2;
    Output2Cfg.ResetSource = HRTIM_OUTPUTRESET_TIMPER;
    
    status = HAL_HRTIM_WaveformOutputConfig(&hhrtim1_fb,
                                           HRTIM_TIMERINDEX_TIMER_A,
                                           HRTIM_OUTPUT_TA2,
                                           &Output2Cfg);
    if(status != HAL_OK) return status;
    
    // 配置TimerA死区
    status = HAL_HRTIM_DeadTimeConfig(&hhrtim1_fb,
                                     HRTIM_TIMERINDEX_TIMER_A,
                                     (HRTIM_DeadTimeCfgTypeDef*)&FB_PPGDeadTimeCfg);
    
    return status;
}

/**
 * @brief 配置TimerD（2号炉头滞后臂Q3/Q4）
 */
static HAL_StatusTypeDef Configure_TimerD_Ch2(void)
{
    HAL_StatusTypeDef status;
    
    // 配置TimerD时基（预分频器必须与MASTER相同！）
    status = HAL_HRTIM_TimeBaseConfig(&hhrtim1_fb,
                                     HRTIM_TIMERINDEX_TIMER_D,
                                     (HRTIM_TimeBaseCfgTypeDef*)&FB_PPGTimeBaseCfg);
    if(status != HAL_OK) return status;
    
    // 配置TimerD定时器参数
    HRTIM_TimerCfgTypeDef TimerDCfg = {0};
    TimerDCfg.InterruptRequests = HRTIM_TIM_IT_NONE;
    TimerDCfg.DMARequests = HRTIM_TIM_DMA_NONE;
    TimerDCfg.HalfModeEnable = HRTIM_HALFMODE_DISABLED;
    TimerDCfg.InterleavedMode = HRTIM_INTERLEAVED_MODE_DISABLED;
    TimerDCfg.StartOnSync = HRTIM_SYNCSTART_ENABLED;
    TimerDCfg.ResetOnSync = HRTIM_SYNCRESET_DISABLED;
    TimerDCfg.PreloadEnable = HRTIM_PRELOAD_ENABLED;
    TimerDCfg.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;
    TimerDCfg.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;
    TimerDCfg.RepetitionUpdate = HRTIM_UPDATEONREPETITION_ENABLED;
    TimerDCfg.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;
    TimerDCfg.FaultEnable = HRTIM_TIMFAULTENABLE_FAULT5 | HRTIM_TIMFAULTENABLE_FAULT6;
    TimerDCfg.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;
    TimerDCfg.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_ENABLED;
    TimerDCfg.DelayedProtectionMode = HRTIM_TIMER_D_E_DELAYEDPROTECTION_DISABLED;
    TimerDCfg.BalancedIdleAutomaticResume = HRTIM_OUTPUTBIAR_DISABLED;
    
    // ★ 关键：使用MASTER更新触发
    TimerDCfg.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_MASTER;
    
    // ★ 关键：TimerD由MASTER CMP1复位（移相控制）
    TimerDCfg.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_CMP1;
    TimerDCfg.ResetUpdate = HRTIM_TIMUPDATEONRESET_DISABLED;
    TimerDCfg.ReSyncUpdate = HRTIM_TIMERESYNC_UPDATE_UNCONDITIONAL;
    
    status = HAL_HRTIM_WaveformTimerConfig(&hhrtim1_fb,
                                          HRTIM_TIMERINDEX_TIMER_D,
                                          &TimerDCfg);
    if(status != HAL_OK) return status;
    
    // 配置TimerD比较寄存器
    status = HAL_HRTIM_WaveformCompareConfig(&hhrtim1_fb,
                                            HRTIM_TIMERINDEX_TIMER_D,
                                            HRTIM_COMPAREUNIT_2,
                                            (HRTIM_CompareCfgTypeDef*)&FB_PPGCompareCfg);
    if(status != HAL_OK) return status;
    
    // 配置TD1输出（Q3）
    HRTIM_OutputCfgTypeDef Output1Cfg = FB_PPGOutputCfg;
    Output1Cfg.SetSource = HRTIM_OUTPUTSET_TIMPER;
    Output1Cfg.ResetSource = HRTIM_OUTPUTRESET_TIMCMP2;
    
    status = HAL_HRTIM_WaveformOutputConfig(&hhrtim1_fb,
                                           HRTIM_TIMERINDEX_TIMER_D,
                                           HRTIM_OUTPUT_TD1,
                                           &Output1Cfg);
    if(status != HAL_OK) return status;
    
    // 配置TD2输出（Q4，互补）
    HRTIM_OutputCfgTypeDef Output2Cfg = FB_PPGOutputCfg;
    Output2Cfg.SetSource = HRTIM_OUTPUTSET_TIMCMP2;
    Output2Cfg.ResetSource = HRTIM_OUTPUTRESET_TIMPER;
    
    status = HAL_HRTIM_WaveformOutputConfig(&hhrtim1_fb,
                                           HRTIM_TIMERINDEX_TIMER_D,
                                           HRTIM_OUTPUT_TD2,
                                           &Output2Cfg);
    if(status != HAL_OK) return status;
    
    // 配置TimerD死区
    status = HAL_HRTIM_DeadTimeConfig(&hhrtim1_fb,
                                     HRTIM_TIMERINDEX_TIMER_D,
                                     (HRTIM_DeadTimeCfgTypeDef*)&FB_PPGDeadTimeCfg);
    
    return status;
}

/*============================================================================
 *                      API函数实现（第一阶段核心功能）
 *============================================================================*/

/**
 * @brief 初始化移相全桥HRTIM
 * @note 按照规格书22.3.24节的初始化顺序
 * @note 第一阶段：仅配置1号炉头（PotCh1 = TimerB + TimerE）
 *       第二阶段：可扩展为双炉头（增加PotCh2 = TimerA + TimerD）
 */
void API_FB_HRTIM1_Init(void)
{
    HAL_StatusTypeDef status;
    
    // 步骤1：DLL校准
    while(!__HAL_HRTIM_GET_FLAG(&hhrtim1_fb, HRTIM_FLAG_DLLRDY)) {
        SET_BIT(HRTIM1->sCommonRegs.DLLCR, HRTIM_DLLCR_CAL);
    }
    SET_BIT(HRTIM1->sCommonRegs.DLLCR, HRTIM_DLLCR_CALEN);
    
    // 步骤2：初始化HRTIM句柄
    hhrtim1_fb.Instance = HRTIM1;
    hhrtim1_fb.Init.HRTIMInterruptResquests = HRTIM_IT_NONE;
    hhrtim1_fb.Init.SyncOptions = HRTIM_SYNCOPTION_NONE;
    
    status = HAL_HRTIM_Init(&hhrtim1_fb);
    if(status != HAL_OK) {
        Error_Handler();
        return;
    }
    
    // 步骤3：配置MASTER定时器（必须在Slave之前）
    status = Configure_MasterTimer();
    if(status != HAL_OK) {
        Error_Handler();
        return;
    }
    
    // 步骤4：配置1号炉头 - TimerB（超前臂）
    status = Configure_TimerB();
    if(status != HAL_OK) {
        Error_Handler();
        return;
    }
    
    // 步骤5：配置1号炉头 - TimerE（滞后臂）
    status = Configure_TimerE();
    if(status != HAL_OK) {
        Error_Handler();
        return;
    }
    
    // 步骤6：MSP后初始化
    HAL_HRTIM_MspPostInit(&hhrtim1_fb);
    
    // 步骤7：使能1号炉头输出（PotCh1）
    HAL_HRTIM_WaveformOutputStart(&hhrtim1_fb, 
                                 HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2 |
                                 HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2);
    
    // 初始化状态
    fb_status.is_running = 0;
    fb_status.current_freq = FB_DEFAULT_FREQ;
    fb_status.current_duty = 30;
    fb_status.current_phase_shift = 90;
    fb_status.period_count = FB_DEFAULT_PERIOD;
    fb_status.duty_count = FB_DEFAULT_PERIOD * 30 / 100;
    fb_status.phase_shift_count = FB_DEFAULT_PERIOD / 4;
    fb_status.deadtime_count = FB_DEFAULT_DEADTIME;
}

/**
 * @brief 启动/停止PWM输出
 * @param ppgCh 通道号（PotCh1/PotCh2/PotAll）
 * @param flag 0=停止，1=启动
 * @note 按照半桥API风格，支持单通道或全部通道控制
 */
void API_FB_PPG_OnOff(uint8_t ppgCh, uint8_t flag)
{
    if(ppgCh == PotAll) {
        // 全部通道控制
        if(flag) {
            SET_BIT(HRTIM1->sMasterRegs.MCR, 
                    HRTIM_MCR_MCEN |    // MASTER
                    HRTIM_MCR_TBCEN |   // TimerB (PotCh1超前臂)
                    HRTIM_MCR_TECEN |   // TimerE (PotCh1滞后臂)
                    HRTIM_MCR_TACEN |   // TimerA (PotCh2超前臂)
                    HRTIM_MCR_TDCEN);   // TimerD (PotCh2滞后臂)
        } else {
            CLEAR_BIT(HRTIM1->sMasterRegs.MCR, 
                     HRTIM_MCR_MCEN | 
                     HRTIM_MCR_TBCEN | 
                     HRTIM_MCR_TECEN |
                     HRTIM_MCR_TACEN |
                     HRTIM_MCR_TDCEN);
        }
    } else if(IS_FULLBRIDGE_CH1(ppgCh)) {
        // 1号炉头控制（PotCh1 = TimerB + TimerE）
        if(flag) {
            SET_BIT(HRTIM1->sMasterRegs.MCR, 
                    HRTIM_MCR_MCEN |
                    HRTIM_MCR_TBCEN |
                    HRTIM_MCR_TECEN);
        } else {
            CLEAR_BIT(HRTIM1->sMasterRegs.MCR, 
                     HRTIM_MCR_TBCEN |
                     HRTIM_MCR_TECEN);
        }
    } else if(IS_FULLBRIDGE_CH2(ppgCh)) {
        // 2号炉头控制（PotCh2 = TimerA + TimerD）
        if(flag) {
            SET_BIT(HRTIM1->sMasterRegs.MCR, 
                    HRTIM_MCR_MCEN |
                    HRTIM_MCR_TACEN |
                    HRTIM_MCR_TDCEN);
        } else {
            CLEAR_BIT(HRTIM1->sMasterRegs.MCR, 
                     HRTIM_MCR_TACEN |
                     HRTIM_MCR_TDCEN);
        }
    }
    
    fb_status.is_running = flag;
}

/**
 * @brief 设置PWM周期（全局）
 * @param value 周期值
 * @note 全桥中MASTER决定周期，所有Slave定时器同步更新
 */
void API_FB_PPG_setPeriod(uint16_t value)
{
    HRTIM1->sMasterRegs.MPER = value;   // MASTER
    
    // 1号炉头（PotCh1）
    HRTIM1->sTimerxRegs[1].PERxR = value;  // TimerB (索引1)
    HRTIM1->sTimerxRegs[4].PERxR = value;  // TimerE (索引4)
    
    // 2号炉头（PotCh2）- 预留
    HRTIM1->sTimerxRegs[0].PERxR = value;  // TimerA (索引0)
    HRTIM1->sTimerxRegs[3].PERxR = value;  // TimerD (索引3)
    
    fb_status.period_count = value;
    
    // 重新计算占空比和移相
    fb_status.duty_count = (uint32_t)value * fb_status.current_duty / 100;
    fb_status.phase_shift_count = (uint32_t)value * fb_status.current_phase_shift / 360;
    
    // 1号炉头
    HRTIM1->sTimerxRegs[1].CMP2xR = fb_status.duty_count;  // TimerB
    HRTIM1->sTimerxRegs[4].CMP2xR = fb_status.duty_count;  // TimerE
    
    // 2号炉头
    HRTIM1->sTimerxRegs[0].CMP2xR = fb_status.duty_count;  // TimerA
    HRTIM1->sTimerxRegs[3].CMP2xR = fb_status.duty_count;  // TimerD
    
    // 移相控制（MASTER CMP1）
    HRTIM1->sMasterRegs.MCMP1R = fb_status.phase_shift_count;
}

/**
 * @brief 设置PPG值（全桥炉头统一控制）
 * @param ppgCh 通道号（PotCh1/PotCh2）
 * @param value PPG参数（包含period, duty, phase_shift）
 * @note 同时配置超前臂和滞后臂的占空比，以及移相角
 */
void API_FB_PPG_setValue(uint8_t ppgCh, FB_PPGvalueDef value)
{
    // 更新周期（如果变化）
    if(value.period != fb_status.period_count) {
        API_FB_PPG_setPeriod(value.period);
    }
    
    // 计算占空比计数值
    uint16_t duty_count = (uint32_t)value.period * value.duty / 100;
    
    // 根据通道号配置对应的定时器对
    if(IS_FULLBRIDGE_CH1(ppgCh)) {
        // 1号炉头：TimerB（超前臂）+ TimerE（滞后臂）
        HRTIM1->sTimerxRegs[1].CMP2xR = duty_count;  // TimerB占空比
        HRTIM1->sTimerxRegs[4].CMP2xR = duty_count;  // TimerE占空比
    } else if(IS_FULLBRIDGE_CH2(ppgCh)) {
        // 2号炉头：TimerA（超前臂）+ TimerD（滞后臂）
        HRTIM1->sTimerxRegs[0].CMP2xR = duty_count;  // TimerA占空比
        HRTIM1->sTimerxRegs[3].CMP2xR = duty_count;  // TimerD占空比
    }
    
    // 更新移相角（如果变化）
    if(value.phase_shift != fb_status.current_phase_shift) {
        API_FB_SetPhaseShift(ppgCh, value.phase_shift);
    }
    
    // 更新状态
    fb_status.duty_count = duty_count;
    fb_status.current_duty = value.duty;
}

/**
 * @brief 获取PPG值
 */
FB_PPGvalueDef API_FB_PPG_getValue(uint8_t ppgCh)
{
    FB_PPGvalueDef value;
    value.period = fb_status.period_count;
    value.duty = fb_status.current_duty;
    value.phase_shift = fb_status.current_phase_shift;
    return value;
}

/**
 * @brief 设置移相角（核心功能）
 * @param ppgCh 通道号（PotCh1/PotCh2）
 * @param angle_deg 移相角度（0-180度）
 * @note 
 * - 第一阶段：两个炉头共享MASTER CMP1，移相角相同
 * - 第二阶段：可扩展为独立移相（使用不同MASTER CMP寄存器）
 */
void API_FB_SetPhaseShift(uint8_t ppgCh, uint16_t angle_deg)
{
    if(angle_deg > FB_PHASE_SHIFT_MAX) {
        angle_deg = FB_PHASE_SHIFT_MAX;
    }
    
    uint16_t phase_shift_count = (uint32_t)fb_status.period_count * angle_deg / 360;
    
    // 第一阶段：所有炉头共享同一个移相角
    HRTIM1->sMasterRegs.MCMP1R = phase_shift_count;  // 更新MASTER CMP1
    
    fb_status.phase_shift_count = phase_shift_count;
    fb_status.current_phase_shift = angle_deg;
}

/**
 * @brief 获取移相角
 * @param ppgCh 通道号（保留兼容性）
 * @return 移相角度（度）
 */
uint16_t API_FB_GetPhaseShift(uint8_t ppgCh)
{
    return fb_status.current_phase_shift;
}

/**
 * @brief 判断电流采集窗口
 */
uint8_t API_FB_GetCurrentWindow(
    uint16_t hrtim_value,
    uint16_t duty_count,
    uint16_t phase_shift_count,
    uint16_t period)
{
    // 正向窗口：Q1(TA1)和Q4(TC2)同时导通
    // TA1高：[0, duty_count]
    // TC2高：[phase_shift_count + duty_count, period] ∪ [0, ...]
    
    // 简化判断逻辑
    if(hrtim_value <= duty_count) {
        // Q1导通区间
        if(hrtim_value >= phase_shift_count && 
           hrtim_value <= phase_shift_count + duty_count) {
            return 1;  // 正向窗口
        }
    }
    
    // 反向窗口：Q2(TA2)和Q3(TC1)同时导通
    uint16_t half_period = period / 2;
    uint16_t rev_start = half_period + phase_shift_count;
    uint16_t rev_end = rev_start + duty_count;
    
    if(hrtim_value >= rev_start && hrtim_value <= rev_end) {
        if(hrtim_value >= half_period && hrtim_value <= half_period + duty_count) {
            return 2;  // 反向窗口
        }
    }
    
    return 0;
}

/**
 * @brief 获取周期
 */
uint32_t API_FB_PPG_GetPeroid(uint8_t ppgCh)
{
    return fb_status.period_count;
}

/**
 * @brief 获取死区值
 */
uint32_t API_FB_PPG_getDeadTime(uint8_t ppgCh)
{
    return fb_status.deadtime_count;
}

/**
 * @brief 设置死区时间
 * @param ppgCh 通道号（PotCh1/PotCh2）
 * @param upDts 上升沿死区
 * @param downDts 下降沿死区
 * @note 同时配置超前臂和滞后臂的死区时间
 */
void API_FB_PPG_DeadTime(uint8_t ppgCh, uint8_t upDts, uint8_t downDts)
{
    HRTIM_DeadTimeCfgTypeDef DeadTimeCfg = FB_PPGDeadTimeCfg;
    DeadTimeCfg.RisingValue = upDts;
    DeadTimeCfg.FallingValue = downDts;
    
    if(IS_FULLBRIDGE_CH1(ppgCh)) {
        // 1号炉头：TimerB + TimerE
        HAL_HRTIM_DeadTimeConfig(&hhrtim1_fb, HRTIM_TIMERINDEX_TIMER_B, &DeadTimeCfg);
        HAL_HRTIM_DeadTimeConfig(&hhrtim1_fb, HRTIM_TIMERINDEX_TIMER_E, &DeadTimeCfg);
    } else if(IS_FULLBRIDGE_CH2(ppgCh)) {
        // 2号炉头：TimerA + TimerD
        HAL_HRTIM_DeadTimeConfig(&hhrtim1_fb, HRTIM_TIMERINDEX_TIMER_A, &DeadTimeCfg);
        HAL_HRTIM_DeadTimeConfig(&hhrtim1_fb, HRTIM_TIMERINDEX_TIMER_D, &DeadTimeCfg);
    }
    
    fb_status.deadtime_count = (upDts + downDts) / 2;
}

/**
 * @brief 获取减去死区的DUTY点
 */
FB_PPGpointDef API_FB_PPG_getValueDeadTime(uint8_t ppgCh)
{
    FB_PPGpointDef point;
    uint16_t deadtime = fb_status.deadtime_count;
    
    point.highOn = deadtime;
    point.highOff = fb_status.duty_count;
    point.lowOn = fb_status.duty_count + deadtime;
    point.lowOff = fb_status.period_count;
    point.phaseShiftPoint = fb_status.phase_shift_count;
    
    return point;
}

/**
 * @brief 获取运行状态
 */
void API_FB_GetStatus(FullBridgeStatusTypeDef* status)
{
    if(status != NULL) {
        *status = fb_status;
    }
}

/*============================================================================
 *                    兼容性和占位函数
 *============================================================================*/

void API_FB_SystemClocks_Init(void) { API_SystemClocks_Init(); }
void API_FB_PPG_SET_CONTINUOUS(uint8_t ch) {}
void API_FB_PPG_SET_SINGLE(uint8_t ch) {}
void API_FB_PPG_setPluse(uint8_t ppgCh, uint16_t value) {}
void API_FB_PPG_setPeriodChx(uint8_t ppgCh, uint16_t value) { API_FB_PPG_setPeriod(value); }
void API_FB_PPG_setValueChx(uint8_t ppgCh, uint16_t period, uint16_t duty) {
    FB_PPGvalueDef val = {period, duty, fb_status.current_phase_shift};
    API_FB_PPG_setValue(ppgCh, val);
}

uint8_t API_FB_PPG_BkFlag_Pot1(void) { return 0; }
uint8_t API_FB_PPG_BkFlag_Pot2(void) { return 0; }
uint8_t API_FB_PPG_BkFlag_Pot3(void) { return 0; }
uint8_t API_FB_PPG_BkFlag_Pot4(void) { return 0; }

void API_FB_Comp_SetSelPanCheck(uint8_t ppgCh) {}
void API_FB_Comp_SetSelPowerOn(uint8_t ppgCh) {}

void API_FB_HRTIM_DISABLE_IT_REST(void) {}
void API_FB_HRTIM_ENABLE_IT_REST(void) {}
void API_FB_HRTIM_BASE_DISABLE_IT_UPD(void) {}
void API_FB_HRTIM_BASE_ENABLE_IT_UPD(void) {}
void API_FB_HRTIM_BASE_DISABLE_IT_CMP(void) {}
void API_FB_HRTIM_BASE_ENABLE_IT_CMP(void) {}

uint8_t API_FB_HRTIM_GET_IT_UPD(uint8_t ch) { return 0; }
void API_FB_HRTIM_CLEAR_IT_UPD(uint8_t ch) {}

uint32_t API_FB_HRTIM_GetAddressTestCnt(void) { return 0; }
uint32_t API_FB_HRTIM_GetAddressTxaCnt(uint8_t ch) { return 0; }
uint32_t API_FB_HRTIM_GetTxaCnt(uint8_t ch) { return 0; }

void API_FB_HRTIM_CHECK_PAN_PLUSE(uint8_t ch) {}
void API_FB_TIMsynchronous(void) {}
void API_FB_TIMsynchronousPower(void) {}
void API_FB_HRTIM_PAN_CLEAR_FLAG(uint32_t ch) {}
void API_FB_HRTIM_SetDmaHandle(uint32_t* addr) {}

void API_FB_SoftStart(uint32_t target_freq, uint16_t target_duty, uint16_t step_ms) {}
void API_FB_SoftStop(uint16_t step_ms) {}
void API_FB_FaultRecovery(void) {}
void API_FB_GetHrtimStatusForDMA(FullBridgeHrtimStatusTypeDef* status) {}

/**
 * @brief 快速设置占空比和移相角（原子操作）
 * @param ppgCh 通道号（PotCh1/PotCh2）
 * @param duty_percent 占空比
 * @param phase_shift_deg 移相角
 * @note 用于需要同时更新的场景，避免中间状态
 */
void API_FB_SetDutyAndPhaseShift(uint8_t ppgCh, uint16_t duty_percent, uint16_t phase_shift_deg)
{
    // 先设置占空比
    FB_PPGvalueDef value;
    value.period = fb_status.period_count;
    value.duty = duty_percent;
    value.phase_shift = phase_shift_deg;
    
    API_FB_PPG_setValue(ppgCh, value);
}

/**
 * @brief 检查是否在全桥安全工作区域内（ZVS条件）
 * @param duty_percent 占空比
 * @param phase_shift_deg 移相角
 * @return 1:安全, 0:不安全
 */
uint8_t API_FB_IsSafeOperatingArea(uint16_t duty_percent, uint16_t phase_shift_deg)
{
    // ZVS条件：移相角应大于占空比对应的角度
    // 简化判断：phase_shift >= duty * 2（留有余量）
    if (duty_percent < FB_DUTY_MIN || duty_percent > FB_DUTY_MAX) {
        return 0;
    }
    if (phase_shift_deg < FB_PHASE_SHIFT_MIN || phase_shift_deg > FB_PHASE_SHIFT_MAX) {
        return 0;
    }
    // ZVS区域检查（可根据实际拓扑调整）
    if (phase_shift_deg < (duty_percent * 2)) {
        return 0;  // 可能无法实现ZVS
    }
    return 1;
}

__weak void API_FB_IRQHandler_Callback(uint8_t timer_id, uint32_t interrupt_flag) {}
