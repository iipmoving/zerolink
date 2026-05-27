# 设计方法论

> 从实现过程中提炼，指导后续开发。

## 1. 延时确认模式：符号化封装

延时→确认这类模式（如选中15s后确认、定时5s自动确认）应抽象为可复用符号/函数：

```
{on_timeout: 15s, do: "confirm_select"}
{on_timeout: 5s, do: "confirm_timer"}
```

- **引擎侧**：维护符号→确认函数的映射表，调度时按名调用
- **JSON侧**：只声明触发条件和符号名，不写实现逻辑
- **优势**：将来改为纯JSON驱动时，只需将函数引用替换为JSON规则块

## 2. JSON 大原则：声明 WHAT，不写 HOW

> 这是检查 JSON 设计正确性的最高准则。

- **JSON 侧**：声明"要什么"——触发条件、目标状态、显示模式名、进程名
- **引擎侧**：负责"怎么做"——状态转移、消息投递、计时管理、显示驱动
- **判断标准**：如果 JSON 里出现计时器管理代码、状态转移函数调用、显示逻辑——它就写错了
- **正确示例**：
  ```jsonc
  // 声明意图：超时做什么
  { "on": "timeout", "ms": 15000, "do": "confirm_select" }

  // 声明显示模式：引擎负责计时和切换
  { "display": "alternating", "items": ["power", "timer"], "interval_ms": 5000 }

  // 声明进程：独立生命周期
  { "process": "timer_setting",
    "enter_on": { "trigger": "key", "key": "TIMER", "evt": "tap" },
    "display": { "mode": "timer_setting" },
    "exit_conditions": [
      { "trigger": "timeout", "ms": 15000, "action": "confirm_timer" },
      { "trigger": "key", "key": "TIMER", "evt": "tap", "action": "confirm_timer" },
      { "trigger": "key", "key": "TIMER", "evt": "long", "action": "cancel_timer" }
    ]
  }
  ```
- **错误示例**：JSON 里写 `setTimeout(...)`、`head.node = 'cooking'`、`if (Date.now() - start > 15000)` → 全错

## 3. 状态机 = 全局状态 × Zone 状态 × 正交进程

- **全局状态**（POWERING_UP / VERSION_SHOW / POWERED_OFF / WORKING / PAUSED / DEEP_SLEEP）：描述整机阶段。CHILD_LOCK 是按键过滤器，不是状态。
- **Zone 状态**（仅 3 个：idle / selecting / cooking）：描述炉头的核心运行态。同一时刻每个 Zone 只能处于一个 node。
- **正交进程**（boost_active / timer_setting / timer_active）：叠加在 Zone 状态之上的独立标志。不改变 node，有自己的生命周期（进入条件 → 运行 → 终止条件列表 → 退出动作）。
- 三者独立并行，不互相覆盖。

### 为什么进程是标志不是状态

- **避免组合爆炸**：3 状态 × 3 进程标志 = 最多 27 种组合。如果写成状态，需要 3×2×2 = 12 个状态名（idle/cooking/boost/timer_setting/cooking_timer/boost_timer...），且每加一个进程就倍增。
- **显示决策是优先级链**：timer_setting > boost > power_level，再用 blink/alternate 叠加。switch-case 7 路分支变成优先级链 + 标志判断。
- **进程退出条件独立**：每个进程有自己的终止条件列表（超时 OR 按键 OR 归零），用 JSON 声明，引擎轮询。

## 4. 选中态是 Zone 状态，不是进程

selecting 是 3 个 Zone 状态之一（不是进程标志），因为它改变核心行为：
- 只有 selecting 态有 15s 超时确认逻辑
- 只有 selecting 态决定是否压入选择序列
- 只有 selecting 态触发 seg_blink 闪烁
- 确认后 node 变为 cooking 或 idle，这是核心状态转移

## 5. 选择序列（Selection Stack）

- 最大深度4的LIFO栈
- 炉头确认非零功率 → 压入栈顶
- 炉头归零 → 从栈移除，LED回退到栈顶
- 栈空 → LED档位灯全灭，找第一个 cooking 炉头兜底
- 选头时LED即时跟随选中炉头，确认后跟随栈顶

## 6. 进程退出条件 = 多条件 OR 判断

进程的结束不是单一条件，而是一个多条件判断函数。任一条件命中即执行对应退出动作：

```
exit_conditions = [条件A, 条件B, 条件C, ...]
// 引擎轮询: hit = exit_conditions.find(c => c.trigger 满足)
// hit → 执行 hit.action
```

- **JSON 声明**：列出所有可能的退出条件（超时、按键、归零），每个条件带 action 名
- **引擎轮询**：每 tick 扫描活跃进程的 exit_conditions，命中任一条即执行对应退出动作
- **这不写 if-else**：JSON 只列条件，引擎负责检测和匹配
- **示例**：timer_setting 的退出条件 = `[15s超时→确认, 按TIMER TAP→确认, 长按TIMER→取消]`——三条平行，不存在嵌套优先级

## 7. 计时器统一归零

任何有效按键 → 所有计时器同时归零。计时器包括：选中15s、定时设置15s、全空闲30s、关机30s、Boost 5min。

- 不是"只重置相关计时器"——是全部
- 无效按键（关机态误按、童锁拦截、PAUSED 非开关/暂停、A 类模式）不归零
- 这保证了：用户有操作意愿时，系统不会因某个计时器到期而意外跳转状态
