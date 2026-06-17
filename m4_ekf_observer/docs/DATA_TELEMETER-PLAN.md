# 项目计划: Calculator → ElecParams 数据流 MODBUS 遥测

> **目标**: 将 0 号炉头的 Calculator 输入(20周期×1ms)和 ElecParams 输出(1组×20ms)通过 MODBUS 传输到 PC，GUI 保存到 CSV 验证数据流
> **创建日期**: 2026-06-17
> **方案**: 新建 `app/data_telemetry.c/h` 内嵌缓冲区，在 elec_params.c 中复制数据，MODBUS 0x7000 区映射

---

## 一、数据结构

### 1.1 总览

| 区域 | 字段数 | 数据类型 | 说明 |
|------|--------|---------|------|
| 控制 | 4 words | uint16 | R/W |
| Calculator 输入 | 20周期 × N字段 | uint16 | 复制 s_inPara |
| ElecParams 输出 | 1组 × 13字段 | float | 复制 s_outPara |

**ElecParams 输出是 float 类型**（见 `MODULE_OUTPUT_PARAMS(ElecParams, EKF_LKF)`），直接存 float 到缓冲区，PC 端 CSV 显示时转小数。

### 1.2 Calculator 每周期字段

从 `MODULE_INPUT_PARAMS(Calculator, ElecParams)` 中提取：

```
hrtim_highOff        uint16
hrtim_lowOff         uint16
hrtim_highOn         uint16
hrtim_lowOn          uint16
peak_current         uint16
active_current_sum_high  uint32  → 拆 2×uint16
active_current_sum_low   uint32  → 拆 2×uint16
voltage_sum            uint32  → 拆 2×uint16
voltage_count          uint16
zero_cross_high        uint16
zero_cross_low         uint16
peak_point             uint16
```

共 15 字段（其中 3 个 uint32 拆成 6 个 uint16），实际占用 **18 uint16/周期**。
20 周期 = 360 words。

### 1.3 ElecParams 输出字段

从 `MODULE_OUTPUT_PARAMS(ElecParams, EKF_LKF)`（float 类型）：

```
I_peak_A       float   (A)
Vdc_mean       float   (V)
phi_deg        float   (°)
f_sw_Hz        float   (Hz)
L_uH           float   (μH)
f_res_kHz      float   (kHz)
Q_factor       float
R_ohm          float   (Ω)
I_rms          float   (A)
P_W            float   (W)
Z_mag_ohm      float   (Ω)
X_ohm          float   (Ω)
valid          uint8 + 3 padding
```

13 字段 = 12×float(4B) + 1×uint8 + 3pad = **52 bytes = 26 uint16**。

### 1.4 总大小

```
控制:        4 words (16B)
Calculator: 20×18 = 360 words (720B)
ElecParams:  26 words (52B)
总计:       390 words = 780 bytes
```

---

## 二、文件位置

```
项目根: D:\OBSIDIAN\MOVING IH\ZEROLINK\m4_ekf_observer\

新建文件:
  app/data_telemetry.h        — 结构体 + API
  app/data_telemetry.c        — 初始化 + 复制函数

修改文件:
  src/.../BaseClass/src/elec_params.c    — user_Process 中调用 Telemetry_Copy
  src/.../modbus/src/Modbus_Lib_Init_An_Analysis.c  — 新增 Area 5
  src/.../modbus/src/Modbus_Lib_Init_An_Analysis.h   — 新增 area 索引

PC 端:
  tools/ekf_tuner/m4_modbus_tool.py  — 新增 read_telemetry_pipe()
  tools/ekf_tuner/m4_gui.py          — 新增数据流验证面板

Keil:
  src/.../Projects/Keil/ReTek.uvprojx  — 新增 data_telemetry.c 编译
```

**include 路径**（参考 elec_params.c 中已有的 `#include "API_gpio.h"`）：
- `app/data_telemetry.h` 从 `BaseClass/src/` 引用：`#include "../../../../app/data_telemetry.h"`
- `app/data_telemetry.c` 从 Keil 项目引用，include path 已有 `../../../../../app`

---

## 三、实施步骤

### Step 1: 新建 app/data_telemetry.h

```c
/**
 * @file    data_telemetry.h
 * @brief   Telemetry — Calculator→ElecParams 数据流 MODBUS 遥测
 * @note    内嵌缓冲区，零指针跟随，MODBUS 直接映射
 */

#ifndef DATA_TELEMETRY_H
#define DATA_TELEMETRY_H

#include <stdint.h>

#define TELE_CALC_PERIODS  20
#define TELE_CALC_WORDS    18   /* 每周期 18 uint16 (含 uint32 split) */
#define TELE_ELEC_WORDS    26   /* 12 float(2words) + valid(1word) */
#define TELE_TOTAL_WORDS   (4 + TELE_CALC_PERIODS * TELE_CALC_WORDS + TELE_ELEC_WORDS)
/* 4 + 360 + 26 = 390 */

/* ---- 控制寄存器 ---- */
typedef struct {
    uint16_t ctrl_start;      /* 写1=开始监测, 写0=停止 */
    uint16_t reserved;
    uint16_t status;          /* bit0=running, bit1=calc_ready, bit2=elec_ready */
    uint16_t reserved2;
} TelemCtrl_t;

/* ---- 单周期数据 (18 uint16) ---- */
typedef struct {
    uint16_t hrtim_highOff;
    uint16_t hrtim_lowOff;
    uint16_t hrtim_highOn;
    uint16_t hrtim_lowOn;
    uint16_t peak_current;
    uint16_t act_curr_high_lo;
    uint16_t act_curr_high_hi;
    uint16_t act_curr_low_lo;
    uint16_t act_curr_low_hi;
    uint16_t volt_sum_lo;
    uint16_t volt_sum_hi;
    uint16_t voltage_count;
    uint16_t zero_cross_high;
    uint16_t zero_cross_low;
    uint16_t peak_point;
    uint16_t res[3];          /* 补齐到 18 */
} TelemCalcPeriod_t;

/* ---- 输出聚合 ---- */
typedef struct {
    TelemCtrl_t    ctrl;
    TelemCalcPeriod_t calc[TELE_CALC_PERIODS];
    float          elec[12];  /* 12 float = 24 uint16 */
    uint16_t       elec_valid;
} TelemetryData_t;

/* API */
void Telemetry_Init(void);
TelemetryData_t* Telemetry_GetData(void);

#endif /* DATA_TELEMETRY_H */
```

### Step 2: 新建 app/data_telemetry.c

```c
/**
 * @file    data_telemetry.c
 * @brief   Telemetry — 数据流复制实现
 */

#include "data_telemetry.h"
#include <string.h>

static TelemetryData_t s_data;

void Telemetry_Init(void)
{
    memset(&s_data, 0, sizeof(s_data));
}

TelemetryData_t* Telemetry_GetData(void)
{
    return &s_data;
}
```

### Step 3: 修改 BaseClass/src/elec_params.c

在 `user_Process` 开头和结尾分别复制输入和输出：

```c
// elec_params.c 顶部新增 include
#include "../../../../app/data_telemetry.h"
```

在 `user_Process` 中：

```c
static void user_Process(MODULE_INPUT(ElecParams) *in, MODULE_OUTPUT(ElecParams) *out, ElecParams_PipeFlags_t flags)
{
    if (!(in->Calculator_params->status & ST_NEW)) return;

    TelemetryData_t *td = Telemetry_GetData();
    td->ctrl.status |= 0x01;  // running

    /* ---- 复制 Calculator → ElecParams 输入 (head 0, 20周期) ---- */
    // in->Calculator_params 指向 MODULE_INPUT_LINK(Calculator, ElecParams)
    // params[head][cycle] 是 MODULE_INPUT_PARAMS(Calculator, ElecParams)
    const MODULE_INPUT_PARAMS(Calculator, ElecParams) *calc =
        &in->Calculator_params->params[0][0];  // head 0, 20周期

    for (uint8_t c = 0; c < TELE_CALC_PERIODS; c++) {
        TelemCalcPeriod_t *dst = &td->calc[c];
        const MODULE_INPUT_PARAMS(Calculator, ElecParams) *src = &calc[c];

        dst->hrtim_highOff   = src->hrtim_highOff;
        dst->hrtim_lowOff    = src->hrtim_lowOff;
        dst->hrtim_highOn    = src->hrtim_highOn;
        dst->hrtim_lowOn     = src->hrtim_lowOn;
        dst->peak_current    = src->peak_current;
        dst->act_curr_high_lo = (uint16_t)src->active_current_sum_high;
        dst->act_curr_high_hi = (uint16_t)(src->active_current_sum_high >> 16);
        dst->act_curr_low_lo  = (uint16_t)src->active_current_sum_low;
        dst->act_curr_low_hi  = (uint16_t)(src->active_current_sum_low >> 16);
        dst->volt_sum_lo      = (uint16_t)src->voltage_sum;
        dst->volt_sum_hi      = (uint16_t)(src->voltage_sum >> 16);
        dst->voltage_count    = src->voltage_count;
        dst->zero_cross_high  = src->zero_cross_high;
        dst->zero_cross_low   = src->zero_cross_low;
        dst->peak_point       = src->peak_point;
    }
    td->ctrl.status |= 0x02;  // calc_ready

    /* ---- 原有计算逻辑 ---- */
    API_GPIO_WritePin(DebugB_pin, 1);

    ElecParams_Ws *ws = &new;
    if (ws < (ElecParams_Ws *)0x10000000) return;

    for (uint8_t h = 0; h < ELEC_POTMAX; h++) {
        MODULE_OUTPUT_PARAMS(ElecParams, EKF_LKF)* result;
        result = &out->EKF_LKF_params.params[h];
        const MODULE_INPUT_PARAMS(Calculator, ElecParams) *cycles =
            &in->Calculator_params->params[h][PERIO_CNT];
        uint8_t calc_ok = ElecParams_Calc(result, cycles, ws);
    }
    API_GPIO_WritePin(DebugB_pin, 0);

    /* ---- 复制 ElecParams → EKF_LKF 输出 (head 0) ---- */
    // out->EKF_LKF_params.params[0] 是 MODULE_OUTPUT_PARAMS(ElecParams, EKF_LKF)
    // 注意: 这个结构体是 float 类型，直接复制到 td->elec[]
    const MODULE_OUTPUT_PARAMS(ElecParams, EKF_LKF) *ep =
        &out->EKF_LKF_params.params[0];

    // float 直接 memcpy (4B = 2 uint16)
    memcpy(td->elec, ep, 12 * sizeof(float));
    td->elec_valid = (uint16_t)ep->valid;

    td->ctrl.status |= 0x04;  // elec_ready
}
```

### Step 4: 修改 modbus/src/Modbus_Lib_Init_An_Analysis.c

```c
// 修改 AREA_COUNT
#define DF_Modbus_AREA_COUNT    6   /* 5→6 */

// 在 Modbus_Cofg_Init_SET 末尾, for 循环内新增 Area 5:
/* Area 5: 0x7000 Telemetry (只读 + ctrl R/W) */
s_areas[i][5].Start_Address   = 0x7000;
s_areas[i][5].End_Address     = 0x7000 + TELE_TOTAL_WORDS;  // 0x7000 + 390 = 0x7186
s_areas[i][5].Data_ptr        = (void*)Telemetry_GetData();
s_areas[i][5].Data_ptr_EEPROM = NULL;
s_areas[i][5].Check_Write_Data = NULL;
s_areas[i][5].Data_Size       = sizeof(unsigned short);
s_areas[i][5].Data_Pyte       = 0;
```

### Step 5: 修改 modbus/src/Modbus_Lib_Init_An_Analysis.h

```c
// 新增 area 索引
#define AREA_TELEMETRY    5
```

### Step 6: 修改 ReTek.uvprojx

在 `AppLibSrc` 组中新增：
```xml
<File>
  <FileName>data_telemetry.c</FileName>
  <FileType>1</FileType>
  <FilePath>..\..\..\..\..\app\data_telemetry.c</FilePath>
</File>
```

### Step 7: PC 端 — m4_modbus_tool.py

```python
TELEM_START_ADDR = 0x7000
TELEM_COUNT = 390

def read_telemetry_pipe(self) -> dict | None:
    """读取 0x7000 Telemetry 数据流"""
    raw = self.read_registers(TELEM_START_ADDR, TELEM_COUNT)
    if raw is None:
        return None
    
    result = {"timestamp": datetime.now().isoformat()}
    
    # 控制
    result["ctrl_start"] = raw[0]
    result["status"] = raw[2]
    
    # Calculator: 20周期 × 18字段
    result["calc_periods"] = []
    for p in range(20):
        base = 4 + p * 18
        period = {}
        period["hrtim_highOff"] = raw[base + 0]
        period["hrtim_lowOff"] = raw[base + 1]
        period["hrtim_highOn"] = raw[base + 2]
        period["hrtim_lowOn"] = raw[base + 3]
        period["peak_current"] = raw[base + 4]
        period["act_curr_high"] = (raw[base + 6] << 16) | raw[base + 5]
        period["act_curr_low"] = (raw[base + 8] << 16) | raw[base + 7]
        period["volt_sum"] = (raw[base + 10] << 16) | raw[base + 9]
        period["voltage_count"] = raw[base + 11]
        period["zero_cross_high"] = raw[base + 12]
        period["zero_cross_low"] = raw[base + 13]
        period["peak_point"] = raw[base + 14]
        result["calc_periods"].append(period)
    
    # ElecParams: 12 float → 从 raw[4 + 360] 开始, 每float=2words
    ep_base = 4 + 360
    result["elec"] = {}
    float_names = [
        "I_peak_A", "Vdc_mean", "phi_deg", "f_sw_Hz",
        "L_uH", "f_res_kHz", "Q_factor", "R_ohm",
        "I_rms", "P_W", "Z_mag_ohm", "X_ohm"
    ]
    import struct
    for i, name in enumerate(float_names):
        w0 = raw[ep_base + i * 2]
        w1 = raw[ep_base + i * 2 + 1]
        uint32_val = w1 << 16 | w0
        bytes_val = struct.pack('<I', uint32_val)
        result["elec"][name] = struct.unpack('<f', bytes_val)[0]
    result["elec"]["valid"] = raw[ep_base + 24]
    
    return result
```

### Step 8: PC 端 — m4_gui.py

新增 "数据流验证" 标签页：
- Calculator 数据表格 (20行 × 15列关键字段)
- ElecParams 数据表格 (1行 × 12列, float 值直接显示)
- 保存 CSV 按钮

---

## 四、验证方案

1. **Calculator 完整性**: 20 周期中 `hrtim_lowOff` 应接近 26112 (PERIOD_TICKS)
2. **ElecParams valid**: 应为 1
3. **数据流一致性**: ElecParams `I_peak_A` ≈ Calculator `peak_current × 0.02523`

---

## 五、文件清单

| 文件 | 操作 | 说明 |
|------|------|------|
| `app/data_telemetry.h` | **新建** | 结构体 + API |
| `app/data_telemetry.c` | **新建** | 初始化 |
| `BaseClass/src/elec_params.c` | **修改** | user_Process 中 Telemetry 复制 |
| `modbus/src/Modbus_Lib_Init_An_Analysis.c` | **修改** | AREA_COUNT 5→6, Area5 |
| `modbus/src/Modbus_Lib_Init_An_Analysis.h` | **修改** | AREA_TELEMETRY=5 |
| `tools/ekf_tuner/m4_modbus_tool.py` | **修改** | read_telemetry_pipe() |
| `tools/ekf_tuner/m4_gui.py` | **修改** | 数据流验证面板 |
| `ReTek.uvprojx` | **修改** | 新增 data_telemetry.c |
