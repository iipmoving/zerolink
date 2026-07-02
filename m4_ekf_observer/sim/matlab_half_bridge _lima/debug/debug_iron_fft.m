% debug_iron_fft — FFT提取铁锅基波分量, 重算 |Z| 和 φ
function debug_iron_fft()
cal.ADC_BITS = 12; cal.ADC_STEPS = 4096; cal.VREF = 3.3;
cal.V_SCALE = cal.VREF/cal.ADC_STEPS/(6.2e3/(270e3*3+6.2e3));
cal.I_SCALE = cal.VREF/cal.ADC_STEPS/(330/(10e3+330));
cal.VDC_SCALE = cal.V_SCALE;
C_FARAD = 0.94e-6;

% 铁锅高功率帧: 153424 f4
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

fprintf('=== 逐帧铁锅 FFT 分析 ===\n');
fprintf('Frame  f_sw[kHz]  L_time[uH]  L_fft[uH]  phi_time  phi_fft  THD[%%]\n');

for target_f = 0:5  % 帧0-5
    frame = frames{target_f+1};
    N = fN(target_f+1);
    t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
    CMP=frame(1,6:9);

    I_adc = min(max(I_adc, 0), 4095);
    Vdc_adc = min(max(Vdc_adc, 0), 4095);
    I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.VDC_SCALE;

    EDGE = 2;
    I_trim = I((EDGE+1):(N-EDGE));
    t_trim = t_us((EDGE+1):(N-EDGE));
    Vdc_trim = Vdc((EDGE+1):(N-EDGE));

    % 时域法参数
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

    % 时域法
    D_U = max(0,(CMP(2)-CMP(1))*t_per_cnt_us)/(HRTIM_period_s*1e6)*100;
    I_rms_time = rms(I_trim);
    Vdc_mean = mean(Vdc_trim);
    V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U/100);

    % FFT
    dt = mean(diff(t_trim))*1e-6;  % seconds
    n_fft = 512;
    I_fft = fft(I_trim - mean(I_trim), n_fft);
    I_mag = abs(I_fft(1:n_fft/2)) / length(I_trim) * 2;  % peak amplitude

    % 找基波 (f_sw附近最大峰)
    f_axis = (0:n_fft/2-1)/(n_fft*dt);
    [~, f_idx] = min(abs(f_axis - f_sw));
    % 搜索 f_idx 附近的最大值 (实际基波可能在 f_sw 附近的 bin)
    search_range = max(1,f_idx-2):min(length(I_mag),f_idx+2);
    [~, local_max] = max(I_mag(search_range));
    fund_idx = search_range(local_max);
    I_fund_peak = I_mag(fund_idx);
    I_fund_rms = I_fund_peak / sqrt(2);
    fund_freq = f_axis(fund_idx);

    % 基波相位 (相对参考: 从电流过零点提取)
    % FFT 的 phase 是相对 t=0 的, 但 t=0 不一定对齐 CMP_UON
    % 方法: 用电流基波的 arg 减去电压基波的 arg
    % 但我们没有电压波形, 只有 Vdc 平均值
    % 替代: 用时域 φ 检测同样的方法, 但用在重建的基波信号上
    I_fund_angle = angle(I_fft(fund_idx));  % radians at t=0

    % 重建基波时域信号
    t0 = t_trim(1)*1e-6;
    I_fund_t = I_fund_peak * cos(2*pi*fund_freq*(t_trim*1e-6 - t0) + I_fund_angle);

    % 基波过零点检测 (用于 φ)
    % 找到 I_fund_t 从负到正的过零点
    zc_fund = [];
    for i = 2:length(I_fund_t)
        if I_fund_t(i-1) <= 0 && I_fund_t(i) > 0
            frac = -I_fund_t(i-1) / (I_fund_t(i) - I_fund_t(i-1));
            zc_fund(end+1) = i - 1 + frac;  % fractional index
        end
    end

    % 如果没有过零点, 跳过
    if length(zc_fund) < 1
        continue;
    end

    % CMP_UON 对应的 CNT 值在时间轴上的位置
    CU = CMP(1);
    % 找到 CU 在 CNT 序列中的分数位置
    cu_frac_idx = NaN;
    for i = 2:length(CNT)
        c1 = CNT(i-1); c2 = CNT(i);
        if c1 <= CU && CU <= c2
            cu_frac_idx = (i-1) + (CU - c1)/(c2 - c1);
            break;
        end
    end
    % 处理CNT绕回
    if isnan(cu_frac_idx)
        for i = 2:length(CNT)
            if CNT(i-1) > CNT(i)  % wrap point
                if CU >= CNT(i-1) || CU <= CNT(i)
                    if CU >= CNT(i-1)
                        cu_frac_idx = (i-1) + (CU - CNT(i-1))/(CNT(i) + CNT_range - CNT(i-1));
                    else
                        cu_frac_idx = (i-1) + (CU + CNT_range - CNT(i-1))/(CNT(i) + CNT_range - CNT(i-1));
                    end
                    break;
                end
            end
        end
    end

    % φ from FFT fundamental: 第一个上升过零点到 CMP_UON 的时间差
    if ~isnan(cu_frac_idx) && ~isempty(zc_fund)
        zc_first = zc_fund(1);
        % 周期 (以index计)
        period_idx = zc_fund(end) - zc_fund(1);
        if length(zc_fund) >= 3
            period_idx = (zc_fund(end) - zc_fund(1)) / (length(zc_fund) - 1);
        end

        % CU 到过零点的距离 (索引差)
        dist_idx = zc_first - cu_frac_idx;
        if dist_idx < 0, dist_idx = dist_idx + period_idx; end
        phi_fft = (dist_idx / period_idx) * 360;
        phi_fft = mod(phi_fft, 360);
        if phi_fft > 180, phi_fft = phi_fft - 360; end
    else
        phi_fft = NaN;
    end

    % RLC from FFT
    omega = 2*pi*f_sw;
    if ~isnan(phi_fft) && I_fund_rms > 0.1
        Z_fund = V1 / I_fund_rms;
        R_fund = Z_fund * cosd(phi_fft);
        X_fund = Z_fund * sind(phi_fft);
        L_fund = (1/(omega*C_FARAD) + X_fund) / omega;
    else
        L_fund = NaN; Z_fund = NaN;
    end

    % 时域法 L (对比)
    % 用时域 φ 和时域 I_RMS
    % 重新计算时域 φ
    raw.t=t_us*1e-6; raw.I=I; raw.Vdc=Vdc; raw.CNT=CNT;
    raw.CMP=repmat(CMP,N,1); raw.t_us=t_us;
    raw.meta.edge_skip=2; raw.meta.t_per_cnt_us=t_per_cnt_us;
    raw.meta.HRTIM_period_s=HRTIM_period_s;
    raw.meta.I_robust_pk=robust_peak(I,2);

    timing.T_hrtim_us=HRTIM_period_s*1e6;
    timing.f_sw_kHz=f_sw/1e3;
    timing.CMP_UON=CMP(1)*ones(N,1); timing.CMP_UOFF=CMP(2)*ones(N,1);
    timing.CMP_LON=CMP(3)*ones(N,1); timing.CMP_LOFF=CMP(4)*ones(N,1);
    timing.D_U_pct=D_U*ones(N,1); timing.D_L_pct=zeros(N,1);
    waveform.I_RMS_all=I_rms_time; waveform.I_peak_all=max(I_trim);
    ph_time = calc_phase_mini(raw, timing);

    omega = 2*pi*f_sw;
    if ph_time.usable && I_rms_time > 0.1
        Z_time = V1 / I_rms_time;
        R_time = Z_time * cosd(ph_time.phi_deg);
        X_time = Z_time * sind(ph_time.phi_deg);
        L_time = (1/(omega*C_FARAD) + X_time) / omega;
    else
        L_time = NaN;
    end

    % THD
    harm_power = 0;
    for h = 2:10
        hidx = round(h * fund_idx);
        if hidx <= length(I_mag) && hidx > 0 && hidx ~= fund_idx
            harm_power = harm_power + I_mag(hidx)^2;
        end
    end
    THD = sqrt(harm_power) / I_mag(fund_idx) * 100;

    fprintf('%2d     %8.2f  %10.1f  %9.1f  %8.1f  %7.1f  %7.1f\n', ...
        target_f, f_sw/1e3, L_time*1e6, L_fund*1e6, ...
        ph_time.phi_deg, phi_fft, THD);
end
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
