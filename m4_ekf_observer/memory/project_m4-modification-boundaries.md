---
name: m4-modification-boundaries
description: M4 项目修改边界 — No-Go 区域 vs 安全区域，增量开发核心约束
metadata:
  type: project
---

# M4 修改边界

## No-Go 区域（只读不写，修改需用户明确指令）

- `src/RX32G410_FW_HAL_V1.3N/` — 原厂固件库全部
- HRTIM 半桥驱动 (PER/CMP/DT 寄存器)
- ADC 采样逻辑
- MODBUS 帧解析
- IGBT/硬件保护

**Why:** 这些模块已经过硬件验证，改错可能导致功率级损坏或通讯断连。

## 安全修改区域（自由修改）

- `app/ekf/` — EKF 观测器
- `app/pot_detect/` — 锅具检测
- `app/var_gain_pid/` — 变增益 PID
- `app/data_logger/` — 数据记录
- `tools/ekf_tuner/` — PC 端 Python 工具

## MODBUS 寄存器扩展规则

- 新增只读寄存器 (0x1015+): 低风险，可自主添加
- 新增可写寄存器: 需确认不与其他从站冲突
- 修改现有寄存器含义: **禁止**，会破坏 PC 工具兼容性

## 每次修改后检查

1. 编译 0 error, 0 warning
2. 如果改了寄存器 → `m4_modbus_tool.py COM3 --read`
3. 如果改了 EKF/PID → `run_ekf_tests.py`
