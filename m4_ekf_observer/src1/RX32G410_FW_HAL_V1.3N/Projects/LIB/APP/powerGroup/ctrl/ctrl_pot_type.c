/* ctrl_pot_type.c — 锅类型判断
 * layer: CTRL (APP logic)
 *
 * 职责: 判断锅具材质（铁锅/钢锅），设定最大 PPG。
 *       自声明 per-channel 状态，不 extern。
 *
 * 迁出自: app_power_claude.c
 *    APP_POWER_PotTypeCheck        (line 1654)
 *    APP_POWER_PotTypeCheck_FRE    (line 1434)
 *    APP_POWER_PotTypeCheckConfirm (line 1566)
 *    pot_max_ppg_set               (line 2156)
 *    get_MAXMIN_PPG                (line 2444)
 */
#include "ctrl.h"
#include "../drv/drv.h"
#include "API_HRTIM.h"          /* MIN_FRE_PWM, MAX_FRE_PWM, etc */

/* ===== 通道数 ===== */
#define CTRL_POT_CH_MAX  4

/* ===== 锅类型常量 ===== */
enum {
    POT_UNKNOWN = 0,
    POT_IRON    = 1,    /* 铁锅 */
    POT_STEEL   = 2     /* 钢锅 */
};

/* ===== 功率常量 ===== */
#define IRON_POWER_H   (1000/25)
#define IRON_POWER_L   (600/25)

/* ===== 自声明 per-channel 状态 ===== */
struct pot_ch_ctx {
    uint8_t  ch;
    uint8_t  pot_type;          /* 锅类型 */
    uint16_t ppg_max;           /* 本锅最大 PPG */
    uint16_t ppg_power_adj;     /* 功率修正 PPG (was s_ppg_power_adj) */
    uint16_t power_res;         /* 等效电阻 (res) */
    uint8_t  count;             /* 稳定计数 */
};

static struct pot_ch_ctx s_ch[CTRL_POT_CH_MAX];
static uint8_t s_pot_num;


/* ===================================================================
 *  ctrl_PotTypeCheck — 锅类型判断主入口
 *  原: APP_POWER_PotTypeCheck
 *  每控制周期调用
 * =================================================================== */
void ctrl_PotTypeCheck(void)
{
    /* TODO: 完整锅类型判断逻辑
     * 依赖: 各频率下等效电阻 (PowerMem[...].staticReg->potPowerSave[].res)
     * 待 Switcher 就绪后从 ADC/FMAC 模块读取
     */
}


/* ===================================================================
 *  ctrl_PotTypeCheckFRE — 特定频率下判断锅类型
 *  原: APP_POWER_PotTypeCheck_FRE
 *  ch: 0=40K, 1=30K, 2=29.5K
 * =================================================================== */
void ctrl_PotTypeCheckFRE(uint8_t ch)
{
    if (ch >= CTRL_POT_CH_MAX) return;
    struct pot_ch_ctx *p = &s_ch[ch];

    /* TODO: 读取 ADC + FMAC 计算结果, 更新 potPowerSave */
    (void)p;
}


/* ===================================================================
 *  ctrl_PotTypeConfirm — 确认锅类型
 *  原: APP_POWER_PotTypeCheckConfirm
 * =================================================================== */
void ctrl_PotTypeConfirm(void)
{
    /* TODO: 根据各频率等效电阻判断材质 */
}


/* ===================================================================
 *  ctrl_PotMaxPpgSet — 设定最大 PPG
 *  原: pot_max_ppg_set (line 2156)
 *  根据当前功率与 PPG 周期推算最大功率周期
 * =================================================================== */
void ctrl_PotMaxPpgSet(uint8_t ch)
{
    if (ch >= CTRL_POT_CH_MAX) return;
    struct pot_ch_ctx *p = &s_ch[ch];
    (void)p;

    /* TODO: 功率推算逻辑
     *    t_ppg_out = PowerMinFre;
     *    if (ActualPower == TargetPower && MaxPowerM > ActualPower)
     *        t_ppg_out += adj * PPG_PER_25W
     */
}


/* ===================================================================
 *  ctrl_PotGetMinMaxPPG — 读取/计算最大最小 PPG
 *  原: get_MAXMIN_PPG (line 2444)
 * =================================================================== */
void ctrl_PotGetMinMaxPPG(uint8_t ch)
{
    if (ch >= CTRL_POT_CH_MAX) return;
    struct pot_ch_ctx *p = &s_ch[ch];
    (void)p;

    /* TODO: 根据锅类型设定 ppg_max */
}


/* ===================================================================
 *  ctrl_PotInit — 初始化
 * =================================================================== */
void ctrl_PotInit(uint8_t pot_num)
{
    uint8_t i;
    s_pot_num = (pot_num > CTRL_POT_CH_MAX) ? CTRL_POT_CH_MAX : pot_num;

    for (i = 0; i < s_pot_num; i++) {
        struct pot_ch_ctx *p = &s_ch[i];
        p->ch = i;
        p->pot_type = POT_UNKNOWN;
        p->ppg_max = MIN_FRE_PWM;
        p->ppg_power_adj = MIN_FRE_PWM;
        p->power_res = 0;
        p->count = 0;
    }
}


/* ===================================================================
 *  ctrl_PotGetType — 读取锅类型
 * =================================================================== */
uint8_t ctrl_PotGetType(uint8_t ch)
{
    if (ch >= CTRL_POT_CH_MAX) return POT_UNKNOWN;
    return s_ch[ch].pot_type;
}


/* ===================================================================
 *  ctrl_PotSetType — 设定锅类型
 * =================================================================== */
void ctrl_PotSetType(uint8_t ch, uint8_t type)
{
    if (ch >= CTRL_POT_CH_MAX) return;
    s_ch[ch].pot_type = type;
}
