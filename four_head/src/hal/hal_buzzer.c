/**
 * hal_buzzer.c —— 蜂鸣器HAL实现（TIM1 PWM + GPIOE.1/4）
 *
 * 依赖: hal_buzzer.h
 * 层级: HAL —— ISR频率驱动 + 硬件操作
 *
 * 尾音原理:
 *   蜂鸣器驱动电路在信号引脚与地之间串有电容。
 *   鸣叫时: PE1推挽输出, Toggle产生方波, 通过电容驱动蜂鸣器。
 *   到时后: PE1切高阻输入(HI), 电容存储的电荷通过蜂鸣器自身负载
 *           电阻自然放电(RC衰减), 形成"尾音"效果, 而非突然截止。
 *
 * ISR驱动由TIMER1_IRQHandler调用, 中断频率可变(由HAL_BUZZ_PWM_SET控制)。
 */
#include "hal_buzzer.h"

/* ========== ISR共享状态（仅2个，其余10个已移入DRV层静态变量） ========== */
uint8_t g_buzz_hz_timer;
uint8_t g_buzz_my_active;

/* ========== 普通蜂鸣器驱动（~1KHz方波 + 尾音衰减） ========== */
static void Buzz_Drive_4K(void)
{
    if (g_buzz_hz_timer != 0u) {
        HAL_BUZZ_PWR(1);
        HAL_BUZZ_SIG_OUT;
        HAL_BUZZ_SIG_TOGGLE;
    } else {
        /* 高阻输入: 电容通过蜂鸣器缓慢放电 → 尾音 */
        HAL_BUZZ_SIG_IN;
        HAL_BUZZ_SIG(0);
    }
}

/* ========== 美声/和弦驱动（可变频率 + PE4包络控制） ========== */
static void Buzz_Drive_MY(void)
{
    if (g_buzz_my_active != 0u) {
        HAL_BUZZ_SIG_OUT;
        HAL_BUZZ_SIG_TOGGLE;
    } else {
        HAL_BUZZ_SIG(0);
    }
}

/* ========== 公共接口 ========== */

void HAL_Buzzer_Init(void)
{
    TIM_TimeBaseInitTypeDef tim_init;

    /* GPIO初始化 */
    HAL_BUZZ_SIG_OUT;
    HAL_BUZZ_SIG(0);
    HAL_BUZZ_PWR_OUT;
    HAL_BUZZ_PWR(1);

    /* TIM1: APB0=48MHz, 默认2000Hz中断 → 1000Hz方波输出 */
    RCC_APB0Cmd(ENABLE);
    RCC_APB0PeriphClockCmd(RCC_APB0Periph_TIM1, ENABLE);

    TIM_TimeBaseStructInit(&tim_init);
    tim_init.TIM_CounterMode = TIM_CounterMode_Up;
    tim_init.TIM_EXENX       = TIM_EXENX_Disable;
    tim_init.TIM_Preload     = 65535u - (48000000u / 2000u);
    tim_init.TIM_Prescaler   = TIM_PRESCALER_1;
    tim_init.TIM_WorkMode    = TIM_WorkMode_Timer;
    TIM_TIMBaseInit(TIM1, &tim_init);

    TIM_ITConfig(TIM1, TIM_IT_TI | TIM_IT_INTEN, DISABLE);
    NVIC_DisableIRQ(TIMER1_IRQn);
    TIM_Cmd(TIM1, ENABLE);

    /* 清零ISR共享状态 */
    g_buzz_hz_timer  = 0u;
    g_buzz_my_active = 0u;
}

void HAL_Buzzer_ISR_Drive(void)
{
    if (g_buzz_my_active != 0u) {
        Buzz_Drive_MY();
    } else {
        Buzz_Drive_4K();
    }
}

void HAL_Buzzer_IntSync(void)
{
    if (g_buzz_hz_timer != 0u || g_buzz_my_active != 0u) {
        HAL_BUZZ_INT_ON();
    } else {
        HAL_BUZZ_INT_OFF();
    }
}

/* ========== DRV层setter/getter（封装ISR共享变量访问） ========== */

void HAL_Buzzer_SetTimer(uint8_t val)
{
    g_buzz_hz_timer = val;
}

uint8_t HAL_Buzzer_GetTimer(void)
{
    return g_buzz_hz_timer;
}

void HAL_Buzzer_SetActive(uint8_t val)
{
    g_buzz_my_active = val;
}

/* ========== TIMER1中断处理 ========== */
void TIMER1_IRQHandler(void)
{
    if (TIM_GetFlagStatus(TIM1, TIM_Flag_TI) == SET) {
        TIM_ClearFlag(TIM1, TIM_Flag_TI);
        HAL_Buzzer_ISR_Drive();
    }
}
