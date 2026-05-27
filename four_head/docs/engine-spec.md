# HMI 逻辑引擎规格书

> 本文件描述 JSON 路由表驱动的动作函数行为。JS 模拟器和 C 固件共用此规格。
> 引擎口号：JSON 决定"做什么"，引擎实现"怎么做"。

---

## 一、引擎总览

### 1.1 输入输出

```
输入:  MSG_KEY_EVENT(key_code, event_type)     — 按键事件
       MSG_TIMER_100MS()                        — 100ms 节拍
       MSG_TIMER_1S()                           — 1s 节拍

输出:  MSG_DISPLAY_REFRESH(seg_chars[8], seg_blink[4], seg_effect, leds[...])
       MSG_BUZZER_CTRL(cmd)                     — 蜂鸣器控制

       seg_chars = 内容 (Always), seg_effect = 特效 (Optional)
       显示 = ShowASCII(content) ⊕ ShowEffect(type, params)
       详见: docs/architecture/two-method-display-refactor-guide.md

内部状态:
       globalState.mode          — GLOBAL_POWERING_UP / VERSION_SHOW /
                                    POWERED_OFF / WORKING / PAUSED / DEEP_SLEEP
       globalState.paused        — bool, 暂停标志
       globalState.child_lock    — bool, 童锁标志
       globalState.power_on_tick — 上电计时器

       heads[i]:
         .node          — ZONE_IDLE / ZONE_SELECTING / ZONE_COOKING
         .power_level   — 0~9, -1=Boost中
         .original_power  — 进Boost前保存的档位
         .boost_active  — bool
         .timer_setting — bool
         .timer_active  — bool
         .timer_value   — 1~99 分钟
         .timer_remaining_ms — 定时倒计时剩余 ms

       hot_head        — 当前热点炉头索引 (0~3), -1=无
       select_stack[]  — LIFO 栈, 元素=炉头索引, 最大4
       blink_phase     — 闪烁相位 0/1 (每 blink_phase_ms 切换)
       alternate_phase — 交替显示相位 0/1 (每 5000ms 切换)
```

### 1.2 全局路由优先级

按键到达时按以下优先级匹配，命中即执行，不再向下匹配：

```
1. child_lock 激活?     → 仅白名单键放行, 其余忽略
2. paused?              → 仅 POWER long / PAUSE tap 放行
3. global_routes[mode]  → 全局态路由
4. process_routes[进程] → 当前活跃进程路由
5. zone_routes[node]    → Zone态路由
```

---

## 二、全局模式切换

### 2.1 `goWorking()`

| 项目 | 行为 |
|------|------|
| 触发 | POWERED_OFF/DEEP_SLEEP 下 POWER long |
| 动作 | 1. globalState.mode = WORKING |
| | 2. 四头: node=idle, power_level=0, 清所有进程标志 |
| | 3. hotHead = -1, 清空选择栈 |
| | 4. 执行 `global_routes.working.enter_actions[]` (来自 JSON) |
| | 5. 启动全 idle 守卫计时器 |

### 2.2 `goPoweredOff()`

| 项目 | 行为 |
|------|------|
| 触发 | WORKING/PAUSED 下 POWER long |
| 动作 | 1. globalState.mode = POWERED_OFF |
| | 2. 四头: node=idle, 清所有进程标志和计时器 |
| | 3. hotHead = -1, 清空选择栈 |
| | 4. 执行 `global_routes.powered_off.enter_actions[]` |
| | 5. 启动关机→休眠 30s 计时器 |

### 2.3 `enterDeepSleep()`

| 项目 | 行为 |
|------|------|
| 触发 | POWERED_OFF 下 30s 无操作超时 |
| 动作 | 1. globalState.mode = DEEP_SLEEP |
| | 2. 显示全灭, LED全灭 |
| | 3. 硬件进入低功耗模式（MCU具体实现） |

---

## 三、Zone 状态机动作

### 3.1 `selectHead(idx)`

| 项目 | 行为 |
|------|------|
| 触发 | 按炉头选择键 (HEAD_1~4 tap) |
| 当前 hotHead | 行为 |
|---|---|
| = idx (同一头) | **自确认**: 直接调 `confirmSelect(idx)`，不等超时 |
| ≠ idx (切头) | **旧头确认**: 调 `confirmSelect(old_idx)`，然后 idx 进入 selecting |
| -1 (无热点) | idx 直接进入 selecting，设 hotHead=idx |
| 后续 | 重置全空闲计时器 |

### 3.2 `confirmSelect(idx)`

| 项目 | 行为 |
|------|------|
| 触发 | selecting 超时 / 自按 / 切头确认 |
| if power_level > 0 | 1. heads[idx].node = COOKING |
| | 2. `pushToStack(idx)` |
| | 3. heads[idx].seg_blink = false |
| if power_level = 0 | 1. heads[idx].node = IDLE |
| | 2. `removeFromStack(idx)` |
| | 3. hotHead 交接: 栈非空→栈顶; 栈空→找第一个 cooking 兜底; 都没有→hotHead=-1 |
| 通用 | hotHead 确认后 LED 同步 |

### 3.3 `handlePowerKey(level)`

| 上下文 | 行为 |
|--------|------|
| selecting 态 | 1. heads[hotHead].power_level = level |
| | 2. 重置 selecting 超时计时器 (select_confirm_ms) |
| | 3. **不改变 node**, 保持 selecting |
| | 4. 如果 boost_active → 清 boost, 恢复原档位再设新档: |
| |    - 清 boost_active, 停 boost_timer |
| |    - power_level = level (不恢复 original_power) |
| cooking 态 (单头) | 1. heads[hotHead].power_level = level |
| | 2. if level=0 → heads[hotHead].node = IDLE, `removeFromStack(hotHead)` |
| | 3. 如果 boost_active → 清 boost, 再设新档 |
| cooking 态 (多头) | 必须先选: 忽略按键, 或由 resolveTarget 决定 |
| 通用 | 更新 LED, 重置全空闲计时器, 如有 timer_active 则保持 |

### 3.4 `resolveTarget()`

| 条件 | 返回 |
|------|------|
| 4 头全 idle | 忽略档位键 (无炉头可操作) |
| 仅 1 头 cooking, 无 selecting | 直接返回该头 (单头快捷改档) |
| 有 selecting 头 | 返回 selecting 头 |
| 多头 cooking, 无 selecting | 忽略 (必须先选) |

---

## 四、正交进程动作

### 4.1 `enterBoost()`

| 项目 | 行为 |
|------|------|
| 触发 | selecting/cooking 下 9 long |
| 前置条件 | boost_active=false (不能重复进入) |
| 动作 | 1. heads[hotHead].original_power = heads[hotHead].power_level |
| | 2. heads[hotHead].power_level = -1 (Boost专用值) |
| | 3. heads[hotHead].boost_active = true |
| | 4. 启动 Boost 5min 计时器 |
| | 5. 段码显示优先级: boost_active 优先 (显示 "P") |
| | 6. 如果当前在 selecting → 保持 selecting; 如果在 cooking → 保持 cooking |

### 4.2 `exitBoost()`

| 项目 | 行为 |
|------|------|
| 触发 | Boost 5min 超时 |
| 动作 | 1. heads[hotHead].boost_active = false |
| | 2. heads[hotHead].power_level = heads[hotHead].original_power |
| | 3. 停 boost_timer |
| | 4. 显示回退到 power_level 或 timer_active 交替模式 |

### 4.3 `exitBoostAndSetPower(level)`

| 项目 | 行为 |
|------|------|
| 触发 | Boost 中按 0~9 数字键 |
| 动作 | 1. heads[hotHead].boost_active = false |
| | 2. heads[hotHead].power_level = level (不恢复 original_power) |
| | 3. 停 boost_timer |
| | 4. 调 `handlePowerKey(level)` 处理新档位 |

### 4.4 `enterTimerSetting()`

| 项目 | 行为 |
|------|------|
| 触发 | selecting/cooking 下 TIMER tap |
| 前置条件 | timer_setting=false (不能重复进入) |
| 动作 | 1. heads[hotHead].timer_setting = true |
| | 2. heads[hotHead].timer_value = default_timer_min (来自 JSON) |
| | 3. 启动定时设置 15s 确认超时计时器 |
| | 4. 段码显示优先级: timer_setting 最高 (显示时间, 带小数点色) |

### 4.5 `confirmTimer()`

| 项目 | 行为 |
|------|------|
| 触发 | timer_setting 下 TIMER tap / 15s 超时 |
| 动作 | 1. heads[hotHead].timer_setting = false |
| | 2. heads[hotHead].timer_active = true |
| | 3. heads[hotHead].timer_remaining_ms = timer_value × 60 × 1000 |
| | 4. 停 timer_setting 超时计时器 |
| | 5. 段码进入交替模式 (档位/时间各5s) |

### 4.6 `cancelTimerSetting()`

| 项目 | 行为 |
|------|------|
| 触发 | timer_setting 下 TIMER long |
| 动作 | 1. heads[hotHead].timer_setting = false |
| | 2. heads[hotHead].timer_value = 0 |
| | 3. 显示回退到 power_level 或 boost |

### 4.7 `cancelTimerActive()`

| 项目 | 行为 |
|------|------|
| 触发 | timer_active 下 TIMER long |
| 动作 | 1. heads[hotHead].timer_active = false |
| | 2. heads[hotHead].timer_value = 0 |
| | 3. heads[hotHead].timer_remaining_ms = 0 |
| | 4. 停倒计时, 显示回退到 power_level 或 boost |

### 4.8 `handleTimerAdjust(delta)`

| 项目 | 行为 |
|------|------|
| 触发 | timer_setting 下 PLUS / MINUS tap |
| 动作 | 1. heads[hotHead].timer_value += delta |
| | 2. 钳位到 [adjust_min, adjust_max] |
| | 3. 显示更新为新的 timer_value |
| | 4. 重置 timer_setting 15s 超时计时器 |

### 4.9 `togglePause()`

| 项目 | 行为 |
|------|------|
| 触发 | WORKING 下 PAUSE tap |
| if paused=false | 1. globalState.paused = true |
| | 2. 冻结所有进程 (timer/boost 暂停计时) |
| | 3. 功率输出置 0 |
| | 4. 工作炉头显示 "PA" |
| if paused=true | 1. globalState.paused = false |
| | 2. 恢复所有进程 (timer/boost 继续计时) |
| | 3. 功率输出恢复 |
| | 4. 显示恢复原模式 |

### 4.10 `toggleChildLock()`

| 项目 | 行为 |
|------|------|
| 触发 | 任意非上电态下 CHILD_LOCK long |
| 动作 | 1. globalState.child_lock = !globalState.child_lock |
| | 2. 童锁 LED 同步 |
| | 3. child_lock=true 时: 拦截除白名单外的所有按键 (白名单来自 JSON `child_lock.block_all_except`) |

---

## 五、定时器节拍

### 5.1 `onTimer100ms()` — 超时检测

每 100ms 执行一次。遍历所有计时器查超时：

```
1. 全局守卫:
   遍历 guarded_actions[] (来自 JSON):
     if globalState.mode == guarded_actions[i].mode
     AND (模式进入后累计 ms >= guarded_actions[i].ms)
        → 执行 guarded_actions[i].action
        → 重置该守卫计时器

2. 模式内超时:
   if globalState.mode == state_timeouts[].mode
   AND 累计 ms >= state_timeouts[].ms
      → 执行 state_timeouts[].action

3. Zone 超时 (如 selecting 15s):
   for each head with node == ZONE_SELECTING:
     if heads[i].selecting_timer_ms >= zone_timeouts[ZONE_SELECTING].ms
       → zone_timeouts[ZONE_SELECTING].action  (i.e. confirmSelect(i))

4. 进程超时 (如 timer_setting 15s, boost 5min):
   for each head:
     if heads[i].timer_setting and timer_setting_timer_ms >= timer_confirm_ms
       → confirmTimer(i)
     if heads[i].boost_active and boost_timer_ms >= boost_max_ms
       → exitBoost(i)
```

**关键规则**: 任何有效按键 (见 §六) → 所有计时器归零。

### 5.2 `onTimer1s()` — 倒计时

每 1000ms 执行一次：

```
for each head i:
  if heads[i].timer_active AND heads[i].timer_remaining_ms > 0:
     heads[i].timer_remaining_ms -= 1000
     if heads[i].timer_remaining_ms <= 0:
       → 定时归零: heads[i].node = IDLE
       → heads[i].power_level = 0
       → heads[i].timer_active = false
       → removeFromStack(i)
       → 同步显示/LED
       → 蜂鸣器长响
```

---

## 六、计时器统一归零规则

**任何有效按键 → 所有计时器归零**。

| 按键分类 | 是否归零 |
|----------|---------|
| POWER long (开关机) | ✅ |
| HEAD_1~4 tap (选头) | ✅ |
| 0~9 tap (档位) | ✅ |
| TIMER tap/long | ✅ |
| PLUS/MINUS tap | ✅ |
| PAUSE tap | ✅ |
| CHILD_LOCK long | ✅ |
| 上电序列中的按键 | ❌ (6s内按键锁) |
| child_lock 拦截的按键 | ❌ |
| PAUSED 下非 POWER/PAUSE 按键 | ❌ |
| DEEP_SLEEP 下所有按键 | ❌ (仅 POWER long 唤醒) |

---

## 七、显示派生规则

### 7.1 段码优先级链

按优先级从高到低遍历 `priority_chain[]` (来自 JSON)，命中即取：

| 优先级 | 条件 | 显示字符 |
|--------|------|---------|
| 0 (最高) | heads[i].timer_setting == true | timer_value 两位数 |
| 1 | heads[i].boost_active == true | "P" |
| 2 | 默认 | power_level 一位数 (0~9) |

### 7.2 闪烁规则

```
blink[i] = (heads[i].node == ZONE_SELECTING)
            AND NOT (heads[i].timer_setting OR heads[i].boost_active)
blink_phase 每 blink_phase_ms 翻转。
```

例外: paused 时强制关闭闪烁 (`blink[i] = false`)。

### 7.3 交替规则

```
if heads[i].timer_active == true:
  段码在 power_level 和 timer_value 之间交替显示, 各 5000ms
  交替相位不影响其他炉头

if heads[i].timer_active AND heads[i].boost_active:
  段码在 "P" 和 timer_value 之间交替显示, 各 5000ms
```

### 7.4 LED 规则

| LED | 规则 |
|-----|------|
| 电源灯 | globalState.mode: POWERED_OFF→闪烁; DEEP_SLEEP→灭; 其他→亮 |
| 童锁灯 | globalState.child_lock == true → 亮 |
| 暂停灯 | globalState.paused == true → 亮 |
| 定时灯 | 任意炉头 timer_active == true → 亮 |
| 选头灯 | 对应炉头 node == SELECTING → 亮 |
| 档位灯 (0~9) | 跟随 hotHead 的 power_level: hotHead=-1→全灭; boost_active→亮9; 否则亮对应档位 |

---

## 八、选择序列栈管理

### 8.1 `pushToStack(idx)`

| 项目 | 行为 |
|------|------|
| 触发 | confirmSelect 且 power_level > 0 |
| 规则 | 1. 如果 idx 已在栈中 → 先移除再压入 (移到栈顶) |
| | 2. 压入栈顶 |
| | 3. 如果栈长 > max_depth → 不移除 (不溢出) |

### 8.2 `removeFromStack(idx)`

| 项目 | 行为 |
|------|------|
| 触发 | confirmSelect 且 power_level = 0 |
| 规则 | 1. 找到 idx 在栈中的位置并移除 |
| | 2. 后续元素前移 |

### 8.3 `getStackTop()`

返回栈尾元素 (LIFO)。栈空返回 -1。

### 8.4 `reassignHotHead(idx)`

| 项目 | 行为 |
|------|------|
| 触发 | 当前 hotHead 确认到 idle 时 |
| 规则 | 1. 栈非空 → 栈顶接任 hotHead |
| | 2. 栈空 → 遍历 4 头找第一个 cooking 的接任 |
| | 3. 没有 cooking → hotHead = -1 (**全 idle**) |
| | 4. 全 idle → 启动全空闲→关机 30s 守卫计时器 |

---

## 九、上电序列

### 9.1 时序

```
[POWERING_UP]:   所有段全亮 ("8888") + 所有 LED 亮 → 3s →
[VERSION_SHOW]:  上排 "V1.0", 下排 "P1.0" + LED 全灭 → 3s →
[POWERED_OFF]:   所有段 "--" (不闪烁) + 仅电源灯闪烁 → 6s内按键无效
```

### 9.2 序列步骤来自 JSON

每步由 `power_on_sequence[]` JSON 数组描述：
- `delay_ms`: 该步持续时间
- `seg_chars`: 段码显示字符 (8字符)
- `leds_mask`: LED 掩码 (bitfield)
- `goto`: 完成后的目标全局模式

引擎按步骤索引递增执行, 每步等待 `delay_ms` 后进入下一步。

---

## 十、引擎函数清单

| # | 函数 | 输入 | 影响 |
|---|------|------|------|
| E01 | `goWorking()` | — | 全局模式, 四头状态 |
| E02 | `goPoweredOff()` | — | 全局模式, 四头状态 |
| E03 | `enterDeepSleep()` | — | 全局模式, 显示 |
| E04 | `selectHead(idx)` | 炉头索引 | hotHead, zone node, 栈 |
| E05 | `confirmSelect(idx)` | 炉头索引 | zone node, 栈, LED |
| E06 | `handlePowerKey(level)` | 档位值 | power_level, zone node, boost |
| E07 | `resolveTarget()` | — | 返回目标炉头索引 |
| E08 | `enterBoost()` | — | boost_active, power_level |
| E09 | `exitBoost()` | — | boost_active, power_level |
| E10 | `exitBoostAndSetPower(level)` | 档位值 | boost_active, power_level |
| E11 | `enterTimerSetting()` | — | timer_setting, timer_value |
| E12 | `confirmTimer()` | — | timer_setting→active, 倒计时 |
| E13 | `cancelTimerSetting()` | — | timer_setting |
| E14 | `cancelTimerActive()` | — | timer_active |
| E15 | `handleTimerAdjust(delta)` | ±1 | timer_value |
| E16 | `togglePause()` | — | globalState.paused |
| E17 | `toggleChildLock()` | — | globalState.child_lock |
| E18 | `onTimer100ms()` | — | 超时检测 |
| E19 | `onTimer1s()` | — | 倒计时 |
| E20 | `pushToStack(idx)` | 炉头索引 | 选择栈 |
| E21 | `removeFromStack(idx)` | 炉头索引 | 选择栈 |
| E22 | `getStackTop()` | — | 返回栈顶索引 |
| E23 | `reassignHotHead(idx)` | 炉头索引 | hotHead, LED |

---

*本规格书为 JSON 路由驱动的动作层行为定义。JS 模拟器和 C 固件的引擎实现均以此为准。*
*版本: 1.0 | 日期: 2026-05-24*
