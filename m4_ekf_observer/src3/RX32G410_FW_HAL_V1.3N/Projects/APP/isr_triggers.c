/**
 * @file    isr_triggers.c
 * @brief   ISR 触发 PendSV 入口 (v2.3)
 *
 * 所有 ISR 只调用 Switcher_TriggerPendSV(source)，不做任何处理
 * 实际工作在 PendSV_Handler 中统一调度
 */

#define STD_MODULE_ENABLE_ISR  1
#include "std_module.h"
#include "pendsv_switcher.h"

/* ========== ADC ISR ========== */
void ADC_IRQHandler(void)
{
    /* 清除 ADC 中断标志 (示意) */
    /* ADC->SR &= ~ADC_SR_EOC; */

    /* 触发 PendSV */
    Switcher_TriggerPendSV(ISR_SOURCE_ADC);
}

/* ========== Timer ISR ========== */
void TIM1_IRQHandler(void)
{
    /* 清除 Timer 中断标志 (示意) */
    /* TIM1->SR &= ~TIM_SR_UIF; */

    /* 触发 PendSV */
    Switcher_TriggerPendSV(ISR_SOURCE_TIMER);
}

/* ========== Fault ISR ========== */
void Fault_IRQHandler(void)
{
    /* 触发 PendSV */
    Switcher_TriggerPendSV(ISR_SOURCE_FAULT);
}
