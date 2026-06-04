% compare_peak_methods.m — 三方案同帧对比: I_peak → L
function compare_peak_methods()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;
C_FARAD = 0.90e-6;
base = '../../tools/ekf_tuner/captures/captures/';

steel_files = {'capture_20260602_152037.csv'};
iron_files = {
    'capture_20260602_153316.csv', ...
    'capture_20260602_153326.csv', ...
    'capture_20260602_153338.csv', ...
    'capture_20260602_153347.csv', ...
    'capture_20260602_153424.csv'};

all_results = [];

for pot = 1:2
    if pot == 1
        flist = steel_files; label = 'STEEL';
    else
        flist = iron_files; label = 'IRON';
    end

    for fi = 1:length(flist)
        csv_path = [base flist{fi}];
        [frames, fN] = parse_csv_frames(csv_path);

        for f = 1:length(frames)
            r = process_one_frame(frames{f}, fN(f), cal, C_FARAD);
            if ~isnan(r.phi_deg) && r.Ipk_m1 > 1.0
                r.file = flist{fi};
                r.pot = label;
                all_results = [all_results; r];
            end
        end
    end
end

if isempty(all_results)
    fprintf('No valid frames.\n'); return;
end

%% 按文件统计 L CV
fprintf('========== 三方案 I_peak → L 对比 ==========\n');
fprintf('%-40s %5s %8s %8s %8s %8s %8s %8s\n', ...
    'File','N', 'L_M1_CV%','L_M2_CV%','L_M3s_CV%','L_M3i_CV%','M1vsM2%','M1vsM3%');
fprintf('%s\n', repmat('-',1,105));

files = unique({all_results.file}, 'stable');
for fi = 1:length(files)
    mask = strcmp({all_results.file}, files{fi});
    rr = all_results(mask);
    n = sum(mask);
    if n < 3, continue; end
    L1 = [rr.L_uH_m1]'; L2 = [rr.L_uH_m2]'; L3 = [rr.L_uH_m3]';
    cv1 = std(L1)/mean(L1)*100;
    cv2 = std(L2)/mean(L2)*100;
    cv3 = std(L3)/mean(L3)*100;

    % same-frame delta
    delta12 = mean(abs(L2 - L1) ./ L1 * 100);
    delta13 = mean(abs(L3 - L1) ./ L1 * 100);

    fprintf('%-40s %5d %8.2f %8.2f %8.2f %8.2f %8.2f\n', ...
        files{fi}(1:min(40,end)), n, cv1, cv2, cv3, delta12, delta13);
end

%% 全局统计
fprintf('\n========== 全局统计 ==========\n');
pots = {'STEEL','IRON'};
for pi = 1:2
    mask = strcmp({all_results.pot}, pots{pi});
    rr = all_results(mask);
    if sum(mask) < 3, continue; end
    L1 = [rr.L_uH_m1]'; L2 = [rr.L_uH_m2]'; L3 = [rr.L_uH_m3]';
    fprintf('\n--- %s (N=%d) ---\n', pots{pi}, sum(mask));
    fprintf('Method 1 (98%%分位+导通区间): L=%.1f±%.1f uH  CV=%.1f%%\n', ...
        mean(L1), std(L1), std(L1)/mean(L1)*100);
    fprintf('Method 2 (正弦拟合):         L=%.1f±%.1f uH  CV=%.1f%%\n', ...
        mean(L2), std(L2), std(L2)/mean(L2)*100);
    fprintf('Method 3 (RMS/K, K=%.3f):   L=%.1f±%.1f uH  CV=%.1f%%\n', ...
        rr(1).K_used, mean(L3), std(L3), std(L3)/mean(L3)*100);

    delta12 = mean(abs(L2 - L1) ./ L1 * 100);
    delta13 = mean(abs(L3 - L1) ./ L1 * 100);
    fprintf('M1↔M2 mean delta: %.1f%%  M1↔M3 mean delta: %.1f%%\n', delta12, delta13);
end

%% 逐帧明细
fprintf('\n========== 逐帧明细 (前30帧) ==========\n');
fprintf('%-28s %4s %7s %7s %7s %7s %7s %7s %7s\n', ...
    'File','Fr','Ipk_M1','Ipk_M2','Ipk_M3','L_M1','L_M2','L_M3','K_rms');
nshow = min(30, length(all_results));
for i = 1:nshow
    r = all_results(i);
    [~,fname] = fileparts(r.file);
    fprintf('%-28s %4d %7.2f %7.2f %7.2f %7.1f %7.1f %7.1f %7.4f\n', ...
        fname(1:min(28,end)), i, r.Ipk_m1, r.Ipk_m2, r.Ipk_m3, ...
        r.L_uH_m1, r.L_uH_m2, r.L_uH_m3, r.K_actual);
end
end

function r = process_one_frame(frame, N, cal, C_FARAD)
t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
CMP=frame(1,6:9);
I_adc=min(max(I_adc,0),4095); Vdc_adc=min(max(Vdc_adc,0),4095);
I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.VDC_SCALE;
CU=CMP(1); CO=CMP(2);  % CU=上管开, CO=上管关

% 去尾部饱和
seg_end = N;
while seg_end > 1 && I_adc(seg_end) > 4090, seg_end = seg_end - 1; end
if seg_end > 1, seg_end = seg_end - 1; end
if seg_end < 20, r = nan_result(); return; end
I_clean = I(1:seg_end);
CNT_clean = CNT(1:seg_end);
t_clean = t_us(1:seg_end);
Vdc_mean = mean(Vdc(1:seg_end));
n_clean = length(I_clean);

% 频率 + CNT_range
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
Tu = HRTIM_period_s * 1e6;

% 导通区间: CNT 在 CU 到 CO 之间的采样点
% 处理 CNT 可能是递减的情况(比较器值,CNT递增)
cond_mask = false(n_clean, 1);
for i = 1:n_clean
    cnt = CNT_clean(i);
    % CNT 递增, CU < CO: 正常导通区间
    if CU < CO
        cond_mask(i) = (cnt >= CU && cnt <= CO);
    else
        % CU > CO: 跨越 CNT 绕回点
        cond_mask(i) = (cnt >= CU || cnt <= CO);
    end
end
cond_idx = find(cond_mask);

if length(cond_idx) < 6, r = nan_result(); return; end
I_cond = I_clean(cond_idx);
t_cond = t_clean(cond_idx);
n_cond = length(I_cond);

% 谷点: 导通区间内最小值
[~, valley_rel] = min(I_cond);
valley_i = cond_idx(valley_rel);

% 上升沿: 从谷点到导通区间末端(关断点附近)
rise_start = valley_i;
% 关断点: 导通区间最后一个点
[~, coff_rel] = max(CNT_clean(cond_idx));
rise_end = cond_idx(coff_rel);

if rise_end <= rise_start + 4, r = nan_result(); return; end
I_rise = I_clean(rise_start:rise_end);
t_rise = t_clean(rise_start:rise_end);
n_rise = length(I_rise);
t_rise_sec = t_rise * 1e-6;

% ==== Method 1: 98%分位 (导通区间上升沿) ====
I_rise_sort = sort(I_rise);
idx98 = max(1, round(length(I_rise_sort) * 0.98));
r.Ipk_m1 = I_rise_sort(idx98);

% ==== Method 2: 正弦拟合 (上升沿, 从谷点开始) ====
% I(t) = A*cos(ω*(t-t0)) + B*sin(ω*(t-t0)) + C
t0 = t_rise_sec(1);
dt = t_rise_sec - t0;
X = [cos(omega * dt), sin(omega * dt), ones(n_rise, 1)];
coeff = X \ I_rise(:);
A = coeff(1); B = coeff(2); C = coeff(3);
r.Ipk_m2 = sqrt(A^2 + B^2);  % 基波幅值 = 峰值(全波整流 = 基波峰值)
r.I_dc = C;  % 直流偏置 (= I_zero 等效)

% ==== Method 3: RMS / K ====
I_ac = I_rise - r.I_dc;
r.I_rms_on = rms(I_ac);
r.K_actual = r.I_rms_on / r.Ipk_m2;
% 使用实测 K_actual 中位数
r.Ipk_m3 = r.I_rms_on / 0.68;
r.K_used = 0.68;

% ==== 相位检测 (同 debug_didt_l2.m) ====
Ipk_ref = r.Ipk_m1;
vi = [];
for i = 2:(n_clean-1)
    if I_clean(i-1)>I_clean(i) && I_clean(i)<I_clean(i+1) && I_clean(i)<Ipk_ref*0.15
        vi(end+1) = i;
    end
end
phi_deg = NaN;
if ~isempty(vi)
    vi_k = vi(1); cnt_interp = CNT_clean(vi_k);
    dc = cnt_interp - CU;
    cpc = Tu / t_per_cnt_us;
    if dc < 0, dc = dc + cpc; end
    phi_deg = (dc / cpc) * 360;
    if phi_deg > 180, phi_deg = phi_deg - 360; end
end
r.phi_deg = phi_deg;

% ==== L 计算 (KVL: L = (Vdc/2 + V_C_peak) / (I_peak * omega)) ====
r.L_uH_m1 = calc_L(r.Ipk_m1, Vdc_mean, omega, C_FARAD);
r.L_uH_m2 = calc_L(r.Ipk_m2, Vdc_mean, omega, C_FARAD);
r.L_uH_m3 = calc_L(r.Ipk_m3, Vdc_mean, omega, C_FARAD);

r.f_sw_kHz = f_sw / 1e3;
r.Vdc_mean = Vdc_mean;
end

function LuH = calc_L(Ipk, Vdc_mean, omega, C_FARAD)
    V_C_peak = Ipk / (omega * C_FARAD);
    LuH = (Vdc_mean/2 + V_C_peak) / (Ipk * omega) * 1e6;
end

function r = nan_result()
r.Ipk_m1=NaN; r.Ipk_m2=NaN; r.Ipk_m3=NaN; r.I_rms_on=NaN;
r.K_actual=NaN; r.K_used=NaN;
r.L_uH_m1=NaN; r.L_uH_m2=NaN; r.L_uH_m3=NaN;
r.phi_deg=NaN; r.f_sw_kHz=NaN; r.Vdc_mean=NaN; r.I_dc=NaN;
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
