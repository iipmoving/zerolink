# JSON 驱动灯板逻辑可行性评估

> 对照：JSON规则驱动架构 V3.0 × 四头电磁炉完整规格书 V1.0

---

## 一、核心结论：可行，但需要微调架构

**可行度：85%**

JSON 驱动方案覆盖了灯板逻辑的绝大部分场景，但有 3 个地方需要补充或调整架构。

---

## 二、完美匹配项（JSON 原方案直接覆盖）

### 2.1 元素状态标识化 ↔ 数码管/LED 控制

| 规格书需求 | JSON 映射 | 匹配度 |
|---|---|---|
| 数码管显示功率档位 | `segment.set_mode("power")` | ✅ |
| 数码管显示 --（待机） | `segment.set_mode("dash")` | ✅ |
| 数码管显示 PA（暂停） | `segment.show_ascii("PA")` | ✅ |
| 数码管显示 P（Boost） | `segment.show_ascii("P")` | ✅ |
| 数码管交替显示时间+功率 | `segment.set_mode("alternating")` | ✅ |
| LED 闪烁/常亮/熄灭 | `led.xxx.set_state("blink/on/off")` | ✅ |
| 小数点常亮/熄灭 | 已有 `segment.set_decimal` | ✅ |
| 0~9 档位指示灯 | `led.power_level_X.on/off` | ✅ |

**结论**：状态标识化设计与规格书完全对应，零改动。

### 2.2 独立进程 ↔ 定时/Boost/暂停/童锁

| 规格书需求 | JSON 独立进程 | 匹配度 |
|---|---|---|
| 每炉头独立定时 1~99 分钟 | `timer_process` | ✅ |
| Boost 5 分钟超时回到原档位 | `boost_process` | ✅ |
| 全局暂停所有工作炉头 | `pause_process` | ✅ |
| 童锁长按 1.5 秒开启/关闭 | `child_lock_process` | ✅ |
| 定时结束 → 蜂鸣器响 | 可追加 `buzzer_process` | ✅ 需微补 |

**结论**：独立进程模型完美适配定时/Boost/暂停/童锁。

### 2.3 上电序列独立进程

| 步骤 | JSON 实现 | 匹配度 |
|---|---|---|
| 全显 3 秒 | `{"display": "all_segments", "show": "8888"}, {"delay": 3000}` | ✅ |
| 版本显示 3 秒 | `{"display": "seg_1", "show": "V1.0"}`, `{"display": "seg_2", "show": "P1.0"}` | ✅ |
| 待机态(--闪烁，电源灯闪烁) | `segment.set_mode("dash")` + `led.power.blink` | ✅ |

**结论**：完全匹配。

### 2.4 四炉头独立逻辑区

规格书里每个炉头有独立的状态（档位、定时、Boost），这正好是 JSON 架构 "4份完全独立实例 + 热点炉头判断" 的设计。

**结论**：完全匹配。

---

## 三、需要微调项（架构已有框架，需补充细节）

### 3.1 9 键的松开检测

规格书要求 9 键用**松开检测**（按下不触发，松手才判断短按/长按）。

**现状**：JSON 架构的事件模型只有 `condition: "short_press"` / `"long_press"`，没有区分"按下触发"和"松开触发"。

**建议**：在事件模型中增加一个 event_type 字段：

```json
{
  "trigger": "key_9",
  "event_type": "release",     // 新增：按 release 触发，默认 press
  "condition": "short_press",  // < 1.5s 短按
  "next": "node_working"
},
{
  "trigger": "key_9",
  "event_type": "release",
  "condition": "long_press",   // >= 1.5s 长按 → Boost
  "call": "boost_process.enter",
  "next": "node_boost"
}
```

**影响面**：小。只在事件匹配引擎加一个字段过滤即可。

### 3.2 5 秒选中超时的特殊规则

规格书的 5 秒超时规则比 JSON 架构想的更细：

> 选中炉头后 5 秒无操作：
> - 档位=0 → 回 Standby（显示 -- 闪烁）
> - 档位>0 → 停止闪烁，进入工作态

**现状**：JSON 架构的 timeout 条件没有区分"是否已有功率"。

**建议**：保持超时事件不变，在 `functions.deselect` 函数内加判断逻辑：

```json
"functions": {
  "timeout_exit": {
    "actions": [
      {
        "if": "state.power_level == 0",
        "then": {"call": "functions.goto_standby"},
        "else": {"call": "functions.enter_working"}
      }
    ]
  }
}
```

**影响面**：小。函数调用本身已经有条件判断能力（JSON 架构第 4.2.3 节的 `if/then/else`）。

### 3.3 组合模式下显示炉头选择的闪烁

规格书要求：
> 组合生效后，按组内任一炉头键 → 对应数码管数字闪烁 5 秒，表示该组被选中

**现状**：组合模式下，选中炉头不再显示"--"闪烁，而是**数字闪烁**。

**建议**：在选中态节点增加 "zone" 模式分支：

```json
"node_selected_zone": {
  "description": "组合模式下选中组",
  "enter_actions": [
    {"action": "segment.set_mode", "args": ["power"]},
    {"action": "segment.set_blink", "args": [true]},   // 数字闪烁
    {"action": "led.zone_selected.on"}
  ]
}
```

**影响面**：小。新增一个节点即可。

---

## 四、需要补充架构项

### 4.1 组合模式（无区组合） — 需要全局状态管理层

**这是最大的架构gap。**

JSON 架构默认每个炉头是**完全独立**的实例，但组合模式带来了跨炉头的耦合：

| 组合类型 | 耦合内容 |
|---|---|
| 1+2 / 3+4（上下） | 档位/定时同步 |
| 1+3 / 2+4（左右） | 档位/定时同步 |
| 全组合 | 全部同步 |
| 未组合 | 独立 |

**问题**：当一个炉头的档位改变时，同组其他炉头的档位也必须跟着变。这破坏了"每个炉头独立状态"的原子性。

**建议方案**：在全局状态管理层增加一个 **Zone Manager**：

```
Head 1 独立状态 ─┐
Head 2 独立状态 ─┤
Head 3 独立状态 ─┤  ← Zone Manager 负责写入时的同步
Head 4 独立状态 ─┘
                  │
                  ├─ 组合前：各头独立写入
                  └─ 组合后：写入头A → Zone Manager 自动同步到头B
```

具体实现方式（推荐第②种）：

| 方案 | 做法 | 评价 |
|---|---|---|
| ① JSON 层耦合 | 在事件触发时同时调用多个炉头的 set_power | 会导致 JSON 文件冗长 |
| ② 处理器层耦合 | Zone Manager 在处理器层拦截 set_power，自动同步同组炉头 | ✅ 推荐，JSON 无需改动 |
| ③ 数据模型耦合 | 一组炉头共享同一个 power_level | 复杂，不推荐 |

**推荐方案②的 JSON 描述**（JSON 层面完全无感知）：

```json
{
  // JSON 仍然只操作单个炉头
  "actions": [
    {"action": "state.power_level", "set": 5}
  ],
  
  // Zone Manager 在处理器层自动同步
  // zone_group = [1, 2] → head_2.power_level 自动设为 5
}
```

### 4.2 组合模式的显示符号

规格书要求组合选择态显示：
- 竖杠 | \|（上下组合）
- 横杠 -（左右组合）
- 方框 □（全组合）
- 互不相连图案（未连接）

**建议**：将组合显示符号注册为新的 segment display_mode：

```json
"segment": {
  "display_modes": {
    "zone_symbol": {
      "description": "组合连接符号显示",
      "symbols": {
        "vertical_bar": "|",
        "horizontal_bar": "-",
        "square": "□",
        "disconnected": "交错"  // 具体显示图案由处理器渲染
      }
    }
  }
}
```

### 4.3 组合自动补全规则

> 选中 2 个 → 剩余 2 个自动配成一对
> 选中 1 或 3 个 → 不补全
> 选中 4 个 → 全组合

这些是纯业务逻辑，**不适合放 JSON**，应该在处理器层的 `ZoneManager` 里实现。

---

## 五、改动量汇总

| 模块 | 改动内容 | 工作量评估 |
|---|---|---|
| **JSON 引擎** | 事件模型增加 `event_type: "release"` | 小（0.5天） |
| **JSON 引擎** | 函数增加 `if/then/else` 条件判断 | 已有框架，微调（0.5天） |
| **JSON 引擎** | 新增 `segment.set_mode("zone_symbol")` | 小（0.5天） |
| **JSON 配置** | head_template 补全规格书完整状态机 | 中（1天） |
| **处理器层** | 新增 `ZoneManager`（组合耦合逻辑） | 中（1.5天） |
| **处理器层** | 新增 `BuzzerHandler`（蜂鸣器） | 小（0.5天） |

**总计**：约 **4.5 天**，其中 JSON 相关改动约 **2.5 天**，处理器层新增约 **2 天**。

---

## 六、总结

| 评估维度 | 结论 |
|---|---|
| 灯板逻辑适配度 | ✅ 80% 完美匹配，20% 微调即可 |
| 最大的架构 gap | 组合模式（无区组合）的跨炉头耦合 |
| 是否需要推翻架构 | ❌ 不需要，补充 Zone Manager 即可 |
| 推荐实施策略 | 先做 P0 主线调功率（无组合），再补组合模式 |
