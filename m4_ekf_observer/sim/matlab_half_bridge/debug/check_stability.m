% check_stability — 铁锅+钢锅 L稳定性: 同频段不同功率 vs 跨频段趋势
function check_stability()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;
cal.HRTIM_CLK_MHZ = 144;
C_FARAD = 0.94e-6;

% 所有铁锅文件 (第二轮)
iron_files = {
    'capture_20260602_153307.csv'
    'capture_20260602_153311.csv'
    'capture_20260602_153314.csv'
    'capture_20260602_153316.csv'
    'capture_20260602_153326.csv'
    'capture_20260602_153329.csv'
    'capture_20260602_153336.csv'
    'capture_20260602_153338.csv'
    'capture_20260602_153340.csv'
    'capture_20260602_153347.csv'
    'capture_20260602_153348.csv'
    'capture_20260602_153358.csv'
    'capture_20260602_153400.csv'
    'capture_20260602_153410.csv'
    'capture_20260602_153412.csv'
    'capture_20260602_153413.csv'
    'capture_20260602_153421.csv'
    'capture_20260602_153422.csv'
    'capture_20260602_153424.csv'
};

% 所有钢锅文件
steel_files = {
    'capture_20260602_152019.csv'
    'capture_20260602_152020.csv'
    'capture_20260602_152022.csv'
    'capture_20260602_152035.csv'
    'capture_20260602_152037.csv'
    'capture_20260602_152040.csv'
    'capture_20260602_152049.csv'
    'capture_20260602_152052.csv'
    'capture_20260602_152053.csv'
    'capture_20260602_152104.csv'
    'capture_20260602_152106.csv'
    'capture_20260602_152108.csv'
};

base = '../../tools/ekf_tuner/captures/captures/';

fprintf('========== 铁锅: 按频率分组, 看同频下L vs P ==========\n');
process_set(iron_files, base, cal, C_FARAD, 'Iron');

fprintf('\n========== 钢锅: 按频率分组, 看同频下L vs P ==========\n');
process_set(steel_files, base, cal, C_FARAD, 'Steel');
end

function process_set(file_list, base, cal, C_FARAD, label)
    all_r = [];

    for fidx = 1:length(file_list)
        csv_path = [base file_list{fidx}];
        [fd, fN] = parse_csv(csv_path);
        for f = 1:length(fd)
            r = calc_one_frame(fd{f}, fN{f}, cal, C_FARAD);
            if ~isnan(r.phi_deg) && r.I_peak_A > 2
                all_r = [all_r; r];
            end
        end
    end

    % 按频率分组 (0.5kHz 步长)
    freqs = [all_r.f_sw_kHz];
    f_edges = floor(min(freqs)):0.5:ceil(max(freqs))+0.5;

    fprintf('\n%-12s  %6s  %8s  %8s  %8s  %8s  %8s\n', ...
        'f_range[kHz]', 'N', 'L_mean', 'L_std', 'L_min', 'L_max', 'P_range[W]');
    fprintf('%s\n', repmat('-',1,80));

    for b = 1:(length(f_edges)-1)
        lo = f_edges(b); hi = f_edges(b+1);
        mask = freqs >= lo & freqs < hi;
        if sum(mask) >= 2
            Ls = [all_r(mask).L_uH];
            Ps = [all_r(mask).P_W];
            fprintf('%-12s  %6d  %8.1f  %8.1f  %8.1f  %8.1f  %6.0f-%-6.0f\n', ...
                sprintf('[%.1f-%.1f]', lo, hi), sum(mask), ...
                mean(Ls), std(Ls), min(Ls), max(Ls), min(Ps), max(Ps));
        end
    end

    % 各主要频率段详细列出
    fprintf('\n--- 主要频段详细: f_sw, L, P, Vdc, phi, Q ---\n');
    bands = {[23 25], [25 27], [29 32], [36 40]};
    for b = 1:length(bands)
        lo = bands{b}(1); hi = bands{b}(2);
        mask = freqs >= lo & freqs < hi;
        if sum(mask) >= 2
            fprintf('\n[%d-%dkHz] N=%d:\n', lo, hi, sum(mask));
            idx = find(mask);
            for i = 1:length(idx)
                r = all_r(idx(i));
                fprintf('  f=%.2fk  L=%.1fuH  P=%.0fW  Vdc=%.0fV  phi=%.1f  Q=%.2f\n', ...
                    r.f_sw_kHz, r.L_uH, r.P_W, r.Vdc_mean, r.phi_deg, r.Q);
            end
            Ls = [all_r(mask).L_uH];
            fprintf('  >> L=%.1f+-%.1f uH  CV=%.1f%%\n', mean(Ls), std(Ls), std(Ls)/mean(Ls)*100);
        end
    end

    % 总体 L vs f 趋势
    fprintf('\n--- 频段汇总 ---\n');
    fprintf('%-12s  %6s  %8s  %8s  %8s\n', 'Band', 'N', 'L_mean', 'L_std', 'CV%');
    bands2 = {[23 26], [26 28], [28 31], [31 33], [36 38]};
    for b = 1:length(bands2)
        lo = bands2{b}(1); hi = bands2{b}(2);
        mask = freqs >= lo & freqs < hi;
        if sum(mask) >= 2
            Ls = [all_r(mask).L_uH];
            fprintf('%-12s  %6d  %8.1f  %8.1f  %8.1f\n', ...
                sprintf('[%d-%dkHz]', lo, hi), sum(mask), mean(Ls), std(Ls), std(Ls)/mean(Ls)*100);
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

    % ==== CNT monotonic segment: cut valid resonant cycle ====
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
    % Trim FMAC garbage from tail of best segment
    seg_end = best_seg(2);
    while seg_end > best_seg(1) && I_adc(seg_end) > 4090
        seg_end = seg_end - 1;
    end
    if seg_end > best_seg(1), seg_end = seg_end - 1; end
    seg_start = best_seg(1);

    % Extract clean data
    I_clean = I(seg_start:seg_end);
    Vdc_clean = Vdc(seg_start:seg_end);
    CNT_clean = CNT(seg_start:seg_end);
    t_clean = t_us(seg_start:seg_end);

    if length(I_clean) < 10
        r.f_sw_kHz=NaN; r.L_uH=NaN; r.Q=NaN; r.R_ohm=NaN;
        r.P_W=NaN; r.phi_deg=NaN; r.I_peak_A=NaN;
        r.Vdc_mean=NaN; r.I_RMS_A=NaN; return;
    end

    % Timing from clean segment
    dCNT_c = diff(CNT_clean); dCNT_pos = dCNT_c(dCNT_c > 0);
    if isempty(dCNT_pos)
        r.f_sw_kHz=NaN; r.L_uH=NaN; r.Q=NaN; r.R_ohm=NaN;
        r.P_W=NaN; r.phi_deg=NaN; r.I_peak_A=NaN;
        r.Vdc_mean=NaN; r.I_RMS_A=NaN; return;
    end
    avg_dcnt = mean(dCNT_pos);
    dtv = diff(t_clean); dtv(dtv <= 0) = [];
    avg_dt_us = mean(dtv);
    t_per_cnt_us = avg_dt_us / avg_dcnt;

    % CNT range from full segment (before FMAC trim)
    seg_full_CNT = CNT(best_seg(1):best_seg(2));
    wi = find(diff(seg_full_CNT) < 0, 1);
    if ~isempty(wi)
        CNT_range = seg_full_CNT(wi) + round(avg_dcnt) - seg_full_CNT(wi+1);
    else
        CNT_range = max(seg_full_CNT) - min(seg_full_CNT);
    end
    HRTIM_period_s = CNT_range * t_per_cnt_us * 1e-6;
    if HRTIM_period_s <= 0, HRTIM_period_s = 1/30000; end
    f_sw = 1/HRTIM_period_s;

    Tu = HRTIM_period_s * 1e6;
    D_U = max(0, (CO-CU) * t_per_cnt_us) / Tu;

    I_rms = rms(I_clean);
    Ipk = robust_peak(I_clean, 0);
    Vdc_mean = mean(Vdc_clean);

    % Phase detection on clean data
    phi_val = detect_phi(I_clean, CNT_clean, CU, Tu, t_per_cnt_us, Ipk);

    omega=2*pi*f_sw;
    if ~isnan(phi_val) && I_rms>0.1
        V1=(sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
        Z=V1/I_rms; X=Z*sind(phi_val);
        L=(1/(omega*C_FARAD)+X)/omega*1e6;
        R_val=Z*cosd(phi_val);
        fr=1/(2*pi*sqrt(L*1e-6*C_FARAD));
        o0=2*pi*fr;
        if R_val>0, Qv=o0*L*1e-6/R_val; else, Qv=NaN; end
    else
        L=NaN; Qv=NaN; R_val=NaN;
    end

    r.f_sw_kHz=f_sw/1e3; r.L_uH=L; r.Q=Qv; r.R_ohm=R_val;
    r.P_W=Vdc_mean*interp_at_cnt(CO,CNT_clean,I_clean)*D_U/2;
    r.phi_deg=phi_val; r.I_peak_A=Ipk;
    r.Vdc_mean=Vdc_mean; r.I_RMS_A=I_rms;
end

function phi_val = detect_phi(I, CNT, CU, Tu, t_per_cnt_us, Ipk)
    phi_val = NaN;
    cpc=Tu/t_per_cnt_us;
    nI=length(I);

    % Valley detection
    vi=[];
    i_start = max(2, 1);
    i_end = min(nI, nI-1);
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

    % Extrapolated zero-crossings
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
