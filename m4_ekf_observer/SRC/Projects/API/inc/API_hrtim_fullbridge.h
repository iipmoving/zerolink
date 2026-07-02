/**
 ******************************************************************************
 * @file    API_hrtim_fullbridge.h
 * @brief   全桥HRTIM驱动API头文件
 * @note    基于现有半桥API (API_hrtim.h) 扩展，不修改原有代码
 *          - 使用 PotCh1(TimerB)+PotCh2(TimerE) 或 PotCh3(TimerA)+PotCh4(TimerD) 组成全桥
 *          - 支持独立模式和Master同步模式
 *          - 支持调频模式和调功模式
 *          - ★ APP层不暴露任何HAL层类型/宏/常量，全部通过PotChX封装 ★
 ******************************************************************************
 */

#ifndef __HRTIM_FULLBRIDGE_H__
#define __HRTIM_FULLBRIDGE_H__

#include <stdint.h>
#include "API_hrtim.h"       /* 仅使用 PotCh1/PotCh2 等通道常量、PPGvalueDef */

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 *                        【新增】全桥模式枚举定义
 *============================================================================*/

/** @brief 同步模式选择 */
typedef enum {
    FB_SYNC_MODE_INDEPENDENT = 0,   /*!< 独立模式: 保留原半桥行为，各通道独立周期 */
    FB_SYNC_MODE_MASTER     = 1,    /*!< Master同步模式: 选择Master通道作为同步源，统一周期 */
} FB_SyncModeTypeDef;

/** @brief 全桥工作模式选择 */
typedef enum {
    FB_BRIDGE_MODE_FREQ_MOD  = 0,  /*!< 调频模式: 频率、相位可独立设置，前后桥臂50%互补输出 */
    FB_BRIDGE_MODE_POWER_MOD = 1,  /*!< 调功模式: 前后桥臂互补输出，使用统一CMP值调节 */
} FB_BridgeModeTypeDef;

/*============================================================================
 *                        【新增】全桥配置结构体 (仅标准C类型，无HAL依赖)
 *============================================================================*/

/**
 * @brief 全桥死区配置 (每桥臂独立)
 * @note  计数值与HRTIM分频系数相关, DIV4时每count≈333ns
 */
typedef struct {
    uint16_t risingValue;           /*!< 超前臂上升沿死区计数值 */
    uint16_t fallingValue;          /*!< 超前臂下降沿死区计数值 */
    uint16_t risingValueLag;        /*!< 滞后臂上升沿死区计数值 */
    uint16_t fallingValueLag;       /*!< 滞后臂下降沿死区计数值 */
} FB_DeadTimeTypeDef;

/**
 * @brief 全桥PWM参数结构体 (对应原 PPGvalueDef)
 */
typedef struct {
    uint16_t period;                /*!< PWM周期计数值 */
    uint16_t cmpValue;              /*!< 统一CMP比较值 (调功模式: 控制占空比) */
    uint16_t phaseShift;            /*!< 桥臂间相位差计数值 (调频模式: 移相控制) */
} FB_ValueTypeDef;

/**
 * @brief 全桥主配置结构体
 * @note  通过 API_FB_GetConfig() 获取指定通道的配置指针
 */
typedef struct {
    /* ---- 同步配置 ---- */
    FB_SyncModeTypeDef   syncMode;          /*!< 同步模式: 独立 or Master同步 */

    /* ---- 全桥模式 ---- */
    FB_BridgeModeTypeDef bridgeMode;        /*!< 全桥模式: 调频 or 调功 */

    /* ---- 死区配置 ---- */
    FB_DeadTimeTypeDef   deadTime;          /*!< 死区参数 */

    /* ---- 运行参数 ---- */
    FB_ValueTypeDef      value;             /*!< 当前PWM参数 */

    /* ---- 运行状态 ---- */
    uint8_t              isRunning;         /*!< 运行状态标志: 0=停止, 1=运行中 */
} FB_ConfigTypeDef;

/*============================================================================
 *                        【新增】全桥配置宏定义 (API层可见，均不含HAL依赖)
 *============================================================================*/

/* 默认频率25kHz */
#define FB_DEFAULT_FREQ_HZ          25000U

/* 默认死区计数值 (约1.5us @ DIV4, 192MHz HRTIM clock) */
#define FB_DEFAULT_DEADTIME_RISING  48U
#define FB_DEFAULT_DEADTIME_FALLING 48U

/* 默认占空比百分比 (调功模式, 30%) */
#define FB_DEFAULT_DUTY_PERCENT     30U

/*============================================================================
 *               【新增】全桥API函数声明 (通道用PotCh1/PotCh2标识)
 *============================================================================*
 *  全桥通道映射:
 *    fbCh = PotCh1 (0): 超前臂PotCh1(TimerB) + 滞后臂PotCh2(TimerE)
 *    fbCh = PotCh2 (1): 超前臂PotCh3(TimerA) + 滞后臂PotCh4(TimerD)
 *
 *  所有函数中的 fbCh 参数取值为 PotCh1 或 PotCh2
 *============================================================================*/

/* ------- 初始化函数 (两种同步模式分别独立函数) ------- */

/**
 * @brief  全桥初始化 - 独立同步模式
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @note   各通道保留独立周期，不依赖Master定时器
 *         此函数仅配置fbCh对应的超前/滞后臂，不影响其他通道
 */
void API_FB_Init_Independent(uint8_t fbCh);

/**
 * @brief  全桥初始化 - Master同步模式
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @note   使用Master定时器作为统一同步源，所有Slave通道共享统一周期
 *         超前臂由Master PER复位，滞后臂由Master CMP1复位(实现移相)
 */
void API_FB_Init_MasterSync(uint8_t fbCh);

/* ------- 频率与参数设置 ------- */

/**
 * @brief  设置全桥频率
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  freqHz: 开关频率 (Hz)
 * @note   自动计算并更新各定时器的周期寄存器
 *         调频模式: 自动保持CMP2=PER/2 (50%占空比)
 *         调功模式: 按比例缩放CMP值保持占空比
 */
void API_FB_SetFrequency(uint8_t fbCh, uint32_t freqHz);

/**
 * @brief  设置全桥相位差
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  phaseShiftCount: 相位差计数值 (0 ~ period-1)
 * @note   调频模式: 设置滞后臂相对超前臂的相位偏移 (移相全桥)
 *         Master同步模式: 通过更新Master CMP1实现
 */
void API_FB_SetPhaseShift(uint8_t fbCh, uint16_t phaseShiftCount);

void API_FB_SetPhaseShiftSimple(uint8_t fbCh, uint16_t phaseShiftCount); //不檢查狀態,直接更新相移
uint16_t API_FB_GetPhaseShiftSimple(uint8_t fbCh); //不檢查狀態,直接得到相移

void API_FB_SetPreiodCountSimple(uint8_t fbCh, uint32_t period);         //不檢查狀態,直接更新週期 相移要單獨算   
uint16_t API_FB_GetPreiodCountSimple(uint8_t fbCh);					//PWM周期返回


/**
 * @brief  设置统一CMP值 (调功模式核心接口)
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  cmpValue: CMP比较值 (0 ~ period-1)
 * @note   一次调用同时设置超前臂和滞后臂的CMP2值
 *         死区由硬件自动插入
 */
void API_FB_SetCmpValue(uint8_t fbCh, uint16_t cmpValue);

/* ------- 启动/停止控制 ------- */

/**
 * @brief  全桥启动 (参考原 API_PPG_OnOff)
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @note   启动Master计数器(同步模式)和各通道计数器，使能PWM输出引脚
 */
void API_FB_Start(uint8_t fbCh);

/**
 * @brief  全桥停止 (参考原 API_PPG_OnOff)
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @note   停止PWM输出和计数器
 */
void API_FB_Stop(uint8_t fbCh);

/* ------- 同步模式切换 ------- */

/**
 * @brief  同步模式切换
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  newMode: 目标同步模式
 * @note   切换前需先停止全桥输出，切换后重新初始化定时器
 */
void API_FB_SwitchSyncMode(uint8_t fbCh, FB_SyncModeTypeDef newMode);

/**
 * @brief  全桥模式切换 (调频/调功)
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  newMode: 目标全桥模式
 * @note   切换前需先停止全桥输出
 */
void API_FB_SwitchBridgeMode(uint8_t fbCh, FB_BridgeModeTypeDef newMode);

/* ------- 死区设置 ------- */

/**
 * @brief  设置全桥死区时间
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  risingNs: 上升沿死区 (ns)
 * @param  fallingNs: 下降沿死区 (ns)
 * @note   自动转换为HRTIM计数值，同时设置超前臂和滞后臂
 */
void API_FB_SetDeadTime(uint8_t fbCh, uint16_t risingNs, uint16_t fallingNs);

/* ------- 状态获取 ------- */

/** @brief 获取全桥当前周期值 */
uint32_t API_FB_GetPeriod(uint8_t fbCh);

/** @brief 获取全桥当前频率 (Hz) */
uint32_t API_FB_GetFrequency(uint8_t fbCh);

/** @brief 获取当前全桥参数 */
FB_ValueTypeDef API_FB_GetValue(uint8_t fbCh);

/** @brief 获取全桥配置结构体指针 (用于批量读写配置) */
FB_ConfigTypeDef* API_FB_GetConfig(uint8_t fbCh);

/* ------- 全桥特有 - 调频/调功波形输出 (一站式初始化+启动) ------- */

/**
 * @brief  输出调频波形 (频率调制模式)
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  freqHz: 初始频率 (Hz)
 * @param  phaseShiftCount: 初始相位差计数值
 * @note   初始化前后桥臂为50%占空比互补输出，可独立调节频率和相位
 *         内部调用 Init_MasterSync + Start
 */
void API_FB_OutputFreqModulation(uint8_t fbCh, uint32_t freqHz, uint16_t phaseShiftCount);

/**
 * @brief  输出调功波形 (功率调制模式)
 * @param  fbCh: 全桥通道号 (PotCh1 或 PotCh2)
 * @param  freqHz: 初始频率 (Hz)
 * @param  cmpValue: 初始CMP比较值
 * @note   初始化前后桥臂互补输出，使用统一CMP值调节功率
 *         内部调用 Init_MasterSync + Start
 */
void API_FB_OutputPowerModulation(uint8_t fbCh, uint32_t freqHz, uint16_t cmpValue);

/**
 * @brief 全桥检锅单脉冲启动 — HRTIM SimpleOnePulse 模式
 *        超前臂 + 滞后臂同时各输出 1 个脉冲后自动停止
 * @param fbCh 全桥通道号 (PotCh1=0, PotCh2=1)
 * @note  脉宽 = 当前 HRTIM 周期 × 60%
 *        完成后通过 HRTIM 中断通知
 */
void API_FB_SinglePulseStart(uint8_t fbCh);




#ifdef __cplusplus
}
#endif

#endif /* __HRTIM_FULLBRIDGE_H__ */
