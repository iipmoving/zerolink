/**
 * drv_key.h —— 按键驱动层接口
 *
 * 依赖: msg_scheduler.h
 * 层级: DRV —— 物理→逻辑映射 + 去抖 + 发送MSG_KEY_EVENT
 *
 * 触摸通道→PCB丝印→逻辑键码映射:
 *   通道7-24, 28-30 → 21个触摸焊盘(DF_TK1~DF_TK21) → 逻辑键码
 *
 * 去抖策略: 连续2次读到相同键值才确认
 * 组合键: 相邻功率键同时按下(如TK12+TK13→Power_1)
 */
#ifndef DRV_KEY_H
//#define DRV_KEY_H

#include <stdint.h>

/* ========== 逻辑键码 ========== */
typedef enum {
    KEY_NONE = 0,

    /* 操作键 */
    KEY_ONOFF,              /* 开关                                   */
    KEY_SUB,                /* 减                                     */
    KEY_ADD,                /* 加                                     */
    KEY_STOP,               /* 停止/暂停                              */
    KEY_TIME_SET,           /* 定时设置                               */
    KEY_BIND,               /* 双圈绑定                               */
    KEY_LOCK,               /* 童锁                                   */

    /* 功率调节键（方向型） */
    KEY_RIGHT_P_SET,        /* 右功率设置                             */
    KEY_LEFT_P_SET,         /* 左功率设置                             */
    KEY_RIGHT_P_SET_UP,     /* 上右功率设置                           */
    KEY_LEFT_P_SET_UP,      /* 上左功率设置                           */

    /* 功率直选键 0-9 */
    KEY_POWER_0,
    KEY_POWER_1,
    KEY_POWER_2,
    KEY_POWER_3,
    KEY_POWER_4,
    KEY_POWER_5,
    KEY_POWER_6,
    KEY_POWER_7,
    KEY_POWER_8,
    KEY_POWER_9,

    KEY_COUNT
} KeyCode_t;

/* ========== 按键参数（可调） ========== */
#define KEY_DEBOUNCE_CNT   2u     /* 去抖确认次数（每次10ms）              */
#define KEY_LONG_THRESH   100u    /* 长按阈值（100次×10ms=1s）             */
#define KEY_REPEAT_PERIOD  30u    /* 连按周期（30次×10ms=300ms）           */
#define KEY_RELEASE_DB_CNT 5u     /* 松手去抖次数（连续5次读0才确认松手）   */

/* ========== 按键状态 ========== */
#define KEY_STATE_PRESS   0x01u   /* 单按未松手（去抖通过立刻发）          */
#define KEY_STATE_LONG    0x02u   /* 长按（≥1s）                           */
#define KEY_STATE_RELEASE 0x04u   /* 释放                                  */
#define KEY_STATE_REPEAT  0x08u   /* 连按（长按后每300ms）                 */
#define KEY_STATE_TAP     0x10u   /* 单按已松手（<1s松手，未发过LONG）     */

/* ========== 公共接口 ========== */
void Drv_Key_Init(void);
void Drv_Key_Scan(void);         /* 每10ms调用(槽位3)                      */

#endif /* DRV_KEY_H */
