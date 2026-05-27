/**
 * state_module.c — HMI 状态机核心模块 (v2.0 __weak 架构)
 *
 * 拥有: s_heads[4], s_global, s_hot_head, s_select_stack, s_display
 * 输入(强符号): HmiState_OnKey, HmiState_OnSelectTimeout,
 *                HmiState_OnTimerExpired, HmiState_OnBoostTimeout
 * 输出(__weak):  Display_OnStateChange, Timer_OnStateChange,
 *                Display_OnRefresh, Buzzer_OnBeep
 *
 * 层间依赖: app/app_hmi.h (类型), drv/drv_key.h (键码)
 * 不依赖: msg_scheduler, msg_def, drv_display
 */
#include "app/app_hmi.h"
#include "modules/state_module.h"
#include "drv/drv_key.h"
#include "cfg/hmi_data.h"
#include <string.h>

/* ================================================================
 * 一、模块静态状态变量
 * ================================================================ */

static HmiHead_t        s_heads[4];
static HmiGlobalState_t s_global;
static HmiDisplayCache_t s_display;
static int8_t           s_hot_head;        /* -1=无 */
static int8_t           s_select_stack[4]; /* 选择序列栈 */
static uint8_t          s_stack_count;
static uint32_t         s_tick_100ms;      /* 100ms 计数器 */
static uint32_t         s_idle_ticks;      /* 空闲起始 tick */
static uint32_t         s_off_ticks;       /* 关机起始 tick */

/* ================================================================
 * 二、__weak 桩函数 (输出 — 其他模块提供强实现)
 * ================================================================ */

/* 在 state_module.h 中已使用 WEAK 宏声明：
 *   Display_OnStateChange, Timer_OnStateChange
 * 以下是模块内部需要的额外输出桩：
 */

__attribute__((weak))
void Display_OnRefresh(const HmiDisplayCache_t *cache)
{
    (void)cache;
}

__attribute__((weak))
void Buzzer_OnBeep(uint8_t valid)
{
    (void)valid;
}

/* ================================================================
 * 三、前向声明 (全部 static 辅助函数)
 * ================================================================ */

/* 按键映射 */
static int8_t  head_key_to_index(uint8_t key);
static uint8_t head_index_to_key(uint8_t idx);
static int8_t  digit_key_to_level(uint8_t key);
static uint8_t is_head_key(uint8_t key);

/* 路由 */
static const HmiRoute_t *hmi_find_route(const HmiRoute_t *table,
    uint8_t count, uint8_t key, uint8_t event);
static uint8_t hmi_match_dynamic_route(const HmiRoute_t *route,
    uint8_t key, uint8_t event);

/* 动作分发 */
static void hmi_execute_action(HmiAction_t action, int8_t param,
    uint8_t key, uint8_t silent);
static void hmi_apply_actions(const HmiAction_t *actions,
    uint8_t count, uint8_t silent);

/* 孤岛函数 — 全局模式 */
static void go_working(void);
static void go_powered_off(void);
static void enter_deep_sleep(void);
static void toggle_pause(void);
static void toggle_child_lock(void);

/* 孤岛函数 — 炉头选择与功率 */
static void select_head(uint8_t index);
static void confirm_select(int8_t head_idx);
static void handle_power_key(uint8_t level);

/* 孤岛函数 — Boost */
static void enter_boost(void);
static void exit_boost(int8_t idx);
static void exit_boost_set_power(uint8_t level);

/* 孤岛函数 — 定时 */
static void enter_timer_setting(int8_t keep_value);
static void confirm_timer(int8_t head_idx);
static void cancel_timer_setting(void);
static void cancel_timer_active(void);
static void handle_timer_adjust(int8_t delta);
static int8_t find_timer_head(void);

/* 目标解析与栈管理 */
static int8_t  resolve_target(void);
static void    push_to_stack(uint8_t idx);
static void    remove_from_stack(uint8_t idx);
static int8_t  get_stack_top(void);
static void    reassign_hot_head(uint8_t old_idx);
static void    clear_all_heads(void);
static void    reset_idle_timer(void);

/* 显示 */
static void post_display(void);
static void derive_seg_mode(void);
static void derive_seg_blink(void);
static void update_head_display(uint8_t idx);
static void update_all_displays(void);
static void show_dash(void);
static void show_pa(void);
static void show_all_off(void);

/* LED */
static void leds_all_off(void);
static void sync_leds(void);

/* 蜂鸣 */
static void post_buzzer(uint8_t valid);

/* 辅助 */
static uint8_t all_heads_idle(void);
static uint8_t any_timer_setting(void);
static uint8_t any_timer_active(void);
static void    run_power_on_seq_step(void);

/* ================================================================
 * 四、初始化
 * ================================================================ */

void State_Init(void)
{
    uint8_t i;

    memset(s_heads, 0, sizeof(s_heads));
    for (i = 0; i < 4; i++) {
        s_heads[i].node = HMI_ZONE_IDLE;
    }

    memset(&s_global, 0, sizeof(s_global));
    s_global.in_power_on_seq = 1;
    s_global.power_on_step   = 0;
    s_global.mode            = HMI_NODE_POWERED_OFF;

    s_hot_head    = -1;
    s_stack_count = 0;
    for (i = 0; i < 4; i++) {
        s_select_stack[i] = -1;
    }

    s_tick_100ms = 0;
    s_idle_ticks = 0;
    s_off_ticks  = 0;

    memset(&s_display, 0, sizeof(s_display));
    s_display.hot_head_idx = -1;
    s_display.seg_mode = HMI_SEG_MODE_POWER;
    for (i = 0; i < 8; i++) {
        s_display.seg_chars[i] = ' ';
    }

    /* 启动上电序列 */
    run_power_on_seq_step();
}

/* ================================================================
 * 五、按键事件处理 (强符号, 由 key_module 调用)
 * ================================================================ */

void HmiState_OnKey(uint16_t param, uint16_t evt_packed, void *data_ptr)
{
    uint8_t          key;
    uint8_t          evt_raw;
    uint8_t          event_type;
    const HmiRoute_t *route;
    int8_t           target;
    const HmiGlobalNodeCfg_t *global_cfg;
    const HmiZoneCfg_t       *zone_cfg;
    const HmiProcessCfg_t    *proc_cfg;
    HmiHead_t       *h;
    uint8_t          i;
    uint8_t          matched;

    (void)data_ptr;
    (void)evt_packed;

    key = (uint8_t)(param & 0xFFu);
    evt_raw = (uint8_t)((param >> 8) & 0xFFu);

    /* ---- 上电序列中忽略所有按键 ---- */
    if (s_global.in_power_on_seq) {
        post_buzzer(0);
        return;
    }

    /* 只处理 TAP 和 LONG */
    if (evt_raw != KEY_STATE_TAP && evt_raw != KEY_STATE_LONG) {
        return;
    }
    event_type = (evt_raw == KEY_STATE_TAP) ? HMI_EVT_TAP : HMI_EVT_LONG;

    /* ---- 1. 全局路由 ---- */
    if (s_global.mode < HMI_NODE_COUNT) {
        global_cfg = &hmi_cfg.global_nodes[s_global.mode];
        route = hmi_find_route(global_cfg->routes,
            global_cfg->route_count, key, event_type);
        if (route != NULL) {
            hmi_execute_action((HmiAction_t)route->action,
                route->param, key, 0);
            if (s_global.mode == HMI_NODE_WORKING) {
                reset_idle_timer();
            }
            return;
        }
    }

    /* ---- 2. 休眠态拒绝 ---- */
    if (s_global.mode == HMI_NODE_DEEP_SLEEP) {
        post_buzzer(0);
        return;
    }

    /* ---- 3. 关机态拒绝 ---- */
    if (s_global.mode == HMI_NODE_POWERED_OFF) {
        post_buzzer(0);
        return;
    }

    /* ---- 4. 童锁拦截 ---- */
    if (s_global.child_lock) {
        matched = 0;
        for (i = 0; i < hmi_cfg.child_lock_whitelist_len; i++) {
            if (hmi_cfg.child_lock_whitelist[i].key == key
                && hmi_cfg.child_lock_whitelist[i].event == event_type) {
                matched = 1;
                break;
            }
        }
        if (!matched) {
            post_buzzer(0);
            return;
        }
    }

    /* ---- 5. 暂停态拒绝 ---- */
    if (s_global.paused) {
        post_buzzer(0);
        return;
    }

    /* 有效按键 → 重置空闲计时 */
    reset_idle_timer();

    /* ---- 6. 确定目标炉头 ---- */
    target = resolve_target();

    /* ---- 7. 进程路由 ---- */
    if (target >= 0) {
        h = &s_heads[target];

        /* 按活跃进程优先级查找 */
        if (h->timer_setting) {
            proc_cfg = &hmi_cfg.process_nodes[HMI_PROC_TIMER_SETTING];
            route = hmi_find_route(proc_cfg->routes,
                proc_cfg->route_count, key, event_type);
            if (route != NULL) {
                hmi_execute_action((HmiAction_t)route->action,
                    route->param, key, 0);
                return;
            }
        }
        if (h->timer_active) {
            proc_cfg = &hmi_cfg.process_nodes[HMI_PROC_TIMER_ACTIVE];
            route = hmi_find_route(proc_cfg->routes,
                proc_cfg->route_count, key, event_type);
            if (route != NULL) {
                hmi_execute_action((HmiAction_t)route->action,
                    route->param, key, 0);
                return;
            }
        }
        if (h->boost_active) {
            proc_cfg = &hmi_cfg.process_nodes[HMI_PROC_BOOST_ACTIVE];
            route = hmi_find_route(proc_cfg->routes,
                proc_cfg->route_count, key, event_type);
            if (route != NULL) {
                hmi_execute_action((HmiAction_t)route->action,
                    route->param, key, 0);
                return;
            }
        }

        /* 进程拥有按键则不放行到 Zone 路由 */
        if (h->timer_setting || h->timer_active || h->boost_active) {
            if (is_head_key(key) || digit_key_to_level(key) >= 0) {
                /* 头键/数字键: 进程不拥有, 放行到 Zone */
            } else {
                /* TIMER/PLUS/MINUS 等: 进程已拥有, 有效但无匹配则吞掉 */
                post_buzzer(1);
                return;
            }
        }
    }

    /* ---- 8. Zone 路由 ---- */
    if (target >= 0) {
        h = &s_heads[target];
        if (h->node < HMI_ZONE_COUNT) {
            zone_cfg = &hmi_cfg.zone_nodes[h->node];

            /* 线性扫描, 检查动态键 */
            for (i = 0; i < zone_cfg->route_count; i++) {
                route = &zone_cfg->routes[i];
                if (hmi_match_dynamic_route(route, key, event_type)) {
                    hmi_execute_action((HmiAction_t)route->action,
                        route->param, key, 0);
                    return;
                }
            }
        }
    }

    /* ---- 9. 未匹配 ---- */
    post_buzzer(0);
}

/* ================================================================
 * 六、超时/定时回调 (强符号, 由 timer_module 调用)
 * ================================================================ */

void HmiState_OnSelectTimeout(uint8_t zone)
{
    if (zone < 4u && s_heads[zone].node == HMI_ZONE_SELECTING) {
        confirm_select((int8_t)zone);
    }
}

void HmiState_OnTimerExpired(uint8_t zone)
{
    HmiHead_t *h;

    if (zone >= 4u) return;
    h = &s_heads[zone];
    if (!h->timer_active || h->timer_value > 0) return;

    h->timer_active      = 0;
    h->node              = HMI_ZONE_IDLE;
    h->power_level       = 0;
    h->boost_active      = 0;
    h->boost_remaining_ms = 0;

    remove_from_stack((int8_t)zone);
    reassign_hot_head((int8_t)zone);

    sync_leds();
    update_head_display((int8_t)zone);
    post_display();

    Display_OnStateChange(zone, &s_heads[zone], &s_global);
    Timer_OnStateChange(zone, &s_heads[zone]);
}

void HmiState_OnBoostTimeout(uint8_t zone)
{
    if (zone < 4u && s_heads[zone].boost_active) {
        exit_boost((int8_t)zone);
    }
}

/* ================================================================
 * 七、时钟节拍 (每 100ms 由 timer_module 调用)
 * ================================================================ */

void State_OnTick(void)
{
    uint8_t          i;
    HmiHead_t       *h;
    uint32_t         elapsed_ms;
    uint16_t         timeout_ms;
    const HmiGlobalNodeCfg_t *global_cfg;
    const HmiGuard_t *guard;
    uint8_t          g;

    s_tick_100ms++;

    /* 上电序列中: 推进步骤 */
    if (s_global.in_power_on_seq) {
        run_power_on_seq_step();
        return;
    }

    /* Zone 超时: selecting → confirm_select */
    for (i = 0; i < 4; i++) {
        h = &s_heads[i];
        if (h->node == HMI_ZONE_SELECTING && h->select_ticks > 0) {
            timeout_ms = hmi_cfg.timeouts[HMI_TO_SELECT_CONFIRM_MS];
            elapsed_ms = (s_tick_100ms - h->select_ticks) * 100u;
            if (elapsed_ms >= (uint32_t)timeout_ms) {
                hmi_execute_action(HMI_ACT_CONFIRM_SELECT, 0, 0, 1);
            }
        }
    }

    /* 进程超时: timer_setting → confirm_timer */
    for (i = 0; i < 4; i++) {
        h = &s_heads[i];
        if (h->timer_setting && h->timer_set_ticks > 0) {
            timeout_ms = hmi_cfg.timeouts[HMI_TO_TIMER_CONFIRM_MS];
            elapsed_ms = (s_tick_100ms - h->timer_set_ticks) * 100u;
            if (elapsed_ms >= (uint32_t)timeout_ms) {
                confirm_timer((int8_t)i);
            }
        }
    }

    /* 进程超时: boost_active 倒计时 */
    for (i = 0; i < 4; i++) {
        h = &s_heads[i];
        if (h->boost_active && h->boost_remaining_ms > 0) {
            h->boost_remaining_ms -= 100;
            if (h->boost_remaining_ms <= 0) {
                exit_boost((int8_t)i);
            }
        }
    }

    /* Guard 检查: 遍历当前全局模式的 guards */
    if (s_global.mode < HMI_NODE_COUNT) {
        global_cfg = &hmi_cfg.global_nodes[s_global.mode];
        for (g = 0; g < global_cfg->guard_count; g++) {
            guard = &global_cfg->guards[g];
            timeout_ms = hmi_cfg.timeouts[guard->ms_key];
            if (guard->check == HMI_GUARD_ALWAYS) {
                if (s_off_ticks > 0) {
                    elapsed_ms = (s_tick_100ms - s_off_ticks) * 100u;
                    if (elapsed_ms >= (uint32_t)timeout_ms) {
                        hmi_execute_action(
                            (HmiAction_t)guard->action,
                            guard->param, 0, 1);
                        s_off_ticks = 0;
                    }
                }
            } else if (guard->check == HMI_GUARD_ALL_IDLE) {
                if (s_idle_ticks > 0 && all_heads_idle()) {
                    elapsed_ms = (s_tick_100ms - s_idle_ticks) * 100u;
                    if (elapsed_ms >= (uint32_t)timeout_ms) {
                        hmi_execute_action(
                            (HmiAction_t)guard->action,
                            guard->param, 0, 1);
                        s_idle_ticks = 0;
                    }
                }
            }
        }
    }

    sync_leds();
    post_display();
}

/* ================================================================
 * 八、路由查找
 * ================================================================ */

static const HmiRoute_t *hmi_find_route(const HmiRoute_t *table,
    uint8_t count, uint8_t key, uint8_t event)
{
    uint8_t i;
    for (i = 0; i < count; i++) {
        if (table[i].key == (uint8_t)key && table[i].event == event) {
            return &table[i];
        }
    }
    return NULL;
}

static uint8_t hmi_match_dynamic_route(const HmiRoute_t *route,
    uint8_t key, uint8_t event)
{
    if (route->event != event) {
        return 0;
    }
    if (route->key == HMI_KEY_HEAD_SELF) {
        if (s_hot_head < 0) return 0;
        return (key == head_index_to_key((uint8_t)s_hot_head)) ? 1 : 0;
    }
    if (route->key == HMI_KEY_HEAD_OTHER) {
        if (s_hot_head < 0) return 0;
        return (is_head_key(key)
            && key != head_index_to_key((uint8_t)s_hot_head)) ? 1 : 0;
    }
    return (route->key == (uint8_t)key) ? 1 : 0;
}

/* ================================================================
 * 九、动作分发
 * ================================================================ */

static void hmi_execute_action(HmiAction_t action, int8_t param,
    uint8_t key, uint8_t silent)
{
    if (param == HMI_PARAM_DYNAMIC) {
        param = head_key_to_index(key);
        if (param < 0) {
            param = s_hot_head;
        }
    }

    switch (action) {
    /* 全局模式 */
    case HMI_ACT_GO_WORKING:           go_working(); break;
    case HMI_ACT_GO_POWERED_OFF:       go_powered_off(); break;
    case HMI_ACT_GO_DEEP_SLEEP:        enter_deep_sleep(); break;
    case HMI_ACT_TOGGLE_PAUSE:         toggle_pause(); break;
    case HMI_ACT_TOGGLE_CHILD_LOCK:    toggle_child_lock(); break;
    case HMI_ACT_CLEAR_HOTHEAD:        s_hot_head = -1; sync_leds();
                                        post_display(); break;
    case HMI_ACT_RESET_IDLE_TIMER:     reset_idle_timer(); break;
    case HMI_ACT_RESET_OFF_TIMER:      s_off_ticks = s_tick_100ms; break;

    /* Zone */
    case HMI_ACT_SELECT_HEAD:
        if (param >= 0) select_head((uint8_t)param);
        else select_head((uint8_t)head_key_to_index(key));
        break;
    case HMI_ACT_CONFIRM_SELECT:
    case HMI_ACT_CONFIRM_SELECT_IMMEDIATE:
        confirm_select(s_hot_head); break;
    case HMI_ACT_SET_POWER:            handle_power_key((uint8_t)param); break;

    /* Boost */
    case HMI_ACT_ENTER_BOOST:          enter_boost(); break;
    case HMI_ACT_EXIT_BOOST:           exit_boost(s_hot_head); break;
    case HMI_ACT_EXIT_BOOST_SET_POWER: exit_boost_set_power((uint8_t)param); break;

    /* Timer */
    case HMI_ACT_ENTER_TIMER_SETTING:  enter_timer_setting(param); break;
    case HMI_ACT_CONFIRM_TIMER:        confirm_timer(find_timer_head()); break;
    case HMI_ACT_CANCEL_TIMER_SETTING: cancel_timer_setting(); break;
    case HMI_ACT_CANCEL_TIMER_ACTIVE:  cancel_timer_active(); break;
    case HMI_ACT_TIMER_ADJUST:         handle_timer_adjust(param); break;

    /* 显示 */
    case HMI_ACT_SHOW_DASH:            show_dash(); break;
    case HMI_ACT_SHOW_PA:              show_pa(); break;
    case HMI_ACT_SHOW_POWER_MODE:      update_all_displays(); post_display(); break;
    case HMI_ACT_DISPLAY_ALL_OFF:      show_all_off(); break;
    case HMI_ACT_CLEAR_ALL_HEADS:      clear_all_heads(); break;
    case HMI_ACT_HOTHEAD_CLEAR:        s_hot_head = -1; s_stack_count = 0; break;

    /* LED */
    case HMI_ACT_LED_POWER_ON:         s_display.leds_power = 1; break;
    case HMI_ACT_LED_POWER_OFF:        s_display.leds_power = 0; break;
    case HMI_ACT_LED_ALL_OFF:          leds_all_off(); break;
    case HMI_ACT_LED_PAUSE_ON:         s_display.leds_pause = 1; break;
    case HMI_ACT_LED_PAUSE_OFF:        s_display.leds_pause = 0; break;
    case HMI_ACT_LED_TIMER_ON:         s_display.leds_timer = 1; break;
    case HMI_ACT_LED_TIMER_OFF:        s_display.leds_timer = 0; break;
    case HMI_ACT_LED_CHILD_LOCK_ON:    s_display.leds_child_lock = 1; break;
    case HMI_ACT_LED_CHILD_LOCK_OFF:   s_display.leds_child_lock = 0; break;

    /* 蜂鸣 */
    case HMI_ACT_BEEP_VALID:           post_buzzer(1); return;
    case HMI_ACT_BEEP_INVALID:         post_buzzer(0); return;

    default: return;
    }

    if (!silent) {
        post_buzzer(1);
    }
}

static void hmi_apply_actions(const HmiAction_t *actions,
    uint8_t count, uint8_t silent)
{
    uint8_t i;
    for (i = 0; i < count; i++) {
        hmi_execute_action(actions[i], 0, 0, silent);
    }
}

/* ================================================================
 * 十、孤岛函数 — 全局模式
 * ================================================================ */

static void go_working(void)
{
    const HmiGlobalNodeCfg_t *old_cfg;
    const HmiGlobalNodeCfg_t *new_cfg;

    if (s_global.mode < HMI_NODE_COUNT) {
        old_cfg = &hmi_cfg.global_nodes[s_global.mode];
        if (old_cfg->exit_actions != NULL && old_cfg->exit_count > 0) {
            hmi_apply_actions(old_cfg->exit_actions,
                old_cfg->exit_count, 1);
        }
    }

    s_global.mode       = HMI_NODE_WORKING;
    s_global.paused     = 0;
    s_global.child_lock = 0;
    s_idle_ticks        = s_tick_100ms;
    s_off_ticks         = 0;

    new_cfg = &hmi_cfg.global_nodes[HMI_NODE_WORKING];
    if (new_cfg->enter_actions != NULL && new_cfg->enter_count > 0) {
        hmi_apply_actions(new_cfg->enter_actions,
            new_cfg->enter_count, 1);
    }

    sync_leds();
    post_display();
}

static void go_powered_off(void)
{
    const HmiGlobalNodeCfg_t *old_cfg;
    const HmiGlobalNodeCfg_t *new_cfg;

    if (s_global.mode < HMI_NODE_COUNT) {
        old_cfg = &hmi_cfg.global_nodes[s_global.mode];
        if (old_cfg->exit_actions != NULL && old_cfg->exit_count > 0) {
            hmi_apply_actions(old_cfg->exit_actions,
                old_cfg->exit_count, 1);
        }
    }

    s_global.mode    = HMI_NODE_POWERED_OFF;
    s_global.paused  = 0;
    s_idle_ticks     = 0;
    s_off_ticks      = s_tick_100ms;

    new_cfg = &hmi_cfg.global_nodes[HMI_NODE_POWERED_OFF];
    if (new_cfg->enter_actions != NULL && new_cfg->enter_count > 0) {
        hmi_apply_actions(new_cfg->enter_actions,
            new_cfg->enter_count, 1);
    }

    sync_leds();
    post_display();
}

static void enter_deep_sleep(void)
{
    const HmiGlobalNodeCfg_t *old_cfg;
    const HmiGlobalNodeCfg_t *new_cfg;

    if (s_global.mode < HMI_NODE_COUNT) {
        old_cfg = &hmi_cfg.global_nodes[s_global.mode];
        if (old_cfg->exit_actions != NULL && old_cfg->exit_count > 0) {
            hmi_apply_actions(old_cfg->exit_actions,
                old_cfg->exit_count, 1);
        }
    }

    s_global.mode = HMI_NODE_DEEP_SLEEP;
    s_idle_ticks  = 0;
    s_off_ticks   = 0;

    new_cfg = &hmi_cfg.global_nodes[HMI_NODE_DEEP_SLEEP];
    if (new_cfg->enter_actions != NULL && new_cfg->enter_count > 0) {
        hmi_apply_actions(new_cfg->enter_actions,
            new_cfg->enter_count, 1);
    }

    sync_leds();
    post_display();
}

static void toggle_pause(void)
{
    const HmiGlobalNodeCfg_t *paused_cfg;

    s_global.paused = (uint8_t)(s_global.paused ? 0 : 1);
    paused_cfg = &hmi_cfg.global_nodes[HMI_NODE_PAUSED];

    if (s_global.paused) {
        if (paused_cfg->enter_actions != NULL
            && paused_cfg->enter_count > 0) {
            hmi_apply_actions(paused_cfg->enter_actions,
                paused_cfg->enter_count, 1);
        }
    } else {
        if (paused_cfg->exit_actions != NULL
            && paused_cfg->exit_count > 0) {
            hmi_apply_actions(paused_cfg->exit_actions,
                paused_cfg->exit_count, 1);
        }
    }

    sync_leds();
    post_display();
}

static void toggle_child_lock(void)
{
    s_global.child_lock = (uint8_t)(s_global.child_lock ? 0 : 1);
    if (!s_global.child_lock) {
        reset_idle_timer();
    }
    sync_leds();
    post_display();
}

/* ================================================================
 * 十一、孤岛函数 — 炉头选择与功率
 * ================================================================ */

static void select_head(uint8_t index)
{
    HmiHead_t *h;
    HmiHead_t *old;

    if (index >= 4) return;

    h = &s_heads[index];

    /* 按同一炉头 = 手动确认 (仅 selecting 态) */
    if ((int8_t)index == s_hot_head) {
        if (h->node == HMI_ZONE_SELECTING) {
            confirm_select((int8_t)index);
            return;
        }
        h->node         = HMI_ZONE_SELECTING;
        h->select_ticks = s_tick_100ms;
        sync_leds();
        update_all_displays();
        post_display();
        Display_OnStateChange(index, &s_heads[index], &s_global);
        Timer_OnStateChange(index, &s_heads[index]);
        return;
    }

    /* 旧炉头自动确认 */
    if (s_hot_head >= 0) {
        old = &s_heads[s_hot_head];
        if (old->node == HMI_ZONE_SELECTING) {
            old->node        = (old->power_level > 0)
                ? HMI_ZONE_COOKING : HMI_ZONE_IDLE;
            old->select_ticks = 0;
            if (old->power_level > 0) {
                push_to_stack((uint8_t)s_hot_head);
            } else {
                remove_from_stack((uint8_t)s_hot_head);
            }
        }
    }

    s_hot_head       = (int8_t)index;
    h->node          = HMI_ZONE_SELECTING;
    h->select_ticks  = s_tick_100ms;

    sync_leds();
    update_all_displays();
    post_display();
    Display_OnStateChange(index, &s_heads[index], &s_global);
    Timer_OnStateChange(index, &s_heads[index]);
}

static void confirm_select(int8_t head_idx)
{
    HmiHead_t *h;

    if (head_idx < 0 || head_idx >= 4) return;

    h = &s_heads[head_idx];
    h->node        = (h->power_level > 0)
        ? HMI_ZONE_COOKING : HMI_ZONE_IDLE;
    h->select_ticks = 0;

    if (h->power_level > 0) {
        push_to_stack((uint8_t)head_idx);
    } else {
        remove_from_stack((uint8_t)head_idx);
        reassign_hot_head((uint8_t)head_idx);
    }

    sync_leds();
    update_all_displays();
    post_display();
    Display_OnStateChange((uint8_t)head_idx, &s_heads[head_idx], &s_global);
    Timer_OnStateChange((uint8_t)head_idx, &s_heads[head_idx]);
}

static void handle_power_key(uint8_t level)
{
    int8_t    target;
    HmiHead_t *h;

    target = resolve_target();
    if (target < 0) {
        post_buzzer(0);
        return;
    }

    h = &s_heads[target];

    if (level > 9) return;

    /* Boost 中按档位键 → 退出 Boost */
    if (h->boost_active) {
        h->boost_active      = 0;
        h->boost_remaining_ms = 0;
        if (level == 0) {
            h->power_level = h->original_power;
        }
    }

    if (level == 0) {
        h->power_level = 0;
        if (h->node == HMI_ZONE_SELECTING) {
            h->select_ticks = s_tick_100ms;
        } else {
            h->node          = HMI_ZONE_IDLE;
            h->timer_active  = 0;
            h->timer_setting = 0;
            h->timer_set_ticks = 0;
        }
        remove_from_stack((uint8_t)target);
        reassign_hot_head((uint8_t)target);
    } else {
        h->power_level = level;
        if (h->node == HMI_ZONE_SELECTING) {
            h->select_ticks = s_tick_100ms;
        } else {
            h->node = HMI_ZONE_COOKING;
            push_to_stack((uint8_t)target);
        }
    }

    sync_leds();
    update_all_displays();
    post_display();
    Display_OnStateChange((uint8_t)target, &s_heads[target], &s_global);
    Timer_OnStateChange((uint8_t)target, &s_heads[target]);
}

/* ================================================================
 * 十二、孤岛函数 — Boost
 * ================================================================ */

static void enter_boost(void)
{
    int8_t    target;
    HmiHead_t *h;
    uint8_t   boost_level;

    target = resolve_target();
    if (target < 0) return;

    h = &s_heads[target];
    h->original_power = h->power_level;
    h->boost_active   = 1;
    h->boost_remaining_ms = (int32_t)hmi_cfg.timeouts[HMI_TO_BOOST_MAX_MS];

    boost_level = hmi_cfg.elements->boost_power_level;
    h->power_level = boost_level;

    sync_leds();
    push_to_stack((uint8_t)target);
    update_all_displays();
    post_display();
    Display_OnStateChange((uint8_t)target, &s_heads[target], &s_global);
    Timer_OnStateChange((uint8_t)target, &s_heads[target]);
}

static void exit_boost(int8_t idx)
{
    HmiHead_t *h;

    if (idx < 0 || idx >= 4) return;

    h = &s_heads[idx];
    h->power_level       = h->original_power;
    h->boost_active      = 0;
    h->boost_remaining_ms = 0;

    if (h->node == HMI_ZONE_SELECTING) {
        confirm_select(idx);
    }

    sync_leds();
    update_all_displays();
    post_display();
    Display_OnStateChange((uint8_t)idx, &s_heads[idx], &s_global);
    Timer_OnStateChange((uint8_t)idx, &s_heads[idx]);
}

static void exit_boost_set_power(uint8_t level)
{
    HmiHead_t *h;

    if (s_hot_head < 0 || s_hot_head >= 4) return;

    h = &s_heads[s_hot_head];

    if (!h->boost_active) {
        handle_power_key(level);
        return;
    }

    h->boost_active      = 0;
    h->boost_remaining_ms = 0;
    handle_power_key(level);

    if (h->node == HMI_ZONE_SELECTING) {
        confirm_select(s_hot_head);
    }
}

/* ================================================================
 * 十三、孤岛函数 — 定时
 * ================================================================ */

static void enter_timer_setting(int8_t keep_value)
{
    int8_t    target;
    HmiHead_t *h;

    target = resolve_target();
    if (target < 0) return;

    h = &s_heads[target];
    if (h->node != HMI_ZONE_SELECTING && h->node != HMI_ZONE_COOKING) {
        return;
    }

    h->timer_active     = 0;
    h->timer_setting    = 1;
    if (!keep_value) {
        h->timer_value = hmi_cfg.timeouts[HMI_TO_DEFAULT_TIMER_MIN];
    }
    h->timer_set_ticks  = s_tick_100ms;

    update_all_displays();
    post_display();
    Display_OnStateChange((uint8_t)target, &s_heads[target], &s_global);
    Timer_OnStateChange((uint8_t)target, &s_heads[target]);
}

static void confirm_timer(int8_t head_idx)
{
    HmiHead_t *h;

    if (head_idx < 0 || head_idx >= 4) return;

    h = &s_heads[head_idx];
    h->timer_setting    = 0;
    h->timer_active     = 1;
    h->timer_set_ticks  = 0;

    if (h->node == HMI_ZONE_SELECTING) {
        h->node        = (h->power_level > 0)
            ? HMI_ZONE_COOKING : HMI_ZONE_IDLE;
        h->select_ticks = 0;
        if (h->power_level > 0) {
            push_to_stack((uint8_t)head_idx);
        } else {
            remove_from_stack((uint8_t)head_idx);
            reassign_hot_head((uint8_t)head_idx);
        }
    }

    sync_leds();
    update_all_displays();
    post_display();
    Display_OnStateChange((uint8_t)head_idx, &s_heads[head_idx], &s_global);
    Timer_OnStateChange((uint8_t)head_idx, &s_heads[head_idx]);
}

static void cancel_timer_setting(void)
{
    int8_t    target;
    HmiHead_t *h;

    target = find_timer_head();
    if (target < 0) return;

    h = &s_heads[target];
    if (!h->timer_setting) return;

    h->timer_setting    = 0;
    h->timer_value      = 0;
    h->timer_set_ticks  = 0;

    sync_leds();
    update_all_displays();
    post_display();
    Display_OnStateChange((uint8_t)target, &s_heads[target], &s_global);
    Timer_OnStateChange((uint8_t)target, &s_heads[target]);
}

static void cancel_timer_active(void)
{
    int8_t    target;
    HmiHead_t *h;

    target = find_timer_head();
    if (target < 0) return;

    h = &s_heads[target];
    if (!h->timer_active) return;

    h->timer_active = 0;
    h->timer_value  = 0;

    sync_leds();
    update_all_displays();
    post_display();
    Display_OnStateChange((uint8_t)target, &s_heads[target], &s_global);
    Timer_OnStateChange((uint8_t)target, &s_heads[target]);
}

static void handle_timer_adjust(int8_t delta)
{
    int8_t    target;
    HmiHead_t *h;
    uint8_t   t_min, t_max, t_step;
    int16_t   new_val;

    target = resolve_target();
    if (target < 0) return;

    h = &s_heads[target];
    if (!h->timer_setting) return;

    t_step = hmi_cfg.elements->timer_adjust_step;
    t_min  = hmi_cfg.elements->timer_adjust_min;
    t_max  = hmi_cfg.elements->timer_adjust_max;

    new_val = (int16_t)h->timer_value + (int16_t)delta * (int16_t)t_step;

    if (new_val < (int16_t)t_min) {
        new_val = (int16_t)t_min;
    }
    if (new_val > (int16_t)t_max) {
        new_val = (int16_t)t_max;
    }

    h->timer_value      = (uint16_t)new_val;
    h->timer_set_ticks  = s_tick_100ms;

    update_all_displays();
    post_display();
    Display_OnStateChange((uint8_t)target, &s_heads[target], &s_global);
    Timer_OnStateChange((uint8_t)target, &s_heads[target]);
}

static int8_t find_timer_head(void)
{
    return s_hot_head;
}

/* ================================================================
 * 十四、孤岛函数 — 目标解析与栈管理
 * ================================================================ */

static int8_t resolve_target(void)
{
    HmiHead_t *hh;
    uint8_t   i;
    uint8_t   cooking_count;
    int8_t    last_cooking;

    if (s_hot_head >= 0) {
        hh = &s_heads[s_hot_head];
        if (hh->node != HMI_ZONE_IDLE) {
            return s_hot_head;
        }
    }

    cooking_count = 0;
    last_cooking  = -1;
    for (i = 0; i < 4; i++) {
        if (s_heads[i].node != HMI_ZONE_IDLE) {
            cooking_count++;
            last_cooking = (int8_t)i;
        }
    }

    if (cooking_count == 1) {
        s_hot_head = last_cooking;
        return last_cooking;
    }

    return -1;
}

static void push_to_stack(uint8_t idx)
{
    uint8_t i, j;
    uint8_t max_depth;

    for (i = 0; i < s_stack_count; i++) {
        if (s_select_stack[i] == (int8_t)idx) {
            for (j = i; j < s_stack_count - 1; j++) {
                s_select_stack[j] = s_select_stack[j + 1];
            }
            s_select_stack[s_stack_count - 1] = (int8_t)idx;
            return;
        }
    }

    s_select_stack[s_stack_count] = (int8_t)idx;
    s_stack_count++;

    max_depth = hmi_cfg.elements->stack_max_depth;
    if (s_stack_count > max_depth) {
        for (i = 0; i < s_stack_count - 1; i++) {
            s_select_stack[i] = s_select_stack[i + 1];
        }
        s_stack_count--;
    }
}

static void remove_from_stack(uint8_t idx)
{
    uint8_t i, j;
    for (i = 0; i < s_stack_count; i++) {
        if (s_select_stack[i] == (int8_t)idx) {
            for (j = i; j < s_stack_count - 1; j++) {
                s_select_stack[j] = s_select_stack[j + 1];
            }
            s_stack_count--;
            s_select_stack[s_stack_count] = -1;
            return;
        }
    }
}

static int8_t get_stack_top(void)
{
    if (s_stack_count == 0) return -1;
    return s_select_stack[s_stack_count - 1];
}

static void reassign_hot_head(uint8_t old_idx)
{
    int8_t top;

    if ((int8_t)old_idx != s_hot_head) return;
    if (s_heads[old_idx].node != HMI_ZONE_IDLE) return;

    top = get_stack_top();
    if (top >= 0) {
        s_hot_head = top;
    }
}

static void clear_all_heads(void)
{
    uint8_t i;
    for (i = 0; i < 4; i++) {
        s_heads[i].node               = HMI_ZONE_IDLE;
        s_heads[i].power_level        = 0;
        s_heads[i].boost_active       = 0;
        s_heads[i].boost_remaining_ms = 0;
        s_heads[i].timer_setting      = 0;
        s_heads[i].timer_active       = 0;
        s_heads[i].timer_value        = 0;
        s_heads[i].select_ticks       = 0;
        s_heads[i].timer_set_ticks    = 0;
    }

    leds_all_off();
    s_hot_head    = -1;
    s_stack_count = 0;
}

/* ================================================================
 * 十五、显示派生
 * ================================================================ */

static void post_display(void)
{
    derive_seg_mode();
    derive_seg_blink();

    s_display.hot_head_idx = s_hot_head;

    if (s_display.seg_mode == HMI_SEG_MODE_POWER
        || s_display.seg_mode == HMI_SEG_MODE_TIMER_SETTING) {
        update_all_displays();
    }

    Display_OnRefresh(&s_display);
}

static void derive_seg_mode(void)
{
    const HmiModeRule_t *rules;
    uint8_t i;

    rules = hmi_cfg.mode_rules;
    for (i = 0; i < hmi_cfg.mode_rule_count; i++) {
        switch (rules[i].condition) {
        case HMI_MODE_COND_POWERED_OFF:
            if (s_global.mode == HMI_NODE_POWERED_OFF) {
                s_display.seg_mode = rules[i].seg_mode;
                return;
            }
            break;
        case HMI_MODE_COND_DEEP_SLEEP:
            if (s_global.mode == HMI_NODE_DEEP_SLEEP) {
                s_display.seg_mode = rules[i].seg_mode;
                return;
            }
            break;
        case HMI_MODE_COND_PAUSED:
            if (s_global.paused) {
                s_display.seg_mode = rules[i].seg_mode;
                return;
            }
            break;
        case HMI_MODE_COND_ANY_TIMER_SETTING:
            if (any_timer_setting()) {
                s_display.seg_mode = rules[i].seg_mode;
                return;
            }
            break;
        default:
            break;
        }
    }

    s_display.seg_mode = hmi_cfg.elements->mode_default;
}

static void derive_seg_blink(void)
{
    HmiHead_t *h;
    uint8_t i;

    if (s_global.paused) {
        for (i = 0; i < 4; i++) {
            s_display.seg_blink[i] =
                hmi_cfg.elements->blink_pause_override;
        }
        return;
    }

    for (i = 0; i < 4; i++) {
        h = &s_heads[i];
        if ((h->node == HMI_ZONE_SELECTING || h->timer_setting)
            && !h->boost_active) {
            s_display.seg_blink[i] = 1;
        } else {
            s_display.seg_blink[i] = 0;
        }
    }
}

static void update_head_display(uint8_t idx)
{
    HmiHead_t *h;
    uint8_t   base;
    uint8_t   tv;
    uint8_t   show_timer;

    h    = &s_heads[idx];
    base = idx * 2;

    show_timer = 0;

    if (h->timer_setting) {
        show_timer = 1;
    } else if (h->timer_active) {
        show_timer = ((s_tick_100ms / 50u) & 1u);
    }

    if (show_timer) {
        tv = (uint8_t)h->timer_value;
        if (tv >= 10) {
            s_display.seg_chars[base]     = (char)('0' + (tv / 10));
            s_display.seg_chars[base + 1] = (char)('0' + (tv % 10));
        } else {
            s_display.seg_chars[base]     = (char)('0' + tv);
            s_display.seg_chars[base + 1] = ' ';
        }
    } else if (h->boost_active) {
        s_display.seg_chars[base]     = 'P';
        s_display.seg_chars[base + 1] = ' ';
    } else if (h->power_level == 0) {
        s_display.seg_chars[base]     = '0';
        s_display.seg_chars[base + 1] = '0';
    } else {
        s_display.seg_chars[base]     = (char)('0' + h->power_level);
        s_display.seg_chars[base + 1] = ' ';
    }
}

static void update_all_displays(void)
{
    uint8_t i;
    for (i = 0; i < 4; i++) {
        update_head_display(i);
    }
}

static void show_dash(void)
{
    const char *dash;
    uint8_t i;

    dash = hmi_cfg.patterns->dash;
    for (i = 0; i < 8; i++) {
        s_display.seg_chars[i] = dash[i];
    }
}

static void show_pa(void)
{
    const char *pa;
    uint8_t i;

    pa = hmi_cfg.patterns->pa;
    for (i = 0; i < 8; i++) {
        s_display.seg_chars[i] = pa[i];
    }
}

static void show_all_off(void)
{
    const char *off;
    uint8_t i;

    off = hmi_cfg.patterns->off;
    for (i = 0; i < 8; i++) {
        s_display.seg_chars[i] = off[i];
    }
}

/* ================================================================
 * 十六、LED 同步
 * ================================================================ */

static void leds_all_off(void)
{
    uint8_t i;

    s_display.leds_power      = 0;
    s_display.leds_timer      = 0;
    s_display.leds_pause      = 0;
    s_display.leds_child_lock = 0;

    for (i = 0; i < 4; i++) {
        s_display.leds_head_select[i] = 0;
    }
    for (i = 0; i < 10; i++) {
        s_display.leds_power_level[i] = 0;
    }
}

static void sync_leds(void)
{
    HmiHead_t *hh;
    uint8_t   i;

    if (s_global.mode == HMI_NODE_DEEP_SLEEP) {
        s_display.leds_power = 0;
    } else if (s_global.mode == HMI_NODE_POWERED_OFF) {
        s_display.leds_power = ((s_tick_100ms / 5u) & 1u);
    } else {
        s_display.leds_power = 1;
    }

    s_display.leds_timer = any_timer_active() ? 1 : 0;
    s_display.leds_pause = s_global.paused;
    s_display.leds_child_lock = s_global.child_lock;

    for (i = 0; i < 4; i++) {
        s_display.leds_head_select[i] =
            (s_heads[i].node == HMI_ZONE_SELECTING) ? 1 : 0;
    }

    for (i = 0; i < 10; i++) {
        s_display.leds_power_level[i] = 0;
    }

    if (s_hot_head >= 0) {
        hh = &s_heads[s_hot_head];
        if (hh->boost_active) {
            if (hmi_cfg.elements->level_led_mode) {
                s_display.leds_power_level[
                    hmi_cfg.elements->boost_power_level] = 1;
            } else {
                for (i = 0;
                    i <= hmi_cfg.elements->boost_power_level && i < 10;
                    i++) {
                    s_display.leds_power_level[i] = 1;
                }
            }
        } else if (hh->power_level < 10) {
            if (hmi_cfg.elements->level_led_mode) {
                s_display.leds_power_level[hh->power_level] = 1;
            } else {
                for (i = 0; i <= hh->power_level; i++) {
                    s_display.leds_power_level[i] = 1;
                }
            }
        }
    }
}

/* ================================================================
 * 十七、上电序列
 * ================================================================ */

static void run_power_on_seq_step(void)
{
    const HmiPowerOnStep_t *step;
    uint8_t i;

    if (!s_global.in_power_on_seq) return;

    if (s_global.power_on_step >= hmi_cfg.power_on_seq_len) {
        s_global.in_power_on_seq = 0;
        go_powered_off();
        return;
    }

    step = &hmi_cfg.power_on_seq[s_global.power_on_step];

    if (step->goto_node >= 0) {
        s_global.in_power_on_seq = 0;
        if (step->goto_node == HMI_NODE_POWERED_OFF) {
            go_powered_off();
        } else if (step->goto_node == HMI_NODE_WORKING) {
            go_working();
        } else if (step->goto_node == HMI_NODE_DEEP_SLEEP) {
            enter_deep_sleep();
        } else if (step->goto_node == HMI_NODE_PAUSED) {
            go_powered_off();
        }
        return;
    }

    if (s_global.power_on_step_ticks == 0) {
        s_display.seg_mode = step->seg_mode;

        for (i = 0; i < 8; i++) {
            if (step->seg_chars[i] != '\0') {
                s_display.seg_chars[i] = step->seg_chars[i];
            } else {
                s_display.seg_chars[i] = ' ';
            }
        }

        if (step->leds_all == HMI_LEDS_ALL_ON) {
            s_display.leds_power      = 1;
            s_display.leds_timer      = 1;
            s_display.leds_pause      = 1;
            s_display.leds_child_lock = 1;
            for (i = 0; i < 4; i++)  s_display.leds_head_select[i] = 1;
            for (i = 0; i < 10; i++) s_display.leds_power_level[i] = 1;
        } else if (step->leds_all == HMI_LEDS_ALL_OFF) {
            leds_all_off();
        }

        post_display();
    }

    s_global.power_on_step_ticks++;

    if (step->delay_ms > 0) {
        if ((uint32_t)s_global.power_on_step_ticks * 100u
            >= (uint32_t)step->delay_ms) {
            s_global.power_on_step++;
            s_global.power_on_step_ticks = 0;
        }
    } else {
        s_global.power_on_step++;
        s_global.power_on_step_ticks = 0;
    }
}

/* ================================================================
 * 十八、辅助函数
 * ================================================================ */

static void reset_idle_timer(void)
{
    s_idle_ticks = s_tick_100ms;
    s_off_ticks  = 0;
}

static uint8_t all_heads_idle(void)
{
    uint8_t i;
    for (i = 0; i < 4; i++) {
        if (s_heads[i].node != HMI_ZONE_IDLE) return 0;
    }
    return 1;
}

static uint8_t any_timer_setting(void)
{
    uint8_t i;
    for (i = 0; i < 4; i++) {
        if (s_heads[i].timer_setting) return 1;
    }
    return 0;
}

static uint8_t any_timer_active(void)
{
    uint8_t i;
    for (i = 0; i < 4; i++) {
        if (s_heads[i].timer_active) return 1;
    }
    return 0;
}

static void post_buzzer(uint8_t valid)
{
    Buzzer_OnBeep(valid);
}

/* ================================================================
 * 十九、按键→索引映射
 * ================================================================ */

static int8_t head_key_to_index(uint8_t key)
{
    switch (key) {
    case KEY_LEFT_P_SET_UP:   return 0;
    case KEY_RIGHT_P_SET_UP:  return 1;
    case KEY_LEFT_P_SET:      return 2;
    case KEY_RIGHT_P_SET:     return 3;
    default:                  return -1;
    }
}

static uint8_t head_index_to_key(uint8_t idx)
{
    switch (idx) {
    case 0: return KEY_LEFT_P_SET_UP;
    case 1: return KEY_RIGHT_P_SET_UP;
    case 2: return KEY_LEFT_P_SET;
    case 3: return KEY_RIGHT_P_SET;
    default: return KEY_NONE;
    }
}

static int8_t digit_key_to_level(uint8_t key)
{
    if (key >= KEY_POWER_0 && key <= KEY_POWER_9) {
        return (int8_t)(key - KEY_POWER_0);
    }
    return -1;
}

static uint8_t is_head_key(uint8_t key)
{
    return (key == KEY_LEFT_P_SET
        || key == KEY_RIGHT_P_SET
        || key == KEY_LEFT_P_SET_UP
        || key == KEY_RIGHT_P_SET_UP) ? 1 : 0;
}

/* ================================================================
 * 二十、查询函数 (WASM engine_get_* 委托)
 * ================================================================ */

uint8_t State_GetGlobalMode(void)      { return s_global.mode; }
uint8_t State_IsChildLock(void)        { return s_global.child_lock; }
uint8_t State_IsPaused(void)            { return s_global.paused; }
int8_t  State_GetHotHead(void)         { return s_hot_head; }
uint8_t State_GetStackDepth(void)      { return s_stack_count; }

int8_t State_GetStackAt(uint8_t pos)
{
    return (pos < s_stack_count) ? s_select_stack[pos] : -1;
}

uint8_t State_GetZoneNode(uint8_t idx)
{
    return (idx < 4u) ? s_heads[idx].node : 0u;
}

uint8_t State_GetZonePower(uint8_t idx)
{
    return (idx < 4u) ? s_heads[idx].power_level : 0u;
}

uint8_t State_GetZoneBoost(uint8_t idx)
{
    return (idx < 4u) ? s_heads[idx].boost_active : 0u;
}

uint8_t State_GetZoneTimerSetting(uint8_t idx)
{
    return (idx < 4u) ? s_heads[idx].timer_setting : 0u;
}

uint8_t State_GetZoneTimerActive(uint8_t idx)
{
    return (idx < 4u) ? s_heads[idx].timer_active : 0u;
}

uint16_t State_GetZoneTimerValue(uint8_t idx)
{
    return (idx < 4u) ? s_heads[idx].timer_value : 0u;
}

/* Display cache query */
uint8_t State_GetSegChar(uint8_t pos)
{
    return (pos < 8u) ? (uint8_t)s_display.seg_chars[pos] : 0u;
}

uint8_t State_GetSegBlink(uint8_t zone)
{
    return (zone < 4u) ? s_display.seg_blink[zone] : 0u;
}

uint8_t State_GetSegMode(void)              { return s_display.seg_mode; }
uint8_t State_GetLedPower(void)             { return s_display.leds_power; }
uint8_t State_GetLedTimer(void)             { return s_display.leds_timer; }
uint8_t State_GetLedPause(void)             { return s_display.leds_pause; }
uint8_t State_GetLedChildLock(void)         { return s_display.leds_child_lock; }

uint8_t State_GetLedHeadSelect(uint8_t idx)
{
    return (idx < 4u) ? s_display.leds_head_select[idx] : 0u;
}

uint8_t State_GetLedPowerLevel(uint8_t idx)
{
    return (idx < 10u) ? s_display.leds_power_level[idx] : 0u;
}

/* ================================================================
 * 二十一、强制操作 (测试用)
 * ================================================================ */

void State_ForceSelectConfirm(uint8_t idx)
{
    if (idx < 4u && s_heads[idx].node == HMI_ZONE_SELECTING) {
        confirm_select((int8_t)idx);
    }
}

void State_ForceBoostExit(uint8_t idx)
{
    if (idx < 4u && s_heads[idx].boost_active) {
        exit_boost((int8_t)idx);
    }
}

void State_ForceTimerExpire(uint8_t idx)
{
    if (idx < 4u && s_heads[idx].timer_active
        && s_heads[idx].timer_value > 0) {
        s_heads[idx].timer_value        = 0;
        s_heads[idx].timer_active       = 0;
        s_heads[idx].node               = HMI_ZONE_IDLE;
        s_heads[idx].power_level        = 0;
        s_heads[idx].boost_active       = 0;
        s_heads[idx].boost_remaining_ms = 0;
        remove_from_stack((int8_t)idx);
        reassign_hot_head((int8_t)idx);
        update_head_display((int8_t)idx);
    }
}
