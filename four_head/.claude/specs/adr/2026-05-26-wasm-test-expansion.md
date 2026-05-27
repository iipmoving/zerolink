# ADR 2026-05-26: WASM 双引擎测试扩展

## 状态
已接受

## 背景
WASM 双引擎测试基础设施已完成基础冒烟测试（7 场景，12 断言，全部通过）。键码三层一致性已验证（C `drv_key.h` ↔ WASM adapter `KEY_CODE` ↔ JS engine `KEY_NAME`）。测试员报告发现 4 项改进点，其中日志噪音和覆盖率不足为 P0。

## 决策

### 1. 日志噪音消除
在 `test_wasm_basic.js` 中为 `MSG_DISPLAY_REFRESH` 和 `MSG_BUZZER_CTRL` 注册 no-op handler，因为测试环境无实际硬件。

### 2. 测试扩展策略
- **复用 JS test_runner.js 的 34 条数据流作为验收标准**，逐 Phase 扩展 WASM 双引擎测试
- **Phase1 优先**（Flow-00~06 + Flow-07~08）达到 9/34 覆盖率
- **不做 test_runner.js 重构**：JS 用闭包 `verify(fn)`、WASM 用 `assertState/assertDisplay`，合并成本高于收益
- 新增测试场景使用**独立 setup**（复用已有引擎状态或描述清楚前置条件）

### 3. 覆盖率节奏
- P0（本轮）：T008 噪音消除 + T009 Phase1 扩展 → 9/34 (26%)
- P1（下轮）：T010~T011 Phase2 + 边界测试 → 12/34 (35%)
- 后续：Phase3-4 逐 Phase 扩展

## 影响
- `test_wasm_basic.js` 行数增加约 50%
- 不修改被测代码、不修改 JSON 规则

---

*决策人：技术负责人，2026-05-26*
