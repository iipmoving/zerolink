# JSON → M0 C 数组转换 + 引擎实现

## 背景

桌面端 `four_head_v4.json` 驱动 HMI 引擎已验证通过（34/34 测试，90% 覆盖率）。已完成数据层转换（枚举+结构体+生成器）。现在要实现 `app_hmi.c` 引擎，从生成的配置表读取路由/超时/规则，驱动 M0 平台上的 HMI 状态机。

**核心要求：** 尽可能简单明了的用数组表示参数，用枚举替代 ASCII 码代表信息。

---

## 一、已完成：数据层（阶段 1-4）

| 文件 | 状态 |
|------|------|
| `Claude/app/app_hmi.h` | 完成 — 7 枚举 + 12 结构体 |
| `Claude/cfg/gen_hmi.js` | 完成 — JSON→C 生成器 |
| `Claude/cfg/hmi_data.c` | 完成 — 生成的 const 数组 |
| `Claude/cfg/hmi_data.h` | 完成 — extern 声明 |

---

## 二、引擎实现：app_hmi.c（本次范围）

### 2.1 模块结构

遵循现有 APP 模块模式（参照 `app_cooking.c`）：

```
app_hmi.h  — 类型定义（已有）+ 公共 API 声明
app_hmi.c  — 引擎实现
  ├── 静态状态变量（heads[4], global_state, display_cache, select_stack）
  ├── 消息回调（on_key_event, on_timer_100ms, on_timer_1s）
  ├── 动作函数（selectHead, handlePowerKey, enterBoost, ...）
  ├── 显示派生（postDisplay: ModeRule→seg_mode, BlinkRule→seg_blink）
  └── 公共 API（App_Hmi_Init, App_Hmi_Run）
```

### 2.2 引擎内部状态

```c
/* 炉头实例 */
typedef struct {
    HmiZoneNode_t node;          /* idle/selecting/cooking */
    uint8_t       power_level;   /* 0-9 */
    uint8_t       original_power;/* Boost 前档位 */
    uint8_t       boost_active;  /* bool */
    uint8_t       timer_setting; /* bool */
    uint8_t       timer_active;  /* bool */
    uint16_t      timer_value;   /* 分钟 1-99 */
    uint32_t      select_ticks;  /* 选中时刻(tick) */
    uint32_t      timer_set_ticks; /* 定时设置时刻 */
    int32_t       boost_remaining_ticks; /* Boost 剩余 tick */
} HmiHead_t;

/* 全局状态 */
typedef struct {
    HmiGlobalNode_t mode;
    uint8_t         child_lock;   /* bool */
    uint8_t         paused;       /* bool */
    uint8_t         power_on_step;/* 上电序列步骤 */
} HmiGlobalState_t;

/* 段码显示缓存 */
typedef struct {
    char     seg_chars[8];        /* 8 个段码字符 */
    uint8_t  seg_blink[4];        /* 每个 Zone 是否闪烁 */
    uint8_t  seg_mode;           /* HmiSegMode_t */
    uint8_t  leds_power;         /* bool */
    uint8_t  leds_timer;         /* bool */
    uint8_t  leds_pause;         /* bool */
    uint8_t  leds_child_lock;    /* bool */
    uint8_t  leds_head_select[4];/* bool[4] */
    uint8_t  leds_power_level[10];/* bool[10] */
} HmiDisplayCache_t;

/* 引擎顶层状态（全部 static） */
static HmiHead_t        s_heads[4];
static HmiGlobalState_t s_global;
static HmiDisplayCache_t s_display;
static int8_t           s_hot_head;     /* -1=无 */
static int8_t           s_select_stack[4]; /* 最多4个 */
static uint8_t          s_stack_count;
static uint32_t         s_tick_100ms;   /* 100ms 计数器 */
static uint32_t         s_idle_ticks;   /* 空闲起始 tick */
static uint32_t         s_off_ticks;    /* 关机起始 tick */
```

### 2.3 公开 API

```c
/* app_hmi.h 新增 */
void App_Hmi_Init(void);
void App_Hmi_Run(void);  /* 每 10ms 调用（槽位 7/8/9 预留） */
```

### 2.4 消息订阅

```c
void App_Hmi_Init(void) {
    /* 初始化状态 */
    memset(s_heads, 0, sizeof(s_heads));
    s_global.mode = HMI_NODE_POWERED_OFF;
    /* ... */
    
    /* 注册回调 */
    MsgScheduler_Register(MSG_KEY_EVENT,   on_key_event);
    MsgScheduler_Register(MSG_TIMER_100MS, on_timer_100ms);
    MsgScheduler_Register(MSG_TIMER_1S,    on_timer_1s);
    
    /* 启动上电序列 */
    run_power_on_seq();
}
```

### 2.5 路由查找（从生成的配置表读取）

```c
/* 线性扫描路由表，返回匹配条目或 NULL */
const HmiRoute_t *hmi_find_route(const HmiRoute_t *table, uint8_t count,
                                  KeyCode_t key, uint8_t event)
{
    uint8_t i;
    for (i = 0; i < count; i++) {
        if (table[i].key == key && table[i].event == event)
            return &table[i];
    }
    return NULL;
}
```

查找顺序与 JS 引擎一致：
1. **全局路由** → `g_hmi_cfg.global_nodes[s_global.mode].routes`
2. **童锁拦截** → 白名单 `g_hmi_cfg.child_lock_whitelist`
3. **进程路由** → timer_setting > timer_active > boost_active
4. **Zone 路由** → idle / selecting / cooking

动态键解析：
- `HMI_KEY_HEAD_SELF` (0xFF) → 匹配 `key == head_key_of(hot_head)`
- `HMI_KEY_HEAD_OTHER` (0xFE) → 匹配 `is_head_key(key) && key != head_key_of(hot_head)`
- `HMI_PARAM_DYNAMIC` (-1) → `param = head_index_from_key(key)`

### 2.6 动作分发

```c
static void hmi_execute_action(HmiAction_t action, int8_t param, 
                                KeyCode_t key, uint8_t silent)
{
    switch (action) {
        case HMI_ACT_GO_WORKING:        go_working(); break;
        case HMI_ACT_GO_POWERED_OFF:    go_powered_off(); break;
        case HMI_ACT_GO_DEEP_SLEEP:     enter_deep_sleep(); break;
        case HMI_ACT_TOGGLE_PAUSE:      toggle_pause(); break;
        case HMI_ACT_TOGGLE_CHILD_LOCK: toggle_child_lock(); break;
        case HMI_ACT_SELECT_HEAD:       select_head(param); break;
        case HMI_ACT_CONFIRM_SELECT:    confirm_select(s_hot_head); break;
        case HMI_ACT_CONFIRM_SELECT_IMMEDIATE: confirm_select(s_hot_head); break;
        case HMI_ACT_SET_POWER:         handle_power_key(param); break;
        case HMI_ACT_ENTER_BOOST:       enter_boost(); break;
        case HMI_ACT_EXIT_BOOST:        exit_boost(s_hot_head); break;
        case HMI_ACT_EXIT_BOOST_SET_POWER: exit_boost_set_power(param); break;
        case HMI_ACT_ENTER_TIMER_SETTING: enter_timer_setting(); break;
        case HMI_ACT_CONFIRM_TIMER:     confirm_timer(find_timer_head()); break;
        case HMI_ACT_CANCEL_TIMER_SETTING: cancel_timer_setting(); break;
        case HMI_ACT_CANCEL_TIMER_ACTIVE:  cancel_timer_active(); break;
        case HMI_ACT_TIMER_ADJUST:      handle_timer_adjust(param); break;
        /* 显示 / LED / 蜂鸣 / 杂项 ... */
        case HMI_ACT_SHOW_DASH:         show_dash(); break;
        case HMI_ACT_SHOW_PA:           show_pa(); break;
        case HMI_ACT_SHOW_POWER_MODE:   update_all_displays(); post_display(); break;
        case HMI_ACT_DISPLAY_ALL_OFF:   show_all_off(); break;
        case HMI_ACT_CLEAR_ALL_HEADS:   clear_all_heads(); break;
        case HMI_ACT_HOTHEAD_CLEAR:     s_hot_head = -1; s_stack_count = 0; break;
        case HMI_ACT_CLEAR_HOTHEAD:     s_hot_head = -1; break;
        case HMI_ACT_LED_POWER_ON:      s_display.leds_power = 1; break;
        case HMI_ACT_LED_POWER_OFF:     s_display.leds_power = 0; break;
        case HMI_ACT_LED_ALL_OFF:       leds_all_off(); break;
        case HMI_ACT_LED_PAUSE_ON:      s_display.leds_pause = 1; break;
        /* ... 其他 LED 动作 ... */
        case HMI_ACT_RESET_IDLE_TIMER:  reset_idle_timer(); break;
        case HMI_ACT_RESET_OFF_TIMER:   s_off_ticks = s_tick_100ms; break;
        case HMI_ACT_BEEP_VALID:        post_buzzer(1); return;
        case HMI_ACT_BEEP_INVALID:      post_buzzer(0); return;
        default: return;
    }
    if (!silent) post_buzzer(1);
}
```

### 2.7 关键动作函数（"孤岛"标准实现）

这些函数与 JS 引擎同构，但：
- 超时用 tick 计数替代 `Date.now()`
- 直接从 `g_hmi_cfg` 读取配置参数
- 调用 `post_display()` 统一刷新

| 函数 | 行数 | 说明 |
|------|------|------|
| `select_head` | ~25 | 选中状态机，自确认分支 |
| `handle_power_key` | ~30 | 功率切换，清理 boost/timer |
| `confirm_select` | ~20 | 确认逻辑，power>0→cooking |
| `enter_boost` | ~15 | boost 生命周期开始 |
| `exit_boost` | ~15 | 恢复原始功率 |
| `exit_boost_set_power` | ~12 | boost 退出+设档 |
| `enter_timer_setting` | ~12 | 进入定时设置 |
| `confirm_timer` | ~18 | 确认定时→timer_active |
| `cancel_timer_setting` | ~10 | 取消设置 |
| `cancel_timer_active` | ~10 | 取消运行中定时 |
| `handle_timer_adjust` | ~15 | 加减调整（读 elements.timer） |
| `go_working` | ~20 | 进入工作模式（执行 enter_actions） |
| `go_powered_off` | ~20 | 进入关机（执行 enter_actions） |
| `enter_deep_sleep` | ~15 | 进入休眠 |
| `toggle_pause` | ~15 | 暂停/恢复切换 |
| `toggle_child_lock` | ~8 | 童锁切换 |
| `resolve_target` | ~15 | 目标炉头解析 |
| `push_to_stack` | ~10 | 栈管理（读 elements.stack） |
| `clear_all_heads` | ~15 | 重置所有炉头 |

### 2.8 显示派生（post_display）

```c
static void post_display(void) {
    /* 1. ModeRule: 优先级链派生 seg_mode */
    derive_seg_mode();
    
    /* 2. BlinkRule: 从 head 状态派生 seg_blink（含暂停覆盖）*/
    derive_seg_blink();
    
    /* 3. 更新每个炉头的段码字符 */
    update_all_displays();
    
    /* 4. 投递显示刷新消息 */
    Msg_Post(MSG_DISPLAY_REFRESH, 0, &s_display);
}
```

ModeRule 派生逻辑：
```c
static void derive_seg_mode(void) {
    const HmiModeRule_t *rules = g_hmi_cfg.mode_rules;
    uint8_t i;
    for (i = 0; i < g_hmi_cfg.mode_rule_count; i++) {
        switch (rules[i].condition) {
            case HMI_MODE_COND_POWERED_OFF:
                if (s_global.mode == HMI_NODE_POWERED_OFF)
                    { s_display.seg_mode = rules[i].seg_mode; return; }
                break;
            case HMI_MODE_COND_DEEP_SLEEP:
                if (s_global.mode == HMI_NODE_DEEP_SLEEP)
                    { s_display.seg_mode = rules[i].seg_mode; return; }
                break;
            case HMI_MODE_COND_PAUSED:
                if (s_global.paused)
                    { s_display.seg_mode = rules[i].seg_mode; return; }
                break;
            case HMI_MODE_COND_ANY_TIMER_SETTING:
                if (any_timer_setting())
                    { s_display.seg_mode = rules[i].seg_mode; return; }
                break;
        }
    }
    /* 回退默认值 */
    s_display.seg_mode = g_hmi_cfg.elements->mode_default;
}
```

段码字符派生（每个炉头）：
```c
static void update_head_display(uint8_t idx) {
    HmiHead_t *h = &s_heads[idx];
    uint8_t base = idx * 2;
    
    if (h->timer_setting) {
        /* 显示定时值 */
        if (h->timer_value >= 10) {
            s_display.seg_chars[base]   = '0' + (h->timer_value / 10);
            s_display.seg_chars[base+1] = '0' + (h->timer_value % 10);
        } else {
            s_display.seg_chars[base]   = '0' + h->timer_value;
            s_display.seg_chars[base+1] = ' ';
        }
    } else if (h->boost_active) {
        s_display.seg_chars[base]   = 'P';
        s_display.seg_chars[base+1] = ' ';
    } else if (h->power_level == 0) {
        s_display.seg_chars[base]   = '0';
        s_display.seg_chars[base+1] = '0';
    } else {
        s_display.seg_chars[base]   = '0' + h->power_level;
        s_display.seg_chars[base+1] = ' ';
    }
}
```

### 2.9 超时管理（on_timer_100ms）

```c
static void on_timer_100ms(MsgId_t id, uint16_t param, void *data_ptr) {
    s_tick_100ms++;
    
    /* 跳过上电序列 */
    if (s_global.mode == HMI_NODE_POWERED_OFF /* 需特殊标记 */) return;
    
    /* Zone 超时: 遍历 selecting 炉头 */
    for (i = 0; i < 4; i++) {
        if (s_heads[i].node == HMI_ZONE_SELECTING && s_heads[i].select_ticks > 0) {
            uint16_t timeout_ms = g_hmi_cfg.timeouts[HMI_TO_SELECT_CONFIRM_MS];
            uint32_t elapsed = (s_tick_100ms - s_heads[i].select_ticks) * 100;
            if (elapsed >= timeout_ms) {
                hmi_execute_action(HMI_ACT_CONFIRM_SELECT, 0, 0, 1);
            }
        }
    }
    
    /* 进程超时 */
    /* timer_setting 15s → confirm_timer */
    /* boost_active 5min → exit_boost */
    
    /* Guard 检查: 遍历当前全局模式的 guards[] */
    for (g = 0; g < cfg->guard_count; g++) {
        if (cfg->guards[g].check == HMI_GUARD_ALWAYS) {
            if (s_off_ticks > 0 && elapsed >= timeout) { ... }
        } else if (cfg->guards[g].check == HMI_GUARD_ALL_IDLE) {
            if (all_idle() && s_idle_ticks > 0 && elapsed >= timeout) { ... }
        }
    }
}
```

### 2.10 定时倒计时（on_timer_1s）

```c
static void on_timer_1s(MsgId_t id, uint16_t param, void *data_ptr) {
    for (i = 0; i < 4; i++) {
        if (s_heads[i].timer_active && s_heads[i].timer_value > 0) {
            s_heads[i].timer_value--;
            if (s_heads[i].timer_value == 0) {
                /* 归零：idle + 清除进程 */
                s_heads[i].timer_active = 0;
                s_heads[i].node = HMI_ZONE_IDLE;
                s_heads[i].power_level = 0;
                s_heads[i].boost_active = 0;
                remove_from_stack(i);
                reassign_hot_head(i);
            }
        }
    }
    sync_leds();
    post_display();
}
```

### 2.11 按键→路由→动作主流程（on_key_event）

```c
static void on_key_event(MsgId_t id, uint16_t param, void *data_ptr) {
    KeyCode_t key = (KeyCode_t)(param & 0xFF);
    uint8_t   evt = (uint8_t)((param >> 8) & 0xFF);
    
    /* 只处理 TAP 和 LONG */
    if (evt != KEY_STATE_TAP && evt != KEY_STATE_LONG) return;
    uint8_t event_type = (evt == KEY_STATE_TAP) ? HMI_EVT_TAP : HMI_EVT_LONG;
    
    /* 1. 上电序列/休眠 拒绝（全局路由先匹配） */
    
    /* 2. 全局路由查找 */
    /* 3. 童锁拦截 */
    /* 4. 暂停拦截 */
    /* 5. 进程路由查找 */
    /* 6. Zone 路由查找（含 HEAD_SELF/HEAD_OTHER 动态解析）*/
    /* 7. 未匹配 → 无效蜂鸣 */
}
```

### 2.12 LED 同步

```c
static void sync_leds(void) {
    /* Power LED: powered_off→闪烁, working→on, deep_sleep→off */
    /* Timer LED: any(timer_active) */
    /* Pause LED: s_global.paused */
    /* ChildLock LED: s_global.child_lock */
    /* HeadSelect LED: head.node == selecting */
    /* PowerLevel LED: hot_head 的 power_level + boost_override */
}
```

## 三、main.c 集成

```c
/* 在 main.c 初始化序列中添加 */
App_Hmi_Init();

/* 在 10 槽调度中添加 */
/* 槽位 7 */ App_Hmi_Run();
```

## 四、文件变更清单

| 文件 | 操作 | 说明 |
|------|------|------|
| `Claude/app/app_hmi.h` | **修改** | 添加 `App_Hmi_Init/Run` 声明 |
| `Claude/app/app_hmi.c` | **新建** | 引擎实现（~800行） |
| `Claude/src/main.c` | **修改** | 注册 HMI 模块到槽位 7 |
| `Claude/test/test_hmi_engine.c` | **新建(可选)** | C 单元测试 |

## 五、不在此次范围

- **显示渲染器** — `drv_display.c` 需要新增订阅 `MSG_DISPLAY_REFRESH` 的处理函数，将 `HmiDisplayCache_t` 渲染到 SMG 缓冲区和 LED IO。这是独立的显示集成任务。
- **蜂鸣器消息** — 需要 `MSG_BUZZER_CTRL` 消息 ID（当前 `msg_def.h` 中不存在），或直接调用 `Drv_Buzzer_*` 函数。
- **上电序列异步执行** — JS 用 `setTimeout`，C 引擎改用状态机步骤推进（在 `on_timer_100ms` 中检查步骤延时）。

## 六、实现顺序

1. **`app_hmi.c` 骨架**：状态变量 + Init/Run + 消息注册
2. **路由查找**：`hmi_find_route` + 动态键解析
3. **动作分发**：`hmi_execute_action` switch-case
4. **15 个孤岛函数**：selectHead → ... → clearAllHeads
5. **显示派生**：post_display (ModeRule + BlinkRule + seg_chars)
6. **超时管理**：on_timer_100ms (Zone/Process/Guard)
7. **定时倒计时**：on_timer_1s
8. **上电序列**：状态机步骤推进
9. **main.c 集成**：槽位注册

## 七、验证方式

1. **编译**：Keil MDK 编译零错误零警告
2. **对照 JS 测试**：将 34 条 Flow 逐条翻译为 C 单元测试（可选，后续）
3. **手工验证**：上电序列 → 开关机 → 选头设档 → Boost → 定时 → 暂停 → 童锁 → 休眠
