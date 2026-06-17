/**
 * @file    apphmi_io.h
 * @brief   AppHmi Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * 人机交互逻辑 — 从 Key/Power 输入，输出显示到 Display 和蜂鸣到 Buzzer
 *
 * 输入源:
 *   DrvKey → AppHmi  (按键事件 → HMI)
 *   AppPower → AppHmi  (功率状态 → Hmi)
 *
 * 输出目标:
 *   AppHmi → DrvDisplay  (显示数据 → DrvDisplay (段码/LED))
 *   AppHmi → DrvBuzzer  (蜂鸣命令 → DrvBuzzer (@OUTPUT_CALLBACK 即时路由, 不走 Switcher))
 */

#ifndef APPHMI_IO_H
//#define APPHMI_IO_H   /* L0 阻断: 禁用 include guard */

#include <stdint.h>
#include "../core/std_module.h"

#pragma pack(4)

/* ================================================================
 * OUTPUT — 本模块输出的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppHmi → DrvDisplay  输出参数  (显示数据 → DrvDisplay (段码/LED))
 * ------------------------------------------------------------------ */
typedef struct {
    int8_t hot_head_idx;     /* 热点炉头索引 (-1=无) */
    uint8_t seg_chars[8];     /* 8位段码字符 */
    uint8_t seg_blink[4];     /* 炉头闪烁控制 */
    uint8_t seg_mode;     /* 显示模式 */
    uint8_t leds_power;     /* 电源 LED */
    uint8_t leds_timer;     /* 定时 LED */
    uint8_t leds_pause;     /* 暂停 LED */
    uint8_t leds_child_lock;     /* 童锁 LED */
    uint8_t leds_head_select[4];     /* 炉头选择 LED */
    uint8_t leds_power_level[10];     /* 功率档位 LED */
} MODULE_OUTPUT_PARAMS(AppHmi, DrvDisplay);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppHmi, DrvDisplay) *params;
} MODULE_OUTPUT_LINK(AppHmi, DrvDisplay);

/* ------------------------------------------------------------------
 * AppHmi → DrvBuzzer  输出参数  (蜂鸣命令 → DrvBuzzer (@OUTPUT_CALLBACK 即时路由, 不走 Switcher))
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t sound_type;     /* 蜂鸣类型: 1=短按, 2=长按, 3=成功, 4=失败, 5=报警 */
} MODULE_OUTPUT_PARAMS(AppHmi, DrvBuzzer);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppHmi, DrvBuzzer) *params;
} MODULE_OUTPUT_LINK(AppHmi, DrvBuzzer);

/* AppHmi_Output — 输出聚合 (对称命名: 成员 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppHmi, DrvDisplay)  DrvDisplay_params;  /* → DrvDisplay */
    MODULE_OUTPUT_LINK(AppHmi, DrvBuzzer)  DrvBuzzer_params;  /* → DrvBuzzer */
} MODULE_OUTPUT(AppHmi);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * DrvKey → AppHmi  输入参数  (按键事件 → HMI)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t key_code;     /* 按键码 KeyCode_t */
    uint8_t key_state;     /* 按键状态 KEY_STATE_* */
    uint8_t head_index;     /* 关联炉头索引 */
} MODULE_INPUT_PARAMS(DrvKey, AppHmi);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(DrvKey, AppHmi) *params;
} MODULE_INPUT_LINK(DrvKey, AppHmi);

/* ------------------------------------------------------------------
 * AppPower → AppHmi  输入参数  (功率状态 → Hmi)
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t head_index;     /* 炉头索引 0-3 */
    uint8_t power_on;     /* 功率开关 0/1 */
    uint8_t power_level;     /* 功率档位 0-9 */
    uint16_t actual_power;     /* 实际功率 W */
} MODULE_INPUT_PARAMS(AppPower, AppHmi);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppPower, AppHmi) *params;
} MODULE_INPUT_LINK(AppPower, AppHmi);

/* AppHmi_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(DrvKey, AppHmi)  DrvKey_params;  /* 从 DrvKey 来 */
    MODULE_INPUT_LINK(AppPower, AppHmi)  AppPower_params;  /* 从 AppPower 来 */
} MODULE_INPUT(AppHmi);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(AppHmi);

#endif /* APPHMI_IO_H */
