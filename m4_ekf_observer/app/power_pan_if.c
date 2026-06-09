/**
 * @file    power_pan_if.c
 * @brief   检锅触发接口 — 纯业务判断
 * @layer   app
 *
 * 只判断"当前是否需要发起检锅"，设置 pan_request 标志。
 * 从 app_power_claude.c s_pan_check_fun / PanStatusCheck 迁出。
 */
#include "power_pan_if.h"
#include "app_power_hw_io.h"
#include <string.h>

/* ---- 每炉头检锅状态 ---- */
static uint8_t s_pan_delay[POWER_POTMAX];

void PowerPanIf_Run(PowerBase_Input_t *in, uint8_t ch, uint8_t *pan_request, uint8_t *pan_ch)
{
    if (ch >= POWER_POTMAX) return;

    /* ---- 检查 DRV 反馈: 检锅结果 ---- */
    uint8_t pan_result = in->hw_status.pan_result[ch];

    if (pan_result == 0xFF) {
        /* 检测中, 等待 */
        return;
    }

    /* ---- 功率为零且需要检锅 ---- */
    uint16_t target = in->head[ch].target_power;
    uint8_t  on     = in->head[ch].power_on;

    if (on && target > 0 && pan_result == 0) {
        /* 有功率请求但无锅 → 触发检锅 */
        *pan_request = 1;
        *pan_ch     = ch;
        return;
    }

    /* TODO: 迁入完整检锅触发逻辑
     * - s_pan_check_fun: 移锅检测 (功率突降 → 重新检锅)
     * - PanStatusCheck: 起振后脉冲计数检查 */
    (void)s_pan_delay;
}
