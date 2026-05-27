markdown

# Web仿真框架 - JSON规则驱动实施方案 V2.0
## 一、总体概述
### 1.1 项目目标
将电磁炉控制面板的全部逻辑，用JSON配置文件来描述。框架程序加载JSON后自动解析并执行，实现完整的按键显示逻辑仿真。
### 1.2 核心思路
**整个程序的架构完全用JSON来描述。** 原来C代码里有什么逻辑，JSON里就有对应的描述性语言。程序不再是一堆代码文件，而是一份可读、可改、可检索的JSON配置文件。改逻辑就是改JSON，不需要重新编译，不需要懂C语言。
### 1.3 系统架构

规则JSON文件  
│  
├─ 配置块：描述系统有什么（控件、档位、超时参数等）  
├─ 函数库块：声明所有可用函数（内置 + 外部）  
└─ 流程块：主流程 + 触发流，每步调用函数  
│  
↓  
框架解释器（JS引擎）  
│  
┌───────┼───────┐  
↓ ↓ ↓  
内置函数 外部JS DLL/WASM  
(框架自带) (用户写) (C代码)

text

### 1.4 三大核心设计
#### 1.4.1 主流程 + 触发流
整个程序分为**主流程**和**触发流**。
- **主流程**：最基础的调功率流程。上电→待机→开机→选炉头→调功率→工作。这是唯一的主线。
- **触发流**：定时、Boost、组合、暂停、童锁等辅助功能。它们都有一个共同的前置条件——必须先有炉头被选中。触发才存在，不触发就永远不会执行。每个触发流是一个独立的函数调用，执行完后回到主流程。
#### 1.4.2 函数调用机制
每条规则通过 `call` 字段指定要执行的函数名。框架根据 `source` 字段决定去哪里找这个函数：
| source | 说明 |
|--------|------|
| `builtin` | 框架内置函数，JSON流里的 |
| `external` | 外部实现，用户自己写的JS函数 |
如果JSON里写的逻辑描述不了，用户可以直接写JS函数实现，然后注册到函数库里。JSON只需要引用函数名，框架自动关联到对应的实现。
#### 1.4.3 节点 + 事件驱动
主流程由多个**节点（node）**组成。每个节点代表一个稳定的状态，节点内部列出该状态下所有可能的**事件（events）**。每个事件指定：触发条件（按键+事件类型）、执行函数（call）、执行后跳转到哪个节点（next）。
框架在每个节点等待用户操作，匹配到事件后执行对应函数，然后跳转到下一个节点。
### 1.5 JSON文件四大块
| 块名称 | 说明 |
|--------|------|
| `config` | 全局配置：炉头数、超时参数、档位定义、按键设置 |
| `controls` | 控件清单：按键、LED、数码管、蜂鸣器的属性和触发模式 |
| `function_library` | 函数库：声明所有可用函数及其来源 |
| `main_flow` | 主流程：节点列表 + 每个节点的事件列表 |
### 1.6 热点炉头机制
当前操作的炉头号存储在 `hot_head` 中，包含两个字段：
| 字段 | 说明 |
|------|------|
| `hot_head_id` | 当前有效炉头号 |
| `is_selecting` | 是否处于选择闪烁状态 |
默认规则：只有一个炉头时，开机后自动设为选中状态，跳过炉头选择步骤。
### 1.7 按键检测
所有按键的触发模式在 `controls` 块中声明：
| 模式 | 说明 |
|------|------|
| `press` | 按下触发，长按产生连按信号 |
| `release` | 松开触发，按住超过阈值松手时产生长按事件 |
长按阈值统一在 `config.key_settings.default_long_threshold_ms` 中配置，默认为1500ms。
---
## 二、完整示例
以下是一个四炉头电磁炉控制器的完整JSON配置文件。
```json
{
  "project_name": "四炉头电磁炉控制器",
  
  "config": {
    "head_count": 4,
    "single_head_auto_select": true,
    "long_press_ms": 1500,
    "timeouts": {
      "power_on_display_ms": 3000,
      "version_display_ms": 3000,
      "head_select_s": 5,
      "timer_confirm_s": 5,
      "zone_confirm_s": 5
    },
    "timer_range": {"min": 1, "max": 99, "default": 15},
    "boost_max_ms": 300000,
    "key_settings": {
      "default_long_threshold_ms": 1500,
      "long_press_detect_mode": "release"
    }
  },
  
  "controls": {
    "buttons": [
      {"id": "KEY_POWER", "name": "开关", "trigger_mode": "release"},
      {"id": "KEY_CHILD_LOCK", "name": "童锁", "trigger_mode": "release"},
      {"id": "KEY_TIMER", "name": "定时", "trigger_mode": "press"},
      {"id": "KEY_PAUSE", "name": "暂停", "trigger_mode": "press"},
      {"id": "KEY_ZONE", "name": "无区", "trigger_mode": "press"},
      {"id": "KEY_1", "name": "炉头1", "trigger_mode": "press"},
      {"id": "KEY_2", "name": "炉头2", "trigger_mode": "press"},
      {"id": "KEY_3", "name": "炉头3", "trigger_mode": "press"},
      {"id": "KEY_4", "name": "炉头4", "trigger_mode": "press"},
      {"id": "KEY_PLUS", "name": "+", "trigger_mode": "press"},
      {"id": "KEY_MINUS", "name": "-", "trigger_mode": "press"},
      {"id": "KEY_0", "name": "0档", "trigger_mode": "press"},
      {"id": "KEY_1_PWR", "name": "1档", "trigger_mode": "press"},
      {"id": "KEY_2_PWR", "name": "2档", "trigger_mode": "press"},
      {"id": "KEY_3_PWR", "name": "3档", "trigger_mode": "press"},
      {"id": "KEY_4_PWR", "name": "4档", "trigger_mode": "press"},
      {"id": "KEY_5_PWR", "name": "5档", "trigger_mode": "press"},
      {"id": "KEY_6_PWR", "name": "6档", "trigger_mode": "press"},
      {"id": "KEY_7_PWR", "name": "7档", "trigger_mode": "press"},
      {"id": "KEY_8_PWR", "name": "8档", "trigger_mode": "press"},
      {"id": "KEY_9_PWR", "name": "9档", "trigger_mode": "release"}
    ],
    "leds": [
      {"id": "LED_POWER"}, {"id": "LED_CHILD_LOCK"}, {"id": "LED_TIMER"},
      {"id": "LED_PAUSE"}, {"id": "LED_ZONE"},
      {"id": "LED_1"}, {"id": "LED_2"}, {"id": "LED_3"}, {"id": "LED_4"},
      {"id": "LED_LV0"}, {"id": "LED_LV1"}, {"id": "LED_LV2"}, {"id": "LED_LV3"},
      {"id": "LED_LV4"}, {"id": "LED_LV5"}, {"id": "LED_LV6"}, {"id": "LED_LV7"},
      {"id": "LED_LV8"}, {"id": "LED_LV9"}
    ],
    "segments": [
      {"id": "SEG_LEFT", "digits": 4},
      {"id": "SEG_RIGHT", "digits": 4}
    ],
    "buzzer": {"id": "BUZZER"}
  },
  
  "function_library": {
    "power_on_sequence":    {"source": "builtin", "desc": "上电序列"},
    "standby_display":      {"source": "builtin", "desc": "待机态显示"},
    "enter_head_select":    {"source": "builtin", "desc": "进入炉头选择态"},
    "head_select":          {"source": "builtin", "desc": "炉头选择处理"},
    "hot_head_set":         {"source": "builtin", "desc": "设置热点炉头"},
    "power_level_set":      {"source": "builtin", "desc": "设置功率档位"},
    "power_confirm":        {"source": "builtin", "desc": "确认功率设置"},
    "enter_work":           {"source": "builtin", "desc": "进入工作态"},
    "work_display":         {"source": "builtin", "desc": "工作态显示"},
    "shutdown":             {"source": "builtin", "desc": "长按开关关机"},
    "timer_flow":           {"source": "builtin", "desc": "定时完整流程"},
    "boost_flow":           {"source": "builtin", "desc": "Boost完整流程"},
    "boost_exit":           {"source": "builtin", "desc": "退出Boost"},
    "zone_flow":            {"source": "builtin", "desc": "无区组合完整流程"},
    "pause_flow":           {"source": "builtin", "desc": "暂停/恢复流程"},
    "child_lock_flow":      {"source": "builtin", "desc": "童锁/解锁流程"},
    "invalid_key":          {"source": "builtin", "desc": "无效按键响应"},
    "custom_special":       {"source": "external", "desc": "用户自定义逻辑"}
  },
  
  "entry": "power_on_sequence",
  
  "main_flow": [
    {
      "step": 1,
      "call": "power_on_sequence",
      "desc": "上电→全显3秒→版本显示3秒→进入待机",
      "next": "node_standby"
    },
    {
      "step": 2,
      "node": "node_standby",
      "desc": "待机态：数码管闪烁'--'，电源灯闪烁",
      "events": [
        {"key": "KEY_POWER", "event": "long_press", "call": "enter_head_select", "next": "node_head_select", "desc": "长按开关→开机"},
        {"key": "KEY_CHILD_LOCK", "event": "long_press", "call": "child_lock_flow", "next": "node_standby", "desc": "长按童锁→童锁"},
        {"key": "any", "event": "invalid", "call": "invalid_key", "next": "node_standby", "desc": "其他按键无效"}
      ]
    },
    {
      "step": 3,
      "call": "enter_head_select",
      "desc": "进入炉头选择态：所有炉头0档，炉头灯全亮",
      "next": "node_head_select"
    },
    {
      "step": 4,
      "node": "node_head_select",
      "desc": "炉头选择态：等待用户选择炉头",
      "events": [
        {"key": ["KEY_1", "KEY_2", "KEY_3", "KEY_4"], "event": "short_press", "call": "head_select", "next": "node_flashing", "desc": "按炉头键→选中闪烁"},
        {"key": null, "event": "single_head_auto", "call": "hot_head_set", "next": "node_flashing", "desc": "单炉头自动选中"},
        {"key": "KEY_POWER", "event": "long_press", "call": "shutdown", "next": "node_standby", "desc": "长按开关→关机"},
        {"key": "KEY_CHILD_LOCK", "event": "long_press", "call": "child_lock_flow", "next": "node_head_select", "desc": "长按童锁→童锁"},
        {"key": "KEY_ZONE", "event": "short_press", "call": "zone_flow", "next": "node_head_select", "desc": "按无区键→组合设置"}
      ]
    },
    {
      "step": 5,
      "node": "node_flashing",
      "desc": "选中闪烁态：对应数码管闪烁，等待用户操作",
      "events": [
        {"key": ["KEY_0", "KEY_1_PWR", "KEY_2_PWR", "KEY_3_PWR", "KEY_4_PWR", "KEY_5_PWR", "KEY_6_PWR", "KEY_7_PWR", "KEY_8_PWR", "KEY_9_PWR"], "event": "short_press", "call": "power_level_set", "next": "node_flashing", "desc": "按档位键→设置功率"},
        {"key": "KEY_9_PWR", "event": "long_press", "call": "boost_flow", "next": "node_working", "desc": "长按9档→进入Boost"},
        {"key": "KEY_TIMER", "event": "short_press", "call": "timer_flow", "next": "node_working", "desc": "按定时键→定时设置"},
        {"key": "KEY_ZONE", "event": "short_press", "call": "zone_flow", "next": "node_flashing", "desc": "按无区键→组合设置"},
        {"key": "KEY_PAUSE", "event": "short_press", "call": "pause_flow", "next": "node_flashing", "desc": "按暂停键→暂停"},
        {"key": "KEY_POWER", "event": "long_press", "call": "shutdown", "next": "node_standby", "desc": "长按开关→关机"},
        {"key": "KEY_CHILD_LOCK", "event": "long_press", "call": "child_lock_flow", "next": "node_flashing", "desc": "长按童锁→童锁"},
        {"key": null, "event": "timeout_5s_power_0", "call": "shutdown", "next": "node_standby", "desc": "5秒无操作+档位0→回待机"},
        {"key": null, "event": "timeout_5s_power_gt_0", "call": "power_confirm", "next": "node_working", "desc": "5秒无操作+档位>0→进入工作"}
      ]
    },
    {
      "step": 6,
      "call": "power_confirm",
      "desc": "确认功率设置，进入工作态",
      "next": "node_working"
    },
    {
      "step": 7,
      "node": "node_working",
      "desc": "工作态：数码管显示档位，炉头正在工作",
      "events": [
        {"key": "KEY_POWER", "event": "long_press", "call": "shutdown", "next": "node_standby", "desc": "长按开关→关机"},
        {"key": "KEY_CHILD_LOCK", "event": "long_press", "call": "child_lock_flow", "next": "node_working", "desc": "长按童锁→童锁"},
        {"key": "KEY_PAUSE", "event": "short_press", "call": "pause_flow", "next": "node_working", "desc": "按暂停→暂停"},
        {"key": "KEY_ZONE", "event": "short_press", "call": "zone_flow", "next": "node_working", "desc": "按无区→组合"},
        {"key": null, "event": "timer_end", "call": "shutdown", "next": "node_standby", "desc": "定时结束→关机"},
        {"key": null, "event": "boost_timeout", "call": "boost_exit", "next": "node_working", "desc": "Boost超时→退出回原档位"}
      ]
    }
  ]
}

---

## 三、框架执行流程

text

1. 加载JSON文件
2. 解析config、controls、function_library
3. 注册所有函数（builtin→框架内置，external→用户代码）
4. 从entry开始执行主流程
5. 进入节点后，等待按键事件
6. 匹配事件→调用对应函数→更新显示→跳转到next节点
7. 循环

## 四、如何修改逻辑

### 场景一：改功率档位值

修改 `config.power_levels` 数组。

### 场景二：改长按时间

修改 `config.key_settings.default_long_threshold_ms`。

### 场景三：增加一个新的触发流

1. 在 `function_library` 中声明函数名
    
2. 在对应节点的 `events` 中增加一条事件
    
3. 如果逻辑简单，框架内置函数直接处理；如果复杂，用户写JS函数实现
    

### 场景四：改显示规则

修改对应节点的 `call` 函数实现，或修改函数库中对应函数的参数。

## 五、扩展接口

当JSON规则描述不了复杂逻辑时，用户在外部写JS函数，注册到函数库中，然后在事件中通过 `call` 字段引用。框架根据 `source: "external"` 自动去用户代码中查找并调用。