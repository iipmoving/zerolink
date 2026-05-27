/**
 ******************************************************************************
 * @file    emc_logic.h
 * @author  EMC Logic Generator
 * @version V1.0
 * @date    2026-05-16
 * @brief   翰林煮面炉·数码管款 — 逻辑层头文件
 *
 * @details
 *   本文件定义：
 *   1. 时间常量、参数边界宏
 *   2. 状态枚举、按键枚举、事件枚举、动作枚举
 *   3. 硬件输入/显示/控制/锁定等结构体
 *   4. 回调函数类型及注册接口
 *   5. 公有API函数声明
 *
 * @note
 *   - 目标平台：8051 (KEIL C)
 *   - 禁止直接包含任何 HAL 层头文件
 *   - 所有外部硬件访问通过回调接口实现
 *
 * @warning
 *   - 状态枚举不得新增任何状态（规格书硬约束）
 *   - 时间常量值必须与规格书4.3节完全一致
 *
 * Copyright (c) 2026
 ******************************************************************************
 */

#ifndef _EMC_LOGIC_H_
#define _EMC_LOGIC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 包含头文件
 * ======================================================================== */

#include <stdint.h>
#include <stdbool.h>

/* ========================================================================
 * 宏定义常量（规格书第4.3节 + 第4.1/4.2节参数边界）
 * ======================================================================== */

/* --- 时间常量 --- */
#define POWER_ON_SELFTEST_MS      1500U   /* 上电自检持续时间 */
#define VERSION_DISPLAY_MS        1500U   /* 每个版本号显示时间 */
#define LONG_PRESS_MS             1500U   /* 长按判定阈值 */
#define FUNC_SELECT_TIMEOUT_MS   60000U   /* 功能选择超时(回待机) */
#define TEMP_LOCK_THRESHOLD         80U   /* 首次上电锁定解锁温度(°C) */
#define INDICATOR_BLINK_MS         500U   /* 指示灯/数码管闪烁周期 */
#define TEMP_HINT_MS              3000U   /* 临时提示显示时间 */
#define WATER_RESUME_WAIT_MIN_S     30U   /* 水位不足后最短等待时间 */
#define WATER_RESUME_WAIT_MAX_S    120U   /* 水位不足后最长等待时间 */
#define CYCLE_PERIOD_MS             10U   /* 主循环周期 */

/* --- 参数边界 --- */
#define MIN_WATER_ML               100U   /* 最小出水量(ml) */
#define MAX_WATER_ML               500U   /* 最大出水量(ml) */
#define WATER_STEP_ML               20U   /* 水量步进(ml) */
#define MIN_TIME_S                   0U   /* 最小时间(秒) */
#define MAX_TIME_S                 480U   /* 最大时间(秒) = 8分钟 */
#define TIME_STEP_S                 30U   /* 时间步进(秒) */

/* --- 缓冲与计数 --- */
#define KEY_BUFFER_SIZE              8U   /* 按键循环buffer深度 */
#define FUNC_CONFIG_COUNT            11U   /* 功能配置表条目(索引1-10) */
#define DIGIT_COUNT                   4U   /* 数码管位数 */

/* ========================================================================
 * 枚举定义
 * ======================================================================== */

/** @brief 系统状态枚举 — 必须且仅有9个状态（规格书1.1节） */
typedef enum {
    S_POWER_ON    = 0,   /* 上电自检 */
    S_VERSION     = 1,   /* 版本号显示 */
    S_SHUTDOWN    = 2,   /* 关机待机 */
    S_DEMO        = 3,   /* 演示模式 */
    S_STANDBY     = 4,   /* 开机待机 */
    S_FUNC_SELECT = 5,   /* 功能选择 */
    S_COOKING     = 6,   /* 烹饪中 */
    S_PAUSE       = 7,   /* 烹饪暂停 */
    S_FAULT       = 8,   /* 故障 */
    STATE_MAX     = 9    /* 状态总数(校验用) */
} SystemState_e;

/** @brief 按键枚举 — 必须且仅有14个按键（规格书2.1节）
 *
 *  ⚠️ 本机型只有一个 KEY_START_PAUSE 物理按键，
 *  但它兼任电源键角色（在关机/演示/版本状态下执行开机/退出）。
 *  按键驱动调用 key_input_callback() 时，一律传入 KEY_START_PAUSE（值13），
 *  逻辑层内部自动映射。KEY_POWER 保留在枚举中仅因规格书定义完整性。 */
typedef enum {
    KEY_POWER       = 0,   /* 规格书定义 — 本机型无此物理键，保留映射 */
    KEY_M1          = 1,   /* 功能1 */
    KEY_M2          = 2,   /* 功能2 */
    KEY_M3          = 3,   /* 功能3 */
    KEY_M4          = 4,   /* 功能4 */
    KEY_M5          = 5,   /* 功能5 */
    KEY_M6          = 6,   /* 功能6 */
    KEY_M7          = 7,   /* 功能7 */
    KEY_M8          = 8,   /* 功能8 */
    KEY_M9          = 9,   /* 功能9 */
    KEY_M10         = 10,  /* 功能10 */
    KEY_ADD_WATER   = 11,  /* 加水 */
    KEY_ADD_TIME    = 12,  /* 加时间 */
    KEY_START_PAUSE = 13,  /* 启动/暂停 — 兼任电源键角色 */
    KEY_MAX         = 14   /* 按键总数(校验用) */
} KeyCode_e;

/** @brief 按键事件类型 */
typedef enum {
    KEY_EVENT_NULL    = 0,   /* 无事件 */
    KEY_EVENT_PRESS   = 1,   /* 按下 */
    KEY_EVENT_SHORT   = 2,   /* 短按释放（< 1.5s） */
    KEY_EVENT_LONG    = 3,   /* 长按触发（>= 1.5s） */
    KEY_EVENT_REPEAT  = 4,   /* 长按重复（每500ms一次） */
    KEY_EVENT_RELEASE = 5    /* 释放（通用） */
} key_event_t;

/** @brief 状态转移事件枚举 — 与规格书按键及定时事件一一对应 */
typedef enum {
    EV_KEY_POWER_SHORT       = 0,    /* 电源键短按 */
    EV_KEY_POWER_LONG        = 1,    /* 电源键长按 */
    EV_KEY_M1_SHORT          = 2,    /* M1键短按 */
    EV_KEY_M2_SHORT          = 3,    /* M2键短按 */
    EV_KEY_M3_SHORT          = 4,    /* M3键短按 */
    EV_KEY_M4_SHORT          = 5,    /* M4键短按 */
    EV_KEY_M5_SHORT          = 6,    /* M5键短按 */
    EV_KEY_M6_SHORT          = 7,    /* M6键短按 */
    EV_KEY_M7_SHORT          = 8,    /* M7键短按 */
    EV_KEY_M8_SHORT          = 9,    /* M8键短按 */
    EV_KEY_M9_SHORT          = 10,   /* M9键短按 */
    EV_KEY_M10_SHORT         = 11,   /* M10键短按 */
    EV_KEY_ADD_WATER_SHORT   = 12,   /* 加水键短按 */
    EV_KEY_ADD_TIME_SHORT    = 13,   /* 加时间键短按 */
    EV_KEY_START_PAUSE_SHORT = 14,   /* 启动/暂停键短按 */
    EV_KEY_START_PAUSE_LONG  = 15,   /* 启动/暂停键长按 */

    /* 定时事件 */
    EV_TIME_1500MS           = 20,   /* 1.5秒定时 */
    EV_TIME_3000MS           = 21,   /* 3秒定时 */
    EV_TIME_60000MS          = 22,   /* 60秒定时 */
    EV_WATER_ZERO            = 23,   /* 水量归零 */
    EV_TIME_ZERO             = 24,   /* 时间归零 */

    /* 特殊事件 */
    EV_DEMO_ENTER            = 25,   /* 进入演示模式 */

    EV_KEY_ANY               = 30,   /* 任意按键（通配） */
    EV_ANY                   = 31,   /* 任意事件（通配） */
    EVENT_MAX                = 33    /* 事件总数 */
} transition_event_t;

/** @brief 动作枚举 — 状态转移时执行的动作 */
typedef enum {
    ACTION_NONE              = 0,
    ACTION_ENTER_POWER_ON    = 1,
    ACTION_ENTER_VERSION     = 2,
    ACTION_ENTER_SHUTDOWN    = 3,
    ACTION_ENTER_STANDBY     = 4,
    ACTION_ENTER_FUNC_SELECT = 5,
    ACTION_ENTER_COOKING     = 6,
    ACTION_ENTER_PAUSE       = 7,
    ACTION_ENTER_DEMO        = 8,
    ACTION_ENTER_FAULT       = 9,
    ACTION_SHOW_LOCK_HINT    = 10,
    ACTION_SHOW_NWAT_HINT    = 11,
    ACTION_ADJUST_WATER      = 12,
    ACTION_ADJUST_TIME       = 13,
    ACTION_TRY_START_COOK    = 14,
    ACTION_CANCEL_COOK       = 15,
    ACTION_SHOW_END          = 16,   /* 烹饪完成显示End */
    ACTION_RESUME_COOK       = 17,   /* 从暂停恢复（保留冻结参数） */
    ACTION_SWITCH_TO_HEATING = 18    /* 出水→加热阶段切换 */
} action_id_t;

/** @brief 蜂鸣器命令枚举 */
typedef enum {
    BUZZ_CLICK  = 1,   /* 短滴一声（确认反馈） */
    BUZZ_DOUBLE = 2,   /* 双短音（警告） */
    BUZZ_LONG   = 3,   /* 长音（错误） */
    BUZZ_CHORD  = 4,   /* 和弦音（启动） */
    BUZZ_ERROR  = 5    /* 错误音 */
} buzzer_cmd_t;

/** @brief 状态变化类型枚举 */
typedef enum {
    STATUS_STATE_CHANGE = 1,   /* 状态改变 */
    STATUS_FUNC_SELECT  = 2,   /* 功能选择变化 */
    STATUS_COOK_START   = 3,   /* 烹饪开始 */
    STATUS_COOK_END     = 4,   /* 烹饪结束 */
    STATUS_FAULT        = 5,   /* 故障发生 */
    STATUS_PARAM_CHANGE = 6    /* 参数变化（水量/时间） */
} status_type_t;

/** @brief 提示类型枚举 */
typedef enum {
    HINT_NONE         = 0,   /* 无提示 */
    HINT_TEMP_LOCK    = 1,   /* 首次上电锁定提示 */
    HINT_QR_LOCK      = 2,   /* 扫码锁定提示 */
    HINT_WATER_LOW    = 3    /* 水位不足提示 */
} hint_type_t;

/* ========================================================================
 * 结构体定义
 * ======================================================================== */

/** @brief 硬件输入结构体（统一回调获取，只读） */
typedef struct {
    int16_t  water_temp;       /* 水温(0.1°C)，如850=85.0°C */
    uint8_t  water_level_ok;   /* 水位是否正常(1=正常, 0=不足) */
    uint8_t  pot_present;      /* 锅具是否在位(1=在位, 0=移开) */
    uint8_t  fault_code;       /* 故障码(0=无故障) */
} HardwareInputs_t;

/** @brief 标志位域结构体 — 所有BOOL变量集中管理 */
typedef struct {
    uint8_t is_initialized     : 1;   /* 是否已初始化 */
    uint8_t is_water_phase     : 1;   /* 是否在出水阶段 */
    uint8_t is_end_phase       : 1;   /* 是否在End显示阶段(烹饪完成) */
    uint8_t temp_lock_active   : 1;   /* 首次上电锁定 */
    uint8_t qr_lock_active     : 1;   /* 扫码锁定 */
    uint8_t hint_active        : 1;   /* 临时提示是否激活 */
    uint8_t show_time_disp     : 1;   /* S_FUNC_SELECT: 1=显示时间, 0=显示水量 */
    uint8_t reserved           : 1;   /* 保留 */
} EmcFlags_t;

/** @brief 功能配置表条目 */
typedef struct {
    uint16_t default_water;          /* 默认出水量(ml), 范围100~500 */
    uint16_t default_time;           /* 默认加热时间(秒), 范围0~480 */
} FuncConfig_t;

/** @brief 运行时参数 */
typedef struct {
    int8_t   selected_func_id;       /* 当前选中功能(1-10)，-1=未选 */
    uint16_t current_water_ml;       /* 当前设定水量(ml) */
    uint16_t current_time_s;         /* 当前设定时间(秒) */
    uint16_t remain_water_ml;        /* 剩余出水量(ml) */
    uint16_t remain_time_s;          /* 剩余时间(秒) */
} RuntimeParams_t;

/** @brief 锁定状态 */
typedef struct {
    uint16_t water_low_wait_s;       /* 水位不足等待计时(秒) */
} LockState_t;

/** @brief 临时提示状态 */
typedef struct {
    uint8_t  hint_type;              /* 提示类型(见 hint_type_t) */
    uint32_t start_time_ms;          /* 提示开始时间戳 */
    uint8_t  restore_digit[DIGIT_COUNT];  /* 恢复用的原始数码管ASCII */
    uint8_t  restore_dp;             /* 恢复用的小数点掩码 */
    uint8_t  restore_colon;          /* 恢复用的时钟点掩码 */
} HintState_t;

/** @brief 数码管显示缓冲区（ASCII格式） */
typedef struct {
    uint8_t digit[DIGIT_COUNT];      /* 每位1字节ASCII码 */
    uint8_t dp_mask;                 /* bit位小数点: bit0=DIG0, bit1=DIG1, ... */
    uint8_t colon_mask;              /* bit位时钟点: bit0=左, bit1=右 */
    uint8_t special_mode;            /* 特殊模式: 0=正常, 1=跑马灯, 2=闪烁 */
} DispBuffer_t;

/** @brief 段码输出结构体（逻辑层→控件接口） */
typedef struct {
    uint8_t seg[DIGIT_COUNT];        /* 每位1字节段码(7段+DP) */
    uint8_t dp_mask;                 /* 小数点掩码 */
    uint8_t colon_mask;              /* 时钟点掩码 */
} SegCode_t;

/** @brief LED状态结构体（位域） */
typedef struct {
    uint16_t power_led      : 1;     /* 电源指示灯 */
    uint16_t func_leds      : 10;    /* 功能灯M1-M10 */
    uint16_t add_water_led  : 1;     /* 加水指示灯 */
    uint16_t add_time_led   : 1;     /* 加时间指示灯 */
    uint16_t water_disp_led : 1;     /* 水量指示灯 */
    uint16_t time_disp_led  : 1;     /* 时间指示灯 */
    uint16_t led_reserved   : 1;     /* 保留 */
} LedState_t;

/** @brief 按键缓冲条目 */
typedef struct {
    uint8_t  key_code;               /* 按键枚举值 */
    uint8_t  event;                  /* 事件类型 */
    uint16_t timestamp_ms;           /* 事件时间戳(相对时间) */
} KeyEvent_t;

/** @brief 按键循环缓冲区 */
typedef struct {
    uint8_t  head;                   /* 读指针 */
    uint8_t  tail;                   /* 写指针 */
    KeyEvent_t buffer[KEY_BUFFER_SIZE];
} KeyBuffer_t;

/** @brief 状态转移表条目 */
typedef struct {
    uint8_t  next_state;             /* 下一状态（STATE_MAX=保持当前） */
    uint8_t  action;                 /* 执行的动作编号 */
} StateTransition_t;

/** @brief 按键掩码条目 */
typedef struct {
    uint16_t short_mask;             /* 短按有效掩码 */
    uint16_t long_mask;              /* 长按有效掩码 */
} KeyMask_t;

/** @brief 主控制结构体（整合所有状态数据） */
typedef struct {
    /* 系统时间 */
    uint32_t system_time_ms;         /* 系统运行时间(10ms计数) */
    uint32_t state_enter_time_ms;    /* 进入当前状态的时间戳 */

    /* 状态 */
    uint8_t  current_state;          /* 当前状态(0~STATE_MAX-1) */
    uint8_t  previous_state;         /* 上一状态 */

    /* 子结构体 */
    EmcFlags_t      flags;           /* 标志位域 */
    RuntimeParams_t params;          /* 运行时参数 */
    LockState_t     lock;            /* 锁定状态 */
    HintState_t     hint;            /* 临时提示 */
    HardwareInputs_t hw;             /* 硬件输入（每周期拉取） */

    /* 显示 */
    DispBuffer_t disp;               /* 显示缓冲区 */
    SegCode_t    seg;                /* 段码输出 */
    LedState_t   led;                /* LED状态 */

    /* 回调 */
    struct {
        void (*disp_output)(SegCode_t *p_seg, LedState_t *p_led);
        void (*buzzer)(uint8_t cmd);
        void (*status_change)(uint8_t type, void *p_value);
        void (*get_hw_inputs)(HardwareInputs_t *p_inputs);
    } callbacks;
} EmcCtrl_t;

/* ========================================================================
 * 回调函数类型定义
 * ======================================================================== */

/** @brief 显示输出回调：逻辑层→控件层 */
typedef void (*DispOutputCb_t)(SegCode_t *p_seg, LedState_t *p_led);

/** @brief 蜂鸣器回调 */
typedef void (*BuzzerCb_t)(uint8_t cmd);

/** @brief 状态变化回调 */
typedef void (*StatusChangeCb_t)(uint8_t type, void *p_value);

/** @brief 硬件输入回调：硬件层→逻辑层（统一接口，一次拉取） */
typedef void (*GetHardwareInputsCb_t)(HardwareInputs_t *p_inputs);

/* ========================================================================
 * 公有API函数声明
 * ======================================================================== */

/**
 * @brief  逻辑层初始化 — 上电后调用一次
 * @note   进入 S_POWER_ON 状态，开始上电自检流程
 */
void emc_logic_init(void);

/**
 * @brief  10ms周期主循环 — 硬件定时器每10ms调用一次
 * @note   执行顺序：拉取硬件→处理按键→超时检测→倒计时→故障检测→显示更新→输出回调
 */
void key_disp_cycle(void);

/**
 * @brief  按键输入回调 — 底层按键驱动检测到事件后调用
 * @param  key_code  按键码(KeyCode_e)
 *           ⚠️ 本机型仅 KEY_START_PAUSE(13) 为物理按键，
 *              它兼任电源键角色，逻辑层自动映射。
 * @param  event     事件类型(key_event_t)
 * @note   仅将事件写入循环缓冲，不在此处做状态判断
 */
void key_input_callback(uint8_t key_code, uint8_t event);

/**
 * @brief  统一注册回调函数
 * @param  p_disp_cb        显示输出回调(可为NULL)
 * @param  p_buzzer_cb      蜂鸣器回调(可为NULL)
 * @param  p_status_cb      状态变化回调(可为NULL)
 * @param  p_hw_inputs_cb   硬件输入回调(可为NULL)
 */
void emc_logic_register_callbacks(DispOutputCb_t p_disp_cb,
                                  BuzzerCb_t p_buzzer_cb,
                                  StatusChangeCb_t p_status_cb,
                                  GetHardwareInputsCb_t p_hw_inputs_cb);

/**
 * @brief  获取主控制结构体指针（供测试/调试用，只读）
 * @return 指向全局 EmcCtrl_t 实例的指针
 */
const EmcCtrl_t *emc_logic_get_ctrl(void);

#ifdef __cplusplus
}
#endif

#endif /* _EMC_LOGIC_H_ */
