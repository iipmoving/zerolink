---
name: p0-1-low-difficulty-fixes
description: P0-1 四项低难度JSON化修复已完成，JSON覆盖率 62%→68%
metadata: 
  node_type: memory
  type: project
  originSessionId: 8468b018-0dc2-4dea-925a-f289e7ef9c0b
---

P0-1 完成于 2026-05-23，34/34 测试通过。

四项修改：
1. power_key_long_ms 加入 cfg() 映射表 — JSON超时可读
2. child_lock.block_all_except 白名单生效 — JSON声明→引擎读取
3. char_mapping.*.source 动态属性查找 — getSourceVal() 替代硬编码属性名
4. led_rules.power_led states_map — readLEDRule() + applyLEDRulesForMode()

Why: 用户要求JSON覆盖率最大化，这四项是不改架构直接可做的低难度修复。
How to apply: 后续P0-2开始需要定义元素接口，改动量加大。
