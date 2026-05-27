# JSON 规格符合度审查报告

> 参照：`review-v8-json-spec-methodology.md` 审查标准
> 审查对象：`config/灯板逻辑_head_template.json`（下文简称"灯板JSON"）
> 对照依据：`hmi-data-flow-table.md` V7.0 × `design-methodology.md` × `four_head_v4.json`

---

## 1. 33 条数据流路由覆盖度

### 1.1 已覆盖（21/33）

| Flow# | 描述 | 灯板JSON | v4.json 参考 |
|-------|------|---------|-------------|
| 00 | 上电序列 | ✅ 3步完配 | ✅ |
| 01 | 关机↔开机 | ✅ POWER long → node_normal | ✅ `POWER long: go_working` |
| 02 | 选中炉头 | ✅ HEAD_N → node_selected | ✅ `HEAD_N tap: select_head N` |
| 03 | 设 0→idle | ✅ key 0 → set_power_0 → node_idle | ✅ |
| 04 | 设档→cooking | ✅ key 1-8 → set_power → node_working | ✅ |
| 05 | 短按 9 | ✅ KEY_9 event:release short_press → set_power 9 | ✅ |
| 06 | 长按 9→Boost | ✅ KEY_9 event:release long_press → boost_process.enter | ✅ |
| 09 | 选中→定时 | ✅ KEY_TIMER → node_timer_setting | ✅ |
| 11 | cooking→设0 | ✅ key 0 → set_power_0 → node_idle | ✅ |
| 12 | cooking→Boost | ✅ KEY_9 long → boost_process.enter | ✅ |
| 13 | 暂停 | ✅ toggle_pause | ✅ |
| 14 | cooking→定时 | ✅ KEY_TIMER → node_timer_setting | ✅ |
| 16 | Boost→数字键退出 | ✅ exit_by_key | ✅ |
| 17 | Boost 5min超时 | ✅ boost_timer.timeout → on_timeout | ✅ |
| 18 | Boost→定时键 | ✅ exit_by_non_power_key → node_timer_setting | ✅ |
| 19 | 暂停恢复 | ✅ toggle_pause | ✅ |
| 20 | 定时加减 | ✅ PLUS/MINUS → adjust ±1 | ✅ |
| 21 | 定时手动确认 | ✅ KEY_TIMER short → confirm | ✅ |
| 22 | 定时自动确认 | ✅ 5s timeout → confirm | ✅ |
| 23 | 定时取消 | ✅ KEY_TIMER long → cancel | ✅ |
| 24/25 | 童锁开关 | ✅ CHILD_LOCK long → toggle | ✅ |
| 26 | 关机 | ✅ POWER long → power_off → standby | ✅ |

### 1.2 未覆盖（12/33）

| Flow# | 描述 | 缺失情况 | 严重度 |
|-------|------|---------|--------|
| 07/08 | 选中15s超时确认 | ✅ 有超时逻辑，但 **5s ≠ 15s**（常量值错误） | 🔴 P0 |
| 10 | 单头快捷改档 | ❌ 缺 `resolveTarget()` 快捷逻辑：仅1头cooking时直接按档位键即生效 | 🟡 P1 |
| 15 | 定时归零→idle | ❌ 缺 `timer_active` 独立进程 + `timer_zero` 退出条件 | 🔴 P0 |
| 27 | 多炉头独立工作 | ⚠️ head_template 引擎×4 做了，但缺 **选择序列栈（LIFO）** | 🟡 P1 |
| 28 | 多炉头独立定时 | ⚠️ 同27，缺选择序列支持 | 🟡 P1 |
| 29 | Boost+定时交替 | ⚠️ 有 `boost_alternating` 显示模式，但缺进程层支持 | 🟡 P1 |
| 30 | 全 idle 30s→关机 | ❌ 完全缺失：无 `idle_to_off` 守卫计时器 | 🔴 P0 |
| 31 | POWERED_OFF 30s→休眠 | ❌ 完全缺失：无 `off_to_sleep` 计时器 | 🟡 P1 |
| 32 | 休眠→唤醒 | ❌ 完全缺失：无 `DEEP_SLEEP` 全局状态 | 🟡 P1 |
| 33 | 自按确认 | ❌ 完全缺失：无 `HEAD_SELF tap` → `confirm_immediate` | 🟡 P1 |

---

## 2. 动作名与引擎约定的一致性

### 2.1 路由表达格式不符

| 维度 | v4.json（参考实现） | 灯板JSON | 偏差分析 |
|------|-------------------|---------|---------|
| 路由键 | `"POWER long"` 字符串拼接 | `{trigger, event, condition}` 结构化 | 格式差异，双方都可行 |
| 动作值 | `"set_power 5"` 字符串+参数 | `{call, args}` 结构化 | 格式差异 |
| 超时表达 | `"timeout": {"ms_key":"select_confirm_ms", "action":"confirm_select"}` | `{"condition":"timeout(5000)", "call":"..."}` | 灯板JSON硬编码值，没有引timeout常量 |
| 进程退出 | `process_routes` 独立段 + `timeout` | 嵌入在 node events 数组里 | 结构差异大 |

### 2.2 动作命名偏差

| v4.json 动作名 | 灯板JSON 调用名 | 匹配？ |
|---------------|----------------|--------|
| `confirm_select` | `functions.timeout_exit` | ⚠️ 功能相同但名称不同 |
| `set_power N` | `functions.set_power`, `args: [N]` | ✅ 语义一致 |
| `enter_boost` | `boost_process.enter` | ✅ |
| `exit_boost` | `boost_process.on_timeout` | ⚠️ 名称不同，语义一致 |
| `exit_boost_set_power N` | `boost_process.exit_by_key(N)` | ✅ |
| `confirm_timer` | `timer_process.confirm` | ✅ |
| `cancel_timer_setting` | `timer_process.cancel` | ✅ |
| `select_head N` | 非独立函数，全局态通过trigger_key路由 | ⚠️ 架构不同 |
| `go_working` / `go_powered_off` | `global_state_machine.functions.power_on/off` | ✅ |

---

## 3. 超时常量对应关系

### 3.1 现状对比

| 常量 | hmi-data-flow 规格值 | 灯板JSON | v4.json | 判定 |
|------|-------------------|---------|---------|------|
| `select_confirm_ms` | 15000 | **5000**（硬编码） | 15000（常量引用） | 🔴 **错误** |
| `timer_confirm_ms` | 15000 | **5000**（硬编码） | 15000（常量引用） | 🔴 **错误** |
| `boost_max_ms` | 300000 | 300000（硬编码） | 300000（常量引用） | ✅ |
| `idle_to_off_ms` | 30000 | **缺失** | 30000 | 🔴 **缺失** |
| `off_to_sleep_ms` | 30000 | **缺失** | 30000 | 🔴 **缺失** |
| `default_timer_min` | 15 | 15（属性中） | 15 | ✅ |
| `power_on_all_on_ms` | 3000 | 3000（delay） | 3000 | ✅ |
| `version_show_ms` | 3000 | 3000（delay） | 3000 | ✅ |
| `power_key_long_ms` | 1500 | 引擎侧判断（`long_press`条件） | 1500 | ⚠️ 需引擎保证阈值 |

### 3.2 根因

灯板JSON**没有集中式的 `timeouts` 段**，所有超时值散落在各节点事件中且硬编码数字。这不仅导致选择超时和定时确认超时写成了5s（应为15s），而且无法统一审计。

---

## 4. 方法论体现度

### 4.1 已体现的设计模式

| 方法论规则 | 体现情况 | 评价 |
|-----------|---------|------|
| 声明WHAT不写HOW | ✅ 灯板JSON没有计时管理、显示绘制、状态转移函数体 | 符合 |
| 状态机=全局×Zone×正交进程 | ✅ 全局 state_machine + head_template(zone) + 独立 processes | 符合 |
| selecting是zone状态不是进程 | ✅ node_selected 是 zone 节点，有 15s 超时（虽然值写错） | 符合 |
| 进程退出条件多条件OR | ⚠️ 有概念但实现方式不同（events vs exit_conditions） | 部分符合 |

### 4.2 未体现的设计模式

| 方法论规则 | 缺失情况 | 影响 | 严重度 |
|-----------|---------|------|--------|
| **延时确认符号化封装** | 超时值硬编码，没有 `timeouts` 常量表 | 审计困难、改值困难 | 🔴 |
| **选择序列（Selection Stack）** | 完全没有LIFO栈、hotHead移交、LED回退逻辑 | 多炉头交互不可预测 | 🔴 |
| **计时器统一归零** | 任何有效按键→全部计时器归零。灯板JSON无此机制 | 用户操作后可能意外跳转 | 🟡 |
| **进程退出条件 vs 节点事件** | 进程退出条件在v4中是 `process_routes` + `timeout` 独立段；灯板JSON嵌在 node events 里 | 结构不够清晰 | 🟡 |
| **显示决策优先级链** | v4用 `display.priority_chain: ["timer_setting","boost_active","power_level"]`；灯板JSON嵌在 node enter_actions 的 if/else 里 | 改显示优先级要改多处 | 🟡 |

### 4.3 显示模型偏离

**v4.json 的显示模型**（方法论推荐）：
```json
"display": {
  "priority_chain": ["timer_setting", "boost_active", "power_level"],
  "blink_rule": { "condition": "node == selecting" },
  "alternate_rule": { "condition": "timer_active == true" },
  "led_rules": { "power_led": {"global_mode": {"powered_off": "blink", ...}} }
}
```

**灯板JSON 的显示模型**：
- 显示模式注册在 `element_registry` 但无优先级链
- 显示逻辑埋在 node enter_actions 的 if/else 里
- LED 规则分散在各 enter/exit actions 里

**后果**：新增一种显示模式（如温度显示）需要在所有节点的 enter_actions 里追加判断，违反"改JSON即改行为"原则。

---

## 5. 具体 BUG 清单

### 5.1 🔴 P0 错误（必须修）

| # | 位置 | 问题 | 正确值 |
|---|------|------|--------|
| B1 | `node_selected` timeout(5000) | 选中超时5s | 应 **15s** |
| B2 | `node_timer_setting` timeout(5000) | 定时确认超时5s | 应 **15s** |
| B3 | `node_standby` 显示 `dash+blink` | 待机态"--"闪烁 | hmi数据流表V7：POWERED_OFF 时 **"--"不闪烁**，仅电源灯闪烁 |
| B4 | `power_off` 函数设置 `dash+blink` | 关机态"--"闪烁 | 应不闪烁 |
| B5 | 缺 `all_idle→POWERED_OFF 30s` 守卫 | 全idle后不会自动关机 | 需加 `idle_to_off_ms: 30000` 守卫 |
| B6 | 缺 `POWERED_OFF→DEEP_SLEEP 30s` 计时 | 关机后不进入休眠 | 需加 `off_to_sleep_ms: 30000` 计时 |
| B7 | 缺 `DEEP_SLEEP` 全局状态 | 无休眠态 | 需添加 `node_deep_sleep` 节点 |
| B8 | 缺选择序列栈 | 无LIFO栈/hotHead移交 | 需补充栈逻辑（引擎函数） |

### 5.2 🟡 P1 问题（建议修）

| # | 位置 | 问题 |
|---|------|------|
| B9 | 全局 | 没有集中式 `timeouts` 段，常量散落硬编码 |
| B10 | `node_working` | 没有区分"单头快捷改档"和"多头必须先选"（缺 `resolveTarget()`） |
| B11 | `node_selected` | 缺 `HEAD_SELF` 自按确认（Flow-33） |
| B12 | 全局 | 缺 `resetAllTimers()` 计时器统一归零逻辑 |

### 5.3 架构偏差（方法论层面）

| # | 问题 | 建议 |
|---|------|------|
| D1 | 进程用 node events 实现，没有独立的 process_routes | 按 v4.json 拆出 `process_routes` 段 + `exit_conditions` |
| D2 | 显示逻辑埋在 enter_actions 里 | 改用 v4 的 `display.priority_chain` + `led_rules` |
| D3 | 动作名不一致（`timeout_exit` vs `confirm_select`） | 对齐 v4 命名约定 |

---

## 6. 改进建议

### 6.1 建议一：补充 `timeouts` 段

```json
"timeouts": {
  "select_confirm_ms": 15000,
  "timer_confirm_ms": 15000,
  "boost_max_ms": 300000,
  "idle_to_off_ms": 30000,
  "off_to_sleep_ms": 30000,
  "default_timer_min": 15,
  "power_on_all_on_ms": 3000,
  "version_show_ms": 3000,
  "power_key_long_ms": 1500
}
```

所有节点引用 `timeouts.select_confirm_ms` 而非硬编码数字。

### 6.2 建议二：补全全局状态机

```
node_powered_off ──30s→ node_deep_sleep
    │                      │
    └──长按开关──→ node_working
```

### 6.3 建议三：按 v4.json 重构为三段式路由

```
global_routes   → 全局态按键路由
zone_routes     → Zone态按键路由（idle/selecting/cooking）
process_routes  → 进程退出条件（timer_setting/timer_active/boost_active）
```

### 6.4 建议四：补充选择序列栈

```json
"selection_stack": {
  "description": "LIFO栈，最大深度4",
  "type": "engine_function",
  "functions": ["pushToStack", "removeFromStack", "getStackTop", "reassignHotHead"]
}
```

（这必须是引擎函数，不能是JSON声明）

---

## 7. 综合评分

| 维度 | 得分 | 说明 |
|------|------|------|
| 33条路由覆盖度 | 21/33 (64%) | 缺12条，含4条P0 |
| 动作名一致性 | 70% | 结构差异大，语义基本一致 |
| 超常量对应 | 4/9 (44%) | 5个正确、2个值错、2个缺失 |
| 方法论体现度 | 55% | 核心设计模式缺选择序列、显示优先级链 |
| 整体符合度 | **≈60%** | 基础架构对，细节偏差多 |

---

## 8. 改进优先级

| 优先级 | 待办 | 工作量 |
|--------|------|--------|
| 🔴 P0 | 修复超时值 5s→15s（B1/B2） | <5分钟 |
| 🔴 P0 | 修复待机态"--"闪烁（B3/B4） | <5分钟 |
| 🔴 P0 | 补全 idle_to_off + off_to_sleep + deep_sleep（B5/B6/B7） | 30分钟 |
| 🔴 P0 | 补充选择序列栈（B8） | 2小时（引擎函数） |
| 🟡 P1 | 增加集中式 timeouts 段（B9） | 15分钟 |
| 🟡 P1 | 对齐显示模型为 priority_chain（D2） | 1小时 |
| 🟡 P1 | 对齐进程模型为 process_routes（D1） | 1小时 |

---

*审查日期：2026-05-22*
*审查基准：hmi-data-flow-table.md V7.0 / design-methodology.md / four_head_v4.json*
