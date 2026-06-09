% debug_iron_fft2 — FFT提取基波I_RMS, φ保持时域法, 分离谐波效应
function debug_iron_fft2()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
C_FARAD = 0.94e-6;

csv_path = '../../tools/ekf_tuner/captures/capture_20260602_153424.csv';
fid = fopen(csv_path,'r');
fgetl(fid);
frames={}; fN=[];
while ~feof(fid)
    line=fgetl(fid); if line==-1, break; end
    line=strtrim(line); if isempty(line), continue; end
    if startsWith(line,'SIZE')
        parts=strsplit(line,','); N=str2double(parts{2}); fN(end+1)=N;
        fr=zeros(N,11);
        for i=1:N
            dl=fgetl(fid); dl=strtrim(dl);
            if isempty(dl), i=i-1; continue; end
            parts=strsplit(dl,',');
            for j=1:min(length(parts),11)
                v=str2double(parts{j}); if ~isnan(v), fr(i,j)=v; end
            end
        end
        frames{end+1}=fr;
    end
end
fclose(fid);

fprintf('Frame  f_sw[kHz]  I_RMS_total  I_RMS_fund  ratio   L_total[uH]  L_fund[uH]  phi[deg]  THD[%%]\n');
fprintf('%s\n', repmat('-',1,95));

for target_f = 0:5
    frame = frames{target_f+1};
    N = fN(target_f+1);
    t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
    CMP=frame(1,6:9);

    I_adc = min(max(I_adc, 0), 4095);
    Vdc_adc = min(max(Vdc_adc, 0), 4095);
    I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.V_SCALE;

    EDGE = 2;
    I_trim = I((EDGE+1):(N-EDGE));
    t_trim = t_us((EDGE+1):(N-EDGE));
    Vdc_mean = mean(Vdc((EDGE+1):(N-EDGE)));

    % Timing
    dCNT=diff(CNT); dCNT_pos=dCNT(dCNT>0);
    avg_dcnt=mean(dCNT_pos);
    wrap_idx=find(diff(CNT)<0,1);
    if ~isempty(wrap_idx)
        CNT_range=CNT(wrap_idx)+round(avg_dcnt)-CNT(wrap_idx+1);
    else, CNT_range=max(CNT)-min(CNT); end
    dtv=diff(t_us); dtv(dtv<=0)=[];
    avg_dt_us=mean(dtv);
    t_per_cnt_us=avg_dt_us/avg_dcnt;
    HRTIM_period_s=CNT_range*t_per_cnt_us*1e-6;
    f_sw=1/HRTIM_period_s;
    D_U = max(0,(CMP(2)-CMP(1))*t_per_cnt_us)/(HRTIM_period_s*1e6);

    % V1_RMS
    V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
    omega = 2*pi*f_sw;

    % 时域 I_RMS
    I_rms_total = rms(I_trim);

    % FFT 基波 I_RMS
    dt = mean(diff(t_trim))*1e-6;
    n_fft = 512;
    I_demean = I_trim - mean(I_trim);
    I_fft = fft(I_demean, n_fft);
    I_mag = abs(I_fft(1:n_fft/2)) / length(I_trim) * 2;
    f_axis = (0:n_fft/2-1)/(n_fft*dt);

    % 找 f_sw 附近的基波峰
    [~, f_idx] = min(abs(f_axis - f_sw));
    sr = max(1,f_idx-2):min(length(I_mag),f_idx+2);
    [~, lm] = max(I_mag(sr));
    fund_idx = sr(lm);
    I_fund_peak = I_mag(fund_idx);
    I_rms_fund = I_fund_peak / sqrt(2);

    % THD (排除基波附近±1 bin)
    harm_power = 0;
    for i = 1:length(I_mag)
        if abs(i - fund_idx) > 1
            harm_power = harm_power + I_mag(i)^2;
        end
    end
    THD = sqrt(harm_power) / I_mag(fund_idx) * 100;

    % 时域 φ
    raw.t=t_us*1e-6; raw.I=I; raw.Vdc=Vdc; raw.CNT=CNT;
    raw.CMP=repmat(CMP,N,1); raw.t_us=t_us;
    raw.meta.edge_skip=EDGE; raw.meta.t_per_cnt_us=t_per_cnt_us;
    raw.meta.HRTIM_period_s=HRTIM_period_s;
    timing.T_hrtim_us=HRTIM_period_s*1e6; timing.f_sw_kHz=f_sw/1e3;
    timing.CMP_UON=CMP(1)*ones(N,1); timing.CMP_UOFF=CMP(2)*ones(N,1);
    timing.CMP_LON=CMP(3)*ones(N,1); timing.CMP_LOFF=CMP(4)*ones(N,1);
    ph = calc_phase_mini(raw, timing);

    % L with total I_RMS
    if ph.usable && I_rms_total > 0.1
        Z_total = V1 / I_rms_total;
        X_total = Z_total * sind(ph.phi_deg);
        L_total = (1/(omega*C_FARAD) + X_total) / omega * 1e6;
    else
        L_total = NaN;
    end

    % L with fundamental I_RMS
    if ph.usable && I_rms_fund > 0.1
        Z_fund = V1 / I_rms_fund;
        X_fund = Z_fund * sind(ph.phi_deg);
        L_fund = (1/(omega*C_FARAD) + X_fund) / omega * 1e6;
    else
        L_fund = NaN;
    end

    fprintf('%2d     %8.2f  %11.3f  %11.3f  %5.2f  %11.1f  %10.1f  %8.1f  %7.1f\n', ...
        target_f, f_sw/1e3, I_rms_total, I_rms_fund, I_rms_fund/I_rms_total, ...
        L_total, L_fund, ph.phi_deg, THD);
end
fprintf('\n电桥参考: L=65uH@30kHz\n');
end

function pk=robust_peak(I,edge_skip)
if nargin<2, edge_skip=0; end
n=length(I);
if n<=2*edge_skip, pk=max(I); return; end
It=I((edge_skip+1):(n-edge_skip));
Is=sort(It); idx=max(1,round(length(Is)*0.98)); pk=Is(idx);
end

function ph=calc_phase_mini(raw,timing)
I=raw.I; CNT=raw.CNT;
if timing.CMP_UON(1)==0, ph.phi_deg=NaN; ph.cos_phi=NaN; ph.usable=false; return; end
CU=timing.CMP_UON(1); Tu=timing.T_hrtim_us;
Ipk=robust_peak(I,raw.meta.edge_skip); n=length(I); cpc=Tu/raw.meta.t_per_cnt_us;
edge=raw.meta.edge_skip;
vi=[]; for i=(edge+1):(n-edge)
    if I(i-1)>I(i)&&I(i)<I(i+1)&&I(i)<Ipk*0.15, vi(end+1)=i; end
end
pv=compute_phi_from_indices(vi,I,CNT,CU,cpc);
pe=extrapolate_zero_crossings(I,CNT,Ipk,CU,cpc);
ap=[pv pe]; if isempty(ap), ph.phi_deg=NaN; ph.cos_phi=NaN; ph.usable=false; return; end
ap_pos=ap(ap>0); if ~isempty(ap_pos), pa=median(ap_pos); else, pa=median(ap); end
ph.phi_deg=pa; ph.cos_phi=cosd(pa); ph.usable=true;
end

function pl=compute_phi_from_indices(idx,I,CNT,CU,cpc)
pl=[]; for k=1:length(idx)
    vi=idx(k); cnt_interp=CNT(vi);
    if vi>1 && vi<length(I)
        y_lo=I(vi-1); y_mid=I(vi); y_hi=I(vi+1);
        denom=y_lo+y_hi-2*y_mid;
        if denom>0.01
            frac=(y_hi-y_lo)/(2*denom);
            frac=max(-0.5,min(0.5,frac));
            if frac>=0, cnt_interp=CNT(vi)+frac*(CNT(vi+1)-CNT(vi));
            else, cnt_interp=CNT(vi)+frac*(CNT(vi)-CNT(vi-1)); end
        end
    end
    dc=cnt_interp-CU; if dc<0, dc=dc+cpc; end
    ph=(dc/cpc)*360; ph=mod(ph,360); if ph>180, ph=ph-360; end
    if ph>=8&&ph<=85, pl(end+1)=ph; end
end; end

function pl=extrapolate_zero_crossings(I,CNT,Ipk,CU,cpc)
pl=[]; if Ipk<1, return; end
n=length(I); lo=Ipk*0.15; hi=Ipk*0.50;
for i=3:n
    if I(i-1)<lo&&I(i)>=lo
        sp=[]; for j=i:min(n,i+10)
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
                if ph>=8&&ph<=85, pl(end+1)=ph; end
            end
        end
    end
end; end
