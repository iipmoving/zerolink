# ONBOARDING.md — 新 AI 会话启动指南

> **用途**: 全新 AI 会话的第一个入口。读完本文件 + 核心路径 5 篇方法论文档 + 模板文件，应能独立启动一个符合本架构的新项目。HMI/灯板项目需额外读 3 篇扩展。

---

## 快速上手：三条 SKILL

日常开发不需要从头读文档。用以下四条 SKILL 即可覆盖完整周期：

| 阶段 | SKILL | 做什么 |
|------|-------|--------|
| 立项 | `/init-project` | 交互式创建新项目骨架 |
| 编码 | `/new-module` | 6 步向导创建方法论合规模块 |
| 修改 | `/modify-module` | 7 步扫描式重构现有模块 |
| 验证 | `/check` | 一键五件套审计 |

详见 **[WORKFLOW.md](WORKFLOW.md)** — 工作流速查表。

---

## 在开始之前：你在构建一个约束系统

本方法论不是一套建议文档。它是一套**约束系统**。

你的角色：你是这套系统的**构建者和维护者**。你写的每一行代码，都在为"未来的使用者"设定边界。

**核心原则**: 违规应该被**阻断**，而不是被提醒。

| 阻断层级 | 机制 | 使用者撞上什么 |
|----------|------|---------------|
| L0 物理阻断 | 编译器报错 | 绝对无法通过编译 |
| L1 提交阻断 | pre-commit tool 非零退出 | 能编译但不能提交 |
| L2 生成一致性 | 工具从数据源生成，手动改 → 被 L1 检出 | 正确结果由工具产生 |
| L3 文档约定 | 命名规范、注释格式 | 仅在 L0-L2 无法覆盖时使用 |

**设计自检**: 每加一条规则，问自己——"如果将来一个不认识我的 AI，不读任何文档，直接改代码，他会撞上什么？"

---

## 阅读顺序

### 核心路径（所有项目，~30 分钟）

| 顺序 | 文件 | 用时 | 回答什么问题 |
|------|------|------|-------------|
| C1 | `01-architecture.md` | 8 min | 项目怎么分层？铁律是什么？ |
| C2 | `02-weak-callback.md` | 8 min | 模块间怎么通信？三种机制怎么选？ |
| C3 | `05-interface-management.md` | 3 min | interface_map.h 怎么维护？_LINK 命名约定？ |
| C4 | `07-struct-generation.md` | 5 min | 跨模块结构体怎么一致性管理？ |
| C5 | `08-data-switcher.md` | 4 min | 结构化数据怎么路由？Switcher 怎么接线？ |
| C6 | `09-std-module.md` | 6 min | **v2.1 核心**: `MODULE_SKELETON` + `MODULE_EXPORT` — 骨架宏就是范式 |
| C7 | `10-data-contract.md` | 4 min | 数据交互说明书：编码前对齐模块 I/O，指针直传 |
| C8 | `core/std_module.h` (项目内) | 3 min | 宏定义源码参考 |

### HMI/UI 扩展（灯板等 JSON 驱动项目，+16 分钟）

| 顺序 | 文件 | 用时 | 回答什么问题 |
|------|------|------|-------------|
| H1 | `03-dual-engine.md` | 5 min | 怎么保证 C 和 JS 实现一致？ |
| H2 | `04-golden-output.md` | 5 min | 从单体拆模块怎么不出错？ |
| H3 | `06-json-driven.md` | 6 min | JSON 规则怎么驱动 C/JS 双引擎？ |

> **非灯板项目只需读核心路径 5 篇。** HMI 扩展仅当项目有 JSON→JS→WASM→C 双引擎需求时阅读。

读完方法论后，阅读模板文件了解具体格式：
- `templates/CLAUDE.md` — 项目指令模板
- `templates/interface_map.h` — __weak 配对表模板
- `templates/RESTART.md` — 项目重启入口模板（交付时生成）

---

## 新项目启动 SOP

> **标注说明**: `[通用]` = 所有项目必做 / `[HMI]` = 仅 JSON 驱动 UI 项目 / `[Switcher]` = 使用数据交换机的项目

### 阶段 0: 环境就绪 `[通用]`
1. 安装工具链（armcc / gcc / Node.js）
2. 初始化 git
3. 从 `templates/` 复制并填写：
   - `CLAUDE.md` — 项目根目录，每次会话自动加载
   - `tech-stack.md` — 技术栈
   - `ubiquitous-language.md` — DDD 通用语言
   - `api-conventions.md` — 接口约定
   - `code-style.md` — 代码规范
4. 复制 `templates/structs.json` 到 `cfg/structs.json` — 跨模块结构体数据源
5. 配置 `tools/check_deps.py` 的层规则（如层名不同则修改）
6. 运行 `python ../.claude/tools/check_deps.py --self-test`
7. 运行 `python ../.claude/tools/generate_structs.py --check` — 验证结构体生成工具链

### 阶段 1: JSON 规则 + JS 引擎 `[HMI]`
1. 定义 JSON Schema（global/zone/process/actions/elements/patterns）
2. 编写 JS 参考引擎
3. 编写 JS 独立测试（目标 30+ 条）
4. **门禁**: JS 测试 100% PASS

### 阶段 2: JSON→C 数据生成 `[HMI]`
1. 编写/适配 `v4json_to_c.py`
2. 生成 C 配置数据
3. **门禁**: C 编译 0e0w，sizeof 检查

### 阶段 2.5: 结构体注册 + 生成 `[通用]`
1. 编辑 `cfg/structs.json` — 定义所有跨模块结构体（参照 `07-struct-generation.md`）
2. 运行 `python ../.claude/tools/generate_structs.py` — 生成各模块 `types.h`
3. 运行 `python ../.claude/tools/check_structs.py` — 验证一致性
4. **门禁**: `check_structs.py` PASS + `types.h` 已生成

### 阶段 2.7: 数据交互说明书 `[通用]`
在模块分解前，先创建 `docs/data-contract.md`：
1. 列出所有模块及其数据依赖
2. 对齐每条数据流的 I/O 结构体（生产者输出 ≡ 消费者输入）
3. 确保模块组内全部指针直传
4. **门禁**: 每条数据流的结构体对齐已确认，不需要字段级复制

### 阶段 3: 单体 C 引擎 + 黄金输出录制 `[HMI]`
1. 在单一 `app_hmi.c` 中实现完整状态机
2. 插入录制桩
3. 跑全量测试序列 → 生成 `golden_output.json`
4. **门禁**: 双引擎对比 100% PASS

### 阶段 4-7: 模块分解 `[通用]`
1. 按 `04-golden-output.md` 三阶段流程拆分（通用项目直接按三段式范式拆）
2. 每完成一个模块: 独立编译通过 + 功能验证
3. **门禁**: 每模块独立编译通过

### 阶段 8: 接线 + 全量验证 `[通用]`
1. `main.c` 接线层
2. 数据交换机接线 `[Switcher]`: 在 `data_switcher.c` 中编写 Output_t → Input_t 路由规则
3. `check_deps.py` → 0 violations
4. `check_weak_pairs.py` → 0 violations
5. `check_include.py` → 0 violations `[Switcher]`
6. **门禁**: `[HMI]` 双引擎全量 100% PASS / `[通用]` 全功能测试通过

### 阶段 9: 文档归档 `[通用]`
1. 验收报告（AR）
2. 结项报告（PHASE-CLOSURE）
3. 资料清单（PROJECT-INVENTORY）
4. Git 里程碑提交

---

## 每次编码后的强制检查

### 通用（所有项目）

```bash
python ../.claude/tools/check_deps.py         # 层依赖审计 → 必须 0 violations
python ../.claude/tools/check_weak_pairs.py   # __weak 配对一致性 → 必须 0 violations
python ../.claude/tools/check_structs.py      # 结构体一致性 → 必须 PASS
armcc -c ... → 0 error, 0 warning  # 独立编译 (含 L0 头文件私有化验证)
```

### 数据交换机项目附加

```bash
python ../.claude/tools/check_include.py      # _io.h 全路径 include 权限 → 必须 0 violations
```

### HMI/灯板项目附加

```bash
node test_wasm_basic.js             # 双引擎对比 → 必须 100% PASS
```

---

## 核心铁律（刻在 DNA 里）

1. **解耦第一** — 层间隔离绝对不可妥协。违反即停止。
2. **头文件私有化** — DRV/APP/PROTO 的 .h 必须注释 `#define` include guard，编译器 L0 阻断外部引用。
3. **JSON 覆盖逻辑** `[HMI]` — JSON 是唯一权威数据源。JS 先行，C 跟随。非 HMI 项目跳过。
4. **统一出入口** — 回调不散落。输入输出集中管理。
5. **独立声明 — 生成器管一致性** — 跨模块类型每模块独立声明，由 `generate_structs.py` 从 `structs.json` 生成，`check_structs.py` 验证。
6. **三种通信互补** — 同步 __weak + 异步 Msg_Post + 结构化数据交换机。按场景选择，不互相替代。
7. **方法论文档** — 每次发现问题先更新方法论，再修代码。
