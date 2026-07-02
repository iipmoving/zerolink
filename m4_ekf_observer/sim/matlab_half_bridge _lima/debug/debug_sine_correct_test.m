% debug_sine_correct_test.m — 注入人工尖峰, 验证正弦修正效果 + 计算量评估
function debug_sine_correct_test()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
base = '../../tools/ekf_tuner/captures/captures/';

% 取一个铁锅帧, 波形清晰
[frames, fN] = parse_csv_frames([base 'capture_20260602_153316.csv']);
fr = frames{4}; N = fN(4);

I_adc=fr(:,2); CNT=fr(:,5); CMP=fr(1,6:9); t_us=fr(:,1);
I_adc=min(max(I_adc,0),4095);
I = I_adc * cal.I_SCALE;
CU=CMP(1); CO=CMP(2);

% 清洗 + 导通区间
seg_end=N;
while seg_end>1 && I_adc(seg_end)>4090, seg_end=seg_end-1; end
if seg_end>1, seg_end=seg_end-1; end
CNT_seg=CNT(1:seg_end); I_seg=I(1:seg_end); t_seg=t_us(1:seg_end);
n_seg=seg_end;

% 频率
dCNT=diff(CNT_seg); dCNT_pos=dCNT(dCNT>0);
avg_dcnt=mean(dCNT_pos);
dtv=diff(t_seg); dtv(dtv<=0)=[];
avg_dt_us=mean(dtv);
t_per_cnt_us=avg_dt_us/avg_dcnt;
wi=find(diff(CNT(1:seg_end))<0,1);
if ~isempty(wi)
    CNT_range=CNT_seg(wi)+round(avg_dcnt)-CNT_seg(wi+1);
else
    CNT_range=max(CNT_seg)-min(CNT_seg);
end
f_sw=1/(CNT_range*t_per_cnt_us*1e-6);
omega=2*pi*f_sw;

% 导通区间 mask
cond=false(n_seg,1);
for i=1:n_seg
    cnt=CNT_seg(i);
    if CU<CO, cond(i)=(cnt>=CU && cnt<=CO);
    else, cond(i)=(cnt>=CU || cnt<=CO); end
end
cond_idx=find(cond);
I_cond=I_seg(cond_idx);
t_cond=t_seg(cond_idx);
n_cond=length(I_cond);
[Ipk_raw, pk_rel] = max(I_cond);

%% 注入尖峰: 在峰值点加不同幅度的干扰
spike_levels = [0, 10, 20, 30, 50];  % ADC counts 的尖峰
n_test = length(spike_levels);

fprintf('========== 正弦修正抗尖峰测试 ==========\n');
fprintf('原始 I_peak = %.2f A  (ADC=%d)\n', Ipk_raw, round(Ipk_raw/cal.I_SCALE));
fprintf('f_sw=%.2fkHz  omega=%.1f rad/s  dt=%.3fus\n\n', f_sw/1e3, omega, avg_dt_us);
fprintf('%-10s %8s %8s %8s %8s\n', 'Spike(ADC)','Ipk_raw','Ipk_corr','Delta','Correct?');
fprintf('%s\n', repmat('-',1,50));

for si = 1:n_test
    spike = spike_levels(si);
    I_test = I_cond;
    I_test(pk_rel) = I_test(pk_rel) + spike * cal.I_SCALE;  % 注入尖峰

    % 正弦修正
    Ipk_corr = sine_correct_peak(I_test, t_cond, omega, pk_rel);
    Ipk_raw_spk = max(I_test);

    ok = abs(Ipk_corr - Ipk_raw) < abs(Ipk_raw_spk - Ipk_raw) * 0.3;
    fprintf('%-10d %8.2f %8.2f %+8.2f %8s\n', ...
        spike, Ipk_raw_spk, Ipk_corr, Ipk_corr-Ipk_raw, ...
        iff(ok, 'OK', 'FAIL'));
end

%% 计算量评估
fprintf('\n========== C端计算量评估 ==========\n');
fprintf('每周期操作:\n');
fprintf('  max()扫描(导通区间):   ~100次比较 (已做)\n');
fprintf('  sin/cos预计算:         4×2=8次 (每个采样点已知dt)\n');
fprintf('  正规方程累加(2×2):    6个累加器 × 4点 = 24次 MAC\n');
fprintf('  解2×2线性系统:         det, A, B = ~15次乘除 + ~10次加减\n');
fprintf('  I_peak = sqrt(A²+B²):  3次乘法 + 1次开方\n');
fprintf('  偏差检测(4-6点):      每点1次sin+1次cos+1次预测+1次比较\n');
fprintf('  ---------------------------------------------------\n');
fprintf('  合计: ~60次浮点乘法 + 30次加法 + 1次开方 + 8次sin/cos\n');
fprintf('  M4 FPU单周期FMAC: 相当于 ~100个FPU周期\n');
fprintf('  @48MHz, 30kHz开关频率: < 0.1%% CPU\n');

%% 图示: 尖峰修正效果
spike_show = 30;  % 展示30 ADC尖峰
I_show = I_cond;
I_show(pk_rel) = I_show(pk_rel) + spike_show * cal.I_SCALE;
Ipk_corr_show = sine_correct_peak(I_show, t_cond, omega, pk_rel);

figure('Position',[100 100 900 400]);
plot_range = max(1,pk_rel-15):min(n_cond,pk_rel+8);
plot(t_cond(plot_range)*1e6, I_show(plot_range), 'b.-', 'MarkerSize',8); hold on;
plot(t_cond(pk_rel)*1e6, I_show(pk_rel), 'rx', 'MarkerSize',12, 'LineWidth',2);
yline(Ipk_raw, 'g--', 'True peak');
yline(Ipk_corr_show, 'm--', 'Corrected');
legend('Signal','Injected spike','True peak','After correction','Location','best');
xlabel('Time (us)'); ylabel('Current (A)');
title(sprintf('Sine correction: %d ADC spike -> corrected %.2fA (true=%.2fA)', ...
    spike_show, Ipk_corr_show, Ipk_raw));
grid on;
fprintf('\nFigure saved. Close figure window to continue.\n');
saveas(gcf, 'sine_correct_demo.png');
fprintf('PNG saved: sine_correct_demo.png\n');
end

function Ipk_corr = sine_correct_peak(I_data, t_data, omega, pk_idx)
% 峰顶正弦修正: 排除疑似尖峰点(峰值), 用周围3-4点拟合, 预测峰值
% 关键: 尖峰点不参与拟合, 只用邻居的曲线形状推算峰值
n = length(I_data);

% 取峰值邻域5点 (pk-2..pk+2)
half_w = 2;
pk_start = max(1, pk_idx - half_w);
pk_end = min(n, pk_idx + half_w);
pk_range = pk_start:pk_end;

if length(pk_range) < 4
    Ipk_corr = max(I_data(pk_range));
    return;
end

% 拟合用点: 排除峰值本身(pk_idx), 用其余3-4点
fit_idx = setdiff(pk_range, pk_idx);
if length(fit_idx) < 3
    Ipk_corr = max(I_data(pk_range));
    return;
end

% 局部正弦拟合: I(t) = A*sin(ωt) + B*cos(ωt) + C
% 只用周边点 (不含峰值)
t_use = t_data(fit_idx) * 1e-6;
I_use = I_data(fit_idx);
t0 = t_use(1);
dt = t_use - t0;
S = sin(omega * dt);
C_ = cos(omega * dt);
n_use = length(fit_idx);

% 正规方程累加 (3×3)
sum_s2 = sum(S.^2); sum_c2 = sum(C_.^2);
sum_sc = sum(S .* C_);
sum_s = sum(S); sum_c = sum(C_);
sum_si = sum(S .* I_use);
sum_ci = sum(C_ .* I_use);
sum_i = sum(I_use);

% Cramer解 3×3
M11=sum_s2; M12=sum_sc; M13=sum_s;
M21=sum_sc; M22=sum_c2; M23=sum_c;
M31=sum_s;  M32=sum_c;  M33=n_use;

detM = M11*(M22*M33-M23*M32) - M12*(M21*M33-M23*M31) + M13*(M21*M32-M22*M31);
if abs(detM) < 1e-12
    Ipk_corr = max(I_data(pk_range));
    return;
end

A = (sum_si*(M22*M33-M23*M32) - M12*(sum_ci*M33-M23*sum_i) + M13*(sum_ci*M32-M22*sum_i)) / detM;
B = (M11*(sum_ci*M33-M23*sum_i) - sum_si*(M21*M33-M23*M31) + M13*(M21*sum_i-sum_ci*M31)) / detM;
C_val = (M11*(M22*sum_i-sum_ci*M32) - M12*(M21*sum_i-sum_ci*M31) + sum_si*(M21*M32-M22*M31)) / detM;

% 用拟合正弦预测峰值点的值
dt_pk = t_data(pk_idx) * 1e-6 - t0;
I_pred_pk = A * sin(omega * dt_pk) + B * cos(omega * dt_pk) + C_val;

% 预测整段, 取最大值 (排除尖峰后真正的峰值点可能在 pk_idx 附近)
I_pk_range_pred = zeros(length(pk_range), 1);
for k = 1:length(pk_range)
    ci = pk_range(k);
    dt_k = t_data(ci) * 1e-6 - t0;
    I_pk_range_pred(k) = A * sin(omega * dt_k) + B * cos(omega * dt_k) + C_val;
end

% 峰值判断: 实际值 vs 预测值
thresh = 0.3;  % 0.3A 偏差阈值
I_actual_pk = I_data(pk_idx);
if abs(I_actual_pk - I_pred_pk) > thresh
    % 尖峰: 用拟合预测的峰值代替
    Ipk_corr = max(I_pk_range_pred);
else
    % 正常: 保持实际 max()
    Ipk_corr = I_actual_pk;
end
end

function s = iff(flag, t, f)
if flag, s = t; else, s = f; end
end

function [frames, fN] = parse_csv_frames(csv_path)
fid=fopen(csv_path,'r');
fgetl(fid); ncol=length(strsplit(fgetl(fid),',')); frewind(fid); fgetl(fid);
frames={}; fN=[];
while ~feof(fid)
    line=fgetl(fid);
    if ~ischar(line)||isempty(strtrim(line)), continue; end
    if startsWith(line,'SIZE')
        parts=strsplit(line,','); N=str2double(parts{2}); fN(end+1)=N;
        fr=zeros(N,ncol);
        for i=1:N
            dl=fgetl(fid); dl=strtrim(dl);
            if isempty(dl), i=i-1; continue; end
            parts=strsplit(dl,',');
            for j=1:min(length(parts),ncol)
                v=str2double(parts{j}); if ~isnan(v), fr(i,j)=v; end
            end
        end
        frames{end+1}=fr;
    end
end
fclose(fid);
end
