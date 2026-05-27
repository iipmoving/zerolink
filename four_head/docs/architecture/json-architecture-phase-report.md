# HMI JSON 驱动架构 — 阶段报告

> 状态: 稳定节点 | JSON覆盖率 ~90% | 测试 34/34 通过
> 日期: 2026-05-23

## 一、核心设计原则

**能配置化、参数化的信息全部用 JSON 表述，执行函数变成单向独立逻辑的程序段。**

每个问题停留在一个孤岛上——改 JSON 即改行为，引擎只做解释。AI 修改时面对的是独立信息处理问题，不需要大量上下文。

---

## 二、三层架构

```
┌──────────────────────────────────────────────────────┐
│  Zone Logic (引擎内置)                                │
│  selectHead / handlePowerKey / enterBoost / etc.     │
│  → 纯执行函数，所有决策参数从 JSON 读                  │
├──────────────────────────────────────────────────────┤
│  JSON Binding (声明式绑定)                            │
│  BlinkRule / ModeRule / 路由表 / Guards / Timeouts   │
│  → 从状态派生显示属性，从消息映射动作                   │
├──────────────────────────────────────────────────────┤
│  Presentation Controls (可配置元素)                   │
│  SlotElement / LevelLED / StatusLED ×5               │
│  → 元素对象持有 JSON 属性，引擎调方法不写缓存           │
└──────────────────────────────────────────────────────┘
```

---

## 三、显示双方法模型 (Two-Method Display)

### 3.1 核心认知

数码管显示有两个正交维度，不应混在一个枚举里：

```
显示 = ShowASCII(content) ⊕ ShowEffect(type, params)
         ↑ 内容层              ↑ 特效层
         APP 决定              APP 选择, DRV 执行
```

- **内容 (seg_chars[8])**：始终存在。APP 只管"这块屏上是什么"——档位数字、定时值、"PA"、"--"
- **特效 (seg_effect)**：可叠加。描述"怎么呈现"——常亮、闪烁、跑马灯、翻滚

### 3.2 分离原则

| 维度 | 谁决定 | 谁执行 | 可配置项 |
|------|--------|--------|---------|
| 内容 (ShowASCII) | APP | DRV → HAL | `value_map`, `boost_char`, `zero_char` |
| 特效 (ShowEffect) | APP 选择类型 | DRV 特效状态机 | `phase_ms`, `speed_ms`, `loop` |

每个 Zone 独立维护自己的内容和特效。加新特效只改 DRV 的 `show_effect()`，APP 的 `seg_chars` 赋值一行不动。

### 3.3 与现有元素的对应

```
内容层 → SlotElement (value_map 查表, seg_chars 直写)
特效层 → BlinkRule → 收敛为 EffectRule 的一种
         ModeRule  → 拆分为 ContentRule + EffectRule
         跑马灯    → EffectRule 新增类型
```

现有的 `seg_blink[4]` 不是独立的显示决策——它是 `effect=BLINK` 时的 per-zone 参数。

### 3.4 JSON 表达

```json
{
  "elements": {
    "segment_slot": {
      "value_map": {...},       // 内容属性
      "boost_char": "P",
      "zero_char": "0"
    },
    "effects": {
      "blink":   { "phase_ms": 300 },
      "marquee": { "speed_ms": 200, "loop": true }
    }
  }
}
```

重构指引详见 `docs/architecture/two-method-display-refactor-guide.md`。

### 3.5 LED 梯度显示：逻辑输出值，元素决定呈现

```
逻辑层输出: level = 5                                  (只一个数字)
LevelLED 根据自身 JSON 属性决定怎么展示:
  mode = "single"    → 只亮第 5 颗 LED                 (LevelLED 属性决定)
  mode = "gradient"  → 亮第 0~5 颗 LED (柱状条)       (LevelLED 属性决定)
```

LevelLED 的 JSON 属性增加 `mode` 字段：

```json
"elements": {
  "led": {
    "level": {
      "count": 10,
      "mode": "gradient",        // "single" | "gradient"
      "boost_override": { ... }
    }
  }
}
```

逻辑层永远只调 `levelLED.setLevel(n)`——是亮单颗还是亮柱状条，是 LevelLED 自己读 `mode` 决定的。

### 3.6 长按阈值统一到 timeouts

所有时间阈值收敛到 `timeouts` 段，包括按键长按：

```json
"timeouts": {
  "select_confirm_ms": 15000,
  "deep_sleep_ms": 30000,
  "long_press_ms": 1500,          // 长按判定阈值
  "key_debounce_ms": 50           // 去抖（已在驱动层实现）
}
```

键码编码方案（已验证）：键码最高位 `0x80` 标记"按下/释放"，驱动层硬件去抖后直接投递消息，长按判定是 Zone Logic 读 `long_press_ms` 做的。

### 3.7 消息总线对接

所有元素更新走消息总线，不直接调用：

```
规则引擎 (postDisplay)              显示元素 (DRV)
─────────────────────              ────────────────
derive seg_chars, seg_effect
    │
    └→ Msg_Post(DISPLAY_REFRESH, cache)
                                       │
         on_display_refresh()  ←───────┘
              │
              ├→ ShowASCII(d.seg_chars)    → s_io_work[]
              └→ ShowEffect(d.seg_effect)  → s_io_work[] (叠加)
```

规则引擎是**消息发布者**，显示元素是**消息订阅者**。引擎不直接调 `Drv_Display_*`，元素不直接读 `heads[]`。

### 3.8 转换工具同步

JSON 规则稳定后，`cfg/gen_hmi.js`（JS → C 转换工具）需同步更新：

| 新增 JSON 字段 | C 端落点 | 转换任务 |
|---------------|---------|---------|
| `elements.effects.*` | `HmiElementCfg_t` 新增段 | 生成 `hmi_effects` 数组 |
| `elements.led.level.mode` | `HmiElementCfg_t.led.level_mode` | 枚举映射 `"single"→0, "gradient"→1` |
| `timeouts.long_press_ms` | `HmiTimeouts_t.long_press_ms` | 直接数值 |
| `timeouts.key_debounce_ms` | `HmiTimeouts_t.key_debounce_ms` | 直接数值 |

---

## 四、JSON 命名方式

### 4.1 文件约定

- 逻辑规则: `logic/four_head_v4.json`
- 界面布局: `fourSave.json`（独立，仅渲染器使用）
- 版本号在 `meta.version` 中声明

### 4.2 顶层命名空间

```json
{
  "meta": {},              // name, version, head_count, description
  "timeouts": {},          // 所有超时阈值 (ms)
  "power_on_sequence": [], // 上电序列步骤
  "global_routes": {},     // 全局模式按键路由
  "zone_routes": {},       // Zone 状态按键路由
  "process_routes": {},    // 正交进程按键路由
  "display": {},           // 显示决策规则 (旧路径, 逐步废弃)
  "elements": {}           // 控件元素属性声明 (新路径)
}
```

### 4.3 路由键格式

```
"<KEY_SHORT> <event>"  →  "<action> [param]"

示例:
  "POWER long"          →  "go_working"
  "HEAD_1 tap"          →  "select_head 0"
  "5 tap"               →  "set_power 5"
  "9 long"              →  "enter_boost"
  "TIMER tap"           →  "enter_timer_setting"
  "HEAD_SELF tap"       →  "confirm_select_immediate"  (动态解析)
  "HEAD_OTHER tap"      →  "select_head %head"          (%head 替换为实际索引)
```

按键名: `POWER | TIMER | PAUSE | CHILD_LOCK | HEAD_1~4 | ZONE | PLUS | MINUS | 0~9`
事件: `tap | long`

### 4.4 动作命名

```
全局:  go_working | go_powered_off | go_deep_sleep | toggle_pause | toggle_child_lock
Zone:  select_head <N> | confirm_select | confirm_select_immediate | set_power <N>
Boost: enter_boost | exit_boost | exit_boost_set_power <N>
定时:  enter_timer_setting | confirm_timer | cancel_timer_setting | cancel_timer_active | timer_adjust <delta>
显示:  show_dash | show_pa | show_power_mode | display_all_off | clear_all_heads | hothead_clear
LED:   led_power_on | led_power_off | led_timer_on | led_timer_off | led_pause_on | led_pause_off | led_child_lock_on | led_child_lock_off | led_all_off
```

所有 LED 动作被视为"命令"（有意保留），与 StatusLED "状态派生"区分。

### 4.5 进入/退出动作

每个全局模式的 `enter_actions` / `exit_actions` 按数组顺序执行:

```json
"powered_off": {
  "enter_actions": ["clear_all_heads", "show_dash", "led_power_on", ...],
  "routes": { ... },
  "guards": [ ... ]
}
```

### 4.6 超时引用

`timeouts` 中所有 key 以 `_ms` 结尾。包括按键长按阈值和去抖时间：

```json
"timeouts": {
  "select_confirm_ms": 15000,
  "deep_sleep_ms": 30000,
  "long_press_ms": 1500,       // 长按判定阈值
  "key_debounce_ms": 50        // 去抖（驱动层已实现，JSON 为声明性记录）
}
```

路由中的 `ms_key` 引用这些 key:

引擎通过 `cfg(key)` 函数读取，JSON 优先，回退 `hmi_config.js`。

### 4.7 元素属性命名

| 元素 | JSON 键 | 属性风格 |
|------|---------|---------|
| SlotElement | `elements.segment_slot` | `digit_count`, `value_map`, `boost_char`, `zero_char` |
| LevelLED | `elements.led.level` | `count`, `mode`, `boost_override` |
| StatusLED | `elements.led.<name>` | `states`/`flag`/`condition`/`array` + `ledField` |
| BlinkRule | `elements.blink` | `phase_ms`, `condition`, `exclude_when`, `pause_override` |
| ModeRule | `elements.mode` | `default`, `rules[{when, value}]` |
| Timer params | `elements.timer` | `adjust_min`, `adjust_max`, `adjust_step` |
| Boost params | `elements.boost` | `power_level` |
| Display patterns | `elements.display_patterns` | `dash`, `pa`, `off` |
| Stack | `elements.stack` | `max_depth` |

属性命名: `snake_case`，布尔值用 `true/false`，枚举用字符串。

---

## 五、函数调用方式

### 5.1 元素创建模式 (Factory Pattern)

每个元素类型暴露 `create<Name>(props)` 工厂函数，返回方法对象:

```js
// slot_element.js
var SlotElement = (function() {
    function createSlotElement(props) { ... return { render }; }
    return { createSlotElement };
})();

// led_element.js
var LEDElement = (function() {
    function createLevelLED(props) { ... return { setLevel, setBoost, clear, syncToCache }; }
    function createStatusLED(props) { ... return { syncToCache }; }
    return { createLevelLED, createStatusLED };
})();

// blink_rule.js
var BlinkRule = (function() {
    function createBlinkRule(props) { ... return { shouldBlink, syncToCache, phaseMs }; }
    return { createBlinkRule };
})();
```

**规则:**
- 所有元素用 IIFE 模块模式，挂到 `window` 级 `var`
- `props` 来自 JSON 对应段，`null` 时工厂使用内置默认值
- 返回对象是纯方法集合，不暴露内部状态

### 5.2 引擎集成模式 (init → postDisplay)

```js
// init() — 创建元素实例
var slotProps = rules.elements.segment_slot || null;
slots.push(SlotElement.createSlotElement(slotProps));

// postDisplay() — 从状态派生显示
blinkRule.syncToCache(heads, displayCache, globalState);
modeRule.syncToCache(globalState, heads, displayCache);
```

**init 顺序**: MessageBus → load JSON → create elements → set globalState → register handlers → start sequence

**postDisplay 顺序**: ModeRule (seg_mode) → BlinkRule (seg_blink) → MessageBus.post(DISPLAY_REFRESH)

### 5.3 回调注册模式

```js
MessageBus.register(MsgId.MSG_KEY_EVENT, onKeyEvent);
MessageBus.register(MsgId.MSG_TIMER_100MS, onTimer100ms);
MessageBus.register(MsgId.MSG_TIMER_1S, onTimer1s);
```

所有引擎入口通过消息总线，不直接暴露函数。

### 5.4 动作分发模式

```js
// JSON action string → switch-case → 引擎函数
function executeAction(actionStr, target, keyName, silent) {
    var parts = actionStr.split(' ');
    var action = parts[0];
    var param = parts[1] || 0;
    switch (action) {
        case 'select_head': selectHead(param); break;
        case 'set_power':   handlePowerKey(param); break;
        // ...
    }
}
```

`%head` 参数在分发前动态替换为实际炉头索引。

### 5.5 配置读取模式

```js
// 超时: cfg() 统一入口，JSON优先，hmi_config.js回退
function cfg(key) {
    if (rules && rules.timeouts && rules.timeouts[key] !== undefined)
        return rules.timeouts[key];
    // fallback to hmi_config.js globals
}

// 元素属性: 直接读 rules.elements.<section>，不存在则默认
var bCfg = (rules && rules.elements && rules.elements.boost) ? rules.elements.boost : {};
h.power_level = bCfg.power_level !== undefined ? bCfg.power_level : 9;
```

---

## 六、覆盖度演进

| 阶段 | 内容 | 消除硬编码 | 测试 |
|------|------|-----------|------|
| P0-1 | timeouts, power_on_seq, enter/exit actions, display chain | ~30处 | 34/34 |
| P0-2 | SlotElement + value_map 查表渲染 | 4×2位数码管渲染 | 34/34 |
| P0-3 | LEDElement (LevelLED + StatusLED 骨架) | syncLED() 19→8行 | 34/34 |
| P0-4 | StatusLED 全面集成 5 种状态灯 | ~35处 LED 硬编码 | 34/34 |
| P1 | BlinkRule 闪烁绑定 | ~21处 seg_blink | 34/34 |
| P2 | ModeRule seg_mode 优先级链 | ~15处 seg_mode | 34/34 |
| P3 | 6孤岛参数挤出 + 回退死代码删除 | 40行+6处 | 34/34 |

**总计**: 7 轮迭代，~107 处硬编码移除，覆盖率 62% → 90%，测试始终保持 34/34。

---

## 七、当前 JSON 元素清单

| 实例 | 类型 | JSON 段 | 实例数 | 职责 |
|------|------|---------|--------|------|
| slotZ0~3 | SlotElement | `elements.segment_slot` | 4 | 炉头数字→段码字符映射 |
| levelLED | LevelLED | `elements.led.level` | 1 | 10档位 LED (mode=single/gradient) |
| powerLED | StatusLED | `elements.led.power` | 1 | 电源 LED (states-map型) |
| timerLED | StatusLED | `elements.led.timer` | 1 | 定时 LED (condition型) |
| pauseLED | StatusLED | `elements.led.pause` | 1 | 暂停 LED (flag型) |
| lockLED | StatusLED | `elements.led.child_lock` | 1 | 童锁 LED (flag型) |
| selLED | StatusLED | `elements.led.head_select` | 1 | 选头 LED ×4 (array型) |
| blinkRule | BlinkRule | `elements.blink` | 1 | 闪烁条件+暂停覆盖 |
| modeRule | ModeRule | `elements.mode` | 1 | 全局显示模式优先级链 |
| - | EffectCfg | `elements.effects` | 1 | 特效参数 (blink/marquee/...) |

**9 个元素类型，12 个实例 + 1 个特效配置。全部从 JSON 读取属性，引擎不存默认值。**

---

## 八、剩余孤岛 (可接受的引擎内置代码)

| 函数 | 类型 | 可 JSON 化参数 |
|------|------|-------------|
| selectHead | 选中状态机 | 自确认行为标志 |
| handlePowerKey | 功率切换 | 清timer/boost策略 |
| confirmSelect | 确认逻辑 | power>0阈值 |
| enterBoost/exitBoost | Boost生命周期 | auto_confirm |
| enterTimerSetting/confirm/cancel | 定时生命周期 | allowed_nodes, cancel_action |
| resolveTarget | 目标解析 | auto_select_single规则 |
| pushToStack/removeFromStack | 栈管理 | max_depth (已JSON) |
| clearAllHeads | 重置 | 重置字段列表 |
| onTimer1s | 倒计时 | 归零行为 |

这些函数的决策参数可进一步 JSON 化（二档），但当前作为引擎标准实现已可接受。

---

## 九、测试体系

- **测试定义**: `js/test_runner.js` — 34 条数据流测试，按 Phase 1~4 组织
- **Node.js 运行**: `run_tests_node.js` — VM 沙箱加载浏览器模块
- **浏览器交互**: `test_hmi.html` — Canvas 渲染 + 命令行输入
- **浏览器自动**: `test_auto.html` — 一键运行全部测试

测试覆盖上电序列、开关机、选头、设档、Boost、定时、暂停、童锁、多炉头、休眠全部路径。

---

## 十、经验总结

1. **先剥离参数，再剥离逻辑。** P0-1 从 timeouts 入手（纯数值），最容易验证。P1/P2 剥离显示派生逻辑（条件求值），需要定义规则格式。顺序错了会返工。

2. **元素对象模式是正确抽象粒度。** 每个元素: JSON 属性 → 工厂创建 → 方法接口 → syncToCache 写入。BlinkRule/ModeRule 是这个模式的自然延伸。

3. **JSON 优先，回退兜底。** 每个 cfg()/props 读取都有默认值。JSON 缺失时引擎不崩溃，测试保持全绿。这保证了"改 JSON 即改行为"的安全性。

4. **postDisplay() 是唯一显示出口。** 所有显示属性派生集中在这一处，顺序: ModeRule → BlinkRule → MessageBus.post。不再有任何函数直接写 displayCache 的显示模式字段。

5. **暂停是 flag 不是 mode。** `globalState.paused` 与 `globalState.mode='working'` 叠加。ModeRule 和 BlinkRule 都检查 flag 而非 mode。这是产品语义的正确表达。

6. **死代码要果断删除。** `onTimer100ms` 回退 ~40 行和 `postDisplay` 的 `mode==='paused'` 检查都是永假分支。保留它们增加维护负担。

---

*本报告由技术负责人归档，文档管理员审核。后续迭代以此为基线。*
