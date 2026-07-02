/**
 * @file    debug_uart.h
 * @brief   调试串口输出模块 (I2C 模式: UARTX = Uart2)
 *
 * 用法:
 *   Debug_Send("power", 1234);     → 输出 "power=1234\r\n"
 *   Debug_Send("state", "OK");     → 输出 "state=OK\r\n"
 *   Debug_Printf("seq=%d", n);     → 输出 "seq=123\r\n"
 *
 * 依赖: API_UART_DMA_SendValue() — DMA 发送, 非阻塞
 *       调试串口已在 SystemInitial() 中初始化
 */
#ifndef DEBUG_UART_H
#define DEBUG_UART_H

#include <stdint.h>

/* 调试输出缓冲区大小 (字节) */
#define DEBUG_BUF_SIZE  128

/**
 * @brief 发送 key=value 调试信息
 * @param key  字符串键名
 * @param val  整数值
 */
void Debug_Send(const char *key, int32_t val);

/**
 * @brief 发送 key=string 调试信息
 * @param key  字符串键名
 * @param val  字符串值
 */
void Debug_Send_Str(const char *key, const char *val);

/**
 * @brief 发送格式化调试信息
 * @param fmt  printf 风格格式串
 * @param ...  可变参数
 */
void Debug_Printf(const char *fmt, ...);

/**
 * @brief 发送原始字符串 (直接输出, 不加 key=)
 * @param str  字符串
 */
void Debug_Puts(const char *str);

#endif /* DEBUG_UART_H */