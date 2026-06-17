/* mw_current_limit.c — 限流封装
 * layer: MW (中间层)
 *
 * 职责: 封装限流算法，对 ppg_delta 做限幅。
 *       自声明 per-channel 限流参数。
 */
#include "mw.h"

/* ===== 通道数 ===== */
#define MW_LIMIT_CH_MAX  4

/* ===== 自声明 per-channel 限流参数 ===== */
struct limit_ch_ctx {
    int16_t max_delta_inc;     /* 最大单次增量 */
    int16_t max_delta_dec;     /* 最大单次减量 */
    int16_t max_ppg;           /* 本通道最大 PPG */
    int16_t min_ppg;           /* 本通道最小 PPG */
};

static struct limit_ch_ctx s_ch[MW_LIMIT_CH_MAX];


/* ===================================================================
 *  mw_CurrentLimit_Clip — 对 ppg_delta 做限幅
 *  返回限幅后的 delta
 * =================================================================== */
int16_t mw_CurrentLimit_Clip(uint8_t ch, int16_t delta_ppg)
{
    if (ch >= MW_LIMIT_CH_MAX) return delta_ppg;
    struct limit_ch_ctx *p = &s_ch[ch];

    /* 增量/减量限幅 */
    if (delta_ppg > 0 && delta_ppg > p->max_delta_inc) {
        return p->max_delta_inc;
    }
    if (delta_ppg < 0 && delta_ppg < -p->max_delta_dec) {
        return -p->max_delta_dec;
    }

    return delta_ppg;
}


/* ===================================================================
 *  mw_CurrentLimit_SetMaxDelta — 设置最大步长
 * =================================================================== */
void mw_CurrentLimit_SetMaxDelta(uint8_t ch, int16_t inc, int16_t dec)
{
    if (ch >= MW_LIMIT_CH_MAX) return;
    s_ch[ch].max_delta_inc = inc;
    s_ch[ch].max_delta_dec = dec;
}


/* ===================================================================
 *  mw_CurrentLimit_Init — 初始化
 * =================================================================== */
void mw_CurrentLimit_Init(uint8_t pot_num)
{
    uint8_t i;
    uint8_t n = (pot_num > MW_LIMIT_CH_MAX) ? MW_LIMIT_CH_MAX : pot_num;
    for (i = 0; i < n; i++) {
        s_ch[i].max_delta_inc = 200;   /* 默认值, 待标定 */
        s_ch[i].max_delta_dec = 200;
        s_ch[i].max_ppg = 60000;
        s_ch[i].min_ppg = 1000;
    }
}
