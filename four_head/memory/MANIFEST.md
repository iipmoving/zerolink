---
name: manifest
description: 项目完整文件清单——方法论、工具、规格、架构、代码模块的绝对路径索引
metadata:
  type: reference
  originSessionId: 7eb949aa-9578-417f-ab12-d6428ae59747
---

# 项目文件清单 (Document Manifest)

> 生成日期: 2026-05-26
> 里程碑: __weak 回调零依赖架构 v2.0 完成
>
> 用途: 换一个全新的 AI 对话，读一遍本清单 + `memory/` 下的方法论文档，能独立搭建出相同架构的项目。

---

## 一、方法论文件 (AI Memory)

**双写规则**: 所有方法论文件同时存在于两个位置，内容一致:
- 自动记忆: `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\`
- 项目目录: `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\memory\`

**项目目录优先** — 新AI会话应优先从项目目录读取。

### 核心方法论

| 文件 | 绝对路径 | 说明 |
|------|---------|------|
| MEMORY.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\MEMORY.md` | 记忆索引 (入口) |
| MANIFEST.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\MANIFEST.md` | 本文件 — 完整文件清单 |

| 文件 | 绝对路径 | 版本 | 说明 |
|------|---------|------|------|
| methodology_first-principle.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\methodology_first-principle.md` | — | ★ 最高优先级: 方法论是唯一产出 |
| methodology_ai-zero-coupling-complete.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\methodology_ai-zero-coupling-complete.md` | v2.0 | AI零耦合嵌入式架构方法论完整版 |
| methodology_weak-callback-zero-dependency.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\methodology_weak-callback-zero-dependency.md` | v2.0 | ★ __weak回调零依赖实施方法 (本次里程碑核心文档) |
| methodology_independent-type-declarations.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\methodology_independent-type-declarations.md` | — | 独立声明铁律: 跨模块类型各自声明 |
| methodology_ai-managed-dependency-injection.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\methodology_ai-managed-dependency-injection.md` | v2.0 | AI管理__weak回调配对替代DI |
| methodology_golden-output-decomposition.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\methodology_golden-output-decomposition.md` | — | ★ 模块分解黄金输出验证: 禁止重新实现, 必须逐字段一致 |
| methodology_ai-message-id-namespace.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\methodology_ai-message-id-namespace.md` | deprecated | [已废弃 v2.0] 消息ID命名空间管理 |

### 反馈与经验

| 文件 | 绝对路径 | 说明 |
|------|---------|------|
| feedback_core-principles-iron-laws.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\feedback_core-principles-iron-laws.md` | 核心铁律: 解耦第一, JSON覆盖逻辑第二 |
| feedback_json_coverage.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\feedback_json_coverage.md` | JSON覆盖率是核心目标 |
| feedback_touch_polling.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\feedback_touch_polling.md` | 触摸库自带中断, 10ms读一次即可 |
| feedback_key_release_tap.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\feedback_key_release_tap.md` | 松手判定TAP, 长按抑制误触发 |
| design-principle-json-isolation.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\design-principle-json-isolation.md` | 核心设计原则: 孤岛隔离 |

### 项目里程碑

| 文件 | 绝对路径 | 说明 |
|------|---------|------|
| milestone-87pct-json-coverage.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\milestone-87pct-json-coverage.md` | 里程碑: 87% JSON覆盖率 |
| project_p0-1-complete.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\project_p0-1-complete.md` | P0-1 四项JSON化完成 |
| project_p0-2-complete.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\project_p0-2-complete.md` | P0-2 SlotElement + value_map |
| project_p0-3-complete.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\project_p0-3-complete.md` | P0-3 LEDElement |
| project_p0-4-complete.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\project_p0-4-complete.md` | P0-4 StatusLED 全面集成 |
| project_p1-blink-binding.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\project_p1-blink-binding.md` | P1 BlinkRule 闪烁绑定 |
| project_p2-mode-rule.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\project_p2-mode-rule.md` | P2 ModeRule 优先级链 |
| project_wasm-test-round2-complete.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\project_wasm-test-round2-complete.md` | WASM第二轮测试完成 |
| project_arch-coupling-audit.md | `C:\Users\moving\.claude\projects\d--OBSIDIAN-MOVING-IH---------four-head\memory\project_arch-coupling-audit.md` | 架构耦合审查 |

---

## 二、项目规格文件 (CLAUDE.md + specs)

**项目根**: `D:\OBSIDIAN\MOVING IH\低耦合程序架构\`

| 文件 | 绝对路径 | 说明 |
|------|---------|------|
| CLAUDE.md (根) | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\CLAUDE.md` | 硬件配置、调度模式、已用库 |
| CLAUDE.md (four_head) | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\CLAUDE.md` | 项目级: 方法论第一, 铁律, 术语, 接口约定 |
| tech-stack.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\.claude\specs\tech-stack.md` | 技术栈: MCU, 编译器, 外设, 依赖库 |
| code-style.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\.claude\specs\code-style.md` | 代码规范: 命名, 注释, 文件结构, 格式 |
| ubiquitous-language.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\.claude\specs\ubiquitous-language.md` | DDD通用语言: 所有术语唯一定义 |
| api-conventions.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\.claude\specs\api-conventions.md` | 接口约定: 回调 vs 消息, 调度模式, 依赖规则 |

---

## 三、自动化工具

**路径**: `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\tools\`

| 文件 | 绝对路径 | 说明 |
|------|---------|------|
| check_deps.py | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\tools\check_deps.py` | 层依赖审计: app→drv/hal 违规检测 |
| check_msgs.py | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\tools\check_msgs.py` | (v1.0残留) 消息ID一致性检查, 待更新为 check_weak_pairs.py |
| v4json_to_c.py | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\tools\v4json_to_c.py` | JSON配置 → C结构体自动生成 |

---

## 四、核心基础设施

**路径**: `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\core\`

| 文件 | 绝对路径 | 说明 |
|------|---------|------|
| interface_map.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\core\interface_map.h` | ★ __weak回调配对映射表 + 结构体配对表 (唯一真相源, 文档, 禁止include) |
| msg_scheduler.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\core\msg_scheduler.h` | [废弃 v2.0] #error守卫, 防止误include |
| msg_scheduler.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\core\msg_scheduler.c` | [废弃 v2.0] #error守卫, 仅MSG_SCHEDULER_ALLOWED可编译 |
| msg_def.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\core\msg_def.h` | [废弃 v2.0] #error守卫, Msg_t/MsgId_t仅兼容模式可用 |

---

## 五、源代码模块

**路径**: `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\`

### APP 层 (业务逻辑)

| 文件 | 绝对路径 | __weak 发出 | __weak 接收 |
|------|---------|------------|------------|
| app_hmi.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\app\app_hmi.c` | DrvDisplay_OnRefresh, DrvBuzzer_OnCtrl | AppHmi_OnKey, AppHmi_OnTimer100ms, AppHmi_OnTimer1s |
| app_hmi.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\app\app_hmi.h` | — | — |
| app_cooking.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\app\app_cooking.c` | AppPower_OnPowerCtrl, DrvDisplay_OnRefresh | AppCooking_OnKey, AppCooking_OnRegData, AppCooking_OnTimer1s |
| app_cooking.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\app\app_cooking.h` | — | — |
| app_power.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\app\app_power.c` | AppCommMgr_OnPowerCmd | AppPower_OnPowerCtrl, AppPower_OnRegData, AppPower_OnSystemError |
| app_power.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\app\app_power.h` | — | — |
| app_protect.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\app\app_protect.c` | AppPower_OnSystemError | AppProtect_OnRegData |
| app_protect.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\app\app_protect.h` | — | — |
| app_comm_mgr.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\app\app_comm_mgr.c` | AppPower_OnRegData, AppCooking_OnRegData, AppProtect_OnRegData, DrvCommMgr_OnSendReq | AppCommMgr_OnDataUpdate, AppCommMgr_OnTxDone, AppCommMgr_OnPowerCmd |
| app_comm_mgr.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\app\app_comm_mgr.h` | — | — |

### DRV 层 (设备驱动)

| 文件 | 绝对路径 | __weak 发出 | __weak 接收 |
|------|---------|------------|------------|
| drv_key.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\drv\drv_key.c` | AppHmi_OnKey, AppCooking_OnKey | — |
| drv_key.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\drv\drv_key.h` | — | — |
| drv_display.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\drv\drv_display.c` | — | DrvDisplay_OnRefresh |
| drv_display.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\drv\drv_display.h` | — | — |
| drv_buzzer.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\drv\drv_buzzer.c` | — | DrvBuzzer_OnCtrl |
| drv_buzzer.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\drv\drv_buzzer.h` | — | — |
| drv_comm_mgr.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\drv\drv_comm_mgr.c` | AppCommMgr_OnTxDone, AppCommMgr_OnDataUpdate | DrvCommMgr_OnSendReq |
| drv_comm_mgr.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\drv\drv_comm_mgr.h` | — | — |
| drv_comm.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\drv\drv_comm.c` | — | — |
| drv_comm.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\drv\drv_comm.h` | — | — |

### 入口

| 文件 | 绝对路径 | __weak 发出 | __weak 接收 |
|------|---------|------------|------------|
| main.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\src\main.c` | AppHmi_OnTimer100ms, AppHmi_OnTimer1s, AppCooking_OnTimer1s | — |

### HAL 层 (硬件抽象, 零依赖)

| 文件 | 绝对路径 |
|------|---------|
| hal_comm.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\hal\hal_comm.c` |
| hal_comm.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\hal\hal_comm.h` |
| hal_timer.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\hal\hal_timer.c` |
| hal_timer.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\hal\hal_timer.h` |
| hal_uart.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\hal\hal_uart.c` |
| hal_uart.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\hal\hal_uart.h` |
| hal_gpio.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\hal\hal_gpio.c` |
| hal_gpio.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\hal\hal_gpio.h` |
| hal_buzzer.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\hal\hal_buzzer.c` |
| hal_buzzer.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\hal\hal_buzzer.h` |
| hal_key.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\hal\hal_key.c` |
| hal_key.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\hal\hal_key.h` |

### PROTO 层 (协议解析, 纯函数)

| 文件 | 绝对路径 |
|------|---------|
| proto_modbus.c | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\proto\proto_modbus.c` |
| proto_modbus.h | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\Claude\proto\proto_modbus.h` |

---

## 六、架构与评审文档

**路径**: `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\`

| 文件 | 绝对路径 | 说明 |
|------|---------|------|
| engine-spec.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\engine-spec.md` | 引擎规格 |

### architecture/

| 文件 | 绝对路径 | 说明 |
|------|---------|------|
| 四头电磁炉架构方案.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\architecture\四头电磁炉架构方案.md` | 整体架构方案 |
| 嵌入式灯板控制系统架构说明.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\architecture\嵌入式灯板控制系统架构说明.md` | 灯板控制系统架构 |
| 项目结构程序示例.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\architecture\项目结构程序示例.md` | 项目结构示例代码 |
| json-architecture-phase-report.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\architecture\json-architecture-phase-report.md` | JSON架构阶段报告 |
| json-coverage-analysis.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\architecture\json-coverage-analysis.md` | JSON覆盖率分析 |
| hmi-json-message-test-plan.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\architecture\hmi-json-message-test-plan.md` | HMI JSON消息测试计划 |
| hardware-verification-workflow.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\architecture\hardware-verification-workflow.md` | 硬件验证工作流 |
| js-json-to-c-encoding-guide.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\architecture\js-json-to-c-encoding-guide.md` | JS→C编码转换指南 |
| two-method-display-refactor-guide.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\architecture\two-method-display-refactor-guide.md` | 双方法显示重构指南 |
| Web仿真框架 - 逻辑层JSON规则驱动实施方案 V2.0.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\architecture\Web仿真框架 - 逻辑层JSON规则驱动实施方案 V2.0.md` | Web仿真框架方案 |

### specs/

| 文件 | 绝对路径 | 说明 |
|------|---------|------|
| 四头电磁炉完整规格书 V1.0.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\specs\四头电磁炉完整规格书 V1.0.md` | 产品规格书 |
| 4头电磁炉Modbus通信协议规格书.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\specs\4头电磁炉Modbus通信协议规格书.md` | MODBUS协议规格 |
| hmi-data-flow-table.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\specs\hmi-data-flow-table.md` | HMI数据流表 |

### reviews/

| 文件 | 绝对路径 | 说明 |
|------|---------|------|
| REVIEW_LAYER_DECOUPLE.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\REVIEW_LAYER_DECOUPLE.md` | 层解耦评审 |
| REVIEW_REPORT.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\REVIEW_REPORT.md` | 评审报告v1 |
| REVIEW_REPORT_V2.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\REVIEW_REPORT_V2.md` | 评审报告v2 |
| audit-decoupled-architecture.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\audit-decoupled-architecture.md` | 解耦架构审计 |
| bug-log-hmi-select-blink.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\bug-log-hmi-select-blink.md` | HMI选中闪烁Bug日志 |
| review-v8-json-spec-methodology.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\review-v8-json-spec-methodology.md` | V8 JSON规格方法论评审 |
| review-json-灯板逻辑-审计报告.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\review-json-灯板逻辑-审计报告.md` | JSON灯板逻辑审计 |
| hmi-compliance-audit-2026-05-25.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\hmi-compliance-audit-2026-05-25.md` | HMI合规审计 |
| hmi-verification-2026-05-25.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\hmi-verification-2026-05-25.md` | HMI验证 |
| hmi-verification-final-2026-05-25.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\hmi-verification-final-2026-05-25.md` | HMI最终验证 |
| hmi-encoding-compliance-check.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\hmi-encoding-compliance-check.md` | HMI编码合规检查 |
| hmi-verification-round3-2026-05-25.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\hmi-verification-round3-2026-05-25.md` | HMI第三轮验证 |
| hmi-timer-logic-check.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\hmi-timer-logic-check.md` | HMI定时逻辑检查 |
| led-test-mode-archive.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\led-test-mode-archive.md` | LED测试模式归档 |
| audit-weak-transition-2026-05-26.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\reviews\audit-weak-transition-2026-05-26.md` | ★ __weak v2.0过渡审计报告 |

### ai-guidelines/

| 文件 | 绝对路径 | 说明 |
|------|---------|------|
| design-methodology.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\ai-guidelines\design-methodology.md` | 设计方法论 |
| AI嵌入式编程约束.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\ai-guidelines\AI嵌入式编程约束.md` | AI嵌入式编程约束 |
| AI提示词.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\ai-guidelines\AI提示词.md` | AI提示词模板 |
| AI编程规范-自检清单.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\ai-guidelines\AI编程规范-自检清单.md` | 自检清单 |
| js-to-c-conversion.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\ai-guidelines\js-to-c-conversion.md` | JS→C转换指南 |
| hmi-logic-workflow.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\ai-guidelines\hmi-logic-workflow.md` | HMI逻辑工作流 |

### references/

| 文件 | 绝对路径 | 说明 |
|------|---------|------|
| SMG_Disp_General_Lib_用户手册.md | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\docs\references\SMG_Disp_General_Lib_用户手册.md` | 段码库手册 |

---

## 七、外部资源 (非版本管理)

| 用途 | 绝对路径 |
|------|---------|
| 参考程序 (硬件配置权威来源) | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\参考程序` |
| 芯片资料 (数据手册/TRM) | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\芯片资料` |
| 半桥/全桥加热控制 | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\m4_ekf_observer` |
| MODBUS测试工具 | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\m4_ekf_observer\tools\ekf_tuner\m4_modbus_tool.py` |
| HMI JS+JSON验证程序 | `D:\OBSIDIAN\MOVING IH\低耦合程序架构\four_head\sim\test_hmi` |

---

## 八、本次里程碑变更摘要 (2026-05-26)

**里程碑 1**: 彻底消除 MsgScheduler，将所有跨模块通信转换为 __weak 回调直调。

**里程碑 2**: app_hmi.c 模块分解 — 1897 行单块 → 5 个 __weak 独立模块, WASM 双引擎 66/66 PASS。

**变更范围**:
- 10 个生产 .c 文件修改
- 16 条消息通道转换
- 34 个 Msg_Post + MsgScheduler_Register 调用消除
- 16 个 __weak 空壳函数新增
- `core/msg_scheduler.c/h` 废弃
- `core/interface_map.h` 重写为 __weak 配对格式

**新增方法论文档**:
- `methodology_weak-callback-zero-dependency.md` — __weak 架构实施方法 (核心)
- `methodology_golden-output-decomposition.md` — ★ 模块分解黄金输出验证: 禁止重新实现, 必须逐字段一致
- 本 MANIFEST.md — 完整文件清单

**更新方法论文档**:
- `methodology_ai-zero-coupling-complete.md` → v2.0
- `methodology_ai-managed-dependency-injection.md` → v2.0
- `methodology_ai-message-id-namespace.md` → deprecated

**新增项目文档**:
- `docs/reviews/AR-2026-05-26-MODULAR.md` — 验收报告 (第三方审计用)
- `docs/reviews/PHASE-CLOSURE-2026-05-26.md` — 阶段结项报告
- `docs/PROJECT-INVENTORY.md` — 项目资料清单

---

*维护: AI (技术负责人+文档管理员角色)*
*最后更新: 2026-05-26*
