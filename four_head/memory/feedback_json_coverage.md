---
name: json-coverage-goal
description: 三层剥离架构——Zone Logic / JSON Binding / Presentation Controls，面向长期扩展
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 8468b018-0dc2-4dea-925a-f289e7ef9c0b
---

## 核心架构：三层剥离

```
Zone Logic (纯逻辑，每头独立完整)
  │  输出: power_level=5, timer_value=15, node=selecting, boost_active=true
  │  不关心: 这东西显示成什么、亮哪个灯
  │  永不变: 每个炉头有独立完整的逻辑流，输出语义值
  ▼
JSON Binding Layer (声明式绑定，连接逻辑与表现)
  │  声明: "slot[hotHead] ← zone.power_level, 用 value_map 渲染"
  │        "levelLed ← zone[hotHead].power_level, mode=single"
  │        "powerLed.state ← global.mode → behavior 查表"
  │  不关心: 控件怎么画、用什么颜色
  ▼
Presentation Controls (控件对象，读自己属性渲染输出)
  │  SegmentDisplay 读 value_map → 查表 → 输出字符
  │  LED 读 mode=single → 亮对应灯
  │  永不变: 控件类型固定 (数码管/LED/按键/蜂鸣器)
  │  会变: 属性值 (位数/映射表/模式)、新控件类型加入
```

## 三层职责

| 层 | 做什么 | 输入 | 输出 | 永远不变的是什么 |
|----|--------|------|------|-----------------|
| Zone Logic | 状态转移、定时倒数、功率/Boost/Timer管理 | 按键事件、定时器节拍 | 语义值 (power_level, timer_value, node等) | 每个炉头有独立完整逻辑流 |
| JSON Binding | 声明控件←→逻辑值的绑定关系、映射规则 | 逻辑层输出的语义值 + JSON绑定声明 | 控件方法调用 (带已映射的值) | 绑定是声明式的，不写在代码里 |
| Controls | 读属性→渲染输出 | 控件方法调用 + 控件自身JSON属性 | 显示字符、LED状态 | 控件类型固定，属性可配置，接受扩展 |

## "套壳"本质

同一个 Zone Logic 输出 `power_level=5`：
- 壳A: value_map={"5":" 5"} → 显示" 5" (档位模式)
- 壳B: value_map={"5":"10"} → 显示"10" (1000W模式)
- 壳C: value_map={"5":" V"} → 显示" V" (罗马字符)
- 壳D: digit_count=4, value_map={"5":"1000"} → 显示"1000" (4位W数)

**Zone Logic 一行代码不变。** 改JSON的 value_map 即改壳。

## 已有控件类型 + 可扩展方向

| 控件类型 | 属性 (JSON预配置) | 方法 (逻辑/绑定层调用) | 扩展示例 |
|---------|-------------------|----------------------|---------|
| SegmentDisplay | digit_count, value_map, blink_ms, dash_char, boost_char | setValue(n), setMode(mode), setBlink(bool) | 2位→4位, 档位→W数→罗马字符 |
| LED | mode(single/gradient), count, states_map, boost_override | setLevel(n), setState(on/off/blink), bindGlobalMode(mode) | 单点→梯度, 10灯→12灯 |
| LEDGroup | leds: [{name, states_map}] | setByName(name, state) | 新增LED个体 |
| Timer | range_min/max, step, countdown_unit, display_format | setValue(n), adjust(delta), getDisplay() | 0-99分→0-2:00时:分 |
| KeyFilter | whitelist, mode(block_all_except/allow_all) | filter(keyName)→bool | 不同状态不同白名单 |
| Buzzer | valid_freq, invalid_freq, duration_ms | beep(valid/invalid) | 不同音调模式 |

## 绑定层 JSON 结构草案

```json
"bindings": {
  "slot_bindings": {
    "source": "zones[hothead].power_level",
    "target": "segment_element",
    "transform": "value_map",
    "mode_bindings": {
      "timer_setting": { "source": "zones[hothead].timer_value", "format": "2digit" },
      "boost_active":  { "override_char": " P" }
    }
  },
  "level_led_bindings": {
    "source": "zones[hothead].power_level",
    "target": "level_led"
  },
  "power_led_bindings": {
    "source": "global.mode",
    "target": "power_led",
    "transform": "states_map"
  }
}
```

绑定层说清楚三件事：
1. **数据从哪来** (source): zones[hothead].power_level / global.mode
2. **数据到哪去** (target): segment_element / level_led / power_led
3. **怎么转换** (transform): value_map / states_map / format_template

引擎只做一件事：监听source变化 → 应用transform → 调用target的方法。

## 实现优先级

| 优先级 | 任务 | 影响 |
|--------|------|------|
| P0 | SegmentDisplay 元素 + value_map | 消除最大的硬编码块 displayCharForHead |
| P0 | LED 元素 + states_map | 消除 syncLED 和分散的LED操作 |
| P1 | Timer 元素属性 | 定时范围/步长/格式可配 |
| P1 | KeyFilter 白名单 | 童锁行为JSON化 |
| P2 | 绑定层 | 声明式source→target→transform |
| P2 | 格式模板引擎 | mm:ss / hh:mm 等复杂格式 |
| P3 | 新控件类型 | 温显NTC、触摸滑块等 |
