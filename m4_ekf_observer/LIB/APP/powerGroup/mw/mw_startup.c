/* mw_startup.c — 起振操作封装
 * layer: MW (中间层)
 *
 * 职责: 封装起振流程: 开启 PWM → 起振脉冲序列。
 */
#include "mw.h"
#include "../drv/drv.h"


void mw_Startup(uint8_t ch)
{
    if (ch >= 4) return;

    /* 起振序列: 开输出 → 发检锅脉冲 */
    drv_pwm_on(ch);
    drv_StartPPG(ch);
}


void mw_StartupAll(void)
{
    uint8_t i;
    for (i = 0; i < 4; i++) {
        mw_Startup(i);
    }
}
