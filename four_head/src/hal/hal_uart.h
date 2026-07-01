/**
 * hal_uart.h —— 调试串口接口（UART3）
 *
 * 依赖: sc32f1xxx_uart.h（固件库）
 * 层级: HAL
 *
 * 基础 API (Print/PutChar/HexDump): 始终阻塞式，轮询 TX 标志
 * HAL_UART_Debug_Printf: 格式化打印，阻塞/非阻塞由 DEBUG_PRINT_MODE 控制
 *   - BLOCKING:    vsprintf → PutChar 逐字节轮询
 *   - NONBLOCKING: vsnprintf → 环形缓冲 → TX 中断发送
 */
#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>

/* ========== 调试串口选择: 3=UART3 ========== */
#define DEBUG_UART_SEL  3u

/* ========== 调试打印模式 ========== */
#define DEBUG_PRINT_MODE_BLOCKING     0u
#define DEBUG_PRINT_MODE_NONBLOCKING  1u

#ifndef DEBUG_PRINT_MODE
#define DEBUG_PRINT_MODE  DEBUG_PRINT_MODE_NONBLOCKING
#endif

/* ========== 环形缓冲配置（仅非阻塞模式） ========== */
#if DEBUG_PRINT_MODE == DEBUG_PRINT_MODE_NONBLOCKING
#define DEBUG_TX_BUF_SIZE  256u
#endif

/* ---- 基础 API（始终阻塞） ---- */
void HAL_UART_Debug_Init(void);
void HAL_UART_Debug_PutChar(uint8_t ch);
void HAL_UART_Debug_Print(const char *str);
void HAL_UART_Debug_HexDump(const uint8_t *data, uint16_t len);

/* ---- 格式化打印（阻塞/非阻塞由 DEBUG_PRINT_MODE 决定） ---- */
int HAL_UART_Debug_Printf(const char *fmt, ...);

/* ---- 非阻塞便捷函数 ---- */
void HAL_UART_Debug_Print_NB(const char *str);

/* ---- 状态查询 ---- */
uint8_t HAL_UART_Debug_TxBusy(void);

/* 主循环等待段调用: 非阻塞模式安全网，阻塞模式空函数 */
void HAL_UART_Debug_Flush(void);

/* ---- 调试串口接收 (非阻塞, 供对齐模块轮询) ---- */
void    HAL_UART_Debug_RX_Enable(void);
uint8_t HAL_UART_Debug_GetChar(uint8_t *ch);  /* 返回1=有字符 */

/* UART1_3_5_7816 共享中断入口 */
void HAL_UART_Debug_ISR(void);

#endif /* HAL_UART_H */
