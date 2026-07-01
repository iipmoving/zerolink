// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
#include "../include_io/app_power_io.h"

static void Init(void);
MODULE_SKELETON(AppPower);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;
    struct {
        uint8_t appcommmgr   : 1;  /* AppCommMgr 数据就绪 */
        uint8_t appcooking   : 1;  /* AppCooking 数据就绪 */
        uint8_t appprotect   : 1;  /* AppProtect 数据就绪 */
    } bits;
} AppPower_PipeFlags_t;

/* ---- 数据实体（模块私有）---- */
static MODULE_INPUT(AppPower)*   s_inPara;    // 输入参数实体在ADC， 这里只调用不修改
static MODULE_OUTPUT(AppPower)  s_outPara;   // 输出参数缓冲区

/* ---- 消费者 last_seq — seq 有效性比对 (空闲SLOT幂等) ---- */
static uint8_t s_last_seq_AppCommMgr = 0xFF;  /* AppCommMgr→AppPower */
static uint8_t s_last_seq_AppCooking = 0xFF;  /* AppCooking→AppPower */
static uint8_t s_last_seq_AppProtect = 0xFF;  /* AppProtect→AppPower */

/* ---- 内部 OUTPUT_LINK + PARAMS 实例 ---- */
static MODULE_OUTPUT_PARAMS(AppPower, AppHmi)  s_AppPowerToAppHmiParams;
static MODULE_OUTPUT_LINK(AppPower, AppHmi)  s_AppPowerToAppHmiLink;
static MODULE_OUTPUT_PARAMS(AppPower, AppCommMgr)  s_AppPowerToAppCommMgrParams;
static MODULE_OUTPUT_LINK(AppPower, AppCommMgr)  s_AppPowerToAppCommMgrLink;


/* 用户业务入口: flags.bits 指示哪些管道有新数据 (seq 比对通过)
 * 实际定义在用户代码区 (可引用用户变量/函数) */
static void user_Process(MODULE_INPUT(AppPower) *in, MODULE_OUTPUT(AppPower) *out, AppPower_PipeFlags_t flags);

static void ProcessInput(void)
{
    MODULE_INPUT(AppPower) *in  = (MODULE_INPUT(AppPower)*)g_input.para;
    MODULE_OUTPUT(AppPower) *out = (MODULE_OUTPUT(AppPower)*)g_output.para;

    /* === 输入段: seq 有效性比对 === */
    AppPower_PipeFlags_t flags = {0};
    {
        uint8_t cur_seq = in->AppCommMgr_params->seq;
        if (cur_seq != s_last_seq_AppCommMgr) {
            flags.bits.appcommmgr = 1;
            s_last_seq_AppCommMgr = cur_seq;
        }
    }
    {
        uint8_t cur_seq = in->AppCooking_params->seq;
        if (cur_seq != s_last_seq_AppCooking) {
            flags.bits.appcooking = 1;
            s_last_seq_AppCooking = cur_seq;
        }
    }
    {
        uint8_t cur_seq = in->AppProtect_params->seq;
        if (cur_seq != s_last_seq_AppProtect) {
            flags.bits.appprotect = 1;
            s_last_seq_AppProtect = cur_seq;
        }
    }

    /* === 计算段: 用户业务 === */
    user_Process(in, out, flags);
}

MODULE_EXPORT(AppPower);

// ===== [END AI GENERATED] =====
#include "core/std_module.h"
#include "../include_io/app_power_io.h"
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
static PowerCtx_t     s_pwr[POWER_HEAD_COUNT];
static uint8_t        s_tick_10ms;
static uint16_t       s_power_table[] = {
    0, 200, 300, 400, 500, 1000, 1100, 1200, 1300, 1500, 2000
};

/* =================================================================
 * 骨架
 * ================================================================= */



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

/* ========== route 分发: 替代 V1 OnCookingData/OnProtectData/OnCommMgrData ========== */
static void user_Process(MODULE_INPUT(AppPower) *in, MODULE_OUTPUT(AppPower) *out, AppPower_PipeFlags_t flags)
{
    uint8_t  i;
    uint16_t power;

    /* === 输入段: 按 flag 分支 === */
    if (flags.bits.appcooking) {
        MODULE_INPUT_PARAMS(AppCooking, AppPower) *p = in->AppCooking_params->params;
        uint8_t idx = p->head_index;
        if (idx < POWER_HEAD_COUNT) {
            PowerCtx_t *ctx = &s_pwr[idx];
            ctx->onoff        = (p->cooking_state > 0u) ? 1u : 0u;
            ctx->target_power = p->target_power;
            ctx->power_level  = p->power_level;
            if (!ctx->onoff)        ctx->state = 0u;
            else if (ctx->state == 0u) { ctx->state = 2u; ctx->soft_start_cnt = 0u; ctx->saved_high_power = 0u; }
        }
    }
    if (flags.bits.appprotect) {
        MODULE_INPUT_PARAMS(AppProtect, AppPower) *p = in->AppProtect_params->params;
        uint8_t idx = p->head_index;
        if (idx < POWER_HEAD_COUNT) {
            PowerCtx_t *ctx = &s_pwr[idx];
            ctx->fault_byte = p->fault;
            if (p->fault && ctx->state == 2u)   ctx->state = 4u;
            if (!p->fault && ctx->state == 4u)  ctx->state = 2u;
        }
    }
    if (flags.bits.appcommmgr) {
        MODULE_INPUT_PARAMS(AppCommMgr, AppPower) *p = in->AppCommMgr_params->params;
        uint8_t idx = p->head_index;
        if (idx < POWER_HEAD_COUNT) {
            PowerCtx_t *ctx = &s_pwr[idx];
            ctx->online    = p->online;
            ctx->igbt_temp = p->regs[2];
            ctx->bot_temp  = p->regs[3];
            ctx->vol_ad    = p->regs[4];
        }
    }

    /* === 定时段: 100ms 节拍 === */
    s_tick_10ms++;
    if (s_tick_10ms < POWER_RUN_PERIOD_100MS) return;
    s_tick_10ms = 0u;

    for (i = 0u; i < POWER_HEAD_COUNT; i++) {
        PowerCtx_t *ctx = &s_pwr[i];

        switch (ctx->state) {
        case 0u: case 1u: power = 0u; break;
        case 4u: power = (ctx->fault_byte & 0x4000u) ? 800u : 0u; break;
        case 3u: power = 0u; break;
        case 2u:
            power = ctx->target_power;
            if (power == 0u && ctx->power_level > 0u)
                power = level_to_watt(ctx->power_level);
            power = igbt_derate(ctx, power);
            power = top_temp_stop(ctx, power);
            power = soft_start(ctx, power);
            power = interrupted_heat(ctx, power);
            break;
        default: power = 0u; break;
        }

        ctx->output_power = power;

        /* 写 LINK 输出 */
        s_AppPowerToAppHmiParams.head_index    = i;
        s_AppPowerToAppHmiParams.power_on      = (power > 0u) ? 1u : 0u;
        s_AppPowerToAppHmiParams.power_level   = ctx->power_level;
        s_AppPowerToAppHmiParams.actual_power  = power;
        out->AppHmi_params->seq++;

        s_AppPowerToAppCommMgrParams.head_idx      = i;
        s_AppPowerToAppCommMgrParams.power_on      = (power > 0u) ? 1u : 0u;
        s_AppPowerToAppCommMgrParams.target_power  = power;
        out->AppCommMgr_params->seq++;
    }
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
    g_input.para  = &s_inPara;
    g_output.para = &s_outPara;
    memset(&s_outPara, 0, sizeof(s_outPara));
    s_AppPowerToAppHmiLink.params     = &s_AppPowerToAppHmiParams;
    s_outPara.AppHmi_params           = &s_AppPowerToAppHmiLink;
    s_AppPowerToAppCommMgrLink.params = &s_AppPowerToAppCommMgrParams;
    s_outPara.AppCommMgr_params       = &s_AppPowerToAppCommMgrLink;
}

/* =================================================================
 * 导出
 * ================================================================= */


