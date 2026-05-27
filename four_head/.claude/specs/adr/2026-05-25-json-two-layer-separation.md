# ADR: JSON 双层分离 — 逻辑规则与控件渲染分文件

**日期**: 2026-05-25
**状态**: 提案中
**决策者**: 技术负责人

---

## 背景

当前 HMI JSON 规则文件中，逻辑状态流转和显示渲染混在一起。例如一条规则同时包含 "Zone1 选中后设档5"（业务逻辑）和 "segment 显示 5、LED 梯度亮"（渲染方式）。

这个问题和 C 代码中 APP 层直接生成段码本质相同——渲染知识泄漏进了逻辑层。

## 决策

**JSON 拆分为两层两个文件，逻辑规则和控件渲染分离。**

### 架构

```
┌─────────────────────────────────────────────┐
│  rules.json  (逻辑层 — 产品定义)             │
│                                             │
│  状态机流转: 事件 → 条件 → 输出抽象值         │
│  {                                          │
│    "on": "key_5",                           │
│    "when": "zone.selected == 1",            │
│    "then": { "zone": 1, "power": 5 }        │
│  }                                          │
│                     ↓ 输出抽象值              │
├─────────────────────────────────────────────┤
│  widgets.json (控件层 — UI 定义)             │
│                                             │
│  控件属性: 抽象值 → 渲染指令                  │
│  {                                          │
│    "zone_display": {                        │
│      "segment": "{{zone.power}}",           │
│      "dp": "{{hot_head == zone_id}}",       │
│      "blink": "{{zone.state == selecting}}" │
│    },                                       │
│    "led_bar": {                             │
│      "mode": "gradient",                    │
│      "level": "{{zone.power}}"              │
│    }                                        │
│  }                                          │
└─────────────────────────────────────────────┘
```

### 职责边界

| | rules.json | widgets.json |
|---|---|---|
| 管什么 | 状态机、事件路由、条件判断、输出抽象值 | 抽象值到渲染指令的映射、显示模式、段码选择 |
| 改什么时动 | 产品逻辑变更（如档位规则、定时行为） | UI 换肤（如单点改梯度、数码管改液晶屏） |
| 知不知道硬件 | 不知道 | 知道控件类型和渲染参数 |
| 测试方式 | 纯输入→预期输出值，不需 Mock 控件 | 抽象值→预期渲染指令，不需硬件 |
| 依赖 | 零依赖 | 只依赖 rules.json 的输出 schema |

### rules.json 输出 Schema

```json
{
  "zones": [
    {
      "id": 1,
      "state": "cooking",
      "power_level": 5,
      "target_power": 1200,
      "timer_remaining": 0
    }
  ],
  "hot_head": 1,
  "system": "working",
  "indicators": {
    "power_led": true,
    "child_lock_led": false,
    "timer_led": false
  }
}
```

### widgets.json 控件定义

```json
{
  "zone_slot": {
    "template": "{{zones[id].power_level}}",
    "fallback": {
      "selecting": "--",
      "paused": "PA",
      "idle": "--",
      "boost": "P"
    },
    "blink": "{{zones[id].state == 'selecting'}}",
    "dp": "{{hot_head == id}}",
    "dp_blink": false
  },
  "led_bar": {
    "mode": "gradient",
    "level": "{{zones[id].power_level}}",
    "max": 9
  }
}
```

### 与 C 代码对映

```
JSON 层                   C 代码层
───────                   ────────
rules.json  ──抽象值──→  APP/API 层 (api_display, api_led)
widgets.json ──渲染指令─→ DRV 层 (drv_display)
```

两端遵守完全相同原则：逻辑出值，渲染管线消费值。

## 后果

### 正面
- **产品逻辑和 UI 解耦**：改显示效果不动逻辑，改逻辑不动显示
- **测试独立**：rules.json 用纯数据驱动测试，widgets.json 用控件渲染用例测试
- **换 UI 零成本**：数码管换成液晶屏 → 只改 widgets.json，rules.json 不变
- **与 C 代码分层对齐**：两端同构，新成员只需理解一套规则

### 代价
- 规则拆分需要一次重构：现有 JSON 规则文件中的渲染属性要迁移到 widgets.json
- 引擎需要加载和关联两个文件（轻量：解析 schema + 模板求值）

### 迁移路径
1. 定义 rules.json 输出 schema（抽象值格式）
2. 抽取现有 JSON 中的渲染属性 → widgets.json
3. 引擎支持双文件加载 + 模板求值 `{{path.to.value}}`
4. rules.json 回归纯逻辑，逐条验证输出值不变

---
*关联: [[2026-05-25-display-rendering-to-drv]], [[ubiquitous-language]]*
