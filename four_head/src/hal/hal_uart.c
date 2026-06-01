/**
 * hal_uart.c —— 调试串口实现
 *
 * 依赖: hal_uart.h + sc32f1xxx_uart.h
 *
 * 阻塞式 TX: PutChar 等发送完成再返回
 * 仅调试用，打印时可能短暂阻塞主循环（每字符~174us）
 */
#include "hal_uart.h"
#include "sc32f1xxx_uart.h"
#include "sc32f1xxx_rcc.h"
#include "sc32L14xx.h"
#include <stdarg.h>

#define DEBUG_BAUDRATE       57600u
#define DEBUG_CLOCK           48000000u

#if (DEBUG_UART_SEL == 1)

#define DEBUG_UART      UART1

void HAL_UART_Debug_Init(void)
{
    UART_InitTypeDef uart_init;

    RCC_APB0PeriphClockCmd(RCC_APB0Periph_UART1, ENABLE);

    uart_init.UART_ClockFrequency = DEBUG_CLOCK;
    uart_init.UART_BaudRate       = DEBUG_BAUDRATE;
    uart_init.UART_Mode           = UART_Mode_10B;
    UART_Init(DEBUG_UART, &uart_init);

    UART_PinRemapConfig(DEBUG_UART, UART_PinRemap_A);

    UART_TXCmd(DEBUG_UART, ENABLE);

#if DEBUG_PRINT_MODE == DEBUG_PRINT_MODE_NONBLOCKING
    NVIC_EnableIRQ(UART1_3_5_7816_IRQn);
#endif
}

#else

#define DEBUG_UART      UART3

void HAL_UART_Debug_Init(void)
{
    UART_InitTypeDef uart_init;

    RCC_APB2Cmd(ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_UART3, ENABLE);

    uart_init.UART_ClockFrequency = DEBUG_CLOCK;
    uart_init.UART_BaudRate       = DEBUG_BAUDRATE;
    uart_init.UART_Mode           = UART_Mode_10B;
    UART_Init(DEBUG_UART, &uart_init);

    UART_PinRemapConfig(DEBUG_UART, UART_PinRemap_A);

    UART_TXCmd(DEBUG_UART, ENABLE);

#if DEBUG_PRINT_MODE == DEBUG_PRINT_MODE_NONBLOCKING
    NVIC_EnableIRQ(UART1_3_5_7816_IRQn);
#endif
}

#endif /* DEBUG_UART_SEL */

void HAL_UART_Debug_PutChar(uint8_t ch)
{
    UART_SendData(DEBUG_UART, ch);
    while (UART_GetFlagStatus(DEBUG_UART, UART_Flag_TX) == RESET) {}
    UART_ClearFlag(DEBUG_UART, UART_Flag_TX);
}

void HAL_UART_Debug_Print(const char *str)
{
    while (*str != '\0') {
        if (*str == '\n') {
            HAL_UART_Debug_PutChar('\r');
        }
        HAL_UART_Debug_PutChar((uint8_t)*str);
        str++;
    }
}

void HAL_UART_Debug_HexDump(const uint8_t *data, uint16_t len)
{
    static const char hex[] = "0123456789ABCDEF";
    uint16_t i;
    for (i = 0u; i < len; i++) {
        HAL_UART_Debug_PutChar((uint8_t)hex[data[i] >> 4u]);
        HAL_UART_Debug_PutChar((uint8_t)hex[data[i] & 0x0Fu]);
        HAL_UART_Debug_PutChar(' ');
    }
    HAL_UART_Debug_PutChar('\n');
}

/* ================================================================
 * 非阻塞基础设施 —— 环形缓冲 + ISR 驱动发送
 * ================================================================ */
#if DEBUG_PRINT_MODE == DEBUG_PRINT_MODE_NONBLOCKING

#define DEBUG_TX_BUF_MASK  (DEBUG_TX_BUF_SIZE - 1u)

static uint8_t       s_tx_buf[DEBUG_TX_BUF_SIZE];
static volatile uint16_t s_tx_head;   /* ISR 读取位置 */
static uint16_t          s_tx_tail;   /* 主循环写入位置 */

static uint8_t rb_is_empty(void)
{
    return (s_tx_head == s_tx_tail) ? 1u : 0u;
}

/* 返回 1=成功, 0=缓冲满 */
static uint8_t rb_push(uint8_t byte)
{
    uint16_t next = (s_tx_tail + 1u) & DEBUG_TX_BUF_MASK;
    if (next == s_tx_head) return 0u;
    s_tx_buf[s_tx_tail] = byte;
    s_tx_tail = next;
    return 1u;
}

/* 返回 1=成功, 0=缓冲空 */
static uint8_t rb_pop(uint8_t *byte)
{
    if (rb_is_empty()) return 0u;
    *byte = s_tx_buf[s_tx_head];
    s_tx_head = (s_tx_head + 1u) & DEBUG_TX_BUF_MASK;
    return 1u;
}

#endif /* NONBLOCKING */

/* ================================================================
 * HAL_UART_Debug_Printf —— 阻塞/非阻塞由 DEBUG_PRINT_MODE 控制
 * ================================================================ */
#if DEBUG_PRINT_MODE == DEBUG_PRINT_MODE_BLOCKING

int HAL_UART_Debug_Printf(const char *fmt, ...)
{
    static char buf[128];
    va_list args;
    int len;

    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    HAL_UART_Debug_Print(buf);
    return len;
}

#else /* NONBLOCKING */

int HAL_UART_Debug_Printf(const char *fmt, ...)
{
    char buf[128];
    va_list args;
    int len;
    int i;

    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    for (i = 0; i < len && buf[i] != '\0'; i++) {
        if (buf[i] == '\n') {
            if (!rb_push('\r')) break;
        }
        if (!rb_push((uint8_t)buf[i])) break;
    }

    /* 启动 TX 中断发送 */
    if (!rb_is_empty()) {
        UART_ITConfig(DEBUG_UART, UART_IT_EN | UART_IT_TX, ENABLE);
    }

    return i;
}

#endif /* BLOCKING / NONBLOCKING */

/* ================================================================
 * HAL_UART_Debug_Print_NB —— 非阻塞字符串输出
 * ================================================================ */
void HAL_UART_Debug_Print_NB(const char *str)
{
#if DEBUG_PRINT_MODE == DEBUG_PRINT_MODE_NONBLOCKING
    while (*str != '\0') {
        if (*str == '\n') {
            if (!rb_push('\r')) break;
        }
        if (!rb_push((uint8_t)*str)) break;
        str++;
    }
    if (!rb_is_empty()) {
        UART_ITConfig(DEBUG_UART, UART_IT_EN | UART_IT_TX, ENABLE);
    }
#else
    HAL_UART_Debug_Print(str);
#endif
}

/* ================================================================
 * HAL_UART_Debug_TxBusy
 * ================================================================ */
uint8_t HAL_UART_Debug_TxBusy(void)
{
#if DEBUG_PRINT_MODE == DEBUG_PRINT_MODE_NONBLOCKING
    return rb_is_empty() ? 0u : 1u;
#else
    return 0u;
#endif
}

/* ================================================================
 * HAL_UART_Debug_Flush —— 安全网: 确保 TX 中断不会意外关闭
 * ================================================================ */
void HAL_UART_Debug_Flush(void)
{
#if DEBUG_PRINT_MODE == DEBUG_PRINT_MODE_NONBLOCKING
    if (!rb_is_empty()) {
        UART_ITConfig(DEBUG_UART, UART_IT_EN | UART_IT_TX, ENABLE);
    }
#endif
}

/* ================================================================
 * HAL_UART_Debug_ISR —— 从环形缓冲取字节发送
 * ================================================================ */
void HAL_UART_Debug_ISR(void)
{
#if DEBUG_PRINT_MODE == DEBUG_PRINT_MODE_NONBLOCKING
    uint8_t ch;

    if (UART_GetFlagStatus(DEBUG_UART, UART_Flag_TX) == RESET) {
        return;
    }
    UART_ClearFlag(DEBUG_UART, UART_Flag_TX);

    if (rb_pop(&ch)) {
        UART_SendData(DEBUG_UART, ch);
    } else {
        /* 缓冲空 → 关闭 TX 中断，避免空中断 */
        UART_ITConfig(DEBUG_UART, UART_IT_EN | UART_IT_TX, DISABLE);
    }
#endif
}

/* ================================================================
 * HAL_UART_Debug_RX_Enable —— 使能调试串口接收
 * ================================================================ */
void HAL_UART_Debug_RX_Enable(void)
{
    UART_RXCmd(DEBUG_UART, ENABLE);
}

/* ================================================================
 * HAL_UART_Debug_GetChar —— 非阻塞读一个字节
 *   返回: 1=读到字符(存入*ch), 0=无数据
 * ================================================================ */
uint8_t HAL_UART_Debug_GetChar(uint8_t *ch)
{
    if (UART_GetFlagStatus(DEBUG_UART, UART_Flag_RX) == SET) {
        *ch = (uint8_t)UART_ReceiveData(DEBUG_UART);
        UART_ClearFlag(DEBUG_UART, UART_Flag_RX);
        return 1u;
    }
    return 0u;
}

/* ================================================================
 * UART1_3_5_7816_IRQHandler —— 共享中断入口
 * ================================================================ */
void UART1_3_5_7816_IRQHandler(void)
{
    HAL_UART_Debug_ISR();
}
