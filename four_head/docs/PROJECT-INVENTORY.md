# 项目资料清单 — 四头电磁炉低耦合控制程序

> **用途**: 第三方审计 / 新 AI 会话入口 / 知识移交  
> **根路径**: `D:\OBSIDIAN\MOVING IH\低耦合程序架构\`  
> **生成日期**: 2026-05-26  
> **阶段**: __weak 模块分解 + 双引擎同源检测 完成  

---

## 一、阅读路线 (按角色)

| 角色 | 建议阅读顺序 |
|------|-------------|
| **审计员** | 本清单 → AR-2026-05-26-MODULAR.md → PHASE-CLOSURE-2026-05-26.md → interface_map.h → test_wasm_basic.js |
| **新 AI 会话** | `four_head/CLAUDE.md` → `memory/MANIFEST.md` → `memory/methodology_*.md` → 然后按需深入 |
| **开发者 (HMI 扩展)** | `four_head/CLAUDE.md` → 本清单 §五 (SOP) → `modules/` → `js/` → `test_wasm_basic.js` |
| **硬件工程师** | 根 `CLAUDE.md` (硬件配置段) → `参考程序/` → `芯片资料/` |

---

## 二、顶层结构

```
D:\OBSIDIAN\MOVING IH\低耦合程序架构\
├── CLAUDE.md                     ← 硬件配置 + 调度模式 (每个 AI 会话自动加载)
├── four_head\                    ← ★ 本项目的全部内容
│   ├── CLAUDE.md                 ← 项目级指令 (铁律/术语/接口约定)
│   ├── memory\                   ← 方法论文档 (21个.md)
│   ├── docs\                     ← 架构/规格/评审/测试报告
│   ├── .claude\                  ← specs(技术栈/代码规范/术语/接口) + agents(角色) + adr(架构决策)
│   ├── tools\                    ← Python 自动化工具
│   ├── Claude\                   ← C 生产代码 (ARM 目标, Keil MDK)
│   │   ├── app\                  ← APP 业务层
│   │   ├── drv\                  ← DRV 设备驱动层
│   │   ├── hal\                  ← HAL 硬件抽象层
│   │   ├── core\                 ← interface_map.h + [废弃] msg_scheduler
│   │   ├── proto\                ← MODBUS 协议解析
│   │   └── src\                  ← main.c 入口
│   ├── sim\test_hmi\             ← ★ HMI 仿真与测试
│   │   ├── js\                   ← JS 参考引擎 (34条独立测试)
│   │   ├── wasm\                 ← WASM C 引擎 + 双引擎对比测试
│   │   └── four_head_v4.json     ← JSON 规则 (唯一数据源)
│   └── Project\                  ← Keil MDK 项目文件
├── 参考程序\                     ← 硬件配置权威来源 (不在版本管理)
├── 芯片资料\                     ← 数据手册/TRM (不在版本管理)
└── m4_ekf_observer\              ← 半桥/全桥加热控制 (独立项目)
```

---

## 三、完整文件清单

### 3.1 规格文件 (每会话自动加载)

| 文件 | 路径 | 内容 |
|------|------|------|
| 根 CLAUDE.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\CLAUDE.md` | 硬件引脚/外设/时钟/调度模式/已用库 |
| 项目 CLAUDE.md | `four_head\CLAUDE.md` | ★ 铁律(解耦第一/JSON覆盖逻辑/统一出入口) + 术语 + 接口约定 + AI行为规范 |
| 技术栈 | `.claude\specs\tech-stack.md` | MCU/编译器/外设/依赖库 |
| 代码规范 | `.claude\specs\code-style.md` | 命名/注释/文件结构/格式 |
| 通用语言 | `.claude\specs\ubiquitous-language.md` | ★ 所有术语唯一定义 (系统状态/Zone状态/进程/显示布局) |
| 接口约定 | `.claude\specs\api-conventions.md` | 回调 vs 消息 / 依赖规则 / 显示装配器 |

### 3.2 方法论文档 (AI 记忆, 双写)

**双写位置**: `four_head\memory\` = `C:\Users\moving\.claude\projects\...\memory\`

| 文件 | 状态 | 说明 |
|------|------|------|
| `MEMORY.md` | active | 记忆索引 |
| `MANIFEST.md` | active | ★ 完整文件清单 (本文件的详细版) |
| `methodology_first-principle.md` | active | ★ 最高优先级: 方法论是唯一产出 |
| `methodology_weak-callback-zero-dependency.md` | active | ★ __weak 回调零依赖架构 v2.0 (核心) |
| `methodology_ai-zero-coupling-complete.md` | active (v2.0) | AI 零耦合方法论完整版 |
| `methodology_ai-managed-dependency-injection.md` | active (v2.0) | AI 管理 __weak 回调配对 |
| `methodology_independent-type-declarations.md` | active | 独立类型声明铁律 |
| `methodology_ai-message-id-namespace.md` | **deprecated** | 消息 ID 命名空间 (v2.0 不再需要) |
| `feedback_core-principles-iron-laws.md` | active | 核心铁律 (解耦第一, JSON覆盖逻辑第二) |
| `feedback_json_coverage.md` | active | JSON 覆盖率是核心目标 |
| `feedback_key_release_tap.md` | active | 松手判定TAP |
| `feedback_touch_polling.md` | active | 触摸库 10ms 读一次 |
| `design-principle-json-isolation.md` | active | 孤岛隔离原则 |
| `milestone-87pct-json-coverage.md` | — | 里程碑: 87% JSON覆盖率 |
| `project_p0-*.md` (×4) | — | P0 JSON 化交付记录 |
| `project_p1-blink-binding.md` | — | P1 BlinkRule |
| `project_p2-mode-rule.md` | — | P2 ModeRule |
| `project_wasm-test-round2-complete.md` | — | WASM 测试第二轮 |
| `project_arch-coupling-audit.md` | — | 架构耦合审计 |

### 3.3 架构文档

| 文件 | 路径 |
|------|------|
| JSON 架构阶段报告 | `docs\architecture\json-architecture-phase-report.md` |
| Web 仿真框架方案 V2.0 | `docs\architecture\Web仿真框架 - 逻辑层JSON规则驱动实施方案 V2.0.md` |
| JSON 覆盖率分析 | `docs\architecture\json-coverage-analysis.md` |
| HMI JSON 消息测试计划 | `docs\architecture\hmi-json-message-test-plan.md` |
| JS→C 编码转换指南 | `docs\architecture\js-json-to-c-encoding-guide.md` |
| 双方法显示重构指南 | `docs\architecture\two-method-display-refactor-guide.md` |
| 硬件验证工作流 | `docs\architecture\hardware-verification-workflow.md` |
| 整体架构方案 | `docs\architecture\四头电磁炉架构方案.md` |
| 灯板控制系统架构 | `docs\architecture\嵌入式灯板控制系统架构说明.md` |

### 3.4 架构决策记录 (ADR)

| 文件 | 决策 |
|------|------|
| `.claude\specs\adr\2026-05-21-app-layer-architecture.md` | APP 层架构 |
| `.claude\specs\adr\2026-05-24-nonblocking-debug-uart.md` | 非阻塞调试串口 |
| `.claude\specs\adr\2026-05-25-display-rendering-to-drv.md` | 显示渲染下沉 DRV |
| `.claude\specs\adr\2026-05-25-json-two-layer-separation.md` | JSON 两层层分离 |
| `.claude\specs\adr\2026-05-26-wasm-test-expansion.md` | WASM 测试扩展 |
| `.claude\specs\adr\README.md` | ADR 索引 |

### 3.5 产品规格

| 文件 | 路径 |
|------|------|
| 四头电磁炉完整规格书 V1.0 | `docs\specs\四头电磁炉完整规格书 V1.0.md` |
| MODBUS 通信协议规格书 | `docs\specs\4头电磁炉Modbus通信协议规格书.md` |
| HMI 数据流表 | `docs\specs\hmi-data-flow-table.md` |

### 3.6 测试与评审文档

| 文件 | 路径 | 说明 |
|------|------|------|
| ★ 验收报告 | `docs\reviews\AR-2026-05-26-MODULAR.md` | 本阶段正式验收 (第三方审计用) |
| ★ 阶段结项报告 | `docs\reviews\PHASE-CLOSURE-2026-05-26.md` | 资料清单 + SOP + 接口归一化 |
| __weak 过渡审计 | `docs\reviews\audit-weak-transition-2026-05-26.md` | v1.0→v2.0 迁移审计 |
| 层解耦评审 | `docs\reviews\REVIEW_LAYER_DECOUPLE.md` | 依赖关系检查 |
| 评审报告 v1/v2 | `docs\reviews\REVIEW_REPORT*.md` | 前序评审 |
| 解耦架构审计 | `docs\reviews\audit-decoupled-architecture.md` | 独立审计 |
| HMI 合规审计 | `docs\reviews\hmi-compliance-audit-2026-05-25.md` | 合规检查 |
| HMI 验证 (×3轮) | `docs\reviews\hmi-verification-*.md` | HMI 多轮验证 |
| HMI 编码合规 | `docs\reviews\hmi-encoding-compliance-check.md` | 编码规范检查 |
| HMI 定时逻辑 | `docs\reviews\hmi-timer-logic-check.md` | 定时专项检查 |
| LED 测试模式归档 | `docs\reviews\led-test-mode-archive.md` | LED 历史 |
| V8 JSON 规格评审 | `docs\reviews\review-v8-json-spec-methodology.md` | JSON 规格审核 |
| JSON 灯板逻辑审计 | `docs\reviews\review-json-灯板逻辑-审计报告.md` | JSON 灯板审计 |
| Bug: HMI 选中闪烁 | `docs\reviews\bug-log-hmi-select-blink.md` | 已知 Bug 记录 |

### 3.7 AI 指南

| 文件 | 路径 |
|------|------|
| 设计方法论 | `docs\ai-guidelines\design-methodology.md` |
| AI 嵌入式编程约束 | `docs\ai-guidelines\AI嵌入式编程约束.md` |
| AI 提示词模板 | `docs\ai-guidelines\AI提示词.md` |
| 自检清单 | `docs\ai-guidelines\AI编程规范-自检清单.md` |
| JS→C 转换指南 | `docs\ai-guidelines\js-to-c-conversion.md` |
| HMI 逻辑工作流 | `docs\ai-guidelines\hmi-logic-workflow.md` |
| 段码库用户手册 | `docs\references\SMG_Disp_General_Lib_用户手册.md` |

---

## 四、代码模块清单

### 4.1 WASM 仿真引擎 (C)

```
sim\test_hmi\wasm\modules\
├── main.c                    170行  接线层 (init/tick/key 分发 + WASM导出 + DRV桩)
├── key_module.c               86行  按键映射 (Key_Inject → HmiState_OnKey)
├── key_module.h               30行
├── state_module.c           1672行  ★ 状态机核心 (select/confirm/boost/timer/上电序列/栈管理/显示派生)
├── state_module.h             53行
├── timer_module.c            195行  定时器 (超时检测 + 1s倒数 + 三个超时回调)
├── timer_module.h             30行
├── display_module.c           73行  显示 blink 相位 + DrvDisplay_OnRefresh
├── display_module.h           24行
├── interface_map.h            49行  ★ __weak 配对唯一真相源
└── weak_macro.h               11行  WEAK 宏 (ARMCC/Emscripten)

sim\test_hmi\wasm\cfg\
├── hmi_data.c                276行  HMI 配置数据 (v4json_to_c.py 自动生成)
└── hmi_data.h                 21行
```

### 4.2 JS 参考引擎

```
sim\test_hmi\js\
├── json_logic_engine.js    1,245行  ★ 核心引擎 (JSON规则解释器, 状态机)
├── test_runner.js            780行  34条独立测试
├── slot_element.js           120行  数码管 Slot 元素
├── led_element.js             95行  LED 元素
├── blink_rule.js              40行  闪烁规则解释器
├── mode_rule.js               45行  模式规则解释器
├── message_bus.js            135行  消息总线
├── key_handler.js            110行  按键注入 + 长按/短按判定
├── renderer.js               280行  显示渲染器
└── hmi_config.js              20行  HMI 配置
```

### 4.3 WASM 测试与构建

```
sim\test_hmi\wasm\
├── test_wasm_basic.js      1,096行  ★ 35条双引擎对比测试 (66断言)
├── wasm_adapter.js           145行  WASM↔JS 适配 (C enum ↔ JS string 映射)
├── Makefile                   49行  构建 (emcc)
├── build_wasm.bat             10行  Windows 构建
├── test_key_module.html       ~80行  按键模块浏览器测试
├── test_timer_module.html     ~90行  定时器模块浏览器测试
├── test_display_module.html   ~110行 显示模块浏览器测试
├── test_state_module.html    ~100行 状态机浏览器测试
└── test_integration.html     ~400行 35条全流程浏览器集成测试
```

### 4.4 C 生产代码 (ARM 目标)

```
Claude\
├── app\
│   ├── app_hmi.c           1,897行  原单块文件 (保留参考)
│   ├── app_hmi.h              类型定义 (HmiHead_t, HmiGlobalState_t, etc.)
│   ├── app_cooking.c          烹饪控制
│   ├── app_power.c            功率控制
│   ├── app_protect.c          安全保护
│   └── app_comm_mgr.c         通讯管理
├── drv\
│   ├── drv_key.c              按键驱动 (触摸→键码)
│   ├── drv_display.c          显示驱动 (COM扫描)
│   ├── drv_buzzer.c           蜂鸣器驱动
│   └── drv_comm.c             通讯驱动
├── hal\
│   ├── hal_comm.c/h           通讯 HAL (UART0 DMA)
│   ├── hal_timer.c/h          时基 HAL (TIM0 125us)
│   ├── hal_uart.c/h           调试串口 HAL (UART3)
│   ├── hal_gpio.c/h           GPIO HAL
│   ├── hal_buzzer.c/h         蜂鸣器 HAL (TIM1 PWM)
│   └── hal_key.c/h            触摸 HAL
├── core\
│   ├── interface_map.h        ★ __weak 配对映射表 (生产代码版)
│   ├── msg_scheduler.c/h      [已废弃] #error 守卫
│   └── msg_def.h              [已废弃] #error 守卫
├── proto\
│   ├── proto_modbus.c/h       MODBUS RTU 编解码
└── src\
    └── main.c                 入口 (时基 + 调度 + 初始化)
```

### 4.5 自动化工具

```
tools\
├── check_deps.py            层依赖审计 (app→drv/hal 违规检测)
├── check_msgs.py            (v1.0残留) 消息ID检查, 待更新
└── v4json_to_c.py           JSON配置 → C结构体自动生成
```

---

## 五、关键数据流图 (同一页速查)

### 5.1 双引擎同源检测

```
four_head_v4.json (唯一数据源)
    │
    ├──→ js/json_logic_engine.js (参考实现, 快速迭代)
    │       │
    │       └──→ js/test_runner.js (34条独立测试)
    │
    ├──→ tools/v4json_to_c.py
    │       │
    │       └──→ wasm/cfg/hmi_data.c (编译时嵌入)
    │               │
    │               └──→ wasm/modules/*.c (5个模块)
    │                       │
    │                       └──→ wasm/test_wasm_basic.js (35条双引擎对比)
    │                               │
    │                               └── JS引擎状态 ←→ WASM C引擎状态
    │                                   66/66 PASS
    │
    └──→ Claude/app/*.c (ARM生产代码, 独立演进)
```

### 5.2 __weak 模块通信

```
main.c
  ├─ engine_post_key ──→ key_module ──HmiState_OnKey(WEAK)──→ state_module
  ├─ engine_tick_100ms ──→ timer_module ──HmiState_On*Timeout(WEAK)──→ state_module
  ├─ engine_tick_100ms ──→ state_module ──Display_OnStateChange(WEAK)──→ display_module
  │                                    └──Timer_OnStateChange(WEAK)──→ timer_module
  └─ engine_tick_100ms ──→ display_module ──DrvDisplay_OnRefresh(WEAK)──→ main.c(DRV桩)

共 7 条 __weak 配对, 0 条消息队列, 0 个 include 跨模块私有头文件
```

---

## 六、接口一致性检查点 (7 项归一化检查)

每次扩展后必须验证:

| # | 检查项 | 工具/方法 |
|---|--------|----------|
| 1 | JSON → C 数据一致 | 重运行 `v4json_to_c.py` |
| 2 | C enum ↔ JS string 映射 | 人工比对 wasm_adapter.js 三个 C2JS 数组 |
| 3 | 键码三处一致 (C/JS/JSON) | `grep KEY_LEFT_P_SET_UP drv/drv_key.h` ↔ `wasm_adapter.js` KEY_CODE ↔ JSON keyCode |
| 4 | __weak 签名配对 | 人工比对 interface_map.h ↔ 各模块 .h/.c |
| 5 | 新增导出函数两处同步 | Makefile EXPORTED_FUNCS = build_wasm.bat |
| 6 | 层依赖无违规 | `python tools/check_deps.py ../Claude` |
| 7 | 双引擎全部通过 | `node test_wasm_basic.js` → 100% PASS |

---

## 七、外部依赖与参考

| 资源 | 路径 | 用途 |
|------|------|------|
| Emscripten | `D:\emsdk\upstream\emscripten\emcc.bat` | WASM 编译 |
| Node.js | 系统 PATH | 测试运行 |
| Keil MDK V5 | `C:\Keil_v5\ARM\ARMCC\bin\armcc.exe` | ARM 编译 (本阶段未使用) |
| 参考程序 | `..\参考程序\ai专用工程2026.4.27(06B显示板)\` | 硬件引脚/配置权威来源 |
| 芯片手册 | `..\芯片资料\` | SC32L14T 数据手册/TRM |
| 半桥加热项目 | `..\m4_ekf_observer\` | 独立项目, MODBUS 测试工具在此 |

---

*维护: 技术负责人角色. 每次阶段交付后更新.*
