/**
 * app_hmi.c — HMI JSON 驱动引擎实现
 *
 * 从 hmi_cfg 配置表读取路由/超时/规则，驱动状态机。
 * 所有业务规则在 JSON→hmi_data.c 中声明，引擎只做解释。
 *
 * 依赖: app_hmi.h, msg_scheduler.h, drv_key.h, drv_buzzer.h
 * 层级: APP — 通过消息调度器收发
 */

#include "core/std_module.h"
#include "app_hmi.h"
#include <string.h>
#include <stddef.h>

/* ---- 数据结构 ----
 * 输入输出提前规划，按数据源/目的地分组
 *
 * 输入（谁给我）:
 *   drv_key → key (route=1)
 *   main    → tick_100ms (route=2)
 *   main    → tick_1s (route=3)
 *
 * 输出（我给谁）:
 *   drv_display → display (route=1)
 *   drv_buzzer  → buzzer_on (route=2)
 */
typedef struct {
    uint8_t  key_code;
    uint8_t  key_state;
    uint8_t  tick_100ms;
    uint8_t  tick_1s;
} InData_t;

typedef struct {
    uint8_t  has_display;            /* drv_display 消费 */
    uint8_t  has_buzzer;             /* drv_buzzer 消费 */
    uint8_t  buzzer_on;
    uint8_t  res;
    /* 显示缓存 — 36 bytes, 与 drv_display 的 DisplayFrame_t 布局一致 */
    int8_t   hot_head_idx;
    uint8_t  seg_chars[8];
    uint8_t  seg_blink[4];
    uint8_t  seg_mode;
    uint8_t  leds_power;
    uint8_t  leds_timer;
    uint8_t  leds_pause;
    uint8_t  leds_child_lock;
    uint8_t  leds_head_select[4];
    uint8_t  leds_power_level[10];
} OutData_t;

static InData_t  s_in;
static OutData_t s_out;

MODULE_SKELETON();

/* DRV 强符号声明（Switcher 路由调用，APP 不定义 __weak 桩）*/
void DrvDisplay_OnRefresh(uint16_t param, void *data_ptr);

#ifdef HMI_DEBUG_KEYS
__attribute__((weak)) void Drv_Display_SetRawLEDs(uint8_t io8, uint8_t io9, uint8_t io10)
{ (void)io8; (void)io9; (void)io10; }
__attribute__((weak)) void Drv_Display_ShowRawSMG(const char *upper, const char *lower)
{ (void)upper; (void)lower; }
#endif

/* ===== 调试模式: 定义后屏蔽所有HMI逻辑, 仅显示键码+状态 ===== */
/* ================================================================
 * 一、引擎状态变量（全部 static）
 * ================================================================ */
//#define HMI_DEBUG_KEYS
static HmiHead_t        s_heads[4];
static HmiGlobalState_t s_global;
static HmiDisplayCache_t s_display;
static int8_t           s_hot_head;        /* -1=无 */
static int8_t           s_select_stack[4]; /* 选择序列栈 */
static uint8_t          s_stack_count;
static uint32_t         s_tick_100ms;      /* 100ms 计数器 */
static uint32_t         s_idle_ticks;      /* 空闲起始 tick */
static uint32_t         s_off_ticks;       /* 关机起始 tick */

/* 头键 → 炉头索引 (HMI_KEY_LEFT_P_SET→0, HMI_KEY_RIGHT_P_SET→1, etc.) */
static int8_t head_key_to_index(uint8_t key);
/* 炉头索引 → 头键 */
static uint8_t head_index_to_key(uint8_t idx);
/* 数字键 → 档位 (HMI_KEY_POWER_0→0, ... HMI_KEY_POWER_9→9) */
static int8_t digit_key_to_level(uint8_t key);
/* 检查按键是否为头键 */
static uint8_t is_head_key(uint8_t key);

/* ================================================================
 * 二、前向声明
 * ================================================================ */

/* 强符号回调: 由 drv_key / main 直调, 链接器自动接线 */
void AppHmi_OnKey(uint16_t param, void *data_ptr);
void AppHmi_OnTimer100ms(uint16_t param, void *data_ptr);
void AppHmi_OnTimer1s(uint16_t param, void *data_ptr);

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

/* 孤岛函数 */
static void select_head(uint8_t index);
static void confirm_select(int8_t head_idx);
static void handle_power_key(uint8_t level);
static void enter_boost(void);
static void exit_boost(int8_t idx);
static void exit_boost_set_power(uint8_t level);
static void enter_timer_setting(int8_t keep_value);
static void confirm_timer(int8_t head_idx);
static void cancel_timer_setting(void);
static void cancel_timer_active(void);
static void handle_timer_adjust(int8_t delta);
static void go_working(void);
static void go_powered_off(void);
static void enter_deep_sleep(void);
static void toggle_pause(void);
static void toggle_child_lock(void);
static int8_t resolve_target(void);
static int8_t find_timer_head(void);
static void push_to_stack(uint8_t idx);
static void remove_from_stack(uint8_t idx);
static int8_t get_stack_top(void);
static void reassign_hot_head(uint8_t old_idx);
static void clear_all_heads(void);
static void reset_idle_timer(void);

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
static void run_power_on_seq_step(void);

/* ================================================================
 * 三、初始化
 * ================================================================ */

static void Init(void)
{
    uint8_t i;

    /* 清零所有状态 */
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

    /* 清零显示缓存 */
    memset(&s_display, 0, sizeof(s_display));
    s_display.hot_head_idx = -1;   /* 初始无热点炉头 */
    s_display.seg_mode = HMI_SEG_MODE_POWER;
    for (i = 0; i < 8; i++) {
        s_display.seg_chars[i] = ' ';
    }

    /* __weak 自动接线, 无需注册 */

    /* 启动上电序列第0步（调试模式跳过） */
#ifdef HMI_DEBUG_KEYS
    s_global.in_power_on_seq = 0;
    show_all_off();
    DrvDisplay_OnRefresh(0, &s_display);
#else
    run_power_on_seq_step();
#endif

    g_input.para  = &s_in;
    g_output.para = &s_out;
}

static void ProcessInput(void)
{
    /* 当前所有逻辑在回调中处理。ProcessInput 保留供后续 route 分流 */
}

void App_Hmi_Run(void) { /* 保留，main.c 调用 */ }

/* v2.0 桥接: 保留旧入口名 */
void App_Hmi_Init(void) { Constructor(); }

/* ---- 导出 ---- */
MODULE_EXPORT(AppHmi);

/* ================================================================
 * 四、按键 → 路由 → 动作 主流程
 * ================================================================ */

#ifdef HMI_DEBUG_KEYS
/* ===== LED 测试模式 (长按开关进入, +/- 依次点亮每个物理LED) ===== */
/* 按 COM9→COM10→COM11, bit0→bit7 顺序扫描, 跳过空位 */
/* 直写IO字节, 不经过HMI映射 */

#define TEST_LED_COUNT 16u

static uint8_t s_led_test_active;
static uint8_t s_led_test_index;

/* 物理LED表: 按COM扫描顺序排列 */
static const struct {
    uint8_t io; uint8_t mask;
    const char *pos;  /* COM+bit 位置标识 */
} s_phys_leds[TEST_LED_COUNT] = {
    { 8, 0x01, "C9b0" }, { 8, 0x02, "C9b1" }, { 8, 0x04, "C9b2" },
    { 8, 0x08, "C9b3" }, { 8, 0x10, "C9b4" }, { 8, 0x20, "C9b5" },
    { 8, 0x80, "C9b7" },
    { 9, 0x01, "10b0" }, { 9, 0x02, "10b1" },
    { 9, 0x10, "10b4" }, { 9, 0x20, "10b5" },
    {10, 0x01, "11b0" }, {10, 0x02, "11b1" },
    {10, 0x08, "11b3" }, {10, 0x10, "11b4" }, {10, 0x80, "11b7" },
};

/* 仅点亮 idx 指定的一个物理LED, 直写IO */
static void test_led_set(uint8_t idx)
{
    if (idx >= TEST_LED_COUNT) idx = 0u;
    Drv_Display_SetRawLEDs(
        (s_phys_leds[idx].io == 8)  ? s_phys_leds[idx].mask : 0u,
        (s_phys_leds[idx].io == 9)  ? s_phys_leds[idx].mask : 0u,
        (s_phys_leds[idx].io == 10) ? s_phys_leds[idx].mask : 0u
    );
}

/* LED测试界面: 上排 "  XX" (序号), 下排 "C9bX" (COM+bit) */
static void test_led_show(uint8_t idx)
{
    char upper[5];
    char lower[5];

    if (idx >= TEST_LED_COUNT) idx = 0u;

    /* Upper: "  XX" 右对齐序号 */
    upper[0] = ' ';
    if (idx >= 10u) {
        upper[1] = (char)('0' + idx / 10u);
        upper[2] = (char)('0' + idx % 10u);
    } else {
        upper[1] = ' ';
        upper[2] = (char)('0' + idx);
    }
    upper[3] = ' ';
    upper[4] = '\0';

    /* Lower: COM+bit 位置标识, 如 "C9b0" */
    lower[0] = s_phys_leds[idx].pos[0];
    lower[1] = s_phys_leds[idx].pos[1];
    lower[2] = s_phys_leds[idx].pos[2];
    lower[3] = s_phys_leds[idx].pos[3];
    lower[4] = '\0';

    /* 先写SMG, 再置LED */
    Drv_Display_ShowRawSMG(upper, lower);
    test_led_set(idx);
}

static void debug_show_key_event(uint8_t key, uint8_t evt)
{
    static const char hex_chars[] = "0123456789ABCDEF";

    key = key & 0x7Fu;

    // Upper: "KK  " — 每次更新键码
    s_display.seg_chars[0] = hex_chars[(key >> 4) & 0x0Fu];
    s_display.seg_chars[1] = hex_chars[key & 0x0Fu];
    s_display.seg_chars[2] = ' ';
    s_display.seg_chars[3] = ' ';

    // Lower: "P E" — 上下排独立
    if (evt == HMI_KEY_STATE_RELEASE) {
        // RELEASE只更新P→0, E保持上次事件类型不覆盖
        s_display.seg_chars[4] = '0';
    } else {
        if (evt == HMI_KEY_STATE_PRESS || evt == HMI_KEY_STATE_LONG
            || evt == HMI_KEY_STATE_REPEAT) {
            s_display.seg_chars[4] = '1';
        } else {
            s_display.seg_chars[4] = '0';
        }
    }
    s_display.seg_chars[5] = ' ';
    if (evt != HMI_KEY_STATE_RELEASE) {
        if (evt == HMI_KEY_STATE_TAP) {
            s_display.seg_chars[6] = '1';
        } else if (evt == HMI_KEY_STATE_LONG) {
            s_display.seg_chars[6] = '2';
        } else if (evt == HMI_KEY_STATE_REPEAT) {
            s_display.seg_chars[6] = '3';
        } else {
            s_display.seg_chars[6] = ' ';
        }
    }
    s_display.seg_chars[7] = ' ';

    s_display.seg_blink[0] = 0;
    s_display.seg_blink[1] = 0;
    s_display.seg_blink[2] = 0;
    s_display.seg_blink[3] = 0;

    DrvDisplay_OnRefresh(0, &s_display);
}
#endif /* HMI_DEBUG_KEYS */

void AppHmi_OnKey(uint16_t param, void *data_ptr)
{
    uint8_t          key;
    uint8_t          evt;
#ifdef HMI_DEBUG_KEYS
    (void)data_ptr;

    key = (uint8_t)(param & 0xFFu);
    evt = (uint8_t)((param >> 8) & 0xFFu);

    /* ---- LED 测试模式 ---- */
    if (s_led_test_active) {
        if (key == HMI_KEY_ONOFF && evt == HMI_KEY_STATE_LONG) {
            s_led_test_active = 0;
            debug_show_key_event(0, 0);
            return;
        }
        if (key == HMI_KEY_ADD && evt == HMI_KEY_STATE_PRESS) {
            s_led_test_index++;
            if (s_led_test_index >= TEST_LED_COUNT) {
                s_led_test_index = 0;
            }
            test_led_show(s_led_test_index);
            return;
        }
        if (key == HMI_KEY_SUB && evt == HMI_KEY_STATE_PRESS) {
            if (s_led_test_index == 0) {
                s_led_test_index = TEST_LED_COUNT - 1;
            } else {
                s_led_test_index--;
            }
            test_led_show(s_led_test_index);
            return;
        }
        return;
    }

    /* 长按开关 → 进入 LED 测试模式 */
    if (key == HMI_KEY_ONOFF && evt == HMI_KEY_STATE_LONG) {
        s_led_test_active = 1;
        s_led_test_index = 0;
        test_led_show(0);
        return;
    }

    /* 正常调试: 显示键码+状态 */
    debug_show_key_event(key, evt);
    return;
#else
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

    key = (uint8_t)(param & 0xFFu);
    evt = (uint8_t)((param >> 8) & 0xFFu);

    /* ---- 上电序列中忽略所有按键 ---- */
    if (s_global.in_power_on_seq) {
        post_buzzer(0);
        return;
    }

    /* 只处理 TAP 和 LONG */
    if (evt != HMI_KEY_STATE_TAP && evt != HMI_KEY_STATE_LONG) {
        return;
    }
    event_type = (evt == HMI_KEY_STATE_TAP) ? HMI_EVT_TAP : HMI_EVT_LONG;

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
#endif /* HMI_DEBUG_KEYS */
}

/* ================================================================
 * 五、路由查找
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

/* 动态路由匹配: HEAD_SELF/HEAD_OTHER 在运行时解析 */
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

    /* 普通键: 精确匹配 */
    return (route->key == (uint8_t)key) ? 1 : 0;
}

/* ================================================================
 * 六、动作分发
 * ================================================================ */

static void hmi_execute_action(HmiAction_t action, int8_t param,
    uint8_t key, uint8_t silent)
{
    /* 动态参数替换: %head → 实际头索引 */
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
    case HMI_ACT_SHOW_POWER_MODE:      update_all_displays();
                                        post_display(); break;
    case HMI_ACT_DISPLAY_ALL_OFF:      show_all_off(); break;
    case HMI_ACT_CLEAR_ALL_HEADS:      clear_all_heads(); break;
    case HMI_ACT_HOTHEAD_CLEAR:        s_hot_head = -1;
                                        s_stack_count = 0; break;

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
 * 七、孤岛函数 — 全局模式
 * ================================================================ */

static void go_working(void)
{
    const HmiGlobalNodeCfg_t *old_cfg;
    const HmiGlobalNodeCfg_t *new_cfg;

    /* 退出旧模式 */
    if (s_global.mode < HMI_NODE_COUNT) {
        old_cfg = &hmi_cfg.global_nodes[s_global.mode];
        if (old_cfg->exit_actions != NULL && old_cfg->exit_count > 0) {
            hmi_apply_actions(old_cfg->exit_actions,
                old_cfg->exit_count, 1);
        }
    }

    /* 核心状态切换 */
    s_global.mode       = HMI_NODE_WORKING;
    s_global.paused     = 0;
    s_global.child_lock = 0;
    s_idle_ticks        = s_tick_100ms;
    s_off_ticks         = 0;

    /* JSON 声明的进入动作 */
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

    /* 退出旧模式 */
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
 * 八、孤岛函数 — 炉头选择与功率
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
        /* idle/cooking: 重新选中 */
        h->node         = HMI_ZONE_SELECTING;
        h->select_ticks = s_tick_100ms;
        sync_leds();
        update_all_displays();
        post_display();
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
        h->boost_active     = 0;
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
}

/* ================================================================
 * 九、孤岛函数 — Boost
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
 * 十、孤岛函数 — 定时
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
    /* keep_value: 从 timer_active 重进, 保留当前剩余时间 */
    h->timer_set_ticks  = s_tick_100ms;

    update_all_displays();
    post_display();
}

static void confirm_timer(int8_t head_idx)
{
    HmiHead_t *h;

    if (head_idx < 0 || head_idx >= 4) return;

    h = &s_heads[head_idx];
    h->timer_setting    = 0;
    h->timer_active     = 1;
    h->timer_set_ticks  = 0;

    /* 如果还在 selecting 态 → 确认选择 */
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

    h->timer_value     = (uint16_t)new_val;
    h->timer_set_ticks = s_tick_100ms;

    update_all_displays();
    post_display();
}

static int8_t find_timer_head(void)
{
    return s_hot_head;
}

/* ================================================================
 * 十一、孤岛函数 — 目标解析与栈管理
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

    /* 恰好一个非 idle 炉头 → 隐式选中 */
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

    /* 0 或 >1 个 → 拒绝 */
    return -1;
}

static void push_to_stack(uint8_t idx)
{
    uint8_t i, j;
    uint8_t max_depth;

    /* 去重 */
    for (i = 0; i < s_stack_count; i++) {
        if (s_select_stack[i] == (int8_t)idx) {
            /* 移到栈顶 */
            for (j = i; j < s_stack_count - 1; j++) {
                s_select_stack[j] = s_select_stack[j + 1];
            }
            s_select_stack[s_stack_count - 1] = (int8_t)idx;
            return;
        }
    }

    /* 入栈 */
    s_select_stack[s_stack_count] = (int8_t)idx;
    s_stack_count++;

    max_depth = hmi_cfg.elements->stack_max_depth;
    if (s_stack_count > max_depth) {
        /* 移除最旧 */
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
        s_heads[i].node          = HMI_ZONE_IDLE;
        s_heads[i].power_level   = 0;
        s_heads[i].boost_active  = 0;
        s_heads[i].boost_remaining_ms = 0;
        s_heads[i].timer_setting = 0;
        s_heads[i].timer_active  = 0;
        s_heads[i].timer_value   = 0;
        s_heads[i].select_ticks  = 0;
        s_heads[i].timer_set_ticks = 0;
    }

    leds_all_off();
    s_hot_head    = -1;
    s_stack_count = 0;
}

/* ================================================================
 * 十二、显示派生
 * ================================================================ */

static void post_display(void)
{
    /* ModeRule: 优先级链派生 seg_mode */
    derive_seg_mode();

    /* BlinkRule: 从 head 状态派生 seg_blink */
    derive_seg_blink();

    /* 热点炉头索引传递给 DRV 层用于 DP 指示器 */
    s_display.hot_head_idx = s_hot_head;

    /* 仅在区域模式更新每个炉头的段码字符；
     * DASH/OFF/ASCII 模式由 show_dash/show_pa/show_all_off/上电序列预置 */
    if (s_display.seg_mode == HMI_SEG_MODE_POWER
        || s_display.seg_mode == HMI_SEG_MODE_TIMER_SETTING) {
        update_all_displays();
    }

    /* v2.0: 写 g_output — 完整复制显示缓存 */
    s_out.has_display   = 1;
    s_out.hot_head_idx  = s_display.hot_head_idx;
    s_out.seg_mode      = s_display.seg_mode;
    s_out.leds_power    = s_display.leds_power;
    s_out.leds_timer    = s_display.leds_timer;
    s_out.leds_pause    = s_display.leds_pause;
    s_out.leds_child_lock = s_display.leds_child_lock;
    memcpy(s_out.seg_chars, s_display.seg_chars, 8);
    memcpy(s_out.seg_blink, s_display.seg_blink, 4);
    memcpy(s_out.leds_head_select, s_display.leds_head_select, 4);
    memcpy(s_out.leds_power_level, s_display.leds_power_level, 10);
    g_output.info.route = 1;

    /* v1.0 向后兼容 */
    DrvDisplay_OnRefresh(0, &s_display);
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

    /* 回退默认值 */
    s_display.seg_mode = hmi_cfg.elements->mode_default;
}

static void derive_seg_blink(void)
{
    HmiHead_t *h;
    uint8_t i;

    /* 暂停态: 全灭 (pause_override=0) 或保持闪烁 */
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
        show_timer = ((s_tick_100ms / 50u) & 1u);  /* 5s交替 */
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
 * 十三、LED 同步
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

    /* Power LED: JSON elements.led.power states */
    if (s_global.mode == HMI_NODE_DEEP_SLEEP) {
        s_display.leds_power = 0;
    } else if (s_global.mode == HMI_NODE_POWERED_OFF) {
        /* JSON spec: powered_off → blink (500ms phase via 100ms tick) */
        s_display.leds_power = ((s_tick_100ms / 5u) & 1u);
    } else {
        s_display.leds_power = 1;
    }

    /* Timer LED: any timer_active */
    s_display.leds_timer = any_timer_active() ? 1 : 0;

    /* Pause LED */
    s_display.leds_pause = s_global.paused;

    /* Child Lock LED */
    s_display.leds_child_lock = s_global.child_lock;

    /* Head Select LED */
    for (i = 0; i < 4; i++) {
        s_display.leds_head_select[i] =
            (s_heads[i].node == HMI_ZONE_SELECTING) ? 1 : 0;
    }

    /* Power Level LED: hot_head 的档位 */
    for (i = 0; i < 10; i++) {
        s_display.leds_power_level[i] = 0;
    }

    if (s_hot_head >= 0) {
        hh = &s_heads[s_hot_head];
        if (hh->boost_active) {
            if (hmi_cfg.elements->level_led_mode) {
                s_display.leds_power_level[hmi_cfg.elements->boost_power_level] = 1;
            } else {
                for (i = 0; i <= hmi_cfg.elements->boost_power_level && i < 10; i++) {
                    s_display.leds_power_level[i] = 1;
                }
            }
        } else if (hh->power_level < 10) {
            if (hmi_cfg.elements->level_led_mode) {
                /* single: 仅点亮当前档位 */
                s_display.leds_power_level[hh->power_level] = 1;
            } else {
                /* gradient: 0→currentLevel 全亮 */
                for (i = 0; i <= hh->power_level; i++) {
                    s_display.leds_power_level[i] = 1;
                }
            }
        }
    }
}

/* ================================================================
 * 十四、超时管理（每 100ms）
 * ================================================================ */

void AppHmi_OnTimer100ms(uint16_t param, void *data_ptr)
{
#ifdef HMI_DEBUG_KEYS
    (void)param;
    (void)data_ptr;
    return;
#else
    uint8_t          i;
    HmiHead_t       *h;
    uint32_t         elapsed_ms;
    uint16_t         timeout_ms;
    const HmiGlobalNodeCfg_t *global_cfg;
    const HmiGuard_t *guard;
    uint8_t          g;

    (void)param;
    (void)data_ptr;

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
                        s_off_ticks = 0; /* 单次触发 */
                    }
                }
            } else if (guard->check == HMI_GUARD_ALL_IDLE) {
                if (s_idle_ticks > 0 && all_heads_idle()) {
                    elapsed_ms = (s_tick_100ms - s_idle_ticks) * 100u;
                    if (elapsed_ms >= (uint32_t)timeout_ms) {
                        hmi_execute_action(
                            (HmiAction_t)guard->action,
                            guard->param, 0, 1);
                        s_idle_ticks = 0; /* 单次触发 */
                    }
                }
            }
        }
    }
#endif /* HMI_DEBUG_KEYS */
}

/* ================================================================
 * 十五、定时倒计时（每 1s）
 * ================================================================ */

void AppHmi_OnTimer1s(uint16_t param, void *data_ptr)
{
#ifdef HMI_DEBUG_KEYS
    (void)param;
    (void)data_ptr;
    return;
#else
    HmiHead_t *h;
    uint8_t   i;

    (void)param;
    (void)data_ptr;

    if (s_global.in_power_on_seq) return;

    for (i = 0; i < 4; i++) {
        h = &s_heads[i];
        if (h->timer_active && h->timer_value > 0) {
            h->timer_value--;
            if (h->timer_value == 0) {
                h->timer_active    = 0;
                h->node            = HMI_ZONE_IDLE;
                h->power_level     = 0;
                h->boost_active    = 0;
                h->boost_remaining_ms = 0;
                remove_from_stack(i);
                reassign_hot_head(i);
            }
            update_head_display(i);
        }
    }

    sync_leds();
    post_display();
#endif /* HMI_DEBUG_KEYS */
}

/* ================================================================
 * 十六、上电序列
 * ================================================================ */

static void run_power_on_seq_step(void)
{
    const HmiPowerOnStep_t *step;
    uint8_t i;

    if (!s_global.in_power_on_seq) return;

    if (s_global.power_on_step >= hmi_cfg.power_on_seq_len) {
        /* 序列结束 */
        s_global.in_power_on_seq = 0;
        go_powered_off();
        return;
    }

    step = &hmi_cfg.power_on_seq[s_global.power_on_step];

    /* 跳转步骤 (goto) */
    if (step->goto_node >= 0) {
        s_global.in_power_on_seq = 0;
        if (step->goto_node == HMI_NODE_POWERED_OFF) {
            go_powered_off();
        } else if (step->goto_node == HMI_NODE_WORKING) {
            go_working();
        } else if (step->goto_node == HMI_NODE_DEEP_SLEEP) {
            enter_deep_sleep();
        } else if (step->goto_node == HMI_NODE_PAUSED) {
            /* 上电不直接进入暂停，回退到关机 */
            go_powered_off();
        }
        return;
    }

    /* 首次进入当前步骤: 应用显示 */
    if (s_global.power_on_step_ticks == 0) {
        /* seg_mode */
        s_display.seg_mode = step->seg_mode;

        /* seg_chars: 8字符 → 数组 */
        for (i = 0; i < 8; i++) {
            if (step->seg_chars[i] != '\0') {
                s_display.seg_chars[i] = step->seg_chars[i];
            } else {
                s_display.seg_chars[i] = ' ';
            }
        }

        /* LED: all_on / all_off */
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

    /* 计时推进 */
    s_global.power_on_step_ticks++;

    /* 延时到达 → 下一步 */
    if (step->delay_ms > 0) {
        /* 每 100ms tick, 检查是否达到 delay_ms */
        if ((uint32_t)s_global.power_on_step_ticks * 100u
            >= (uint32_t)step->delay_ms) {
            s_global.power_on_step++;
            s_global.power_on_step_ticks = 0;
        }
    } else {
        /* delay_ms=0: 立即跳转下一步 (已在 goto 分支处理) */
        s_global.power_on_step++;
        s_global.power_on_step_ticks = 0;
    }
}

/* ================================================================
 * 十七、辅助函数
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

/* APP 层蜂鸣器 — 输出枚举值 1~N，由中间层映射到 DRV 参数 */
enum {
    BUZZER_INVALID = 0,
    BUZZER_KEY_TAP,      /* 1: 按键短按 */
    BUZZER_KEY_LONG,     /* 2: 按键长按 */
    BUZZER_OP_OK,        /* 3: 操作成功 */
    BUZZER_OP_FAIL,      /* 4: 操作失败 */
    BUZZER_ALARM,        /* 5: 报警 */
};

static void post_buzzer(uint8_t sound)
{
    s_out.has_buzzer = 1;
    s_out.buzzer_on  = sound;
    g_output.info.route = 2;
    /* Switcher _route_hmi 下帧读取 has_buzzer 并映射到 DRV 参数 */
}

/* ================================================================
 * 十八、按键→索引映射
 * ================================================================ */

static int8_t head_key_to_index(uint8_t key)
{
    switch (key) {
    case HMI_KEY_LEFT_P_SET_UP:   return 0;  /* 上左 → Z1 */
    case HMI_KEY_RIGHT_P_SET_UP:  return 1;  /* 上右 → Z2 */
    case HMI_KEY_LEFT_P_SET:      return 2;  /* 下左 → Z3 */
    case HMI_KEY_RIGHT_P_SET:     return 3;  /* 下右 → Z4 */
    default:                  return -1;
    }
}

static uint8_t head_index_to_key(uint8_t idx)
{
    switch (idx) {
    case 0: return HMI_KEY_LEFT_P_SET_UP;   /* Z1 → 上左 */
    case 1: return HMI_KEY_RIGHT_P_SET_UP;  /* Z2 → 上右 */
    case 2: return HMI_KEY_LEFT_P_SET;      /* Z3 → 下左 */
    case 3: return HMI_KEY_RIGHT_P_SET;     /* Z4 → 下右 */
    default: return HMI_KEY_NONE;
    }
}

static int8_t digit_key_to_level(uint8_t key)
{
    if (key >= HMI_KEY_POWER_0 && key <= HMI_KEY_POWER_9) {
        return (int8_t)(key - HMI_KEY_POWER_0);
    }
    return -1;
}

static uint8_t is_head_key(uint8_t key)
{
    return (key == HMI_KEY_LEFT_P_SET
        || key == HMI_KEY_RIGHT_P_SET
        || key == HMI_KEY_LEFT_P_SET_UP
        || key == HMI_KEY_RIGHT_P_SET_UP) ? 1 : 0;
}

/* ========== 对齐模块门禁: 仅 POWERED_OFF 允许进入 ========== */
uint8_t AppSegAlign_CanEnter(void)
{
    return (s_global.mode == HMI_NODE_POWERED_OFF) ? 1u : 0u;
}

/* =================================================================
 * WASM 测试导出 — 仅 WASM_BUILD 生效
 * ================================================================= */
#ifdef WASM_BUILD
#include <stdint.h>
uint8_t  wasm_get_global_mode(void)        { return s_global.mode; }
uint8_t  wasm_is_child_lock(void)          { return s_global.child_lock; }
uint8_t  wasm_is_paused(void)              { return s_global.paused; }
int8_t   wasm_get_hot_head(void)           { return s_hot_head; }
uint8_t  wasm_get_stack_depth(void)        { return s_stack_count; }
int8_t   wasm_get_stack_at(uint8_t p)      { return (p < 4) ? s_select_stack[p] : -1; }
uint8_t  wasm_get_zone_node(uint8_t i)     { return (i < 4) ? s_heads[i].node : 0; }
uint8_t  wasm_get_zone_power(uint8_t i)    { return (i < 4) ? s_heads[i].power_level : 0; }
uint8_t  wasm_get_zone_boost(uint8_t i)    { return (i < 4) ? s_heads[i].boost_active : 0; }
uint8_t  wasm_get_zone_timer_setting(uint8_t i) { return (i < 4) ? s_heads[i].timer_setting : 0; }
uint8_t  wasm_get_zone_timer_active(uint8_t i)  { return (i < 4) ? s_heads[i].timer_active : 0; }
uint8_t  wasm_get_zone_timer_value(uint8_t i)   { return (i < 4) ? s_heads[i].timer_value : 0; }
char     wasm_get_seg_char(uint8_t i)      { return (i < 8) ? s_display.seg_chars[i] : ' '; }
uint8_t  wasm_get_seg_blink(uint8_t i)     { return (i < 4) ? s_display.seg_blink[i] : 0; }
uint8_t  wasm_get_seg_mode(void)           { return s_display.seg_mode; }
uint8_t  wasm_get_led_power(void)          { return s_display.leds_power; }
uint8_t  wasm_get_led_timer(void)          { return s_display.leds_timer; }
uint8_t  wasm_get_led_pause(void)          { return s_display.leds_pause; }
uint8_t  wasm_get_led_child_lock(void)     { return s_display.leds_child_lock; }
uint8_t  wasm_get_led_head_select(uint8_t i) { return (i < 4) ? s_display.leds_head_select[i] : 0; }
uint8_t  wasm_get_led_power_level(uint8_t i) { return (i < 10) ? s_display.leds_power_level[i] : 0; }

/* 强制超时 — 测试用，直接操作内部状态 */
void wasm_force_select_confirm(uint8_t idx) { confirm_select((int8_t)idx); }
void wasm_force_boost_exit(uint8_t idx)     { exit_boost((int8_t)idx); }
void wasm_force_timer_expire(uint8_t idx) {
    if (idx < 4) {
        s_heads[idx].timer_value = 0;
        s_heads[idx].timer_active = 0;
        if (s_heads[idx].timer_setting) {
            s_heads[idx].timer_setting = 0;
            s_heads[idx].select_ticks = 0;
        }
    }
}
#endif /* WASM_BUILD */
