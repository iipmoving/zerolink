% batch_steel — 批量处理钢锅数据, 汇总输出
function batch_steel()
base = '../../tools/ekf_tuner/captures/captures/';
csvs = {
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

all_r = {};  % {file}{frame}

for fidx = 1:length(csvs)
    csv_path = [base csvs{fidx}];
    fprintf('\n=== %s ===\n', csvs{fidx});
    [fd, fN] = parse_csv(csv_path);
    rs = {};
    for f = 1:length(fd)
        r = calc_one_frame_steel(fd{f}, fN{f});
        rs{end+1} = r;
    end
    all_r{fidx} = rs;
end

%% 全帧列表
fprintf('\n');
fprintf('File     Frm  f_sw[kHz]  L[uH]   Q      R[ohm]  P[W]     phi[deg]  Ipk[A]  Vdc[V]\n');
fprintf('%s\n', repmat('-',1,90));

for fidx = 1:length(csvs)
    for fi = 1:length(all_r{fidx})
        r = all_r{fidx}{fi};
        fprintf('%-36s %-4d %8.2f  %7.1f  %7.2f  %7.2f  %8.1f  %7.1f  %7.1f  %6.0f\n', ...
            csvs{fidx}(10:15), fi-1, r.f_sw_kHz, r.L_uH, r.Q, r.R_ohm, ...
            r.P_W, r.phi_deg, r.I_peak_A, r.Vdc_mean);
    end
    fprintf('\n');
end

%% 稳定帧统计
fprintf('========== 稳定帧统计 (phi有效, Ipk>2A) ==========\n');
for fidx = 1:length(csvs)
    fsws=[]; Ls=[]; Qs=[]; Ps=[]; phis=[];
    for fi = 1:length(all_r{fidx})
        r = all_r{fidx}{fi};
        if ~isnan(r.phi_deg) && r.I_peak_A > 2
            fsws(end+1)=r.f_sw_kHz; Ls(end+1)=r.L_uH;
            Qs(end+1)=r.Q; Ps(end+1)=r.P_W; phis(end+1)=r.phi_deg;
        end
    end
    if ~isempty(fsws)
        fprintf('%-6s N=%d  f_sw=%6.1f+-%.0f  L=%5.1f+-%.1f  Q=%5.2f+-%.2f  P=%6.0f+-%.0f  phi=%5.1f+-%.1f\n', ...
            csvs{fidx}(10:15), length(fsws), ...
            mean(fsws), std(fsws), mean(Ls), std(Ls), ...
            mean(Qs), std(Qs), mean(Ps), std(Ps), mean(phis), std(phis));
    end
end

%% 总体统计
fprintf('\n========== 钢锅总体 ==========\n');
all_L=[]; all_Q=[]; all_P=[]; all_phi=[];
for fidx = 1:length(csvs)
    for fi = 1:length(all_r{fidx})
        r = all_r{fidx}{fi};
        if ~isnan(r.phi_deg) && r.I_peak_A > 2
            all_L=[all_L r.L_uH]; all_Q=[all_Q r.Q];
            all_P=[all_P r.P_W]; all_phi=[all_phi r.phi_deg];
        end
    end
end
fprintf('N=%d  L=%.1f+-%.1f uH [%.1f..%.1f]  Q=%.2f+-%.2f  phi=%.1f+-%.1f\n', ...
    length(all_L), mean(all_L), std(all_L), min(all_L), max(all_L), ...
    mean(all_Q), std(all_Q), mean(all_phi), std(all_phi));

end

%% ---- 解析CSV ----
function [frames_data, frames_N] = parse_csv(csv_path)
cal.ADC_BITS = 12; cal.ADC_STEPS = 4096; cal.VREF = 3.3;
cal.V_SCALE = cal.VREF/cal.ADC_STEPS/(6.2e3/(270e3*3+6.2e3));
cal.I_SCALE = cal.VREF/cal.ADC_STEPS/(330/(10e3+330));
cal.VDC_SCALE = cal.V_SCALE; cal.MIN_PULSE_US = 6;
cal.F_AC = 50; cal.HRTIM_CLK_MHZ = 144;

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

%% ---- 单帧计算 (同 run_per_frame 阻抗法) ----
function r = calc_one_frame_steel(frame, N)
cal.ADC_BITS = 12; cal.ADC_STEPS = 4096; cal.VREF = 3.3;
cal.V_SCALE = cal.VREF/cal.ADC_STEPS/(6.2e3/(270e3*3+6.2e3));
cal.I_SCALE = cal.VREF/cal.ADC_STEPS/(330/(10e3+330));
cal.VDC_SCALE = cal.V_SCALE; cal.MIN_PULSE_US = 6;
cal.F_AC = 50; cal.HRTIM_CLK_MHZ = 144;

t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
CMP=frame(1,6:9);

I_adc = min(max(I_adc, 0), 4095);
Vdc_adc = min(max(Vdc_adc, 0), 4095);
I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.VDC_SCALE;
t=t_us*1e-6;

EDGE_SKIP = 2;
I_robust_pk = robust_peak(I, EDGE_SKIP);
Nlen = length(I);

% raw
raw.t=t; raw.I=I; raw.Vdc=Vdc; raw.CNT=CNT;
raw.CMP=repmat(CMP,N,1); raw.t_us=t_us;
raw.meta.edge_skip = EDGE_SKIP;
raw.meta.I_robust_pk = I_robust_pk;

% CNT period
dCNT=diff(CNT); dCNT_pos=dCNT(dCNT>0);
avg_dcnt=1; if ~isempty(dCNT_pos), avg_dcnt=mean(dCNT_pos); end
wrap_idx=find(diff(CNT)<0,1);
if ~isempty(wrap_idx)
    CNT_range=CNT(wrap_idx)+round(avg_dcnt)-CNT(wrap_idx+1);
else
    CNT_range=max(CNT)-min(CNT);
end
dtv=diff(t_us); dtv(dtv<=0)=[];
avg_dt_us=0.5; if ~isempty(dtv), avg_dt_us=mean(dtv); end
if avg_dcnt>0 && avg_dt_us>0, t_per_cnt_us=avg_dt_us/avg_dcnt;
else, t_per_cnt_us=1/cal.HRTIM_CLK_MHZ; end
HRTIM_period_s=CNT_range*t_per_cnt_us*1e-6;
if HRTIM_period_s<=0, HRTIM_period_s=1/30000; end

raw.meta.t_per_cnt_us=t_per_cnt_us;
raw.meta.HRTIM_period_s=HRTIM_period_s;

try
    timing=calc_timing_mini(raw,cal);
    waveform=calc_waveform_mini(raw,timing);
    ph=calc_phase_mini(raw,timing);
    pw=calc_power_mini(raw,waveform,timing,ph);
    rl=calc_rlc_mini(raw,waveform,timing,ph);

    r.f_sw_kHz=timing.f_sw_kHz;
    r.DT1_us=mean(timing.DT1_us); r.DT2_us=mean(timing.DT2_us);
    r.D_U_pct=mean(timing.D_U_pct); r.D_L_pct=mean(timing.D_L_pct);
    r.ctrl_mode=timing.ctrl_mode;
    r.I_peak_A=waveform.I_peak_all; r.I_RMS_A=waveform.I_RMS_all;
    r.I_at_UOFF=pw.I_at_UOFF;
    r.phi_deg=ph.phi_deg; r.cos_phi=ph.cos_phi;
    r.P_W=pw.P; r.P_fund=pw.P_fund;
    r.L_uH=rl.L_uH; r.Q=rl.Q; r.R_ohm=rl.R_ohm;
    r.f_res_kHz=rl.f_res_kHz; r.Vdc_mean=mean(Vdc);
catch
    r.f_sw_kHz=1/HRTIM_period_s/1e3;
    r.DT1_us=NaN; r.DT2_us=NaN; r.D_U_pct=NaN; r.D_L_pct=NaN;
    r.ctrl_mode='?'; r.I_peak_A=max(I); r.I_RMS_A=rms(I);
    r.I_at_UOFF=NaN; r.phi_deg=NaN; r.cos_phi=NaN;
    r.P_W=NaN; r.P_fund=NaN; r.L_uH=NaN; r.Q=NaN; r.R_ohm=NaN;
    r.f_res_kHz=NaN; r.Vdc_mean=mean(Vdc);
end
end

%% ---- 以下为 calc_* 微缩版 (同 run_per_frame 完全一致) ----
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

function waveform=calc_waveform_mini(raw,timing)
I=raw.I; edge=raw.meta.edge_skip; n=length(I);
if n>2*edge, I_trim=I((edge+1):(n-edge)); else, I_trim=I; end
waveform.I_peak_all=robust_peak(I,edge);
waveform.I_RMS_all=rms(I_trim);
waveform.I_avg_all=mean(I_trim);
waveform.f_res_mean_khz=timing.f_sw_kHz;
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

function pw=calc_power_mini(raw,waveform,timing,ph)
I=raw.I; Vdc=raw.Vdc; CNT=raw.CNT;
DU=timing.D_U_pct/100;
Vm=mean(Vdc,'omitnan'); Ir=waveform.I_RMS_all;
I_off=interp_at_cnt(timing.CMP_UOFF(1),CNT,I);
if ~isnan(I_off)&&I_off>0, P=Vm*I_off*mean(DU)/2; else, P=NaN; end
if ph.usable
    Df=sin(pi*mean(DU)); Vf=Vm*(sqrt(2)/pi)*max(Df,0.1);
    Pf=Vf*Ir*ph.cos_phi;
else, Pf=NaN;
end
pw.P=P; pw.P_fund=Pf; pw.I_at_UOFF=I_off;
pw.Vdc_mean=Vm; pw.phi_deg=ph.phi_deg; pw.cos_phi=ph.cos_phi; pw.has_phase=ph.usable;
end

function Ii=interp_at_cnt(tgt,CNT,I)
Ii=NaN; n=length(CNT);
if n<2||isnan(tgt)||tgt==0, return; end
for i=1:(n-1)
    c1=CNT(i); c2=CNT(i+1);
    if c1<=tgt&&tgt<=c2&&c2>c1
        frac=(tgt-c1)/(c2-c1); Ii=I(i)+frac*(I(i+1)-I(i)); return;
    end
end
[~,idx]=min(abs(CNT-tgt)); Ii=I(idx);
end

function rl=calc_rlc_mini(raw,waveform,timing,ph)
C=0.94e-6; Vdc=raw.Vdc; fs=timing.f_sw_kHz*1e3; om=2*pi*fs;
Ir=waveform.I_RMS_all; DU=mean(timing.D_U_pct)/100;
if ~ph.usable||isnan(ph.phi_deg)||Ir<0.1
    rl.valid=false; rl.L_uH=NaN; rl.Q=NaN; rl.R_ohm=NaN; rl.f_res_kHz=NaN; return;
end
Vm=mean(Vdc,'omitnan');
V1=(sqrt(2)/pi)*Vm*sin(pi*DU);
if V1<0.1, rl.valid=false; rl.L_uH=NaN; rl.Q=NaN; rl.R_ohm=NaN; rl.f_res_kHz=NaN; return; end
Z=V1/Ir; R=Z*cosd(ph.phi_deg); X=Z*sind(ph.phi_deg);
L=(1/(om*C)+X)/om;
if L<=0, rl.valid=false; rl.L_uH=NaN; rl.Q=NaN; rl.R_ohm=NaN; rl.f_res_kHz=NaN; return; end
fr=1/(2*pi*sqrt(L*C)); o0=2*pi*fr;
if R>0, Qv=o0*L/R; else, Qv=NaN; end
rl.L_uH=L*1e6; rl.f_res_kHz=fr/1e3; rl.Q=Qv; rl.R_ohm=R; rl.valid=true;
end

function pk=robust_peak(I,edge_skip)
if nargin<2, edge_skip=0; end
n=length(I);
if n<=2*edge_skip, pk=max(I); return; end
It=I((edge_skip+1):(n-edge_skip));
Is=sort(It); idx=max(1,round(length(Is)*0.98)); pk=Is(idx);
end
