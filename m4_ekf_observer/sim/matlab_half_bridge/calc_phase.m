function phase_info = calc_phase(raw, timing)
% calc_phase — 从电流谷值 + HRTIM CMP 提取相位差 φ
%
% 原理:
%   半桥输出电压基波 v₁(t) 在 CMP_UON (上管开通) 时刻过零↑
%   谐振电流 i(t) = I_peak × sin(ωt - φ)
%   整流后 |i| 谷值 ≈ i 过零点 → ωt = φ + nπ
%   φ = (t_valley - t_UON) / T_sw × 360°
%
% 输入: raw (t, I, CNT), timing (CMP_UON, T_hrtim_us, f_sw_kHz)
% 输出: phase_info.phi_deg, .cos_phi, .v_fund_rms, .usable

t      = raw.t;
I      = raw.I;
CNT    = raw.CNT;
t_pcnt = raw.meta.t_per_cnt_us;   % μs/count

CMP_UON  = timing.CMP_UON(1);    % 取第一个值 (整帧不变)
CMP_UOFF = timing.CMP_UOFF(1);
T_sw_us  = timing.T_hrtim_us;
f_sw_hz  = timing.f_sw_kHz * 1e3;

%% 1) 找电流谷值 (整流后 |I| 的极小点)
I_peak_est = max(I);
valley_idx = find(I(2:end-1) < I(1:end-2) & I(2:end-1) < I(3:end) ...
                  & I(2:end-1) < I_peak_est * 0.15);
valley_idx = valley_idx + 1;

if length(valley_idx) < 2
    phase_info.phi_deg = NaN;
    phase_info.cos_phi = NaN;
    phase_info.usable  = false;
    return;
end

%% 2) 对每个谷值计算相位
phi_list = [];
for k = 1:length(valley_idx)
    vi = valley_idx(k);
    cnt_at_valley = CNT(vi);
    
    % 计算电压过零 (CMP_UON) 到电流谷值的距离 (counts)
    % 考虑计数器回绕
    dist_cnt = cnt_at_valley - CMP_UON;
    if dist_cnt < 0
        dist_cnt = dist_cnt + timing.T_hrtim_us / t_pcnt;
    end
    
    % 相位差 (度): 一个周期 360°
    phi_deg = (dist_cnt / (T_sw_us / t_pcnt)) * 360;
    
    % 归一化到 -180° ~ 180°
    phi_deg = mod(phi_deg + 180, 360) - 180;
    
    % 只保留合理范围 (-90° ~ 90°)
    if abs(phi_deg) <= 90
        phi_list(end+1) = phi_deg;
    end
end

if isempty(phi_list)
    phase_info.phi_deg = NaN;
    phase_info.cos_phi = NaN;
    phase_info.usable  = false;
    return;
end

phi_avg  = mean(phi_list);
cos_phi  = cosd(phi_avg);

%% 3) 从相位 + Vdc 还原谐振电压基波
% V_fund_peak = (2/π) × Vdc  (50%占空比)
% V_fund_RMS  = V_fund_peak / √2

% 需要 Vdc 均值 (在外面提供)
% 这里只计算比例因子, 实际电压在主流程中算
v_fund_factor = sqrt(2) / pi;  % V_fund_RMS = Vdc × v_fund_factor

%% 4) 打包
phase_info.phi_deg      = phi_avg;
phase_info.cos_phi      = cos_phi;
phase_info.v_fund_factor = v_fund_factor;
phase_info.n_valleys    = length(phi_list);
phase_info.usable       = true;

fprintf('=== calc_phase 相位提取 ===\n');
fprintf('检测到 %d 个电流谷值, 有效 %d 个\n', length(valley_idx), length(phi_list));
fprintf('相位差 φ = %.2f°  cos(φ) = %.4f\n', phi_avg, cos_phi);
fprintf('V_res 基波因子: V_fund_RMS = Vdc × %.4f\n', v_fund_factor);

end
