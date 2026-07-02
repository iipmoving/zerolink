# 婴儿模块架构 — 半桥/全桥统一方案

## Context

**问题**: m4_ekf_observer 项目需要从"4头半桥"迁移到"2头全桥"。原方案用 `#ifdef FULL_BRIDGE_MODE` 条件编译在现有 `app_power.c` (~6400行) 中硬插分支，导致代码膨胀、维护困难。

**方案**: 引入"婴儿模块"架构模式（基于方法论 v2.2 PULL 范式的 `std_module.h`），将半桥/全桥的公共逻辑抽象为基类，特定逻辑分解为子类，通过 `data_switcher` 中间层做数据流转。模块之间零耦合。

**决策依据**（已与用户确认）:
- PowerCalc 输出结构体带 `bridge_type` 字节，Switcher 路由时读此字节做类型感知
- 子类同时编译，不条件编译 — 运行时选择一个
- 运行时 type byte 分发：PowerCalc.DoWork() 根据 `g_input.bridge_type` 调用对应策略函数
- 基类→Switcher→子类 路由方式
- PowerCalc 本身是基类：共享 `FindZeroCrossing()`、末端插值、归一化、相位角计算
- PowerCalc 子类：只替换窗口积分策略（半桥单 HRTIM vs 全桥双 HRTIM 对角管检测）
- PowerCalc 是纯算法模块，不做任何硬件操作。实际输出走已有控制链 → ISR → HRTIM 寄存器更新
- APP_ADC 先定为基类，预留子类化接口
- 新建方法论模块在 `app/` 下
- PowerBase 当前只消费 measured_power（1ms 路径）；20ms 电参数扩展是独立后续工作

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
│                    Data Switcher                         │
│  注册顺序: AppAdc(0) → PowerBase(1) → PowerCalc(2)      │
│  路由: 每个 DoWork 后自动派发 g_output → 下游 g_input   │
└─────────────────────────────────────────────────────────┘
      │              │              │
      ▼              ▼              ▼
┌──────────┐  ┌──────────┐  ┌──────────────┐
│ AppAdc   │→ │PowerBase │→ │ PowerCalc    │
│ (基类)   │  │ (基类)   │  │ (基类+策略)  │
│          │  │          │  ├──────────────┤
│ ADC采样  │  │ PID      │  │ 共享: 过零点 │
│ 20ms平均 │  │ 目标功率 │  │      峰值检测 │
│ 温度     │  │ 保护/检锅│  │      末端插值 │
│          │  │ 浪涌处理 │  │ 半桥策略: 单HRTIM窗口积分 │
│          │  │          │  │ 全桥策略: 双HRTIM对角管窗口积分 │
└──────────┘  └──────────┘  └──────────────┘
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
    ① 读 g_input.bridge_type 选择策略
    ② _calc_window_integral() → actual_power → g_out
    (纯算法: 不操作任何硬件)
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
    uint16_t voltage_avg;        // 电压 20ms 平均
    uint16_t current_avg;        // 电流 20ms 平均
    uint16_t phase_avg[4];       // 相位 20ms 平均(最大4头)
    uint16_t q_avg[4];           // Q值 20ms 平均
    uint16_t igbt_temp;          // IGBT 温度 ADC
    uint16_t bottom_temp;        // 炉面温度 ADC

    /* === 原始数据指针 (给 PowerCalc 窗口积分) === */
    uint16_t *resonant_current;   // 谐振电流 DMA 缓冲指针
    uint16_t *hrtim_timestamps;   // 半桥: 单路; 全桥: 超前臂 HRTIM 时间戳
    uint16_t *hrtim_lag;          // 半桥: NULL; 全桥: 滞后臂 HRTIM 时间戳
    uint16_t *voltage_raw;        // 电压原始采样数组指针

    /* === 状态 === */
    uint8_t  head_index;          // 当前炉头索引
    uint8_t  dma_ready;           // DMA 数据就绪标志
    uint8_t  valid;               // 整体数据有效标志
    uint8_t  __reserved[3];       // 对齐补齐
} AppAdc_Output_t;                // sizeof ≤ 可配置
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
    int32_t  measured_power;       // 实际功率 (从窗口积分算出)
    uint8_t  power_valid;

    /* 从 MODBUS 路由来 (用户命令) */
    uint16_t target_power;         // 用户设定的目标功率
    uint8_t  power_on;             // 开关命令
    uint8_t  head_index;           // 目标炉头
} PowerBase_Input_t;
```

**输出(g_output)**: `PowerBase_Output_t`

```c
typedef struct {
    int16_t  ppg_delta[4];         // PID 输出: 每炉头调整量(±)
    uint8_t  delta_valid[4];
    uint8_t  state[4];             // POWER_STA_IDLE/RUN/STOP
    uint8_t  any_active;           // 是否有炉头在加热
} PowerBase_Output_t;
```

**核心逻辑 (迁移自 app_power.c)**:

```
ProcessInput():
  1. 消费 ADC 数据 → 更新 PowerCtx[head]
  2. 消费 measured_power → PID 输入
  3. 消费 target_power → 功率管理
  4. 保护链(每100ms):
     surge_Processing()          ← app_power.c:2331
     getOvpValueAdj()            ← app_power.c:1292
     igbt_derate()               ← 温度降功率
     top_temp_stop()             ← 炉面超温
     soft_start()                ← 软启动
  5. 检锅链(每100ms):
     PanStatusCheck()            ← app_power.c:2167
     s_pan_check_fun()           ← app_power.c:3510
  6. PID(每20ms):
     FixedPID_Compute(target - measured) → ppg_delta
  7. 写 g_output.ppg_delta[head] → ST_OUT
```

### 2.3 PowerCalc — 功率计算基类 + 策略子类

PowerCalc 本身也是基类，共享大部分计算逻辑。子类只替换窗口积分策略。

**结构**:

| 文件 | 内容 |
|------|------|
| `app/power_calc.c` | **基类**: DoWork() + 共享工具函数 (FindZeroCrossing, 归一化, 相位角) |
| `app/power_calc_half.c` | **半桥策略**: 单 HRTIM 窗口积分（参考 `power_calculator.c`） |
| `app/power_calc_full.c` | **全桥策略**: 双 HRTIM 对角管窗口积分（参考 `power_calculator_fullbridge.c`） |

**输入(g_input)**: `PowerCalc_Input_t`

```c
/* include/app_power_calc_io.h */
typedef struct {
    /* === 桥类型 (中间层设置) === */
    uint8_t  bridge_type;          // BRIDGE_TYPE_HALF=半桥, BRIDGE_TYPE_FULL=全桥

    /* === 从 AppAdc 路由来 (原始数据指针) === */
    uint16_t *resonant_current;
    uint16_t *hrtim_values;        // 半桥: 单路; 全桥: 超前臂(lead)
    uint16_t *hrtim_lag;           // 全桥: 滞后臂(lag); 半桥: NULL
    uint16_t *voltage_raw;
    uint16_t  voltage_instant;
    uint8_t   adc_valid;

    /* === 从 PowerBase 路由来 (调整量) === */
    int16_t   ppg_delta;
    uint8_t   delta_valid;
    uint8_t   power_state;         // RUN 才输出到硬件
    uint8_t   head_index;

    uint8_t   __reserved[3];       // 对齐补齐
} PowerCalc_Input_t;               // sizeof 固定，无条件编译差异
```

**输出(g_output)**: `PowerCalc_Output_t`

```c
typedef struct {
    uint8_t  bridge_type;          // 输出也带类型，Switcher 可感知
    int32_t  active_power;         // 有功功率 (→ PowerBase 做 PID 反馈)
    int32_t  active_current;
    uint16_t peak_current;
    uint16_t voltage;
    int16_t  phase_angle_up;
    int16_t  phase_angle_down;
    uint8_t  valid;
    uint8_t  __reserved[3];       // 对齐补齐
} PowerCalc_Output_t;
```

### 2.3a PowerCalc_HalfBridge — 半桥策略 (app/power_calc_half.c)

```
_strategy_window_integral_half():
  ① FindZeroCrossing() 定位过零点 (单 HRTIM 计数器)
  ② CalculateAuctalCurrent() 窗口电流积分 (高/低边切换)
  ③ 非对称输出时: 另一窗口也积分
  ④ 功率 = Σ(V×I) / lowOff
  ⑤ 写 g_out.active_power / peak_current / phase_angle
```

**参考**: `ProjectsOld/BaseClass/src/power_calculator.c`

### 2.3b PowerCalc_FullBridge — 全桥策略 (app/power_calc_full.c)

```
_strategy_window_integral_full():
  ① FindZeroCrossing() × 2 (超前臂 lead + 滞后臂 lag)
  ② FB_CalculateAuctalCurrent() 对角管导通窗口积分
     - forward_window: lead < highOff && lag >= lagDuty
     - reverse_window: lead >= highOff && lag < lagDuty
  ③ 功率 = forward_power + reverse_power
  ④ 写 g_out.active_power / voltage / esr / phase_angle
```

**参考**: `ProjectsOld/BaseClass/src/power_calculator_fullbridge.c`

### 2.3c PowerCalc DoWork 内部分发

```c
void PowerCalc_DoWork(void)
{
    PowerCalc_Input_t *in = (PowerCalc_Input_t *)g_input.para;

    if (!in->adc_valid) return;

    switch (in->bridge_type) {
    case BRIDGE_TYPE_HALF:
        _strategy_window_integral_half(in, out);
        break;
    case BRIDGE_TYPE_FULL:
        _strategy_window_integral_full(in, out);
        break;
    default:
        return;  /* 未知桥型: 跳过 */
    }

    g_output.info.status |= ST_OUT;
}
```

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

    /* Slot 2: PowerCalc (统一入口，内部根据 bridge_type 分发) */
    PowerCalc_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork, pOut);
}
```

### 3.2 路由顺序 (同一次 Slot1 调用内)

```
Switcher_Run_Slot1:
  Step 1: AppAdc.DoWork          → g_out = {adc_avg, raw_ptr, dma_ready}
  Route:  AppAdc.g_out → PowerBase.g_in (avg for protection)
          AppAdc.g_out → PowerCalc.g_in (raw_ptr for integration)

  Step 2: PowerBase.DoWork       → g_out = {ppg_delta[], state[]}
  Route:  PowerBase.g_out → PowerCalc.g_in (ppg_delta for output)

  Step 3: PowerCalc.DoWork       → g_out = {active_power, valid, bridge_type}
          内部: 读 bridge_type → dispatch 策略函数 → 窗口积分
  Route:  PowerCalc.g_out → PowerBase.g_in (measured_power for next PID)
```

**关键**: 三步在同一 `Switcher_Run_Slot1` 内完成，通过注册顺序保证:
- PowerBase 先产生 ppg_delta → PowerCalc 在同一周期消费
- PowerCalc 产生的 actual_power → PowerBase 下一周期消费（无反馈延迟）

---

## 四、运行时桥类型分发

### 4.1 Type Byte 定义 (cfg/bridge_type.h)

```c
#ifndef BRIDGE_TYPE_H
#define BRIDGE_TYPE_H

#define BRIDGE_TYPE_HALF      0x01   /* 半桥：单路 HRTIM */
#define BRIDGE_TYPE_FULL      0x02   /* 全桥：双路 HRTIM (lead+lag) */

#endif
```

### 4.2 运行时工作流

```
配置阶段:
  每炉头初始化时设置 bridge_type（从默认配置 / MODBUS 写入 / EEPROM 加载）

运行阶段:
  Switcher_Run_Slot1:
    Step 1: AppAdc.DoWork      → g_out = {adc_avg, raw_ptr, dma_ready}
    Step 2: PowerBase.DoWork   → g_out = {ppg_delta[], state[]}
    Step 3: PowerCalc.DoWork:
              → 读 g_input.bridge_type
              → BRIDGE_TYPE_HALF:  _strategy_window_integral_half()
              → BRIDGE_TYPE_FULL:  _strategy_window_integral_full()
              → 写 g_out.active_power + bridge_type

Switcher 在路由时:
  → 不关心 bridge_type（只传 Para_Grp_t*）
  → 纯婴儿模块模式：模块间无耦合
```

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
| `s_ppg_fun()` — 功率计算 | PowerCalc | 窗口积分策略入口 |
| `i_ppg_control()` — 限幅 | PowerBase | s_ppg_limit |
| `surge_Processing()` | PowerBase | 浪涌处理 |
| `getOvpValueAdj()` | PowerBase | OVP 阈值 |
| `PanStatusCheck()` | PowerBase | 检锅 |
| `s_pan_check_fun()` | PowerBase | 移锅检测 |
| `APP_POWER_PhaseHalfTypeSet()` | PowerBase | 相位限制(通用部分) |
| `APP_POWER_PpgHalfTypeSet()` | PowerCalc_Half | 倍频切换(半桥特有) |

注: `i_ppg_control()` 的输出映射（写 HRTIM 寄存器）不在本架构范围内——走现有控制链 → ISR → HRTIM

### 5.2 过渡策略

**Phase 0**: Pre — 恢复 v2.0 方法论数据就绪机制
- 调试修复 AdcValueFun 的 ICVCok 标志流转
- 确保 PowerBase 输入回调收到 ADC 数据更新通知
- 记录修复过程到 `docs/adc-ready-fix.md`

**Phase 1**: 新建婴儿模块骨架
- 创建 `app/power_base.c`, `app/power_calc.c`, `app/app_adc.c`
- 创建 `include/app_power_io.h`, `include/app_power_calc_io.h`, `include/app_adc_io.h`
- 创建 `cfg/bridge_type.h`
- 新模块编译通过（半桥策略，功能与现有等值）

**Phase 2**: 接入 Switcher 验证
- 修改 `data_switcher.c` 注册新模块
- `__weak` 桥接新旧两套系统，输出对比
- 验证: 新 PowerCalc.measured_power == 旧 app_power.ActualPower

**Phase 3**: 添加全桥策略
- 创建 `app/power_calc_full.c` — 全桥对角管窗口积分
- 所有子类同时编译，运行时通过 bridge_type 选择策略
- 混合模式验证: 1 头全桥 + 3 头半桥

**Phase 4**: 清理
- 逐步移除旧 `LIB/APP/app_power.c` 中已迁移的函数
- 更新 REGISTER 表、CLAUDE.md

---

## 六、文件清单

| 新文件 | 用途 |
|--------|------|
| `cfg/bridge_type.h` | 桥类型枚举 (HALF/FULL) |
| `include/app_adc_io.h` | **改**: 扩展 AppAdc_Output_t (加原始指针+预留) |
| `include/app_power_io.h` | **改**: PowerBase_Input/Output_t |
| `include/app_power_calc_io.h` | **新**: PowerCalc_Input/Output_t (含 bridge_type) |
| `app/app_adc.c` | **新**: APP_ADC 基类 (MODULE_SKELETON) |
| `app/power_base.c` | **新**: PowerBase 基类 (PID+保护+检锅) |
| `app/power_calc.c` | **新**: PowerCalc 基类 (共享工具 + DoWork dispatch) |
| `app/power_calc_half.c` | **新**: 半桥窗口积分策略 |
| `app/power_calc_full.c` | **新**: 全桥对角管窗口积分策略 |
| `core/data_switcher.c` | **改**: 注册新模块 + 连接层路由函数 |

**参考文件 (只读不写)**:
- `ProjectsOld/LIB/APP/app_power.c` — 函数迁移源
- `ProjectsOld/BaseClass/src/power_calculator.c` — 半桥窗口积分算法
- `ProjectsOld/BaseClass/src/power_calculator_fullbridge.c` — 全桥窗口积分算法
- `ProjectsOld/API/inc/API_hrtim_fullbridge.h` — 全桥 HRTIM API

---

## 七、验证方式

1. **编译验证**: 0e0w（所有子类同时编译，无条件编译）
2. **检查工具**:
   - `check_deps.py` — 层依赖
   - `check_structs.py` — S9~S10 结构体一致性
   - `check_output_callback.py` — 输出回调审计
3. **功能验证**: 半桥策略下新 PowerCalc.measured_power == 旧 app_power.ActualPower
4. **类型派发验证**: bridge_type=HALF 走半桥积分, bridge_type=FULL 走全桥积分预期路径

---

*规划创建日期: 2026-06-08*
*基于方法论 v2.2 PULL 范式, 婴儿模块架构*
