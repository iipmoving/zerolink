%% analyze_pot_detection.m — 锅具检测脉冲分析：有效脉冲数 + 自由振荡频率
%
% 数据格式: Index, para1(原始ADC), para2(滤波)
%   采样 1MHz (1μs/点), 每组300点 = 300μs
%   空行分割段, 连续段按 Index 合并成完整分组
%
% 方法:
%   1. 自由振荡频率 → 原始信号峰值间隔法 (峰-峰周期)
%   2. 有效脉冲数 → 过阈值振荡峰值计数

clear; clc; close all;
fprintf('========== 锅具检测脉冲分析 ==========\n\n');

% 采样率参数 (检锅高速模式: ADC PAN->PAN, ~0.25us/点)
% 正常模式: FS_HZ=1e6, DT_US=1.0, N_PER_GRP=300
FS_HZ = 4e6;  DT_US = 0.25;
N_PER_GRP = 2000;

%% 加载
CSV_PATH = 'pot_200w201.csv';
fprintf('[加载] %s\n', CSV_PATH);
[~, basename, ~] = fileparts(CSV_PATH);

if ~exist(CSV_PATH, 'file')
    error('文件不存在: %s', CSV_PATH);
end

fid = fopen(CSV_PATH, 'r');
raw = textscan(fid, '%s', 'Delimiter', '\n'); raw = raw{1}; fclose(fid);

% 过滤: 只保留数字开头的行
idx_all = []; p1_all = [];
for i = 1:length(raw)
    line = strtrim(raw{i});
    if isempty(line) || ~isstrprop(line(1), 'digit'), continue; end
    parts = strsplit(line, ',');
    if length(parts) < 2, continue; end
    idx = str2double(parts{1}); p1 = str2double(parts{2});
    if isnan(idx) || isnan(p1), continue; end
    idx_all(end+1) = idx; p1_all(end+1) = p1;
end
fprintf('[加载] %d 行有效数据\n', length(idx_all));

%% 合并分段为完整 0→299 分组
% 空行/非数字行导致分段, 需要把连续序号段合并
% 找到所有 Index=0 的起点
group_starts = find(idx_all == 0);
groups = {};
for s = 1:length(group_starts)
    start_pos = group_starts(s);
    % 从 Index=0 开始, 持续收集直到 Index 到 299 或下一个 Index=0
    end_pos = start_pos;
    while end_pos < length(idx_all)
        if idx_all(end_pos) == 299
            break;
        end
        % 检查下一段是否连续
        if end_pos + 1 <= length(idx_all)
            next_idx = idx_all(end_pos + 1);
            % 序号跳跃或归零 → 停止
            if next_idx <= idx_all(end_pos) || next_idx > idx_all(end_pos) + 1
                break;
            end
        end
        end_pos = end_pos + 1;
    end
    % 提取并检查长度
    seg = start_pos:end_pos;
    if length(seg) >= N_PER_GRP && idx_all(end_pos) == 299
        groups{end+1} = double(p1_all(seg(1:N_PER_GRP))); %#ok<AGROW>
    end
end

n_full = length(groups);
fprintf('[提取] 完整分组 (300点): %d 组\n\n', n_full);
t_us = (0:N_PER_GRP-1)' * DT_US;

%% 逐组分析
results = [];

for g = 1:n_full
    p1 = groups{g}(:);

    %% A. 激励脉冲峰值
    [pk_val, pk_idx] = max(p1);
    pk_idx = pk_idx(1);

    %% B. 峰值检测 (抛物线插值，亚采样点精度)
    % 先找整数峰值，再用三点抛物线插值得到亚采样点位置
    max_idx = [];     % 整数索引
    max_val = [];     % 插值后的精确峰值
    max_pos = [];     % 插值后的精确位置(μs)
    last_peak = -10;
    for k = pk_idx+2:length(p1)-2
        if p1(k) > p1(k-1) && p1(k) > p1(k+1) && (k - last_peak) >= 8
            % 三点抛物线插值: 通过 (k-1, p1[k-1]), (k, p1[k]), (k+1, p1[k+1])
            y0 = double(p1(k-1)); y1 = double(p1(k)); y2 = double(p1(k+1));
            denom = 2 * (y0 - 2*y1 + y2);
            if abs(denom) > 1e-9
                delta = (y0 - y2) / denom;       % 亚像素偏移量 [-0.5, 0.5]
                peak_pos = (k + delta) * DT_US;  % 精确位置 (μs)
                peak_val = y1 - 0.25 * (y0 - y2) * delta;  % 精确峰值
            else
                peak_pos = k * DT_US;
                peak_val = y1;
            end
            max_idx(end+1) = k;
            max_val(end+1) = peak_val;
            max_pos(end+1) = peak_pos;
            last_peak = k;
        end
    end

    %% C. 频率: 相邻峰值间隔 (亚采样精度)
    if length(max_pos) >= 3
        periods = diff(max_pos);                    % 周期 (μs, 亚采样精度)
        T_avg = mean(periods(1:min(10, end)));
        f_khz = 1000 / T_avg;
    else
        f_khz = NaN;
    end

    %% D. 有效脉冲数 — 严格递减计数
    % 第一个峰是激励脉冲, 从第二个峰(第一振荡峰)开始
    % 每次必须严格递减, 不递减即停止
    if length(max_val) >= 2
        osc_vals = max_val(2:end);
        valid_n = 1;
        for ii = 2:length(osc_vals)
            if osc_vals(ii) < osc_vals(ii-1)
                valid_n = valid_n + 1;
            else
                break;
            end
        end
    else
        valid_n = 0;
    end

    %% 存储
    C_EFF = 39.6e-9;  % 吸收电容36nF + 10%寄生补偿
    f_hz = f_khz * 1000;
    L_uH = 1 / (4 * pi^2 * f_hz^2 * C_EFF) * 1e6;

    results = [results; struct( ...
        'group', g, 'f_khz', f_khz, 'period_us', 1000/f_khz, ...
        'L_uH', L_uH, 'valid_n', valid_n, ...
        'n_peaks', length(max_val), 'pk_val', pk_val)]; %#ok<AGROW>

    fprintf('--- 分组 #%d ---\n', g);
    fprintf('  峰值=%d@%d  频率=%.1fkHz (周期=%.2fμs, 亚采样插值)\n', pk_val, pk_idx, f_khz, 1000/f_khz);
    fprintf('  等效L(C=39.6nF)=%.1fμH  有效脉冲(递减)=%d/%d\n', L_uH, valid_n, length(max_val)-1);
    fprintf('  前5峰(插值后): ');
    for kk = 1:min(5, length(max_val))
        fprintf('%.1f@%.2fus  ', max_val(kk), max_pos(kk));
    end
    fprintf('\n');
    fprintf('  递减链: ');
    for kk = 2:min(valid_n+1, length(max_val))
        fprintf('%.1f ', max_val(kk));
        if kk < valid_n+1, fprintf('> '); end
    end
    fprintf('\n');
end

%% 汇总
fprintf('\n========== 汇总表 ==========\n');
fprintf('%-4s %-8s %-8s %-8s %-8s\n', ...
    '组号', 'f/kHz', 'T/us', 'L(uH)', '有效脉冲');
fprintf(repmat('-',42,1)); fprintf('\n');
for g = 1:n_full
    fprintf('%-4d %-8.1f %-8.1f %-8.1f %-8d\n', ...
        g, results(g).f_khz, results(g).period_us, ...
        results(g).L_uH, results(g).valid_n);
end

fprintf('\n--- 统计 ---\n');
fk = [results.f_khz];
vn = [results.valid_n];
Lc = [results.L_uH];
fprintf('振荡频率: %.1f~%.1f kHz (中位 %.1f)\n', min(fk), max(fk), median(fk));
fprintf('等效L(补偿): %.1f~%.1f uH (中位 %.1f)\n', min(Lc), max(Lc), median(Lc));
fprintf('有效脉冲(递减): %d~%d (中位 %d)\n', min(vn), max(vn), median(vn));

%% 可视化 — 所有8组
figure('Name', '检锅脉冲分析', 'Position', [50, 50, 1600, 900]);
tl = tiledlayout(4, 2, 'Padding', 'compact');
title(tl, sprintf('检锅脉冲时域波形 — %s', basename));

for g = 1:n_full
    p1 = groups{g};
    ax = nexttile;
    plot(t_us, p1, 'b-', 'LineWidth', 1.0); hold on;

    [pk_val, pk_i] = max(p1);
    max_idx = []; max_val = []; last = -10;
    for k = pk_i+2:length(p1)-2
        if p1(k) > p1(k-1) && p1(k) > p1(k+1) && (k - last) >= 8
            max_idx(end+1) = k; max_val(end+1) = p1(k); last = k;
        end
    end

    % 标注有效递减峰(红色)
    valid_cnt = results(g).valid_n;
    if ~isempty(max_val)
        scatter(t_us(max_idx(2:1+valid_cnt)), max_val(2:1+valid_cnt), 20, 'r', 'filled');
        % 标注无效峰(灰色)
        if 2+valid_cnt <= length(max_idx)
            scatter(t_us(max_idx(2+valid_cnt:end)), max_val(2+valid_cnt:end), 12, [0.7 0.7 0.7], 'x');
        end
    end
    hold off;
    xlabel('us'); ylabel('ADC'); grid on;
    title(sprintf('G%d: f=%.0fkHz L=%.0fuH 有效=%d', ...
        g, results(g).f_khz, results(g).L_uH, valid_cnt));
    xlim([0 300]);
end

%% 保存
out_csv = sprintf('pot_detect_%s.csv', basename);
T = struct2table(results);
writetable(T, out_csv);

out_txt = sprintf('pot_detect_%s_report.txt', basename);
fid = fopen(out_txt, 'w');
fprintf(fid, '锅具检测脉冲分析报告\n');
fprintf(fid, '%s\n\n', repmat('=', 50, 1));
fprintf(fid, '文件: %s  采样: 1MHz  每组: 300点=300us\n', basename);
fprintf(fid, '方法: 原始信号峰值间隔法 (无滤波, 最小间隔8us)\n');
fprintf(fid, '有效脉冲: 严格递减计数 (非递减即停止)\n');
fprintf(fid, '等效L: C=39.6nF(吸收电容36nF+10%%寄生)\n\n');
fprintf(fid, '%-4s %-8s %-8s %-8s %-8s\n', ...
    '组号', 'f/kHz', 'T/us', 'L(uH)', '有效脉冲');
fprintf(fid, '%s\n', repmat('-',42,1));
for g = 1:n_full
    fprintf(fid, '%-4d %-8.1f %-8.1f %-8.1f %-8d\n', ...
        g, results(g).f_khz, results(g).period_us, ...
        results(g).L_uH, results(g).valid_n);
end
fprintf(fid, '\n--- 统计 ---\n');
fprintf(fid, '振荡频率: %.1f~%.1f kHz\n', min(fk), max(fk));
fprintf(fid, '等效L: %.1f~%.1f uH\n', min(Lc), max(Lc));
fprintf(fid, '有效脉冲: %d~%d\n', min(vn), max(vn));
fclose(fid);
fprintf('报告已保存: %s\n', out_txt);
fprintf('\n========== 完成 ==========\n');