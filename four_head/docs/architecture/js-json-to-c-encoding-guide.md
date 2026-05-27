# JS+JSON → C 编码对应指南

**日期**: 2026-05-25  
**目的**: 为C程序员提供权威的JSON→C编码映射参考。每个JSON配置段、每个JS引擎函数在C侧有明确的对应位置。  
**原则**: 修改HMI行为 = 修改JSON → 重新生成 hmi_data.c → C引擎自动适配。禁止在 app_hmi.c 中硬编码JSON已声明的规则。

---

## 一、JSON配置段 → C数据结构 映射

> JSON路径格式: `顶层键.子键...` → C结构体路径

### 1.1 timeouts (超时值表)

| JSON路径 | C枚举索引 | hmi_data.c行号 |
|----------|----------|---------------|
| `timeouts.select_confirm_ms` | `HMI_TO_SELECT_CONFIRM_MS` | :9 |
| `timeouts.timer_confirm_ms` | `HMI_TO_TIMER_CONFIRM_MS` | :10 |
| `timeouts.boost_max_ms` | `HMI_TO_BOOST_MAX_MS` | :11 |
| `timeouts.idle_to_standby_ms` | `HMI_TO_IDLE_TO_STANDBY_MS` | :12 |
| `timeouts.idle_to_off_ms` | `HMI_TO_IDLE_TO_OFF_MS` | :13 |
| `timeouts.off_to_sleep_ms` | `HMI_TO_OFF_TO_SLEEP_MS` | :14 |
| `timeouts.default_timer_min` | `HMI_TO_DEFAULT_TIMER_MIN` | :15 |
| `timeouts.power_on_all_on_ms` | `HMI_TO_POWER_ON_ALL_ON_MS` | :16 |
| `timeouts.version_show_ms` | `HMI_TO_VERSION_SHOW_MS` | :17 |
| `timeouts.power_key_long_ms` | `HMI_TO_POWER_KEY_LONG_MS` | :18 |

C引擎通过 `hmi_cfg.timeouts[id]` 读取，类型 `uint32_t`，单位ms。

> **已知BUG**: power_on_all_on_ms 应为300(JSON)而非3000; version_show_ms 应为200(JSON)而非3000。

---

### 1.2 power_on_sequence (上电序列)

| JSON字段 | C结构体字段 (HmiPowerOnStep_t) | 类型 |
|----------|-------------------------------|------|
| `step` | 数组索引 (隐式) | — |
| `delay_ms_key` | `delay_ms` | `uint16_t` (已查表展开) |
| `seg_mode` | `seg_mode` (HmiSegMode_t) | `uint8_t` |
| `seg_chars` | `seg_chars[8]` | `char[8]` |
| `leds` | `leds_all` (HMI_LEDS_NONE/ALL_ON/ALL_OFF) | `uint8_t` |
| `goto` | `goto_node` (HmiGlobalNode_t, -1=无) | `int8_t` |

hmi_data.c:22-26 — 运行时由 `run_power_on_seq_step()` 消费 (app_hmi.c:1588)。

---

### 1.3 global_routes (全局模式路由)

| JSON键 | C枚举 | hmi_data.c数组 |
|--------|------|---------------|
| `global_routes.powered_off` | `HMI_NODE_POWERED_OFF` | `hmi_global_nodes[0]` (:109) |
| `global_routes.deep_sleep` | `HMI_NODE_DEEP_SLEEP` | `hmi_global_nodes[1]` (:115) |
| `global_routes.working` | `HMI_NODE_WORKING` | `hmi_global_nodes[2]` (:121) |
| `global_routes.paused` | `HMI_NODE_PAUSED` | `hmi_global_nodes[3]` (:127) |

每个模式的配置结构 `HmiGlobalNodeCfg_t`:

| JSON子字段 | C结构体字段 | 对应JS函数 |
|-----------|------------|-----------|
| `enter_actions` | `enter_actions[]` + `enter_count` | `applyActions(cfg.enter_actions)` |
| `exit_actions` | `exit_actions[]` + `exit_count` | `applyActions(cfg.exit_actions)` |
| `routes` | `routes[]` + `route_count` | `matchAndExecute()` |
| `guards` | `guards[]` + `guard_count` | `onTimer100ms()` 守卫循环 |

**路由条目** (HmiRoute_t, 4字节):
```c
{ key_code, event(HMI_EVT_TAP/LONG), action(HmiAction_t), param(int8_t) }
```

JSON路由键 `"POWER long"` → C: `{ KEY_ONOFF, HMI_EVT_LONG, HMI_ACT_GO_WORKING, 0 }`

**Guard条目** (HmiGuard_t, 4字节):
```c
{ check(HmiGuardCheck_t), ms_key(HmiTimeoutId_t), action, param }
```

JSON guard `{ "check":"always", "ms_key":"off_to_sleep_ms", "action":"go_deep_sleep" }`  
→ C: `{ HMI_GUARD_ALWAYS, HMI_TO_OFF_TO_SLEEP_MS, HMI_ACT_GO_DEEP_SLEEP, 0 }`

---

### 1.4 zone_routes (Zone状态路由)

| JSON键 | C枚举 | C数组 |
|--------|------|-------|
| `zone_routes.idle` | `HMI_ZONE_IDLE` | `hmi_zone_nodes[0]` (:176) |
| `zone_routes.selecting` | `HMI_ZONE_SELECTING` | `hmi_zone_nodes[1]` (:181) |
| `zone_routes.cooking` | `HMI_ZONE_COOKING` | `hmi_zone_nodes[2]` (:185) |

**动态路由键** (C引擎运行时解析):
| JSON键 | C key字段 | 含义 |
|--------|----------|------|
| `HEAD_SELF tap` | `HMI_KEY_HEAD_SELF` (0xFF) | 按当前hotHead对应的炉头键 |
| `HEAD_OTHER tap` | `HMI_KEY_HEAD_OTHER` (0xFE) | 按其他炉头键 |

`%head` 参数 → C: `HMI_PARAM_DYNAMIC` (-128), 由 `hmi_execute_action()` 动态解析。

**Zone超时** (HmiZoneCfg_t):
| JSON字段 | C字段 |
|----------|-------|
| `timeout.ms_key` | `timeout_ms_key` (0xFF=无超时) |
| `timeout.action` | `timeout_action` |

---

### 1.5 process_routes (进程路由)

| JSON键 | C枚举 | C数组 |
|--------|------|-------|
| `process_routes.timer_setting` | `HMI_PROC_TIMER_SETTING` | `hmi_process_nodes[0]` (:214) |
| `process_routes.timer_active` | `HMI_PROC_TIMER_ACTIVE` | `hmi_process_nodes[1]` (:219) |
| `process_routes.boost_active` | `HMI_PROC_BOOST_ACTIVE` | `hmi_process_nodes[2]` (:223) |

结构体复用 `HmiZoneCfg_t` (别名为 `HmiProcessCfg_t`)。

---

### 1.6 elements (控件元素)

| JSON路径 | C结构体 | 消费状态 |
|----------|---------|---------|
| `elements.timer.adjust_min` | `hmi_elements.timer_adjust_min` | ✅ 已消费 |
| `elements.timer.adjust_max` | `hmi_elements.timer_adjust_max` | ✅ 已消费 |
| `elements.timer.adjust_step` | `hmi_elements.timer_adjust_step` | ✅ 已消费 |
| `elements.boost.power_level` | `hmi_elements.boost_power_level` | ✅ 已消费 |
| `elements.stack.max_depth` | `hmi_elements.stack_max_depth` | ✅ 已消费 |
| `elements.blink.phase_ms` | `hmi_elements.blink_phase_ms` | ✅ 已消费 |
| `elements.blink.pause_override` | `hmi_elements.blink_pause_override` | ✅ 已消费 |
| `elements.mode.default` | `hmi_elements.mode_default` | ✅ 已消费 |
| `elements.segment_slot.value_map` | — | ❌ 未消费 |
| `elements.segment_slot.boost_char` | — | ❌ 未消费 |
| `elements.segment_slot.zero_char` | — | ❌ 未消费 |
| `elements.led.level.mode` | — | ❌ 未消费(硬编码gradient) |
| `elements.led.level.count` | — | ❌ 未消费 |
| `elements.led.level.boost_override` | — | ❌ 未消费 |
| `elements.led.power/timer/pause/child_lock/head_select` | — | ❌ 未消费(全部硬编码) |

---

### 1.7 display (显示决策规则)

| JSON路径 | C消费位置 | 状态 |
|----------|----------|------|
| `display.priority_chain` | `update_head_display()` 硬编码等效逻辑 | ⚠️ 未JSON驱动 |
| `display.char_mapping` | `update_head_display()` 硬编码等效逻辑 | ⚠️ 未JSON驱动 |
| `display.blink_rule.condition` | `derive_seg_blink()` 硬编码 `node==selecting` | ⚠️ 未JSON驱动 |
| `display.blink_rule.phase_ms` | `hmi_elements.blink_phase_ms` (已映射到elements) | ✅ |
| `display.alternate_rule` | `update_head_display()` 硬编码5s周期 | ⚠️ 未JSON驱动 |
| `display.led_rules` | `sync_leds()` 硬编码全部 | ❌ 未JSON驱动 |

---

### 1.8 patterns / mode_rules (显示图案和模式规则)

| JSON路径 | C结构体 | 消费状态 |
|----------|---------|---------|
| `elements.display_patterns.dash` | `hmi_patterns.dash` | ✅ |
| `elements.display_patterns.pa` | `hmi_patterns.pa` | ✅ |
| `elements.display_patterns.off` | `hmi_patterns.off` | ✅ |
| `elements.mode.rules[0..3]` | `hmi_mode_rules[]` | ✅ |
| `elements.mode.default` | `hmi_elements.mode_default` | ✅ |

---

## 二、JS引擎函数 → C函数 一一对应

### 2.1 初始化

| JS函数 (json_logic_engine.js) | 行号 | C函数 (app_hmi.c) | 行号 | 说明 |
|------------------------------|------|-------------------|------|------|
| `init(rulesJson)` | :103 | `App_Hmi_Init()` | :114 | 初始化heads/slots/状态/注册消息回调 |
| `createHeadInstance(index)` | :178 | 内联在 `App_Hmi_Init()` | :114 | HmiHead_t数组初始化 |
| `createDisplayCache()` | :194 | `s_disp` 静态变量 | :26 | HmiDisplayCache_t |

### 2.2 按键路由 (核心)

| JS函数 | 行号 | C函数 | 行号 | 说明 |
|--------|------|-------|------|------|
| `onKeyEvent()` | :304 | `on_key_event()` | :291 | 消息回调入口 |
| `getGlobalConfig(mode)` | :396 | 直接索引 `hmi_global_nodes[mode]` | — | |
| `getActiveProcessConfig(h)` | :410 | 内联在 `on_key_event()` | :291 | 查timer_setting/timer_active/boost_active |
| `resolveHeadRouteKey()` | :419 | `hmi_match_dynamic_route()` | :512 | HEAD_SELF/HEAD_OTHER解析 |
| `matchAndExecute()` | :437 | `hmi_match_dynamic_route()` + 内联遍历 | :512 | 路由表匹配+执行 |
| `isKeyOwnedByProcess()` | :427 | 内联在 `on_key_event()` | :291 | 进程"拥有"按键检查 |
| `resolveTarget()` | :1055 | 内联在 `on_key_event()` | :291 | 确定操作目标炉头 |

### 2.3 动作函数 (Zone Logic)

| JS函数 | 行号 | C函数 | 行号 | 功能 |
|--------|------|-------|------|------|
| `selectHead(index)` | :967 | `select_head()` | :756 | 选中炉头,自确认/切换 |
| `confirmSelect(headIdx)` | :650 | `confirm_select()` | :804 | 15s超时确认 |
| `handlePowerKey(level)` | :1011 | `handle_power_key()` | :827 | 档位0-9 |
| `enterBoost()` | :754 | `enter_boost()` | :882 | 进入Boost |
| `exitBoost(index)` | :770 | `exit_boost()` | :905 | Boost超时退出 |
| `exitBoostAndSetPower(level)` | :782 | `exit_boost_set_power()` | :925 | Boost中按档位键 |
| `enterTimerSetting()` | :695 | `enter_timer_setting()` | :951 | 进入定时设置 |
| `confirmTimer(headIdx)` | :670 | `confirm_timer()` | :972 | 确认定时 |
| `cancelTimerSetting()` | :708 | `cancel_timer_setting()` | :1001 | 取消定时设置 |
| `cancelTimerActive()` | :722 | `cancel_timer_active()` | :1021 | 取消定时运行 |
| `handleTimerAdjust(delta)` | :735 | `handle_timer_adjust()` | :1040 | +/-调整定时值 |
| `findTimerHead()` | :691 | 内联: `s_hot_head` | — | 找定时操作的炉头 |

### 2.4 全局模式转换

| JS函数 | 行号 | C函数 | 行号 | 功能 |
|--------|------|-------|------|------|
| `goWorking()` | :845 | `go_working()` | :629 | 进入WORKING |
| `goPoweredOff()` | :879 | `go_powered_off()` | :661 | 进入POWERED_OFF |
| `enterDeepSleep()` | :908 | `enter_deep_sleep()` | :690 | 进入DEEP_SLEEP |
| `togglePause()` | :942 | `toggle_pause()` | :717 | 切换暂停 |
| `toggleChildLock()` | :934 | `toggle_child_lock()` | :742 | 切换童锁 |
| `clearAllHeads()` | :1100 | `clear_all_heads()` | :1180 | 清空全部炉头 |
| `allLEDsOff()` | :1118 | `leds_all_off()` | :1375 | 全LED灭 |

### 2.5 动作分发器

| JS函数 | 行号 | C函数 | 行号 | 说明 |
|--------|------|-------|------|------|
| `executeAction(actionStr,...)` | :473 | `hmi_execute_action()` | :538 | switch(action)分发 |
| `applyActions(actionList)` | :465 | `hmi_apply_actions()` | :616 | 批量执行enter/exit |

### 2.6 显示更新

| JS函数 | 行号 | C函数 | 行号 | 说明 |
|--------|------|-------|------|------|
| `updateAllDisplays()` | :1171 | `update_all_displays()` | :1330 | 遍历4头更新 |
| `updateDisplayForHead(index)` | :1207 | `update_head_display()` | :1291 | 单头seg_chars写入 |
| `displayCharForHead(h)` | :1175 | 内联在 `update_head_display()` | :1291 | 优先级链: timer_setting>boost>power |
| `postDisplay()` | :1223 | `post_display()` | :1204 | 发送MSG_DISPLAY_REFRESH |
| `derive_seg_mode()` (ModeRule) | mode_rule.js:42 | `derive_seg_mode()` | :1223 | 优先级链求seg_mode |
| `derive_seg_blink()` (BlinkRule) | blink_rule.js:29 | `derive_seg_blink()` | :1264 | 闪烁掩码 |
| `syncLED()` | :824 | `sync_leds()` | :1392 | 档位灯+状态灯同步 |

### 2.7 显示辅助

| JS函数 | 行号 | C函数 | 行号 | 说明 |
|--------|------|-------|------|------|
| `showDash()` | :1088 | `show_dash()` | :1338 | 写 "--------" |
| `showPA()` | :1091 | `show_pa()` | :1349 | 写 "PAPAPAPA" |
| `showAllOff()` | :1096 | `show_all_off()` | :1360 | 写全空格 |
| `getPattern(name, fallback)` | :1079 | 内联: `hmi_patterns.xxx` | — | 读hmi_cfg.patterns |

### 2.8 选择栈管理

| JS函数 | 行号 | C函数 | 行号 | 说明 |
|--------|------|-------|------|------|
| `pushToStack(idx)` | :794 | `push_to_stack()` | :1115 | 压栈 |
| `removeFromStack(idx)` | :805 | `remove_from_stack()` | :1146 | 出栈 |
| `getStackTop()` | :810 | 内联: `s_select_stack[s_stack_top]` | — | 栈顶 |
| `reassignHotHead(headIdx)` | :814 | `reassign_hot_head()` | :1167 | 热点移交 |

### 2.9 守卫/超时

| JS函数 | 行号 | C函数 | 行号 | 说明 |
|--------|------|-------|------|------|
| `onTimer100ms()` | :553 | `on_timer_100ms()` | :1446 | zone超时+进程超时+guards |
| `onTimer1s()` | :615 | `on_timer_1s()` | :1545 | 定时倒计时 |

### 2.10 蜂鸣/辅助

| JS函数 | 行号 | C函数 | 行号 | 说明 |
|--------|------|-------|------|------|
| `postBuzzer(valid)` | :80 | `post_buzzer()` | :1704 | MSG_BUZZER_CTRL |
| `allHeadsIdle()` | :545 | `all_heads_idle()` | :1677 | 四头全idle? |
| `checkAnyTimer()` | :640 | `any_timer_active()` | :1695 | 任一头定时运行? |
| `any_timer_setting()` | — | `any_timer_setting()` | :1686 | 任一头定时设置中? |
| `resetIdleTimer()` | :1128 | `reset_idle_timer()` | :1671 | 重置空闲计时 |
| `cfg(key)` | :85 | `hmi_cfg.timeouts[id]` | — | 超时值查询 |

### 2.11 上电序列

| JS函数 | 行号 | C函数 | 行号 | 说明 |
|--------|------|-------|------|------|
| `runPowerOnSeqStep(step)` | :210 | `run_power_on_seq_step()` | :1588 | 执行一步 |
| `applySeqDisplay(seq)` | :266 | 内联在 `run_power_on_seq_step()` | :1588 | seq→display |

---

## 三、JS渲染元素 → C对应

### 3.1 SlotElement (slot_element.js)

| JS方法 | C对应 | 说明 |
|--------|-------|------|
| `createSlotElement(props)` | — | ❌ C未实现, value_map硬编码在 update_head_display |
| `render(h)` → `["1","0"]` | `update_head_display()` :1291 | C硬编码等效逻辑 |
| `renderPowerLevel(level)` | 内联: `h.power_level`→char | C未读value_map |
| `renderTimerValue(min)` | 内联: 2位数字转字符 | 逻辑一致 |
| `renderBoost()` | 内联: `'P'` + `' '` | C未读boost_char |
| `renderZero()` | 内联: `'0'` + `'0'` | C未读zero_char |

### 3.2 LEDElement (led_element.js)

| JS方法 | C对应 | 说明 |
|--------|-------|------|
| `createLevelLED(props)` | — | ❌ C未实现, mode硬编码为gradient |
| `levelLED.setLevel(n)` | 内联在 sync_leds() | |
| `levelLED.setBoost(b)` | 内联在 sync_leds() | |
| `levelLED.syncToCache(leds)` | `sync_leds()` :1392 | C硬编码gradient模式 |
| `createStatusLED(props)` | — | ❌ C未实现, 硬编码查表 |
| `statusLED.syncToCache(leds, val)` | `sync_leds()` :1392 | C硬编码mode_map+lvl_map |

### 3.3 BlinkRule (blink_rule.js)

| JS方法 | C对应 | 说明 |
|--------|-------|------|
| `createBlinkRule(props)` | — | ❌ C未实例化, hmi_elements存属性 |
| `shouldBlink(head)` | 内联在 `derive_seg_blink()` :1264 | 硬编码 `node==selecting` 且排除条件硬编码 |
| `syncToCache(heads, cache, gs)` | `derive_seg_blink()` :1264 | 暂停覆盖逻辑需验证 |

### 3.4 ModeRule (mode_rule.js)

| JS方法 | C对应 | 说明 |
|--------|-------|------|
| `createModeRule(props)` | — | ❌ C未实例化, 但hmi_mode_rules已声明 |
| `evaluate(globalState, heads)` | `derive_seg_mode()` :1223 | 优先级链遍历 |
| `syncToCache(globalState, heads, cache)` | `derive_seg_mode()` :1223 | 已完全JSON化 ✅ |

---

## 四、显示决策管线 (完整对应)

```
JS管线:                          C管线:
head state                      HmiHead_t
  ↓                               ↓
SlotElement.render(h)          update_head_display()    ← ⚠️ 硬编码,未读value_map
  ↓                               ↓
displayCache.seg_chars         s_disp.seg_chars
  ↓                               ↓
BlinkRule.syncToCache()        derive_seg_blink()       ← ⚠️ exclude_when硬编码
  ↓                               ↓
displayCache.seg_blink         s_disp.seg_blink
  ↓                               ↓
ModeRule.syncToCache()         derive_seg_mode()        ← ✅ 已JSON化
  ↓                               ↓
displayCache.seg_mode          s_disp.seg_mode
  ↓                               ↓
LEDElement.syncToCache()       sync_leds()              ← ❌ 完全硬编码
  ↓                               ↓
displayCache.leds              s_disp.leds_*
  ↓                               ↓
postDisplay() → MSG            post_display() → MSG
  ↓                               ↓
Renderer.draw()                sync_hmi_display()       ← drv_display.c:137
  ↓                               ↓
Canvas pixels                  IO[0..10] 物理缓冲
```

---

## 五、JSON动作字符串 → C动作枚举 映射表

| JSON action | C枚举 | param含义 |
|------------|-------|----------|
| `go_working` | `HMI_ACT_GO_WORKING` | — |
| `go_powered_off` | `HMI_ACT_GO_POWERED_OFF` | — |
| `go_deep_sleep` | `HMI_ACT_GO_DEEP_SLEEP` | — |
| `toggle_pause` | `HMI_ACT_TOGGLE_PAUSE` | — |
| `toggle_child_lock` | `HMI_ACT_TOGGLE_CHILD_LOCK` | — |
| `select_head N` | `HMI_ACT_SELECT_HEAD` | 炉头索引 0-3 |
| `confirm_select` | `HMI_ACT_CONFIRM_SELECT` | — |
| `confirm_select_immediate` | `HMI_ACT_CONFIRM_SELECT_IMMEDIATE` | — |
| `set_power N` | `HMI_ACT_SET_POWER` | 档位 0-9 |
| `enter_boost` | `HMI_ACT_ENTER_BOOST` | — |
| `exit_boost` | `HMI_ACT_EXIT_BOOST` | — |
| `exit_boost_set_power N` | `HMI_ACT_EXIT_BOOST_SET_POWER` | 退出后档位 |
| `enter_timer_setting` | `HMI_ACT_ENTER_TIMER_SETTING` | — |
| `confirm_timer` | `HMI_ACT_CONFIRM_TIMER` | — |
| `cancel_timer_setting` | `HMI_ACT_CANCEL_TIMER_SETTING` | — |
| `cancel_timer_active` | `HMI_ACT_CANCEL_TIMER_ACTIVE` | — |
| `timer_adjust N` | `HMI_ACT_TIMER_ADJUST` | delta (±1) |
| `show_dash` | `HMI_ACT_SHOW_DASH` | — |
| `show_pa` | `HMI_ACT_SHOW_PA` | — |
| `show_power_mode` | `HMI_ACT_SHOW_POWER_MODE` | — |
| `display_all_off` | `HMI_ACT_DISPLAY_ALL_OFF` | — |
| `clear_all_heads` | `HMI_ACT_CLEAR_ALL_HEADS` | — |
| `hothead_clear` | `HMI_ACT_HOTHEAD_CLEAR` | — |
| `clear_hothead` | `HMI_ACT_CLEAR_HOTHEAD` | — |
| `led_power_on/off` | `HMI_ACT_LED_POWER_ON/OFF` | — |
| `led_all_off` | `HMI_ACT_LED_ALL_OFF` | — |
| `led_pause_on/off` | `HMI_ACT_LED_PAUSE_ON/OFF` | — |
| `led_timer_on/off` | `HMI_ACT_LED_TIMER_ON/OFF` | — |
| `led_child_lock_on/off` | `HMI_ACT_LED_CHILD_LOCK_ON/OFF` | — |
| `reset_idle_timer` | `HMI_ACT_RESET_IDLE_TIMER` | — |
| `reset_off_timer` | `HMI_ACT_RESET_OFF_TIMER` | — |
| `beep_valid` | `HMI_ACT_BEEP_VALID` | — |
| `beep_invalid` | `HMI_ACT_BEEP_INVALID` | — |

---

## 六、JSON按键短名 → C键码 映射

| JSON短名 | C枚举 (KeyCode_t) | 用途 |
|----------|------------------|------|
| `POWER` | `KEY_ONOFF` | 开关 |
| `PAUSE` | `KEY_STOP` | 暂停 |
| `CHILD_LOCK` | `KEY_LOCK` | 童锁 |
| `HEAD_1` | `KEY_LEFT_P_SET_UP` | Z1键 (上左) |
| `HEAD_2` | `KEY_RIGHT_P_SET_UP` | Z2键 (上右) |
| `HEAD_3` | `KEY_LEFT_P_SET` | Z3键 (下左) |
| `HEAD_4` | `KEY_RIGHT_P_SET` | Z4键 (下右) |
| `TIMER` | `KEY_TIME_SET` | 定时 |
| `PLUS` | `KEY_ADD` | 加 |
| `MINUS` | `KEY_SUB` | 减 |
| `0`-`9` | `KEY_POWER_0`-`KEY_POWER_9` | 档位 |

**Z1~Z4映射规则** (必须与参考程序 `Key_dispose.c:2856-2859` 一致):
```
KEY_LEFT_P_SET_UP  → Z1 (上左, index 0)
KEY_RIGHT_P_SET_UP → Z2 (上右, index 1)
KEY_LEFT_P_SET     → Z3 (下左, index 2)
KEY_RIGHT_P_SET    → Z4 (下右, index 3)
```

**DDD显示布局验证** (参考 `Disp_Dispose.h`):
```
IO[0..1] = Z1 (上左)    IO[2..3] = Z2 (上右)
IO[4..5] = Z3 (下左)    IO[6..7] = Z4 (下右)
```

---

## 七、编码检查清单 (程序员自检)

修改任何HMI行为前，按以下顺序检查:

1. **[ ] 行为是否可在JSON中声明?** → 如果可以，只改JSON + 重新生成hmi_data.c
2. **[ ] C引擎是否需要新增动作枚举?** → 如需，在 app_hmi.h 和 hmi_execute_action() 同步添加
3. **[ ] 是否需要新增JSON配置段?** → 如需，在 gen_hmi.js 和 C结构体同步添加
4. **[ ] DDD术语是否一致?** → 变量名/注释用Z1~Z4而非"左/右/上/下"
5. **[ ] 按键→Zone映射是否正确?** → 对照第六节映射表
6. **[ ] 是否新增了硬编码?** → 如JSON已声明该规则，必须从hmi_cfg读取而非硬编码
7. **[ ] 编译0 error 0 warning?** → `armcc -c --cpu Cortex-M0+ -DSC32L14xx --c99 ...`

---

*维护规则: JSON配置段变更时同步更新本文档。新增JS函数时在第二节添加对应行。*
