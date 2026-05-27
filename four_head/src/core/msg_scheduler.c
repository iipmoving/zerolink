/**
 * msg_scheduler.c —— [废弃 v2.0]
 *
 * v1.0 消息调度器实现（环形队列+回调注册表），已被 __weak 回调直调架构完全替代。
 * 本文件保留仅作为 v1.0 历史参考，不参与编译。
 *
 * 替代方案: 见 core/interface_map.h + memory/methodology_weak-callback-zero-dependency.md
 */

/* 如需编译测试模块过渡期使用，定义 MSG_SCHEDULER_ALLOWED 后重编译 */
#ifndef MSG_SCHEDULER_ALLOWED
#error "msg_scheduler.c is DEPRECATED (v2.0). Use __weak callbacks instead."
#endif

#include "msg_scheduler.h"

static Msg_t     s_msg_slot[MSG_SLOT_DEPTH];
static uint8_t   s_slot_head;
static uint8_t   s_slot_tail;
static uint8_t   s_slot_count;

typedef struct {
    MsgId_t       msg_id;
    MsgHandler_t  handler;
} HandlerEntry_t;

static HandlerEntry_t s_handlers[MAX_HANDLERS];
static uint8_t        s_handler_count;

void MsgScheduler_Init(void)
{
    s_slot_head  = 0u;
    s_slot_tail  = 0u;
    s_slot_count = 0u;
    s_handler_count = 0u;
}

void MsgScheduler_Register(MsgId_t id, MsgHandler_t handler)
{
    if (handler == NULL) return;
    if (s_handler_count >= MAX_HANDLERS) return;
    s_handlers[s_handler_count].msg_id  = id;
    s_handlers[s_handler_count].handler = handler;
    s_handler_count++;
}

void Msg_Post(MsgId_t id, uint16_t param, void *data_ptr)
{
    if (s_slot_count >= MSG_SLOT_DEPTH) return;
    s_msg_slot[s_slot_tail].id       = id;
    s_msg_slot[s_slot_tail].param    = param;
    s_msg_slot[s_slot_tail].data_ptr = data_ptr;
    s_slot_tail = (s_slot_tail + 1u) % MSG_SLOT_DEPTH;
    s_slot_count++;
}

void MsgScheduler_Run1ms(void)
{
    uint8_t i;
    if (s_slot_count == 0u) return;
    Msg_t msg = s_msg_slot[s_slot_head];
    s_slot_head = (s_slot_head + 1u) % MSG_SLOT_DEPTH;
    s_slot_count--;
    for (i = 0u; i < s_handler_count; i++) {
        if (s_handlers[i].msg_id == msg.id) {
            s_handlers[i].handler(msg.id, msg.param, msg.data_ptr);
        }
    }
}
