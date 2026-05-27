---
name: core-principles-iron-laws
description: 三条核心铁律：解耦第一，JSON覆盖逻辑第二，统一出入口第三。每次编程必须调入。
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 7eb949aa-9578-417f-ab12-d6428ae59747
---

**铁律一：解耦第一** — 层间隔离绝对不可妥协。APP 不得 include DRV/HAL，DRV 不得 include APP，HAL 零依赖。≤1ms 回调，>1ms 消息。仅 DRV→HAL 合法。

**Why:** 2026-05-26 耦合审计发现 12 项违规（8 P0）。根本原因是缺少检查脚本和门禁。已创建 `tools/check_deps.py` 作为编译前门禁。
**How to apply:** 每次编码前确认文件所在层 → 查依赖规则。每次编码后 `python tools/check_deps.py ../Claude`。不通过不得提交。

**铁律二：JSON 覆盖逻辑** — JSON 是规则权威数据源，JS 先行验证，C 跟随实现。

**Why:** 三层剥离架构是核心设计，覆盖率从 62%→87% 依赖此原则。
**How to apply:** HMI 逻辑变更：JSON 声明 → JS 测试通过 (34/34) → C 转换 → WASM 双引擎对比 → 全部 PASS。

**铁律三：统一出入口** — 回调不能随意插入。输入输出集中管理，统一处理。

**Why:** 防止回调散落在业务代码各处，导致数据流不可追踪。用户 2026-05-26 明确要求。
**How to apply:** 回调注册入口唯一（装配器初始化时统一注册）。不新增零散回调路径。新增回调前先确认能否用现有通道。
  - 显示数据流: APP → api_display(装配器) → DRV(回调) → HAL
  - 通信数据流: APP → Msg_Post → DRV(drv_comm) → HAL

**已固化位置:** `four_head/CLAUDE.md` 顶部铁律区。
