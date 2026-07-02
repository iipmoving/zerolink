% debug_104850_f7 — 深入检查 104850 帧7: Ipk=1653 但 Vdc=103V, 为何 L/Q/phi 全崩
csv_path = '../../tools/ekf_tuner/captures/capture_20260602_104850.csv';
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3 + 6.2e3));

fid = fopen(csv_path, 'r');
fgetl(fid);
frames_data = {}; frames_N = {};
while ~feof(fid)
    line = fgetl(fid); if line == -1, break; end
    line = strtrim(line); if isempty(line), continue; end
    if startsWith(line, 'SIZE')
        parts = strsplit(line, ',');
        N = str2double(parts{2}); frames_N{end+1} = N;
        fr = zeros(N, 11);
        for i = 1:N
            dl = fgetl(fid); dl = strtrim(dl);
            if isempty(dl), i = i - 1; continue; end
            parts = strsplit(dl, ',');
            for j = 1:min(length(parts), 11)
                v = str2double(parts{j}); if ~isnan(v), fr(i,j) = v; end
            end
        end
        frames_data{end+1} = fr;
    end
end
fclose(fid);

% 帧7 (MATLAB index 8)
frame = frames_data{8};
N = frames_N{8};
t_us = frame(:,1);
I_adc = frame(:,2);
V_adc = frame(:,3);
Vdc_adc = frame(:,4);
CNT = frame(:,5);
CMP = frame(1, 6:9);
I = I_adc * cal.I_SCALE;
Vdc = Vdc_adc * cal.V_SCALE;

fprintf('=== 104850 帧7 原始数据 ===\n');
fprintf('N=%d  CMP=[%d,%d,%d,%d]\n', N, CMP(1), CMP(2), CMP(3), CMP(4));
fprintf('I_adc: min=%d  max=%d  mean=%.0f\n', min(I_adc), max(I_adc), mean(I_adc));
fprintf('I:     min=%.2fA  max=%.2fA\n', min(I), max(I));
fprintf('Vdc:   min=%.0fV  max=%.0fV  mean=%.0fV\n', min(Vdc), max(Vdc), mean(Vdc));
fprintf('V_adc: min=%d  max=%d  mean=%.0f\n', min(V_adc), max(V_adc), mean(V_adc));

% 打印全部采样点
fprintf('\n=== 全部 %d 个采样点 ===\n', N);
fprintf('Idx   t_us   I_adc   I[A]    Vdc_adc Vdc[V]  CNT\n');
for i = 1:N
    fprintf('%3d  %6.1f  %5d  %7.2f  %5d  %7.1f  %5d\n', ...
        i, t_us(i), I_adc(i), I(i), Vdc_adc(i), Vdc(i), CNT(i));
end

% 谷值检测
Ipk = max(I);
fprintf('\n=== 谷值检测 (阈值=%.2fA) ===\n', Ipk*0.15);
for i = 2:N-1
    if I(i-1) > I(i) && I(i) < I(i+1) && I(i) < Ipk * 0.15
        fprintf('  谷值 idx=%d  I=%.3fA  t=%.1fus  CNT=%d\n', i, I(i), t_us(i), CNT(i));
    end
end

% 检查电流波形是否被削顶
fprintf('\n=== ADC 饱和检查 ===\n');
sat_count = sum(I_adc >= 4090);
fprintf('I_adc >= 4090 的点数: %d/%d\n', sat_count, N);
if sat_count > 0
    fprintf('饱和点位置: ');
    for i = 1:N
        if I_adc(i) >= 4090, fprintf('%d ', i); end
    end
    fprintf('\n');
end

% 检查 I_adc 分布
fprintf('\nI_adc 值分布:\n');
edges = [0 500 1000 1500 2000 2500 3000 3500 4000 4096];
counts = histcounts(I_adc, edges);
for k = 1:length(counts)
    if counts(k) > 0
        fprintf('  [%d,%d): %d\n', edges(k), edges(k+1), counts(k));
    end
end

% 上升沿线性区检查
fprintf('\n=== 线性外推过零检测 ===\n');
I_lo = Ipk * 0.15;
I_hi = Ipk * 0.50;
fprintf('Ipk=%.2fA  I_lo=%.2fA  I_hi=%.2fA\n', Ipk, I_lo, I_hi);
for i = 3:N
    if I(i-1) < I_lo && I(i) >= I_lo
        fprintf('上升沿触发 idx=%d: I=%5.2f->%5.2fA  t=%.1fus\n', i, I(i-1), I(i), t_us(i));
        slope_pts = [];
        for j = i:min(N, i+10)
            if I(j) >= I_lo && I(j) <= I_hi
                slope_pts(end+1) = j;
            end
            if I(j) > I_hi, break; end
        end
        if length(slope_pts) >= 2
            p1 = slope_pts(1); p2 = slope_pts(end);
            fprintf('  线性区点: idx=%d..%d  I=%.2f..%.2fA\n', p1, p2, I(p1), I(p2));
            dI = I(p2) - I(p1);
            if dI > 0.01
                cnt_zero = CNT(p1) - I(p1) * (CNT(p2) - CNT(p1)) / dI;
                fprintf('  外推 CNT_zero=%.1f\n', cnt_zero);
            end
        else
            fprintf('  ★ 线性区点数不足 (%d<2)!\n', length(slope_pts));
        end
    end
end

% CNT 分析
fprintf('\n=== CNT 分析 ===\n');
dCNT = diff(CNT);
fprintf('dCNT: min=%d max=%d\n', min(dCNT), max(dCNT));
wrap = find(dCNT < 0);
fprintf('CNT 绕回点: %s\n', mat2str(wrap));
if ~isempty(wrap)
    for w = 1:length(wrap)
        wi = wrap(w);
        fprintf('  绕回%d: idx %d->%d  CNT %d->%d  t %.1f->%.1fus\n', ...
            w, wi, wi+1, CNT(wi), CNT(wi+1), t_us(wi), t_us(wi+1));
    end
end

% I_adc vs V_adc 对比 (看是否整周期采样)
fprintf('\n=== V_adc 特征 ===\n');
fprintf('V_adc 范围: %d..%d  mean=%.0f\n', min(V_adc), max(V_adc), mean(V_adc));
