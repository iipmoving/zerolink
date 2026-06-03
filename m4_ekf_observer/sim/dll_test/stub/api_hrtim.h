#ifndef STUB_API_HRTIM_H
#define STUB_API_HRTIM_H

/* Stub: MCU HRTIM 寄存器宏 — PC 测试用 */
#define fPWM_div4           4
#define PWM_DIV             fPWM_div4
#define SysPWM_Frequency    (192*1000*1000)
#define PWM_SF              (SysPWM_Frequency*PWM_DIV/2)
#define FRE_500K            500000
#define FRE_500K_PWM        (PWM_SF/FRE_500K)
#define HRTIM_ADJ_500nS     (FRE_500K_PWM/2)
#define HRTIM_ADJ           HRTIM_ADJ_500nS
#define FRE_PER_ADC         HRTIM_ADJ

#endif
