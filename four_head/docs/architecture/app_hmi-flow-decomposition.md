# APP_HMI 流程分解

> 源文件: `Claude/app/app_hmi.c` (1897行) / `sim/test_hmi/wasm/app/app_hmi.c` (同源)
> 生成日期: 2026-05-26
> 用途: 代码审查 / 模块拆分参考 / 新人理解入口

---

## 一、整体架构

```
                    ┌─────────────────────────────────────┐
                    │            app_hmi.c                 │
                    │                                      │
  MSG_KEY_EVENT ──→│ on_key_event ──→ 路由查找 ──→ 动作分发 │
  MSG_TIMER_100MS─→│ on_timer_100ms ──→ 超时/Guard/刷新    │
  MSG_TIMER_1S ──→│ on_timer_1s ──→ 倒计时/过期            │
                    │                                      │
                    │  内部状态:                            │
                    │  s_heads[4]  s_global  s_display     │
                    │  s_hot_head  s_select_stack[]        │
                    │  s_tick_100ms  s_idle_ticks          │
                    │                                      │
  MSG_DISPLAY_REFRESH ←── post_display()                   │
  MSG_BUZZER_CTRL    ←── post_buzzer()                     │
                    └─────────────────────────────────────┘
```

**三个入口回调** (注册在 `App_Hmi_Init()`):
| 回调 | 消息ID | 触发频率 | 职责 |
|------|--------|----------|------|
| `on_key_event` | MSG_KEY_EVENT | 按键事件 | 9段路由 → 动作分发 |
| `on_timer_100ms` | MSG_TIMER_100MS | 每100ms | 超时检测 + Guard + 显示刷新 |
| `on_timer_1s` | MSG_TIMER_1S | 每1s | 定时器倒计时 |

---

## 二、状态变量一览

| 变量 | 类型 | 说明 |
|------|------|------|
| `s_heads[4]` | HmiHead_t | 4个炉头的完整状态 (node/power/timer/boost/select_ticks) |
| `s_global` | HmiGlobalState_t | 全局状态 (mode/paused/child_lock/power_on_seq) |
| `s_display` | HmiDisplayCache_t | 显示缓存 (seg_chars/blink/mode/LEDs) |
| `s_hot_head` | int8_t | 当前热点炉头索引, -1=无 |
| `s_select_stack[4]` | int8_t[4] | 选择序列栈 (LRU, 最大深度可配置) |
| `s_stack_count` | uint8_t | 栈当前深度 |
| `s_tick_100ms` | uint32_t | 100ms计数器, 每tick自增 |
| `s_idle_ticks` | uint32_t | 最后有效按键时的tick快照, 用于空闲超时 |
| `s_off_ticks` | uint32_t | 进入关机态时的tick快照, 用于关机后deep_sleep超时 |

---

## 三、函数分类与调用关系

### 3.1 主入口 (3个)

```
App_Hmi_Init()
  └─ MsgScheduler_Register × 3  (注册回调)
  └─ run_power_on_seq_step()     (启动上电序列)

App_Hmi_Run()                    (空, 保留槽位)
```

### 3.2 按键路由链 (1个核心 + 2个路由辅助)

```
on_key_event(key, evt)
  ├─ [1] 全局路由: hmi_find_route(global_cfg->routes) → hmi_execute_action()
  ├─ [2] 休眠态拒绝
  ├─ [3] 关机态拒绝
  ├─ [4] 童锁拦截 (遍历 child_lock_whitelist)
  ├─ [5] 暂停态拒绝
  ├─ [6] resolve_target() — 确定目标炉头
  ├─ [7] 进程路由 (timer_setting > timer_active > boost_active 优先级)
  │     └─ hmi_find_route(proc_cfg->routes) → hmi_execute_action()
  ├─ [8] Zone路由: hmi_match_dynamic_route() → hmi_execute_action()
  └─ [9] 未匹配 → post_buzzer(0)
```

### 3.3 路由查找 (2个)

| 函数 | 输入 | 输出 | 说明 |
|------|------|------|------|
| `hmi_find_route` | table, count, key, event | `const HmiRoute_t*` 或 NULL | 线性扫描精确匹配 key+event |
| `hmi_match_dynamic_route` | route, key, event | 0/1 | 支持 HEAD_SELF/HEAD_OTHER 动态键 |

### 3.4 动作分发 (2个)

| 函数 | 输入 | 副作用 | 说明 |
|------|------|--------|------|
| `hmi_execute_action` | action, param, key, silent | 调用对应孤岛函数 | 30+ action switch, HMI_PARAM_DYNAMIC 替换 |
| `hmi_apply_actions` | actions[], count, silent | 遍历调用 hmi_execute_action | JSON enter/exit_actions 批处理 |

### 3.5 全局模式孤岛函数 (5个)

| 函数 | 修改的变量 | 调用链 |
|------|-----------|--------|
| `go_working` | s_global.mode/paused/child_lock, s_idle_ticks | exit旧模式 → 切换 → enter新模式 → sync_leds+post_display |
| `go_powered_off` | s_global.mode/paused, s_off_ticks | exit旧模式 → 切换 → enter新模式 → sync_leds+post_display |
| `enter_deep_sleep` | s_global.mode, s_idle_ticks | exit旧模式 → 切换 → enter新模式 → sync_leds+post_display |
| `toggle_pause` | s_global.paused | 翻转paused → apply enter/exit_actions → sync_leds+post_display |
| `toggle_child_lock` | s_global.child_lock | 翻转child_lock → reset_idle_timer(解锁时) → sync_leds+post_display |

**模式切换共同模式**: exit旧模式的exit_actions → 改s_global → enter新模式的enter_actions → sync_leds+post_display

### 3.6 炉头选择与功率 (3个)

| 函数 | 输入 | 修改的变量 | 说明 |
|------|------|-----------|------|
| `select_head` | index(0-3) | s_heads[index].node/select_ticks, s_hot_head, 旧炉头确认, 栈 | 同炉头=确认(idle/cooking重新选中) |
| `confirm_select` | head_idx | s_heads[idx].node/select_ticks, 栈, s_hot_head | selecting→cooking(idle) |
| `handle_power_key` | level(0-9) | s_heads[target].power_level/node/timer/boost, 栈, s_hot_head | 0档=idle清理, >0=cooking入栈 |

### 3.7 Boost (3个)

| 函数 | 输入 | 修改的变量 | 说明 |
|------|------|-----------|------|
| `enter_boost` | (用resolve_target) | s_heads[target].original_power/boost_active/boost_remaining_ms/power_level | 保存原档位→设boost档→入栈 |
| `exit_boost` | idx | s_heads[idx].power_level/boost_active, 可能confirm_select | 恢复原档位 |
| `exit_boost_set_power` | level | s_heads[hot_head].boost_active→handle_power_key→可能confirm_select | Boost中按档位键: 退出boost+设档 |

### 3.8 定时 (6个)

| 函数 | 输入 | 修改的变量 | 说明 |
|------|------|-----------|------|
| `enter_timer_setting` | keep_value | s_heads[target].timer_setting/active/value/set_ticks | keep_value=保留当前剩余时间 |
| `confirm_timer` | head_idx | s_heads[idx].timer_setting/active, 可能confirm_select | 确认定时→启动倒计时 |
| `cancel_timer_setting` | (用find_timer_head) | s_heads[target].timer_setting/value/set_ticks | 取消设置 |
| `cancel_timer_active` | (用find_timer_head) | s_heads[target].timer_active/value | 取消运行中的定时 |
| `handle_timer_adjust` | delta(±1) | s_heads[target].timer_value/set_ticks | +/-调节, step/min/max从JSON读 |
| `find_timer_head` | — | — | 返回s_hot_head (当前唯一实现) |

### 3.9 目标解析与栈管理 (5个)

| 函数 | 输入 | 输出 | 修改 | 说明 |
|------|------|------|------|------|
| `resolve_target` | — | zone_idx 或 -1 | s_hot_head (隐式选中时) | hot_head非idle→返回; 恰好1个非idle→隐式选中; 否则-1 |
| `push_to_stack` | idx | — | s_select_stack[], s_stack_count | 去重+移到栈顶; 超max_depth踢最旧 |
| `remove_from_stack` | idx | — | s_select_stack[], s_stack_count | 移除+前移 |
| `get_stack_top` | — | idx 或 -1 | — | 返回栈顶 |
| `reassign_hot_head` | old_idx | — | s_hot_head | old_idx=head且已变idle→栈顶接管 |
| `clear_all_heads` | — | — | s_heads[4]全清, leds_all_off, s_hot_head=-1, s_stack_count=0 | 全炉头重置 |

### 3.10 显示派生 (7个)

| 函数 | 修改的变量 | 说明 |
|------|-----------|------|
| `post_display` | s_display (触发整体刷新) | 核心调度: derive_seg_mode → derive_seg_blink → update_all_displays → Msg_Post |
| `derive_seg_mode` | s_display.seg_mode | ModeRule优先级链线性扫描 (POWERED_OFF/DEEP_SLEEP/PAUSED/ANY_TIMER_SETTING → default) |
| `derive_seg_blink` | s_display.seg_blink[4] | PAUSED→全灭(可配); selecting或timer_setting(非boost)→闪烁 |
| `update_head_display` | s_display.seg_chars[idx*2], seg_chars[idx*2+1] | timer→显示时间(5s交替); boost→"P"; power=0→"00"; 其他→档位数字 |
| `update_all_displays` | — | 遍历4头调update_head_display |
| `show_dash` | s_display.seg_chars[8] | 全屏显示 "--" (从JSON patterns.dash) |
| `show_pa` | s_display.seg_chars[8] | 全屏显示 "PA" (从JSON patterns.pa) |
| `show_all_off` | s_display.seg_chars[8] | 全屏显示全灭 (从JSON patterns.off) |

### 3.11 LED同步 (2个)

| 函数 | 修改的变量 | 说明 |
|------|-----------|------|
| `leds_all_off` | s_display.leds_* 全0 | 所有LED清零 |
| `sync_leds` | s_display.leds_* 全部 | 根据s_global/s_hot_head/s_heads更新所有LED: power(含blink)/timer/pause/child_lock/head_select/power_level(含single/gradient+boost) |

### 3.12 超时管理 (1个)

**`on_timer_100ms`** — 每100ms被MSG_TIMER_100MS触发:

```
s_tick_100ms++
├─ 上电序列中 → run_power_on_seq_step() → return
├─ Zone超时: selecting 15s → confirm_select
├─ 进程超时: timer_setting 15s → confirm_timer
├─ 进程超时: boost_active 倒计时 → exit_boost(归零时)
├─ Guard检查: 遍历当前global_mode的guards
│   ├─ HMI_GUARD_ALWAYS → off_ticks超时 → action (deep_sleep等)
│   └─ HMI_GUARD_ALL_IDLE → idle_ticks超时+all_idle → action
└─ sync_leds() + post_display()
```

### 3.13 定时倒计时 (1个)

**`on_timer_1s`** — 每1s被MSG_TIMER_1S触发:

```
遍历4头:
  timer_active && timer_value>0 → timer_value--
  归零时: timer_active=0, node=idle, power=0, boost=0, 出栈+reassign
  update_head_display(i)
sync_leds() + post_display()
```

### 3.14 上电序列 (1个)

**`run_power_on_seq_step`** — 每100ms推进:

```
读取 hmi_cfg.power_on_seq[current_step]
├─ goto_node ≥ 0 → 跳转目标模式 (go_powered_off/go_working/enter_deep_sleep)
├─ 首次进入 → 应用 seg_mode/seg_chars/LED
├─ power_on_step_ticks++
└─ 延时到达 → power_on_step++
```

### 3.15 辅助函数 (5个)

| 函数 | 逻辑 | 调用者 |
|------|------|--------|
| `reset_idle_timer` | s_idle_ticks = s_tick_100ms, s_off_ticks = 0 | on_key_event, toggle_child_lock, go_working |
| `all_heads_idle` | 遍历4头检查node==IDLE | Guard检查、外部查询 |
| `any_timer_setting` | 遍历4头检查timer_setting | derive_seg_mode |
| `any_timer_active` | 遍历4头检查timer_active | sync_leds |
| `post_buzzer` | Msg_Post(MSG_BUZZER_CTRL, valid?1:0) | 各动作函数 |

### 3.16 按键映射 (4个静态函数)

| 函数 | 输入 | 输出 | 说明 |
|------|------|------|------|
| `head_key_to_index` | key | 0-3 或 -1 | KEY_LEFT_P_SET_UP→0 ... KEY_RIGHT_P_SET→3 |
| `head_index_to_key` | idx(0-3) | key | 逆映射 |
| `digit_key_to_level` | key | 0-9 或 -1 | KEY_POWER_0→0 ... KEY_POWER_9→9 |
| `is_head_key` | key | 0/1 | 判断是否为4个头键之一 |

---

## 四、两条主数据流

### 4.1 按键事件流

```
engine_post_key(key, evt)
  → on_key_event(MSG_KEY_EVENT, param)
    → [上电序列中?] 拒绝
    → [evt不是TAP/LONG?] 拒绝
    → 1. 全局路由匹配 → hmi_execute_action → 孤岛函数 → sync_leds+post_display
    → 2. DEEP_SLEEP拒绝
    → 3. POWERED_OFF拒绝
    → 4. 童锁白名单检查
    → 5. PAUSED拒绝
    → reset_idle_timer()
    → 6. resolve_target()
    → 7. 进程路由 (timer_setting>timer_active>boost_active)
    → 8. Zone路由 (hmi_match_dynamic_route, 支持HEAD_SELF/HEAD_OTHER)
    → 9. 未匹配 → post_buzzer(0)
```

### 4.2 定时刷新流

```
engine_tick_100ms()
  → on_timer_100ms()
    → s_tick_100ms++
    → [上电序列] run_power_on_seq_step()
    → selecting超时 → confirm_select
    → timer_setting超时 → confirm_timer
    → boost倒计时 → exit_boost
    → Guard检查 → deep_sleep等
    → sync_leds() + post_display()
      → post_display()
        → derive_seg_mode()    (ModeRule优先级链)
        → derive_seg_blink()   (闪烁标记)
        → hot_head_idx = s_hot_head
        → update_all_displays() (仅POWER/TIMER_SETTING模式)
        → Msg_Post(MSG_DISPLAY_REFRESH)

engine_tick_1s()
  → on_timer_1s()
    → 遍历timer_active → timer_value--
    → 归零 → idle+出栈
    → sync_leds() + post_display()
```

---

## 五、函数修改状态矩阵

| 函数 | heads | global | display | hot_head | stack | tick相关 |
|------|-------|--------|---------|----------|-------|----------|
| go_working | — | mode/paused/lock | — | — | — | idle_ticks/off_ticks |
| go_powered_off | — | mode/paused | — | — | — | idle_ticks/off_ticks |
| enter_deep_sleep | — | mode | — | — | — | idle/off |
| toggle_pause | — | paused | — | — | — | — |
| toggle_child_lock | — | child_lock | — | — | — | — |
| select_head | node/ticks×2 | — | — | ✓ | push/remove | — |
| confirm_select | node/ticks | — | — | (via reassign) | push/remove | — |
| handle_power_key | power/node/timer | — | — | (via reassign) | push/remove | — |
| enter_boost | power/boost | — | — | — | push | — |
| exit_boost | power/boost | — | — | — | — | — |
| exit_boost_set_power | power/boost | — | — | — | — | — |
| enter_timer_setting | timer_* | — | — | — | — | — |
| confirm_timer | timer/node | — | — | — | push/remove | — |
| cancel_timer_setting | timer_* | — | — | — | — | — |
| cancel_timer_active | timer | — | — | — | — | — |
| handle_timer_adjust | timer_value/ticks | — | — | — | — | — |
| clear_all_heads | 全清 | — | — | -1 | 清空 | — |
| resolve_target | — | — | — | (隐式设置) | — | — |
| push_to_stack | — | — | — | — | ✓ | — |
| remove_from_stack | — | — | — | — | ✓ | — |
| reassign_hot_head | — | — | — | ✓ | — | — |
| post_display | — | — | mode/blink/chars/hot_head_idx | — | — | — |
| derive_seg_mode | — | — | seg_mode | — | — | — |
| derive_seg_blink | — | — | seg_blink[4] | — | — | — |
| update_head_display | — | — | seg_chars[pos*2] | — | — | — |
| sync_leds | — | — | leds_* 全部 | — | — | — |
| leds_all_off | — | — | leds_* 全0 | — | — | — |
| on_timer_100ms | (超时触发) | — | — | — | — | s_tick_100ms++ |
| on_timer_1s | timer_value/node | — | — | (via reassign) | (via remove) | — |

---

## 六、JSON配置驱动点

引擎本身不包含业务规则, 所有可配置项从 `hmi_cfg` (由 `four_head_v4.json` → `v4json_to_c.py` → `hmi_data.c` 生成) 读取:

| 配置项 | 读取位置 | 用途 |
|--------|----------|------|
| `global_nodes[].routes` | on_key_event §1 | 全局路由表 |
| `global_nodes[].enter/exit_actions` | go_working等 | 模式切换动作 |
| `global_nodes[].guards` | on_timer_100ms | 超时Guard |
| `zone_nodes[].routes` | on_key_event §8 | Zone路由表 |
| `process_nodes[].routes` | on_key_event §7 | 进程路由表 |
| `mode_rules[]` | derive_seg_mode | seg_mode优先级链 |
| `timeouts[]` | on_timer_100ms, enter_boost, enter_timer_setting | 各超时时长 |
| `elements.*` | sync_leds, handle_timer_adjust, derive_seg_blink | LED模式/档位/步长等 |
| `patterns.*` | show_dash/show_pa/show_all_off | 显示模式字符 |
| `power_on_seq[]` | run_power_on_seq_step | 上电序列步骤 |
| `child_lock_whitelist[]` | on_key_event §4 | 童锁白名单 |

---

## 七、待优化点

1. **显示派生职责**: `post_display` / `derive_seg_mode` / `derive_seg_blink` / `update_head_display` / `sync_leds` 目前都在 app_hmi.c 中。按架构目标应迁移到 display_module, 但前次尝试因Agent自行简化逻辑导致29条断言失败。再次迁移时必须遵循[[golden-output-decomposition]]方法论。

2. **find_timer_head**: 当前仅返回 `s_hot_head`, 是简化实现。多炉头同时有定时时逻辑待完善。

3. **上电序列 goto_node=NODE_PAUSED**: 当前回退到 powered_off, 因为上电直接进入暂停无实际意义。

4. **LED测试模式**: `#ifdef HMI_DEBUG_KEYS` 块占用 ~200行, 与核心逻辑耦合在同一文件。可考虑抽离为独立调试模块。
