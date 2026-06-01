#!/usr/bin/env python3
"""
锅具识别器 — dI/df 斜率判别法
==============================

核心逻辑:
  远离谐振点: 大的 Δf 产生小的 ΔI → |dI/df| 小
  靠近谐振点: 小的 Δf 产生大的 ΔI → |dI/df| 大

  共振曲线 I(f) = Imax / sqrt(1 + Q^2 * (f/f0 - f0/f)^2)
  在高频侧 (f > f0): |dI/df| ∝ 1/(f - f0)^p (近似)
  → 1/|dI/df|^(1/p) ∝ f - f0 → 线性外推 x截距 = f0

  铁锅: f0 ≈ 18-20kHz (设计最低频), 工作频率远高于 f0
  钢锅: f0 ≈ 22-27kHz, 工作频率接近或穿越 f0

用法:
  python ekf_q_model.py --csv data.csv
  python ekf_q_model.py --self-test
"""

import numpy as np
from dataclasses import dataclass, field
from collections import defaultdict
import csv, sys, math, json


@dataclass
class PotAnalysis:
    pot_type:    str          # "iron" | "steel" | "unknown"
    f0_est:      float        # phase-model f0 [Hz] (all points, primary)
    q_factor:    float        # Q factor from phase model
    q_cv:        float        # Q coefficient of variation (consistency)
    phase_rms:   float        # RMS phase prediction error [deg]
    f0_didf:     float        # dI/df f0 [Hz] (p=1, stable points, reference)
    r_squared:   float        # dI/df R^2
    slope_ratio: float        # |dI/df|_low / |dI/df|_high
    n_valid:     int
    per_power:   list = field(default_factory=list)
    sweep_slopes: list = field(default_factory=list)


# ======================================================================
# 数据加载
# ======================================================================

def _load_csv(csv_path: str):
    freqs, phases, currents, powers, targets = [], [], [], [], []
    with open(csv_path, 'r') as f:
        for row in csv.DictReader(f):
            try:
                freqs.append(float(row.get('freq_hz', 0)))
                phases.append(float(row.get('phase_deg', 0)))
                currents.append(float(row.get('res_cur_adc', 0)))
                powers.append(float(row.get('power_actual', 0)))
                targets.append(float(row.get('power_target', 0)))
            except (ValueError, TypeError):
                continue
    return (np.array(freqs), np.array(phases),
            np.array(currents), np.array(powers), np.array(targets))


def _filter_valid(freqs, currents, powers, min_curr=5, max_freq=55000):
    """排除 idle / 噪声 / 无效数据"""
    v = np.ones(len(freqs), dtype=bool)
    v &= (freqs > 0) & (freqs < max_freq)
    v &= (currents >= min_curr)
    v &= (powers > 0)
    v &= (freqs < 39900) | (freqs > 40100)   # 40kHz idle 噪声
    return v


# ======================================================================
# 稳态工作点提取
# ======================================================================

def _extract_stable_points(freqs, currents, phases, targets):
    """在每个功率段取稳态工作点 (最后 40% 的数据, 避开功率爬升)"""
    stable_pts = []   # [(f_median, i_median, phase_median, power_target), ...]

    # find ranges where power_target is constant
    prev_t = targets[0]
    seg_start = 0
    for i in range(1, len(targets)):
        if targets[i] != prev_t:
            # segment ended
            seg_len = i - seg_start
            if seg_len >= 20 and prev_t >= 500:
                # use last 40% of the segment (steady state)
                n_steady = max(int(seg_len * 0.4), 10)
                seg = slice(i - n_steady, i)
                f_med = float(np.median(freqs[seg]))
                i_med = float(np.median(currents[seg]))
                ph_med = float(np.median(phases[seg]))
                stable_pts.append((f_med, i_med, ph_med, prev_t))
            seg_start = i
            prev_t = targets[i]

    # last segment
    seg_len = len(targets) - seg_start
    if seg_len >= 20 and prev_t >= 500:
        n_steady = max(int(seg_len * 0.4), 10)
        seg = slice(len(targets) - n_steady, len(targets))
        f_med = float(np.median(freqs[seg]))
        i_med = float(np.median(currents[seg]))
        ph_med = float(np.median(phases[seg]))
        stable_pts.append((f_med, i_med, ph_med, prev_t))

    return stable_pts


# ======================================================================
# 扫频段提取 (功率突变后的频率下降过程)
# ======================================================================

def _extract_sweeps(freqs, currents, powers, targets):
    """检测功率目标变化, 提取随后的频率扫掠段"""
    sweeps = []  # [(f_arr, i_arr, power_target), ...]

    i = 1
    while i < len(targets):
        if targets[i] > targets[i-1] and targets[i] >= 1000:
            # power increase detected, find the sweep
            sweep_f, sweep_i = [], []
            j = i
            prev_f = freqs[j] if j < len(freqs) else 0
            while j < len(freqs) and j < i + 60:  # max ~6 seconds
                f = freqs[j]
                # during sweep: frequency is descending significantly
                if len(sweep_f) > 0 and f > sweep_f[-1] + 200:
                    break  # frequency reversed upward, sweep done
                if f > 0 and currents[j] > 0:
                    sweep_f.append(f)
                    sweep_i.append(currents[j])
                j += 1

            if len(sweep_f) >= 8:
                sweeps.append((np.array(sweep_f), np.array(sweep_i), targets[i]))
            i = j
        else:
            i += 1

    return sweeps


# ======================================================================
# 核心: dI/df 斜率计算与 f0 外推
# ======================================================================

def _compute_slopes_from_stable(stable_pts):
    """从稳态点之间的差分计算 dI/df"""
    slopes = []  # [(f_mid, abs(dI/df), power_level), ...]
    for i in range(len(stable_pts) - 1):
        f1, i1, ph1, p1 = stable_pts[i]
        f2, i2, ph2, p2 = stable_pts[i+1]

        df = f1 - f2  # frequency drops as power increases
        if abs(df) < 300:
            continue

        di_df = abs(i2 - i1) / abs(df)  # ADC per Hz
        f_mid = (f1 + f2) / 2
        slopes.append((f_mid, di_df, max(p1, p2)))

    return slopes


def _compute_slopes_from_sweeps(sweeps):
    """从扫频段内相邻点计算 dI/df (更密集的采样)"""
    slopes = []
    for f_arr, i_arr, pw_target in sweeps:
        # use only descending part
        for k in range(len(f_arr) - 1):
            df = f_arr[k] - f_arr[k+1]
            if 80 < df < 3000:  # meaningful frequency step
                di = i_arr[k+1] - i_arr[k]
                if di > 0:  # current should rise as freq drops
                    f_mid = (f_arr[k] + f_arr[k+1]) / 2
                    di_df = di / df
                    slopes.append((f_mid, di_df, pw_target))

    return slopes


def _estimate_f0(slope_pairs):
    """
    拟合 |dI/df| = A / (f - f0)  →  f0  (p=1, 仅用于对比)

    线性化: 1/|dI/df| = a*f + b, f0 = -b/a
    加权回归, 权重 ∝ |dI/df| (近谐振点更可信)

    slope_pairs: [(f_mid, |dI/df|), ...]
    返回: (f0, r2)
    """
    if len(slope_pairs) < 2:
        return 0.0, 0.0

    f_arr = np.array([s[0] for s in slope_pairs])
    di_arr = np.array([s[1] for s in slope_pairs])

    valid = di_arr > 1e-8
    if valid.sum() < 2:
        return 0.0, 0.0

    f_arr = f_arr[valid]
    di_arr = di_arr[valid]
    w = di_arr / np.max(di_arr)

    # p=1: y = 1/|dI/df|
    y = 1.0 / di_arr

    sw = np.sum(w)
    swx = np.sum(w * f_arr)
    swy = np.sum(w * y)
    swxx = np.sum(w * f_arr * f_arr)
    swxy = np.sum(w * f_arr * y)

    denom = sw * swxx - swx * swx
    if abs(denom) < 1e-12:
        return 0.0, 0.0

    a = (sw * swxy - swx * swy) / denom
    b = (swy - a * swx) / sw

    f0 = -b / a if abs(a) > 1e-12 else 0.0

    y_pred = a * f_arr + b
    ss_res = float(np.sum(w * (y - y_pred) ** 2))
    ss_tot = float(np.sum(w * (y - np.mean(y)) ** 2))
    r2 = 1 - ss_res / ss_tot if ss_tot > 0 else 0.0

    return f0, r2


def _estimate_f0_joint(stable_pts, slopes_stable):
    """
    相位修正的 dI/df 回归: 逐点融合相位信息

    核心: 1/|dI/df| ∝ (f-f0) 且 tan(φ) ∝ (f-f0)
    → 1/|dI/df| 与 tan(φ) 应成正比, 比值 r = (1/|dI/df|) / tan(φ) ≈ const

    对每个数据点, 用相位推算"应有的" y 值, 与实测 y 值融合:
      y_blend = α * y_measured + (1-α) * y_phase
      α = |dI/df| / max(|dI/df|)  ← 低频 dI/df 大 → α≈1 (信实测)
                                   高频 dI/df 小 → α≈0 (信相位)

    这就是"高频修得多, 低频修得少"的物理实现.

    stable_pts: [(f, i, phase, power), ...]
    slopes_stable: [(f_mid, |dI/df|, power), ...]
    返回: (f0_joint, f0_didf, f0_phase, blend_r2)
    """
    if len(stable_pts) < 2 or len(slopes_stable) < 2:
        return 0.0, 0.0, 0.0, 0.0

    # 每个斜率对: 中点频率 + 插值相位
    slope_data = []
    for f_mid, didf, pw in slopes_stable:
        phase_mid = 0.0
        for k in range(len(stable_pts) - 1):
            f1, _, ph1, p1 = stable_pts[k]
            f2, _, ph2, p2 = stable_pts[k + 1]
            if min(p1, p2) <= pw <= max(p1, p2):
                if abs(f1 - f2) > 1:
                    t = (f_mid - f1) / (f2 - f1)
                    phase_mid = ph1 + t * (ph2 - ph1)
                else:
                    phase_mid = (ph1 + ph2) / 2
                break
        if phase_mid >= 5:  # 相位太小, tan 不可靠
            slope_data.append((f_mid, didf, phase_mid))

    if len(slope_data) < 2:
        return 0.0, 0.0, 0.0, 0.0

    # 1. 原始 p=1 回归 (仅用于对比)
    f0_didf, r2_didf = _estimate_f0([(f, s) for f, s, _ in slope_data])

    # 2. 相位 f0 (仅用于对比)
    f0_phase = _f0_from_phase_tan_ratio(stable_pts)

    # 3. 逐点相位修正回归
    f_arr = np.array([s[0] for s in slope_data])
    di_arr = np.array([s[1] for s in slope_data])
    ph_arr = np.array([s[2] for s in slope_data])

    # 原始 y 值
    y_measured = 1.0 / di_arr  # = (f - f0) / A

    # 相位推算的 y 值: tan(φ) ∝ (f-f0), 所以 y ∝ tan(φ)
    tan_phi = np.tan(np.radians(ph_arr))

    # 找 r = y/tan(φ) 的中位数 (排除异常值)
    ratios = y_measured / tan_phi
    r_med = float(np.median(ratios))

    # y_phase = r_med * tan(φ) → 相位推算的 y 值
    y_phase = r_med * tan_phi

    # 融合权重: α = |dI/df| / max(|dI/df|)
    # 高频 (|dI/df| 小) → α ≈ 0, 信相位
    # 低频 (|dI/df| 大) → α ≈ 1, 信实测
    alpha = di_arr / np.max(di_arr)

    y_blend = alpha * y_measured + (1.0 - alpha) * y_phase

    # 加权回归 (权重 ∝ |dI/df|, 同原始方法)
    w = di_arr / np.max(di_arr)

    sw = np.sum(w)
    swx = np.sum(w * f_arr)
    swy = np.sum(w * y_blend)
    swxx = np.sum(w * f_arr * f_arr)
    swxy = np.sum(w * f_arr * y_blend)

    denom = sw * swxx - swx * swx
    if abs(denom) < 1e-12:
        return f0_didf, f0_didf, f0_phase, r2_didf

    a = (sw * swxy - swx * swy) / denom
    b = (swy - a * swx) / sw
    f0_joint = -b / a if abs(a) > 1e-12 else f0_didf

    # 拟合优度 (对融合后的 y)
    y_pred = a * f_arr + b
    ss_res = float(np.sum(w * (y_blend - y_pred) ** 2))
    ss_tot = float(np.sum(w * (y_blend - np.mean(y_blend)) ** 2))
    r2_blend = 1 - ss_res / ss_tot if ss_tot > 0 else 0.0

    return f0_joint, f0_didf, f0_phase, r2_blend


def _f0_from_phase_tan_ratio(stable_pts):
    """
    从相位-频率数据估计 f0, 使用 tan(φ) 比值法消 Q

    RLC 模型: tan(φ) = Q * (f/f0 - f0/f)
    两频点比值消去 Q → 直接解 f0

    stable_pts: [(f, i, phase, power), ...]
    返回: f0_phase (0 if insufficient data)
    """
    if len(stable_pts) < 2:
        return 0.0

    f0_estimates = []

    for i in range(len(stable_pts)):
        for j in range(i + 1, len(stable_pts)):
            f1, _, ph1, _ = stable_pts[i]
            f2, _, ph2, _ = stable_pts[j]

            if ph1 < 5 or ph2 < 5:
                continue
            if abs(f1 - f2) < 500:
                continue

            tan1 = np.tan(np.radians(ph1))
            tan2 = np.tan(np.radians(ph2))

            if tan1 < 1e-6 or tan2 < 1e-6:
                continue

            R = tan1 / tan2

            # f0^2 = (f2*f1^2 - R*f1*f2^2) / (f2 - R*f1)
            num = f2 * f1 * f1 - R * f1 * f2 * f2
            den = f2 - R * f1

            if abs(den) < 1e-9:
                continue

            f0_sq = num / den
            if f0_sq > 0:
                f0 = np.sqrt(f0_sq)
                if 10000 < f0 < min(f1, f2):
                    f0_estimates.append(f0)

    if len(f0_estimates) < 1:
        return 0.0

    return float(np.median(f0_estimates))


def _fit_phase_model_all(freqs, phases):
    """
    用全部有效采样点拟合 phase 模型: tan(φ) = Q * (f/f0 - f0/f)

    网格搜索 f0, 最小化各点 Q_i 的变异系数 (CV).
    每一对 (f, φ) 都是瞬时阻抗测量, 不受是否稳态影响.

    freqs, phases: 全部有效数据点 (已过滤)
    返回: (f0, Q, q_cv, phase_rms, n_used)
    """
    if len(freqs) < 20:
        return 0.0, 0.0, 999.0, 999.0, 0

    # 过滤极端值
    valid = (phases > 5) & (phases < 85) & (freqs > 21000) & (freqs < 40000)
    f_v = freqs[valid]
    ph_v = phases[valid]

    if len(f_v) < 20:
        return 0.0, 0.0, 999.0, 999.0, 0

    # 子采样提速 (目标 ~500 点)
    step = max(1, len(f_v) // 500)
    idx = np.arange(0, len(f_v), step)
    f_sub = f_v[idx]
    ph_sub = ph_v[idx]

    f_min_data = f_sub.min()
    best_f0 = 0.0
    best_Q = 0.0
    best_cv = 999.0

    for f0_test in np.linspace(12000, f_min_data - 50, 500):
        Q_vals = []
        for f, ph in zip(f_sub, ph_sub):
            g = f / f0_test - f0_test / f
            if g < 1e-6:
                continue
            Q_vals.append(np.tan(np.radians(ph)) / g)

        if len(Q_vals) < 10:
            continue

        Q_arr = np.array(Q_vals)
        cv = float(np.std(Q_arr) / np.mean(Q_arr))
        if cv < best_cv:
            best_cv = cv
            best_f0 = f0_test
            best_Q = float(np.mean(Q_arr))

    if best_f0 <= 0:
        return 0.0, 0.0, 999.0, 999.0, 0

    # 用最优 f0 计算全部点的 Q 和 RMS
    Q_all = []
    ph_pred = []
    ph_act = []
    for f, ph in zip(f_v, ph_v):
        g = f / best_f0 - best_f0 / f
        if g > 1e-6:
            Q_all.append(np.tan(np.radians(ph)) / g)
            ph_pred.append(np.degrees(np.arctan(best_Q * g)))
            ph_act.append(ph)

    Q_all = np.array(Q_all)
    ph_pred = np.array(ph_pred)
    ph_act = np.array(ph_act)
    rms = float(np.sqrt(np.mean((ph_pred - ph_act) ** 2)))
    q_cv = float(np.std(Q_all) / np.mean(Q_all))
    q_mean = float(np.mean(Q_all))

    return best_f0, q_mean, q_cv, rms, len(Q_all)


# ======================================================================
# 判别逻辑
# ======================================================================

def _classify(f0_est, f_min_high_pwr, slope_ratio):
    """
    基于外推 f0 + 高功率工作频率 判别锅具类型

    判据 1: f0_est < 19kHz → 铁锅 (谐振点远在设计最低频以下)
    判据 2: f0_est >= 20kHz → 钢锅 (谐振点在可观测范围)
    判据 3: 19-20kHz 灰色区域 → 参考工作频率辅助判别
    """
    if f0_est <= 0:
        return "unknown"

    if f0_est < 19000:
        return "iron"
    elif f0_est >= 20000:
        return "steel"
    else:
        # 灰色区域 (19-20kHz), 用工作频率辅助
        if f_min_high_pwr > 0 and f_min_high_pwr < 25000:
            return "iron"
        else:
            return "steel"


# ======================================================================
# 统一分析入口
# ======================================================================

def analyze_csv(csv_path: str) -> PotAnalysis:
    freqs, phases, currents, powers, targets = _load_csv(csv_path)
    if len(freqs) < 20:
        return PotAnalysis("unknown", 0, 0, 0, 0, 0, 0, 0, 0, [], [])

    valid = _filter_valid(freqs, currents, powers)
    f_all = freqs[valid]
    ph_all = phases[valid]
    i_all = currents[valid]
    pw_all = powers[valid]
    tgt_all = targets[valid]

    if len(f_all) < 20:
        return PotAnalysis("unknown", 0, 0, 0, 0, 0, 0, 0, valid.sum(), [], [])

    # 1. Phase model: all-points grid search (PRIMARY)
    f0_phase, q_factor, q_cv, phase_rms, n_phase = _fit_phase_model_all(f_all, ph_all)

    # 2. dI/df reference — p=1 on stable points only
    stable_pts = _extract_stable_points(f_all, i_all, ph_all, tgt_all)
    slopes_stable = _compute_slopes_from_stable(stable_pts)

    if len(slopes_stable) >= 2:
        f0_didf, r2 = _estimate_f0([(f, s) for f, s, _ in slopes_stable])
    else:
        f0_didf, r2 = 0.0, 0.0

    # 3. Slope ratio (adaptive freq band split)
    if len(slopes_stable) >= 2:
        all_fm = [f for f, _, _ in slopes_stable]
        f_max = max(all_fm)
        f_min = min(all_fm)
        f_mid_adaptive = (f_max + f_min) / 2
        high_band = [(f, s) for f, s, _ in slopes_stable if f >= f_mid_adaptive]
        low_band  = [(f, s) for f, s, _ in slopes_stable if f < f_mid_adaptive]
        slope_high = np.mean([s for _, s in high_band]) if high_band else 0
        slope_low  = np.mean([s for _, s in low_band]) if low_band else 0
        slope_ratio = slope_low / slope_high if slope_high > 1e-9 else 0
    else:
        slope_ratio = 0

    # 4. Sweep segments
    sweeps = _extract_sweeps(f_all, i_all, pw_all, tgt_all)
    slopes_sweep = _compute_slopes_from_sweeps(sweeps)

    # 5. Classification (phase model f0 as primary)
    f0_est = f0_phase if f0_phase > 0 else f0_didf
    hi_pwr_mask = pw_all >= 1500
    f_min_high = float(np.min(f_all[hi_pwr_mask])) if hi_pwr_mask.sum() > 5 else 0
    pot_type = _classify(f0_est, f_min_high, slope_ratio)

    # 6. Per-power summary
    per_power = []
    for f_med, i_med, ph_med, pw in stable_pts:
        local_slope = 0.0
        for fm, s, p in slopes_stable:
            if abs(p - pw) < 200:
                local_slope = s
                break
        per_power.append((pw, f_med, i_med, ph_med, local_slope))

    return PotAnalysis(pot_type, f0_est, q_factor, q_cv, phase_rms,
                       f0_didf, r2, slope_ratio, valid.sum(),
                       per_power, slopes_sweep)


# ======================================================================
# 报告
# ======================================================================

def print_report(r: PotAnalysis, label: str = ""):
    tag = f" [{label}]" if label else ""
    print(f"\n{'='*60}")
    print(f"  Pot Analysis — Phase Model + dI/df Reference{tag}")
    print(f"{'='*60}")

    # --- Primary: Phase model (all points) ---
    print(f"  Type:        {r.pot_type.upper()}")
    if r.f0_est > 0:
        tag_f0 = "< 19k = iron" if r.f0_est < 19000 else ">= 19k = steel"
        print(f"  f0 (phase):  {r.f0_est:.0f} Hz  ({tag_f0})  <-- PRIMARY")
        print(f"  Q factor:    {r.q_factor:.1f}")
        print(f"  Q CV:        {r.q_cv:.3f}  (lower = more consistent)")
        print(f"  Phase RMS:   {r.phase_rms:.1f} deg  (prediction error)")
    else:
        print(f"  f0 (phase):  N/A (insufficient data)")

    # --- Reference: dI/df p=1 on stable points ---
    print(f"  ---")
    if r.f0_didf > 0:
        tag_didf = "< 19k = iron" if r.f0_didf < 19000 else ">= 19k = steel"
        diff = abs(r.f0_est - r.f0_didf) if r.f0_est > 0 else 0
        print(f"  f0 (dI/df):  {r.f0_didf:.0f} Hz  ({tag_didf})  "
              f"R^2={r.r_squared:.4f}  diff={diff:.0f}Hz")
    else:
        print(f"  f0 (dI/df):  N/A (insufficient stable points)")

    print(f"  Slope ratio: {r.slope_ratio:.1f}x  "
          f"(low-f |dI/df| / high-f |dI/df|)")
    print(f"  N valid:     {r.n_valid}")

    # Stable points table
    if r.per_power:
        print(f"\n  Stable Operating Points:")
        print(f"  {'Pow':>5s}  {'f_med':>7s}  {'I_med':>7s}  "
              f"{'Phase':>7s}  {'dI/df':>11s}")
        print(f"  {'-'*48}")
        for pw, fm, im, ph, sl in r.per_power:
            sl_str = f"{sl*1e3:.1f} ADC/kHz" if sl > 0 else "—"
            print(f"  {pw:>5.0f}  {fm:>7.0f}  {im:>7.0f}  "
                  f"{ph:>6.1f}deg  {sl_str:>11s}")

    # Result summary
    print(f"\n  >>> Result: ", end="")
    if r.pot_type == "iron":
        print("IRON POT — f0 below 19kHz, far from resonance in operating band")
        print(f"      Resonance is deep below design minimum.")
        print(f"      Recommend lower-frequency sweep (<22kHz) for accurate f0.")
    elif r.pot_type == "steel":
        print("STEEL POT — f0 in/near operating range")
        if r.q_cv < 0.15:
            print(f"      Phase model consistent (Q CV={r.q_cv:.3f}), "
                  f"high confidence.")
        else:
            print(f"      Moderate phase consistency (Q CV={r.q_cv:.3f}).")
    elif r.pot_type == "unknown":
        print("UNKNOWN — insufficient data for classification")
        if r.n_valid < 50:
            print(f"      Only {r.n_valid} valid data points, need more sweep data.")


# ======================================================================
# 自测
# ======================================================================

def self_test():
    """验证判别逻辑的边界条件"""
    print("=== dI/df Slope Discriminator Self-Test ===\n")

    # 测试 _classify 函数
    test_cases = [
        # (f0_est, f_min_high, slope_ratio, expected_type)
        (15000, 23000, 3.0, "iron"),     # f0=15k 远低于18k → 铁锅
        (18000, 22500, 4.0, "iron"),     # f0=18k < 19k → 铁锅
        (18500, 23500, 5.0, "iron"),     # f0=18.5k < 19k → 铁锅
        (19500, 24000, 6.0, "iron"),     # 灰色区域, f_min<25k → 铁锅
        (19500, 27000, 6.0, "steel"),    # 灰色区域, f_min>25k → 钢锅
        (20000, 26000, 8.0, "steel"),    # f0=20k >= 20k → 钢锅
        (25000, 28000, 15.0, "steel"),   # 标准钢锅
        (22000, 26500, 12.0, "steel"),   # 钢锅
        (0, 23000, 0, "unknown"),        # 无效 f0
    ]

    all_ok = True
    for f0e, fmin, sr, exp in test_cases:
        got = _classify(f0e, fmin, sr)
        ok = "OK" if got == exp else "FAIL"
        if got != exp:
            all_ok = False
        print(f"  f0={f0e:>6.0f} f_min_high={fmin:>6.0f} "
              f"slope_ratio={sr:.0f}x  ->  {got:7s}  [{ok}]")

    # 测试 f0 外推逻辑
    print(f"\n  f0 extrapolation test:")
    # 模拟铁锅斜率: 25→56→88 ADC/kHz at 28.3, 25.0, 23.5 kHz
    # p=1 外推: 工作频率离 f0 越远, f0 估计偏高是预期行为
    iron_slopes = [(28300, 25e-6), (25000, 56e-6), (23500, 88e-6)]
    f0_i, r2_i = _estimate_f0(iron_slopes)
    iron_ok = 18000 < f0_i < 24000  # p=1 对远场铁锅估计在 18-24kHz
    print(f"    Iron sim: f0={f0_i:.0f} Hz, R^2={r2_i:.3f} "
          f"{'OK' if iron_ok else 'FAIL'}")

    # simulate steel: 19->42->71 ADC/kHz at 33.0, 28.9, 26.9 kHz
    steel_slopes = [(33000, 19e-6), (28900, 42e-6), (26900, 71e-6)]
    f0_s, r2_s = _estimate_f0(steel_slopes)
    steel_ok = f0_s >= 20000  # 钢锅 f0 在 20kHz+
    print(f"    Steel sim: f0={f0_s:.0f} Hz, R^2={r2_s:.3f} "
          f"{'OK' if steel_ok else 'FAIL'}")

    if not iron_ok or not steel_ok:
        all_ok = False

    print(f"\n  {'ALL PASS' if all_ok else 'SOME FAILED'}")
    return all_ok


# ======================================================================
# CLI
# ======================================================================

if __name__ == '__main__':
    if '--self-test' in sys.argv:
        ok = self_test()
        sys.exit(0 if ok else 1)

    elif '--csv' in sys.argv:
        idx = sys.argv.index('--csv')
        csv_path = sys.argv[idx + 1]
        result = analyze_csv(csv_path)
        print_report(result, csv_path)

        if '--json' in sys.argv:
            out = {
                "pot_type": result.pot_type,
                "f0_est": round(result.f0_est, 1),
                "q_factor": round(result.q_factor, 1),
                "q_cv": round(result.q_cv, 3),
                "phase_rms": round(result.phase_rms, 1),
                "f0_didf": round(result.f0_didf, 1),
                "slope_ratio": round(result.slope_ratio, 2),
                "r_squared": round(result.r_squared, 3),
                "n_valid": int(result.n_valid),
                "per_power": [
                    {"power_w": pw, "f_median": round(fm),
                     "i_median": round(im), "phase_deg": round(ph, 1)}
                    for pw, fm, im, ph, sl in result.per_power
                ]
            }
            print("\n--- JSON ---")
            print(json.dumps(out, indent=2))
    else:
        print(__doc__)
