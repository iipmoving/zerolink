function impedance = calc_impedance(waveform, power)
% calc_impedance — 负载参数估算 (仅 I + Vdc, 无谐振电压)
%
% 无谐振电压 → 无法直接计算 |Z| = V_RMS / I_RMS
% 改为从电流波形包络估算 RLC 参数:
%
% 方法:
%   半桥输出电压基波幅值 ≈ Vdc/π (傅里叶基波, 50%占空比)
%   阻抗模 |Z|_est ≈ V_fund_rms / I_RMS
%   电阻 R_est ≈ P_est / I_RMS²
%   电抗 X_est = sqrt(|Z|² - R²)
%   L/C 从 X_est 和 f_res 反推
%
% 输入:
%   waveform : calc_waveform 输出 (含 f_res, I_RMS)
%   power    : calc_power 输出 (含 P, Vdc_mean)
% 输出:
%   impedance struct (全部标注为估算)

%% 输入
I_RMS     = waveform.I_RMS_all;
f_res     = waveform.f_res_mean_khz * 1e3;  % Hz
P_est     = power.P;
Vdc_mean  = power.Vdc_mean;

%% 阻抗模 (从 V_fund 估算)
% 半桥: V_fund_rms = Vdc/π  (对称50%占空比时)
V_fund_rms = Vdc_mean / pi;
Z_mag_est  = V_fund_rms / (I_RMS + eps);

%% 电阻 (从 P = I²R)
R_est = P_est / (I_RMS^2 + eps);

%% 电抗
if Z_mag_est > R_est
    X_est = sqrt(Z_mag_est^2 - R_est^2);
else
    X_est = 0;
end

%% 网络类型判别 (从负载性质)
% 电流相位落后电压 → 感性; 超前 → 容性
% 无相位实测, 从功率因数角推断符号
phi_power = power.phi_deg;
if abs(phi_power) < 5
    X_est = 0;
    network_type = '纯阻性 (≈谐振点)';
elseif phi_power > 0
    % PF角 > 0 → 感性 (电流落后电压)
    X_est = abs(X_est);
    network_type = '感性 (估算)';
else
    X_est = -abs(X_est);
    network_type = '容性 (估算)';
end

%% 反推 L / C
omega = 2 * pi * f_res;
L_eq  = 0;
C_eq  = Inf;

if abs(X_est) < 0.01
    L_eq = 0;  C_eq = Inf;
elseif X_est > 0
    L_eq = X_est / omega;          % H
    C_eq = Inf;
else
    C_eq = -1 / (omega * X_est);   % F
    L_eq = 0;
end

%% Q 值
Q_factor = 0;
if R_est > 0
    if X_est > 0
        Q_factor = omega * L_eq / R_est;
    elseif X_est < 0
        Q_factor = 1 / (omega * C_eq * R_est);
    end
end

%% 带宽
BW = f_res / (Q_factor + eps);

%% 打包
impedance.Z_mag   = Z_mag_est;      % Ω (估算)
impedance.phi_rad = atan2(X_est, R_est);
impedance.phi_deg = rad2deg(impedance.phi_rad);
impedance.R       = R_est;          % Ω (从P=I²R)
impedance.X       = X_est;          % Ω (估算)
impedance.L_eq    = L_eq;           % H
impedance.C_eq    = C_eq;           % F
impedance.Q       = Q_factor;
impedance.BW      = BW;             % Hz
impedance.network_type = network_type;
impedance.has_vres     = false;     % 无谐振电压标记

fprintf('=== calc_impedance 负载估算 (无谐振电压) ===\n');
fprintf('|Z|≈%.3fΩ  R≈%.3fΩ  X≈%.3fΩ  φ≈%.1f°\n', Z_mag_est, R_est, X_est, impedance.phi_deg);
fprintf('网络类型: %s\n', network_type);
if L_eq > 0
    fprintf('L_eq≈%.3f μH\n', L_eq * 1e6);
end
if isfinite(C_eq) && ~isinf(C_eq)
    fprintf('C_eq≈%.3f nF\n', C_eq * 1e9);
end
fprintf('Q≈%.2f  BW≈%.1f Hz\n', Q_factor, BW);
fprintf('⚠ 全部阻抗值为估算(基波法), 需V_resonant精确\n');

end
