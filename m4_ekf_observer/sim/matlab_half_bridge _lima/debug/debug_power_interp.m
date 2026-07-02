% debug_power_interp — 关断点插值: 只看电流积分
% 排除 Vdc/对称/P_W缩放干扰, 直接比较积分量
function debug_power_interp()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;

base = '../../tools/ekf_tuner/captures/captures/';
iron_files = {
    'capture_20260602_153307.csv','capture_20260602_153311.csv', ...
    'capture_20260602_153314.csv','capture_20260602_153316.csv', ...
    'capture_20260602_153326.csv','capture_20260602_153329.csv', ...
    'capture_20260602_153336.csv','capture_20260602_153338.csv', ...
    'capture_20260602_153340.csv','capture_20260602_153347.csv', ...
    'capture_20260602_153348.csv','capture_20260602_153358.csv', ...
    'capture_20260602_153400.csv','capture_20260602_153410.csv', ...
    'capture_20260602_153412.csv','capture_20260602_153413.csv', ...
    'capture_20260602_153421.csv','capture_20260602_153422.csv', ...
    'capture_20260602_153424.csv'
};

fprintf('========== 关断点插值: 电流积分对比 ==========\n');

all_I_no = []; all_I_lin = []; all_I_full = [];
all_nsamp = []; all_frac = [];

for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        [I_no, I_lin, I_full, nsamp, last_frac, valid] = ...
            calc_current_int(fd{f}, fN{f}, cal);
        if valid
            all_I_no(end+1) = I_no;
            all_I_lin(end+1) = I_lin;
            all_I_full(end+1) = I_full;
            all_nsamp(end+1) = nsamp;
            all_frac(end+1) = last_frac;
        end
    end
end

N = length(all_I_no);
fprintf('有效帧: %d\n\n', N);

fprintf('--- 积分窗口样本数分布 ---\n');
fprintf('mean=%.1f  P10=%.0f  P50=%.0f  P90=%.0f  min=%.0f  max=%.0f\n', ...
    mean(all_nsamp), prctile(all_nsamp,10), median(all_nsamp), ...
    prctile(all_nsamp,90), min(all_nsamp), max(all_nsamp));

% 短窗口 (<10 samples) vs 长窗口
short = all_nsamp < 10;
long = all_nsamp >= 10;
fprintf('短窗口(<10样): %d帧 长窗口: %d帧\n', sum(short), sum(long));

% 插值影响 = (不插值 - 插值) / 插值
d_pct_no = (all_I_no - all_I_lin) ./ all_I_lin * 100;
d_pct_full = (all_I_full - all_I_lin) ./ all_I_lin * 100;

fprintf('\n--- 插值影响 (%% of 插值积分值) ---\n');
fprintf('%-20s %8s %8s %8s %8s\n', '', 'mean%', 'P50%', 'P90%', 'max%');
fprintf('%-20s %7.1f%% %7.1f%% %7.1f%% %7.1f%%\n', ...
    '不插值偏差', mean(abs(d_pct_no)), median(abs(d_pct_no)), ...
    prctile(abs(d_pct_no),90), max(abs(d_pct_no)));
fprintf('%-20s %7.1f%% %7.1f%% %7.1f%% %7.1f%%\n', ...
    '全含偏差', mean(abs(d_pct_full)), median(abs(d_pct_full)), ...
    prctile(abs(d_pct_full),90), max(abs(d_pct_full)));

fprintf('\n--- 按样本数分档 ---\n');
fprintf('%-16s %5s %10s %10s %10s\n', '窗口样本数','N','|Δ|mean%','|Δ|max%','last_frac');
bins = [2 5 8 12 20 100];
for b = 1:(length(bins)-1)
    bm = all_nsamp >= bins(b) & all_nsamp < bins(b+1);
    if sum(bm) >= 3
        fprintf('%-16s %5d %9.1f%% %9.1f%% %9.2f\n', ...
            sprintf('%d-%d',bins(b),bins(b+1)), sum(bm), ...
            mean(abs(d_pct_no(bm))), max(abs(d_pct_no(bm))), mean(all_frac(bm)));
    end
end

fprintf('\n--- 短窗口 (Nsamp<10) 详细 ---\n');
if sum(short) >= 3
    fprintf('N=%d  |Δ|mean=%.1f%%  |Δ|max=%.1f%%  frac_mean=%.2f\n', ...
        sum(short), mean(abs(d_pct_no(short))), max(abs(d_pct_no(short))), ...
        mean(all_frac(short)));
end
fprintf('--- 长窗口 (Nsamp>=10) 详细 ---\n');
if sum(long) >= 3
    fprintf('N=%d  |Δ|mean=%.1f%%  |Δ|max=%.1f%%  frac_mean=%.2f\n', ...
        sum(long), mean(abs(d_pct_no(long))), max(abs(d_pct_no(long))), ...
        mean(all_frac(long)));
end

fprintf('\n===== 结论 =====\n');
mean_d = mean(abs(d_pct_no));
if mean_d < 0.5
    fprintf('插值影响 <0.5%%, C实现无需插值 (直接丢弃边界点)\n');
elseif mean_d < 2.0
    fprintf('插值影响 %.1f%%, 建议短窗口(<10样)插值, 长窗口忽略\n', mean_d);
else
    fprintf('插值影响 %.1f%%, 建议统一用线性插值 (成本极低: 1次乘加)\n', mean_d);
end
end

function [I_int_no, I_int_lin, I_int_full, nsamp, last_frac, valid] = ...
        calc_current_int(frame, N, cal)
    t_us=frame(:,1); I_adc=frame(:,2); CNT=frame(:,5);
    CMP=frame(1,6:9);
    I_adc = min(max(I_adc, 0), 4095);
    CO = CMP(2);  % 上管关断 CNT

    % 最长单调段
    wrap_idx = find(diff(CNT) < 0);
    breaks = [0; wrap_idx(:); N];
    best_seg = [1 N]; best_len = 0;
    for b = 1:(length(breaks)-1)
        seg_len = breaks(b+1) - breaks(b);
        if seg_len > best_len
            best_len = seg_len; best_seg = [breaks(b)+1, breaks(b+1)];
        end
    end
    seg_end = best_seg(2);
    while seg_end > best_seg(1) && I_adc(seg_end) > 4090, seg_end = seg_end - 1; end
    if seg_end > best_seg(1), seg_end = seg_end - 1; end
    seg_start = best_seg(1);

    I_adc_seg = I_adc(seg_start:seg_end);
    CNT_seg = CNT(seg_start:seg_end);
    n_seg = length(I_adc_seg);

    % 零偏
    I_zero = median(I_adc_seg);
    % 绝对值电流 (有功功率与电流方向无关, 取绝对值)
    I_abs = abs(single(I_adc_seg) - I_zero) * cal.I_SCALE;

    % CO 索引
    co_idx = n_seg;
    for i = 1:n_seg
        if CNT_seg(i) >= CO, co_idx = i; break; end
    end

    % 上管谷值 (raw ADC 局部最小)
    valley_up = 1;
    min_v = 1e9;
    for i = 2:min(co_idx, n_seg)-1
        v = I_adc_seg(i);
        if v < I_adc_seg(i-1) && v < I_adc_seg(i+1) && v < min_v
            min_v = v; valley_up = i;
        end
    end
    % 谷值找不到用段首
    if valley_up == 1 && co_idx > 5
        [~, vi] = min(I_adc_seg(1:co_idx-2));
        valley_up = max(1, vi);
    end

    % last_frac
    if co_idx > 1 && co_idx <= n_seg
        d = CNT_seg(co_idx) - CNT_seg(co_idx-1);
        if d > 0 && d < 5000
            last_frac = (double(CO) - double(CNT_seg(co_idx-1))) / double(d);
        else
            last_frac = 1.0;
        end
    else
        last_frac = 1.0;
    end
    last_frac = max(0, min(1.0, last_frac));

    nsamp = co_idx - valley_up;
    if nsamp < 2
        I_int_no=NaN; I_int_lin=NaN; I_int_full=NaN; valid=false; return;
    end

    % 方法1: 无插值 — 全计入 co_idx-1 (忽略部分性)
    I_int_no = sum(I_abs(valley_up:co_idx-1));

    % 方法2: 插值 — co_idx-1 只取 last_frac
    I_int_lin = sum(I_abs(valley_up:co_idx-2)) + I_abs(co_idx-1) * last_frac;

    % 方法3: 全含 — co_idx 也纳入
    if co_idx <= n_seg
        I_int_full = sum(I_abs(valley_up:co_idx-2)) + I_abs(co_idx-1) + I_abs(co_idx);
    else
        I_int_full = sum(I_abs(valley_up:co_idx-1));
    end

    valid = I_int_no > 0 && I_int_lin > 0;
end

function [frames_data, frames_N] = parse_csv(csv_path)
    fid = fopen(csv_path, 'r');
    header_line = fgetl(fid);
    ncol = length(strsplit(header_line, ','));
    frames_data = {}; frames_N = {};
    while ~feof(fid)
        line = fgetl(fid); if line == -1, break; end
        line = strtrim(line); if isempty(line), continue; end
        if startsWith(line, 'SIZE')
            parts = strsplit(line, ','); N_val = str2double(parts{2}); frames_N{end+1} = N_val;
            fr = zeros(N_val, ncol);
            for i = 1:N_val
                dl = fgetl(fid); dl = strtrim(dl);
                if isempty(dl), i = i - 1; continue; end
                parts = strsplit(dl, ',');
                for j = 1:min(length(parts), ncol)
                    v = str2double(parts{j}); if ~isnan(v), fr(i, j) = v; end
                end
            end
            frames_data{end+1} = fr;
        end
    end
    fclose(fid);
end
