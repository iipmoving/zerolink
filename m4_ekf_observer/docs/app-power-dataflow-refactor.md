# APP_POWER 数据流重构 — APP↔DRV 分层规划

> 基于 `generic-sleeping-metcalfe.md` 婴儿模块架构 + 实际现状修订
> 日期: 2026-06-09 | 状态: 规划阶段

---

## 一、Context

### 1.1 现状

| 项目 | 说明 |
|------|------|
| 编译文件 | `LIB/APP/app_power_claude.c` (~6400行) |
| 当前架构 | `MODULE_SKELETON(APP_Power)` + 函数指针表调用空桩 |
| HRTIM 依赖 | 已剥离 Phase 1（全部 `_stub_*` 空函数），但数据黑洞化——计算完即丢弃 |
| 检锅/起振 | 内联在 APP_Power 中，~30+ 处散布 |
| 周期 | APP_Power 每 10ms 执行一次 |

### 1.2 问题

- 函数指针表（`AppPowerFunDef`）+ 空桩只是"伪解耦"，数据仍无归宿
- 检锅状态机、脉冲 DMA/FMAC 处理与功率控制逻辑交织
- 读 HRTIM 操作（`FunPPGgetValue` / `FunTimBkFlag`）不能移除，因为当前状态与保护逻辑耦合
- 半桥→全桥切换需要在 APP 中改动

### 1.3 目标

1. **APP = 纯业务**：只做 PID + 目标功率 + 状态决策，零 HRTIM 访问
2. **DRV = 单入口多 Route**：`DoWork` 内按 Route 分发到 检锅/输出累积/保护/同步/状态反馈
3. **数据流驱动**：APP → g_out.hw_cmd → data_switcher → DrvHrtimConsumer.g_in
4. **桥模式无关**：APP 不知道半桥/全桥，DRV 编译时选实现

---

## 二、架构总览

```
      10ms 周期                       1ms/ISR 实时
┌────────────────────┐        ┌──────────────────────┐
│  APP 层             │        │  BASE_CLASS 层        │
│                     │        │                       │
│  app_power_claude.c │        │  drv_hrtim_consumer.c │
│                     │        │                       │
│  ├─ power_pid       │  g_out │  DoWork →             │
│  ├─ power_protect   │───────→│  ├─ Route: pan_detect  │
│  ├─ power_pan_if    │  hw_   │  ├─ Route: output_apply│
│  ├─ power_output    │  cmd   │  ├─ Route: protect     │
│  └─ hw_cmd 打包     │        │  ├─ Route: sync       │
│                     │        │  └─ Route: status_fb   │
│  g_in ← hw_status   │←───────│       │               │
│         (反馈)      │g_out   │       ▼               │
└────────────────────┘        │  ISR 缓冲变量          │
         │                    │       │               │
         ▼                    │       ▼               │
  ┌──────────────┐            │  ISR → HRTIM 寄存器    │
  │ data_switcher │            └──────────────────────┘
  │ Slot 0: ADC   │
  │ Slot 1: Power │
  │ Slot 2: Drv   │
  └──────────────┘
```

### 2.1 路由顺序（同一次 Slot1 内）

```
Step 0: AppAdc.DoWork       → g_out.adc_data
↓ Route: ADC.g_out → Power.g_in.pAdc + Drv.g_in.adc (保护用)

Step 1: APP_Power.DoWork    → g_out.hw_cmd
  InputCallback:   g_in ← adc_data + hw_status(反馈)
  ProcessInput:
    power_pid()       → ppg_delta[4]
    power_protect()   → limit/flags
    power_pan_if()    → pan_request (只设标志)
    power_output()    → 打包 hw_cmd {delta, power_on, pan_request, ...}
↓ Route: Power.g_out.hw_cmd → Drv.g_in

Step 2: DrvHrtimConsumer.DoWork
  Route_pan_detect()   → 检锅状态机 + 起振序列
  Route_output_apply() → shadow_duty += delta → ISR 缓冲
  Route_protect()      → BK/限流/故障
  Route_sync()         → Master 同步周期
  Route_status_fb()    → hw_status{pan_result, bk_flag, current_duty, ...} → g_out
↓ Route: Drv.g_out.hw_status → Power.g_in (下周期)
```

---

## 三、数据结构定义

### 3.1 PowerHw_Command_t — APP → DRV 命令

```c
/* include/app_power_hw_io.h */
#pragma pack(4)

/* APP → DRV: 抽象功率控制命令（与桥模式无关） */
typedef struct {
    /* === 增量命令（DRV 内部累积） === */
    int16_t  ppg_delta[POWER_POTMAX];     /* PID 输出增减量                    */
    uint8_t  delta_valid[POWER_POTMAX];   /* 增量有效                          */

    /* === 开关命令 === */
    uint8_t  power_on[POWER_POTMAX];      /* 1=加热, 0=停止                    */
    uint8_t  power_state[POWER_POTMAX];   /* POWER_STA_IDLE/RUN/STOP (来自 APP) */

    /* === 检锅请求 === */
    uint8_t  pan_request_ch;              /* 请求检锅的通道号 (0xFF=无请求)    */

    /* === 同步请求 === */
    uint8_t  sync_request;                /* 请求频率同步                      */
    uint16_t target_freq;                 /* 目标频率（可选）                  */

} PowerHw_Command_t;
```

### 3.2 PowerHw_Status_t — DRV → APP 反馈

```c
/* DRV → APP: 硬件状态反馈（DRV 从 HW 或影子变量读取） */
typedef struct {
    /* === 当前输出状态（DRV 影子变量） === */
    uint16_t current_duty[POWER_POTMAX];  /* 当前 duty（给 APP 限幅参考）     */
    uint16_t current_period[POWER_POTMAX];/* 当前周期                          */

    /* === 检锅结果 === */
    uint8_t  pan_result[POWER_POTMAX];    /* 0=无锅, 1=有锅, 0xFF=检测中      */
    uint8_t  pan_pulse_count;             /* 检锅脉冲数（调试用）              */
    uint8_t  pan_fault;                   /* 检锅故障                          */

    /* === 保护状态 === */
    uint8_t  bk_flag[POWER_POTMAX];       /* BK 标志（DRV 从 HW 读取）        */
    uint8_t  fault[POWER_POTMAX];         /* 硬件故障码                        */
    uint8_t  surge_flag;                  /* 浪涌标志                          */

    /* === 数据有效 === */
    uint8_t  valid;

} PowerHw_Status_t;
```

### 3.3 扩展 PowerBase 接口

```c
/* include/app_power_io.h — 修改部分 */

/* 输入: 新增 hw_status 字段 */
typedef struct {
    PowerAdc_InHead_t    pAdc[POWER_POTMAX];
    PowerComm_InHead_t   head[POWER_POTMAX];
    PowerHw_Status_t     hw_status;          /* ← 新增: DRV 反馈               */
} PowerBase_Input_t;

/* 输出: 新增 hw_cmd 字段 */
typedef struct PowerBase_Output {
    PowerBase_OutHead_t  head[4];            /* ppg_delta + 状态               */
    PowerHw_Command_t    hw_cmd;             /* ← 新增: DRV 命令               */
} PowerBase_Output_t;
```

---

## 四、模块分解

### 4.1 APP 功能包（从 app_power_claude.c 拆出）

| 包名 | 文件 | 职责 | 原有代码来源 |
|------|------|------|-------------|
| power_pid | `app/power_pid.c/.h` | FixedPID_Compute | `s_ppg_fun()` PID 部分 |
| power_protect | `app/power_protect.c/.h` | 过压/过温/限流/浪涌判断 | `surge_Processing()`, `getOvpValueAdj()`, `igbt_derate()`, `top_temp_stop()` |
| power_pan_if | `app/power_pan_if.c/.h` | 检锅触发接口（只设 flag，不操作 HW） | 将原检锅状态机抽成"何时请求检锅"的业务判断 |
| power_output | `app/power_output.c/.h` | 收集各包结果 → 填充 hw_cmd | 原 ProcessInput 结尾 |

**约束**:
- APP 包之间平级，互不 include
- APP 包可以 include `app_power_io.h` / `app_adc_io.h`
- APP 包不可以 include `base_class/` 或 `src/` HAL
- APP 包不可以调用任何 `drv_*` / `API_*` 函数

### 4.2 DRV Route（base_class 层）

| Route | 文件 | 职责 | 原有代码来源 |
|-------|------|------|-------------|
| pan_detect | `base_class/src/drv_pan_detect.c` | 检锅状态机 + 起振脉冲序列 + DMA/FMAC 脉冲计数 | `PanStatusCheck()`, `check_pot_pluse()`, `s_pan_check_fun()`, `API_DMA_PAN_IRQHandlerCallBack`, `API_POWER_PanCheckPluse`, `RealTimePulseDetector` 全部 |
| output_apply | `base_class/src/drv_output_apply.c` | 累积 shadow_duty += ppg_delta + 准备 ISR 缓冲 | `FunPPGsetDuty()`, `FunPPGonOff()`, `FunDeadTimeSetValue()` |
| protect | `base_class/src/drv_protect.c` | 读取 HW 保护状态 (BK/限流) + 限幅 | `FunTimBkFlag()`, `FunPPGgetValue()` 限幅相关 |
| sync | `base_class/src/drv_sync.c` | Master 同步周期管理 | `drv_hrtim_master_sync_set_period()` |
| status_fb | 内联在 `drv_hrtim_consumer.c` | 收集各 Route 结果 → `PowerHw_Status_t` | 新逻辑 |

**约束**:
- Route 之间通过 `drv_hrtim_consumer.c` 内定义的共享结构体（`shadow_duty[]`、`shadow_period[]` 等）通信
- Route 文件可以有 `#ifdef FULL_BRIDGE_MODE` 条件编译分支
- Route 文件可以 `#include "API_HRTIM.h"` / `drv_hrtim.h`
- Route 文件不可以 `#include` APP 层任何 `.h`

### 4.3 DrvHrtimConsumer 主模块

```c
/* base_class/drv_hrtim_consumer.c */

MODULE_SKELETON(DrvHrtim);

/* === 共享内部状态（各 Route 共用） === */
static uint16_t s_shadow_duty[POWER_POTMAX];
static uint16_t s_shadow_period[POWER_POTMAX];
static uint8_t  s_shadow_on_off[POWER_POTMAX];
/* ... */

/* === ProcessInput: 单入口多 Route === */
static void ProcessInput(void)
{
    if (!(g_input.info.status & ST_NEW)) return;
    g_input.info.status &= ~ST_NEW;

    PowerHw_Command_t *cmd = (PowerHw_Command_t *)g_input.para;

    Route_pan_detect(cmd);          /* 1. 检锅 + 起振              */
    Route_protect(cmd);             /* 2. 保护/BK 读取 + 限幅      */
    Route_sync(cmd);                /* 3. 同步管理                 */
    Route_output_apply(cmd);        /* 4. 累积 duty → ISR 缓冲     */
    Route_status_fb();              /* 5. 收集 → g_out.hw_status   */
}

MODULE_EXPORT(DrvHrtim);
```

---

## 五、文件清单

### 5.1 新增文件（10 个）

```
m4_ekf_observer/
├── include/
│   └── app_power_hw_io.h          [NEW] PowerHw_Command_t / PowerHw_Status_t
│
├── app/
│   ├── power_pid.c                [NEW] PID 功能包
│   ├── power_pid.h                [NEW]
│   ├── power_protect.c            [NEW] 保护逻辑功能包
│   ├── power_protect.h            [NEW]
│   ├── power_pan_if.c             [NEW] 检锅触发接口
│   ├── power_pan_if.h             [NEW]
│   ├── power_output.c             [NEW] hw_cmd 打包
│   └── power_output.h             [NEW]
│
└── base_class/
    ├── drv_hrtim_consumer.c        [NEW] 单入口多 Route 主模块
    ├── inc/
    │   ├── drv_pan_detect.h        [NEW]
    │   ├── drv_output_apply.h      [NEW]
    │   ├── drv_protect.h           [NEW]
    │   └── drv_sync.h              [NEW]
    └── src/
        ├── drv_pan_detect.c        [NEW] 检锅状态机迁入
        ├── drv_output_apply.c      [NEW] 输出累积迁入
        ├── drv_protect.c           [NEW] 保护逻辑迁入
        └── drv_sync.c              [NEW] 同步管理迁入
```

### 5.2 修改文件（4 个）

```
m4_ekf_observer/
├── include/app_power_io.h         [MOD] +hw_cmd, +hw_status
├── LIB/APP/app_power_claude.c     [MOD] 拆包 + 删 HRTIM 桩 + 删检锅 + 删函数指针表
├── core/data_switcher.c           [MOD] +Slot 2, +路由
└── core/interface_map.h           [MOD] +新路由通道注册
```

### 5.3 删除内容（从 app_power_claude.c）

```
删除清单:
  _stub_ppg_init()                  L37
  _stub_ppg_dead_time()             L38
  _stub_ppg_set_duty()              L39
  _stub_ppg_get_value()             L40
  _stub_ppg_on_off()                L41
  _stub_ppg_get_adc_value()         L42
  _stub_pan_count_init()            L43
  _stub_pan_count_get_value()       L44
  _stub_pan_count_set_value()       L45
  _stub_tim_bk_flag()               L46

  AppPowerFunDef 结构体              L328-349 (函数指针表)

  RealTimePulseDetector              L378-415
  PanPluse 实例                      L421

  全部函数指针表初始化数组            L508-572

  检锅状态机函数:
    PanStatusCheck()                 L2177-2209
    s_pan_check_fun()                L3520-3597
    check_pot_pluse()                L3647-3728
    API_HRTIM_PanOffCallBack()       L3641
    APP_POWER_PanStartPluse()        L1452
    PowerPanCheckFun()               L1935
    s_pan_pot_check()                L~940

  脉冲计数/DMA/FMAC:
    API_DMA_PAN_IRQHandlerCallBack   L6019
    API_POWER_PanCheckPluse()        L6315
    APP_POWER_PanPluseMessage()      L6271
    pulse_detector_reset()           L~490
    Pulse_detector_process()         L~504
    APP_POWER_FmacSetPan()           (间接引用)

  全部 FunPPGsetDuty/FunPPGonOff/FunDeadTime 宏调用  (~20处)
  全部 FunPPGgetValue/FunTimBkFlag 宏调用           (~12处)
  全部 PPGsetValueCh1~4 空函数                      L6366-6369
  全部 PPGgetValueCh1~4 空函数                      L6371-6374
  全部 PPGsetDutyCh1~4 空函数                       L6376-6379
  全部 PPGdeadTimeCh1~4 空函数                      L6381-6384
  全部 APP_POWER_PPG_BkFlag_Pot1~4 空函数           L6386-6389
```

---

## 六、迁移步骤（按顺序执行）

| 阶段 | 步骤 | 内容 | 验证 |
|------|------|------|------|
| **1** | 1.1 | 新建 `include/app_power_hw_io.h` — `PowerHw_Command_t` + `PowerHw_Status_t` | `check_structs.py` |
| **1** | 1.2 | 新建 `power_pid.c/.h` — 从 `s_ppg_fun` 拆出 PID | 编译 0e0w |
| **1** | 1.3 | 新建 `power_protect.c/.h` — 从 surge/ovp/derate 拆出保护逻辑 | 编译 0e0w |
| **1** | 1.4 | 新建 `power_pan_if.c/.h` — 检锅触发接口（纯业务判断何时发请求） | 编译 0e0w |
| **1** | 1.5 | 新建 `power_output.c/.h` — 打包 `hw_cmd` | 编译 0e0w |
| **2** | 2.1 | 新建 `drv_pan_detect.c/.h` — 检锅状态机迁入 | 编译 0e0w |
| **2** | 2.2 | 新建 `drv_output_apply.c/.h` — 输出累积迁入 | 编译 0e0w |
| **2** | 2.3 | 新建 `drv_protect.c/.h` — 保护读取迁入 | 编译 0e0w |
| **2** | 2.4 | 新建 `drv_sync.c/.h` — 同步管理迁入 | 编译 0e0w |
| **2** | 2.5 | 新建 `drv_hrtim_consumer.c` — 主模块 + 5 Route 调度 | 编译 0e0w |
| **3** | 3.1 | 改 `include/app_power_io.h` — 扩展 `PowerBase_Input/Output_t` | `check_structs.py` |
| **3** | 3.2 | 改 `app_power_claude.c` — 替换为功能包调用 + 删 HRTIM 桩 + 删检锅代码 | 编译 0e0w |
| **3** | 3.3 | 改 `core/data_switcher.c` — 新增 Slot 2 + 路由 | 编译 0e0w |
| **3** | 3.4 | 改 `core/interface_map.h` — 注册新路由通道 | `check_weak_pairs.py` |
| **4** | 4.0 | **五件套验证** — check_deps + weak_pairs + structs + output_callback + 编译 | 全部 PASS |

---

## 七、验证方式

| 阶段 | 验证内容 | 命令/方式 |
|------|---------|----------|
| 编译 | 每步 0e0w | `armcc -c --cpu Cortex-M4 --c99 ...` |
| 层依赖 | APP 不 include base_class | `check_deps.py` |
| 结构体 | hw_cmd / hw_status 一致性 | `check_structs.py` |
| __weak 配对 | 路由通道完整性 | `check_weak_pairs.py` |
| 输出回调 | ST_OUT 合规 | `check_output_callback.py` |
| 功能 | 半桥模式下输出与基线一致 | PC 工具对比实际功率 |
| 功能 | 全桥模式下编译通过 | 条件编译切换验证 |

---

## 八、注意事项

1. **APP 功能包只迁逻辑，不改逻辑** — 原有业务语义不变，只改变数据流向
2. **函数指针表整块删除** — 不再需要 `PowerControl->funAdr->_PPG*` 间接层
3. **读操作短期走 __weak 回调** — `FunPPGgetValue` 等读操作 Step 3.2 阶段先通过 `__weak` 桥接，待 DRV 反馈稳定后切到 `g_in.hw_status`
4. **DRV 影子变量** — `s_shadow_duty[]` 由 drv_output_apply 维护，初始值为 0，每次 `cmd->ppg_delta[ch]` 累加
5. **检锅状态机迁移** — 原有 pan 状态变量（`PanPluse`、`PanCheckStep` 等）全部迁入 `drv_pan_detect.c` 作为 `static`
6. **半桥/全桥切换** — 切换点在 `drv_output_apply.c` / `drv_pan_detect.c` 内 `#ifdef FULL_BRIDGE_MODE`，APP 完全无感
