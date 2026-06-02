% list_all_frames — 三文件逐帧全参数输出
function list_all_frames()
cal.ADC_BITS = 12; cal.ADC_STEPS = 4096; cal.VREF = 3.3;
cal.V_SCALE = cal.VREF/cal.ADC_STEPS/(6.2e3/(270e3*3+6.2e3));
cal.I_SCALE = cal.VREF/cal.ADC_STEPS/(330/(10e3+330));
cal.VDC_SCALE = cal.V_SCALE; cal.MIN_PULSE_US = 6;
cal.F_AC = 50; cal.HRTIM_CLK_MHZ = 144;

base = '../../tools/ekf_tuner/';
csvs = {'capture_20260602_104849.csv','capture_20260602_104850.csv','capture_20260602_104852.csv'};
labels = {'104849','104850','104852'};

all_r = {};
for fidx = 1:3
    [fd, fN, ~, nc] = parse_csv([base csvs{fidx}]);
    rs = {};
    for f = 1:length(fd)
        rs{end+1} = calc_one_frame(fd{f}, fN{f}, cal);
    end
    all_r{fidx} = rs;
end

%% 全帧列表
fprintf('\n');
fprintf('File     Frm  f_sw[kHz]  L[uH]   Q      R[ohm]  P[W]     phi[deg]  Ipk[A]  Vdc[V]\n');
fprintf('%s\n', repmat('-',1,90));

for fidx = 1:3
    for fi = 1:length(all_r{fidx})
        r = all_r{fidx}{fi};
        fprintf('%-8s %-4d %8.2f  %7.1f  %7.2f  %7.2f  %8.1f  %7.1f  %7.1f  %6.0f\n', ...
            labels{fidx}, fi-1, r.f_sw_kHz, r.L_uH, r.Q, r.R_ohm, ...
            r.P_W, r.phi_deg, r.I_peak_A, r.Vdc_mean);
    end
    fprintf('\n');
end

%% 异常标记
fprintf('=== 异常帧标记 ===\n');
for fidx = 1:3
    for fi = 1:length(all_r{fidx})
        r = all_r{fidx}{fi};
        issues = {};
        if r.I_peak_A > 100, issues{end+1} = 'Ipk爆表'; end
        if r.Q > 20 || r.Q < 0, issues{end+1} = 'Q异常'; end
        if r.P_W > 5000, issues{end+1} = 'P异常'; end
        if ~isempty(issues)
            fprintf('%s 帧%d: %s\n', labels{fidx}, fi-1, strjoin(issues,', '));
        end
    end
end
fprintf('\n');
end

%% ---- 以下同 run_per_frame 内部逻辑 ----
function [frames_data, frames_N, header_line, n_input_cols] = parse_csv(csv_path)
fid = fopen(csv_path,'r');
header_line = fgetl(fid);
n_input_cols = length(strsplit(header_line,','));
frames_data={}; frames_N={};
while ~feof(fid)
    line=fgetl(fid); if line==-1, break; end
    line=strtrim(line); if isempty(line), continue; end
    if startsWith(line,'SIZE')
        parts=strsplit(line,','); N=str2double(parts{2}); frames_N{end+1}=N;
        fr=zeros(N,n_input_cols);
        for i=1:N
            dl=fgetl(fid); dl=strtrim(dl);
            if isempty(dl), i=i-1; continue; end
            parts=strsplit(dl,',');
            for j=1:min(length(parts),n_input_cols)
                v=str2double(parts{j}); if ~isnan(v), fr(i,j)=v; end
            end
        end
        frames_data{end+1}=fr;
    end
end
fclose(fid);
end

function r = calc_one_frame(frame, N, cal)
t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
CMP=frame(1,6:9);
I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.VDC_SCALE;
raw.t=t_us*1e-6; raw.I=I; raw.Vdc=Vdc; raw.CNT=CNT;
raw.CMP=repmat(CMP,N,1); raw.t_us=t_us;

dCNT=diff(CNT); dCNT_pos=dCNT(dCNT>0);
avg_dcnt=1; if ~isempty(dCNT_pos), avg_dcnt=mean(dCNT_pos); end
wi=find(diff(CNT)<0,1);
if ~isempty(wi), CNT_range=CNT(wi)+round(avg_dcnt)-CNT(wi+1);
else, CNT_range=max(CNT)-min(CNT); end
dtv=diff(t_us); dtv(dtv<=0)=[];
avg_dt_us=0.5; if ~isempty(dtv), avg_dt_us=mean(dtv); end
tpc=1/cal.HRTIM_CLK_MHZ;
if avg_dcnt>0 && avg_dt_us>0, tpc=avg_dt_us/avg_dcnt; end
HPs=CNT_range*tpc*1e-6; if HPs<=0, HPs=1/30000; end
raw.meta.t_per_cnt_us=tpc; raw.meta.HRTIM_period_s=HPs;

try
    tim=calc_timing_mini(raw,cal);
    ph=calc_phase_mini(raw,tim);
    rl=calc_rlc_mini(raw,[],tim,ph);
    pw=calc_power_mini(raw,[],tim,ph);
    r.f_sw_kHz=tim.f_sw_kHz; r.DT1_us=mean(tim.DT1_us); r.DT2_us=mean(tim.DT2_us);
    r.D_U_pct=mean(tim.D_U_pct); r.D_L_pct=mean(tim.D_L_pct);
    r.ctrl_mode=tim.ctrl_mode; r.I_peak_A=max(I); r.I_RMS_A=rms(I);
    r.phi_deg=ph.phi_deg; r.cos_phi=ph.cos_phi;
    r.P_W=pw.P; r.L_uH=rl.L_uH; r.Q=rl.Q; r.R_ohm=rl.R_ohm;
    r.f_res_kHz=rl.f_res_kHz; r.Vdc_mean=mean(Vdc);
catch
    r.f_sw_kHz=1/HPs/1e3; r.DT1_us=NaN; r.DT2_us=NaN;
    r.D_U_pct=NaN; r.D_L_pct=NaN; r.ctrl_mode='?';
    r.I_peak_A=max(I); r.I_RMS_A=rms(I); r.phi_deg=NaN; r.cos_phi=NaN;
    r.P_W=NaN; r.L_uH=NaN; r.Q=NaN; r.R_ohm=NaN; r.f_res_kHz=NaN; r.Vdc_mean=mean(Vdc);
end
end

function timing=calc_timing_mini(raw,cal)
CMP=raw.CMP; tpc=raw.meta.t_per_cnt_us;
CU=CMP(:,1); CO=CMP(:,2); CL=CMP(:,3); CF=CMP(:,4);
Tu=raw.meta.HRTIM_period_s*1e6;
T_UON=max(0,(CO-CU)*tpc); T_LON=max(0,(CF-CL)*tpc);
D_U=T_UON./Tu*100; D_L=T_LON./Tu*100;
DT1=max(0,(CL-CO)*tpc);
DT2=(CU-CF)*tpc;
for i=1:length(DT2), if DT2(i)<0, DT2(i)=DT2(i)+Tu; end; end
if std(D_U)<2 && mean(D_U)>45, cm='FM'; else, cm='PWM'; end
timing.f_sw_kHz=1/(Tu*1e-6)/1e3; timing.T_hrtim_us=Tu;
timing.D_U_pct=D_U; timing.D_L_pct=D_L;
timing.DT1_us=DT1; timing.DT2_us=DT2;
timing.CMP_UON=CU; timing.CMP_UOFF=CO; timing.CMP_LON=CL; timing.CMP_LOFF=CF;
timing.ctrl_mode=cm; timing.D_std=std(D_U);
end

function ph=calc_phase_mini(raw,timing)
I=raw.I; CNT=raw.CNT;
if timing.CMP_UON(1)==0, ph.phi_deg=NaN; ph.cos_phi=NaN; ph.usable=false; return; end
CU=timing.CMP_UON(1); Tu=timing.T_hrtim_us;
Ipk=max(I); n=length(I); cpc=Tu/raw.meta.t_per_cnt_us;
vi=[]; for i=2:n-1, if I(i-1)>I(i)&&I(i)<I(i+1)&&I(i)<Ipk*0.15, vi(end+1)=i; end; end
pv=phi_from_idx(vi,I,CNT,CU,cpc);
pe=extrap_zeros(I,CNT,Ipk,CU,cpc);
ap=[pv pe]; if isempty(ap), ph.phi_deg=NaN; ph.cos_phi=NaN; ph.usable=false; return; end
ap_pos=ap(ap>0); if ~isempty(ap_pos), pa=median(ap_pos); else, pa=median(ap); end
ph.phi_deg=pa; ph.cos_phi=cosd(pa); ph.usable=true;
end

function pl=phi_from_idx(idx,I,CNT,CU,cpc)
% 谷值相位检测, 带抛物线插值突破0.5μs采样分辨率
pl=[]; for k=1:length(idx)
    vi=idx(k); cnt_interp=CNT(vi);
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
    dc=cnt_interp-CU; if dc<0, dc=dc+cpc; end
    ph=(dc/cpc)*360; ph=mod(ph,360); if ph>180, ph=ph-360; end
    if ph>=8&&ph<=85, pl(end+1)=ph; end
end; end

function pl=extrap_zeros(I,CNT,Ipk,CU,cpc)
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

function pw=calc_power_mini(raw,~,timing,ph)
I=raw.I; Vdc=raw.Vdc; DU=timing.D_U_pct/100;
Vm=mean(Vdc,'omitnan'); Ir=rms(I);
if ph.usable
    Df=sin(pi*mean(DU)); Vf=Vm*(sqrt(2)/pi)*max(Df,0.1);
    P=Vf*Ir*ph.cos_phi;
else
    P=Vm*mean(abs(I))*mean(DU)*0.88;
end
pw.P=P; pw.Vdc_mean=Vm; pw.phi_deg=ph.phi_deg; pw.cos_phi=ph.cos_phi; pw.has_phase=ph.usable;
end

function rl=calc_rlc_mini(raw,~,timing,ph)
C=0.9e-6; I=raw.I; t=raw.t; Vdc=raw.Vdc;
fs=timing.f_sw_kHz*1e3; om=2*pi*fs; Ipk=max(I); Ir=rms(I);
vi=find(I(2:end-1)<I(1:end-2)&I(2:end-1)<I(3:end)&I(2:end-1)<Ipk*0.15)+1;
if isempty(vi), rl.L_uH=NaN; rl.Q=NaN; rl.R_ohm=NaN; rl.f_res_kHz=NaN; return; end
v=vi(1); dt=mean(diff(t)); if dt<=0, dt=0.5e-6; end
if v<=1||v>=length(I), rl.L_uH=NaN; rl.Q=NaN; rl.R_ohm=NaN; rl.f_res_kHz=NaN; return; end
Vd=Vdc(v); VC=Ipk/(om*C);
L=(Vd/2+VC)/(Ipk*om); fr=1/(2*pi*sqrt(L*C));
if ph.usable&&~isnan(ph.phi_deg)
    Q=tand(ph.phi_deg)/(fs/fr-fr/fs);
else, Q=NaN;
end
o0=2*pi*fr;
if ~isnan(Q)&&Q>0, R=o0*L/Q; else, R=NaN; end
rl.L_uH=L*1e6; rl.f_res_kHz=fr/1e3; rl.Q=Q; rl.R_ohm=R;
end
