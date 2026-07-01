/**
 * hal_timer.c —— TIM0 125us时基实现
 *
 * 依赖: hal_timer.h + hal_key.h + 固件库
 * 层级: HAL —— 不include msg_scheduler.h
 *
 * ISR: 125us周期
 *   - 每次中断: HAL_Key_Poll() 驱动触摸库状态机
 *   - 每8次中断: 置1ms标志 + tick++
 * TIM0: APB0=48MHz, 向上计数, Preload=65535-6000, 125us
 *
 * 参考: SC_Init.c → SC_TIM0_Init()
 */
#include "hal_timer.h"
#include "sc32f1xxx_tim.h"
#include "sc32f1xxx_rcc.h"
#include "sc32L14xx.h"

static volatile uint32_t s_tick_ms;
static volatile uint8_t  s_1ms_pending;
static          uint8_t  s_isr_125us_cnt;    /* 125us→1ms 分频计数器 */

/* APB0=48MHz, 125us=8000Hz, ticks=48000000/8000=6000 */
#define TIM0_PRESCALER      TIM_PRESCALER_1
#define TIM0_PRELOAD        (65535u - (48000000u / 8000u))   /* = 59535 */
#define TIM0_DIV_1MS        8u                               /* 8×125us=1ms */

void HAL_Timer_Init(void)
{
    TIM_TimeBaseInitTypeDef tim_init;

    s_tick_ms       = 0u;
    s_1ms_pending   = 0u;
    s_isr_125us_cnt = 0u;

    RCC_APB0PeriphClockCmd(RCC_APB0Periph_TIM0, ENABLE);

    TIM_TimeBaseStructInit(&tim_init);
    tim_init.TIM_Prescaler   = TIM0_PRESCALER;
    tim_init.TIM_WorkMode    = TIM_WorkMode_Timer;
    tim_init.TIM_CounterMode = TIM_CounterMode_Up;
    tim_init.TIM_EXENX       = TIM_EXENX_Disable;
    tim_init.TIM_Preload     = TIM0_PRELOAD;
    TIM_TIMBaseInit(TIM0, &tim_init);

    NVIC_SetPriority(TIMER0_IRQn, 0u);   /* 最高优先级，与参考一致 */
    TIM_ITConfig(TIM0, TIM_IT_TI | TIM_IT_INTEN, ENABLE);
    NVIC_EnableIRQ(TIMER0_IRQn);

    TIM_Cmd(TIM0, ENABLE);
}

uint32_t HAL_Timer_GetTick(void)
{
    return s_tick_ms;
}

uint8_t HAL_Timer_1msElapsed(void)
{
    if (s_1ms_pending != 0u) {
        s_1ms_pending = 0u;
        return 1u;
    }
    return 0u;
}

/* ========== TIM0中断 —— 125us周期 ========== */
void TIMER0_IRQHandler(void)
{
    TIM_ClearFlag(TIM0, TIM_Flag_TI);

    // 驱动触摸库状态机 (触摸库自带中断, 不需125us轮询; 10ms读一次即可)
//    HAL_Key_Poll();

    /* 125us→1ms 分频 */
    s_isr_125us_cnt++;
    if (s_isr_125us_cnt >= TIM0_DIV_1MS) {
        s_isr_125us_cnt = 0u;
        s_tick_ms++;
        s_1ms_pending = 1u;
    }
}
