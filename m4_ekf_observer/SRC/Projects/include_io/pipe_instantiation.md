# HALF 项目管道实例化关系

> 生成时间: 2026-06-16
> 项目: m4_ekf_observer (HALF)
> 架构: v2.3 LINK+PARAMS PULL 路由

---

## 数据流总览

```
AppAdc ──┬──→ AppPower     (4 fields: voltage, power, txa[4], phase[4])
         └──→ Calculator   (4 fields: resonant_current**, hrtim_values**, voltage_data**, input*)

Calculator ──┬──→ ElecParams     (12 fields: hrtim/peak/current_sum/voltage/zero_cross)
             └──→ AppPower       (4 fields: resonant_current, voltage, phase_angle, valid)

ElecParams ──┬──→ EKF_LKF        (10 fields: valid, I_peak, Vdc, phi, L, f_res, R, f_sw, P, Q)
             └──→ AppPower       (13 fields: 同上 + I_rms, Z_mag, X_ohm)

EKF_LKF ──→ AppPower            (5 fields: R, L, f_res, Q, valid)
```

## 管道实例化详细表

### Pipe 1: AppAdc → Calculator

| 端 | 变量 | 类型 | 声明位置 |
|---|---|---|---|
| Producer 输出 | `s_outPara.Calculator_params` | `MODULE_OUTPUT_LINK(AppAdc, Calculator)` | `adc_sensor.c:16` |
| ↳ `.params` | `MODULE_OUTPUT_PARAMS(AppAdc, Calculator)*` | 指向 `AdcFunRam.inputValue` 原始缓冲区 | |
| Consumer 输入 | `s_inPara->AppAdc_params` | `MODULE_INPUT_LINK(AppAdc, Calculator)*` | `calculator.c:15` |
| Switcher 赋值 | `s_inPara->AppAdc_params = &s_outPara.Calculator_params` | `data_switcher.c:63` | |

### Pipe 2: AppAdc → AppPower

| 端 | 变量 | 类型 | 声明位置 |
|---|---|---|---|
| Producer 输出 | `s_outPara.AppPower_params` | `MODULE_OUTPUT_LINK(AppAdc, AppPower)` | `adc_sensor.c:16` |
| ↳ `.params[4]` | 内嵌 `AppAdc_to_AppPower_Output_Params[4]` | 每炉头 voltage/power/txa/phase | |
| Consumer 输入 | `s_inPara->AppAdc_params` | `MODULE_INPUT_LINK(AppAdc, AppPower)*` | `app_power.c:18` |
| Switcher 赋值 | `s_inPara->AppAdc_params = &s_outPara.AppPower_params` | `data_switcher.c:54` | |

### Pipe 3: Calculator → ElecParams

| 端 | 变量 | 类型 | 声明位置 |
|---|---|---|---|
| Producer 输出 | `s_outPara.ElecParams_params` | `MODULE_OUTPUT_LINK(Calculator, ElecParams)` | `calculator.c:16` |
| ↳ `.params[4][20]` | 内嵌 `Calculator_to_ElecParams_Output_Params[4][20]` | 4 炉头 × 20 周期 | |
| Consumer 输入 | `s_inPara->Calculator_params` | `MODULE_INPUT_LINK(Calculator, ElecParams)*` | `elec_params.c:15` |
| Switcher 赋值 | `s_inPara->Calculator_params = &s_outPara.ElecParams_params` | `data_switcher.c:75` | |

### Pipe 4: Calculator → AppPower

| 端 | 变量 | 类型 | 声明位置 |
|---|---|---|---|
| Producer 输出 | `s_outPara.AppPower_params` | `MODULE_OUTPUT_LINK(Calculator, AppPower)` | `calculator.c:16` |
| ↳ `.params[4]` | 内嵌 `Calculator_to_AppPower_Output_Params[4]` | 20ms 周期累积平均值 | |
| Consumer 输入 | `s_inPara->Calculator_params` | `MODULE_INPUT_LINK(Calculator, AppPower)*` | `app_power.c:18` |
| Switcher 赋值 | `s_inPara->Calculator_params = &s_outPara.AppPower_params` | `data_switcher.c:55` | |

### Pipe 5: ElecParams → EKF_LKF

| 端 | 变量 | 类型 | 声明位置 |
|---|---|---|---|
| Producer 输出 | `s_outPara.EKF_LKF_params` | `MODULE_OUTPUT_LINK(ElecParams, EKF_LKF)` | `elec_params.c:16` |
| ↳ `.params[4]` | 内嵌 `ElecParams_to_EKF_LKF_Output_Params[4]` | 电参数观测值 | |
| Consumer 输入 | `s_inPara->ElecParams_params` | `MODULE_INPUT_LINK(ElecParams, EKF_LKF)*` | `ekf_lkf.c:15` |
| Switcher 赋值 | `s_inPara->ElecParams_params = &s_outPara.EKF_LKF_params` | `data_switcher.c:69` | |

### Pipe 6: ElecParams → AppPower

| 端 | 变量 | 类型 | 声明位置 |
|---|---|---|---|
| Producer 输出 | `s_outPara.AppPower_params` | `MODULE_OUTPUT_LINK(ElecParams, AppPower)` | `elec_params.c:16` |
| ↳ `.params[4]` | 内嵌 `ElecParams_to_AppPower_Output_Params[4]` | 含 I_rms/Z/X 的完整电参数 | |
| Consumer 输入 | `s_inPara->ElecParams_params` | `MODULE_INPUT_LINK(ElecParams, AppPower)*` | `app_power.c:18` |
| Switcher 赋值 | `s_inPara->ElecParams_params = &s_outPara.AppPower_params` | `data_switcher.c:56` | |

### Pipe 7: EKF_LKF → AppPower

| 端 | 变量 | 类型 | 声明位置 |
|---|---|---|---|
| Producer 输出 | `s_outPara.AppPower_params` | `MODULE_OUTPUT_LINK(EKF_LKF, AppPower)` | `ekf_lkf.c:16` |
| ↳ `.params[4]` | 内嵌 `EKF_LKF_to_AppPower_Output_Params[4]` | 滤波后的 R/L/f_res/Q | |
| Consumer 输入 | `s_inPara->EKF_LKF_params` | `MODULE_INPUT_LINK(EKF_LKF, AppPower)*` | `app_power.c:18` |
| Switcher 赋值 | `s_inPara->EKF_LKF_params = &s_outPara.AppPower_params` | `data_switcher.c:57` | |

---

## 模块 I/O 实体汇总

### AppAdc (adc_sensor.c)

| 变量 | 类型 | 用途 |
|---|---|---|
| `s_inPara` | `MODULE_INPUT(AppAdc)*` | 无输入管道 |
| `s_outPara` | `MODULE_OUTPUT(AppAdc)` | 持有 Calculator_params + AppPower_params 两个输出 LINK |

### Calculator (calculator.c)

| 变量 | 类型 | 用途 |
|---|---|---|
| `s_inPara` | `MODULE_INPUT(Calculator)*` | 接收 AppAdc → Calculator |
| `s_outPara` | `MODULE_OUTPUT(Calculator)` | 持有 ElecParams_params + AppPower_params 两个输出 LINK |

### ElecParams (elec_params.c)

| 变量 | 类型 | 用途 |
|---|---|---|
| `s_inPara` | `MODULE_INPUT(ElecParams)*` | 接收 Calculator → ElecParams |
| `s_outPara` | `MODULE_OUTPUT(ElecParams)` | 持有 EKF_LKF_params + AppPower_params 两个输出 LINK |

### EKF_LKF (ekf_lkf.c)

| 变量 | 类型 | 用途 |
|---|---|---|
| `s_inPara` | `MODULE_INPUT(EKF_LKF)*` | 接收 ElecParams → EKF_LKF |
| `s_outPara` | `MODULE_OUTPUT(EKF_LKF)` | 持有 AppPower_params 一个输出 LINK |

### AppPower (app_power.c)

| 变量 | 类型 | 用途 |
|---|---|---|
| `s_inPara` | `MODULE_INPUT(AppPower)*` | 接收 4 条输入管道 (AppAdc/Calculator/ElecParams/EKF_LKF) |
| `s_outPara` | `MODULE_OUTPUT(AppPower)` | 无输出管道 (终端消费者) |

---

## 关键观察

### 1. AppPower 的 s_inPara 是 4 路输入共享

`MODULE_INPUT(AppPower)` 包含 4 个成员，每个指向不同 producer 的输出 LINK：

```c
typedef struct {
    MODULE_INPUT_LINK(AppAdc, AppPower) *AppAdc_params;
    MODULE_INPUT_LINK(Calculator, AppPower) *Calculator_params;
    MODULE_INPUT_LINK(ElecParams, AppPower) *ElecParams_params;
    MODULE_INPUT_LINK(EKF_LKF, AppPower) *EKF_LKF_params;
} MODULE_INPUT(AppPower);
```

Switcher 通过 4 条 `INPUT_GET_SLOT` 分别赋值：

```c
// data_switcher.c:52-58
INPUT_CALLBACK(AppAdc, AppPower) {
    INPUT_GET_SLOT(AppAdc, AppPower);
    INPUT_GET_SLOT(Calculator, AppPower);
    INPUT_GET_SLOT(ElecParams, AppPower);
    INPUT_GET_SLOT(EKF_LKF, AppPower);
}
```

### 2. Calculator / ElecParams 各输出到 2 个 consumer

- Calculator 的 `s_outPara` 同时持有 `ElecParams_params` 和 `AppPower_params`
- ElecParams 的 `s_outPara` 同时持有 `EKF_LKF_params` 和 `AppPower_params`

Switcher 对每个 consumer 单独赋值，两者指向同一块物理内存。

### 3. AppAdc→Calculator 的 `.params` 是指针，其他是内嵌数组

| 管道 | params 类型 | 说明 |
|---|---|---|
| AppAdc → Calculator | `MODULE_OUTPUT_PARAMS*` (指针) | 指向 `AdcFunRam.inputValue` 原始缓冲区 |
| AppAdc → AppPower | `MODULE_OUTPUT_PARAMS[4]` (内嵌数组) | 每炉头独立存储 |
| Calculator → ElecParams | `MODULE_OUTPUT_PARAMS[4][20]` (二维数组) | 4 炉头 × 20 周期 |
| Calculator → AppPower | `MODULE_OUTPUT_PARAMS[4]` (内嵌数组) | 20ms 周期累积平均值 |
| ElecParams → EKF_LKF | `MODULE_OUTPUT_PARAMS[4]` (内嵌数组) | |
| ElecParams → AppPower | `MODULE_OUTPUT_PARAMS[4]` (内嵌数组) | |
| EKF_LKF → AppPower | `MODULE_OUTPUT_PARAMS[4]` (内嵌数组) | |

### 4. EKF_LKF 使用 ST_OUT 而非 ST_NEW

ElecParams 同时输出给 EKF_LKF 和 AppPower，数据不能被独占消费，所以 EKF_LKF 检查 `ST_OUT`：

```c
// ekf_lkf.c:287
if (!(in->ElecParams_params->status & ST_OUT)) return;
// 不清除 ST_OUT — 这是 ElecParams 共享输出 LINK
```

其他消费者均检查 `ST_NEW`：

```c
// calculator.c:27
flags.bits.appadc = (in->AppAdc_params->status & ST_NEW) ? 1 : 0;
```

### 5. 数据实体初始化链

```
Switcher_Init()
  → SLOT_GETIO(模块)  → 模块_GetIO(&pIn, &pOut, &pDoWork)
    → pIn  = &g_input, pOut = &g_output, pDoWork = DoWork

第一次 DoWork()
  → Constructor()
    → Init()
      → s_inPara = NULL
      → g_input.para = &s_inPara    ← 关键: para 指向 s_inPara 变量地址
      → g_output.para = &s_outPara

InputCallback(Producer, Consumer)
  → INPUT_GET_SLOT(Producer, Consumer)
    → __out = (MODULE_OUTPUT(Producer)*)s_slot[SLOT(Producer)].pOut->para
    → __in  = (MODULE_INPUT(Consumer)*)s_slot[SLOT(Consumer)].pIn->para
    → __in->Producer_params = &__out->Consumer_params
      ↑ 这行通过 g_input.para (= &s_inPara) 间接写入 s_inPara 的内存

ProcessInput()
  → in = (MODULE_INPUT(Consumer)*)g_input.para
    ↑ in 的值 = s_inPara 的值 = 指向 producer params 的指针
  → flags = 检查 in->Producer_params->status & ST_NEW
```

---

*生成方式: 人工审计 + codeGen/check_null_ptrs.py 辅助验证*
