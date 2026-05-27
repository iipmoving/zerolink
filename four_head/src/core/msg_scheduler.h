/**
 * msg_scheduler.h —— [废弃 v2.0]
 *
 * v1.0 消息调度器，已被 __weak 回调直调架构完全替代。
 * 禁止任何新代码 #include 本文件。
 *
 * 替代方案: 见 core/interface_map.h + memory/methodology_weak-callback-zero-dependency.md
 */
#ifndef MSG_SCHEDULER_H
#define MSG_SCHEDULER_H

#ifdef MSG_SCHEDULER_ALLOWED
/* 仅测试文件兼容过渡期允许，生产代码禁止定义此宏 */
#include "msg_def.h"
#define MSG_SLOT_DEPTH   8u
#define MAX_HANDLERS     16u
void MsgScheduler_Init(void);
void MsgScheduler_Register(MsgId_t id, MsgHandler_t handler);
void Msg_Post(MsgId_t id, uint16_t param, void *data_ptr);
void MsgScheduler_Run1ms(void);
#else
#error "msg_scheduler.h is DEPRECATED (v2.0). Use __weak callbacks instead. See interface_map.h."
#endif

#endif /* MSG_SCHEDULER_H */
