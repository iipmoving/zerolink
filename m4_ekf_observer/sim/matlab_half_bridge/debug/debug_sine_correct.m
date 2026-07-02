% debug_sine_correct.m — 峰顶3-4点正弦修正, 抗尖峰干扰
function debug_sine_correct()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
base = '../../tools/ekf_tuner/captures/captures/';

files = {
    'capture_20260602_152037.csv', ...  % steel
    'capture_20260602_153316.csv', ...  % iron
};

for fi = 1:length(files)
    csv_path = [base files{fi}];
    [frames, fN] = parse_csv_frames(csv_path);
    fprintf('\n========== %s (%d frames) ==========\n', files{fi}, length(frames));

    for f = 1:length(frames)
        r = correct_one_frame(frames{f}, fN(f), cal);
        if ~isnan(r.Ipk_raw)
            action = '';
            if r.n_fixed > 0
                action = sprintf('  FIXED %d pts', r.n_fixed);
            end
            fprintf('  F%2d: Ipk raw=%.2f  corr=%.2f  delta=%.2fA  (max_I=%dADC)%s\n', ...
                f, r.Ipk_raw, r.Ipk_corr, r.Ipk_corr - r.Ipk_raw, r.max_adc, action);
        end
    end
end
end

function r = correct_one_frame(frame, N, cal)
I_adc = frame(:,2); CNT = frame(:,5);
CMP = frame(1,6:9);
I_adc = min(max(I_adc,0), 4095);
I = I_adc * cal.I_SCALE;
CU = CMP(1); CO = CMP(2);

% 去尾部饱和
seg_end = N;
while seg_end > 1 && I_adc(seg_end) > 4090, seg_end = seg_end - 1; end
if seg_end > 1, seg_end = seg_end - 1; end
I_clean = I(1:seg_end);
CNT_clean = CNT(1:seg_end);
t_clean = frame(1:seg_end, 1);
n_clean = seg_end;

% 频率
dCNT = diff(CNT_clean); dCNT_pos = dCNT(dCNT>0);
if isempty(dCNT_pos), r = nan_result(); return; end
avg_dcnt = mean(dCNT_pos);
dtv = diff(t_clean); dtv(dtv<=0)=[];
avg_dt_us = mean(dtv);
t_per_cnt_us = avg_dt_us / avg_dcnt;
CNT_full = CNT(1:seg_end);
wi = find(diff(CNT_full) < 0, 1);
if ~isempty(wi)
    CNT_range = CNT_full(wi) + round(avg_dcnt) - CNT_full(wi+1);
else
    CNT_range = max(CNT_full) - min(CNT_full);
end
HRTIM_period_s = CNT_range * t_per_cnt_us * 1e-6;
f_sw = 1 / HRTIM_period_s;
omega = 2 * pi * f_sw;

% 导通区间 CU→CO
cond_mask = false(n_clean, 1);
for i = 1:n_clean
    cnt = CNT_clean(i);
    if CU < CO
        cond_mask(i) = (cnt >= CU && cnt <= CO);
    else
        cond_mask(i) = (cnt >= CU || cnt <= CO);
    end
end
cond_idx = find(cond_mask);
if length(cond_idx) < 6, r = nan_result(); return; end

I_cond = I_clean(cond_idx);
t_cond = t_clean(cond_idx);
n_cond = length(I_cond);

% Ipk_raw = max (导通区间内裸峰值)
[r.Ipk_raw, raw_peak_rel] = max(I_cond);
r.max_adc = I_adc(cond_idx(raw_peak_rel));

% ==== 峰顶正弦修正: 取峰值附近4点, 拟合正弦, 检测/修正离群点 ====
% 峰值邻域: raw_peak 前后各取点, 共4点
half_w = 2;  % 峰值两侧各扩展2个候选, 选4个
pk_range_start = max(1, raw_peak_rel - half_w);
pk_range_end = min(n_cond, raw_peak_rel + half_w);
pk_range = pk_range_start:pk_range_end;
n_pk = length(pk_range);

if n_pk < 4
    % 不够4点, 全用
    use_idx = pk_range;
else
    % 选包含峰值的连续4点: 峰值在中间
    if raw_peak_rel - pk_range_start < 2
        use_idx = pk_range(1:4);
    elseif pk_range_end - raw_peak_rel < 2
        use_idx = pk_range(n_pk-3:n_pk);
    else
        use_idx = (raw_peak_rel-1):(raw_peak_rel+2);
    end
end
use_idx = use_idx(use_idx >= 1 & use_idx <= n_cond);
n_use = length(use_idx);

I_fix = I_cond;
n_fixed = 0;

if n_use >= 4
    % 局部正弦拟合: I(t) = A*sin(ωt) + B*cos(ωt) + C
    t_use = t_cond(use_idx) * 1e-6;  % seconds
    t0 = t_use(1);
    dt = t_use - t0;
    X = [sin(omega * dt), cos(omega * dt), ones(n_use, 1)];
    coeff = X \ I_cond(use_idx);
    A = coeff(1); B = coeff(2); C = coeff(3);

    % 对整个峰顶区域做正弦预测 + 偏差检测
    check_range = pk_range;
    for k = 1:length(check_range)
        ci = check_range(k);
        dt_k = t_cond(ci) * 1e-6 - t0;
        I_pred = A * sin(omega * dt_k) + B * cos(omega * dt_k) + C;
        I_actual = I_cond(ci);
        err = abs(I_actual - I_pred);

        % 判定离群: 偏差 > 满量程的2% 或 > 0.5A (经验阈值)
        thresh = max(0.02 * 30, 0.5);  % 30A满量程*2% = 0.6A, 取0.5A
        if err > thresh
            I_fix(ci) = I_pred;
            n_fixed = n_fixed + 1;
        end
    end

    % 修正后的峰值
    r.Ipk_corr = max(I_fix(pk_range));

    % 拟合正弦的幅值 (作为峰值备选参考)
    r.Ipk_fit = sqrt(A^2 + B^2);
else
    r.Ipk_corr = r.Ipk_raw;
    r.Ipk_fit = r.Ipk_raw;
end

r.n_fixed = n_fixed;
end

function r = nan_result()
r.Ipk_raw = NaN; r.Ipk_corr = NaN; r.Ipk_fit = NaN;
r.max_adc = NaN; r.n_fixed = 0;
end

function [frames, fN] = parse_csv_frames(csv_path)
fid = fopen(csv_path,'r');
header = fgetl(fid);
ncol = length(strsplit(header,','));
frames = {}; fN = [];
while ~feof(fid)
    line = fgetl(fid);
    if ~ischar(line)||isempty(strtrim(line)), continue; end
    if startsWith(line,'SIZE')
        parts = strsplit(line,','); N = str2double(parts{2}); fN(end+1)=N;
        fr = zeros(N,ncol);
        for i=1:N
            dl = fgetl(fid); dl = strtrim(dl);
            if isempty(dl), i=i-1; continue; end
            parts = strsplit(dl,',');
            for j=1:min(length(parts),ncol)
                v = str2double(parts{j}); if ~isnan(v), fr(i,j)=v; end
            end
        end
        frames{end+1} = fr;
    end
end
fclose(fid);
end
