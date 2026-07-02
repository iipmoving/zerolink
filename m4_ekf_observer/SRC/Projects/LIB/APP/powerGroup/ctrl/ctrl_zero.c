/* ctrl_zero.c — 过零策略
 * layer: CTRL (APP logic)
 *
 * 职责: 过零同步检查、倍频/同频过零切换。
 *       自声明 per-channel 状态，不 extern。
 *
 * 迁出自: app_power_claude.c
 *    APP_POWER_ZeroSync (line 4942)
 *    powerZeroChange    (line 4449)
 */
#include "ctrl.h"
#include "../drv/drv.h"
#include "API_HRTIM.h"          /* FRE_30K_PWM */

/* ===== 外部: __weak 系统接口 ===== */
__attribute__((weak)) uint16_t* Adc_GetHrtimSyncBuffAdr(void) { return 0; }
__attribute__((weak)) void      Adc_ClearCeilQAvg(uint8_t ch) { (void)ch; }

/* ===== 通道数 ===== */
#define CTRL_ZERO_CH_MAX  4

/* ===== 自声明 per-channel 状态 ===== */
struct zero_ch_ctx {
    uint8_t  cycle_roll;        /* 0=同频, 1=倍频 */
    uint8_t  power_cycle_type;  /* 0=单频, 1=倍频切换 */
    uint16_t ppg_duty_actual;   /* 当前占空比 */
    uint16_t base_duty;         /* 切换前占空比 */
};

static struct zero_global_ctx {
    uint16_t power_cycle;       /* 全局周期 */
    uint8_t  pot_num;           /* 通道数 */
} s_glb;

static struct zero_ch_ctx s_ch[CTRL_ZERO_CH_MAX];


/* ===================================================================
 *  ctrl_ZeroSync — 过零同步检查
 *  原: APP_POWER_ZeroSync (line 4942)
 *
 *  遍历各通道，在过零点处理倍频/同频切换。
 *  返回 ppgChange: 非0 表示需要更新周期
 * =================================================================== */
uint8_t ctrl_ZeroSync(void)
{
    uint8_t ppg_change = 0;
    uint8_t i;

    for (i = 0; i < s_glb.pot_num; i++) {
        struct zero_ch_ctx *p = &s_ch[i];

        if (p->power_cycle_type) {
            /* 倍频切换模式 */
            if (s_glb.power_cycle < FRE_30K_PWM * 2) {
                p->power_cycle_type = 0;
                p->cycle_roll = 0;
            } else if (p->cycle_roll == 0) {
                ppg_change = 1;
                p->cycle_roll = 1;          /* 切到倍频 */
                Adc_ClearCeilQAvg(i);
            }
        } else {
            /* 单频模式 */
            if (p->cycle_roll) {
                ppg_change = 1;
                p->cycle_roll = 0;          /* 切回同频 */
                p->ppg_duty_actual = p->base_duty;
            }
        }
    }

    return ppg_change;
}


/* ===================================================================
 *  ctrl_ZeroChange — 过零变化入口
 * =================================================================== */
void ctrl_ZeroChange(void)
{
    ctrl_ZeroSync();
}


/* ===================================================================
 *  ctrl_ZeroInit — 初始化
 * =================================================================== */
void ctrl_ZeroInit(uint8_t pot_num)
{
    uint8_t i;
    s_glb.pot_num = (pot_num > CTRL_ZERO_CH_MAX) ? CTRL_ZERO_CH_MAX : pot_num;
    s_glb.power_cycle = 0;

    for (i = 0; i < s_glb.pot_num; i++) {
        struct zero_ch_ctx *p = &s_ch[i];
        p->cycle_roll = 0;
        p->power_cycle_type = 0;
        p->ppg_duty_actual = 0;
        p->base_duty = 0;
    }
}


/* ===================================================================
 *  ctrl_ZeroUpdate — 由 ctrl_power 更新周期/状态
 * =================================================================== */
void ctrl_ZeroUpdate(uint8_t ch, uint16_t power_cycle, uint16_t duty,
                      uint8_t cycle_roll, uint8_t cycle_type)
{
    if (ch >= CTRL_ZERO_CH_MAX) return;
    struct zero_ch_ctx *p = &s_ch[ch];
    s_glb.power_cycle = power_cycle;
    p->cycle_roll = cycle_roll;
    p->power_cycle_type = cycle_type;
    p->ppg_duty_actual = duty;
    p->base_duty = duty;
}
