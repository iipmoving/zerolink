# 零耦合方法论 · 工作流速查

> 三条 SKILL 覆盖完整开发周期：立项 → 编码 → 验证。

---

## 一、立项：创建新项目

```
/init-project
```

回答 5 个问题后自动生成完整项目骨架。

| 问题 | 示例 |
|------|------|
| 项目名 | `motor_controller` |
| 位置 | 回车 = 集群根目录 |
| MCU | RX32G410 / SC32L14T / 自定义 |
| 编译器 | ARMCC / GCC |
| 层目录 | 回车 = 默认，或用 `base_class` 替代 `drv` |

---

## 二、编码：新增模块

```
/new-module
```

6 步交互式向导：

| 步骤 | 做什么 | 产出 |
|------|--------|------|
| 1 | 确定模块名 + 所属层 | 确认层白名单 |
| 2 | 生成 `.h` | include guard 注释 + 公有接口 |
| 3 | 生成 `.c` | 三段式骨架（输入→计算→输出）+ __weak 空壳 |
| 4 | 跨模块结构体 | 编辑 `cfg/structs.json` → 运行 `generate_structs.py` |
| 5 | 注册 __weak 配对 | 更新 `core/interface_map.h` |
| 6 | 验证 | `/check` 四件套全 PASS |

每步确认后进入下一步。

---

## 三、验证：每次编码后

```
/check
```

| # | 检查项 | 命令 |
|---|--------|------|
| 1 | 层依赖审计 | `check_deps.py` |
| 2 | __weak 配对 | `check_weak_pairs.py` |
| 3 | 结构体一致性 | `check_structs.py` |
| 4 | 编译 | armcc / gcc |

快速模式（跳过编译）：`/check --quick`

---

## 四、提交

```
git add <files>
git commit -m "..."
```

pre-commit hook 自动跑前三件套，不通过则阻断提交。

---

## 完整示例：新建一个锅具检测模块

```
/new-module
  > 模块名: pot_detect
  > 层: app
  > 确认层规则...
  > [生成 app/pot_detect.h + pot_detect.c]
  > 有跨模块结构体? → 是 → 编辑 cfg/structs.json
  > 有 __weak 配对? → 是 → 更新 core/interface_map.h
  > /check → 4/4 PASS

git add app/pot_detect/ cfg/ core/
git commit -m "feat: add pot_detect module"
```

---

## 方法论文件索引

| 文件 | 用途 |
|------|------|
| `01-architecture.md` | 分层规则、三段式范式、回调分级 |
| `02-weak-callback.md` | __weak 与 Msg_Post 互补通信 |
| `07-struct-generation.md` | structs.json → types.h 生成流程 |
| `ONBOARDING.md` | 新人 30 秒速览入口 |
| `WORKFLOW.md` | 本文档 |

## 工具索引

| 工具 | 用途 |
|------|------|
| `check_deps.py` | 层依赖规则检查 |
| `check_weak_pairs.py` | __weak 配对一致性 |
| `generate_structs.py` | structs.json → types.h |
| `check_structs.py` | 结构体一致性验证 |
