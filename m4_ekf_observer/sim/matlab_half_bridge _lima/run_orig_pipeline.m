% run_orig_pipeline.m — 用原始 MATLAB 流水线处理真实 CSV
clear; clc;
fprintf('========== 原始 MATLAB 流水线 — 全参数输出 ==========\n\n');

%% 标定
calibration;
cal.MIN_PULSE_US = MIN_PULSE_US;
cal.HRTIM_CLK_MHZ = 144;
cal.I_SCALE   = I_SCALE;
cal.V_SCALE   = V_SCALE;
cal.VDC_SCALE = VDC_SCALE;

%% 加载 CSV (用 export_golden 同款解析，兼容原始 load_data)
CSV_PATH = 'r104850.csv';

fid = fopen(CSV_PATH, 'r');
fgetl(fid);  % skip header
all_t = []; all_I = []; all_Vdc = []; all_CNT = [];
all_CU = []; all_CO = []; all_CL = []; all_LO = [];
last_CMP = [0 0 0 0];

while ~feof(fid)
    line = fgetl(fid);
    if ~ischar(line), break; end
    line = strtrim(line);
    if isempty(line) || startsWith(line, 'SIZE'), continue; end
    parts = strsplit(line, ',', 'CollapseDelimiters', false);
    t_val = str2double(strtrim(parts{1}));
    if isnan(t_val), continue; end
    all_t(end+1)   = t_val;
    all_I(end+1)   = str2double(strtrim(parts{2}));
    all_Vdc(end+1) = str2double(strtrim(parts{4}));
    all_CNT(end+1) = str2double(strtrim(parts{5}));
    for k = 6:9
        if length(parts) >= k
            v = str2double(strtrim(parts{k}));
            if ~isnan(v), last_CMP(k-5) = v; end
        end
    end
    all_CU(end+1) = last_CMP(1);
    all_CO(end+1) = last_CMP(2);
    all_CL(end+1) = last_CMP(3);
    all_LO(end+1) = last_CMP(4);
end
fclose(fid);

fprintf('Loaded %d samples from %s\n', length(all_t), CSV_PATH);

%% 检测 CNT 绕回, 取第一个完整周期 (跳过不完整首段)
% 切除野值 (I_adc > 60000)
bad = find(all_I > 60000);
if ~isempty(bad)
    all_I(bad) = 3;
end

dCNT = diff(all_CNT);
wrap_idx = find(dCNT < 0);
bounds = [1; wrap_idx(:)+1; length(all_t)];
% 跳过太短的周期 (与 export_golden 一致: >= 15)
for b = 1:(length(bounds)-1)
    s = bounds(b); e_i = bounds(b+1) - 1;
    if e_i - s + 1 >= 15
        break;
    end
end

fprintf('Using cycle: samples %d..%d (%d pts)\n', s, e_i, e_i-s+1);

% 只取这个周期
all_t   = all_t(s:e_i);
all_I   = all_I(s:e_i);
all_Vdc = all_Vdc(s:e_i);
all_CNT = all_CNT(s:e_i);
all_CU  = all_CU(s:e_i);
all_CO  = all_CO(s:e_i);
all_CL  = all_CL(s:e_i);
all_LO  = all_LO(s:e_i);

fprintf('Cycle: n=%d  I_adc=[%d,%d]  CNT=[%d,%d]\n', ...
    e_i-s+1, min(all_I), max(all_I), min(all_CNT), max(all_CNT));

%% 构造 raw struct (兼容原始 calc_* 接口)
raw.t    = all_t' * 1e-6;      % μs → s
raw.t_us = all_t';
raw.I    = all_I' * I_SCALE;   % ADC → A
raw.V    = all_Vdc' * VDC_SCALE;  % V (母线电压)
raw.Vdc  = all_Vdc' * VDC_SCALE;
raw.CNT  = all_CNT';
raw.CMP  = [all_CU' all_CO' all_CL' all_LO'];
raw.I_adc   = all_I';
raw.V_adc   = all_Vdc';
raw.Vdc_adc = all_Vdc';
raw.POWER   = NaN(length(all_t), 1);

% 元信息
dt_us = median(diff(all_t));
dCNT = diff(all_CNT); dCNTp = dCNT(dCNT > 0);
avg_dcnt = mean(dCNTp);
t_per_cnt_us = dt_us / avg_dcnt;

raw.meta.csv_path      = CSV_PATH;
raw.meta.n_rows        = length(all_t);
raw.meta.n_cols        = 9;
raw.meta.hrtim_clk_mhz = round(1 / (t_per_cnt_us * 1e-6) / 1e6);
raw.meta.t_per_cnt_us  = t_per_cnt_us;
raw.meta.CNT_min       = min(all_CNT);
raw.meta.CNT_max       = max(all_CNT);

wrap_fix = find(diff(all_CNT) < 0, 1);
if ~isempty(wrap_fix)
    cnt_range = all_CNT(wrap_fix) + round(avg_dcnt) - all_CNT(wrap_fix+1);
else
    cnt_range = max(all_CNT) - min(all_CNT);
end
raw.meta.CNT_range     = cnt_range;
raw.meta.HRTIM_period_s = cnt_range * t_per_cnt_us * 1e-6;
raw.meta.f_sw          = 1 / raw.meta.HRTIM_period_s;
T_total_ms = (max(all_t) - min(all_t)) * 1e-3;
if abs(T_total_ms - 20) < 2, raw.meta.data_mode = 'A';
else, raw.meta.data_mode = 'B'; end

%% 运行原始流水线
fprintf('\n--- 1. 时序分析 ---\n');
timing = calc_timing(raw, cal);

fprintf('\n--- 2. 时域参量 ---\n');
waveform = calc_waveform(raw, timing);

fprintf('\n--- 3. 相位提取 ---\n');
% 抑制 calc_phase 内循环 fprintf: 临时用 scalar D_U
tmp_D_U = timing.D_U_pct; timing.D_U_pct = mean(timing.D_U_pct);
phase_info = calc_phase(raw, timing);
timing.D_U_pct = tmp_D_U;  % 恢复

fprintf('\n--- 4. 功率计算 ---\n');
power = calc_power(raw, waveform, timing, phase_info);

fprintf('\n--- 5. RLC 推算 (KVL) ---\n');
rlc = calc_rlc(raw, waveform, timing, phase_info);

fprintf('\n--- 6. 阻抗建模 (V_fund 基波法) ---\n');
impedance = calc_impedance(waveform, power, timing);

%% ======== 汇总输出 ========
fprintf('\n========== 全参数汇总 ==========\n');
fprintf('CSV: %s (%d samples, mode=%s)\n', CSV_PATH, e_i-s+1, raw.meta.data_mode);

fprintf('\n—— 时序 ——\n');
fprintf('  f_sw          = %.2f kHz\n', timing.f_sw_kHz);
fprintf('  T_hrtim       = %.2f us\n', timing.T_hrtim_us);
fprintf('  D_U           = %.1f %%\n', mean(timing.D_U_pct));
fprintf('  DT1           = %.3f us\n', mean(timing.DT1_us));
fprintf('  DT2           = %.3f us\n', mean(timing.DT2_us));
fprintf('  ctrl_mode     = %s\n', timing.ctrl_mode);

fprintf('\n—— 波形 ——\n');
fprintf('  f_res_mean    = %.3f kHz  (谷值间隔法)\n', waveform.f_res_mean_khz);
fprintf('  f_res_std     = %.3f kHz\n', waveform.f_res_std_khz);
fprintf('  I_peak_all    = %.2f A\n', waveform.I_peak_all);
fprintf('  I_RMS_all     = %.3f A\n', waveform.I_RMS_all);
fprintf('  Vdc_mean      = %.1f V\n', waveform.V_avg_all);

fprintf('\n—— 相位 ——\n');
fprintf('  phi_deg       = %.1f deg\n', phase_info.phi_deg);
fprintf('  cos_phi       = %.4f\n', phase_info.cos_phi);
fprintf('  n_valleys     = %d\n', phase_info.n_valleys);

fprintf('\n—— 功率 ——\n');
fprintf('  P (基波法)    = %.1f W\n', power.P);
fprintf('  S             = %.1f VA\n', power.S);
fprintf('  Q             = %.1f var\n', power.Q);
fprintf('  PF            = %.4f\n', power.PF);
fprintf('  P_dc (直流法) = %.1f W\n', power.P_dc_est);

fprintf('\n—— RLC (KVL 封闭解) ——\n');
if rlc.valid
    fprintf('  L             = %.2f uH\n', rlc.L_uH);
    fprintf('  f_res         = %.2f kHz\n', rlc.f_res_kHz);
    fprintf('  Q             = %.2f\n', rlc.Q);
    fprintf('  R             = %.3f ohm\n', rlc.R_ohm);
    fprintf('  V_C_peak      = %.1f V\n', rlc.V_C_peak_V);
else
    fprintf('  (无效)\n');
end

fprintf('\n—— 阻抗 (V_fund 基波法估算) ——\n');
fprintf('  |Z|           = %.3f ohm\n', impedance.Z_mag);
fprintf('  R (P=I^2R)    = %.3f ohm\n', impedance.R);
fprintf('  X             = %.3f ohm\n', impedance.X);
fprintf('  L_eq          = %.2f uH\n', impedance.L_eq * 1e6);
fprintf('  Q             = %.2f\n', impedance.Q);
fprintf('  BW            = %.1f Hz\n', impedance.BW);
fprintf('  network_type  = %s\n', impedance.network_type);

fprintf('\n========== 完成 ==========\n');
