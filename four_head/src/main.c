/**
 * main.c —— 四头电磁炉控制程序入口
 *
 * 调度模式:
 *   每1ms: 1ms_ISR() + ExecSlot_Run(轮转1个槽)
 *   11槽 × 1ms = 11ms 完整周期
 *   每个模块每11ms被调用一次，用调用次数做独立计时
 *
 * main只声明 __weak 入口, 实例在 data_switcher.c
 * 所有模块调用链在 Switcher_Run_{SlotName} 强符号里实现
 */
#include "sc32_conf.h"
#include "system_sc32f1xxx.h"
#include "hal/hal_timer.h"
#include "hal/hal_uart.h"
#include "hal/hal_gpio.h"
#include "hal/hal_buzzer.h"
#include "drv/drv_buzzer.h"
#include "drv/drv_display.h"
#include <stddef.h>

/* __weak 调度入口 — Switcher 提供强符号实现 */
__weak void Slot_Init(void)        { }
__weak void Slot_every1ms(void)    { }
__weak void Switcher_Run_Slot0(void) { }
__weak void Switcher_Run_Slot1(void) { }
__weak void Switcher_Run_Slot2(void) { }
__weak void Switcher_Run_Slot3(void) { }
__weak void Switcher_Run_Slot4(void) { }
__weak void Switcher_Run_Slot5(void) { }
__weak void Switcher_Run_Slot6(void) { }
__weak void Switcher_Run_Slot7(void) { }
__weak void Switcher_Run_Slot8(void) { }
__weak void Switcher_Run_Slot9(void) { }
__weak void Switcher_Run_Slot10(void) { }
__weak void Slot_loop1ms(void)     { }

/* ========== 时间片调度 ========== */
static uint8_t s_prog_slot;

static void ExecSlot_Run(void)
{
    switch (s_prog_slot) {
    case 0u:  Switcher_Run_Slot0();  break;
    case 1u:  Switcher_Run_Slot1();  break;
    case 2u:  Switcher_Run_Slot2();  break;
    case 3u:  Switcher_Run_Slot3();  break;
    case 4u:  Switcher_Run_Slot4();  break;
    case 5u:  Switcher_Run_Slot5();  break;
    case 6u:  Switcher_Run_Slot6();  break;
    case 7u:  Switcher_Run_Slot7();  break;
    case 8u:  Switcher_Run_Slot8();  break;
    case 9u:  Switcher_Run_Slot9();  break;
    case 10u: Switcher_Run_Slot10(); break;
    default: break;
    }
    s_prog_slot = (s_prog_slot + 1u) % 11u;
}

int main(void)
{
    HAL_UART_Debug_Init();
    HAL_UART_Debug_Print("12345");

    HAL_GPIO_Init();
    HAL_Buzzer_Init();
    Slot_Init();

    s_prog_slot = 0u;
    HAL_Timer_Init();

    HAL_UART_Debug_Print("[SYS] Init done, 4-head IH controller ready.\r\n");

    for (;;) {
        if (HAL_Timer_1msElapsed()) {
            Drv_Buzzer_Timer_1ms();
            Drv_Display_Scan();
            Slot_every1ms();
            ExecSlot_Run();
        }

        HAL_UART_Debug_Flush();
        Slot_loop1ms();
    }
}
