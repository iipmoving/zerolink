/**
 * test_module_a.c —— 测试模块A（How are you 对话发起方）
 *
 * v2.0: 已迁移到 __weak 回调直调架构。
 *       定义 MSG_SCHEDULER_ALLOWED 可切回 v1.0 消息调度器模式。
 *
 * 对话流:
 *   A —TestB_OnChatA(round)→ B  (__weak直调)
 *   A ←TestA_OnChatB(round)— B   (强符号接收)
 *   ...共5轮
 */
#include "hal/hal_uart.h"
#include <stddef.h>

/* __weak 发送给 B (v2.0 方式) */
__weak void TestB_OnChatA(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

/* __weak 发送给自己 (v2.0 方式, 兼容旧结构) */
__weak void TestA_OnChatB(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

#define CHAT_ROUNDS 5u

static uint8_t s_round;

/* 收到B的回复 (强符号, 被 test_module_b.c 的 __weak TestA_OnChatB 调用) */
void TestA_OnChatB(uint16_t param, void *data_ptr)
{
    (void)data_ptr;

    HAL_UART_Debug_Print("  [A] recv <- B: \"I'm fine, thank you!\" (round=");
    HAL_UART_Debug_PutChar('0' + (uint8_t)param);
    HAL_UART_Debug_Print(")\r\n");

    s_round = (uint8_t)param + 1u;
    if (s_round < CHAT_ROUNDS) {
        HAL_UART_Debug_Print("  [A] send -> B: \"How are you?\" (round=");
        HAL_UART_Debug_PutChar('0' + s_round);
        HAL_UART_Debug_Print(")\r\n");
        TestB_OnChatA(s_round, NULL);
    } else {
        HAL_UART_Debug_Print("  [A] chat end.\r\n");
    }
}

void TestA_StartChat(void)
{
    s_round = 0u;
    HAL_UART_Debug_Print("\r\n=== Chat Demo Start (2 modules, __weak v2.0) ===\r\n");
    HAL_UART_Debug_Print("  [A] send -> B: \"How are you?\" (round=0)\r\n");
    TestB_OnChatA(0u, NULL);
}

void TestA_Init(void)
{
    /* v2.0: 无需注册, __weak 链接器自动接线 */
}
