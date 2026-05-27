---
name: p0-4-status-leds
description: P0-4 StatusLED 全面集成 — syncStatusLEDs() 替代 35处硬编码状态LED写入，引擎状态→LED单向数据流完成
metadata:
  node_type: memory
  type: project
  originSessionId: 8468b018-0dc2-4dea-925a-f289e7ef9c0b
---

P0-4 完成于 2026-05-23。

## 变更

- **led_element.js**: StatusLED 重写，支持三种求值类型：
  - `states`: global_mode → behavior 映射表（power LED）
  - `flag`: 直接读 globalState[flagName]（pause, child_lock）
  - `condition`: 引擎侧求值函数（any_timer_active, head_selecting）
  - `array`: 数组型 LED（head_select[4]）
- **four_head_v4.json**: `elements.led` 新增 timer/pause/child_lock/head_select 四个状态 LED 声明
- **json_logic_engine.js**: 
  - `init()` 创建 5 个 StatusLED 实例（power/timer/pause/child_lock/head_select）
  - 新增 `syncStatusLEDs()` 函数：遍历所有 StatusLED，从 engine state 求值→写入 displayCache.leds
  - 移除 `applyLEDRulesForMode()`（被 syncStatusLEDs 替代）
  - 移除 ~20 处 hardcoded `displayCache.leds.{timer,pause_btn,child_lock,head_select} = ` 直接写入
  - 在 8 个状态转移函数中插入 `syncStatusLEDs()` 调用：goWorking, goPoweredOff, enterDeepSleep, togglePause, toggleChildLock, selectHead, handlePowerKey, confirmSelect, confirmTimer, cancelTimerSetting, cancelTimerActive, onTimer1s, clearAllHeads

## 保留的硬编码

- `allLEDsOn()`: 全显测试函数（line 287-291）
- `enter_actions` 处理中的 `led_*_on/off`: JSON 命令式动作（line 514-523）
- `allLEDsOff()`: 全灭重置命令（line 1157-1165）
- 这些是**命令**而非**状态求值**，语义不同，保留合理

## 效果

- 改 JSON `"timer": { "condition": "any_timer_active", "ledField": "timer" }` → 定时灯跟随任意炉头定时状态
- 改 JSON `"pause": { "flag": "paused", "ledField": "pause_btn" }` → 暂停灯跟随全局暂停标志
- 新增状态灯（如故障灯）只需在 JSON `elements.led` 添加声明 + 引擎 syncStatusLEDs() 添加一行求值
- 引擎中不再有分散的 `displayCache.leds.timer = checkAnyTimer()` 式写入
- 向后兼容：无 elements.led 段时 statusLEDs[] 为空，syncStatusLEDs() 直接返回

## 验证

`node run_tests_node.js` → 34/34 通过

## 架构意义

P0-1~P0-4 完成了三层架构中 **Zone Logic → Presentation Controls** 的单向数据流闭环：

```
Zone Logic 输出: power_level, node, boost_active, timer_active, ...
       ↓
syncStatusLEDs() 求值: flag/condition → boolean
       ↓
StatusLED.syncToCache() → displayCache.leds
       ↓
Renderer 读取 displayCache.leds → 绘制
```

Why: syncStatusLEDs() 是 JSON Binding Layer 的雏形——声明 source→target→transform，引擎只做解释。
How to apply: P1 将泛化为通用 Binding Engine（source→target→transform 声明式机制）。
