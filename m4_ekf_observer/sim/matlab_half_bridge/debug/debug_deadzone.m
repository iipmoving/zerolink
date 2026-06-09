% debug_deadzone — 中段线性拟合外推趋势零点, 量化交越死区对I_RMS的影响
function debug_deadzone()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.VDC_SCALE = cal.V_SCALE;
C_FARAD = 0.94e-6;

base = '../../tools/ekf_tuner/captures/captures/';
iron_files = {
    'capture_20260602_153307.csv','capture_20260602_153311.csv', ...
    'capture_20260602_153314.csv','capture_20260602_153316.csv', ...
    'capture_20260602_153326.csv','capture_20260602_153329.csv', ...
    'capture_20260602_153336.csv','capture_20260602_153338.csv', ...
    'capture_20260602_153340.csv','capture_20260602_153347.csv', ...
    'capture_20260602_153348.csv','capture_20260602_153358.csv', ...
    'capture_20260602_153400.csv','capture_20260602_153410.csv', ...
    'capture_20260602_153412.csv','capture_20260602_153413.csv', ...
    'capture_20260602_153421.csv','capture_20260602_153422.csv', ...
    'capture_20260602_153424.csv'
};

fprintf('Vdc[V]   Ipk[A]  di/dt[A/us]  t_dead_us  dead%%   L_raw  L_corr\n');
fprintf('%s\n', repmat('-',1,75));

all_r = [];

for fidx = 1:length(iron_files)
    csv_path = [base iron_files{fidx}];
    [fd, fN] = parse_csv(csv_path);
    for f = 1:length(fd)
        frame = fd{f}; N_frame = fN{f};
        t_us=frame(:,1); I_adc=frame(:,2); Vdc_adc=frame(:,4); CNT=frame(:,5);
        CMP=frame(1,6:9);
        I_adc = min(max(I_adc, 0), 4095);
        Vdc_adc = min(max(Vdc_adc, 0), 4095);
        I=I_adc*cal.I_SCALE; Vdc=Vdc_adc*cal.VDC_SCALE;
        CU=CMP(1); CO=CMP(2);

        wrap_idx = find(diff(CNT) < 0);
        breaks = [0; wrap_idx(:); N_frame];
        best_seg = [1 N_frame]; best_len = 0;
        for b = 1:(length(breaks)-1)
            seg_len = breaks(b+1) - breaks(b);
            if seg_len > best_len
                best_len = seg_len; best_seg = [breaks(b)+1, breaks(b+1)];
            end
        end
        seg_end = best_seg(2);
        while seg_end > best_seg(1) && I_adc(seg_end) > 4090, seg_end = seg_end - 1; end
        if seg_end > best_seg(1), seg_end = seg_end - 1; end
        seg_start = best_seg(1);
        I_clean = I(seg_start:seg_end);
        t_clean = t_us(seg_start:seg_end);
        CNT_clean = CNT(seg_start:seg_end);

        if length(I_clean) < 30, continue; end
        Ipk = robust_peak(I_clean, 0);
        if Ipk < 2, continue; end

        % ==== 找 I_min 位置, 然后找上升沿 ====
        [I_min, idx_min] = min(I_clean);
        [I_max, idx_max] = max(I_clean);
        n = length(I_clean);
        dt_us = mean(diff(t_clean));

        % 建环形索引路径: idx_min → idx_max (沿递增方向)
        rise_idx = [];
        i = idx_min;
        while i ~= idx_max
            i = i + 1;
            if i > n, i = 1; end
            rise_idx(end+1) = i;
        end
        rise_I_raw = I_clean(rise_idx);
        n_rise = length(rise_idx);

        if n_rise < 6, continue; end

        % 30%-70% 线性段
        I_lo = I_min + (I_max - I_min) * 0.30;
        I_hi = I_min + (I_max - I_min) * 0.70;
        lin_idx = find(rise_I_raw >= I_lo & rise_I_raw <= I_hi);
        if length(lin_idx) < 3, continue; end

        t_rel = (0:n_rise-1)' * dt_us;  % 相对时间轴 (从 idx_min+1 开始)
        p = polyfit(t_rel(lin_idx), rise_I_raw(lin_idx), 1);
        didt = p(1);  % A/us

        % 趋势零点: I = didt * (t - t0) 其中 I=0 时
        % 即 I = p(1)*t + p(2), 0 = p(1)*t0 + p(2), t0 = -p(2)/p(1)
        t_trend_zero = -p(2) / p(1);

        % 死区时间 = 趋势零点到实际 min 的时间差
        % 趋势零点在 idx_min 之前 t_trend_zero (负值), 说明真实过零点比实测min早
        dt_dead = -t_trend_zero;  % 正值 = 死区宽度
        if dt_dead < 0 || dt_dead > 20, continue; end  % 不合理值过滤

        % 半周期 = 趋势零点到峰值的时间
        half_period = t_rel(end) - t_trend_zero;  % 从趋势零点到 I_max
        dead_pct = dt_dead / half_period * 100;
        if dead_pct < 0 || dead_pct > 50, continue; end

        % ==== L 原始计算 ====
        Vdc_mean = mean(Vdc(seg_start:seg_end));
        dCNT = diff(CNT_clean); dCNT_pos = dCNT(dCNT > 0);
        if isempty(dCNT_pos), continue; end
        avg_dcnt = mean(dCNT_pos);
        dtv = diff(t_clean); dtv(dtv <= 0) = [];
        avg_dt_us = mean(dtv);
        t_per_cnt_us = avg_dt_us / avg_dcnt;
        seg_full_CNT = CNT(best_seg(1):best_seg(2));
        wi = find(diff(seg_full_CNT) < 0, 1);
        if ~isempty(wi)
            CNT_range = seg_full_CNT(wi) + round(avg_dcnt) - seg_full_CNT(wi+1);
        else
            CNT_range = max(seg_full_CNT) - min(seg_full_CNT);
        end
        HRTIM_period_s = CNT_range * t_per_cnt_us * 1e-6;
        f_sw = 1/HRTIM_period_s;
        Tu = HRTIM_period_s * 1e6;
        D_U = max(0, (CO-CU) * t_per_cnt_us) / Tu;
        phi_val = detect_phi(I_clean, CNT_clean, CU, Tu, t_per_cnt_us, Ipk);
        omega = 2*pi*f_sw;
        I_rms = rms(I_clean);
        if isnan(phi_val) || I_rms < 0.1, continue; end
        V1 = (sqrt(2)/pi)*Vdc_mean*sin(pi*D_U);
        L_raw = (1/(omega*C_FARAD) + V1/I_rms*sind(phi_val)) / omega * 1e6;

        % ==== 死区修正: 补回被削掉的 I_RMS ====
        % 三角波在底部被削平 dt_dead, 损失的 RMS 近似为:
        % I_rms_loss ≈ I_avg_at_deadzone × sqrt(dead_pct)
        % 简化: I_rms_corr = I_rms × (1 + dead_pct/100 × k)
        % k≈0.5 因为死区在最小值附近, 电流值小, 对RMS贡献小
        corr_factor = 1 + dead_pct/100 * 0.5;
        I_rms_corr = I_rms * corr_factor;
        L_corr = (1/(omega*C_FARAD) + V1/I_rms_corr*sind(phi_val)) / omega * 1e6;

        r.Vdc = Vdc_mean; r.Ipk = Ipk; r.didt = didt;
        r.dt_dead = dt_dead; r.dead_pct = dead_pct;
        r.L_raw = L_raw; r.L_corr = L_corr;
        all_r = [all_r; r];

        fprintf('%-6.0f   %5.1f    %7.3f     %7.2f    %4.1f   %6.1f  %6.1f\n', ...
            Vdc_mean, Ipk, didt, dt_dead, dead_pct, L_raw, L_corr);
    end
end

fprintf('\n========== 死区宽度 vs Vdc ==========\n');
V_edges = [70 90 110 150 200 250 300];
for b = 1:length(V_edges)-1
    mask = [all_r.Vdc] >= V_edges(b) & [all_r.Vdc] < V_edges(b+1);
    if sum(mask) >= 2
        fprintf('Vdc=%3d-%-3dV  N=%2d  dt_dead=%.1f+-%.1fus  dead%%=%.1f+-%.1f%%  L_raw=%.1f  L_corr=%.1f\n', ...
            V_edges(b), V_edges(b+1), sum(mask), ...
            mean([all_r(mask).dt_dead]), std([all_r(mask).dt_dead]), ...
            mean([all_r(mask).dead_pct]), std([all_r(mask).dead_pct]), ...
            mean([all_r(mask).L_raw]), mean([all_r(mask).L_corr]));
    end
end

fprintf('\nL_raw:  mean=%.1f  std=%.1f  CV=%.1f%%\n', ...
    mean([all_r.L_raw]), std([all_r.L_raw]), std([all_r.L_raw])/mean([all_r.L_raw])*100);
fprintf('L_corr: mean=%.1f  std=%.1f  CV=%.1f%%\n', ...
    mean([all_r.L_corr]), std([all_r.L_corr]), std([all_r.L_corr])/mean([all_r.L_corr])*100);
end

function phi_val = detect_phi(I, CNT, CU, Tu, t_per_cnt_us, Ipk)
    phi_val = NaN; cpc=Tu/t_per_cnt_us; nI=length(I);
    vi=[];
    for i=2:(nI-1)
        if I(i-1)>I(i)&&I(i)<I(i+1)&&I(i)<Ipk*0.15, vi(end+1)=i; end
    end
    pv=[]; for k=1:length(vi)
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
    pe=[]; lo=Ipk*0.15; hi=Ipk*0.50;
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
