/* mw_shutdown.c — 关机操作封装
 * layer: MW (中间层)
 *
 * 职责: 封装关机流程: 调 DRV PWM 停止输出 + 状态清理。
 *       APP 调 mw_Shutdown(ch, reason) 替代直接调 s_pwm_off。
 */
#include "mw.h"
#include "../drv/drv.h"

/* 通道状态 (私有, MW 持有) */
static struct shutdown_ctx {
    uint8_t state;
} s_ctx[4];


void mw_Shutdown(uint8_t ch, uint8_t reason)
{
    if (ch >= 4) return;
    struct shutdown_ctx *p = &s_ctx[ch];
    drv_pwm_off(ch, (int8_t)reason);
    p->state = 0;
}


void mw_SetState(uint8_t ch, uint8_t state)
{
    if (ch >= 4) return;
    struct shutdown_ctx *p = &s_ctx[ch];
    p->state = state;
}


uint8_t mw_GetState(uint8_t ch)
{
    if (ch >= 4) return 0;
    struct shutdown_ctx *p = &s_ctx[ch];
    return p->state;
}
