# IH 电参数计算模块 — 设计文档

> **项目**: 半桥IH电磁炉 电参数计算 C 模块  
> **状态**: 设计完成，待编码  
> **日期**: 2026-06-04  
> **关联**: MATLAB 模型 `sim/matlab_half_bridge _lima/`，交叉验证 `keil_port/ih_elec_params.py`

---

## 一、概述

### 1.1 目标

在 MCU 20ms 控制周期内，基于 1ms 积累的原始数据，计算 IH 加热系统的全部电参数：等效电感 L、谐振频率 f_res、品质因数 Q、等效电阻 R、有功功率 P、阻抗模 |Z| 等。

### 1.2 定位

本模块接受 20ms 帧结束后的批量数据，**不做实时采样**，不做 1ms 路径的峰值检测/电流积分。

### 1.3 与 MATLAB 模型的关系

公式链与 `export_golden.m` / `export_golden_batch.m` 逐行对齐，通过 `keil_port/ih_elec_params.py` 做 C DLL vs MATLAB golden 的交叉验证。

---

## 二、输入参数

### 2.1 输入列表

| # | 参数 | 符号 | 类型 | 数组长度 | 原始单位 | 说明 |
|---|------|------|------|---------|---------|------|
| 1 | 谐振电流峰值 | `I_peak_buf` | `uint16_t` | 20 | ADC 计数值 | 每开关周期一个值 |
| 2 | 母线电压 | `Vdc_buf` | `uint16_t` | 20 | ADC 计数值 | 每开关周期一个值 |
| 3 | 相位角 | `phi_buf` | `uint16_t` | 20 | 0.1° | 每开关周期一个值 |
| 4 | 开关频率 | `f_sw_buf` | `uint16_t` | 20 | Hz | 每开关周期一个值（暂缓） |
| 5 | 导通占比 | `duty_ratio` | `uint16_t` | 1 | 0.01% | `(highOff-highOn)×2/lowOff` |

> 导通占比单位定义：`5000 = 50.00%`，范围 0~10000。用于还原 φ 的真实电气角度。

### 2.2 输入处理

#### I_peak（峰值电流）
- **丢弃规则**: I_peak 低于阈值 `I_PEAK_MIN` 的周期，该周期的 4 元组全部丢弃
- **权重修正**: 对剩余 k 个有效周期：
  1. 找出最大 I_peak → `I_max`
  2. 该周期 L 作为参考 `L_ref`
  3. 其他周期 L 修正: `L_corr[i] = L_ref + (L_raw[i] - L_ref) × (I_peak[i] / I_max)²`
  4. 取 `median(L_corr[])` 作为最终 L
- **输出回显**: 有效 I_peak 的中位数

#### Vdc（母线电压）
- 全 20 点取算术平均

#### φ（相位角）
- 只保留高 I_peak 周期的 φ 值（与 I_peak 丢弃规则同步）
- 有效值取算术平均
- 帧间: `φ_smooth = 0.7 × φ_new + 0.3 × φ_prev`
- 还原真实角度: `φ_real = φ_c / duty_ratio`（duty_ratio 以 per-unit 计）

#### 导通占比（duty_ratio）
- 由 HRTIM 时序在 1ms 路径计算（或由 20ms 路径读取 CMP 参数计算）
- 帧内恒定，单值传入

---

## 三、计算管线

### 3.1 逐周期 L 计算

对每个有效周期 i，独立执行 KVL 封闭解：

```
输入: I_peak[i](A), Vdc[i](V), f_sw[i](Hz)
已知: C = 0.9 μF

ω_sw    = 2π × f_sw[i]                         (rad/s)
V_C_pk  = I_peak[i] / (ω_sw × C)                (V)
L[i]    = (Vdc[i]/2 + V_C_pk) / (I_peak[i] × ω_sw) × 1e6  (μH)
```

条件: `L[i] > 0`，否则丢弃该周期。

### 3.2 L 修正

```
L_ref = L[argmax(I_peak)]              // 最高 I_peak 周期的 L
I_max = max(I_peak)
for i in 有效周期:
    ratio  = I_peak[i] / I_max
    weight = ratio²
    L_corr[i] = L_ref + (L[i] - L_ref) × weight

L_med = median(L_corr[])
```

### 3.3 L → f_res

```
f_res = 1 / (2π√(L_med × 1e-6 × C))           (Hz)
```

### 3.4 L + φ → Q / R / P / Z

```
φ_real = φ_smooth / duty_ratio                 还原到真实角度
φ_rad  = φ_real × π / 180
ω_sw   = 2π × f_sw_median
ω_res  = 2π × f_res

tan_φ  = tan(φ_rad)
ratio  = f_sw_median / (f_res / 1e3)
Q      = tan_φ / (ratio - 1/ratio)              (ratio≠1)

R      = ω_res × (L_med × 1e-6) / Q            (Q > 0)

I_rms  = I_peak_med / √2
P      = I_rms² × R

X_L    = ω_sw × (L_med × 1e-6)
X_C    = 1 / (ω_sw × C)
X      = X_L - X_C
Z      = √(R² + X²)
```

### 3.5 Kalman 滤波（L 后处理）

```
状态: x = L_uH (一维标量)
模型: F = 1 (随机游走)

预测:
  x_pred = x_est
  P_pred = P + Q_kal                    (Q_kal = 0.10)

更新:
  innov  = L_meas - x_pred
  K      = P_pred / (P_pred + R_kal)    (R_kal = R₀ / (I_rms / 10))
  x_est  = x_pred + K × innov
  P      = (1 - K) × P_pred

突变检测:
  |innov| > 3.5σ_run                   (σ_run = 50 点滑动标准差)
  → 重置滤波器, 更新即时值
```

#### 两条输出

| 通道 | 用途 | 响应 |
|------|------|------|
| 稳定值 `L_stable` | 电参数计算、锅具识别 | 滤波后平滑输出 |
| 即时值 `L_fast` | 抬锅/移锅/干烧检测 | 突变检测，30ms 响应 |

---

## 四、输出参数

| 参数 | 符号 | 单位 | 来源 |
|------|------|------|------|
| 等效电感 | `L_uH` | μH | Kalman 稳定值 |
| 谐振频率 | `f_res_kHz` | kHz | L + C 反推 |
| 品质因数 | `Q_factor` | — | φ + f_sw/f_res |
| 等效电阻 | `R_ohm` | Ω | ω₀L/Q |
| 有功功率 | `P_W` | W | I_rms² × R |
| 有效电流 | `I_rms` | A | I_peak / √2 |
| 阻抗模 | `Z_mag_ohm` | Ω | √(R² + X²) |
| 总电抗 | `X_ohm` | Ω | X_L - X_C |
| 相位角 | `phi_deg` | ° | 还原后的真实 φ |
| 状态 | `status` | enum | 正常/抬锅/移锅/干烧/空载 |

---

## 五、接口 API

```c
typedef struct {
    uint16_t* I_peak_buf;       /* [20] 谐振电流峰值 ADC */
    uint16_t* Vdc_buf;          /* [20] 母线电压 ADC */
    uint16_t* phi_buf;          /* [20] 相位角 (0.1°) */
    uint16_t* f_sw_buf;         /* [20] 开关频率 (Hz) — 暂缓 */
    uint16_t  duty_ratio;       /* 导通占比 0.01% */
    uint8_t   count;            /* 有效数据个数 (≤20) */
} IH_ElecInput;

typedef struct {
    float L_uH;                 /* 等效电感 */
    float f_res_kHz;            /* 谐振频率 */
    float Q_factor;             /* 品质因数 */
    float R_ohm;                /* 等效电阻 */
    float P_W;                  /* 有功功率 */
    float I_rms;                /* 有效电流 */
    float Z_mag_ohm;            /* 阻抗模 */
    float X_ohm;                /* 总电抗 */
    float phi_deg;              /* 真实相位角 */
    float Vdc_mean;             /* 母线电压均值 */

    float L_stable;             /* Kalman 稳定值 */
    float L_fast;               /* Kalman 即时值 */
    uint8_t event;              /* 0=正常, 1=抬锅, 2=移锅, 3=干烧 */

    bool valid;
} IH_ElecResult;

void IH_CalculateParams(IH_ElecInput* in, IH_ElecResult* out);
```

---

## 六、交叉验证方案

| 层级 | 工具 | 验证内容 |
|------|------|---------|
| Python 参考 | `ih_elec_params.py` | 公式链与 MATLAB 342/342 周期 0.00% 偏差 |
| C DLL vs Python | `armclang` → `.dll` → `cross_validate` | C 实现 vs Python 参考 |
| C DLL vs Golden | `--golden golden_batch.json` | C 实现 vs MATLAB golden |
| MCU 在线 | 串口输出 L 值 | 对比 PC 工具离线计算结果 |

---

## 七、待实现事项

| # | 事项 | 依赖 |
|---|------|------|
| 1 | φ 还原方案确定（1ms 改算法 vs 传入 duty_ratio） | 选一个 |
| 2 | f_sw 输入（含上下管不等长） | 用户提供 |
| 3 | Kalman 滤波器 C 编码 | 参数从 MATLAB 移植 |
| 4 | 抬锅/移锅/干烧状态机 | Kalman 突变检测输出 |
| 5 | `power_calculator.c` 集成 | 本模块完成之后 |
