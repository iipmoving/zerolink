# four_head 实施进展审查报告

> 审查日期: 2026-06-24
> 工具: check_paradigm, check_switcher_macros, check_output_callback, check_deps, check_structs, check_weak_pairs, check_scheduler_symbols (新增), codeGen 生成器比对
> 数据源: four_head/src/json/project.json (v2.3)

---

## 一、全量检查结果

| # | 检查项 | 结果 | 问题数 | 说明 |
|---|--------|------|--------|------|
| 1 | check_scheduler_symbols | **FAIL** | 14 | main.c 的 14 个 weak 符号全为空调用 |
| 2 | check_paradigm | FAIL | 13 | 全部在 `#if 0` 块内，使用旧 g_input.info.status |
| 3 | check_switcher_macros | WARN | 34 | `seq` 字段命名应改为 `res[N]` |
| 4 | check_output_callback | PASS | 0 | 输出回调合规 |
| 5 | check_weak_pairs | PASS | 0 | __weak 配对一致 |
| 6 | check_structs | PASS | 0 | 结构体一致性通过 |
| 7 | check_deps | FAIL (32) | 32 | 全部是 `_io.h` include — 需更新白名单 |
| 8 | check_include | FAIL (995) | 995 | 全部在 worktree 副本和 vendor 库，非项目代码 |
| 9 | **生成器比对** (code_gen.py --only-pipes) | **PASS** | 0 | 生成输出与实际源码完全一致 |

---

## 二、按严重度分级

### P0 — 调度符号断裂 (阻断)

data_switcher.c 未覆盖 main.c 的任何调度入口。14 个 __weak 空壳实际效果:

| main.c 调用 | 当前符号 | 实际执行 | data_switcher.c 应提供 |
|-------------|---------|---------|----------------------|
| `Slot_Init()` | __weak 空壳 | ❌ 无 | `Slot_Init() { Switcher_Init(); Switcher_Run_All(); }` |
| `Slot_every1ms()` | __weak 空壳 | ❌ 无 | 编排需要每 1ms 轮询的模块 DoWork |
| `Slot_loop1ms()` | __weak 空壳 | ❌ 无 | 编排后台任务模块 DoWork |
| `Switcher_Run_Slot0~9` | __weak 空壳 | ❌ 无 | 一对一映射 `s_slot[SLOT_xxx].pDoWork()` |

### P1 — 模块业务逻辑 `#if 0` 禁用

| 模块 | 活动 user_Process | `#if 0` 内代码行数 | 状态 |
|------|------------------|-------------------|------|
| app_cooking.c | `(void)in; (void)out; (void)flags` | ~100 行 | ❌ 需迁移 |
| app_power.c | `(void)in; (void)out; (void)flags` | ~80 行 | ❌ 需迁移 |
| app_hmi.c | 逻辑在回调中 | 约 30 行 | ⚠️ 通过回调执行，绕过 PULL |

### P2 — 工具白名单需更新

| 工具 | 误报数 | 原因 |
|------|--------|------|
| check_deps.py | 32 | `_io.h` include 需加入白名单: 模块可 include 自己的 `_io.h`, Switcher 可 include 所有 |
| check_include.py | 995 | 需跳过 `.claude/worktrees/` 目录 + vendor 库 |
| check_switcher_macros.py | 34 | `seq` 字段命名警告 — 需确认是否必须改为 `res[N]` |

### P3 — seq 初始化边界

当前 `s_last_seq` 初始值为 0，Producer seq 也从 0 开始 → **首次数据不会触发处理**。需 Producer seq 从 1 开始。

---

## 三、生成器对比验证

比对 `codeGen/code_gen.py --only-pipes` 输出与 `four_head/src/` 实际文件:

| 文件类型 | 生成 vs 实际 | 结论 |
|---------|-------------|------|
| `include_io/*_io.h` (11个) | 仅差末尾换行符 | ✅ project.json 是准确的单一数据源 |
| `core/data_switcher.c` | 仅差末尾换行符 | ✅ 路由拓扑与 project.json 一致 |

---

## 四、审查工具交付物

| 文件 | 路径 |
|------|------|
| audit-fourhead SKILL | `.claude/skills/audit-fourhead/SKILL.md` |
| 调度符号检查工具 | `.claude/tools/check_scheduler_symbols.py` |

两者均已就绪，可在后续审查中直接使用 `/audit-fourhead` 触发，或在开发中运行 `check_scheduler_symbols.py` 验证调度链路完整性。