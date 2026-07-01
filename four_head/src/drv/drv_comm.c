/**
 * drv_comm.c — MODBUS通讯驱动层实现
 *
 * 封装 HAL 层 hal_comm + hal_uart，是 APP 层访问通讯硬件的唯一合法入口。
 * 依赖: drv_comm.h + hal_comm.h + hal_uart.h
 * 层级: DRV — 唯一可调用 HAL 的层
 */
#include "drv_comm.h"
#include "../hal/hal_comm.h"
#include "../hal/hal_uart.h"
#include <stdarg.h>
#include <stdio.h>

/* ========== MODBUS 通讯接口（封装 hal_comm）========== */

void Drv_Comm_Init(void)
{
    HAL_Comm_Init();
}

uint8_t Drv_Comm_Send(const uint8_t *data, uint16_t len)
{
    return HAL_Comm_Send(data, len);
}

uint16_t Drv_Comm_Available(void)
{
    return HAL_Comm_Available();
}

uint16_t Drv_Comm_Read(uint8_t *buf, uint16_t max_len)
{
    return HAL_Comm_Read(buf, max_len);
}

void Drv_Comm_Flush(void)
{
    HAL_Comm_Flush();
}

uint8_t Drv_Comm_TxDone(void)
{
    return HAL_Comm_TxDone();
}

uint16_t Drv_Comm_RecvPoll(void)
{
    return HAL_Comm_RecvPoll();
}

/* ISR 入口 — 透传 HAL ISR */
void Drv_Comm_UART_ISR(void)
{
    HAL_Comm_UART_ISR();
}

void Drv_Comm_DMA_ISR(void)
{
    HAL_Comm_DMA_ISR();
}

/* ========== 调试串口接口（封装 hal_uart）========== */

void Drv_Comm_Debug_Init(void)
{
    HAL_UART_Debug_Init();
}

void Drv_Comm_Debug_PutChar(uint8_t ch)
{
    HAL_UART_Debug_PutChar(ch);
}

void Drv_Comm_Debug_Print(const char *str)
{
    HAL_UART_Debug_Print(str);
}

void Drv_Comm_Debug_HexDump(const uint8_t *data, uint16_t len)
{
    HAL_UART_Debug_HexDump(data, len);
}

int Drv_Comm_Debug_Printf(const char *fmt, ...)
{
    char buf[128];
    int ret;
    va_list args;
    va_start(args, fmt);
    ret = vsprintf(buf, fmt, args);
    va_end(args);
    if (ret > 0) {
        HAL_UART_Debug_Print(buf);
    }
    return ret;
}

void Drv_Comm_Debug_Print_NB(const char *str)
{
    HAL_UART_Debug_Print_NB(str);
}

uint8_t Drv_Comm_Debug_TxBusy(void)
{
    return HAL_UART_Debug_TxBusy();
}

void Drv_Comm_Debug_Flush(void)
{
    HAL_UART_Debug_Flush();
}

void Drv_Comm_Debug_ISR(void)
{
    HAL_UART_Debug_ISR();
}
