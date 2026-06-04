% verify_L_reference.m — 用 reference CSV 重新计算所有参数
% 公式链: L (KVL dI/dt) → f_res → Q(phi) → R → Z, X
clear; clc;

CSV_PATH = 'D:\OBSIDIAN\MOVING IH\低耦合程序架构\m4_ekf_observer\tools\ekf_tuner\capture_20260602_104850_result.csv';

% 标定常数
V_SCALE  = 0.10606;
I_SCALE  = 0.02522;
VDC_SCALE = 0.10606;
C_uF = 0.9;
C_F = C_uF * 1e-6;

%% 读取第一周期
fid = fopen(CSV_PATH, 'r');
hdr = strsplit(fgetl(fid), ',', 'CollapseDelimiters', false);
fgetl(fid);  % SIZE row

all_t=[]; all_I=[]; all_Vdc=[]; all_CNT=[]; all_CU=[]; all_CO=[]; all_CL=[]; all_LO=[];
last_CMP=[0 0 0 0]; ref=[];

while ~feof(fid)
    line = fgetl(fid);
    if ~ischar(line), break; end
    line = strtrim(line);
    if isempty(line), break; end
    parts = strsplit(line, ',', 'CollapseDelimiters', false);
    t_val = str2double(strtrim(parts{1}));
    if isnan(t_val), break; end

    all_t(end+1)   = t_val;
    all_I(end+1)   = str2double(strtrim(parts{2}));
    all_Vdc(end+1) = str2double(strtrim(parts{4}));
    all_CNT(end+1) = str2double(strtrim(parts{5}));

    for k = 6:9
        if length(parts) >= k
            v = str2double(strtrim(parts{k}));
            if ~isnan(v), last_CMP(k-5)=v; end
        end
    end
    all_CU(end+1)=last_CMP(1); all_CO(end+1)=last_CMP(2);
    all_CL(end+1)=last_CMP(3); all_LO(end+1)=last_CMP(4);

    % 提取 reference 值 (只用第一行)
    if isempty(ref) && length(parts) >= 24
        ref.f_sw  = str2double(strtrim(parts{10}));
        ref.DT1   = str2double(strtrim(parts{11}));
        ref.DT2   = str2double(strtrim(parts{12}));
        ref.D_U   = str2double(strtrim(parts{13}));
        ref.I_pk  = str2double(strtrim(parts{16}));
        ref.I_rms = str2double(strtrim(parts{17}));
        ref.phi   = str2double(strtrim(parts{18}));
        ref.cosf  = str2double(strtrim(parts{19}));
        ref.P_W   = str2double(strtrim(parts{20}));
        ref.L_uH  = str2double(strtrim(parts{21}));
        ref.Q     = str2double(strtrim(parts{22}));
        ref.R_ohm = str2double(strtrim(parts{23}));
        ref.f_res = str2double(strtrim(parts{24}));
    end
end
fclose(fid);

n = length(all_t);
fprintf('=== %d samples ===\n', n);

%% CMP
CU=median(all_CU(all_CU>0)); CO=median(all_CO(all_CO>0));
CL=median(all_CL(all_CL>0)); LO=median(all_LO(all_LO>0));
if isnan(CU),CU=0; end; if isnan(CO),CO=0; end
if isnan(CL),CL=0; end; if isnan(LO),LO=0; end
fprintf('CMP: CU=%.0f CO=%.0f CL=%.0f LO=%.0f\n', CU, CO, CL, LO);

%% perAdc
dCNT = diff(all_CNT);
perAdc = round(mean(dCNT(dCNT > 0)));
fprintf('perAdc=%d\n', perAdc);

%% ==== 1. 时序参数 ====
f_sw_kHz = 2000 * perAdc / LO;
D_U_pct  = (CO - CU) / LO * 100;
DT1_us   = (CL - CO) * 0.5 / perAdc;
% DT2: 下管关→上管开, 跨CNT绕回
dt2_cnt  = CU + 65536 - LO;
DT2_us   = dt2_cnt * 0.5 / perAdc;
f_sw_Hz  = f_sw_kHz * 1e3;
omega_sw = 2 * pi * f_sw_Hz;

fprintf('\n--- 时序 ---\n');
fprintf('f_sw=%.2f kHz  D_U=%.2f%%  DT1=%.3f us  DT2=%.3f us\n', f_sw_kHz, D_U_pct, DT1_us, DT2_us);
fprintf('  ref:  f_sw=%.2f  D_U=%.2f  DT1=%.3f  DT2=%.3f\n', ref.f_sw, ref.D_U, ref.DT1, ref.DT2);

%% ==== 2. I_zero + 谷值检测 ====
Isorted = sort(all_I);
I_zero = mean(Isorted(1:min(3, length(Isorted))));

start_i = 5;
co_idx = floor(CO / perAdc) + start_i;
if co_idx > n, co_idx = n-2; end

v_up = find_valley_m(all_I, start_i, min(co_idx+2, n));
v_dn = find_valley_m(all_I, co_idx, n);
if v_up==0, v_up=start_i; end
if v_dn==0, v_dn=co_idx+1; end
fprintf('I_zero=%.1f  v_up=%d(I=%d)  v_dn=%d(I=%d)\n', I_zero, v_up, all_I(v_up), v_dn, all_I(v_dn));

%% ==== 3. I_peak / I_rms (peak method — 与 reference 一致) ====
I_peak = (max(all_I(all_I < 60000)) - I_zero) * I_SCALE;
I_rms  = I_peak * 0.70710678;
fprintf('\n--- 电流 ---\n');
fprintf('I_peak=%.3f A  I_rms=%.4f A\n', I_peak, I_rms);
fprintf('  ref: I_peak=%.3f  I_rms=%.4f\n', ref.I_pk, ref.I_rms);

%% ==== 4. phi (谷值 vs highOn) ====
phi_deg = 0;
if v_up > start_i && all_CNT(v_up) > CU
    dist = all_CNT(v_up) - CU;
    if LO > 0
        phi_deg = dist / LO * 360.0;
        if phi_deg > 180, phi_deg = phi_deg - 360; end
        if phi_deg < -180, phi_deg = phi_deg + 360; end
    end
end
fprintf('\n--- 相位 ---\n');
fprintf('phi=%.2f deg  cos=%.4f\n', phi_deg, cosd(phi_deg));
fprintf('  ref: phi=%.2f  cos=%.4f\n', ref.phi, ref.cosf);

%% ==== 5. P_W (I×Vdc 加权积分) ====
Vdc_mean = mean(all_Vdc) * VDC_SCALE;

if all_CNT(v_up) > CO, end_cnt = CL; else, end_cnt = CO; end

s_up=0; vdc_sum=0; vn=0; i=v_up;
while i <= n
    I_act = (all_I(i) - I_zero) * I_SCALE;
    Vdc_v = all_Vdc(i) * VDC_SCALE;
    weight = 1.0;
    if i < n
        dC = all_CNT(i+1) - all_CNT(i);
        if dC > 0 && dC < 10000
            d2 = end_cnt - all_CNT(i);
            if d2 > 0 && d2 < dC, weight = d2/dC; end
        end
    end
    s_up = s_up + I_act * Vdc_v * weight;
    vdc_sum = vdc_sum + Vdc_v; vn = vn + 1;
    if all_CNT(i) >= end_cnt, break; end
    i = i + 1;
end

N_cycle = CL / perAdc;
if N_cycle < 1, N_cycle = 1; end
sym = (CO*2+10) >= CL;

if sym
    P_W = s_up * 2.0 / N_cycle;
else
    if all_CNT(v_dn) > CO, end_dn=CO; else, end_dn=CL; end
    s_dn=0; j=v_dn;
    while j <= n
        I_act = (all_I(j) - I_zero) * I_SCALE;
        Vdc_v = all_Vdc(j) * VDC_SCALE;
        weight = 1.0;
        if j < n
            dC = all_CNT(j+1) - all_CNT(j);
            if dC > 0 && dC < 10000
                d2 = end_dn - all_CNT(j);
                if d2 > 0 && d2 < dC, weight = d2/dC; end
            end
        end
        s_dn = s_dn + I_act * Vdc_v * weight;
        if all_CNT(j) >= end_dn, break; end
        j = j + 1;
    end
    if s_dn > 0, P_W = (s_up+s_dn)/N_cycle; else, P_W = s_up/N_cycle; end
end
if P_W < 0, P_W = 0; end

fprintf('\n--- 功率 ---\n');
fprintf('P_W=%.3f W  Vdc=%.1f V\n', P_W, Vdc_mean);
fprintf('  ref: P_W=%.3f\n', ref.P_W);

%% ====== ★ 核心: L (KVL dI/dt 封闭解) ======
V_C_peak = I_peak / (omega_sw * C_F);
L_uH = (Vdc_mean/2 + V_C_peak) / (I_peak * omega_sw) * 1e6;

fprintf('\n========== ★ L (KVL dI/dt) ==========\n');
fprintf('  Vdc/2          = %.1f V\n', Vdc_mean/2);
fprintf('  I_peak         = %.4f A\n', I_peak);
fprintf('  omega_sw       = 2π×%.2fkHz = %.1f rad/s\n', f_sw_kHz, omega_sw);
fprintf('  V_C_peak       = I_peak/(ωC) = %.2f V\n', V_C_peak);
fprintf('  L = (Vdc/2 + V_C_peak) / (I_peak×ω_sw)\n');
fprintf('    = (%.1f + %.1f) / (%.4f × %.1f)\n', Vdc_mean/2, V_C_peak, I_peak, omega_sw);
fprintf('    = %.1f / %.1f\n', Vdc_mean/2+V_C_peak, I_peak*omega_sw);
fprintf('    = %.2f uH\n', L_uH);
fprintf('  ref: L = %.2f uH\n', ref.L_uH);

%% ====== ★ 从 L 反推其它参数 ======
f_res_Hz  = 1 / (2*pi*sqrt(L_uH*1e-6 * C_F));
f_res_kHz = f_res_Hz / 1e3;
omega_res = 2 * pi * f_res_Hz;

fprintf('\n========== 从 L 反推 ==========\n');
fprintf('  f_res = 1/(2π√(LC)) = %.2f kHz  (ref: %.2f)\n', f_res_kHz, ref.f_res);

% Q from phase angle formula
ratio  = f_sw_Hz / f_res_Hz;
tan_phi = tand(phi_deg);
Q_val  = tan_phi / (ratio - 1/ratio);
R_ohm  = omega_res * L_uH * 1e-6 / Q_val;

fprintf('  ratio = f_sw/f_res = %.3f\n', ratio);
fprintf('  tan(φ) = %.4f\n', tan_phi);
fprintf('  Q = tan(φ) / (ratio - 1/ratio) = %.3f  (ref: %.3f)\n', Q_val, ref.Q);
fprintf('  R = ω_res×L/Q = %.3f Ω  (ref: %.3f)\n', R_ohm, ref.R_ohm);

% 阻抗
X_L = omega_res * L_uH * 1e-6;
X_C = 1 / (omega_res * C_F);
X_ohm = X_L - X_C;
Z_mag = sqrt(R_ohm^2 + X_ohm^2);
cos_phi_est = R_ohm / Z_mag;

fprintf('\n--- 阻抗 ---\n');
fprintf('  X_L = ωL = %.3f Ω\n', X_L);
fprintf('  X_C = 1/(ωC) = %.3f Ω\n', X_C);
fprintf('  X   = X_L - X_C = %.3f Ω\n', X_ohm);
fprintf('  |Z| = √(R²+X²) = %.3f Ω\n', Z_mag);
fprintf('  cos(φ) = R/|Z| = %.4f  (从phi: %.4f)\n', cos_phi_est, cosd(phi_deg));

% 交叉验证
P_from_R = I_rms^2 * R_ohm;
fprintf('\n--- 交叉验证 ---\n');
fprintf('  P_from_R = I_rms²×R = %.2f²×%.3f = %.2f W\n', I_rms, R_ohm, P_from_R);
fprintf('  P_W (积分) = %.2f W  (Δ=%.1fW)\n', P_W, abs(P_from_R-P_W));

%% ====== 对比汇总 ======
fprintf('\n========== 汇总 ==========\n');
fprintf('%-15s %10s %10s %8s\n', 'Param', 'Computed', 'Ref', 'Δ');
fprintf('%-15s %10.2f %10.2f %8.2f\n', 'f_sw[kHz]', f_sw_kHz, ref.f_sw, f_sw_kHz-ref.f_sw);
fprintf('%-15s %10.3f %10.3f %8.3f\n', 'DT1[us]', DT1_us, ref.DT1, DT1_us-ref.DT1);
fprintf('%-15s %10.3f %10.3f %8.3f\n', 'DT2[us]', DT2_us, ref.DT2, DT2_us-ref.DT2);
fprintf('%-15s %10.2f %10.2f %8.2f\n', 'D_U[%%]', D_U_pct, ref.D_U, D_U_pct-ref.D_U);
fprintf('%-15s %10.3f %10.3f %8.3f\n', 'I_peak[A]', I_peak, ref.I_pk, I_peak-ref.I_pk);
fprintf('%-15s %10.4f %10.4f %8.4f\n', 'I_rms[A]', I_rms, ref.I_rms, I_rms-ref.I_rms);
fprintf('%-15s %10.2f %10.2f %8.2f\n', 'phi[deg]', phi_deg, ref.phi, phi_deg-ref.phi);
fprintf('%-15s %10.3f %10.3f %8.3f\n', 'P_W[W]', P_W, ref.P_W, P_W-ref.P_W);
fprintf('%-15s %10.2f %10.2f %8.2f\n', 'L[uH]', L_uH, ref.L_uH, L_uH-ref.L_uH);
fprintf('%-15s %10.3f %10.3f %8.3f\n', 'Q', Q_val, ref.Q, Q_val-ref.Q);
fprintf('%-15s %10.3f %10.3f %8.3f\n', 'R[ohm]', R_ohm, ref.R_ohm, R_ohm-ref.R_ohm);
fprintf('%-15s %10.2f %10.2f %8.2f\n', 'f_res[kHz]', f_res_kHz, ref.f_res, f_res_kHz-ref.f_res);

fprintf('\n========== 完成 ==========\n');

%% ---- 子函数 ----
function v = find_valley_m(adc, start_i, end_i)
    v = 0;
    if end_i <= start_i, return; end
    direction = uint16(65535);
    min_sum = uint32(4294967295);
    pre = double(adc(start_i));
    for i = (start_i+1):min(end_i, start_i+199)
        cur = double(adc(i));
        direction = bitshift(direction, 1);
        if cur > pre
            direction = bitor(direction, uint16(1));
        else
            direction = bitand(direction, bitcmp(uint16(1)));
        end
        if bitand(direction, uint16(3)) == 1
            pi = i - 1;
            if pi > start_i && (pi+1) <= end_i
                zr = uint32(adc(pi-1)) + uint32(adc(pi+1));
                mid2 = uint32(adc(pi)) * 2;
                if mid2 <= zr && zr < min_sum
                    min_sum = zr; v = pi;
                end
            end
        end
        pre = cur;
    end
end
