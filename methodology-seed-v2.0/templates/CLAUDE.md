# CLAUDE.md — 项目指令模板

> 用法: 复制到新项目根目录，填入实际值。所有 `{占位符}` 替换为项目具体信息。

---

## 基础约定

- **规划文件存放于项目根目录**，文件名用英文 slug 格式。
- **方法论/记忆文件必须双写**: 自动记忆目录 + 项目目录 `memory/`。
- **里程碑节点必须 Git 提交**: 架构变更、方法论更新、重大功能完成等里程碑必须做 git commit。

---

## {项目名称}

{一句话描述项目用途}

## 核心架构原则

**业务模块之间零交叉 include，__weak 回调是唯一桥梁。**
- HAL 层不 include 任何业务头文件
- DRV 层只 include HAL 接口 + 定义 __weak 给 APP
- ISR 只设时基标志，不在中断中做业务处理

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

**每次编码前**: 确认你编辑的文件在哪个层 → 查依赖规则 → 违规即停止。
**每次编码后**: 运行 `python tools/check_deps.py` → 不通过即修复。

### 铁律二：JSON 覆盖逻辑
**JSON 是规则权威数据源。JS 先行验证，C 跟随实现。**

```
Zone Logic (孤岛函数) ← 纯计算，不依赖外部状态
    ↓
JSON Binding (路由+动作) ← rules.json 唯一数据源
    ↓
Presentation Controls ← SlotElement/LEDElement/BlinkRule/ModeRule
```

**每次修改 HMI 逻辑**: JSON 声明 → JS 测试通过 → C 转换 → WASM 双引擎对比 → 全部 PASS。
**禁止**: 绕过 JSON 直接在 C 里硬编码规则。JS 测试未通过之前不写 C。

### 铁律三：统一出入口
**回调不能随意插入。输入输出集中管理，统一处理。**

- 回调注册入口唯一：模块初始化时统一注册，不散落在业务代码各处
- 数据流方向明确：APP 输出抽象值 → 装配器转换 → DRV 接收 → HAL 执行
- 不新增零散回调路径：新增前先确认能否用现有通道

---

## 技术栈
见 `.claude/specs/tech-stack.md`

## 代码规范
见 `.claude/specs/code-style.md`

## 通用语言
见 `.claude/specs/ubiquitous-language.md`

## 接口约定
见 `.claude/specs/api-conventions.md`

## 架构决策记录
所有决策见：`.claude/specs/adr/`

---

## 硬件配置

<!-- 按项目填写: MCU型号/主频/外设/引脚/中断等 -->

---

## 调度模式

<!-- 按项目填写: 时基/槽数/调用方式 -->

---

## 编译环境

<!-- 按项目填写: 编译器/标准/编译命令 -->

---

## 提交前强制检查 (每次编码后必做)

```bash
python tools/check_deps.py           # 层依赖审计 → 0 violations
python tools/check_weak_pairs.py     # __weak 配对一致性 → 0 violations
armcc -c ... → 0 error, 0 warning    # 独立编译
node test_wasm_basic.js               # 双引擎对比 → 100% PASS (HMI变更时)
```

## AI 行为规范

### 开工前：澄清而非猜测
- 假设显性化：列出所有不确定、靠推断得出的结论
- 多解释呈现：存在多种理解时全部列出
- 遇到困惑立即停止

### 编码时：最小化与不越界
- 只写解决问题所必需的代码
- 不添加未被要求的功能
- 不为单次使用的代码创建抽象
- 不"改进"相邻代码、注释或格式
- 匹配现有代码风格

### 完成后：自检
- 所有 #include 符合所在层依赖规则
- APP→APP 通信只用 __weak 回调
- HAL 调用只存在于 DRV 层
- 新模块放在正确的层
- 跨模块结构体独立声明，interface_map.h 已更新
- HMI 规则变更: JSON 先改 → JS 测试通过 → 再写 C
