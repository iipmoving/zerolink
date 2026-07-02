% export_golden.m — golden 数据导出 (与 C power_calculator.c 算法一致)
% 用法: matlab -batch "run('export_golden.m')"
% 输出: golden.json

clear; clc;

CSV_DIR = fileparts(mfilename('fullpath'));
if isempty(CSV_DIR), CSV_DIR = pwd; end
CSV_FILES = {'r104850.csv', 'r104849.csv', 'r104852.csv'};
OUTPUT = fullfile(CSV_DIR, 'golden.json');

% 标定常数 (与 power_calculator.h 一致)
V_SCALE  = 0.10606;
I_SCALE  = 0.02523;
VDC_SCALE = 0.10606;

golden = struct();
golden.files = {};

for f_idx = 1:length(CSV_FILES)
    csv_path = fullfile(CSV_DIR, CSV_FILES{f_idx});
    if ~exist(csv_path, 'file')
        fprintf('[export_golden] SKIP: %s not found\n', csv_path);
        continue;
    end

    fprintf('[export_golden] Processing %s ...\n', CSV_FILES{f_idx});

    % ---- 1. 读取原始数据 ----
    fid = fopen(csv_path, 'r');
    fgetl(fid);  % skip header

    all_t = []; all_I = []; all_Vdc = []; all_CNT = [];
    all_CU = []; all_CO = []; all_CL = []; all_LO = [];
    last_CMP = [0 0 0 0];

    while ~feof(fid)
        line = fgetl(fid);
        if ~ischar(line), break; end
        line = strtrim(line);
        if isempty(line), continue; end
        if startsWith(line, 'SIZE'), continue; end

        parts = strsplit(line, ',', 'CollapseDelimiters', false);
        first = strtrim(parts{1});
        t_val = str2double(first);
        if isnan(t_val), continue; end  % parameter row, skip

        % data row: t_us, I_adc, V_adc, Vdc_adc, CNT, CU, CO, CL, LO
        all_t(end+1)   = t_val;
        all_I(end+1)   = str2double(strtrim(parts{2}));
        all_Vdc(end+1) = str2double(strtrim(parts{4}));
        all_CNT(end+1) = str2double(strtrim(parts{5}));

        % CMP 列 (稀疏, 前向填充)
        for k = 6:9
            if length(parts) >= k
                v = str2double(strtrim(parts{k}));
                if ~isnan(v), last_CMP(k-5) = v; end
            end
        end
        all_CU(end+1) = last_CMP(1);
        all_CO(end+1) = last_CMP(2);
        all_CL(end+1) = last_CMP(3);
        all_LO(end+1) = last_CMP(4);
    end
    fclose(fid);

    n_total = length(all_t);
    fprintf('[export_golden]   %d data rows\n', n_total);

    % ---- 2. CNT 绕回检测 → 周期边界 ----
    dCNT = diff(all_CNT);
    wrap_idx = find(dCNT < 0);
    bounds = [1; wrap_idx(:)+1; n_total];
    % 只保留长度 >= 15 的周期
    cycles = {};
    for b = 1:(length(bounds)-1)
        s = bounds(b);
        e = bounds(b+1) - 1;
        if e - s + 1 >= 15
            cycles{end+1} = [s, e];
        end
    end
    fprintf('[export_golden]   %d CNT wrap cycles\n', length(cycles));

    % ---- 3. 逐周期计算 golden ----
    all_cycles = {};

    for c = 1:length(cycles)
        s = cycles{c}(1); e = cycles{c}(2);
        n = e - s + 1;

        cur = all_I(s:e);
        hrt = all_CNT(s:e);
        vlt = all_Vdc(s:e);

        % CMP 取中位数
        seg_CU = all_CU(s:e); CU = median(seg_CU(seg_CU > 0));
        seg_CO = all_CO(s:e); CO = median(seg_CO(seg_CO > 0));
        seg_CL = all_CL(s:e); CL = median(seg_CL(seg_CL > 0));
        seg_LO = all_LO(s:e); LO = median(seg_LO(seg_LO > 0));
        if isnan(CU), CU = 0; end
        if isnan(CO), CO = 0; end
        if isnan(CL), CL = 0; end
        if isnan(LO), LO = 0; end

        % perAdc
        dC = diff(hrt); dC_pos = dC(dC > 0);
        if isempty(dC_pos), per = 384; else per = round(mean(dC_pos)); end

        % input struct
        inp.highOn  = CU;
        inp.highOff = CO;
        inp.lowOn   = CL;
        inp.lowOff  = LO;
        inp.perAdc  = per;

        % ---- 3a. I_zero: 全范围最小3值均值 ----
        Isorted = sort(cur);
        I_zero = mean(Isorted(1:min(3, length(Isorted))));

        % ---- 3b. 谷值检测 (方向跟踪法, 与 C _FindValley_f 一致) ----
        start_i = 5;       % skip first 4 (1-indexed = C 的 start+4)
        co_idx = floor(inp.highOff / per) + start_i;
        if co_idx > n, co_idx = n - 2; end

        v_up = find_valley_m(cur, start_i, min(co_idx+2, n));
        v_dn = find_valley_m(cur, co_idx, n);

        if v_up == 0, v_up = start_i; end
        if v_dn == 0, v_dn = co_idx + 1; end

        % ---- 3c. P_W: I×Vdc 积分 ----
        if hrt(v_up) > inp.highOff
            end_cnt = inp.lowOff;
        else
            end_cnt = inp.highOff;
        end

        s_up = 0; vdc_sum = 0; vn = 0;
        i = v_up;
        while i <= n
            I_act = (cur(i) - I_zero) * I_SCALE;
            Vdc_v = vlt(i) * VDC_SCALE;

            weight = 1.0;
            if i < n
                dCNT_i = hrt(i+1) - hrt(i);
                if dCNT_i > 0 && dCNT_i < 10000
                    dist = end_cnt - hrt(i);
                    if dist > 0 && dist < dCNT_i
                        weight = dist / dCNT_i;
                    end
                end
            end

            s_up = s_up + I_act * Vdc_v * weight;
            vdc_sum = vdc_sum + Vdc_v;
            vn = vn + 1;

            if hrt(i) >= end_cnt, break; end
            i = i + 1;
        end

        N_cycle = inp.lowOff / per;
        if N_cycle < 1.0, N_cycle = 1.0; end

        sym = (inp.highOff * 2 + 10) >= inp.lowOff;
        s_dn = 0;
        if ~sym
            if hrt(v_dn) > inp.highOff
                end_dn = inp.highOff;
            else
                end_dn = inp.lowOff;
            end
            j = v_dn;
            while j <= n
                I_act = (cur(j) - I_zero) * I_SCALE;
                Vdc_v = vlt(j) * VDC_SCALE;
                weight = 1.0;
                if j < n
                    dCNT_j = hrt(j+1) - hrt(j);
                    if dCNT_j > 0 && dCNT_j < 10000
                        dist = end_dn - hrt(j);
                        if dist > 0 && dist < dCNT_j
                            weight = dist / dCNT_j;
                        end
                    end
                end
                s_dn = s_dn + I_act * Vdc_v * weight;
                if hrt(j) >= end_dn, break; end
                j = j + 1;
            end
        end

        if sym,        P_W = s_up * 2.0 / N_cycle;
        elseif s_dn>0, P_W = (s_up + s_dn) / N_cycle;
        else,          P_W = s_up / N_cycle;
        end
        if P_W < 0, P_W = 0; end

        % ---- 3d. f_sw ----
        f_sw_kHz = 2000.0 * per / inp.lowOff;

        % ---- 3e. I_peak / I_rms: 峰值检测法 (dI/dt 验证) ----
        I_adc_max = max(cur(cur < 60000));
        I_peak = (I_adc_max - I_zero) * I_SCALE;
        I_rms = I_peak * 0.70710678;

        % ---- 3f. phi ----
        phi_deg = 0;
        if v_up > start_i && hrt(v_up) > inp.highOn
            dist = hrt(v_up) - inp.highOn;
            T_sw = inp.lowOff;
            if T_sw > 0
                phi_deg = dist / T_sw * 360.0;
                if phi_deg > 180, phi_deg = phi_deg - 360; end
                if phi_deg < -180, phi_deg = phi_deg + 360; end
            end
        end

        % ---- 3g. f_res: v_up→v_dn 谐振半周期 ----
        f_res_kHz_i = f_sw_kHz;  % fallback = f_sw
        if v_up > start_i && v_dn > v_up && v_dn <= n
            T_half_cnt = double(hrt(v_dn)) - double(hrt(v_up));
            if T_half_cnt > 0
                T_half_us = T_half_cnt * 0.5 / per;
                f_res_kHz_i = 1000.0 / (2.0 * T_half_us);
            end
        end

        % ---- 3h. 其他 ----
        Vdc_mean = vdc_sum / vn;
        D_U = (inp.highOff - inp.highOn) / inp.lowOff * 100.0;
        DT1_us = (inp.lowOn - inp.highOff) * 0.5 / per;
        dt2 = inp.highOn - inp.lowOff;
        if dt2 < 0, dt2 = dt2 + inp.lowOff; end
        DT2_us = dt2 * 0.5 / per;
        cos_phi = cosd(phi_deg);

        % ---- 4. 打包 ----
        gc = struct();
        gc.idx = c;
        gc.input = struct(...
            'highOn', inp.highOn, 'highOff', inp.highOff, ...
            'lowOn', inp.lowOn, 'lowOff', inp.lowOff, ...
            'perAdc', inp.perAdc);
        gc.n_samples = n;
        % KVL dI/dt formula chain (matches C power_calculator.c CalculateElecParams_20ms)
        C_uF_kvl = 0.9; C_F_kvl = C_uF_kvl * 1e-6;
        omega_sw_kvl = 2 * pi * f_sw_kHz * 1e3;
        V_C_peak_kvl = I_peak / (omega_sw_kvl * C_F_kvl);
        L_uH_kvl = (Vdc_mean/2 + V_C_peak_kvl) / (I_peak * omega_sw_kvl) * 1e6;
        if L_uH_kvl < 0, L_uH_kvl = 0; end
        if L_uH_kvl > 0.001
            f_res_kvl = 1 / (2*pi*sqrt(L_uH_kvl*1e-6*C_F_kvl)) / 1e3;
        else
            f_res_kvl = f_sw_kHz;
        end
        omega_res_kvl = 2 * pi * f_res_kvl * 1e3;
        tan_phi_kvl = tand(phi_deg);
        ratio_kvl = f_sw_kHz / f_res_kvl;
        denom_kvl = ratio_kvl - 1/ratio_kvl;
        if abs(denom_kvl) > 0.001
            Q_kvl = tan_phi_kvl / denom_kvl;
        else
            Q_kvl = 0;
        end
        if Q_kvl > 0.001
            R_kvl = omega_res_kvl * L_uH_kvl * 1e-6 / Q_kvl;
        else
            R_kvl = 0;
        end
        X_L_sw_kvl = omega_sw_kvl * L_uH_kvl * 1e-6;
        X_C_sw_kvl = 1 / (omega_sw_kvl * C_F_kvl);
        X_kvl = X_L_sw_kvl - X_C_sw_kvl;
        Z_kvl = sqrt(R_kvl*R_kvl + X_kvl*X_kvl);

        gc.golden = struct(...
            'P_W',      P_W, ...
            'I_rms',    I_rms, ...
            'I_peak',   I_peak, ...
            'phi_deg',  phi_deg, ...
            'cos_phi',  cos_phi, ...
            'f_sw_kHz', f_sw_kHz, ...
            'f_res_kHz',f_res_kHz_i, ...
            'D_U_pct',  D_U, ...
            'DT1_us',   DT1_us, ...
            'DT2_us',   DT2_us, ...
            'Vdc_mean', Vdc_mean, ...
            'L_uH',     L_uH_kvl, ...
            'Q_factor', Q_kvl, ...
            'R_ohm',    R_kvl, ...
            'Z_mag_ohm',Z_kvl, ...
            'X_ohm',    X_kvl);

        all_cycles{end+1} = gc;

        fprintf('  C%d: n=%d P=%.1fW I_rms=%.3fA I_peak=%.3fA phi=%.1fdeg f_sw=%.1f f_res=%.1fkHz L=%.1fuH\n', ...
            c, n, P_W, I_rms, I_peak, phi_deg, f_sw_kHz, f_res_kHz_i);
    end

    % ---- 4. 文件级汇总: f_res + 阻抗 (与 C CalculateElecParams_20ms 一致) ----
    summary = struct();

    % 收集所有周期的平均值
    all_P_W   = zeros(1, length(all_cycles));
    all_I_rms = zeros(1, length(all_cycles));
    all_I_peak = zeros(1, length(all_cycles));
    all_Vdc   = zeros(1, length(all_cycles));
    all_D_U   = zeros(1, length(all_cycles));
    all_phi   = zeros(1, length(all_cycles));
    all_f_sw  = zeros(1, length(all_cycles));
    all_f_res = zeros(1, length(all_cycles));
    all_DT1   = zeros(1, length(all_cycles));
    all_DT2   = zeros(1, length(all_cycles));
    for i = 1:length(all_cycles)
        g = all_cycles{i}.golden;
        all_P_W(i)   = g.P_W;
        all_I_rms(i) = g.I_rms;
        all_I_peak(i)= g.I_peak;
        all_Vdc(i)   = g.Vdc_mean;
        all_D_U(i)   = g.D_U_pct;
        all_phi(i)   = g.phi_deg;
        all_f_sw(i)  = g.f_sw_kHz;
        all_f_res(i) = g.f_res_kHz;
        all_DT1(i)   = g.DT1_us;
        all_DT2(i)   = g.DT2_us;
    end

    P_W_avg   = mean(all_P_W);
    I_rms_avg = mean(all_I_rms);
    Vdc_avg   = mean(all_Vdc);
    D_U_avg   = mean(all_D_U) / 100.0;
    phi_avg   = mean(all_phi);

    % ---- f_res: per-cycle v_up→v_dn 谷值间隔法均值 (≈f_sw) ----
    f_valley_kHz = mean(all_f_res);

    % ======== 阻抗参数 (KVL dI/dt 封闭解, 从 L 反推) ========
    C_uF = 0.9;
    C_F  = C_uF * 1e-6;
    f_sw_avg = mean(all_f_sw);
    f_sw_Hz  = f_sw_avg * 1e3;
    omega_sw = 2 * pi * f_sw_Hz;
    I_peak_avg = mean(all_I_peak);

    % L: KVL dI/dt 封闭解 (dI/dt 验证)
    %   Vdc/2 + V_C_peak = L × I_peak × ω_sw
    %   V_C_peak = I_peak / (ω_sw × C)
    V_C_peak = I_peak_avg / (omega_sw * C_F);
    L_uH = (Vdc_avg/2 + V_C_peak) / (I_peak_avg * omega_sw) * 1e6;
    if L_uH < 0, L_uH = 0; end

    % f_res: 从 L 和 C 反推
    if L_uH > 0.001
        f_res_Hz = 1 / (2 * pi * sqrt(L_uH * 1e-6 * C_F));
    else
        f_res_Hz = f_sw_Hz;  % fallback
    end
    f_res_kHz = f_res_Hz / 1e3;
    omega_res = 2 * pi * f_res_Hz;

    % Q: 从相位角和频率比
    tan_phi = tand(phi_avg);
    ratio = f_sw_Hz / f_res_Hz;
    if abs(ratio - 1/ratio) > 0.001
        Q_factor = tan_phi / (ratio - 1/ratio);
    else
        Q_factor = 0;
    end

    % R: 从 Q 和 L
    if Q_factor > 0.001
        R_ohm = omega_res * L_uH * 1e-6 / Q_factor;
    else
        R_ohm = 0;
    end

    % 阻抗 @ f_sw (工作频率下的阻抗, 非谐振点)
    X_L_sw = omega_sw * L_uH * 1e-6;
    X_C_sw = 1 / (omega_sw * C_F);
    X_ohm  = X_L_sw - X_C_sw;
    Z_mag  = sqrt(R_ohm * R_ohm + X_ohm * X_ohm);

    cos_phi_avg = cosd(phi_avg);

    summary.f_res_kHz  = f_res_kHz;
    summary.f_sw_kHz   = f_sw_avg;
    summary.Vdc_mean   = Vdc_avg;
    summary.P_W        = P_W_avg;
    summary.I_rms      = I_rms_avg;
    summary.I_peak     = I_peak_avg;
    summary.phi_deg    = phi_avg;
    summary.cos_phi    = cos_phi_avg;
    summary.D_U_pct    = D_U_avg * 100.0;
    summary.DT1_us     = mean(all_DT1);
    summary.DT2_us     = mean(all_DT2);
    summary.Z_mag_ohm  = Z_mag;
    summary.R_ohm      = R_ohm;
    summary.X_ohm      = X_ohm;
    summary.L_uH       = L_uH;
    summary.Q_factor   = Q_factor;
    summary.valid      = (I_rms_avg > 0.001);

    fprintf('  Summary: f_res=%.2fkHz P=%.1fW I_rms=%.3fA phi=%.1fdeg L=%.1fuH Q=%.2f Z=%.3f R=%.3f X=%.2f (KVL dI/dt)\n', ...
        f_res_kHz, P_W_avg, I_rms_avg, phi_avg, L_uH, Q_factor, Z_mag, R_ohm, X_ohm);

    fd = struct();
    fd.name = CSV_FILES{f_idx};
    fd.n_cycles = length(all_cycles);
    fd.summary = summary;
    fd.cycles = all_cycles;
    golden.files{end+1} = fd;
end

% ---- 导出 JSON ----
fprintf('[export_golden] Writing %s ...\n', OUTPUT);
jsonStr = jsonencode(golden, 'PrettyPrint', true);
fid = fopen(OUTPUT, 'w');
fprintf(fid, '%s', jsonStr);
fclose(fid);
fprintf('[export_golden] Done. %d files.\n', length(golden.files));


%% ---- 子函数: 谷值检测 (方向跟踪法, 与 C _FindValley_f 一致) ----
function v = find_valley_m(adc, start_i, end_i)
    v = 0;
    if end_i <= start_i, return; end
    direction = uint16(65535);  % 0xFFFF
    min_sum = uint32(4294967295);  % 0xFFFFFFFF
    pre = double(adc(start_i));

    for i = (start_i + 1):min(end_i, start_i + 199)
        cur = double(adc(i));
        direction = bitshift(direction, 1);
        if cur > pre
            direction = bitor(direction, uint16(1));
        else
            direction = bitand(direction, bitcmp(uint16(1)));
        end

        if bitand(direction, uint16(3)) == 1
            pi = i - 1;
            if pi > start_i && (pi + 1) <= end_i
                zr = uint32(adc(pi - 1)) + uint32(adc(pi + 1));
                mid2 = uint32(adc(pi)) * 2;
                if mid2 <= zr && zr < min_sum
                    min_sum = zr;
                    v = pi;
                end
            end
        end
        pre = cur;
    end

end
