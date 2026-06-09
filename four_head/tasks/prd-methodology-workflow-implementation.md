# PRD: 方法论工作流固化 — 从文档到 AI 自动执行

## Introduction

将 `methodology-seed/` 中已完整定义的零耦合方法论，从"AI 需要阅读的参考文档"升级为"AI 不可绕过的自动约束系统"。核心是将方法论规则与 Claude Code 的 CLAUDE.md、memory、SKILL 机制绑定，使 AI 在每次编码时自动执行检查、在新增模块时自动遵循 SOP、在提交前自动被工具阻断。

**目标项目**: `m4_ekf_observer/`（半桥 EKF 观测器，RX32G410，活跃开发中，已有大量现有模块需要渐进导入）。`four_head/` 已完成方法论落地，作为参考模板。

## Goals

- 让 AI 在 **每次编码后** 自动执行四件套检查（`check_deps.py` + `check_weak_pairs.py` + `check_structs.py` + 编译），无需人工提醒
- 让 AI 在 **新增模块时** 自动按方法论 7 步 SOP 创建文件骨架、注册 `interface_map.h`、编辑 `structs.json`
- 覆盖 m4_ekf_observer 项目路径（ARMCC V5 → GCC/ARMCC for RX32G410，调整工具链适配）
- 提供 **增量导入方案**：新模块全部按方法论，原有模块逐步过渡
- 输出物：更新 `m4_ekf_observer/CLAUDE.md` + 项目级 `.claude/skills/` + `tools/` + `cfg/structs.json`

## User Stories

### US-001: m4_ekf_observer 项目初始化 — 从 methodology-seed 复制工具链
**Description:** As a developer, I need the methodology toolchain (check_deps.py, check_weak_pairs.py, generate_structs.py, check_structs.py) working in m4_ekf_observer with correct layer rules for that project's architecture.

**Acceptance Criteria:**
- [ ] 从 `methodology-seed/tools/` 复制全部工具到 `m4_ekf_observer/tools/`
- [ ] 根据 m4_ekf_observer 的层结构（app/drv/hal/proto/core）配置 `check_deps.py` 的白名单
- [ ] 从 `methodology-seed/templates/` 复制 `structs.json` 模板到 `m4_ekf_observer/cfg/structs.json`
- [ ] 运行 `python tools/check_deps.py --self-test` 通过
- [ ] 运行 `python tools/generate_structs.py --check` 通过
- [ ] 工具链适配 m4_ekf_observer 编译器（GCC/ARMCC for RX32G410）

### US-002: m4_ekf_observer 项目 CLAUDE.md — 全面 AI 行为约束
**Description:** As a developer, I need a comprehensive CLAUDE.md that binds AI to the methodology at every step — not just listing rules, but giving clear "when you do X, you MUST run Y" instructions that leave no room for interpretation.

**Acceptance Criteria:**
- [ ] 编写 `m4_ekf_observer/CLAUDE.md`，包含：
  - 分层规则 + 白名单（AI 编辑前先确认层级）
  - 每次编码后强制四件套（明确 Bash 命令 + 不通过不提交）
  - 新增模块 7 步 SOP（逐条可执行指令）
  - 新增 __weak 通道的步骤清单
  - JSON 规则变更流程（如项目涉及 HMI 逻辑层）
- [ ] CLAUDE.md 风格：命令式、短句、可执行，不写"应该"、"建议"
- [ ] 与 `methodology-seed/` 的规则保持一致性
- [ ] 补充 m4_ekf_observer 项目特有的约束（EKF 参数整定流程、MODBUS 协议层规则等）

### US-003: pre-commit hook — 提交前自动阻断
**Description:** As a developer, I need a pre-commit hook that automatically runs all four checks and blocks the commit if any fail, so that no violation can enter the repository.

**Acceptance Criteria:**
- [ ] 创建 `.git/hooks/pre-commit`（或项目级 `tools/pre-commit` + 安装脚本）
- [ ] Hook 依序执行：`check_deps.py` → `check_weak_pairs.py` → `check_structs.py` → 编译检查
- [ ] 任意一步非零退出 → 阻断提交，打印明确错误信息
- [ ] Hook 可被 `--no-verify` 跳过但会在输出中警告
- [ ] 提供 Windows 兼容的 `.cmd` / `.ps1` 版本（m4_ekf_observer 开发环境）

### US-004: SKILL: `/check` — 手动四件套检查
**Description:** As a developer, I want a `/check` SKILL that runs all four checks in one invocation, showing clear pass/fail output with violation locations.

**Acceptance Criteria:**
- [ ] 创建 `.claude/skills/check/SKILL.md`
- [ ] `/check` 调用后依次执行四件套
- [ ] 输出格式：每步 [PASS] / [FAIL] + 失败时显示具体文件和行号
- [ ] `user-invocable: true`，`description` 含触发词
- [ ] 安装到 `m4_ekf_observer/.claude/skills/`（项目级）

### US-005: SKILL: `/new-module` — 交互式新模块向导
**Description:** As a developer, I want a `/new-module` SKILL that interactively guides me through creating a new module following the methodology SOP: determine layer → create .h/.c → register interface_map.h → edit structs.json → run checks.

**Acceptance Criteria:**
- [ ] 创建 `.claude/skills/new-module/SKILL.md`
- [ ] 向导步骤：1) 问模块名和层级 2) 创建 .h（注释 `#define` include guard） 3) 创建 .c（含 __weak 空壳模板） 4) 注册到 `interface_map.h` 5) 如有跨模块结构体，编辑 `cfg/structs.json` 6) 运行 `generate_structs.py` 7) 运行四件套
- [ ] 每步完成后确认才进入下一步
- [ ] 自动推断模块所属层级的白名单约束
- [ ] `user-invocable: true`

### US-006: 渐进迁移方案 — m4_ekf_observer 现有模块适配
**Description:** As a developer, I need a clear plan for gradually migrating existing m4_ekf_observer modules to the methodology, starting with new modules and incrementally retrofitting old ones.

**Acceptance Criteria:**
- [ ] `m4_ekf_observer/docs/migration-plan.md` 包含：
  - 当前模块清单 + 层级分类（哪些在 app/drv/hal/proto）
  - 迁移优先级：先新模块，再边界模块（与多个模块交互的），最后内部模块
  - 每个模块的迁移检查清单（include 违规 / __weak 替换 / structs.json 注册）
  - 迁移期间的双轨策略（新 __weak 通道与旧消息队列通道并存）
- [ ] 明确"不迁移"的模块边界（如 vendor/ 第三方代码不要求重构）
- [ ] 首轮迁移目标：当前正在开发的新功能模块

### US-007: 自动化触发 — CLAUDE.md 中嵌入提交前自动检查提示
**Description:** As a developer, I want the AI to automatically remind/execute checks before any git commit, without me having to type `/check`.

**Acceptance Criteria:**
- [ ] `CLAUDE.md` 中包含明确的"提交前必做"指令段，AI 读后会主动执行
- [ ] 指令为命令式：例如 "Before creating any commit, you MUST run `python tools/check_deps.py ../src` and confirm 0 violations."
- [ ] 在 memory 中记录"AI 每次提交前自动跑四件套"的行为模式
- [ ] 验证方式：在 m4_ekf_observer 中做一次测试提交，确认 AI 自动执行了检查

## Functional Requirements

- FR-1: `m4_ekf_observer/tools/` 包含全部四个检查脚本，且通过自检
- FR-2: `m4_ekf_observer/CLAUDE.md` 包含分层规则、四件套命令、新增模块 SOP
- FR-3: `m4_ekf_observer/.claude/skills/` 含 `/check` 和 `/new-module` 两个 SKILL
- FR-4: `m4_ekf_observer/cfg/structs.json` 初始化完成
- FR-5: `m4_ekf_observer/docs/migration-plan.md` 包含迁移路径
- FR-6: pre-commit hook 在 Windows 环境可运行
- FR-7: 工具脚本的 shebang/路径适配 Windows（`python` 而非 `python3`，路径用 `../src` 相对路径）
- FR-8: 所有新增文件不破坏 m4_ekf_observer 现有编译

## Non-Goals

- 不修改 four_head/ 现有代码和 CLAUDE.md（已完成方法论落地，作为参考）
- 不修改 methodology-seed/ 内容（种子是源，项目是实例）
- 不要求 m4_ekf_observer 所有现有模块立即重构
- 不编写新的方法论文档（现有 methodology-seed/ 已覆盖）
- 不实现 Ralph 自动化循环（那是后续 PRD 的事，本 PRD 专注方法论绑定）

## Technical Considerations

- **Windows 兼容**: m4_ekf_observer 开发环境在 Windows，pre-commit hook 需要 `.ps1` 或 `.cmd` 格式
- **编译器差异**: RX32G410 用 ARMCC V5 或 GCC，需确认 `check_deps.py` 的编译验证步骤适配
- **层结构差异**: m4_ekf_observer 的层命名可能与 four_head 不同，需先审计现有结构
- **CLAUDE.md 长度**: 需要在"全面"和"AI 每轮都会加载"之间平衡——过长的 CLAUDE.md 消耗 context
- **memory vs SKILL vs CLAUDE.md**: 三者分工：
  - CLAUDE.md = 项目级规则（每次会话自动加载）
  - SKILL = 用户手动触发的交互式向导
  - memory = AI 行为模式记录（跨会话持久化）

## Success Metrics

- 在 m4_ekf_observer 中新增一个模块时，AI 自动执行四件套检查（无需用户提醒）
- `/new-module` 能在 3 轮对话内引导完成一个新模块的创建 + 注册
- pre-commit hook 成功阻断过一次违规提交（验证阻断有效）
- 渐进迁移的首个模块（当前开发中的新功能）按方法论完成

## Open Questions

- m4_ekf_observer 的编译器是 ARMCC V5 还是 GCC？工具链路径是否需要从 four_head 的 armcc 切换？
- m4_ekf_observer 现有的层命名是什么？直接用 app/drv/hal 还是有不同的叫法？
- 是否需要为 RX32G410（Cortex-M4）适配 `check_deps.py` 中的 `--self-test` 编译命令？
- m4_ekf_observer 是否需要 JSON 驱动层（像 four_head 的 HMI 那样），还是主要是纯算法/EKF 逻辑？
