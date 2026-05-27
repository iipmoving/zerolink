/**
 * main.c —— 四头电磁炉控制程序入口
 *
 * 调度模式:
 *   每1ms: 起始段(MsgScheduler) + 执行段(轮转1个槽)
 *   10槽 × 1ms = 10ms 完整周期
 *   每个模块每10ms被调用一次，用调用次数做独立计时
 *
 * 不依赖 hal_timer 计时 —— 模块自己数调用次数
 */
#include "sc32_conf.h"
#include "system_sc32f1xxx.h"
#include "hal/hal_timer.h"
#include "hal/hal_uart.h"
#include "hal/hal_gpio.h"
#include "hal/hal_buzzer.h"
#include <stddef.h>
#include "test/test_module_a.h"
#include "test/test_module_b.h"
#include "drv/drv_key.h"
#include "drv/drv_display.h"
#include "drv/drv_buzzer.h"
#include "drv/drv_comm_mgr.h"
#include "app/app_comm_mgr.h"
#include "app/app_protect.h"
#include "app/app_power.h"
#include "app/app_cooking.h"
#include "app/app_hmi.h"

/* __weak 定时回调: 链接器自动接线, interface_map.h 文档化 */
__weak void AppHmi_OnTimer100ms(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }
__weak void AppHmi_OnTimer1s(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }
__weak void AppCooking_OnTimer1s(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

/* ========== 按键事件处理（调试用，HMI引擎接管后禁用）========== */
#if 0
static void Key_OnEvent(MsgId_t id, uint16_t param, void *data_ptr)
{
    uint8_t key_code = (uint8_t)(param & 0xFFu);
    uint8_t key_state = (uint8_t)((param >> 8) & 0xFFu);
    const char *state_name;
    (void)id; (void)data_ptr;

    /* 刷新显示: 上=键码hex, 下=按键类型 */
    Drv_Display_ShowKey(key_code, key_state);

    /* 调试串口打印 */
    if (key_state == KEY_STATE_PRESS)         state_name = "PRESS";
    else if (key_state == KEY_STATE_LONG)     state_name = "LONG";
    else if (key_state == KEY_STATE_RELEASE)  state_name = "RELEASE";
    else if (key_state == KEY_STATE_REPEAT)   state_name = "REPEAT";
    else if (key_state == KEY_STATE_TAP)      state_name = "TAP";
    else                                      state_name = "?";
    HAL_UART_Debug_Print("[KEY] ");
    HAL_UART_Debug_Print(state_name);
    HAL_UART_Debug_Print(" code=0x");
    HAL_UART_Debug_HexDump(&key_code, 1u);
    HAL_UART_Debug_Print("\r\n");

    /* 按键0~9: 蜂鸣器测试 */
    if (key_state == KEY_STATE_PRESS) {
        switch (key_code) {
        case KEY_POWER_0: Drv_Buzzer_Select(DRV_BUZZ_OUT_NORM, DRV_BUZZ_KEY);    break;
        case KEY_POWER_1: Drv_Buzzer_Select(DRV_BUZZ_OUT_MY,   DRV_BUZZ_MY_KEY);  break;
        case KEY_POWER_2: Drv_Buzzer_Select(DRV_BUZZ_OUT_MY,   DRV_BUZZ_MY_ON);   break;
        case KEY_POWER_3: Drv_Buzzer_Select(DRV_BUZZ_OUT_MY,   DRV_BUZZ_MY_OFF);  break;
        case KEY_POWER_4: Drv_Buzzer_Select(DRV_BUZZ_OUT_MY,   DRV_BUZZ_MY_ADD);  break;
        case KEY_POWER_5: Drv_Buzzer_Select(DRV_BUZZ_OUT_MY,   DRV_BUZZ_MY_SUB);  break;
        case KEY_POWER_6: Drv_Buzzer_Select(DRV_BUZZ_OUT_MY,   DRV_BUZZ_MY_SD);   break;
        case KEY_POWER_7: Drv_Buzzer_Select(DRV_BUZZ_OUT_MY,   DRV_BUZZ_MY_END);  break;
        case KEY_POWER_8: Drv_Buzzer_Select(DRV_BUZZ_OUT_MY,   DRV_BUZZ_MY_START);break;
        case KEY_POWER_9: Drv_Buzzer_Select(DRV_BUZZ_OUT_MY,   DRV_BUZZ_MY_SMALLSTAR); break;
        default: break;
        }
    }
}
#endif /* 0 — 按键事件处理调试 */

/* ========== 心跳指示（已禁用 —— PE1现由buzzer独占） ========== */
#if 0
static void Heartbeat_OnTimer100ms(MsgId_t id, uint16_t param, void *data_ptr)
{
    (void)id; (void)param; (void)data_ptr;
    HAL_GPIO_Toggle(HAL_IO_BUZZER);
}
#endif

/* ========== 槽位0: 定时消息生成 ========== */
/* 本槽每10ms调用一次，数10次=100ms，数100次=1s */
static void Slot_TimerTick(void)
{
    static uint8_t  cnt_100ms;
    static uint16_t cnt_1s;

    cnt_100ms++;
    if (cnt_100ms >= 10u) {
        cnt_100ms = 0u;
        AppHmi_OnTimer100ms(0u, NULL);
    }

    cnt_1s++;
    if (cnt_1s >= 100u) {
        cnt_1s = 0u;
        AppHmi_OnTimer1s(0u, NULL);
        AppCooking_OnTimer1s(0u, NULL);
    }
}

/* ========== 10槽分配 ========== */
/*
 * 每槽1ms，10槽=10ms完整周期:
 *   0: 定时消息(100ms/1s)
 *   1: 显示控制
 *   2: 功率/输出控制
 *   3: 按键处理
 *   4: 通讯处理
 *   5: 保护/故障检测
 *   6: 烹饪状态机
 *   7: HMI 引擎
 *   8-9: 预留
 */
static uint8_t s_prog_slot;

static void ExecSlot_Run(void)
{
    switch (s_prog_slot) {
    case 0u: Slot_TimerTick();     break;
    case 1u: Drv_Display_Update();  break;
    case 2u: App_Power_Run();        break;
    case 3u: Drv_Key_Scan();        break;
    case 4u: Drv_CommMgr_Update(); App_CommMgr_Run(); break;
    case 5u: App_Protect_Run();      break;
    case 6u: App_Cooking_Run();     break;
    case 7u: App_Hmi_Run();            break;
    case 8u:
    case 9u: /* 预留 */            break;
    default: break;
    }
}

/* ========== 主函数 ========== */
int main(void)
{
    HAL_UART_Debug_Init();
    HAL_UART_Debug_Print("12345");

    HAL_Buzzer_Init();
    HAL_GPIO_Init();
    Drv_CommMgr_Init();

    /* MsgScheduler 已移除——全部改用 __weak 直调 */
    /* MsgScheduler_Register(MSG_KEY_EVENT, Key_OnEvent); -- 禁用: 由HMI引擎接管按键处理 */
//    TestA_Init();
//    TestB_Init();
    Drv_Key_Init();
    Drv_Display_Init();
    Drv_Buzzer_Init();
    App_CommMgr_Init();
    App_Protect_Init();
    App_Power_Init();
    App_Cooking_Init();
    App_Hmi_Init();

    s_prog_slot = 0u;
    HAL_Timer_Init();

    HAL_UART_Debug_Print("[SYS] Init done, 4-head IH controller ready.\r\n");
//    TestA_StartChat();

    for (;;) {
        if (HAL_Timer_1msElapsed()) {
            /* === 蜂鸣器1ms时基 === */
            Drv_Buzzer_Timer_1ms();
            /* === COM扫描: 优先保证时序精确 === */
            Drv_Display_Scan();
            /* === 执行段: 轮转一个槽（每1ms一个） === */
            ExecSlot_Run();
            s_prog_slot = (s_prog_slot + 1u) % 10u;
        }

        /* === 等待段: 调试串口TX + 通讯解码等 === */
        HAL_UART_Debug_Flush();
    }
}
