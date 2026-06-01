---
name: m4-project-overview
description: M4 半桥 EKF 观测器项目 — 在已验证固件上增量开发，不是零依赖架构
metadata:
  type: project
---

# M4 半桥 EKF 观测器 — 项目概述

## 定位

在已验证的 M4 半桥固件上**增量开发** EKF 观测器 + 锅具检测 + 变增益 PID。
与 [[four_head-project]] 不同：本项目是**修改现有固件**，不构建零依赖架构。

**Why:** M4 固件已经过硬件验证，改动风险高。新代码可以错，旧代码不能破。

**How to apply:** 每次操作先确认修改在哪个区域（No-Go / 安全 / 需审批）。

## 关键参数

| 维度 | 值 |
|------|-----|
| MCU | RX32G410 (Cortex-M4F, 384MHz, FPU) |
| 编译器 | ARMCC V5.06 (Keil MDK), C99 |
| 控制方式 | PC (Python) → MODBUS RTU → M4 从站 |
| 控制周期 | 20ms (Task_TimeChip1) |
| 从站地址 | 5, 10, 15, 20 (四头) |
| 波特率 | 115200, 8N1 |

## 开发模式

```
PC 算法整定 (Python) → 验证通过 → 移植 C 到 M4
```

## 方法论关系

本项目**消费**方法论（用到再取），不强制执行 four_head 的六大铁律。
方法论是 four_head 的产出，M4 项目按需引用。
