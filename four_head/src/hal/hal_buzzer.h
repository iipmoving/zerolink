/**
 * hal_buzzer.h —— 蜂鸣器HAL接口（TIM1 PWM + GPIOE.1/4）
 *
 * 依赖: sc32f1xxx_tim.h + sc32f1xxx_gpio.h + sc32f1xxx_rcc.h
 * 层级: HAL —— 纯硬件操作，不include业务头文件
 *
 * 引脚:
 *   PE1 = 蜂鸣器信号 (TIM1 PWM 频率驱动, 推挽输出↔高阻输入实现尾音)
 *   PE4 = 蜂鸣器电源控制
 *
 * APB0 时钟: 48MHz (HCLK Div1)
 * TIM1 频率公式: TIM_Preload = 65535 - (48000000 / PWM_Hz)
 *   中断频率 = PWM_Hz,  输出方波 = PWM_Hz / 2 (ISR中Toggle一次=半个周期)
 */
#ifndef HAL_BUZZER_H
#define HAL_BUZZER_H

#include <stdint.h>
#include "sc32f1xxx_gpio.h"
#include "sc32f1xxx_tim.h"
#include "sc32f1xxx_rcc.h"

/* ========== 条件编译: 是否启用和弦美音模式 ========== */
#define HAL_BUZZ_USE_MY

/* ========== GPIO 快速操作宏（ISR中高频调用） ========== */

/* 信号引脚 PE1 */
#define HAL_BUZZ_SIG(value)   GPIO_WriteBit(GPIOE, GPIO_Pin_1, (value) ? Bit_SET : Bit_RESET)
#define HAL_BUZZ_SIG_TOGGLE   GPIO_TogglePins(GPIOE, GPIO_Pin_1)

#define HAL_BUZZ_SIG_OUT      do { \
    GPIO_InitTypeDef _init; \
    _init.GPIO_Pin = GPIO_Pin_1; \
    _init.GPIO_Mode = GPIO_Mode_OUT_PP; \
    _init.GPIO_DriveLevel = GPIO_DriveLevel_0; \
    GPIO_Init(GPIOE, &_init); \
} while(0)

#define HAL_BUZZ_SIG_IN       do { \
    GPIO_InitTypeDef _init; \
    _init.GPIO_Pin = GPIO_Pin_1; \
    _init.GPIO_Mode = GPIO_Mode_IN_HI; \
    _init.GPIO_DriveLevel = GPIO_DriveLevel_0; \
    GPIO_Init(GPIOE, &_init); \
} while(0)

#define HAL_BUZZ_SIG_OFF      do { HAL_BUZZ_SIG_IN; HAL_BUZZ_SIG(0); } while(0)

/* 电源引脚 PE4 */
#define HAL_BUZZ_PWR(value)   GPIO_WriteBit(GPIOE, GPIO_Pin_4, (value) ? Bit_SET : Bit_RESET)

#define HAL_BUZZ_PWR_OUT      do { \
    GPIO_InitTypeDef _init; \
    _init.GPIO_Pin = GPIO_Pin_4; \
    _init.GPIO_Mode = GPIO_Mode_OUT_PP; \
    _init.GPIO_DriveLevel = GPIO_DriveLevel_0; \
    GPIO_Init(GPIOE, &_init); \
} while(0)

/* 全关 */
#define HAL_BUZZ_ALL_OFF      do { HAL_BUZZ_SIG(0); HAL_BUZZ_PWR(0); } while(0)

/* ========== TIM1 PWM 控制宏 ========== */

/* 设置TIM1中断频率 (APB0=48MHz) */
#define HAL_BUZZ_PWM_SET(pwm_hz)  do { \
    TIM_TimeBaseInitTypeDef _tinit; \
    _tinit.TIM_CounterMode = TIM_CounterMode_Up; \
    _tinit.TIM_EXENX = TIM_EXENX_Disable; \
    _tinit.TIM_Preload = 65535u - (48000000u / (uint32_t)(pwm_hz)); \
    _tinit.TIM_Prescaler = TIM_PRESCALER_1; \
    _tinit.TIM_WorkMode = TIM_WorkMode_Timer; \
    TIM_TIMBaseInit(TIM1, &_tinit); \
} while(0)

#define HAL_BUZZ_PWM_DEFAULT()  HAL_BUZZ_PWM_SET(2000u)  /* ~1000Hz输出 */

/* TIM1中断开关 */
#define HAL_BUZZ_INT_ON()     do { \
    TIM_ITConfig(TIM1, TIM_IT_TI | TIM_IT_INTEN, ENABLE); \
    NVIC_EnableIRQ(TIMER1_IRQn); \
} while(0)

#define HAL_BUZZ_INT_OFF()    do { \
    TIM_ITConfig(TIM1, TIM_IT_TI | TIM_IT_INTEN, DISABLE); \
    NVIC_DisableIRQ(TIMER1_IRQn); \
    HAL_BUZZ_PWR(0); \
} while(0)

/* ========== 公共接口 ========== */

void HAL_Buzzer_Init(void);

/* ISR频率驱动: 根据全局状态Toggle GPIO（由TIMER1_IRQHandler调用） */
void HAL_Buzzer_ISR_Drive(void);

/* 中断同步: 有声音开中断, 静音关中断 */
void HAL_Buzzer_IntSync(void);

/* ========== ISR共享状态（仅2个，ISR高频读取；DRV通过setter写入） ========== */

extern uint8_t g_buzz_hz_timer;    /* 鸣叫剩余时间(×10ms), ISR读 */
extern uint8_t g_buzz_my_active;   /* 美声激活标志, ISR读 */

/* DRV层通过以下API读写ISR共享变量（不直接操作extern） */
void    HAL_Buzzer_SetTimer(uint8_t val);
uint8_t HAL_Buzzer_GetTimer(void);
void    HAL_Buzzer_SetActive(uint8_t val);

#endif /* HAL_BUZZER_H */
