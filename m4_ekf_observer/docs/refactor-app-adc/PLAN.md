# APP_ADC 解耦重构 · 分步执行计划

> 原始文件: `LIB/APP/APP_ADC.C` (3203行) + `APP/POWER/inc/APP_ADC.H` (241行)
> 目标: 按零耦合方法论拆分为多个独立模块, 全局变量→__weak/void* 接口化

---

## 现状诊断

### 文件规模

| 文件 | 行数 | 说明 |
|------|------|------|
| `APP_ADC.C` | 3203 | 主文件, 含 239 行死代码 |
| `APP_ADC.H` | 241 | 头文件, 含废弃类型/枚举 |
| `app_power.c` | 6151 | 紧耦合, 实现 APP_ADC 的 __weak 回调 STRONG 版 |
| `class_power_calculator.c` | ~1550 | 直接读写 APP_ADC 全局变量 |
| `APP_ADC.C.bak` | — | 备份文件 (应删除) |
| `APP_ADC - 副本.C` | — | 副本文件 (应删除) |

### 核心问题

1. **死代码**: ~240行 PC 端测试代码 (`main()` × 2) + ~100行废弃注释
2. **跨层 include**: `#include "app_power.h"` (APP→APP 违规)
3. **文件中段 include**: L827, L1263, L2177, L2489 四处非标准 include
4. **全局变量暴露**: 13个全局变量被 3+ 个外部文件直接读写
5. **双向紧耦合**: APP_ADC 声明 `__weak` → app_power 提供 STRONG 实现, 同时 app_power 调用 APP_ADC 函数
6. **多职责混杂**: ADC DMA + HRTIM 管理 + 功率计算 + 温度采集 + 锅检测 + 调试输出

### 全局变量耦合矩阵

| 全局变量 | 生产者 | 消费者 | 外部写者 |
|----------|--------|--------|----------|
| `AdcFromApiDma20ms` | APP_ADC | class_power_calculator, resonant_msg | **class_power_calculator 也写!** |
| `powerResult20ms[4]` | APP_ADC | ih_elec_params (已解耦) | — |
| `inputArray[4]` | (外部填充) | APP_ADC, power_calculator | class_power_calculator? |
| `TxA_ADC_AdcDmaBuff` | API DMA ISR | APP_ADC, app_power, class_power_calculator | app_power! |

---

## Step 0: 创建方法论工作副本 (5分钟, 零风险)

**操作**: 将 APP_ADC.C/.H 复制到新位置, 原文件保持不变。
后续所有修改在副本上进行, 编译通过后再替换原文件。

```
原文件:
  LIB/APP/APP_ADC.C          ← 不动
  APP/POWER/inc/APP_ADC.H    ← 不动

工作副本:
  app/app_adc_core/app_adc_core.c    ← 逐步缩减的副本
  app/app_adc_core/app_adc_core.h    ← 逐步清理的副本
```

**验证**: 文件复制成功, 原文件未被修改
**依赖**: 无

---

## Step 1: 删除无效代码 (10分钟, 零风险)

纯删除操作, 不改变任何逻辑。

### 1.1 删除备份文件
- `LIB/APP/APP_ADC.C.bak`
- `LIB/APP/APP_ADC - 副本.C`

### 1.2 删除 APP_ADC.C 中的死代码

| 行号 | 删除内容 | 行数 |
|------|----------|------|
| L2965-3052 | `#if 0` 块: `find_main_pulse_edges()` + `main()` + `CopyISRToRAM` 注释 | 88 |
| L3055-3204 | `#if 0` 块: `moving_average_filter()` + 第2个 `main()` | 150 |
| L76-86 | 注释掉的 `ADC_DualChannel_Def`, `ADC_FourChannel_Def` | 11 |
| L151-159 | 注释掉的 `ADC_POT_INPUT_DEF` | 9 |
| L260-270 | `#if 0` 的 __weak 参考声明 | 11 |
| L2906-2918 | 注释掉的 `getAdcBuff*` 函数 | 13 |
| L2927-2962 | 注释掉的 `API_DMA_T1A_IRQHandlerCallBack` | 36 |

**预计减少**: ~318 行 (约 10%)

### 1.3 删除 APP_ADC.H 中的废弃内容

- 废弃的枚举值 (AdcGroup 中非实际使用的)
- 注释掉的 `ADC_POT_INPUT_DEF` (L151-159)
- 审查每个 `extern` 函数声明是否仍被调用

**验证**: 编译通过 (死代码在 `#if 0` 中, 删除不影响编译)
**依赖**: Step 0

---

## Step 2: 清理头文件与 include (15分钟)

### 2.1 修复文件中段 include

将 4 处文件中段 include 移至文件顶部:

| 当前行 | include | 处理 |
|--------|---------|------|
| L827 | `#include "API_I2C.H"` | 移至顶部 |
| L1263 | `#include "API_OPAMP.H"` | 移至顶部 |
| L2177 | `#include "API_UART.H"` | 移至顶部 |
| L2489 | `#include "wave_capture.h"` | **重复 include, 删除** (顶部已有) |

### 2.2 标记跨层违规

- `#include "app_power.h"` — **不能直接删除** (被调用的函数需要声明)
  - 方案: 使用 `__weak` 桩替代直接调用 → 放到 Step 6 处理
  - 当前: 添加 `// FIXME: APP→APP 跨层引用, 待 Step 6 用 __weak 替代`

### 2.3 添加 @layer 注释

在文件头部按方法论格式标注当前层和已知违规:

```c
/**
 * @file    APP_ADC.C
 * @layer   APP (待拆分: 含 DRV/APP 混合职责)
 * @deps    API_hrtim, API_adc, API_gpio, API_TIM, api_dma, API_FMAC,
 *          API_I2C, API_OPAMP, API_UART, power_calculator, adc_processing,
 *          printMessage, wave_capture, app_power (FIXME: 跨层)
 */
```

**验证**: 编译通过, `check_deps.py` 输出已知违规列表 (app_power 待后续修复)
**依赖**: Step 1

---

## Step 3: 提取 app_adc_value 模块 (30分钟)

**目标**: 将 ADC 值处理逻辑从 APP_ADC 中拆出, 成为独立的 APP 层模块。

### 3.1 新建文件

```
app/app_adc_value/app_adc_value.h
app/app_adc_value/app_adc_value.c
```

### 3.2 迁移函数

从 APP_ADC.C 移入 `app_adc_value.c`:

| 函数 | 原行号 | 说明 |
|------|--------|------|
| `AdcValueFun()` | L680 | 统一处理 ADC 值 |
| `APP_ADC_AVG_Fun()` | L571 | 20ms ADC 平均值 |
| `APP_ADC_GET_TxaAvg()` | L778 | 获取 TXA 平均值 |
| `APP_ADC_GET_TEMPE()` | L791 | 获取温度值 |
| `APP_ADC_GetFanAd_AdcValue()` | L916 | 风扇 ADC |
| `APP_ADC_GetTxAadcValue()` | L938 | TXA ADC 值 |
| `APP_ADC_GeTempeAdcValue()` | L978 | 温度 ADC 值 |
| `getADCmax()` | L1759 | 获取 ADC 最大值 |
| `getADCinputValue()` | L1887 | 获取 ADC 输入值 |

### 3.3 内部状态

移入为 `static` 变量:
- `APP_ADC_DEF AdcFunRam` → `static APP_ADC_DEF _adc_ram`
- `APP_ADC_AVG_BUFF_DEF AdcFromApiDma20ms` → **保留在原文件** (被 class_power_calculator 直接访问, 后续处理)

### 3.4 接口

```c
// app_adc_value.h — 对外接口
uint8_t  AppAdcValue_Run(void);           // 原 AdcValueFun
void     AppAdcValue_AvgRun(void);        // 原 APP_ADC_AVG_Fun
void     AppAdcValue_GetTxaAvg(void);     // 原 APP_ADC_GET_TxaAvg
void     AppAdcValue_GetTempe(void);      // 原 APP_ADC_GET_TEMPE
uint32_t AppAdcValue_GetInput(uint8_t ch); // 原 getADCinputValue
```

**验证**: 编译通过, 外部调用方更新 include 路径
**依赖**: Step 2

---

## Step 4: 提取 app_power_cycle 模块 (45分钟)

**目标**: 将功率计算管线独立出来, 封装 `powerResult20ms` 和 `inputArray`。

### 4.1 新建文件

```
app/app_power_cycle/app_power_cycle.h
app/app_power_cycle/app_power_cycle.c
```

### 4.2 迁移函数

| 函数 | 原行号 | 说明 |
|------|--------|------|
| `APP_ADC_CalculatePower()` | L2655 | 20ms 功率计算 ★ |
| `APP_ADC_TimDmaEnd()` | L2590 | 1ms DMA 结束处理 |
| `APP_ADC_MesageBuff()` | L2804 | 消息缓冲 |
| `APP_ADC_TxaBuffChange()` | L2362 | 数据转换 |
| `APP_ADC_GetTxaPeiodPoint()` | L2507 | 周期检测 |
| `APP_ADC_TxaPublicBuffInit()` | L2784 | 缓冲初始化 |
| `APP_ADC_FmacSetTxa()` | L2562 | FMAC 配置 |

### 4.3 封装全局变量

```c
// app_power_cycle.c 内部 (static)
static IH_ElecInputDef     _powerResult20ms[4];  // 原 powerResult20ms
static PowerCalculatorInputDef _inputArray[4];   // 原 inputArray

// 对外只通过接口访问
void AppPowerCycle_GetResult(uint8_t ch, IH_ElecInputDef *out);
void AppPowerCycle_SetInput(uint8_t ch, const PowerCalculatorInputDef *in);
```

### 4.4 接口

```c
// app_power_cycle.h
void AppPowerCycle_Run(void);  // 替代 APP_ADC_CalculatePower
void AppPowerCycle_TimDmaEnd(void);

// __weak 回调 — 输出到电参数计算
__weak void IhElecParams_Calculate(void *in, void *out) {}
```

**验证**: 编译通过, `powerResult20ms` 不再被外部直接访问
**依赖**: Step 3 (ADC 值处理已独立)

---

## Step 5: 提取 DRV 层 (ADC DMA + HRTIM) (60分钟)

**目标**: 将与硬件直接交互的 DMA/HRTIM 管理抽到 DRV 层。

### 5.1 新建文件

```
drv/drv_adc_dma/drv_adc_dma.h
drv/drv_adc_dma/drv_adc_dma.c
drv/drv_hrtim_sync/drv_hrtim_sync.h
drv/drv_hrtim_sync/drv_hrtim_sync.c
```

### 5.2 drv_adc_dma 迁移

| 函数 | 原行号 |
|------|--------|
| `API_DMA_TxA_IRQHandlerCallBack()` | L1325 |
| `API_DMA_M2M_OverCallback()` | L2922 |
| `API_ADC_PENDV_IRQHandler()` | L2164 |
| `API_ADC_DMA_RecoverPan()` | L1900 |
| `APP_ADC_DMA_RecoverPan()` | L1934 |
| `API_ADC_DMA_TimStart()` | L1960 |
| `API_ADC_DMA_RecoverTxa()` | L1987 |
| `API_ADC_DMA_RecoverFun()` | L1994 |
| `APP_ADC_DMA_RecoverHrtim()` | L2077 |

内部状态: `TxA_ADC_AdcDmaBuff`

### 5.3 drv_hrtim_sync 迁移

| 函数 | 原行号 |
|------|--------|
| `API_HRTIM1_TEST_UPD_IRQHandlerCallback()` | L1110 |
| `API_HRTIM1_TEST_CMP1_IRQHandlerCallBack()` | L1167 |
| `API_ADC_Instance_SaveCallBack()` | L2110 |
| `API_ADC_Instance_RecoverCallBack()` | L2142 |
| `APP_ADC_SELECT_GROUP()` | L1265 |
| `APP_ADC_PanSwChange()` | L1284 |

### 5.4 ISR 回调注册

DRV 层继续提供 `__weak` ISR 回调桩, APP 层实现 STRONG 版本:

```c
// drv_adc_dma.h — DRV 层声明
__weak void DrvAdcDma_TxaComplete(void) {}

// app_power_cycle.c — APP 层实现 STRONG
void DrvAdcDma_TxaComplete(void) { /* 原 APP_ADC_TimDmaEnd 逻辑 */ }
```

**验证**: 编译通过, `check_deps.py` DRV→APP 方向无新增违规
**依赖**: Step 4

---

## Step 6: 打破 APP_ADC ↔ app_power 双向耦合 (60分钟)

**目标**: 消除 `#include "app_power.h"`, 所有调用走 `__weak` + `interface_map.h`。

### 6.1 梳理当前耦合

**APP_ADC → app_power (直接调用)**:
- `APP_ADC_GetPowerTxa()` → 被 app_power 调用
- `APP_ADC_TxaAvgReset()` → 被 app_power 调用
- `APP_ADC_GetHrtimSyncBuffAdr()` → 被 app_power 调用
- `APP_ADC_ClearCeilQAvg()` → 被 app_power 调用
- `APP_ADC_IsTxaDmaStart()` → 被 app_power 调用
- `APP_ADC_PanSwChange()` → 被 app_power 调用
- `APP_ADC_DMA_RecoverPan()` → 被 app_power 调用
- `APP_ADC_getOverAdcChannel()` → 被 app_power 调用

**APP_ADC 声明 __weak → app_power 提供 STRONG**:
- `APP_ADC_DebugValueCallBack()` → app_power L4591
- `APP_ADC_IRQ_PPGstepDecT12aCallBack()` → app_power L5120
- `APP_ADC_IRQ_PPGstepChangeCallBack()` → app_power L5539
- `APP_ADC_IRQ_PPGstepDecTxA()` → app_power L6045

### 6.2 重定义接口

每对 `APP_ADC func → app_power 调用` 改为 `__weak` 回调:

```c
// 在 app_power.c 需要数据时, 改为声明 __weak 桩:
__weak uint32_t AdcData_GetPowerTxa(uint8_t ch) { return 0; }
__weak void     AdcData_TxaAvgReset(uint8_t ch) {}
__weak uint16_t* AdcData_GetHrtimSyncBuff(void) { return NULL; }

// APP_ADC 侧提供 STRONG 实现:
uint32_t AdcData_GetPowerTxa(uint8_t ch) { ... }
void     AdcData_TxaAvgReset(uint8_t ch) { ... }
```

### 6.3 更新 interface_map.h

```c
/* Pair X: app_power → drv_adc_dma (功率数据查询) */
 * 发送方: app_power.c  WEAK uint32_t AdcData_GetPowerTxa(uint8_t ch) { return 0; }
 * 接收方: drv_adc_dma.c  uint32_t AdcData_GetPowerTxa(uint8_t ch)
```

**验证**: `#include "app_power.h"` 可从 APP_ADC 中移除, `check_weak_pairs.py` PASS
**依赖**: Step 5

---

## Step 7: 集成验证与清理 (20分钟)

### 7.1 运行完整检查

```bash
# 1. 层依赖审计
python tools/check_deps.py . --project m4-ekf

# 2. __weak 配对验证
python tools/check_weak_pairs.py .

# 3. 结构体一致性
python tools/check_structs.py .

# 4. 编译验证
armcc -c ... → 0 error, 0 warning
```

### 7.2 更新 interface_map.h

确保所有新增 __weak 配对都已注册。

### 7.3 替换原文件

用新模块替换 LIB/APP/APP_ADC.C 中的原有实现:
- 原 APP_ADC.C 中保留 `#include` 新模块头文件 + 转发调用 (过渡期)
- 或直接删除原文件, 更新所有 include 路径

### 7.4 Git 提交

```
[Milestone] APP_ADC 解耦: N模块拆分, M个__weak接口, 0跨层违规
```

**验证**: `/check` 4项全 PASS
**依赖**: Step 6

---

## 风险与回滚

| 风险 | 缓解措施 |
|------|----------|
| 拆分过程中引入 bug | 每步编译验证, 保留原文件备份 |
| app_power.c 耦合过深 | Step 6 可能需要多轮迭代 |
| 编译时间过长 | 只编译变更模块 |
| 全局变量访问链路断裂 | 每步确认所有引用点已更新 |

**回滚**: 原始文件在 `docs/refactor-app-adc/APP_ADC.C.original` 和 `.H.original`

---

## 时间估算

| Step | 内容 | 预计时间 |
|------|------|----------|
| 0 | 创建工作副本 | 5 min |
| 1 | 删除无效代码 | 10 min |
| 2 | 清理头文件 | 15 min |
| 3 | 提取 app_adc_value | 30 min |
| 4 | 提取 app_power_cycle | 45 min |
| 5 | 提取 DRV 层 | 60 min |
| 6 | 打破双向耦合 | 60 min |
| 7 | 集成验证 | 20 min |
| **总计** | | **~4 小时** |

---

*创建: 2026-06-05 | 状态: 待用户审阅*
