/**
 * app_power.c —— 4炉头功率管理与输出控制实现
 *
 * 依赖: app_power.h (仅本模块头文件)
 * 层级: APP —— 应用层功率管理
 * 零外部依赖: 无 Msg_Post/MsgScheduler_Register，无跨模块 include
 *            使用 __weak 回调函数直接调用，链接器接线
 *            所有跨模块函数对由 interface_map.h 文档化管理
 *
 * 参考: 参考程序 User_Main/APP/work_power_out.c
 *
 * 功率输出流程:
 *   目标功率 → IGBT降功率 → 炉面高温停止 →
 *   软启动 → 间断加热 → AppCommMgr_OnPowerCmd(弱符号)
 */
#include "app_power.h"
#include <string.h>

/* ===== 输出通道: 弱符号默认(无接收方时静默丢弃) ===== */
__weak void AppCommMgr_OnPowerCmd(uint16_t param, void *data_ptr)
{
    (void)param;
    (void)data_ptr;
}

/* 独立声明 — 与 app_comm_mgr.c 的 CommPowerCmd_t 布局一致 */
typedef struct {
    uint8_t  head_idx;
    uint16_t power_watt;
} PowerOutput_t;

/* 独立声明 — 与 app_comm_mgr.h 的 RegData_t 布局一致 (AI保证) */
#define POWER_REG_COUNT  22u
typedef struct {
    uint8_t  head_index;
    uint8_t  slave_addr;
    uint8_t  online;
    uint16_t regs[POWER_REG_COUNT];
} PowerRegData_t;

/* ========== 故障位定义(本地,与app_protect.h保持同步) ========== */
#define FAULT_BIT_HARDWARE  14u

/* ProtectEvent_t 结构: head_index(1B) + slave_addr(1B) + fault(2B) */
#define PROTECT_EVENT_FAULT_OFFSET  2u

/* ========== 功率等级表 (参考 DEF_POWER_LV_PCL) ========== */
static const uint16_t s_power_table[] = {
    0,    200,  300,  400,  500,
    1000, 1100, 1200, 1300, 1500,
    2000
};

/* ========== 炉头功率上下文 ========== */
typedef struct {
    PowerState_t   state;
    uint16_t       target_power;
    uint16_t       output_power;
    uint8_t        power_level;
    uint8_t        work_mode;
    uint16_t       target_temp;
    uint8_t        onoff;
    uint16_t       igbt_temp;
    uint16_t       bot_temp;
    uint16_t       vol_ad;
    uint16_t       fault_byte;
    uint8_t        online;
    uint8_t        soft_start_cnt;
    uint16_t       saved_high_power;
    uint8_t        jd_cycle;
    uint8_t        jd_on_ratio;
} PowerCtx_t;

static PowerCtx_t s_pwr[POWER_HEAD_COUNT];
static uint8_t    s_tick_10ms;

/* ========== 输入: 功率控制(强符号,由 app_cooking 调用) ========== */
void AppPower_OnPowerCtrl(uint16_t param, void *data_ptr)
{
    PowerCtrl_t *ctrl;
    PowerCtx_t  *ctx;
    uint8_t      idx;

    ctrl = (PowerCtrl_t *)data_ptr;
    if (ctrl == NULL) return;

    idx = (uint8_t)(param & 0xFFu);
    if (idx >= POWER_HEAD_COUNT) return;

    ctx = &s_pwr[idx];

    ctx->onoff        = ctrl->onoff;
    ctx->target_power = ctrl->target_power;
    ctx->power_level  = ctrl->power_level;
    ctx->work_mode    = ctrl->work_mode;
    ctx->target_temp  = ctrl->target_temp;

    if (!ctrl->onoff) {
        ctx->state = POWER_STA_OFF;
    } else if (ctx->state == POWER_STA_OFF) {
        ctx->state = POWER_STA_RUN;
        ctx->soft_start_cnt = 0;
        ctx->saved_high_power = 0;
    }
}

/* ========== 输入: 系统错误(强符号,由 app_protect 调用) ========== */
void AppPower_OnSystemError(uint16_t param, void *data_ptr)
{
    uint8_t    *raw;
    uint16_t    fault_byte;
    PowerCtx_t *ctx;
    uint8_t     idx;

    if (data_ptr == NULL) return;

    idx = (uint8_t)(param & 0xFFu);
    if (idx >= POWER_HEAD_COUNT) return;

    /* ProtectEvent_t: head_index(1) + slave_addr(1) + fault(2) */
    raw = (uint8_t *)data_ptr;
    fault_byte = raw[PROTECT_EVENT_FAULT_OFFSET]
               | ((uint16_t)raw[PROTECT_EVENT_FAULT_OFFSET + 1u] << 8);

    ctx = &s_pwr[idx];
    ctx->fault_byte = fault_byte;

    if (fault_byte != 0u && ctx->state == POWER_STA_RUN) {
        ctx->state = POWER_STA_ERROR;
    } else if (fault_byte == 0u && ctx->state == POWER_STA_ERROR) {
        ctx->state = POWER_STA_RUN;
    }
}

/* ========== 输入: 寄存器数据(强符号,由 app_comm_mgr 调用) ========== */
void AppPower_OnRegData(uint16_t param, void *data_ptr)
{
    PowerRegData_t *reg;
    PowerCtx_t *ctx;
    uint8_t idx;

    reg = (PowerRegData_t *)data_ptr;
    if (reg == NULL) return;

    idx = (uint8_t)(param & 0xFFu);
    if (idx >= POWER_HEAD_COUNT) return;

    ctx = &s_pwr[idx];
    ctx->online = reg->online;

    ctx->igbt_temp = reg->regs[2];
    ctx->bot_temp  = reg->regs[3];
    ctx->vol_ad    = reg->regs[4];
}

/* ========== 内部: IGBT高温降功率 (简化线性算法) ========== */
static uint16_t igbt_derate(PowerCtx_t *ctx, uint16_t power)
{
    uint16_t temp;
    uint16_t derate_pct;

    if (power == 0u) return 0u;

    temp = ctx->igbt_temp;
    if (temp < POWER_IGBT_DOWN_START) {
        return power;
    }

    if (temp >= POWER_IGBT_LIMIT_TEMP) {
        return 0u;
    }

    derate_pct = 100u - ((temp - POWER_IGBT_DOWN_START) * 100u
                / (POWER_IGBT_LIMIT_TEMP - POWER_IGBT_DOWN_START));
    return (power * derate_pct) / 100u;
}

/* ========== 内部: 炉面高温停止 ========== */
static uint16_t top_temp_stop(PowerCtx_t *ctx, uint16_t power)
{
    uint16_t temp;

    if (power == 0u) return 0u;

    temp = ctx->bot_temp;
    if (temp >= POWER_TOP_STOP_TEMP) {
        ctx->state = POWER_STA_PROTECT;
        return 0u;
    }
    if (ctx->state == POWER_STA_PROTECT
        && temp <= POWER_TOP_RESUME_TEMP) {
        ctx->state = POWER_STA_RUN;
    }
    if (ctx->state == POWER_STA_PROTECT) {
        return 0u;
    }
    return power;
}

/* ========== 内部: 高功率软启动 ========== */
static uint16_t soft_start(PowerCtx_t *ctx, uint16_t power)
{
    if (power <= POWER_HIGH_SOFT_START) {
        ctx->saved_high_power = 0;
        ctx->soft_start_cnt = 0;
        return power;
    }

    if (ctx->saved_high_power != power) {
        ctx->saved_high_power = power;
        ctx->soft_start_cnt = POWER_SOFT_START_TIME;
    }

    if (ctx->soft_start_cnt > 0u) {
        ctx->soft_start_cnt--;
        return POWER_HIGH_SOFT_START;
    }

    return ctx->saved_high_power;
}

/* ========== 内部: 间断加热 (低功率周期控制) ========== */
static uint16_t interrupted_heat(PowerCtx_t *ctx, uint16_t power)
{
    if (power >= 100u) {
        ctx->jd_cycle = 0;
        return power;
    }

    ctx->jd_cycle++;
    if (ctx->jd_cycle >= 60u) {
        ctx->jd_cycle = 0u;
    }

    if (power >= 50u) {
        ctx->jd_on_ratio = 50u;
    } else if (power >= 25u) {
        ctx->jd_on_ratio = 25u;
    } else {
        return 0u;
    }

    if (ctx->jd_cycle < (60u * ctx->jd_on_ratio / 100u)) {
        return 100u;
    }
    return 0u;
}

/* ========== 内部: 发送功率输出(弱符号回调,不关心接收方) ========== */
static void send_power_cmd(uint8_t head_idx, uint16_t power_watt)
{
    static PowerOutput_t s_output;
    s_output.head_idx   = head_idx;
    s_output.power_watt = power_watt;
    AppCommMgr_OnPowerCmd((uint16_t)head_idx, &s_output);
}

/* ========== 内部: 功率等级转瓦数 ========== */
static uint16_t level_to_watt(uint8_t level)
{
    if (level > POWER_LV_MAX) {
        return 0u;
    }
    return s_power_table[level];
}

/* ========== 初始化 ========== */
void App_Power_Init(void)
{
    uint8_t i;

    for (i = 0; i < POWER_HEAD_COUNT; i++) {
        s_pwr[i].state            = POWER_STA_OFF;
        s_pwr[i].target_power     = 0u;
        s_pwr[i].output_power     = 0u;
        s_pwr[i].power_level      = 0u;
        s_pwr[i].work_mode        = 0u;
        s_pwr[i].target_temp      = 0u;
        s_pwr[i].onoff            = 0u;
        s_pwr[i].igbt_temp        = 25u;
        s_pwr[i].bot_temp         = 25u;
        s_pwr[i].vol_ad           = 0u;
        s_pwr[i].fault_byte       = 0u;
        s_pwr[i].online           = 0u;
        s_pwr[i].soft_start_cnt   = 0u;
        s_pwr[i].saved_high_power = 0u;
        s_pwr[i].jd_cycle         = 0u;
        s_pwr[i].jd_on_ratio      = 0u;
    }
    s_tick_10ms = 0u;
}

/* ========== 每10ms槽位调用 ========== */
void App_Power_Run(void)
{
    uint8_t  i;
    uint16_t power;

    s_tick_10ms++;
    if (s_tick_10ms < POWER_RUN_PERIOD_100MS) {
        return;
    }
    s_tick_10ms = 0u;

    for (i = 0; i < POWER_HEAD_COUNT; i++) {
        PowerCtx_t *ctx = &s_pwr[i];

        switch (ctx->state) {
        case POWER_STA_OFF:
        case POWER_STA_IDLE:
            power = 0u;
            break;

        case POWER_STA_ERROR:
            if (ctx->fault_byte & (1u << FAULT_BIT_HARDWARE)) {
                power = 800u;
            } else {
                power = 0u;
            }
            break;

        case POWER_STA_PROTECT:
            power = 0u;
            break;

        case POWER_STA_RUN:
            power = ctx->target_power;
            if (power == 0u && ctx->power_level > 0u) {
                power = level_to_watt(ctx->power_level);
            }

            power = igbt_derate(ctx, power);
            power = top_temp_stop(ctx, power);
            power = soft_start(ctx, power);
            power = interrupted_heat(ctx, power);
            break;

        default:
            power = 0u;
            break;
        }

        ctx->output_power = power;
        send_power_cmd(i, power);
    }
}
