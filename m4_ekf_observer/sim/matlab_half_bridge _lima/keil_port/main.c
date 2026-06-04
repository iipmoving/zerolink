/******************************************************************************
 * main.c — 半桥感应加热参数计算 (RX32G410 示例)
 *
 * MCU: 中山翰林电器 RX32G410
 *   内核: Cortex-M4F @192MHz (零等待)
 *   硬件: HRTIM ×1 (162ps, 6×16b), ADC ×3 (12b, 4Msps)
 *   加速: CORDIC (三角函数), FMAC (滤波器)
 *
 * 本例演示:
 *   1. HRTIM 同步触发 ADC 注入组采样 → DMA → RawBuffer_t
 *   2. IH_Calculate() → 全部参数计算
 *   3. UART 输出 / LCD 显示
 *
 * 外设驱动 (需芯片厂商 HAL 库):
 *   - rx32g410_hal_hrtim.c/h
 *   - rx32g410_hal_adc.c/h
 *   - rx32g410_hal_uart.c/h
 *   - rx32g410_hal_cordic.c/h   (CORDIC 驱动)
 *
 * Keil MDK 配置:
 *   Device: RX32G410
 *   FPU:    Single Precision (VFPv4) — 必须
 *   AC6:    -O2 -ffast-math -mcpu=cortex-m4 -mfpu=fpv4-sp-d16
 *   Include: 芯片厂商 Keil Pack
 ******************************************************************************/

#include <string.h>
#include <stdio.h>
#include "ih_calculate.h"
#include "ih_params.h"

/* ====================== 缓冲区 ====================== */

#define MAX_POINTS      2048

static RawPoint_t   g_buf[MAX_POINTS];
static RawBuffer_t  g_raw;
static IH_Result_t  g_res;
static char         g_tx[1024];

/* ====================== 外设声明 ====================== */

/* 由芯片厂商 HAL 库提供 (此处仅声明) */
extern void     HAL_Init(void);
extern void     SystemClock_Config(void);       /* 192MHz */
extern void     MX_HRTIM1_Init(void);           /* HRTIM 初始化 */
extern void     MX_ADC1_Init(void);             /* ADC 注入组 */
extern void     MX_CORDIC_Init(void);           /* CORDIC 初始化 */
extern void     MX_USART1_Init(uint32_t baud);  /* 串口 115200 */
extern uint32_t HAL_GetTick(void);

/* DMA 接收完成回调 (在中断中调用) */
static volatile uint32_t g_frame_ready = 0;
static volatile uint32_t g_frame_count = 0;

void DMA_ADC_Complete_Callback(void)
{
    g_frame_ready = 1;
    g_frame_count++;
}

/* ====================== 主函数 ====================== */

int main(void)
{
    /* --- 硬件初始化 --- */
    HAL_Init();
    SystemClock_Config();           /* 192MHz */
    MX_CORDIC_Init();               /* CORDIC 协处理器 */
    MX_USART1_Init(115200);         /* 串口 115200 8N1 */
    MX_ADC1_Init();                 /* ADC1 注入组, 4Msps */
    MX_HRTIM1_Init();               /* HRTIM1, 162ps 分辨率 */

    /* --- 配置 RawBuffer_t --- */
    g_raw.points       = g_buf;
    g_raw.count        = 0;
    g_raw.n_cols       = 9;
    g_raw.t_per_cnt_us = T_PER_CNT_US;  /* 162ps → 1.62e-4 μs/cnt */
    g_raw.hrtim_clk_mhz = 1.0f / (T_PER_CNT_US * 1e-6f) / 1e6f; /* ≈6170 MHz */
    g_raw.t_span_ms    = T_AC_MS;       /* 20ms 一帧 */

    /* --- 启动 HRTIM + ADC 同步采集 --- */
    /* HRTIM CNT 递增, 在特定计数值触发 ADC 注入组采样 */
    /* ADC 完成 → DMA → g_buf[] */
    /* 每帧点数由 HRTIM 周期和 ADC 采样率决定 */
    HAL_HRTIM_Start(&hhrtim1, HRTIM_TIMER_A);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)g_buf, MAX_POINTS * 9);

    sprintf(g_tx, "=== RX32G410 半桥IH参数计算 ===\r\n");
    USART_SendString(g_tx);

    /* --- 主循环 --- */
    while (1)
    {
        if (g_frame_ready)
        {
            g_frame_ready = 0;

            /* 更新帧信息 */
            g_raw.count = get_dma_received_count();
            if (g_raw.count > MAX_POINTS) g_raw.count = MAX_POINTS;

            if (g_raw.count < 10) continue;

            float t0_ms = g_buf[0].t_us / 1000.0f;
            float t1_ms = g_buf[g_raw.count - 1].t_us / 1000.0f;
            g_raw.t_span_ms = t1_ms - t0_ms;

            /* 执行计算 */
            uint32_t t_start = HAL_GetTick();
            int32_t ret = IH_Calculate(&g_raw, &g_res);
            uint32_t t_cost = HAL_GetTick() - t_start;

            if (ret == 0)
            {
                /* 串口输出 */
                int len = IH_ResultToString(&g_res, g_tx, sizeof(g_tx));
                USART_SendString(g_tx);

                snprintf(g_tx, sizeof(g_tx),
                         "帧#%lu %lu点 %lums\r\n",
                         g_frame_count, g_raw.count, t_cost);
                USART_SendString(g_tx);
            }

            /* 重新启动 DMA (单次模式需重启) */
            HAL_ADC_Start_DMA(&hadc1, (uint32_t *)g_buf, MAX_POINTS * 9);
        }
    }
}

/* ====================== 系统时钟 (示例) ====================== */

void SystemClock_Config(void)
{
    /* RX32G410: HSE 8MHz → PLL → 192MHz SYSCLK */
    /* 实际使用芯片厂商 CubeMX 生成 */
    /* RCC_OscInitTypeDef / RCC_ClkInitTypeDef */
}

/* ====================== 串口包装 ====================== */

void USART_SendString(const char *s)
{
    /* HAL_UART_Transit(&husart1, (uint8_t*)s, strlen(s), 1000); */
    (void)s;
}

uint32_t get_dma_received_count(void)
{
    /* 返回 DMA 已接收的半字数 */
    return 0;   /* 替换为: DMA_GetCount(&hdma_adc1) / 9 */
}
