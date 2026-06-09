% debug_irms_correct — 同频段内 I_rms线性修正, 再文件级聚合
function debug_irms_correct()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;
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

% ---- 逐帧读取 ----
all_r = [];
for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        r = calc_one_frame(fd{f}, fN{f}, cal, C_FARAD);
        if ~isnan(r.phi_deg) && r.Ipk_A > 2 && r.Vdc_mean >= 200
            r.file_idx = fidx;
            all_r = [all_r; r];
        end
    end
end

L_raw   = [all_r.L_uH]';
I_rms   = [all_r.I_RMS_A]';
freqs   = [all_r.f_sw_kHz]';
files   = [all_r.file_idx]';
Vdc     = [all_r.Vdc_mean]';

% ---- 按频段分别拟合 L = a + b*I_rms ----
f_bands = {[23 26], [26 28], [28 31], [31 33]};
band_names = {'23-26kHz','26-28kHz','28-31kHz','31-33kHz'};

fprintf('========== 频段内 L vs I_rms 拟合 ==========\n');
fprintf('%-12s %5s %8s %8s %8s\n', 'Band','N','L=a+b*Irms','残差std','残差CV%');
fprintf('%s\n', repmat('-',1,55));

% 存每帧的修正后 L
L_corr = nan(size(L_raw));

for bi = 1:length(f_bands)
    bm = freqs >= f_bands{bi}(1) & freqs < f_bands{bi}(2);
    if sum(bm) < 5, continue; end
    Lb = L_raw(bm); Ib = I_rms(bm);
    p = polyfit(Ib, Lb, 1);
    L_pred = polyval(p, Ib);
    res = Lb - L_pred;
    % 修正到本频段 I_rms 中值: L_corr = L_raw - b*(I - I_med)
    I_ref = median(Ib);
    L_corr(bm) = Lb - p(1) * (Ib - I_ref);
    fprintf('%-12s %5d  L=%.2f%+.2f*I  %.2f   %.1f%%\n', ...
        band_names{bi}, sum(bm), p(2), p(1), std(res), std(res)/mean(Lb)*100);
end

all_r_corr = all_r;
for i = 1:length(all_r)
    all_r_corr(i).L_uH = L_corr(i);
end

% ---- 文件级聚合: 修正前 vs 修正后 ----
file_L_raw = []; file_L_corr = []; file_f = [];
for fidx = 1:length(iron_files)
    fm = files == fidx;
    if sum(fm) >= 3
        file_L_raw(end+1)  = median(L_raw(fm));
        if ~all(isnan(L_corr(fm)))
            file_L_corr(end+1) = median(L_corr(fm & ~isnan(L_corr)));
        else
            file_L_corr(end+1) = NaN;
        end
        file_f(end+1) = median(freqs(fm));
    end
end

fprintf('\n========== 文件级: raw vs I_rms修正 ==========\n');
print_concentration(file_L_raw, 'Raw', file_f);
fprintf('\n');
print_concentration(file_L_corr, 'I_rms修正', file_f);

% 全频段汇总: 按文件打印
fprintf('\n--- 每文件: L_raw → L_corr ---\n');
[~, si] = sort(file_L_raw);
fprintf('%-6s %22s %6s %7s %7s %7s\n', 'Band','File','f_kHz','L_raw','L_corr','Δ');
for i = 1:length(si)
    idx = si(i);
    fname = iron_files{idx}(9:end-4);
    fk = file_f(idx);
    if fk < 27, band = '23-26';
    elseif fk < 29, band = '26-28';
    elseif fk < 32, band = '28-31';
    else band = '31-33'; end
    fprintf('%-6s %22s %5.2f %7.1f %7.1f %+6.1f\n', ...
        band, fname, fk, file_L_raw(idx), file_L_corr(idx), ...
        file_L_corr(idx) - file_L_raw(idx));
end

% 修正系数汇总
fprintf('\n--- 各频段修正系数 (供C实现参考) ---\n');
fprintf('L_corr = L_raw - k * (I_rms - I_ref)\n');
fprintf('%-12s %8s %8s\n', 'Band','k','I_ref');
for bi = 1:length(f_bands)
    bm = freqs >= f_bands{bi}(1) & freqs < f_bands{bi}(2);
    if sum(bm) < 5, continue; end
    Lb = L_raw(bm); Ib = I_rms(bm);
    p = polyfit(Ib, Lb, 1);
    fprintf('%-12s %8.2f %8.1f\n', band_names{bi}, -p(1), median(Ib));
end
end

function print_concentration(L, label, freqs)
    valid = ~isnan(L);
    Lv = L(valid);
    if isempty(Lv), fprintf('[%s] no data\n', label); return; end
    med = median(Lv);
    iqr_v = prctile(Lv,75)-prctile(Lv,25);
    p1090 = prctile(Lv,90)-prctile(Lv,10);

    fprintf('[%s] N=%d  med=%.1f  IQR=%.1f (%.1f%%)  P10-P90=%.1f\n', ...
        label, length(Lv), med, iqr_v, iqr_v/med*100, p1090);
    for b = [1 2 3 5]
        in = sum(abs(Lv-med)/med*100 <= b);
        fprintf('  ±%d%%: %d/%d = %.0f%%\n', b, in, length(Lv), in/length(Lv)*100);
    end
    if nargin >= 3 && ~isempty(freqs)
        fv = freqs(valid);
        bands = {[23 26], [26 28], [28 31], [31 33]};
        band_names = {'23-26kHz','26-28kHz','28-31kHz','31-33kHz'};
        for bi = 1:length(bands)
            bm = fv >= bands{bi}(1) & fv < bands{bi}(2);
            if sum(bm) >= 3
                Lb = Lv(bm); mb = median(Lb);
                iqr_b = prctile(Lb,75)-prctile(Lb,25);
                p1 = sum(abs(Lb-mb)/mb*100<=1)/length(Lb)*100;
                p2 = sum(abs(Lb-mb)/mb*100<=2)/length(Lb)*100;
                fprintf('  [%s] N=%d IQR=%.1f(%.1f%%) ±1%%:%.0f%% ±2%%:%.0f%%\n', ...
                    band_names{bi}, length(Lb), iqr_b, iqr_b/mb*100, p1, p2);
            end
        end
    end
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
    omega = 2*pi*f_sw;

    I_rms = rms(I_clean);
    Ipk = robust_peak(I_clean, 0);
    Vdc_mean = mean(Vdc(seg_start:seg_end));
    phi_val = detect_phi(I_clean, CNT_clean, CU, Tu, t_per_cnt_us, Ipk);

    if ~isnan(phi_val) && I_rms > 0.1
        V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
        Z = V1/I_rms; X = Z*sind(phi_val);
        L = (1/(omega*C_FARAD)+X)/omega*1e6;
    else
        L = NaN;
    end
    r.f_sw_kHz=f_sw/1e3; r.L_uH=L;
    r.phi_deg=phi_val; r.Ipk_A=Ipk;
    r.Vdc_mean=Vdc_mean; r.I_RMS_A=I_rms; r.D_U=D_U;
end

function phi_val = detect_phi(I, CNT, CU, Tu, t_per_cnt_us, Ipk)
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

function pk=robust_peak(I,edge)
    n=length(I); if n<=2*edge, pk=max(I); return; end
    It=I((edge+1):(n-edge)); Is=sort(It);
    pk=Is(max(1,round(length(Is)*0.98)));
end
