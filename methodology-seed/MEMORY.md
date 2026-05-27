# 方法论种子 — 索引

> 本目录是 AI 零耦合嵌入式开发方法论的**纯规则提取版**。
> 剥离了所有项目特定内容，可直接复制为新项目的种子。

---

## 快速启动

**新 AI 会话第一入口**: `ONBOARDING.md`

## 方法论核心 (按阅读顺序)

- [ONBOARDING.md](ONBOARDING.md) — 新 AI 会话启动指南, 九阶段 SOP
- [01-architecture.md](01-architecture.md) — 五层架构 + 六大铁律
- [02-weak-callback.md](02-weak-callback.md) — __weak 回调零依赖通信模式
- [03-dual-engine.md](03-dual-engine.md) — 双引擎同源检测体系
- [04-golden-output.md](04-golden-output.md) — 模块分解三阶段验证
- [05-interface-management.md](05-interface-management.md) — interface_map.h + AI 配对管理 + _IN/_OUT 命名
- [06-json-driven.md](06-json-driven.md) — JSON→JS→C 三层驱动
- [07-struct-generation.md](07-struct-generation.md) — 生成式结构体管理 (JSON→types.h)

## 自动化工具

- [check_deps.py](tools/check_deps.py) — 层依赖审计 (禁止跨层 include)
- [check_weak_pairs.py](tools/check_weak_pairs.py) — __weak 配对一致性检查
- [generate_structs.py](tools/generate_structs.py) — 从 structs.json 生成模块 types.h
- [check_structs.py](tools/check_structs.py) — 结构体一致性验证 (pre-commit hook)

## 项目模板

- [CLAUDE.md](templates/CLAUDE.md) — 项目指令模板 (每次会话自动加载)
- [structs.json](templates/structs.json) — 跨模块结构体定义模板 (单一数据源)
- [tech-stack.md](templates/tech-stack.md) — 技术栈声明模板
- [ubiquitous-language.md](templates/ubiquitous-language.md) — DDD 通用语言模板
- [api-conventions.md](templates/api-conventions.md) — 接口约定模板
- [code-style.md](templates/code-style.md) — 代码规范模板
- [interface_map.h](templates/interface_map.h) — __weak 配对映射表模板

---

## 新项目启动检查清单

- [ ] 复制 `templates/` 到新项目的对应位置
- [ ] 填写技术栈、通用语言、代码规范
- [ ] 配置 `check_deps.py` 的层规则
- [ ] 初始化 `cfg/structs.json` (从模板复制)
- [ ] 创建空 `interface_map.h`
- [ ] 初始化 git + `.gitignore`
- [ ] 阶段 0: 运行 `check_deps.py --self-test` + `generate_structs.py --check` 验证工具链
- [ ] 按 `ONBOARDING.md` 九阶段执行

---

*生成日期: 2026-05-27*
*来源: 四头电磁炉低耦合控制程序 — 方法论提取*
