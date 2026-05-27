---
name: project-p2-mode-rule
description: P2 ModeRule — seg_mode 声明式优先级链绑定，~15处硬编码移除，34/34 通过
metadata: 
  node_type: memory
  type: project
  originSessionId: 8468b018-0dc2-4dea-925a-f289e7ef9c0b
---

P2 ModeRule 完成：声明式 seg_mode 绑定引擎。

**Why:** seg_mode 是 `globalState.mode + per-head flags` 的纯函数，之前 ~15 处硬编码写入。ModeRule 在 postDisplay() 中从状态派生，消除了所有直接写入。

**How to apply:** 新增全局显示模式规则时，修改 `four_head_v4.json` → `elements.mode.rules` 的优先级链即可，引擎零改动。规则按顺序求值，第一匹配者胜。

**关键设计决策:**
- `"when": "paused"` 检查 `globalState.paused` 标志（而非 `globalState.mode === 'paused'`），与引擎内部 paused 以 flag 形式叠加在 working 上的设计一致
- 开机序列 (`power_on_seq`/`version_show`) 不受 ModeRule 控制，postDisplay() 中 guard 跳过
- 4个状态转换函数 (goWorking/goPoweredOff/enterDeepSleep/togglePause) 统一在 applyActions 后调用 postDisplay()，确保 ModeRule 总是有机会从状态派生 seg_mode
- 1个测试 (Flow-19) 的期望值从 `seg_mode='power'` 更新为 `seg_mode='timer_setting'`：旧代码在 handlePowerKey 中硬编码 `seg_mode='power'` 作为视觉覆盖，ModeRule 正确检测到 timer_setting 进程仍活跃

**变更文件:**
- `js/mode_rule.js` — 新建 55 行
- `js/json_logic_engine.js` — 集成 ModeRule (~+10行) + 删除 ~15 处 seg_mode 写入
- `logic/four_head_v4.json` — 添加 elements.mode
- `run_tests_node.js` + `test_hmi.html` + `test_auto.html` — 加载 mode_rule.js
- `js/test_runner.js` — Flow-19 断言更新
