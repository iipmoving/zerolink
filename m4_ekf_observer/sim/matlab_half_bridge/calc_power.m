function power = calc_power(raw, waveform, timing, phase_info)
% calc_power — 功率估算 (Vdc + I + 相位)
%
% 两路估算并存:
%   方法A: 直流侧法  P_dc = Vdc × I_dc_est (原方法, 保留对照)
%   方法B: 基波法    P = V_fund_RMS × I_RMS × cos(φ) (新增, 更准)
%
% 输入: raw, waveform, timing, phase_info (含 φ, cos_φ)

t   = raw.t;
I   = raw.I;
Vdc = raw.Vdc;
D_U = timing.D_U_pct / 100;

%% 0) 直流母线分析 (共用)
Vdc_mean      = mean(Vdc, 'omitnan');
Vdc_ripple_pk = (max(Vdc) - min(Vdc)) / 2;
Vdc_ripple_pct = Vdc_ripple_pk / Vdc_mean * 100;

%% 1) 方法A: 直流侧法 (原)
I_abs_mean = mean(abs(I));
I_dc_est   = I_abs_mean * mean(D_U);
P_dc       = Vdc_mean * I_dc_est;

%% 2) 方法B: 基波法 (含相位修正)
I_RMS = waveform.I_RMS_all;

if phase_info.usable
    cos_phi = phase_info.cos_phi;
    
    % 半桥输出电压基波有效值
    % V_fund_rms = Vdc × √2/π  (50%占空比时)
    % 非50%时: V_fund_rms = Vdc × √2/π × sin(π×D)
    D_factor = sin(pi * mean(D_U));
    V_fund_rms = Vdc_mean * (sqrt(2) / pi) * max(D_factor, 0.1);
    
    % 有功功率 P = V_fund × I_RMS × cos(φ)
    P_est_B = V_fund_rms * I_RMS * cos_phi;
    
    % 视在功率 S = V_fund × I_RMS
    S_est_B = V_fund_rms * I_RMS;
    
    % 无功 Q = V_fund × I_RMS × sin(φ)
    sin_phi = sind(phase_info.phi_deg);
    Q_est_B = V_fund_rms * I_RMS * abs(sin_phi);
    
    % 功率因数
    PF_B = cos_phi;
    
    % 等效电阻 R = P / I_RMS²
    R_eq_B = P_est_B / (I_RMS^2 + eps);
    
    has_phase = true;
else
    % 无相位数据 → 回退到方法A的估算
    P_est_B   = P_dc * 0.88;
    S_est_B   = Vdc_mean * I_RMS / pi;
    Q_est_B   = sqrt(max(0, S_est_B^2 - P_est_B^2));
    PF_B      = P_est_B / (S_est_B + eps);
    R_eq_B    = P_est_B / (I_RMS^2 + eps);
    has_phase = false;
end

%% 3) 单周期能量
t_span = max(t) - min(t);
E_cycle = P_est_B * t_span;

%% 4) 打包
power.P         = P_est_B;           % W (基波法, 更准)
power.S         = S_est_B;
power.Q         = Q_est_B;
power.PF        = PF_B;
power.phi_deg   = phase_info.phi_deg;
power.p_inst    = [];
power.E_cycle   = E_cycle;
power.E_total   = E_cycle;
power.R_eq      = R_eq_B;
power.Vdc_mean  = Vdc_mean;
power.Vdc_ripple_pk = Vdc_ripple_pk;
power.Vdc_ripple_pct = Vdc_ripple_pct;
power.Idc_est   = I_dc_est;
power.P_dc_est  = P_dc;
power.has_vdc   = all(isfinite(Vdc));
power.has_vres  = false;
power.has_phase = has_phase;
power.cos_phi   = phase_info.cos_phi;

fprintf('=== calc_power 功率 (基波+相位法) ===\n');
fprintf('方法A(直流侧): P_dc=%.1fW  I_dc(均)=%.2fA\n', P_dc, I_dc_est);
if has_phase
    fprintf('方法B(基波法): φ=%.1f°  cosφ=%.4f  V_fund=%.1f Vrms\n', ...
            phase_info.phi_deg, cos_phi, V_fund_rms);
    fprintf('  P=%.1fW  S=%.1fVA  PF=%.4f\n', P_est_B, S_est_B, PF_B);
    fprintf('  R_eq=%.3fΩ  (从P=I²R)\n', R_eq_B);
else
    fprintf('方法B: 无相位数据, 已回退估算\n');
end

end
