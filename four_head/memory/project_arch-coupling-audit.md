---
name: arch-coupling-audit-complete
description: 架构耦合修复完成 — 12项违规 → 0，check_deps.py 通过
metadata:
  node_type: memory
  type: project
  originSessionId: 7eb949aa-9578-417f-ab12-d6428ae59747
---

架构耦合审查与修复已于 2026-05-26 完成。check_deps.py 扫描 44 个文件，0 违规。

**Why:** 用户发现跨层调用嫌疑。解耦是第一前提。12 项原始违规全部修复。
**How to apply:** 每次编码后 `python tools/check_deps.py Claude` → 不通过不提交。

## 修复清单

| # | 原始违规 | 修复方式 |
|---|---------|---------|
| 1 | app_comm_mgr → hal_comm | 消息驱动: APP → Msg_Post(MSG_COMM_SEND_REQ) → DRV handler → Drv_Comm_Send → HAL |
| 2 | app_power → hal_comm | 同上 |
| 3 | app_hmi → drv_key | app_hmi.h 独立声明 HmiKeyCode_t (与 drv_key.h 的 KeyCode_t 值一致但无 include 依赖) |
| 4 | app_hmi → drv_display | 移除 include，HMI_DEBUG_KEYS 内用 extern 声明替代 |
| 5 | drv_display → app_hmi | drv_display.c 独立声明 DisplayFrame_t (与 HmiDisplayCache_t 布局一致) |
| 6 | cfg/hmi_data → drv_key | cfg 使用 app_hmi.h 的 HMI_KEY_* 枚举值 |

## 新增基础设施

- `drv/drv_comm.h/c` — DRV 层封装 HAL_Comm + HAL_UART，APP 不得直接 include
- `drv/drv_comm_mgr.h/c` — DRV 侧 MSG_COMM_SEND_REQ 处理器 + 10ms TX/RX 轮询
- `core/interface_map.h` — 文档文件（禁止 include），记录跨模块结构体配对
- `core/msg_def.h` — 新增 MSG_COMM_SEND_REQ 消息 ID

## 核心原则（本次确立）

1. 每个模块独立声明跨层类型，相同布局不同命名，互不 include
2. AI 负责维护类型一致性（interface_map.h 作文档 + sizeof/offsetof 验证）
3. 通讯路径: APP → Msg_Post → DRV handler → Drv_Comm_* → HAL
4. 接收路径: HAL → Drv_Comm_* → DRV handler → Msg_Post → APP handler
