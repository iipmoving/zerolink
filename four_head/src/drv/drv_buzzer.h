/**
 * drv_buzzer.h —— 蜂鸣器驱动层接口
 *
 * 依赖: hal_buzzer.h
 * 层级: DRV —— 声音模式选择 + 音符序列调度
 *
 * 两种模式:
 *   普通蜂鸣器: 固定频率方波, count×on_time+interval 循环
 *   美声/和弦: 可变频率音符序列, PE1频率驱动 + PE4包络控制, 电容尾音
 *
 * 调用链:
 *   TIMER1_IRQHandler → HAL_Buzzer_ISR_Drive() (GPIO toggle, ~125us级)
 *   主循环1ms → Drv_Buzzer_Timer_1ms() (时序推进, 1ms级)
 */
#ifndef DRV_BUZZER_H
//#define DRV_BUZZER_H

#include <stdint.h>

/* ========== 蜂鸣器模式选择 ========== */
#define DRV_BUZZ_OUT_MY    1u    /* 美声/和弦模式 */
#define DRV_BUZZ_OUT_NORM  2u    /* 普通蜂鸣器模式 */

/* ========== 美声模式枚举（和弦音符序列） ========== */
enum {
    DRV_BUZZ_MY_FAULT = 0,
    DRV_BUZZ_MY_SD,           /* 上电音 */
    DRV_BUZZ_MY_ON,           /* 开机音 */
    DRV_BUZZ_MY_OFF,          /* 关机音 */
    DRV_BUZZ_MY_ADD,          /* 增加音 */
    DRV_BUZZ_MY_SUB,          /* 减少音 */
    DRV_BUZZ_MY_WORK,         /* 工作提示音 */
    DRV_BUZZ_MY_DING,         /* 叮 */
    DRV_BUZZ_MY_DONG,         /* 咚 */
    DRV_BUZZ_MY_END,          /* 工作结束音 */
    DRV_BUZZ_MY_P_ON,         /* 上电旋律(旧) */
    DRV_BUZZ_MY_KEY,          /* 按键音 */
    DRV_BUZZ_MY_EER,          /* 错误音 */
    DRV_BUZZ_MY_NEW_P_ON,     /* 上电音(新) */
    DRV_BUZZ_MY_START,        /* 启动音 */
    DRV_BUZZ_MY_SMALLSTAR     /* 小星星歌曲 */
};

/* ========== 普通蜂鸣器模式枚举 ========== */
enum {
    DRV_BUZZ_DBUG = 64,       /* 调试: 5次短鸣 */
    DRV_BUZZ_SD,              /* 上电: 1次30ms */
    DRV_BUZZ_KEY,             /* 按键: 1次20ms */
    DRV_BUZZ_END_B5,          /* 结束5次 */
    DRV_BUZZ_END_B10,         /* 结束10次 */
    DRV_BUZZ_LONG05,          /* 长鸣50ms */
    DRV_BUZZ_LONG1S_120,      /* 1秒: 120次×100ms */
    DRV_BUZZ_06,              /* 6次短鸣 */
    DRV_BUZZ_06_TIPS,         /* 6次提示 */
    DRV_BUZZ_03_COOKTIPS,     /* 烹饪提示3次 */
    DRV_BUZZ_ERR,             /* 错误: 10次短鸣 */
    DRV_BUZZ_LONG1S_5,        /* 5次长鸣 */
    DRV_BUZZ_TIPS,            /* 提示: 2次短鸣 */
    DRV_BUZZ_TIPS_1,          /* 提示: 1次短鸣 */
    DRV_BUZZ_PAN_ON           /* 锅具检测: 5次 */
};

/* ========== 可调参数 ========== */

#define BUZZ_BLANK_MS        150u  /* 连续调用消隐窗口(ms) */
#define BUZZ_TICK_DIV_10MS    10u  /* 1ms→10ms分频系数 */

/* ========== 公共接口 ========== */

void Drv_Buzzer_Init(void);

/* 选择并启动蜂鸣器音效 (out_sel: DRV_BUZZ_OUT_MY 或 DRV_BUZZ_OUT_NORM) */
void Drv_Buzzer_Select(uint8_t out_sel, uint8_t mode);

/* 1ms时基处理: 美声步进(1ms) + 普通模式倒计时(每10ms) */
void Drv_Buzzer_Timer_1ms(void);

#endif /* DRV_BUZZER_H */
