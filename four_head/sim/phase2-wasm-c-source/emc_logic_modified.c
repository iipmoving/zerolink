/**
 ******************************************************************************
 * @file    emc_logic.c
 * @author  EMC Logic Generator
 * @version V1.0
 * @date    2026-05-16
 * @brief   翰林煮面炉·数码管款 — 逻辑层实现
 *
 * @details
 *   实现煮面炉核心状态机，包括：
 *   1. 9状态完整覆盖（S_POWER_ON ~ S_FAULT）
 *   2. 掩码表驱动按键过滤
 *   3. 状态转移表驱动状态切换
 *   4. 10ms周期主循环（key_disp_cycle）
 *   5. 显示逻辑更新 + 段码转换
 *   6. 锁定检查（首次上电/扫码/水位不足）
 *
 * @note
 *   - 不包含任何 HAL 层头文件
 *   - 所有外部硬件访问通过回调接口实现
 *   - 所有 BOOL 变量集中在 EmcFlags_t 位域
 *
 * Copyright (c) 2026
 ******************************************************************************
 */

#include "emc_logic.h"

/* ========================================================================
 * 私有宏定义
 * ======================================================================== */

/* 按键掩码定义 — 每bit对应一个key_code */
#define MASK_KEY_POWER        (1U << 0)
#define MASK_KEY_M1           (1U << 1)
#define MASK_KEY_M2           (1U << 2)
#define MASK_KEY_M3           (1U << 3)
#define MASK_KEY_M4           (1U << 4)
#define MASK_KEY_M5           (1U << 5)
#define MASK_KEY_M6           (1U << 6)
#define MASK_KEY_M7           (1U << 7)
#define MASK_KEY_M8           (1U << 8)
#define MASK_KEY_M9           (1U << 9)
#define MASK_KEY_M10          (1U << 10)
#define MASK_KEY_ADD_WATER    (1U << 11)
#define MASK_KEY_ADD_TIME     (1U << 12)
#define MASK_KEY_START_PAUSE  (1U << 13)

#define MASK_M1_M10 ( \
    MASK_KEY_M1  | MASK_KEY_M2  | MASK_KEY_M3  | MASK_KEY_M4  | MASK_KEY_M5 | \
    MASK_KEY_M6  | MASK_KEY_M7  | MASK_KEY_M8  | MASK_KEY_M9  | MASK_KEY_M10 )

#define MASK_ANY_FUNC_KEY ( \
    MASK_M1_M10 | MASK_KEY_ADD_WATER | MASK_KEY_ADD_TIME | MASK_KEY_START_PAUSE )

/* ========================================================================
 * 静态常量 — 功能配置表（索引1-10对应M1-M10，规格书要求不得硬编码）
 * ======================================================================== */

static const FuncConfig_t s_func_configs[FUNC_CONFIG_COUNT] = {
    [1]  = { .default_water = 200, .default_time = 180 },   /* M1: 200ml, 3分钟  */
    [2]  = { .default_water = 250, .default_time = 240 },   /* M2: 250ml, 4分钟  */
    [3]  = { .default_water = 300, .default_time = 300 },   /* M3: 300ml, 5分钟  */
    [4]  = { .default_water = 200, .default_time = 120 },   /* M4: 200ml, 2分钟  */
    [5]  = { .default_water = 350, .default_time = 360 },   /* M5: 350ml, 6分钟  */
    [6]  = { .default_water = 400, .default_time = 420 },   /* M6: 400ml, 7分钟  */
    [7]  = { .default_water = 150, .default_time = 90  },   /* M7: 150ml, 1.5分钟*/
    [8]  = { .default_water = 450, .default_time = 480 },   /* M8: 450ml, 8分钟  */
    [9]  = { .default_water = 250, .default_time = 200 },   /* M9: 250ml, ~3.3分钟*/
    [10] = { .default_water = 300, .default_time = 250 },   /* M10: 300ml, ~4.2分钟*/
};

/* ========================================================================
 * 静态常量 — 按键掩码表（按状态定义，state_key_masks[current_state]查表）
 * ======================================================================== */

static const KeyMask_t s_state_key_masks[STATE_MAX] = {
    /* S_POWER_ON = 0: 所有按键锁定 */
    [S_POWER_ON]    = { .short_mask = 0,                   .long_mask = 0 },

    /* S_VERSION = 1: 仅电源键快速开机 */
    [S_VERSION]     = { .short_mask = MASK_KEY_POWER,      .long_mask = 0 },

    /* S_SHUTDOWN = 2: 仅电源键短按开机 */
    [S_SHUTDOWN]    = { .short_mask = MASK_KEY_POWER,      .long_mask = 0 },

    /* S_DEMO = 3: 仅电源键退出演示 */
    [S_DEMO]        = { .short_mask = MASK_KEY_POWER,      .long_mask = 0 },

    /* S_STANDBY = 4: M1-M10选择功能 + ADD_TIME调整时间 + POWER长按关机 */
    [S_STANDBY]     = {
        .short_mask = MASK_M1_M10 | MASK_KEY_ADD_TIME,
        .long_mask  = MASK_KEY_POWER
    },

    /* S_FUNC_SELECT = 5: M1-M10切换 + ADD_WATER/ADD_TIME + START_PAUSE短按启动/长按取消 */
    [S_FUNC_SELECT] = {
        .short_mask = MASK_M1_M10 | MASK_KEY_ADD_WATER | MASK_KEY_ADD_TIME | MASK_KEY_START_PAUSE,
        .long_mask  = MASK_KEY_START_PAUSE        /* 长按取消 → S_STANDBY */
    },

    /* S_COOKING = 6: START_PAUSE短按暂停 + START_PAUSE长按取消 */
    [S_COOKING]     = {
        .short_mask = MASK_KEY_START_PAUSE,
        .long_mask  = MASK_KEY_START_PAUSE
    },

    /* S_PAUSE = 7: START_PAUSE短按恢复 + START_PAUSE长按取消 */
    [S_PAUSE]       = {
        .short_mask = MASK_KEY_START_PAUSE,
        .long_mask  = MASK_KEY_START_PAUSE
    },

    /* S_FAULT = 8: 所有按键锁定（SAF-01） */
    [S_FAULT]       = { .short_mask = 0,                   .long_mask = 0 },
};

/* ========================================================================
 * 静态常量 — ASCII→段码查表（7段数码管，bit0=a, bit1=b, ... bit6=g, bit7=DP）
 * ======================================================================== */

static const uint8_t s_ascii_to_segcode[128] = {
    ['0'] = 0x3F, ['1'] = 0x06, ['2'] = 0x5B, ['3'] = 0x4F,
    ['4'] = 0x66, ['5'] = 0x6D, ['6'] = 0x7D, ['7'] = 0x07,
    ['8'] = 0x7F, ['9'] = 0x6F,
    ['A'] = 0x77, ['B'] = 0x7C, ['C'] = 0x39, ['D'] = 0x5E,
    ['E'] = 0x79, ['F'] = 0x71, ['G'] = 0x3D, ['H'] = 0x76,
    ['I'] = 0x06, ['J'] = 0x1E, ['L'] = 0x38, ['N'] = 0x37,
    ['O'] = 0x3F, ['P'] = 0x73, ['U'] = 0x3E,
    ['-'] = 0x40, [' '] = 0x00, ['_'] = 0x08,
    ['c'] = 0x58, ['d'] = 0x5E, ['n'] = 0x54, ['o'] = 0x5C,
    ['r'] = 0x50, ['t'] = 0x78, ['y'] = 0x6E,
};

/* ========================================================================
 * 静态变量 — 全局控制器实例 + 按键缓冲区
 * ======================================================================== */

static EmcCtrl_t s_emc;                            /* 主控制结构体 */
static KeyBuffer_t s_key_buffer;                    /* 按键循环缓冲 */
static StateTransition_t s_state_trans_table[STATE_MAX][EVENT_MAX];  /* 转移表 */
static uint8_t s_any_key_processed;                /* 本周期是否有按键被处理 */

/* ========================================================================
 * 静态函数声明
 * ======================================================================== */

/* --- 状态转移表初始化 --- */
static void init_state_trans_table(void);

/* --- 按键处理 --- */
static uint8_t is_key_valid(uint8_t key_code, uint8_t event);
static uint8_t check_lock_conditions(const KeyEvent_t *p_evt);
static uint8_t map_key_to_transition_event(uint8_t key_code, uint8_t key_event);
static void process_key_event(const KeyEvent_t *p_evt);
static void execute_action(uint8_t action, const KeyEvent_t *p_evt);

/* --- 状态进入/退出 --- */
static void enter_power_on(void);
static void enter_version(void);
static void enter_shutdown(void);
static void enter_demo(void);
static void enter_standby(void);
static void enter_func_select(void);
static void enter_cooking(void);
static void enter_pause(void);
static void enter_fault(void);
static void exit_func_select(void);
static void exit_cooking(void);

/* --- 状态切换 --- */
static void change_state(uint8_t new_state);

/* --- 主循环子步骤 --- */
static void check_state_timeout(void);
static void check_hint_timeout(void);
static void update_cooking_countdown(void);
static void check_fault_conditions(void);
static void check_end_phase_exit(void);

/* --- 显示 --- */
static void update_display(void);
static void convert_to_segcode(void);
static void update_led_state(void);
static void save_display_for_hint(void);
static void restore_display_after_hint(void);

/* --- 辅助 --- */
static uint16_t adjust_water(uint16_t current, int16_t step);
static uint16_t adjust_time(uint16_t current, int16_t step);
static void reset_cooking_params(void);
static void request_buzzer(uint8_t cmd);
static void notify_status_change(uint8_t type, void *p_value);

/* ========================================================================
 * 公有函数实现
 * ======================================================================== */

/**
 * @brief  逻辑层初始化
 */
void emc_logic_init(void) {
    uint8_t s, e;

    /* 清零控制结构体 */
    for (s = 0; s < sizeof(s_emc); s++) {
        ((uint8_t *)&s_emc)[s] = 0;
    }

    /* 清零按键缓冲 */
    s_key_buffer.head = 0;
    s_key_buffer.tail = 0;
    for (s = 0; s < KEY_BUFFER_SIZE; s++) {
        s_key_buffer.buffer[s].key_code = 0;
        s_key_buffer.buffer[s].event = KEY_EVENT_NULL;
        s_key_buffer.buffer[s].timestamp_ms = 0;
    }

    /* 初始化状态转移表（先全部设为默认：保持状态，无动作） */
    for (s = 0; s < STATE_MAX; s++) {
        for (e = 0; e < EVENT_MAX; e++) {
            s_state_trans_table[s][e].next_state = STATE_MAX;
            s_state_trans_table[s][e].action = ACTION_NONE;
        }
    }
    init_state_trans_table();

    /* 设置初始状态 */
    s_emc.current_state = S_POWER_ON;
    s_emc.previous_state = S_POWER_ON;
    s_emc.system_time_ms = 0;
    s_emc.flags.is_initialized = 1;

    enter_power_on();
}

/**
 * @brief  注册回调函数
 */
void emc_logic_register_callbacks(DispOutputCb_t p_disp_cb,
                                  BuzzerCb_t p_buzzer_cb,
                                  StatusChangeCb_t p_status_cb,
                                  GetHardwareInputsCb_t p_hw_inputs_cb) {
    s_emc.callbacks.disp_output   = (void (*)(SegCode_t *, LedState_t *))p_disp_cb;
    s_emc.callbacks.buzzer        = (void (*)(uint8_t))p_buzzer_cb;
    s_emc.callbacks.status_change = (void (*)(uint8_t, void *))p_status_cb;
    s_emc.callbacks.get_hw_inputs = (void (*)(HardwareInputs_t *))p_hw_inputs_cb;
}

/**
 * @brief  获取控制结构体只读指针
 */
const EmcCtrl_t *emc_logic_get_ctrl(void) {
    return &s_emc;
}

/**
 * @brief  按键输入回调 — 仅写入缓冲，不做状态判断
 */
void key_input_callback(uint8_t key_code, uint8_t event) {
    uint8_t next_tail;

    if (key_code >= KEY_MAX) {
        return;
    }

    next_tail = (s_key_buffer.tail + 1) % KEY_BUFFER_SIZE;

    s_key_buffer.buffer[s_key_buffer.tail].key_code     = key_code;
    s_key_buffer.buffer[s_key_buffer.tail].event        = event;
    s_key_buffer.buffer[s_key_buffer.tail].timestamp_ms = s_emc.system_time_ms;

    s_key_buffer.tail = next_tail;

    /* 若buffer满，移动head丢弃最旧数据 */
    if (s_key_buffer.tail == s_key_buffer.head) {
        s_key_buffer.head = (s_key_buffer.head + 1) % KEY_BUFFER_SIZE;
    }
}

/**
 * @brief  10ms周期主循环 — 硬件定时器每10ms调用一次
 */
void key_disp_cycle(void) {
    /* Step 0: 拉取硬件输入 */
    if (s_emc.callbacks.get_hw_inputs) {
        s_emc.callbacks.get_hw_inputs(&s_emc.hw);
    }

    /* 水温达标自动解锁首次上电锁定（SAF-03） */
    if (s_emc.flags.temp_lock_active
        && s_emc.hw.water_temp >= (int16_t)(TEMP_LOCK_THRESHOLD * 10)) {
        s_emc.flags.temp_lock_active = 0;
    }

    /* Step 1: 更新系统时间 */
    s_emc.system_time_ms += CYCLE_PERIOD_MS;

    /* Step 2: 处理按键缓冲 */
    s_any_key_processed = 0;
    while (s_key_buffer.head != s_key_buffer.tail) {
        KeyEvent_t *p_evt = &s_key_buffer.buffer[s_key_buffer.head];

        /* End阶段特殊处理：任意有效按键 → 标记退出End */
        if (s_emc.flags.is_end_phase) {
            uint8_t key_ok = is_key_valid(p_evt->key_code, p_evt->event);
            if (key_ok || (p_evt->key_code >= KEY_M1 && p_evt->key_code <= KEY_ADD_TIME
                           && (p_evt->event == KEY_EVENT_SHORT
                               || p_evt->event == KEY_EVENT_PRESS))) {
                s_any_key_processed = 1;
            }
            s_key_buffer.head = (s_key_buffer.head + 1) % KEY_BUFFER_SIZE;
            continue;
        }

        /* 2.1 掩码检查 */
        if (is_key_valid(p_evt->key_code, p_evt->event)) {
            /* 2.2 锁定检查 */
            if (check_lock_conditions(p_evt)) {
                /* 2.3 状态机处理 */
                process_key_event(p_evt);
                s_any_key_processed = 1;
            }
        }

        /* 2.4 移动读指针 */
        s_key_buffer.head = (s_key_buffer.head + 1) % KEY_BUFFER_SIZE;
    }

    /* End阶段特殊处理：任意按键或锅具移开 → 退出End */
    if (s_emc.flags.is_end_phase && s_any_key_processed) {
        reset_cooking_params();
        change_state(S_STANDBY);
        notify_status_change(STATUS_COOK_END, 0);
        return;   /* 本周期已完成状态切换，跳过后续 */
    }

    /* Step 3: 状态超时检测 */
    check_state_timeout();

    /* Step 4: 临时提示超时检测 */
    check_hint_timeout();

    /* Step 5: 烹饪倒计时更新 */
    if (s_emc.current_state == S_COOKING) {
        update_cooking_countdown();
    }

    /* Step 6: 故障检测（持续监测） */
    check_fault_conditions();

    /* End阶段: 锅具移开检查 */
    if (s_emc.flags.is_end_phase) {
        check_end_phase_exit();
    }

    /* Step 7: 显示更新 */
    update_display();
    convert_to_segcode();
    update_led_state();

    /* Step 8: 调用输出回调 */
    if (s_emc.callbacks.disp_output) {
        s_emc.callbacks.disp_output(&s_emc.seg, &s_emc.led);
    }
}

/* ========================================================================
 * 静态函数 — 状态转移表初始化
 * ======================================================================== */

static void init_state_trans_table(void) {
    /* ===== S_POWER_ON ===== */
    s_state_trans_table[S_POWER_ON][EV_TIME_1500MS]
        = (StateTransition_t){ .next_state = S_VERSION,  .action = ACTION_NONE };
    s_state_trans_table[S_POWER_ON][EV_KEY_POWER_SHORT]
        = (StateTransition_t){ .next_state = S_SHUTDOWN, .action = ACTION_NONE };

    /* ===== S_VERSION ===== */
    s_state_trans_table[S_VERSION][EV_TIME_1500MS]
        = (StateTransition_t){ .next_state = S_SHUTDOWN, .action = ACTION_NONE };
    s_state_trans_table[S_VERSION][EV_KEY_POWER_SHORT]
        = (StateTransition_t){ .next_state = S_SHUTDOWN, .action = ACTION_NONE };

    /* ===== S_SHUTDOWN ===== */
    s_state_trans_table[S_SHUTDOWN][EV_KEY_POWER_SHORT]
        = (StateTransition_t){ .next_state = S_STANDBY,  .action = ACTION_NONE };
    s_state_trans_table[S_SHUTDOWN][EV_DEMO_ENTER]
        = (StateTransition_t){ .next_state = S_DEMO,     .action = ACTION_ENTER_DEMO };

    /* ===== S_DEMO ===== */
    s_state_trans_table[S_DEMO][EV_KEY_POWER_SHORT]
        = (StateTransition_t){ .next_state = S_SHUTDOWN, .action = ACTION_NONE };
    s_state_trans_table[S_DEMO][EV_KEY_ANY]
        = (StateTransition_t){ .next_state = STATE_MAX,  .action = ACTION_NONE };

    /* ===== S_STANDBY ===== */
    s_state_trans_table[S_STANDBY][EV_KEY_POWER_LONG]
        = (StateTransition_t){ .next_state = S_SHUTDOWN, .action = ACTION_NONE };
    s_state_trans_table[S_STANDBY][EV_KEY_M1_SHORT]
        = (StateTransition_t){ .next_state = S_FUNC_SELECT, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_STANDBY][EV_KEY_M2_SHORT]
        = (StateTransition_t){ .next_state = S_FUNC_SELECT, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_STANDBY][EV_KEY_M3_SHORT]
        = (StateTransition_t){ .next_state = S_FUNC_SELECT, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_STANDBY][EV_KEY_M4_SHORT]
        = (StateTransition_t){ .next_state = S_FUNC_SELECT, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_STANDBY][EV_KEY_M5_SHORT]
        = (StateTransition_t){ .next_state = S_FUNC_SELECT, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_STANDBY][EV_KEY_M6_SHORT]
        = (StateTransition_t){ .next_state = S_FUNC_SELECT, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_STANDBY][EV_KEY_M7_SHORT]
        = (StateTransition_t){ .next_state = S_FUNC_SELECT, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_STANDBY][EV_KEY_M8_SHORT]
        = (StateTransition_t){ .next_state = S_FUNC_SELECT, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_STANDBY][EV_KEY_M9_SHORT]
        = (StateTransition_t){ .next_state = S_FUNC_SELECT, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_STANDBY][EV_KEY_M10_SHORT]
        = (StateTransition_t){ .next_state = S_FUNC_SELECT, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_STANDBY][EV_KEY_ADD_TIME_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ADJUST_TIME };

    /* ===== S_FUNC_SELECT ===== */
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_M1_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_M2_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_M3_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_M4_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_M5_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_M6_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_M7_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_M8_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_M9_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_M10_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ENTER_FUNC_SELECT };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_ADD_WATER_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ADJUST_WATER };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_ADD_TIME_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_ADJUST_TIME };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_START_PAUSE_SHORT]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_TRY_START_COOK };
    s_state_trans_table[S_FUNC_SELECT][EV_KEY_START_PAUSE_LONG]
        = (StateTransition_t){ .next_state = S_STANDBY, .action = ACTION_CANCEL_COOK };
    s_state_trans_table[S_FUNC_SELECT][EV_TIME_60000MS]
        = (StateTransition_t){ .next_state = S_STANDBY, .action = ACTION_NONE };

    /* ===== S_COOKING ===== */
    s_state_trans_table[S_COOKING][EV_KEY_START_PAUSE_SHORT]
        = (StateTransition_t){ .next_state = S_PAUSE,   .action = ACTION_NONE };
    s_state_trans_table[S_COOKING][EV_KEY_START_PAUSE_LONG]
        = (StateTransition_t){ .next_state = S_STANDBY, .action = ACTION_CANCEL_COOK };
    s_state_trans_table[S_COOKING][EV_WATER_ZERO]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_SWITCH_TO_HEATING };
    s_state_trans_table[S_COOKING][EV_TIME_ZERO]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_SHOW_END };

    /* ===== S_PAUSE ===== */
    s_state_trans_table[S_PAUSE][EV_KEY_START_PAUSE_SHORT]
        = (StateTransition_t){ .next_state = S_COOKING, .action = ACTION_RESUME_COOK };
    s_state_trans_table[S_PAUSE][EV_KEY_START_PAUSE_LONG]
        = (StateTransition_t){ .next_state = S_STANDBY, .action = ACTION_CANCEL_COOK };

    /* ===== S_FAULT ===== */
    s_state_trans_table[S_FAULT][EV_ANY]
        = (StateTransition_t){ .next_state = STATE_MAX, .action = ACTION_NONE };
}

/* ========================================================================
 * 静态函数 — 按键处理
 * ======================================================================== */

/**
 * @brief  掩码查表：检查按键在当前状态下是否有效
 * @return 1=有效, 0=无效（被锁定）
 */
static uint8_t is_key_valid(uint8_t key_code, uint8_t event) {
    uint16_t mask;

    if (key_code >= 16) {
        return 0;
    }

    if (event == KEY_EVENT_SHORT || event == KEY_EVENT_PRESS) {
        mask = s_state_key_masks[s_emc.current_state].short_mask;
    } else if (event == KEY_EVENT_LONG || event == KEY_EVENT_REPEAT) {
        mask = s_state_key_masks[s_emc.current_state].long_mask;
    } else {
        return 0;
    }

    return (mask >> key_code) & 0x01U;
}

/**
 * @brief  锁定条件检查（首次上电锁定 / 扫码锁定）
 * @return 1=通过（允许）, 0=锁定（拒绝）
 */
static uint8_t check_lock_conditions(const KeyEvent_t *p_evt) {
    uint8_t key_code = p_evt->key_code;

    /* 首次上电锁定：仅允许电源键/启动暂停键 */
    if (s_emc.flags.temp_lock_active) {
        if (key_code != KEY_POWER && key_code != KEY_START_PAUSE) {
            /* 显示锁定提示，3秒后恢复 */
            save_display_for_hint();
            s_emc.disp.digit[0] = 'L';
            s_emc.disp.digit[1] = 'O';
            s_emc.disp.digit[2] = 'C';
            s_emc.disp.digit[3] = 'K';
            s_emc.disp.dp_mask = 0;
            s_emc.disp.colon_mask = 0;
            s_emc.flags.hint_active = 1;
            s_emc.hint.hint_type = HINT_TEMP_LOCK;
            s_emc.hint.start_time_ms = s_emc.system_time_ms;
            return 0;
        }
    }

    /* 扫码锁定：仅允许电源键/启动暂停键 */
    if (s_emc.flags.qr_lock_active) {
        if (key_code != KEY_POWER && key_code != KEY_START_PAUSE) {
            save_display_for_hint();
            s_emc.disp.digit[0] = 'C';
            s_emc.disp.digit[1] = 'o';
            s_emc.disp.digit[2] = 'd';
            s_emc.disp.digit[3] = 'E';
            s_emc.disp.dp_mask = 0;
            s_emc.disp.colon_mask = 0;
            s_emc.flags.hint_active = 1;
            s_emc.hint.hint_type = HINT_QR_LOCK;
            s_emc.hint.start_time_ms = s_emc.system_time_ms;
            return 0;
        }
    }

    return 1;
}

/**
 * @brief  按键码+事件类型 → 转移事件枚举值
 */
static uint8_t map_key_to_transition_event(uint8_t key_code, uint8_t key_event) {
    if (key_code == KEY_POWER) {
        if (key_event == KEY_EVENT_SHORT || key_event == KEY_EVENT_PRESS) {
            return EV_KEY_POWER_SHORT;
        }
        if (key_event == KEY_EVENT_LONG || key_event == KEY_EVENT_REPEAT) {
            return EV_KEY_POWER_LONG;
        }
    }

    if (key_code == KEY_START_PAUSE) {
        if (key_event == KEY_EVENT_SHORT || key_event == KEY_EVENT_PRESS) {
            return EV_KEY_START_PAUSE_SHORT;
        }
        if (key_event == KEY_EVENT_LONG || key_event == KEY_EVENT_REPEAT) {
            return EV_KEY_START_PAUSE_LONG;
        }
    }

    /* M1-M10, ADD_WATER, ADD_TIME: 仅短按产生事件 */
    if (key_code >= KEY_M1 && key_code <= KEY_ADD_TIME) {
        if (key_event == KEY_EVENT_SHORT || key_event == KEY_EVENT_PRESS) {
            return (uint8_t)(EV_KEY_M1_SHORT + (key_code - 1));
        }
    }

    return EVENT_MAX;
}

/**
 * @brief  处理按键事件：查转移表 → 执行动作 → 状态转移
 */
static void process_key_event(const KeyEvent_t *p_evt) {
    uint8_t trans_event;
    StateTransition_t trans;

    /* End阶段特殊处理：任意按键退出 */
    if (s_emc.flags.is_end_phase) {
        return;   /* 在key_disp_cycle中统一处理 */
    }

    trans_event = map_key_to_transition_event(p_evt->key_code, p_evt->event);
    if (trans_event >= EVENT_MAX) {
        return;
    }

    trans = s_state_trans_table[s_emc.current_state][trans_event];

    /* 执行动作 */
    if (trans.action != ACTION_NONE) {
        execute_action(trans.action, p_evt);
    }

    /* 状态转移 */
    if (trans.next_state < STATE_MAX) {
        change_state(trans.next_state);
    }
}

/**
 * @brief  执行动作（状态转移表中action字段对应的处理）
 */
static void execute_action(uint8_t action, const KeyEvent_t *p_evt) {
    (void)p_evt;   /* 部分动作不使用按键参数 */

    switch (action) {
    case ACTION_ENTER_FUNC_SELECT:
        /* 先执行退出再进入（切换功能） */
        if (s_emc.current_state == S_FUNC_SELECT) {
            exit_func_select();
        }
        s_emc.params.selected_func_id = (int8_t)p_evt->key_code;
        enter_func_select();
        break;

    case ACTION_ADJUST_WATER:
        s_emc.params.current_water_ml =
            adjust_water(s_emc.params.current_water_ml, (int16_t)WATER_STEP_ML);
        /* 设置为显示水量模式 */
        s_emc.flags.show_time_in_func_select = 0;
        /* 刷新显示 */
        update_display();
        request_buzzer(BUZZ_CLICK);
        notify_status_change(STATUS_PARAM_CHANGE, &s_emc.params.current_water_ml);
        break;

    case ACTION_ADJUST_TIME:
        s_emc.params.current_time_s =
            adjust_time(s_emc.params.current_time_s, (int16_t)TIME_STEP_S);
        /* 设置为显示时间模式 */
        s_emc.flags.show_time_in_func_select = 1;
        update_display();
        request_buzzer(BUZZ_CLICK);
        notify_status_change(STATUS_PARAM_CHANGE, &s_emc.params.current_time_s);
        break;

    case ACTION_TRY_START_COOK:
        /* SAF-05: 水位检查 */
        if (!s_emc.hw.water_level_ok) {
            /* 显示 NWAT 3秒 */
            save_display_for_hint();
            s_emc.disp.digit[0] = 'N';
            s_emc.disp.digit[1] = 'W';
            s_emc.disp.digit[2] = 'A';
            s_emc.disp.digit[3] = 'T';
            s_emc.disp.dp_mask = 0;
            s_emc.disp.colon_mask = 0;
            s_emc.flags.hint_active = 1;
            s_emc.hint.hint_type = HINT_WATER_LOW;
            s_emc.hint.start_time_ms = s_emc.system_time_ms;
            s_emc.lock.water_low_wait_s = WATER_RESUME_WAIT_MIN_S;
            request_buzzer(BUZZ_DOUBLE);
            break;
        }
        /* 水位正常 → 进入烹饪 */
        change_state(S_COOKING);
        request_buzzer(BUZZ_CHORD);
        notify_status_change(STATUS_COOK_START, 0);
        break;

    case ACTION_CANCEL_COOK:
        /* SAF-02: 完全重置烹饪参数 */
        reset_cooking_params();
        s_emc.flags.is_end_phase = 0;
        break;

    case ACTION_SHOW_END:
        /* 烹饪完成 → 显示 End，不改变主状态 */
        s_emc.flags.is_end_phase = 1;
        s_emc.flags.is_water_phase = 0;
        request_buzzer(BUZZ_CHORD);
        break;

    case ACTION_RESUME_COOK:
        /* 从暂停恢复：参数已在冻结状态，不需要额外处理 */
        request_buzzer(BUZZ_CLICK);
        break;

    case ACTION_SWITCH_TO_HEATING:
        s_emc.flags.is_water_phase = 0;
        break;

    case ACTION_ENTER_DEMO:
        enter_demo();
        break;

    case ACTION_SHOW_LOCK_HINT:
        /* 锁定提示（已在 check_lock_conditions 中处理） */
        break;

    default:
        break;
    }
}

/* ========================================================================
 * 静态函数 — 状态进入
 * ======================================================================== */

static void enter_power_on(void) {
    uint8_t i;

    s_emc.current_state = S_POWER_ON;
    s_emc.state_enter_time_ms = s_emc.system_time_ms;

    /* 显示 "8888" 全亮 */
    for (i = 0; i < DIGIT_COUNT; i++) {
        s_emc.disp.digit[i] = '8';
    }
    s_emc.disp.dp_mask   = 0x0F;   /* 全部小数点 */
    s_emc.disp.colon_mask = 0x03;  /* 两时钟点都亮 */
    s_emc.disp.special_mode = 0;

    /* 指示灯光亮 */
    s_emc.led.power_led = 1;
    s_emc.led.func_leds = 0x3FF;   /* M1-M10全亮 */

    /* 检查首次上电锁定条件 */
    if (s_emc.hw.water_temp < (int16_t)(TEMP_LOCK_THRESHOLD * 10)) {
        s_emc.flags.temp_lock_active = 1;
    }
}

static void enter_version(void) {
    s_emc.current_state = S_VERSION;
    s_emc.state_enter_time_ms = s_emc.system_time_ms;

    /* 显示版本号 "V1.0" */
    s_emc.disp.digit[0] = 'V';
    s_emc.disp.digit[1] = '1';
    s_emc.disp.digit[2] = '.';
    s_emc.disp.digit[3] = '0';
    s_emc.disp.dp_mask   = 0;
    s_emc.disp.colon_mask = 0;
    s_emc.disp.special_mode = 0;

    s_emc.led.power_led = 1;
    s_emc.led.func_leds = 0x3FF;
}

static void enter_shutdown(void) {
    s_emc.current_state = S_SHUTDOWN;
    s_emc.state_enter_time_ms = s_emc.system_time_ms;

    /* 显示 "---" */
    s_emc.disp.digit[0] = '-';
    s_emc.disp.digit[1] = '-';
    s_emc.disp.digit[2] = '-';
    s_emc.disp.digit[3] = ' ';
    s_emc.disp.dp_mask   = 0;
    s_emc.disp.colon_mask = 0;
    s_emc.disp.special_mode = 2;   /* 闪烁(0.5s周期，外部控件层控制) */

    /* 电源灯闪烁(控件层控制)，功能灯全灭 */
    s_emc.led.power_led = 1;
    s_emc.led.func_leds = 0;
    s_emc.led.add_water_led  = 0;
    s_emc.led.add_time_led   = 0;
    s_emc.led.water_disp_led = 0;
    s_emc.led.time_disp_led  = 0;

    reset_cooking_params();
}

static void enter_demo(void) {
    s_emc.current_state = S_DEMO;
    s_emc.state_enter_time_ms = s_emc.system_time_ms;

    /* 显示 "----" 跑马灯（由下往上） */
    s_emc.disp.digit[0] = '-';
    s_emc.disp.digit[1] = '-';
    s_emc.disp.digit[2] = '-';
    s_emc.disp.digit[3] = '-';
    s_emc.disp.dp_mask   = 0;
    s_emc.disp.colon_mask = 0;
    s_emc.disp.special_mode = 1;   /* 跑马灯 */

    s_emc.led.power_led = 1;
    s_emc.led.func_leds = 0;
}

static void enter_standby(void) {
    s_emc.current_state = S_STANDBY;
    s_emc.state_enter_time_ms = s_emc.system_time_ms;

    /* 清除临时提示标志 */
    s_emc.flags.hint_active = 0;
    s_emc.hint.hint_type = HINT_NONE;

    /* 清除End标志 */
    s_emc.flags.is_end_phase = 0;

    update_display();   /* 显示水温 */
    update_led_state();
}

static void enter_func_select(void) {
    const FuncConfig_t *p_cfg;

    s_emc.current_state = S_FUNC_SELECT;
    s_emc.state_enter_time_ms = s_emc.system_time_ms;

    /* 加载功能默认参数 */
    if (s_emc.params.selected_func_id >= 1
        && s_emc.params.selected_func_id <= 10) {
        p_cfg = &s_func_configs[(uint8_t)s_emc.params.selected_func_id];
        s_emc.params.current_water_ml = p_cfg->default_water;
        s_emc.params.current_time_s   = p_cfg->default_time;
    }

    /* 初始化显示模式为水量 */
    s_emc.flags.show_time_in_func_select = 0;

    update_display();
    request_buzzer(BUZZ_CLICK);
    notify_status_change(STATUS_FUNC_SELECT, &s_emc.params.selected_func_id);
}

static void enter_cooking(void) {
    s_emc.current_state = S_COOKING;
    s_emc.state_enter_time_ms = s_emc.system_time_ms;

    /* 区分两种进入来源：
     * ① S_FUNC_SELECT → S_COOKING（首次启动）：初始化参数
     * ② S_PAUSE → S_COOKING（从暂停恢复）：保持冻结值（LOG-06） */
    if (s_emc.previous_state != S_PAUSE) {
        s_emc.params.remain_water_ml = s_emc.params.current_water_ml;
        s_emc.params.remain_time_s   = s_emc.params.current_time_s;
        s_emc.flags.is_water_phase   = 1;
        s_emc.flags.is_end_phase     = 0;
    }

    update_display();
}

static void enter_pause(void) {
    s_emc.current_state = S_PAUSE;
    s_emc.state_enter_time_ms = s_emc.system_time_ms;

    /* 显示 "PA" */
    s_emc.disp.digit[0] = 'P';
    s_emc.disp.digit[1] = 'A';
    s_emc.disp.digit[2] = ' ';
    s_emc.disp.digit[3] = ' ';
    s_emc.disp.dp_mask   = 0;
    s_emc.disp.colon_mask = 0;
    s_emc.disp.special_mode = 0;

    update_led_state();
}

static void enter_fault(void) {
    s_emc.current_state = S_FAULT;
    s_emc.state_enter_time_ms = s_emc.system_time_ms;

    /* 显示 "Exx" */
    s_emc.disp.digit[0] = 'E';
    s_emc.disp.digit[1] = (s_emc.hw.fault_code / 10) + '0';
    s_emc.disp.digit[2] = (s_emc.hw.fault_code % 10) + '0';
    s_emc.disp.digit[3] = ' ';
    s_emc.disp.dp_mask   = 0;
    s_emc.disp.colon_mask = 0;
    s_emc.disp.special_mode = 0;

    s_emc.led.power_led = 0;
    s_emc.led.func_leds = 0;

    notify_status_change(STATUS_FAULT, &s_emc.hw.fault_code);
}

/* ========================================================================
 * 静态函数 — 状态退出
 * ======================================================================== */

static void exit_func_select(void) {
    /* 功能选择退出时不做特殊清理，参数保留用于可能的重新进入 */
}

static void exit_cooking(void) {
    /* 烹饪退出：如果不是暂停（即取消或完成），清理End标志 */
}

/* ========================================================================
 * 静态函数 — 状态切换
 * ======================================================================== */

static void change_state(uint8_t new_state) {
    if (new_state >= STATE_MAX) {
        return;
    }

    /* 记录上一状态 */
    s_emc.previous_state = s_emc.current_state;

    /* 根据状态执行进入动作 */
    switch (new_state) {
    case S_POWER_ON:    enter_power_on();    break;
    case S_VERSION:     enter_version();     break;
    case S_SHUTDOWN:    enter_shutdown();    break;
    case S_DEMO:        enter_demo();        break;
    case S_STANDBY:     enter_standby();     break;
    case S_FUNC_SELECT: enter_func_select(); break;
    case S_COOKING:     enter_cooking();     break;
    case S_PAUSE:       enter_pause();       break;
    case S_FAULT:       enter_fault();       break;
    default:            break;
    }

    /* 通知状态变化 */
    if (s_emc.callbacks.status_change) {
        s_emc.callbacks.status_change(STATUS_STATE_CHANGE, &s_emc.current_state);
    }
}

/* ========================================================================
 * 静态函数 — 主循环子步骤
 * ======================================================================== */

/**
 * @brief  检查状态超时
 */
static void check_state_timeout(void) {
    uint32_t elapsed = s_emc.system_time_ms - s_emc.state_enter_time_ms;

    switch (s_emc.current_state) {
    case S_POWER_ON:
        if (elapsed >= POWER_ON_SELFTEST_MS) {
            change_state(S_VERSION);
        }
        break;

    case S_VERSION:
        if (elapsed >= VERSION_DISPLAY_MS) {
            change_state(S_SHUTDOWN);
        }
        break;

    case S_FUNC_SELECT:
        /* LOG-05: 60秒超时必须返回待机 */
        if (elapsed >= FUNC_SELECT_TIMEOUT_MS) {
            reset_cooking_params();
            change_state(S_STANDBY);
        }
        break;

    default:
        break;
    }
}

/**
 * @brief  检查临时提示超时（3秒后恢复原显示）
 */
static void check_hint_timeout(void) {
    if (!s_emc.flags.hint_active) {
        return;
    }

    if (s_emc.system_time_ms - s_emc.hint.start_time_ms >= TEMP_HINT_MS) {
        restore_display_after_hint();
        s_emc.flags.hint_active = 0;
        s_emc.hint.hint_type = HINT_NONE;
    }
}

/**
 * @brief  更新烹饪倒计时
 */
static void update_cooking_countdown(void) {
    /* End阶段不更新倒计时 */
    if (s_emc.flags.is_end_phase) {
        return;
    }

    /* 暂停冻结倒计时 */
    if (s_emc.current_state != S_COOKING) {
        return;
    }

    /* 每10ms递减（100个周期=1秒） */
    /* 实际每1秒递减，通过累加10ms计数判断 */
    {
        static uint16_t s_ms_accumulator = 0;
        s_ms_accumulator += CYCLE_PERIOD_MS;

        if (s_ms_accumulator < 1000) {
            return;
        }
        s_ms_accumulator -= 1000;

        if (s_emc.flags.is_water_phase) {
            /* 出水阶段：递减水量 */
            if (s_emc.params.remain_water_ml > 0) {
                s_emc.params.remain_water_ml--;
            }
            if (s_emc.params.remain_water_ml == 0) {
                /* 切换至加热阶段 */
                s_emc.flags.is_water_phase = 0;
            }
        }

        /* 加热阶段：递减时间 */
        if (!s_emc.flags.is_water_phase && !s_emc.flags.is_end_phase) {
            if (s_emc.params.remain_time_s > 0) {
                s_emc.params.remain_time_s--;
            }
            if (s_emc.params.remain_time_s == 0) {
                /* 时间归零 → 显示End */
                execute_action(ACTION_SHOW_END, 0);
            }
        }
    }
}

/**
 * @brief  持续故障检测
 */
static void check_fault_conditions(void) {
    /* SAF-01: 故障状态下按键已通过掩码表锁定 */
    if (s_emc.current_state == S_FAULT) {
        return;
    }

    /* 检测到故障码 → 进入故障状态 */
    if (s_emc.hw.fault_code != 0) {
        change_state(S_FAULT);
    }
}

/**
 * @brief  End阶段：检测锅具移开
 */
static void check_end_phase_exit(void) {
    if (s_emc.flags.is_end_phase && !s_emc.hw.pot_present) {
        reset_cooking_params();
        change_state(S_STANDBY);
        notify_status_change(STATUS_COOK_END, 0);
    }
}

/* ========================================================================
 * 静态函数 — 显示
 * ======================================================================== */

static void update_display(void) {
    uint8_t i;
    int16_t temp;
    uint8_t temp_val;
    uint16_t val;
    uint16_t sec;
    uint8_t min;
    uint8_t ss;

    /* 清零 */
    for (i = 0; i < DIGIT_COUNT; i++) {
        s_emc.disp.digit[i] = ' ';
    }
    s_emc.disp.dp_mask   = 0;
    s_emc.disp.colon_mask = 0;
    s_emc.disp.special_mode = 0;

    /* 临时提示优先（LOCK / Code / NWAT） */
    if (s_emc.flags.hint_active) {
        return;   /* 提示内容已在 check_lock_conditions / ACTION_TRY_START_COOK 中设置 */
    }

    switch (s_emc.current_state) {
    case S_POWER_ON:
        for (i = 0; i < DIGIT_COUNT; i++) {
            s_emc.disp.digit[i] = '8';
        }
        s_emc.disp.colon_mask = 0x03;
        break;

    case S_VERSION:
        s_emc.disp.digit[0] = 'V';
        s_emc.disp.digit[1] = '1';
        s_emc.disp.digit[2] = '.';
        s_emc.disp.digit[3] = '0';
        break;

    case S_SHUTDOWN:
        s_emc.disp.digit[0] = '-';
        s_emc.disp.digit[1] = '-';
        s_emc.disp.digit[2] = '-';
        s_emc.disp.special_mode = 2;
        break;

    case S_DEMO:
        for (i = 0; i < DIGIT_COUNT; i++) {
            s_emc.disp.digit[i] = '-';
        }
        s_emc.disp.special_mode = 1;
        break;

    case S_STANDBY:
        /* 显示水温 "XX°" */
        temp = s_emc.hw.water_temp;
        temp_val = (uint8_t)(temp / 10);
        s_emc.disp.digit[0] = (temp_val / 10) + '0';
        s_emc.disp.digit[1] = (temp_val % 10) + '0';
        s_emc.disp.digit[2] = ' ';
        s_emc.disp.digit[3] = ' ';
        s_emc.disp.dp_mask = 0x04;   /* DIG2 表示° */
        break;

    case S_FUNC_SELECT:
        /* 根据标志决定显示水量还是时间 */
        if (s_emc.flags.show_time_in_func_select) {
            /* 显示加热时间 X:XX */
            val = s_emc.params.current_time_s;
            min = (uint8_t)(val / 60);
            ss  = (uint8_t)(val % 60);
            s_emc.disp.digit[0] = min + '0';
            s_emc.disp.digit[1] = ':';
            s_emc.disp.digit[2] = (ss / 10) + '0';
            s_emc.disp.digit[3] = (ss % 10) + '0';
            s_emc.disp.colon_mask = 0x02;  /* 点亮冒号 */
        } else {
            /* 默认显示出水量 XXX */
            val = s_emc.params.current_water_ml;
            s_emc.disp.digit[0] = (val / 100) + '0';
            s_emc.disp.digit[1] = ((val % 100) / 10) + '0';
            s_emc.disp.digit[2] = (val % 10) + '0';
            s_emc.disp.colon_mask = 0;
        }
        break;

    case S_COOKING:
        if (s_emc.flags.is_end_phase) {
            /* 显示 "End" */
            s_emc.disp.digit[0] = 'E';
            s_emc.disp.digit[1] = 'n';
            s_emc.disp.digit[2] = 'd';
        } else if (s_emc.flags.is_water_phase) {
            /* 出水阶段：显示剩余水量 XXX */
            val = s_emc.params.remain_water_ml;
            s_emc.disp.digit[0] = (val / 100) + '0';
            s_emc.disp.digit[1] = ((val % 100) / 10) + '0';
            s_emc.disp.digit[2] = (val % 10) + '0';
        } else {
            /* 加热阶段：显示剩余时间 X:XX */
            sec = s_emc.params.remain_time_s;
            min = (uint8_t)(sec / 60);
            ss  = (uint8_t)(sec % 60);
            s_emc.disp.digit[0] = min + '0';
            s_emc.disp.digit[1] = ':';
            s_emc.disp.digit[2] = (ss / 10) + '0';
            s_emc.disp.digit[3] = (ss % 10) + '0';
            s_emc.disp.colon_mask = 0x02;
        }
        break;

    case S_PAUSE:
        s_emc.disp.digit[0] = 'P';
        s_emc.disp.digit[1] = 'A';
        break;

    case S_FAULT:
        s_emc.disp.digit[0] = 'E';
        s_emc.disp.digit[1] = (s_emc.hw.fault_code / 10) + '0';
        s_emc.disp.digit[2] = (s_emc.hw.fault_code % 10) + '0';
        break;

    default:
        break;
    }
}

/**
 * @brief  ASCII → 段码转换
 */
static void convert_to_segcode(void) {
    uint8_t i;
    uint8_t ascii;

    for (i = 0; i < DIGIT_COUNT; i++) {
        ascii = s_emc.disp.digit[i];
        s_emc.seg.seg[i] = s_ascii_to_segcode[ascii];
    }
    s_emc.seg.dp_mask    = s_emc.disp.dp_mask;
    s_emc.seg.colon_mask = s_emc.disp.colon_mask;
}

/**
 * @brief  更新LED状态（按规格书3.3节指示灯行为映射）
 */
static void update_led_state(void) {
    uint8_t func_id;

    /* 清零所有辅助LED */
    s_emc.led.add_water_led  = 0;
    s_emc.led.add_time_led   = 0;
    s_emc.led.water_disp_led = 0;
    s_emc.led.time_disp_led  = 0;

    switch (s_emc.current_state) {
    case S_SHUTDOWN:
        s_emc.led.power_led = 1;    /* 闪烁由控件层控制 */
        s_emc.led.func_leds = 0;
        break;

    case S_DEMO:
        s_emc.led.power_led = 1;
        s_emc.led.func_leds = 0;
        break;

    case S_STANDBY:
        s_emc.led.power_led = 1;
        s_emc.led.func_leds = 0x3FF;   /* 全亮 */
        break;

    case S_FUNC_SELECT:
        s_emc.led.power_led = 1;
        func_id = (uint8_t)s_emc.params.selected_func_id;
        if (func_id >= 1 && func_id <= 10) {
            s_emc.led.func_leds = (uint16_t)(1U << (func_id - 1));
        }
        /* 可调参数指示灯常亮 */
        s_emc.led.add_water_led = 1;
        s_emc.led.add_time_led  = 1;
        break;

    case S_COOKING:
    case S_PAUSE:
        s_emc.led.power_led = 1;
        func_id = (uint8_t)s_emc.params.selected_func_id;
        if (func_id >= 1 && func_id <= 10) {
            s_emc.led.func_leds = (uint16_t)(1U << (func_id - 1));
        }
        break;

    case S_FAULT:
        s_emc.led.power_led = 0;
        s_emc.led.func_leds = 0;
        break;

    default:
        break;
    }
}

/**
 * @brief  保存当前显示内容用于临时提示恢复
 */
static void save_display_for_hint(void) {
    uint8_t i;
    for (i = 0; i < DIGIT_COUNT; i++) {
        s_emc.hint.restore_digit[i] = s_emc.disp.digit[i];
    }
    s_emc.hint.restore_dp    = s_emc.disp.dp_mask;
    s_emc.hint.restore_colon = s_emc.disp.colon_mask;
}

/**
 * @brief  临时提示结束后恢复原显示
 */
static void restore_display_after_hint(void) {
    uint8_t i;
    for (i = 0; i < DIGIT_COUNT; i++) {
        s_emc.disp.digit[i] = s_emc.hint.restore_digit[i];
    }
    s_emc.disp.dp_mask    = s_emc.hint.restore_dp;
    s_emc.disp.colon_mask = s_emc.hint.restore_colon;
}

/* ========================================================================
 * 静态函数 — 辅助
 * ======================================================================== */

/**
 * @brief  水量环形调整（LOG-09）
 */
static uint16_t adjust_water(uint16_t current, int16_t step) {
    int16_t new_val = (int16_t)current + step;
    if (new_val > (int16_t)MAX_WATER_ML) {
        return MIN_WATER_ML;
    }
    if (new_val < (int16_t)MIN_WATER_ML) {
        return MAX_WATER_ML;
    }
    return (uint16_t)new_val;
}

/**
 * @brief  时间环形调整（LOG-09）
 */
static uint16_t adjust_time(uint16_t current, int16_t step) {
    int16_t new_val = (int16_t)current + step;
    if (new_val > (int16_t)MAX_TIME_S) {
        return MIN_TIME_S;
    }
    if (new_val < (int16_t)MIN_TIME_S) {
        return MAX_TIME_S;
    }
    return (uint16_t)new_val;
}

/**
 * @brief  重置烹饪参数（SAF-02）
 */
static void reset_cooking_params(void) {
    s_emc.params.selected_func_id = -1;
    s_emc.params.remain_water_ml  = 0;
    s_emc.params.remain_time_s    = 0;
    s_emc.flags.is_water_phase    = 1;
    s_emc.flags.is_end_phase      = 0;
}

/**
 * @brief  请求蜂鸣器（NULL安全）
 */
static void request_buzzer(uint8_t cmd) {
    if (s_emc.callbacks.buzzer) {
        s_emc.callbacks.buzzer(cmd);
    }
}

/**
 * @brief  通知状态变化（NULL安全）
 */
static void notify_status_change(uint8_t type, void *p_value) {
    if (s_emc.callbacks.status_change) {
        s_emc.callbacks.status_change(type, p_value);
    }
}
