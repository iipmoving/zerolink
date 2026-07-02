% run_ih.m — 快速运行半桥IH模型
% 用法: matlab -batch "cd('d:\Reasonix\half_bridge_IH'); run_ih; exit"

% 加载标定 — 生成 cal 结构体
cal.ADC_BITS = 12;
cal.ADC_STEPS = 4096;
cal.VREF = 3.3;
cal.V_SCALE = cal.VREF / cal.ADC_STEPS / (6.2e3 / (270e3*3 + 6.2e3));
cal.I_SCALE = cal.VREF / cal.ADC_STEPS / (330 / (10e3 + 330));
cal.VDC_SCALE = cal.V_SCALE;
cal.MIN_PULSE_US = 6;
cal.F_AC = 50;
cal.HRTIM_CLK_MHZ = 144;
cal.V_AC_RMS = 220;
cal.V_AC_PK = 220 * sqrt(2);

fprintf('[cal] V_SCALE=%.5f  I_SCALE=%.5f  VDC_SCALE=%.5f\n', ...
        cal.V_SCALE, cal.I_SCALE, cal.VDC_SCALE);

% 加载数据
raw = load_data('data_simple.csv', cal);

fprintf('\n===== 原始数据概要 =====\n');
fprintf('采样点数: %d\n', raw.meta.n_rows);
fprintf('时间跨度: %.2f μs (%.3f ms)\n', ...
        max(raw.t_us)-min(raw.t_us), (max(raw.t)-min(raw.t))*1000);

% Vdc 统计
Vdc = raw.Vdc;
fprintf('\n===== Vdc (母线电压) 分析 =====\n');
fprintf('Vdc 均值: %.2f V\n', mean(Vdc, 'omitnan'));
fprintf('Vdc 最大值: %.2f V\n', max(Vdc));
fprintf('Vdc 最小值: %.2f V\n', min(Vdc));
fprintf('Vdc 纹波(pk-pk): %.2f V\n', max(Vdc)-min(Vdc));
fprintf('Vdc 纹波率: %.2f %%\n', (max(Vdc)-min(Vdc))/mean(Vdc)*100);

% HRTIM 时序
timing = calc_timing(raw, cal);

% 时域 (电流)
waveform = calc_waveform(raw, timing);

% 相位提取 (从电流谷值 + CMP)
phase_info = calc_phase(raw, timing);

% 功率 (基波+相位法)
power = calc_power(raw, waveform, timing, phase_info);

% RLC 闭环推算 (主) — Vdc+dI/dt+C, 最准
rlc = calc_rlc(raw, waveform, timing, phase_info);

% 输出参数汇总
fprintf('\n===== 参数汇总 =====\n');
fprintf('控制模式: %s\n', timing.ctrl_mode_str);
fprintf('开关频率: %.2f kHz\n', timing.f_sw_kHz);
fprintf('谐振频率: %.3f kHz  (从 L=%.2fμH + C=%.1fμF)\n', ...
        rlc.f_res_kHz, rlc.L_uH, rlc.C_uF);
fprintf('I_peak: %.2f A\n', waveform.I_peak_all);
fprintf('I_RMS: %.2f A\n', waveform.I_RMS_all);
fprintf('Vdc_mean: %.2f V\n', power.Vdc_mean);
fprintf('P_dc(直流侧): %.1f W  (参考)\n', power.P_dc_est);
fprintf('P(基波+相位): %.1f W\n', power.P);
fprintf('P(RLC闭环):   %.1f W  ★ 最准\n', rlc.P_W);
fprintf('φ=%.1f°  cosφ=%.4f  Q=%.2f  R=%.3fΩ\n', ...
        power.phi_deg, power.cos_phi, rlc.Q, rlc.R_ohm);
fprintf('L=%.2fμH  C=%.1fμF  f_res=%.2fkHz\n', ...
        rlc.L_uH, rlc.C_uF, rlc.f_res_kHz);
fprintf('死区 DT1=%.3fμs  DT2=%.3fμs\n', mean(timing.DT1_us), mean(timing.DT2_us));
fprintf('D_U=%.1f%%  D_L=%.1f%%\n', mean(timing.D_U_pct), mean(timing.D_L_pct));

fprintf('\n===== 完成 =====\n');
