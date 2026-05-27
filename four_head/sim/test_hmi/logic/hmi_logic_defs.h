/**
 * hmi_logic_defs.h — HMI 逻辑数据定义（自动生成）
 * 源文件: four_head_v4.json
 * 生成时间: 2026-05-24 10:10
 *
 * 使用方式:
 *   1. #include "hmi_logic_defs.h"
 *   2. #include "hmi_logic_data.c"  (或在工程中加入该.c文件)
 *   3. 用枚举值查表
 */
#ifndef __HMI_LOGIC_DEFS_H__
#define __HMI_LOGIC_DEFS_H__

#include <stdint.h>

/* ============================================================
 * 按键编码
 * ============================================================ */
typedef enum {
        KEY_TIMER = 1,
    KEY_PAUSE = 2,
    KEY_CHILD_LOCK = 3,
    KEY_POWER = 4,
    KEY_HEAD_1 = 5,
    KEY_HEAD_2 = 6,
    KEY_HEAD_3 = 7,
    KEY_HEAD_4 = 8,
    KEY_ZONE = 9,
    KEY_MINUS = 10,
    KEY_PLUS = 11,
    KEY_0 = 12,
    KEY_1 = 13,
    KEY_2 = 14,
    KEY_3 = 15,
    KEY_4 = 16,
    KEY_5 = 17,
    KEY_6 = 18,
    KEY_7 = 19,
    KEY_8 = 20,
    KEY_9 = 21,
} key_code_t;

/* ============================================================
 * 按键事件类型
 * ============================================================ */
typedef enum {
    EVT_TAP     = 0,
    EVT_LONG    = 1,
    EVT_RELEASE = 2
} event_type_t;

/* ============================================================
 * 全局模式
 * ============================================================ */
typedef enum {
        GLOBAL_POWERING_UP = 0,
    GLOBAL_VERSION_SHOW = 1,
    GLOBAL_POWERED_OFF = 2,
    GLOBAL_WORKING = 3,
    GLOBAL_PAUSED = 4,
    GLOBAL_DEEP_SLEEP = 5,
} global_mode_t;

/* ============================================================
 * Zone 节点状态
 * ============================================================ */
typedef enum {
    ZONE_IDLE       = 0,
    ZONE_SELECTING  = 1,
    ZONE_COOKING    = 2
} zone_node_t;

/* ============================================================
 * 正交进程类型
 * ============================================================ */
typedef enum {
    PROC_TIMER_SETTING  = 0,
    PROC_TIMER_ACTIVE   = 1,
    PROC_BOOST_ACTIVE   = 2
} process_type_t;

/* ============================================================
 * 动作枚举（路由目标）
 * ============================================================ */
typedef enum {
    ACT_NONE                = 0,
        ACT_GO_WORKING = 1,
    ACT_GO_POWERED_OFF = 2,
    ACT_GO_DEEP_SLEEP = 3,
    ACT_TOGGLE_PAUSE = 4,
    ACT_TOGGLE_CHILD_LOCK = 5,
    ACT_SELECT_HEAD = 10,
    ACT_CONFIRM_SELECT = 11,
    ACT_CONFIRM_SELECT_IMMEDIATE = 12,
    ACT_SET_POWER = 13,
    ACT_ENTER_BOOST = 20,
    ACT_EXIT_BOOST = 21,
    ACT_EXIT_BOOST_SET_POWER = 22,
    ACT_ENTER_TIMER_SETTING = 30,
    ACT_CONFIRM_TIMER = 31,
    ACT_CANCEL_TIMER_SETTING = 32,
    ACT_CANCEL_TIMER_ACTIVE = 33,
    ACT_TIMER_ADJUST = 34,
    ACT_SHOW_DASH = 40,
    ACT_SHOW_PA = 41,
    ACT_SHOW_POWER_MODE = 42,
    ACT_DISPLAY_ALL_OFF = 43,
    ACT_CLEAR_ALL_HEADS = 44,
    ACT_HOTHEAD_CLEAR = 45,
    ACT_RESET_IDLE_TIMER = 46,
    ACT_RESET_OFF_TIMER = 47,
    ACT_LED_POWER_ON = 50,
    ACT_LED_POWER_OFF = 51,
    ACT_LED_POWER_BLINK = 52,
    ACT_LED_TIMER_ON = 53,
    ACT_LED_TIMER_OFF = 54,
    ACT_LED_PAUSE_ON = 55,
    ACT_LED_PAUSE_OFF = 56,
    ACT_LED_CHILD_LOCK_ON = 57,
    ACT_LED_CHILD_LOCK_OFF = 58,
    ACT_LED_ALL_ON = 59,
    ACT_LED_ALL_OFF = 60,
} action_t;

/* ============================================================
 * 超时索引
 * ============================================================ */
typedef enum {
    TMO_SELECT_CONFIRM_MS = 0,
    TMO_TIMER_CONFIRM_MS = 1,
    TMO_BOOST_MAX_MS = 2,
    TMO_IDLE_TO_STANDBY_MS = 3,
    TMO_IDLE_TO_OFF_MS = 4,
    TMO_OFF_TO_SLEEP_MS = 5,
    TMO_DEFAULT_TIMER_MIN_MS = 6,
    TMO_POWER_ON_ALL_ON_MS = 7,
    TMO_VERSION_SHOW_MS = 8,
    TMO_POWER_KEY_LONG_MS = 9,
    TMO_COUNT = 10
} timeout_idx_t;

/* ============================================================
 * 路由表编码宏
 * ============================================================ */
#define KEY_COUNT           22
#define EVT_COUNT           2
#define GLOBAL_MODE_COUNT   6
#define ZONE_COUNT          3
#define PROCESS_COUNT       3

#define ROUTE_SLOT(key, evt)    ((key) * EVT_COUNT + (evt))
#define ROUTE_ENCODE(act, param) (((int16_t)(act) << 8) | (uint8_t)(param))
#define ROUTE_ACTION(val)       ((int16_t)((val) >> 8))
#define ROUTE_PARAM(val)        ((uint8_t)(val))

#define ROUTE_NONE              (-1)

/* ============================================================
 * 上电序列步进结构
 * ============================================================ */
typedef struct {
    uint16_t delay_ms;
    const char seg_chars[9];
    uint16_t leds_mask;
    int8_t   goto_mode;      /* -1 = 不跳转 */
} power_on_step_t;

/* ============================================================
 * 外部数据声明（定义在 hmi_logic_data.c）
 * ============================================================ */
extern const uint16_t timeout_values[TMO_COUNT];
extern const int16_t  global_route_table[GLOBAL_MODE_COUNT][KEY_COUNT * EVT_COUNT];
extern const int16_t  zone_route_table[ZONE_COUNT][KEY_COUNT * EVT_COUNT];
extern const int16_t  process_route_table[PROCESS_COUNT][KEY_COUNT * EVT_COUNT];
extern const power_on_step_t power_on_sequence[3];
extern const uint8_t  power_on_sequence_count;

extern const uint8_t  child_lock_whitelist[1];
extern const uint8_t  child_lock_whitelist_count;

extern const uint8_t  self_other_route_count;
extern const uint8_t  self_other_route_data[][4];  /* zone, is_self, evt, action */

extern const uint8_t  enter_action_count;
extern const uint8_t  enter_action_data[][9];      /* mode, act0..act7 */

extern const uint8_t  exit_action_count;
extern const uint8_t  exit_action_data[][9];

extern const int16_t  guarded_actions[][3];        /* mode, ms, action */
extern const uint8_t  guarded_action_count;

extern const int16_t  state_timeouts[][3];          /* mode, ms, action */
extern const uint8_t  state_timeout_count;

extern const int16_t  zone_timeouts[][3];
extern const uint8_t  zone_timeout_count;

extern const int16_t  process_timeouts[][3];
extern const uint8_t  process_timeout_count;

extern const uint8_t  priority_chain[3];
extern const uint8_t  blink_phase_ms;
extern const uint8_t  blink_exclude[];

extern const uint8_t  segment_digit_count;
extern const uint8_t  segment_boost_char;
extern const uint8_t  boost_power_level;
extern const uint8_t  timer_adjust_min;
extern const uint8_t  timer_adjust_max;
extern const uint8_t  timer_adjust_step;
extern const uint8_t  stack_max_depth;

extern const uint8_t  alternate_enabled;
extern const uint8_t  alternate_phases[2][2];       /* [phase][source/duration_ms] */

#endif /* __HMI_LOGIC_DEFS_H__ */
