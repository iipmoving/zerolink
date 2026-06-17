/**
 * app_hmi_io.h —— AppHmi 输入输出接口定义 (v2.3)
 *
 * 数据流:
 *   INPUT: DrvKey → AppHmi (按键事件)
 *   INPUT: AppPower → AppHmi (功率状态)
 *   OUTPUT: AppHmi → DrvDisplay (显示数据)
 *   OUTPUT: AppHmi → DrvBuzzer (蜂鸣命令)
 *
 * Include 权限:
 *   - 仅 app_hmi.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/app_hmi_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef APP_HMI_IO_H
#define APP_HMI_IO_H

#include "../core/std_module.h"
#include <stdint.h>

/* ========== OUTPUT (AppHmi 给别的模块提供的数据) ========== */

/* AppHmi_to_DrvDisplay_Params — 输出给显示驱动的数据 (v2.3 完整模块名) */
typedef struct {
    int8_t   hot_head_idx;        /* 热点炉头索引 (-1=无) */
    uint8_t  seg_chars[8];        /* 8位段码字符 */
    uint8_t  seg_blink[4];        /* 炉头闪烁控制 */
    uint8_t  seg_mode;            /* 显示模式 */
    uint8_t  leds_power;          /* 电源LED */
    uint8_t  leds_timer;          /* 定时LED */
    uint8_t  leds_pause;          /* 暂停LED */
    uint8_t  leds_child_lock;     /* 童锁LED */
    uint8_t  leds_head_select[4]; /* 炉头选择LED */
    uint8_t  leds_power_level[10];/* 功率档位LED */
} MODULE_OUTPUT_PARAMS(AppHmi, DrvDisplay);

/* AppHmi_to_DrvDisplay_Output_Link — 输出管道 */
typedef struct {
    uint8_t  status;          /* ST_NEW/ST_OUT */
    uint8_t  max_count;       /* 最大炉头数 = 4 */
    uint8_t  count;           /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppHmi, DrvDisplay) *params;
} MODULE_OUTPUT_LINK(AppHmi, DrvDisplay);

/* AppHmi_to_DrvBuzzer_Params — 输出给蜂鸣器的数据 */
typedef struct {
    uint8_t  sound_type;      /* 蜂鸣类型: 1=短按, 2=长按, 3=成功, 4=失败, 5=报警 */
    uint8_t  res[3];
} MODULE_OUTPUT_PARAMS(AppHmi, DrvBuzzer);

/* AppHmi_to_DrvBuzzer_Output_Link — 输出管道 (@OUTPUT_CALLBACK 即时路由) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(AppHmi, DrvBuzzer) *params;
} MODULE_OUTPUT_LINK(AppHmi, DrvBuzzer);

/* AppHmi_Output — 输出聚合 (v2.3 对称命名: 成员名 = {Consumer}_params) */
typedef struct {
    MODULE_OUTPUT_LINK(AppHmi, DrvDisplay) *DrvDisplay_params;  /* → DrvDisplay */
    MODULE_OUTPUT_LINK(AppHmi, DrvBuzzer)  *DrvBuzzer_params;   /* → DrvBuzzer (@OUTPUT_CALLBACK) */
} MODULE_OUTPUT(AppHmi);

/* ========== INPUT (AppHmi 从别的模块得到的数据) ========== */

/* DrvKey_to_AppHmi_Input_Params — 从 DrvKey 得到的数据 (布局与 DrvKey OUTPUT 一致) */
typedef struct {
    uint8_t  key_code;        /* 逻辑键码 KeyCode_t */
    uint8_t  key_state;       /* 按键状态: 按下/释放/长按/重复 */
    uint8_t  head_index;      /* 关联炉头索引 */
    uint8_t  res[1];
} MODULE_INPUT_PARAMS(DrvKey, AppHmi);

/* DrvKey_to_AppHmi_Input_Link — 输入管道 (与 DrvKey OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(DrvKey, AppHmi) *params;
} MODULE_INPUT_LINK(DrvKey, AppHmi);

/* AppPower_to_AppHmi_Input_Params — 从 AppPower 得到的数据 */
typedef struct {
    uint8_t  head_index;      /* 炉头索引 */
    uint8_t  power_on;        /* 功率开关 */
    uint8_t  power_level;     /* 功率档位 */
    uint8_t  res[1];
    uint16_t actual_power;    /* 实际功率 W */
} MODULE_INPUT_PARAMS(AppPower, AppHmi);

/* AppPower_to_AppHmi_Input_Link — 输入管道 (与 AppPower OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppPower, AppHmi) *params;
} MODULE_INPUT_LINK(AppPower, AppHmi);

/* AppHmi_Input — 输入聚合 (v2.3 对称命名: 成员名 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(DrvKey, AppHmi)   *DrvKey_params;   /* 从 DrvKey 得到 (INPUT_GET_SLOT 宏展开使用) */
    MODULE_INPUT_LINK(AppPower, AppHmi) *AppPower_params; /* 从 AppPower 得到 (INPUT_GET_SLOT 宏展开使用) */
} MODULE_INPUT(AppHmi);

#endif /* APP_HMI_IO_H */