# 婴儿模块架构 — 半桥/全桥统一方案

  

## Context

  

**问题**: m4_ekf_observer 项目需要从"4头半桥"迁移到"2头全桥"。原方案用 `#ifdef FULL_BRIDGE_MODE` 条件编译在现有 `app_power.c` (~6400行) 中硬插分支，导致代码膨胀、维护困难。

  

**方案**: 引入"婴儿模块"架构模式（基于方法论 v2.2 PULL 范式的 `std_module.h`），将半桥/全桥的公共逻辑抽象为基类，特定逻辑分解为子类，通过 `data_switcher` 中间层做数据流转。模块之间零耦合。

  

**决策依据**（已与用户确认）:

- 基类输出"PPG 相对调整量"(PID结果)，子类负责功率计算(V×I窗口积分) + 输出映射(duty/移相)

- 编译时选择子类（`#define FULL_BRIDGE_MODE`）

- 基类→Switcher→子类 路由方式

- APP_ADC 先定为基类，预留子类化接口

- 新建方法论模块在 `app/` 下

  

---

  

## 一、婴儿模块架构总览

  

### 1.1 核心概念

  

每个"婴儿模块"：

- 只知道自己的 g_input (饿了) 和 g_output (拉了)

- 不关心谁给输入、输出给谁

- data_switcher 负责所有数据路由

- `std_module.h` 的 `MODULE_SKELETON` 天然实现此模式

  

### 1.2 模块分解

  

```

┌─────────────────────────────────────────────────────────┐

│                    Data Switcher                         │

│  注册顺序: AppAdc(0) → PowerBase(1) → PowerCalc(2)      │

│  路由: 每个 DoWork 后自动派发 g_output → 下游 g_input   │

└─────────────────────────────────────────────────────────┘

      │              │              │

      ▼              ▼              ▼

┌──────────┐  ┌──────────┐  ┌──────────────┐

│ AppAdc   │→ │PowerBase │→ │ PowerCalc    │

│ (基类)   │  │ (基类)   │  │ (子类)       │

│          │  │          │  ├──────────────┤

│ ADC采样  │  │ PID      │  │ 半桥: 单窗口 │

│ 20ms平均 │  │ 目标功率 │  │      积分+占空比  │

│ 温度     │  │ 保护/检锅│  │ 全桥: 双窗口 │

│          │  │ 浪涌处理 │  │      积分+移相 │

└──────────┘  └──────────┘  └──────────────┘

                               ↕ (反馈)

                            actual_power

```

  

### 1.3 单周期数据流 (20ms)

  

```

Step 1: AppAdc.DoWork

  读取 ADC DMA 缓冲 → 20ms 平均 → g_out.adc_data

  路由: → PowerBase.g_in (avg values for protection)

       → PowerCalc.g_in (raw pointers for window integral)

  

Step 2: PowerBase.DoWork

  消费 ADC avg + 上周期 actual_power → PID → g_out.ppg_delta

  路由: → PowerCalc.g_in (ppg_delta for output adjustment)

  

Step 3: PowerCalc.DoWork

  消费 ADC raw + ppg_delta:

    ① _calc_window_integral() → actual_power → g_out

    ② _apply_output(ppg_delta) → 写 HRTIM 硬件

  路由: → PowerBase.g_in (actual_power for next PID cycle)

```

  

---

  

## 二、模块详细设计

  

### 2.1 AppAdc — APP_ADC 基类 (app/app_adc.c)

  

**输入(g_input)**: 无（硬实时 ISR 直接写 DMA 缓冲）

**输出(g_output)**: `AppAdc_Output_t`

  

```c

/* include/app_adc_io.h */

typedef struct {

    /* === 20ms 平均值 (给 PowerBase 保护用) === */

    uint16_t voltage_avg;        // 电压 20ms 平均

    uint16_t current_avg;        // 电流 20ms 平均

    uint16_t phase_avg[4];       // 相位 20ms 平均(最大4头)

    uint16_t q_avg[4];           // Q值 20ms 平均

    uint16_t igbt_temp;          // IGBT 温度 ADC

    uint16_t bottom_temp;        // 炉面温度 ADC

  

    /* === 原始数据指针 (给 PowerCalc 窗口积分) === */

    uint16_t *resonant_current;   // 谐振电流 DMA 缓冲指针

    uint16_t *hrtim_timestamps;   // HRTIM 时间戳指针

#if defined(FULL_BRIDGE_MODE)

    uint16_t *hrtim_lag;          // 全桥: 滞后臂 HRTIM 时间戳

#endif

    uint16_t *voltage_raw;        // 电压原始采样数组指针

  

    /* === 状态 === */

    uint8_t  head_index;          // 当前炉头索引

    uint8_t  dma_ready;           // DMA 数据就绪标志

    uint8_t  valid;               // 整体数据有效标志

    uint8_t  __reserved[13];      // 扩展预留

} AppAdc_Output_t;                // sizeof ≤ 可配置

```

  

**核心逻辑**:

- `Adc_DoWork()`: 每 1ms 检查 20ms 结算是否完成

- `APP_ADC_AVG_Fun()`: 20ms 平均计算（同现有逻辑）

- `AdcValueFun()`: 将平均值 → g_output

- 每帧置 ST_OUT

  

### 2.2 PowerBase — APP_POWER 基类 (app/power_base.c)

  

**输入(g_input)**: `PowerBase_Input_t`

```c

/* include/app_power_io.h */

typedef struct {

    /* 从 AppAdc 路由来 */

    uint16_t voltage_adc;

    uint16_t current_adc;

    uint16_t igbt_temp_adc;

    uint16_t bottom_temp_adc;

  

    /* 从 PowerCalc 路由来 (反馈) */

    int32_t  measured_power;       // 实际功率 (从窗口积分算出)

    uint8_t  power_valid;

  

    /* 从 MODBUS 路由来 (用户命令) */

    uint16_t target_power;         // 用户设定的目标功率

    uint8_t  power_on;             // 开关命令

    uint8_t  head_index;           // 目标炉头

} PowerBase_Input_t;

```

  

**输出(g_output)**: `PowerBase_Output_t`

```c

typedef struct {

    int16_t  ppg_delta[4];         // PID 输出: 每炉头调整量(±)

    uint8_t  delta_valid[4];

    uint8_t  state[4];             // POWER_STA_IDLE/RUN/STOP

    uint8_t  any_active;           // 是否有炉头在加热

} PowerBase_Output_t;

```

  

**核心逻辑 (迁移自 app_power.c)**:

```

ProcessInput():

  1. 消费 ADC 数据 → 更新 PowerCtx[head]

  2. 消费 measured_power → PID 输入

  3. 消费 target_power → 功率管理

  4. 保护链(每100ms):

     surge_Processing()          ← app_power.c:2331

     getOvpValueAdj()            ← app_power.c:1292

     igbt_derate()               ← 温度降功率

     top_temp_stop()             ← 炉面超温

     soft_start()                ← 软启动

  5. 检锅链(每100ms):

     PanStatusCheck()            ← app_power.c:2167

     s_pan_check_fun()           ← app_power.c:3510

  6. PID(每20ms):

     FixedPID_Compute(target - measured) → ppg_delta

  7. 写 g_output.ppg_delta[head] → ST_OUT

```

  

### 2.3 PowerCalc — 功率计算子类

  

**输入(g_input)**: `PowerCalc_Input_t`

```c

/* include/app_power_calc_io.h */

typedef struct {

    /* 从 AppAdc 路由来 (原始数据指针) */

    uint16_t *resonant_current;

    uint16_t *hrtim_values;        // 半桥: 单路; 全桥: 超前臂

    uint16_t *voltage_raw;

#if defined(FULL_BRIDGE_MODE)

    uint16_t *hrtim_lag;           // 全桥: 滞后臂 HRTIM

#endif

    uint16_t  voltage_instant;     // 当前电压瞬时值

    uint8_t   adc_valid;

  

    /* 从 PowerBase 路由来 (调整量) */

    int16_t   ppg_delta;

    uint8_t   delta_valid;

    uint8_t   power_state;         // RUN 才输出到硬件

    uint8_t   head_index;

} PowerCalc_Input_t;

```

  

**输出(g_output)**: `PowerCalc_Output_t`

```c

typedef struct {

    int32_t  active_power;         // 有功功率 (→ PowerBase 做 PID 反馈)

    int32_t  active_current;

    uint16_t peak_current;

    uint16_t voltage;

    int16_t  phase_angle_up;

    int16_t  phase_angle_down;

    uint8_t  valid;

} PowerCalc_Output_t;

```

  

### 2.3a PowerCalc_HalfBridge (app/power_calc_half.c)

  

```

ProcessInput():

  if (adc_valid):

    _calc_window_integral_half():

      ① FindZeroCrossing() 定位过零点

      ② CalculateAuctalCurrent() 单窗口电流积分

      ③ 功率 = Σ(V×I) / lowOff

      ④ 写 g_out.active_power

  

  if (delta_valid && state==RUN):

    _apply_output_half():

      ① duty = clamp(g_duty + ppg_delta, MIN, MAX)

      ② API_PPG_setValue(head, duty)  ← 半桥 API

```

  

**参考**: `base_class/src/power_calculator.c` — `CalculatePower()` 半桥单窗口积分

  

### 2.3b PowerCalc_FullBridge (app/power_calc_full.c)

  

```

ProcessInput():

  if (adc_valid):

    _calc_window_integral_full():

      ① FindZeroCrossing() × 2 (超前臂+滞后臂)

      ② FB_CalculateAuctalCurrent() 双窗口积分(对角管)

      ③ 功率 = forward_power + reverse_power

      ④ 写 g_out.active_power

  

  if (delta_valid && state==RUN):

    _apply_output_full():

      ① phase = clamp(g_phase + ppg_delta, 0, MASTER_PERIOD-1)

      ② API_FB_SetPhaseShift(head, phase)  ← 全桥 API

      ③ 同频管理: 双头同频，MASTER 统一周期

```

  

**参考**: `ProjectsOld/BaseClass/src/power_calculator_fullbridge.c` — `FB_CalculatePower()` 全桥双窗口积分

  

---

  

## 三、Data Switcher 路由

  

### 3.1 注册顺序 (data_switcher.c)

  

```c

void Switcher_Init(void)

{

    Para_Grp_t *pIn, *pOut;

    void (*pWork)(void);

  

    /* Slot 0: ADC 生产者 */

    AppAdc_GetIO(&pIn, &pOut, &pWork);

    Switcher_Register(pWork, pOut);

  

    /* Slot 1: PowerBase PID */

    PowerBase_GetIO(&pIn, &pOut, &pWork);

    Switcher_Register(pWork, pOut);

  

    /* Slot 2: 编译时选择子类 */

#ifdef FULL_BRIDGE_MODE

    PowerCalc_Full_GetIO(&pIn, &pOut, &pWork);

#else

    PowerCalc_Half_GetIO(&pIn, &pOut, &pWork);

#endif

    Switcher_Register(pWork, pOut);

}

```

  

### 3.2 路由顺序 (同一次 Slot1 调用内)

  

```

Switcher_Run_Slot1:

  Step 1: AppAdc.DoWork          → g_out = {adc_avg, raw_ptr, dma_ready}

  Route:  AppAdc.g_out → PowerBase.g_in (avg for protection)

          AppAdc.g_out → PowerCalc.g_in (raw_ptr for integration)

  

  Step 2: PowerBase.DoWork       → g_out = {ppg_delta[], state[]}

  Route:  PowerBase.g_out → PowerCalc.g_in (ppg_delta for output)

  

  Step 3: PowerCalc.DoWork       → g_out = {active_power, valid}

          内部调用: _calc_window_integral() + _apply_output(ppg_delta)

  Route:  PowerCalc.g_out → PowerBase.g_in (measured_power for next PID)

```

  

**关键**: 三步在同一 `Switcher_Run_Slot1` 内完成，通过注册顺序保证:

- PowerBase 先产生 ppg_delta → PowerCalc 在同一周期消费

- PowerCalc 产生的 actual_power → PowerBase 下一周期消费（无反馈延迟）

  

### 3.3 __weak 回调声明 (interface_map.h)

  

新增 3 条路由通道:

  

```

/* Pair P1: AppAdc → PowerBase (ADC 平均值) */

 * 发送方: data_switcher.c  WEAK void PowerBase_OnAdcData(Para_Grp_t *pOut) {}

 * 接收方: power_base.c     void PowerBase_OnAdcData(Para_Grp_t *pOut)

  

/* Pair P2: AppAdc → PowerCalc (ADC 原始数据) */

 * 发送方: data_switcher.c  WEAK void PowerCalc_OnAdcRaw(Para_Grp_t *pOut) {}

 * 接收方: power_calc_*.c   void PowerCalc_OnAdcRaw(Para_Grp_t *pOut)

  

/* Pair P3: PowerBase → PowerCalc (PPG 调整量) */

 * 发送方: data_switcher.c  WEAK void PowerCalc_OnDelta(Para_Grp_t *pOut) {}

 * 接收方: power_calc_*.c   void PowerCalc_OnDelta(Para_Grp_t *pOut)

  

/* Pair P4: PowerCalc → PowerBase (实际功率反馈) */

 * 发送方: data_switcher.c  WEAK void PowerBase_OnMeasuredPower(Para_Grp_t *pOut) {}

 * 接收方: power_base.c     void PowerBase_OnMeasuredPower(Para_Grp_t *pOut)

```

  

---

  

## 四、编译时选择机制

  

### 4.1 统一桥模式头文件 (cfg/bridge_mode.h)

  

```c

#ifndef BRIDGE_MODE_H

#define BRIDGE_MODE_H

  

#if defined(FULL_BRIDGE_MODE)

  #define BRIDGE_HEAD_COUNT    2

#else

  #define HALF_BRIDGE_MODE     1   /* 默认 */

  #define BRIDGE_HEAD_COUNT    4

#endif

  

#endif

```

  

### 4.2 影响范围

  

| 条件 | 编译的文件 | 不编译的文件 |

|------|-----------|-------------|

| 默认(半桥) | `app/power_calc_half.c` | `app/power_calc_full.c` |

| `-D FULL_BRIDGE_MODE` | `app/power_calc_full.c` | `app/power_calc_half.c` |

  

共同编译: `app/power_base.c`, `app/app_adc.c`, `core/data_switcher.c`

  

### 4.3 cfg/structs.json 注册

  

新增结构体:

- S9: `PowerBase_Output_t` → PowerCalc (ppg_delta)

- S10: `PowerCalc_Output_t` → PowerBase (measured_power)

  

---

  

## 五、与现有代码的映射

  

### 5.1 函数迁移表

  

| 原函数 (app_power.c) | 新模块 | 行为 |

|---|---|---|

| `power_con_fun()` | PowerBase | 目标功率管理 |

| `s_ppg_fun()` — PID 部分 | PowerBase | FixedPID_Compute |

| `s_ppg_fun()` — 功率计算 | PowerCalc_* | _calc_window_integral |

| `i_ppg_control()` — 输出映射 | PowerCalc_* | _apply_output |

| `i_ppg_control()` — 限幅 | PowerBase | s_ppg_limit |

| `surge_Processing()` | PowerBase | 浪涌处理 |

| `getOvpValueAdj()` | PowerBase | OVP 阈值 |

| `PanStatusCheck()` | PowerBase | 检锅 |

| `s_pan_check_fun()` | PowerBase | 移锅检测 |

| `APP_POWER_PhaseHalfTypeSet()` | PowerBase | 相位限制(通用部分) |

| `APP_POWER_PpgHalfTypeSet()` | PowerCalc_Half | 倍频切换(半桥特有) |

  

### 5.2 过渡策略

  

**Phase 0**: 新建方法论模块 + I/O 结构体

- 创建 `app/power_base.c`, `app/power_calc_half.c`, `app/app_adc.c`

- 创建 `include/app_power_io.h`, `include/app_power_calc_io.h`

- 创建 `cfg/bridge_mode.h`

- 新模块编译通过（半桥模式，功能与现有等值）

  

**Phase 1**: 接入 Switcher 验证

- 修改 `data_switcher.c` 注册新模块

- 输出对比: 新 PowerCalc.measured_power == 旧 app_power.ActualPower

- `__weak` 桥接新旧两套系统的输出

  

**Phase 2**: 添加全桥变体

- 创建 `app/power_calc_full.c`

- 新增 `FULL_BRIDGE_MODE` 编译选项

- 全桥模式下编译验证 0e0w

  

**Phase 3**: 清理

- 移除旧 `LIB/APP/app_power.c` 中已迁移的函数（逐步）

- 更新 REGISTER 表、CLAUDE.md

  

---

  

## 六、文件清单

  

| 新文件 | 用途 |

|--------|------|

| `cfg/bridge_mode.h` | 桥模式统一头文件 (HEAD_COUNT) |

| `include/app_adc_io.h` | **改**: 扩展 AppAdc_Output_t (加原始指针+预留) |

| `include/app_power_io.h` | **改**: PowerBase_Input/Output_t |

| `include/app_power_calc_io.h` | **新**: PowerCalc_Input/Output_t |

| `app/app_adc.c` | **新**: APP_ADC 基类 (MODULE_SKELETON) |

| `app/power_base.c` | **新**: PowerBase 基类 (PID+保护+检锅) |

| `app/power_calc_half.c` | **新**: 半桥子类 (单窗口积分+duty输出) |

| `app/power_calc_full.c` | **新**: 全桥子类 (双窗口积分+移相输出) |

| `core/data_switcher.c` | **改**: 条件编译注册 + 4条路由函数 |

| `core/interface_map.h` | **改**: 新增 P1~P4 路由通道说明 |

  

**参考文件 (只读不写)**:

- `ProjectsOld/LIB/APP/app_power.c` — 函数迁移源

- `ProjectsOld/BaseClass/src/power_calculator.c` — 半桥窗口积分算法

- `ProjectsOld/BaseClass/src/power_calculator_fullbridge.c` — 全桥窗口积分算法

- `ProjectsOld/API/inc/API_hrtim_fullbridge.h` — 全桥 HRTIM API

  

---

  

## 七、验证方式

  

1. **编译验证**: 半桥模式 0e0w，全桥模式 0e0w

2. **五件套**:

   - `check_deps.py` — 层依赖

   - `check_weak_pairs.py` — 新增 P1~P4 配对

   - `check_structs.py` — S9~S10 结构体一致性

   - `check_output_callback.py` — 输出回调审计

3. **功能验证**: 半桥模式下新 PowerCalc 计算的 measured_power 与旧系统 ActualPower 一致