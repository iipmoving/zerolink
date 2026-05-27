/**
 * hmi_logic_data.c — HMI 逻辑常量数据（自动生成）
 * 源文件: four_head_v4.json
 * 生成时间: 2026-05-24 10:10
 *
 * 直接加入KEIL工程编译, 零运行时JSON解析开销。
 */
#include "hmi_logic_defs.h"

/* ============================================================
 * 1. 超时值表
 * ============================================================ */
const uint16_t timeout_values[TMO_COUNT] = {
    15000, 15000, 3000, 15000, 30000, 30000, 15, 3000, 3000, 1500
};

/* ============================================================
 * 2. 全局路由表 [GLOBAL_MODE_COUNT][KEY_COUNT * 2]
 *    槽值 = ROUTE_ENCODE(action, param), -1 = 无路由
 * ============================================================ */
const int16_t global_route_table[GLOBAL_MODE_COUNT][KEY_COUNT * EVT_COUNT] = {
    [0] = { -1 },
    [1] = { -1 },
    [2] = {
    [ROUTE_SLOT(4, 1)] = 256
    },
    [3] = {
    [ROUTE_SLOT(2, 0)] = 1024,
    [ROUTE_SLOT(3, 1)] = 1280,
    [ROUTE_SLOT(4, 1)] = 512,
    [ROUTE_SLOT(5, 0)] = 2560,
    [ROUTE_SLOT(6, 0)] = 2561,
    [ROUTE_SLOT(7, 0)] = 2562,
    [ROUTE_SLOT(8, 0)] = 2563
    },
    [4] = {
    [ROUTE_SLOT(2, 0)] = 1024,
    [ROUTE_SLOT(4, 1)] = 512
    },
    [5] = {
    [ROUTE_SLOT(4, 1)] = 256
    },
};

/* ============================================================
 * 3. Zone 路由表 [ZONE_COUNT][KEY_COUNT * 2]
 * ============================================================ */
const int16_t zone_route_table[ZONE_COUNT][KEY_COUNT * EVT_COUNT] = {
    [0] = {
    [ROUTE_SLOT(5, 0)] = 2560,
    [ROUTE_SLOT(6, 0)] = 2561,
    [ROUTE_SLOT(7, 0)] = 2562,
    [ROUTE_SLOT(8, 0)] = 2563
    },
    [1] = {
    [ROUTE_SLOT(1, 0)] = 7680,
    [ROUTE_SLOT(12, 0)] = 3328,
    [ROUTE_SLOT(13, 0)] = 3329,
    [ROUTE_SLOT(14, 0)] = 3330,
    [ROUTE_SLOT(15, 0)] = 3331,
    [ROUTE_SLOT(16, 0)] = 3332,
    [ROUTE_SLOT(17, 0)] = 3333,
    [ROUTE_SLOT(18, 0)] = 3334,
    [ROUTE_SLOT(19, 0)] = 3335,
    [ROUTE_SLOT(20, 0)] = 3336,
    [ROUTE_SLOT(21, 0)] = 3337,
    [ROUTE_SLOT(21, 1)] = 5120
    },
    [2] = {
    [ROUTE_SLOT(1, 0)] = 7680,
    [ROUTE_SLOT(5, 0)] = 2560,
    [ROUTE_SLOT(6, 0)] = 2561,
    [ROUTE_SLOT(7, 0)] = 2562,
    [ROUTE_SLOT(8, 0)] = 2563,
    [ROUTE_SLOT(12, 0)] = 3328,
    [ROUTE_SLOT(13, 0)] = 3329,
    [ROUTE_SLOT(14, 0)] = 3330,
    [ROUTE_SLOT(15, 0)] = 3331,
    [ROUTE_SLOT(16, 0)] = 3332,
    [ROUTE_SLOT(17, 0)] = 3333,
    [ROUTE_SLOT(18, 0)] = 3334,
    [ROUTE_SLOT(19, 0)] = 3335,
    [ROUTE_SLOT(20, 0)] = 3336,
    [ROUTE_SLOT(21, 0)] = 3337,
    [ROUTE_SLOT(21, 1)] = 5120
    },
};

/* ============================================================
 * 4. 进程路由表 [PROCESS_COUNT][KEY_COUNT * 2]
 * ============================================================ */
const int16_t process_route_table[PROCESS_COUNT][KEY_COUNT * EVT_COUNT] = {
    [0] = {
    [ROUTE_SLOT(1, 0)] = 7936,
    [ROUTE_SLOT(1, 1)] = 8192,
    [ROUTE_SLOT(11, 0)] = 8705
    },
    [1] = {
    [ROUTE_SLOT(1, 1)] = 8448
    },
    [2] = {
    [ROUTE_SLOT(1, 0)] = 7680,
    [ROUTE_SLOT(12, 0)] = 5632,
    [ROUTE_SLOT(13, 0)] = 5633,
    [ROUTE_SLOT(14, 0)] = 5634,
    [ROUTE_SLOT(15, 0)] = 5635,
    [ROUTE_SLOT(16, 0)] = 5636,
    [ROUTE_SLOT(17, 0)] = 5637,
    [ROUTE_SLOT(18, 0)] = 5638,
    [ROUTE_SLOT(19, 0)] = 5639,
    [ROUTE_SLOT(20, 0)] = 5640,
    [ROUTE_SLOT(21, 0)] = 5641
    },
};

/* ============================================================
 * 5. 上电序列
 * ============================================================ */
const power_on_step_t power_on_sequence[3] = {
    { 3000, "88888888", 65535, -1 },
    { 3000, "V2.3P2.5", 0, -1 },
    { 0, """", 0, 2 },
};
const uint8_t power_on_sequence_count = 3;

/* ============================================================
 * 6. 童锁白名单
 * ============================================================ */
const uint8_t child_lock_whitelist[1] = { 4 };
const uint8_t child_lock_whitelist_count = 1;

/* ============================================================
 * 7. HEAD_SELF / HEAD_OTHER 特殊路由
 *    [zone, is_self, evt, action]
 * ============================================================ */
const uint8_t self_other_route_data[2][4] = {
    { 1, 1, 0, 12 },
    { 1, 0, 0, 10 },
};
const uint8_t self_other_route_count = 2;

/* ============================================================
 * 8. 全局模式进入动作
 *    [mode, act0..act7] (act=0 = end)
 * ============================================================ */
const uint8_t enter_action_data[4][9] = {
    { 2, 44, 40, 50, 54, 56, 58, 47, 0 },
    { 5, 44, 43, 60, 0, 0, 0, 0, 0 },
    { 3, 44, 45, 42, 50, 54, 56, 58, 46 },
    { 4, 41, 55, 54, 0, 0, 0, 0, 0 },
};
const uint8_t enter_action_count = 4;

/* ============================================================
 * 9. 全局模式退出动作
 * ============================================================ */
const uint8_t exit_action_data[1][9] = {
    { 4, 42, 56, 46, 0, 0, 0, 0, 0 },
};
const uint8_t exit_action_count = 1;

/* ============================================================
 * 10. 守卫 (全idle→关机 等)
 *     [mode, ms, action]
 * ============================================================ */
const int16_t guarded_actions[3][3] = {
    { 2, 30000, 3 },
    { 3, 15000, 0 },
    { 3, 30000, 2 },
};
const uint8_t guarded_action_count = 3;

const int16_t state_timeouts[0][3] = {
};
const uint8_t state_timeout_count = 0;

const int16_t zone_timeouts[1][3] = {
    { 1, 15000, 11 },
};
const uint8_t zone_timeout_count = 1;

const int16_t process_timeouts[2][3] = {
    { 0, 15000, 31 },
    { 2, 3000, 21 },
};
const uint8_t process_timeout_count = 2;

/* ============================================================
 * 11. 显示规则 & 元素属性
 * ============================================================ */
const uint8_t priority_chain[3] = { 0, 1, 2 };
const uint8_t blink_phase_ms = 300;
const uint8_t blink_exclude[] = { 0, 1 };
const uint8_t segment_digit_count = 2;
const uint8_t segment_boost_char = 80;
const uint8_t boost_power_level = 9;
const uint8_t timer_adjust_min = 1;
const uint8_t timer_adjust_max = 99;
const uint8_t timer_adjust_step = 1;
const uint8_t stack_max_depth = 4;
const uint8_t alternate_enabled = 1;
const uint8_t alternate_phases[2][2] = {
    { 0, 5000 },
    { 1, 5000 },
};
