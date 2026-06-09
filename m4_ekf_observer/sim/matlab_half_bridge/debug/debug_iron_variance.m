% debug_iron_variance — 铁锅同频段L方差根因分析
function debug_iron_variance()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;
cal.HRTIM_CLK_MHZ = 144;
C_FARAD = 0.94e-6;

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

base = '../../tools/ekf_tuner/captures/captures/';
all_r = [];

for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        r = calc_one_frame(fd{f}, fN{f}, cal, C_FARAD);
        if ~isnan(r.phi_deg) && r.I_peak_A > 2
            r.file = fidx; r.frame = f;
            all_r = [all_r; r];
        end
    end
end

fprintf('Total valid iron frames: %d\n', length(all_r));

%% 选最大频段做深度分析: 23-26kHz (样本最多)
mask = [all_r.f_sw_kHz] >= 23 & [all_r.f_sw_kHz] < 26;
band = all_r(mask);
fprintf('\n========== 23-26kHz band: N=%d ==========\n', sum(mask));

% 按L排序, 看extreme frames
[~, idx] = sort([band.L_uH]);
fprintf('\n--- Bottom 5 (lowest L) ---\n');
for i = 1:min(5,length(idx))
    r = band(idx(i));
    fprintf('L=%.1fuH  P=%.0fW  Vdc=%.0fV  phi=%.1fdeg  Ipk=%.2fA  f=%.2fkHz  D_U=%.3f  Q=%.2f  file=%d f=%d\n', ...
        r.L_uH, r.P_W, r.Vdc_mean, r.phi_deg, r.I_peak_A, r.f_sw_kHz, r.D_U, r.Q, r.file, r.frame);
end
fprintf('\n--- Top 5 (highest L) ---\n');
for i = max(1,length(idx)-4):length(idx)
    r = band(idx(i));
    fprintf('L=%.1fuH  P=%.0fW  Vdc=%.0fV  phi=%.1fdeg  Ipk=%.2fA  f=%.2fkHz  D_U=%.3f  Q=%.2f  file=%d f=%d\n', ...
        r.L_uH, r.P_W, r.Vdc_mean, r.phi_deg, r.I_peak_A, r.f_sw_kHz, r.D_U, r.Q, r.file, r.frame);
end

%% L vs 各参数的相关系数
fprintf('\n--- Pearson correlation: L vs each param (23-26kHz) ---\n');
vars = {'P_W','Vdc_mean','phi_deg','I_peak_A','f_sw_kHz','D_U','Q','I_RMS_A'};
Lv = [band.L_uH]';
for v = 1:length(vars)
    xv = [band.(vars{v})]';
    R = corrcoef(Lv, xv);
    fprintf('L vs %-12s: r=%.4f\n', vars{v}, R(1,2));
end

%% 关键: L公式分解 — 看Z和phi各自的贡献
fprintf('\n--- L formula decomposition (23-26kHz) ---\n');
fprintf('L = (1/(wC) + Z*sind(phi)) / w\n');
fprintf('Z = V1 / I_RMS\n');
fprintf('V1 = (sqrt(2)/pi)*Vdc*sin(pi*D_U)\n\n');

for i = 1:length(band)
    r = band(i);
    omega = 2*pi*r.f_sw_kHz*1e3;
    V1 = (sqrt(2)/pi)*r.Vdc_mean*sin(pi*r.D_U);
    Z_val = V1 / r.I_RMS_A;
    Xc = 1/(omega*C_FARAD);
    X = Z_val * sind(r.phi_deg);
    band(i).V1_calc = V1;
    band(i).Z_calc = Z_val;
    band(i).Xc = Xc;
    band(i).X_calc = X;
    band(i).L_from_Zphi = (Xc + X) / omega * 1e6;
end

% Z, phi, V1 各自的变异
Zv = [band.Z_calc];
phiv = [band.phi_deg];
V1v = [band.V1_calc];
Xv = [band.X_calc];
fprintf('Z:    mean=%.2f  std=%.2f  CV=%.1f%%\n', mean(Zv), std(Zv), std(Zv)/mean(Zv)*100);
fprintf('phi:  mean=%.1f  std=%.1f  CV=%.1f%%\n', mean(phiv), std(phiv), std(phiv)/mean(phiv)*100);
fprintf('V1:   mean=%.1f  std=%.1f  CV=%.1f%%\n', mean(V1v), std(V1v), std(V1v)/mean(V1v)*100);
fprintf('X=Z*sin(phi): mean=%.2f  std=%.2f  CV=%.1f%%\n', mean(Xv), std(Xv), std(Xv)/mean(Xv)*100);
fprintf('L:    mean=%.1f  std=%.1f  CV=%.1f%%\n', mean(Lv), std(Lv), std(Lv)/mean(Lv)*100);

%% Xc vs X 的贡献
fprintf('\n--- Capacitive vs Inductive reactance ---\n');
Xc_vals = [band.Xc];
fprintf('Xc=1/(wC): mean=%.2f  CV=%.1f%%\n', mean(Xc_vals), std(Xc_vals)/mean(Xc_vals)*100);
fprintf('X=Z*sin(phi): mean=%.2f  CV=%.1f%%\n', mean(Xv), std(Xv)/mean(Xv)*100);
fprintf('Xc dominates L: Xc/(Xc+X) = mean=%.3f\n', mean(Xc_vals ./ (Xc_vals + Xv)));

%% 看功率分档
fprintf('\n--- L by power bins (23-26kHz) ---\n');
Pv = [band.P_W];
P_edges = [0 200 500 1000 1500 2000 3000];
for b = 1:length(P_edges)-1
    pm = Pv >= P_edges(b) & Pv < P_edges(b+1);
    if sum(pm) >= 2
        Ls = Lv(pm);
        fprintf('P=%4d-%-4dW  N=%2d  L=%.1f+-%.1f uH  CV=%.1f%%  phi=%.1f+-%.1fdeg  Vdc=%.0f+-%.0fV\n', ...
            P_edges(b), P_edges(b+1), sum(pm), mean(Ls), std(Ls), std(Ls)/mean(Ls)*100, ...
            mean(phiv(pm)), std(phiv(pm)), mean([band(pm).Vdc_mean]), std([band(pm).Vdc_mean]));
    end
end

%% 排查 phi 检测: 看所有谷值候选的离散度
fprintf('\n--- Phi detection debug on extreme frames ---\n');
% 最低L帧和最高L帧, 重新跑phi检测, 看中间变量
extreme_idx = [idx(1) idx(end)];  % lowest and highest L
for ei = 1:2
    r = band(extreme_idx(ei));
    csv_path = [base iron_files{r.file}];
    [fd, fN] = parse_csv(csv_path);
    frame = fd{r.frame};
    N = fN{r.frame};

    debug_phi_detection(frame, N, cal, C_FARAD, r);
end

end

%% ===== Sub-functions =====

function r = calc_one_frame(frame, N, cal, C_FARAD)
    t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
    CMP=frame(1,6:9);

    I_adc = min(max(I_adc, 0), 4095);
    Vdc_adc = min(max(Vdc_adc, 0), 4095);
    I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.VDC_SCALE;
    EDGE=2;

    dCNT=diff(CNT); dCNT_pos=dCNT(dCNT>0);
    avg_dcnt=1; if ~isempty(dCNT_pos), avg_dcnt=mean(dCNT_pos); end
    wrap_idx=find(diff(CNT)<0,1);
    if ~isempty(wrap_idx), CNT_range=CNT(wrap_idx)+round(avg_dcnt)-CNT(wrap_idx+1);
    else, CNT_range=max(CNT)-min(CNT); end
    dtv=diff(t_us); dtv(dtv<=0)=[];
    avg_dt_us=0.5; if ~isempty(dtv), avg_dt_us=mean(dtv); end
    if avg_dcnt>0 && avg_dt_us>0, t_per_cnt_us=avg_dt_us/avg_dcnt;
    else, t_per_cnt_us=1/cal.HRTIM_CLK_MHZ; end
    HRTIM_period_s=CNT_range*t_per_cnt_us*1e-6;
    if HRTIM_period_s<=0, HRTIM_period_s=1/30000; end
    f_sw=1/HRTIM_period_s;

    Tu=HRTIM_period_s*1e6;
    CU=CMP(1); CO=CMP(2);
    D_U = max(0,(CO-CU)*t_per_cnt_us)/Tu;
    I_trim = I((EDGE+1):(N-EDGE));
    I_rms = rms(I_trim);
    Ipk = robust_peak(I, EDGE);
    Vdc_mean = mean(Vdc((EDGE+1):(N-EDGE)));

    % phi detection
    phi_val = detect_phi(I, CNT, CU, Tu, t_per_cnt_us, EDGE, Ipk);

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
    r.P_W=Vdc_mean*interp_at_cnt(CO,CNT,I)*D_U/2;
    r.phi_deg=phi_val; r.I_peak_A=Ipk;
    r.Vdc_mean=Vdc_mean; r.I_RMS_A=I_rms; r.D_U=D_U;
end

function phi_val = detect_phi(I, CNT, CU, Tu, t_per_cnt_us, EDGE, Ipk)
    phi_val = NaN;
    cpc=Tu/t_per_cnt_us;
    nI=length(I);

    % valley detection
    vi=[];
    for i=(EDGE+1):(nI-EDGE)
        if I(i-1)>I(i)&&I(i)<I(i+1)&&I(i)<Ipk*0.15, vi(end+1)=i; end
    end
    % parabola interpolation
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
    % extrapolate zero crossings
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

function debug_phi_detection(frame, N, cal, C_FARAD, r)
    t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
    CMP=frame(1,6:9);
    I_adc=min(max(I_adc,0),4095);
    I=I_adc*cal.I_SCALE;
    EDGE=2; Ipk=robust_peak(I,EDGE);

    % timing
    dCNT=diff(CNT); dCNT_pos=dCNT(dCNT>0);
    avg_dcnt=mean(dCNT_pos);
    wrap_idx=find(diff(CNT)<0,1);
    if ~isempty(wrap_idx), CNT_range=CNT(wrap_idx)+round(avg_dcnt)-CNT(wrap_idx+1);
    else, CNT_range=max(CNT)-min(CNT); end
    avg_dt_us=mean(diff(t_us(diff(t_us)>0)));
    t_per_cnt_us=avg_dt_us/avg_dcnt;
    HRTIM_period_s=CNT_range*t_per_cnt_us*1e-6;
    Tu=HRTIM_period_s*1e6;
    CU=CMP(1); CO=CMP(2);
    D_U=max(0,(CO-CU)*t_per_cnt_us)/Tu;
    cpc=Tu/t_per_cnt_us;
    nI=length(I);

    fprintf('\n--- Debug frame: L=%.1fuH  P=%.0fW  f=%.2fkHz  Vdc=%.0fV ---\n', ...
        r.L_uH, r.P_W, r.f_sw_kHz, r.Vdc_mean);
    fprintf('Ipk=%.2fA  I_RMS=%.2fA  D_U=%.3f\n', Ipk, r.I_RMS_A, D_U);

    % list all valley candidates
    vi=[];
    for i=(EDGE+1):(nI-EDGE)
        if I(i-1)>I(i)&&I(i)<I(i+1)&&I(i)<Ipk*0.15, vi(end+1)=i; end
    end
    fprintf('Valley candidates (I < %.2fA): %d found at indices: %s\n', Ipk*0.15, length(vi), mat2str(vi));

    % parabola interpolation results
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
        fprintf('  valley@idx=%d: I=%.3fA  raw_CNT=%d  interp_CNT=%.1f  phi=%.1fdeg  accept=%d\n', ...
            vi_k, I(vi_k), CNT(vi_k), cnt_interp, ph, ph>=8&&ph<=85);
    end

    % extrapolate zero crossing results
    pe=[];
    lo=Ipk*0.15; hi=Ipk*0.50;
    zc_count=0;
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
                    zc_count=zc_count+1;
                end
            end
        end
    end
    fprintf('Extrapolated zero-crossings: %d found\n', zc_count);
    if ~isempty(pe), fprintf('  phi values: %s\n', mat2str(pe,3)); end

    ap=[pv pe];
    ap_pos=ap(ap>0);
    fprintf('All phi candidates: %s\n', mat2str(ap,3));
    fprintf('Positive phi candidates: %s\n', mat2str(ap_pos,3));
    if ~isempty(ap_pos)
        fprintf('MEDIAN phi = %.1f deg\n', median(ap_pos));
    end
    fprintf('MEAN phi = %.1f deg\n', mean(ap));
    fprintf('STD phi = %.1f deg\n', std(ap));
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
