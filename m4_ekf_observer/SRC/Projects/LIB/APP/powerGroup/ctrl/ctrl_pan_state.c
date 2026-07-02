/* ctrl_pan_state.c — 检锅状态机
 * layer: CTRL (APP logic)
 *
 * 职责: 检锅状态机、移锅检测、无锅故障确认。
 *       自声明 per-channel 状态，不 extern。
 *
 * 迁出自: app_power_claude.c
 *    PowerPanCheckFun   (line 1832)
 *    PanStatusCheck     (line 2074)
 *    s_pan_check_fun    (line 3417)
 *    s_pan_err_adj      (line 3545)
 *    s_check_pan_leave  (line 3595)
 */
#include "ctrl.h"
#include "../drv/drv.h"
#include "API_HRTIM.h"

/* ===== 通道数 ===== */
#define CTRL_PAN_CH_MAX  4

/* ===== 检锅结果常量 ===== */
enum {
    C_POT_MAY   = 0,    /* 检锅中 */
    C_POT_IN    = 1,    /* 确认有锅 */
    C_POT_ERR   = 2,    /* 确认无锅 */
    C_MAIN_ERR  = 3     /* 电路故障 */
};

/* ===== 检锅状态枚举 ===== */
enum {
    PAN_OFF = 0,            /* 关功率 */
    PAN_CHECK,              /* 检锅 */
    PAN_FREQ_PROBE,         /* 频率试探 */
    PAN_POT_IN,             /* 确认有锅 */
    PAN_HEAT_FREQ,          /* 调频加热 */
    PAN_HEAT_DUTY           /* 占空比加热 */
};

/* ===== 自声明 per-channel 状态 ===== */
struct pan_ch_ctx {
    uint8_t  ch;             /* 通道号 */
    uint8_t  state;          /* 检锅状态 (PAN_*) */
    uint8_t  err_cnt;        /* 无锅连续确认次数 */
    uint8_t  leave_cnt;      /* 移锅累积计数 */
    uint16_t pulse_count;    /* 检锅脉冲计数 */
    uint16_t surge_delay;    /* 浪涌延时 (ms*100) */
};

static struct pan_ch_ctx s_ch[CTRL_PAN_CH_MAX];
static uint8_t s_pot_num;


/* ===================================================================
 *  ctrl_PanStateRun — 检锅状态机主入口
 *  原: s_pan_check_fun + PanStatusCheck
 *  每控制周期 (20ms) 调用一次
 * =================================================================== */
void ctrl_PanStateRun(uint8_t ch)
{
    if (ch >= CTRL_PAN_CH_MAX) return;
    struct pan_ch_ctx *p = &s_ch[ch];
    (void)p;

    /* TODO: 完整检锅状态机
     * 依赖: check_pot_in() 结果, PowerMem 功率状态, 硬件定时器
     * 待 Switcher 就绪后接入:
     *   1. 读检锅脉冲计数 (drv_pan_count / drv_fmac)
     *   2. 状态转移: OFF → CHECK → FREQ → HEAT
     *   3. 加热中定期调 s_check_pan_leave() 移锅检测
     */
}


/* ===================================================================
 *  ctrl_PanStateCheckLeave — 移锅检测
 *  原: s_check_pan_leave (line 3595)
 *
 *  加热中检测: 实际功率 < 移锅功率 → 累加 leave_cnt
 *              累加超限 → 返回移锅状态
 * =================================================================== */
void ctrl_PanStateCheckLeave(void)
{
    /* TODO: 移锅检测逻辑
     * 依赖: 实际功率, 电流, PPG, 反压等 PowerMem 数据
     * 待 Switcher 就绪后从 g_output.para 读取
     */
}


/* ===================================================================
 *  ctrl_PanErrAdj — 无锅故障确认
 *  原: s_pan_err_adj (line 3545)
 *
 *  连续 lens 次确认无锅 → 返回 C_POT_ERR
 *  LoadValue==0x0f → 返回 C_MAIN_ERR
 * =================================================================== */
uint8_t ctrl_PanErrAdj(uint8_t ch, uint8_t lens)
{
    if (ch >= CTRL_PAN_CH_MAX) return C_POT_MAY;
    struct pan_ch_ctx *p = &s_ch[ch];

    if (p->leave_cnt >= lens) {
        p->leave_cnt = 0;
        /* LoadValue==0x0f → 电路故障, 待 Switcher 接入 */
        return C_POT_ERR;
    }

    p->leave_cnt++;
    return C_POT_MAY;
}


/* ===================================================================
 *  ctrl_PanInit — 初始化
 * =================================================================== */
void ctrl_PanInit(uint8_t pot_num)
{
    uint8_t i;
    s_pot_num = (pot_num > CTRL_PAN_CH_MAX) ? CTRL_PAN_CH_MAX : pot_num;

    for (i = 0; i < s_pot_num; i++) {
        struct pan_ch_ctx *p = &s_ch[i];
        p->ch = i;
        p->state = PAN_OFF;
        p->err_cnt = 0;
        p->leave_cnt = 0;
        p->pulse_count = 0;
        p->surge_delay = 0;
    }
}


/* ===================================================================
 *  ctrl_PanSetState — 外部设置检锅状态
 * =================================================================== */
void ctrl_PanSetState(uint8_t ch, uint8_t state)
{
    if (ch >= CTRL_PAN_CH_MAX) return;
    s_ch[ch].state = state;
}


/* ===================================================================
 *  ctrl_PanGetState — 读取检锅状态
 * =================================================================== */
uint8_t ctrl_PanGetState(uint8_t ch)
{
    if (ch >= CTRL_PAN_CH_MAX) return PAN_OFF;
    return s_ch[ch].state;
}
