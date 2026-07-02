function run_per_frame(csv_path, out_path)
% run_per_frame — 逐帧计算 IH 参量, 结果附加到 CSV 新列
%
% 用法: run_per_frame('capture_20260602_092104.csv', 'capture_092104_result.csv')

if nargin < 2
    [p, name, ~] = fileparts(csv_path);
    out_path = fullfile(p, [name '_result.csv']);
end

%% 标定
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

%% 结果列名
result_cols = {'f_sw_kHz','DT1_us','DT2_us','D_U_pct','D_L_pct', ...
               'ctrl_mode','I_peak_A','I_RMS_A','I_at_UOFF','phi_deg','cos_phi', ...
               'P_W','P_fund','L_uH','Q','R_ohm','f_res_kHz'};

%% 读取 CSV, 按帧分割
fid = fopen(csv_path, 'r');
if fid < 0, error('无法打开: %s', csv_path); end

header_line = fgetl(fid);  % 表头
n_input_cols = length(strsplit(header_line, ','));  % 输入列数
frames_data = {};
frames_N    = {};
frames_size_row = {};  % 保存每帧的SIZE行模板

while ~feof(fid)
    line = fgetl(fid);
    if line == -1, break; end
    line = strtrim(line);
    if isempty(line), continue; end  % 空行跳过

    if startsWith(line, 'SIZE')
        size_parts = strsplit(line, ',');
        N = str2double(size_parts{2});
        frames_N{end+1} = N;
        frames_size_row{end+1} = size_parts;  % 保存SIZE行

        frame_rows = zeros(N, n_input_cols);
        for i = 1:N
            dline = fgetl(fid);
            if dline == -1, break; end
            dline = strtrim(dline);
            if isempty(dline)
                i = i - 1; continue;
            end
            parts = strsplit(dline, ',');
            n_parts = min(length(parts), n_input_cols);
            for j = 1:n_parts
                val = str2double(parts{j});
                if ~isnan(val)
                    frame_rows(i, j) = val;
                end
            end
        end
        frames_data{end+1} = frame_rows;
    end
end
fclose(fid);

n_frames = length(frames_data);
fprintf('检测到 %d 帧\n', n_frames);

%% 逐帧计算
results = cell(n_frames, 1);

for f = 1:n_frames
    frame = frames_data{f};
    N = frames_N{f};

    t_us   = frame(:, 1);
    I_adc  = frame(:, 2);
    V_adc  = frame(:, 3);
    Vdc_adc_val = frame(:, 4);
    CNT    = frame(:, 5);
    CMP    = frame(1, 6:9);  % 帧级参数, 第一行有效

    % ADC 限幅: 12-bit ADC 合法范围 [0, 4095], 超出为传输错误/毛刺
    I_adc  = min(max(I_adc, 0), 4095);
    V_adc  = min(max(V_adc, 0), 4095);
    Vdc_adc_val = min(max(Vdc_adc_val, 0), 4095);

    % 物理量还原
    t      = t_us * 1e-6;
    I      = I_adc * cal.I_SCALE;
    Vdc    = Vdc_adc_val * cal.VDC_SCALE;

    % 保护带: 帧头帧尾各 EDGE_SKIP 个点是 FMAC/DMA 过渡区, 数据不可靠
    % 找第一个有效过零点作为分析起点
    EDGE_SKIP = 2;
    I_robust_pk = robust_peak(I, EDGE_SKIP);
    valid_start = EDGE_SKIP + 1;
    for i = (EDGE_SKIP+1):(N-EDGE_SKIP)
        if I(i) < I_robust_pk * 0.15 && I(i-1) > I(i)
            valid_start = i;
            break;
        end
    end

    %% 构建 mini-raw 结构体
    raw.t       = t;
    raw.I       = I;
    raw.V       = Vdc;
    raw.Vdc     = Vdc;
    raw.CNT     = CNT;
    raw.CMP     = repmat(CMP, N, 1);
    raw.I_adc   = I_adc;
    raw.V_adc   = V_adc;
    raw.Vdc_adc = Vdc_adc_val;
    raw.t_us    = t_us;
    raw.meta.valid_start = valid_start;
    raw.meta.edge_skip   = EDGE_SKIP;
    raw.meta.I_robust_pk = I_robust_pk;

    % CNT 周期检测 (每帧独立)
    dCNT = diff(CNT);
    dCNT_positive = dCNT(dCNT > 0);
    if ~isempty(dCNT_positive)
        avg_dcnt = mean(dCNT_positive);
    else
        avg_dcnt = 1;
    end

    % 检测 CNT 绕回
    wrap_idx = find(diff(CNT) < 0, 1);
    if ~isempty(wrap_idx)
        cnt_before = CNT(wrap_idx);
        cnt_after  = CNT(wrap_idx + 1);
        CNT_range = cnt_before + round(avg_dcnt) - cnt_after;
    else
        CNT_range = max(CNT) - min(CNT);
    end

    dt_us_vec = diff(t_us);
    dt_us_vec(dt_us_vec <= 0) = [];
    if ~isempty(dt_us_vec)
        avg_dt_us = mean(dt_us_vec);
    else
        avg_dt_us = 0.5;
    end

    if avg_dcnt > 0 && avg_dt_us > 0
        t_per_cnt_us = avg_dt_us / avg_dcnt;
    else
        t_per_cnt_us = 1 / (cal.HRTIM_CLK_MHZ);
    end

    HRTIM_period_s = CNT_range * t_per_cnt_us * 1e-6;
    if HRTIM_period_s <= 0
        HRTIM_period_s = 1 / 30000;  % fallback 30kHz
    end
    f_sw_hz = 1 / HRTIM_period_s;

    raw.meta.t_per_cnt_us  = t_per_cnt_us;
    raw.meta.HRTIM_period_s = HRTIM_period_s;
    raw.meta.f_sw          = f_sw_hz;
    raw.meta.CNT_range     = CNT_range;
    raw.meta.CNT_min       = min(CNT);
    raw.meta.CNT_max       = max(CNT);
    raw.meta.n_rows        = N;
    raw.meta.n_cols        = 9;

    %% 调用计算管线
    try
        timing = calc_timing_mini(raw, cal);
        waveform = calc_waveform_mini(raw, timing);
        phase_info = calc_phase_mini(raw, timing);
        power = calc_power_mini(raw, waveform, timing, phase_info);
        rlc = calc_rlc_mini(raw, waveform, timing, phase_info);

        r = struct();
        r.f_sw_kHz  = timing.f_sw_kHz;
        r.DT1_us    = mean(timing.DT1_us);
        r.DT2_us    = mean(timing.DT2_us);
        r.D_U_pct   = mean(timing.D_U_pct);
        r.D_L_pct   = mean(timing.D_L_pct);
        r.ctrl_mode = timing.ctrl_mode;
        r.I_peak_A  = waveform.I_peak_all;
        r.I_RMS_A   = waveform.I_RMS_all;
        r.I_at_UOFF  = power.I_at_UOFF;
        r.phi_deg   = phase_info.phi_deg;
        r.cos_phi   = phase_info.cos_phi;
        r.P_W       = power.P;
        r.P_fund    = power.P_fund;
        r.L_uH      = rlc.L_uH;
        r.Q         = rlc.Q;
        r.R_ohm     = rlc.R_ohm;
        r.f_res_kHz = rlc.f_res_kHz;
    catch ME
        fprintf('帧%d 计算失败: %s\n', f-1, ME.message);
        r = struct();
        for c = result_cols
            r.(c{1}) = NaN;
        end
    end

    results{f} = r;
    fprintf('帧%d: f_sw=%.2fkHz  L=%.1fuH  Q=%.2f  P=%.0fW(P_fund=%.0fW)  I_off=%.1fA  phi=%.1f°\n', ...
            f-1, r.f_sw_kHz, r.L_uH, r.Q, r.P_W, r.P_fund, r.I_at_UOFF, r.phi_deg);
end

%% I_peak 修正 L: 高 I_peak 周期为参考, 低 I_peak 向参考修正
% L_corr = L_ref + (L_raw - L_ref) × (I_peak / I_max)^2
% I_peak < I_max×0.1 的周期丢弃 (信号太弱不可靠)
I_vals = zeros(n_frames, 1);
L_vals = zeros(n_frames, 1);
for f = 1:n_frames
    I_vals(f) = results{f}.I_peak_A;
    L_vals(f) = results{f}.L_uH;
end
[I_max, idx_max] = max(I_vals);
L_ref = L_vals(idx_max);
fprintf('[L_corr] 参考周期: I_peak=%.2fA, L=%.2fuH\n', I_max, L_ref);
for f = 1:n_frames
    ratio = I_vals(f) / I_max;
    if ratio < 0.1
        results{f}.L_uH_corr = NaN;
        results{f}.L_valid   = false;
        fprintf('[L_corr] 帧%d: I_peak=%.2fA (ratio=%.3f) 丢弃\n', f, I_vals(f), ratio);
    else
        weight = ratio^2;
        L_corr = L_ref + (L_vals(f) - L_ref) * weight;
        results{f}.L_uH_corr = L_corr;
        results{f}.L_valid   = true;
        delta = L_corr - L_vals(f);
        fprintf('[L_corr] 帧%d: I_peak=%.2fA L_raw=%.2f → L_corr=%.2f (%.2f)\n', ...
                f, I_vals(f), L_vals(f), L_corr, delta);
    end
end

%% 写出 CSV: 原始数据行 + 每帧末尾纵向结果摘要
fout = fopen(out_path, 'w');
fprintf(fout, '%s\n', header_line);

for f = 1:n_frames
    frame = frames_data{f};
    r = results{f};
    N = frames_N{f};
    sz_template = frames_size_row{f};

    sparse_col = false(1, n_input_cols);
    for col = 1:n_input_cols
        if col <= length(sz_template)
            sparse_col(col) = strcmp(strtrim(sz_template{col}), '1');
        end
    end

    % SIZE 行 (仅原始列)
    sz_parts = cell(1, n_input_cols);
    for col = 1:min(n_input_cols, length(sz_template))
        sz_parts{col} = sz_template{col};
    end
    fprintf(fout, '%s\n', strjoin(sz_parts, ','));

    % 数据行 (仅原始列, 不重复结果)
    for i = 1:N
        fields = cell(1, n_input_cols);
        for col = 1:n_input_cols
            if sparse_col(col) && i > 1
                fields{col} = '';
            else
                fields{col} = sprintf('%g', frame(i, col));
            end
        end
        fprintf(fout, '%s\n', strjoin(fields, ','));
    end

    % 纵向结果摘要
    for j = 1:length(result_cols)
        name  = result_cols{j};
        val   = r.(name);
        if isnumeric(val) && ~isnan(val)
            val_str = sprintf('%.4g', val);
        elseif ischar(val) || isstring(val)
            val_str = char(val);
        else
            val_str = '';
        end
        fprintf(fout, '%s,%s\n', name, val_str);
    end

    fprintf(fout, '\n');  % 帧间空行
end

fclose(fout);
fprintf('\n写入: %s\n', out_path);
end

%% =========== 以下为每帧独立版 calc_* 函数 (去掉 fprintf) ===========

function timing = calc_timing_mini(raw, cal)
    t = raw.t; CNT = raw.CNT; CMP = raw.CMP;
    t_per_cnt = raw.meta.t_per_cnt_us;
    min_pulse = cal.MIN_PULSE_US;

    CMP_UON = CMP(:,1); CMP_UOFF = CMP(:,2);
    CMP_LON = CMP(:,3); CMP_LOFF = CMP(:,4);

    T_hrtim_us = raw.meta.HRTIM_period_s * 1e6;
    if T_hrtim_us <= 0
        T_hrtim_us = raw.meta.CNT_range * t_per_cnt;
    end

    T_UON_us = max(0, (CMP_UOFF - CMP_UON) * t_per_cnt);
    T_LON_us = max(0, (CMP_LOFF - CMP_LON) * t_per_cnt);

    D_U_pct = T_UON_us ./ T_hrtim_us * 100;
    D_L_pct = T_LON_us ./ T_hrtim_us * 100;

    DT1_us_raw = max(0, (CMP_LON - CMP_UOFF) * t_per_cnt);
    DT2_us_raw = (CMP_UON - CMP_LOFF) * t_per_cnt;
    for i = 1:length(DT2_us_raw)
        if DT2_us_raw(i) < 0
            DT2_us_raw(i) = DT2_us_raw(i) + T_hrtim_us;
        end
    end

    D_std = std(D_U_pct);
    mean_D = mean(D_U_pct);

    if D_std < 2 && mean_D > 45
        ctrl_mode = 'FM';
    else
        ctrl_mode = 'PWM';
    end

    timing.CMP_UON  = CMP_UON;   timing.CMP_UOFF = CMP_UOFF;
    timing.CMP_LON  = CMP_LON;   timing.CMP_LOFF = CMP_LOFF;
    timing.T_UON_us = T_UON_us;  timing.T_LON_us = T_LON_us;
    timing.T_hrtim_us = T_hrtim_us;
    timing.f_sw_kHz   = 1 / (T_hrtim_us * 1e-6) / 1e3;
    timing.D_U_pct  = D_U_pct;   timing.D_L_pct = D_L_pct;
    timing.DT1_us   = DT1_us_raw; timing.DT2_us = DT2_us_raw;
    timing.D_std    = D_std;     timing.ctrl_mode = ctrl_mode;
end

function waveform = calc_waveform_mini(raw, timing)
    I = raw.I;
    CNT = raw.CNT;
    CMP_UOFF = timing.CMP_UOFF(1);
    edge = raw.meta.edge_skip;
    n = length(I);

    if n > 2*edge
        I_trim = I((edge+1):(n-edge));
    else
        I_trim = I;
    end

    I_peak_all = fit_sine_peak(I, CNT, CMP_UOFF, edge);
    I_RMS_all  = rms(I_trim);
    I_avg_all  = mean(I_trim);

    waveform.I_peak_all = I_peak_all;
    waveform.I_RMS_all  = I_RMS_all;
    waveform.I_avg_all  = I_avg_all;
    waveform.f_res_mean_khz = timing.f_sw_kHz;
end

function phase_info = calc_phase_mini(raw, timing)
    I = raw.I; CNT = raw.CNT;

    if timing.CMP_UON(1) == 0
        phase_info.phi_deg = NaN; phase_info.cos_phi = NaN;
        phase_info.usable = false; return;
    end

    CMP_UON = timing.CMP_UON(1);
    T_sw_us = timing.T_hrtim_us;
    I_peak_est = robust_peak(I, raw.meta.edge_skip);
    n = length(I);
    cnt_per_cycle = T_sw_us / raw.meta.t_per_cnt_us;

    %% 1) 谷值检测: 局部极小 + 低于峰值15% (跳过保护带)
    edge = raw.meta.edge_skip;
    valley_idx = [];
    for i = (edge+1):(n-edge)
        if I(i-1) > I(i) && I(i) < I(i+1) && I(i) < I_peak_est * 0.15
            valley_idx(end+1) = i;
        end
    end

    %% 2) 从谷值计算候选相位 (含抛物线插值, 突破0.5μs采样分辨率)
    phi_valley = compute_phi_from_indices(valley_idx, I, CNT, CMP_UON, cnt_per_cycle);

    %% 3) 线性外推备用: 用上升沿线性区外推 I=0 点
    phi_extrap = extrapolate_zero_crossings(I, CNT, I_peak_est, ...
                                            CMP_UON, cnt_per_cycle);

    %% 4) 交叉验证 & 选举
    all_phi = [phi_valley, phi_extrap];
    if isempty(all_phi)
        phase_info.phi_deg = NaN; phase_info.cos_phi = NaN;
        phase_info.usable = false; return;
    end

    % 正相位 (感性), 中位数
    phi_pos = all_phi(all_phi > 0);
    if ~isempty(phi_pos)
        phi_avg = median(phi_pos);
    else
        phi_avg = median(all_phi);
    end

    phase_info.phi_deg = phi_avg;
    phase_info.cos_phi = cosd(phi_avg);
    phase_info.usable  = true;
end

function phi_list = compute_phi_from_indices(indices, I, CNT, CMP_UON, cnt_per_cycle)
% 谷值相位检测, 带抛物线插值突破0.5μs采样分辨率
% 对每个谷值 vi 拟合抛物线通过 (vi-1, vi, vi+1) 三点,
% 求子采样最小点位置, 线性插值 CNT 到该位置
% 若底部被二极管交越失真削平 (分母≈0), 退回原始采样点
    phi_list = [];
    for k = 1:length(indices)
        vi = indices(k);
        % 抛物线插值: 求子采样最小点
        cnt_interp = CNT(vi);  % 默认回退值
        if vi > 1 && vi < length(I)
            y_lo = I(vi-1); y_mid = I(vi); y_hi = I(vi+1);
            denom = y_lo + y_hi - 2 * y_mid;
            if denom > 0.01  % 谷底有足够曲率 (非削平), 做插值
                frac = (y_hi - y_lo) / (2 * denom);
                frac = max(-0.5, min(0.5, frac));
                if frac >= 0
                    cnt_interp = CNT(vi) + frac * (CNT(vi+1) - CNT(vi));
                else
                    cnt_interp = CNT(vi) + frac * (CNT(vi) - CNT(vi-1));
                end
            end
        end
        dist_cnt = cnt_interp - CMP_UON;
        if dist_cnt < 0
            dist_cnt = dist_cnt + cnt_per_cycle;
        end
        phi_deg = (dist_cnt / cnt_per_cycle) * 360;
        phi_deg = mod(phi_deg, 360);
        if phi_deg > 180
            phi_deg = phi_deg - 360;
        end
        if phi_deg >= 8 && phi_deg <= 85
            phi_list(end+1) = phi_deg;
        end
    end
end

function phi_list = extrapolate_zero_crossings(I, CNT, I_peak, ...
                                               CMP_UON, cnt_per_cycle)
% 线性外推过零点: 当电流底部被整流失真切削时,
% 取上升沿 15%-50% 线性区的点外推回 I=0 的位置
    phi_list = [];
    if I_peak < 1.0
        return;  % 信号太弱, 不可靠
    end

    n = length(I);
    I_lo = I_peak * 0.15;
    I_hi = I_peak * 0.50;

    % 检测上升沿: I 从前一点低于 5% 到当前高于 lo
    for i = 3:n
        % 上升沿起点: 前一个点刚离开底部平坦区
        if I(i-1) < I_lo && I(i) >= I_lo
            % 向前找到"刚刚开始上升"的点 (last point before clean slope)
            % 向后找 lo..hi 范围内的点用于拟合
            slope_pts = [];
            for j = i:min(n, i+10)
                if I(j) >= I_lo && I(j) <= I_hi
                    slope_pts(end+1) = j;
                end
                if I(j) > I_hi
                    break;
                end
            end

            if length(slope_pts) >= 2
                % 取首尾两点做线性外推
                p1 = slope_pts(1);
                p2 = slope_pts(end);
                dI = I(p2) - I(p1);
                if dI > 0.01
                    % CNT 线性外推到 I=0
                    cnt_zero = CNT(p1) - I(p1) * (CNT(p2) - CNT(p1)) / dI;

                    % 处理 CNT 绕回
                    if cnt_zero < 0
                        cnt_zero = cnt_zero + cnt_per_cycle;
                    end

                    dist_cnt = cnt_zero - CMP_UON;
                    if dist_cnt < 0
                        dist_cnt = dist_cnt + cnt_per_cycle;
                    end

                    phi_deg = (dist_cnt / cnt_per_cycle) * 360;
                    phi_deg = mod(phi_deg, 360);
                    if phi_deg > 180
                        phi_deg = phi_deg - 360;
                    end

                    if phi_deg >= 8 && phi_deg <= 85
                        phi_list(end+1) = phi_deg;
                    end
                end
            end
        end
    end
end
function power = calc_power_mini(raw, waveform, timing, phase_info)
% 功率计算:
%   P      = Vdc × I_at_UOFF × D/2   (简化直流侧公式, C移植友好, 无三角函数)
%   P_fund = V_fund_rms × I_RMS × cosφ (基波法, MATLAB参考)
    I = raw.I; Vdc = raw.Vdc; CNT = raw.CNT;
    D_U = timing.D_U_pct / 100;
    Vdc_mean = mean(Vdc, 'omitnan');
    I_RMS = waveform.I_RMS_all;

    %% 关断点插值 → 简化功率 (C 可直译)
    I_at_UOFF = interp_at_cnt(timing.CMP_UOFF(1), CNT, I);
    if ~isnan(I_at_UOFF) && I_at_UOFF > 0
        P = Vdc_mean * I_at_UOFF * mean(D_U) / 2;
    else
        P = NaN;
    end

    %% 基波法参考 (需 sin/cos/tan, MATLAB only)
    if phase_info.usable
        cos_phi = phase_info.cos_phi;
        D_factor = sin(pi * mean(D_U));
        V_fund_rms = Vdc_mean * (sqrt(2) / pi) * max(D_factor, 0.1);
        P_fund = V_fund_rms * I_RMS * cos_phi;
    else
        P_fund = NaN;
    end

    power.P         = P;
    power.P_fund    = P_fund;
    power.I_at_UOFF  = I_at_UOFF;
    power.Vdc_mean  = Vdc_mean;
    power.phi_deg   = phase_info.phi_deg;
    power.cos_phi   = phase_info.cos_phi;
    power.has_phase = phase_info.usable;
end

function I_interp = interp_at_cnt(target_cnt, CNT, I)
% 线性插值 I 在 target_cnt 处的值
% 按 CNT 递增方向找双侧括号点, 线性插值; 未找到则用最近邻
    I_interp = NaN;
    n = length(CNT);
    if n < 2 || isnan(target_cnt) || target_cnt == 0
        return;
    end
    for i = 1:(n-1)
        c1 = CNT(i); c2 = CNT(i+1);
        if c1 <= target_cnt && target_cnt <= c2 && c2 > c1
            frac = (target_cnt - c1) / (c2 - c1);
            I_interp = I(i) + frac * (I(i+1) - I(i));
            return;
        end
    end
    % 未找到双侧括号 (可能在绕回边界附近), 回退最近邻
    [~, idx] = min(abs(CNT - target_cnt));
    I_interp = I(idx);
end

function rlc = calc_rlc_mini(raw, waveform, timing, phase_info)
% RLC 参数从基波阻抗直接解 (不依赖峰值假设)
%   V1_RMS = (√2/π) × Vdc × sin(πD)     — 方波基波有效值
%   |Z|    = V1_RMS / I_RMS              — 阻抗模
%   R      = |Z| × cos(φ)               — 串联电阻
%   ωL−1/ωC = |Z| × sin(φ)              — 电抗
%   L      = [1/(ωC) + |Z|×sin(φ)] / ω  — 直接解出
    C_FARAD = 0.94e-6;  % 0.47uF × 2 并联
    Vdc_raw = raw.Vdc;
    f_sw = timing.f_sw_kHz * 1e3;
    omega = 2 * pi * f_sw;
    I_RMS = waveform.I_RMS_all;
    D_U = mean(timing.D_U_pct) / 100;

    if ~phase_info.usable || isnan(phase_info.phi_deg) || I_RMS < 0.1
        rlc.valid = false; rlc.L_uH = NaN; rlc.Q = NaN;
        rlc.R_ohm = NaN; rlc.f_res_kHz = NaN;
        return;
    end

    phi = phase_info.phi_deg;
    Vdc_mean = mean(Vdc_raw, 'omitnan');

    % 基波电压有效值 (半桥方波)
    V1_RMS = (sqrt(2) / pi) * Vdc_mean * sin(pi * D_U);
    if V1_RMS < 0.1
        rlc.valid = false; rlc.L_uH = NaN; rlc.Q = NaN;
        rlc.R_ohm = NaN; rlc.f_res_kHz = NaN;
        return;
    end

    % 串联 RLC 阻抗
    Z_mag = V1_RMS / I_RMS;
    R = Z_mag * cosd(phi);
    X = Z_mag * sind(phi);          % ωL - 1/(ωC)

    % L 从电抗直接解
    L_val = (1/(omega * C_FARAD) + X) / omega;
    if L_val <= 0
        rlc.valid = false; rlc.L_uH = NaN; rlc.Q = NaN;
        rlc.R_ohm = NaN; rlc.f_res_kHz = NaN;
        return;
    end

    f_res = 1 / (2 * pi * sqrt(L_val * C_FARAD));
    omega_0 = 2 * pi * f_res;

    if R > 0
        Q_val = omega_0 * L_val / R;
    else
        Q_val = NaN;
    end

    rlc.L_uH  = L_val * 1e6;
    rlc.f_res_kHz = f_res / 1e3;
    rlc.Q     = Q_val;
    rlc.R_ohm = R;
    rlc.valid = true;
end

function pk = robust_peak(I, edge_skip)
% 鲁棒峰值: 排除前后 edge_skip 个保护带样本, 取 98% 分位数防单点离群
    if nargin < 2, edge_skip = 0; end
    n = length(I);
    if n <= 2*edge_skip
        pk = max(I); return;
    end
    I_trim = I((edge_skip+1):(n-edge_skip));
    I_sort = sort(I_trim);
    idx = max(1, round(length(I_sort) * 0.98));
    pk = I_sort(idx);
end

function pk = fit_sine_peak(I, CNT, CMP_UOFF, edge_skip)
% 窗口内正弦拟合峰值: 只取 [CNT起点, CMP_UOFF] 区间,
% 对每个谐振半周期做正弦拟合, 取拟合幅值作为 I_peak
%
% 原理:
%   1) CNT 从低到高扫描, 排除 FMAC/DMA 过渡区和 IGBT 关断后噪声
%   2) 谷值法找周期边界
%   3) 对每个半周期峰值附近做 I = A*sin(ωt) 拟合
%   4) 用拟合幅值 A 作为该周期 I_peak, 取中位数输出

    n = length(I);

    %% 1) 确定有效窗口: [第一谷值, CMP_UOFF]
    % idx_min(CNT绕回点)→第一谷值之间是上一周期关断噪音, 排除
    % CMP_UOFF 关断后可能有开关噪音, 排除
    n = length(I);

    % 先以 [idx_min, CMP_UOFF] 做初始窗口找谷值
    [CNT_min, idx_min] = min(CNT);
    [~, idx_off] = min(abs(CNT - CMP_UOFF));
    win_end = min(idx_off, n - edge_skip);

    scope_start = idx_min;
    scope_end   = win_end;
    if scope_end <= scope_start + 10
        pk = robust_peak(I, edge_skip);
        return;
    end

    % 初始窗口内找谷值
    I_scope = I(scope_start:scope_end);
    I_pk_est_scope = max(I_scope);
    first_valley = [];
    for i = 2:(scope_end - scope_start)
        if I_scope(i-1) > I_scope(i) && I_scope(i) < I_scope(i+1) && I_scope(i) < I_pk_est_scope * 0.15
            first_valley = i;
            break;
        end
    end

    % 窗口起点 = 第一个谷值 (排除关断噪音)
    if isempty(first_valley)
        win_start = idx_min;
    else
        win_start = scope_start + first_valley - 1;
    end

    if win_end <= win_start + 5
        pk = robust_peak(I, edge_skip);
        return;
    end

    I_win = I(win_start:win_end);
    n_win = length(I_win);

    %% 2) 谷值检测 (窗口内)
    I_pk_est = max(I_win);
    valley = [];
    for i = 2:(n_win-1)
        if I_win(i-1) > I_win(i) && I_win(i) < I_win(i+1) && I_win(i) < I_pk_est * 0.15
            valley(end+1) = i;
        end
    end

    if length(valley) < 2
        pk = robust_peak(I, edge_skip);
        return;
    end

    %% 3) 逐半周期正弦拟合
    n_cycles = length(valley) - 1;
    A_vals = zeros(n_cycles, 1);
    residual_vals = zeros(n_cycles, 1);

    for c = 1:n_cycles
        seg_start = valley(c);
        seg_end   = valley(c+1);
        seg_len   = seg_end - seg_start;
        if seg_len < 6
            continue;
        end

        Is = I_win(seg_start:seg_end);
        ts = (1:seg_len)';

        % 已知频率 (谷间期 ≈ 半周期, 粗略估算)
        T_half = seg_len;
        omega_est = pi / T_half;

        % 只取峰值附近 30%-100% 段做拟合 (避开底部噪声)
        I_max_s = max(Is);
        fit_region = Is > I_max_s * 0.30;
        if sum(fit_region) < 4
            continue;
        end

        Is_fit = Is(fit_region);
        ts_fit = ts(fit_region);

        % 最小二乘拟合: I = A*sin(ωt)  (忽略 DC, 因为整流后全正)
        % 简化: 用峰值附近幅度估计 A = max(Is_fit) 并用 sin 校正
        % 更准: A = mean(Is_fit ./ sin(omega_est * ts_fit))
        sin_ref = sin(omega_est * ts_fit);
        sin_ref(abs(sin_ref) < 0.1) = 0.1;  % 防除零
        A_candidate = mean(Is_fit ./ sin_ref);
        A_candidate = max(A_candidate, I_max_s * 0.5);
        A_candidate = min(A_candidate, I_max_s * 2.0);

        % 残差
        I_pred = A_candidate * sin(omega_est * ts);
        res = rms(Is - I_pred) / max(Is);
        residual_vals(c) = res;

        % 只保留拟合好的周期 (残差 < 30%)
        if res < 0.30
            A_vals(c) = A_candidate;
        end
    end

    %% 4) 中位数输出
    A_valid = A_vals(A_vals > 0);
    if isempty(A_valid)
        pk = robust_peak(I, edge_skip);
    else
        pk = median(A_valid);
    end
end
