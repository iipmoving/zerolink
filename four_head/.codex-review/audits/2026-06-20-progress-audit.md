---
name: progress-audit-2026-06-20
description: 项目完成度审查 — v2.3 范式落地 ~70%，处于迁移过渡期，方法论基础设施缺失
metadata:
  node_type: memory
  type: project
  auditor: Codex
---

Codex 于 2026-06-20 对 `four_head/src` 做了一次方法论合规审查。整体结论：五层架构与 v2.3 模块范式已落地约 70%，项目正处于"手动迁移 → 自动化检查"的中间状态。核心骨架已搭好，但方法论基础设施文件与配套检查工具尚未部署到位。

**Why:** 用户要求 Codex 接手项目进展审查职责。本文件作为审查基线，后续每次审查对比此基线判断进展。
**How to apply:** 推进任一改动后，重跑 `python tools/check_deps.py . --project four_head`，对照下方缺口清单逐项消除；缺口清零后更新本记忆。

## 一、已完成（做得好）

| 维度 | 状态 |
|------|------|
| 五层架构 app/drv/hal/proto/core | 完整 |
| `src/core/std_module.h` v2.3 | 已就位（含 seq 有效性 + route 子功能入口） |
| `src/core/data_switcher.c` | 已实现，`SLOT_GETIO` 注册 11 个模块 |
| `_io.h` 接口文件 | `include_io/` 11 个模块，v2.3 LINK+PARAMS 命名 |
| MODULE_SKELETON + MODULE_EXPORT | 11 个新模块全覆盖 |

## 二、核心缺口（按优先级）

| 优先级 | 缺口 | 说明 |
|--------|------|------|
| P0 | 无 `deps_config.json` | check_deps 只能用预设规则跑，导致 `include/` 路径误报 |
| P0 | 检查工具缺 7 个 | 仅有 check_deps.py；缺 check_weak_pairs / check_structs / check_include / check_output_callback / check_paradigm / generate_structs |
| P1 | 无 `cfg/interface_map.h` | __weak 配对无单一真相源，check_weak_pairs 无法运行 |
| P1 | 旧版模块文件未清理 | app/ 6 个旧命名(appcommmgr 等) + drv/ 4 个旧命名(drvbuzzer 等)，include/ 20 个旧 _io.h，新旧并存 |
| P1 | Switcher 层违规 | data_switcher.c 直接 include hal/drv 原始头文件（应只 include _io.h） |
| P2 | 无 `cfg/structs.json` | 跨模块 struct 未纳入生成式管理 |
| P2 | 无 `docs/data-contract.md` | I/O 对齐无事先设计文档 |
| P2 | 2 个孤立 __weak | app_hmi.c:62/64 Drv_Display_SetRawLEDsCallback / Drv_Display_ShowRawSMGCallback 无强符号实现 |

## 三、check_deps 基线（2026-06-20）

扫描 67 个文件，报告 29 项违规：app/ 6 + drv/ 5 + proto/ 1 + core/ 15 + weak 2。
其中 app/drv 的 `include/*_io.h` 引用多为误报（缺 deps_config.json 所致），core/15 与 weak/2 为真实问题。

## 四、Codex 职责

- 角色：项目进展审查者（technical reviewer）。
- 每轮审查产出：缺口清单 + 优先级 + check 工具结果，对比本基线判断净进展。
- 不在审查中自行重构；改动需用户确认后按方法论 SOP 推进。
