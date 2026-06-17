/* mw_protect.c — 保护检测封装
 * layer: MW (中间层)
 *
 * 职责: 封装保护检测接口，APP 通过这层读取故障状态。
 *       自声明 per-channel 状态。
 */
#include "mw.h"
#include "../drv/drv.h"

/* ===== 通道数 ===== */
#define MW_PROTECT_CH_MAX  4

/* ===== 自声明 per-channel 状态 ===== */
struct prot_ch_ctx {
    uint8_t bk_flag;         /* 浪涌标志 */
    uint8_t overcurrent;     /* 过流标志 */
};

static struct prot_ch_ctx s_ch[MW_PROTECT_CH_MAX];


/* ===================================================================
 *  mw_Protect_CheckBK — 检查 BK 浪涌标志
 *  返回 0=正常, 非0=浪涌
 * =================================================================== */
uint8_t mw_Protect_CheckBK(uint8_t ch)
{
    if (ch >= MW_PROTECT_CH_MAX) return 0;

    /* TODO: 接入实际 BK 检测 (API_PPG_BkFlag_PotX / FunTimBkFlag)
     * 当前返回自维护状态, 由外部更新
     */
    return s_ch[ch].bk_flag;
}


/* ===================================================================
 *  mw_Protect_CheckOvercurrent — 过流检测
 *  返回 0=正常, 非0=过流
 * =================================================================== */
uint8_t mw_Protect_CheckOvercurrent(uint8_t ch)
{
    if (ch >= MW_PROTECT_CH_MAX) return 0;

    /* TODO: 接入实际过流检测 (ADC AWD / CMP) */
    return s_ch[ch].overcurrent;
}


/* ===================================================================
 *  mw_Protect_SetBK — 外部更新 BK 标志 (ISR/DRV 调用)
 * =================================================================== */
void mw_Protect_SetBK(uint8_t ch, uint8_t flag)
{
    if (ch >= MW_PROTECT_CH_MAX) return;
    s_ch[ch].bk_flag = flag;
}


/* ===================================================================
 *  mw_Protect_SetOvercurrent — 外部更新过流标志
 * =================================================================== */
void mw_Protect_SetOvercurrent(uint8_t ch, uint8_t flag)
{
    if (ch >= MW_PROTECT_CH_MAX) return;
    s_ch[ch].overcurrent = flag;
}


/* ===================================================================
 *  mw_Protect_Init — 初始化
 * =================================================================== */
void mw_Protect_Init(uint8_t pot_num)
{
    uint8_t i;
    uint8_t n = (pot_num > MW_PROTECT_CH_MAX) ? MW_PROTECT_CH_MAX : pot_num;
    for (i = 0; i < n; i++) {
        s_ch[i].bk_flag = 0;
        s_ch[i].overcurrent = 0;
    }
}
