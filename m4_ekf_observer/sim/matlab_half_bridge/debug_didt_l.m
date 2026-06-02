% debug_didt_l — di/dt法 vs RMS法 对比, 只在上升沿30%-70%线性段取di/dt反推I1_rms
function debug_didt_l()
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
fprintf('file                   f_kHz  L_rms  L_didt  diff_uH  didt_Aus  r2     N_lin\n');
fprintf('%s\n', repmat('-',1,85));

for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        r = calc_one_frame_didt(fd{f}, fN{f}, cal, C_FARAD);
        if ~isnan(r.phi_deg) && r.Ipk_A > 2
            all_r = [all_r; r];
            fprintf('%-24s %5.2f %6.1f %6.1f  %+6.1f   %7.3f  %6.4f  %4d\n', ...
                iron_files{fidx}(1:min(24,end)), r.f_sw_kHz, r.L_uH_rms, r.L_uH_didt, ...
                r.L_uH_didt - r.L_uH_rms, r.didt, r.r2_lin, r.N_lin);
        end
    end
end

% ---- 高压段对比 ----
mask = [all_r.Vdc_mean] >= 200;
hv = all_r(mask);
L_rms = [hv.L_uH_rms]';
L_didt = [hv.L_uH_didt]';

fprintf('\n========== Vdc>200V: RMS vs di/dt ==========\n');
fprintf('RMS:   N=%d  mean=%.1f  std=%.1f  CV=%.1f%%  skew=%.2f\n', ...
    sum(mask), mean(L_rms), std(L_rms), std(L_rms)/mean(L_rms)*100, skewness(L_rms));
fprintf('di/dt: N=%d  mean=%.1f  std=%.1f  CV=%.1f%%  skew=%.2f\n', ...
    sum(mask), mean(L_didt), std(L_didt), std(L_didt)/mean(L_didt)*100, skewness(L_didt));

% 残差
L_res_rms = L_rms - mean(L_rms);
L_res_didt = L_didt - mean(L_didt);
fprintf('RMS残差std=%.2f  di/dt残差std=%.2f\n', std(L_res_rms), std(L_res_didt));

% 按频段分
freqs = [hv.f_sw_kHz]';
bands = {[23 26], [26 28], [28 31], [31 33]};
fprintf('\n--- By frequency band ---\n');
fprintf('%-14s %5s  %7s %7s  %7s %7s\n', 'Band','N','CV_rms%','CV_didt%','mean_rms','mean_didt');
for b = 1:length(bands)
    bm = freqs >= bands{b}(1) & freqs < bands{b}(2);
    if sum(bm) >= 3
        fprintf('[%d-%dkHz]     %4d   %6.1f  %6.1f   %6.1f  %6.1f\n', ...
            bands{b}(1), bands{b}(2), sum(bm), ...
            std(L_rms(bm))/mean(L_rms(bm))*100, ...
            std(L_didt(bm))/mean(L_didt(bm))*100, ...
            mean(L_rms(bm)), mean(L_didt(bm)));
    end
end

% di/dt 线性段 r² 分布
r2_all = [hv.r2_lin]';
fprintf('\n--- di/dt fit R^2 distribution ---\n');
fprintf('min=%.4f  P25=%.4f  median=%.4f  P75=%.4f  max=%.4f\n', ...
    min(r2_all), prctile(r2_all,25), median(r2_all), prctile(r2_all,75), max(r2_all));

% L_rms vs L_didt 散点统计
fprintf('\n--- L_didt - L_rms delta ---\n');
dL = L_didt - L_rms;
fprintf('mean_delta=%.1f  std=%.1f  min=%.1f  max=%.1f\n', mean(dL), std(dL), min(dL), max(dL));

end

function r = calc_one_frame_didt(frame, N, cal, C_FARAD)
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

    % 频率计算
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

    % ==== di/dt 法: 30%-70% 线性段拟合 ====
    n = length(I_clean);
    [I_max, idx_max] = max(I_clean);

    % 从峰值往回找上升沿起点: 第一个低于 15% I_max 的谷值点
    idx_valley = 1;
    for k = idx_max:-1:2
        if I_clean(k) < I_max * 0.15
            idx_valley = k;
            break;
        end
    end
    % fallback: 如果回溯没找到 (起点本来就 >15%)，取 idx_max 之前的最小值
    if idx_valley == 1 && I_clean(1) > I_max * 0.15
        [~, idx_valley] = min(I_clean(1:idx_max));
    end
    I_valley = I_clean(idx_valley);

    % 上升沿
    rise_I = I_clean(idx_valley:idx_max);
    rise_t = t_clean(idx_valley:idx_max);

    % 30%-70% 线性段 (相对于 valley→peak 区间)
    delta_I = I_max - I_valley;
    I_lo = I_valley + delta_I * 0.30;
    I_hi = I_valley + delta_I * 0.70;

    % 选 30%-70% 区间
    lin_mask = rise_I >= I_lo & rise_I <= I_hi;
    N_lin = sum(lin_mask);

    if N_lin < 3
        % 线性段不够, 退回 RMS 法
        didt = NaN; r2 = NaN;
        I1_rms_didt = NaN;
        L_didt = NaN;
    else
        % 线性拟合
        t_lin = rise_t(lin_mask);
        I_lin = rise_I(lin_mask);
        p = polyfit(t_lin, I_lin, 1);
        didt = p(1);  % A/us

        % r²
        I_pred = polyval(p, t_lin);
        SS_res = sum((I_lin - I_pred).^2);
        SS_tot = sum((I_lin - mean(I_lin)).^2);
        r2 = 1 - SS_res / SS_tot;

        % 中点处的 cos 修正
        t_mid = mean(t_lin);
        t0 = t_clean(idx_valley);
        dt_from_min_us = t_mid - t0;
        T4_us = (1e6 / f_sw) / 4;
        omega_t = (pi/2) * (dt_from_min_us / T4_us);
        omega_t = max(0.1, min(pi/2 - 0.1, omega_t));
        cos_corr = cos(omega_t);

        Ipk_est = (didt * 1e6) / (omega * cos_corr);  % didt A/us → A/s → / (ω×cos)
        I1_rms_didt = abs(Ipk_est) / sqrt(2);

        % 用 I1_rms_didt 算 L
        if ~isnan(phi_val) && I1_rms_didt > 0.1
            V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
            Z_didt = V1 / I1_rms_didt;
            X_didt = Z_didt * sind(phi_val);
            L_didt = (1/(omega*C_FARAD) + X_didt) / omega * 1e6;
        else
            L_didt = NaN;
        end
    end

    % ==== RMS 法 L ====
    if ~isnan(phi_val) && I_rms > 0.1
        V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
        Z_rms = V1 / I_rms;
        X_rms = Z_rms * sind(phi_val);
        L_rms = (1/(omega*C_FARAD) + X_rms) / omega * 1e6;
    else
        L_rms = NaN;
    end

    r.f_sw_kHz  = f_sw/1e3;
    r.L_uH_rms  = L_rms;
    r.L_uH_didt = L_didt;
    r.didt      = didt;
    r.r2_lin    = r2;
    r.N_lin     = N_lin;
    r.phi_deg   = phi_val;
    r.Ipk_A     = Ipk;
    r.Vdc_mean  = Vdc_mean;
    r.I_RMS_A   = I_rms;
    r.D_U       = D_U;
end

function r = nan_result()
    r.f_sw_kHz=NaN; r.L_uH_rms=NaN; r.L_uH_didt=NaN;
    r.didt=NaN; r.r2_lin=NaN; r.N_lin=0;
    r.phi_deg=NaN; r.Ipk_A=NaN; r.Vdc_mean=NaN; r.I_RMS_A=NaN; r.D_U=NaN;
end

% ====== helper functions ======
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
