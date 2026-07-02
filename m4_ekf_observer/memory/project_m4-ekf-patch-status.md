---
name: m4-ekf-patch-status
description: EKF 遥测寄存器补丁当前状态 — modbus_ekf_regs 已创建，尚未集成到主固件
metadata:
  type: project
---

# EKF 遥测寄存器补丁状态

## 已完成

- `app/ekf/modbus_ekf_regs.h` — EKF 遥测寄存器结构体 + 接口声明
- `app/ekf/modbus_ekf_regs.c` — EKF 数据刷新实现
- `app/ekf/FIRMWARE_PATCH.md` — 详细补丁说明文档

## 待完成（需用户提供完整固件源文件后执行）

FIRMWARE_PATCH.md 描述的 5 处修改：

1. `Modbus_Lib_Init_An_Analysis.c` 添加 `#include "modbus_ekf_regs.h"`
2. ARM 区域数量宏: 3 → 4 (四个从机各加一个 ARM 区域)
3. 四个从机的 `Modbus_Cofg_Init_SET()` 追加第 4 区域 (0x1020-0x102A)
4. `Modbus_Protocol_Analysis_Main()` 中每个从机调用 `EKF_Regs_Update()`
5. `app_power.c` 覆盖 `API_POWER_EKF_GetTelemetry()` 弱回调

## EKF 遥测寄存器布局 (0x1020-0x1029)

| 地址 | 字段 | 说明 |
|------|------|------|
| 0x1020 | Phase_Angle | 相位值 |
| 0x1021 | Freq_Hz_Hi | 频率高 16 位 |
| 0x1022 | Freq_Hz_Lo | 频率低 16 位 |
| 0x1023 | PPG_Period | PPG 周期 |
| 0x1024 | PPG_Duty | PPG 占空比 |
| 0x1025 | Delta_PPG | PID 增量 (TODO) |
| 0x1026 | Resonant_Curr | 谐振电流 (TODO) |
| 0x1027-0x1029 | Reserved | 预留 |

## 验证方法

```bash
python tools/ekf_tuner/m4_modbus_tool.py COM3 --read-ekf
```

**Why:** EKF 观测器需要从固件读取相位、频率等遥测数据才能在 PC 端整定。补丁以最小侵入方式新增独立的只读寄存器区域。

**How to apply:** 用户提供完整固件源文件后，按 FIRMWARE_PATCH.md 步骤执行 5 处修改。不要提前执行。
