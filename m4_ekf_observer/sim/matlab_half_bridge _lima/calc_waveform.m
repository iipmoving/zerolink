function waveform = calc_waveform(raw, timing)
% calc_waveform — 时域参量 (仅 I 整流后, 无谐振电压)
%
% 电流为全波整流后 |I|, 过零检测失效
% 改用**谷值检测**找周期:
%   |I| 在每个谐振半周期会跌到近零 → 找局部最小值
%
% 电压 V = Vdc (母线电压, 近乎恒定), 不用于谐振分析

t = raw.t;
I = raw.I;       % 整流后电流 |I| (A)
V = raw.V;       % Vdc (母线电压, 仅取均值参考)

%% 0) 判断电流是否整流 (全正或近零)
I_is_rectified = all(I >= -0.1);

%% 1) 周期检测: 整流电流用谷值法
if I_is_rectified
    % 找局部极小值 (整流后的"坑")
    % 条件: I[i] < I[i-1] AND I[i] < I[i+1]
    valley = find(I(2:end-1) < I(1:end-2) & I(2:end-1) < I(3:end));
    valley = valley + 1;  % 索引对齐

    % 过滤掉太浅的谷 (噪声干扰): I谷值 < I_peak × 0.15
    I_pk_e = max(I);
    valley = valley(I(valley) < I_pk_e * 0.15);

    % 每两个谷之间 = 一个谐振半周期
    half_cycles = length(valley) - 1;
    cycle_start = valley(1:end-1);
    cycle_end   = valley(2:end);
    n_cycles_used = half_cycles;
else
    % 原过零法 (备用)
    zcd = find(I(1:end-1) <= 0 & I(2:end) > 0);
    n_cycles = length(zcd) - 1;
    if n_cycles >= 1
        cycle_start = zcd(1:end-1);
        cycle_end   = zcd(2:end);
        n_cycles_used = n_cycles;
    else
        cycle_start = 1;
        cycle_end   = length(t);
        n_cycles_used = 1;
    end
end

%% 2) 逐半周期计算
f_res_khz = zeros(n_cycles_used, 1);
I_peak    = zeros(n_cycles_used, 1);
I_RMS     = zeros(n_cycles_used, 1);
I_avg     = zeros(n_cycles_used, 1);
CF_I      = zeros(n_cycles_used, 1);
FF_I      = zeros(n_cycles_used, 1);

for k = 1:n_cycles_used
    idx = cycle_start(k):cycle_end(k);
    if length(idx) < 4, continue; end

    Ik = I(idx);

    % 谐振频率: 每半周期 = 谐振半周期
    T_half = t(cycle_end(k)) - t(cycle_start(k));
    f_res_khz(k) = 1 / (T_half * 2) / 1e3;  % kHz

    I_peak(k) = max(Ik);
    I_RMS(k)  = rms(Ik);
    I_avg(k)  = mean(Ik);

    if I_avg(k) > 0,  FF_I(k) = I_RMS(k) / I_avg(k); end
    if I_RMS(k) > 0,  CF_I(k) = I_peak(k) / I_RMS(k); end
end

%% 3) 整段统计
I_peak_all = max(I);
I_RMS_all  = rms(I);
I_avg_all  = mean(I);

%% 4) Vdc 统计 (仅均值, 不做谐振分析)
Vdc_mean = mean(V);
Vdc_rms  = rms(V);

%% 5) 打包
waveform.f_res_khz  = f_res_khz;
waveform.I_peak     = I_peak;
waveform.I_RMS      = I_RMS;
waveform.I_avg      = I_avg;
waveform.CF_I       = CF_I;
waveform.FF_I       = FF_I;
waveform.n_cycles   = n_cycles_used;

waveform.I_peak_all = I_peak_all;
waveform.I_RMS_all  = I_RMS_all;
waveform.I_avg_all  = I_avg_all;

% Vdc 统计 (非谐振电压!)
waveform.V_peak_all = NaN;    % 无意义
waveform.V_RMS_all  = Vdc_rms;
waveform.V_avg_all  = Vdc_mean;

% 相位差: 无谐振电压 → 无法测量
waveform.phi_deg     = NaN(n_cycles_used, 1);
waveform.phi_deg_avg = NaN;

% 频率统计
if n_cycles_used > 1
    waveform.f_res_mean_khz = mean(f_res_khz);
    waveform.f_res_std_khz  = std(f_res_khz);
else
    waveform.f_res_mean_khz = f_res_khz(1);
    waveform.f_res_std_khz  = 0;
end

fprintf('=== calc_waveform 时域参量 (整流后电流) ===\n');
fprintf('检测方式: 谷值法 (全波整流)\n');
fprintf('电流基频(谷值法): %.3f kHz  (=f_sw 验证)\n', waveform.f_res_mean_khz);
fprintf('I_peak=%.2fA  I_RMS=%.2fA  I_avg=%.2fA\n', I_peak_all, I_RMS_all, I_avg_all);
fprintf('Vdc(均)=%.1fV  (仅母线电压, 非谐振电压)\n', Vdc_mean);
fprintf('峰值因数 CF=%.2f  波形因数 FF=%.2f\n', ...
        I_peak_all / I_RMS_all, I_RMS_all / I_avg_all);


end
