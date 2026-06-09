# APP_ADC.C 调用路径与 app_power 关联分析

**文件**: `LIB/APP/APP_ADC.C` — 2862 行, 15 个 #include  
**分析**: 2026-06-04

---

## 一、调用入口 (55 个非 static 函数)

### 分组总表

| 分组           | 函数数 | 说明                              |
| ------------ | --- | ------------------------------- |
| 1ms 电源计算     | 14  | ADC → DMA → FMAC → 峰值 → 积分 → 输出 |
| 20ms 汇总      | 5   | 累加平均 → 20ms 结果                  |
| DMA/HRTIM 恢复 | 11  | DMA 异常恢复、实例保存/恢复                |
| 中断回调         | 10  | HRTIM/ADC/DMA 中断入口              |
| 温度/风扇/面板     | 3   | ADC 通道切换、温度读取                   |
| 消息队列         | 2   | PrintMessage/WaveCapture 输出     |
| 调试/工具        | 10  | FMAC 回调、定时结束等                   |

> **关键发现**: 55 个入口中 **0 个 static**，全部对外公开。大量函数应设为 static 以减少耦合。

---

## 二、主要调用路径

### 路径 1: 1ms 电源计算 (主路径)

```
外部触发: API_DMA_TxA_IRQHandlerCallBack()
    → APP_ADC_TxaBuffChange()          // 缓冲切换
    → API_ADC_PENDV_IRQHandler()       // 等转换完成
    → API_FMAC_AppAdcOverCallBack()    // FMAC 完成
    → APP_ADC_TimDmaEnd()              // 定标结束
    → APP_ADC_CalculatePower()         // ← 主计算入口
        ├─ CalculatePower()              → power_calculator.h
        │    └─ 内部: CalculateAuctalCurrent() → 电流积分 + 峰值
        │       ├─ hrtim_values, resonant_current → DMA 缓冲区
        │       └─ PowerCalculatorInputDef → 输入参数
        ├─ memcpy → powerResult20ms[]   → IH_CycleDataDef 保存
        ├─ AdcFromApiDma20ms.xxx[] 写入 → app_power 数据桥
        ├─ APP_ADC_DebugValueCallBack()
        ├─ WaveCapture_IsReady()        → wave_capture.h
        ├─ APP_ADC_TxaMessageOut()
        │    ├─ PrintMessagePush()      → printMessage.h
        │    ├─ WaveCapture_PushMessage() → wave_capture.h
        │    └─ API_GPIO_WritePin()     → API_gpio.h
        └─ PrintMessageOut()            → printMessage.h
```

### 路径 2: 20ms 汇总

```
外部触发: APP_ADC_AVG_Fun()           // 20ms 定时
    ├─ APP_ADC_TxaAvgSum() ×4         // 4 路累加
    ├─ ME_UDIV()                       // 除法
    └─ AdcFromApiDma20ms 读取         // 温度、电压、电流等
```

### 路径 3: DMA 恢复

```
API_DMA_RecoverTxa() / RecoverFun() / RecoverPan()
    → API_DMA_RECOVER / API_DMA_REST
    → API_HRTIM_GetAddressTxaCnt()     → API_hrtim.h
    → APP_POWER_GetPanDmaBuffAddress() → app_power.h
```

---

## 三、app_power.h 关联

### 3.1 跨层引用说明

`APP_ADC.C` 所在路径: `LIB/APP/`
`app_power.h` 所在路径: `APP/POWER/inc/`

代码注释明确列为 **FIXME — APP→APP 跨层引用**，待改为 `__weak` 回调解耦。

### 3.2 调用的 app_power 函数

| 函数 | 在 APP_ADC.C 调用位置 | 用途 |
|------|----------------------|------|
| `APP_POWER_SetTxaAwdValue()` | 第 1024, 1264 行 | 设置 ADC WatchDog 阈值 (谐振电流保护) |
| `APP_POWER_GetPanDmaBuffAddress()` | 第 1899 行 | DMA 恢复时获取 PAN ADC 缓冲区地址 |
| `APP_POWER_CycleChange()` | 第 835 行 (注释掉) | 同频/倍频切换 |
| `APP_POWER_CycleReset()` | 第 840 行 (注释掉) | 恢复倍频 |

### 3.3 访问的 app_power 全局变量

**`AdcFromApiDma20ms`** (类型 `APP_ADC_AVG_BUFF_DEF`):

```c
typedef struct {
    uint8_t  num;                       // 有效计数
    uint8_t  count;                     // 100ms 计数
    uint16_t start;                     // 有效读数位置
    ADC_VcIcChannel_Def VcIc[AdcAvageCount];   // 电压电流原始数据

    uint16_t Txa[4][TxaAvageCount];     // [炉头][周期] 有功电流
    uint16_t voltage[4][TxaAvageCount]; // 电压均值
    uint16_t phase[4][TxaAvageCount];   // 相位角
    uint16_t phaseValue[4][TxaAvageCount]; // 相位辅助值
    uint16_t iPeak[4][TxaAvageCount];   // 峰值电流
    uint16_t ceilQ[4][TxaAvageCount];   // Q 值
    uint16_t esr[4][TxaAvageCount];     // 等效电阻

    ADC_Tempe_Def Tempe;                // 温度 ADC
    ADC_VcIcChannel_Def FanAd;          // 风扇 ADC
    ADC_VcIcChannel_Def res;            // 保留
} APP_ADC_AVG_BUFF_DEF;

APP_ADC_AVG_BUFF_DEF AdcFromApiDma20ms;  // 全局实例
```

**APP_ADC.C 中对 `AdcFromApiDma20ms` 的写入位置** (1ms 路径输出):

```c
AdcFromApiDma20ms.Txa[potCh][count]      = result.active_current;    // 第2660行
AdcFromApiDma20ms.voltage[potCh][count]   = result.voltage;          // 第2661行
AdcFromApiDma20ms.phase[potCh][count]     = result.phase_angleUp;    // 第2662行
AdcFromApiDma20ms.iPeak[potCh][count]     = result.peak_current;     // 第2663行
AdcFromApiDma20ms.ceilQ[potCh][count]     = inputArray[potCh].highOff; // 第2667行
AdcFromApiDma20ms.esr[potCh][count]       = result.esr;              // 第2668行
AdcFromApiDma20ms.phaseValue[potCh][count]= result.zero_cross_high   // 第2669行
```

**APP_ADC.C 中对 `AdcFromApiDma20ms` 的读取位置** (20ms 路径输入):

```c
AdcFromApiDma20ms.Txa[ch][num]            // 第436, 472行
AdcFromApiDma20ms.voltage[ch][num]        // 第444, 480行
AdcFromApiDma20ms.phase[ch][num]          // 第455行
AdcFromApiDma20ms.ceilQ[ch][num]          // 第452行
AdcFromApiDma20ms.Tempe.buff[...]        // 第610~619, 786~807行
AdcFromApiDma20ms.num                     // 第565, 569, 851行
AdcFromApiDma20ms.count                   // 第1012, 2550, 2561, 2575行
```

### 3.4 其他 app_power 符号引用

| 符号 | 类型 | 在 APP_ADC.C 中 |
|------|------|-----------------|
| `PotCh1/2/3/4` | `enum` | DMA 恢复, 通道切换 |
| `PotChWork` | `#define` | 当前工作炉头 |
| `PotNum` | 宏 | 炉头数 |
| `TxaAvageCount` (源于 `app_power.h` 或 `APP_ADC.H`) | 宏 | 20ms 数据深度 |

---

## 四、外部依赖按 .h 分类

| 头文件 | 路径 | 调用函数/符号 | 被哪些路径使用 |
|--------|------|-------------|-------------|
| **`APP_ADC.H`** | `LIB/APP/` | 自身声明、内部数据结构 | 全部 |
| **`API_hrtim.h`** | `API/` | `API_HRTIM_GetAddressTxaCnt()` | DMA 恢复 |
| **`API_adc.h`** | `API/` | ADC 通道切换、AWD 设置 | 1ms, DMA恢复 |
| **`API_gpio.h`** | `API/` | `API_GPIO_WritePin()` | 1ms 调试IO |
| **`API_TIM.h`** | `API/` | TIM 控制 | DMA 恢复 |
| **`api_dma.h`** | `API/` | `API_DMA_RECOVER`, `API_DMA_REST` | DMA 恢复 |
| **`app_power.h`** | `APP/POWER/inc/` | `AdcFromApiDma20ms`, `APP_POWER_SetTxaAwdValue()`, `PotCh1..4` | **1ms 输出, 20ms 输入, DMA恢复** |
| **`adc_processing.h`** | `BaseClass/` | ADC 数据处理 | 1ms |
| **`API_FMAC.H`** | `API/` | FMAC 滤波器 | 1ms |
| **`printMessage.h`** | `BaseClass/` | `PrintMessagePush()`, `PrintMessageOut()` | 1ms 消息 |
| **`power_calculator.h`** | `BaseClass/` | `CalculatePower()`, `PowerResult`, `PowerCalculatorInputDef` | **1ms 核心** |
| **`wave_capture.h`** | `BaseClass/` | `WaveCapture_IsReady()`, `WaveCapture_PushMessage()` | 1ms 消息 |
| **`API_I2C.H`** | `API/` | I2C 温度传感器 | 温度读取 |
| **`API_OPAMP.H`** | `API/` | OPAMP 配置 | 1ms |
| **`API_UART.H`** | `API/` | 串口调试 | 调试 |

---

## 五、数据流简化图

```
DMA → FMAC → APP_ADC_CalculatePower()
                │
                ├─ CalculatePower() → PowerResult → IH_CycleDataDef[20]
                │                                       │
                │                              AdcFromApiDma20ms[]
                │                              ┌─ Txa[4][20]  有功电流
                │                              ├─ voltage[4][20]
                │                              ├─ phase[4][20]
                │                              ├─ iPeak[4][20]
                │                              └─ ceilQ[4][20]
                │                                       │
                └─ TxaMessageOut() → PrintMessage / WaveCapture
                                                          
20ms: APP_ADC_AVG_Fun() → 读取 AdcFromApiDma20ms[] → 平均
```

**IH_CycleDataDef[20]** 是新增的数据桥, `powerResult20ms[potCh]` 保存每次 `CalculatePower()` 的结果, 供 20ms 的 `IH_CalculateParams()` 直接消费。
