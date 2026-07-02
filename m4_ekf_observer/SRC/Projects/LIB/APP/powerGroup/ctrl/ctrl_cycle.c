/* ctrl_cycle.c — 周期/频率策略
 * layer: CTRL (APP logic)
 *
 * 职责: 同频/倍频切换、周期计算、通道周期分配。
 *       自声明 per-channel 状态，不 extern。
 *
 * 迁出自: app_power_claude.c
 *    APP_POWER_GetCycleType       (line 5027)
 *    APP_POWER_CycleChange        (line 5163)
 *    APP_POWER_CycleReset         (line 5173)
 *    APP_POWER_SetPowerCycleType  (line 5185)
 *    APP_POWER_ResetPowerCycleType (line 5206)
 *    APP_POWER_DutyLess50         (line 4907)
 */
#include "ctrl.h"
#include "../drv/drv.h"
#include "API_HRTIM.h"          /* PotNum, FRE_30K_PWM, etc */

/* ===== 通道数 ===== */
#define CTRL_CH_MAX  4

/* ===== 外部: __weak 系统接口 (未就绪时静默) ===== */
__attribute__((weak)) uint8_t Adc_IsTxaDmaStart(void) { return 0; }

/* ===== 自声明 per-channel 状态 ===== */
struct cycle_ch_ctx {
    uint16_t ppg_duty_actual;   /* 当前占空比 (was PowerMem[i].staticReg->PPGdutyActual) */
    uint16_t base_duty;         /* 切换前占空比 (was cycleChange.baseDuty) */
    uint16_t power_cycle;       /* 本通道周期 (was powerCycle[i]) */
    uint8_t  cycle_roll;        /* 0=同频, 1=倍频 (was cycleRoll) */
    uint8_t  power_single;      /* 单独调频标志 (was flag.bit.powerSingle) */
    uint8_t  power_cycle_type;  /* 0=单频, 1=倍频切换 (was flag.bit.PowerCycleType) */
    uint8_t  duty50;            /* 占空比>50%标志 (was flag.bit.PowerDuty50) */
};

/* ===== 全局周期状态 ===== */
static struct cycle_global_ctx {
    uint16_t power_cycle;       /* 全局同步周期 (was PowerCycle) */
    uint16_t cycle_change_cnt;  /* 同频切换计数 (was CycleChangeCnt) */
    uint8_t  change_status;     /* 切换状态 (was PowerChangeStatus) */
    uint8_t  pot_num;           /* 通道数 */
} s_glb;

static struct cycle_ch_ctx s_ch[CTRL_CH_MAX];

/* ===== 内部: 更新状态 ===== */
static void cycle_set_duty50(uint8_t ch, uint16_t cycle_half)
{
    struct cycle_ch_ctx *p = &s_ch[ch];
    if (p->ppg_duty_actual >= cycle_half) {
        if (!p->duty50) {
            p->duty50 = 1;
        }
    } else {
        if (p->duty50) {
            p->duty50 = 0;
        }
    }
}


/* ===================================================================
 *  ctrl_GetCycleType — 计算全局周期和各通道周期/占空比
 *  原: APP_POWER_GetCycleType (line 5027)
 *
 *  遍历各通道取最大频率 → 计算 PowerCycle → 分配各通道 duty/cycle
 *  返回 ppgChange 标志: 非0 表示需要更新 HRTIM (周期有变化)
 * =================================================================== */
uint8_t ctrl_GetCycleType(uint8_t change)
{
    uint8_t  ppg_change = change;
    uint16_t ppg_max = 0;
    uint16_t power_cycle_half;
    uint8_t  i;

    if (Adc_IsTxaDmaStart()) {
        return 0;
    }

    /* ---- 取各通道最大频率 (最小 period) ---- */
    for (i = 0; i < s_glb.pot_num; i++) {
        struct cycle_ch_ctx *p = &s_ch[i];
        uint16_t ppg_temp = p->ppg_duty_actual;

        if (ppg_temp < MAX_FRE_PWM) {
            ppg_temp = MAX_FRE_PWM;
        }

        if (ppg_max < ppg_temp) {
            ppg_max = ppg_temp;
        }
    }

    /* --- PowerMinFre 上限 --- */
    if (ppg_max > MIN_FRE_PWM) {
        ppg_max = MIN_FRE_PWM;
    }

    /* 偶数对齐 (PowerCycle 为 4 的倍数) */
    if (ppg_max & 1) {
        ppg_max += 1;
    }

    s_glb.power_cycle = ppg_max * 2;
    power_cycle_half = ppg_max;

    /* ---- 分配各通道周期/占空比 ---- */
    for (i = 0; i < s_glb.pot_num; i++) {
        struct cycle_ch_ctx *p = &s_ch[i];

        if (p->power_single) {
            /* 非同步炉头: 50% 占空比 */
            p->power_cycle = p->ppg_duty_actual * 2;
        } else {
            uint16_t ch_cycle_half;

            p->power_cycle = s_glb.power_cycle >> (p->cycle_roll);
            ch_cycle_half  = p->power_cycle / 2;
            if (p->ppg_duty_actual > ch_cycle_half) {
                p->ppg_duty_actual = ch_cycle_half;
            }
        }
    }

    return ppg_change;
}


/* ===================================================================
 *  ctrl_DutyLess50 — 检查各通道占空比是否 >50%
 *  原: APP_POWER_DutyLess50 (line 4907)
 * =================================================================== */
void ctrl_DutyLess50(void)
{
    uint16_t power_cycle_half = s_glb.power_cycle / 2 - 2;
    uint8_t  i;

    for (i = 0; i < s_glb.pot_num; i++) {
        cycle_set_duty50(i, power_cycle_half);
    }
}


/* ===================================================================
 *  ctrl_CycleChange — 切换到同频模式 (提高功率)
 *  原: APP_POWER_CycleChange (line 5163)
 *
 *  连续调用 10 次后触发 POWER_CHANGE_CYCLE_CHANGE
 * =================================================================== */
void ctrl_CycleChange(void)
{
    s_glb.cycle_change_cnt++;
    if (s_glb.cycle_change_cnt == 10) {
        s_glb.change_status = 4;  /* POWER_CHANGE_CYCLE_CHANGE */
    }
}


/* ===================================================================
 *  ctrl_CycleReset — 恢复倍频
 *  原: APP_POWER_CycleReset (line 5173)
 * =================================================================== */
void ctrl_CycleReset(void)
{
    if (s_glb.cycle_change_cnt >= 10) {
        s_glb.cycle_change_cnt = 0;
        s_glb.change_status = 5;  /* POWER_CHANGE_CYCLE_RESET */
    }
}


/* ===================================================================
 *  ctrl_SetPowerCycleType — 强制各通道切到同频
 *  原: APP_POWER_SetPowerCycleType (line 5185)
 * =================================================================== */
void ctrl_SetPowerCycleType(void)
{
    uint8_t ppg_change = 0;
    uint8_t i;

    for (i = 0; i < s_glb.pot_num; i++) {
        struct cycle_ch_ctx *p = &s_ch[i];

        if (p->power_single) continue;

        if (p->cycle_roll && p->base_duty) {
            ppg_change = 1;
            p->cycle_roll = 0;
            p->ppg_duty_actual = p->base_duty;
        }
    }

    ctrl_GetCycleType(ppg_change);
}


/* ===================================================================
 *  ctrl_ResetPowerCycleType — 恢复倍频
 *  原: APP_POWER_ResetPowerCycleType (line 5206)
 * =================================================================== */
void ctrl_ResetPowerCycleType(void)
{
    uint8_t ppg_change = 0;
    uint8_t i;

    for (i = 0; i < s_glb.pot_num; i++) {
        struct cycle_ch_ctx *p = &s_ch[i];

        if (p->power_single) continue;

        if (p->power_cycle_type) {
            if (s_glb.power_cycle < FRE_30K_PWM * 2) {
                p->power_cycle_type = 0;
                p->cycle_roll = 0;
            } else if (p->cycle_roll == 0) {
                ppg_change = 1;
                p->cycle_roll = 1;
                p->ppg_duty_actual = MAX_FRE_PWM;
            }
        }
    }

    ctrl_GetCycleType(ppg_change);
}


/* ===================================================================
 *  ctrl_CycleGetPowerCycle — 读取当前全局周期
 * =================================================================== */
uint16_t ctrl_CycleGetPowerCycle(void)
{
    return s_glb.power_cycle;
}


/* ===================================================================
 *  ctrl_CycleGetChPowerCycle — 读取指定通道周期
 * =================================================================== */
uint16_t ctrl_CycleGetChPowerCycle(uint8_t ch)
{
    if (ch >= CTRL_CH_MAX) return 0;
    return s_ch[ch].power_cycle;
}


/* ===================================================================
 *  ctrl_CycleGetChDuty — 读取指定通道占空比
 * =================================================================== */
uint16_t ctrl_CycleGetChDuty(uint8_t ch)
{
    if (ch >= CTRL_CH_MAX) return 0;
    return s_ch[ch].ppg_duty_actual;
}


/* ===================================================================
 *  ctrl_CycleInit — 初始化 (由 ctrl_PowerRun 或使用者调用)
 * =================================================================== */
void ctrl_CycleInit(uint8_t pot_num)
{
    uint8_t i;
    s_glb.pot_num = (pot_num > CTRL_CH_MAX) ? CTRL_CH_MAX : pot_num;
    s_glb.power_cycle = 0;
    s_glb.cycle_change_cnt = 0;
    s_glb.change_status = 0;

    for (i = 0; i < s_glb.pot_num; i++) {
        struct cycle_ch_ctx *p = &s_ch[i];
        p->ppg_duty_actual = START_FRE_PWM;
        p->base_duty = 0;
        p->power_cycle = START_FRE_PWM * 2;
        p->cycle_roll = 0;
        p->power_single = 0;
        p->power_cycle_type = 0;
        p->duty50 = 0;
    }
}


/* ===================================================================
 *  ctrl_CycleUpdateDuty — 由 ctrl_power 在 PID 计算后更新 duty
 * =================================================================== */
void ctrl_CycleUpdateDuty(uint8_t ch, uint16_t duty)
{
    if (ch >= CTRL_CH_MAX) return;
    s_ch[ch].ppg_duty_actual = duty;
}


/* ===================================================================
 *  ctrl_CycleGetChangeStatus — 读取当前切换状态并清零
 * =================================================================== */
uint8_t ctrl_CycleGetChangeStatus(void)
{
    uint8_t st = s_glb.change_status;
    s_glb.change_status = 0;
    return st;
}
