/* msg_scheduler.h — WASM stub: 消息调度器空实现 */
#ifndef MSG_SCHEDULER_H
#define MSG_SCHEDULER_H

#include "msg_def.h"

/* WASM: 消息总线不连接真实硬件驱动, 回调注册和投递都是空操作 */
#define MsgScheduler_Register(id, handler) ((void)0)
#define Msg_Post(id, param, ptr)           ((void)0)

#endif
