# MODBUS 谐振参数输出接口 — 规划文档 v2

## 1. 现状链路

```
APP_ADC_CalculatePower()                              [APP_ADC.C:2635]
  │  已有: currentAdr, hrtimAdr, voltageAdr, &inputArray[potCh]
  │
  ├─ CalculatePower()         → 功率/相位/ESR           (保留)
  │
  ├─ APP_ADC_TxaMessageOut()  → 填充 MessageDef         (删除)
  │     └─ PrintMessagePush() → 拷贝到 messgeOutBuff
  │
  └─ PrintMessageOut()        → printf CSV → UART3       (删除)
```

**数据来源** (已有，不动):
- `TxaHrtimBuff[ch]` — HRTIM 时间戳数组
- `TxaAdcBuff[ch]` — 谐振电流 ADC 数组
- `TxaVcBuff[ch]` — 母线电压 ADC 数组
- `inputArray[ch]` — 周期参数 (start, end, highOff, highOn, period, ...)
- `fmacLeveNum` — FMAC 滤波级别偏移

这些数据的采集和填充 (`APP_ADC_MesageBuff`) **不需要改**。

## 2. 改造方案

### 2.1 调用链替换

```
APP_ADC_CalculatePower()                              [修改]
  │
  ├─ CalculatePower()         → 功率/相位/ESR           (保留)
  │
  └─ ResonantAnalysis_Run()   → f0, Q, L               (新增)
        │
        ├─ CalcResonantParams(current, hrtim, voltage, count,
        │                    highOff, highOn, period, &f0, &q, &l)
        │
        └─ 写结果到 MODBUS 0x4000 寄存器区
```

原有 `APP_ADC_TxaMessageOut` 和 `PrintMessageOut` 调用**移除**。

### 2.2 MODBUS 寄存器: 0x4000 区 (新增)

| 地址 | 名称 | 类型 | 字节 | 说明 |
|------|------|------|------|------|
| 0x4000 | F0_LO | uint16_t | 2 | f0 低字 (Hz) |
| 0x4001 | F0_HI | uint16_t | 2 | f0 高字 |
| 0x4002 | Q_LO | int16_t | 2 | Q 值 Q12 低字 |
| 0x4003 | Q_HI | int16_t | 2 | Q 值 Q12 高字 |
| 0x4004 | L_LO | int16_t | 2 | L_adc Q12 低字 |
| 0x4005 | L_HI | int16_t | 2 | L_adc Q12 高字 |
| 0x4006 | I_PEAK | uint16_t | 2 | 峰值电流 ADC |
| 0x4007 | FLAGS | uint16_t | 2 | bit0=f0有效 bit1=Q有效 bit2=L有效 |
| 0x4008 | F_SW | uint16_t | 2 | 开关频率 (HRTIM_CLK/period) |
| 0x4009 | CYCLE | uint16_t | 2 | 周期计数 (每周期 +1, 用于检测更新) |
| | | | **20 bytes** | |

一次 MODBUS 读 0x4000-0x4009 (10 个寄存器) 即可获取全部结果。
帧长 = 10×2 + 5 = 25 bytes，传输时间 = 25/11520 = **2.2ms**。

### 2.3 更新频率

每周期 (f_sw ≈ 30kHz → 33μs) 调用一次 `CalcResonantParams`，结果覆盖写入 0x4000 区。
Host 可以任意频率轮询 (推荐 10Hz~100Hz)，读到的总是最新值。
`CYCLE` 字段自增，Host 据此判断是否有新数据。

### 2.4 内存复用

`resonant_f0.c` 的 `CalcResonantParams` 不需要额外大缓冲 — 它直接使用
`APP_ADC_CalculatePower` 传入的 `currentAdr/hrtimAdr/voltageAdr` 指针，
这些指针指向 `APP_ADC_MesageBuff` 分配的 `Txa*Buff[]`。

原有的 `messgeOutBuff[MessageBuffSize]` (~1440 bytes) 和 `buff[2046]`
不再使用，可以留着备用。

## 3. 实施清单

| # | 文件 | 改动 | 行数估计 |
|---|------|------|----------|
| 1 | `resonant_f0.c` | 新增 `ResonantAnalysis_Run()` — 封装 CalcResonantParams 调用 + MODBUS 写入 | +25 |
| 2 | `resonant_f0.h` | 声明 `ResonantAnalysis_Run()` | +8 |
| 3 | `Modbus_Lib_Init_An_Analysis.c` | 新增 0x4000 寄存器区 + 结果缓存结构体 + 初始化 | +50 |
| 4 | `Modbus_Lib_Init_An_Analysis.h` | 新增 `ResonantResult` 结构体 + `ResonantResult_Write()` 声明 | +20 |
| 5 | `APP_ADC.C` | `APP_ADC_CalculatePower()` 中: 删除 Message 调用，插入 `ResonantAnalysis_Run()` | -15 +5 |
| | | **总计** | ~100 |

不修改 `Modbus_Analysis_Lib.c` (协议引擎不动)。

## 4. 数据流总结

```
┌─────────────────────────────────────────────────────────────┐
│ APP_ADC_CalculatePower()  每个谐振周期调用一次                 │
│                                                             │
│  CalculatePower(current, hrtim, voltage, input)              │
│    → PowerResult (功率, 相位, ESR)                           │
│                                                             │
│  ResonantAnalysis_Run(current, hrtim, voltage, count, input) │
│    → CalcResonantParams()                                   │
│      → f0 (ZC间距), Q (衰减比), L (di/dt)                     │
│    → 写入 s_resonant_result                                 │
│    → MODBUS 0x4000 区可见                                    │
└─────────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────────┐
│ MODBUS Host (PC工具 / 上位机)                                │
│                                                             │
│  读 0x4000-0x4009 (10 regs, 25 bytes, 2.2ms)               │
│    → f0, Q, L, I_peak, flags, cycle_cnt                     │
│                                                             │
│  轮询 10Hz: 25×10 = 250 bytes/s, 占用 2.2% 带宽              │
│  轮询 100Hz: 2500 bytes/s, 占用 22% 带宽                     │
└─────────────────────────────────────────────────────────────┘
```

## 5. 实施状态 (2026-05-30)

| # | 文件 | 状态 |
|---|------|------|
| 1 | `resonant_f0.h` | 新建 — `ResonantResult` 结构体 + `CalcResonantParams` + `ResonantAnalysis_Run` |
| 2 | `resonant_f0.c` | 新建 — 算法实现 + `g_resonant_result[4]` 全局变量 |
| 3 | `resonant_msg.h` | 新建 — 复制 `printMessage.h`, 类型重命名 |
| 4 | `resonant_msg.c` | 新建 — 复制 `printMessage.c`, `printf` → `CalcResonantParams` + MODBUS 写 |
| 5 | `Modbus_Lib_Init_An_Analysis.c` | **已修改** — +0x4000 只读区 (+14 行) |
| 6 | `APP_ADC.C` | **不动** — 调用方自行替换 `APP_ADC_TxaMessageOut` → `ResonantMsg_Push` |

## 6. 硬件参数 → 物理量转换

从 ADC 原始值到物理量需要以下硬件参数（在调用 `CalcResonantParams` 前换算，
或在 MATLAB 离线分析时输入）：

### 6.1 参数清单

| 参数 | 符号 | 单位 | 说明 |
|------|------|------|------|
| 电压分压比 | `Kv` | V/V | `R_bot / (R_top + R_bot)`, 典型 1/100 |
| 互感器变比 | `N_ct` | A/A | 如 100:1 |
| 采样电阻 | `R_burden` | Ω | 互感器二次侧, 典型 10-200Ω |
| 运放增益 | `G_amp` | V/V | 差分放大器增益, 典型 1 |
| ADC 参考电压 | `V_ref` | V | 典型 3.3V |
| ADC 分辨率 | — | bits | 12 bits → 4096 |
| 谐振电感 | `L_known` | μH | 线圈标称值, 用于交叉验证 |
| 谐振电容 | `C_known` | μF | 电容标称值, 典型 0.27-0.47μF |

### 6.2 换算公式

```
V_per_adc  = V_ref / 4096 / Kv              (V/ADC)
I_per_adc  = (V_ref / 4096) / (R_burden/N_ct) / G_amp   (A/ADC)

V_bus(V)      = V_adc × V_per_adc
I_resonant(A) = I_adc × I_per_adc
L(μH)         = V_bus × dt / dI × 1e6
C_calc(μF)    = 1 / ((2π·f0)² × L) × 1e6     (交叉验证)
I_peak(A)     = max(I_resonant)
I_rms(A)      = I_peak / √2
ESR(Ω)        = √(L/C) / Q                     (串联谐振)
P(W)          = V_bus × I_rms × cos(φ) × 0.5   (半桥因子)
```

### 6.3 MATLAB 计算工具

`tools/ekf_tuner/calc_physical_params.m` — 输入硬件参数 + 每周期 ADC 数据,
输出全部物理量并生成对比图表。硬件参数在脚本顶部 `HW.*` 节配置。

## 7. 确认项

- [x] 波特率 115200
- [x] 不需要原始波形输出 (0x4100 区不做)
- [x] 复用 MESSAGE 已有的 Txa*Buff 指针，不额外分配大内存
- [x] 原 PrintMessage 调用链停止执行
- [x] 硬件参数作为可配置输入，支持 MATLAB → MCU 迭代闭环
