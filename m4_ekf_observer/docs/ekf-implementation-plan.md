# EKF 卡尔曼滤波 — 桌面端到C端落地计划

## 目录
1. [物理模型](#1-物理模型)
2. [桌面端原型 (Python)](#2-桌面端原型)
3. [测试场景清单](#3-测试场景清单)
4. [C端移植](#4-c端移植)
5. [验证闭环](#5-验证闭环)

---

## 1. 物理模型

### 1.1 半桥谐振跟踪

电磁炉半桥电路的核心控制目标是 **谐振频率跟踪** — 保持驱动频率与LC谐振频率一致，使IGBT在零电压/零电流点开关，功率传输效率最高。

| 物理量 | 符号 | 来源 | MODBUS寄存器 |
|--------|------|------|-------------|
| 相位角 (0.1°) | φ | `phaseValue` (ADC) | 0x1020 |
| 驱动频率 (Hz) | f_drive | 384MHz / prioed | 0x1021-22 |
| PPG 周期 | T_ppg | prioed (HRTIM) | 0x1024 |
| PPG 占空比 | D | duty (HRTIM) | 0x1025 |
| 谐振频率 (Hz) | f_res | **待估计** | — |

### 1.2 EKF 状态模型

```
状态向量:  x = [f_res]              (谐振频率, 1维)
          或 x = [f_res, df/dt]     (谐振频率+漂移率, 2维可选)

观测:      z = φ (相位角, 从 phaseValue ADC 读出)

控制输入:  u = f_drive (驱动频率 = 384MHz / prioed)

状态方程:  f_res_{k+1} = f_res_k + w_k     (随机游走)
观测方程:  φ_k = K_p · (f_drive_k - f_res_k) + v_k
           其中 K_p 是相位-频率斜率 (需要从数据标定)
```

### 1.3 简化 EKF (1维)

桌面端先用 1 维模型，参数少、收敛快：

```
预测:
  f_res⁻ = f_res              (谐振频率不变)
  P⁻ = P + Q                  (协方差+过程噪声)

更新:
  y = φ_meas - h(f_res⁻)      (相位残差)
  h(f_res⁻) = K_p · (f_drive - f_res⁻)
  H = ∂h/∂f = -K_p            (观测雅可比)

  K = P⁻·H / (H²·P⁻ + R)     (卡尔曼增益)

  f_res = f_res⁻ + K·y        (更新谐振频率估计)
  P = (1 - K·H)·P⁻            (更新协方差)
```

---

## 2. 桌面端原型 (Python)

### 2.1 阶段 A: 离线数据采集

目标: 采集足够的数据来标定 `K_p`, `Q`, `R`。

**采集程序** (`tools/ekf_tuner/ekf_data_collector.py`):
```python
# 对单个炉头, 连续采集以下字段到 CSV:
#   timestamp, f_drive(Hz), phase_deg, ppg_period, ppg_duty, power_w
# 采集频率: 50ms (20Hz)
# 时长: 每个测试场景 60-120s
```

### 2.2 阶段 B: 参数标定

从采集数据中离线标定:

| 参数 | 含义 | 标定方法 |
|------|------|----------|
| `K_p` | 相位-频率斜率 (deg/Hz) | 线性回归 φ ~ f_drive, 取斜率 |
| `Q` | 过程噪声 (Hz²) | 稳态频率波动方差 × Δt |
| `R` | 观测噪声 (deg²) | 稳态相位残差方差 |

### 2.3 阶段 C: EKF 离线验证

在 Python 中实现 EKF, 用采集数据回放验证:
- 读取 CSV → 逐帧 feed EKF → 对比估计值 vs 实际
- 调优 Q/R 直至收敛速度快且不过冲

### 2.4 阶段 D: 实时 EKF 运行

Python 端实时运行 EKF, 通过 MODBUS 读取数据并实时输出估计结果:
- 50ms 采样 → 20Hz EKF 更新
- 终端打印 f_res 估计值
- 桌面端验证通过后, 记录最终参数 (K_p, Q, R, P0)

---

## 3. 测试场景清单

> **你需要准备以下测试场景。每个场景预热 30s 后开始采集。**

### 场景 1: 空载稳态 (基准)
| 项目 | 值 |
|------|-----|
| 目的 | 获取空载谐振频率基准, 标定 R (观测噪声) |
| 锅具 | 无锅 (空载) |
| 功率 | 0W (仅起振, 不加热) |
| 时长 | 60s |
| 采集 | 20Hz, 4头 |

### 场景 2: 固定功率稳态 (铁锅)
| 项目 | 值 |
|------|-----|
| 目的 | 铁锅各档位稳态数据, 标定 K_p, Q |
| 锅具 | 标准铁锅 (Φ20cm) |
| 功率 | **500W / 1000W / 1500W / 2000W** 各 60s |
| 时长 | 4×60s = 240s |
| 采集 | 20Hz, 单头 |

### 场景 3: 功率爬坡
| 项目 | 值 |
|------|-----|
| 目的 | 验证 EKF 对快速频率变化的跟踪能力 |
| 锅具 | 标准铁锅 (Φ20cm) |
| 功率 | 500W → 2000W, 每 15s 升 500W (4 步) |
| 时长 | 120s (含升/降各一轮) |
| 采集 | 20Hz, 单头 |

### 场景 4: 锅具切换
| 项目 | 值 |
|------|-----|
| 目的 | 验证 EKF 对不同锅具谐振特性的适应 |
| 锅具 | 铁锅 → 钢锅 → 铁锅, 每 60s 切换 |
| 功率 | 1000W 固定 |
| 时长 | 180s |
| 采集 | 20Hz, 单头 |

### 场景 5: 多炉头并发
| 项目 | 值 |
|------|-----|
| 目的 | 验证 4 头同时工作时 EKF 的独立性 |
| 锅具 | 4 个铁锅 (各炉头一个) |
| 功率 | 各 1000W |
| 时长 | 120s |
| 采集 | 20Hz, 4头 (轮询, 每个头 50ms → 全四头 200ms/轮) |

### 场景 6: 空载→负载突变
| 项目 | 值 |
|------|-----|
| 目的 | 验证 EKF 对负载突变(放锅/移锅)的响应速度 |
| 锅具 | 无锅 → 放铁锅 → 移锅 → 放钢锅 |
| 功率 | 500W |
| 时长 | 120s |
| 采集 | 20Hz, 单头 |

---

## 4. C端移植

### 4.1 文件规划

```
app/ekf/
  ├── modbus_ekf_regs.h       (已有 — MODBUS 寄存器接口)
  ├── modbus_ekf_regs.c       (已有 — 数据刷新)
  ├── ekf_resonant_track.h    (新增 — EKF 核心算法)
  └── ekf_resonant_track.c    (新增 — EKF 实现, 从 app_power 调用)
```

### 4.2 C 结构体

```c
/* ekf_resonant_track.h */
typedef struct {
    float   f_res;       /* 估计的谐振频率 (Hz) */
    float   P;           /* 估计协方差 */
    float   K_p;         /* 相位-频率斜率 (deg/Hz) */
    float   Q;           /* 过程噪声 */
    float   R;           /* 观测噪声 */
    uint8_t initialized; /* 是否已收敛 */
} EKF_ResonantTracker_t;
```

### 4.3 C API

```c
/* 初始化, 设置标定好的 K_p, Q, R */
void EKF_Tracker_Init(EKF_ResonantTracker_t *ekf,
                      float K_p, float Q, float R,
                      float init_freq);

/* 每步迭代: 输入相位测量值和当前驱动频率, 输出估计谐振频率 */
float EKF_Tracker_Update(EKF_ResonantTracker_t *ekf,
                         float phase_deg,     /* 相位测量 (度) */
                         float f_drive_hz);   /* 当前驱动频率 (Hz) */
```

### 4.4 定点数考虑

Cortex-M0+ 无 FPU, `float` 运算很慢 (~100+ cycles per op)。如果 50ms 更新一次, 浮点开销可接受 (几个 float 运算 << 50ms)。如果后续要加速, 可切换为 `Q16.16` 定点:

```
f_res:  Q16.16  (uint32_t, 范围 0-65536 Hz, 精度 0.000015 Hz)
P:      Q16.16
K:      Q16.16
```

桌面端先用 float 验证, C 端移植时根据性能决定是否定点化。

### 4.5 调用位置

在 `API_POWER_EKF_GetTelemetry()` (app_power.c) 中调用 EKF:

```c
void API_POWER_EKF_GetTelemetry(uint8_t chn, EKF_Telemetry_t *ekf)
{
    PPGvalueDef ppg;
    float phase_deg, f_drive, f_res;

    /* ... 读取 phaseValue, prioed ... (已有代码) */

    /* ---- EKF 更新 ---- */
    phase_deg = (float)(int16_t)ekf->Phase_Angle / 10.0f;
    f_drive   = (float)freq_hz;
    f_res     = EKF_Tracker_Update(&g_EKF[chn], phase_deg, f_drive);

    /* 输出谐振频率到保留寄存器 */
    ekf->Res3 = (uint16_t)f_res;  /* 0x1029: 谐振频率整数部分 */
}
```

---

## 5. 验证闭环

```
桌面端 (Python)                      C端 (固件)
─────────────                        ──────────
1. 数据采集 (CSV)
      ↓
2. 参数标定 (K_p, Q, R)
      ↓
3. EKF 回放验证 ──── 确认算法正确 ──→ 4. C 代码移植
                                          ↓
5. 固件烧录, 实时对比 ←── MODBUS 回读 ← 6. 固件运行 EKF
      ↓
7. 残差分析, 参数微调
      ↓
8. 最终参数烧入固件 → 闭环完成
```

每一步的验收标准:
- **步骤 1-2**: CSV 数据覆盖 6 个场景, K_p/R 标定值稳定 (标准差 < 10%)
- **步骤 3**: EKF 估计值与实际频率误差 < 50Hz (稳态), 收敛 < 5s
- **步骤 4**: C 代码编译通过 0 error 0 warning
- **步骤 5-6**: C 端输出与 Python 端输出偏差 < 1Hz (双精度对比)
- **步骤 7**: 所有 6 场景残差分析通过
- **步骤 8**: 参数固化, 代码 merge

---

*创建: 2026-05-26, 待技术负责人审核*
