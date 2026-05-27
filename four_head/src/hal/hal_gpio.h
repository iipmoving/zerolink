/**
 * hal_gpio.h —— GPIO抽象接口
 *
 * 依赖: sc32f1xxx_gpio.h（固件库）
 * 层级: HAL —— 只封装硬件操作，不include msg_def.h
 *
 * 所有IO操作通过此层，业务模块不直接调用固件库GPIO函数
 */
#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>

/* ========== IO功能枚举 ========== */
typedef enum {
    HAL_IO_BUZZER,       /* 蜂鸣器 */
    HAL_IO_HEATER1,      /* 炉头1加热使能 */
    HAL_IO_HEATER2,      /* 炉头2加热使能 */
    HAL_IO_HEATER3,      /* 炉头3加热使能 */
    HAL_IO_HEATER4,      /* 炉头4加热使能 */
    HAL_IO_FAN1,         /* 风机1 */
    HAL_IO_FAN2,         /* 风机2 */
    HAL_IO_COUNT
} HAL_IO_t;

/* 初始化所有GPIO */
void HAL_GPIO_Init(void);

/* 设置IO输出电平: 1=高, 0=低 */
void HAL_GPIO_Write(HAL_IO_t io, uint8_t level);

/* 翻转IO输出 */
void HAL_GPIO_Toggle(HAL_IO_t io);

/* 读取IO输入电平 */
uint8_t HAL_GPIO_Read(HAL_IO_t io);

#endif /* HAL_GPIO_H */
