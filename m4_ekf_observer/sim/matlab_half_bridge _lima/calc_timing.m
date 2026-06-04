function timing = calc_timing(raw, cal)
% calc_timing — HRTIM 时序分析：四边沿 CMP + 死区 + 模式检测
%
% 输入:
%   raw : load_data 输出 struct
%   cal : calibration struct
% 输出:
%   timing : struct 含以下字段

%% 基本参数
t          = raw.t;
CNT        = raw.CNT;
CMP        = raw.CMP;          % [UON, UOFF, LON, LOFF]
t_per_cnt  = raw.meta.t_per_cnt_us;   % μs/count
f_sw       = raw.meta.f_sw;           % Hz
min_pulse  = cal.MIN_PULSE_US;        % μs

%% 逐点计算时序
n = length(t);

% 从 CMP 提取四边沿
CMP_UON  = CMP(:, 1);
CMP_UOFF = CMP(:, 2);
CMP_LON  = CMP(:, 3);
CMP_LOFF = CMP(:, 4);

% 各边沿的实际时间 (μs)
t_UON_us  = CMP_UON  * t_per_cnt;
t_UOFF_us = CMP_UOFF * t_per_cnt;
t_LON_us  = CMP_LON  * t_per_cnt;
t_LOFF_us = CMP_LOFF * t_per_cnt;

% 导通时间 (μs)
T_UON_us = max(0, t_UOFF_us - t_UON_us);
T_LON_us = max(0, t_LOFF_us - t_LON_us);

% HRTIM 周期 (μs)
T_hrtim_us = raw.meta.HRTIM_period_s * 1e6;
if T_hrtim_us <= 0
    T_hrtim_us = raw.meta.CNT_range * t_per_cnt;
end

% 占空比 (%)
D_U_pct = T_UON_us ./ T_hrtim_us * 100;
D_L_pct = T_LON_us ./ T_hrtim_us * 100;

% 死区时间 (μs)
% 死区1: 上管关断 → 下管开通
DT1_us = max(0, t_LON_us - t_UOFF_us);
% 死区2: 下管关断 → 上管开通 (可能跨周期，用模运算修正)
DT2_us_raw = t_UON_us - t_LOFF_us;
% 处理跨周期情况: 如果 DT2_raw < 0，加上一个周期
DT2_us = zeros(size(DT2_us_raw));
for i = 1:length(DT2_us_raw)
    if DT2_us_raw(i) >= 0
        DT2_us(i) = DT2_us_raw(i);
    else
        DT2_us(i) = DT2_us_raw(i) + T_hrtim_us;
    end
end

%% 最小脉宽检查
pulse_ok_U = T_UON_us >= min_pulse;
pulse_ok_L = T_LON_us >= min_pulse;

%% 工作模式判别（每个采样点）
% 若上下管占空比之差 < 2% → 50% 互补模式
D_diff = abs(D_U_pct - D_L_pct);
mode_50pct = D_diff < 2;

% 模式汇总
if all(mode_50pct)
    mode_str = '50% 互补模式';
elseif ~any(mode_50pct)
    mode_str = '上短下长非对称模式';
else
    mode_str = '混模 (数据中模式有切换)';
end

%% 有效占空比 (决定半桥中点偏置)
% 在非对称模式下，上管占空比 ≠ 50%
D_eff_pct = D_U_pct;  % 以上管为参考

%% 开关频率确认
f_sw_measured = 1 ./ (diff(t) + eps);  % Hz
f_sw_median   = median(f_sw_measured(f_sw_measured > 0));

%% 控制模式判别 (FM vs PWM)
% FM: 频率变化大, 占空比稳定
% PWM: 频率稳定, 占空比变化大
f_sw_hz = f_sw_median;  % Hz, 从实测中位数取得

% 从 D_eff_pct 的变异性判断
D_std = std(D_U_pct);
% 从 CMP 边沿变化判断: 如果 CMP_UON/UOFF 在移动说明在调脉宽
CMP_UOFF_std = std(CMP_UOFF);
CMP_UON_std  = std(CMP_UON);

% 判断逻辑:
% 若占空比几乎不变(<2%变化)且CMP对称 → FM调频模式
% 若占空比明显变化(>2%)且CMP不对称 → PWM调占空比模式
if D_std < 2 && mean(D_U_pct) > 45
    ctrl_mode = 'FM';        % 调频模式 (50%互补, 变频率)
    ctrl_mode_str = '调频模式 (FM) — 固定50%占空比, 变频率调功';
else
    ctrl_mode = 'PWM';       % 调占空比模式 (固定频率, 变脉宽)
    ctrl_mode_str = '调占空比模式 (PWM) — 固定频率, 变脉宽调功';
end

%% 频率变化范围 (FM模式关键指标)
if strcmp(ctrl_mode, 'FM')
    % 在FM模式下, 通过CNT周期逐段检测频率
    % 整段CNT_range已作为平均, 逐段计算频率变化
    % 简化: 将数据分段
    seg_size = max(1, floor(n / 10));
    f_sw_seg = zeros(floor(n/seg_size), 1);
    for seg = 1:length(f_sw_seg)
        idx_seg = (seg-1)*seg_size + 1 : min(seg*seg_size, n);
        cnt_range_seg = max(CNT(idx_seg)) - min(CNT(idx_seg));
        if cnt_range_seg > 0
            t_per_cnt = raw.meta.t_per_cnt_us;
            T_seg_us = cnt_range_seg * t_per_cnt;
            f_sw_seg(seg) = 1 / (T_seg_us * 1e-6);
        end
    end
    f_sw_seg = f_sw_seg(f_sw_seg > 0);
    timing.f_sw_min_kHz = min(f_sw_seg) / 1e3;
    timing.f_sw_max_kHz = max(f_sw_seg) / 1e3;
    timing.f_sw_range_kHz = timing.f_sw_max_kHz - timing.f_sw_min_kHz;
else
    f_sw_kHz_val = f_sw_median / 1e3;
    timing.f_sw_min_kHz = f_sw_kHz_val;
    timing.f_sw_max_kHz = f_sw_kHz_val;
    timing.f_sw_range_kHz = 0;
end

%% 打包输出
timing.CMP_UON     = CMP_UON;
timing.CMP_UOFF    = CMP_UOFF;
timing.CMP_LON     = CMP_LON;
timing.CMP_LOFF    = CMP_LOFF;

timing.t_UON_us    = t_UON_us;
timing.t_UOFF_us   = t_UOFF_us;
timing.t_LON_us    = t_LON_us;
timing.t_LOFF_us   = t_LOFF_us;

timing.T_UON_us    = T_UON_us;       % 上管导通时间 μs
timing.T_LON_us    = T_LON_us;       % 下管导通时间 μs
timing.T_hrtim_us  = T_hrtim_us;     % HRTIM 周期 μs
timing.f_sw_kHz    = 1 / (T_hrtim_us * 1e-6) / 1e3;  % kHz

timing.D_U_pct     = D_U_pct;        % 上管占空比 %
timing.D_L_pct     = D_L_pct;        % 下管占空比 %
timing.D_eff_pct   = D_eff_pct;      % 有效占空比 %
timing.DT1_us      = DT1_us;         % 死区1 μs (上关→下开)
timing.DT2_us      = DT2_us;         % 死区2 μs (下关→上开)
timing.DT_avg_us   = (mean(DT1_us) + mean(DT2_us)) / 2;  % 平均死区 μs

timing.pulse_ok_U  = pulse_ok_U;     % 上管脉宽是否 ≥ 6μs
timing.pulse_ok_L  = pulse_ok_L;     % 下管脉宽是否 ≥ 6μs
timing.mode_50pct  = mode_50pct;     % true=互补, false=非对称
timing.mode_str    = mode_str;
timing.f_sw_median   = f_sw_median;  % 实测开关频率中位数 Hz
timing.ctrl_mode     = ctrl_mode;    % 'FM' 或 'PWM'
timing.ctrl_mode_str = ctrl_mode_str;
timing.D_std         = D_std;        % 占空比标准差 (判别依据)

%% 打印摘要
fprintf('=== calc_timing 时序分析 ===\n');
fprintf('控制模式: %s\n', ctrl_mode_str);
fprintf('半桥模式: %s\n', mode_str);
fprintf('开关频率: %.2f kHz\n', timing.f_sw_kHz);
if strcmp(ctrl_mode, 'FM')
    fprintf('  ├ 调频范围: %.2f ~ %.2f kHz (Δf=%.2f kHz)\n', ...
            timing.f_sw_min_kHz, timing.f_sw_max_kHz, timing.f_sw_range_kHz);
    fprintf('  └ 占空比(固定): %.1f%%\n', mean(D_U_pct));
else
    fprintf('  ├ 频率(固定): %.2f kHz\n', timing.f_sw_kHz);
    fprintf('  └ 占空比范围: %.1f%% ~ %.1f%% (σ=%.2f)\n', ...
            min(D_U_pct), max(D_U_pct), D_std);
end
fprintf('HRTIM 周期: %.2f μs\n', T_hrtim_us);
fprintf('上管导通: %.2f μs (D=%.1f%%)  下管导通: %.2f μs (D=%.1f%%)\n', ...
        mean(T_UON_us), mean(D_U_pct), mean(T_LON_us), mean(D_L_pct));
fprintf('死区1(上关→下开): %.3f μs  死区2(下关→上开): %.3f μs\n', ...
        mean(DT1_us), mean(DT2_us));
fprintf('最小脉宽(6μs)检查: 上管%s 下管%s\n', ...
        iff_h(all(pulse_ok_U), 'OK', '⚠失败'), ...
        iff_h(all(pulse_ok_L), 'OK', '⚠失败'));

end

%% 辅助: 简化 if-else 用于 fprintf
function s = iff_h(cond, t, f)
    if cond, s = t; else, s = f; end
end
