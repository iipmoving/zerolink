% debug_concentration — L集中度分析: RMS vs Slope
function debug_concentration()
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

% ---- 铁锅 ----
all_r = [];
for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        r = calc_one_frame_slope(fd{f}, fN{f}, cal, C_FARAD);
        if ~isnan(r.phi_deg) && r.Ipk_A > 2
            all_r = [all_r; r];
        end
    end
end

mask = [all_r.Vdc_mean] >= 200;
hv = all_r(mask);
L_rms = [hv.L_uH_rms]';
L_slope = [hv.L_uH_slope]';
freqs = [hv.f_sw_kHz]';

fprintf('========== Vdc>200V 集中度: RMS vs Slope ==========\n');
print_concentration(L_rms, 'RMS', freqs);
fprintf('\n');
print_concentration(L_slope, 'Slope', freqs);

% 同帧对比
both_valid = ~isnan(L_slope);
fprintf('\n========== 同帧对比 (N=%d) ==========\n', sum(both_valid));
print_concentration(L_rms(both_valid), 'RMS (same frames)', freqs(both_valid));
print_concentration(L_slope(both_valid), 'Slope (same frames)', freqs(both_valid));

% ---- 钢锅参考 ----
fprintf('\n========== 钢锅参考 (Steel) ==========\n');
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
        r = calc_one_frame_slope(fd{f}, fN{f}, cal, C_FARAD);
        if ~isnan(r.phi_deg) && r.Ipk_A > 2
            all_steel = [all_steel; r];
        end
    end
end
ms = [all_steel.Vdc_mean] >= 200;
steel_hv = all_steel(ms);
Ls_steel = [steel_hv.L_uH_rms]';
fs_steel = [steel_hv.f_sw_kHz]';
print_concentration(Ls_steel, 'Steel RMS', fs_steel);
end

function print_concentration(L, label, freqs)
    valid = ~isnan(L);
    Lv = L(valid);
    if isempty(Lv)
        fprintf('%s: no valid data\n', label);
        return;
    end
    med = median(Lv);
    p25 = prctile(Lv, 25);
    p75 = prctile(Lv, 75);
    p10 = prctile(Lv, 10);
    p90 = prctile(Lv, 90);
    p05 = prctile(Lv, 5);
    p95 = prctile(Lv, 95);

    % 各精度带内的占比
    band_pcts = [1 2 3 5];
    for b = 1:length(band_pcts)
        band = band_pcts(b);
        in_band = sum(abs(Lv - med) / med * 100 <= band);
        fprintf('[%s]  median=%.1f  IQR=%.1f (%.1f%%)  P10-P90=%.1f  P5-P95=%.1f  |  ±%d%%: %d/%d=%.0f%%\n', ...
            label, med, p75-p25, (p75-p25)/med*100, p90-p10, p95-p05, ...
            band, in_band, length(Lv), in_band/length(Lv)*100);
    end

    % 按频段
    if nargin >= 3 && ~isempty(freqs)
        fv = freqs(valid);
        bands = {[23 26], [26 28], [28 31], [31 33]};
        for bi = 1:length(bands)
            bm = fv >= bands{bi}(1) & fv < bands{bi}(2);
            if sum(bm) >= 5
                Lb = Lv(bm);
                mb = median(Lb);
                iqr_b = prctile(Lb,75)-prctile(Lb,25);
                p1 = sum(abs(Lb-mb)/mb*100<=1)/length(Lb)*100;
                p2 = sum(abs(Lb-mb)/mb*100<=2)/length(Lb)*100;
                fprintf('  [%d-%dkHz] N=%-2d  med=%.1f  IQR=%.1f (%.1f%%)  ±1%%:%.0f%%  ±2%%:%.0f%%\n', ...
                    bands{bi}(1), bands{bi}(2), length(Lb), mb, iqr_b, iqr_b/mb*100, p1, p2);
            end
        end
    end
end

function r = calc_one_frame_slope(frame, N, cal, C_FARAD)
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
    t_clean = t_us(seg_start:seg_end);

    dCNT = diff(CNT_clean); dCNT_pos = dCNT(dCNT > 0);
    if isempty(dCNT_pos), r = nan_result(); return; end
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
    omega = 2*pi*f_sw;

    I_rms = rms(I_clean);
    Ipk = robust_peak(I_clean, 0);
    Vdc_mean = mean(Vdc(seg_start:seg_end));
    phi_val = detect_phi(I_clean, CNT_clean, CU, Tu, t_per_cnt_us, Ipk);

    % RMS法
    if ~isnan(phi_val) && I_rms > 0.1
        V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
        Z = V1/I_rms; X = Z*sind(phi_val);
        L_rms = (1/(omega*C_FARAD)+X)/omega*1e6;
    else
        L_rms = NaN;
    end

    % Slope法
    n = length(I_clean);
    [I_max, idx_max] = max(I_clean);
    [~, idx_valley] = min(I_clean(1:idx_max));
    I_valley = I_clean(idx_valley);
    t0 = t_clean(idx_valley);
    n_rise = idx_max - idx_valley;

    L_slope = NaN;
    if n_rise >= 8
        slopes = zeros(n_rise, 1);
        for k = 1:n_rise
            dt_k = t_clean(idx_valley+k) - t0;
            if dt_k > 0
                slopes(k) = (I_clean(idx_valley+k) - I_valley) / dt_k;
            end
        end
        k_start = max(3, round(n_rise * 0.20));
        k_end   = min(n_rise, round(n_rise * 0.60));
        if k_end - k_start >= 3
            didt = mean(slopes(k_start:k_end));
            idx_mid = idx_valley + round((k_start + k_end) / 2);
            dt_mid = t_clean(idx_mid) - t0;
            T4_us = (1e6 / f_sw) / 4;
            omega_t = (pi/2) * (dt_mid / T4_us);
            omega_t = max(0.1, min(pi/2 - 0.1, omega_t));
            Ipk_est = (didt * 1e6) / (omega * cos(omega_t));
            I1_rms_slope = abs(Ipk_est) / sqrt(2);
            if ~isnan(phi_val) && I1_rms_slope > 0.1
                V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
                Z_slope = V1 / I1_rms_slope;
                X_slope = Z_slope * sind(phi_val);
                L_slope = (1/(omega*C_FARAD) + X_slope) / omega * 1e6;
            end
        end
    end

    r.f_sw_kHz   = f_sw/1e3;
    r.L_uH_rms   = L_rms;
    r.L_uH_slope = L_slope;
    r.phi_deg    = phi_val;
    r.Ipk_A      = Ipk;
    r.Vdc_mean   = Vdc_mean;
    r.I_RMS_A    = I_rms;
    r.D_U        = D_U;
end

function r = nan_result()
    r.f_sw_kHz=NaN; r.L_uH_rms=NaN; r.L_uH_slope=NaN;
    r.phi_deg=NaN; r.Ipk_A=NaN; r.Vdc_mean=NaN; r.I_RMS_A=NaN; r.D_U=NaN;
end

function phi_val = detect_phi(I, CNT, CU, Tu, t_per_cnt_us, Ipk)
    phi_val=NaN; cpc=Tu/t_per_cnt_us; nI=length(I);
    vi=[];
    for i=2:(nI-1)
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

function pk=robust_peak(I,edge)
    n=length(I); if n<=2*edge, pk=max(I); return; end
    It=I((edge+1):(n-edge)); Is=sort(It);
    pk=Is(max(1,round(length(Is)*0.98)));
end
