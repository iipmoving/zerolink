/**
 * hal_comm.c —— MODBUS通讯UART0 DMA实现
 *
 * 依赖: hal_comm.h + sc32f1xxx_uart.h + sc32f1xxx_dma.h
 * 层级: HAL —— 只封装硬件操作，不include msg_def.h
 *
 * 设计:
 *   RX: DMA0 不定长接收 —— 设256字节max，轮询检测帧间隔判定帧结束
 *   TX: DMA1 单次发送 —— 丢缓存即走，TC中断通知完成
 *
 * UART0: pins 48/47 (默认映射), 57600-10B, APB0 48MHz
 * 帧检测: 连续2次轮询(20ms)无新字节 → 帧结束 (>> MODBUS 3.5char=0.6ms)
 */
#include "hal_comm.h"
#include "sc32f1xxx_uart.h"
#include "sc32f1xxx_dma.h"
#include "sc32f1xxx_rcc.h"
#include "sc32L14xx.h"

/* ========== 配置常量 ========== */
#define COMM_BAUDRATE       57600u
#define COMM_CLOCK           48000000u
#define RX_DMA_BUF_SIZE     256u    /* MODBUS最大帧长                      */
#define TX_DMA_BUF_SIZE     256u    /* TX DMA缓冲                          */
#define RX_IDLE_POLLS       2u      /* 连续N次无新字节→帧结束               */

/* ========== 静态缓冲 ========== */
static uint8_t  s_rx_buf[RX_DMA_BUF_SIZE];     /* DMA RX目的地址               */
static uint8_t  s_tx_buf[TX_DMA_BUF_SIZE];     /* DMA TX源地址                 */
static uint16_t s_rx_last_cnt;                  /* 上次DMA剩余计数              */
static uint8_t  s_rx_idle_cnt;                  /* 连续无数据轮询次数            */
static uint8_t  s_rx_active;                    /* 当前是否在收帧               */
static volatile uint8_t s_tx_busy;              /* TX DMA进行中标志              */

/* ========== 内部辅助 ========== */

/* 获取DMA已接收字节数 */
static uint16_t rx_received(void)
{
    uint32_t remain;
    remain = DMA_GetCurrDataCounter(DMA0);
    if (remain > (uint32_t)RX_DMA_BUF_SIZE) {
        return 0u;  /* DMA未启动或异常 */
    }
    return (uint16_t)((uint32_t)RX_DMA_BUF_SIZE - remain);
}

/* 重置RX DMA，准备接收下一帧 */
static void rx_dma_reset(void)
{
    DMA_Cmd(DMA0, DISABLE);
    DMA_SetCurrDataCounter(DMA0, (uint32_t)RX_DMA_BUF_SIZE);
    /* 重置DMA内部状态后重新使能 */
    DMA_ChannelReset(DMA0);
    DMA_Cmd(DMA0, ENABLE);
    s_rx_last_cnt  = RX_DMA_BUF_SIZE;
    s_rx_idle_cnt  = 0u;
    s_rx_active    = 0u;
}

/* ========== 公共接口 ========== */

void HAL_Comm_Init(void)
{
    UART_InitTypeDef  uart_init;
    DMA_InitTypeDef   dma_init;

    /* ---- 1. 时钟 ---- */
    RCC_APB0Cmd(ENABLE);
    RCC_APB0PeriphClockCmd(RCC_APB0Periph_UART0, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA, ENABLE);   /* 所有DMA通道共用 */

    /* ---- 2. UART0: 57600-10B (pin48/47 硬件默认功能，无需GPIO配置) ---- */
    uart_init.UART_ClockFrequency = COMM_CLOCK;
    uart_init.UART_BaudRate       = COMM_BAUDRATE;
    uart_init.UART_Mode           = UART_Mode_10B;
    UART_Init(UART0, &uart_init);

    /* 使能DMA RX请求，TX请求在发送时使能 */
    UART_DMACmd(UART0, UART_DMAReq_RX, ENABLE);
    /* 使能UART收发 */
    UART_TXCmd(UART0, ENABLE);
    UART_RXCmd(UART0, ENABLE);

    /* ---- 3. DMA0: RX (外设→内存, 不定长) ---- */
    DMA_StructInit(&dma_init);
    dma_init.DMA_Priority     = DMA_Priority_HIGH;
    dma_init.DMA_CircularMode = DMA_CircularMode_Disable;  /* 单次模式 */
    dma_init.DMA_DataSize     = DMA_DataSize_Byte;
    dma_init.DMA_SourceMode   = DMA_SourceMode_FIXED;       /* UART DATA寄存器不变 */
    dma_init.DMA_TargetMode   = DMA_TargetMode_INC;         /* RAM地址递增 */
    dma_init.DMA_Burst        = DMA_Burst_Disable;
    dma_init.DMA_BufferSize   = (uint32_t)RX_DMA_BUF_SIZE;
    dma_init.DMA_Request      = DMA_Request_UART0_RX;
    dma_init.DMA_SrcAddress   = (uint32_t)&(UART0->UART_DATA);
    dma_init.DMA_DstAddress   = (uint32_t)s_rx_buf;
    DMA_Init(DMA0, &dma_init);
    DMA_Cmd(DMA0, ENABLE);

    /* ---- 4. DMA1: TX (内存→外设, 单次) ---- */
    /* 仅做初始化，不启动；发送时重新配置参数 */
    DMA_StructInit(&dma_init);
    dma_init.DMA_Priority     = DMA_Priority_HIGH;
    dma_init.DMA_CircularMode = DMA_CircularMode_Disable;
    dma_init.DMA_DataSize     = DMA_DataSize_Byte;
    dma_init.DMA_SourceMode   = DMA_SourceMode_INC;          /* RAM地址递增 */
    dma_init.DMA_TargetMode   = DMA_TargetMode_FIXED;        /* UART DATA寄存器不变 */
    dma_init.DMA_Burst        = DMA_Burst_Disable;
    dma_init.DMA_BufferSize   = 0u;                          /* 发送时设置 */
    dma_init.DMA_Request      = DMA_Request_UART0_TX;
    dma_init.DMA_SrcAddress   = (uint32_t)s_tx_buf;
    dma_init.DMA_DstAddress   = (uint32_t)&(UART0->UART_DATA);
    DMA_Init(DMA1, &dma_init);

    /* 使能DMA1 TC中断(发送完成) */
    DMA_ITConfig(DMA1, DMA_IT_TCIE, ENABLE);
    NVIC_EnableIRQ(DMA1_IRQn);

    /* ---- 5. 状态初始化 ---- */
    s_rx_last_cnt  = RX_DMA_BUF_SIZE;
    s_rx_idle_cnt  = 0u;
    s_rx_active    = 0u;
    s_tx_busy      = 0u;
}

uint8_t HAL_Comm_Send(const uint8_t *data, uint16_t len)
{
    uint16_t i;

    if (s_tx_busy != 0u || len == 0u || len > TX_DMA_BUF_SIZE) {
        return 1u;  /* 忙或参数无效 */
    }

    /* 拷贝数据到TX DMA缓冲 */
    for (i = 0u; i < len; i++) {
        s_tx_buf[i] = data[i];
    }

    /* 配置DMA1并启动 */
    DMA_Cmd(DMA1, DISABLE);
    DMA_SetCurrDataCounter(DMA1, (uint32_t)len);
    DMA_SetSrcAddress(DMA1, (uint32_t)s_tx_buf);
    DMA_ChannelReset(DMA1);
    s_tx_busy = 1u;

    /* 使能UART TX DMA请求 */
    UART_DMACmd(UART0, UART_DMAReq_TX, ENABLE);
    DMA_Cmd(DMA1, ENABLE);

    return 0u;
}

uint16_t HAL_Comm_Available(void)
{
    return rx_received();
}

uint16_t HAL_Comm_Read(uint8_t *buf, uint16_t max_len)
{
    uint16_t avail;
    uint16_t i;

    avail = rx_received();
    if (avail > max_len) {
        avail = max_len;
    }
    for (i = 0u; i < avail; i++) {
        buf[i] = s_rx_buf[i];
    }
    return avail;
}

void HAL_Comm_Flush(void)
{
    rx_dma_reset();
}

uint8_t HAL_Comm_TxDone(void)
{
    return (s_tx_busy == 0u) ? 1u : 0u;
}

/* ========== 轮询: 帧检测 + 帧结束判定 ========== */

/**
 * HAL_Comm_RecvPoll —— 每10ms槽位调用一次
 *
 * 用DMA计数器变化检测帧边界:
 *   连续 RX_IDLE_POLLS 次无新字节 → 帧结束
 *   返回: 帧长度(>0=完整帧), 0=无帧或收帧中
 *
 * 调用者拿到长度后应立即 HAL_Comm_Read() 取走数据,
 * 取走后调用 HAL_Comm_Flush() 重置DMA准备下一帧。
 */
uint16_t HAL_Comm_RecvPoll(void)
{
    uint16_t received;

    received = rx_received();

    if (received == 0u) {
        /* 无数据 */
        s_rx_active = 0u;
        return 0u;
    }

    if (!s_rx_active) {
        /* 首次收到数据 —— 开始新帧 */
        s_rx_active   = 1u;
        s_rx_last_cnt = received;
        s_rx_idle_cnt = 0u;
        return 0u;  /* 帧刚开始，未完 */
    }

    if (received == s_rx_last_cnt) {
        /* DMA计数无变化 —— 无新字节 */
        s_rx_idle_cnt++;
        if (s_rx_idle_cnt >= RX_IDLE_POLLS) {
            /* 帧结束 */
            return received;
        }
        return 0u;  /* 还在等帧结束 */
    }

    /* 有新字节到达 —— 更新计数，继续等待 */
    s_rx_last_cnt = received;
    s_rx_idle_cnt = 0u;

    /* 缓冲区满保护：若收到256字节，直接算帧结束 */
    if (received >= RX_DMA_BUF_SIZE) {
        return received;
    }

    return 0u;
}

/* ========== ISR入口 ========== */

/**
 * HAL_Comm_DMA_ISR —— DMA1 TX完成中断 (DMA1_IRQn)
 */
void HAL_Comm_DMA_ISR(void)
{
    if (DMA_GetFlagStatus(DMA1, DMA_FLAG_TCIF) == SET) {
        DMA_ClearFlag(DMA1, DMA_FLAG_TCIF);
        DMA_Cmd(DMA1, DISABLE);
        UART_DMACmd(UART0, UART_DMAReq_TX, DISABLE);
        s_tx_busy = 0u;
    }
    /* 清除其他标志(HT, TE) */
    if (DMA_GetFlagStatus(DMA1, DMA_FLAG_HTIF) == SET) {
        DMA_ClearFlag(DMA1, DMA_FLAG_HTIF);
    }
    if (DMA_GetFlagStatus(DMA1, DMA_FLAG_TEIF) == SET) {
        DMA_ClearFlag(DMA1, DMA_FLAG_TEIF);
        DMA_Cmd(DMA1, DISABLE);
        UART_DMACmd(UART0, UART_DMAReq_TX, DISABLE);
        s_tx_busy = 0u;
    }
}

/**
 * HAL_Comm_UART_ISR —— UART0中断 (UART0_2_4_IRQn)
 *
 * 当前DMA模式下，UART中断主要用于异常恢复。
 * 正常收发由DMA处理。
 */
void HAL_Comm_UART_ISR(void)
{
    /* RX溢出/帧错误等 —— 清除标志，不阻塞 */
    if (UART_GetFlagStatus(UART0, UART_Flag_RX) == SET) {
        UART_ClearFlag(UART0, UART_Flag_RX);
    }
    if (UART_GetFlagStatus(UART0, UART_Flag_TX) == SET) {
        UART_ClearFlag(UART0, UART_Flag_TX);
    }
}

/* ========== 中断向量入口（由启动文件调用）========== */

void UART0_2_4_IRQHandler(void)
{
    HAL_Comm_UART_ISR();
}

void DMA1_IRQHandler(void)
{
    HAL_Comm_DMA_ISR();
}
