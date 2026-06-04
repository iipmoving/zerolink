% debug_iron_L — 深究铁锅单帧 φ 和 L 计算
function debug_iron_L()
csv_path = '../../tools/ekf_tuner/capture_20260602_153307.csv';

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

% 挑 L 偏高的帧: f5 (Ipk=3.6A 小功率, L=116.6) 和 f0 (正常帧对比)
for target_f = [5, 1]
    fprintf('\n========== 帧%d ==========\n', target_f);
    frame = frames_data{target_f+1};
    N = frames_N{target_f+1};

    t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
    CMP=frame(1,6:9);

    I_adc = min(max(I_adc, 0), 4095);
    Vdc_adc = min(max(Vdc_adc, 0), 4095);
    I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.VDC_SCALE;

    fprintf('CMP=[%d,%d,%d,%d]  N=%d\n', CMP(1), CMP(2), CMP(3), CMP(4), N);
    fprintf('I: min=%.3f max=%.3fA  Vdc: min=%.0f max=%.0f mean=%.0fV\n', min(I), max(I), min(Vdc), max(Vdc), mean(Vdc));

    %% CNT / timing
    dCNT=diff(CNT); dCNT_pos=dCNT(dCNT>0);
    avg_dcnt=mean(dCNT_pos);
    wrap_idx=find(diff(CNT)<0,1);
    if ~isempty(wrap_idx)
        CNT_range=CNT(wrap_idx)+round(avg_dcnt)-CNT(wrap_idx+1);
    else
        CNT_range=max(CNT)-min(CNT);
    end
    dtv=diff(t_us); dtv(dtv<=0)=[];
    avg_dt_us=mean(dtv);
    t_per_cnt_us=avg_dt_us/avg_dcnt;
    HRTIM_period_s=CNT_range*t_per_cnt_us*1e-6;
    f_sw=1/HRTIM_period_s;
    Tu=HRTIM_period_s*1e6;
    cpc=Tu/t_per_cnt_us;  % cnt per cycle

    fprintf('f_sw=%.2fkHz  Tu=%.2fus  t_per_cnt=%.4fus  cpc=%.0f  CNT_range=%d\n', ...
        f_sw/1e3, Tu, t_per_cnt_us, cpc, CNT_range);

    %% 打印全部采样点
    fprintf('\n--- 全部采样点 ---\n');
    fprintf('Idx  t_us    I[A]      Vdc[V]   CNT\n');
    for i=1:N
        fprintf('%3d  %6.1f  %8.3f  %7.1f  %5d\n', i, t_us(i), I(i), Vdc(i), CNT(i));
    end

    %% φ 检测详细
    Ipk=robust_peak(I,2);
    CU=CMP(1); CO=CMP(2);
    D_U = max(0,(CO-CU)*t_per_cnt_us)/Tu*100;
    fprintf('\nIpk=%.3fA  CMP_UON=%d  D_U=%.1f%%\n', Ipk, CU, D_U);

    % 谷值检测
    fprintf('\n--- 谷值检测 (I<%.3fA) ---\n', Ipk*0.15);
    for i=3:(N-2)
        if I(i-1)>I(i) && I(i)<I(i+1) && I(i)<Ipk*0.15
            % 抛物线插值
            cnt_interp=CNT(i);
            y_lo=I(i-1); y_mid=I(i); y_hi=I(i+1);
            denom=y_lo+y_hi-2*y_mid;
            if denom>0.01
                frac=(y_hi-y_lo)/(2*denom);
                frac=max(-0.5,min(0.5,frac));
                if frac>=0, cnt_interp=CNT(i)+frac*(CNT(i+1)-CNT(i));
                else, cnt_interp=CNT(i)+frac*(CNT(i)-CNT(i-1)); end
            end
            dc=cnt_interp-CU; if dc<0, dc=dc+cpc; end
            phi=(dc/cpc)*360; phi=mod(phi,360); if phi>180, phi=phi-360; end
            fprintf('  谷值 idx=%d  I=%.4fA  CNT=%d  interp_CNT=%.1f  phi=%.1f°  denom=%.4f  frac=%.4f\n', ...
                i, I(i), CNT(i), cnt_interp, phi, denom, frac);
        end
    end

    % 上升沿外推
    fprintf('\n--- 上升沿过零外推 ---\n');
    lo=Ipk*0.15; hi=Ipk*0.50;
    for i=3:N
        if I(i-1)<lo && I(i)>=lo
            sp=[];
            for j=i:min(N,i+10)
                if I(j)>=lo && I(j)<=hi, sp(end+1)=j; end
                if I(j)>hi, break; end
            end
            if length(sp)>=2
                p1=sp(1); p2=sp(end); dI=I(p2)-I(p1);
                if dI>0.01
                    cz=CNT(p1)-I(p1)*(CNT(p2)-CNT(p1))/dI;
                    if cz<0, cz=cz+cpc; end
                    dc=cz-CU; if dc<0, dc=dc+cpc; end
                    phi=(dc/cpc)*360; phi=mod(phi,360); if phi>180, phi=phi-360; end
                    fprintf('  外推: idx=%d..%d  I=%.3f..%.3fA  CNT_zero=%.1f  phi=%.1f°\n', ...
                        p1, p2, I(p1), I(p2), cz, phi);
                end
            else
                fprintf('  上升沿 idx=%d 但线性点数不足(%d)\n', i, length(sp));
            end
        end
    end

    %% I_RMS 和 |Z| 手工验算
    I_trim=I(3:(N-2));
    I_RMS=rms(I_trim);
    I_off=interp_at_cnt(CO, CNT, I);
    D_U_mean=D_U/100;
    Vdc_mean=mean(Vdc,'omitnan');
    V1=(sqrt(2)/pi)*Vdc_mean*sin(pi*D_U_mean);
    Z=V1/I_RMS;

    fprintf('\n--- 手工验算 |Z| ---\n');
    fprintf('I_RMS=%.3fA  I_off=%.3fA  Vdc_mean=%.1fV  D_U=%.3f\n', I_RMS, I_off, Vdc_mean, D_U_mean);
    fprintf('V1_RMS=%.2fV  |Z|=%.2f ohm\n', V1, Z);
    fprintf('φ 候选值需人工判断, 当前自动选举结果:\n');

    % 跑完整 cal_phase_mini 看最终 phi
    raw.t=t_us*1e-6; raw.I=I; raw.Vdc=Vdc; raw.CNT=CNT;
    raw.CMP=repmat(CMP,N,1); raw.t_us=t_us;
    raw.meta.edge_skip=2; raw.meta.t_per_cnt_us=t_per_cnt_us;
    raw.meta.HRTIM_period_s=HRTIM_period_s;

    timing.f_sw_kHz=f_sw/1e3; timing.T_hrtim_us=Tu;
    timing.CMP_UON=CMP(:,1); timing.CMP_UOFF=CMP(:,2);
    timing.CMP_LON=CMP(:,3); timing.CMP_LOFF=CMP(:,4);
    timing.D_U_pct=D_U*ones(N,1); timing.D_L_pct=zeros(N,1);

    waveform.I_RMS_all=I_RMS; waveform.I_peak_all=Ipk;

    ph=calc_phase_mini(raw,timing);
    fprintf('最终 φ=%.1f° cosφ=%.4f usable=%d\n', ph.phi_deg, ph.cos_phi, ph.usable);

    % RLC 验算
    C=0.94e-6; om=2*pi*f_sw;
    R_est=Z*cosd(ph.phi_deg);
    X=Z*sind(ph.phi_deg);
    L=(1/(om*C)+X)/om;
    fprintf('R=%.2f  X=%.2f  L=%.1fuH  (1/ωC=%.2f)\n', R_est, X, L*1e6, 1/(om*C));
end
end

function pk=robust_peak(I,edge_skip)
if nargin<2, edge_skip=0; end
n=length(I);
if n<=2*edge_skip, pk=max(I); return; end
It=I((edge_skip+1):(n-edge_skip));
Is=sort(It); idx=max(1,round(length(Is)*0.98)); pk=Is(idx);
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
