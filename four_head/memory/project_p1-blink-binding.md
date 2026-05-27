---
name: p1-blink-binding
description: P1 BlinkRule — 闪烁逻辑 JSON 声明化，phase_ms 可配置，~21处硬编码 seg_blink 写入移除
metadata:
  node_type: memory
  type: project
  originSessionId: 8468b018-0dc2-4dea-925a-f289e7ef9c0b
---

P1 完成于 2026-05-23。

## 变更

- 新增 `js/blink_rule.js` — BlinkRule 元素（55行）
  - `shouldBlink(head)` 纯函数：node=="selecting" && !timer_setting && !boost_active
  - `syncToCache(heads, displayCache)` 遍历4头求值→写入 seg_blink[]
  - `phaseMs` 从JSON `elements.blink.phase_ms` 读取，默认500
  - `exclude_when` 声明覆盖规则，默认 ["timer_setting", "boost_active"]
- `four_head_v4.json` 新增 `elements.blink`: phase_ms=300, condition, exclude_when
- `json_logic_engine.js`:
  - `init()` 创建 blinkRule 实例
  - `getDisplayRules()` 从 blinkRule.phaseMs 读取闪烁间隔（替代旧的 display.blink_rule 读取）
  - `postDisplay()` 顶部调用 blinkRule.syncToCache() + 暂停覆盖
  - **删除 ~21处** `displayCache.seg_blink[...] = ...` 硬编码写入
- 3个加载文件添加 blink_rule.js 引用

## 删除的硬编码分布

| 函数 | 删除数 |
|------|--------|
| runPowerOnSeqStep + applySeqDisplay | 3 |
| selectHead | 3 |
| handlePowerKey | 2 |
| confirmSelect | 1 |
| confirmTimer | 1 |
| enterTimerSetting | 1 |
| cancelTimerSetting | 1 |
| enterBoost | 1 |
| goWorking/goPoweredOff/enterDeepSleep 回退 | 3 |
| togglePause 回退 | 1 |
| showDash/showPA/showAllOff | 3 |

## 效果

- 改 JSON `phase_ms: 200` → 闪烁加快，引擎+渲染器零改动
- 改 JSON `exclude_when: ["timer_setting"]` → Boost期间闪烁恢复，引擎零改动
- 改 JSON `condition: "node == cooking"` → 加热时才闪烁（假设），引擎零改动
- 引擎中 seg_blink 只剩3处引用（数据定义+postDisplay消息发送）
- 向后兼容：无 elements.blink 段时 blinkRule=null，postDisplay 跳过同步，seg_blink 保留旧值

## 验证

- `node run_tests_node.js` → 34/34 通过
- seg_blink 求值与删除前的硬编码语义完全一致
- 暂停覆盖: postDisplay 强制全false（因为PA显示期间不应闪烁）

## 架构意义

P1 是 Binding Element 模式的验证——BlinkRule 不同于之前的 SlotElement/LEDElement：
- SlotElement/LEDElement: "属性预配置，方法运行时调用"（props→methods）
- BlinkRule: "条件→求值→目标"（condition→evaluate→target）

这建立了 `source→condition→target` 的声明链，JSON 是唯一变更点。为 P2 泛化提供了模式基础。

Why: 用户要求闪烁间隔可JSON配置，并作为 P1 Binding Engine 的第一个落地用例。
How to apply: P2 将泛化此模式到更多绑定类型（seg_mode、alternate），可能引入表达式求值。
