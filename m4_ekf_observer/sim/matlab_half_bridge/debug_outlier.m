% debug_outlier — 检查 L=59.7uH 异常帧的波形和 phi 检测
function debug_outlier()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;
cal.HRTIM_CLK_MHZ = 144;
C_FARAD = 0.94e-6;

base = '../../tools/ekf_tuner/';

% 异常帧: file=12(capture_20260602_153400.csv) frame=2, L=59.7uH
% 对照帧: 选一个同频段正常帧
% 先找出所有帧的L,确认异常帧位置
iron_files = {
    'capture_20260602_153307.csv'  % 1
    'capture_20260602_153311.csv'  % 2
    'capture_20260602_153314.csv'  % 3
    'capture_20260602_153316.csv'  % 4
    'capture_20260602_153326.csv'  % 5
    'capture_20260602_153329.csv'  % 6
    'capture_20260602_153336.csv'  % 7
    'capture_20260602_153338.csv'  % 8
    'capture_20260602_153340.csv'  % 9
    'capture_20260602_153347.csv'  % 10
    'capture_20260602_153348.csv'  % 11
    'capture_20260602_153358.csv'  % 12
    'capture_20260602_153400.csv'  % 13
    'capture_20260602_153410.csv'  % 14
    'capture_20260602_153412.csv'  % 15
    'capture_20260602_153413.csv'  % 16
    'capture_20260602_153421.csv'  % 17
    'capture_20260602_153422.csv'  % 18
    'capture_20260602_153424.csv'  % 19
};

% 异常帧: file=12 (153358), frame=2  L=59.7uH P=549W
% 找同文件的其他帧做对比
csv_path = [base iron_files{12}];
[fd, fN] = parse_csv(csv_path);
fprintf('File 12 (153358) has %d frames\n', length(fd));
for f = 1:length(fd)
    r = calc_one_frame(fd{f}, fN{f}, cal, C_FARAD);
    fprintf('  Frame %d: L=%.1fuH  P=%.0fW  Vdc=%.0fV  phi=%.1fdeg  Ipk=%.2fA  f=%.2fkHz  D_U=%.3f\n', ...
        f, r.L_uH, r.P_W, r.Vdc_mean, r.phi_deg, r.I_peak_A, r.f_sw_kHz, r.D_U);
end

% 对比: 找 file=12 里 L 正常的一帧 (比如 frame 5 或 6)
% 和异常帧 frame 2 波形对比
fprintf('\n========== Frame 2 (outlier) vs Frame 4 (normal) waveform ==========\n');

for target = [2 4]
    frame = fd{target};
    N = fN{target};
    t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
    CMP=frame(1,6:9);

    I_adc=min(max(I_adc,0),4095);
    I=I_adc*cal.I_SCALE;
    EDGE=2;
    Ipk=robust_peak(I,EDGE);

    % Timing
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

    % Dump I waveform for manual inspection
    fprintf('\n--- Frame %d: N=%d samples, Ipk=%.2fA, D_U=%.3f, Tu=%.1fus, cpc=%.1f ---\n', ...
        target, N, Ipk, D_U, Tu, cpc);
    fprintf('CNT range: [%d, %d]\n', min(CNT), max(CNT));
    fprintf('CU=%d, CO=%d\n', CU, CO);

    % Check for FMAC garbage (0xFFFD ~= 65533)
    fmac_bad = find(I_adc > 4090, 10);
    if ~isempty(fmac_bad)
        fprintf('FMAC garbage at indices: %s (I_adc=%s)\n', ...
            mat2str(fmac_bad), mat2str(I_adc(fmac_bad)'));
    end

    % Check valley detection quality
    nI=length(I);
    vi=[];
    for i=(EDGE+1):(nI-EDGE)
        if I(i-1)>I(i)&&I(i)<I(i+1)&&I(i)<Ipk*0.15, vi(end+1)=i; end
    end
    fprintf('Valleys detected: %d at indices: %s\n', length(vi), mat2str(vi));

    % Show all valleys with their I values
    for k = 1:length(vi)
        idx_v = vi(k);
        fprintf('  idx=%d: I=%.4fA, neighbors I[%d]=%.4f, I[%d]=%.4f, I[%d]=%.4f\n', ...
            idx_v, I(idx_v), idx_v-1, I(idx_v-1), idx_v, I(idx_v), idx_v+1, I(idx_v+1));
    end

    % Show I around suspect extra valleys (indices 80-100 for outlier)
    if target == 2
        fprintf('\nI waveform around idx 80-100:\n');
        for i = 80:min(100, nI)
            is_valley = any(vi == i);
            fprintf('  idx=%d: I=%.4fA %s\n', i, I(i), ternary(is_valley, '<<<VALLEY', ''));
        end
    end

    % Rerun phi with full debug: show ALL phi candidates
    phi_val = detect_phi_verbose(I, CNT, CU, Tu, t_per_cnt_us, EDGE, Ipk);
end

end

function s = ternary(cond, t, f)
    if cond, s = t; else, s = f; end
end

function phi_val = detect_phi_verbose(I, CNT, CU, Tu, t_per_cnt_us, EDGE, Ipk)
    phi_val = NaN;
    cpc=Tu/t_per_cnt_us;
    nI=length(I);

    vi=[];
    for i=(EDGE+1):(nI-EDGE)
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
        accept = ph>=8&&ph<=85;
        if accept, pv(end+1)=ph; end
        fprintf('  Valley phi: idx=%d I=%.4f rawCNT=%d interpCNT=%.1f phi=%.1fdeg accept=%d\n', ...
            vi_k, I(vi_k), CNT(vi_k), cnt_interp, ph, accept);
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
                    accept = ph>=8&&ph<=85;
                    if accept, pe(end+1)=ph; end
                    fprintf('  Extrap phi: cz=%.1f ph=%.1fdeg accept=%d\n', cz, ph, accept);
                end
            end
        end
    end

    ap=[pv pe];
    fprintf('  All candidates: %s\n', mat2str(ap,3));
    if ~isempty(ap)
        ap_pos=ap(ap>0);
        if ~isempty(ap_pos), phi_val=median(ap_pos);
        else, phi_val=median(ap); end
    end
    fprintf('  Final phi = %.1f deg (median of positive)\n', phi_val);
end

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
    phi_val = detect_phi_basic(I, CNT, CU, Tu, t_per_cnt_us, EDGE, Ipk);
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
    r.f_sw_kHz=f_sw/1e3; r.L_uH=L; r.Q=Qv;
    r.P_W=Vdc_mean*interp_at_cnt(CO,CNT,I)*D_U/2;
    r.phi_deg=phi_val; r.I_peak_A=Ipk;
    r.Vdc_mean=Vdc_mean; r.I_RMS_A=I_rms; r.D_U=D_U;
end

function phi_val = detect_phi_basic(I, CNT, CU, Tu, t_per_cnt_us, EDGE, Ipk)
    phi_val = NaN;
    cpc=Tu/t_per_cnt_us;
    nI=length(I);
    vi=[];
    for i=(EDGE+1):(nI-EDGE)
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
