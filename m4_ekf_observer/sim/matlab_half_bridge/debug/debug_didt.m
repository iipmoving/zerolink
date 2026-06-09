% debug_didt — 从波形中段提取 di/dt, 避开零交叉区和关断尖峰
function debug_didt()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;
cal.HRTIM_CLK_MHZ = 144;
C_FARAD = 0.94e-6;

base = '../../tools/ekf_tuner/captures/captures/';

% 挑选铁锅同频段(24kHz附近)不同Vdc的帧, 看波形线性段
% 低V: 153307 f=2 (Vdc=88V L=112.8)
% 中V: 153307 f=1 (Vdc=172V L=89.9)
% 高V: 153347 f=5 (Vdc=291V L=74.2)

test_files = {
    {'capture_20260602_153307.csv', 2, 'Iron 24kHz low-V  Vdc=88V'},
    {'capture_20260602_153307.csv', 1, 'Iron 24kHz mid-V  Vdc=172V'},
    {'capture_20260602_153347.csv', 5, 'Iron 24kHz high-V Vdc=291V'},
};

for t = 1:length(test_files)
    csv_path = [base test_files{t}{1}];
    target_f = test_files{t}{2};
    label = test_files{t}{3};

    [fd, fN] = parse_csv(csv_path);
    frame = fd{target_f};
    N = fN{target_f};
    t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
    CMP=frame(1,6:9);

    I_adc = min(max(I_adc, 0), 4095);
    Vdc_adc = min(max(Vdc_adc, 0), 4095);
    I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.VDC_SCALE;
    CU=CMP(1); CO=CMP(2);

    % CNT segment
    wrap_idx = find(diff(CNT) < 0);
    breaks = [0; wrap_idx(:); N];
    best_seg = [1 N]; best_len = 0;
    for b = 1:(length(breaks)-1)
        seg_len = breaks(b+1) - breaks(b);
        if seg_len > best_len
            best_len = seg_len;
            best_seg = [breaks(b)+1, breaks(b+1)];
        end
    end
    seg_end = best_seg(2);
    while seg_end > best_seg(1) && I_adc(seg_end) > 4090
        seg_end = seg_end - 1;
    end
    if seg_end > best_seg(1), seg_end = seg_end - 1; end
    seg_start = best_seg(1);

    I_clean = I(seg_start:seg_end);
    t_clean = t_us(seg_start:seg_end);
    CNT_clean = CNT(seg_start:seg_end);
    Vdc_clean = Vdc(seg_start:seg_end);
    n = length(I_clean);

    % Timing
    dCNT = diff(CNT_clean); dCNT_pos = dCNT(dCNT > 0);
    avg_dcnt = mean(dCNT_pos);
    dtv = diff(t_clean); dtv(dtv <= 0) = [];
    avg_dt_us = mean(dtv);
    t_per_cnt_us = avg_dt_us / avg_dcnt;

    seg_full_CNT = CNT(best_seg(1):best_seg(2));
    wi = find(diff(seg_full_CNT) < 0, 1);
    if ~isempty(wi)
        CNT_range = seg_full_CNT(wi) + round(avg_dcnt) - seg_full_CNT(wi+1);
    else
        CNT_range = max(seg_full_CNT) - min(seg_full_CNT);
    end
    HRTIM_period_s = CNT_range * t_per_cnt_us * 1e-6;
    f_sw = 1/HRTIM_period_s;
    Tu = HRTIM_period_s * 1e6;
    D_U = max(0, (CO-CU) * t_per_cnt_us) / Tu;

    Ipk = robust_peak(I_clean, 0);
    Vdc_mean = mean(Vdc_clean);

    % Find current rising edge: 从最小值到最大值的区间
    [I_min, idx_min] = min(I_clean);
    [I_max, idx_max] = max(I_clean);

    % 上升沿: idx_min → idx_max (如果 idx_min < idx_max) 或绕回
    % 取连续上升段
    if idx_min < idx_max
        rise_I = I_clean(idx_min:idx_max);
        rise_t = t_clean(idx_min:idx_max);
    else
        % 绕回: idx_min→end 和 1→idx_max 合并
        rise_I = [I_clean(idx_min:end); I_clean(1:idx_max)];
        rise_t = [t_clean(idx_min:end); t_clean(1:idx_max)];
    end

    fprintf('\n========== %s ==========\n', label);
    fprintf('N_clean=%d  f_sw=%.2fkHz  Ipk=%.2fA  Vdc=%.0fV  D_U=%.3f\n', ...
        n, f_sw/1e3, Ipk, Vdc_mean, D_U);
    fprintf('I_min=%.3fA@idx=%d  I_max=%.3fA@idx=%d\n', I_min, idx_min, I_max, idx_max);

    % 上升沿分三段分析
    n_rise = length(rise_I);
    fprintf('Rising edge: %d samples (%.1f%% of peak-to-peak)\n', n_rise, n_rise/n*100);

    % 避开底部15%(交越区)和顶部15%(关断尖峰区)
    % 取30%-70% 区间作为"干净线性段"
    lo_pct = 0.30; hi_pct = 0.70;
    I_lo = I_min + (I_max - I_min) * lo_pct;
    I_hi = I_min + (I_max - I_min) * hi_pct;

    lin_idx = find(rise_I >= I_lo & rise_I <= I_hi);
    if length(lin_idx) >= 3
        lin_I = rise_I(lin_idx);
        lin_t = rise_t(lin_idx);
        % Linear fit: I = a*t + b
        p = polyfit(lin_t - lin_t(1), lin_I, 1);
        didt_lin = p(1);  % A/us
        fprintf('Linear fit (30%%-70%%): di/dt = %.2f A/us  R^2=%.4f\n', ...
            didt_lin, corrcoef(lin_t-lin_t(1), lin_I).^2);
        fprintf('  Vdc/L_ideal = %.2f A/us (L=%.0fuH assumed)\n', ...
            Vdc_mean/88*1e-6, 88);
    end

    % 交越区宽度: 看底部电流上升变慢的区域
    % 从 I_min 到 I_lo, 对比理想 di/dt 和实际
    bot_idx = find(rise_I >= I_min & rise_I < I_lo);
    if length(bot_idx) >= 2
        bot_I = rise_I(bot_idx);
        bot_t = rise_t(bot_idx);
        % 实际通过时间
        dt_actual = bot_t(end) - bot_t(1);
        % 理想通过时间 (用线性段 di/dt)
        dt_ideal = (bot_I(end) - bot_I(1)) / didt_lin;
        fprintf('Bottom 0-30%%: dt_actual=%.2fus  dt_ideal=%.2fus  ratio=%.2f\n', ...
            dt_actual, dt_ideal, dt_actual/dt_ideal);
    end

    % 关断尖峰区
    top_idx = find(rise_I >= I_hi & rise_I <= I_max);
    if length(top_idx) >= 2
        top_I = rise_I(top_idx);
        top_t = rise_t(top_idx);
        dt_actual_top = top_t(end) - top_t(1);
        dt_ideal_top = (top_I(end) - top_I(1)) / didt_lin;
        fprintf('Top 70-100%%:  dt_actual=%.2fus  dt_ideal=%.2fus  ratio=%.2f\n', ...
            dt_actual_top, dt_ideal_top, dt_actual_top/dt_ideal_top);
    end

    % 保存关键数据用于批量分析
    r.Vdc = Vdc_mean; r.Ipk = Ipk; r.f_sw = f_sw;
    r.didt = didt_lin;
    r.bot_ratio = dt_actual/dt_ideal;
    r.top_ratio = dt_actual_top/dt_ideal_top;
    results(t) = r;
end

%% 批量: 所有铁锅帧的 di/dt vs Vdc
fprintf('\n\n========== Batch: di/dt analysis on ALL iron frames ==========\n');
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

all_bot_ratio = []; all_Vdc = []; all_L = []; all_Ipk = [];

for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        frame = fd{f}; N_frame = fN{f};
        t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
        CMP=frame(1,6:9);
        I_adc = min(max(I_adc, 0), 4095);
        Vdc_adc = min(max(Vdc_adc, 0), 4095);
        I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.VDC_SCALE;
        CU=CMP(1); CO=CMP(2);

        % CNT segment
        wrap_idx = find(diff(CNT) < 0);
        breaks = [0; wrap_idx(:); N_frame];
        best_seg = [1 N_frame]; best_len = 0;
        for b = 1:(length(breaks)-1)
            seg_len = breaks(b+1) - breaks(b);
            if seg_len > best_len
                best_len = seg_len; best_seg = [breaks(b)+1, breaks(b+1)];
            end
        end
        seg_end = best_seg(2);
        while seg_end > best_seg(1) && I_adc(seg_end) > 4090
            seg_end = seg_end - 1;
        end
        if seg_end > best_seg(1), seg_end = seg_end - 1; end
        seg_start = best_seg(1);
        I_clean = I(seg_start:seg_end);
        t_clean = t_us(seg_start:seg_end);
        CNT_clean = CNT(seg_start:seg_end);
        Vdc_clean = Vdc(seg_start:seg_end);

        if length(I_clean) < 20, continue; end
        Ipk = robust_peak(I_clean, 0);
        if Ipk < 2, continue; end
        Vdc_mean = mean(Vdc_clean);

        % Timing + L
        dCNT = diff(CNT_clean); dCNT_pos = dCNT(dCNT > 0);
        if isempty(dCNT_pos), continue; end
        avg_dcnt = mean(dCNT_pos);
        dtv = diff(t_clean); dtv(dtv <= 0) = [];
        avg_dt_us = mean(dtv);
        t_per_cnt_us = avg_dt_us / avg_dcnt;
        seg_full_CNT = CNT(best_seg(1):best_seg(2));
        wi = find(diff(seg_full_CNT) < 0, 1);
        if ~isempty(wi)
            CNT_range = seg_full_CNT(wi) + round(avg_dcnt) - seg_full_CNT(wi+1);
        else
            CNT_range = max(seg_full_CNT) - min(seg_full_CNT);
        end
        HRTIM_period_s = CNT_range * t_per_cnt_us * 1e-6;
        f_sw = 1/HRTIM_period_s;
        phi_val = detect_phi(I_clean, CNT_clean, CU, HRTIM_period_s*1e6, t_per_cnt_us, Ipk);

        % L
        Tu = HRTIM_period_s * 1e6;
        D_U = max(0, (CO-CU) * t_per_cnt_us) / Tu;
        omega = 2*pi*f_sw;
        I_rms = rms(I_clean);
        if ~isnan(phi_val) && I_rms > 0.1
            V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
            Z = V1/I_rms; X = Z*sind(phi_val);
            L_val = (1/(omega*C_FARAD)+X)/omega*1e6;
        else, continue; end

        % Find rising edge and linear region
        [I_min, idx_min] = min(I_clean);
        [I_max, idx_max] = max(I_clean);
        if idx_min < idx_max
            rise_I = I_clean(idx_min:idx_max);
            rise_t = t_clean(idx_min:idx_max);
        else
            rise_I = [I_clean(idx_min:end); I_clean(1:idx_max)];
            rise_t = [t_clean(idx_min:end); t_clean(1:idx_max)];
        end
        if length(rise_I) < 6, continue; end

        I_lo = I_min + (I_max - I_min) * 0.30;
        I_hi = I_min + (I_max - I_min) * 0.70;
        lin_idx = find(rise_I >= I_lo & rise_I <= I_hi);
        if length(lin_idx) < 3, continue; end
        lin_I = rise_I(lin_idx); lin_t = rise_t(lin_idx);
        p = polyfit(lin_t - lin_t(1), lin_I, 1);
        didt_lin = p(1);

        bot_idx = find(rise_I >= I_min & rise_I < I_lo);
        if length(bot_idx) >= 2
            bot_I = rise_I(bot_idx); bot_t = rise_t(bot_idx);
            dt_actual = bot_t(end) - bot_t(1);
            dt_ideal = (bot_I(end) - bot_I(1)) / didt_lin;
            bot_ratio = dt_actual / dt_ideal;
        else, continue; end

        all_bot_ratio = [all_bot_ratio; bot_ratio];
        all_Vdc = [all_Vdc; Vdc_mean];
        all_L = [all_L; L_val];
        all_Ipk = [all_Ipk; Ipk];
    end
end

fprintf('Valid frames: %d\n', length(all_Vdc));

% 交越区宽度 vs Vdc
fprintf('\n--- Bottom ratio (dt_actual/dt_ideal) vs Vdc ---\n');
% 按Vdc分档
V_edges = [70 90 110 150 200 250 300];
for b = 1:length(V_edges)-1
    mask = all_Vdc >= V_edges(b) & all_Vdc < V_edges(b+1);
    if sum(mask) >= 2
        fprintf('Vdc=%3d-%-3dV  N=%2d  bot_ratio=%.3f+-%.3f  L=%.1f+-%.1f\n', ...
            V_edges(b), V_edges(b+1), sum(mask), ...
            mean(all_bot_ratio(mask)), std(all_bot_ratio(mask)), ...
            mean(all_L(mask)), std(all_L(mask)));
    end
end

% Correlation
fprintf('\nCorrelation with Vdc:\n');
fprintf('  bot_ratio vs Vdc: r=%.3f\n', corrcoef(all_bot_ratio, all_Vdc));
fprintf('  L vs Vdc: r=%.3f\n', corrcoef(all_L, all_Vdc));
fprintf('  L vs bot_ratio: r=%.3f\n', corrcoef(all_L, all_bot_ratio));
fprintf('  bot_ratio vs Ipk: r=%.3f\n', corrcoef(all_bot_ratio, all_Ipk));

end

function phi_val = detect_phi(I, CNT, CU, Tu, t_per_cnt_us, Ipk)
    phi_val = NaN;
    cpc=Tu/t_per_cnt_us;
    nI=length(I);
    vi=[];
    for i=2:(nI-1)
        if I(i-1)>I(i)&&I(i)<I(i+1)&&I(i)<Ipk*0.15, vi(end+1)=i; end
    end
    pv=[];
    for k=1:length(vi)
        vi_k=vi(k); cnt_interp=CNT(vi_k);
        if vi_k>1 && vi_k<length(I)
            y_lo=I(vi_k-1); y_mid=I(vi_k); y_hi=I(vi_k+1);
            denom=y_lo+y_hi-2*y_mid;
            if denom>0.01
                frac=(y_hi-y_lo)/(2*denom); frac=max(-0.5,min(0.5,frac));
                if frac>=0, cnt_interp=CNT(vi_k)+frac*(CNT(vi_k+1)-CNT(vi_k));
                else, cnt_interp=CNT(vi_k)+frac*(CNT(vi_k)-CNT(vi_k-1)); end
            end
        end
        dc=cnt_interp-CU; if dc<0, dc=dc+cpc; end
        ph=(dc/cpc)*360; ph=mod(ph,360); if ph>180, ph=ph-360; end
        if ph>=8&&ph<=85, pv(end+1)=ph; end
    end
    pe=[];
    lo=Ipk*0.15; hi=Ipk*0.50;
    for i=3:nI
        if I(i-1)<lo&&I(i)>=lo
            sp=[]; for j=i:min(nI,i+10)
                if I(j)>=lo&&I(j)<=hi, sp(end+1)=j; end
                if I(j)>hi, break; end
            end
            if length(sp)>=2
                p1=sp(1); p2=sp(end); dI=I(p2)-I(p1);
                if dI>0.01
                    cz=CNT(p1)-I(p1)*(CNT(p2)-CNT(p1))/dI;
                    if cz<0, cz=cz+cpc; end
                    dc=cz-CU; if dc<0, dc=dc+cpc; end
                    ph=(dc/cpc)*360; ph=mod(ph,360); if ph>180, ph=ph-360; end
                    if ph>=8&&ph<=85, pe(end+1)=ph; end
                end
            end
        end
    end
    ap=[pv pe];
    if ~isempty(ap)
        ap_pos=ap(ap>0);
        if ~isempty(ap_pos), phi_val=median(ap_pos);
        else, phi_val=median(ap); end
    end
end

function [frames_data, frames_N] = parse_csv(csv_path)
    fid=fopen(csv_path,'r');
    fgetl(fid);
    ncol=length(strsplit(fgetl(fid),',')); frewind(fid); fgetl(fid);
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

function pk=robust_peak(I,edge)
    n=length(I); if n<=2*edge, pk=max(I); return; end
    It=I((edge+1):(n-edge)); Is=sort(It);
    pk=Is(max(1,round(length(Is)*0.98)));
end
