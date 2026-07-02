---
name: m4-known-issues
description: M4 固件已知待修复项 — 相位/频率/ADC 等问题清单
metadata:
  type: project
---

# M4 固件已知待修复项

1. **Phase_Position (0x100E) = 0**: 固件未填充，需从 phaseValue 或 phase_angle 读取
2. **频率未上报**: HZ_Cnt (0x100C) 是计数器非实际频率，EKF 补丁通过 0x1021-0x1022 新增频率上报
3. **电压/电流为 8-bit ADC**: 需校准系数才能换算物理单位

**Why:** 这些是固件现有问题，EKF 需要准确的相位和频率数据。在 EKF 补丁中已通过新增寄存器 0x1020-0x1022 部分解决。

**How to apply:** EKF 补丁集成后，验证 0x1021-0x1022 频率值是否合理（应在 20000-60000 Hz 范围）。
