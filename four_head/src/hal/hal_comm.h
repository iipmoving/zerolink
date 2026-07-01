/**
 * hal_comm.h —— MODBUS通讯UART0接口 (DMA RX + DMA TX)
 *
 * 依赖: sc32f1xxx_uart.h + sc32f1xxx_dma.h (固件库)
 * 层级: HAL —— 只封装硬件操作，不include msg_def.h
 *
 * UART0: pins 48/47, 57600-10B, APB0 48MHz
 * RX:  DMA0 不定长 —— 256字节缓冲，轮询帧间隔检测帧结束
 * TX:  DMA1 单次发送 —— 丢缓存即走，TC中断通知完成
 *
 * 使用方式:
 *   1. HAL_Comm_Init() 初始化
 *   2. 每10ms槽位调用 HAL_Comm_RecvPoll()，返回值>0=帧就绪
 *   3. HAL_Comm_Read() 取走帧数据，HAL_Comm_Flush() 重置DMA
 *   4. HAL_Comm_Send() 发送数据（非阻塞，DMA后台发）
 *   5. ISR中调用 HAL_Comm_UART_ISR() + HAL_Comm_DMA_ISR()
 */
#ifndef HAL_COMM_H
#define HAL_COMM_H

#include <stdint.h>

/* ========== 公共接口 ========== */

/* 初始化UART0 + DMA0 RX + DMA1 TX */
void HAL_Comm_Init(void);

/* 发送数据（非阻塞，拷贝到DMA缓冲后立即返回）
 * 返回: 0=成功, 1=忙或参数无效 */
uint8_t HAL_Comm_Send(const uint8_t *data, uint16_t len);

/* 查询RX缓冲中已收字节数（帧未结束时也返回） */
uint16_t HAL_Comm_Available(void);

/* 从RX缓冲读取字节，返回实际读取数 */
uint16_t HAL_Comm_Read(uint8_t *buf, uint16_t max_len);

/* 重置RX DMA，丢弃当前帧 */
void HAL_Comm_Flush(void);

/* 查询TX DMA是否空闲 */
uint8_t HAL_Comm_TxDone(void);

/* 轮询帧检测: 返回>0=完整帧长度, 0=无帧或收帧中
 * 每10ms调用一次，内部通过DMA计数停滞判定帧结束 */
uint16_t HAL_Comm_RecvPoll(void);

/* ========== ISR入口（需在中断向量表中调用）========== */

/* UART0中断处理 (UART0_2_4_IRQn) —— 异常恢复 */
void HAL_Comm_UART_ISR(void);

/* DMA1中断处理 (DMA1_IRQn) —— TX完成 */
void HAL_Comm_DMA_ISR(void);

#endif /* HAL_COMM_H */
