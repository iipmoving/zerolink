/**
 * app_hmi.h — HMI JSON 驱动引擎类型定义
 *
 * 所有枚举+结构体对应 four_head_v4.json 的语义。
 * 数据表由 cfg/gen_hmi.js 从 JSON 自动生成到 cfg/hmi_data.c。
 * 引擎实现见 app_hmi.c。
 *
 * 依赖: drv_key.h (KeyCode_t, KEY_STATE_*)
 * 层级: APP —— 通过消息调度器收发
 */
#ifndef APP_HMI_H
#define APP_HMI_H

#include <stdint.h>

/* ================================================================
 * 一、系统状态枚举
 * ================================================================ */

/* 全局模式（替代 JSON global_routes 的字符串键） */
typedef enum {
    HMI_NODE_POWERED_OFF = 0,
    HMI_NODE_DEEP_SLEEP,
    HMI_NODE_WORKING,
    HMI_NODE_PAUSED,
    HMI_NODE_COUNT
} HmiGlobalNode_t;

/* Zone 状态 — 每个炉头的 node */
typedef enum {
    HMI_ZONE_IDLE = 0,
    HMI_ZONE_SELECTING,
    HMI_ZONE_COOKING,
    HMI_ZONE_COUNT
} HmiZoneNode_t;

/* 正交进程（叠加在 Zone 状态上） */
typedef enum {
    HMI_PROC_TIMER_SETTING = 0,
    HMI_PROC_TIMER_ACTIVE,
    HMI_PROC_BOOST_ACTIVE,
    HMI_PROC_COUNT
} HmiProcessNode_t;

/* ================================================================
 * 二、动作枚举（替代 JSON action 字符串）
 * ================================================================ */

typedef enum {
    /* 全局模式转换 */
    HMI_ACT_GO_WORKING = 0,
    HMI_ACT_GO_POWERED_OFF,
    HMI_ACT_GO_DEEP_SLEEP,
    HMI_ACT_TOGGLE_PAUSE,
    HMI_ACT_TOGGLE_CHILD_LOCK,

    /* Zone — 选中 + 功率 */
    HMI_ACT_SELECT_HEAD,           /* param = 炉头索引 0-3            */
    HMI_ACT_CONFIRM_SELECT,
    HMI_ACT_CONFIRM_SELECT_IMMEDIATE,
    HMI_ACT_SET_POWER,             /* param = 档位 0-9               */

    /* Boost */
    HMI_ACT_ENTER_BOOST,
    HMI_ACT_EXIT_BOOST,
    HMI_ACT_EXIT_BOOST_SET_POWER,  /* param = 退出后档位              */

    /* 定时 */
    HMI_ACT_ENTER_TIMER_SETTING,
    HMI_ACT_CONFIRM_TIMER,
    HMI_ACT_CANCEL_TIMER_SETTING,
    HMI_ACT_CANCEL_TIMER_ACTIVE,
    HMI_ACT_TIMER_ADJUST,          /* param = delta (有符号)          */

    /* 显示内容 */
    HMI_ACT_SHOW_DASH,
    HMI_ACT_SHOW_PA,
    HMI_ACT_SHOW_POWER_MODE,
    HMI_ACT_DISPLAY_ALL_OFF,

    /* 炉头管理 */
    HMI_ACT_CLEAR_ALL_HEADS,
    HMI_ACT_HOTHEAD_CLEAR,
    HMI_ACT_CLEAR_HOTHEAD,

    /* LED 指令 */
    HMI_ACT_LED_POWER_ON,
    HMI_ACT_LED_POWER_OFF,
    HMI_ACT_LED_ALL_OFF,
    HMI_ACT_LED_PAUSE_ON,
    HMI_ACT_LED_PAUSE_OFF,
    HMI_ACT_LED_TIMER_ON,
    HMI_ACT_LED_TIMER_OFF,
    HMI_ACT_LED_CHILD_LOCK_ON,
    HMI_ACT_LED_CHILD_LOCK_OFF,

    /* 计时器重置 */
    HMI_ACT_RESET_IDLE_TIMER,
    HMI_ACT_RESET_OFF_TIMER,

    /* 蜂鸣 */
    HMI_ACT_BEEP_VALID,
    HMI_ACT_BEEP_INVALID,

    HMI_ACT_COUNT
} HmiAction_t;

/* ================================================================
 * 三、显示枚举
 * ================================================================ */

/* 段码显示模式（替代 'dash'/'power'/'ascii'/'off'/'timer_setting'） */
typedef enum {
    HMI_SEG_MODE_POWER = 0,
    HMI_SEG_MODE_DASH,
    HMI_SEG_MODE_OFF,
    HMI_SEG_MODE_ASCII,
    HMI_SEG_MODE_TIMER_SETTING,
    HMI_SEG_MODE_COUNT
} HmiSegMode_t;

/* ================================================================
 * 四、超时索引枚举（替代 JSON timeouts 的字符串键）
 * ================================================================ */

typedef enum {
    HMI_TO_SELECT_CONFIRM_MS = 0,
    HMI_TO_TIMER_CONFIRM_MS,
    HMI_TO_BOOST_MAX_MS,
    HMI_TO_IDLE_TO_STANDBY_MS,
    HMI_TO_IDLE_TO_OFF_MS,
    HMI_TO_OFF_TO_SLEEP_MS,
    HMI_TO_DEFAULT_TIMER_MIN,
    HMI_TO_POWER_ON_ALL_ON_MS,
    HMI_TO_VERSION_SHOW_MS,
    HMI_TO_POWER_KEY_LONG_MS,
    HMI_TO_COUNT
} HmiTimeoutId_t;

/* ================================================================
 * 五、ModeRule 条件枚举
 * ================================================================ */

typedef enum {
    HMI_MODE_COND_POWERED_OFF = 0,
    HMI_MODE_COND_DEEP_SLEEP,
    HMI_MODE_COND_PAUSED,
    HMI_MODE_COND_ANY_TIMER_SETTING,
    HMI_MODE_COND_COUNT
} HmiModeCondition_t;

/* ================================================================
 * 六、Guard 检查类型
 * ================================================================ */

typedef enum {
    HMI_GUARD_ALWAYS = 0,
    HMI_GUARD_ALL_IDLE,
    HMI_GUARD_COUNT
} HmiGuardCheck_t;

/* ================================================================
 * 七、数据结构
 * ================================================================ */

/* 事件类型简码（路由表用） */
#define HMI_EVT_TAP   0u
#define HMI_EVT_LONG  1u

/* 动态路由键（非物理按键，引擎运行时解析） */
#define HMI_KEY_HEAD_SELF   0xFFu  /* 按当前 hotHead 对应的炉头键       */
#define HMI_KEY_HEAD_OTHER  0xFEu  /* 按其他炉头键（切换选中）          */
#define HMI_PARAM_DYNAMIC   (-128)  /* 动作参数动态解析, 避让合法负值(-1)  */

/* 上电序列 LED 指令 */
#define HMI_LEDS_NONE   0u
#define HMI_LEDS_ALL_ON  1u
#define HMI_LEDS_ALL_OFF 2u

/* 路由表条目 — 4 字节 */
#pragma pack(2)
typedef struct {
    uint8_t  key;       /* KeyCode_t: 物理按键码                      */
    uint8_t  event;     /* HMI_EVT_TAP / HMI_EVT_LONG                 */
    uint8_t  action;    /* HmiAction_t: 匹配后执行的动作               */
    int8_t   param;     /* 动作参数 (-1 = 动态 %head)                  */
} HmiRoute_t;
#pragma pack()

/* Guard 条目 — 4 字节 */
#pragma pack(2)
typedef struct {
    uint8_t  check;     /* HmiGuardCheck_t                            */
    uint8_t  ms_key;    /* HmiTimeoutId_t: 超时阈值索引               */
    uint8_t  action;    /* HmiAction_t                                */
    int8_t   param;
} HmiGuard_t;
#pragma pack()

/* 上电序列步骤 */
typedef struct {
    uint16_t delay_ms;       /* 本步骤持续时间，0=立即跳转              */
    uint8_t  seg_mode;       /* HmiSegMode_t                          */
    char     seg_chars[8];   /* 8 个 ASCII 字符                       */
    uint8_t  leds_all;       /* HMI_LEDS_NONE / ALL_ON / ALL_OFF      */
    int8_t   goto_node;      /* -1=无跳转, 否则 HmiGlobalNode_t       */
} HmiPowerOnStep_t;

/* Zone / 进程节点配置（同形） */
typedef struct {
    const HmiRoute_t *routes;
    uint8_t           route_count;
    uint8_t           timeout_ms_key;  /* HmiTimeoutId_t, 0xFF=无超时  */
    uint8_t           timeout_action;  /* HmiAction_t                  */
    int8_t            timeout_param;
} HmiZoneCfg_t;

typedef HmiZoneCfg_t HmiProcessCfg_t;

/* 全局模式配置 */
typedef struct {
    const HmiAction_t *enter_actions;
    uint8_t            enter_count;
    const HmiAction_t *exit_actions;
    uint8_t            exit_count;
    const HmiRoute_t  *routes;
    uint8_t            route_count;
    const HmiGuard_t  *guards;
    uint8_t            guard_count;
} HmiGlobalNodeCfg_t;

/* ModeRule 条目 — 2 字节 */
typedef struct {
    uint8_t condition;  /* HmiModeCondition_t                         */
    uint8_t seg_mode;   /* HmiSegMode_t: 匹配后的显示模式              */
} HmiModeRule_t;

/* 显示图案（替代 showDash/showPA/showAllOff 硬编码） */
typedef struct {
    char dash[8];
    char pa[8];
    char off[8];
} HmiDisplayPatterns_t;

/* 元素参数聚合 */
typedef struct {
    uint8_t  timer_adjust_min;
    uint8_t  timer_adjust_max;
    uint8_t  timer_adjust_step;
    uint8_t  boost_power_level;
    uint8_t  stack_max_depth;
    uint16_t blink_phase_ms;
    uint8_t  blink_pause_override;   /* 1=暂停时闪烁，0=暂停时全灭   */
    uint8_t  mode_default;           /* HmiSegMode_t: ModeRule 回退值 */
    uint8_t  level_led_mode;         /* 0=gradient, 1=single          */
} HmiElementCfg_t;

/* 顶级配置聚合（只读，全部在 Flash 中） */
typedef struct {
    const uint32_t             *timeouts;
    const HmiPowerOnStep_t     *power_on_seq;
    uint8_t                     power_on_seq_len;
    const HmiGlobalNodeCfg_t   *global_nodes;
    const HmiZoneCfg_t         *zone_nodes;
    const HmiProcessCfg_t      *process_nodes;
    const HmiElementCfg_t      *elements;
    const HmiModeRule_t        *mode_rules;
    uint8_t                     mode_rule_count;
    const HmiDisplayPatterns_t *patterns;
    const HmiRoute_t           *child_lock_whitelist;
    uint8_t                     child_lock_whitelist_len;
} HmiConfig_t;

/* ================================================================
 * 八、引擎内部状态类型
 * ================================================================ */

/* 炉头运行时状态 */
typedef struct {
    uint8_t  node;               /* HmiZoneNode_t                       */
    uint8_t  power_level;        /* 0-9                                 */
    uint8_t  original_power;     /* Boost 前档位                        */
    uint8_t  boost_active;       /* bool                                */
    uint8_t  timer_setting;      /* bool                                */
    uint8_t  timer_active;       /* bool                                */
    uint16_t timer_value;        /* 分钟 1-99                           */
    uint32_t select_ticks;       /* 选中时刻 (100ms tick)               */
    uint32_t timer_set_ticks;    /* 定时设置时刻                         */
    int32_t  boost_remaining_ms; /* Boost 剩余 ms                       */
} HmiHead_t;

/* 引擎全局状态 */
typedef struct {
    uint8_t mode;                /* HmiGlobalNode_t                     */
    uint8_t child_lock;          /* bool                                */
    uint8_t paused;              /* bool                                */
    uint8_t in_power_on_seq;     /* bool: 上电序列进行中                  */
    uint8_t power_on_step;       /* 当前上电序列步骤                      */
    uint8_t power_on_step_ticks; /* 当前步骤已过 tick 数                  */
} HmiGlobalState_t;

/* 段码显示缓存 */
typedef struct {
    char    seg_chars[8];        /* 8 个段码字符                         */
    uint8_t seg_blink[4];        /* 每个 Zone 是否闪烁                   */
    uint8_t seg_mode;            /* HmiSegMode_t                        */
    int8_t  hot_head_idx;        /* 热点炉头索引 -1=无, 用于DP指示器      */
    uint8_t leds_power;          /* 电源 LED                            */
    uint8_t leds_timer;          /* 定时 LED                            */
    uint8_t leds_pause;          /* 暂停 LED                            */
    uint8_t leds_child_lock;     /* 童锁 LED                            */
    uint8_t leds_head_select[4]; /* 选头 LED ×4                         */
    uint8_t leds_power_level[10];/* 档位 LED ×10                        */
} HmiDisplayCache_t;

/* ================================================================
 * 九、公共 API
 * ================================================================ */

void App_Hmi_Init(void);
void App_Hmi_Run(void);

/* ================================================================
 * 十、外部引用
 * ================================================================ */

#include "cfg/hmi_data.h"

#endif /* APP_HMI_H */
