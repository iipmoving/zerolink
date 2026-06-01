# resonant_msg 模块 — MESSAGE 配置说明

## 1. 概述

`resonant_msg.c/h` 是 `printMessage.c/h` 的复制品，唯一的区别是：

| | printMessage | resonant_msg |
|---|---|---|
| 输出方式 | `printf` CSV → UART3 (阻塞) | `CalcResonantParams` → `g_resonant_result[]` → MODBUS 0x4000 |
| 阻塞 | 是 (printf 耗时) | 否 (MODBUS DMA 中断响应) |
| 接口 | `PrintMessagePush` / `PrintMessageOut` | `ResonantMsg_Push` / `ResonantMsg_Out` |

其余结构（Push/Pop 机制、buff 管理、lock 机制）完全相同。

## 2. 数据流

```
APP_ADC_CalculatePower()      [调用方 — 每周期触发一次]
  │
  ├─ 配置 MessageDef:
  │    array[0] = TxaHrtimBuff   → HRTIM 时间戳
  │    array[1] = TxaVcBuff      → 母线电压 ADC
  │    array[2] = TxaAdcBuff     → 谐振电流 ADC (原始)
  │    array[3] = TxaFmacBuff    → FMAC 滤波电流
  │    paraArray = {highOff, highOn, period, ...}
  │
  ├─ ResonantMsg_Push(message)   → 拷贝到 resonant_outBuff
  │
  └─ ResonantMsg_Out()           → CalcResonantParams → g_resonant_result[]
                                         │
                                         ▼
                                  MODBUS 0x4000 区 (Host 轮询)
```

## 3. array[] 索引定义

`RESONANT_MSG_BUFF_SIZE = 8`，索引固定：

| 索引 | 数据源 | 长度 | 用途 |
|------|--------|------|------|
| 0 | `TxaHrtimBuff[PotChWork]` | 整周期 | HRTIM 时间戳 → f0 计算 |
| 1 | `TxaVcBuff[PotChWork]` | 整周期 | 母线电压 → L 计算 |
| 2 | `TxaAdcBuff[PotChWork]` | 整周期 | 原始 ADC 电流 (备用) |
| 3 | `TxaFmacBuff[PotChWork]` | 整周期 | FMAC 滤波电流 → f0/Q/L 主输入 |
| 4 | `AdcFromApiDma20ms.voltage[]` | 20 | 电压历史 (短) |
| 5 | `AdcFromApiDma20ms.phase[]` | 20 | 相位历史 (短) |
| 6 | `AdcFromApiDma20ms.Power[]` | 20 | 功率历史 (短) |
| 7 | `AdcFromApiDma20ms.Txa[]` | 20 | 电流历史 (短) |

**谐振分析只用 array[0-3]**（长数组），array[4-7] 保留不用。

## 4. paraArray 配置

paraArray 是可配置的 `uint16_t` 参数数组。调用方在 Push 前设置 `buff` 指针和 `size`。

### 当前配置 (size=4)

| 索引 | 值 | 来源 |
|------|-----|------|
| 0 | `highOff` | `AdcFromApiDma20ms.ceilQ[PotChWork][count]` |
| 1 | phase | `AdcFromApiDma20ms.phase[PotChWork][count]` |
| 2 | zc_high | `AdcFromApiDma20ms.phaseValue[PotChWork][count]` |
| 3 | power | `arrayPoint[PotChWork]` |

### 扩展配置 (size=6, 推荐)

要启用完整的 f0/Q/L 计算，需添加 `highOn` 和 `period`：

```c
uint16_t para[6];
para[0] = AdcFromApiDma20ms.ceilQ[PotChWork][count];   // highOff
para[1] = AdcFromApiDma20ms.phase[PotChWork][count];    // phase
para[2] = AdcFromApiDma20ms.phaseValue[PotChWork][count]; // zc_high
para[3] = arrayPoint[PotChWork];                         // power
para[4] = inputCh->highOn;                               // ★ 上管开通 HRTIM 值
para[5] = inputCh->lowOff;                               // ★ PWM 周期 HRTIM ticks

message.paraArray.buff = para;
message.paraArray.size = 6;     // ← 改为 6
```

### 自动回退

当 `paraArray.size <= 4` 时（para[4]/para[5] 不存在）：

- `highOn` = 0 → Q 计算可能无效（flags bit1=0）
- `period` = 自动推算（max HRTIM 值 + 1）→ f_sw 近似值

## 5. 调用方式

与原有 printMessage 调用完全一致，替换两行即可：

```c
// 原代码:
APP_ADC_TxaMessageOut(inputArray);  // 配置 MessageDef + PrintMessagePush
if (PrintMessageOut()) {}           // 阻塞 printf 输出

// 替换为:
ResonantMsg_Push(message);          // 同上 (拷贝数据)
ResonantMsg_Out();                  // 非阻塞 → 写 MODBUS 寄存器
```

建议在原有 `MessageCnt` 节流逻辑处替换，保持 ~5000 周期一次的触发频率：

```c
static uint16_t s_tick = 0;
s_tick++;
if (s_tick >= 600) {       // 600 周期 ≈ 20ms @30kHz
    s_tick = 0;
    ResonantMsg_Push(message);
    ResonantMsg_Out();
}
```

## 6. MODBUS 0x4000 寄存器 (输出)

| 地址 | 名称 | 类型 | 说明 |
|------|------|------|------|
| 0x4000 | F0_LO | uint16 | f0 低字 (Hz) |
| 0x4001 | F0_HI | uint16 | f0 高字 |
| 0x4002 | Q_LO | int16 | Q 值 Q12 低字 |
| 0x4003 | Q_HI | int16 | Q 值 Q12 高字 |
| 0x4004 | L_LO | int16 | L_adc Q12 低字 |
| 0x4005 | L_HI | int16 | L_adc Q12 高字 |
| 0x4006 | I_PEAK | uint16 | 峰值电流 ADC |
| 0x4007 | FLAGS | uint16 | bit0=f0 bit1=Q bit2=L |
| 0x4008 | F_SW | uint16 | 开关频率 Hz |
| 0x4009 | CYCLE | uint16 | 更新计数 (自增) |

一次读 0x4000-0x4009 (10 regs, 25 bytes 帧, 2.2ms @115200)。

## 7. 文件清单

| 文件 | 作用 | 状态 |
|------|------|------|
| `resonant_f0.h` | `ResonantResult` 结构体 + `CalcResonantParams` 声明 | 新增 |
| `resonant_f0.c` | `CalcResonantParams` 实现 + `g_resonant_result[]` | 新增 |
| `resonant_msg.h` | `ResonantMsgDef` + Push/Out API (复制 printMessage.h) | 新增 |
| `resonant_msg.c` | Push/Pop/Out 实现 (printf → MODBUS) | 新增 |
| `Modbus_Lib_Init_An_Analysis.c` | 0x4000 寄存器区注册 (+14 行) | 修改 |
