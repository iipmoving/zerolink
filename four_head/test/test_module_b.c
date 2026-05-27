/**
 * test_module_b.c —— 测试模块B（How are you 对话应答方）
 *
 * v2.0: 已迁移到 __weak 回调直调架构。
 *       定义 MSG_SCHEDULER_ALLOWED 可切回 v1.0 消息调度器模式。
 *
 * 对话流:
 *   B ←TestB_OnChatA(round)— A   (强符号接收)
 *   B —TestA_OnChatB(round)→ A   (__weak直调)
 */
#include "hal/hal_uart.h"
#include <stddef.h>

/* __weak 发送给 A (v2.0 方式) */
__weak void TestA_OnChatB(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

/* 收到A的问候 (强符号, 被 test_module_a.c 的 __weak TestB_OnChatA 调用) */
void TestB_OnChatA(uint16_t param, void *data_ptr)
{
    (void)data_ptr;

    HAL_UART_Debug_Print("  [B] recv <- A: \"How are you?\" (round=");
    HAL_UART_Debug_PutChar('0' + (uint8_t)param);
    HAL_UART_Debug_Print(")\r\n");

    HAL_UART_Debug_Print("  [B] send -> A: \"I'm fine, thank you!\" (round=");
    HAL_UART_Debug_PutChar('0' + (uint8_t)param);
    HAL_UART_Debug_Print(")\r\n");
    TestA_OnChatB(param, NULL);
}

void TestB_Init(void)
{
    /* v2.0: 无需注册, __weak 链接器自动接线 */
}
