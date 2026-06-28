/**
 * @file    debug_uart.c
 * @brief   调试串口输出实现
 *
 * 使用 API_UART_DMA_SendValue() 通过 UARTX (Uart2) 输出。
 * DMA 发送为非阻塞, 调用后立即返回。
 *
 * 注意: DMA 发送需要缓冲区在发送期间保持有效,
 * 因此使用 static 全局缓冲区, 而非栈上局部变量。
 *
 * 数据格式: "key=value\r\n"   — PC 端按行解析
 */
#include "debug_uart.h"
#include "API_UART.H"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

/* DMA 发送缓冲区 — static 确保 DMA 读取期间地址有效 */
static char s_dma_buf[DEBUG_BUF_SIZE];

/* ================================================================
 * Debug_Send — 输出 key=value
 * ================================================================ */
void Debug_Send(const char *key, int32_t val)
{
    int n = snprintf(s_dma_buf, sizeof(s_dma_buf), "%s=%d\r\n", key, (int)val);
    if (n > 0)
    {
        uint16_t len = (n < (int)sizeof(s_dma_buf)) ? (uint16_t)n : (uint16_t)(sizeof(s_dma_buf) - 1);
        API_UART_DMA_SendValue(UARTX, (uint8_t*)s_dma_buf, len);
    }
}

/* ================================================================
 * Debug_Send_Str — 输出 key=string
 * ================================================================ */
void Debug_Send_Str(const char *key, const char *val)
{
    int n = snprintf(s_dma_buf, sizeof(s_dma_buf), "%s=%s\r\n", key, val);
    if (n > 0)
    {
        uint16_t len = (n < (int)sizeof(s_dma_buf)) ? (uint16_t)n : (uint16_t)(sizeof(s_dma_buf) - 1);
        API_UART_DMA_SendValue(UARTX, (uint8_t*)s_dma_buf, len);
    }
}

/* ================================================================
 * Debug_Printf — 格式化输出
 * ================================================================ */
void Debug_Printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(s_dma_buf, sizeof(s_dma_buf), fmt, args);
    va_end(args);
    if (n > 0)
    {
        uint16_t len = (n < (int)sizeof(s_dma_buf)) ? (uint16_t)n : (uint16_t)(sizeof(s_dma_buf) - 1);
        API_UART_DMA_SendValue(UARTX, (uint8_t*)s_dma_buf, len);
    }
}

/* ================================================================
 * Debug_Puts — 直接输出字符串
 * ================================================================ */
void Debug_Puts(const char *str)
{
    uint16_t len = (uint16_t)strlen(str);
    if (len > 0)
    {
        if (len >= sizeof(s_dma_buf))
            len = (uint16_t)(sizeof(s_dma_buf) - 1);
        memcpy(s_dma_buf, str, len);
        s_dma_buf[len] = '\0';
        API_UART_DMA_SendValue(UARTX, (uint8_t*)s_dma_buf, len);
    }
}