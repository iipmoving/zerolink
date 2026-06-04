% compare_3files — 直接调用 run_per_frame 内部逻辑，内存中对比三个文件
% 避免 CSV 文本解析问题

function compare_3files()
cal.ADC_BITS = 12;
cal.ADC_STEPS = 4096;
cal.VREF = 3.3;
cal.V_SCALE = cal.VREF / cal.ADC_STEPS / (6.2e3 / (270e3*3 + 6.2e3));
cal.I_SCALE = cal.VREF / cal.ADC_STEPS / (330 / (10e3 + 330));
cal.VDC_SCALE = cal.V_SCALE;
cal.MIN_PULSE_US = 6;
cal.F_AC = 50;
cal.HRTIM_CLK_MHZ = 144;
cal.V_AC_RMS = 220;
cal.V_AC_PK = 220 * sqrt(2);

base = '../../tools/ekf_tuner/';
csvs = {'capture_20260602_104849.csv', ...
        'capture_20260602_104850.csv', ...
        'capture_20260602_104852.csv'};
labels = {'104849', '104850', '104852'};

all_results = {};  % {file}{frame}.struct

for fidx = 1:length(csvs)
    csv_path = [base csvs{fidx}];
    [frames_data, frames_N, header_line, n_input_cols] = parse_csv(csv_path);
    results = {};
    for f = 1:length(frames_data)
        r = calc_one_frame(frames_data{f}, frames_N{f}, cal);
        results{end+1} = r;
    end
    all_results{fidx} = results;
end

%% 逐帧对比
fprintf('\n========== 三文件逐帧对比 ==========\n');
fprintf('%-6s %-4s %-8s %-8s %-7s %-8s %-8s %-6s %-6s\n', ...
        'File','Frm','f_sw','L_uH','Q','R_ohm','P_W','phi','Ipeak');
fprintf('%s\n', repmat('-', 1, 75));

for fidx = 1:length(all_results)
    for fi = 1:length(all_results{fidx})
        r = all_results{fidx}{fi};
        fprintf('%-6s %-4d %8.2f %8.1f %7.2f %8.2f %8.1f %6.1f %6.1f\n', ...
                labels{fidx}, fi-1, r.f_sw_kHz, r.L_uH, r.Q, r.R_ohm, r.P_W, r.phi_deg, r.I_peak_A);
    end
    fprintf('\n');
end

%% 稳定帧统计
fprintf('========== 稳定帧统计 (phi有效, Ipeak>2A) ==========\n');
fprintf('%-6s %-5s %-12s %-12s %-10s %-10s %-10s\n', ...
        'File','N','f_sw_kHz','L_uH','Q','P_W','phi_deg');
fprintf('%s\n', repmat('-', 1, 75));

for fidx = 1:length(all_results)
    f_sws=[]; Ls=[]; Qs=[]; Ps=[]; phis=[];
    for fi = 1:length(all_results{fidx})
        r = all_results{fidx}{fi};
        if ~isnan(r.phi_deg) && r.I_peak_A > 2
            f_sws(end+1)=r.f_sw_kHz; Ls(end+1)=r.L_uH;
            Qs(end+1)=r.Q; Ps(end+1)=r.P_W; phis(end+1)=r.phi_deg;
        end
    end
    if ~isempty(f_sws)
        fprintf('%-6s %-5d %8.2f+-%-5.1f %8.1f+-%-5.1f %7.2f+-%-5.2f %8.1f+-%-6.1f %6.1f+-%-4.1f\n', ...
                labels{fidx}, length(f_sws), ...
                mean(f_sws), std(f_sws), mean(Ls), std(Ls), ...
                mean(Qs), std(Qs), mean(Ps), std(Ps), mean(phis), std(phis));
    else
        fprintf('%-6s (无稳定帧)\n', labels{fidx});
    end
end

%% 104850 vs 104852 偏差
fprintf('\n========== 104850 vs 104852 逐帧偏差 ==========\n');
d1 = all_results{2}; d2 = all_results{3};
n = min(length(d1), length(d2));
fprintf('Frm  d_f_sw   d_L    d_Q    d_P     d_phi\n');
for fi = 1:n
    r1=d1{fi}; r2=d2{fi};
    fprintf('%2d   %+6.2f %+7.1f %+6.2f %+8.1f %+7.1f\n', fi-1, ...
        r1.f_sw_kHz-r2.f_sw_kHz, r1.L_uH-r2.L_uH, ...
        r1.Q-r2.Q, r1.P_W-r2.P_W, r1.phi_deg-r2.phi_deg);
end
end

%% ---- 解析CSV (同 run_per_frame 的解析逻辑) ----
function [frames_data, frames_N, header_line, n_input_cols] = parse_csv(csv_path)
fid = fopen(csv_path, 'r');
if fid < 0, error('无法打开: %s', csv_path); end
header_line = fgetl(fid);
n_input_cols = length(strsplit(header_line, ','));
frames_data = {};
frames_N = {};

while ~feof(fid)
    line = fgetl(fid);
    if line == -1, break; end
    line = strtrim(line);
    if isempty(line), continue; end
    if startsWith(line, 'SIZE')
        parts = strsplit(line, ',');
        N = str2double(parts{2});
        frames_N{end+1} = N;
        frame_rows = zeros(N, n_input_cols);
        for i = 1:N
            dline = fgetl(fid);
            if dline == -1, break; end
            dline = strtrim(dline);
            if isempty(dline)
                i = i - 1; continue;
            end
            parts = strsplit(dline, ',');
            for j = 1:min(length(parts), n_input_cols)
                val = str2double(parts{j});
                if ~isnan(val), frame_rows(i, j) = val; end
            end
        end
        frames_data{end+1} = frame_rows;
    end
end
fclose(fid);
end

%% ---- 单帧计算 (同 run_per_frame 内部逻辑, 精简返回) ----
function r = calc_one_frame(frame, N, cal)
t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
CMP=frame(1,6:9);
t=t_us*1e-6; I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.VDC_SCALE;

raw.t=t; raw.I=I; raw.V=Vdc; raw.Vdc=Vdc; raw.CNT=CNT;
raw.CMP=repmat(CMP,N,1); raw.I_adc=I_adc; raw.Vdc_adc=Vdc_adc; raw.t_us=t_us;

% CNT 周期
dCNT=diff(CNT); dCNT_pos=dCNT(dCNT>0);
avg_dcnt = isempty(dCNT_pos)*1 + ~isempty(dCNT_pos)*mean(dCNT_pos);
wrap_idx=find(diff(CNT)<0,1);
if ~isempty(wrap_idx)
    CNT_range=CNT(wrap_idx)+round(avg_dcnt)-CNT(wrap_idx+1);
else
    CNT_range=max(CNT)-min(CNT);
end
dt_us_vec=diff(t_us); dt_us_vec(dt_us_vec<=0)=[];
avg_dt_us = isempty(dt_us_vec)*0.5 + ~isempty(dt_us_vec)*mean(dt_us_vec);
t_per_cnt_us = (avg_dcnt>0 && avg_dt_us>0)*avg_dt_us/avg_dcnt + ~(avg_dcnt>0 && avg_dt_us>0)/(cal.HRTIM_CLK_MHZ);
HRTIM_period_s=CNT_range*t_per_cnt_us*1e-6;
if HRTIM_period_s<=0, HRTIM_period_s=1/30000; end

raw.meta.t_per_cnt_us=t_per_cnt_us;
raw.meta.HRTIM_period_s=HRTIM_period_s;

try
    timing=calc_timing_mini(raw,cal);
    phase_info=calc_phase_mini(raw,timing);
    rlc=calc_rlc_mini(raw,[],timing,phase_info);
    power=calc_power_mini(raw,[],timing,phase_info);

    r.f_sw_kHz=timing.f_sw_kHz;
    r.DT1_us=mean(timing.DT1_us); r.DT2_us=mean(timing.DT2_us);
    r.D_U_pct=mean(timing.D_U_pct); r.D_L_pct=mean(timing.D_L_pct);
    r.ctrl_mode=timing.ctrl_mode;
    r.I_peak_A=max(I); r.I_RMS_A=rms(I);
    r.phi_deg=phase_info.phi_deg; r.cos_phi=phase_info.cos_phi;
    r.P_W=power.P; r.L_uH=rlc.L_uH; r.Q=rlc.Q;
    r.R_ohm=rlc.R_ohm; r.f_res_kHz=rlc.f_res_kHz;
catch ME
    r.f_sw_kHz=1/(HRTIM_period_s)/1e3;
    r.DT1_us=NaN; r.DT2_us=NaN; r.D_U_pct=NaN; r.D_L_pct=NaN;
    r.ctrl_mode='?'; r.I_peak_A=max(I); r.I_RMS_A=rms(I);
    r.phi_deg=NaN; r.cos_phi=NaN; r.P_W=NaN; r.L_uH=NaN;
    r.Q=NaN; r.R_ohm=NaN; r.f_res_kHz=NaN;
end
end

%% ---- 计算管线 (从 run_per_frame.m 复制) ----
function timing = calc_timing_mini(raw, cal)
t=raw.t; CNT=raw.CNT; CMP=raw.CMP;
t_per_cnt=raw.meta.t_per_cnt_us;
CMP_UON=CMP(:,1); CMP_UOFF=CMP(:,2);
CMP_LON=CMP(:,3); CMP_LOFF=CMP(:,4);
T_hrtim_us=raw.meta.HRTIM_period_s*1e6;
if T_hrtim_us<=0, T_hrtim_us=raw.meta.CNT_range*t_per_cnt; end
T_UON_us=max(0,(CMP_UOFF-CMP_UON)*t_per_cnt);
T_LON_us=max(0,(CMP_LOFF-CMP_LON)*t_per_cnt);
D_U_pct=T_UON_us./T_hrtim_us*100;
D_L_pct=T_LON_us./T_hrtim_us*100;
DT1_us_raw=max(0,(CMP_LON-CMP_UOFF)*t_per_cnt);
DT2_us_raw=(CMP_UON-CMP_LOFF)*t_per_cnt;
for i=1:length(DT2_us_raw)
    if DT2_us_raw(i)<0, DT2_us_raw(i)=DT2_us_raw(i)+T_hrtim_us; end
end
D_std=std(D_U_pct); mean_D=mean(D_U_pct);
if D_std<2 && mean_D>45, ctrl_mode='FM'; else, ctrl_mode='PWM'; end
timing.CMP_UON=CMP_UON; timing.CMP_UOFF=CMP_UOFF;
timing.CMP_LON=CMP_LON; timing.CMP_LOFF=CMP_LOFF;
timing.T_UON_us=T_UON_us; timing.T_LON_us=T_LON_us;
timing.T_hrtim_us=T_hrtim_us;
timing.f_sw_kHz=1/(T_hrtim_us*1e-6)/1e3;
timing.D_U_pct=D_U_pct; timing.D_L_pct=D_L_pct;
timing.DT1_us=DT1_us_raw; timing.DT2_us=DT2_us_raw;
timing.D_std=D_std; timing.ctrl_mode=ctrl_mode;
end

function phase_info = calc_phase_mini(raw, timing)
I=raw.I; CNT=raw.CNT;
if timing.CMP_UON(1)==0
    phase_info.phi_deg=NaN; phase_info.cos_phi=NaN;
    phase_info.usable=false; return;
end
CMP_UON=timing.CMP_UON(1);
T_sw_us=timing.T_hrtim_us;
I_peak_est=max(I); n=length(I);
cnt_per_cycle=T_sw_us/raw.meta.t_per_cnt_us;

valley_idx=[];
for i=2:(n-1)
    if I(i-1)>I(i) && I(i)<I(i+1) && I(i)<I_peak_est*0.15
        valley_idx(end+1)=i;
    end
end
phi_valley=compute_phi(valley_idx,I,CNT,CMP_UON,cnt_per_cycle);
phi_extrap=extrapolate_zeros(I,CNT,I_peak_est,CMP_UON,cnt_per_cycle);
all_phi=[phi_valley, phi_extrap];
if isempty(all_phi)
    phase_info.phi_deg=NaN; phase_info.cos_phi=NaN;
    phase_info.usable=false; return;
end
phi_pos=all_phi(all_phi>0);
if ~isempty(phi_pos), phi_avg=median(phi_pos);
else, phi_avg=median(all_phi); end
phase_info.phi_deg=phi_avg;
phase_info.cos_phi=cosd(phi_avg);
phase_info.usable=true;
end

function phi_list=compute_phi(indices,I,CNT,CMP_UON,cnt_per_cycle)
% 谷值相位检测, 带抛物线插值突破0.5μs采样分辨率
phi_list=[];
for k=1:length(indices)
    vi=indices(k);
    cnt_interp=CNT(vi);
    if vi>1 && vi<length(I)
        y_lo=I(vi-1); y_mid=I(vi); y_hi=I(vi+1);
        denom=y_lo+y_hi-2*y_mid;
        if denom>0.01
            frac=(y_hi-y_lo)/(2*denom);
            frac=max(-0.5,min(0.5,frac));
            if frac>=0
                cnt_interp=CNT(vi)+frac*(CNT(vi+1)-CNT(vi));
            else
                cnt_interp=CNT(vi)+frac*(CNT(vi)-CNT(vi-1));
            end
        end
    end
    dist_cnt=cnt_interp-CMP_UON;
    if dist_cnt<0, dist_cnt=dist_cnt+cnt_per_cycle; end
    phi_deg=(dist_cnt/cnt_per_cycle)*360;
    phi_deg=mod(phi_deg,360);
    if phi_deg>180, phi_deg=phi_deg-360; end
    if phi_deg>=8 && phi_deg<=85, phi_list(end+1)=phi_deg; end
end
end

function phi_list=extrapolate_zeros(I,CNT,I_peak,CMP_UON,cnt_per_cycle)
phi_list=[];
if I_peak<1.0, return; end
n=length(I); I_lo=I_peak*0.15; I_hi=I_peak*0.50;
for i=3:n
    if I(i-1)<I_lo && I(i)>=I_lo
        slope_pts=[];
        for j=i:min(n,i+10)
            if I(j)>=I_lo && I(j)<=I_hi, slope_pts(end+1)=j; end
            if I(j)>I_hi, break; end
        end
        if length(slope_pts)>=2
            p1=slope_pts(1); p2=slope_pts(end);
            dI=I(p2)-I(p1);
            if dI>0.01
                cnt_zero=CNT(p1)-I(p1)*(CNT(p2)-CNT(p1))/dI;
                if cnt_zero<0, cnt_zero=cnt_zero+cnt_per_cycle; end
                dist_cnt=cnt_zero-CMP_UON;
                if dist_cnt<0, dist_cnt=dist_cnt+cnt_per_cycle; end
                phi_deg=(dist_cnt/cnt_per_cycle)*360;
                phi_deg=mod(phi_deg,360);
                if phi_deg>180, phi_deg=phi_deg-360; end
                if phi_deg>=8 && phi_deg<=85, phi_list(end+1)=phi_deg; end
            end
        end
    end
end
end

function power = calc_power_mini(raw, ~, timing, phase_info)
I=raw.I; Vdc=raw.Vdc;
D_U=timing.D_U_pct/100;
Vdc_mean=mean(Vdc,'omitnan'); I_RMS=rms(I);
if phase_info.usable
    cos_phi=phase_info.cos_phi;
    D_factor=sin(pi*mean(D_U));
    V_fund_rms=Vdc_mean*(sqrt(2)/pi)*max(D_factor,0.1);
    P_est=V_fund_rms*I_RMS*cos_phi;
else
    I_abs_mean=mean(abs(I));
    I_dc_est=I_abs_mean*mean(D_U);
    P_est=Vdc_mean*I_dc_est*0.88;
end
power.P=P_est; power.Vdc_mean=Vdc_mean;
power.phi_deg=phase_info.phi_deg; power.cos_phi=phase_info.cos_phi;
power.has_phase=phase_info.usable;
end

function rlc = calc_rlc_mini(raw, ~, timing, phase_info)
C_FARAD=0.9e-6;
I=raw.I; t=raw.t; Vdc_raw=raw.Vdc;
f_sw=timing.f_sw_kHz*1e3; omega=2*pi*f_sw;
I_peak=max(I); I_RMS=rms(I);
I_peak_est=max(I);
valley_idx=find(I(2:end-1)<I(1:end-2)&I(2:end-1)<I(3:end)&I(2:end-1)<I_peak_est*0.15);
valley_idx=valley_idx+1;
if isempty(valley_idx)
    rlc.valid=false; rlc.L_uH=NaN; rlc.Q=NaN;
    rlc.R_ohm=NaN; rlc.P_W=NaN; rlc.f_res_kHz=NaN; return;
end
vi=valley_idx(1);
dt_sec=mean(diff(t)); if dt_sec<=0, dt_sec=0.5e-6; end
if vi>1 && vi<length(I)
    di_dt=(abs(I(vi-1)-I(vi))+abs(I(vi+1)-I(vi)))/2/dt_sec;
else
    rlc.valid=false; rlc.L_uH=NaN; rlc.Q=NaN;
    rlc.R_ohm=NaN; rlc.P_W=NaN; rlc.f_res_kHz=NaN; return;
end
Vdc=Vdc_raw(vi);
V_C_peak=I_peak/(omega*C_FARAD);
L_val=(Vdc/2+V_C_peak)/(I_peak*omega);
f_res=1/(2*pi*sqrt(L_val*C_FARAD));
if phase_info.usable && ~isnan(phase_info.phi_deg)
    tan_phi=tand(phase_info.phi_deg);
    ratio=f_sw/f_res;
    Q_val=tan_phi/(ratio-1/ratio);
else
    Q_val=NaN;
end
omega_0=2*pi*f_res;
if ~isnan(Q_val) && Q_val>0
    R_val=omega_0*L_val/Q_val;
    P_val=I_RMS^2*R_val;
else
    R_val=NaN; P_val=NaN;
end
rlc.C_uF=C_FARAD*1e6; rlc.L_uH=L_val*1e6;
rlc.f_res_kHz=f_res/1e3; rlc.Q=Q_val;
rlc.R_ohm=R_val; rlc.P_W=P_val; rlc.valid=true;
end
