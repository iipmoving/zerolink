---
name: m4-hrtim-master-sync-simplify
description: HRTIM MASTER 同步简化计划 — 清理 IO 触发同步残余，支持独立时钟输出
metadata:
  type: project
---

# HRTIM MASTER 同步简化计划

> 工作名称: **`hrtim-master-sync`**
> 状态: **待执行** (2026-05-27 评估完成)
> **前置条件: EKF 观测器开发完成后再启动。** (用户决策 2026-05-27)
> 目标: 用 MASTER 硬件同步替代 IO 引脚模拟同步，清理 `SYN_BY_INT` 过零检查和 preload 开关舞蹈

## 背景

原系统使用 GPIO 引脚 (PB6 = HRTIM_SCIN) 的外部事件触发同步，在 HRTIM ISR 中通过 `SYN_BY_INT` 保证过零点上管先开通。改用 MASTER 统一时钟源后，硬件自动处理同步，大量手工代码可简化。

## 评估详情

见 2026-05-27 技术负责人评估，关键发现：

- **4 个层次**的 IO 同步复杂性：GPIO toggle、SYN_BY_INT 中断状态机、preload 禁用/启用舞蹈、调用点分散
- **4 个 ISR 不需要动**：PAN(检锅)、TEST1(过流保护)、TEST2(ADC时序) 与同步机制无关
- **独立时钟**通过 `HRTIM_MCR` 的 TxCEN 位 (bits 17-22) 实现，每个 Timer 可独立启停

## 任务清单

| 编号 | 文件 | 做什么 | 依赖 | 优先级 |
|------|------|--------|------|--------|
| T001 | `API_hrtim.c` | 新增 `API_HRTIM_ConfigIndependent()` — 单头独立时钟模式，ResetTrigger=NONE, UpdateTrigger=NONE | 无 | P0 |
| T002 | `API_hrtim.c` | 清理 IO 同步残余 5 处: SyncInputSource→INTERNALEVENT, 删除 SYN_BY_INT, 简化 TIMsynchronous/TIMsynchronousPower/CHECK_PAN_PLUSE | 无 | P0 |
| T003 | `app_task.c` | 删除 `HRTIM_SYN_pin` pull-down/pull-up (L327-334) | 无 | P0 |
| T004 | `app_power.c` | 替换 `TIMsynchronousPower()` → `API_HRTIM_MasterSync_StartAll()`, 清理注释的 TIMsynchronous 调用 | T002 | P0 |
| T005 | `API_HRTIM.h` | 新增独立模式声明, 标记 deprecated | T001, T002 | P1 |

## 修改范围

```
修改: API_hrtim.c, API_HRTIM.h, app_task.c, app_power.c (4 文件)
不动: rx32g4xx_it.c, PAN/TEST1/TEST2 ISR, HAL 驱动层
```

## 风险

- 检锅起振 (`API_HRTIM_CHECK_PAN_PLUSE`) 保留 PAN 中断设置，仅删 GPIO toggle — 不影响检锅时序
- 独立模式各炉头不同频率时 MASTER 周期不匹配 → 用 `ConfigIndependent` 各头自己的 ResetTrigger=NONE
- 编译需完整 Keil 工程

**Why:** IO 同步代码分布在 4 个文件中，随 MASTER 同步引入而变为死代码。清理后代码路径更直接，也支持多炉头独立频率运行。

**How to apply:** 用户说"执行 hrtim-master-sync"时，按 T001→T002→T003→T004→T005 顺序逐任务执行，每步编译验证。
