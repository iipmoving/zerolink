/**
 * WASM 包装器 - 桥接C代码和JavaScript
 */

#include "emc_logic.h"
#include <emscripten.h>
#include <stdint.h>

// ============================================================================
// JS回调函数（使用EM_JS宏）
// ============================================================================

EM_JS(void, js_disp_output, (const SegCode_t *p_seg, const LedState_t *p_led), {
    // 这个函数会在JS层被覆盖
});

EM_JS(void, js_buzzer_output, (uint8_t cmd), {
    // 这个函数会在JS层被覆盖
});

EM_JS(void, js_status_change, (uint8_t type, void *p_value), {
    // 这个函数会在JS层被覆盖
});

// ============================================================================
// 导出给JS调用的函数
// ============================================================================

/**
 * 初始化逻辑层
 */
EMSCRIPTEN_KEEPALIVE
void wasm_emc_init(void) {
    emc_logic_init();
}

/**
 * 10ms周期主循环
 */
EMSCRIPTEN_KEEPALIVE
void wasm_key_disp_cycle(void) {
    key_disp_cycle();
}

/**
 * 按键输入
 */
EMSCRIPTEN_KEEPALIVE
void wasm_key_input(uint8_t key_code, uint8_t event) {
    key_input_callback(key_code, event);
}

/**
 * 获取控制结构指针（用于调试）
 */
EMSCRIPTEN_KEEPALIVE
const EmcCtrl_t* wasm_get_ctrl(void) {
    return emc_logic_get_ctrl();
}

// ============================================================================

/**
 * 显示输出回调实现
 */
static void disp_output_bridge(SegCode_t *p_seg, LedState_t *p_led) {
    if (js_disp_output) {
        js_disp_output(p_seg, p_led);
    }
}

/**
 * 蜂鸣器回调实现
 */
static void buzzer_output_bridge(uint8_t cmd) {
    if (js_buzzer_output) {
        js_buzzer_output(cmd);
    }
}

/**
 * 状态变化回调实现
 */
static void status_change_bridge(uint8_t type, void *p_value) {
    if (js_status_change) {
        js_status_change(type, p_value);
    }
}

/**
 * 注册所有回调
 */
EMSCRIPTEN_KEEPALIVE
void wasm_register_callbacks(void) {
    emc_logic_register_callbacks(
        disp_output_bridge,
        buzzer_output_bridge,
        status_change_bridge,
        NULL  // get_hw_inputs 暂不使用
    );
}
