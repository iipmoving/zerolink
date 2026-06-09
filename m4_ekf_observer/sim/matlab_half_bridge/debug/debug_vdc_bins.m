% debug_vdc_bins — 按Vdc分档统计L集中度, 高频段单独看
function debug_vdc_bins()
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
for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        r = calc_one_frame(fd{f}, fN{f}, cal, C_FARAD);
        if ~isnan(r.phi_deg) && r.Ipk_A > 2
            all_r = [all_r; r];
        end
    end
end

mask = [all_r.Vdc_mean] >= 200;
hv = all_r(mask);

L_all = [hv.L_uH]';
V_all = [hv.Vdc_mean]';
F_all = [hv.f_sw_kHz]';
I_all = [hv.I_RMS_A]';

% Vdc 分档 (200-250, 250-280, 280+)
V_bins = [200 230 250 270 290 310];
fprintf('========== L by Vdc bins (>200V) ==========\n');
fprintf('%-14s %5s %7s %7s %7s %7s %7s %7s %7s\n', ...
    'Vdc[V]','N','L_med','L_mean','L_std','IQR','I_med','±1%','±2%');
fprintf('%s\n', repmat('-',1,90));
for b = 1:(length(V_bins)-1)
    bm = V_all >= V_bins(b) & V_all < V_bins(b+1);
    if sum(bm) >= 4
        Lb = L_all(bm); Lm = median(Lb); Ib = I_all(bm);
        p1 = sum(abs(Lb-Lm)/Lm*100<=1)/length(Lb)*100;
        p2 = sum(abs(Lb-Lm)/Lm*100<=2)/length(Lb)*100;
        iqr_b = prctile(Lb,75)-prctile(Lb,25);
        fprintf('%-14s %5d %7.1f %7.1f %7.2f %7.1f %7.1f %6.0f%% %6.0f%%\n', ...
            sprintf('%d-%dV',V_bins(b),V_bins(b+1)), sum(bm), Lm, mean(Lb), std(Lb), ...
            iqr_b, median(Ib), p1, p2);
    end
end

% 同频段23-26kHz, 按Vdc细分
fprintf('\n========== 23-26kHz: L by Vdc bins ==========\n');
bm_f = F_all >= 23 & F_all < 26;
band = hv(bm_f);
Lb_all = [band.L_uH]'; Vb_all = [band.Vdc_mean]'; Ib_all = [band.I_RMS_A]';
fprintf('N=%d  L=%.1f+-%.1f  IQR=%.1f\n', sum(bm_f), mean(Lb_all), std(Lb_all), ...
    prctile(Lb_all,75)-prctile(Lb_all,25));
fprintf('L vs Vdc: r=%.3f\n', corr(Lb_all, Vb_all));
fprintf('L vs I_rms: r=%.3f\n', corr(Lb_all, Ib_all));

fprintf('\n%-14s %5s %7s %7s %7s %7s %7s %7s %7s\n', ...
    'Vdc[V]','N','L_med','L_mean','L_std','IQR','I_med','±1%','±2%');
fprintf('%s\n', repmat('-',1,90));
V_fine = [200 220 240 260 280 300 320];
for b = 1:(length(V_fine)-1)
    bm = Vb_all >= V_fine(b) & Vb_all < V_fine(b+1);
    if sum(bm) >= 3
        Lb = Lb_all(bm); Lm = median(Lb); Ib = Ib_all(bm);
        p1 = sum(abs(Lb-Lm)/Lm*100<=1)/length(Lb)*100;
        p2 = sum(abs(Lb-Lm)/Lm*100<=2)/length(Lb)*100;
        iqr_b = prctile(Lb,75)-prctile(Lb,25);
        fprintf('%-14s %5d %7.1f %7.1f %7.2f %7.1f %7.1f %6.0f%% %6.0f%%\n', ...
            sprintf('%d-%dV',V_fine(b),V_fine(b+1)), sum(bm), Lm, mean(Lb), std(Lb), ...
            iqr_b, median(Ib), p1, p2);
    end
end

% 各Vdc档的线性拟合 L = a + b*Vdc
fprintf('\n--- L vs Vdc 线性拟合 ---\n');
p = polyfit(Vb_all, Lb_all, 1);
fprintf('L = %.2f + %.3f * Vdc\n', p(2), p(1));
L_pred = polyval(p, Vb_all);
L_res = Lb_all - L_pred;
fprintf('Residuals: std=%.2f  CV=%.1f%%\n', std(L_res), std(L_res)/mean(Lb_all)*100);

fprintf('\n--- L vs I_rms 线性拟合 (同频段) ---\n');
p2 = polyfit(Ib_all, Lb_all, 1);
fprintf('L = %.2f + %.2f * I_rms\n', p2(2), p2(1));
L_pred2 = polyval(p2, Ib_all);
L_res2 = Lb_all - L_pred2;
fprintf('Residuals: std=%.2f  CV=%.1f%%\n', std(L_res2), std(L_res2)/mean(Lb_all)*100);
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
