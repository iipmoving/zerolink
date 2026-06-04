% debug_cut_cycle — 用CNT单调性切出有效谐振周期, 排除FMAC过渡区
function debug_cut_cycle()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;
cal.HRTIM_CLK_MHZ = 144;
C_FARAD = 0.94e-6;

base = '../../tools/ekf_tuner/captures/captures/';

test_frames = {
    {'capture_20260602_153358.csv', 2, 'Iron outlier L=59.7'},
    {'capture_20260602_153358.csv', 1, 'Iron normal L=79.3'},
    {'capture_20260602_153424.csv', 4, 'Iron high-P L=74.6'},
    {'capture_20260602_152037.csv', 4, 'Steel normal'},
};

for t = 1:length(test_frames)
    csv_path = [base test_frames{t}{1}];
    target_f = test_frames{t}{2};
    label = test_frames{t}{3};

    [fd, fN] = parse_csv(csv_path);
    frame = fd{target_f};
    N = fN{target_f};
    t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
    CMP=frame(1,6:9);

    I_adc = min(max(I_adc, 0), 4095);
    Vdc_adc = min(max(Vdc_adc, 0), 4095);
    I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.VDC_SCALE;
    CU=CMP(1); CO=CMP(2);

    % ==== 新方法: CNT单调段 + 尾部FMAC切除 ====
    wrap_idx = find(diff(CNT) < 0);
    breaks = [0; wrap_idx(:); N];

    % 找最长单调段
    best_seg = [1 N]; best_len = 0;
    for b = 1:(length(breaks)-1)
        seg_start = breaks(b)+1;
        seg_end = breaks(b+1);
        seg_len = seg_end - seg_start + 1;
        if seg_len > best_len
            best_len = seg_len;
            best_seg = [seg_start, seg_end];
        end
    end

    % 从尾部切除 FMAC 垃圾: 从段尾往前找第一个I_adc>4090
    seg_start = best_seg(1); seg_end = best_seg(2);
    while seg_end > seg_start && I_adc(seg_end) > 4090
        seg_end = seg_end - 1;
    end
    % 再多切1个安全边界
    if seg_end > seg_start, seg_end = seg_end - 1; end

    I_clean = I(seg_start:seg_end);
    Vdc_clean = Vdc(seg_start:seg_end);
    CNT_clean = CNT(seg_start:seg_end);
    t_clean = t_us(seg_start:seg_end);

    % Timing from clean segment
    dCNT_c = diff(CNT_clean); dCNT_pos = dCNT_c(dCNT_c > 0);
    if isempty(dCNT_pos)
        fprintf('  WARNING: no positive dCNT in clean segment, skipping\n');
        continue;
    end
    avg_dcnt = mean(dCNT_pos);
    dtv = diff(t_clean); dtv(dtv <= 0) = [];
    avg_dt_us = mean(dtv);
    t_per_cnt_us = avg_dt_us / avg_dcnt;

    % CNT range: use the full (uncleaned) segment to get correct wrap
    full_seg_CNT = CNT(best_seg(1):best_seg(2));
    wi = find(diff(full_seg_CNT) < 0, 1);
    if ~isempty(wi)
        CNT_range = full_seg_CNT(wi) + round(avg_dcnt) - full_seg_CNT(wi+1);
    else
        CNT_range = max(full_seg_CNT) - min(full_seg_CNT);
    end
    HRTIM_period_s = CNT_range * t_per_cnt_us * 1e-6;
    if HRTIM_period_s <= 0, HRTIM_period_s = 1/30000; end
    f_sw_new = 1/HRTIM_period_s;
    Tu = HRTIM_period_s * 1e6;
    D_U = max(0, (CO-CU) * t_per_cnt_us) / Tu;

    I_rms_new = rms(I_clean);
    Ipk_new = robust_peak(I_clean, 0);
    Vdc_mean_new = mean(Vdc_clean);
    phi_new = detect_phi(I_clean, CNT_clean, CU, Tu, t_per_cnt_us, 0, Ipk_new);

    omega_new = 2*pi*f_sw_new;
    if ~isnan(phi_new) && I_rms_new > 0.1
        V1_new = (sqrt(2)/pi)*Vdc_mean_new*sin(pi*D_U);
        Z_new = V1_new/I_rms_new; X_new = Z_new*sind(phi_new);
        L_new = (1/(omega_new*C_FARAD)+X_new)/omega_new*1e6;
    else, L_new = NaN; end

    % ==== 老方法 (EDGE trim) ====
    EDGE = 2;
    dCNT_all = diff(CNT); dCNT_pos_all = dCNT_all(dCNT_all > 0);
    avg_dcnt_all = mean(dCNT_pos_all);
    dtv_all = diff(t_us); dtv_all(dtv_all <= 0) = [];
    avg_dt_us_all = mean(dtv_all);
    t_per_cnt_us_old = avg_dt_us_all / avg_dcnt_all;
    wi_old = find(diff(CNT) < 0, 1);
    if ~isempty(wi_old)
        CNT_range_old = CNT(wi_old) + round(avg_dcnt_all) - CNT(wi_old+1);
    else
        CNT_range_old = max(CNT) - min(CNT);
    end
    HRTIM_period_s_old = CNT_range_old * t_per_cnt_us_old * 1e-6;
    f_sw_old = 1/HRTIM_period_s_old;
    I_old = I((EDGE+1):(N-EDGE));
    I_rms_old = rms(I_old);
    Ipk_old = robust_peak(I, EDGE);
    Vdc_mean_old = mean(Vdc((EDGE+1):(N-EDGE)));
    phi_old = detect_phi(I, CNT, CU, HRTIM_period_s_old*1e6, t_per_cnt_us_old, EDGE, Ipk_old);
    omega_old = 2*pi*f_sw_old;
    if ~isnan(phi_old) && I_rms_old > 0.1
        V1_old = (sqrt(2)/pi)*Vdc_mean_old*sin(pi*D_U);
        Z_old = V1_old/I_rms_old; X_old = Z_old*sind(phi_old);
        L_old = (1/(omega_old*C_FARAD)+X_old)/omega_old*1e6;
    else, L_old = NaN; end

    fprintf('\n========== %s ==========\n', label);
    fprintf('CNT wraps at: %s\n', mat2str(wrap_idx));
    fprintf('Best segment: [%d-%d], after FMAC trim: [%d-%d] (%d samples)\n', ...
        best_seg(1), best_seg(2), seg_start, seg_end, length(I_clean));
    fprintf('%-18s  Old(EDGE)    New(CNT+trim)\n', '');
    fprintf('%-18s  %5d         %5d\n', 'N_samples:', length(I_old), length(I_clean));
    fprintf('%-18s  %8.2f     %8.2f\n', 'f_sw[kHz]:', f_sw_old/1e3, f_sw_new/1e3);
    fprintf('%-18s  %8.3f     %8.3f\n', 'I_RMS[A]:', I_rms_old, I_rms_new);
    fprintf('%-18s  %8.2f     %8.2f\n', 'Ipk[A]:', Ipk_old, Ipk_new);
    fprintf('%-18s  %8.1f     %8.1f\n', 'phi[deg]:', phi_old, phi_new);
    fprintf('%-18s  %8.1f     %8.1f\n', 'L[uH]:', L_old, L_new);
end

end

function phi_val = detect_phi(I, CNT, CU, Tu, t_per_cnt_us, EDGE, Ipk)
    phi_val = NaN;
    cpc=Tu/t_per_cnt_us;
    nI=length(I);
    vi=[];
    i_start = max(EDGE+1, 2);
    i_end = min(nI-EDGE, nI-1);
    for i=i_start:i_end
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
