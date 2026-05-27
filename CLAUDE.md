# CLAUDE.md — 低耦合程序架构 · 项目集群总控

> **定位**: 本仓库是一个**多项目集群**。本文件负责集群级的导航、协调和公共约束。
> 各子项目有自己的 `CLAUDE.md`，描述具体的项目目标、技术栈和开发规则。

---

## 一、角色定位

我们是 **AI 零耦合嵌入式开发方法论的共同缔造者**，不只是项目执行者。

核心原则：**违规应该被阻断，而不是被提醒。**

| 阻断层级 | 机制 | 说明 |
|----------|------|------|
| L0 物理阻断 | 编译器报错 | 绝对无法通过编译 |
| L1 提交阻断 | pre-commit tool 非零退出 | 能编译但不能提交 |
| L2 生成一致性 | 工具从单一数据源生成 | 正确结果由工具产生 |
| L3 文档约定 | 命名规范、注释格式 | 最后手段，需定期审视能否升级 |

**设计自检**: "如果将来一个不认识我们的 AI，不读任何文档，直接改代码——他会撞上什么？"

---

## 二、项目集群清单

```
低耦合程序架构/                    # 项目集群根
│
├── methodology-seed/             # ★ 方法论种子 — 新项目从这里复制
│
├── four_head/                    # 项目1: 四头灯板零依赖系统 (SC32L14T)
│   ├── src/                      #   C 源码 (app/drv/hal/proto/core/api/cfg/vendor)
│   ├── Project/                  #   KEIL MDK 项目
│   ├── sim/                      #   WASM/JS 仿真 (桌面模拟器 + 测试)
│   ├── test/                     #   单元测试
│   ├── docs/                     #   项目文档
│   ├── tools/                    #   项目工具
│   └── CLAUDE.md                 #   项目级指令
│
├── m4_ekf_observer/              # 项目2: 半桥 EKF 观测器 (RX32G410)
│   ├── src/                      #   MCU 固件库
│   ├── app/                      #   应用层 (EKF/锅检测/变增益PID)
│   ├── tools/                    #   EKF 整定 + MODBUS 测试工具
│   └── CLAUDE.md                 #   项目级指令 (待创建)
│
├── common/                       # 公共资源
│   ├── ref-programs/             #   参考程序 (硬件配置权威来源)
│   ├── chip-docs/                #   芯片资料 (数据手册/TRM/应用指南)
│   └── docs/                     #   跨项目公共文档
│
└── archive/                      # 历史归档
    ├── four_head_ih_cooker_v0/   #   最早原型 (KEIL v4)
    └── four_head_ih_cooker_v1/   #   中间版本 (KEIL v5)
```

### 项目定位

| 项目 | MCU | 用途 | 状态 |
|------|-----|------|------|
| `four_head/` | SC32L14T (M0+) | 四头电磁炉灯板控制 — 零依赖解耦架构 | 活跃开发 |
| `m4_ekf_observer/` | RX32G410 (M4) | 半桥/全桥加热控制 — EKF 观测器 + PID | 维护中 |

### 仿真环境

| 位置 | 说明 |
|------|------|
| `four_head/sim/test_hmi/` | **灯板桌面模拟器** — Canvas 面板 + JSON 规则引擎 + 自动化测试 |
| `four_head/sim/phase1-js-logic/` | 面板编辑器 — 拖拽布局 + JS/WASM 双逻辑层 |
| `four_head/sim/phase2-wasm-c-source/` | C 状态机 → WASM 编译源码 |

---

## 三、方法论：一切从这里开始

**`methodology-seed/`** 是本集群的核心资产。它是一套可复制的约束系统——复制到任何新项目，就能建立同样的零依赖架构。

**入口**: `methodology-seed/ONBOARDING.md`
**阅读顺序**: 01-architecture → 02-weak-callback → 03-dual-engine → 04-golden-output → 05-interface-management → 06-json-driven → 07-struct-generation

---

## 四、集群级规则

### 4.1 方法论优先

任何项目中发现新问题 → 先分析是否是方法论漏洞 → 更新 `methodology-seed/` → 再修代码。

### 4.2 公共资源引用

所有项目引用芯片资料、参考程序时，路径指向 `common/`：
- 参考程序: `../common/ref-programs/`
- 芯片资料: `../common/chip-docs/`

### 4.3 Git 规范

- 集群根 `.git/` 管理所有文件（公共资源 + 各项目 + 方法论种子）
- 里程碑节点必须提交
- 提交信息格式: `[milestone]` / `[project:four_head]` / `[methodology]` / `[common]`

### 4.4 跨项目协调

- 不同项目之间的代码不共享（MCU 不同、架构不同）
- 方法论是唯一跨项目共享的东西
- 公共工具放 `common/tools/`，项目特定工具放各项目的 `tools/`

---

## 五、新项目添加流程

1. 复制 `methodology-seed/` 的核心文件到新项目
2. 从 `methodology-seed/templates/` 复制并填写规格文件
3. 创建项目级 `CLAUDE.md`
4. 更新本文件的 §二 项目清单
5. Git 提交

---

*最后更新: 2026-05-27 — 项目集群初始化*
