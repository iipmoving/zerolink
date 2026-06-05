# ONBOARDING.md — 新 AI 会话启动指南

> **用途**: 全新 AI 会话的第一个入口。读完本文件 + 引用的 7 篇方法论文档 + 模板文件，应能独立启动一个符合本架构的新项目。

---

## 快速上手：三条 SKILL

日常开发不需要从头读文档。用以下三条 SKILL 即可覆盖完整周期：

| 阶段 | SKILL | 做什么 |
|------|-------|--------|
| 立项 | `/init-project` | 交互式创建新项目骨架 |
| 编码 | `/new-module` | 6 步向导创建方法论合规模块 |
| 验证 | `/check` | 一键四件套审计 |

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

## 阅读顺序（总计 ~35 分钟）

| 顺序 | 文件 | 用时 | 回答什么问题 |
|------|------|------|-------------|
| 1 | `01-architecture.md` | 8 min | 项目怎么分层？铁律是什么？ |
| 2 | `02-weak-callback.md` | 6 min | 模块间怎么通信？ |
| 3 | `03-dual-engine.md` | 5 min | 怎么保证 C 和 JS 实现一致？ |
| 4 | `04-golden-output.md` | 5 min | 从单体拆模块怎么不出错？ |
| 5 | `05-interface-management.md` | 3 min | interface_map.h 怎么维护？ |
| 6 | `06-json-driven.md` | 3 min | JSON 规则怎么驱动 C/JS 双引擎？ |
| 7 | `07-struct-generation.md` | 5 min | 跨模块结构体怎么一致性管理？ |

读完方法论后，阅读模板文件了解具体格式：
- `templates/CLAUDE.md` — 项目指令模板
- `templates/interface_map.h` — __weak 配对表模板
- `templates/RESTART.md` — 项目重启入口模板（交付时生成）

---

## 新项目启动 SOP（九阶段）

### 阶段 0: 环境就绪
1. 安装工具链（emcc / Node.js / armcc）
2. 初始化 git
3. 从 `templates/` 复制并填写：
   - `CLAUDE.md` — 项目根目录，每次会话自动加载
   - `tech-stack.md` — 技术栈
   - `ubiquitous-language.md` — DDD 通用语言
   - `api-conventions.md` — 接口约定
   - `code-style.md` — 代码规范
4. 复制 `templates/structs.json` 到 `cfg/structs.json` — 跨模块结构体数据源
5. 配置 `tools/check_deps.py` 的层规则（如层名不同则修改）
6. 运行 `python tools/check_deps.py --self-test`
7. 运行 `python tools/generate_structs.py --check` — 验证结构体生成工具链

### 阶段 1: JSON 规则 + JS 引擎
1. 定义 JSON Schema（global/zone/process/actions/elements/patterns）
2. 编写 JS 参考引擎
3. 编写 JS 独立测试（目标 30+ 条）
4. **门禁**: JS 测试 100% PASS

### 阶段 2: JSON→C 数据生成
1. 编写/适配 `v4json_to_c.py`
2. 生成 C 配置数据
3. **门禁**: C 编译 0e0w，sizeof 检查

### 阶段 2.5: 结构体注册 + 生成
1. 编辑 `cfg/structs.json` — 定义所有跨模块结构体（参照 `07-struct-generation.md`）
2. 运行 `python tools/generate_structs.py` — 生成各模块 `types.h`
3. 运行 `python tools/check_structs.py` — 验证一致性
4. **门禁**: `check_structs.py` PASS + `types.h` 已生成

### 阶段 3: 单体 C 引擎 + 黄金输出录制
1. 在单一 `app_hmi.c` 中实现完整状态机
2. 插入录制桩
3. 跑全量测试序列 → 生成 `golden_output.json`
4. **门禁**: 双引擎对比 100% PASS

### 阶段 4-7: 模块分解
1. 按 `04-golden-output.md` 三阶段流程拆分
2. 拆分顺序: state_module → timer_module → display_module → key_module
3. 每完成一个模块: 影子 harness PASS + 双引擎 PASS
4. **门禁**: 每模块独立编译通过

### 阶段 8: 接线 + 全量验证
1. `main.c` 接线层（engine_init/tick/key）
2. 浏览器测试 HTML
3. `check_deps.py` → 0 violations
4. `check_weak_pairs.py` → 0 violations
5. **门禁**: 双引擎全量 100% PASS

### 阶段 9: 文档归档
1. 验收报告（AR）
2. 结项报告（PHASE-CLOSURE）
3. 资料清单（PROJECT-INVENTORY）
4. Git 里程碑提交

---

## 每次编码后的强制检查

```bash
python tools/check_deps.py         # 层依赖审计 → 必须 0 violations
python tools/check_weak_pairs.py   # __weak 配对一致性 → 必须 0 violations
python tools/check_structs.py      # 结构体一致性 → 必须 PASS
armcc -c ... → 0 error, 0 warning  # 独立编译 (含 L0 头文件私有化验证)
node test_wasm_basic.js             # 双引擎对比 → 必须 100% PASS (如有 HMI 变更)
```

---

## 核心铁律（刻在 DNA 里）

1. **解耦第一** — 层间隔离绝对不可妥协。违反即停止。
2. **头文件私有化** — DRV/APP/PROTO 的 .h 必须注释 `#define` include guard，编译器 L0 阻断外部引用。
3. **JSON 覆盖逻辑** — JSON 是唯一权威数据源。JS 先行，C 跟随。
4. **统一出入口** — 回调不散落。输入输出集中管理。
5. **独立声明 — 生成器管一致性** — 跨模块类型每模块独立声明，由 `generate_structs.py` 从 `structs.json` 生成，`check_structs.py` 验证。
6. **同步 __weak + 异步 Msg_Post** — 同时间片用 __weak 直调，跨时间片用 Msg_Post 消息。不互相替代。
7. **方法论文档** — 每次发现问题先更新方法论，再修代码。
