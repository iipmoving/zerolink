# M4 半桥电磁炉 MODBUS RTU 通讯协议

> 基于 I2C 版协议 `双核IH控制器通讯协议(完整版XX70)V1.1` 改写为 MODBUS RTU 版本。
> 寄存器顺序严格遵循 I2C 协议的读状态顺序 (0x10-0x1F)，再追加 MODBUS 扩展寄存器。

---

## 一、物理层

| 项目 | 参数 |
|------|------|
| 接口 | UART (TTL) |
| 波特率 | 57600 bps |
| 数据位 | 8 |
| 校验位 | None |
| 停止位 | 1 |
| 协议 | MODBUS RTU (Master-Slave) |
| CRC | CRC-16-MODBUS, 多项式 0xA001, **低字节在前** |

## 二、从机地址

| 炉头 | 地址 | 说明 |
|------|------|------|
| Zone 1 | 0x05 | 炉头1 |
| Zone 2 | 0x0A (10) | 炉头2 |
| Zone 3 | 0x0F (15) | 炉头3 |
| Zone 4 | 0x14 (20) | 炉头4 |

支持的功能码: **0x03** (读保持寄存器), **0x06** (写单个寄存器), **0x10** (写多个寄存器)

## 三、寄存器地图总览

| 区域 | 起始地址 | 数量 | 读写 | 说明 |
|------|----------|------|------|------|
| 0x1000 | 4096 | 21 | R | 运行状态 (I2C 0x10-0x1F 映射 + MODBUS 扩展) |
| 0x2000 | 8192 | 21 | R/W | 工作参数+控制命令 (初始化参数+功率/风机/抖频) |
| 0x3000 | 12288 | 4 | R/W | 系统设置 (校准/地址/波特率/保存) |

---

## 四、0x1000 区域: 只读状态寄存器 (21 x uint16)

> **分区**: 前 9 个为常用轮询区 (每次心跳一起读), 后 12 个为调试区 (测试时按需读)。
> **顺序**: 0x1000-0x100F 严格对应 I2C 读状态顺序 (0x10-0x1F), 0x1010-0x1014 为 MODBUS 扩展。

### 4.1 常用轮询区 (0x1000-0x1008): 每次 FC03 必读

| 偏移 | 地址 | I2C 对应 | 名称 | 说明 | 单位 |
|------|------|----------|------|------|------|
| 0 | 0x1000 | 0x10 IHStatus | SYS_Sta | **系统状态**: BIT7=初始化完成, BIT6=功率稳定, BIT5=功率平衡, BIT4=有锅, BIT3-0=炉头号 | 见 §6.2 |
| 1 | 0x1001 | 0x11 VoltageValue | Vol_AD | 电压 AD 值 | — |
| 2 | 0x1002 | 0x12 CurrentValue | Current_AD | 电流 AD 值 | — |
| 3 | 0x1003 | 0x13 Sensor1Value | IGBT_AD | IGBT 温度传感器 AD | — |
| 4 | 0x1004 | 0x14 Sensor2Value | Bot_AD | 炉面(底部)温度 AD | — |
| 5 | 0x1005 | 0x15 Sensor3Value | Top_AD | 顶部温度 AD (预留) | — |
| 6 | 0x1006 | 0x16 ActualPower | Practical_Power | 实际输出功率 | W (= 原始值×25) |
| 7 | 0x1007 | 0x17 TargetPower | target_Power | 目标功率回读 | ×25W |
| 8 | 0x1008 | 0x18 ActualPPG | Practical_PPG | 实际加热 PPG 值 | — |

### 4.2 调试区 (0x1009-0x1014): 按需读取

| 偏移 | 地址 | I2C 对应 | 名称 | 说明 | 单位 |
|------|------|----------|------|------|------|
| 9 | 0x1009 | 0x19 PowerStatus | P_limited_STA | 功率限制状态 (低 nibble) | 见 §6.1 |
| 10 | 0x100A | 0x1A LoadValue | Pan_pulsating | 检锅脉冲计数 (低4位) + 浪涌标志 (高4位) | — |
| 11 | 0x100B | 0x1B VCNTValue | HVol_Cnt | 反压计数器 | — |
| 12 | 0x100C | 0x1C PWMValue_L | PWMValue_L | 频率限制值低位 (预留) | — |
| 13 | 0x100D | 0x1D PWMValue_H | PWMValue_H | 频率限制值高位 (预留) | — |
| 14 | 0x100E | 0x1E PowerAdjust | PowerAdjust | PPG 修正值 (预留) | — |
| 15 | 0x100F | 0x1F Version | Version_Number | 主板固件版本号 | — |
| 16 | 0x1010 | (MODBUS 扩展) | Fan_AD | 风扇 AD | — |
| 17 | 0x1011 | (MODBUS 扩展) | ERROR | 故障码 (0=正常) | 见 §7 |
| 18 | 0x1012 | (MODBUS 扩展) | interior_ERR | 内部故障码 | 见 §7 |
| 19 | 0x1013 | (MODBUS 扩展) | HZ_Cnt | 频率计数器 | — |
| 20 | 0x1014 | (MODBUS 扩展) | Discard_Cnt | 丢波计数器 | — |

---

## 五、0x2000 区域: 读写参数/控制寄存器 (21 x uint16)

### 5.1 初始化参数 (0x2000-0x200D + 0x2014)

| 偏移 | 地址 | 名称 | 说明 | 范围 |
|------|------|------|------|------|
| 0 | 0x2000 | CheckPanLV | 检锅强度: 高8bit=间隔时间(s), 低8bit=检锅强度 | 0-0xFFFF |
| 1 | 0x2001 | PPG_Max | 最大 PPG 限制 | — |
| 2 | 0x2002 | PanPower | 移锅功率阈值 | ×25W |
| 3 | 0x2003 | HVolLimited | 反压限制 | — |
| 4 | 0x2004 | LoadCurrent | 负载有效电流 | — |
| 5 | 0x2005 | CurrentCal | 电流修正系数 | — |
| 6 | 0x2006 | PowerMIX | 最小连续功率 | ×25W |
| 7 | 0x2007 | PowerMAX | 最大连续功率 | ×25W |
| 8 | 0x2008 | WrongPan | 恶劣锅具保护功率 | ×25W |
| 9 | 0x2009 | SyntonyCurrent | 谐振电流保护值 (高 nibble) | — |
| 10 | 0x200A | PhasePan | 移锅相位 (高 nibble) | — |
| 11 | 0x200B | PhaseMix | 最小相位: 高6bit=最小相位, 低2bit=低电压电流限制 | — |
| 12 | 0x200C | SteelCal | 钢铁锅修正: 高4bit=delta from min phase, 低4bit=钢铁限制差 | — |
| 13 | 0x200D | NPanSyntonyC | 移锅谐振电流限制 | — |
| 20 | 0x2014 | SyntonyShort | 短路保护谐振电流 (高 nibble) | — |

### 5.2 控制参数 (0x200E-0x2013) — 心跳包下发此4个寄存器

| 偏移 | 地址 | 名称 | 说明 |
|------|------|------|------|
| 14 | **0x200E** | **Work_STA** | **工作状态控制字** (见 §6.3) |
| 15 | 0x200F | FAN_Speed | 风扇转速: 0-4 档, 0=停转 |
| 16 | 0x2010 | target_Power | 目标功率 (原码为 W/25, 例 40=1000W) |
| 17 | 0x2011 | IntermittentHeat | 间断加热: 高8bit=加热时间(0.1s), 低8bit=停止时间(0.1s) |
| 18 | 0x2012 | jitter_freq | 抖频参数 (对应 I2C 协议的 powerSwitch 字节) |
| 19 | 0x2013 | BuzzCof | 蜂鸣器控制 |

---

## 六、关键位域定义

### 6.1 功率限制状态 (0x1009 P_limited_STA, 低 nibble)

| Bit | 名称 | 说明 |
|-----|------|------|
| BIT0 | B_PPGMAX | 频率超限 |
| BIT1 | B_VCOUT | 电流超限 |
| BIT2 | B_LOWV | 低电压电流限制 |
| BIT3 | B_PWDEAD | 功率死区 |

### 6.2 系统状态 (0x1000 SYS_Sta / ihStatus)

| Bit | 宏 | 说明 |
|-----|-----|------|
| BIT7 (0x80) | B_INIT_SUC_FLAG | **初始化成功标志**: 0=未初始化, 1=已完成初始化 |
| BIT6 (0x40) | B_POW_STB_FLAG | 功率稳定标志: 0=未稳定, 1=稳定 |
| BIT5 (0x20) | B_POW_ARRIVE_FLAG | 功率到达平衡标志 (启动阶段) |
| BIT4 (0x10) | B_PAN_ADJ_FLAG | 锅具存在标志: 0=有锅, 1=无锅 |
| BIT3-0 | — | **炉头号指示** (1=炉头1, 2=炉头2, 3=炉头3, 4=炉头4) |

### 6.3 工作状态控制字 (0x200E Work_STA)

此寄存器映射到 I2C 协议的 `powerControlSet` 字节，使用 **4-bit 反码校验**:

```
高 nibble = ~低 nibble (按位取反)
```

例: 若低 4bit=0x05, 则完整字节应为 0xA5。

当 BIT15 (0x8000) 置位时，触发功率校准模式请求。

---

## 七、故障码

### 故障码 (0x1011 ERROR)

| 故障码 | 名称 | 说明 |
|--------|------|------|
| 0x00 | 正常 | 无故障 |
| 0x01 | E0 | 硬件故障 |
| 0x02 | E1 | IGBT 过温 |
| 0x03 | E2 | 功率过压 |
| 0x04 | E3 | 功率欠压 |
| 0x05 | E4 | 顶部传感器开路 |
| 0x06 | E5 | 顶部传感器短路 |
| 0x07 | E6 | 底部传感器开路 |
| 0x08 | E7 | 底部传感器短路 |
| 0x0A | E10 | 无锅 |

### 内部故障码 (0x1012 interior_ERR)

| 值 | 宏 | 说明 |
|----|-----|------|
| 0x01 | PowerOffSurge | 浪涌停机 |
| 0x02 | PowerOffCommLost | **通讯超时保护停机** |
| 0x11 | PowerOffNoPan | 无锅停机 |
| 0x12 | PowerOffZero | 零功率停机 |
| 0x13 | PowerOffCheckPan | 中途检锅停机 |

---

## 八、数据格式

### 8.1 字节序

| 层级 | 字节序 |
|------|--------|
| 寄存器数据在总线上 | **大端** (高字节在前) |
| CRC 在总线上 | **小端** (低字节在前) |
| 寄存器地址 | **大端** (高字节在前) |

### 8.2 支持的功能码

| 功能码 | 名称 | 请求范围 |
|--------|------|----------|
| 0x03 | 读保持寄存器 | 1-125 个寄存器 |
| 0x06 | 写单个寄存器 | 1 个寄存器 |
| 0x10 | 写多个寄存器 | 1-123 个寄存器 |

---

## 九、通讯流程

### 9.1 上电初始化流程

```
┌──────────┐                              ┌──────────┐
│  Master  │                              │  Slave   │
└────┬─────┘                              └────┬─────┘
     │                                         │
     │  [Slave 上电, SYS_Sta.bit7 = 0]         │
     │                                         │
     │  FC03 读 0x1000, 9 regs (常用轮询区)    │
     │ ──────────────────────────────────────> │
     │  ← SYS_Sta = 0x01 (炉头1, 未初始化)     │
     │                                         │
     │  FC10 写全部 0x2000 初始化参数          │
     │  (CheckPanLV, PPG_Max, PanPower, ...)   │
     │ ──────────────────────────────────────> │
     │                                         │── API_UART_RxInitCallback()
     │                                         │   参数写入 IH 硬件
     │                                         │
     │  [主机持续轮询, 直到 SYS_Sta.bit7=1]    │
     │                                         │
     │  FC03 读 0x1000, 9 regs                 │
     │ ──────────────────────────────────────> │
     │  ← SYS_Sta = 0x81 (炉头1, 初始化完成)   │
     │                                         │
     │  [进入心跳维持阶段]                      │
```

### 9.2 心跳周期 (每次 ~500ms)

每个心跳周期做两件事: **读状态** + **写控制**。

**第一步 — 读常用轮询区 (FC03)**:

```
FC03 读 0x1000, 9 regs → 18 bytes data:

Byte 0-1:  SYS_Sta (0x1000)         — 系统状态 + 炉头号
Byte 2-3:  Vol_AD (0x1001)          — 电压 AD
Byte 4-5:  Current_AD (0x1002)      — 电流 AD
Byte 6-7:  IGBT_AD (0x1003)         — IGBT 温度 AD
Byte 8-9:  Bot_AD (0x1004)          — 炉面温度 AD
Byte 10-11: Top_AD (0x1005)         — 顶部温度 AD (预留)
Byte 12-13: Practical_Power (0x1006) — 实际功率 (×25W)
Byte 14-15: target_Power (0x1007)   — 目标功率回读
Byte 16-17: Practical_PPG (0x1008)  — 实际 PPG
```

**第二步 — 写控制寄存器 (FC10)**:

```
FC10 写 0x200E, 5 regs → 10 bytes data:

Byte 0-1:  Work_STA (0x200E)       — 工作状态控制字
Byte 2-3:  FAN_Speed (0x200F)      — 风扇转速
Byte 4-5:  target_Power (0x2010)   — 目标功率 (W/25)
Byte 6-7:  IntermittentHeat (0x2011) — 间断加热 (心跳中写 0=连续加热)
Byte 8-9:  jitter_freq (0x2012)    — 抖频参数
```

- **超时后果**: 从机在约 500ms-1s 无心跳后触发 `PowerOffCommLost` 自动保护停机

**注意**: 写地址连续 (0x200E-0x2012, 5 个寄存器)，与 `PowerControlDef` 结构体的字段顺序对应:
```
{powerControlSet, fanSpeed, powerSetm, intermittentHeat, powerSwitch}
     0x200E         0x200F     0x2010       0x2011          0x2012
```

### 9.3 功率调节流程

```
┌──────────┐                              ┌──────────┐
│  Master  │                              │  Slave   │
└────┬─────┘                              └────┬─────┘
     │                                         │
     │  [当前: 心跳中, Work_STA=0xA5(工作),    │
     │   target_Power=40(1000W)]               │
     │                                         │
     │  调节功率到 2000W:                       │
     │  FC10 写: target_Power=80 (2000/25)     │
     │ ──────────────────────────────────────> │
     │                                         │── API_UART_RxControlCallback()
     │                                         │   更新目标功率
     │                                         │
     │  [下次心跳自动带新功率值]                 │
```

### 9.4 关机流程

```
     │  FC10 写: Work_STA=0xA0 (低4bit=0, 停止) │
     │           target_Power=0                  │
     │ ──────────────────────────────────────> │
     │                                         │── 停止加热
     │                                         │
     │  [停止心跳, 或心跳带功率=0]              │
```

---

## 十、与 I2C 协议的对应关系

### 10.1 读状态寄存器映射 (0x1000-0x100F ↔ I2C 0x10-0x1F)

| I2C 地址 | I2C 名称 | MODBUS 地址 | MODBUS 名称 | 说明 |
|----------|----------|-------------|-------------|------|
| 0x10 | IHStatus | 0x1000 | SYS_Sta | 系统状态 (BIT3-0=炉头号) |
| 0x11 | VoltageValue | 0x1001 | Vol_AD | 电压 AD |
| 0x12 | CurrentValue | 0x1002 | Current_AD | 电流 AD |
| 0x13 | Sensor1Value | 0x1003 | IGBT_AD | IGBT 温度 AD |
| 0x14 | Sensor2Value | 0x1004 | Bot_AD | 炉面温度 AD |
| 0x15 | Sensor3Value | 0x1005 | Top_AD | 顶部温度 AD (预留) |
| 0x16 | ActualPower | 0x1006 | Practical_Power | 实际功率 |
| 0x17 | TargetPower | 0x1007 | target_Power | 目标功率回读 |
| 0x18 | ActualPPG | 0x1008 | Practical_PPG | 实际 PPG |
| 0x19 | PowerStatus | 0x1009 | P_limited_STA | 功率限制状态 |
| 0x1A | LoadValue | 0x100A | Pan_pulsating | 检锅脉冲 + 浪涌 |
| 0x1B | VCNTValue | 0x100B | HVol_Cnt | 反压计数器 |
| 0x1C | PWMValue_L | 0x100C | PWMValue_L | 频率限制值低位 (预留) |
| 0x1D | PWMValue_H | 0x100D | PWMValue_H | 频率限制值高位 (预留) |
| 0x1E | PowerAdjust | 0x100E | PowerAdjust | PPG 修正值 (预留) |
| 0x1F | Version | 0x100F | Version_Number | 版本号 |

### 10.2 控制寄存器映射 (0x200E-0x2012 ↔ I2C 控制帧)

| I2C 字节 | 名称 | MODBUS 寄存器 |
|----------|------|---------------|
| I2cCammandByte | 命令字节 | (MODBUS 无对应, 功能码替代) |
| I2cStatusByte | 状态控制字节 (4bit反码) | 0x200E Work_STA 低字节 |
| I2cPowerSwitchByte | 功率开关/抖频 | 0x2012 jitter_freq |
| I2cPowerSetMByte | 功率设定值 | 0x2010 target_Power |
| I2cFanSpeedMByte | 风机转速 | 0x200F FAN_Speed |
| I2cKvalueByte | K 值 | (当前代码固定 0x00) |

**核心差异**: MODBUS 版将 I2C 的 5 字节控制帧拆分为独立寄存器，通过 FC10 批量写完成同等工作。

### 10.3 MODBUS 扩展寄存器 (0x1010-0x1014)

以下寄存器没有对应的 I2C 字段，是 MODBUS 版本新增:

| 地址 | 名称 | 来源 |
|------|------|------|
| 0x1010 | Fan_AD | 风扇 AD 采样 |
| 0x1011 | ERROR | 故障码 (从 ihStatus 中分离, 见 §7) |
| 0x1012 | interior_ERR | 内部故障码 (从 LoadValue 高4位浪涌标志分离, 见 §7) |
| 0x1013 | HZ_Cnt | 频率计数器 |
| 0x1014 | Discard_Cnt | 丢波计数器 |

---

## 十一、0x3000 系统设置区域

| 偏移 | 地址 | 名称 | 说明 |
|------|------|------|------|
| 0 | 0x3000 | PowerCalibration | 功率校准值 (范围 36-96) |
| 1 | 0x3001 | SlaveAddr | 从机地址设置 (1-255) |
| 2 | 0x3002 | BaudRateSET | 波特率设置 (写 10 倍值, 如 57600→5760) |
| 3 | 0x3003 | SaveOrder | 目标炉头选择 (1-8) |

**保存命令**: 向 0x3550 (13648) 写入触发保存到 EEPROM。

---

*文档版本: V2.0*
*更新日期: 2026-05-26*
*基于: M4 半桥控制源程序 + I2C 双核IH控制器通讯协议 V1.1*
*V2.0 变更: 0x1000 区域寄存器顺序严格对齐 I2C 读状态顺序 (0x10-0x1F)*
