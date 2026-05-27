/* msg_def.h — WASM stub: 消息ID + 消息结构体 */
#ifndef MSG_DEF_H
#define MSG_DEF_H

#include <stdint.h>
#include <stddef.h>

#define CT_ASSERT(expr, tag) /* WASM: skip compile-time assert */

typedef enum {
    MSG_KEY_EVENT,
    MSG_COOKING_CTRL,
    MSG_POWER_CTRL,
    MSG_FAN_CTRL,
    MSG_DISPLAY_REFRESH,
    MSG_TIMER_100MS,
    MSG_TIMER_1S,
    MSG_COMM_TX_DONE,
    MSG_COMM_DATA_UPDATE,
    MSG_SYSTEM_ERROR,
    MSG_TEST_CHAT_A,
    MSG_TEST_CHAT_B,
    MSG_COMM_POLL_TICK,
    MSG_REG_DATA_READY,
    MSG_BUZZER_CTRL,
    MSG_COUNT
} MsgId_t;

#pragma pack(4)
typedef struct {
    MsgId_t  id;
    uint16_t param;
    void    *data_ptr;
} Msg_t;
#pragma pack()

typedef void (*MsgHandler_t)(MsgId_t id, uint16_t param, void *data_ptr);

#endif
