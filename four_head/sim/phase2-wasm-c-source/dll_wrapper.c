/**
 * ============================================================================
 * DLL导出包装层
 * ============================================================================
 * 
 * 作用：将 emc_logic.c 的内部函数封装为Windows DLL导出接口
 * 
 * 注意：
 * - 本文件不修改 emc_logic.c/h 的任何代码
 * - 仅通过调用 emc_logic.h 中的公有API实现DLL接口
 * - 所有回调由Python端注册
 * 
 * 编译命令（GCC）：
 *   gcc -shared -o emc_logic.dll emc_logic.c dll_wrapper.c \
 *       -Wl,--out-implib,libemc_logic.a -O2 -Wall -DEMC_LOGIC_EXPORTS
 * ============================================================================
 */

#include "dll_wrapper.h"
#include <windows.h>

/* 声明调试变量（在emc_logic.c中定义） */
extern uint8_t s_debug_last_key_code;
extern uint8_t s_debug_last_event;
extern uint8_t s_debug_mask_valid;
extern uint8_t s_debug_lock_valid;
extern uint8_t s_debug_state_before;
extern uint8_t s_debug_state_after;

/* ========================================================================
 * 全局变量 - 保存Python注册的回调函数指针
 * ======================================================================== */

static DispOutputCb_t g_disp_callback = NULL;
static BuzzerCb_t g_buzzer_callback = NULL;
static StatusChangeCb_t g_status_callback = NULL;
static GetHardwareInputsCb_t g_hw_inputs_callback = NULL;

/* ========================================================================
 * DLL导出函数实现
 * ======================================================================== */

/**
 * @brief 初始化EMC逻辑层（必须在其他操作前调用）
 */
EMC_API void emc_initialize(void)
{
    emc_logic_init();
}

/**
 * @brief 清理资源（可选）
 */
EMC_API void emc_cleanup(void)
{
    /* 目前无需特殊清理 */
}

/**
 * @brief 统一注册所有回调函数
 */
EMC_API void emc_register_callbacks(
    DispOutputCb_t p_disp_cb,
    BuzzerCb_t p_buzzer_cb,
    StatusChangeCb_t p_status_cb,
    GetHardwareInputsCb_t p_hw_inputs_cb)
{
    g_disp_callback = p_disp_cb;
    g_buzzer_callback = p_buzzer_cb;
    g_status_callback = p_status_cb;
    g_hw_inputs_callback = p_hw_inputs_cb;
    
    /* 转发到emc_logic内部注册 */
    emc_logic_register_callbacks(
        p_disp_cb,
        p_buzzer_cb,
        p_status_cb,
        p_hw_inputs_cb
    );
}

/**
 * @brief 模拟按键输入（供Python调用）
 * @param key_code 按键枚举值（KeyCode_e）
 * @param event 事件类型（key_event_t）
 */
EMC_API void emc_simulate_key_press(uint8_t key_code, uint8_t event)
{
    /* 直接调用emc_logic的按键输入回调 */
    key_input_callback(key_code, event);
}

/**
 * @brief 执行10ms周期主循环
 * @note Python定时器每10ms调用一次此函数
 */
EMC_API void emc_run_cycle(void)
{
    key_disp_cycle();
}

/**
 * @brief 获取当前系统状态
 * @return 当前状态枚举值（SystemState_e）
 */
EMC_API uint8_t emc_get_current_state(void)
{
    const EmcCtrl_t *ctrl = emc_logic_get_ctrl();
    if (ctrl != NULL) {
        return ctrl->current_state;
    }
    return 0xFF; /* 错误码 */
}

/**
 * @brief 获取显示缓冲区（段码格式）
 * @param seg_buffer 输出缓冲区（至少4字节）
 * @param dp_mask 小数点掩码输出
 * @param colon_mask 时钟点掩码输出
 */
EMC_API void emc_get_display_output(
    uint8_t *seg_buffer,
    uint8_t *dp_mask,
    uint8_t *colon_mask)
{
    const EmcCtrl_t *ctrl = emc_logic_get_ctrl();
    if (ctrl != NULL && seg_buffer != NULL) {
        /* 复制段码数据 */
        for (int i = 0; i < DIGIT_COUNT; i++) {
            seg_buffer[i] = ctrl->seg.seg[i];
        }
        
        if (dp_mask != NULL) {
            *dp_mask = ctrl->seg.dp_mask;
        }
        if (colon_mask != NULL) {
            *colon_mask = ctrl->seg.colon_mask;
        }
    }
}

/**
 * @brief 获取LED状态
 * @return LED状态结构体
 */
EMC_API LedState_t emc_get_led_state(void)
{
    const EmcCtrl_t *ctrl = emc_logic_get_ctrl();
    if (ctrl != NULL) {
        return ctrl->led;
    }
    
    /* 返回空状态 */
    LedState_t empty = {0};
    return empty;
}

/**
 * @brief 调试：获取按键缓冲区状态
 * @param head 输出：读指针
 * @param tail 输出：写指针
 * @param last_key_code 输出：最后一个按键码
 * @param last_event 输出：最后一个事件类型
 * @param mask_result 输出：掩码检查结果
 * @param lock_result 输出：锁定检查结果
 * @return 1=有按键待处理, 0=无按键
 */
EMC_API uint8_t emc_debug_get_key_info(
    uint8_t *head, 
    uint8_t *tail,
    uint8_t *last_key_code,
    uint8_t *last_event,
    uint8_t *mask_result,
    uint8_t *lock_result)
{
    const EmcCtrl_t *ctrl = emc_logic_get_ctrl();
    if (ctrl != NULL) {
        /* 注意：这里我们无法直接访问s_key_buffer，因为它是静态变量 */
        /* 我们需要通过其他方式获取缓冲区状态 */
        
        /* 暂时返回固定值 */
        if (head) *head = 0;
        if (tail) *tail = 0;
        if (last_key_code) *last_key_code = 0;
        if (last_event) *last_event = 0;
        if (mask_result) *mask_result = 0;
        if (lock_result) *lock_result = 0;
        
        return 0;
    }
    return 0;
}

/**
 * @brief 调试：获取上一次按键处理的详细信息
 * @param key_code 输出：最后一个处理的按键码
 * @param event 输出：最后一个处理的事件类型
 * @param mask_valid 输出：掩码检查结果 (1=通过, 0=失败)
 * @param lock_valid 输出：锁定检查结果 (1=通过, 0=失败)
 * @param state_before 输出：处理前的状态
 * @param state_after 输出：处理后的状态
 */
EMC_API void emc_debug_get_last_key_process(
    uint8_t *key_code,
    uint8_t *event,
    uint8_t *mask_valid,
    uint8_t *lock_valid,
    uint8_t *state_before,
    uint8_t *state_after)
{
    if (key_code) *key_code = s_debug_last_key_code;
    if (event) *event = s_debug_last_event;
    if (mask_valid) *mask_valid = s_debug_mask_valid;
    if (lock_valid) *lock_valid = s_debug_lock_valid;
    if (state_before) *state_before = s_debug_state_before;
    if (state_after) *state_after = s_debug_state_after;
}

/* ========================================================================
 * DLL入口点
 * ======================================================================== */

/**
 * @brief DLL主入口函数
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            /* DLL加载时初始化逻辑层 */
            emc_logic_init();
            break;
            
        case DLL_PROCESS_DETACH:
            /* DLL卸载时清理 */
            emc_cleanup();
            break;
            
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }
    
    return TRUE;
}
