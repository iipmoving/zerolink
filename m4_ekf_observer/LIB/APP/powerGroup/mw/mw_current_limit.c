/* mw_current_limit.c — 限流封装
 * layer: MW (中间层)
 *
 * 职责: 封装限流算法，对 ppg_delta 做限幅。
 *       目前为直通占位，待接入实际限流参数。
 */
#include "mw.h"


int16_t mw_CurrentLimit_Clip(uint8_t ch, int16_t delta_ppg)
{
    /* TODO: 接入实际限流参数 (max_delta 等) */
    (void)ch;
    return delta_ppg;  /* 直通 */
}
