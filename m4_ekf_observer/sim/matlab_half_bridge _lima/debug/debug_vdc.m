% debug_vdc — check Vdc calibration
csv_path = '../../tools/ekf_tuner/captures/capture_20260602_092111.csv';

cal.ADC_BITS = 12; cal.ADC_STEPS = 4096; cal.VREF = 3.3;
cal.V_SCALE = cal.VREF/cal.ADC_STEPS/(6.2e3/(270e3*3+6.2e3));
cal.I_SCALE = cal.VREF/cal.ADC_STEPS/(330/(10e3+330));
cal.VDC_SCALE = cal.V_SCALE;

data = readmatrix(csv_path, 'NumHeaderLines', 1);
size_idx = find(isnan(data(:, 1)));
data(size_idx, :) = [];

I_adc = data(:, 2);
V_adc = data(:, 3);
I = I_adc * cal.I_SCALE;
Vdc = V_adc * cal.V_SCALE;

fprintf('=== 标定常数 ===\n');
fprintf('V_SCALE = %.5f V/count\n', cal.V_SCALE);
fprintf('I_SCALE = %.5f A/count\n', cal.I_SCALE);
fprintf('\n=== Vdc (全帧) ===\n');
fprintf('V_adc: min=%d max=%d mean=%d\n', round(min(V_adc)), round(max(V_adc)), round(mean(V_adc)));
fprintf('Vdc:   min=%.1fV max=%.1fV mean=%.1fV\n', min(Vdc), max(Vdc), mean(Vdc));
fprintf('\n=== I (全帧) ===\n');
fprintf('I_adc: min=%d max=%d\n', round(min(I_adc)), round(max(I_adc)));
fprintf('I:     min=%.2fA max=%.2fA\n', min(I), max(I));

% Find valley and peak
[~, valley_i] = min(I);
[~, peak_i] = max(I);
fprintf('\n=== 谷值/峰值点 ===\n');
fprintf('谷值点 idx=%d: Vdc=%.1fV  I=%.3fA  V_adc=%d  I_adc=%d\n', ...
        valley_i, Vdc(valley_i), I(valley_i), round(V_adc(valley_i)), round(I_adc(valley_i)));
fprintf('峰值点 idx=%d: Vdc=%.1fV  I=%.2fA  V_adc=%d  I_adc=%d\n', ...
        peak_i, Vdc(peak_i), I(peak_i), round(V_adc(peak_i)), round(I_adc(peak_i)));

% Formula check
C_FARAD = 0.9e-6;
f_sw = 30480;
omega = 2 * pi * f_sw;
I_peak_val = I(peak_i);
Vdc_valley_val = Vdc(valley_i);
V_C_peak = I_peak_val / (omega * C_FARAD);
L_val = (Vdc_valley_val/2 + V_C_peak) / (I_peak_val * omega);
fprintf('\n=== L 计算 (用 Vdc@谷值) ===\n');
fprintf('I_peak=%.2fA  Vdc@valley=%.1fV  f_sw=%.1fkHz\n', I_peak_val, Vdc_valley_val, f_sw/1e3);
fprintf('omega=%.0f  V_C_peak=%.1fV\n', omega, V_C_peak);
fprintf('C_term=1/(w^2*C)=%.1fuH  Vdc_term=%.1fuH\n', ...
        1/(omega^2*C_FARAD)*1e6, Vdc_valley_val/(2*I_peak_val*omega)*1e6);
fprintf('L=%.1fuH\n', L_val*1e6);

% Also compute with frame-average Vdc for comparison
Vdc_mean_val = mean(Vdc);
L_mean = (Vdc_mean_val/2 + V_C_peak) / (I_peak_val * omega);
fprintf('\n=== 对比: 用 Vdc 均值=%.1fV → L=%.1fuH\n', Vdc_mean_val, L_mean*1e6);

% Expected: if Vdc ~ 310V (220V AC)
Vdc_expected = 311;
L_expected = (Vdc_expected/2 + V_C_peak) / (I_peak_val * omega);
fprintf('        用 Vdc=311V         → L=%.1fuH\n', L_expected*1e6);
