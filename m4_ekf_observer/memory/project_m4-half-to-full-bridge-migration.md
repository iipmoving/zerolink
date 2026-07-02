---
name: m4-half-to-full-bridge-migration
description: 4半桥↔2移相全桥迁移计划 — `FULL_BRIDGE_MODE` 一键切换，双炉头同频MASTER同步，移相分别调功
metadata:
  type: project
---

# 4半桥 ↔ 2移相全桥 迁移计划

> 工作名称: **`migrate-full-bridge`**
> 状态: **待执行** (2026-05-27 评估完成)
> 关联: [[m4-hrtim-master-sync-simplify]] (应先执行 hrtim-master-sync 简化)
> **前置条件: EKF 观测器开发完成后再启动。** (用户决策 2026-05-27)
> **核心要求**: 半桥/全桥通过单一编译标识快速切换，不维护两套代码

## 一、切换开关设计

### 1.1 开关定义

**位置**: [app_power.h](src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/app_power.h) — 文件顶部，include 之后

```c
// ============================================================
// 桥模式选择 (二选一，注释掉 = 禁用)
// ============================================================
#define FULL_BRIDGE_MODE    // ← 注释此行 = 4半桥，取消注释 = 2移相全桥
```

### 1.2 派生常量 (开关的下游定义)

```c
#ifdef FULL_BRIDGE_MODE
  #define POTNUM          2    // 2 个移相全桥炉头
  #define FB_CHANNELS     2
  #define FB_FUN_TABLES   2
#else
  #define POTNUM          4    // 4 个半桥炉头 (现有值)
  #define FB_CHANNELS     0
  #define FB_FUN_TABLES   4
#endif
```

### 1.3 分布原则

| 位置 | 策略 |
|------|------|
| `app_power.h` | 开关定义 + 派生常量 |
| `app_power.c` | 数组/函数表/循环边界全部 `#ifdef` 条件化 |
| `app_task.c` | 初始化路径 `#ifdef` 分支 |
| `API_HRTIM.h` | PotCh 常量不变 (全桥复用), 新增注释 |
| ISR 文件 | 尽量避免条件编译; 通过运行时空转处理 |

---

## 二、目标架构

### 半桥模式 (`FULL_BRIDGE_MODE` 注释掉)

```
PotCh1 (Timer B) → TB1/TB2 互补 → 炉头1, 独立频率, duty 调功
PotCh2 (Timer E) → TE1/TE2 互补 → 炉头2, 独立频率, duty 调功
PotCh3 (Timer A) → TA1/TA2 互补 → 炉头3, 独立频率, duty 调功
PotCh4 (Timer D) → TD1/TD2 互补 → 炉头4, 独立频率, duty 调功
```

### 全桥模式 (`FULL_BRIDGE_MODE` 定义)

```
MASTER 定时器    → 统一频率时钟源

FB1 超前臂 = Timer B (Q1上管/Q2下管)  ┐
FB1 滞后臂 = Timer E (Q3上管/Q4下管)  ┤ MCMP1 移相调功 → 炉头1

FB2 超前臂 = Timer A (Q1上管/Q2下管)  ┐
FB2 滞后臂 = Timer D (Q3上管/Q4下管)  ┤ MCMP2 移相调功 → 炉头2
```

---

## 三、现有全桥 API (已就绪，无需修改)

新版 `API_hrtim_fullbridge.c/h` 已实现驱动层全部功能：

| 函数 | 作用 |
|------|------|
| `API_FB_Init_MasterSync(fbCh)` | 初始化 Master 同步模式 |
| `API_FB_SetFrequency(fbCh, freqHz)` | 设置频率 (更新 MASTER 周期 + 按比例缩放移相) |
| `API_FB_SetPhaseShift(fbCh, count)` | 独立设置移相 (MCMP1 或 MCMP2) |
| `API_FB_Start(fbCh)` / `API_FB_Stop(fbCh)` | 启停 |
| `API_FB_OutputFreqModulation(fbCh, freq, phase)` | 一站式: 配置 + 启动 |

**硬件映射** (`FB_HW_MAP[]`)：FB1→MCMP1, FB2→MCMP2，已支持独立移相。

---

## 四、需修改的文件与精确位置

### 文件 1: `app_power.h` — 开关 + 派生常量 + 数据结构

**位置**: [app_power.h](src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/inc/app_power.h)

| 标记 | 当前 | 操作 |
|------|------|------|
| HDR-1 | 无开关 | 在 include 之后插入 §1.1 开关定义 + §1.2 派生常量 |
| HDR-2 | `AppPowerStaticDef` 结构体 | `#ifdef FULL_BRIDGE_MODE` 追加 `uint16_t phaseShift;` 字段 |

### 文件 2: `app_power.c` — 条件编译主体

**文件**: [app_power.c](src/RX32G410_FW_HAL_V1.3N/Projects/LIB/APP/app_power.c)

#### 2a. 数组维度 (L540-563 附近)

| 标记 | 当前 | 改为 |
|------|------|------|
| ARR-1 | `#define POTNUM 4` (L540) | 删除此行，改用 `app_power.h` 中的定义 |
| ARR-2 | `MasterCh=0, SlaveCh=1, MasterRCh=2, SlaveRCh=3` (L542-545) | `#ifndef FULL_BRIDGE_MODE` 包裹 — 仅半桥需要 |
| ARR-3 | `PowerMem[4], PowerInput[4], PowerStaticReg[4], PowerKeepReg[4]` | `PowerMem[POTNUM]` 等 — 维度跟随宏 |

#### 2b. 函数表 (L395-471)

| 标记 | 当前 | 改为 |
|------|------|------|
| FT-1 | `Power1FunTable..Power4FunTable` 4个 | `#ifdef FULL_BRIDGE_MODE` → 2个 / `#else` → 4个 |
| FT-2 | 函数表内的 `_PPGinit` 等指针 | 全桥时指向全桥包装函数 (FB 版 PPG 包装器) |

#### 2c. per-channel PPG 包装器 (L5719-5803)

| 标记 | 当前 | 改为 |
|------|------|------|
| PPG-1 | `PPGsetDutyCh1..4` 各4个 | `#ifdef FULL_BRIDGE_MODE` → CH1/CH2 体内调 `API_FB_*` / `#else` → 保持原样 |
| PPG-2 | `PPGsetValueCh1..4` | 同上 |
| PPG-3 | `PPGgetValueCh1..4` | 同上 |
| PPG-4 | `PPGonOffCh1..4` | 同上 → `API_FB_Start`/`API_FB_Stop` |

#### 2d. 功率控制核心 (L1355-1409)

| 标记 | 当前 | 改为 |
|------|------|------|
| CORE-1 | `PowerControlFun()` | 半桥分支不变; 全桥分支: PID 输出 delta 映射为 `phaseShift` 而非 `ppgDuty` |
| CORE-2 | `s_ppg_fun()` (L2716-2943) | `#ifdef FULL_BRIDGE_MODE` 内 `ActualPower = V*I/g_p25_ad` 不变; 控制输出 `delta_phase` |
| CORE-3 | `i_ppg_control()` (L1400附近) | 半桥: 写 PPG duty; 全桥: 写 `API_FB_SetPhaseShift(fbCh, newPhase)` |
| CORE-4 | `power_con_fun()` (L2288-2436) | 功率计算保留; 全桥模式下 `g_power_adc_trig` 映射到 phase 而非 duty |

#### 2e. 循环边界 (全局)

| 标记 | 函数 | 改为 |
|------|------|------|
| LOOP-1 | `PowerTypeFun()` L1045-1130 | `for(i=0;i<POTNUM;i++)` — 自动跟随宏 |
| LOOP-2 | `powerZeroChange()` L4738-4838 | 同上 |
| LOOP-3 | `APP_POWER_PanStartPluse()` L1331-1343 | 同上 |
| LOOP-4 | `APP_POWER_CompSetValue()` L986-1027 | 同上 |
| LOOP-5 | `APP_POWER_PotCheckRest()` L4444-4465 | 同上 |
| LOOP-6 | `u_power_init()` L5017-5078 | 同上 + Power1/2FunTable 引用 `#ifdef` |

#### 2f. 同步

| 标记 | 当前 | 改为 |
|------|------|------|
| SYNC-1 | L5587 `TIMsynchronousPower()` | `#ifdef FULL_BRIDGE_MODE` → `API_HRTIM_MasterSync_StartAll()` / `#else` → 保持 |

#### 2g. 频率管理

| 标记 | 当前 | 改为 |
|------|------|------|
| FREQ-1 | `PPGsetHalf(ch, pwm)` 每通道独立频率 | `#ifdef FULL_BRIDGE_MODE` → `API_FB_SetFrequency(fbCh, freqHz)` 双头同频 |
| FREQ-2 | L5579-5588 周期偏差检测 | `#ifndef FULL_BRIDGE_MODE` 包裹 — 全桥下 MASTER 统一周期不需此检查 |

### 文件 3: `app_task.c` — 系统初始化

**文件**: [app_task.c](src/RX32G410_FW_HAL_V1.3N/Projects/APP/POWER/src/app_task.c)

| 标记 | 行号 | 当前 | 改为 |
|------|------|------|------|
| TASK-1 | L281-282 | `API_HRTIM1_Init()` + `MasterSync_InitMaster(...)` | `#ifdef FULL_BRIDGE_MODE` → + `API_FB_Init_MasterSync(PotCh1)` + `API_FB_Init_MasterSync(PotCh2)` |
| TASK-2 | L316-324 | `API_PPG_setValue(PotCh1..4, value)` 4× | `#ifdef FULL_BRIDGE_MODE` → `API_FB_OutputFreqModulation` 2× / `#else` → 保持 |
| TASK-3 | L327-334 | `HRTIM_SYN_pin` toggle | (已在 hrtim-master-sync 中处理) |

### 文件 4: `API_HRTIM.h` — 通道常量 (几乎不改)

**文件**: [API_HRTIM.h:18-30](src/RX32G410_FW_HAL_V1.3N/Projects/API/inc/API_HRTIM.h#L18)

| 标记 | 当前 | 操作 |
|------|------|------|
| APIH-1 | `PotCh1..4, PotMax, PotNum` | **保留不变**, 新增注释: "全桥模式: PotCh1=FB1, PotCh2=FB2, PotCh3/4 未使用但保留索引" |

### 文件 5: `rx32g4xx_it.c` — ISR 入口 (不改)

ISR 入口保持 4 个，全桥模式下 PotCh3/PotCh4 对应的 `HRTIM1_TIMA_IRQHandler` / `HRTIM1_TIMD_IRQHandler` 不会被触发 (全桥初始化后那些 Timer 不会产生 PAN 中断)。

### 文件 6: `API_hrtim_fullbridge.c` — 不需修改

`FB_HW_MAP[]` 已适配 FB1=MCMP1, FB2=MCMP2。

### 文件 7: MODBUS / CLAUDE.md 寄存器表

| 标记 | 当前 | 改为 |
|------|------|------|
| MOD-1 | 0x1000-0x1014 遥测 4通道 | `#ifdef FULL_BRIDGE_MODE` → 前 2 通道有效 / `#else` → 4通道 |
| MOD-2 | Phase_Position (0x100E) | 全桥上报 phase_shift (0x1020-0x1021, EKF 补丁预留) |

---

## 五、不变的部分 (两种模式共用)

| 模块 | 说明 |
|------|------|
| HRTIM HAL 驱动 | 完全不改 |
| MODBUS 帧解析 | 帧结构不变 |
| PID (`FixedPID_Compute`) | 算法不变 |
| NTC 温度/风扇/保护 | 不变 |
| I2C/FMAC/DMA | 不变 |
| Timer C + Timer F | ADC 触发和保护角色不变 |

---

## 六、执行顺序

```
Phase 0: hrtim-master-sync (前置清理)
  ↓
Phase 1: 开关框架 (HDR-1/2, ARR-1/2/3, FT-1/2)
  植入 FULL_BRIDGE_MODE 开关 + 派生常量 + 数组维度 + 函数表条件化
  验证: 半桥模式编译 0e0w
  ↓
Phase 2: PPG 包装器条件化 (PPG-1~4, LOOP-1~6)
  每通道包装器 #ifdef 双实现, 循环自动跟随 POTNUM
  验证: 半桥/全桥两模式分别编译 0e0w
  ↓
Phase 3: 功率控制核心适配 (CORE-1~4, SYNC-1, FREQ-1/2)
  控制输出路径条件化
  验证: 全桥模式编译 0e0w
  ↓
Phase 4: 系统初始化 (TASK-1~3, APIH-1)
  app_task.c 初始化条件化
  ↓
Phase 5: MODBUS + 锅检适配
  ↓
Phase 6: 硬件测试
```

## 七、关键约束

1. **双炉头同频**: 全桥模式下两个 `fbCh` 必须调用相同频率
2. **移相范围**: 0~(MASTER_PERIOD-1), 0=同相(最大功率), 靠近 PERIOD=反相(最小功率)
3. **切换方式**: 用户改一行 (`#define FULL_BRIDGE_MODE` 注释/取消注释)，重新编译即可
4. **死区时间**: 全桥需要死区防止直通，`API_FB_SetDeadTime()` 已就绪
5. **编译验证**: 需要完整 Keil 工程 (`ReTek.uvprojx`)

**Why:** 移相全桥功率更大、损耗更低。双头同频消除差频干扰。一键切换确保两种配置同步演进不分裂。

**How to apply:** 用户说"执行 migrate-full-bridge"时，从 Phase 1 开始，每 Phase 结束分别编译半桥/全桥模式验证。
