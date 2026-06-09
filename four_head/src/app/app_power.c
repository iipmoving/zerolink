/**
 * @file    app_power.c
 * @brief   4炉头功率管理
 * @layer   APP
 *
 * 输入: PowerCtrl(烹饪→route=1), SystemError(保护→route=2), RegData(遥测→route=3)
 * 输出: PowerCmd(→app_comm_mgr, route=1)
 * 定时: 100ms 执行功率链（IGBT降功率→炉面高温→软启动→间断加热）
 */
#include "core/std_module.h"
#include "app_power.h"
#include <string.h>

/* =================================================================
 * 数据结构
 * ================================================================= */

/* 输入 — 统一存放三个数据源 */
typedef struct {
    uint8_t  src_valid;        /* bit0=ctrl有效, bit1=err有效, bit2=reg有效 */
    uint8_t  ctrl_head;
    uint8_t  ctrl_onoff;
    uint16_t ctrl_power;
    uint8_t  err_head;
    uint16_t err_fault;
    uint8_t  reg_head;
    uint8_t  reg_online;
    uint16_t reg_igbt;
    uint16_t reg_bot;
    uint16_t reg_vol;
} InData_t;

/* 输出 — 4炉头功率命令 */
typedef struct {
    uint8_t  heads_valid;      /* bit0-3 对应 head0-3 有输出 */
    uint8_t  head_idx[4];
    uint16_t power_watt[4];
} OutData_t;

/* 炉头状态上下文 */
typedef struct {
    uint8_t  state;            /* PowerState_t */
    uint16_t target_power;
    uint16_t output_power;
    uint8_t  power_level;
    uint8_t  work_mode;
    uint16_t target_temp;
    uint8_t  onoff;
    uint16_t igbt_temp;
    uint16_t bot_temp;
    uint16_t vol_ad;
    uint16_t fault_byte;
    uint8_t  online;
    uint8_t  soft_start_cnt;
    uint16_t saved_high_power;
    uint8_t  jd_cycle;
    uint8_t  jd_on_ratio;
} PowerCtx_t;

/* =================================================================
 * 静态存储
 * ================================================================= */

static InData_t  s_in;
static OutData_t s_out;
static PowerCtx_t     s_pwr[POWER_HEAD_COUNT];
static uint8_t        s_tick_10ms;
static uint16_t       s_power_table[] = {
    0, 200, 300, 400, 500, 1000, 1100, 1200, 1300, 1500, 2000
};

/* =================================================================
 * 骨架
 * ================================================================= */

MODULE_SKELETON(AppPower);

/* =================================================================
 * 内部函数
 * ================================================================= */

static uint16_t igbt_derate(PowerCtx_t *ctx, uint16_t power)
{
    uint16_t t = ctx->igbt_temp;
    if (power == 0 || t < POWER_IGBT_DOWN_START) return power;
    if (t >= POWER_IGBT_LIMIT_TEMP) return 0;
    return power * (100 - (t - POWER_IGBT_DOWN_START) * 100
           / (POWER_IGBT_LIMIT_TEMP - POWER_IGBT_DOWN_START)) / 100;
}

static uint16_t top_temp_stop(PowerCtx_t *ctx, uint16_t power)
{
    uint16_t t = ctx->bot_temp;
    if (power == 0) return 0;
    if (t >= POWER_TOP_STOP_TEMP)  { ctx->state = 3; return 0; } /* PROTECT */
    if (ctx->state == 3 && t <= POWER_TOP_RESUME_TEMP) ctx->state = 2; /* RUN */
    return (ctx->state == 3) ? 0 : power;
}

static uint16_t soft_start(PowerCtx_t *ctx, uint16_t power)
{
    if (power <= POWER_HIGH_SOFT_START) { ctx->saved_high_power = 0; ctx->soft_start_cnt = 0; return power; }
    if (ctx->saved_high_power != power) { ctx->saved_high_power = power; ctx->soft_start_cnt = POWER_SOFT_START_TIME; }
    if (ctx->soft_start_cnt > 0) { ctx->soft_start_cnt--; return POWER_HIGH_SOFT_START; }
    return ctx->saved_high_power;
}

static uint16_t interrupted_heat(PowerCtx_t *ctx, uint16_t power)
{
    if (power >= 100) { ctx->jd_cycle = 0; return power; }
    ctx->jd_cycle = (ctx->jd_cycle + 1) % 60;
    if (power >= 50) ctx->jd_on_ratio = 50;
    else if (power >= 25) ctx->jd_on_ratio = 25;
    else return 0;
    return (ctx->jd_cycle < 60 * ctx->jd_on_ratio / 100) ? 100 : 0;
}

static uint16_t level_to_watt(uint8_t level)
{
    return (level > POWER_LV_MAX) ? 0 : s_power_table[level];
}

/* =================================================================
 * ProcessInput — 每帧调用，处理输入 + 定时计算
 * ================================================================= */

static void ProcessInput(void)
{
    uint8_t  i;
    uint16_t power;

    /* ====== 输入段 ====== */
    if (g_input.info.status & ST_NEW) {
        InData_t *in = (InData_t *)g_input.para;

        if (in->src_valid & 0x01) {  /* PowerCtrl */
            uint8_t idx = in->ctrl_head;
            if (idx < POWER_HEAD_COUNT) {
                PowerCtx_t *ctx = &s_pwr[idx];
                ctx->onoff        = in->ctrl_onoff;
                ctx->target_power = in->ctrl_power;
                if (!in->ctrl_onoff)      ctx->state = 0;          /* OFF */
                else if (ctx->state == 0) { ctx->state = 2;        /* RUN */
                    ctx->soft_start_cnt = 0; ctx->saved_high_power = 0; }
            }
        }
        if (in->src_valid & 0x02) {  /* SystemError */
            uint8_t idx = in->err_head;
            if (idx < POWER_HEAD_COUNT) {
                PowerCtx_t *ctx = &s_pwr[idx];
                ctx->fault_byte = in->err_fault;
                if (in->err_fault && ctx->state == 2)      ctx->state = 4;   /* ERROR */
                if (!in->err_fault && ctx->state == 4)     ctx->state = 2;   /* →RUN */
            }
        }
        if (in->src_valid & 0x04) {  /* RegData */
            uint8_t idx = in->reg_head;
            if (idx < POWER_HEAD_COUNT) {
                PowerCtx_t *ctx = &s_pwr[idx];
                ctx->online    = in->reg_online;
                ctx->igbt_temp = in->reg_igbt;
                ctx->bot_temp  = in->reg_bot;
                ctx->vol_ad    = in->reg_vol;
            }
        }

        in->src_valid = 0;
        g_input.info.status &= ~ST_NEW;
    }

    /* ====== 定时段：100ms 节拍 ====== */
    s_tick_10ms++;
    if (s_tick_10ms < POWER_RUN_PERIOD_100MS) return;
    s_tick_10ms = 0;

    OutData_t *out = (OutData_t *)g_output.para;
    out->heads_valid = 0;

    for (i = 0; i < POWER_HEAD_COUNT; i++) {
        PowerCtx_t *ctx = &s_pwr[i];

        switch (ctx->state) {
        case 0: case 1: power = 0; break;                     /* OFF/IDLE */
        case 4: power = (ctx->fault_byte & 0x4000) ? 800 : 0; break; /* ERROR: 硬件故障800W */
        case 3: power = 0; break;                               /* PROTECT */
        case 2:                                                 /* RUN */
            power = ctx->target_power;
            if (power == 0 && ctx->power_level > 0)
                power = level_to_watt(ctx->power_level);
            power = igbt_derate(ctx, power);
            power = top_temp_stop(ctx, power);
            power = soft_start(ctx, power);
            power = interrupted_heat(ctx, power);
            break;
        default: power = 0; break;
        }

        ctx->output_power = power;
        out->heads_valid |= (uint8_t)(1u << i);
        out->head_idx[i] = i;
        out->power_watt[i] = power;
    }

    g_output.info.route = g_input.info.route;  /* 透传输入 route */
}

/* =================================================================
 * Init — 绑定 para 指针
 * ================================================================= */

static void Init(void)
{
    uint8_t i;
    for (i = 0; i < POWER_HEAD_COUNT; i++) {
        memset(&s_pwr[i], 0, sizeof(PowerCtx_t));
        s_pwr[i].state     = 0;  /* OFF */
        s_pwr[i].igbt_temp = 25;
        s_pwr[i].bot_temp  = 25;
    }
    s_tick_10ms = 0;
    memset(&s_in, 0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

/* =================================================================
 * 导出
 * ================================================================= */

MODULE_EXPORT(AppPower);

/* =================================================================
 * Consumer 回调: Switcher PULL 路由 → 写 g_input.para + ST_NEW
 * ================================================================= */

/* app_cooking → PowerCtrl */
void AppPower_OnCookingData(Para_Grp_t *pOut)
{
    InData_t *in = (InData_t *)g_input.para;
    /* pOut->para 指向 CookingPowerCmd_Item_t */
    uint8_t *src = (uint8_t *)pOut->para;
    in->ctrl_head   = src[0];   /* head_idx */
    in->ctrl_onoff  = src[1];   /* onoff */
    in->ctrl_power  = *(uint16_t *)&src[2];  /* target_power */
    in->src_valid  |= 0x01;
    g_input.info.status |= ST_NEW;
}

/* app_protect → SystemError */
void AppPower_OnProtectData(Para_Grp_t *pOut)
{
    InData_t *in = (InData_t *)g_input.para;
    uint8_t *src = (uint8_t *)pOut->para;
    in->err_head  = src[0];    /* head_idx */
    in->err_fault = *(uint16_t *)&src[2];  /* fault */
    in->src_valid |= 0x02;
    g_input.info.status |= ST_NEW;
}

/* app_comm_mgr → RegData */
void AppPower_OnCommMgrData(Para_Grp_t *pOut)
{
    InData_t *in = (InData_t *)g_input.para;
    uint8_t *src = (uint8_t *)pOut->para;
    in->reg_head   = src[0];    /* head_idx */
    in->reg_online = src[2];    /* online */
    in->reg_igbt   = *(uint16_t *)&src[10];  /* regs[2] */
    in->reg_bot    = *(uint16_t *)&src[12];  /* regs[3] */
    in->reg_vol    = *(uint16_t *)&src[14];  /* regs[4] */
    in->src_valid |= 0x04;
    g_input.info.status |= ST_NEW;
}
