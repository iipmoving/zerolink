/* timer_module.c — 简易定时器模块实现
 *
 * 管理四种超时/定时逻辑:
 *   1. 选中超时: 炉头进入 SELECTING 后 15s 无操作 → 自动确认
 *   2. 定时设置超时: 进入 timer_setting 后 15s 无操作 → 自动确认
 *   3. 强火超时: Boost 持续 5min → 自动退出
 *   4. 倒计时: timer_active 下每秒递减 → 归零触发 TimerExpired
 *
 * 所有时间阈值从 hmi_cfg.timeouts[] 读取 (单位 ms; 内部除 100 转 tick)。
 *
 * 简化说明: 本模块不包含 deep_sleep 守卫、all_idle 守卫等复杂检查
 * (这些保留在 state_module 中)。
 */

#include "modules/timer_module.h"
#include "cfg/hmi_data.h"

/* ================================================================
 * 模块静态状态
 * ================================================================ */

/* 全局 100ms tick 计数器, 由 Timer_OnTick() 递增 */
static uint32_t s_timer_tick;

/* 每个炉头的定时器状态缓存 (由 Timer_OnStateChange 从 HmiHead_t 同步) */
static struct {
    uint8_t  node;              /* HmiZoneNode_t: IDLE/SELECTING/COOKING   */
    uint8_t  boost_active;      /* 强火标志                                */
    uint8_t  timer_setting;     /* 定时设置模式                            */
    uint8_t  timer_active;      /* 倒计时运行中                            */
    uint16_t timer_value;       /* 倒计时剩余值 (秒, 每秒减1)              */
    uint32_t select_ticks;      /* 进入 SELECTING 时的 tick 快照           */
    uint32_t timer_set_ticks;   /* 进入 timer_setting 时的 tick 快照       */
    int32_t  boost_remaining_ms;/* Boost 剩余毫秒数                        */
} s_timer_heads[4];

/* 全局模式缓存 (由外部设置, 本模块暂不主动使用) */
static uint8_t s_global_mode;
static uint8_t s_paused;
static uint32_t s_idle_ticks;
static uint32_t s_off_ticks;
static uint8_t s_all_idle;

/* ================================================================
 * __weak 输出桩 — 空实现, 链接器若找到强符号则自动替换
 * ================================================================ */

__attribute__((weak))
void HmiState_OnSelectTimeout(uint8_t zone)
{
    (void)zone;
}

__attribute__((weak))
void HmiState_OnTimerExpired(uint8_t zone)
{
    (void)zone;
}

__attribute__((weak))
void HmiState_OnBoostTimeout(uint8_t zone)
{
    (void)zone;
}

/* ================================================================
 * 初始化: 所有状态归零
 * ================================================================ */

void Timer_Init(void)
{
    uint8_t i;

    s_timer_tick = 0;
    s_global_mode = 0;
    s_paused = 0;
    s_idle_ticks = 0;
    s_off_ticks = 0;
    s_all_idle = 0;

    for (i = 0; i < 4; i++) {
        s_timer_heads[i].node              = 0;
        s_timer_heads[i].boost_active      = 0;
        s_timer_heads[i].timer_setting     = 0;
        s_timer_heads[i].timer_active      = 0;
        s_timer_heads[i].timer_value       = 0;
        s_timer_heads[i].select_ticks      = 0;
        s_timer_heads[i].timer_set_ticks   = 0;
        s_timer_heads[i].boost_remaining_ms = 0;
    }
}

/* ================================================================
 * Timer_OnStateChange — 由 state_module 通过 __weak 回调调用
 *
 * 将 HmiHead_t 中与本模块相关的字段同步到 s_timer_heads[] 缓存。
 * 注意: select_ticks / timer_set_ticks / boost_remaining_ms 已在
 * state_module 中维护, 本模块仅做缓存。
 * ================================================================ */

void Timer_OnStateChange(uint8_t zone, const HmiHead_t *head)
{
    s_timer_heads[zone].node              = head->node;
    s_timer_heads[zone].boost_active      = head->boost_active;
    s_timer_heads[zone].timer_setting     = head->timer_setting;
    s_timer_heads[zone].timer_active      = head->timer_active;
    s_timer_heads[zone].timer_value       = head->timer_value;
    s_timer_heads[zone].select_ticks      = head->select_ticks;
    s_timer_heads[zone].timer_set_ticks   = head->timer_set_ticks;
    s_timer_heads[zone].boost_remaining_ms = head->boost_remaining_ms;
}

/* ================================================================
 * Timer_OnTick — 每 100ms 调用一次
 *
 * 检查:
 *   - 选中超时 (select_ticks → OnSelectTimeout)
 *   - 定时设置超时 (timer_set_ticks → OnSelectTimeout 自动确认)
 *   - 强火剩余递减 (boost_remaining_ms → OnBoostTimeout)
 *
 * 超时阈值从 hmi_cfg.timeouts[] 读取 (ms), 内部 /100 换算为 tick。
 * ================================================================ */

void Timer_OnTick(void)
{
    uint8_t  i;
    uint32_t select_tmo_ticks;   /* 选中超时, 以 tick 为单位 */
    uint32_t timer_cfm_ticks;    /* 定时设置超时, 以 tick 为单位 */

    s_timer_tick++;

    /* 从配置读取超时值, 转 ms→tick (ms/100 = tick) */
    select_tmo_ticks = hmi_cfg.timeouts[HMI_TO_SELECT_CONFIRM_MS] / 100;
    timer_cfm_ticks  = hmi_cfg.timeouts[HMI_TO_TIMER_CONFIRM_MS] / 100;

    for (i = 0; i < 4; i++) {
        /* ---- 1. 选中超时 ---- */
        if (s_timer_heads[i].node == HMI_ZONE_SELECTING) {
            if (s_timer_tick - s_timer_heads[i].select_ticks >= select_tmo_ticks) {
                HmiState_OnSelectTimeout(i);
                /* 防止重复触发: 将快照更新为当前 tick */
                s_timer_heads[i].select_ticks = s_timer_tick;
            }
        }

        /* ---- 2. 定时设置超时 → 自动确认 ---- */
        if (s_timer_heads[i].timer_setting) {
            if (s_timer_tick - s_timer_heads[i].timer_set_ticks >= timer_cfm_ticks) {
                /* 定时设置超时同样走 OnSelectTimeout (自动确认) */
                HmiState_OnSelectTimeout(i);
                s_timer_heads[i].timer_set_ticks = s_timer_tick;
            }
        }

        /* ---- 3. 强火超时 ---- */
        if (s_timer_heads[i].boost_active) {
            s_timer_heads[i].boost_remaining_ms -= 100;
            if (s_timer_heads[i].boost_remaining_ms <= 0) {
                HmiState_OnBoostTimeout(i);
                s_timer_heads[i].boost_remaining_ms = 0;
            }
        }
    }
}

/* ================================================================
 * Timer_OnSecondTick — 每秒调用一次
 *
 * 检查每个炉头的倒计时 (timer_active):
 *   - timer_value > 0 → 减 1
 *   - 减到 0 → 触发 HmiState_OnTimerExpired
 * ================================================================ */

void Timer_OnSecondTick(void)
{
    uint8_t i;

    for (i = 0; i < 4; i++) {
        if (s_timer_heads[i].timer_active && s_timer_heads[i].timer_value > 0) {
            s_timer_heads[i].timer_value--;
            if (s_timer_heads[i].timer_value == 0) {
                HmiState_OnTimerExpired(i);
            }
        }
    }
}

/* ================================================================
 * Timer_GetTick — 返回当前 100ms tick 值
 * ================================================================ */

uint32_t Timer_GetTick(void)
{
    return s_timer_tick;
}
