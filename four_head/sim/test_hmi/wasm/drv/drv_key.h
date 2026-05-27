/* drv_key.h — WASM stub: 按键码 + 状态常量 */
/* 值与实际MCU drv_key.h 严格一致, 由 wasm_adapter.js KEY_CODE 对应 */
#ifndef DRV_KEY_H
#define DRV_KEY_H

#include <stdint.h>

typedef enum {
    KEY_NONE = 0,
    KEY_SUB = 2,        /* MINUS / KEY_SUB        */
    KEY_ADD = 3,        /* PLUS  / KEY_ADD        */
    KEY_ONOFF = 4,      /* POWER / KEY_ONOFF      */
    KEY_TIME_SET = 5,   /* TIMER / KEY_TIME_SET   */
    KEY_STOP = 6,       /* PAUSE / KEY_STOP       */
    KEY_LOCK = 8,       /* CHILD_LOCK / KEY_LOCK  */
    KEY_BIND = 9,       /* ZONE / KEY_BIND        */
    KEY_RIGHT_P_SET = 12,      /* HEAD_4 */
    KEY_LEFT_P_SET = 13,       /* HEAD_3 */
    KEY_RIGHT_P_SET_UP = 14,   /* HEAD_2 */
    KEY_LEFT_P_SET_UP = 15,    /* HEAD_1 */
    KEY_POWER_0 = 22,
    KEY_POWER_1 = 23,
    KEY_POWER_2 = 24,
    KEY_POWER_3 = 25,
    KEY_POWER_4 = 26,
    KEY_POWER_5 = 27,
    KEY_POWER_6 = 28,
    KEY_POWER_7 = 29,
    KEY_POWER_8 = 30,
    KEY_POWER_9 = 31,
    KEY_COUNT
} KeyCode_t;

#define KEY_STATE_PRESS   0x01u
#define KEY_STATE_LONG    0x02u
#define KEY_STATE_RELEASE 0x04u
#define KEY_STATE_REPEAT  0x08u
#define KEY_STATE_TAP     0x10u

#define KEY_DEBOUNCE_CNT   2u

/* WASM: 不需要硬件驱动 */
#define Drv_Key_Init()  ((void)0)
#define Drv_Key_Scan()  ((void)0)

#endif
