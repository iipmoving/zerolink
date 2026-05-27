# 定时逻辑专项检查报告

> 基于 `four_head_v4.json` × `json_logic_engine.js` × 用户补充需求
> 日期: 2026-05-25

---

## 完整定时流程（按你的描述）

```
selecting/cooking
    │
    ├── TIMER tap ──────────→ timer_setting (数字闪烁, 显示时间)
    │                              │
    │                    ┌─────────┼──────────┐
    │                    │         │          │
    │               TIMER tap    TIMER long   15s超时
    │                    │         │          │
    │                    ↓         ↓          ↓
    │              timer_active   取消      timer_active
    │              (交替显示)    (回原态)   (交替显示)
    │                    │
    │            ┌───────┼───────┐
    │            │               │
    │       TIMER tap         TIMER long
    │      (重进设置)          (取消定时)
    │            │
    │            ↓
    │      timer_setting
    │      (剩余时间为准)
    │            │
    │            └──→ 循环...
    │
    ├── timer归零 ──────────→ idle, 蜂鸣器长响
```

---

## 发现问题

### 🔴 P0-1: timer_active 缺少 TIMER tap 路由

**现状** — `process_routes.timer_active` 只有 TIMER long：

```json
"timer_active": {
  "routes": {
    "TIMER long": "cancel_timer_active"
  }
}
```

**你的需求** — timer_active 时按 TIMER tap → 重新进入 timer_setting，时间以当前剩余为准。

**缺的**：
```json
"TIMER tap": "enter_timer_setting"
```

**影响**：当前 timer_active 下按 TIMER tap 没有任何响应（进程路由没匹配到，退到 zone 路由也不匹配 TIMER tap in cooking）。

---

### 🟡 P1-1: timer_setting 闪烁方向反了

**JSON 现状** — `elements.blink.exclude_when` 包含 `"timer_setting"`：

```json
"exclude_when": ["timer_setting", "boost_active"]
```

意思是 timer_setting 时**抑制闪烁**。

**你的需求** — 定时设置时**数字需要闪烁**（给用户视觉反馈）。

**修正**：从 `exclude_when` 里删掉 `"timer_setting"`，只保留 `"boost_active"`：

```json
"exclude_when": ["boost_active"]
```

这样闪烁条件变为：`node==selecting && !boost_active`——timer_setting 时你在 selecting 态，当然闪。

---

### 🟡 P1-2: 重进 timer_setting 的时间来源

**现状** — `enterTimerSetting()` 固定设 `timer_value = default_timer_min` (15分钟)。

**你的需求** — 如果从 timer_active 重进 timer_setting，默认值应该是**当前剩余时间**。

**需要改引擎函数行为**（不能纯 JSON 解决）：

```
enterTimerSetting():
  if (heads[hotHead].timer_active):
      // 重进：以剩余时间为准
      timer_value = ceil(timer_remaining_ms / 60000)
  else:
      // 首次进入：默认定时时间
      timer_value = hmi_cfg.timeouts[HMI_TO_DEFAULT_TIMER_MIN]
```

**C 引擎 `app_hmi.c:enter_timer_setting()` 需要加这个判断**。JS 引擎 `json_logic_engine.js:695` 也一样要改。

---

## 修正路径

| # | 修什么 | 改哪里 | 工作量 |
|---|--------|--------|--------|
| P0-1 | timer_active 加 TIMER tap → enter_timer_setting | `four_head_v4.json` 1行 | <1分钟 |
| P1-1 | exclude_when 删 timer_setting | `four_head_v4.json` 1行 | <1分钟 |
| P1-2 | enterTimerSetting() 重进时取剩余时间 | `app_hmi.c` + `json_logic_engine.js` | 10分钟 |

P0-1 和 P1-1 改 JSON 就完事，P1-2 需要改 C 引擎和 JS 引擎两侧的 `enterTimerSetting()`。
