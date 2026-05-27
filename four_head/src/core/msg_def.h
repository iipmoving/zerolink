/**
 * msg_def.h —— [废弃 v2.0]
 *
 * v1.0 消息类型定义，已被 __weak 回调直调架构完全替代。
 * 禁止任何新代码 #include 本文件。
 *
 * 替代方案: __weak 回调使用统一签名 (uint16_t param, void *data_ptr)，无需 Msg_t/MsgId_t。
 */
#ifndef MSG_DEF_H
#define MSG_DEF_H

#ifdef MSG_SCHEDULER_ALLOWED
/* 仅测试文件兼容过渡期允许 */
#include <stdint.h>
#include <stddef.h>
#define CT_ASSERT(expr, tag) extern int __ct_##tag[(expr) ? 1 : -1]
typedef uint16_t MsgId_t;
#define MSG_COUNT  17u
#pragma pack(4)
typedef struct {
    MsgId_t  id;
    uint16_t param;
    void    *data_ptr;
} Msg_t;
#pragma pack()
typedef void (*MsgHandler_t)(MsgId_t id, uint16_t param, void *data_ptr);
#else
#error "msg_def.h is DEPRECATED (v2.0). Use __weak callbacks instead."
#endif

#endif /* MSG_DEF_H */
