/* hmi_data.c — 由 gen_hmi.js 自动生成, 勿手动编辑 */
/* 数据来源: sim/test_hmi/logic/four_head_v4.json   */

#include "app/app_hmi.h"
#include "drv/drv_key.h"

/* ===== 超时配置 (按 HmiTimeoutId_t 索引) ===== */
const uint32_t hmi_timeouts[HMI_TO_COUNT] = {
    [HMI_TO_SELECT_CONFIRM_MS] = 15000,
    [HMI_TO_TIMER_CONFIRM_MS] = 15000,
    [HMI_TO_BOOST_MAX_MS] = 3000,
    [HMI_TO_IDLE_TO_STANDBY_MS] = 15000,
    [HMI_TO_IDLE_TO_OFF_MS] = 30000,
    [HMI_TO_OFF_TO_SLEEP_MS] = 30000,
    [HMI_TO_DEFAULT_TIMER_MIN] = 15,
    [HMI_TO_POWER_ON_ALL_ON_MS] = 3000,
    [HMI_TO_VERSION_SHOW_MS] = 3000,
    [HMI_TO_POWER_KEY_LONG_MS] = 1500,
};

/* ===== 上电序列 ===== */
const HmiPowerOnStep_t hmi_power_on_seq[] = {
    { 3000, HMI_SEG_MODE_ASCII, "88888888", HMI_LEDS_ALL_ON, -1 },
    { 3000, HMI_SEG_MODE_ASCII, "V2.3P2.5", HMI_LEDS_ALL_OFF, -1 },
    { 0, 0, "", 0, HMI_NODE_POWERED_OFF },  /* step=2 goto=powered_off */
};

/* ===== 全局路由 ===== */
static const HmiAction_t s_enter_powered_off_0[] = {
    HMI_ACT_CLEAR_ALL_HEADS,
    HMI_ACT_SHOW_DASH,
    HMI_ACT_LED_POWER_ON,
    HMI_ACT_LED_TIMER_OFF,
    HMI_ACT_LED_PAUSE_OFF,
    HMI_ACT_LED_CHILD_LOCK_OFF,
    HMI_ACT_RESET_OFF_TIMER
};
static const HmiAction_t s_exit_powered_off_1[] = {
        (HmiAction_t)0 /* empty terminator */
};
static const HmiRoute_t s_routes_powered_off_2[] = {
    { KEY_ONOFF, HMI_EVT_LONG, HMI_ACT_GO_WORKING, 0 }
};
static const HmiGuard_t s_guards_powered_off_3[] = {
    { HMI_GUARD_ALWAYS, HMI_TO_OFF_TO_SLEEP_MS, HMI_ACT_GO_DEEP_SLEEP, 0 }
};

static const HmiAction_t s_enter_deep_sleep_4[] = {
    HMI_ACT_CLEAR_ALL_HEADS,
    HMI_ACT_DISPLAY_ALL_OFF,
    HMI_ACT_LED_ALL_OFF
};
static const HmiAction_t s_exit_deep_sleep_5[] = {
        (HmiAction_t)0 /* empty terminator */
};
static const HmiRoute_t s_routes_deep_sleep_6[] = {
    { KEY_ONOFF, HMI_EVT_LONG, HMI_ACT_GO_WORKING, 0 }
};
static const HmiGuard_t s_guards_deep_sleep_7[] = {
    { 0, 0, 0, 0 } /* empty terminator */
};

static const HmiAction_t s_enter_working_8[] = {
    HMI_ACT_CLEAR_ALL_HEADS,
    HMI_ACT_HOTHEAD_CLEAR,
    HMI_ACT_SHOW_POWER_MODE,
    HMI_ACT_LED_POWER_ON,
    HMI_ACT_LED_TIMER_OFF,
    HMI_ACT_LED_PAUSE_OFF,
    HMI_ACT_LED_CHILD_LOCK_OFF,
    HMI_ACT_RESET_IDLE_TIMER
};
static const HmiAction_t s_exit_working_9[] = {
        (HmiAction_t)0 /* empty terminator */
};
static const HmiRoute_t s_routes_working_10[] = {
    { KEY_ONOFF, HMI_EVT_LONG, HMI_ACT_GO_POWERED_OFF, 0 },
    { KEY_STOP, HMI_EVT_TAP, HMI_ACT_TOGGLE_PAUSE, 0 },
    { KEY_LOCK, HMI_EVT_LONG, HMI_ACT_TOGGLE_CHILD_LOCK, 0 },
    { KEY_LEFT_P_SET_UP, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, 0 },
    { KEY_RIGHT_P_SET_UP, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, 1 },
    { KEY_LEFT_P_SET, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, 2 },
    { KEY_RIGHT_P_SET, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, 3 }
};
static const HmiGuard_t s_guards_working_11[] = {
    { HMI_GUARD_ALL_IDLE, HMI_TO_IDLE_TO_STANDBY_MS, HMI_ACT_CLEAR_HOTHEAD, 0 },
    { HMI_GUARD_ALL_IDLE, HMI_TO_IDLE_TO_OFF_MS, HMI_ACT_GO_POWERED_OFF, 0 }
};

static const HmiAction_t s_enter_paused_12[] = {
    HMI_ACT_SHOW_PA,
    HMI_ACT_LED_PAUSE_ON,
    HMI_ACT_LED_TIMER_OFF
};
static const HmiAction_t s_exit_paused_13[] = {
    HMI_ACT_SHOW_POWER_MODE,
    HMI_ACT_LED_PAUSE_OFF,
    HMI_ACT_RESET_IDLE_TIMER
};
static const HmiRoute_t s_routes_paused_14[] = {
    { KEY_ONOFF, HMI_EVT_LONG, HMI_ACT_GO_POWERED_OFF, 0 },
    { KEY_STOP, HMI_EVT_TAP, HMI_ACT_TOGGLE_PAUSE, 0 }
};
static const HmiGuard_t s_guards_paused_15[] = {
    { 0, 0, 0, 0 } /* empty terminator */
};

const HmiGlobalNodeCfg_t hmi_global_nodes[HMI_NODE_COUNT] = {
    [HMI_NODE_POWERED_OFF] = {
        s_enter_powered_off_0, 7,
        s_exit_powered_off_1, 0,
        s_routes_powered_off_2, 1,
        s_guards_powered_off_3, 1,
    },
    [HMI_NODE_DEEP_SLEEP] = {
        s_enter_deep_sleep_4, 3,
        s_exit_deep_sleep_5, 0,
        s_routes_deep_sleep_6, 1,
        s_guards_deep_sleep_7, 0,
    },
    [HMI_NODE_WORKING] = {
        s_enter_working_8, 8,
        s_exit_working_9, 0,
        s_routes_working_10, 7,
        s_guards_working_11, 2,
    },
    [HMI_NODE_PAUSED] = {
        s_enter_paused_12, 3,
        s_exit_paused_13, 3,
        s_routes_paused_14, 2,
        s_guards_paused_15, 0,
    }
};

/* ===== Zone 路由 ===== */
static const HmiRoute_t s_zone_routes_idle_16[] = {
    { KEY_LEFT_P_SET_UP, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, 0 },
    { KEY_RIGHT_P_SET_UP, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, 1 },
    { KEY_LEFT_P_SET, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, 2 },
    { KEY_RIGHT_P_SET, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, 3 }
};
static const HmiRoute_t s_zone_routes_selecting_17[] = {
    { HMI_KEY_HEAD_SELF, HMI_EVT_TAP, HMI_ACT_CONFIRM_SELECT_IMMEDIATE, 0 },
    { HMI_KEY_HEAD_OTHER, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, -128 },
    { KEY_POWER_0, HMI_EVT_TAP, HMI_ACT_SET_POWER, 0 },
    { KEY_POWER_1, HMI_EVT_TAP, HMI_ACT_SET_POWER, 1 },
    { KEY_POWER_2, HMI_EVT_TAP, HMI_ACT_SET_POWER, 2 },
    { KEY_POWER_3, HMI_EVT_TAP, HMI_ACT_SET_POWER, 3 },
    { KEY_POWER_4, HMI_EVT_TAP, HMI_ACT_SET_POWER, 4 },
    { KEY_POWER_5, HMI_EVT_TAP, HMI_ACT_SET_POWER, 5 },
    { KEY_POWER_6, HMI_EVT_TAP, HMI_ACT_SET_POWER, 6 },
    { KEY_POWER_7, HMI_EVT_TAP, HMI_ACT_SET_POWER, 7 },
    { KEY_POWER_8, HMI_EVT_TAP, HMI_ACT_SET_POWER, 8 },
    { KEY_POWER_9, HMI_EVT_TAP, HMI_ACT_SET_POWER, 9 },
    { KEY_POWER_9, HMI_EVT_LONG, HMI_ACT_ENTER_BOOST, 0 },
    { KEY_TIME_SET, HMI_EVT_TAP, HMI_ACT_ENTER_TIMER_SETTING, 0 }
};
static const HmiRoute_t s_zone_routes_cooking_18[] = {
    { KEY_LEFT_P_SET_UP, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, 0 },
    { KEY_RIGHT_P_SET_UP, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, 1 },
    { KEY_LEFT_P_SET, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, 2 },
    { KEY_RIGHT_P_SET, HMI_EVT_TAP, HMI_ACT_SELECT_HEAD, 3 },
    { KEY_POWER_0, HMI_EVT_TAP, HMI_ACT_SET_POWER, 0 },
    { KEY_POWER_1, HMI_EVT_TAP, HMI_ACT_SET_POWER, 1 },
    { KEY_POWER_2, HMI_EVT_TAP, HMI_ACT_SET_POWER, 2 },
    { KEY_POWER_3, HMI_EVT_TAP, HMI_ACT_SET_POWER, 3 },
    { KEY_POWER_4, HMI_EVT_TAP, HMI_ACT_SET_POWER, 4 },
    { KEY_POWER_5, HMI_EVT_TAP, HMI_ACT_SET_POWER, 5 },
    { KEY_POWER_6, HMI_EVT_TAP, HMI_ACT_SET_POWER, 6 },
    { KEY_POWER_7, HMI_EVT_TAP, HMI_ACT_SET_POWER, 7 },
    { KEY_POWER_8, HMI_EVT_TAP, HMI_ACT_SET_POWER, 8 },
    { KEY_POWER_9, HMI_EVT_TAP, HMI_ACT_SET_POWER, 9 },
    { KEY_POWER_9, HMI_EVT_LONG, HMI_ACT_ENTER_BOOST, 0 },
    { KEY_TIME_SET, HMI_EVT_TAP, HMI_ACT_ENTER_TIMER_SETTING, 0 }
};
const HmiZoneCfg_t hmi_zone_nodes[HMI_ZONE_COUNT] = {
    [HMI_ZONE_IDLE] = {
        s_zone_routes_idle_16, 4,
        0xFF, 0, 0,
    },
    [HMI_ZONE_SELECTING] = {
        s_zone_routes_selecting_17, 14,
        HMI_TO_SELECT_CONFIRM_MS, HMI_ACT_CONFIRM_SELECT, 0,
    },
    [HMI_ZONE_COOKING] = {
        s_zone_routes_cooking_18, 16,
        0xFF, 0, 0,
    }
};

/* ===== 进程路由 ===== */
static const HmiRoute_t s_proc_routes_timer_setting_19[] = {
    { KEY_TIME_SET, HMI_EVT_TAP, HMI_ACT_CONFIRM_TIMER, 0 },
    { KEY_TIME_SET, HMI_EVT_LONG, HMI_ACT_CANCEL_TIMER_SETTING, 0 },
    { KEY_ADD, HMI_EVT_TAP, HMI_ACT_TIMER_ADJUST, 1 },
    { KEY_SUB, HMI_EVT_TAP, HMI_ACT_TIMER_ADJUST, -1 }
};
static const HmiRoute_t s_proc_routes_timer_active_20[] = {
    { KEY_TIME_SET, HMI_EVT_TAP, HMI_ACT_ENTER_TIMER_SETTING, 1 },
    { KEY_TIME_SET, HMI_EVT_LONG, HMI_ACT_CANCEL_TIMER_ACTIVE, 0 }
};
static const HmiRoute_t s_proc_routes_boost_active_21[] = {
    { KEY_POWER_0, HMI_EVT_TAP, HMI_ACT_EXIT_BOOST_SET_POWER, 0 },
    { KEY_POWER_1, HMI_EVT_TAP, HMI_ACT_EXIT_BOOST_SET_POWER, 1 },
    { KEY_POWER_2, HMI_EVT_TAP, HMI_ACT_EXIT_BOOST_SET_POWER, 2 },
    { KEY_POWER_3, HMI_EVT_TAP, HMI_ACT_EXIT_BOOST_SET_POWER, 3 },
    { KEY_POWER_4, HMI_EVT_TAP, HMI_ACT_EXIT_BOOST_SET_POWER, 4 },
    { KEY_POWER_5, HMI_EVT_TAP, HMI_ACT_EXIT_BOOST_SET_POWER, 5 },
    { KEY_POWER_6, HMI_EVT_TAP, HMI_ACT_EXIT_BOOST_SET_POWER, 6 },
    { KEY_POWER_7, HMI_EVT_TAP, HMI_ACT_EXIT_BOOST_SET_POWER, 7 },
    { KEY_POWER_8, HMI_EVT_TAP, HMI_ACT_EXIT_BOOST_SET_POWER, 8 },
    { KEY_POWER_9, HMI_EVT_TAP, HMI_ACT_EXIT_BOOST_SET_POWER, 9 },
    { KEY_TIME_SET, HMI_EVT_TAP, HMI_ACT_ENTER_TIMER_SETTING, 0 }
};
const HmiProcessCfg_t hmi_process_nodes[HMI_PROC_COUNT] = {
    [HMI_PROC_TIMER_SETTING] = {
        s_proc_routes_timer_setting_19, 4,
        HMI_TO_TIMER_CONFIRM_MS, HMI_ACT_CONFIRM_TIMER, 0,
    },
    [HMI_PROC_TIMER_ACTIVE] = {
        s_proc_routes_timer_active_20, 2,
        0xFF, 0, 0,
    },
    [HMI_PROC_BOOST_ACTIVE] = {
        s_proc_routes_boost_active_21, 11,
        HMI_TO_BOOST_MAX_MS, HMI_ACT_EXIT_BOOST, 0,
    }
};

/* ===== 元素配置 ===== */
const HmiElementCfg_t hmi_elements = {
    .timer_adjust_min     = 1,
    .timer_adjust_max     = 99,
    .timer_adjust_step    = 1,
    .boost_power_level    = 9,
    .stack_max_depth      = 4,
    .blink_phase_ms       = 300,
    .blink_pause_override = 0,
    .mode_default         = HMI_SEG_MODE_POWER,
    .level_led_mode       = 1,  /* single: JSON elements.led.level.mode */
};

const HmiModeRule_t hmi_mode_rules[] = {
    { HMI_MODE_COND_POWERED_OFF, HMI_SEG_MODE_DASH },
    { HMI_MODE_COND_DEEP_SLEEP, HMI_SEG_MODE_OFF },
    { HMI_MODE_COND_PAUSED, HMI_SEG_MODE_ASCII },
    { HMI_MODE_COND_ANY_TIMER_SETTING, HMI_SEG_MODE_TIMER_SETTING },
};

const HmiDisplayPatterns_t hmi_patterns = {
    .dash = "--------",
    .pa   = "PAPAPAPA",
    .off  = "        ",
};

/* ===== 童锁白名单 ===== */
static const HmiRoute_t s_child_lock_whitelist[] = {
    { KEY_ONOFF, HMI_EVT_TAP, HMI_ACT_BEEP_VALID, 0 },
    { KEY_ONOFF, HMI_EVT_LONG, HMI_ACT_BEEP_VALID, 0 }
};

/* ===== 顶级配置聚合 ===== */
const HmiConfig_t hmi_cfg = {
    .timeouts                = hmi_timeouts,
    .power_on_seq            = hmi_power_on_seq,
    .power_on_seq_len        = HMI_POWER_ON_SEQ_LEN,
    .global_nodes            = hmi_global_nodes,
    .zone_nodes              = hmi_zone_nodes,
    .process_nodes           = hmi_process_nodes,
    .elements                = &hmi_elements,
    .mode_rules              = hmi_mode_rules,
    .mode_rule_count         = HMI_MODE_RULE_COUNT,
    .patterns                = &hmi_patterns,
    .child_lock_whitelist    = s_child_lock_whitelist,
    .child_lock_whitelist_len = HMI_CHILD_LOCK_WL_LEN,
};
