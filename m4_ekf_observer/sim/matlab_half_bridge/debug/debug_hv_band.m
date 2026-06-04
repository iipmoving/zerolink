% debug_hv_band — Vdc>200V 铁锅高压段, 排除交越失真, 看剩余偏差来源
function debug_hv_band()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;
cal.HRTIM_CLK_MHZ = 144;
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

% 只取 Vdc > 200V
mask = [all_r.Vdc_mean] >= 200;
hv = all_r(mask);
fprintf('Vdc>200V frames: %d / %d total\n', sum(mask), length(all_r));

%% 按频率分组, 看同频下高压段 L 的 CV
freqs = [hv.f_sw_kHz];
fprintf('\n--- High-V L by frequency bands ---\n');
f_edges = [23 26 28 31 33 36 38];
fprintf('%-16s %6s %8s %8s %8s  %8s %8s\n', 'Band', 'N', 'L_mean', 'L_std', 'CV%', 'Vdc_range', 'P_range');
fprintf('%s\n', repmat('-',1,80));
for b = 1:(length(f_edges)-1)
    lo = f_edges(b); hi = f_edges(b+1);
    bm = freqs >= lo & freqs < hi;
    if sum(bm) >= 3
        Ls = [hv(bm).L_uH];
        Vdcs = [hv(bm).Vdc_mean];
        Ps = [hv(bm).P_W];
        fprintf('[%2d-%-2dkHz]     %4d  %8.1f  %8.1f  %7.1f  %4.0f-%-4.0fV %4.0f-%-4.0fW\n', ...
            lo, hi, sum(bm), mean(Ls), std(Ls), std(Ls)/mean(Ls)*100, ...
            min(Vdcs), max(Vdcs), min(Ps), max(Ps));
    end
end

%% 最大频段 (23-26kHz) 高压详细
mask2 = [hv.f_sw_kHz] >= 23 & [hv.f_sw_kHz] < 26;
band = hv(mask2);
fprintf('\n========== 23-26kHz Vdc>200V 详细 N=%d ==========\n', sum(mask2));
fprintf('L=%.1f+-%.1f uH  CV=%.1f%%\n', mean([band.L_uH]), std([band.L_uH]), ...
    std([band.L_uH])/mean([band.L_uH])*100);

% 看 L vs 各参数的残差
fprintf('\n--- Per-frame: f_sw, L, Vdc, phi, D_U, I_RMS, Q ---\n');
for i = 1:length(band)
    r = band(i);
    fprintf('f=%.2fk L=%.1fuH Vdc=%.0fV phi=%.1f D_U=%.3f I_RMS=%.1fA Q=%.2f P=%.0fW\n', ...
        r.f_sw_kHz, r.L_uH, r.Vdc_mean, r.phi_deg, r.D_U, r.I_RMS_A, r.Q, r.P_W);
end

%% 相关性
fprintf('\n--- L correlations in HV band ---\n');
vars = {'Vdc_mean','phi_deg','D_U','I_RMS_A','Q','f_sw_kHz','P_W'};
for v = 1:length(vars)
    R = corrcoef([band.L_uH]', [band.(vars{v})]');
    fprintf('L vs %-12s: r=%.4f\n', vars{v}, R(1,2));
end

%% 分解: L 的波动来自 Z 还是 phi?
fprintf('\n--- L formula decomposition ---\n');
for i = 1:length(band)
    r = band(i);
    omega = 2*pi*r.f_sw_kHz*1e3;
    V1 = (sqrt(2)/pi)*r.Vdc_mean*sin(pi*r.D_U);
    Z_val = V1 / r.I_RMS_A;
    X = Z_val * sind(r.phi_deg);
    band(i).V1 = V1;
    band(i).Z_ohm = Z_val;
    band(i).X_ohm = X;
    band(i).Xc = 1/(omega*C_FARAD);
    band(i).L_check = (band(i).Xc + X) / omega * 1e6;
end

Xc_all = [band.Xc]; X_all = [band.X_ohm];
fprintf('Xc=1/wC:  mean=%.2f  CV=%.1f%%\n', mean(Xc_all), std(Xc_all)/mean(Xc_all)*100);
fprintf('X=Z*sinφ: mean=%.2f  CV=%.1f%%\n', mean(X_all), std(X_all)/mean(X_all)*100);
fprintf('Xc/(Xc+X): mean=%.3f (Xc dominates?)\n', mean(Xc_all ./ (Xc_all + X_all)));

Z_all = [band.Z_ohm]; phi_all = [band.phi_deg];
fprintf('Z:        mean=%.2f  CV=%.1f%%\n', mean(Z_all), std(Z_all)/mean(Z_all)*100);
fprintf('phi:      mean=%.1f  CV=%.1f%%\n', mean(phi_all), std(phi_all)/mean(phi_all)*100);
fprintf('V1:       mean=%.1f  CV=%.1f%%\n', mean([band.V1]), std([band.V1])/mean([band.V1])*100);
fprintf('I_RMS:    mean=%.1f  CV=%.1f%%\n', mean([band.I_RMS_A]), std([band.I_RMS_A])/mean([band.I_RMS_A])*100);

%% 钢锅高压段对比
fprintf('\n========== Steel Vdc>200V (reference) ==========\n');
steel_files = {
    'capture_20260602_152019.csv','capture_20260602_152020.csv', ...
    'capture_20260602_152022.csv','capture_20260602_152035.csv', ...
    'capture_20260602_152037.csv','capture_20260602_152040.csv', ...
    'capture_20260602_152049.csv','capture_20260602_152052.csv', ...
    'capture_20260602_152053.csv','capture_20260602_152104.csv', ...
    'capture_20260602_152106.csv','capture_20260602_152108.csv'
};
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
fprintf('Steel Vdc>200V: N=%d  L=%.1f+-%.1f uH  CV=%.1f%%\n', ...
    sum(ms), mean([steel_hv.L_uH]), std([steel_hv.L_uH]), ...
    std([steel_hv.L_uH])/mean([steel_hv.L_uH])*100);

end

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
    while seg_end > best_seg(1) && I_adc(seg_end) > 4090, seg_end = seg_end - 1; end
    if seg_end > best_seg(1), seg_end = seg_end - 1; end
    I_clean = I(best_seg(1):seg_end);
    CNT_clean = CNT(best_seg(1):seg_end);
    t_clean = t_us(best_seg(1):seg_end);

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
    Vdc_mean = mean(Vdc(best_seg(1):seg_end));
    phi_val = detect_phi(I_clean, CNT_clean, CU, Tu, t_per_cnt_us, Ipk);

    omega = 2*pi*f_sw;
    if ~isnan(phi_val) && I_rms > 0.1
        V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
        Z = V1/I_rms; X = Z*sind(phi_val);
        L = (1/(omega*C_FARAD)+X)/omega*1e6;
        R_val = Z*cosd(phi_val);
        fr = 1/(2*pi*sqrt(L*1e-6*C_FARAD));
        Qv = 2*pi*fr*L*1e-6/R_val;
    else
        L=NaN; Qv=NaN; R_val=NaN;
    end
    r.f_sw_kHz=f_sw/1e3; r.L_uH=L; r.Q=Qv;
    r.P_W=Vdc_mean*interp_at_cnt(CO,CNT_clean,I_clean)*D_U/2;
    r.phi_deg=phi_val; r.I_peak_A=Ipk;
    r.Vdc_mean=Vdc_mean; r.I_RMS_A=I_rms; r.D_U=D_U;
end

function phi_val = detect_phi(I, CNT, CU, Tu, t_per_cnt_us, Ipk)
    phi_val=NaN; cpc=Tu/t_per_cnt_us; nI=length(I);
    vi=[]; for i=2:(nI-1)
        if I(i-1)>I(i)&&I(i)<I(i+1)&&I(i)<Ipk*0.15, vi(end+1)=i; end
    end
    pv=[]; for k=1:length(vi)
        vi_k=vi(k); cnt_interp=CNT(vi_k);
        if vi_k>1&&vi_k<length(I)
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
