/********************************************************************************
 * 文件名: app_power_io.h
 * 描述: APP_POWER 模块的 I/O 结构体定义（v2.2 PULL 架构）
 * 说明: 将 HRTIM 操作从函数指针调用改为数据流模式
 *       - 输入：HRTIM 状态反馈（BK标志、PPG当前值等）
 *       - 输出：HRTIM 控制命令（PPG设置、开关控制、死区时间等）
 ********************************************************************************/

#ifndef _APP_POWER_IO_H_
#define _APP_POWER_IO_H_

#include <stdint.h>
#include "app_power_constants.h"

/* ===================================================================
 * HRTIM 控制命令结构体（输出到 DRV_HRTIM）
 * =================================================================== */

// PPG 控制命令
typedef struct {
    uint8_t     cmd_valid;       // 命令有效标志 (1=有效)
    uint8_t     cmd_type;        // 命令类型
    uint16_t    period;          // PPG 周期值
    uint16_t    duty;            // PPG 占空比值
    uint8_t     dead_time_up;    // 上管死区时间
    uint8_t     dead_time_down;  // 下管死区时间
    uint8_t     enable;          // 使能控制 (1=开启, 0=关闭)
} AppPower_HrtimPpgCmd_t;

// HRTIM 通道控制命令（4个通道）
typedef struct {
    AppPower_HrtimPpgCmd_t  ch[4];      // 4个通道的 PPG 控制命令
    uint8_t                 sync_enable; // 主同步使能
    uint16_t                sync_period; // 主同步周期
} AppPower_HrtimCmd_t;

/* ===================================================================
 * HRTIM 状态反馈结构体（从 DRV_HRTIM 输入）
 * =================================================================== */

// PPG 状态反馈
typedef struct {
    uint16_t    period;          // 当前 PPG 周期
    uint16_t    duty;            // 当前 PPG 占空比
    uint8_t     bk_flag;         // 刹车标志
    uint8_t     status;          // 状态标志
} AppPower_HrtimPpgStatus_t;

// HRTIM 通道状态反馈（4个通道）
typedef struct {
    AppPower_HrtimPpgStatus_t  ch[4];   // 4个通道的状态反馈
    uint8_t                    sync_status; // 主同步状态
} AppPower_HrtimStatus_t;

/* ===================================================================
 * APP_POWER 输入结构体
 * =================================================================== */
typedef struct {
    // HRTIM 状态反馈
    AppPower_HrtimStatus_t  hrtim_status;
    
    // ADC 采样值
    uint16_t    adc_voltage;     // 电压采样值
    uint16_t    adc_current;     // 电流采样值
    uint16_t    adc_igbt_temp;   // IGBT 温度采样值
    uint16_t    adc_bottom_temp; // 底部温度采样值
    uint16_t    adc_top_temp;    // 顶部温度采样值
    
    // 控制输入
    uint8_t     power_enable;     // 功率使能
    uint16_t    target_power;     // 目标功率 (0-2000W)
    uint8_t     pot_type;         // 锅具类型 (0=无锅, 1=铁锅, 2=钢锅)
    
    // 保护输入
    uint8_t     protect_ovp;      // 过压保护标志
    uint8_t     protect_ocp;      // 过流保护标志
    uint8_t     protect_otp;      // 过热保护标志
    
    // 系统状态
    uint8_t     sys_status;       // 系统状态
    uint32_t    sys_tick;         // 系统时钟 tick
} APP_POWER_Input_t;

/* ===================================================================
 * APP_POWER 输出结构体
 * =================================================================== */
typedef struct {
    // HRTIM 控制命令
    AppPower_HrtimCmd_t     hrtim_cmd;
    
    // 状态输出
    uint8_t     power_status;     // 功率状态
    uint16_t    actual_power;     // 实际功率
    uint16_t    ppg_value;        // 当前 PPG 值
    uint8_t     pot_detected;     // 锅具检测结果
    uint8_t     error_code;       // 错误代码
    
    // 保护输出
    uint8_t     protect_trigger;  // 保护触发标志
    uint8_t     protect_action;   // 保护动作
    
    // 调试信息
    uint16_t    debug_data[8];    // 调试数据
} APP_POWER_Output_t;

/* ===================================================================
 * 模块导出声明（遵循 v2.2 PULL 架构）
 * =================================================================== */
MODULE_IO_H(APP_POWER);

#endif // _APP_POWER_IO_H_