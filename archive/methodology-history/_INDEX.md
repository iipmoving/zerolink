# 零耦合方法论 — 版本演进史

> 本文档追溯方法论从 v0 到 v2.4 的完整演化路径。
> 历史版本归档于此，当前最新方法论在 [[methodology-seed-v2.0/ONBOARDING|methodology-seed-v2.0/]]。

---

## 版本总览

| 版本 | 日期 | 核心变更 | Git Commit |
|------|------|---------|------------|
| v0 奠基 | 5月前期 | 约束系统架构奠基 — 方法论种子 + 生成式结构体管理 | `de87c23` |
| v1.0 | 5-26 上午 | MsgScheduler 消息总线 — AI 管理消息 ID + 依赖注入 | `f8d6777` |
| v2.0 | 5-26 下午 | **__weak 回调直调** — 消除消息调度器，链接器接管 | `b792d47` |
| v2.1 | 5-26 晚间 | 黄金输出验证 — 模块分解逐字段匹配 | `98eecd0` |
| v2.2 | 6月初 | **PULL 范式** — MODULE_SKELETON + Switcher 显式路由 | `78bd7f3` |
| v2.3 | 6月中 | **LINK+PARAMS** — codeGen 自动化 + project.json 单一数据源 | `9f73c1c` |
| v2.4 | 6月下 | **OUTPUT_LINK 指针化** — 多维数组 + 指针直传 | `403a7c2` |

---

## v0 — 约束系统种子 (2026-05 前期)

**关键词**: 头文件私有化, check_deps.py, 分层架构

最早的方法论种子 — 从实践中提取的核心约束：
- L0 编译器阻断: 头文件 `#define` 注释掉 → 重复包含编译报错
- L1 提交阻断: `check_deps.py` 层依赖审计
- 生成式结构体管理: 单一数据源生成 types.h

**形式**: 方法论种子目录 + Python 工具链 (`check_deps.py`, `check_structs.py`, `check_weak_pairs.py`)

---

## v1.0 — MsgScheduler 消息总线 (2026-05-26 上午)

**关键词**: 消息队列, AI 管理命名空间, interface_map.h

最初的消息调度器架构：
- 消息 ID 由 AI 统一管理 (`msg_def.h`)
- 模块间通过 `Msg_Post()` + `MsgScheduler_Register()` 通信
- `interface_map.h` 维护 AI-MANAGED 段标记
- __weak 回调概念首次提出 (但仍是消息调度器为主)

**问题**: 消息调度器仍然是"唯一桥梁"——模块间仍需通过调度器中转。

### 归档文件

| 文件 | 说明 |
|------|------|
| [[v1.0-msg-scheduler/methodology_first-principle\|方法论第一原则]] | 最高优先级：方法论是唯一产出 |
| [[v1.0-msg-scheduler/methodology_independent-type-declarations\|独立类型声明]] | 跨模块结构体独立声明，AI 维护 |
| [[v1.0-msg-scheduler/methodology_ai-managed-dependency-injection\|AI 管理依赖注入]] | AI 管理 __weak 回调配对 |
| [[v1.0-msg-scheduler/methodology_ai-message-id-namespace\|消息 ID 命名空间]] | [已废弃] AI 管理消息 ID |

---

## v2.0 — __weak 回调直调 (2026-05-26 下午)

**关键词**: __weak, Data Switcher, 五层架构

**核心突破**: 消除消息调度器。发送方直接调 `Receiver_OnXxx(param, data)`，链接器接管一切。

架构核心：
- 五层架构 (APP/PROTO/BASE_CLASS/CORE/VENDOR)
- __weak 空壳 → 接收方强符号覆盖 → 零耦合直接调用
- Data Switcher 周期性结构化数据路由
- 模块三段式范式: 输入 → 计算 → 输出

### 归档文件

| 文件 | 说明 |
|------|------|
| [[v2.0-data-switcher/methodology_ai-zero-coupling-complete\|零耦合架构全集]] | v2.0 完整方法论 (v1.0→v2.0 演进全记录) |
| [[v2.0-data-switcher/methodology_weak-callback-zero-dependency\|__weak 零依赖回调]] | __weak 回调机制设计 |
| [[v2.0-data-switcher/audit-weak-transition-2026-05-26\|__weak 过渡审计]] | 从 MsgScheduler 过渡到 __weak 的全面审查 |

---

## v2.1 — 黄金输出验证 (2026-05-26 晚间)

**关键词**: Golden Output, 模块分解, 逐字段匹配

**触发**: display_module 模块分解导致 29 条断言失败。

解决方案：新模块不是"重新实现"而是"精确搬运"。
- 给相同输入，每个输出字段与原始代码逐字段一致
- 验证标准不是"看着合理"，而是"逐字段匹配"

### 归档文件

| 文件 | 说明 |
|------|------|
| [[v2.2-pull-paradigm/methodology_golden-output-decomposition\|黄金输出分解]] | 模块分解黄金输出验证协议 |

---

## v2.2 — PULL 范式 (2026-06 初)

**关键词**: MODULE_SKELETON, PULL 路由, four_head 统一

全集群统一到 v2.2 PULL 范式：
- `MODULE_SKELETON()` 宏定义模块骨架
- `ProcessInput` + `Init` + `MODULE_EXPORT` 标准化
- Switcher 显式路由 (producer → g_output → Switcher → consumer)
- `include_io/` 头文件隔离
- 五件套验证: check_deps + check_weak_pairs + check_structs + check_output_callback + 编译

### 归档文件

| 文件 | 说明 |
|------|------|
| [[v2.2-pull-paradigm/review-v8-json-spec-methodology\|v8 JSON 规范审查]] | JSON 架构方法论审查报告 |

---

## v2.3 — LINK+PARAMS + codeGen (2026-06 中)

**关键词**: 代码生成器, project.json, m4_ekf_observer 升级

工具化飞跃：
- `codeGen/code_gen.py` 从 `project.json` 自动生成模块接口 + data_switcher
- `LINK` 宏 + `PARAMS` 宏标准化数据流声明
- m4_ekf_observer 6 模块全面升级到 LINK+PARAMS
- JSON 作为唯一数据源 → GUI 确认 → 生成 → 编译

**当前方法论**: `methodology-seed-v2.0/` 对应 v2.3 状态。

---

## v2.4 — OUTPUT_LINK 指针化 (当前最新, 2026-06 下)

**关键词**: 指针直传, 多维数组 dims

**最新变更** (commit `403a7c2`):
- `OUTPUT_LINK` 宏改为指针传递而非值拷贝
- 多维数组通过 `dims[]` 传递维度信息
- `INPUT_CALLBACK` 单参数范式
- 更新方法论文档 + SKILL + 生成器同步

---

## 方法论核心演化逻辑

```
v1.0                        v2.0                      v2.2                   v2.3/v2.4
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ MsgScheduler    │ →  │ __weak 直调     │ →  │ PULL 范式       │ →  │ LINK+PARAMS     │
│                 │    │                 │    │                 │    │  + codeGen      │
│ 消息队列异步    │    │ 链接器同步接线  │    │ Switcher 路由   │    │ 自动生成        │
│ AI 管理消息 ID  │    │ Data Switcher   │    │ MODULE_SKELETON │    │ project.json    │
│ interface_map.h │    │ 五层架构        │    │ include_io/     │    │ 指针化 OUTPUT   │
└─────────────────┘    └─────────────────┘    └─────────────────┘    └─────────────────┘
       │                       │                      │                       │
       │ 发现问题:              │ 发现: 拆分模块时     │ 发现问题:              │
       │ 消息调度仍是耦合桥     │ AI倾向"重实现"       │ 手动写接口不一致      │
       │                       │ 而非"搬运"          │                       │
```

**核心趋势**: 耦合度持续降低，自动化持续提升，人为失误空间持续压缩。

---

## 当前最新方法论

当前活动方法论位于 [[methodology-seed-v2.0/ONBOARDING|methodology-seed-v2.0/]]，包含 10 篇核心文档 + 模板 + 工具链。

---

*索引生成于 2026-06-24 · 基于 git 历史 + 文档内容分析*
