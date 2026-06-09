% debug_didt_l2 — 零点为起点, 逐点求斜率, 取最稳定区间
function debug_didt_l2()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;
cal.HRTIM_CLK_MHZ = 144;
C_FARAD = 0.94e-6;

base = '../../tools/ekf_tuner/captures/captures/';
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
fprintf('file                   f_kHz  L_rms  L_slope diff  didt  N_use\n');
fprintf('%s\n', repmat('-',1,75));

for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        r = calc_one_frame_slope(fd{f}, fN{f}, cal, C_FARAD);
        if ~isnan(r.phi_deg) && r.Ipk_A > 2
            all_r = [all_r; r];
            fprintf('%-24s %5.2f %6.1f %6.1f %+5.1f %6.3f %5d\n', ...
                iron_files{fidx}(1:min(24,end)), r.f_sw_kHz, r.L_uH_rms, ...
                r.L_uH_slope, r.L_uH_slope - r.L_uH_rms, r.didt, r.N_use);
        end
    end
end

mask = [all_r.Vdc_mean] >= 200;
hv = all_r(mask);
L_rms = [hv.L_uH_rms]';
L_slope = [hv.L_uH_slope]';
valid = ~isnan(L_slope);

fprintf('\n========== Vdc>200V: RMS vs Slope ==========\n');
fprintf('RMS:    N=%d  mean=%.1f  std=%.1f  CV=%.1f%%\n', ...
    sum(mask), mean(L_rms), std(L_rms), std(L_rms)/mean(L_rms)*100);
fprintf('Slope:  N=%d  mean=%.1f  std=%.1f  CV=%.1f%%\n', ...
    sum(valid), mean(L_slope(valid)), std(L_slope(valid)), ...
    std(L_slope(valid))/mean(L_slope(valid))*100);

% 同帧对比 (只取两种方法都有效的帧)
both = valid;
fprintf('\n--- Same-frame comparison ---\n');
fprintf('RMS:    N=%d  mean=%.1f  std=%.1f  CV=%.1f%%\n', ...
    sum(both), mean(L_rms(both)), std(L_rms(both)), std(L_rms(both))/mean(L_rms(both))*100);
fprintf('Slope:  N=%d  mean=%.1f  std=%.1f  CV=%.1f%%\n', ...
    sum(both), mean(L_slope(both)), std(L_slope(both)), std(L_slope(both))/mean(L_slope(both))*100);

% 按频段
freqs = [hv.f_sw_kHz]';
bands = {[23 26], [26 28], [28 31], [31 33]};
fprintf('\n--- By frequency band (same-frame) ---\n');
fprintf('%-14s %5s  %7s %7s\n', 'Band','N','CV_rms%','CV_slope%');
for b = 1:length(bands)
    bm = freqs >= bands{b}(1) & freqs < bands{b}(2) & both';
    if sum(bm) >= 3
        fprintf('[%d-%dkHz]     %4d   %6.1f  %6.1f\n', ...
            bands{b}(1), bands{b}(2), sum(bm), ...
            std(L_rms(bm))/mean(L_rms(bm))*100, ...
            std(L_slope(bm))/mean(L_slope(bm))*100);
    end
end

dL = L_slope(both) - L_rms(both);
fprintf('\n--- L_slope - L_rms delta ---\n');
fprintf('mean=%.1f  std=%.1f  min=%.1f  max=%.1f\n', mean(dL), std(dL), min(dL), max(dL));
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

    % 频率
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

    % ==== 逐点斜率法: 零点→每点求斜率, 找最稳定区间 ====
    n = length(I_clean);
    [I_max, idx_max] = max(I_clean);

    % 找上升沿起点: 峰值前最低点
    [~, idx_valley] = min(I_clean(1:idx_max));
    I_valley = I_clean(idx_valley);
    t0 = t_clean(idx_valley);

    % 上升沿段
    n_rise = idx_max - idx_valley;
    didt = NaN;
    N_use = 0;

    if n_rise >= 4
        % 逐点算斜率: slope_k = (I_k - I_valley) / (t_k - t_valley)
        slopes = zeros(n_rise, 1);
        for k = 1:n_rise
            idx = idx_valley + k;
            dt_k = t_clean(idx) - t0;
            if dt_k > 0
                slopes(k) = (I_clean(idx) - I_valley) / dt_k;
            end
        end

        % 固定取 20%-60% 上升沿: 避开死区(0-20%)和峰值平坦(60-100%)
        k_start = max(3, round(n_rise * 0.20));
        k_end   = min(n_rise, round(n_rise * 0.60));
        if k_end - k_start >= 3
            didt = mean(slopes(k_start:k_end));
            N_use = k_end - k_start + 1;
        else
            didt = NaN;
            N_use = 0;
        end

        % 从 di/dt 反推基波电流, 取 20%-60% 段中点做 cos 修正
        idx_mid = idx_valley + round((k_start + k_end) / 2);
        dt_mid = t_clean(idx_mid) - t0;
        T4_us = (1e6 / f_sw) / 4;
        omega_t = (pi/2) * (dt_mid / T4_us);
        omega_t = max(0.1, min(pi/2 - 0.1, omega_t));
        cos_corr = cos(omega_t);

        Ipk_est = (didt * 1e6) / (omega * cos_corr);
        I1_rms_slope = abs(Ipk_est) / sqrt(2);
    else
        I1_rms_slope = NaN;
    end

    % L by slope method
    if ~isnan(phi_val) && ~isnan(I1_rms_slope) && I1_rms_slope > 0.1
        V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
        Z_slope = V1 / I1_rms_slope;
        X_slope = Z_slope * sind(phi_val);
        L_slope = (1/(omega*C_FARAD) + X_slope) / omega * 1e6;
    else
        L_slope = NaN;
    end

    % L by RMS
    if ~isnan(phi_val) && I_rms > 0.1
        V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
        Z_rms = V1 / I_rms;
        X_rms = Z_rms * sind(phi_val);
        L_rms = (1/(omega*C_FARAD) + X_rms) / omega * 1e6;
    else
        L_rms = NaN;
    end

    r.f_sw_kHz   = f_sw/1e3;
    r.L_uH_rms   = L_rms;
    r.L_uH_slope = L_slope;
    r.didt       = didt;
    r.N_use      = N_use;
    r.phi_deg    = phi_val;
    r.Ipk_A      = Ipk;
    r.Vdc_mean   = Vdc_mean;
    r.I_RMS_A    = I_rms;
    r.D_U        = D_U;
end

function r = nan_result()
    r.f_sw_kHz=NaN; r.L_uH_rms=NaN; r.L_uH_slope=NaN;
    r.didt=NaN; r.N_use=0;
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
