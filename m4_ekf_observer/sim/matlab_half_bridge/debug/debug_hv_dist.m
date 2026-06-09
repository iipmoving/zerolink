% debug_hv_dist — 高压段 L 分布分析
function debug_hv_dist()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE; cal.HRTIM_CLK_MHZ = 144;
C_FARAD = 0.94e-6;
base = '../../tools/ekf_tuner/captures/captures/';

iron_files = {'capture_20260602_153307.csv','capture_20260602_153311.csv','capture_20260602_153314.csv','capture_20260602_153316.csv','capture_20260602_153326.csv','capture_20260602_153329.csv','capture_20260602_153336.csv','capture_20260602_153338.csv','capture_20260602_153340.csv','capture_20260602_153347.csv','capture_20260602_153348.csv','capture_20260602_153358.csv','capture_20260602_153400.csv','capture_20260602_153410.csv','capture_20260602_153412.csv','capture_20260602_153413.csv','capture_20260602_153421.csv','capture_20260602_153422.csv','capture_20260602_153424.csv'};

all_r = [];
for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        r = calc_one_frame(fd{f}, fN{f}, cal, C_FARAD);
        if ~isnan(r.phi_deg) && r.I_peak_A > 2
            all_r = [all_r; r];
        end
    end
end

% 高压段
mask = [all_r.Vdc_mean] >= 200;
hv = all_r(mask);
L_hv = [hv.L_uH]';
Vdc_hv = [hv.Vdc_mean]';
Irms_hv = [hv.I_RMS_A]';

% 按频率分
freqs = [hv.f_sw_kHz]';
bands = {[23 26], [26 28], [28 31], [31 33]};
colors = {'r','b','g','m'};

fprintf('========== L Distribution: Vdc>200V ==========\n');
fprintf('N=%d  mean=%.1f  std=%.1f  CV=%.1f%%\n', length(L_hv), mean(L_hv), std(L_hv), std(L_hv)/mean(L_hv)*100);
fprintf('min=%.1f  max=%.1f  range=%.1f\n', min(L_hv), max(L_hv), max(L_hv)-min(L_hv));
fprintf('skewness=%.2f  kurtosis=%.2f\n', skewness(L_hv), kurtosis(L_hv));

% 分频段统计
fprintf('\n--- By frequency band ---\n');
fprintf('%-14s %5s %7s %7s %7s %7s %7s %7s\n', 'Band','N','mean','std','CV%','min','max','skew');
for b = 1:length(bands)
    bm = freqs >= bands{b}(1) & freqs < bands{b}(2);
    if sum(bm) >= 3
        Lb = L_hv(bm);
        fprintf('[%d-%dkHz]     %4d  %6.1f  %6.1f  %6.1f  %6.1f  %6.1f  %6.2f\n', ...
            bands{b}(1), bands{b}(2), sum(bm), mean(Lb), std(Lb), ...
            std(Lb)/mean(Lb)*100, min(Lb), max(Lb), skewness(Lb));
    end
end

% 最大频段 23-26kHz 详细分布
bm23 = freqs >= 23 & freqs < 26;
L23 = L_hv(bm23);
fprintf('\n--- 23-26kHz N=%d, sorted L values ---\n', sum(bm23));
Ls = sort(L23);
% 打印排序后的 L，观察是否有跳变
for i = 1:length(Ls)
    marker = '';
    if i > 1 && Ls(i) - Ls(i-1) > 3, marker = ' <-- GAP'; end
    fprintf('  %2d: L=%.1f uH%s\n', i, Ls(i), marker);
end

% Q-Q 分位数
fprintf('\n--- Quantiles (all HV) ---\n');
pcts = [5 10 25 50 75 90 95];
for i = 1:length(pcts)
    fprintf('  P%02d: %.1f uH\n', pcts(i), prctile(L_hv, pcts(i)));
end

% L vs I_RMS 残差 (去线性趋势)
fprintf('\n--- L residuals after removing I_RMS linear trend ---\n');
p = polyfit(Irms_hv, L_hv, 1);
L_pred = polyval(p, Irms_hv);
L_res = L_hv - L_pred;
fprintf('L = %.2f + %.2f * I_RMS\n', p(2), p(1));
fprintf('Residuals: mean=%.2f  std=%.2f  min=%.2f  max=%.2f\n', ...
    mean(L_res), std(L_res), min(L_res), max(L_res));
fprintf('Residual CV = %.1f%%\n', std(L_res)/mean(L_hv)*100);

% 残差排序
[~, idx_sort] = sort(L_res);
fprintf('Top 5 negative residuals (L too low):\n');
for i = 1:min(5, length(idx_sort))
    j = idx_sort(i);
    fprintf('  L=%.1f  L_pred=%.1f  res=%.1f  Vdc=%.0f  I=%.1fA  phi=%.1f  Q=%.2f\n', ...
        L_hv(j), L_pred(j), L_res(j), Vdc_hv(j), Irms_hv(j), hv(j).phi_deg, hv(j).Q);
end
fprintf('Top 5 positive residuals (L too high):\n');
for i = max(1, length(idx_sort)-4):length(idx_sort)
    j = idx_sort(i);
    fprintf('  L=%.1f  L_pred=%.1f  res=%.1f  Vdc=%.0f  I=%.1fA  phi=%.1f  Q=%.2f\n', ...
        L_hv(j), L_pred(j), L_res(j), Vdc_hv(j), Irms_hv(j), hv(j).phi_deg, hv(j).Q);
end

fprintf('\n--- Steel HV reference ---\n');
steel_files = {'capture_20260602_152019.csv','capture_20260602_152020.csv','capture_20260602_152022.csv','capture_20260602_152035.csv','capture_20260602_152037.csv','capture_20260602_152040.csv','capture_20260602_152049.csv','capture_20260602_152052.csv','capture_20260602_152053.csv','capture_20260602_152104.csv','capture_20260602_152106.csv','capture_20260602_152108.csv'};
all_steel = [];
for fidx = 1:length(steel_files)
    csv_path = [base steel_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        r = calc_one_frame(fd{f}, fN{f}, cal, C_FARAD);
        if ~isnan(r.phi_deg) && r.I_peak_A > 2
            all_steel = [all_steel; r];
        end
    end
end
ms = [all_steel.Vdc_mean] >= 200;
steel_hv = all_steel(ms);
Ls_hv = [steel_hv.L_uH]';
fprintf('N=%d  mean=%.1f  std=%.1f  CV=%.1f%%  skew=%.2f  kurt=%.2f\n', ...
    sum(ms), mean(Ls_hv), std(Ls_hv), std(Ls_hv)/mean(Ls_hv)*100, skewness(Ls_hv), kurtosis(Ls_hv));
end

% helper functions
function r = calc_one_frame(frame, N, cal, C_FARAD)
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
    while seg_end > best_seg(1) && I_adc(seg_end) > 4090
        seg_end = seg_end - 1;
    end
    if seg_end > best_seg(1), seg_end = seg_end - 1; end
    seg_start = best_seg(1);
    I_clean = I(seg_start:seg_end);
    CNT_clean = CNT(seg_start:seg_end);
    t_clean = t_us(seg_start:seg_end);
    dCNT = diff(CNT_clean); dCNT_pos = dCNT(dCNT > 0);
    if isempty(dCNT_pos), r.f_sw_kHz=NaN; r.L_uH=NaN; return; end
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
    I_rms = rms(I_clean);
    Ipk = robust_peak(I_clean, 0);
    Vdc_mean = mean(Vdc(seg_start:seg_end));
    phi_val = detect_phi(I_clean, CNT_clean, CU, Tu, t_per_cnt_us, Ipk);
    omega = 2*pi*f_sw;
    if ~isnan(phi_val) && I_rms > 0.1
        V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
        Z = V1/I_rms; X = Z*sind(phi_val);
        L = (1/(omega*C_FARAD)+X)/omega*1e6;
        R_val = Z*cosd(phi_val);
        fr = 1/(2*pi*sqrt(L*1e-6*C_FARAD));
        if R_val>0, Qv=2*pi*fr*L*1e-6/R_val; else, Qv=NaN; end
    else, L=NaN; Qv=NaN; end
    r.f_sw_kHz=f_sw/1e3; r.L_uH=L; r.Q=Qv;
    r.P_W=Vdc_mean*interp_at_cnt(CO,CNT_clean,I_clean)*D_U/2;
    r.phi_deg=phi_val; r.I_peak_A=Ipk;
    r.Vdc_mean=Vdc_mean; r.I_RMS_A=I_rms; r.D_U=D_U;
end
function phi_val = detect_phi(I, CNT, CU, Tu, t_per_cnt_us, Ipk)
    phi_val=NaN; cpc=Tu/t_per_cnt_us; nI=length(I);
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
function pk=robust_peak(I,edge)
    n=length(I); if n<=2*edge, pk=max(I); return; end
    It=I((edge+1):(n-edge)); Is=sort(It);
    pk=Is(max(1,round(length(Is)*0.98)));
end
function Ii=interp_at_cnt(tgt,CNT,I)
    Ii=NaN; n=length(CNT);
    for i=1:(n-1)
        c1=CNT(i); c2=CNT(i+1);
        if c1<=tgt&&tgt<=c2&&c2>c1
            frac=(tgt-c1)/(c2-c1); Ii=I(i)+frac*(I(i+1)-I(i)); return;
        end
    end
    [~,idx]=min(abs(CNT-tgt)); Ii=I(idx);
end
