% debug_fmac_impact — 量化 FMAC 垃圾数据对所有帧的影响
function debug_fmac_impact()
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

base = '../../tools/ekf_tuner/';

fprintf('File                           Frame  N    FMAC_n  FMAC_positions\n');
fprintf('%s\n', repmat('-',1,100));

total_frames = 0;
frames_with_fmac = 0;
all_r_clean = [];
all_r_dirty = [];

for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        total_frames = total_frames + 1;
        frame = fd{f};
        N = fN{f};
        I_adc = frame(:,2);
        EDGE = 2;

        % Find FMAC garbage: I_adc > 4090 (ADC saturation) in trimmed range
        trim_range = (EDGE+1):(N-EDGE);
        fmac_idx = find(I_adc(trim_range) > 4090);

        if ~isempty(fmac_idx)
            frames_with_fmac = frames_with_fmac + 1;
            fprintf('%-30s  %2d    %3d  %3d     %s\n', ...
                iron_files{fidx}, f, N, length(fmac_idx), mat2str(fmac_idx(1:min(5,end))'));
        end

        % Calculate L with and without FMAC cleaning
        r_orig = calc_one_frame(frame, N, cal, C_FARAD);

        % Clean version: zero out FMAC samples in I_trim
        I_adc_clean = I_adc;
        I_adc_clean(I_adc > 4090) = 0;  % zero out garbage
        frame_clean = frame;
        frame_clean(:,2) = I_adc_clean;
        r_clean = calc_one_frame(frame_clean, N, cal, C_FARAD);

        if ~isnan(r_orig.phi_deg) && r_orig.I_peak_A > 2
            all_r_dirty = [all_r_dirty; r_orig];
            all_r_clean = [all_r_clean; r_clean];
        end
    end
end

fprintf('\nTotal frames: %d, with FMAC garbage: %d (%.1f%%)\n', ...
    total_frames, frames_with_fmac, frames_with_fmac/total_frames*100);

%% Compare dirty vs clean for frames with significant difference
fprintf('\n--- Frames where FMAC cleaning changes L by >3%% ---\n');
fprintf('File                              Fr  L_dirty  L_clean  Delta%%  phi_d  phi_c  I_RMS_d  I_RMS_c\n');
count = 0;
for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        frame = fd{f};
        N = fN{f};

        r_orig = calc_one_frame(frame, N, cal, C_FARAD);
        I_adc_clean = frame(:,2);
        I_adc_clean(I_adc_clean > 4090) = 0;
        frame_clean = frame;
        frame_clean(:,2) = I_adc_clean;
        r_clean = calc_one_frame(frame_clean, N, cal, C_FARAD);

        if ~isnan(r_orig.phi_deg) && r_orig.I_peak_A > 2
            delta = abs(r_clean.L_uH - r_orig.L_uH) / r_clean.L_uH * 100;
            if delta > 3
                count = count + 1;
                fprintf('%-33s  %2d  %7.1f  %7.1f  %5.1f  %5.1f  %5.1f  %7.3f  %7.3f\n', ...
                    iron_files{fidx}, f, r_orig.L_uH, r_clean.L_uH, delta, ...
                    r_orig.phi_deg, r_clean.phi_deg, r_orig.I_RMS_A, r_clean.I_RMS_A);
            end
        end
    end
end
fprintf('Frames with >3%% L change: %d / %d\n', count, length(all_r_clean));

%% Overall stats: clean vs dirty
fprintf('\n--- Overall L stats (23-26kHz): dirty vs clean ---\n');
mask_d = [all_r_dirty.f_sw_kHz] >= 23 & [all_r_dirty.f_sw_kHz] < 26;
mask_c = [all_r_clean.f_sw_kHz] >= 23 & [all_r_clean.f_sw_kHz] < 26;

Ld = [all_r_dirty(mask_d).L_uH];
Lc = [all_r_clean(mask_c).L_uH];
fprintf('Dirty: mean=%.1f  std=%.1f  CV=%.1f%%  N=%d\n', mean(Ld), std(Ld), std(Ld)/mean(Ld)*100, sum(mask_d));
fprintf('Clean: mean=%.1f  std=%.1f  CV=%.1f%%  N=%d\n', mean(Lc), std(Lc), std(Lc)/mean(Lc)*100, sum(mask_c));

% Also compare within power bins for clean version
fprintf('\n--- Clean L by power bins (23-26kHz) ---\n');
band = all_r_clean(mask_c);
Pv = [band.P_W];
Lv = [band.L_uH]';
P_edges = [0 200 500 1000 1500 2000 3000];
for b = 1:length(P_edges)-1
    pm = Pv >= P_edges(b) & Pv < P_edges(b+1);
    if sum(pm) >= 2
        Ls = Lv(pm);
        fprintf('P=%4d-%-4dW  N=%2d  L=%.1f+-%.1f uH  CV=%.1f%%\n', ...
            P_edges(b), P_edges(b+1), sum(pm), mean(Ls), std(Ls), std(Ls)/mean(Ls)*100);
    end
end
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
    phi_val = detect_phi(I, CNT, CU, Tu, t_per_cnt_us, EDGE, Ipk);
    omega=2*pi*f_sw;
    if ~isnan(phi_val) && I_rms>0.1
        V1=(sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
        Z=V1/I_rms; X=Z*sind(phi_val);
        L=(1/(omega*C_FARAD)+X)/omega*1e6;
    else
        L=NaN;
    end
    r.f_sw_kHz=f_sw/1e3; r.L_uH=L;
    r.P_W=Vdc_mean*interp_at_cnt(CO,CNT,I)*D_U/2;
    r.phi_deg=phi_val; r.I_peak_A=Ipk;
    r.Vdc_mean=Vdc_mean; r.I_RMS_A=I_rms; r.D_U=D_U;
end

function phi_val = detect_phi(I, CNT, CU, Tu, t_per_cnt_us, EDGE, Ipk)
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
