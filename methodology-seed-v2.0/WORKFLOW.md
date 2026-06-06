# 零耦合方法论 · 工作流速查

> 四条 SKILL 覆盖完整开发周期：立项 → 编码 → 修改 → 验证。

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

## 三、修改：重构现有模块

```
/modify-module
```

7 步扫描式修改向导：

| 步骤 | 做什么 | 产出 |
|------|--------|------|
| 1 | 确定目标模块 + 所在层 | 模块全路径 |
| 2 | 全量扫描（.h/.c/interface_map/structs.json/include 引用） | 依赖图谱 + 违规清单 |
| 3 | 执行路径分析（入口→数据流→include→三段式→状态字） | 路径分类 + 违规定位 |
| 4 | 影响分析（谁依赖我 / 我依赖谁 / 变更半径） | 影响范围评估 |
| 5 | 生成修改方案 | 原子步骤 + 验证命令 + 回滚检查点 |
| 6 | 逐步执行 + 逐步验证 | 每步通过才进入下一步 |
| 7 | 全量审计 `/check` | 五件套全 PASS |

---

## 四、验证：每次编码后

```
/check
```

| # | 检查项 | 命令 | 适用 |
|---|--------|------|------|
| 1 | 层依赖审计 | `check_deps.py` | 通用 |
| 2 | __weak 配对 | `check_weak_pairs.py` | 通用 |
| 3 | 结构体一致性 | `check_structs.py` | 通用 |
| 4 | 数据交换机 include 权限 | `check_include.py` | Switcher 项目 |
| 5 | 编译 | armcc / gcc | 通用 |

快速模式（跳过编译）：`/check --quick`

---

## 五、提交

```
git add <files>
git commit -m "..."
```

pre-commit hook 自动跑前三件套，不通过则阻断提交。

### 5.1 提交前自检

```
/check                           # 跑全部检查
/check --quick                   # 跳过编译，只跑 1-4
```

全部 PASS 后再 `git commit`。不要在 hook 失败后 `--no-verify`。

---

## 六、Git 安全操作

### 6.1 回退前：永远先保存现场

**任何 `git reset`、`git checkout --`、`git restore` 之前，先 stash 或建备份分支。** 这些操作不可逆，丢失的代码无法恢复。

```
# 方案 A: stash（推荐 — 轻量）
git stash push -m "WIP: before revert — $(date +%Y-%m-%d)" --include-untracked
git stash list                    # 确认 stash 存在
# ... 回退操作 ...
# git stash pop                   # 需要时恢复

# 方案 B: 备份分支（安全 — 永久保留）
git checkout -b backup/before-revert-$(date +%Y%m%d)
git checkout -                     # 切回原分支
# ... 回退操作 ...
```

**铁律**: `git reset --hard` 前必须确认 `git stash list` 或备份分支已创建。禁止裸 `reset --hard` 不检查现场。

### 6.2 回退操作速查

| 场景 | 命令 | 前提 |
|------|------|------|
| 撤销未暂存的修改 | `git stash` 或 `git checkout -- <file>` | 确认修改不需要保留 |
| 撤销已暂存的修改 | `git reset HEAD <file>` → 再 stash | 先 unstage |
| 撤销最近一次 commit（保留修改） | `git reset --soft HEAD~1` | 修改回到工作区 |
| 撤销最近一次 commit（丢弃修改） | `git stash` → `git reset --hard HEAD~1` | **必须先 stash** |
| 回到某个历史 commit 查看 | `git checkout <hash>` (detached HEAD) | 看完 `git switch -` 回来 |
| 永久回到历史 commit | `git stash` → 建备份分支 → `git reset --hard <hash>` | **双重保险** |

### 6.3 禁止操作

- `git push --force` 到 main/master — 永远禁止
- `git reset --hard` 不先 stash — 永远禁止
- `git clean -fd` 不先 `git stash --include-untracked` — 永远禁止

---

## 完整示例：修改功率模块输出接口

```
/modify-module
  > 模块: app_power
  > 扫描中...
    [VIOLATION] app/power.c:3: includes forbidden "drv/adc.h"
    [INFO] 3 __weak pairs registered in interface_map.h
    [INFO] struct "PowerOut" owner=app_power, consumed by [display, comm]
  > 修改目标: 将 adc.h 直接引用改为 __weak 回调
  > 影响半径: Moderate (interface_map.h + power.c 变更)
  > [生成方案]: 5 个原子步骤
  > 逐步执行...
    Step 1: 移除 #include "drv/adc.h" → check_deps PASS
    Step 2: 添加 __weak void Adc_OnData(...) → 编译 PASS
    Step 3: 更新 interface_map.h → check_weak_pairs PASS
    Step 4: 重构 DoWork 输入段 → 编译 PASS
    Step 5: /check 全量 → ALL PASS
  > 提交
```

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

| 文件 | 用途 | 适用范围 |
|------|------|---------|
| `01-architecture.md` | 分层规则、三段式范式、回调分级 | 通用 |
| `02-weak-callback.md` | __weak + Msg_Post + 数据交换机 三种通信 | 通用 |
| `05-interface-management.md` | interface_map.h 维护 + _LINK 命名约定 | 通用 |
| `07-struct-generation.md` | structs.json → types.h 生成流程 | 通用 |
| `08-data-switcher.md` | 数据交换机：GetIO/DoWork/状态字协议 | Switcher 项目 |
| `09-std-module.md` | std_module.h：MODULE_SKELETON + MODULE_EXPORT + Para_Grp_t | **v2.1 核心** |
| `03-dual-engine.md` | JS/C 双引擎同源检测 | HMI 项目 |
| `04-golden-output.md` | 黄金输出录制 + 模块分解验证 | HMI 项目 |
| `06-json-driven.md` | JSON 三层驱动架构 | HMI 项目 |
| `ONBOARDING.md` | 新人 30 秒速览入口 | 通用 |
| `WORKFLOW.md` | 本文档 | 通用 |

## 工具索引

| 工具 | 用途 | 适用范围 |
|------|------|---------|
| `check_deps.py` | 层依赖规则检查 | 通用 |
| `check_weak_pairs.py` | __weak 配对一致性 | 通用 |
| `check_structs.py` | 结构体一致性验证 | 通用 |
| `check_include.py` | 数据交换机 _io.h include 权限 | Switcher 项目 |
| `generate_structs.py` | structs.json → types.h | 通用 |
