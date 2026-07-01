/**
 * system_sc32f1xxx.c —— 系统时钟初始化
 *
 * 与参考程序 SC_Init.c → SC_RCC_Init() 完全一致:
 *   HIRC 48MHz → SYSCLK → HCLK(Div1) → APB0/1/2(Div1)
 *   不使用外部晶振(HXT/LXT)
 */
#include "sc32f1xxx_rcc.h"
#include "sc32f1xxx_option.h"
#include "system_sc32f1xxx.h"

uint32_t SystemCoreClock = 48000000u;

void SystemInit(void)
{
    RCC_Unlock(0xFF);

    RCC_HXTCmd(DISABLE);
    RCC_LXTCmd(DISABLE);

    RCC_LIRCCmd(ENABLE);
    RCC_HIRCCmd(ENABLE);
    RCC_ITConfig(ENABLE);
    RCC_HIRCDIV1Cmd(ENABLE);

    RCC_SYSCLKConfig(RCC_SYSCLKSource_HIRC);
    RCC_HCLKConfig(RCC_SYSCLK_Div1);

    RCC_APB0Config(RCC_HCLK_Div1);
    RCC_APB0Cmd(ENABLE);

    RCC_APB1Config(RCC_HCLK_Div1);
    RCC_APB1Cmd(ENABLE);

    RCC_APB2Config(RCC_HCLK_Div1);
    RCC_APB2Cmd(ENABLE);

    /* 关JTAG/SWD，释放PA0(SWCLK)→COM7, PA1(SWDIO)→COM6 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_IFB, ENABLE);
    OPTION_JTAGCmd(DISABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_IFB, DISABLE);

    SystemCoreClockUpdate();
}

void SystemCoreClockUpdate(void)
{
    SystemCoreClock = 48000000u;
}
