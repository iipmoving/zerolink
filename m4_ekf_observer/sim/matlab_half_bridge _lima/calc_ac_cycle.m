function ac_cycle = calc_ac_cycle(raw, waveform, power)
% calc_ac_cycle — 50Hz 交流周期包络分析
%
% 对模式 A 数据 (完整20ms交流周期)，分析:
%   - 工频周期边界
%   - 包络变化趋势 (I_RMS, P, f_res 随时间变化)
%   - 调制深度
%   - 半周期能量分配

t     = raw.t;
I     = raw.I;
V     = raw.V;
cal   = raw.meta;  % 部分元信息

%% 1) 检测工频周期边界
% 从 V 的 50Hz 包络过零点检测
% 方法: 对 V 做低通或取包络 → 过零检测
% 更稳健: 取 |V| 的峰值包络

% 包络提取：用 Hilbert 变换或滑动峰值
% 简单方法: 滑动 RMS 窗口 ~1ms (一个谐振周期)
win_size = max(1, round(length(t) / 50));  % ~每个工频周期分50段
half_win = floor(win_size / 2);

I_env = zeros(size(I));
V_env = zeros(size(V));
for i = 1:length(t)
    i_start = max(1, i - half_win);
    i_end   = min(length(t), i + half_win);
    I_env(i) = rms(I(i_start:i_end));
    V_env(i) = rms(V(i_start:i_end));
end

%% 2) 工频过零点检测 (从 I_env 或 V_env)
% 找 V_env 的谷值点 → 对应工频过零
% 或用原始 V 的 50Hz 分量过零
% 简单: 对 I_env 做高通或差分找周期
dI_env = diff(I_env);
% 谷值: dI_env 从负变正
valley_idx = find(dI_env(1:end-1) <= 0 & dI_env(2:end) > 0);

if length(valley_idx) < 2
    % 包络不够清晰，退回仅做统计
    ac_cycle.n_ac_cycles = 0;
    ac_cycle.has_ac_data = false;
    fprintf('[calc_ac_cycle] ⚠ 无法分辨工频周期，可能是模式B数据(单周期)\n');
    return;
end

%% 3) 逐个工频半周期分析
ac_cycle.has_ac_data = true;
n_half = length(valley_idx) - 1;
ac_cycle.n_ac_cycles = floor(n_half / 2);  % 完整工频周期数

% 逐半周期统计
half_t_start = t(valley_idx(1:end-1));
half_t_end   = t(valley_idx(2:end));
half_I_RMS   = zeros(n_half, 1);
half_V_RMS   = zeros(n_half, 1);
half_P       = zeros(n_half, 1);
half_E       = zeros(n_half, 1);

for k = 1:n_half
    idx = valley_idx(k):valley_idx(k+1);
    if length(idx) < 3
        continue;
    end
    half_I_RMS(k) = rms(I(idx));
    half_V_RMS(k) = rms(V(idx));
    half_P(k)     = mean(V(idx) .* I(idx));
    half_E(k)     = trapz(t(idx), V(idx) .* I(idx));
end

%% 4) 包络特征
I_env_peak = max(I_env);
I_env_valley = min(I_env);
mod_depth = (I_env_peak - I_env_valley) / (I_env_peak + I_env_valley + eps);

%% 5) 打包
ac_cycle.t          = t;
ac_cycle.I_env      = I_env;
ac_cycle.V_env      = V_env;
ac_cycle.valley_idx = valley_idx;
ac_cycle.n_half     = n_half;
ac_cycle.half_t_start   = half_t_start;
ac_cycle.half_t_end     = half_t_end;
ac_cycle.half_I_RMS     = half_I_RMS;
ac_cycle.half_V_RMS     = half_V_RMS;
ac_cycle.half_P         = half_P;
ac_cycle.half_E         = half_E;
ac_cycle.I_env_peak     = I_env_peak;
ac_cycle.I_env_valley   = I_env_valley;
ac_cycle.mod_depth      = mod_depth;

fprintf('=== calc_ac_cycle 工频包络分析 ===\n');
fprintf('检测到 %d 个工频半周期 (≈%d 个完整交流周期)\n', n_half, ac_cycle.n_ac_cycles);
fprintf('电流包络: peak=%.2fA  valley=%.2fA 调制深度=%.2f\n', ...
        I_env_peak, I_env_valley, mod_depth);
if n_half > 0
    fprintf('半周期平均功率: %.2fW (min=%.2f max=%.2f)\n', ...
            mean(half_P), min(half_P), max(half_P));
end

end
