% validate_didt_method.m
% 对比两种 L 计算方法:
%   Method 1 (I_peak*omega): L = (Vdc/2 + V_C_peak) / (I_peak * omega)
%   Method 2 (实测 dI/dt):   L = (Vdc/2 + V_C_peak) / (di_dt_measured)
%
% 相位角: 文件内有效周期平均, 剔除低 I_peak 周期

function validate_didt_method()
    close all;

    base = '../../tools/ekf_tuner/captures/captures/';
    steel_file = 'capture_20260602_152037.csv';
    iron_file  = 'capture_20260602_153316.csv';

    fprintf('=============================================================\n');
    fprintf('  dI/dt 验证: I_peak*omega 法 vs 实测 dI/dt 法\n');
    fprintf('=============================================================\n');

    for pot_idx = 1:2
        if pot_idx == 1
            csv_name = steel_file;
            pot_label = 'STEEL POT (钢锅)';
        else
            csv_name = iron_file;
            pot_label = 'IRON POT (铁锅)';
        end

        csv_path = [base csv_name];
        fprintf('\n========== %s ==========\n', pot_label);
        fprintf('File: %s\n', csv_name);

        result = process_file(csv_path);
        print_results(result);
    end
end

function result = process_file(csv_path)
    %% 硬件常数
    C_FARAD = 0.90e-6;
    I_SCALE = 3.3/4096/(330/(10e3+330));
    V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
    HRTIM_CLK = 768e6;

    %% 加载
    [frames, n_cycles] = load_csv_frames(csv_path);
    if n_cycles == 0
        result = []; return;
    end

    %% 逐周期处理
    cycles_L1 = [];  % I_peak*omega 法
    cycles_L2 = [];  % 实测 dI/dt 法
    cycles_phi = [];
    cycles_Ipk = [];
    cycles_fsw = [];
    cycles_P = [];
    cycles_Vdc = [];
    cycles_didt = [];
    cycles_didt_ratio = [];  % measured_didt / (Ipk*omega)

    for fi = 1:n_cycles
        frame = frames{fi};
        N = size(frame, 1);
        if N < 15, continue; end

        t_us   = frame(:,1);
        I_adc  = frame(:,2);
        Vdc_adc = frame(:,4);
        CNT    = frame(:,5);
        CMP    = frame(1, 6:9);
        CU = CMP(1); CO = CMP(2);

        %% 清洗: 去尾部饱和
        seg_end = N;
        while seg_end > 1 && I_adc(seg_end) > 4090
            seg_end = seg_end - 1;
        end
        if seg_end > 1, seg_end = seg_end - 1; end
        if seg_end < 20, continue; end

        I_clean = I_adc(1:seg_end) * I_SCALE;
        CNT_clean = CNT(1:seg_end);
        t_clean = t_us(1:seg_end);
        Vdc_mean = mean(Vdc_adc(1:seg_end)) * V_SCALE;

        %% 频率
        dCNT = diff(CNT_clean);
        dCNT_pos = dCNT(dCNT > 0);
        if isempty(dCNT_pos), continue; end
        avg_dcnt = mean(dCNT_pos);
        dt_us = mean(diff(t_clean(diff(t_clean) > 0)));
        t_per_cnt_us = dt_us / avg_dcnt;

        CNT_all = CNT(1:seg_end);
        wi = find(diff(CNT_all) < 0, 1);
        if ~isempty(wi)
            CNT_range = CNT_all(wi) + round(avg_dcnt) - CNT_all(wi+1);
        else
            CNT_range = max(CNT_all) - min(CNT_all);
        end
        f_sw = 1 / (CNT_range * t_per_cnt_us * 1e-6);
        omega = 2 * pi * f_sw;

        %% I_peak (鲁棒)
        I_clean_sorted = sort(I_clean);
        I_peak = I_clean_sorted(max(1, round(length(I_clean_sorted) * 0.98)));

        %% 谷点 (过零点) 找法: 找 < 15% I_peak 的局部最小值
        valley_candidates = [];
        for i = 3:(length(I_clean)-2)
            if I_clean(i) < I_peak * 0.20 ...
               && I_clean(i) <= I_clean(i-1) ...
               && I_clean(i) <= I_clean(i+1)
                valley_candidates(end+1) = i;
            end
        end

        if isempty(valley_candidates)
            % fallback: 找全局最小在上升前的谷
            [~, vi] = min(I_clean(1:round(length(I_clean)*0.5)));
        else
            vi = valley_candidates(1);
        end

        %% Method 2: 实测 dI/dt at valley
        if vi > 1 && vi < seg_end
            di_adc = I_adc(vi+1) - I_adc(vi-1);
            di_amps = di_adc * I_SCALE;

            % dt from CNT
            dt_cnt = CNT(vi+1) - CNT(vi-1);
            if dt_cnt < 0
                dt_cnt = dt_cnt + CNT_range;
            end
            dt_sec = dt_cnt * t_per_cnt_us * 1e-6;

            if dt_sec > 0
                didt_measured = di_amps / dt_sec;
            else
                didt_measured = NaN;
            end
        else
            didt_measured = NaN;
        end

        %% Method 1: I_peak * omega (理论值)
        didt_theoretical = I_peak * omega;

        %% phi: 从谷点位置算
        if ~isnan(didt_measured) && vi > 1
            t_valley_us = t_clean(vi);
            % 找到 CNT 过 CMP_UOFF 的点 (上管关断)
            % 简化: 用谷点 CNT 和 CU 的差算相位
            cnt_valley = CNT(vi);
            dc = cnt_valley - CU;
            if dc < 0, dc = dc + CNT_range; end
            phi_deg = (dc / CNT_range) * 360;
            if phi_deg > 180, phi_deg = phi_deg - 360; end
        else
            phi_deg = NaN;
        end

        %% V_C_peak
        V_C_peak = I_peak / (omega * C_FARAD);

        %% L calculation
        % Method 1: L1 = (Vdc/2 + V_C_peak) / (I_peak * omega)
        L1 = (Vdc_mean/2 + V_C_peak) / (I_peak * omega) * 1e6;  % uH

        % Method 2: L2 = (Vdc/2 + V_C_peak) / (measured_di_dt)
        if ~isnan(didt_measured) && didt_measured > 0
            L2 = (Vdc_mean/2 + V_C_peak) / didt_measured * 1e6;  % uH
        else
            L2 = NaN;
        end

        %% P = I_rms^2 * R (近似, 需要 Q)
        % 这里只记录, 不用于方法比较

        %% 记录 (只记录 L1/L2 同时有效的周期)
        if ~isnan(L2)
            cycles_L1(end+1) = L1;
            cycles_L2(end+1) = L2;
            cycles_didt(end+1) = didt_measured;
            cycles_didt_ratio(end+1) = didt_measured / didt_theoretical;
            cycles_Ipk(end+1) = I_peak;
            cycles_fsw(end+1) = f_sw / 1e3;
            cycles_Vdc(end+1) = Vdc_mean;
            if ~isnan(phi_deg)
                cycles_phi(end+1) = phi_deg;
            end
        end

        % 功率估算
        I_rms_est = I_peak / sqrt(2);
        cycles_P(end+1) = Vdc_mean * I_rms_est;  % 近似
    end

    %% 相位角: 文件内平均, 去掉低 I_peak
    if ~isempty(cycles_Ipk) && ~isempty(cycles_phi)
        Ipk_max_file = max(cycles_Ipk);
        Ipk_threshold = Ipk_max_file * 0.5;  % 低于 50% 峰值的不参与 phi 平均
        valid_phi_mask = cycles_Ipk >= Ipk_threshold;
        phi_avg = mean(cycles_phi(valid_phi_mask));
        phi_std = std(cycles_phi(valid_phi_mask));
        n_phi_used = sum(valid_phi_mask);
        n_phi_total = length(cycles_phi);
    else
        phi_avg = NaN; phi_std = NaN;
        n_phi_used = 0; n_phi_total = 0;
    end

    %% 打包
    result.csv_name = csv_path;
    result.n_cycles = n_cycles;
    result.n_valid = length(cycles_L1);
    result.phi_avg = phi_avg;
    result.phi_std = phi_std;
    result.n_phi_used = n_phi_used;
    result.n_phi_total = n_phi_total;
    result.Ipk_threshold = Ipk_max_file * 0.5;
    result.L1_mean = mean(cycles_L1);
    result.L1_std  = std(cycles_L1);
    result.L1_cv   = std(cycles_L1) / mean(cycles_L1) * 100;
    result.L2_mean = mean(cycles_L2);
    result.L2_std  = std(cycles_L2);
    result.L2_cv   = std(cycles_L2) / mean(cycles_L2) * 100;
    result.delta_L_mean = mean(cycles_L2 - cycles_L1);
    result.delta_L_std  = std(cycles_L2 - cycles_L1);
    result.didt_ratio_mean = mean(cycles_didt_ratio);
    result.didt_ratio_std  = std(cycles_didt_ratio);
    result.L1_all = cycles_L1;
    result.L2_all = cycles_L2;
    result.didt_ratio_all = cycles_didt_ratio;
    result.Ipk_all = cycles_Ipk;
    result.phi_all = cycles_phi;
    result.fsw_all = cycles_fsw;
end

function print_results(r)
    fprintf('\n--- File statistics ---\n');
    fprintf('Total cycles in file: %d\n', r.n_cycles);
    fprintf('Valid cycles:          %d\n', r.n_valid);
    fprintf('\n--- Phase angle (file average) ---\n');
    fprintf('I_peak threshold:      %.1f A (50%% of max I_peak)\n', r.Ipk_threshold);
    fprintf('Phi cycles used/avail: %d / %d\n', r.n_phi_used, r.n_phi_total);
    fprintf('Phi_avg (file):        %.2f +/- %.2f deg\n', r.phi_avg, r.phi_std);
    fprintf('\n--- Method 1: I_peak * omega ---\n');
    fprintf('L1 mean:  %.2f uH\n', r.L1_mean);
    fprintf('L1 std:   %.2f uH\n', r.L1_std);
    fprintf('L1 CV:    %.2f %%\n', r.L1_cv);
    fprintf('\n--- Method 2: measured dI/dt ---\n');
    fprintf('L2 mean:  %.2f uH\n', r.L2_mean);
    fprintf('L2 std:   %.2f uH\n', r.L2_std);
    fprintf('L2 CV:    %.2f %%\n', r.L2_cv);
    fprintf('\n--- Delta (Method2 - Method1) ---\n');
    fprintf('Delta L mean:  %+.2f uH\n', r.delta_L_mean);
    fprintf('Delta L std:   %.2f uH\n', r.delta_L_std);
    fprintf('Delta L pct:   %+.2f %%\n', r.delta_L_mean / r.L1_mean * 100);
    fprintf('\n--- dI/dt ratio (measured / Ipk*omega) ---\n');
    fprintf('Ratio mean:  %.4f\n', r.didt_ratio_mean);
    fprintf('Ratio std:   %.4f\n', r.didt_ratio_std);
    fprintf('(Ratio=1.0 means pure sine at zero-crossing)\n');

    % 逐周期明细
    fprintf('\n--- Per-cycle detail ---\n');
    fprintf('Cycle  Ipk(A)  fsw(kHz)  phi(deg)  L1(uH)  L2(uH)  delta(uH)  di/dt ratio\n');
    fprintf('%s\n', repmat('-', 1, 78));
    n = min(length(r.L1_all), length(r.L2_all));
    for i = 1:n
        phi_i = NaN;
        if i <= length(r.phi_all), phi_i = r.phi_all(i); end
        fsw_i = NaN;
        if i <= length(r.fsw_all), fsw_i = r.fsw_all(i); end
        fprintf('%3d    %6.1f  %7.2f   %7.2f   %6.2f  %6.2f   %+7.2f     %.4f\n', ...
            i, r.Ipk_all(i), fsw_i, phi_i, ...
            r.L1_all(i), r.L2_all(i), r.L2_all(i)-r.L1_all(i), ...
            r.didt_ratio_all(i));
    end
end

function [frames, n_cycles] = load_csv_frames(csv_path)
    fid = fopen(csv_path, 'r');
    if fid < 0
        frames = {}; n_cycles = 0; return;
    end
    header = fgetl(fid);
    ncol = length(strsplit(header, ','));

    frames = {};
    while ~feof(fid)
        line = fgetl(fid);
        if ~ischar(line) || isempty(strtrim(line))
            continue;
        end
        if startsWith(line, 'SIZE')
            parts = strsplit(line, ',');
            N = str2double(parts{2});
            fr = zeros(N, ncol);
            for i = 1:N
                dl = fgetl(fid);
                dl = strtrim(dl);
                if isempty(dl), i = i - 1; continue; end
                parts = strsplit(dl, ',');
                for j = 1:min(length(parts), ncol)
                    v = str2double(parts{j});
                    if ~isnan(v), fr(i,j) = v; end
                end
            end
            frames{end+1} = fr;
        end
    end
    fclose(fid);
    n_cycles = length(frames);
end
