% debug_didt_diag — 诊断 di/dt 法失败帧的波形特征
function debug_didt_diag()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;
C_FARAD = 0.94e-6;

base = '../../tools/ekf_tuner/captures/captures/';

% 手动挑几个失败帧和成功帧对比
test_cases = {
    % file, frame_idx
    {'capture_20260602_153307.csv', 1}   % N_lin=0
    {'capture_20260602_153307.csv', 6}   % N_lin=1 L=121.6
    {'capture_20260602_153326.csv', 1}   % N_lin=7 L=82.5 (成功对比)
};

for tc = 1:length(test_cases)
    fname = test_cases{tc}{1};
    fidx  = test_cases{tc}{2};
    csv_path = [base fname];
    [fd, fN] = parse_csv(csv_path);
    frame = fd{fidx};
    N = fN{fidx};

    t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
    CMP=frame(1,6:9);
    I_adc = min(max(I_adc, 0), 4095);
    Vdc_adc = min(max(Vdc_adc, 0), 4095);
    I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.VDC_SCALE;
    CU=CMP(1); CO=CMP(2);

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
    I_clean = I(seg_start:seg_end);
    t_clean = t_us(seg_start:seg_end);
    n = length(I_clean);

    [I_max, idx_max] = max(I_clean);

    % 山谷检测
    idx_valley = 1;
    for k = idx_max:-1:2
        if I_clean(k) < I_max * 0.15
            idx_valley = k;
            break;
        end
    end
    if idx_valley == 1 && I_clean(1) > I_max * 0.15
        [~, idx_valley] = min(I_clean(1:idx_max));
    end
    I_valley = I_clean(idx_valley);

    delta_I = I_max - I_valley;
    I_lo = I_valley + delta_I * 0.30;
    I_hi = I_valley + delta_I * 0.70;

    fprintf('\n========== %s frame %d ==========\n', fname, fidx);
    fprintf('n=%d  idx_max=%d  idx_valley=%d  I_valley=%.2fA (%.0f%% I_max)\n', ...
        n, idx_max, idx_valley, I_valley, I_valley/I_max*100);
    fprintf('I_max=%.1fA  I_lo=%.1fA  I_hi=%.1fA\n', I_max, I_lo, I_hi);

    % 上升沿区间
    rise_I = I_clean(idx_valley:idx_max);
    rise_t = t_clean(idx_valley:idx_max);
    fprintf('Rising edge: %d samples, I: %.2f→%.2f, dt=%.1fus\n', ...
        length(rise_I), rise_I(1), rise_I(end), rise_t(end)-rise_t(1));

    n_lo = sum(rise_I >= I_lo & rise_I <= I_hi);
    fprintf('In [I_lo, I_hi]: %d points\n', n_lo);

    % 画波形
    figure('Position',[100 100 900 400]);
    plot(t_clean - t_clean(1), I_clean, 'b-', 'LineWidth',1.2); hold on;
    % 标记 idx_max
    plot(t_clean(idx_max)-t_clean(1), I_max, 'r^', 'MarkerSize',10, 'MarkerFaceColor','r');
    % 标记 idx_valley
    plot(t_clean(idx_valley)-t_clean(1), I_valley, 'gv', 'MarkerSize',8, 'MarkerFaceColor','g');
    % 标记 I_lo, I_hi
    yline(I_lo, 'k--', 'I_{lo} 30%');
    yline(I_hi, 'k--', 'I_{hi} 70%');
    % 上升沿高亮
    plot(rise_t - t_clean(1), rise_I, 'm-', 'LineWidth',2.5);
    xlabel('t (us)'); ylabel('I (A)');
    title(sprintf('%s f%d: n=%d, N_{lin}=%d, I_{max}=%.1fA, I_{valley}=%.2fA', fname, fidx, n, n_lo, I_max, I_valley));
    grid on;
end
end

function [frames_data, frames_N] = parse_csv(csv_path)
    fid=fopen(csv_path,'r');
    header_line=fgetl(fid);
    ncol=length(strsplit(header_line,','));
    frames_data={}; frames_N={};
    while ~feof(fid)
        line=fgetl(fid); if line==-1, break; end
        line=strtrim(line); if isempty(line), continue; end
        if startsWith(line,'SIZE')
            parts=strsplit(line,','); N=str2double(parts{2}); frames_N{end+1}=N;
            fr=zeros(N,ncol);
            for i=1:N
                dl=fgetl(fid); dl=strtrim(dl);
                if isempty(dl), i=i-1; continue; end
                parts=strsplit(dl,',');
                for j=1:min(length(parts),ncol)
                    v=str2double(parts{j}); if ~isnan(v), fr(i,j)=v; end
                end
            end
            frames_data{end+1}=fr;
        end
    end
    fclose(fid);
end
