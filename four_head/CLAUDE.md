# 项目共享信息

> **新 AI / 新工程师**: 先读 `RESTART.md` — 30 秒速览 → 约束系统 → 接口表 → 工作流 → 快速参考卡。
> 本文件为补充细节。RESTART.md 是唯一重启入口。

## 🎯 最高优先级：方法论是唯一产出

**本项目（四头电磁炉）的真正目的不是做出一个能用的电磁炉。**
**真正目的是建立一套 AI 零耦合嵌入式开发的、可复制的标准方法论。**

每次发现问题，流程不是"修完就走"，而是：
```
1. 根因分析    → 方法论缺了什么才让这个BUG能发生？
2. 更新方法论  → methodology_*.md 补上缺失的规则
3. 更新工具    → check_*.py 加检查项，确保不会再犯
4. 修代码      → 最后才是改代码
```

**成功标准**: 换一个全新的 AI 对话，读一遍 `memory/` 下的方法论文档，能独立搭建出相同架构的项目。

---

## ⚡ 核心铁律 (每次编程必须遵守，刻在 DNA 里)

### 铁律一：解耦第一
**层间隔离是绝对的。任何情况下不得妥协。**

```
APP 层: 不得 #include 任何 drv/ 或 hal/ 头文件
DRV 层: 不得 #include 任何 app/ 头文件
HAL 层: 零依赖，不知上层存在
APP↔APP: 只通过 __weak 回调通信，禁止直接 include 或调用
仅 DRV → HAL 合法
```

**两层阻断机制**:
| 层级 | 机制 | 阻断什么 |
|------|------|---------|
| **L0 编译器** | 头文件 `//#define GUARD_H` (include guard 故意失效) | 同一 .c 内二次包含 → 重定义报错 |
| **L1 pre-commit** | `tools/check_deps.py` | 跨层 include 语句 → 提交阻断 |

**DRV/APP/PROTO 层的每个 .h 必须注释掉 `#define`**:
```c
#ifndef MODULE_NAME_H
//#define MODULE_NAME_H   // ← 注释掉, 禁用 include guard
...
#endif
```
HAL 层不适用 — DRV 需要引用 HAL 头文件。

**每次编码前**: 确认你编辑的文件在哪个层 → 查依赖规则 → 违规即停止。
**每次编码后**: 运行 `python tools/check_deps.py ../src` → 不通过即修复。

### 铁律三：统一出入口
**回调不能随意插入。输入输出集中管理，统一处理。**

- 回调注册入口唯一：模块初始化时在装配器（如 `api_display`）统一注册，不散落在业务代码各处
- 数据流方向明确：APP 输出抽象值 → 装配器转换 → DRV 接收 → HAL 执行
- 不新增零散回调路径：每次想加回调前，先确认能否用现有的消息/回调通道

### 铁律二：JSON 覆盖逻辑
**JSON 是规则权威数据源。JS 先行验证，C 跟随实现。**

```
Zone Logic (孤岛函数) ← 纯计算，不依赖外部状态
    ↓
JSON Binding (路由+动作) ← four_head_v4.json 唯一数据源
    ↓
Presentation Controls ← SlotElement/LEDElement/BlinkRule/ModeRule
```

**每次修改 HMI 逻辑**: JSON 声明 → JS 测试通过 (34/34) → C 转换 → WASM 双引擎对比 → 全部 PASS。
**禁止**: 绕过 JSON 直接在 C 里硬编码规则。JS 测试未通过之前不写 C。

---

## 技术栈
见 @.claude/specs/tech-stack.md

## 代码规范
见 @.claude/specs/code-style.md

## 通用语言 (Ubiquitous Language)
所有沟通必须对齐术语，见 @.claude/specs/ubiquitous-language.md
出现"上电/关机/待机/Z1 Slot/COOKING"等词时以此为唯一解释。

## 接口约定
见 @.claude/specs/api-conventions.md

## 架构决策记录
所有决策见：.claude/specs/adr/

## 当前角色
- 技术负责人：@.claude/agents/tech-lead.md
- 程序员：@.claude/agents/developer.md
- 测试员：@.claude/agents/tester.md
- 文档管理员：@.claude/agents/doc-keeper.md

## 项目文档
- 架构方案：`docs/architecture/`
- 产品规格书：`docs/specs/`
- 代码审查报告：`docs/reviews/`
- 参考资料：`docs/references/`

## 外部资料路径

以下路径指向工作区外的大文件资料，不在版本管理中：

| 用途 | 路径 |
|------|------|
| 参考程序（硬件配置权威来源） | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\common\ref-programs\` |
| 芯片资料（数据手册/TRM/应用指南） | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\common\chip-docs\` |

## 编译要求

- **所有代码编写完毕后必须通过 armcc 编译**（0 error, 0 warning）
- **项目文件**: `Project/32L14Tdmoe.uvprojx`（Keil MDK V5）
- **编译器**: `C:/Keil_v5/ARM/ARMCC/bin/armcc.exe`（ARMCC V5.06 update 6 build 750）
- **编译命令**: 从 `Project/` 目录执行
  ```
  armcc -c --cpu Cortex-M0+ -DSC32L14xx --c99 \
    -I "../src/vendor/FWLib/SC32F1XXX_Lib/inc" -I "../src/vendor/CMSIS" \
    -I "../src/vendor/MCU_Drivers" \
    -I "../src/app" -I "../src" -I "../src/hal" \
    -I "../src/drv" -I "../src/proto" -I "../src/cfg" \
    <source.c> -o <output.o>
  ```

## 当前待解决问题
- HMI JSON 驱动架构 P0\~P3 阶段已交付（覆盖率 62%→90%，测试 34/34 通过）
  - 阶段报告：`docs/architecture/json-architecture-phase-report.md`
  - 原实施计划 `docs/architecture/hmi-json-message-test-plan.md` 已执行完毕，测试界面、V4.0 格式、Canvas 交互均已实现
  - 下一阶段：孤岛函数参数 JSON 化（`docs/architecture/json-architecture-phase-report.md` 第7节），待技术负责人排期
- 调度槽位 7-9 预留，待后续功能扩展


# 补充章节：AI行为规范

## 一、核心原则

以下行为规范适用于所有参与项目的AI（包括DeepSeek、Claude Code、通义灵码等）。这些规范偏向于“谨慎”而非“速度”。对于简单任务，可适当放宽。

## 二、开工前：澄清而非猜测

### 2.1 假设显性化
在分析文档中必须单设一节“【假设与推断】”，明确列出所有不确定、靠推断得出的结论。人工确认时优先核查此节。

### 2.2 多解释呈现
如果存在多种理解方式，全部列出，不自行选择。如果存在更简单的方法，主动提出。

### 2.3 遇到困惑立即停止
如果某个需求不清晰，停止执行，明确指出困惑点，等待澄清。

## 三、编码时：最小化与不越界

### 3.1 最小化代码
只写解决问题所必需的代码。不添加任何未被要求的功能。不为单次使用的代码创建抽象。不添加未被要求的“灵活性”或“可配置性”。不为不可能发生的场景添加错误处理。

### 3.2 简洁性自检
代码生成后，自查是否有可合并的相似逻辑、是否有只用一次的抽象。如有，交付前自行精简。最终代码行数不应超过必要行数的1.5倍。

### 3.3 不越界修改
只修改必须修改的代码。不“改进”相邻代码、注释或格式。不重构未出问题的部分。匹配现有代码风格，即使与个人偏好不同。如果发现无关的死代码，口头提及即可，不自行删除。

### 3.4 自清门户
当修改导致某些导入、变量、函数不再使用时，自行清理。但不要删除修改前就已存在的死代码。

## 四、执行时：验证闭环

### 4.1 定义成功标准
将模糊任务转化为可验证的目标：
- “添加验证” → “为无效输入编写测试用例，然后使其通过”
- “修复Bug” → “编写能复现Bug的测试用例，然后使其通过”
- “重构X” → “确保重构前后所有测试用例通过”

### 4.2 多步骤任务的分步验证
复杂任务必须拆分步骤，每步先声明验证标准，再动手执行。验证不通过不进入下一步。每步格式：
1. [步骤描述] → 验证：[验证方法]
2. [步骤描述] → 验证：[验证方法]

## 五、最终检验

每个被修改的代码行，都必须能直接追溯到用户的需求。如果某个修改无法解释“为什么需要它”，则不应存在。

## 六、行为规范生效的标志

以下现象说明行为规范正在发挥作用：
- 提交的代码中，不必要的修改越来越少
- 因过度复杂化而需要返工的情况越来越少
- 在实现之前提出澄清问题的次数，多于实现之后发现错误的次数

---

## 七、提交前强制检查 (每次编码后必做)

### 7.0 里程碑提交

**架构变更、方法论更新、重大功能完成等里程碑节点必须做 git commit。**
- AI 应在里程碑完成后主动提议或执行 `git commit`
- 提交信息格式: `[milestone] <简短描述>`，正文列出关键变更

### 7.1 方法论更新

**最重要**: 本次是否发现了新类型的违规/问题？
- 有 → 先更新 `memory/methodology_*.md` + `MEMORY.md`
- 无 → 确认现有方法论已覆盖本次所有变更

### 7.2 层依赖审计

```bash
python tools/check_deps.py ../src
```

**不通过 = 不得提交。** 违规类型:
- `app/*.c` 包含 `drv/` 或 `hal/` → 必须移除
- `drv/*.c` 包含 `app/` → 必须移除
- `hal/*.c` 包含 `core/` `app/` `drv/` `proto/` → 必须移除
- `proto/*.c` 包含 `app/` `drv/` `hal/` → 必须移除

### 7.3 消息通道一致性

```bash
python tools/check_msgs.py ../src
```

**不通过 = 不得提交。** 检查项:
- 每条消息通道的 Send/Register 成对存在
- 同名数值未被不同通道重复使用
- `*_MSG_*_IN` / `*_MSG_*_OUT` 命名符合约定

### 7.4 自检清单

在标记任何任务为完成前，逐条确认:
- [ ] 所有 `#include` 符合所在层的依赖规则？
- [ ] APP→APP 通信只用 Msg_Post，无直接 include 或函数调用？
- [ ] HAL 调用只存在于 DRV 层？
- [ ] 新模块放在正确的层？(APP=业务逻辑, DRV=硬件封装, proto=协议, core=基础设施)
- [ ] 消息ID使用本地 `#define`，无共享 MSG_* 枚举引用？
- [ ] 跨模块结构体独立声明，interface_map.h 已更新？
- [ ] HMI 规则变更: JSON 先改 → JS 测试通过 → 再写 C？

### 7.5 编译验证

```
armcc -c --cpu Cortex-M0+ -DSC32L14xx --c99 ... → 0 error, 0 warning
```