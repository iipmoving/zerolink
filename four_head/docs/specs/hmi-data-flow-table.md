# 四头电磁炉 HMI 状态机规格书

> 单点真相来源。V7.0：Zone 状态坍缩为 3 个，正交进程独立。
> 按键编码: fourSave.json key_code (1=定时 2=暂停 3=童锁 4=开关 5-8=炉头1-4 9=无区 10=减 11=加 12-21=档位0-9)

---

## 一、系统双层模型 (A/B 双线)

全局模式分 A/B 两类，互斥。同一时刻系统只处于一个全局模式。

### 1.1 A 线 — 公共不可操控

炉头不接受任何操作。按键响应仅限长按开关（开机/唤醒）。

```
POWERING_UP(3s) → VERSION_SHOW(3s) → POWERED_OFF ←→ DEEP_SLEEP
                                          ↑              ↑
                                    全idle后30s     关机后30s无操作
```

| 模式 | 进入条件 | 退出条件 | 数码管显示 | LED |
|------|----------|----------|-----------|-----|
| `POWERING_UP` | MCU 上电复位 | 3s 后自动 | 8 段全亮（8888） | 全部 LED 亮 |
| `VERSION_SHOW` | 全显 3s 结束 | 3s 后自动 | Top Panel "V1.0", Bottom Panel "P1.0" | 全灭 |
| `POWERED_OFF` | 版本结束 / WORKING 下全 idle 30s / 任何态长按开关 | 长按开关 1.5s → WORKING | "--" **不闪烁**（4 个 Z{N} Slot 均显示 "--"） | **电源灯闪烁**，其余全灭 |
| `DEEP_SLEEP` | POWERED_OFF 下 30s 无操作 | 长按开关 1.5s → WORKING | 全灭 | 全灭 |

### 1.2 B 线 — 可操控

```
POWERED_OFF 长按开关 1.5s → WORKING ←→ PAUSED(冻结)
                                 │
                         各炉头独立状态机
```

| 模式 | 进入条件 | 退出条件 | 说明 |
|------|----------|----------|------|
| `WORKING` | POWERED_OFF / DEEP_SLEEP 下长按开关 1.5s | 长按开关 → POWERED_OFF / 全 idle 30s → POWERED_OFF | 各炉头按自身状态独立显示 |
| `PAUSED` | WORKING 下按暂停键 | 按暂停键恢复 / 长按开关关机 | **冻结**：不改任何炉头状态/进程。仅功率输出为 0 |

- **PAUSED = 紧急停止**：状态机冻结、功率置零。恢复后原样继续。
- **CHILD_LOCK = 按键过滤器**：不改变全局模式，仅拦截除开关外的所有按键。

### 1.3 全局模式流转总图

```
POWERING_UP(3s) → VERSION_SHOW(3s) → POWERED_OFF ←→ DEEP_SLEEP
                                          ↑    ↑        ↑
                                          │  长按开关    │30s无操作
                                          │    ↓         │
                                          │  WORKING ←───┘
                                          │    ↑↓ 暂停键
                                          │  PAUSED
                                          │
                                    全idle 30s
```

---

## 二、Zone 状态（仅 3 个）

WORKING 下每个 Zone 独立维护自己的 node。同一时刻每个 Zone 只处于一个 node。

```
       ┌──────────────────────────┐
       │                          │
       ↓                          │
    idle ──按炉头键──→ selecting ──┤
       ↑               │          │
       │   按档位键    │ 15s/自确认 │
       │  (更新值,     │ (>0)     │
       │   不退出)     ↓          │
       │            cooking ←─────┘
       │               │
       └──设0档────────┘
```

| 状态 | 英文 | Z{N} Slot 显示 | seg_blink | 触发条件 | 超时 |
|------|------|----------------|-----------|----------|------|
| 空闲 | `idle` | "0" 常亮 | false | WORKING 进入时默认 / cooking 下设 0 档 | — |
| 选中闪烁 | `selecting` | 档位数字闪烁（未设档="0"） | **仅本炉头** true | 按对应 Zone 键 | **15s** 无操作 → 确认 |
| 加热 | `cooking` | 档位数字常亮 | false | selecting 确认时 power_level>0 / 单头快捷设档 | — |

**确认规则**（selecting → cooking 或 idle）：
- 15s 超时自动确认：power_level > 0 → cooking，压入选择序列栈顶 / power_level = 0 → idle，移除
- 同一炉头再按（自确认）：立即确认，不等 15s
- 切换炉头：旧 hotHead 自动确认

**selecting 态按档位键**：更新 power_level，**保持 selecting**，重置 15s 计时器。不改变 node。

---

## 三、正交进程（叠加在 Zone 状态之上）

进程不改变 `node`。它们是标志 + 独立的生命周期（进入条件 → 运行 → 终止条件判断 → 退出动作）。

### 3.1 进程清单

| 进程 | 英文 | 可进入的 node | 进入方式 | 显示效果 | 终止条件（任一命中） | 退出动作 |
|------|------|--------------|----------|----------|---------------------|----------|
| 定时设置 | `timer_setting` | selecting / cooking | 按定时键（TAP） | 显示时间数字，小数点色 | ① 15s 超时 ② 按定时键 TAP ③ 长按定时键 LONG | ①或②: 确认→启动 `timer_active` 进程; ③: 取消→清 timer_value |
| 定时运行 | `timer_active` | cooking (不可和 selecting 并存) | timer_setting 确认 | 交替显示: 档位/时间各 5s | ① 倒计时归零 ② 长按定时键 LONG | ①: cooking→idle, power=0, 弹出栈; ②: 取消→清 timer_value, 回 cooking |
| 强火 | `boost_active` | selecting / cooking | selecting/cooking 态长按 9 | 显示 "P" 替代档位数字 | ① 5min 超时 ② 按任意档位键 TAP | ①: 清标志, 恢复原档位; ②: 清标志, 按档位键正常处理 |

### 3.2 进程叠加组合

| node | boost_active | timer_setting | timer_active | 显示 | 备注 |
|------|-------------|---------------|--------------|------|------|
| idle | - | - | - | "0" | 不允许任何进程 |
| selecting | false | false | false | 档位数字闪烁 | 纯选中态 |
| selecting | false | true | false | 时间数字 | 选中态按定时键 |
| selecting | true | false | false | "P" 闪烁 | 选中态长按 9 |
| cooking | false | false | false | 档位数字常亮 | 纯加热态 |
| cooking | false | true | false | 时间数字 | 加热态按定时键 |
| cooking | false | false | true | 档位/时间交替 | 定时运行中 |
| cooking | true | false | false | "P" 常亮 | Boost 中 |
| cooking | true | false | true | "P"/时间交替 | Boost+定时运行 |

### 3.3 进程退出条件 — JSON 表达

引擎不写 if-else。进程的终止条件在 JSON 中声明，引擎轮询命中任一条即执行退出动作。

```jsonc
// timer_setting 进程
{
  "process": "timer_setting",
  "enter_on": { "trigger": "key", "key": "TIMER", "evt": "tap" },
  "enter_action": { "set_flag": "timer_setting", "init_timer_value": 15 },
  "display": { "mode": "timer_setting", "color": "#44aaff" },
  "exit_conditions": [
    { "trigger": "timeout", "ms": 15000, "action": "confirm_timer" },
    { "trigger": "key", "key": "TIMER", "evt": "tap", "action": "confirm_timer" },
    { "trigger": "key", "key": "TIMER", "evt": "long", "action": "cancel_timer" }
  ]
}

// timer_active 进程
{
  "process": "timer_active",
  "enter_on": "confirm_timer",
  "enter_action": { "clear_flag": "timer_setting", "set_flag": "timer_active" },
  "display": { "mode": "alternating", "items": ["power", "timer"], "interval_ms": 5000 },
  "exit_conditions": [
    { "trigger": "timer_zero", "action": "head_to_idle" },
    { "trigger": "key", "key": "TIMER", "evt": "long", "action": "cancel_timer" }
  ]
}

// boost_active 进程
{
  "process": "boost_active",
  "enter_on": { "trigger": "key", "key": "9", "evt": "long" },
  "enter_action": { "set_flag": "boost_active", "save_original_power": true },
  "display": { "override_char": "P" },
  "exit_conditions": [
    { "trigger": "timeout", "ms": 300000, "action": "restore_power" },
    { "trigger": "key", "key": "0-8", "evt": "tap", "action": "clear_boost_and_handle_power" }
  ]
}
```

---

## 四、机制一：热点炉头队列

### 4.1 核心概念

```
选中炉头 (Selected Head)
  = 处于 selecting node 的炉头，最多 1 个。
  = 选中炉头一定是热点炉头。

热点炉头 (Hot Head)
  = LED 档位灯的唯一数据源。取值 = zone_id(0-3) 或 -1（清空）。
  = **待机**：hotHead = -1 → 档位 LED 全灭。系统干净，从未有过非 0 输出。
  = **0 功率**：hotHead ≥ 0 且 power_level = 0 → 档位 LED 亮 0。炉头曾有过非 0 输出后回到 0。
  = 选中炉头一定是热点炉头。
  = hotHead 只有确认后（15s 超时/自按/切换）且栈空时才清空（→ -1）。

选择序列 (Selection Stack)
  = 已确认功率 > 0 的炉头的 LIFO 栈，最大深度 4。
  = 确认时 power_level > 0 则压入栈顶，power_level = 0 则从栈移除。
  = hotHead 确认到 idle 且栈非空 → 栈顶接任 hotHead。
  = hotHead 确认到 idle 且栈空 → hotHead = -1（清空，进入待机）。
```

### 4.2 操作规则

| 操作 | 对 hotHead | 对选择序列 | 对 LED |
|------|-----------|-----------|--------|
| 按炉头 N 键（N = hotHead, selecting） | **自确认**：立即确认（>0→cooking, =0→idle） | >0 压栈 / =0 移除 | 跟新 hotHead |
| 按炉头 N 键（N ≠ hotHead） | 旧 hotHead 确认 → N 成为新 hotHead | >0 压入旧 / =0 移除旧 | 切到新 hotHead（含 0 档） |
| 按档位键（selecting） | 更新 power_level，保持 selecting，重置 15s | 不变 | 跟 hotHead（含 0 档） |
| 按档位键（cooking） | 立即生效：新档或 idle | >0 压栈 / =0 移除 | 跟 hotHead |
| 15s 确认 (>0) | → cooking，仍为 hotHead | 压入栈顶 | 跟 hotHead（档位数字） |
| 15s 确认 (=0) | → idle，**退出**热点炉头位置 | 移除该炉头 | 栈顶炉头成为新 hotHead，LED 切到栈顶 |
| 栈空 | — | 空 | 找第一个 cooking 炉头兜底 |

---

## 五、机制二：计时器统一归零

### 5.1 计时器清单

| 计时器 | 时长 | 触发后果 | 配置常量 |
|--------|------|----------|----------|
| 选中计时 | 15s | selecting → 自动确认 | `CFG_SELECT_TIMEOUT_MS` |
| 定时设置计时 | 15s | timer_setting → 自动确认 | `CFG_TIMER_CONFIRM_MS` |
| 全空闲计时 | 30s | 四头全 idle → POWERED_OFF | `CFG_IDLE_TO_OFF_MS` |
| 关机计时 | 30s | POWERED_OFF → DEEP_SLEEP | `CFG_OFF_TO_SLEEP_MS` |
| Boost 计时 | 5min | boost_active → 清标志 | `CFG_BOOST_TIMEOUT_MS` |

### 5.2 统一清零规则

**任何有效按键 → 所有计时器全部归零。**

有效按键：长按开关、长按童锁、炉头选择键、档位键、定时键、+/- 键、暂停键。
无效按键：A 类模式按键、关机态非开关、童锁拦截、PAUSED 非开关/暂停。

---

## 六、配置常量

| 常量 | 值 | 说明 |
|------|-----|------|
| `CFG_SELECT_TIMEOUT_MS` | 15000 | 选中→确认 15s |
| `CFG_TIMER_CONFIRM_MS` | 15000 | 定时设置→自动确认 15s |
| `CFG_IDLE_TO_OFF_MS` | 30000 | 全 idle→POWERED_OFF 30s |
| `CFG_OFF_TO_SLEEP_MS` | 30000 | POWERED_OFF→DEEP_SLEEP 30s |
| `CFG_BOOST_TIMEOUT_MS` | 300000 | Boost→恢复 5min |
| `CFG_DEFAULT_TIMER_MIN` | 15 | 默认定时 15 分钟 |

---

## 七、显示行为

### 7.1 数码管映射

```
         ┌──────────────┬──────────────┐
         │    Z1 Slot    │    Z2 Slot    │
  Top    │  seg[0..1]    │  seg[2..3]    │
         ├──────────────┼──────────────┤
         │    Z3 Slot    │    Z4 Slot    │
  Bottom │  seg[4..5]    │  seg[6..7]    │
         └──────────────┴──────────────┘
```

### 7.2 显示决策（优先级从高到低）

```
1. timer_setting 活跃?  → 显示时间数字 (timer_value)
2. boost_active 活跃?   → 显示 "P"
3. 否则:                 → 显示档位数字 (power_level)
4. node=selecting?      → seg_blink[head]=true
5. timer_active 活跃?   → 档位/时间交替 (各 5s)
```

### 7.3 LED 灯

| LED | 亮条件 |
|-----|--------|
| 电源灯 | POWERING_UP: 亮; POWERED_OFF: **闪烁**; WORKING: 亮; DEEP_SLEEP: 灭 |
| 童锁灯 | CHILD_LOCK 激活时 |
| 定时灯 | 任意炉头 timer_active=true 时 |
| 暂停灯 | PAUSED 时 |
| 无区灯 | 组合设置模式时 |
| 档位灯 0-8 | 跟随热点炉头档位 |
| 档位灯 9-P | 热点炉头 boost_active 时 |

---

## 八、数据流（单向）

### Flow-00: 上电序列
```
[POWERING_UP] 全显 3s → [VERSION_SHOW] V1.0/P1.0 3s → [POWERED_OFF] "--" + 电源灯闪烁
```

### Flow-01: 关机→开机
```
[POWERED_OFF] 长按开关 1.5s → [WORKING] 4 头 idle, 显示 "0 0 0 0"
```
测试: `key POWER long`
验证: global_mode=working, 4 头 node=idle, seg_blink=[f,f,f,f]

### Flow-02: 选中炉头
```
[WORKING] 按炉头键 → 该炉头 selecting, Zn 区闪烁, LED 跟 hotHead
```
测试: `key HEAD_1 tap`
验证: heads[0].node=selecting, seg_blink=[t,f,f,f], hotHead=0

### Flow-03: 选中→设 0 档→确认 idle
```
[selecting] 按 0 键 → power=0, 保持 selecting, 重置 15s
→ 15s 超时/自按确认 → idle, 移出栈
```
测试: `key 0 tap` → `forceSelectTimeout`
验证: heads[0].node=idle, power_level=0

### Flow-04: 选中→设档→确认 cooking
```
[selecting] 按 1-8 键 → 更新 power_level, 保持 selecting, 重置 15s
→ 15s 超时/自按确认 → cooking, 压栈
```
测试: `key 5 tap` → `forceSelectTimeout`
验证: heads[0].node=cooking, power_level=5

### Flow-05: 选中→短按 9
```
[selecting] 短按 9 → power=9, 保持 selecting, 重置 15s → 确认→cooking
```
测试: `key 9 tap` → `forceSelectTimeout`
验证: heads[0].node=cooking, power_level=9

### Flow-06: 选中→长按 9→Boost
```
[selecting] 长按 9 → boost_active=true, 显示 "P", 5min 计时
```
测试: `key 9 long`
验证: boost_active=true, 显示 "P"

### Flow-07: 选中→15s 超时（全 0→idle）
```
[selecting, power=0] 15s 无操作 → idle, 全 idle 启动 30s 关机计时
```

### Flow-08: 选中→15s 超时（有档位→cooking）
```
[selecting, power>0] 15s 无操作 → cooking, 压栈
```

### Flow-09: 选中→按定时键
```
[selecting] 按定时键 → timer_setting 进程启动, 显示时间, 默认 15min
```
测试: `key TIMER tap`
验证: timer_setting 活跃, timer_value=15

### Flow-10: 单头快捷改档
```
[cooking, 仅 1 头 cooking] 直接按档位键 → 立即更新 power_level
```
测试: `key 3 tap`
验证: heads[0].power_level=3

### Flow-11: cooking→设 0 档
```
[cooking] 按 0 → idle, power=0, 移出栈, LED 回退栈顶
```
测试: `key 0 tap`
验证: heads[0].node=idle

### Flow-12: cooking→长按 9→Boost
```
[cooking] 长按 9 → boost_active=true, 保存原档位, 显示 "P"
```
测试: `key 9 long`
验证: boost_active=true, original_power 已保存

### Flow-13: 暂停
```
[WORKING] 按暂停 → [PAUSED] 所有进程冻结, 功率=0
```
测试: `key PAUSE tap`
验证: paused=true

### Flow-14: cooking→按定时键
```
[cooking] 按定时键 → timer_setting 进程启动
```
测试: `key TIMER tap`
验证: timer_setting 活跃

### Flow-15: 定时归零→停机
```
[timer_active] 倒计时归零 → idle, power=0, 弹出栈
```
测试: `forceTimerExpire`
验证: heads[0].node=idle, power_level=0

### Flow-16: Boost→按数字键退出
```
[boost_active] 按 0-8 → 清 boost_active, 按档位键正常处理
```
测试: `key 5 tap`
验证: boost_active=false, power_level=5

### Flow-17: Boost→5min 超时
```
[boost_active] 5min 无操作 → 清 boost_active, 恢复原档位
```
测试: `forceBoostTimeout`
验证: boost_active=false, power_level=original_power

### Flow-18: Boost→按定时键
```
[boost_active] 按定时键 → timer_setting 进程启动
```
测试: `key TIMER tap`
验证: boost_active=true, timer_setting 活跃

### Flow-19: 暂停→恢复
```
[PAUSED] 按暂停 → [WORKING] 恢复原状态, 功率恢复
```
测试: `key PAUSE tap`
验证: paused=false

### Flow-20: 定时设置→加减调整
```
[timer_setting] +/- → timer_value ±1 (1~99), 重置 15s 确认计时
```
测试: `key PLUS tap` / `key MINUS tap`

### Flow-21: 定时设置→手动确认
```
[timer_setting] 按定时键 TAP → 确认: 清 timer_setting, 启动 timer_active
```
测试: `key TIMER tap`
验证: timer_setting=false, timer_active=true

### Flow-22: 定时设置→15s 自动确认
```
[timer_setting] 15s 无操作 → 同 Flow-21
```

### Flow-23: 定时设置→长按取消
```
[timer_setting] 长按定时键 LONG → 清 timer_setting, 清 timer_value, 回原态
```
测试: `key TIMER long`
验证: timer_setting=false, timer_value=0

### Flow-24: 童锁
```
长按童锁 1.5s → child_lock=true, 童锁灯亮
```
测试: `key CHILD_LOCK long`

### Flow-25: 童锁解锁
```
[child_lock] 长按童锁 1.5s → child_lock=false
```

### Flow-26: 关机
```
任意 WORKING/PAUSED 长按开关 1.5s → [POWERED_OFF] 全部 idle, "--" 不闪烁, 电源灯闪烁
```
测试: `key POWER long`
验证: global_mode=powered_off, seg_blink=[f,f,f,f]

### Flow-27: 多炉头独立工作
```
头1 设 5 档 cooking → 头2 设 3 档 cooking, 各自独立, LED 跟栈顶
```

### Flow-28: 多炉头独立定时
```
头1 timer_active, 头2 无定时 → 独立倒计时
```

### Flow-29: Boost+定时交替
```
[boost_active + timer_active] 显示 "P"/时间交替
```

### Flow-30: 全 idle→30s→关机
```
[WORKING] 四头全 idle, 30s 无操作 → [POWERED_OFF]
```

### Flow-31: 关机→30s→休眠
```
[POWERED_OFF] 30s 无操作 → [DEEP_SLEEP]
```

### Flow-32: 休眠→唤醒
```
[DEEP_SLEEP] 长按开关 1.5s → [WORKING]
```

### Flow-33: 同一炉头自按确认
```
[selecting] 按同一炉头键 → 立即确认 (不等 15s)
```
测试: `key HEAD_1 tap` `key 5 tap` `key HEAD_1 tap`
验证: heads[0].node=cooking (立即)

---

## 九、测试汇总

| Flow# | 路径 | 测试命令 | P |
|-------|------|----------|----|
| 00 | 上电序列 | 自动 | P0 |
| 01 | 关机→开机 | `key POWER long` | P0 |
| 02 | 选中炉头 | `key HEAD_1 tap` | P0 |
| 03 | 设 0→idle | `key 0 tap` → `forceSelectTimeout` | P0 |
| 04 | 设档→cooking | `key 5 tap` → `forceSelectTimeout` | P0 |
| 05 | 短按 9 | `key 9 tap` → `forceSelectTimeout` | P0 |
| 06 | 长按 9→Boost | `key 9 long` | P0 |
| 07 | 选中超时(全0) | 等 15s | P1 |
| 08 | 选中超时(有档) | 等 15s | P1 |
| 09 | 选中→定时 | `key TIMER tap` | P0 |
| 10 | 单头快捷 | `key 3 tap` | P0 |
| 11 | 0 档弹出序列 | `key 0 tap` | P1 |
| 12 | 加热→Boost | `key 9 long` | P1 |
| 13 | 暂停 | `key PAUSE tap` | P0 |
| 14 | 加热→定时 | `key TIMER tap` | P1 |
| 15 | 定时归零 | `forceTimerExpire` | P2 |
| 16 | Boost 退出 | `key 5 tap` | P1 |
| 17 | Boost 超时 | `forceBoostTimeout` | P2 |
| 18 | Boost→定时 | `key TIMER tap` | P1 |
| 19 | 暂停恢复 | `key PAUSE tap` | P0 |
| 20 | 定时加减 | `key PLUS/MINUS tap` | P1 |
| 21 | 定时手动确认 | `key TIMER tap` | P1 |
| 22 | 定时自动确认 | 等 15s | P2 |
| 23 | 定时取消 | `key TIMER long` | P1 |
| 24 | 童锁 | `key CHILD_LOCK long` | P1 |
| 25 | 童锁解锁 | `key CHILD_LOCK long` | P1 |
| 26 | 关机 | `key POWER long` | P0 |
| 27 | 多炉头工作 | 序列操作 | P1 |
| 28 | 多炉头定时 | 序列操作 | P2 |
| 29 | Boost+定时交替 | 序列操作 | P2 |
| 30 | 全 idle→关机 | 等 30s | P2 |
| 31 | 关机→休眠 | 等 30s | P2 |
| 32 | 休眠→唤醒 | `key POWER long` | P1 |
| 33 | 自按确认 | `key HEAD_1 tap` `key 5 tap` `key HEAD_1 tap` | P1 |

---

## 十、代码重构待办

| # | 偏差 | 代码现状 | 正确行为 |
|---|------|----------|----------|
| 1 | node 枚举 | 7 种 (idle/selecting/cooking/boost/timer_setting/cooking_timer/boost_timer) | 3 种 (idle/selecting/cooking) |
| 2 | 进程管理 | 用 node 区分，switch-case 7 路分支 | 正交标志 + 进程退出条件轮询 |
| 3 | 显示决策 | `updateDisplayForHead()` switch 7 种 node | 优先级链: timer_setting→boost→power, 再叠加 blink/alternate |
| 4 | 全局模式名 | `'standing_by'` | `'working'` |
| 5 | 全 idle 处理 | `checkAllIdle()` 直接调 `goStandingBy()` | 启动 30s 计时器 → POWERED_OFF |
| 6 | POWERED_OFF 闪烁 | `seg_blink = [t,t,t,t]` | `seg_blink = [f,f,f,f]`, 仅电源 LED 闪烁 |
| 7 | 计时器清零 | 分散在各处手动重置 | 统一 `resetAllTimers()` |
| 8 | 进程退出条件 | 硬编码 if-else | JSON 声明的 exit_conditions 列表 |

---

---

## 十一、JSON 声明式逻辑点（18 个）

> 这些是"规则/配置"——改 JSON 即改行为，不用动程序代码。

### 11.1 全局状态机规则（4 个）

| # | 逻辑点 | JSON 表达 | 所属节点 |
|---|--------|----------|----------|
| G1 | 上电全显3s→版本3s→关机 | `power_on_sequence: [{step:0, delay:3000, seg:"8888"}, {step:1, delay:3000, seg:"V1.0/P1.0"}, {step:2, goto:"powered_off"}]` | `power_on_sequence` |
| G2 | 长按开关→开机/关机 | `on: {"key":"POWER", "evt":"long", "toggle":["working","powered_off"]}` | `global_state_machine` |
| G3 | 关机态30s无操作→休眠 | `powered_off: {..., timeout: 30000, goto: "deep_sleep"}` | `powered_off` 节点 |
| G4 | 休眠态仅响应开关长按 | `deep_sleep: {on: {"KEY_POWER": {evt:"long", goto:"working"}}}` | `deep_sleep` 节点 |

### 11.2 Zone 状态转移规则（6 个）

| # | 逻辑点 | JSON 表达 | 所属节点 |
|---|--------|----------|----------|
| Z1 | 按炉头键→选中并闪烁 | `on: {"KEY_HEAD_N": {goto:"selecting", action:"set_hotHead"}}` | `idle` / `cooking` |
| Z2 | 选中态按档位键→设档+重置15s | `on: {"KEY_0~9": {action:"set_power_level", reset_timer:15000}}` | `selecting` |
| Z3 | 选中态15s无操作→自动确认 | `selecting: {timeout: 15000, goto_by_power: {0:"idle", ">0":"cooking"}}` | `selecting` 节点 |
| Z4 | 同一炉头再按→手动确认 | `on: {"KEY_HEAD_SELF": {action:"confirm_immediate"}}` | `selecting` |
| Z5 | 切炉头→旧炉头自动确认 | `on: {"KEY_HEAD_OTHER": {action:"confirm_old_head", goto:"selecting"}}` | `selecting` |
| Z6 | cooking态按0→idle | `on: {"KEY_0": {goto:"idle", action:"remove_from_stack"}}` | `cooking` |

### 11.3 正交进程规则（5 个）

| # | 逻辑点 | JSON 表达 | 所属节点 |
|---|--------|----------|----------|
| P1 | 长按9→进入Boost(5min超时) | `on: {"KEY_9": {evt:"long", call:"processes.boost.enter"}}` | `selecting` / `cooking` |
| P2 | Boost中按任意档位→退出Boost | `boost_active.exit_on: [{trigger:"key", key:"0-9", evt:"tap", action:"exit_boost_and_set_power"}]` | `processes.boost_active` |
| P3 | 定时键→进入/确认/取消定时（三态） | `call:"processes.timer"` → 内部: `idle→timer_setting→timer_active→idle` | `processes.timer` |
| P4 | 定时设置15s自动确认 | `timer_setting.exit_conditions: [{trigger:"timeout", ms:15000, action:"confirm_timer"}]` | `processes.timer_setting` |
| P5 | 定时归零→idle | `timer_active.exit_conditions: [{trigger:"timer_zero", action:"head_to_idle"}]` | `processes.timer_active` |

### 11.4 全局控制规则（3 个）

| # | 逻辑点 | JSON 表达 | 所属节点 |
|---|--------|----------|----------|
| C1 | 暂停键→冻结/恢复 | `on: {"KEY_PAUSE": {evt:"tap", toggle:"paused"}}` | `global_state_machine` |
| C2 | 童锁长按→锁定/解锁 | `on: {"KEY_CHILD_LOCK": {evt:"long", toggle:"child_lock"}}` | `global_state_machine` |
| C3 | 全idle 30s→关机（A线） | `working: {guard:"all_idle", timeout: 30000, goto: "powered_off"}` | `working` 节点 |

---

## 十二、程序引擎函数（15 个）

> 这些是"算法/机制"——必须写代码。但**每个都是单一功能函数，平均 8 行**，修改任何逻辑点只需定位唯一函数。

### 12.1 消息分发（1 个）

| 函数 | 功能 | 行数 | 为何必须是代码 |
|------|------|------|---------------|
| `onKeyEvent(msgId, param)` | 解析按键消息 → 匹配当前全局/Zone状态 → 路由到具体 handler | ~40 | 消息解包、状态匹配、路由分发是引擎核心循环，JSON 只声明规则，引擎负责执行匹配 |

**内部结构**（扁平条件分支，互不依赖）：
```
全局状态守卫(上电序列/休眠/关机/童锁/暂停) → 按键类型分支 → 调具体handler
```
修改某一状态下的按键行为：找到对应的 `if` 块，改一行。

### 12.2 超时检测（2 个）

| 函数 | 功能 | 行数 | 为何必须是代码 |
|------|------|------|---------------|
| `onTimer100ms(param)` | 100ms 节拍：遍历4头检测选中超时/定时确认超时/Boost超时/全idle计时 | ~30 | 时间源(Date.now())是平台API，JSON 只管超时值和后果 |
| `onTimer1s(param)` | 1s 节拍：遍历4头倒计时减1，归零触发 idle | ~15 | 倒计时是算术运算，JSON 只管归零后的动作 |

### 12.3 Zone 操作（5 个）

| 函数 | 功能 | 行数 | 为何必须是代码 |
|------|------|------|---------------|
| `selectHead(idx)` | 旧hotHead确认 → 新头进入selecting → 更新LED | ~18 | 涉及两个炉头状态联动（旧确认+新选中），是原子事务 |
| `confirmSelect(idx)` | selecting确认：node→cooking/idle，压栈/移栈，清闪烁 | ~14 | 状态+栈+LED三联动，原子操作 |
| `handlePowerKey(level)` | 档位键：根据当前node（selecting/cooking）决定立即生效或仅设值 | ~22 | selecting/cooking分支逻辑不同，需判断上下文 |
| `resolveTarget()` | 确定操作目标：选中态用hotHead，单头cooking直接用，多头需先选 | ~12 | 快捷操作规则，依赖4头状态遍历 |
| `handleTimerAdjust(delta)` | +/- 调整定时值：范围钳位 1~99，重置15s | ~8 | 纯算术+边界钳位 |

### 12.4 正交进程（3 个）

| 函数 | 功能 | 行数 | 为何必须是代码 |
|------|------|------|---------------|
| `handleTimerKey(keyState)` | 定时键三态流转：idle→timer_setting / timer_setting→确认或取消 / timer_active→取消 | ~28 | 同一按键不同状态三种行为，需上下文判断 |
| `confirmTimer(idx)` | 定时确认：清timer_setting→设timer_active，seg_mode切回power | ~14 | 标志切换+显示模式联动 |
| `enterBoost()` / `exitBoost(idx)` | Boost进出：保存/恢复原档位，设5min计时 | ~10+8 | 保存原值→改档位→设计时器，原子操作 |

### 12.5 全局操作（2 个）

| 函数 | 功能 | 行数 | 为何必须是代码 |
|------|------|------|---------------|
| `goWorking()` / `goPoweredOff()` / `enterDeepSleep()` | 全局模式切换：批量重置所有炉头+LED+显示 | ~20+20+10 | 批量状态重置是引擎"事务"，一次改多个数据结构 |
| `togglePause()` / `toggleChildLock()` | 暂停/童锁切换：单标志翻转+显示联动 | ~10+6 | 标志翻转+显示副作用 |

### 12.6 显示与LED（2 个）

| 函数 | 功能 | 行数 | 为何必须是代码 |
|------|------|------|---------------|
| `displayCharForHead(h)` | 显示优先级链：timer_setting→boost→power_level | ~6 | 优先级链是硬决策，JSON 只管每级的字符来源 |
| `syncLED()` | LED 档位灯跟随 hotHead：清数组→读hotHead档位→设对应灯 | ~7 | 数组操作，hotHead=-1(待机)时提前返回 |

### 12.7 选择序列（2 个）

| 函数 | 功能 | 行数 | 为何必须是代码 |
|------|------|------|---------------|
| `pushToStack(idx)` / `removeFromStack(idx)` / `getStackTop()` | 栈操作：LIFO，最大深度4，去重 | ~3+3+1 | 标准数组操作，数据结构维护 |
| `reassignHotHead(idx)` | hotHead确认idle后移交：栈有人→栈顶接任，栈空→留原地 | ~6 | 移交流水线逻辑 |

---

## 十三、分工总结

```
                    逻辑点总数: 33
                    ┌──────────┼──────────┐
               JSON声明式    程序函数      渲染/驱动
               18 个(55%)   15 个(45%)   (不计入)
                    │            │
              改JSON即可    定位唯一函数
              不动代码      改几行完成
```

| 对比维度 | JSON 声明式 | 程序函数 |
|----------|-----------|---------|
| 数量 | 18 个逻辑点 | 15 个函数 |
| 平均大小 | 5-10 行 JSON | 8 行 JS |
| 修改方式 | 改 JSON 字段值 | 找到唯一函数，改几行 |
| 是否需要理解引擎 | 否 | 否（每个函数自包含） |
| 典型修改耗时 | 秒级 | 分钟级 |
| 举例 | 把超时从15s改成20s → 改一个数字 | 新增"双击炉头键清零定时" → 在 `handleTimerKey` 加2行 |

**关键结论：程序函数全是"单一功能、短小、自包含"的。没有上帝函数，没有跨函数耦合需要追踪。新增逻辑点 = 在对应函数加几行，不会牵一发动全身。**

---

*最后更新：2026-05-22 V7.0。Zone 状态坍缩为 3 个，boost/timer 转为正交进程。新增十一~十三章：JSON逻辑与程序函数分工。*
