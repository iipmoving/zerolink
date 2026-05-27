/**
 * ============================================================================
 * DLL包装层头文件
 * ============================================================================
 * 
 * 定义Windows DLL导出接口
 * 所有函数都使用 __declspec(dllexport) 标记
 * ============================================================================
 */

#ifndef DLL_WRAPPER_H
#define DLL_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "../c_logic/emc_logic.h"

/* Windows DLL导出宏 */
#ifdef EMC_LOGIC_EXPORTS
    #define EMC_API __declspec(dllexport)
#else
    #define EMC_API __declspec(dllimport)
#endif

/* ========================================================================
 * DLL导出函数声明
 * ======================================================================== */

/**
 * @brief 初始化EMC逻辑层（必须在其他操作前调用）
 */
EMC_API void emc_initialize(void);

/**
 * @brief 清理资源（可选）
 */
EMC_API void emc_cleanup(void);

/**
 * @brief 统一注册所有回调函数
 * @param p_disp_cb 显示输出回调
 * @param p_buzzer_cb 蜂鸣器回调
 * @param p_status_cb 状态变化回调
 * @param p_hw_inputs_cb 硬件输入回调
 */
EMC_API void emc_register_callbacks(
    DispOutputCb_t p_disp_cb,
    BuzzerCb_t p_buzzer_cb,
    StatusChangeCb_t p_status_cb,
    GetHardwareInputsCb_t p_hw_inputs_cb);

/**
 * @brief 模拟按键输入（供Python调用）
 * @param key_code 按键枚举值（KeyCode_e）
 * @param event 事件类型（key_event_t）
 */
EMC_API void emc_simulate_key_press(uint8_t key_code, uint8_t event);

/**
 * @brief 执行10ms周期主循环
 */
EMC_API void emc_run_cycle(void);

/**
 * @brief 获取当前系统状态
 * @return 当前状态枚举值（SystemState_e）
 */
EMC_API uint8_t emc_get_current_state(void);

/**
 * @brief 获取显示缓冲区（段码格式）
 * @param seg_buffer 输出缓冲区（至少4字节）
 * @param dp_mask 小数点掩码输出
 * @param colon_mask 时钟点掩码输出
 */
EMC_API void emc_get_display_output(
    uint8_t *seg_buffer,
    uint8_t *dp_mask,
    uint8_t *colon_mask);

/**
 * @brief 获取LED状态
 * @return LED状态结构体
 */
EMC_API LedState_t emc_get_led_state(void);

#ifdef __cplusplus
}
#endif

#endif /* DLL_WRAPPER_H */
