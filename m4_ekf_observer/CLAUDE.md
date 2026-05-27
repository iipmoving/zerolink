# M4 EKF 观测器 — 半桥电磁炉负载特性在线观测与自适应控制

## 系统架构

```
PC (Python, MODBUS 主站)
  │ MODBUS RTU (USB转串口)
  ▼
M4 RX32G410 (MODBUS 从站, 站号 5/10/15/20)
  ├── 功率控制 (HRTIM 半桥, PID)
  ├── ADC 采样 (电压/电流/相位/谐振)
  ├── EKF 观测器 (待实现)
  └── 锅具判断 (待实现)
```

PC 直接通过 MODBUS RTU 控制 M4 半桥电磁炉，下发功率指令，读取遥测数据。
EKF 参数先在 PC 端离线整定，验证后下发到 M4 运行。

## MODBUS 物理层 (已确认)

| 参数 | 值 |
|------|-----|
| UART | UART2 |
| 波特率 | **115200** (Uart1BaudRate) |
| 数据位 | 8 |
| 校验 | 无 (N) |
| 停止位 | 1 |
| CRC | MODBUS CRC-16, 低字节优先 (CRC_Order=0) |
| 从站地址 | 5 (Slave1), 10 (Slave2), 15 (Slave3), 20 (Slave4) |
| 可配置 | 地址通过 0x3001, 波特率通过 0x3002 |
| RX | DMA 接收, 50字节缓冲 (DF_MB_Uart_Rx_LONG=50) |
| TX | DMA 发送 |

## MODBUS 寄存器表 (已确认, 来自 Modbus_Lib_Init_An_Analysis.c)

### 只读遥测: 0x1000-0x1014 (IH_STA_READ, 21寄存器)

| 地址 | 字段 | 类型 | 说明 |
|------|------|------|------|
| 0x1000 | Vol_AD | uint16 (8-bit有效) | 电压 ADC |
| 0x1001 | Current_AD | uint16 (8-bit有效) | 电流 ADC |
| 0x1002 | IGBT_AD | uint16 (8-bit有效) | IGBT 温度 ADC |
| 0x1003 | Bot_AD | uint16 (8-bit有效) | 炉面温度 ADC |
| 0x1004 | Fan_AD | uint16 | 风扇 ADC |
| 0x1005 | Top_AD | uint16 (8-bit有效) | 顶部温度 ADC |
| 0x1006 | Practical_Power | uint16 | 实际功率 (W) = actualPowerDiv25 × 25 |
| 0x1007 | Practical_PPG | uint16 | 实际加热 PPG 值 |
| 0x1008 | P_limited_STA | uint16 | 功率限制状态 |
| 0x1009 | Pan_pulsating | uint16 | 检锅脉冲计数 (低4位) |
| 0x100A | Resonate_Curr | uint16 | 谐振电流 ADC |
| 0x100B | HVol_Cnt | uint16 (8-bit有效) | 反压计数器 |
| 0x100C | HZ_Cnt | uint16 (8-bit有效) | 频率计数器 |
| 0x100D | Discard_Cnt | uint16 | 丢波计数器 |
| 0x100E | Phase_Position | uint16 | **相位值 (⚠️ 固件未填充, 始终=0)** |
| 0x100F | ERROR | uint16 | 故障码 (低4位) |
| 0x1010 | interior_ERR | uint16 | 内部故障 (浪涌标志) |
| 0x1011 | Version_Number | uint16 | 主板版本号 |
| 0x1012 | Equivalent_Resistance | uint16 (8-bit有效) | 等效电阻 |
| 0x1013 | _25W_Power | uint16 (8-bit有效) | 25W 修正值 |
| 0x1014 | SYS_Sta | uint16 | 系统状态 |

### 可读写控制: 0x2000-0x2014 (IH_STA_READ_WRITE, 21寄存器)

| 地址 | 字段 | 说明 |
|------|------|------|
| 0x2000 | Check_Pan_LV | 检锅强度设定 |
| 0x2001 | PPG_Max | 最大 PPG 限制 |
| 0x2002 | Pan_Power | 移锅功率 |
| ... | ... | ... |
| 0x200E | Work_STA | **工作状态 (0x10=启动, 0=停止)** |
| 0x200F | FAN_Speed | 风扇转速 |
| 0x2010 | target_Power | **目标功率 (单位: 25W, 例: 40=1000W)** |
| 0x2011 | intermittent_Heat | 间断加热 |
| 0x2012 | jitter_frequency | 抖频参数 |
| 0x2013 | BuzzCof | 蜂鸣器控制 |
| 0x2014 | syntony_Current_Short | 短路保护 |

### 可读写系统设置: 0x3000-0x3003 (IH_STA_READ_WRITE_SYS_SET, 4寄存器)

| 地址 | 字段 | 说明 |
|------|------|------|
| 0x3000 | Power_Calibration | 功率校准值 (36-96) |
| 0x3001 | Slave_Addr | 从机地址 |
| 0x3002 | Baud_rate_SET | 波特率设置 |
| 0x3003 | Save_order | 保存命令 (写1保存) |

## 数据流 (控制周期 20ms)

```
20ms 控制周期 (Task_TimeChip1 → PowerTypeFun → PowerControlFun):
  1. PPGgetAdcValue()      读取: VoltageValue(8-bit ADC), CurrentValue(8-bit ADC),
                                powerPhase(原始ADC), powerAdcFactTxa(谐振功率)
  2. s_ppg_fun()            实际功率 = V_ADC × I_ADC / g_p25_ad (以25W为单位)
                            调用 FixedPID_Compute(target, actual) → delta_ppg 步长
  3. i_ppg_control()        应用 PPG 步长到 HRTIM PER/CMP 寄存器
                            f = 384MHz / prioed

MODBUS 数据同步:
  Update_Static_Register_DATA() → 从 COMM_RUN 结构体填充 0x1000-0x1014 寄存器
```

## 待修复的 M4 固件问题

1. **Phase_Position (0x100E) 始终为 0**: `Update_Static_Register_DATA()` 中 `_0x1000->Phase_Position=0x00`
   - 数据源: `PowerControl->staticReg->phaseValue` (原始ADC) 或 `PhaseController.phase_angle` (0.1°单位)
   - 修复: 从 phaseValue 或 phase_angle 读取并写入寄存器

2. **频率未上报**: HZ_Cnt (0x100C) 是8-bit计数器, 不是实际频率
   - 数据源: `API_PPG_getValue(ch).prioed` → `f_hz = 384000000 / prioed`
   - 建议新增寄存器: 0x1015-0x1016 (HRTIM频率, Float×2 或 uint32 Hz)

3. **电压/电流为8-bit ADC 原始值**: 无法直接换算为实际 V/A
   - 需要校准系数才能转换为物理单位
   - 功率 (0x1006) 已经是 W 单位, 可直接使用

## PC 工具

`tools/ekf_tuner/m4_modbus_tool.py` — MODBUS 通信与数据采集

```bash
pip install pymodbus pyserial
python m4_modbus_tool.py COM3           # 交互模式
python m4_modbus_tool.py COM3 --read     # 单次读取
python m4_modbus_tool.py COM3 --log 30   # 记录30秒
python m4_modbus_tool.py COM3 --power 1000 --on  # 设定1000W并启动
```

## 编码约定

- C 代码风格与 four_head 保持一致
- EKF 矩阵运算显式展开, 不依赖矩阵库
- 中文注释, M4 FPU 硬件指令可用
- 所有浮点分母加 1e-9f 防除零, 协方差对角线非负保护
