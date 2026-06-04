% debug_power_verify — 验证 P_W 上下管分离算法 vs 基波法
% 复现 C 代码 CalculateAuctalCurrent + CalculatePower 逻辑
function debug_power_verify()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;
C_FARAD = 0.94e-6;

base = '../../tools/ekf_tuner/';
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

fprintf('========== P_W: C上下管分离 vs 基波法 ==========\n');
fprintf('%-6s %8s %8s %8s %8s %8s\n', ...
    'File','Frm','P_C','P_fund','P_old','Δ(C-fund)');

all_P_C = []; all_P_fund = []; all_P_old = []; all_I_rms = []; all_Vdc = [];

for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        [P_C, P_fund, P_old, I_rms, Vdc_mean, valid] = ...
            calc_power_both(fd{f}, fN{f}, cal, C_FARAD);
        if valid
            all_P_C(end+1) = P_C;
            all_P_fund(end+1) = P_fund;
            all_P_old(end+1) = P_old;
            all_I_rms(end+1) = I_rms;
            all_Vdc(end+1) = Vdc_mean;
            if f <= 3
                fprintf('%-6d %8d %8.1f %8.1f %8.1f %+8.1f\n', ...
                    fidx, f, P_C, P_fund, P_old, P_C-P_fund);
            end
        end
    end
end

N = length(all_P_C);
fprintf('\n--- 统计 (N=%d) ---\n', N);
fprintf('P_C (上下管分离):  mean=%.1f  std=%.1f  CV=%.1f%%\n', ...
    mean(all_P_C), std(all_P_C), std(all_P_C)/mean(all_P_C)*100);
fprintf('P_fund (基波法):   mean=%.1f  std=%.1f  CV=%.1f%%\n', ...
    mean(all_P_fund), std(all_P_fund), std(all_P_fund)/mean(all_P_fund)*100);

diff_C_fund = all_P_C - all_P_fund;
fprintf('Δ(C-fund):        mean=%.1f  std=%.1fW\n', mean(diff_C_fund), std(diff_C_fund));

ratio = all_P_C ./ all_P_fund;
fprintf('P_C/P_fund ratio:  mean=%.3f  std=%.3f  min=%.3f  max=%.3f\n', ...
    mean(ratio), std(ratio), min(ratio), max(ratio));

% P_C vs I_rms (验证一致性)
fprintf('\n--- P_C vs I_rms 相关性 ---\n');
fprintf('P_C vs I_rms: r=%.3f\n', corr(all_P_C', all_I_rms'));
fprintf('P_fund vs I_rms: r=%.3f\n', corr(all_P_fund', all_I_rms'));

% P_C vs Vdc
fprintf('\n--- P vs Vdc ---\n');
fprintf('P_C vs Vdc: r=%.3f\n', corr(all_P_C', all_Vdc'));

% P_C / I_rms^2 稳定性 (等效R)
R_eq_C = all_P_C ./ (all_I_rms.^2);
R_eq_fund = all_P_fund ./ (all_I_rms.^2);
fprintf('\n--- 等效电阻 R=P/I² ---\n');
fprintf('R_eq (C法):     mean=%.1f  CV=%.1f%%\n', mean(R_eq_C), std(R_eq_C)/mean(R_eq_C)*100);
fprintf('R_eq (基波法):  mean=%.1f  CV=%.1f%%\n', mean(R_eq_fund), std(R_eq_fund)/mean(R_eq_fund)*100);

% 按I_rms分档
fprintf('\n--- 按 I_rms 分档 ---\n');
I_bins = [5 10 15 20 25 35];
fprintf('%-14s %5s %8s %8s %8s %8s\n', 'I_rms[A]','N','P_C','P_fund','Δ','Δ%%');
for b = 1:(length(I_bins)-1)
    bm = all_I_rms >= I_bins(b) & all_I_rms < I_bins(b+1);
    if sum(bm) >= 3
        d = diff_C_fund(bm);
        fprintf('%-14s %5d %8.1f %8.1f %8.1f %7.1f%%\n', ...
            sprintf('%d-%d',I_bins(b),I_bins(b+1)), sum(bm), ...
            mean(all_P_C(bm)), mean(all_P_fund(bm)), mean(d), mean(d)/mean(all_P_fund(bm))*100);
    end
end

% P_C/(Vdc*I_rms) ≈ PF
fprintf('\n--- P_C/(Vdc*I_rms) 估算PF ---\n');
PF_est = all_P_C ./ (all_Vdc .* all_I_rms);
fprintf('mean=%.3f  std=%.3f  P10=%.3f  P90=%.3f\n', ...
    mean(PF_est), std(PF_est), prctile(PF_est,10), prctile(PF_est,90));

figure('Position',[50 200 1200 500]);
subplot(1,2,1);
scatter(all_P_fund, all_P_C, 20, all_I_rms, 'filled');
hold on; plot([0 max(all_P_fund)], [0 max(all_P_fund)], 'k--');
xlabel('P_{fund} (基波法) W'); ylabel('P_C (上下管分离) W');
title(sprintf('P_W 两种算法对比 (N=%d), r=%.4f', N, corr(all_P_C', all_P_fund')));
colorbar; grid on;

subplot(1,2,2);
scatter(all_I_rms, all_P_C ./ all_P_fund, 20, all_Vdc, 'filled');
hold on; yline(1.0, 'k--');
xlabel('I_{rms} (A)'); ylabel('P_C / P_{fund}');
title(sprintf('比值 vs I_{rms}, 均值=%.3f±%.3f', mean(ratio), std(ratio)));
colorbar; grid on;
end

function [P_C, P_fund, P_old, I_rms, Vdc_mean, valid] = calc_power_both(frame, N, cal, C_FARAD)
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
    CNT_clean = CNT(seg_start:seg_end);
    Vdc_clean = Vdc(seg_start:seg_end);

    dCNT = diff(CNT_clean); dCNT_pos = dCNT(dCNT > 0);
    if isempty(dCNT_pos)
        P_C=NaN; P_fund=NaN; P_old=NaN; I_rms=NaN; Vdc_mean=NaN; valid=false; return;
    end
    avg_dcnt = mean(dCNT_pos);
    dtv = diff(t_us(seg_start:seg_end)); dtv(dtv <= 0) = [];
    avg_dt_us = mean(dtv);
    t_per_cnt_us = avg_dt_us / avg_dcnt;
    seg_full_CNT = CNT(best_seg(1):best_seg(2));
    wi = find(diff(seg_full_CNT) < 0, 1);
    if ~isempty(wi)
        CNT_range = seg_full_CNT(wi) + round(avg_dcnt) - seg_full_CNT(wi+1);
    else
        CNT_range = max(seg_full_CNT) - min(seg_full_CNT);
    end
    period_cnt = CNT_range;
    Tu = period_cnt * t_per_cnt_us;
    f_sw = 1/(period_cnt * t_per_cnt_us * 1e-6);
    omega = 2*pi*f_sw;

    I_rms = rms(I_clean);
    Ipk = max(I_clean(1:end-1));
    Vdc_mean = mean(Vdc_clean);

    %% ==== 方法A: C 上下管分离 (复现 CalculateAuctalCurrent) ====
    % 找过零点 (谐振电流谷值)
    n_clean = length(I_clean);
    zero_up = 1; zero_down = 1;
    % 上管过零点: 在 [1, CO位置] 内找
    co_idx = find(CNT_clean >= CO, 1);
    if isempty(co_idx), co_idx = n_clean; end
    [~, zidx] = min(I_clean(1:co_idx));
    zero_up = zidx;

    % 下管过零点: 在 [CO位置+1, end] 内找
    if co_idx < n_clean
        [~, zidx2] = min(I_clean(co_idx:end));
        zero_down = co_idx - 1 + zidx2;
    else
        zero_down = zero_up;
    end

    % 上管电流积分: 从 zero_up 到 CO
    sum_I_up = 0; sum_V_up = 0; n_up = 0;
    for k = zero_up:co_idx
        if CNT_clean(k) <= CO
            sum_I_up = sum_I_up + I_clean(k);
            sum_V_up = sum_V_up + Vdc_clean(k);
            n_up = n_up + 1;
        end
    end

    % 下管电流积分: 从 zero_down 到周期结束
    sum_I_down = 0; sum_V_down = 0; n_down = 0;
    for k = zero_down:n_clean
        sum_I_down = sum_I_down + I_clean(k);
        sum_V_down = sum_V_down + Vdc_clean(k);
        n_down = n_down + 1;
    end

    if n_up < 2
        P_C = NaN; P_fund = NaN; P_old = NaN;
        I_rms = NaN; Vdc_mean = NaN; valid = false; return;
    end

    % 平均 Vdc 在区间内
    V_up_avg = sum_V_up / n_up;
    if n_down > 0, V_down_avg = sum_V_down / n_down; else, V_down_avg = V_up_avg; end

    % I 积分 (ADC单位→实际电流)
    % C代码: currentSumPower = currentSum * hrtim_per_adc
    % 这里: I 已经是实际电流(A), 积分 = mean(I) * N_samples
    I_up_int = sum_I_up;   % 电流积分和 (未归一化)
    I_down_int = sum_I_down;

    % 对称检测: CO*2+10 < period
    is_sym = (CO*2 + 10) >= period_cnt;

    if is_sym
        P_C = V_up_avg * I_up_int * 2 / length(I_clean);
    else
        P_C = (V_up_avg * I_up_int + V_down_avg * I_down_int) / length(I_clean);
    end

    %% ==== 方法B: 基波法 (原 calc_power.m) ====
    D_U = max(0, (CO-CU) * t_per_cnt_us) / Tu;
    Ipk_val = max(I_clean(1:min(end, co_idx)));
    phi_val = detect_phi_mini(I_clean, CNT_clean, CU, Tu, t_per_cnt_us, Ipk_val);

    if ~isnan(phi_val) && I_rms > 0.1
        V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
        P_fund = V1 * I_rms * cosd(phi_val);
    else
        P_fund = NaN;
    end

    %% ==== 方法C: 旧法 mean(I*V) ====
    P_old = mean(I_clean .* Vdc_clean);

    valid = ~isnan(P_C) && ~isnan(P_fund) && P_C > 0 && P_fund > 0;
end

function phi_val = detect_phi_mini(I, CNT, CU, Tu, t_per_cnt_us, Ipk)
    phi_val=NaN; cpc=Tu/t_per_cnt_us; nI=length(I);
    vi=[]; for i=2:(nI-1)
        if I(i-1)>I(i)&&I(i)<I(i+1)&&I(i)<Ipk*0.15, vi(end+1)=i; end
    end
    pv=[]; for k=1:length(vi)
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
    pe=[]; lo=Ipk*0.15; hi=Ipk*0.50;
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
