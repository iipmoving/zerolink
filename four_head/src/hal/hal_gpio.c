/**
 * hal_gpio.c —— GPIO抽象实现
 *
 * 依赖: hal_gpio.h + sc32f1xxx_gpio.h
 *
 * 引脚映射表（与 CLAUDE.md 硬件配置一致）:
 *   蜂鸣器:  PE1 (TIM1 PWM)
 *   加热1-4、风机1-2: 参考程序注释掉，实物未接，此处保留枚举但禁用初始化
 */
#include "hal_gpio.h"
#include "sc32f1xxx_gpio.h"

/* ========== 引脚映射 ========== */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
} IOMap_t;

static const IOMap_t s_io_map[HAL_IO_COUNT] = {
    [HAL_IO_BUZZER]  = { GPIOE, GPIO_Pin_1 },  /* 由hal_buzzer重配置，此处仅占位 */
#if 0  /* 加热/风扇 IO 实物未接，且与显示 COM 口冲突(PA1/4/5/6) */
    [HAL_IO_HEATER1] = { GPIOA, GPIO_Pin_1 },
    [HAL_IO_HEATER2] = { GPIOA, GPIO_Pin_2 },
    [HAL_IO_HEATER3] = { GPIOA, GPIO_Pin_3 },
    [HAL_IO_HEATER4] = { GPIOA, GPIO_Pin_4 },
    [HAL_IO_FAN1]    = { GPIOA, GPIO_Pin_5 },
    [HAL_IO_FAN2]    = { GPIOA, GPIO_Pin_6 },
#endif
};

void HAL_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio_init;
    uint8_t i;

    /* 所有IO初始化为推挽输出，默认低电平 */
    gpio_init.GPIO_Mode       = GPIO_Mode_OUT_PP;
    gpio_init.GPIO_DriveLevel = GPIO_DriveLevel_0;

    for (i = 0u; i < HAL_IO_COUNT; i++) {
        if (s_io_map[i].pin == 0u) continue;   /* 跳过未启用的IO */
        gpio_init.GPIO_Pin = s_io_map[i].pin;
        GPIO_Init(s_io_map[i].port, &gpio_init);
        GPIO_ResetBits(s_io_map[i].port, s_io_map[i].pin);
    }
}

void HAL_GPIO_Write(HAL_IO_t io, uint8_t level)
{
    if (io >= HAL_IO_COUNT || s_io_map[io].pin == 0u) return;
    GPIO_WriteBit(s_io_map[io].port, s_io_map[io].pin,
                  level ? Bit_SET : Bit_RESET);
}

void HAL_GPIO_Toggle(HAL_IO_t io)
{
    if (io >= HAL_IO_COUNT || s_io_map[io].pin == 0u) return;
    GPIO_TogglePins(s_io_map[io].port, s_io_map[io].pin);
}

uint8_t HAL_GPIO_Read(HAL_IO_t io)
{
    if (io >= HAL_IO_COUNT || s_io_map[io].pin == 0u) return 0u;
    return (GPIO_ReadDataBit(s_io_map[io].port, s_io_map[io].pin) == Bit_SET) ? 1u : 0u;
}
