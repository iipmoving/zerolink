#!/usr/bin/env python3
"""
峰值噪声滤除算法 — 完整验证 (30kHz 铁锅)
CMP_UOFF=12681, perAdc=384
"""

import csv

I_SCALE = 0.02523
csv_path = r'D:\OBSIDIAN\MOVING IH\低耦合程序架构\m4_ekf_observer\tools\ekf_tuner\captures\capture_20260602_145533.csv'
CMP_OFF = 12681  # 从 golden 获取

with open(csv_path) as f:
    lines = f.readlines()

data = []
for line in lines[1:]:
    line = line.strip()
    if not line or line.startswith('SIZE'):
        continue
    parts = line.split(',')
    try:
        data.append((float(parts[0]), float(parts[1]), float(parts[4])))
    except:
        continue

cnts = [d[2] for d in data]
wraps = [i for i in range(1, len(cnts)) if cnts[i] < cnts[i-1] - 1000]
bounds = [0] + wraps + [len(data)]

print('===== 峰值噪声滤除验证 =====')
print('CSV: capture_20260602_145533.csv')
print('CMP_UOFF=%d' % CMP_OFF)
print()

hdr = '%4s %4s %6s %9s %9s %8s %8s %8s  %s' % ('周期','n','pk','I_raw','I_corr','d3','d2','d1','判断')
print(hdr)
print('-' * 76)

noise_count = 0
total = 0

for ci in range(len(bounds)-1):
    s, e = bounds[ci], bounds[ci+1]
    if e - s < 15:
        continue
    seg = data[s:e]
    I_A = [d[1]*I_SCALE for d in seg]
    cnt = [d[2] for d in seg]
    n = len(I_A)

    min_idx = min(range(n), key=lambda i: cnt[i])
    win_start = min_idx
    win_end = min(range(n), key=lambda i: abs(cnt[i] - CMP_OFF))
    if win_end <= win_start + 2:
        win_end = n - 3
    win_end = min(win_end, n - 3)

    # 窗口内找峰值 (局部最大)
    peaks = []
    for i in range(win_start + 2, win_end):
        if I_A[i] > I_A[i-1] and I_A[i] > I_A[i+1] and I_A[i] > max(I_A) * 0.3:
            peaks.append(i)

    pk_idx = max(peaks, key=lambda i: I_A[i]) if peaks else (win_start + I_A[win_start:win_end].index(max(I_A[win_start:win_end])))
    pv = I_A[pk_idx]
    total += 1

    # === 核心噪声滤除算法 ===
    if pk_idx >= 3 and pk_idx < n:
        d3 = I_A[pk_idx-2] - I_A[pk_idx-3]
        d2 = I_A[pk_idx-1] - I_A[pk_idx-2]
        d1 = pv - I_A[pk_idx-1]

        if abs(d2) > 0.001 and abs(d1) > abs(d2) * 1.5:
            # 噪声: 最后一步跳变异常大
            ratio = d2 / d3 if abs(d3) > 0.001 else 0.7
            pred_d1 = d2 * ratio
            corr = I_A[pk_idx-1] + pred_d1 if pred_d1 > 0 else pv
            judge = 'NOISE'
            noise_count += 1
        else:
            corr = pv
            judge = 'OK'
    else:
        corr = pv
        d3 = d2 = d1 = 0
        judge = '-'
    
    # 对比修正前后的 I_peak (影响 L 计算)
    if corr != pv:
        delta_pct = (corr - pv) / pv * 100
        print('%4d %4d %6d %9.3f %9.3f %8.4f %8.4f %8.4f  %s (Δ=%.1f%%)' % (
            ci+1, n, pk_idx, pv, corr, d3, d2, d1, judge, delta_pct))
    else:
        print('%4d %4d %6d %9.3f %9.3f %8.4f %8.4f %8.4f  %s' % (
            ci+1, n, pk_idx, pv, corr, d3, d2, d1, judge))

print()
print('检测结果: %d 周期, %d 噪声 (%.1f%%)' % (total, noise_count, noise_count/total*100 if total else 0))
