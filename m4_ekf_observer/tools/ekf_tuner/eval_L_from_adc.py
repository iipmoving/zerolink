"""
Evaluate L (inductance) computation from per-cycle raw ADC data.

Physics:
  L = V_bus * dt / dI  (during switch-on, before current peak)
  f0 = 1 / (2 * pi * sqrt(L * C))  (resonant frequency)
  Q = f0 / (f2 - f1) where f1,f2 are -3dB points, or from decay envelope

Input: 数据.md — one half-bridge PWM cycle with per-us ADC readings
  Columns: Index, HRTIM value, 电压(bus voltage ADC), 谐振电流(resonant current ADC), 滤波值(filtered current)
  Params: highOff=10130, phase angle=90°, lowOn=15, phaseUpHrtim=684
  HRTIM clock: 384MHz, ADC_PER=384 ticks=1us

From previous run:
  - Period ≈ 26112 HRTIM ticks, f_sw ≈ 14,706 Hz
  - ON-period rise: di/dt = 54.7 ADC/us, R^2 = 0.9963
  - Rise time = 13.00 us, Delta I = 684 ADC
  - Zero-crossings at indices 15, 48 → resonant half-period ≈ 33 us → f_res ≈ 15,152 Hz
"""

import numpy as np
from collections import namedtuple

# --- Raw data from 数据.md ---
# Format: (Index, HRTIM, Voltage_ADC, Current_ADC, Filtered_Current)
raw_data = [
    (0, 25304, 2854, 622, 466),
    (1, 25688, 2856, 633, 670),
    (2, 40, 2847, 639, 647),
    (3, 424, 2779, 646, 628),
    (4, 808, 2874, 639, 658),
    (5, 1192, 2862, 693, 666),
    (6, 1576, 2839, 632, 640),
    (7, 1960, 2822, 567, 561),
    (8, 2344, 2861, 472, 469),
    (9, 2728, 2869, 380, 385),
    (10, 3112, 2867, 319, 318),
    (11, 3496, 2871, 258, 253),
    (12, 3880, 2867, 175, 178),
    (13, 4264, 2868, 100, 101),
    (14, 4648, 2862, 42, 41),
    (15, 5032, 2868, 11, 23),
    (16, 5416, 2865, 58, 58),
    (17, 5800, 2865, 127, 126),
    (18, 6184, 2867, 194, 193),
    (19, 6568, 2866, 250, 252),
    (20, 6952, 2868, 312, 307),
    (21, 7336, 2862, 358, 363),
    (22, 7720, 2858, 424, 419),
    (23, 8104, 2857, 477, 478),
    (24, 8488, 2851, 534, 533),
    (25, 8872, 2854, 586, 584),
    (26, 9256, 2846, 631, 632),
    (27, 9640, 2844, 676, 671),
    (28, 10024, 2838, 705, 707),
    (29, 10408, 2824, 746, 745),
    (30, 10792, 2829, 779, 779),
    (31, 11176, 2861, 795, 778),
    (32, 11560, 2846, 742, 760),
    (33, 11944, 2875, 755, 743),
    (34, 12328, 2884, 732, 732),
    (35, 12712, 2860, 695, 691),
    (36, 13096, 2868, 627, 624),
    (37, 13456, 2873, 551, 555),
    (38, 13840, 2877, 497, 495),
    (39, 14224, 2883, 442, 439),
    (40, 14616, 2885, 372, 372),
    (41, 15016, 2892, 301, 301),
    (42, 15400, 2890, 239, 243),
    (43, 15784, 2892, 201, 198),
    (44, 16168, 2895, 153, 151),
    (45, 16552, 2897, 95, 99),
    (46, 16936, 2906, 52, 49),
    (47, 17320, 2903, 12, 14),
    (48, 17704, 2908, 4, 13),
    (49, 18088, 2903, 52, 49),
    (50, 18472, 2904, 101, 100),
    (51, 18856, 2903, 146, 147),
    (52, 19240, 2899, 191, 191),
    (53, 19624, 2903, 236, 233),
    (54, 20008, 2898, 271, 274),
    (55, 20392, 2900, 316, 310),
    (56, 20776, 2894, 342, 346),
    (57, 21160, 2892, 384, 381),
    (58, 21544, 2889, 419, 420),
    (59, 21928, 2885, 458, 455),
    (60, 22312, 2887, 486, 486),
    (61, 22696, 2882, 514, 514),
    (62, 23080, 2879, 540, 536),
    (63, 23464, 2874, 551, 554),
    (64, 23848, 2872, 575, 573),
    (65, 24232, 2869, 596, 595),
    (66, 24616, 2864, 614, 614),
    (67, 25000, 2866, 630, 628),
    (68, 25384, 2858, 638, 640),
    (69, 25768, 2856, 647, 660),
    (70, 120, 2850, 638, 625),
    (71, 504, 2792, 643, 584),
    (72, 888, 2839, 670, 920),
]

# Extract arrays
n = len(raw_data)
idx   = np.array([r[0] for r in raw_data])
hrtim = np.array([r[1] for r in raw_data], dtype=np.int32)
v_adc = np.array([r[2] for r in raw_data], dtype=np.float64)
i_adc = np.array([r[3] for r in raw_data], dtype=np.float64)
i_filt = np.array([r[4] for r in raw_data], dtype=np.float64)

# --- Parameters ---
highOff  = 10130       # upper switch turn-off HRTIM time
lowOn    = 15          # deadtime
phaseUpHrtim = 684     # zero-crossing HRTIM offset
HRTIM_CLK = 384_000_000  # 384 MHz
ADC_PER   = 384          # 1 ADC sample per 384 HRTIM ticks = 1 us
ADC_REF   = 2.5          # Assumed ADC reference voltage (V)
ADC_BITS  = 12           # 12-bit ADC

# --- HRTIM unwrapping ---
# HRTIM wraps around when it exceeds the PWM period.
# Detect wrap points and reconstruct absolute time
# Estimate PWM period from data: max HRTIM value before wrap
# HRTIM runs from 0 to PWM_PERIOD, then wraps
# From data: max HRTIM ≈ 25768 (idx 69), then wraps to 120 (idx 70)
# But the waveform shows two ON periods per cycle, so full period is 2x highOff range
period_ticks = 26112  # from previous computation

# Recompute: HRTIM values span 0 to ~26112 per electrical cycle
# Check: idx 0 HRTIM=25304, idx 1=25688, idx 2=40 (wrap), ...
# From idx 2 to idx 34, HRTIM goes 40→12328 (span ~12k), then idx 35-69
# goes 12712→25768 (span ~13k). Total period ≈ 26112.

hrtim_unwrapped = hrtim.astype(np.int64).copy()
wraps = []
cum_shift = 0
for i in range(1, n):
    if hrtim[i-1] - hrtim[i] > 20000:  # large negative jump = wrap
        wraps.append(i)
        cum_shift += period_ticks
    hrtim_unwrapped[i] += cum_shift

print(f"HRTIM wraps at indices: {wraps}")
print(f"Estimated PWM period: {period_ticks} ticks = {period_ticks/HRTIM_CLK*1e6:.1f} us")
print(f"f_sw = {HRTIM_CLK/period_ticks:.1f} Hz")

# --- Time axis in us ---
# HRTIM ticks → us: 384 ticks = 1 us (HRTIM_CLK = 384 MHz, ADC_PER = 384)
time_us = hrtim_unwrapped.astype(np.float64) / ADC_PER
time_us -= time_us[0]  # relative to start

# --- 1. Find the ON period (upper switch conducting, current rising) ---
# The ON period starts when HRTIM resets (after wrap) and current begins rising
# From the data, the ON period is idx 2-28 (after first wrap)
# Current minimum is at idx 15 (filtered=23), but this is during resonance...
# Actually, let me re-examine the switching sequence:
#   - highOff=10130: upper switch turns off at HRTIM=10130
#   - lowOn=15: deadtime then lower switch turns on
#   - phaseUpHrtim=684: zero-crossing detected here
# The switch-ON (upper) happens at the beginning of each PWM cycle (HRTIM=0 area)
# HRTIM wraps at idx 2 (40) and idx 70 (120)
# So the ON period (upper switch on) is from wrap to highOff

print("\n=== WAVEFORM ANALYSIS ===")

# Find ON-period: HRTIM between 0~highOff after each wrap
# First ON period: idx 2 (HRTIM=40) to just before highOff
# Second ON period: idx 70 (HRTIM=120) to idx 72 (HRTIM=888)

# The full resonant cycle includes:
# 1. Upper switch ON: HRTIM 0 → highOff (~10130), current rises then peaks
# 2. Deadtime: highOff → highOff+lowOn
# 3. Lower switch ON: highOff+lowOn → end of period, current resonates back

# Let's find the ON-period segment more precisely
# After the wrap at idx 2, HRTIM goes from 40 → ... → should reach ~10130
# But the data only goes to idx 28 (HRTIM=10024) before the next wrap

# Actually the wrap at idx 2 (40) marks the START of the ON period
# ON period should continue until HRTIM ≈ highOff = 10130
# But HRTIM at idx 28 is 10024, and idx 29 jumps back to 10408 (no, it's 10408)
# Let me check the HRTIM progression more carefully

print("HRTIM progression:")
for i in range(n):
    marker = ""
    if i in wraps:
        marker = " <-- WRAP (switch ON)"
    if hrtim[i] < 200 and i > 0 and i-1 not in wraps:
        marker = " <-- near zero"
    print(f"  idx {i:2d}: HRTIM={hrtim[i]:5d}  V={v_adc[i]:4.0f}  I_raw={i_adc[i]:4.0f}  I_filt={i_filt[i]:4.0f}{marker}")

# The wraps at idx 2 (40) and idx 70 (120) are the PWM cycle starts
# ON period is from wrap to highOff (10130)
# First ON period: idx 2..28 (HRTIM 40..10024, stops just before 10130)
# Then there's a gap, and we see the resonant return current

# --- 2. Extract ON-period linear rise region ---
# During ON period, V_bus is applied across L, so I rises linearly
# We need to identify the linear region (before current peak or before turn-off)

# From first ON period (idx 2-28):
# Current starts at idx 2 (I_filt=647) and rises to idx 27 (I_filt=671), peaks at idx 28 (I_filt=707)
# Wait, that doesn't look right. Let me re-examine.

# Actually, looking at i_filt:
# idx 2-14: I_filt goes 647→628→658→666→640→561→469→385→318→253→178→101→41
# This is going DOWN. So current is actually FALLING during idx 2-14!
# idx 15: I_filt=23 (near zero crossing!)
# idx 15-28: I_filt goes 23→58→126→193→252→307→363→419→478→533→584→632→671→707
# This is going UP. So current is RISING during idx 15-28!

# Hmm, this means:
# - idx 2-14: resonant current falling (lower switch was on, current returns through diode)
# - idx 15: zero crossing
# - idx 15-28: resonant current rising (upper switch is on, V_bus drives current)
# - idx 28-48: current falls again (resonance)
# - idx 48: zero crossing
# - idx 48+: current rises again (next half cycle)

# Wait, but the parameter says "phaseUpHrtim=684" which is the zero-crossing timing.
# And HRTIM at idx 15 is 5032. Hmm, doesn't directly match.

# Let me reconsider: In a half-bridge IH cooker:
# - Upper switch ON: bus voltage applied, resonant current rises
# - Upper switch OFF + deadtime: freewheeling
# - Lower switch ON: resonant current continues in reverse
# - Lower switch OFF + deadtime: freewheeling

# The key "linear rise" is during upper switch ON, where V = L * di/dt

# Looking at the data again more carefully:
# The HRTIM wraps indicate new PWM cycles. At each wrap, the upper switch turns ON.
# But the current waveform is the RESONANT current, which includes both the forced
# response (linear di/dt) and the resonant response.

# Actually, let me re-think. The half-bridge works like this:
# - The resonant tank (L + C) resonates at f0
# - The PWM switches at f_sw (which is lower than f0 for ZVS)
# - During upper switch ON: V_bus is applied to the resonant tank
# - The current has TWO components: forced (V_bus * t / L) and resonant

# In a series resonant converter:
#   I(t) = I_DC(t) + I_AC(t)
#   I_DC(t) = (V_bus/L) * t  (linear ramp during ON time)
#   I_AC(t) = I_peak * sin(2pi*f0*t + φ)  (resonant component)

# The "filtered" value might already separate these components!

# Let me look at the filtered current (i_filt) more carefully:
# idx 2 (start of cycle): i_filt=647
# idx 15 (zero crossing): i_filt=23
# idx 28: i_filt=707

# The difference between raw and filtered:
# raw - filtered at idx 2: 639-647 = -8
# raw - filtered at idx 14: 42-41 = 1
# raw - filtered at idx 15: 11-23 = -12
# raw - filtered at idx 28: 705-707 = -2

# It seems like the filtered value is very close to raw. The "滤波值" might be a
# simple moving average or kalman filter output.

# OK let me take a different approach. Let me just compute di/dt from the
# current rise period directly, without trying to understand every detail of
# the circuit operation.

# From the data, the clear "rise" segments are:
# Segment A: idx 15→28 (I_filt: 23→707, Delta t≈13us)
#   This is where current rises during upper switch ON
# Segment B: idx 48→66 (I_filt: 13→614, similar)

# --- 3. Linear fit on rising edge ---
print("\n=== ON-PERIOD LINEAR RISE (idx 15→28) ===")

# Use filtered current for cleaner signal
x_rise = time_us[15:29]  # inclusive of 28
y_rise = i_filt[15:29]

# Linear regression
A = np.vstack([x_rise, np.ones_like(x_rise)]).T
slope, intercept = np.linalg.lstsq(A, y_rise, rcond=None)[0]

y_fit = slope * x_rise + intercept
residuals = y_rise - y_fit
ss_res = np.sum(residuals**2)
ss_tot = np.sum((y_rise - np.mean(y_rise))**2)
r_squared = 1 - ss_res / ss_tot

print(f"  Slope (di/dt): {slope:.2f} ADC/us")
print(f"  R^2: {r_squared:.4f}")
print(f"  Delta t: {x_rise[-1] - x_rise[0]:.2f} us")
print(f"  Delta I: {y_rise[-1] - y_rise[0]:.0f} ADC")
print(f"  Mean V_bus in this window: {np.mean(v_adc[15:29]):.0f} ADC")

# --- 4. Compute L in ADC units ---
v_mean = np.mean(v_adc[15:29])
di_dt = slope
L_adc = v_mean / di_dt  # ADC / (ADC/us) = us (in ADC time units)
print(f"\n  L (ADC units): {L_adc:.2f} ADC·us/ADC = {L_adc:.2f} V·us/A-equivalent")

# --- 5. Find zero-crossings for f0 ---
print("\n=== RESONANT FREQUENCY (f0) FROM ZERO-CROSSINGS ===")

# Zero-crossings in filtered current
zc_indices = []
for i in range(1, n):
    if (i_filt[i-1] > 0 and i_filt[i] <= 0) or (i_filt[i-1] < 0 and i_filt[i] >= 0):
        # Interpolate for more precise timing
        if i_filt[i] != i_filt[i-1]:
            alpha = abs(i_filt[i-1]) / abs(i_filt[i] - i_filt[i-1])
        else:
            alpha = 0.0
        zc_time = time_us[i-1] + alpha * (time_us[i] - time_us[i-1])
        zc_indices.append((i, zc_time, 'pos2neg' if i_filt[i-1] > 0 else 'neg2pos'))

print(f"  Zero-crossings found: {len(zc_indices)}")
for zc in zc_indices:
    print(f"    idx {zc[0]:2d} @ {zc[1]:.1f} us  ({zc[2]})")

# Compute resonant period from consecutive same-direction ZC
# Resonant period = time between two positive-going (or negative-going) ZCs
# Half-period = time between adjacent positive and negative ZCs
if len(zc_indices) >= 4:
    # Half periods (pos2neg → neg2pos)
    half_periods = []
    for i in range(len(zc_indices) - 1):
        hp = zc_indices[i+1][1] - zc_indices[i][1]
        half_periods.append(hp)

    # Full periods
    full_periods = []
    for i in range(len(zc_indices) - 2):
        if zc_indices[i][2] == zc_indices[i+2][2]:  # same direction
            fp = zc_indices[i+2][1] - zc_indices[i][1]
            full_periods.append(fp)

    if half_periods:
        hp_mean = np.mean(half_periods)
        f0_hp = 1.0 / (2 * hp_mean * 1e-6)
        print(f"\n  Half-periods: {[f'{hp:.2f}' for hp in half_periods]} us")
        print(f"  Mean half-period: {hp_mean:.2f} us")
        print(f"  f0 (from half-period): {f0_hp:.0f} Hz")

    if full_periods:
        fp_mean = np.mean(full_periods)
        f0_fp = 1.0 / (fp_mean * 1e-6)
        print(f"  Full periods: {[f'{fp:.2f}' for fp in full_periods]} us")
        print(f"  Mean full period: {fp_mean:.2f} us")
        print(f"  f0 (from full period): {f0_fp:.0f} Hz")

# --- 6. Compute Q from decay envelope ---
print("\n=== Q-FACTOR FROM DECAY ENVELOPE ===")

# In a resonant circuit, the current envelope decays as e^(-pi*f0*t/Q)
# Between consecutive peaks (one full cycle apart), amplitude ratio = e^(-2pi/Q)
# Q = 2pi / |ln(I_peak2/I_peak1)|

# Find current peaks (absolute value)
peaks = []
for i in range(2, n-2):
    # Find local maxima of absolute current
    if (abs(i_filt[i]) > abs(i_filt[i-1]) and
        abs(i_filt[i]) > abs(i_filt[i-2]) and
        abs(i_filt[i]) >= abs(i_filt[i+1]) and
        abs(i_filt[i]) >= abs(i_filt[i+2])):
        peaks.append((i, time_us[i], i_filt[i]))

print(f"  Peaks found: {len(peaks)}")
for p in peaks:
    print(f"    idx {p[0]:2d} @ {p[1]:.1f} us  I={p[2]:.0f}")

if len(peaks) >= 3:
    # Compute Q from successive peak ratios
    # For a series RLC: I_peak(n+1)/I_peak(n) = e^(-pi/Q) per half-cycle
    # So per full cycle: ratio = e^(-2pi/Q)
    # Q = -pi / ln(r_half) per half cycle
    # Q = -2pi / ln(r_full) per full cycle

    q_estimates = []
    for i in range(1, len(peaks)):
        ratio = abs(peaks[i][2]) / abs(peaks[i-1][2])
        if ratio > 0 and ratio < 1:  # decaying
            # This is a half-cycle ratio
            q_half = -np.pi / np.log(ratio)
            q_estimates.append(q_half)

    if q_estimates:
        q_mean = np.mean(q_estimates)
        q_std = np.std(q_estimates)
        print(f"\n  Q estimates (per half-cycle): {[f'{q:.1f}' for q in q_estimates]}")
        print(f"  Mean Q: {q_mean:.1f} +/- {q_std:.1f}")

# --- 7. Cross-validate: L from di/dt vs L from f0 ---
print("\n=== CROSS-VALIDATION ===")

# If we have f0 from ZC timing, and we assume a resonant capacitor value,
# we can compute L from f0: L = 1 / (4pi^2 f0^2 C)
# And compare with L from di/dt

# Typical IH resonant capacitor values: 0.22, 0.27, 0.33 uF
# Given f0 estimation (~15kHz from ZC), let's check which C gives consistent L

# First, the switching frequency f_sw from HRTIM period
f_sw = HRTIM_CLK / period_ticks
print(f"  f_sw (PWM frequency): {f_sw:.1f} Hz")

# f0 from ZC (should be close to f_res)
if len(zc_indices) >= 3:
    # Use the most stable half-period
    hp_list = []
    for i in range(len(zc_indices) - 1):
        hp = zc_indices[i+1][1] - zc_indices[i][1]
        hp_list.append(hp)
    hp_median = np.median(hp_list)
    f0_zc = 1.0 / (2 * hp_median * 1e-6)
    print(f"  f0 (ZC, median half-period = {hp_median:.1f} us): {f0_zc:.0f} Hz")

# --- 8. Estimate physical L assuming reasonable circuit parameters ---
print("\n=== PHYSICAL L ESTIMATE (with assumptions) ===")

# Assumption 1: ADC reference = 2.5V, ADC is 12-bit (4096)
# Assumption 2: Voltage divider: 220V AC → some DC level after rectification and divider
#   Typical half-bridge: V_bus ≈ 310V DC (220 * 1.414)
#   To bring to ADC range: divider ~ 310/(ADC_REF * ADC_reading/4096)
#   With ADC_reading ≈ 2850 and ADC_REF = 2.5V:
#     V_at_ADC_pin = 2.5 * 2850 / 4096 = 1.74V
#     Divider ratio ≈ 310 / 1.74 = 178:1

# Let me check: at idx 29-31 (during OFF period with current still high),
# voltage ADC reads ~2824-2861, consistent with bus voltage

# Without exact calibration, compute "apparent L" per ADC count
# L [V·s/A] = V_bus_actual [V] * dt[s] / dI_actual[A]
# V_bus_actual = V_adc * V_scale (V/ADC)
# I_actual = I_adc * I_scale (A/ADC)
# L = (V_adc * V_scale) * dt / (dI_adc * I_scale)
#   = (V_scale / I_scale) * (V_adc * dt / dI_adc)
#   = K * L_adc_units

print(f"  L in ADC units: {L_adc:.1f} ADC_V·us/ADC_I")
print(f"  = {L_adc:.1f} × (V_scale / I_scale) uH")
print()
print("  To get physical L, need:")
print("    V_scale: actual bus voltage per ADC count (V/ADC)")
print("    I_scale: actual current per ADC count (A/ADC)")

# --- 9. Summary ---
print("\n" + "="*60)
print("SUMMARY: Per-cycle L evaluation from raw ADC")
print("="*60)
print(f"  di/dt linearity:     R^2 = {r_squared:.4f} (excellent)")
print(f"  Rise time:           {x_rise[-1]-x_rise[0]:.1f} us")
print(f"  Delta I (ADC):            {y_rise[-1]-y_rise[0]:.0f} ADC")
print(f"  Mean V_bus (ADC):    {v_mean:.0f} ADC")
print(f"  L (ADC units):       {L_adc:.1f} ADC·us/ADC")
if len(zc_indices) >= 3:
    print(f"  f0 (zero-crossing):  {f0_zc:.0f} Hz")
    # Resonant capacitor estimate from L and f0:
    # C = 1 / (4pi^2 f0^2 L)
    # In ADC units: C_adc = 1 / (4pi^2 f0^2 L_adc)
    C_adc_units = 1.0 / (4 * np.pi**2 * f0_zc**2 * L_adc * 1e-6)  # L in us-equivalent
    print(f"  If L={L_adc:.0f} ADC-units, f0={f0_zc:.0f} Hz:")
    print(f"    → C = {C_adc_units:.0f} (ADC_I/(ADC_V·us)) → needs V_scale/I_scale to get Farads")
    print(f"    → C ≈ {C_adc_units * 1e6:.1f} uF-equivalent in ADC units")
if 'q_mean' in dir():
    print(f"  Q (decay envelope):  {q_mean:.1f} +/- {q_std:.1f}")
print()
print("CONCLUSION: di/dt is highly linear (R^2>0.99), suitable for per-cycle L measurement.")
print("Need V_scale and I_scale calibration constants to convert to physical uH.")
print("Then f0 = 1/(2pi√(LC)) can be computed from L and known C.")
