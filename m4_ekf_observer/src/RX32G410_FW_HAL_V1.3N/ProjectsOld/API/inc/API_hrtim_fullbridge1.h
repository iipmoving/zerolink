#ifndef __HRTIM_FULLBRIDGE_H__
#define __HRTIM_FULLBRIDGE_H__

#include <stdint.h>
#include "rx32g4xx_hal.h"


#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 *                        移相全桥HRTIM配置宏定义
 *============================================================================*/

// 系统时钟配置
#define FB_HRTIM_INPUT_CLOCK        192000000U    // HRTIM输入时钟192MHz
#define FB_HRTIM_PRESCALER_RATIO    HRTIM_PRESCALERRATIO_MUL4  // 4倍频 = 768MHz

// 工作频率和周期配置（默认25kHz，可动态调整）
#define FB_DEFAULT_FREQ             25000U        // 默认开关频率25kHz
#define FB_MAX_FREQ                 60000U        // 最大开关频率60kHz
#define FB_MIN_FREQ                 10000U        // 最小开关频率10kHz

// 周期计算：Period = (fHRCK) / fSW
#define FB_PERIOD(freq)             ((FB_HRTIM_INPUT_CLOCK * 4) / (freq))

// 死区时间配置
#define FB_DEADTIME_NS              2000U         // 死区时间2000ns（根据MOSFET特性调整）
#define FB_DEADTIME_PRESCALER       HRTIM_TIMDEADTIME_PRESCALERRATIO_DIV4
#define FB_DEADTIME_COUNT(ns)       (((uint32_t)(ns) * (FB_HRTIM_INPUT_CLOCK / 4)) / 1000000000U)

// 移相角范围
#define FB_PHASE_SHIFT_MIN          0U            // 最小移相角0度
#define FB_PHASE_SHIFT_MAX          180U          // 最大移相角180度

// 占空比范围（考虑死区，最大约45%）
#define FB_DUTY_MIN                 5U            // 最小占空比5%
#define FB_DUTY_MAX                 45U           // 最大占空比45%

// 定时器索引定义
#define FB_TIMER超前臂              HRTIM_TIMERINDEX_TIMER_A   // TimerA: Q1/Q2
#define FB_TIMER滞后臂              HRTIM_TIMERINDEX_TIMER_C   // TimerC: Q3/Q4
#define FB_TIMER_MASTER             HRTIM_TIMERINDEX_MASTER    // MASTER定时器

// 输出通道定义
#define FB_OUTPUT_Q1                HRTIM_OUTPUT_TA1           // Q1: TimerA输出1
#define FB_OUTPUT_Q2                HRTIM_OUTPUT_TA2           // Q2: TimerA输出2（互补）
#define FB_OUTPUT_Q3                HRTIM_OUTPUT_TC1           // Q3: TimerC输出1
#define FB_OUTPUT_Q4                HRTIM_OUTPUT_TC2           // Q4: TimerC输出2（互补）

// 比较单元定义
#define FB_COMPARE_UNIT_DUTY        HRTIM_COMPAREUNIT_2        // CMP2用于占空比控制
#define FB_COMPARE_UNIT_PHASE       HRTIM_COMPAREUNIT_2        // MASTER CMP2用于移相控制

// 故障保护通道（需根据实际硬件连接调整）
#define FB_FAULT_Q1Q2               (HRTIM_TIMFAULTENABLE_FAULT1 | HRTIM_TIMFAULTENABLE_FAULT2)
#define FB_FAULT_Q3Q4               (HRTIM_TIMFAULTENABLE_FAULT3 | HRTIM_TIMFAULTENABLE_FAULT4)

/*============================================================================
 *                          数据结构定义
 *============================================================================*/


enum{
				//炉头序号，作为外部索引，内部指向对应HRTIM OP  CMP BK
	PotCh1=0,	//	Pot1_TimerIndex=HRTIM_TIMERINDEX_TIMER_B,
	PotCh2,		//Pot2_TimerIndex=HRTIM_TIMERINDEX_TIMER_E,
	PotCh3,		//Pot3_TimerIndex=HRTIM_TIMERINDEX_TIMER_A,
	PotCh4,		//Pot4_TimerIndex=HRTIM_TIMERINDEX_TIMER_D,
	PotChTest1,
	PotChBase,
//	PotMaster,		//potmaster是最后一个
	PotMax,	
	PotAll=0xff,
	PotNum=PotChTest1,
};	



/**
 * @brief 移相全桥配置参数结构体（对应半桥的PPGvalueDef）
 */
typedef struct {
    uint16_t period;              // PPG周期（对应半桥prioed）
    uint16_t duty;                // PPG占空比（对应半桥duty）
    uint16_t phase_shift;         // 移相角（全桥特有）
} FB_PPGvalueDef;

/**
 * @brief 移相全桥时间点结构体（对应半桥的PPGpointDef）
 */
typedef struct {
    uint16_t highOn;              // 死区后高端开通（Q1/Q3）
    uint16_t highOff;             // 高端关闭（占空比结束点）
    uint16_t lowOn;               // 死区后低端开通（Q2/Q4）
    uint16_t lowOff;              // 低端关闭（周期结束点）
    uint16_t phaseShiftPoint;     // 移相点（全桥特有，TimerC相对TimerA的偏移）
} FB_PPGpointDef;

/**
 * @brief 移相全桥状态信息结构体
 */
typedef struct {
    uint32_t current_freq;        // 当前开关频率
    uint16_t current_duty;        // 当前占空比
    uint16_t current_phase_shift; // 当前移相角
    uint16_t period_count;        // 周期计数值
    uint16_t duty_count;          // 占空比计数值
    uint16_t phase_shift_count;   // 移相计数值
    uint16_t deadtime_count;      // 死区计数值
    uint8_t is_running;           // 运行状态标志
} FullBridgeStatusTypeDef;

/**
 * @brief HRTIM时间戳和状态结构体（用于DMA传输）
 */
typedef struct {
    uint16_t timer_a_count;       // TimerA计数器值
    uint16_t timer_c_count;       // TimerC计数器值
    uint16_t master_count;        // MASTER计数器值
    uint8_t output_status;        // 输出状态位[Q4|Q3|Q2|Q1]
} FullBridgeHrtimStatusTypeDef;

/*============================================================================
 *                    API函数声明（与半桥对应）
 *============================================================================*/

//----------------------- 初始化函数 -----------------------

/**
 * @brief 初始化移相全桥HRTIM（对应半桥API_HRTIM1_Init）
 * @note 配置MASTER、TimerA、TimerC，设置统一时钟源
 */
void API_FB_HRTIM1_Init(void);

/**
 * @brief 系统时钟初始化（对应半桥API_SystemClocks_Init）
 */
void API_FB_SystemClocks_Init(void);

//----------------------- PPG设置函数（与半桥对应） -----------------------

/**
 * @brief 设置连续输出模式（对应半桥API_PPG_SET_CONTINUOUS）
 * @param ch 通道号（全桥固定使用超前臂/滞后臂，此参数保留兼容性）
 */
void API_FB_PPG_SET_CONTINUOUS(uint8_t ch);

/**
 * @brief 设置单次脉冲模式（对应半桥API_PPG_SET_SINGLE）
 * @param ch 通道号
 */
void API_FB_PPG_SET_SINGLE(uint8_t ch);

/**
 * @brief 设置脉冲值（对应半桥API_PPG_setPluse）
 * @param ppgCh 通道号
 * @param value 脉冲值
 */
void API_FB_PPG_setPluse(uint8_t ppgCh, uint16_t value);

/**
 * @brief 设置指定通道周期（对应半桥API_PPG_setPeriodChx）
 * @param ppgCh 通道号
 * @param value 周期值
 * @note 全桥中所有通道共享同一周期（由MASTER决定）
 */
void API_FB_PPG_setPeriodChx(uint8_t ppgCh, uint16_t value);

/**
 * @brief 设置指定通道的周期和占空比（对应半桥API_PPG_setValueChx）
 * @param ppgCh 通道号
 * @param period 周期值
 * @param duty 占空比值
 */
void API_FB_PPG_setValueChx(uint8_t ppgCh, uint16_t period, uint16_t duty);

/**
 * @brief 设置PWM输出周期（对应半桥API_PPG_setPeriod）
 * @param value 周期值
 * @note 同时更新MASTER、TimerA、TimerC的周期寄存器
 */
void API_FB_PPG_setPeriod(uint16_t value);

/**
 * @brief 设置死区时间（对应半桥API_PPG_DeadTime）
 * @param ppgCh 通道号（0=超前臂，1=滞后臂）
 * @param upDts 上升沿死区
 * @param downDts 下降沿死区
 */
void API_FB_PPG_DeadTime(uint8_t ppgCh, uint8_t upDts, uint8_t downDts);

/**
 * @brief 设置PWM输出参数（对应半桥API_PPG_setValue）
 * @param ppgCh 通道号
 * @param value PPG参数结构体
 */
void API_FB_PPG_setValue(uint8_t ppgCh, FB_PPGvalueDef value);

/**
 * @brief 获取PWM输出参数（对应半桥API_PPG_getValue）
 * @param ppgCh 通道号
 * @return PPG参数结构体
 */
FB_PPGvalueDef API_FB_PPG_getValue(uint8_t ppgCh);

/**
 * @brief 获取减去死区的DUTY参数（对应半桥API_PPG_getValueDeadTime）
 * @param ppgCh 通道号
 * @return PPG时间点结构体
 */
FB_PPGpointDef API_FB_PPG_getValueDeadTime(uint8_t ppgCh);

/**
 * @brief 获取周期值（对应半桥API_PPG_GetPeroid）
 * @param ppgCh 通道号
 * @return 周期计数值
 */
uint32_t API_FB_PPG_GetPeroid(uint8_t ppgCh);

/**
 * @brief 获取死区值（对应半桥API_PPG_getDeadTime）
 * @param ppgCh 通道号
 * @return 死区计数值
 */
uint32_t API_FB_PPG_getDeadTime(uint8_t ppgCh);

/**
 * @brief PWM输出开关控制（对应半桥API_PPG_OnOff）
 * @param ppgCh 通道号（0xFF=全部通道）
 * @param flag 0=关闭，1=开启
 */
void API_FB_PPG_OnOff(uint8_t ppgCh, uint8_t flag);

//----------------------- 移相控制函数（全桥特有） -----------------------

/**
 * @brief 设置移相角（全桥核心功能）
 * @param ppgCh 通道号（PotCh1/PotCh2）
 * @param angle_deg 移相角度（0-180度）
 * @note 
 * - 第一阶段：两个炉头共享MASTER CMP1，移相角相同
 * - 第二阶段：可扩展为独立移相（使用不同MASTER CMP寄存器）
 */
void API_FB_SetPhaseShift(uint8_t ppgCh, uint16_t angle_deg);

/**
 * @brief 获取当前移相角
 * @param ppgCh 通道号（保留兼容性）
 * @return 移相角度（度）
 */
uint16_t API_FB_GetPhaseShift(uint8_t ppgCh);

/**
 * @brief 设置开关频率
 * @param freq_hz 开关频率（Hz）
 * @note 会重新计算并更新所有定时器的周期
 */
void API_FB_SetFrequency(uint32_t freq_hz);

/**
 * @brief 获取当前开关频率
 * @return 开关频率（Hz）
 */
uint32_t API_FB_GetFrequency(void);

//----------------------- 故障和保护函数（与半桥对应） -----------------------

/**
 * @brief 获取BK故障标志-Pot1（对应半桥API_PPG_BkFlag_Pot1）
 * @return 故障标志
 */
uint8_t API_FB_PPG_BkFlag_Pot1(void);

/**
 * @brief 获取BK故障标志-Pot2（对应半桥API_PPG_BkFlag_Pot2）
 */
uint8_t API_FB_PPG_BkFlag_Pot2(void);

/**
 * @brief 获取BK故障标志-Pot3（对应半桥API_PPG_BkFlag_Pot3）
 */
uint8_t API_FB_PPG_BkFlag_Pot3(void);

/**
 * @brief 获取BK故障标志-Pot4（对应半桥API_PPG_BkFlag_Pot4）
 */
uint8_t API_FB_PPG_BkFlag_Pot4(void);

/**
 * @brief 设置COMP选择-检锅模式（对应半桥API_Comp_SetSelPanCheck）
 * @param ppgCh 通道号
 */
void API_FB_Comp_SetSelPanCheck(uint8_t ppgCh);

/**
 * @brief 设置COMP选择-功率输出模式（对应半桥API_Comp_SetSelPowerOn）
 * @param ppgCh 通道号
 */
void API_FB_Comp_SetSelPowerOn(uint8_t ppgCh);

/**
 * @brief 禁用REST中断（对应半桥API_HRTIM_DISABLE_IT_REST）
 */
void API_FB_HRTIM_DISABLE_IT_REST(void);

/**
 * @brief 使能REST中断（对应半桥API_HRTIM_ENABLE_IT_REST）
 */
void API_FB_HRTIM_ENABLE_IT_REST(void);

/**
 * @brief 禁用BASE UPD中断（对应半桥API_HRTIM_BASE_DISABLE_IT_UPD）
 */
void API_FB_HRTIM_BASE_DISABLE_IT_UPD(void);

/**
 * @brief 使能BASE UPD中断（对应半桥API_HRTIM_BASE_ENABLE_IT_UPD）
 */
void API_FB_HRTIM_BASE_ENABLE_IT_UPD(void);

/**
 * @brief 禁用BASE CMP中断（对应半桥API_HRTIM_BASE_DISABLE_IT_CMP）
 */
void API_FB_HRTIM_BASE_DISABLE_IT_CMP(void);

/**
 * @brief 使能BASE CMP中断（对应半桥API_HRTIM_BASE_ENABLE_IT_CMP）
 */
void API_FB_HRTIM_BASE_ENABLE_IT_CMP(void);

/**
 * @brief 获取UPD中断标志（对应半桥API_HRTIM_GET_IT_UPD）
 * @param ch 通道号
 * @return 中断标志
 */
uint8_t API_FB_HRTIM_GET_IT_UPD(uint8_t ch);

/**
 * @brief 清除UPD中断标志（对应半桥API_HRTIM_CLEAR_IT_UPD）
 * @param ch 通道号
 */
void API_FB_HRTIM_CLEAR_IT_UPD(uint8_t ch);

/**
 * @brief 获取Test计数器值（对应半桥API_HRTIM_GetAddressTestCnt）
 * @return 计数器值
 */
uint32_t API_FB_HRTIM_GetAddressTestCnt(void);

/**
 * @brief 获取Txa地址计数器值（对应半桥API_HRTIM_GetAddressTxaCnt）
 * @param ch 通道号
 * @return 计数器值
 */
uint32_t API_FB_HRTIM_GetAddressTxaCnt(uint8_t ch);

/**
 * @brief 获取Txa计数器值（对应半桥API_HRTIM_GetTxaCnt）
 * @param ch 通道号
 * @return 计数器值
 */
uint32_t API_FB_HRTIM_GetTxaCnt(uint8_t ch);

/**
 * @brief 检锅脉冲检查（对应半桥API_HRTIM_CHECK_PAN_PLUSE）
 * @param ch 通道号
 */
void API_FB_HRTIM_CHECK_PAN_PLUSE(uint8_t ch);

/**
 * @brief HRTIM同步（对应半桥TIMsynchronous）
 */
void API_FB_TIMsynchronous(void);

/**
 * @brief HRTIM功率同步（对应半桥TIMsynchronousPower）
 */
void API_FB_TIMsynchronousPower(void);

/**
 * @brief 清除检锅起振CMP2中断标志（对应半桥API_HRTIM_PAN_CLEAR_FLAG）
 * @param ch 通道号
 */
void API_FB_HRTIM_PAN_CLEAR_FLAG(uint32_t ch);

/**
 * @brief 设置DMA句柄（对应半桥API_HRTIM_SetDmaHandle）
 * @param addr DMA目标地址
 */
void API_FB_HRTIM_SetDmaHandle(uint32_t* addr);

//----------------------- 全桥特有高级功能 -----------------------

/**
 * @brief 判断电流采集窗口（全桥核心功能）
 * @param hrtim_value 当前HRTIM计数器值
 * @param duty_count 占空比计数值
 * @param phase_shift_count 移相计数值
 * @param period 周期计数值
 * @return 0:不在窗口, 1:正向窗口(Q1+Q4), 2:反向窗口(Q2+Q3)
 * @note 用于power_calculator_fullbridge中判断对角管导通状态
 */
uint8_t API_FB_GetCurrentWindow(
    uint16_t hrtim_value,
    uint16_t duty_count,
    uint16_t phase_shift_count,
    uint16_t period
);

/**
 * @brief 软启动功能
 * @param target_freq 目标频率
 * @param target_duty 目标占空比
 * @param step_ms 每步间隔时间（ms）
 * @note 逐步增加频率和占空比到目标值
 */
void API_FB_SoftStart(uint32_t target_freq, uint16_t target_duty, uint16_t step_ms);

/**
 * @brief 软停止功能
 * @param step_ms 每步间隔时间（ms）
 * @note 逐步降低占空比和频率后停止
 */
void API_FB_SoftStop(uint16_t step_ms);

/**
 * @brief 故障恢复处理
 * @note 清除故障标志，重新初始化HRTIM
 */
void API_FB_FaultRecovery(void);

/**
 * @brief 获取HRTIM状态用于DMA传输
 * @param status 状态结构体指针
 * @note 供ADC DMA使用，传递HRTIM时间戳和输出状态
 */
void API_FB_GetHrtimStatusForDMA(FullBridgeHrtimStatusTypeDef* status);

/**
 * @brief 获取当前运行状态
 * @param status 状态结构体指针
 */
void API_FB_GetStatus(FullBridgeStatusTypeDef* status);

/**
 * @brief HRTIM中断回调函数（弱定义，用户可重写）
 * @param timer_id 触发中断的定时器ID
 * @param interrupt_flag 中断标志
 * @note 在rx32g4xx_it.c中调用
 */
__weak void API_FB_IRQHandler_Callback(uint8_t timer_id, uint32_t interrupt_flag);

// 外部变量声明（在.c文件中定义）
extern FullBridgeStatusTypeDef fb_status;

#ifdef __cplusplus
}
#endif

#endif /* __HRTIM_FULLBRIDGE_H__ */
