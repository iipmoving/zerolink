# JSON规则驱动架构实施方案 V3.0（最终可落地版）

> **文档版本**：V3.0  
> **创建日期**：2026-02-03  
> **作者**：EMC团队  
> **状态**：待实施  

---

## 一、总体概述

### 1.1 项目目标

将电磁炉控制面板的全部逻辑，用JSON配置文件来描述。框架程序加载JSON后自动解析并执行，实现完整的按键显示逻辑仿真。

### 1.2 【用户分解】核心设计原则

> **说明**：以下原则来自用户的实际设计思路分解，是本项目的方法论基础。

#### 【用户分解1】原则1：声明式配置与逻辑分离

**核心理念**：JSON只声明"是什么状态"，不关心"如何实现"。

**示例对比**：

❌ **错误做法（过度设计）**：
```json
{
  "action": "display_segment",
  "value": 5,
  "blink": false,
  "decimal": false,
  "hardware_pin": "SEG1"
}
```

✅ **正确做法（声明式）**：
```json
{
  "action": "segment.set_mode",
  "args": ["power"]
}
```

**说明**：
- JSON只需要标识显示模式为"power"（功率档位）
- 具体的功率值（5档）由程序内部状态维护
- 显示逻辑（如何渲染段码）由处理器实现
- JSON不关心硬件细节

#### 【用户分解2】原则2：元素状态标识化（核心理念）

> **用户原话**："先将各种元素的属性与方法规划好，比如LED的闪烁、常量、关至少是三种状态，后面还可以加。数码管显示功率、温度、时间、跑马、时间+功率交替这都是专门的状态，JSON只需标志显示的状态，程序自然就可以后面去调用相应的内部变量，根本不用说要显示什么功率值。"

**核心理念**：所有UI元素（LED、数码管、定时器等）的状态用**标识符**表示，具体值由程序内部维护。

**关键理解**：
- ✅ JSON只声明"是什么状态"（如 `segment.set_mode("power")`）
- ❌ JSON不关心"具体值是多少"（如 power_level=5）
- ✅ 程序内部自动从 `internal_state.power_level` 获取真实值
- ✅ 显示逻辑由处理器实现，JSON完全不关心

**示例对比**：

❌ **错误做法（过度设计）**：
```json
{
  "action": "display_segment",
  "value": 5,  // JSON不应该关心具体值
  "blink": false,
  "decimal": false,
  "hardware_pin": "SEG1"
}
```

✅ **正确做法（声明式）**：
```json
{
  "action": "segment.set_mode",
  "args": ["power"]  // JSON只标识显示模式
}
```

**说明**：
- JSON只需要标识显示模式为"power"（功率档位）
- 具体的功率值（5档）由程序内部状态维护
- 显示逻辑（如何渲染段码）由处理器实现
- JSON不关心硬件细节

**扩展性优势**：
- 新增显示模式（如跑马灯），只需在注册表中添加新标识
- JSON使用时无需修改结构，直接调用新标识即可
- 处理器内部实现新逻辑，对外透明

**LED状态标识**：
```json
{
  "states": {
    "off": "关闭",
    "on": "常亮",
    "blink": "闪烁",
    "breathe": "呼吸灯",
    "custom": "自定义"
  }
}
```

**数码管显示模式标识**：
```json
{
  "display_modes": {
    "power": "显示功率档位",
    "temperature": "显示温度",
    "timer": "显示定时时间",
    "marquee": "跑马灯效果",
    "alternating": "交替显示（时间+功率）",
    "ascii": "显示ASCII字符",
    "dash": "显示横杠（待机）",
    "blank": "空白"
  }
}
```

**说明**：
- JSON只需要设置 `segment.set_mode("power")`
- 程序内部自动从 `internal_state.power_level` 获取真实值
- 显示逻辑由 `SegmentHandler` 处理器实现
- 后续新增显示模式（如跑马灯），只需注册新标识，JSON无需修改

#### 【用户分解3】原则3：功能模块独立进程

> **用户原话**："定时、暂停、组合、童锁这些都是独立进程，可以以函数的形式供主功率调整调用，所以只需把调功率的主线搞清楚，就可以进行扩展，有新的功能原来的逻辑处理不了，可以新注册一个元素进行处理，也就是在CASE里加一条。"

**核心理念**：定时、暂停、Boost、童锁等功能作为**独立进程**，以函数形式供主线调用。

**架构示意**：
```
主线：调功率流程（上电→待机→选炉头→调功率→工作）
  │
  ├─ 独立进程1：定时（timer_process）
  ├─ 独立进程2：暂停（pause_process）
  ├─ 独立进程3：Boost（boost_process）
  ├─ 独立进程4：童锁（child_lock_process）
  └─ 独立进程5：组合（zone_process，暂缓）
```

**关键理解**：
- ✅ 主线逻辑只关注调功率流程（核心业务）
- ✅ 其他功能作为独立进程，需要时调用
- ✅ 新增功能只需注册新进程，不影响主线
- ✅ 很多复杂的功能在JSON里可能就是一个标识（如跑马灯）

**JSON调用示例**：
```json
{
  "events": [
    {
      "trigger": "key_timer",
      "call": "timer_process.start",
      "args": [15]
    },
    {
      "trigger": "key_pause",
      "call": "pause_process.toggle"
    }
  ]
}
```

**说明**：
- 主线逻辑只关注调功率流程
- 其他功能作为独立进程，需要时调用
- 新增功能只需注册新进程，不影响主线

**架构示意**：
```
主线：调功率流程（上电→待机→选炉头→调功率→工作）
  │
  ├─ 独立进程1：定时（timer_process）
  ├─ 独立进程2：暂停（pause_process）
  ├─ 独立进程3：Boost（boost_process）
  ├─ 独立进程4：童锁（child_lock_process）
  └─ 独立进程5：组合（zone_process，暂缓）
```

**JSON调用示例**：
```json
{
  "events": [
    {
      "trigger": "key_timer",
      "call": "timer_process.start",
      "args": [15]
    },
    {
      "trigger": "key_pause",
      "call": "pause_process.toggle"
    }
  ]
}
```

**说明**：
- 主线逻辑只关注调功率流程
- 其他功能作为独立进程，需要时调用
- 新增功能只需注册新进程，不影响主线

#### 【用户分解4】原则4：四炉头独立逻辑区

> **用户原话**："你可以在内存里生成四个完全独立的逻辑区，每个炉头都有完整且相同逻辑的元素，只是后面有一个状态机去判断当前热点炉头是哪个，热点炉头的信息才会显示到公共元素区，JSON只需要按单头去控制各种状态。"

**核心理念**：4个炉头共享同一套JSON模板，每个炉头有完全独立的状态。

**架构示意**：
```
┌──────────────────────────────────────────────┐
│  4个完全独立的炉头逻辑区（共享JSON模板）       │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────┐│
│  │ Head 1  │ │ Head 2  │ │ Head 3  │ │Head4││
│  │ 完整逻辑 │ │ 完整逻辑 │ │ 完整逻辑 │ │逻辑 ││
│  │  +状态机│ │  +状态机│ │  +状态机│ │+状态││
│  ─────────┘ └─────────┘ ─────────┘ └─────┘│
│       ↓           ↓           ↓        ↓      │
│   相同JSON模板  相同JSON模板 相同JSON模板 相同 │
──────────────────────────────────────────────┘
                      ↓
              ┌───────────────┐
              │  热点炉头管理器│
              │  判断hot_head │
              └───────┬───────┘
                      ↓
              ───────────────┐
              │  公共显示区    │
              │  显示hot_head │
              └───────────────┘
```

**说明**：
- JSON只需要定义单个炉头的逻辑（head_template）
- 引擎自动实例化4份（head_1, head_2, head_3, head_4）
- 状态机判断当前热点炉头（hot_head_id）
- 只将热点炉头的信息显示到公共显示区

#### 【用户分解5】原则5：上电序列独立进程

> **用户原话**："上电过程是一个公共过程，可以当成一个独立的进程，它把全显、版本号变成特定的属性进行固定输出即可，和后续的炉头工作界面无关。"

**核心理念**：上电过程是独立的初始化序列，与炉头工作逻辑完全无关。

**架构示意**：
```
上电序列进程（独立）
  ├─ 全显3秒
  ├─ 版本显示3秒
  └─ 进入待机态
          ↓
      炉头工作逻辑（主线）
```

**JSON结构**：
```json
{
  "power_on_sequence": {
    "description": "上电初始化序列（独立进程）",
    "steps": [
      {
        "step": 1,
        "actions": [
          {"display": "all_segments", "show": "8888"},
          {"delay": 3000}
        ]
      },
      {
        "step": 2,
        "actions": [
          {"display": "seg_1", "show": "V1.0"},
          {"delay": 3000}
        ]
      }
    ],
    "on_complete": "node_standby"
  }
}
```

**说明**：
- 上电序列是固定流程，与后续炉头逻辑无关
- 执行完成后进入待机态
- 6秒内按键无效（上电保护）

---

## 二、【用户设计思路总结】

> **说明**：本章汇总了用户在整个设计过程中的核心思路和分解，作为后续实施的标准参考。

### 2.1 用户原话记录

#### 2.1.1 关于元素状态标识化

**用户原话**：
> "先将各种元素的属性与方法规划好，比如LED的闪烁、常量、关至少是三种状态，后面还可以加。数码管显示功率、温度、时间、跑马、时间+功率交替这都是专门的状态，JSON只需标志显示的状态，程序自然就可以后面去调用相应的内部变量，根本不用说要显示什么功率值。"

**核心思想**：
- ✅ JSON只声明"是什么状态"（标识符）
- ❌ JSON不关心"具体值是多少"（真实值由程序维护）
- ✅ 新增显示模式只需注册新标识，无需修改JSON结构

#### 2.1.2 关于独立进程设计

**用户原话**：
> "定时、暂停、组合、童锁这些都是独立进程，可以以函数的形式供主功率调整调用，所以只需把调功率的主线搞清楚，就可以进行扩展，有新的功能原来的逻辑处理不了，可以新注册一个元素进行处理，也就是在CASE里加一条，很多复杂的功能在JSON里可能就是一个标识，比如跑马灯，只是数码管理的一个显示方式，和显示ASII并列，具体的程序让状态机去后面调。"

**核心思想**：
- ✅ 主线逻辑只关注调功率流程
- ✅ 其他功能作为独立进程，以函数形式调用
- ✅ 新增功能只需注册新元素/新进程
- ✅ 复杂功能在JSON里只是一个标识

#### 2.1.3 关于四炉头架构

**用户原话**：
> "你可以在内存里生成四个完全独立的逻辑区，每个炉头都有完整且相同逻辑的元素，只是后面有一个状态机去判断当前热点炉头是哪个，热点炉头的信息才会显示到公共元素区，JSON只需要按单头去控制各种状态。"

**核心思想**：
- ✅ 4个炉头共享同一套JSON模板
- ✅ 每个炉头有完全独立的状态
- ✅ 通过状态机判断热点炉头
- ✅ 只显示热点炉头信息到公共区

#### 2.1.4 关于上电序列

**用户原话**：
> "上电过程是一个公共过程，可以当成一个独立的进程，它把全显、版本号变成特定的属性进行固定输出即可，和后续的炉头工作界面无关。"

**核心思想**：
- ✅ 上电序列是独立进程
- ✅ 与炉头工作逻辑完全分离
- ✅ 执行完成后进入待机态

#### 2.1.5 关于简化思路

**用户原话**：
> "BOAST与定时并不冲突，定时是一个独立的进程，只要把指定的定时时间输入，到时间炉头切回待机状态即可，BOAST也是一个延时切换的功能，两者独立就简单了。"

**用户原话**：
> "数码管分割显示非常简单，就认为是4个2位数码管就好了，逻辑层把段码发到驱动层，驱动层自动会去显示到对应COM上，这个逻辑层不用关心了。"

**用户原话**：
> "组合功能可以先放一放。"

**核心思想**：
- ✅ Boost与定时作为独立进程，互不干扰
- ✅ 数码管简化为4个独立的2位数码管
- ✅ 组合功能暂缓实现

### 2.2 用户设计原则总结

| 编号 | 原则名称 | 核心思想 | 关键理解 |
|-----|---------|---------|----------|
| 1 | 声明式配置 | JSON只声明"是什么状态" | 不关心"如何实现" |
| 2 | 元素状态标识化 | 状态用标识符表示 | 具体值由程序维护 |
| 3 | 功能模块独立进程 | 定时、暂停等作为独立进程 | 以函数形式供主线调用 |
| 4 | 主线调功率流程 | 只关注核心业务 | 其他功能独立调用 |
| 5 | 四炉头独立逻辑区 | 共享JSON模板 | 每个炉头独立状态 |
| 6 | 上电序列独立进程 | 与炉头工作完全分离 | 固定流程，执行完进入待机 |
| 7 | 简化设计 | Boost与定时独立 | 数码管简化为4个2位 |

### 2.3 方法论要点

#### 2.3.1 JSON配置的本质

**核心理念**：JSON是**声明式配置**，不是**命令式代码**。

❌ **错误理解**：
```json
{
  "action": "set_power_level",
  "value": 5,  // JSON不应该包含具体值
  "blink": false,
  "decimal": false
}
```

✅ **正确理解**：
```json
{
  "action": "segment.set_mode",
  "args": ["power"]  // JSON只标识显示模式
}
```

#### 2.3.2 程序的职责分工

| 层级 | 职责 | 示例 |
|-----|------|------|
| JSON配置层 | 声明状态标识 | `segment.set_mode("power")` |
| 状态管理层 | 维护真实状态值 | `internal_state.power_level = 5` |
| 处理器层 | 实现显示逻辑 | `SegmentHandler.renderDigit(5)` |
| 驱动层 | 渲染硬件段码 | `driver.display(segments)` |

#### 2.3.3 扩展性设计

**原则**：新增功能只需注册新标识，无需修改JSON结构。

**示例**：
```javascript
// 1. 在注册表中添加新标识
element_registry.segment.display_modes.marquee = {
  description: "跑马灯效果",
  source: "internal.marquee_text",
  format: "scrolling_text"
};

// 2. 在处理器中实现新逻辑
class SegmentHandler {
  static startMarquee(segment) {
    const text = segment.internal_state.marquee_text;
    // 实现跑马灯逻辑
  }
}

// 3. JSON直接使用新标识
{
  "actions": [
    {"action": "segment.set_mode", "args": ["marquee"]},
    {"action": "internal.marquee_text", "set": "Hello World"}
  ]
}
```

### 2.4 实施优先级

根据用户的反馈，实施优先级如下：

| 优先级 | 功能模块 | 状态 | 说明 |
|-------|---------|------|------|
| P0 | 主线调功率流程 | 待实施 | 核心业务，必须首先实现 |
| P0 | 元素状态标识化 | 待实施 | 基础架构，影响所有功能 |
| P1 | 上电序列 | 待实施 | 独立进程，相对简单 |
| P1 | 定时功能 | 待实施 | 独立进程，常用功能 |
| P1 | Boost功能 | 待实施 | 独立进程，常用功能 |
| P1 | 暂停功能 | 待实施 | 独立进程，常用功能 |
| P2 | 童锁功能 | 待实施 | 独立进程，安全功能 |
| P3 | 组合功能 | 暂缓 | 用户明确表示可以先放一放 |
| P3 | 跑马灯显示 | 后续 | 高级显示模式 |

---

## 三、系统架构

### 3.1 整体架构

```
┌──────────────────────────────────────────┐
│          UI层（demo.js）                  │
│   调用 LogicLayerAdapter 统一接口         │
└────────────┬─────────────────────────────┘
             │
   ┌─────────┴──────────────────────────┐
   │         JsonLogicAdapter            │
   │   （新建，实现相同接口）             │
   │   - pressKey(keyCode, event)        │
   │   - runCycle()                      │
   │   - getState()                      │
   │   - getDisplay()                    │
   ─────────┬──────────────────────────┘
             │
   ┌─────────┴──────────────────────────┐
   │      JSON配置 + JS执行引擎          │
   │                                    │
   │  1. 加载JSON配置文件                │
   │  2. 解析节点/事件/函数              │
   │  3. 创建4个炉头实例                 │
   │  4. 执行上电序列                    │
   │  5. 进入主流程状态机                │
   │  6. 处理按键事件                    │
   │  7. 更新内部状态                    │
   │  8. 返回显示数据给UI层              │
   └────────────────────────────────────┘
```

### 3.2 JSON文件结构

```json
{
  "meta": {
    "name": "四头电磁炉控制器",
    "version": "1.0",
    "head_count": 4
  },
  
  // ========== 1. 上电序列（独立进程） ==========
  "power_on_sequence": { ... },
  
  // ========== 2. 元素注册表（可选，定义元素类型） ==========
  "element_registry": { ... },
  
  // ========== 3. 独立功能进程库 ==========
  "independent_processes": {
    "timer_process": { ... },
    "pause_process": { ... },
    "boost_process": { ... },
    "child_lock_process": { ... },
    "zone_process": { ... }
  },
  
  // ========== 4. 单炉头逻辑模板（核心） ==========
  "head_template": {
    "state": { ... },
    "functions": { ... },
    "main_flow": { ... }
  },
  
  // ========== 5. 全局状态机 ==========
  "global_state_machine": {
    "state": { ... },
    "functions": { ... },
    "main_flow": { ... }
  }
}
```

### 3.3 执行流程

```
1. 加载JSON文件
2. 创建4个炉头实例（从head_template克隆）
3. 执行上电序列（power_on_sequence）
4. 进入待机态（node_standby）
5. 等待按键事件
6. 匹配事件 → 调用函数 → 更新状态 → 跳转节点
7. 循环执行runCycle()更新显示
```

---

## 四、核心设计详解

### 4.0 【用户分解】主线调功率流程（核心业务）

> **用户原话**："只需把调功率的主线搞清楚，就可以进行扩展"

**核心理念**：主线逻辑只关注调功率流程，其他功能作为独立进程调用。

#### 3.0.1 主线流程图

```
上电序列（独立进程）
  ↓
待机态（node_standby）
  ├─ 按键：选炉头键 → 进入选中态
  │
选中态（node_selected）
  ├─ 数码管：闪烁显示 "--"
  ├─ LED：炉头选择灯亮
  ├─ 事件1：按数字键（0-9）→ 设置档位 → 工作态
  ├─ 事件2：长按9键 → 进入Boost → Boost态
  ├─ 事件3：按定时键 → 启动定时 → 工作态
  └─ 事件4：5秒超时 → 返回待机态
  │
工作态（node_working）
  ├─ 数码管：显示当前档位
  ├─ LED：功率指示灯亮
  ├─ 事件1：按暂停键 → 暂停态
  ├─ 事件2：定时超时 → 停止加热 → 待机态
  ├─ 事件3：Boost超时 → 回到原档位 → 工作态
  └─ 事件4：按选炉头键 → 切换炉头 → 选中态
  │
Boost态（node_boost）
  ├─ 数码管：显示 "P"
  ├─ 事件1：按数字键 → 退出Boost → 工作态
  └─ 事件2：5分钟超时 → 回到原档位 → 工作态
  │
暂停态（node_paused）
  ├─ 数码管：显示 "PA"
  └─ 事件1：按暂停键 → 恢复工作 → 工作态
```

#### 3.0.2 主线状态机定义

```json
{
  "head_template": {
    "main_flow": {
      "entry": "node_idle",
      "nodes": {
        "node_idle": {
          "description": "空闲态（未选中）",
          "events": [
            {
              "trigger": "key_head_select",
              "condition": "short_press",
              "call": "functions.select",
              "next": "node_selected"
            }
          ]
        },
        
        "node_selected": {
          "description": "选中闪烁态（等待设置档位）",
          "enter_actions": [
            {"action": "segment.set_mode", "args": ["dash"]},
            {"action": "segment.set_blink", "args": [true]},
            {"action": "led.head_select.on"}
          ],
          "events": [
            {
              "trigger": ["key_0", "key_1", ..., "key_9"],
              "condition": "short_press",
              "call": "functions.set_power",
              "args": ["trigger_key_level"],
              "next": "node_working"
            },
            {
              "trigger": "key_9",
              "condition": "long_press",
              "call": "boost_process.enter",
              "next": "node_boost"
            },
            {
              "trigger": "key_timer",
              "condition": "short_press",
              "call": "timer_process.start",
              "args": [15],
              "next": "node_working"
            },
            {
              "trigger": null,
              "condition": "timeout(5000) && power_level == 0",
              "call": "functions.deselect",
              "next": "node_idle"
            },
            {
              "trigger": null,
              "condition": "timeout(5000) && power_level > 0",
              "call": "functions.set_power",
              "args": ["state.power_level"],
              "next": "node_working"
            }
          ]
        },
        
        "node_working": {
          "description": "工作态（正常加热）",
          "enter_actions": [
            {"action": "segment.set_mode", "args": ["power"]},
            {"action": "segment.set_blink", "args": [false]},
            {"action": "led.power.on"}
          ],
          "events": [
            {
              "trigger": "key_pause",
              "condition": "short_press",
              "call": "pause_process.toggle",
              "next": "node_paused"
            },
            {
              "trigger": null,
              "condition": "timer.timeout",
              "call": "timer_process.on_timeout",
              "next": "node_idle"
            },
            {
              "trigger": null,
              "condition": "boost_timer.timeout",
              "call": "boost_process.on_timeout",
              "next": "node_working"
            }
          ]
        },
        
        "node_boost": {
          "description": "Boost模式（最大功率）",
          "enter_actions": [
            {"action": "segment.set_mode", "args": ["ascii"]},
            {"action": "segment.show_ascii", "args": ["P"]}
          ],
          "events": [
            {
              "trigger": ["key_0", "key_1", ..., "key_9"],
              "condition": "short_press",
              "call": "functions.set_power",
              "args": ["trigger_key_level"],
              "next": "node_working"
            },
            {
              "trigger": null,
              "condition": "boost_timer.timeout",
              "call": "boost_process.on_timeout",
              "next": "node_working"
            }
          ]
        },
        
        "node_paused": {
          "description": "暂停态",
          "enter_actions": [
            {"action": "segment.set_mode", "args": ["ascii"]},
            {"action": "segment.show_ascii", "args": ["PA"]}
          ],
          "events": [
            {
              "trigger": "key_pause",
              "condition": "short_press",
              "call": "pause_process.toggle",
              "next": "node_working"
            }
          ]
        }
      }
    }
  }
}
```

#### 3.0.3 主线函数定义

```json
{
  "head_template": {
    "functions": {
      "select": {
        "description": "选中炉头",
        "actions": [
          {"action": "state.selected", "set": true},
          {"action": "led.head_select.on"},
          {"action": "segment.set_blink", "args": [true]}
        ]
      },
      
      "deselect": {
        "description": "取消选中",
        "actions": [
          {"action": "state.selected", "set": false},
          {"action": "led.head_select.off"},
          {"action": "segment.set_blink", "args": [false]}
        ]
      },
      
      "set_power": {
        "description": "设置功率档位",
        "params": ["level"],
        "actions": [
          {"action": "state.power_level", "set": "{level}"},
          {"action": "segment.set_mode", "args": ["power"]},
          {"action": "segment.set_blink", "args": [false]},
          {"action": "state.working", "set": true}
        ]
      },
      
      "stop": {
        "description": "停止加热",
        "actions": [
          {"action": "state.power_level", "set": 0},
          {"action": "state.working", "set": false},
          {"action": "segment.set_mode", "args": ["dash"]},
          {"action": "led.power.off"}
        ]
      }
    }
  }
}
```

#### 3.0.4 主线与独立进程的关系

```
主线调功率流程
  ├─ 核心关注：档位设置、工作状态切换
  ├─ 调用独立进程1：timer_process.start() - 启动定时
  ├─ 调用独立进程2：boost_process.enter() - 进入Boost
  ├─ 调用独立进程3：pause_process.toggle() - 暂停/恢复
  └─ 调用独立进程4：child_lock_process.toggle() - 童锁开关

独立进程特点：
  ├─ 以函数形式供主线调用
  ├─ 内部维护自己的状态（如timer_value、boost_remaining）
  ├─ 触发自己的事件（如timer.timeout、boost.timeout）
  └─ 不影响主线逻辑结构
```

---

### 4.1 元素状态标识化设计

#### 4.1.1 LED元素

**状态标识**：
```json
{
  "led": {
    "type": "led_indicator",
    "states": {
      "off": { "description": "关闭" },
      "on": { "description": "常亮" },
      "blink": { "description": "闪烁", "params": ["rate"] },
      "breathe": { "description": "呼吸灯" },
      "custom": { "description": "自定义模式" }
    },
    "methods": {
      "set_state": { "params": ["state_name"] },
      "on": {},
      "off": {},
      "blink": { "params": ["rate"] }
    }
  }
}
```

**JSON使用示例**：
```json
{
  "actions": [
    {"action": "led.power.set_state", "args": ["blink"]},
    {"action": "led.head_1.on"},
    {"action": "led.timer.off"}
  ]
}
```

**程序内部实现**：
```javascript
class LedHandler {
  static setState(led, stateName) {
    led.properties.state = stateName;
    
    switch(stateName) {
      case 'off':
        led.hardware_pin = 0;
        break;
      case 'on':
        led.hardware_pin = 1;
        break;
      case 'blink':
        this.startBlink(led);
        break;
      case 'breathe':
        this.startBreathe(led);
        break;
    }
  }
}
```

#### 4.1.2 数码管元素

**显示模式标识**：
```json
{
  "segment": {
    "type": "segment_display",
    "display_modes": {
      "power": {
        "description": "显示功率档位",
        "source": "internal.power_level",
        "format": "digit"
      },
      "temperature": {
        "description": "显示温度",
        "source": "internal.temperature",
        "format": "number"
      },
      "timer": {
        "description": "显示定时时间",
        "source": "internal.timer_value",
        "format": "number"
      },
      "marquee": {
        "description": "跑马灯效果",
        "source": "internal.marquee_text",
        "format": "scrolling_text"
      },
      "alternating": {
        "description": "交替显示（时间+功率）",
        "sources": ["internal.timer_value", "internal.power_level"],
        "format": "alternating",
        "interval": 5000
      },
      "ascii": {
        "description": "显示ASCII字符",
        "source": "internal.ascii_text",
        "format": "text"
      },
      "dash": {
        "description": "显示横杠（待机）",
        "source": "constant",
        "value": "--"
      },
      "blank": {
        "description": "空白"
      }
    },
    "methods": {
      "set_mode": { "params": ["mode_name"] },
      "show_power": {},
      "show_timer": {},
      "show_ascii": { "params": ["text"] },
      "set_decimal": { "params": ["flag"] },
      "set_blink": { "params": ["flag"] }
    }
  }
}
```

**JSON使用示例**：
```json
{
  "actions": [
    {"action": "segment.set_mode", "args": ["power"]},
    {"action": "segment.set_mode", "args": ["timer"]},
    {"action": "segment.set_mode", "args": ["alternating"]},
    {"action": "segment.show_ascii", "args": ["PA"]},
    {"action": "segment.set_blink", "args": [true]}
  ]
}
```

**程序内部实现**：
```javascript
class SegmentHandler {
  static setMode(segment, modeName) {
    segment.properties.display_mode = modeName;
    
    switch(modeName) {
      case 'power':
        segment.hardware_segments = this.renderDigit(segment.internal_state.power_level);
        break;
      case 'timer':
        segment.hardware_segments = this.renderNumber(segment.internal_state.timer_value);
        break;
      case 'alternating':
        this.startAlternating(segment);
        break;
      case 'ascii':
        segment.hardware_segments = this.renderAscii(segment.internal_state.ascii_text);
        break;
      case 'dash':
        segment.hardware_segments = this.renderDash();
        break;
    }
  }
}
```

**说明**：
- JSON只需要设置模式标识（"power"、"timer"等）
- 程序自动从内部状态获取真实值（power_level、timer_value等）
- 显示逻辑由处理器实现，JSON不关心
- 新增显示模式只需注册新标识，无需修改JSON结构

### 4.2 独立功能进程设计

#### 4.2.1 定时进程（timer_process）

**功能说明**：独立的倒计时定时器，供主线调用。

**JSON定义**：
```json
{
  "timer_process": {
    "description": "定时功能（独立进程）",
    "type": "countdown_timer",
    "properties": {
      "min_value": 1,
      "max_value": 99,
      "default_value": 15,
      "step": 1
    },
    "functions": {
      "start": {
        "params": ["minutes"],
        "actions": [
          {"timer": "set_value", "args": ["{minutes}"]},
          {"timer": "start"},
          {"display": "decimal", "on": true}
        ]
      },
      "pause": {"timer": "pause"},
      "resume": {"timer": "resume"},
      "stop": [
        {"timer": "stop"},
        {"timer": "reset"},
        {"display": "decimal", "off": true}
      ],
      "on_timeout": [
        {"head": "stop"},
        {"state": "mode", "set": "standby"}
      ]
    }
  }
}
```

**主线调用示例**：
```json
{
  "events": [
    {
      "trigger": "key_timer",
      "condition": "short_press",
      "call": "timer_process.start",
      "args": [15],
      "next": "node_working"
    }
  ]
}
```

#### 4.2.2 Boost进程（boost_process）

**功能说明**：独立的Boost定时器，5分钟超时后回到原档位。

**JSON定义**：
```json
{
  "boost_process": {
    "description": "Boost功能（独立进程）",
    "properties": {
      "max_time": 300000
    },
    "functions": {
      "enter": [
        {"state": "original_power", "save": "state.power_level"},
        {"state": "power_level", "set": "boost"},
        {"display": "segment", "show": "P"},
        {"boost_timer": "start", "args": [300000]}
      ],
      "exit": [
        {"state": "power_level", "restore": "state.original_power"},
        {"display": "segment", "show": "state.original_power"},
        {"boost_timer": "stop"}
      ],
      "on_timeout": [
        {"boost_process": "exit"}
      ]
    }
  }
}
```

#### 4.2.3 暂停进程（pause_process）

**功能说明**：全局暂停所有工作炉头。

**JSON定义**：
```json
{
  "pause_process": {
    "description": "暂停功能（独立进程）",
    "functions": {
      "toggle": [
        {
          "if": "state.paused",
          "then": [
            {"head": "resume"},
            {"timer": "resume"},
            {"boost_timer": "resume"},
            {"state": "paused", "set": false}
          ],
          "else": [
            {"head": "pause"},
            {"timer": "pause"},
            {"boost_timer": "pause"},
            {"display": "segment", "show": "PA"},
            {"state": "paused", "set": true}
          ]
        }
      ]
    }
  }
}
```

#### 4.2.4 童锁进程（child_lock_process）

**功能说明**：锁定所有按键（除开关键外）。

**JSON定义**：
```json
{
  "child_lock_process": {
    "description": "童锁功能（独立进程）",
    "functions": {
      "toggle": [
        {
          "if": "state.child_lock",
          "then": [
            {"display": "led_child_lock", "off": true},
            {"state": "child_lock", "set": false},
            {"state": "lock_all_keys", "set": false}
          ],
          "else": [
            {"display": "led_child_lock", "on": true},
            {"state": "child_lock", "set": true},
            {"state": "lock_all_keys", "set": true}
          ]
        }
      ]
    }
  }
}
```

### 4.3 四炉头独立逻辑区设计

#### 4.3.1 单炉头逻辑模板

**核心思想**：JSON只定义单个炉头的逻辑，引擎自动实例化4份。

**JSON结构**：
```json
{
  "head_template": {
    "description": "单个炉头的完整逻辑（4个炉头共享）",
    
    "state": {
      "power_level": 0,
      "original_power": 0,
      "selected": false,
      "working": false,
      "paused": false,
      "boost_active": false,
      "current_node": "node_idle"
    },
    
    "functions": {
      "select": [
        {"state": "selected", "set": true},
        {"display": "led_head_select", "on": true},
        {"display": "segment", "blink": true}
      ],
      
      "deselect": [
        {"state": "selected", "set": false},
        {"display": "led_head_select", "off": true},
        {"display": "segment", "blink": false}
      ],
      
      "set_power": [
        {"params": ["level"]},
        {"state": "power_level", "set": "{level}"},
        {"display": "segment", "show": "{level}"},
        {"display": "segment", "blink": false},
        {"state": "working", "set": true}
      ]
    },
    
    "main_flow": {
      "entry": "node_idle",
      "nodes": {
        "node_idle": {
          "description": "空闲态",
          "events": []
        },
        
        "node_selected": {
          "description": "选中闪烁态",
          "events": [
            {
              "trigger": ["key_0", "key_1", ..., "key_9"],
              "condition": "short_press",
              "call": "functions.set_power",
              "args": ["trigger_key_level"],
              "next": "node_working"
            },
            {
              "trigger": "key_9",
              "condition": "long_press",
              "call": "boost_process.enter",
              "next": "node_boost"
            },
            {
              "trigger": "key_timer",
              "condition": "short_press",
              "call": "timer_process.start",
              "args": [15],
              "next": "node_working"
            },
            {
              "trigger": null,
              "condition": "timeout(5000) && power_level == 0",
              "call": "functions.deselect",
              "next": "node_idle"
            },
            {
              "trigger": null,
              "condition": "timeout(5000) && power_level > 0",
              "call": "functions.set_power",
              "args": ["state.power_level"],
              "next": "node_working"
            }
          ]
        },
        
        "node_working": {
          "description": "工作态",
          "events": [
            {
              "trigger": "key_pause",
              "condition": "short_press",
              "call": "pause_process.toggle",
              "next": "node_paused"
            },
            {
              "trigger": null,
              "condition": "timer.timeout",
              "call": "timer_process.on_timeout",
              "next": "node_idle"
            },
            {
              "trigger": null,
              "condition": "boost_timer.timeout",
              "call": "boost_process.on_timeout",
              "next": "node_working"
            }
          ]
        },
        
        "node_boost": {
          "description": "Boost模式",
          "events": [
            {
              "trigger": ["key_0", "key_1", ..., "key_9"],
              "condition": "short_press",
              "call": "functions.set_power",
              "args": ["trigger_key_level"],
              "next": "node_working"
            },
            {
              "trigger": null,
              "condition": "boost_timer.timeout",
              "call": "boost_process.on_timeout",
              "next": "node_working"
            }
          ]
        },
        
        "node_paused": {
          "description": "暂停态",
          "events": [
            {
              "trigger": "key_pause",
              "condition": "short_press",
              "call": "pause_process.toggle",
              "next": "node_working"
            }
          ]
        }
      }
    }
  }
}
```

#### 4.3.2 引擎实例化逻辑

**程序实现**：
```javascript
class HeadInstanceManager {
  constructor(headTemplate, headCount) {
    this.heads = new Map();
    this.template = headTemplate;
    
    // 创建4个独立的炉头实例
    for (let i = 1; i <= headCount; i++) {
      const headId = `head_${i}`;
      this.heads.set(headId, this.createHeadInstance(headId));
    }
  }
  
  createHeadInstance(headId) {
    // 深拷贝模板
    const instance = JSON.parse(JSON.stringify(this.template));
    
    // 初始化状态
    instance.state = {
      power_level: 0,
      original_power: 0,
      selected: false,
      working: false,
      paused: false,
      boost_active: false,
      current_node: "node_idle"
    };
    
    // 初始化显示
    instance.display = {
      segment: { mode: 'dash', value: '--', blink: false },
      leds: { head_select: false }
    };
    
    return instance;
  }
  
  getHead(headId) {
    return this.heads.get(headId);
  }
}
```

#### 4.3.3 热点炉头管理

**程序实现**：
```javascript
class HotHeadManager {
  constructor(headManager) {
    this.headManager = headManager;
    this.currentHotHead = null;
  }
  
  switchHotHead(headId) {
    // 取消之前的热点炉头
    if (this.currentHotHead) {
      const oldHead = this.headManager.getHead(this.currentHotHead);
      oldHead.state.selected = false;
    }
    
    // 设置新的热点炉头
    this.currentHotHead = headId;
    const newHead = this.headManager.getHead(headId);
    newHead.state.selected = true;
    
    // 更新公共显示区
    this.updateCommonDisplay(headId);
  }
  
  updateCommonDisplay(headId) {
    const head = this.headManager.getHead(headId);
    
    // 复制热点炉头的显示数据到公共区
    this.commonDisplay.segment = { ...head.display.segment };
    this.commonDisplay.leds = { ...head.display.leds };
  }
  
  getCurrentHotHead() {
    return this.currentHotHead ? this.headManager.getHead(this.currentHotHead) : null;
  }
}
```

### 4.4 上电序列独立进程设计

**JSON定义**：
```json
{
  "power_on_sequence": {
    "description": "上电初始化序列（独立进程，与炉头逻辑无关）",
    "steps": [
      {
        "step": 1,
        "description": "全显3秒（生产测试）",
        "actions": [
          {"display": "all_segments", "show": "8888"},
          {"display": "all_leds", "on": true},
          {"delay": 3000}
        ]
      },
      {
        "step": 2,
        "description": "版本显示3秒",
        "actions": [
          {"display": "seg_1", "show": "V1.0"},
          {"display": "seg_2", "show": "P1.0"},
          {"display": "seg_3", "show": "00"},
          {"display": "seg_4", "show": "00"},
          {"delay": 3000}
        ]
      },
      {
        "step": 3,
        "description": "进入待机态",
        "actions": [
          {"display": "seg_1", "show": "--"},
          {"display": "seg_1", "blink": true},
          {"display": "led_power", "blink": true},
          {"state": "mode", "set": "standby"}
        ]
      }
    ],
    "on_complete": {
      "next": "node_standby",
      "message": "上电完成，进入待机模式"
    }
  }
}
```

**程序实现**：
```javascript
class PowerOnSequenceExecutor {
  constructor(jsonConfig, displayDriver) {
    this.config = jsonConfig;
    this.display = displayDriver;
  }
  
  async execute() {
    console.log('[PowerOnSequence] 开始执行上电序列...');
    
    for (const step of this.config.steps) {
      console.log(`[PowerOnSequence] 执行步骤 ${step.step}: ${step.description}`);
      
      for (const action of step.actions) {
        await this.executeAction(action);
      }
    }
    
    console.log('[PowerOnSequence] ✅ 上电序列完成');
    return this.config.on_complete.next;
  }
  
  async executeAction(action) {
    if (action.delay) {
      await this.delay(action.delay);
    } else {
      await this.display.invoke(action);
    }
  }
  
  delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
  }
}
```

---

## 五、接口兼容性设计

### 5.1 统一接口（LogicLayerAdapter）

**核心原则**：JSON引擎必须实现与JS/WASM完全相同的接口，UI层不感知底层实现。

**接口定义**：
```javascript
class JsonLogicAdapter extends LogicLayerAdapter {
  async init() {
    // 1. 加载JSON配置
    // 2. 创建4个炉头实例
    // 3. 执行上电序列
    // 4. 进入待机态
    this.initialized = true;
  }
  
  pressKey(keyCode, event) {
    // 1. 检查上电是否完成
    // 2. 检查童锁
    // 3. 查找当前节点的事件
    // 4. 匹配事件 → 调用函数 → 更新状态 → 跳转节点
  }
  
  runCycle() {
    // 1. 执行状态机循环
    // 2. 更新定时器
    // 3. 更新显示
  }
  
  getState() {
    // 返回当前状态（与JS/WASM相同格式）
    return {
      power_level: this.getHotHead()?.state.power_level || 0,
      currentNode: this.getHotHead()?.state.currentNode || 'node_idle'
    };
  }
  
  getDisplay() {
    // 返回显示数据（与JS/WASM相同格式）
    return {
      seg: this.getHotHead()?.display.segment || { mode: 'dash', value: '--' },
      led: this.getLedState()
    };
  }
}
```

### 5.2 UI层调用示例

**UI层代码（无需修改）**：
```javascript
// demo.js
function canvasMouseDownHandler(event) {
  const control = findControlByPosition(x, y);
  if (!control || control.type !== 'button') return;
  
  const keyCode = control.config.key_code;
  
  // 通过统一接口调用（不区分JS/WASM/JSON）
  if (AppState.logicLayer) {
    AppState.logicLayer.pressKey(keyCode, KeyEvent.KEY_EVENT_PRESS);
  }
  
  // 刷新显示
  renderCanvas();
}

function renderCanvas() {
  // 获取显示数据
  const display = AppState.logicLayer.getDisplay();
  
  // 更新数码管
  updateSegmentDisplay(display.seg);
  
  // 更新LED
  updateLedDisplay(display.led);
}
```

**说明**：
- UI层只调用 `pressKey()`, `getDisplay()` 等接口
- 不关心底层是JS、WASM还是JSON驱动
- 返回数据格式完全一致

---

## 六、实施计划

### 阶段1：核心引擎（1天）

**目标**：实现JSON逻辑适配器的基础框架。

**任务清单**：
- [ ] 创建 `json_logic_adapter.js` 文件
- [ ] 实现 `JsonLogicAdapter` 类（继承LogicLayerAdapter）
- [ ] 实现 `pressKey()` 方法（事件匹配 + 函数调用）
- [ ] 实现 `runCycle()` 方法（状态机循环）
- [ ] 实现 `getState()` 和 `getDisplay()` 方法
- [ ] 实现 `HeadInstanceManager`（炉头实例管理器）
- [ ] 实现 `HotHeadManager`（热点炉头管理器）

**验收标准**：
- [ ] 能加载JSON配置文件
- [ ] 能创建4个炉头实例
- [ ] 能响应按键事件
- [ ] 能返回显示数据

### 阶段2：单炉头主线逻辑（2天）

**目标**：实现完整的调功率流程。

**任务清单**：
- [ ] 编写 `head_template` JSON配置（单炉头完整逻辑）
- [ ] 实现核心函数：`select`, `deselect`, `set_power`, `stop`
- [ ] 实现主流程状态机：
  - [ ] `node_idle`（空闲态）
  - [ ] `node_selected`（选中闪烁态）
  - [ ] `node_working`（工作态）
  - [ ] `node_boost`（Boost模式）
  - [ ] `node_paused`（暂停态）
- [ ] 实现事件匹配逻辑（trigger + condition）
- [ ] 实现节点跳转逻辑（next）

**验收标准**：
- [ ] 能完成待机→选炉头→调功率→工作的完整流程
- [ ] 能正确处理档位设置（0~9档）
- [ ] 能正确处理5秒超时逻辑
- [ ] 显示数据格式与JS/WASM一致

### 阶段3：独立功能进程（2天）

**目标**：实现定时、Boost、暂停、童锁等独立功能。

**任务清单**：
- [ ] 实现 `power_on_sequence`（上电序列）
- [ ] 实现 `timer_process`（定时功能）
  - [ ] start/pause/resume/stop
  - [ ] 倒计时引擎
  - [ ] 超时事件处理
- [ ] 实现 `boost_process`（Boost功能）
  - [ ] enter/exit
  - [ ] 5分钟超时处理
  - [ ] 回到原档位逻辑
- [ ] 实现 `pause_process`（暂停功能）
  - [ ] toggle（暂停/恢复切换）
- [ ] 实现 `child_lock_process`（童锁功能）
  - [ ] toggle（锁定/解锁切换）
  - [ ] 按键过滤逻辑

**验收标准**：
- [ ] 上电序列能正确执行（全显3秒→版本3秒→待机）
- [ ] 定时功能能正确倒计时和超时
- [ ] Boost功能能正确进入和退出
- [ ] 暂停功能能正确暂停/恢复所有炉头
- [ ] 童锁功能能正确锁定/解锁按键

### 阶段4：集成测试（1天）

**目标**：集成到demo.js，对比JS/WASM行为一致性。

**任务清单**：
- [ ] 在demo.js中添加JSON逻辑层切换选项
- [ ] 实现JSON配置加载逻辑
- [ ] 创建测试页面 `test_json_logic.html`
- [ ] 对比测试：
  - [ ] 上电流程对比
  - [ ] 按键触发对比
  - [ ] 状态转移对比
  - [ ] 显示更新对比
- [ ] 修复不一致问题
- [ ] 性能优化

**验收标准**：
- [ ] JSON模式与JS/WASM模式行为完全一致
- [ ] 所有按键功能正常工作
- [ ] 所有显示更新正确
- [ ] 无内存泄漏
- [ ] 性能满足要求（60fps）

### 阶段5：扩展功能（后续）

**目标**：实现组合功能、跑马灯等高级特性。

**任务清单**：
- [ ] 实现 `zone_process`（组合功能）
  - [ ] 自动补全逻辑
  - [ ] 炉头联动同步
- [ ] 实现跑马灯显示模式
- [ ] 实现呼吸灯效果
- [ ] 实现自定义显示模式
- [ ] 编写JSON配置编辑器

---

## 七、关键技术点

### 7.1 JSON解析与执行

**问题**：如何从JSON配置驱动状态机？

**解决方案**：
```javascript
class JsonStateMachine {
  constructor(nodeConfig) {
    this.currentNode = nodeConfig.entry;
    this.nodes = nodeConfig.nodes;
  }
  
  pressKey(keyCode, event) {
    const node = this.nodes[this.currentNode];
    const matchedEvent = this.findMatchingEvent(node, keyCode, event);
    
    if (!matchedEvent) return;
    
    // 执行动作
    this.executeAction(matchedEvent.call, matchedEvent.args);
    
    // 跳转节点
    if (matchedEvent.next) {
      this.currentNode = matchedEvent.next;
    }
  }
  
  findMatchingEvent(node, keyCode, event) {
    for (const evt of node.events) {
      if (this.matchTrigger(evt.trigger, keyCode) &&
          this.matchCondition(evt.condition, event)) {
        return evt;
      }
    }
    return null;
  }
  
  executeAction(actionRef, args) {
    // 解析动作引用（如 "timer_process.start"）
    // 调用对应的函数
  }
}
```

### 7.2 内部状态管理

**问题**：如何维护程序内部状态（JSON不关心）？

**解决方案**：
```javascript
class HeadState {
  constructor() {
    // 内部状态（JSON不关心）
    this.power_level = 0;
    this.timer_value = 0;
    this.temperature = 0;
    this.marquee_text = "";
    this.ascii_text = "";
    
    // 显示状态（JSON关心）
    this.display_mode = 'dash';
    this.display_blink = false;
  }
  
  setPowerLevel(level) {
    this.power_level = level;
    // 自动更新显示
    if (this.display_mode === 'power') {
      this.updateDisplay();
    }
  }
  
  updateDisplay() {
    // 根据display_mode从内部状态获取值
    switch(this.display_mode) {
      case 'power':
        this.current_value = this.power_level;
        break;
      case 'timer':
        this.current_value = this.timer_value;
        break;
      // ...
    }
  }
}
```

### 7.3 定时器引擎

**问题**：如何实现独立的倒计时定时器？

**解决方案**：
```javascript
class CountdownTimer {
  constructor() {
    this.value = 0;
    this.running = false;
    this.paused = false;
    this.intervalId = null;
    this.timeoutCallback = null;
  }
  
  start(seconds, onTimeout) {
    this.value = seconds;
    this.running = true;
    this.timeoutCallback = onTimeout;
    
    this.intervalId = setInterval(() => {
      if (this.value > 0) {
        this.value--;
      } else {
        this.stop();
        if (this.timeoutCallback) {
          this.timeoutCallback();
        }
      }
    }, 1000);
  }
  
  pause() {
    this.paused = true;
    clearInterval(this.intervalId);
  }
  
  resume() {
    this.paused = false;
    this.start(this.value, this.timeoutCallback);
  }
  
  stop() {
    this.running = false;
    clearInterval(this.intervalId);
  }
  
  reset() {
    this.value = 0;
    this.stop();
  }
}
```

### 7.4 显示更新机制

**问题**：如何确保60fps流畅显示？

**解决方案**：
```javascript
class DisplayUpdater {
  constructor(logicLayer) {
    this.logicLayer = logicLayer;
    this.lastFrameTime = 0;
    this.frameInterval = 1000 / 60; // 60fps
  }
  
  start() {
    this.updateLoop();
  }
  
  updateLoop() {
    requestAnimationFrame((timestamp) => {
      const elapsed = timestamp - this.lastFrameTime;
      
      if (elapsed >= this.frameInterval) {
        this.lastFrameTime = timestamp - (elapsed % this.frameInterval);
        
        // 执行逻辑循环
        this.logicLayer.runCycle();
        
        // 获取显示数据
        const display = this.logicLayer.getDisplay();
        
        // 更新Canvas
        this.renderCanvas(display);
      }
      
      this.updateLoop();
    });
  }
  
  renderCanvas(display) {
    // 更新数码管
    updateSegmentDisplay(display.seg);
    
    // 更新LED
    updateLedDisplay(display.led);
  }
}
```

---

## 八、常见问题与解决方案

### 8.1 如何保证JSON与JS/WASM行为一致？

**问题**：JSON驱动的逻辑可能与现有JS/WASM逻辑有细微差异。

**解决方案**：
1. 创建对比测试工具
2. 相同的按键输入序列
3. 对比每个状态节点的显示输出
4. 记录不一致之处
5. 逐步修复

**测试脚本示例**：
```javascript
async function compareLogicLayers() {
  const jsAdapter = new JSEmcLogicAdapter();
  const jsonAdapter = new JsonLogicAdapter(jsonConfig);
  
  await jsAdapter.init();
  await jsonAdapter.init();
  
  // 测试按键序列
  const testSequence = [
    { key: 13, event: 'long_press', delay: 1500 },
    { key: 1, event: 'short_press', delay: 100 },
    { key: 5, event: 'short_press', delay: 100 }
  ];
  
  for (const test of testSequence) {
    jsAdapter.pressKey(test.key, test.event);
    jsonAdapter.pressKey(test.key, test.event);
    
    await delay(test.delay);
    
    const jsDisplay = jsAdapter.getDisplay();
    const jsonDisplay = jsonAdapter.getDisplay();
    
    if (!deepEqual(jsDisplay, jsonDisplay)) {
      console.error('不一致！', { jsDisplay, jsonDisplay });
    }
  }
}
```

### 8.2 如何处理复杂的条件表达式？

**问题**：JSON中的条件表达式（如 `timeout(5000) && power_level > 0`）如何解析？

**解决方案**：
```javascript
class ConditionEvaluator {
  evaluate(condition, context) {
    // 替换变量
    let expr = condition
      .replace(/state\.(\w+)/g, (_, key) => context.state[key])
      .replace(/timeout\((\d+)\)/g, (_, ms) => this.checkTimeout(ms));
    
    // 执行表达式
    try {
      return eval(expr);
    } catch (e) {
      console.error('条件表达式解析错误:', e);
      return false;
    }
  }
  
  checkTimeout(ms) {
    // 检查是否超时
    return Date.now() - this.nodeEnterTime >= ms;
  }
}
```

### 8.3 如何扩展新的显示模式？

**问题**：如何添加跑马灯、呼吸灯等新显示模式？

**解决方案**：
1. 在 `element_registry` 中注册新标识
2. 在处理器中添加新逻辑
3. JSON直接使用新标识

**示例**：
```json
{
  "element_registry": {
    "segment": {
      "display_modes": {
        "marquee": {
          "description": "跑马灯效果",
          "source": "internal.marquee_text",
          "format": "scrolling_text"
        }
      }
    }
  }
}
```

```javascript
class SegmentHandler {
  static setMode(segment, modeName) {
    switch(modeName) {
      case 'marquee':
        this.startMarquee(segment);
        break;
      // ...
    }
  }
  
  static startMarquee(segment) {
    const text = segment.internal_state.marquee_text;
    // 实现跑马灯逻辑
  }
}
```

```json
{
  "actions": [
    {"action": "segment.set_mode", "args": ["marquee"]},
    {"action": "internal.marquee_text", "set": "Hello World"}
  ]
}
```

---

## 九、文档维护

### 9.1 版本历史

| 版本 | 日期 | 修改内容 | 作者 |
|-----|------|---------|------|
| V1.0 | - | 初始版本 | - |
| V2.0 | - | 增加主流程+触发流设计 | - |
| V3.0 | 2026-02-03 | 最终可落地版，明确元素状态标识化、独立进程、四炉头架构 | EMC团队 |

### 9.2 设计决策记录

#### 决策1：元素状态标识化

**决策内容**：所有UI元素的状态用标识符表示，具体值由程序内部维护。

**理由**：
- JSON只关心"显示什么模式"，不关心"具体值是多少"
- 程序内部自动维护真实状态值
- 扩展新显示模式只需注册新标识，无需修改JSON结构

**影响**：
- JSON配置变得极简
- 处理器实现变得复杂（需要管理内部状态）
- 总体可维护性提升

#### 决策2：功能模块独立进程

**决策内容**：定时、暂停、Boost、童锁等功能作为独立进程，以函数形式供主线调用。

**理由**：
- 主线逻辑只关注调功率流程，保持清晰
- 独立功能需要时调用，不需要时不干扰
- 新增功能只需注册新进程，不影响主线

**影响**：
- JSON结构模块化，易于理解
- 功能之间解耦，易于测试
- 总体架构灵活可扩展

#### 决策3：四炉头独立逻辑区

**决策内容**：4个炉头共享同一套JSON模板，每个炉头有完全独立的状态。

**理由**：
- JSON只需要定义单个炉头的逻辑，极简
- 引擎自动实例化4份，无需重复定义
- 每个炉头独立运行，互不干扰

**影响**：
- JSON文件大小大幅减少
- 逻辑复用性极高
- 扩展到6头、8头只需改配置，无需改JSON

#### 决策4：上电序列独立进程

**决策内容**：上电过程是独立的初始化序列，与炉头工作逻辑完全无关。

**理由**：
- 上电是固定流程，与后续逻辑无关
- 独立执行，完成后进入待机态
- 避免上电逻辑与炉头逻辑耦合

**影响**：
- 上电序列可单独测试
- 炉头逻辑更清晰
- 总体架构更合理

---

## 十、附录

### 10.1 JSON配置完整示例

（详见单独文件：`emc_controller_config.json`）

### 10.2 元素状态标识对照表

| 元素类型 | 状态标识 | 说明 | 内部状态字段 |
|---------|---------|------|-------------|
| LED | off | 关闭 | hardware_pin = 0 |
| LED | on | 常亮 | hardware_pin = 1 |
| LED | blink | 闪烁 | blink_timer 运行 |
| LED | breathe | 呼吸灯 | breathe_phase 更新 |
| 数码管 | power | 显示功率档位 | internal_state.power_level |
| 数码管 | temperature | 显示温度 | internal_state.temperature |
| 数码管 | timer | 显示定时时间 | internal_state.timer_value |
| 数码管 | marquee | 跑马灯效果 | internal_state.marquee_text |
| 数码管 | alternating | 交替显示 | internal_state.alternating_modes |
| 数码管 | ascii | ASCII字符 | internal_state.ascii_text |
| 数码管 | dash | 横杠（待机） | 常量 "--" |
| 数码管 | blank | 空白 | 无显示 |
| 定时器 | idle | 空闲 | value = 0, running = false |
| 定时器 | running | 运行中 | value 递减 |
| 定时器 | paused | 暂停 | value 保持 |
| 定时器 | timeout | 超时 | 触发超时事件 |

### 10.3 独立进程列表

| 进程名称 | 功能 | 主要方法 | 状态 |
|---------|------|---------|------|
| power_on_sequence | 上电序列 | execute() | 已完成 |
| timer_process | 定时功能 | start/pause/resume/stop | 待实现 |
| boost_process | Boost功能 | enter/exit/on_timeout | 待实现 |
| pause_process | 暂停功能 | toggle | 待实现 |
| child_lock_process | 童锁功能 | toggle | 待实现 |
| zone_process | 组合功能 | enter/confirm/cancel | 暂缓 |

### 10.4 主流程状态机

| 节点名称 | 说明 | 主要事件 | 下一节点 |
|---------|------|---------|---------|
| node_idle | 空闲态 | 无 | - |
| node_selected | 选中闪烁态 | 设置档位、进入Boost、设置定时 | node_working / node_boost |
| node_working | 工作态 | 暂停、定时超时、Boost超时 | node_paused / node_idle / node_working |
| node_boost | Boost模式 | 设置档位、Boost超时、设置定时 | node_working / node_boost_with_timer |
| node_boost_with_timer | Boost+定时 | 定时超时、Boost超时 | node_idle / node_working |
| node_paused | 暂停态 | 恢复 | node_working |
| node_timer_setting | 定时设置态 | 加减时间、确认、取消 | node_working / node_selected |

---

**文档结束**
