# [已覆盖] JSON覆盖率分析与提升路线

> 对照方法论"三层剥离"，分析当前项目 `four_head_v4.json` + `json_logic_engine.js` 的差距。

## 一、当前覆盖 vs 目标覆盖

| 元素类型 | 当前状态 | 目标状态 | 差距 |
|---------|---------|---------|------|
| SegmentDisplay | 引擎直接写 seg_chars[] 数组，硬编码字符格式 | 元素对象，value_map查表渲染 | 大 |
| LED | syncLED()硬编码，只读boost_override | 元素对象，属性声明mode/states/count | 大 |
| Timer | 硬编码 1-99分钟，+/-1步长，2位数字格式 | 元素对象，JSON声明范围/步长/format | 中 |
| KeyFilter | block_all_except声明了但引擎不用 | 元素对象，白名单+模式 | 中 |
| 全局模式→LED绑定 | 分散在各 go*() 函数中硬编码 | 绑定层声明，引擎自动查表 | 中 |

## 二、逐元素分析

### 2.1 SegmentDisplay — 当前最硬的部分

**当前代码** (displayCharForHead, 第1160-1191行):
```
优先级链从JSON读（√），但每项的格式化全硬编码：
  timer_setting → formatTimerChars() 写死2位数字
  boost_active  → 读JSON的char/align（部分√），默认'P'写死
  power_level   → 0显示"00"，非0显示"一位+空格"（全硬编码）
```

**目标**: JSON声明 value_map，引擎查表。逻辑层只调 slot.setValue(5)，slot 自己渲染。

**对当前JSON的兼容**: 现有的 priority_chain + char_mapping 可以作为简化声明保留，在它基础上增加 value_map 字段。如果 value_map 存在就用映射表，不存在就用现有硬编码逻辑回退。

### 2.2 LED — 6条规则只用了1条

**当前代码** (syncLED, 第818-837行):
```
只读 boost_override。其余5条规则（power_led, child_lock_led, timer_led,
pause_led, head_select_led）的 JSON 声明完全被忽略。
各LED的亮/灭/闪分散在 goWorking(), goPoweredOff(), toggleChildLock(),
togglePause(), drawLED() 等多处硬编码。
```

**目标**: 绑定层声明 source=global.mode + states_map，引擎自动调控件。

**待定问题**: power LED 在 powered_off 态闪烁——这个闪烁是渲染器 drawLED() 硬编码的（第126-129行）。如果改由JSON的 states_map 驱动，渲染器需要支持 blink 方法。

### 2.3 Timer — 全硬编码

**当前代码**:
```
范围: 1-99 分钟 (硬编码在 handleTimerAdjust 742行)
步长: +/-1 (硬编码)
显示: 10以上2位, 10以下1位+空格 (硬编码在 formatTimerChars 1194行)
递减: onTimer1s 每秒减1分钟 (硬编码 612行)
```

**目标**: 定时器元素属性声明 range/step/format。如果以后要改成秒递减+mm:ss显示，改JSON的 countdown_unit + display_format 即可。

### 2.4 KeyFilter（童锁白名单）— JSON写了但未生效

**当前代码** (onKeyEvent, 第307-309行):
```js
if (globalState.child_lock) {
    postBuzzer(false); return;  // 拦截一切，白名单不生效
}
```

JSON 里 `block_all_except: ["POWER"]` 完全没用。

**目标**: 每个 global_routes mode 声明自己的 key_filter，引擎在路由前先过过滤器。

### 2.5 全局模式→元素行为的绑定

**当前**: 分散在状态转移函数中:
- goWorking(): 硬编码 led_power_on, led_timer_off, ...
- goPoweredOff(): 硬编码 led_power_on, show_dash, ...
- togglePause(): 硬编码 led_pause_on/off

**目标**: 绑定层声明 source=global.mode→target=控件 + states_map，引擎在模式变化时自动遍历执行。

---

## 三、当前JSON字段利用情况

JSON 共 ~68 字段，引擎实际使用 42 个（62%），26 个被忽略。

**被忽略且可以立刻读取的（低难度，不改架构）**:

| 字段 | 位置 | 读取难度 | 说明 |
|------|------|---------|------|
| `child_lock.block_all_except` | global_routes | 低 | 已有JSON，引擎加个数组遍历 |
| `power_key_long_ms` | timeouts | 低 | cfg()加个key映射 |
| `char_mapping.*.source` | display | 低 | 引擎按source字段取数据源 |
| `led_rules.power_led` | display | 低 | 替代硬编码的led_power_on/off动作 |

**需要元素架构改造才能用的（中高难度）**:

| 字段 | 依赖 | 说明 |
|------|------|------|
| `value_map` (新增) | SegmentDisplay元素 | 需要元素对象+查表渲染 |
| `led_rules` 其余5条 | LED元素 | 需要states表+引擎自动遍历 |
| `blink_rule.condition` | 元素属性 | 需要表达式评估器 |
| `alternate_rule.condition/source` | 元素属性 | 需要复合显示协调 |
| `char_mapping.*.format` | 格式模板 | 需要模板引擎 |

---

## 四、6个问题的答案（基于三层剥离架构，已确认）

### Q1: 元素粒度 → 4个独立 Slot 元素

4个炉头 = 4个 Slot 元素实例，各自独立属性（value_map、digit_count等）。绑定层将 `zones[N].power_level` 连接到 `slot[N]`。

价值：炉头1显W数、炉头2显档位、炉头3显罗马字符，各自改 value_map 即可，互不影响。

### Q2: LED档位灯数据绑定 → 绑定层声明

```json
"level_led_binding": {
  "source": "zones[hothead].power_level",
  "target": "level_led"
}
```

引擎职责：监听 source 变化 → 调 `levelLed.setLevel(newValue)`。LED控件只负责"收到值→按mode(single/gradient)渲染"，不关心值从哪来。hotHead 切换时引擎检测到 source 变化，自动推新值。

### Q3: global_state_map → 绑定声明 + 控件属性

两段式：
- LED 控件属性声明支持的行为：`"behaviors": ["on", "off", "blink"]`, `"blink_ms": 500`
- 绑定声明连接全局模式到行为：`"source": "global.mode", "target": "power_led", "transform": "states_map"`, states_map = `{"powered_off": "blink", "working": "on", "deep_sleep": "off"}`

引擎在 global.mode 变化时遍历所有绑定，匹配到 source=global.mode → 查 states_map → 调控件方法。LED控件不知道也不关心全局模式。

### Q4: 向后兼容 → 新增段共存

```json
{
  "display": { ... },
  "elements": { ... },
  "bindings": { ... }
}
```

引擎读取顺序：`elements` + `bindings` 存在 → 用新引擎路径；不存在 → 走现有 `display` 段 + 硬编码回退。客户现场现有JSON不做任何修改就能跑。

### Q5: 复合显示（timer_active交替） → 绑定层交替声明

一个 Slot 绑定多个 source，声明交替规则：

```json
"slot_alternate_binding": {
  "target": "slot[hothead]",
  "condition": "zones[hothead].timer_active == true",
  "phases": [
    { "source": "zones[hothead].power_level", "transform": "value_map", "duration_ms": 5000 },
    { "source": "zones[hothead].timer_value", "transform": "format_timer", "duration_ms": 5000 }
  ]
}
```

引擎检测 condition 为 true → 启动交替定时器 → 按 phases 轮流推值到 slot。condition 为 false → 回退默认单 source。逻辑层和控件层都不感知交替。

### Q6: 按键过滤器 → 每个模式可配白名单

放在每个 global_routes mode 里，与 routes 同级：

```json
"powered_off": {
  "key_filter": { "mode": "whitelist", "keys": ["POWER"] },
  "routes": { ... }
},
"working": {
  "key_filter": { "mode": "none" },
  "routes": { ... }
}
```

引擎在进入路由匹配前，先查当前模式的 key_filter。mode=none 跳过过滤，mode=whitelist 只放行白名单内按键。child_lock 不再是一种特殊全局flag——它只是 working 模式下的一种 key_filter 实例。

---

## 五、实施路线

| 优先级 | 任务 | 影响 | 依赖 |
|--------|------|------|------|
| P0-1 | 低难度4项快速修复 | JSON覆盖率 62%→68% | 无，直接改引擎读取逻辑 |
| P0-2 | Slot 元素类 + value_map | 消灭 displayCharForHead 硬编码 | 需定义元素接口 |
| P0-3 | LED 元素类 + states_map | 消灭 syncLED + 分散LED操作 | 需定义元素接口 |
| P1-1 | 绑定层引擎（source→target→transform） | 状态转移函数不再直接操作LED | P0-2, P0-3 |
| P1-2 | KeyFilter 白名单生效 | 童锁行为JSON化 | P1-1（绑定机制） |
| P1-3 | Timer 元素属性 | 定时范围/步长/格式可配 | 需格式模板引擎 |
| P2 | 交替绑定、格式模板、表达式condition | 复合显示、mm:ss格式 | P1-1 |
| P3 | 新控件类型（温显、滑块） | 未来面板扩展 | 元素接口稳定后 |
