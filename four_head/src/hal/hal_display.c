/**
 * hal_display.c —— 8 SEG + 11 COM 共阴数码管硬件驱动
 *
 * 依赖: hal_display.h + sc32f1xxx_gpio.h + sc32f1xxx_rcc.h
 *
 * SEG (段选, 8线): PC8(A) PB14(B) PE9(C) PC4(D) PD13(E) PB15(F) PE8(G) PC1(H)
 * COM (位选, 11线): PE11(COM1) PE12(COM2) PA10(COM3) PE10(COM4)
 *                    PA6(COM5) PA1(COM6) PA0(COM7) PA7(COM8)
 *                    PC0(COM9) PA5(COM10) PA4(COM11)
 *
 * 扫描方式: COM低电平有效，SEG高电平点亮段
 */
#include "hal_display.h"
#include "sc32f1xxx_gpio.h"
#include "sc32f1xxx_rcc.h"

/* ========== SEG 引脚表 (8个) ========== */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
} SegPin_t;

static const SegPin_t s_seg_pins[DISP_SM8_COUNT] = {
    { GPIOC, GPIO_Pin_8  },   /* SEG A */
    { GPIOB, GPIO_Pin_14 },   /* SEG B */
    { GPIOE, GPIO_Pin_9  },   /* SEG C */
    { GPIOC, GPIO_Pin_4  },   /* SEG D */
    { GPIOD, GPIO_Pin_13 },   /* SEG E */
    { GPIOB, GPIO_Pin_15 },   /* SEG F */
    { GPIOE, GPIO_Pin_8  },   /* SEG G */
    { GPIOC, GPIO_Pin_1  },   /* SEG H (小数点) */
};

/* ========== COM 引脚表 (11个) ========== */
static const SegPin_t s_com_pins[DISP_COM_COUNT] = {
    { GPIOE, GPIO_Pin_11 },   /* COM1  → IO[0] 炉头0 SMG bit0 */
    { GPIOE, GPIO_Pin_12 },   /* COM2  → IO[1] 炉头0 SMG bit1 */
    { GPIOA, GPIO_Pin_10 },   /* COM3  → IO[2] 炉头1 SMG bit0 */
    { GPIOE, GPIO_Pin_10 },   /* COM4  → IO[3] 炉头1 SMG bit1 */
    { GPIOA, GPIO_Pin_6  },   /* COM5  → IO[4] 炉头2 SMG bit0 */
    { GPIOA, GPIO_Pin_1  },   /* COM6  → IO[5] 炉头2 SMG bit1 */
    { GPIOA, GPIO_Pin_0  },   /* COM7  → IO[6] 炉头3 SMG bit0 */
    { GPIOA, GPIO_Pin_7  },   /* COM8  → IO[7] 炉头3 SMG bit1 */
    { GPIOC, GPIO_Pin_0  },   /* COM9  → IO[8] LED组0 */
    { GPIOA, GPIO_Pin_5  },   /* COM10 → IO[9] LED组1 */
    { GPIOA, GPIO_Pin_4  },   /* COM11 → IO[10] LED组2 */
};

/* ========== 初始化 ========== */
void HAL_Display_Init(void)
{
    GPIO_InitTypeDef gpio;
    uint8_t i;

    /* 时钟已在 SystemInit 中使能，此处不重复 */

    /* SEG 全部推挽输出，默认低(灭) */
    gpio.GPIO_Mode       = GPIO_Mode_OUT_PP;
    gpio.GPIO_DriveLevel = GPIO_DriveLevel_0;

    for (i = 0u; i < DISP_SM8_COUNT; i++) {
        gpio.GPIO_Pin = s_seg_pins[i].pin;
        GPIO_Init(s_seg_pins[i].port, &gpio);
        GPIO_ResetBits(s_seg_pins[i].port, s_seg_pins[i].pin);
    }

    /* COM 全部推挽输出，默认高(不选通) */
    for (i = 0u; i < DISP_COM_COUNT; i++) {
        gpio.GPIO_Pin = s_com_pins[i].pin;
        GPIO_Init(s_com_pins[i].port, &gpio);
        GPIO_SetBits(s_com_pins[i].port, s_com_pins[i].pin);
    }
}

/* ========== 扫描一个COM位 ========== */
void HAL_Display_Scan(uint8_t com_idx, uint8_t seg_data, uint8_t prev_com)
{
    uint8_t i;

    if (com_idx >= DISP_COM_COUNT) return;

    /* 1. 关上一COM (只关一个，其它本来就是高) */
    if (prev_com < DISP_COM_COUNT) {
        GPIO_SetBits(s_com_pins[prev_com].port, s_com_pins[prev_com].pin);
    }

    /* 2. 清SEG → 消隐 */
    for (i = 0u; i < DISP_SM8_COUNT; i++) {
        GPIO_ResetBits(s_seg_pins[i].port, s_seg_pins[i].pin);
    }

    /* 3. 输出新段码 */
    for (i = 0u; i < DISP_SM8_COUNT; i++) {
        if (seg_data & (1u << i)) {
            GPIO_SetBits(s_seg_pins[i].port, s_seg_pins[i].pin);
        }
    }

    /* 4. 选通新COM (低有效) */
    GPIO_ResetBits(s_com_pins[com_idx].port, s_com_pins[com_idx].pin);
}
