# prompt: 按 .h 分类整理 APP_ADC 所有外部依赖

## 任务

分析 `APP_ADC.C` 和 `APP_ADC.H`，按头文件分类列出所有外部依赖：被调用的函数、被引用的结构体/枚举/宏、被读写的全局变量。

## 输入文件

```
D:\OBSIDIAN\MOVING IH\低耦合程序架构\m4_ekf_observer\src\RX32G410_FW_HAL_V1.3N\Projects\LIB\APP\APP_ADC.C  (2870行)
D:\OBSIDIAN\MOVING IH\低耦合程序架构
\m4_ekf_observer/src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/APP_ADC.H  (240行)
```

这两个文件是同一个模块，APP_ADC.H 是接口，APP_ADC.C 是实现。

## 步骤

### 1. 收集所有 #include 头文件

从 APP_ADC.C 和 APP_ADC.H 中提取所有 `#include` 语句，列出完整的头文件清单。

当前已有的 include (APP_ADC.C):
```
APP_ADC.H, API_hrtim.h, API_adc.h, API_gpio.h, API_TIM.h, api_dma.h,
app_power.h, adc_processing.h, API_FMAC.H, printMessage.h,
power_calculator.h, wave_capture.h, API_I2C.H, API_OPAMP.H, API_UART.H
```

对每个头文件，找到它在项目中的实际路径并阅读。

### 2. 按头文件逐一分析

对每个 .h 文件：
- **读取该头文件**，列出它声明的所有函数、结构体、typedef、enum、宏、extern 变量
- **在 APP_ADC.C 中搜索**这些符号的实际使用位置
- **区分使用方式**: 调用/读写/传参/取地址

### 3. 输出格式

对每个头文件输出以下表格:

```markdown
### {header_name}.h

路径: {full_path}
层: {HAL/DRV/APP/BaseClass/...}

#### 使用的函数
| 函数名 | APP_ADC 中的位置(行号) | 调用方式 | 说明 |
|--------|------------------------|----------|------|
| FuncName | L1234, L2345 | 直接调用 | 功能描述 |

#### 使用的结构体/类型
| 类型名 | 使用位置 | 用途 | 字段列举 |
|--------|----------|------|----------|
| TypeName | L100 (全局变量声明) | 缓存ADC数据 | field1: uint32_t, field2: ... |

#### 使用的宏/常量
| 宏名 | 使用位置 | 值(如果能找到) |
|------|----------|----------------|
| MACRO_NAME | L456 | 0x300 |

#### 引用的全局变量 (extern)
| 变量名 | 类型 | 使用位置 | 读/写 |
|--------|------|----------|-------|
| g_var | TypeName | L789 | 写 |
```

### 4. 汇总

最后给出总表:

```markdown
## 依赖汇总

| 头文件 | 层 | 函数数 | 结构体数 | 宏数 | 全局变量 | 依赖方向 |
|--------|-----|--------|----------|------|----------|----------|
| API_hrtim.h | VENDOR HAL | 8 | 2 | 5 | 0 | APP→HAL (合法) |
| app_power.h | APP | 3 | 1 | 0 | 0 | APP→APP (违规!) |
| ... | | | | | | |
```

## 约束

1. **不要修改任何文件** — 只读分析
2. **不要猜测** — 如果一个符号找不到定义，标注"[未找到定义]"
3. **搜索要覆盖**: 直接调用、`__weak` 声明、函数指针赋值、`sizeof()`、类型转换
4. **特别关注**: 以下这些在 APP_ADC.C 中大量使用的全局变量，必须明确它们来自哪个 .h:
   - `TxA_ADC_AdcDmaBuff`
   - `AdcFromApiDma20ms`
   - `AdcFunRam`
   - `inputArray[4]`
   - `powerResult20ms[4]`
   - `PotNum`, `PotChWork`
   - `TxaAdcBuff[4]`, `TxaHrtimBuff[4]`, `TxaVcBuff[4]`, `TxaFmacBuff[4]`
5. **标注违规**: 如果符号来自不应该依赖的层，在汇总中标记"违规"

## 预期输出

完整的依赖分析报告，保存到:
```
docs/refactor-app-adc/deps_by_header.md
```

这个报告将直接决定 Step 3-6 的拆分边界和接口设计。
