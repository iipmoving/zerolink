// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
#include "../include_io/app_protect_io.h"

static void Init(void);
MODULE_SKELETON(AppProtect);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;
    struct {
        uint8_t appcommmgr   : 1;  /* AppCommMgr 数据就绪 */
    } bits;
} AppProtect_PipeFlags_t;

/* ---- 数据实体（模块私有）---- */
static MODULE_INPUT(AppProtect)*   s_inPara;    // 输入参数实体在ADC， 这里只调用不修改
static MODULE_OUTPUT(AppProtect)  s_outPara;   // 输出参数缓冲区

/* ---- 消费者 last_seq — seq 有效性比对 (空闲SLOT幂等) ---- */
static uint8_t s_last_seq_AppCommMgr = 0xFF;  /* AppCommMgr→AppProtect */

/* ---- 内部 OUTPUT_LINK + PARAMS 实例 ---- */
static MODULE_OUTPUT_PARAMS(AppProtect, AppPower)  s_AppProtectToAppPowerParams;
static MODULE_OUTPUT_LINK(AppProtect, AppPower)  s_AppProtectToAppPowerLink;


/* 用户业务入口: flags.bits 指示哪些管道有新数据 (seq 比对通过)
 * 输出段请对产出数据的 LINK 执行 seq++: out->AppPower_params->seq++; */
static void user_Process(MODULE_INPUT(AppProtect) *in, MODULE_OUTPUT(AppProtect) *out, AppProtect_PipeFlags_t flags)
{
    (void)in; (void)out; (void)flags;
}

static void ProcessInput(void)
{
    MODULE_INPUT(AppProtect) *in  = (MODULE_INPUT(AppProtect)*)g_input.para;
    MODULE_OUTPUT(AppProtect) *out = (MODULE_OUTPUT(AppProtect)*)g_output.para;

    /* === 输入段: seq 有效性比对 === */
    AppProtect_PipeFlags_t flags = {0};
    {
        uint8_t cur_seq = in->AppCommMgr_params->seq;
        if (cur_seq != s_last_seq_AppCommMgr) {
            flags.bits.appcommmgr = 1;
            s_last_seq_AppCommMgr = cur_seq;
        }
    }

    /* === 计算段: 用户业务 === */
    user_Process(in, out, flags);
}

MODULE_EXPORT(AppProtect);

// ===== [END AI GENERATED] =====
#include "core/std_module.h"
#include "../include_io/app_protect_io.h"
#include "app_protect.h"
#include <stddef.h>
/* 输出 — 系统错误 (Switcher 路由到 app_power) */
typedef struct {
    uint8_t  has_err;
    uint8_t  head_idx;
    uint8_t  slave_addr;
    uint16_t fault;
} OutData_t;

typedef struct { uint8_t dummy; } InData_t;

#if 0  /* DEDUP */
#if 0  /* DEDUP */
MODULE_SKELETON(AppProtect);
#endif  /* DEDUP */
#endif  /* DEDUP */

/* 独立声明 — 与 app_comm_mgr.h 的 RegData_t 布局一致 (AI保证) */
#define PROT_REG_COUNT          22u
#define PROT_SLAVE_ADDR_BASE    5u
#define PROT_SLAVE_ADDR_STEP    5u
typedef struct {
    uint8_t  head_index;
    uint8_t  slave_addr;
    uint8_t  online;
    uint16_t regs[PROT_REG_COUNT];
} ProtRegData_t;

/* APP→APP 走 Switcher 路由，不再用 __weak */

/* ========== 炉头保护上下文 ========== */
typedef struct {
    uint8_t        online;          /* 通讯在线状态 */
    uint8_t        init_done;       /* 电源板初始化完成 */
    uint16_t       regs[PROT_REG_COUNT]; /* 寄存器缓存 */
    ProtectFault_t fault;           /* 当前故障 */
    uint8_t        igbt_err_delay;  /* IGBT故障延迟计数(100ms) */
    uint8_t        bot_err_delay;   /* BOT故障延迟计数(100ms) */
    uint8_t        comm_timeout;    /* 通讯超时计数(100ms) */
    uint8_t        data_age;        /* 数据新鲜度(100ms), >COMM_TIMEOUT则离线 */
} ProtectCtx_t;

static ProtectCtx_t s_ctx[PROTECT_HEAD_COUNT];
static uint8_t      s_tick_10ms;
/* s_event 已移除 — 输出走 g_output */

/* ========== 内部: 单炉头IGBT传感器检测 ========== */
static void check_igbt(uint8_t idx)
{
    ProtectCtx_t *ctx = &s_ctx[idx];
    uint16_t ad;

    if (!ctx->online) return;

    ad = ctx->regs[PROTECT_REG_IGBT_AD];

    /* 开路检测: AD值极低 */
    if (ad <= PROTECT_IGBT_OPEN_AD) {
        if (ctx->igbt_err_delay < PROTECT_ERR_DELAY_100MS) {
            ctx->igbt_err_delay++;
        } else if (!ctx->fault.bits.igbt_open) {
            ctx->fault.bits.igbt_open = 1;
        }
        ctx->fault.bits.igbt_short = 0;
        return;
    }

    /* 短路检测: AD值极高 */
    if (ad >= PROTECT_IGBT_SHORT_AD) {
        if (ctx->igbt_err_delay < PROTECT_ERR_DELAY_100MS) {
            ctx->igbt_err_delay++;
        } else if (!ctx->fault.bits.igbt_short) {
            ctx->fault.bits.igbt_short = 1;
        }
        ctx->fault.bits.igbt_open = 0;
        return;
    }

    /* 超温检测 */
    if (ad > PROTECT_IGBT_HIGH_AD) {
        if (ctx->igbt_err_delay < PROTECT_ERR_DELAY_100MS) {
            ctx->igbt_err_delay++;
        } else if (!ctx->fault.bits.igbt_high) {
            ctx->fault.bits.igbt_high = 1;
        }
    } else if (ad < PROTECT_IGBT_RECOVER_AD) {
        /* 温度降到恢复值以下,清除超温 */
        ctx->fault.bits.igbt_high = 0;
    }

    /* 无开路/短路,清除并重置延迟 */
    ctx->fault.bits.igbt_open  = 0;
    ctx->fault.bits.igbt_short = 0;
    ctx->igbt_err_delay = 0;
}

/* ========== 内部: 单炉头BOT传感器检测 ========== */
static void check_bot(uint8_t idx)
{
    ProtectCtx_t *ctx = &s_ctx[idx];
    uint16_t ad;

    if (!ctx->online) return;

    ad = ctx->regs[PROTECT_REG_BOT_AD];

    if (ad <= PROTECT_BOT_OPEN_AD) {
        if (ctx->bot_err_delay < PROTECT_ERR_DELAY_100MS) {
            ctx->bot_err_delay++;
        } else if (!ctx->fault.bits.bot_open) {
            ctx->fault.bits.bot_open = 1;
        }
        ctx->fault.bits.bot_short = 0;
        return;
    }

    if (ad >= PROTECT_BOT_SHORT_AD) {
        if (ctx->bot_err_delay < PROTECT_ERR_DELAY_100MS) {
            ctx->bot_err_delay++;
        } else if (!ctx->fault.bits.bot_short) {
            ctx->fault.bits.bot_short = 1;
        }
        ctx->fault.bits.bot_open = 0;
        return;
    }

    if (ad > PROTECT_BOT_HIGH_AD) {
        if (ctx->bot_err_delay < PROTECT_ERR_DELAY_100MS) {
            ctx->bot_err_delay++;
        } else if (!ctx->fault.bits.bot_high) {
            ctx->fault.bits.bot_high = 1;
        }
    }
    /* BOT高温不可自动恢复(高于recover值=永久锁存) */

    ctx->fault.bits.bot_open  = 0;
    ctx->fault.bits.bot_short = 0;
    ctx->bot_err_delay = 0;
}

/* ========== 内部: 通讯断开检测 ========== */
static void check_comm(uint8_t idx)
{
    ProtectCtx_t *ctx = &s_ctx[idx];

    /* 数据超时未更新则通讯断开 */
    if (ctx->data_age < 0xFFu) {
        ctx->data_age++;
    }

    if (ctx->data_age > PROTECT_COMM_TIMEOUT_100MS) {
        ctx->online = 0;
        if (ctx->comm_timeout < PROTECT_ERR_DELAY_100MS) {
            ctx->comm_timeout++;
        } else if (!ctx->fault.bits.comm_open) {
            ctx->fault.bits.comm_open = 1;
        }
    } else {
        ctx->online = 1;
        ctx->fault.bits.comm_open = 0;
        ctx->comm_timeout = 0;
    }
}

/* ========== 内部: 电压检测 (共用, 取所有炉头最大AD) ========== */
static void check_voltage(void)
{
    uint8_t  i;
    uint16_t max_ad = 0;
    uint8_t  any_online = 0;

    for (i = 0; i < PROTECT_HEAD_COUNT; i++) {
        if (s_ctx[i].online) {
            any_online = 1;
            if (s_ctx[i].regs[PROTECT_REG_VOL_AD] > max_ad) {
                max_ad = s_ctx[i].regs[PROTECT_REG_VOL_AD];
            }
        }
    }

    if (!any_online) return;

    /* 电压AD值与实际电压的映射由电源板完成,
       这里使用AD值比对 (参考程序用Get_AD_TO_Vol转换后比对) */
    for (i = 0; i < PROTECT_HEAD_COUNT; i++) {
        if (s_ctx[i].online) {
            uint16_t vol_ad = s_ctx[i].regs[PROTECT_REG_VOL_AD];
            if (vol_ad > 0xB4u) { /* >260V对应的AD参考值 */
                s_ctx[i].fault.bits.vol_high = 1;
            } else {
                s_ctx[i].fault.bits.vol_high = 0;
            }
            if (vol_ad < 0x73u) { /* <150V对应的AD参考值 */
                s_ctx[i].fault.bits.vol_low = 1;
            } else {
                s_ctx[i].fault.bits.vol_low = 0;
            }
        }
    }
}

/* ========== 内部: 综合硬件故障判断 ========== */
static void check_hardware(uint8_t idx)
{
    ProtectCtx_t *ctx = &s_ctx[idx];

    /* 任一传感器开路/短路 → 硬件故障 */
    if (ctx->fault.bits.igbt_open  || ctx->fault.bits.igbt_short ||
        ctx->fault.bits.bot_open   || ctx->fault.bits.bot_short  ||
        ctx->fault.bits.fan_open) {
        ctx->fault.bits.hardware = 1;
    } else {
        ctx->fault.bits.hardware = 0;
    }
}

/* ========== __weak 接收: 由 app_comm_mgr 直调 ========== */
#if 0  /* DEDUP: AppProtect_OnRegData */
#if 0  /* DEDUP: AppProtect_OnRegData */
void AppProtect_OnRegData(uint16_t param, void *data_ptr)
{
    ProtRegData_t *reg;
    ProtectCtx_t *ctx;
    uint8_t idx;
    uint8_t i;

    reg = (ProtRegData_t *)data_ptr;
    if (reg == NULL) return;

    idx = (uint8_t)(param & 0xFFu);
    if (idx >= PROTECT_HEAD_COUNT) return;

    ctx = &s_ctx[idx];

    /* 更新寄存器缓存 */
    for (i = 0; i < PROT_REG_COUNT; i++) {
        ctx->regs[i] = reg->regs[i];
    }
    ctx->online   = reg->online;
    ctx->init_done = (reg->regs[PROTECT_REG_STATUS] & 0x80u) ? 1u : 0u;
    ctx->data_age  = 0u;  /* 数据刷新,重置超时计数 */
}
#endif  /* DEDUP: AppProtect_OnRegData */
#endif  /* DEDUP: AppProtect_OnRegData */

#if 0  /* DEDUP: ProcessInput */
#if 0  /* DEDUP: ProcessInput */
static void ProcessInput(void) {}
#endif  /* DEDUP: ProcessInput */
#endif  /* DEDUP: ProcessInput */

/* ========== 初始化 ========== */
static void Init(void)
{
    uint8_t i;

    for (i = 0; i < PROTECT_HEAD_COUNT; i++) {
        s_ctx[i].online         = 0u;
        s_ctx[i].init_done      = 0u;
        s_ctx[i].fault.byte     = 0u;
        s_ctx[i].igbt_err_delay = 0u;
        s_ctx[i].bot_err_delay  = 0u;
        s_ctx[i].comm_timeout   = 0u;
        s_ctx[i].data_age       = 0xFFu;
    }
    s_tick_10ms = 0u;
    g_input.para  = &s_inPara;
    g_output.para = &s_outPara;
    memset(&s_outPara, 0, sizeof(s_outPara));
    s_AppProtectToAppPowerLink.params = &s_AppProtectToAppPowerParams;
    s_outPara.AppPower_params         = &s_AppProtectToAppPowerLink;
}

/* ========== 每10ms槽位调用 ========== */
void App_Protect_Run(void)
{
    uint8_t i;

    s_tick_10ms++;
    if (s_tick_10ms < PROTECT_CHECK_PERIOD_100MS) {
        return;
    }
    s_tick_10ms = 0u;

    /* 共用检测: 电压 */
    check_voltage();

    /* 逐炉头检测 */
    for (i = 0; i < PROTECT_HEAD_COUNT; i++) {
        ProtectFault_t old_fault;

        old_fault.byte = s_ctx[i].fault.byte;

        check_comm(i);
        check_igbt(i);
        check_bot(i);
        check_hardware(i);

        /* 故障状态变化时发送消息 */
        if (s_ctx[i].fault.byte != old_fault.byte) {
            OutData_t *o = (OutData_t *)g_output.para;
            o->has_err     = 1;
            o->head_idx    = i;
            o->slave_addr  = PROT_SLAVE_ADDR_BASE + i * PROT_SLAVE_ADDR_STEP;
            o->fault       = s_ctx[i].fault.byte;
        }
    }
}

#if 0  /* DEDUP */
#if 0  /* DEDUP */
void App_Protect_Init(void) { Constructor(); }
#endif  /* DEDUP */
#endif  /* DEDUP */
#if 0  /* DEDUP */
#if 0  /* DEDUP */
MODULE_EXPORT(AppProtect);
#endif  /* DEDUP */
#endif  /* DEDUP */
