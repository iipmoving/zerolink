---
name: manifest
description: 项目完整文件清单——方法论、工具、规格、架构、代码模块的绝对路径索引
metadata:
  type: reference
  originSessionId: 7eb949aa-9578-417f-ab12-d6428ae59747
---

# 项目文件清单 (Document Manifest)

> 生成日期: 2026-05-27
> 里程碑: 项目集群重构 — src/ 归位 + 公共资源集中 + 集群 CLAUDE.md
>
> 用途: 换一个全新的 AI 对话，读一遍本清单 + `memory/` 下的方法论文档，能独立搭建出相同架构的项目。

---

## 一、项目集群结构

```
低耦合程序架构/                    # 项目集群根
│
├── CLAUDE.md                     # ★ 集群级总控 (硬件配置、项目清单、集群规则)
├── .gitignore
│
├── methodology-seed/             # ★ 方法论种子 — 新项目从这里复制
│   ├── ONBOARDING.md             #   新 AI 启动指南
│   ├── 01-architecture.md        #   五层架构 + 六大铁律 v2.1
│   ├── 02-weak-callback.md
│   ├── 03-dual-engine.md
│   ├── 04-golden-output.md
│   ├── 05-interface-management.md
│   ├── 06-json-driven.md
│   ├── 07-struct-generation.md   #   生成式结构体管理
│   ├── MEMORY.md
│   └── templates/                #   项目模板 (CLAUDE.md, interface_map.h, structs.json, etc.)
│
├── four_head/                    # 项目1: 四头灯板零依赖系统
│   ├── CLAUDE.md                 #   项目级指令
│   ├── src/                      #   源码 (app/drv/hal/proto/core/api/cfg/vendor)
│   ├── Project/                  #   KEIL MDK 项目
│   ├── sim/                      #   WASM/JS 仿真
│   ├── test/                     #   单元测试
│   ├── docs/                     #   项目文档
│   ├── tools/                    #   项目工具
│   ├── memory/                   #   方法论记忆 (双写)
│   └── .claude/                  #   Claude Code 配置 (agents/specs/settings)
│
├── m4_ekf_observer/              # 项目2: 半桥 EKF 观测器
│   ├── CLAUDE.md
│   ├── app/ekf/
│   ├── tools/ekf_tuner/
│   └── src/
│
├── common/                       # 公共资源
│   ├── ref-programs/             #   参考程序 (不在版本管理)
│   ├── chip-docs/                #   芯片资料 (不在版本管理)
│   └── docs/                     #   跨项目公共文档
│
└── archive/                      # 历史归档
    ├── four_head_ih_cooker.zip
    ├── four_head_ih_cooker_v0/   # (不在版本管理)
    └── four_head_ih_cooker_v1/   # (不在版本管理)
```

---

## 二、方法论文件 (AI Memory)

**双写规则**: 所有方法论文件同时存在于两个位置，内容一致:
- 自动记忆: `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\`
- 项目目录: `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\memory\`

**项目目录优先** — 新AI会话应优先从项目目录读取。

### 核心方法论

| 文件 | 项目路径 | 说明 |
|------|---------|------|
| MEMORY.md | `four_head/memory/MEMORY.md` | 记忆索引 (入口) |
| MANIFEST.md | `four_head/memory/MANIFEST.md` | 本文件 — 完整文件清单 |
| methodology_first-principle.md | `four_head/memory/methodology_first-principle.md` | ★ 最高优先级: 方法论是唯一产出 |
| methodology_ai-zero-coupling-complete.md | `four_head/memory/methodology_ai-zero-coupling-complete.md` | AI零耦合嵌入式架构方法论完整版 v2.0 |
| methodology_weak-callback-zero-dependency.md | `four_head/memory/methodology_weak-callback-zero-dependency.md` | ★ __weak回调零依赖实施方法 |
| methodology_independent-type-declarations.md | `four_head/memory/methodology_independent-type-declarations.md` | 独立声明铁律: 跨模块类型各自声明 |
| methodology_ai-managed-dependency-injection.md | `four_head/memory/methodology_ai-managed-dependency-injection.md` | AI管理__weak回调配对 v2.0 |
| methodology_golden-output-decomposition.md | `four_head/memory/methodology_golden-output-decomposition.md` | ★ 模块分解黄金输出验证 |
| methodology_ai-message-id-namespace.md | `four_head/memory/methodology_ai-message-id-namespace.md` | [已废弃 v2.0] 消息ID命名空间管理 |

### 反馈与经验

| 文件 | 项目路径 | 说明 |
|------|---------|------|
| feedback_core-principles-iron-laws.md | `four_head/memory/feedback_core-principles-iron-laws.md` | 核心铁律: 解耦第一, JSON覆盖逻辑第二 |
| feedback_json_coverage.md | `four_head/memory/feedback_json_coverage.md` | JSON覆盖率是核心目标 |
| feedback_touch_polling.md | `four_head/memory/feedback_touch_polling.md` | 触摸库自带中断, 10ms读一次即可 |
| feedback_key_release_tap.md | `four_head/memory/feedback_key_release_tap.md` | 松手判定TAP, 长按抑制误触发 |
| design-principle-json-isolation.md | `four_head/memory/design-principle-json-isolation.md` | 核心设计原则: 孤岛隔离 |

### 项目里程碑

| 文件 | 项目路径 | 说明 |
|------|---------|------|
| milestone-87pct-json-coverage.md | `four_head/memory/milestone-87pct-json-coverage.md` | 里程碑: 87% JSON覆盖率 |
| project_p0-1-complete.md | `four_head/memory/project_p0-1-complete.md` | P0-1 四项JSON化完成 |
| project_p0-2-complete.md | `four_head/memory/project_p0-2-complete.md` | P0-2 SlotElement + value_map |
| project_p0-3-complete.md | `four_head/memory/project_p0-3-complete.md` | P0-3 LEDElement |
| project_p0-4-complete.md | `four_head/memory/project_p0-4-complete.md` | P0-4 StatusLED 全面集成 |
| project_p1-blink-binding.md | `four_head/memory/project_p1-blink-binding.md` | P1 BlinkRule 闪烁绑定 |
| project_p2-mode-rule.md | `four_head/memory/project_p2-mode-rule.md` | P2 ModeRule 优先级链 |
| project_wasm-test-round2-complete.md | `four_head/memory/project_wasm-test-round2-complete.md` | WASM第二轮测试完成 |
| project_arch-coupling-audit.md | `four_head/memory/project_arch-coupling-audit.md` | 架构耦合审查 |

---

## 三、项目规格文件

**项目根**: `D:\OBSIDIAN\MOVING IH\低耦合程序架构\`

| 文件 | 路径 | 说明 |
|------|------|------|
| CLAUDE.md (集群根) | `CLAUDE.md` | ★ 集群总控: 硬件配置、项目清单、集群规则、仿真位置 |
| CLAUDE.md (four_head) | `four_head/CLAUDE.md` | 项目级: 方法论第一, 铁律, 术语, 接口约定 |
| tech-stack.md | `four_head/.claude/specs/tech-stack.md` | 技术栈: MCU, 编译器, 外设, 依赖库 |
| code-style.md | `four_head/.claude/specs/code-style.md` | 代码规范: 命名, 注释, 文件结构, 格式 |
| ubiquitous-language.md | `four_head/.claude/specs/ubiquitous-language.md` | DDD通用语言: 所有术语唯一定义 |
| api-conventions.md | `four_head/.claude/specs/api-conventions.md` | 接口约定: 回调 vs 消息, 调度模式, 依赖规则 |
| hmi-model-v6.md | `four_head/.claude/specs/hmi-model-v6.md` | HMI 模型 v6 |

---

## 四、自动化工具

**路径**: `four_head/tools/`

| 文件 | 路径 | 说明 |
|------|------|------|
| check_deps.py | `four_head/tools/check_deps.py` | 层依赖审计: app→drv/hal 违规检测 |
| check_msgs.py | `four_head/tools/check_msgs.py` | (v1.0残留) 消息ID一致性检查, 待更新为 check_weak_pairs.py |
| v4json_to_c.py | `four_head/tools/v4json_to_c.py` | JSON配置 → C结构体自动生成 |

---

## 五、核心基础设施

**路径**: `four_head/src/core/`

| 文件 | 路径 | 说明 |
|------|------|------|
| interface_map.h | `four_head/src/core/interface_map.h` | ★ __weak回调配对映射表 + 结构体配对表 (唯一真相源, 文档, 禁止include) |
| msg_scheduler.h | `four_head/src/core/msg_scheduler.h` | [废弃 v2.0] #error守卫, 防止误include |
| msg_scheduler.c | `four_head/src/core/msg_scheduler.c` | [废弃 v2.0] #error守卫 |
| msg_def.h | `four_head/src/core/msg_def.h` | [废弃 v2.0] #error守卫 |

---

## 六、源代码模块

**路径**: `four_head/src/`

### APP 层 (业务逻辑)

| 文件 | 路径 | __weak 发出 | __weak 接收 |
|------|------|------------|------------|
| app_hmi.c | `four_head/src/app/app_hmi.c` | DrvDisplay_OnRefresh, DrvBuzzer_OnCtrl | AppHmi_OnKey, AppHmi_OnTimer100ms, AppHmi_OnTimer1s |
| app_hmi.h | `four_head/src/app/app_hmi.h` | — | — |
| app_cooking.c | `four_head/src/app/app_cooking.c` | AppPower_OnPowerCtrl, DrvDisplay_OnRefresh | AppCooking_OnKey, AppCooking_OnRegData, AppCooking_OnTimer1s |
| app_cooking.h | `four_head/src/app/app_cooking.h` | — | — |
| app_power.c | `four_head/src/app/app_power.c` | AppCommMgr_OnPowerCmd | AppPower_OnPowerCtrl, AppPower_OnRegData, AppPower_OnSystemError |
| app_power.h | `four_head/src/app/app_power.h` | — | — |
| app_protect.c | `four_head/src/app/app_protect.c` | AppPower_OnSystemError | AppProtect_OnRegData |
| app_protect.h | `four_head/src/app/app_protect.h` | — | — |
| app_comm_mgr.c | `four_head/src/app/app_comm_mgr.c` | AppPower_OnRegData, AppCooking_OnRegData, AppProtect_OnRegData, DrvCommMgr_OnSendReq | AppCommMgr_OnDataUpdate, AppCommMgr_OnTxDone, AppCommMgr_OnPowerCmd |
| app_comm_mgr.h | `four_head/src/app/app_comm_mgr.h` | — | — |

### DRV 层 (设备驱动)

| 文件 | 路径 | __weak 发出 | __weak 接收 |
|------|------|------------|------------|
| drv_key.c | `four_head/src/drv/drv_key.c` | AppHmi_OnKey, AppCooking_OnKey | — |
| drv_key.h | `four_head/src/drv/drv_key.h` | — | — |
| drv_display.c | `four_head/src/drv/drv_display.c` | — | DrvDisplay_OnRefresh |
| drv_display.h | `four_head/src/drv/drv_display.h` | — | — |
| drv_buzzer.c | `four_head/src/drv/drv_buzzer.c` | — | DrvBuzzer_OnCtrl |
| drv_buzzer.h | `four_head/src/drv/drv_buzzer.h` | — | — |
| drv_comm_mgr.c | `four_head/src/drv/drv_comm_mgr.c` | AppCommMgr_OnTxDone, AppCommMgr_OnDataUpdate | DrvCommMgr_OnSendReq |
| drv_comm_mgr.h | `four_head/src/drv/drv_comm_mgr.h` | — | — |
| drv_comm.c | `four_head/src/drv/drv_comm.c` | — | — |
| drv_comm.h | `four_head/src/drv/drv_comm.h` | — | — |

### 入口

| 文件 | 路径 | __weak 发出 | __weak 接收 |
|------|------|------------|------------|
| main.c | `four_head/src/main.c` | AppHmi_OnTimer100ms, AppHmi_OnTimer1s, AppCooking_OnTimer1s | — |

### HAL 层 (硬件抽象, 零依赖)

| 文件 | 路径 |
|------|------|
| hal_comm.c/h | `four_head/src/hal/hal_comm.c` |
| hal_timer.c/h | `four_head/src/hal/hal_timer.c` |
| hal_uart.c/h | `four_head/src/hal/hal_uart.c` |
| hal_gpio.c/h | `four_head/src/hal/hal_gpio.c` |
| hal_buzzer.c/h | `four_head/src/hal/hal_buzzer.c` |
| hal_key.c/h | `four_head/src/hal/hal_key.c` |
| hal_display.c/h | `four_head/src/hal/hal_display.c` |
| hal_smg.c/h | `four_head/src/hal/hal_smg.c` |

### PROTO 层 (协议解析, 纯函数)

| 文件 | 路径 |
|------|------|
| proto_modbus.c/h | `four_head/src/proto/proto_modbus.c` |

### CFG 层 (配置数据)

| 文件 | 路径 |
|------|------|
| hmi_data.c/h | `four_head/src/cfg/hmi_data.c` |
| gen_hmi.js | `four_head/src/cfg/gen_hmi.js` |
| 灯板逻辑_head_template.json | `four_head/src/cfg/灯板逻辑_head_template.json` |

### Vendor (供应商代码)

| 目录 | 路径 |
|------|------|
| CMSIS | `four_head/src/vendor/CMSIS/` |
| FWLib | `four_head/src/vendor/FWLib/` |
| MCU_Drivers | `four_head/src/vendor/MCU_Drivers/` |
| lib | `four_head/src/vendor/lib/` |

---

## 七、仿真环境

| 位置 | 路径 | 说明 |
|------|------|------|
| 灯板桌面模拟器 | `four_head/sim/test_hmi/` | Canvas 面板 + JSON 规则引擎 + 自动化测试 |
| 面板编辑器 | `four_head/sim/phase1-js-logic/` | 拖拽布局 + JS/WASM 双逻辑层 |
| C→WASM 编译 | `four_head/sim/phase2-wasm-c-source/` | C 状态机 → WASM 编译源码 |
| WASM 模块分解 | `four_head/sim/test_hmi/wasm/modules/` | 5 模块 __weak 架构 (key/state/timer/display/main) |

---

## 八、架构与评审文档

**路径**: `four_head/docs/`

| 文件 | 路径 | 说明 |
|------|------|------|
| engine-spec.md | `four_head/docs/engine-spec.md` | 引擎规格 |
| PROJECT-INVENTORY.md | `four_head/docs/PROJECT-INVENTORY.md` | 项目资料清单 |

### architecture/

| 文件 | 路径 | 说明 |
|------|------|------|
| 四头电磁炉架构方案.md | `four_head/docs/architecture/四头电磁炉架构方案.md` | 整体架构方案 |
| 嵌入式灯板控制系统架构说明.md | `four_head/docs/architecture/嵌入式灯板控制系统架构说明.md` | 灯板控制系统架构 |
| json-architecture-phase-report.md | `four_head/docs/architecture/json-architecture-phase-report.md` | JSON架构阶段报告 |
| json-coverage-analysis.md | `four_head/docs/architecture/json-coverage-analysis.md` | JSON覆盖率分析 |
| hmi-json-message-test-plan.md | `four_head/docs/architecture/hmi-json-message-test-plan.md` | HMI JSON消息测试计划 |
| hardware-verification-workflow.md | `four_head/docs/architecture/hardware-verification-workflow.md` | 硬件验证工作流 |
| js-json-to-c-encoding-guide.md | `four_head/docs/architecture/js-json-to-c-encoding-guide.md` | JS→C编码转换指南 |
| two-method-display-refactor-guide.md | `four_head/docs/architecture/two-method-display-refactor-guide.md` | 双方法显示重构指南 |

### specs/

| 文件 | 路径 | 说明 |
|------|------|------|
| 四头电磁炉完整规格书 V1.0.md | `four_head/docs/specs/四头电磁炉完整规格书 V1.0.md` | 产品规格书 |
| 4头电磁炉Modbus通信协议规格书.md | `four_head/docs/specs/4头电磁炉Modbus通信协议规格书.md` | MODBUS协议规格 |
| hmi-data-flow-table.md | `four_head/docs/specs/hmi-data-flow-table.md` | HMI数据流表 |

### reviews/ (关键评审)

| 文件 | 路径 | 说明 |
|------|------|------|
| audit-weak-transition-2026-05-26.md | `four_head/docs/reviews/audit-weak-transition-2026-05-26.md` | ★ __weak v2.0过渡审计报告 |
| AR-2026-05-26-MODULAR.md | `four_head/docs/reviews/AR-2026-05-26-MODULAR.md` | 验收报告 |
| PHASE-CLOSURE-2026-05-26.md | `four_head/docs/reviews/PHASE-CLOSURE-2026-05-26.md` | 阶段结项报告 |
| audit-decoupled-architecture.md | `four_head/docs/reviews/audit-decoupled-architecture.md` | 解耦架构审计 |

---

## 九、外部资源 (非版本管理)

| 用途 | 路径 |
|------|------|
| 参考程序 (硬件配置权威来源) | `common/ref-programs/` |
| 芯片资料 (数据手册/TRM) | `common/chip-docs/` |
| 半桥/全桥加热控制 | `m4_ekf_observer/` |
| MODBUS测试工具 | `m4_ekf_observer/tools/ekf_tuner/m4_modbus_tool.py` |
| HMI JS+JSON验证程序 | `four_head/sim/test_hmi/` |

---

## 十、2026-05-27 里程碑变更摘要

**里程碑**: 项目集群重构 — 目录归位 + 公共资源集中 + 集群 CLAUDE.md

**变更范围**:
- four_head/src/: 源码从 Claude/ 归入标准 src/ 结构 (app/drv/hal/proto/core/api/cfg)
- four_head/src/vendor/: 供应商代码独立 (CMSIS/FWLib/MCU_Drivers/lib)
- common/: 公共资源集中 (docs/ + ref-programs/ + chip-docs/)
- archive/: 历史版本归档 (zip)
- 根 CLAUDE.md: 定位升级为项目集群总控
- .gitignore: 全面覆盖构建产物/IDE临时文件/worktree/本地配置
- methodology-seed/: 方法论种子 (新项目可复制启动)
- m4_ekf_observer/: 纳入集群管理

**上一个里程碑 (2026-05-26)**:
- 彻底消除 MsgScheduler → __weak 回调直调 v2.0
- app_hmi.c 模块分解: 1897行 → 5个独立模块, WASM 双引擎 66/66 PASS
- 16条消息通道转换, 34个Msg_Post消除
- core/msg_scheduler 废弃

---

*维护: AI (技术负责人+文档管理员角色)*
*最后更新: 2026-05-27*
