function rlc = calc_rlc(raw, waveform, timing, phase_info)
% calc_rlc — 从 Vdc + dI/dt + C 推算 RLC 参数 (封闭解)
%
% 不需要谐振电压测量.
% 已知: C (谐振电容, 固定值)
% 已知: Vdc (母线电压)
% 已知: I(t) (整流后电流)
% 已知: φ (从 calc_phase 提取)
%
% 方法:
%   1) dI/dt 在电流过零处 (谷值) 的斜率
%   2) I_peak, ω 已知 → V_C_peak = I_peak / (ωC)
%   3) KVL: Vdc/2 + V_C_peak = L × I_peak × ω   (半桥中点参考)
%   4) L = (Vdc/2 + V_C_peak) / (I_peak × ω)
%   5) f_res = 1/(2π√(LC))
%   6) Q = tan(φ) / (f_sw/f_res - f_res/f_sw)
%   7) R = ω₀L / Q
%   8) P = I_RMS² × R

%% 输入参数
C_FARAD = 0.9e-6;        % 谐振电容 0.9 μF (可配置)

Vdc   = mean(raw.Vdc, 'omitnan');
I     = raw.I;
t     = raw.t;
f_sw  = timing.f_sw_kHz * 1e3;
omega = 2 * pi * f_sw;
I_peak = waveform.I_peak_all;
I_RMS  = waveform.I_RMS_all;

%% 1) dI/dt: 在电流谷值附近测斜率
% 找谷值 (同 calc_phase)
I_peak_est = max(I);
valley_idx = find(I(2:end-1) < I(1:end-2) & I(2:end-1) < I(3:end) ...
                  & I(2:end-1) < I_peak_est * 0.15);
valley_idx = valley_idx + 1;

if length(valley_idx) < 1
    rlc.valid = false; return;
end

% 取第一个完整谷值, 用前后各1点算斜率
vi = valley_idx(1);
dt_us = mean(diff(t)) * 1e6;  % μs
dt_sec = dt_us * 1e-6;

if vi > 1 && vi < length(I)
    di_before = (I(vi-1) - I(vi)) / dt_sec;
    di_after  = (I(vi+1) - I(vi)) / dt_sec;
    di_dt = (abs(di_before) + abs(di_after)) / 2;
else
    rlc.valid = false; return;
end

%% 2) V_C_peak 和 L
V_C_peak = I_peak / (omega * C_FARAD);

% 半桥中点参考: 上管导通时 V_sw = Vdc/2
% 电流过零(从负→正), V_C 此时在负峰 ≈ -V_C_peak
% KVL: Vdc/2 = V_L + V_C → Vdc/2 = L*I_peak*ω + (-V_C_peak)
% 所以: L*I_peak*ω = Vdc/2 + V_C_peak
L_val = (Vdc/2 + V_C_peak) / (I_peak * omega);

%% 3) 谐振频率
f_res = 1 / (2 * pi * sqrt(L_val * C_FARAD));

%% 4) Q 值 (从相位)
if phase_info.usable
    phi_deg = phase_info.phi_deg;
    tan_phi = tand(phi_deg);
    ratio = f_sw / f_res;
    Q_val = tan_phi / (ratio - 1/ratio);
else
    Q_val = NaN;
end

%% 5) R 和 P
omega_0 = 2 * pi * f_res;
R_val = omega_0 * L_val / Q_val;
P_val = I_RMS^2 * R_val;

%% 6) 打包
rlc.C_uF     = C_FARAD * 1e6;
rlc.L_uH     = L_val * 1e6;
rlc.f_res_kHz = f_res / 1e3;
rlc.f_sw_kHz  = f_sw / 1e3;
rlc.Q        = Q_val;
rlc.R_ohm    = R_val;
rlc.P_W      = P_val;
rlc.Vdc_V    = Vdc;
rlc.V_C_peak_V = V_C_peak;
rlc.di_dt    = di_dt;
rlc.valid    = true;

fprintf('=== calc_rlc 从 Vdc+dI/dt+C 推算 ===\n');
fprintf('已知: C=%.1f μF, Vdc=%.1fV, f_sw=%.2fkHz\n', C_FARAD*1e6, Vdc, f_sw/1e3);
fprintf('dI/dt(过零)=%.2e A/s, I_peak=%.2fA\n', di_dt, I_peak);
fprintf('V_C_peak = I_peak/(ωC) = %.1fV\n', V_C_peak);
fprintf('L = (Vdc/2 + V_C_peak) / (I_peak·ω) = %.2f μH\n', L_val*1e6);
fprintf('f_res = 1/(2π√(LC)) = %.2f kHz\n', f_res/1e3);
fprintf('  → f_sw(%.2f) %s f_res(%.2f)  %s\n', ...
        f_sw/1e3, iff_c(phase_info.phi_deg > 0, '>', '<'), f_res/1e3, ...
        iff_c(phase_info.phi_deg > 0, '感性ZVS ✓', ''));
fprintf('Q = tan(φ) / (f_sw/f_res - f_res/f_sw) = %.2f\n', Q_val);
fprintf('R = ω₀L/Q = %.3f Ω\n', R_val);
fprintf('P = I_RMS² × R = %.1f W  (%.1fA² × %.3fΩ)\n', P_val, I_RMS, R_val);

end

function s = iff_c(cond, t, f)
    if cond, s = t; else, s = f; end
end
