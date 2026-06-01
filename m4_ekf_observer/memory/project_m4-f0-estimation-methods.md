---
name: m4-f0-estimation-methods
description: 半桥谐振锅具识别 — dI/df斜率法 + 相位模型 + 联合修正，f0估计与铁/钢判别
type: project
---

# 谐振参数估计与锅具识别方法总结

> 最后更新: 2026-05-30

## 1. 物理模型

半桥 RLC 串联谐振回路，工作在高频侧 (f > f0):

```
I(f) = I_max / sqrt(1 + Q² × (f/f0 - f/f0)²)
tan(φ) = Q × (f/f0 - f/f0)          ← 相位只与 f/f0 和 Q 有关
```

- **远离谐振点** (f ≫ f0): Δf 大 → ΔI 小 → |dI/df| 小 → 铁锅特征
- **靠近谐振点** (f ≈ f0): Δf 小 → ΔI 大 → |dI/df| 大 → 钢锅特征

## 2. 三种 f0 估计方法

### 2.1 相位模型 (PRIMARY — `_fit_phase_model_all`)

用全部有效采样点拟合 tan(φ) = Q × (f/f0 - f/f0)。

**算法**: 网格搜索 f0 ∈ [12kHz, min(f_data) - 50Hz]，对每个 f0 计算各点的 Q_i = tan(φ_i) / (f_i/f0 - f0/f_i)，最小化 Q 的变异系数 (CV = σ/μ)。

**优点**: 用全部数据点（不限于稳态），Q CV 越小说明模型越一致。

**实现**: [ekf_q_model.py:385-456](tools/ekf_tuner/ekf_q_model.py#L385-L456)

```
返回: (f0, Q_mean, Q_cv, phase_RMS_deg, n_points)
f0 = argmin CV(Q_i)   across f0 grid
```

### 2.2 dI/df 斜率法 (REFERENCE — `_estimate_f0`)

从稳态工作点之间的差分计算 |dI/df|。

**核心关系**: |dI/df| ∝ 1/(f - f0)，即 1/|dI/df| ∝ (f - f0)

**算法**: 加权线性回归 y = a·f + b，其中 y = 1/|dI/df|，权重 ∝ |dI/df|（近谐振点更可信）。f0 = -b/a (x截距)。

**稳态点提取**: 每段恒定功率目标取最后 40% 数据的中位数 (f, I, φ) → 相邻功率段差分得 dI/df。

**限制**: 仅用稳态点，铁锅工作点远离 f0 导致外推距离大、误差放大。p=1 模型对远场铁锅 f0 估计偏高是预期行为。

**实现**: [ekf_q_model.py:185-232](tools/ekf_tuner/ekf_q_model.py#L185-L232)

### 2.3 相位修正联合法 (`_estimate_f0_joint`)

融合相位信息修正高频段的 dI/df 测量噪声。

**核心思想**:
- tan(φ) ∝ (f-f0)，1/|dI/df| ∝ (f-f0) → 两者应成正比
- 高频 (|dI/df| 小): dI/df 噪声大 → 信相位推算 (α ≈ 0)
- 低频 (|dI/df| 大): dI/df 信号强 → 信实测 (α ≈ 1)

**融合公式**:
```
r = median( (1/|dI/df|) / tan(φ) )
y_phase = r × tan(φ)                    ← 相位推算的 y 值
y_blend = α·y_measured + (1-α)·y_phase  ← 逐点融合
α = |dI/df| / max(|dI/df|)              ← 自适应权重
```

然后用 y_blend 做加权回归 → f0。

**实现**: [ekf_q_model.py:235-330](tools/ekf_tuner/ekf_q_model.py#L235-L330)

### 2.4 tan(φ) 比值消 Q 法 (`_f0_from_phase_tan_ratio`)

两频点相除消去 Q，直接解 f0:

```
tan(φ1)/tan(φ2) = (f1/f0 - f0/f1) / (f2/f0 - f0/f2) = R
→ f0² = (f2·f1² - R·f1·f2²) / (f2 - R·f1)
```

所有频点对组合求 f0，取中位数。要求相位 ≥ 5°、频差 ≥ 500Hz。

**实现**: [ekf_q_model.py:333-382](tools/ekf_tuner/ekf_q_model.py#L333-L382)

## 3. 判别逻辑

```
f0 < 19kHz  → IRON (铁锅/不锈钢430)
f0 ≥ 20kHz  → STEEL (钢锅/不锈钢304)
19-20kHz    → 灰色区域: f_min_high_pwr < 25kHz → iron, else steel
```

**实测验证** (user 2026-05-30):
- 铁锅 f0 ≈ 20-21 kHz ✅
- 钢锅 f0 ≈ 23-24 kHz ✅

两种锅型都在 19kHz 以上，说明功率级始终工作在 f > f0 区域（感性区）。铁锅的 f0 更接近设计最低频，钢锅的 f0 更高。

## 4. 分类特征对比

| 特征 | 铁锅 (Iron) | 钢锅 (Steel) |
|------|------------|-------------|
| f0 范围 | 20-21 kHz | 23-24 kHz |
| 工作频率相对 f0 | f ≫ f0，远离谐振 | f > f0，接近谐振 |
| \|dI/df\| | 小（频率变化对电流影响弱） | 大（频率变化对电流影响强） |
| 相位角 | 小（接近阻性） | 较大 |
| slope_ratio (低/高频) | 小（斜率均匀） | 大（近谐振点斜率剧增） |
| 扫频段 | 频率下降慢，I 上升平缓 | 频率下降快，I 快速攀升 |

## 5. 数据采集要求

- 需要功率变化过程（扫频段），至少 2 个不同功率水平的稳态点
- 相位数据对铁锅判别尤其重要（dI/df 信号弱时需相位修正）
- 最小有效数据: 50 点以上，f < 55kHz，I ≥ 5 ADC，P > 0
- 排除 39.9-40.1kHz 空闲噪声段

## 6. 局限与改进方向

1. **铁锅 f0 偏低估计**: p=1 模型在远场外推时 f0 偏小，p 值可能不是严格的 1
2. **相位 < 5° 不可用**: 低功率时 tan(φ) 信噪比差，需剔除
3. **未来方向**: 在 MCU 上实现每周期 CalcResonantParams → f0 实时跟踪 → 替代离线分类
4. **死区补偿**: 低功率时死区时间长，ZC 检测偏移，需插值修正（MATLAB 先验证）

## 7. 关键文件

| 文件 | 说明 |
|------|------|
| [ekf_q_model.py](tools/ekf_tuner/ekf_q_model.py) | f0 估计三种方法 + 锅具判别 |
| [m4_modbus_tool.py](tools/ekf_tuner/m4_modbus_tool.py) | MODBUS 通信，EKF 遥测 0x1020 |
| [m4_gui.py](tools/ekf_tuner/m4_gui.py) | GUI 调试界面，CSV 录制 |
| [calc_physical_params.m](tools/ekf_tuner/calc_physical_params.m) | MATLAB 物理量换算 |
| [resonant_f0.c](src/RX32G410_FW_HAL_V1.3N/Projects/BaseClass/src/resonant_f0.c) | MCU 端 f0/Q/L 实时计算 |
