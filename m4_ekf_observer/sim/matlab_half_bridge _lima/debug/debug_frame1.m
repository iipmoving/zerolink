% debug_frame1 — 检查帧1 (phi=NaN) 的电流波形和过零检测
csv_path = '../../tools/ekf_tuner/captures/capture_20260602_104852.csv';
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3 + 6.2e3));

fid = fopen(csv_path, 'r');
fgetl(fid);  % header
frames_data = {}; frames_N = [];

while ~feof(fid)
    line = fgetl(fid);
    if line == -1, break; end
    line = strtrim(line);
    if isempty(line), continue; end
    if startsWith(line, 'SIZE')
        parts = strsplit(line, ',');
        N = str2double(parts{2});
        frames_N{end+1} = N;
        frame_rows = zeros(N, 11);
        for i = 1:N
            dline = fgetl(fid);
            dline = strtrim(dline);
            if isempty(dline), i = i - 1; continue; end
            parts = strsplit(dline, ',');
            for j = 1:min(length(parts), 11)
                val = str2double(parts{j});
                if ~isnan(val), frame_rows(i, j) = val; end
            end
        end
        frames_data{end+1} = frame_rows;
    end
end
fclose(fid);

% 检查所有帧的 Vdc 均值
fprintf('=== 各帧 Vdc 均值 ===\n');
for f = 1:length(frames_data)
    frame = frames_data{f};
    Vdc_adc = frame(:, 4);
    I_adc = frame(:, 2);
    I = I_adc * cal.I_SCALE;
    fprintf('帧%d: N=%d  Vdc_adc=%.0f  I_min=%.1fA  I_max=%.1fA  I_peak=%.1fA\n', ...
            f-1, frames_N{f}, mean(Vdc_adc), min(I), max(I), max(abs(I)));
end

% 重点检查帧1 (f=2)
f = 2;
frame = frames_data{f};
N = frames_N{f};
t_us = frame(:, 1);
I_adc = frame(:, 2);
Vdc_adc = frame(:, 4);
CNT = frame(:, 5);
I = I_adc * cal.I_SCALE;
Vdc = Vdc_adc * cal.V_SCALE;

fprintf('\n=== 帧1 详细信息 ===\n');
fprintf('Vdc_adc mean=%.0f  min=%.0f  max=%.0f\n', mean(Vdc_adc), min(Vdc_adc), max(Vdc_adc));
fprintf('Vdc mean=%.1fV  min=%.1fV  max=%.1fV\n', mean(Vdc), min(Vdc), max(Vdc));
fprintf('I   min=%.3fA  max=%.3fA  peak=%.3fA\n', min(I), max(I), max(abs(I)));

% 打印前30个采样点的 I 值, 看波形底部
fprintf('\n=== 帧1 前40点 I 值 ===\n');
for i = 1:min(40, N)
    fprintf('%2d: t=%5.1f  I=%7.3fA  Vdc_adc=%4d  CNT=%5d\n', ...
            i, t_us(i), I(i), Vdc_adc(i), CNT(i));
end

% 谷值检测 (当前方法)
I_peak_est = max(abs(I));
valley_idx = [];
for i = 2:(N-1)
    if I(i-1) > I(i) && I(i) < I(i+1) && I(i) < I_peak_est * 0.15
        valley_idx(end+1) = i;
    end
end
fprintf('\n当前谷值检测 (I<%.3fA): %d 个谷值\n', I_peak_est*0.15, length(valley_idx));
for k = 1:length(valley_idx)
    vi = valley_idx(k);
    fprintf('  谷值%d: idx=%d  I=%.3fA  t=%.1fus\n', k, vi, I(vi), t_us(vi));
end

% 底部平坦区检测: 连续多个点接近零
fprintf('\n=== 底部平坦区 (|I| < 0.3A) ===\n');
flat_start = 0;
for i = 1:N
    if abs(I(i)) < 0.3
        if flat_start == 0
            flat_start = i;
        end
    else
        if flat_start > 0 && i - flat_start >= 2
            fprintf('  平坦区: idx %d-%d  I=[%.3f,%.3f]A  t=[%.1f,%.1f]us\n', ...
                    flat_start, i-1, I(flat_start), I(i-1), t_us(flat_start), t_us(i-1));
        end
        flat_start = 0;
    end
end

% 先画帧1和帧2对比
figure('Position', [100 100 1200 500]);
subplot(1,2,1);
plot(t_us, I, 'b.-', 'MarkerSize', 4);
hold on;
if ~isempty(valley_idx)
    plot(t_us(valley_idx), I(valley_idx), 'ro', 'MarkerSize', 8);
end
yline(I_peak_est*0.15, 'r--', '15% threshold');
yline(0, 'k-');
xlabel('t (us)'); ylabel('I (A)');
title(sprintf('Frame 1 (FAIL): Vdc=%d Ipeak=%.1fA', round(mean(Vdc_adc)), max(abs(I))));
grid on;

% 对比: 帧2 (成功帧)
f_ok = 3;
frame_ok = frames_data{f_ok};
I_ok = frame_ok(:,2) * cal.I_SCALE;
t_ok = frame_ok(:,1);
subplot(1,2,2);
plot(t_ok, I_ok, 'b.-', 'MarkerSize', 4);
hold on;
val_ok = [];
Ipk_ok = max(abs(I_ok));
for i = 2:(frames_N{f_ok}-1)
    if I_ok(i-1) > I_ok(i) && I_ok(i) < I_ok(i+1) && I_ok(i) < Ipk_ok * 0.15
        val_ok(end+1) = i;
    end
end
plot(t_ok(val_ok), I_ok(val_ok), 'ro', 'MarkerSize', 8);
yline(Ipk_ok*0.15, 'r--');
yline(0, 'k-');
xlabel('t (us)'); ylabel('I (A)');
title(sprintf('Frame 2 (OK): Vdc=%d Ipeak=%.1fA', round(mean(frame_ok(:,4))), Ipk_ok));
grid on;

saveas(gcf, 'debug_frame1_vs_frame2.png');
fprintf('\n图表保存: debug_frame1_vs_frame2.png\n');
