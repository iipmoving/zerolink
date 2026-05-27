/**
 * drv_comm.h — MODBUS通讯驱动层接口
 *
 * 封装 HAL 层 hal_comm + hal_uart, 是 APP 层访问通讯硬件的唯一合法入口。
 * 依赖: hal/hal_comm.h + hal/hal_uart.h (固件库)
 * 层级: DRV — 唯一可调用 HAL 的层
 *
 * 使用方式:
 *   1. Drv_Comm_Init() 初始化
 *   2. 每10ms槽位调用 Drv_Comm_RecvPoll(), 返回值>0=帧就绪
 *   3. Drv_Comm_Read() 取走帧数据, Drv_Comm_Flush() 重置DMA
 *   4. Drv_Comm_Send() 发送数据 (非阻塞, DMA后台发)
 *   5. ISR中调用 Drv_Comm_UART_ISR() + Drv_Comm_DMA_ISR()
 */
#ifndef DRV_COMM_H
#define DRV_COMM_H

#include <stdint.h>

/* ========== MODBUS 通讯接口 (封装 hal_comm) ========== */

void     Drv_Comm_Init(void);
uint8_t  Drv_Comm_Send(const uint8_t *data, uint16_t len);
uint16_t Drv_Comm_Available(void);
uint16_t Drv_Comm_Read(uint8_t *buf, uint16_t max_len);
void     Drv_Comm_Flush(void);
uint8_t  Drv_Comm_TxDone(void);
uint16_t Drv_Comm_RecvPoll(void);

/* ISR 入口 */
void Drv_Comm_UART_ISR(void);
void Drv_Comm_DMA_ISR(void);

/* ========== 调试串口接口 (封装 hal_uart) ========== */

void Drv_Comm_Debug_Init(void);
void Drv_Comm_Debug_PutChar(uint8_t ch);
void Drv_Comm_Debug_Print(const char *str);
void Drv_Comm_Debug_HexDump(const uint8_t *data, uint16_t len);
int  Drv_Comm_Debug_Printf(const char *fmt, ...);
void Drv_Comm_Debug_Print_NB(const char *str);
uint8_t Drv_Comm_Debug_TxBusy(void);
void Drv_Comm_Debug_Flush(void);
void Drv_Comm_Debug_ISR(void);

#endif /* DRV_COMM_H */
