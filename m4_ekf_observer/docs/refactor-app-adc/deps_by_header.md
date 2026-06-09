# APP_ADC 外部依赖分析报告

## 概述

本报告分析 `APP_ADC.C` 和 `APP_ADC.H` 的所有外部依赖，按头文件分类列出被调用的函数、被引用的结构体/枚举/宏、被读写的全局变量。

---

## 1. APP_ADC.H

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/APP_ADC.H`
层: APP (自身接口)

#### 定义的函数
| 函数名 | 说明 |
|--------|------|
| `AdcValueFun` | 统一处理ADC值，返回1表示有效值输出 |
| `APP_ADC_AVG_Fun` | 每20ms得到一组ADC数据 |
| `getADCinputValue` | 获取指定通道的ADC输入值 |
| `APP_ADC_DMA_RecoverPan` | 恢复检锅DMA |
| `API_ADC_DMA_RecoverTxa` | 恢复检锅DMA |
| `APP_ADC_PanSwChange` | 切换检锅选择口 |
| `APP_ADC_GetPowerTxa` | 得到谐振电流计算的功率值 |
| `CopyISRToRAM` | 将ISR复制到RAM |
| `getIcValueAdress` | 获取IC值地址 |
| `APP_ADC_GetHrtimAddress` | 获取HRTIM地址 |
| `APP_ADC_TimDmaEnd` | 得到100ms提取DMA的结束标志进行TXA滤波求和处理 |
| `getAdcBuffAddress` | 获取ADC缓冲区地址 |
| `getAdcBuffSize` | 获取ADC缓冲区大小 |
| `getAdcBuffSum` | 获取ADC缓冲区求和值 |
| `API_ADC_DMA_RecoverTxaPan` | 恢复检锅DMA |
| `API_ADC_DMA_TimStart` | 启动TXA DMA TIM3 |
| `APP_ADC_SetTXAstepDmaHrtimEnd` | 在HRTIM CMP4第二个中断中结束DMA |
| `APP_ADC_SetTXAstepDmaRest` | 设置TXA步骤DMA恢复 |
| `APP_ADC_TxaAvgReset` | 滤波初始化 |
| `APP_ADC_ClearCeilQAvg` | 重置CEILQ的滤波 |
| `APP_ADC_WaitTxaCalOver` | 等待TXA DMA采集完成 |
| `APP_ADC_GetHrtimSyncBuffAdr` | 得到多炉头同一点的HRTIM值 |
| `APP_ADC_CalculatePower` | 计算功率 |
| `APP_ADC_IsTxaDmaStart` | 检查DMA TXA是否启动 |

#### 定义的结构体/类型
| 类型名 | 用途 |
|--------|------|
| `APP_ADC_AWD_DNTR_DEF` | AWD DMA数据指针缓存结构 |

#### 定义的宏/常量
| 宏名 | 值 | 说明 |
|------|-----|------|
| `HRTIM_PER1US` | 0x300 | 1us对应的HRTIM计数值 |
| `APP_ADC_DNTR_BUFF_MAX` | 5 | AWD缓存最大值 |
| `DIV_VALUE` | 1 | 一阶滤波系数 |
| `DIV_BASE` | 16 | 一阶滤波系数基数 |
| `DIV_VALUE_POWER` | 1 | 功率一阶滤波系数 |
| `DIV_BASE_POWER` | 32 | 功率一阶滤波系数基数 |
| `DIV_VALUE_Q` | 1 | Q值一阶滤波系数 |
| `DIV_BASE_Q` | 2 | Q值一阶滤波系数基数 |
| `DMA_HRTIM` | 0 | ADC DMA读取HRTIM值 |
| `HRTIM_OFFSET` | 3 | TIM hrtim Dma与ADC DMA的偏移 |

#### 定义的枚举
| 枚举名 | 成员 |
|--------|------|
| HRTIM索引 | `HRTIM_Upd`, `HRTIM_HighOn`, `HRTIM_HighOff`, `HRTIM_LowOn`, `HRTIM_LowOff`, `HRTIM_Max` |
| ADC组索引 | `AdcGroupPower1~4`, `AdcGroupVoltage`, `AdcGroupT1A~T4A`, `AdcGroupBottom1~4`, `AdcGroupIgbt1~4`, `AdcGroupFan`, `AdcGroupCeilQ1~4`, `AdcGroupPhase1~4`, `AdcGroupPhaseDown1~4`, `AdcGroupMax` |
| DNTR通道 | `DNTR_T12A`, `DNTR_T34A` |
| TXA通道 | `chT1A~T4A` |
| 脉冲状态 | `pluseStart`, `pluseHighOn`, `pluseHighOff`, `pluseLowOn`, `pluseLowOff`, `pluseDown`, `pluseLowScr`, `pluseEnd` |

---

## 2. API_HRTIM.h

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/API/inc/API_HRTIM.h`
层: VENDOR HAL

#### 使用的函数
| 函数名 | APP_ADC中的位置(行号) | 调用方式 | 说明 |
|--------|------------------------|----------|------|
| `API_HRTIM_DISABLE_IT_UPD` | L1009, L1078 | 直接调用 | 禁用HRTIM更新中断 |
| `API_HRTIM_DISABLE_IT_REST` | L1164 | 直接调用 | 禁用HRTIM复位中断 |
| `API_HRTIM_ENABLE_IT_REST` | L1210, L1227 | 直接调用 | 使能HRTIM复位中断 |
| `API_HRTIM_BASE_ENABLE_IT_CMP` | L1057 | 直接调用 | 使能HRTIM基准比较中断 |
| `API_HRTIM_GetAddressTxaCnt` | L1982, L1993, L2003-2014 | 直接调用 | 获取TXA计数器地址 |
| `TIMsynchronous` | 未在APP_ADC.C中直接调用 | - | HRTIM同步 |

#### 使用的结构体/类型
| 类型名 | 使用位置 | 用途 |
|--------|----------|------|
| `PPGvalueDef` | - | PPG周期和占空比定义 |
| `PPGpointDef` | - | PPG关键点定义 |

#### 使用的宏/常量
| 宏名 | 使用位置 | 值 |
|------|----------|-----|
| `PotNum` | L170, L364, L1343 | 4 (PotChTest1) |
| `PotCh1~4` | L1982, L1993, L2003-2014 | 0~3 |
| `HRTIM_ADJ` | - | 0.5us调整值 |
| `FRE_500K_PWM` | - | 500kHz对应PWM值 |
| `HRTIM_INPUT_CLOCK` | - | 192000000 |

#### 引用的全局变量
| 变量名 | 类型 | 使用位置 | 读/写 |
|--------|------|----------|-------|
| `PotChWork` | uint8_t | - | - |

---

## 3. API_adc.h

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/API/inc/API_adc.h`
层: VENDOR HAL

#### 使用的函数
| 函数名 | APP_ADC中的位置(行号) | 调用方式 | 说明 |
|--------|------------------------|----------|------|
| `API_ADC_GetDRx` | L798-807, L831, L1867-1876, L1898 | 直接调用 | 获取ADC DRx值 |
| `API_ADC_GetAddressDRx` | L1867-1876, L1898, L2019 | 直接调用 | 获取ADC DRx地址 |
| `API_ADC_GetAddressDR` | L1984, L1995 | 直接调用 | 获取ADC DR地址 |
| `API_ADC_T12A_ENABLE_IT_EOC` | L865 | 直接调用 | 使能ADC1 EOC中断 |
| `API_ADC_T12A_DISABLE_IT_EOC` | L1053 | 直接调用 | 禁用ADC1 EOC中断 |
| `API_ADC_SELECT` | L1262 | 直接调用 | 选择ADC通道 |
| `API_ADC_StopAllAdc` | L2038 | 直接调用 | 停止所有ADC |
| `API_ADC_StartAllAdc` | L2053 | 直接调用 | 启动所有ADC |
| `API_ADC_GetAddressInstance` | L2081 | 直接调用 | 获取ADC实例地址 |

#### 使用的结构体/类型
| 类型名 | 使用位置 | 用途 |
|--------|----------|------|
| `ADC_M2M_RecoverDef` | L1981 | DMA恢复定义 |
| `ADC_SELECT_ENUM` | L1952, L2068, L2100 | ADC选择枚举 |
| `ADC_CH_ENUM` | L798-807, L1867-1876 | ADC通道枚举 |

#### 使用的宏/常量
| 宏名 | 使用位置 | 值 |
|------|----------|-----|
| `AdcAvageCount` | L214, L230, L324, L392 | 199 |
| `TxA_ADC_TimBUFF_NUM` | L202 | 8 |
| `TxaAvageCount` | L429, L478, L483 | 20 |
| `Igbt1ADC2_GroupCh` | L798-801, L804-807 | 0~3 |
| `Bottom1ADC1_GroupCh` | L804-807 | 0~3 |
| `Current1ADC2_Group` | L1867-1876 | 0~3 |
| `VoltageADC1_Group` | L1899, L2019 | 0 |
| `TempeGroupZeroMax` | L101 | 11 |

---

## 4. API_gpio.h

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/API/inc/API_gpio.h`
层: VENDOR HAL

#### 使用的函数
| 函数名 | APP_ADC中的位置(行号) | 调用方式 | 说明 |
|--------|------------------------|----------|------|
| `API_GPIO_WritePin` | L1259, L1265, L1283-1286, L1302, L1323, L1356, L1934, L1936 | 直接调用 | 写GPIO引脚 |
| `API_GPIO_Mode` | L1288-1291, L1296 | 直接调用 | 设置GPIO模式 |

#### 使用的宏/常量
| 宏名 | 使用位置 | 值 |
|------|----------|-----|
| `DebugA_pin` | L1259, L1265, L1323, L1356, L1934, L1936 | 0 |
| `PANSW1_pin` | L1283, L1288 | 21 |
| `PANSW2_pin` | L1284, L1289 | 22 |
| `PANSW3_pin` | L1285, L1290 | 23 |
| `PANSW4_pin` | L1286, L1291, L1296 | 24 |
| `MODER_Output` | L1288-1291, L1296 | 1 |

---

## 5. API_FMAC.H

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/API/inc/API_FMAC.H`
层: VENDOR HAL

#### 使用的结构体/类型
| 类型名 | 使用位置 | 用途 |
|--------|----------|------|
| `API_FMAC_MEMDEF` | - | FMAC内存定义 |
| `API_FMAC_PublicMemDEF` | - | FMAC公共内存定义 |

#### 使用的宏/常量
| 宏名 | 使用位置 | 值 |
|------|----------|-----|
| `COEFF_VECTOR_B_SIZE` | - | FmacLeve |
| `FMAC_OFFSET` | L69 | (FmacLeve/2+2) |
| `fmacLeveNum` | L71 | FmacLeve |

---

## 6. app_power.h

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/app_power.h`
层: APP ⚠️ **违规: APP→APP 跨层引用**

#### 使用的函数
| 函数名 | APP_ADC中的位置(行号) | 调用方式 | 说明 |
|--------|------------------------|----------|------|
| `APP_POWER_SetTxaAwdValue` | L1024, L1264 | 直接调用 | 设置AWD值 |
| `APP_POWER_GetPanDmaBuffAddress` | L1899 | 直接调用 | 获取PAN ADC缓存地址 |

#### 使用的宏/常量
| 宏名 | 使用位置 | 值 |
|------|----------|-----|
| `PotChWork` | - | PotCh1(0) |
| `RESONANCE1_SURGE` | - | 1 |
| `RESONANCE2_SURGE` | - | 2 |

#### 引用的全局变量
| 变量名 | 类型 | 使用位置 | 读/写 |
|--------|------|----------|-------|
| `PotChWork` | uint8_t | - | 读 |

---

## 7. adc_processing.h

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/DRV/INC/adc_processing.h`
层: DRV

#### 使用的结构体/类型
| 类型名 | 使用位置 | 用途 |
|--------|----------|------|
| `ADC_Buffer_t` | - | ADC缓冲区结构 |
| `ADC_Buff1_FlagDef` | - | ADC标志位定义 |

#### 使用的函数
| 函数名 | APP_ADC中的位置(行号) | 调用方式 | 说明 |
|--------|------------------------|----------|------|
| `APP_ADC_AvgTxA20us` | - | 声明但未调用 | TXA 20us平均 |
| `API_POWER_PanCheckPluseAsm` | - | 声明但未调用 | PAN脉冲检测汇编 |

---

## 8. power_calculator.h

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/BaseClass/inc/power_calculator.h`
层: BaseClass

#### 使用的结构体/类型
| 类型名 | 使用位置 | 用途 |
|--------|----------|------|
| `PowerCalculatorInputDef` | L257 | 功率计算器输入定义 |
| `IH_ElecInputDef` | L324 | 电参数输入定义 |

#### 使用的函数
| 函数名 | APP_ADC中的位置(行号) | 调用方式 | 说明 |
|--------|------------------------|----------|------|
| `Power_Calculator_GetVoltageBuffAddress` | - | 声明但未调用 | 获取电压缓冲区地址 |
| `Power_Calculator_GetHrtimBuffAddress` | - | 声明但未调用 | 获取HRTIM缓冲区地址 |
| `Power_Calculator_GetTxaBuffAddress` | - | 声明但未调用 | 获取TXA缓冲区地址 |

---

## 9. printMessage.h

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/BaseClass/inc/printMessage.h`
层: BaseClass (Debug)

#### 使用的结构体/类型
| 类型名 | 使用位置 | 用途 |
|--------|----------|------|
| `MessageDef` | - | 消息定义 |
| `MessageBuffDef` | - | 消息缓冲区定义 |

#### 使用的宏/常量
| 宏名 | 使用位置 | 值 |
|------|----------|-----|
| `PRINT_MESSAGE_BUFF_SIZE` | - | 8 |

---

## 10. wave_capture.h

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/BaseClass/inc/wave_capture.h`
层: BaseClass (Debug)

#### 使用的结构体/类型
| 类型名 | 使用位置 | 用途 |
|--------|----------|------|
| `CaptureMsgDef_t` | - | 捕获消息定义 |
| `CaptureMsgBuff_t` | - | 捕获消息缓冲区定义 |
| `WaveCaptureFrame` | - | 波形捕获帧结构 |

---

## 11. API_I2C.H

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/API/inc/API_I2C.H`
层: VENDOR HAL

#### 使用的函数
| 函数名 | APP_ADC中的位置(行号) | 调用方式 | 说明 |
|--------|------------------------|----------|------|
| `API_I2C_CheckBuffMax` | L823 | 直接调用 | 检查I2C缓冲区最大 |

---

## 12. API_OPAMP.H

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/API/inc/API_OPAMP.H`
层: VENDOR HAL

**未在APP_ADC.C中使用**

---

## 13. API_UART.H

路径: `src/RX32G410_FW_HAL_V1.3N/Projects/API/inc/API_UART.H`
层: VENDOR HAL

**未在APP_ADC.C中使用**

---

## 依赖汇总

| 头文件 | 层 | 函数数 | 结构体数 | 宏数 | 全局变量 | 依赖方向 |
|--------|-----|--------|----------|------|----------|----------|
| `APP_ADC.H` | APP | 26 | 1 | 9 | 0 | 自身 |
| `API_HRTIM.h` | VENDOR HAL | 6 | 2 | 6 | 1 | APP→HAL (合法) |
| `API_adc.h` | VENDOR HAL | 8 | 3 | 9 | 0 | APP→HAL (合法) |
| `API_gpio.h` | VENDOR HAL | 2 | 2 | 6 | 0 | APP→HAL (合法) |
| `API_FMAC.H` | VENDOR HAL | 0 | 2 | 3 | 0 | APP→HAL (合法) |
| `app_power.h` | APP | 2 | 0 | 3 | 1 | **APP→APP (违规!)** |
| `adc_processing.h` | DRV | 0 | 2 | 2 | 0 | APP→DRV (合法) |
| `power_calculator.h` | BaseClass | 0 | 2 | 0 | 0 | APP→BaseClass (合法) |
| `printMessage.h` | BaseClass | 0 | 2 | 1 | 0 | APP→BaseClass (合法) |
| `wave_capture.h` | BaseClass | 0 | 3 | 10 | 0 | APP→BaseClass (合法) |
| `API_I2C.H` | VENDOR HAL | 1 | 1 | 0 | 0 | APP→HAL (合法) |
| `API_OPAMP.H` | VENDOR HAL | 1 | 0 | 0 | 0 | APP→HAL (合法) — 补充验证 |
| `API_UART.H` | VENDOR HAL | 0 | 0 | 0 | 0 | ✅ 未使用, 可移除 |

---

## 关键全局变量溯源

| 变量名 | 定义位置 | 类型 | 所属模块 |
|--------|----------|------|----------|
| `TxA_ADC_AdcDmaBuff` | APP_ADC.C:283 | `APP_ADC_AdcTxADMA_BUFF_DEF` | APP_ADC |
| `AdcFromApiDma20ms` | APP_ADC.C:300 | `APP_ADC_AVG_BUFF_DEF` | APP_ADC |
| `AdcFunRam` | APP_ADC.C:301 | `APP_ADC_DEF` | APP_ADC |
| `inputArray[4]` | APP_ADC.C:257 | `PowerCalculatorInputDef` | APP_ADC |
| `powerResult20ms[4]` | APP_ADC.C:324 | `IH_ElecInputDef` | APP_ADC |
| `PotNum` | API_HRTIM.h:29 | enum值(4) | API_HRTIM |
| `PotChWork` | app_power.h:221 | uint8_t | app_power ⚠️ |
| `TxaAdcBuff[4]` | 未找到 | - | [未找到定义] |
| `TxaHrtimBuff[4]` | 未找到 | - | [未找到定义] |
| `TxaVcBuff[4]` | 未找到 | - | [未找到定义] |
| `TxaFmacBuff[4]` | 未找到 | - | [未找到定义] |

---

---

## 补充验证 (2026-06-05)

### 修正 1: API_TIM.h 和 API_DMA.H 文件存在

两个文件实际存在于项目中，但名称不同：
- `api_dma.h` → 实际文件: `Projects/API/inc/API_DMA.H` (大写 `.H`)
- `API_TIM.h` → 实际文件: `Projects/API/inc/API_TIM.H` (大写 `.H`)

Windows 文件系统大小写不敏感，编译可通过。

**API_DMA 实际使用符号** (报告漏列):
| 符号 | 位置 | 说明 |
|------|------|------|
| `API_DMA_GetDmaCndtr` | L1207, L1225 | 获取 DMA 当前 CNDTR 值 |
| `API_DMA_RecoverDef` | L1315 | DMA 恢复配置结构体 |
| `ChDmaCurrentAdc2`, `ChDmaCurrentAdc3` | L1207, L1225 | DMA 通道宏 |
| `API_DMA_TxA_IRQHandlerCallBack` | L1315 | DMA 中断回调 (STRONG 实现) |
| `API_ADC_DMA_TimStart` | L1918 | 启动 TXA DMA 定时器 |
| `API_ADC_DMA_RecoverTxa` | L1987 | 恢复 TXA DMA |
| `APP_ADC_AdcTxADMA_BUFF_DEF` | L176 | DMA 缓冲区类型 |
| `APP_ADC_TimTxADMA_BUFF_DEF` | L209 | TXA DMA 缓冲区类型 |
| `APP_ADC_TempeDMA_BUFF_DEF` | L221 | 温度 DMA 缓冲区类型 |

**API_TIM 实际使用符号** (报告漏列):
| 符号 | 位置 | 说明 |
|------|------|------|
| `API_TIM_100US_RESET` | L866 | 重置 100us 定时器 |
| `API_TIM_TXA_STOP` | L1260 | 停止 TXA 定时器 |
| `API_TIM_TXA_RESET` | L1263 | 重置 TXA 定时器 |

### 修正 2: API_OPAMP.H 实际使用了

报告说"未使用"是错误的。实际调用:
| 符号 | 位置 | 说明 |
|------|------|------|
| `API_OPAMP1_Select(ch)` | L1261 | 切换 OPAMP 输入通道 |

**确认**: `API_OPAMP.H` 不可移除。

### 修正 3: API_UART.H 确认未使用

在 APP_ADC.C 中仅出现于 include 语句和 @deps 注释，无任何函数调用或类型引用。
**可移除**: `#include "API_UART.H"` + `@deps` 中的 `API_UART`。

### 修正 4: CalculatePower / CalculateElecParams_20ms 实际调用了

报告说 `Power_Calculator_Get*BuffAddress` "声明但未调用"，但遗漏了核心调用:
| 函数 | 位置 | 说明 |
|------|------|------|
| `CalculatePower` | L2648 | **单周期功率计算** (APP_ADC 核心调用) |
| `CalculateElecParams_20ms` | L2727 | **20ms 电参数计算** (已注释掉 `#if 0`) |

其中 `CalculateElecParams_20ms` 在 `#if 0` 块中 (已被我们的 `IhElecParams_Calculate` __weak 替代)。

### 修正 5: Txa*Buff 变量定位

4 个指针数组定义在 APP_ADC.C 自身 (L2223-2230, 全局变量):
```c
uint16_t* TxaAdcBuff[4];      // L2223
uint16_t* TxaHrtimBuff[4];    // L2224
uint16_t* TxaVcBuff[4];       // L2225
uint16_t* TxaFmacBuff[4];     // L2230
```
由 APP_ADC_MesageBuff() 分配，APP_ADC_CalculatePower() 消费。不需要外部头文件。

---

## 问题总结

1. **未使用的头文件**: `API_UART.H` 被包含但未使用，建议移除。

2. **API_OPAMP.H 已确认使用**: 报告初版误判为未使用，实际 `API_OPAMP1_Select(ch)` 在 L1261 调用。不可移除。

3. **APP→APP 跨层引用违规**: `APP_ADC.C` 直接引用 `app_power.h`，调用 `APP_POWER_SetTxaAwdValue` / `APP_POWER_GetPanDmaBuffAddress`，引用 `PotChWork`。建议 Step 6 用 `__weak` 回调替代。

4. **全局变量溯源**: 所有 4 个 `Txa*Buff` 指针数组均定义在 APP_ADC.C 自身 (L2223-2230)，不依赖外部头文件。`PotChWork` 来自 `app_power.h` (违规)。

---

## 拆分建议

基于此依赖分析，建议 Step 3-6 的拆分边界：

- **DRV 层**: 提取 DMA/ADC/HRTIM 底层操作，形成 `drv_adc_dma.c/h`
- **APP 层**: 保留业务逻辑，通过 `__weak` 回调与 `app_power` 解耦
- **BaseClass**: `power_calculator` 和 `adc_processing` 保持独立，作为算法库
- **Debug**: `printMessage` 和 `wave_capture` 作为调试工具，可条件编译移除