# 技术栈 — M4 半桥 EKF 观测器

## 硬件平台

| 项目 | 参数 |
|------|------|
| **MCU** | RX32G410 (ARM Cortex-M4F) |
| **指令集** | ARMv7E-M + FPU (单精度硬件浮点) |
| **主频** | 384 MHz (PLL) |
| **Flash** | 待确认 |
| **SRAM** | 待确认 |
| **HRTIM** | 半桥 PWM 生成，频率寄存器 (PER/CMP/DT) |
| **ADC** | 电压/电流/相位/谐振多通道同步采样 |
| **UART2** | MODBUS RTU 物理层，DMA 收发 |

## 编译器

- **编译器**: ARMCC V5.06 (Keil MDK)
- **标准**: C99 (与 four_head 一致)
- **优化**: 待确认

## PC 工具

| 工具 | 路径 | 用途 |
|------|------|------|
| MODBUS 通讯 | `tools/ekf_tuner/m4_modbus_tool.py` | 读取遥测/下发控制/数据采集 |
| EKF 整定 | `tools/ekf_tuner/m4_gui.py` | EKF 参数可视化整定 |
| 测试运行 | `tools/ekf_tuner/run_ekf_tests.py` | 自动化测试 |

## 依赖库

- **pymodbus**: MODBUS RTU 通信 (PC 端)
- **pyserial**: 串口通信 (PC 端)
- **numpy/matplotlib**: EKF 数值计算与可视化 (PC 端，可选)
- **RX32G410_FW_HAL_V1.3N**: M4 原厂固件库 (`src/` 下)

## 固件源结构

```
m4_ekf_observer/
├── src/RX32G410_FW_HAL_V1.3N/  # 原厂固件库 (只读)
│   ├── Drivers/CMSIS/          #   CMSIS-Core CM4
│   ├── Drivers/...             #   HAL + DeviceSupport
│   └── ...                     #   启动/系统文件
├── app/                        # 应用层 (可修改)
│   ├── ekf/                    #   EKF 观测器
│   ├── pot_detect/             #   锅具检测
│   ├── var_gain_pid/           #   变增益 PID
│   └── data_logger/            #   数据记录
├── proto/                      # 协议定义
├── tools/ekf_tuner/            # PC 端工具
└── docs/                       # 文档
```

---

*最后更新: 2026-05-27*
