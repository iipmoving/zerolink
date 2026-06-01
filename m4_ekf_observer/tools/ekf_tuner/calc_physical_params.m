%% calc_physical_params.m
% 半桥谐振参数物理量计算 — 需要硬件参数输入
%
% 输入: 每周期 ADC 原始数据 [Index, HRTIM, Vc_ADC, I_ADC, I_FMAC]
%       从 MODBUS 0x4000 或 printf CSV 获取
% 输出: f0(Hz), Q, L(μH), C_calc(μF), I_peak(A), I_rms(A),
%       V_bus(V), P(W), ESR(Ω), phase_angle(deg)
%
% 使用: 修改下方 "硬件参数" 节, 然后运行

clear; close all; clc;

%% ====== 硬件参数 =========================================================
% 电压: 3×270K 上拉, 6.2K 下拉 → 分压比 1/131.6
% 电流: CT 2000:1 → R_burden 2K → 全波整流 → 分压 10K+330 → ADC (无运放)

% ---- 电压采样 ----
HW.V_ref        = 3.3;          % ADC 参考电压 (V)
HW.ADC_bits     = 12;           % ADC 分辨率
HW.ADC_max      = 2^HW.ADC_bits; % 4096
HW.V_div_top    = 810e3;        % 3×270K (Ω)
HW.V_div_bot    = 6.2e3;        % 6.2K (Ω)
HW.V_div_ratio  = HW.V_div_bot / (HW.V_div_top + HW.V_div_bot);  % 1/131.6
HW.V_per_adc    = HW.V_ref / HW.ADC_max / HW.V_div_ratio;  % V/ADC

% ---- 电流采样 (互感器) ----
HW.CT_ratio     = 2000;         % 互感器 2000:1
HW.R_burden     = 2000;         % 采样电阻 2K (Ω)
HW.R_div_top    = 10e3;         % 整流后分压上电阻 10K
HW.R_div_bot    = 330;          % 整流后分压下电阻 330Ω
HW.I_div_ratio  = HW.R_div_bot / (HW.R_div_top + HW.R_div_bot);  % 1/31.3
HW.opamp_gain   = 1;            % 无运放, 直入 ADC
% I_per_adc: 1A初级 → CT(1/2000) → R_burden(2K)=1V → 分压(1/31.3)=31.9mV
%   → ADC: 31.9mV / 3.3V * 4096 = 39.6 counts/A
%   → 25.2 mA/count
HW.I_per_adc    = (HW.V_ref / HW.ADC_max) ...
                / (HW.R_burden / HW.CT_ratio * HW.I_div_ratio) ...
                / HW.opamp_gain;  % A/ADC

% ---- 校准点: 0x60=96 → 2300W 限功率, 标定用 ----
HW.I_limit_reg  = 0x60;         % 电流限制寄存器值
HW.P_limit_W    = 2300;         % 对应限制功率 (W)

% ---- HRTIM ----
HW.HRTIM_CLK    = 768e6;        % 有效时钟 (Hz) — FRE_PER_ADC=384 per 0.5us

% ---- 谐振槽 ----
HW.C_known_uF   = 0.90;         % 0.45μF×2 并联
HW.L_iron_30k_uH = 65;          % 铁锅 @30kHz
HW.L_iron_20k_uH = 70;          % 铁锅 @20kHz
HW.L_steel_30k_uH = 57;         % 钢锅 @30kHz
HW.PERIOD_TICKS = 26112;        % PWM 周期 (HRTIM ticks), ~29.4kHz
HW.F_SW         = HW.HRTIM_CLK / HW.PERIOD_TICKS;

fprintf('\n===== 硬件参数 =====\n');
fprintf('电压: %.2f mV/ADC  (分压比 1/%.0f)\n', HW.V_per_adc*1000, 1/HW.V_div_ratio);
fprintf('电流: %.3f mA/ADC  (CT %d:1, R_burden=%.0fΩ)\n', HW.I_per_adc*1000, HW.CT_ratio, HW.R_burden);
fprintf('HRTIM: %.0f MHz\n', HW.HRTIM_CLK/1e6);
fprintf('谐振槽: L=%.0f μH, C=%.2f μF, f0_theory=%.0f Hz\n', ...
    HW.L_known_uH, HW.C_known_uF, ...
    1/(2*pi*sqrt(HW.L_known_uH*1e-6*HW.C_known_uF*1e-6)));

%% ====== 数据加载 ========================================================
% 方式1: 从 CSV 文件加载 (每行一个采样点)
%   data = csvread('m4_ekf_YYYYMMDD_HHMMSS.csv');
%
% 方式2: 嵌入式数据 (调试用) — 以下 3 组数据对应不同 Vbus 工况

D1 = [  % 高电压, phase≈90
     0, 25304, 2854, 622, 466;  1, 25688, 2856, 633, 670;
     2,    40, 2847, 639, 647;  3,   424, 2779, 646, 628;
     4,   808, 2874, 639, 658;  5,  1192, 2862, 693, 666;
     6,  1576, 2839, 632, 640;  7,  1960, 2822, 567, 561;
     8,  2344, 2861, 472, 469;  9,  2728, 2869, 380, 385;
    10,  3112, 2867, 319, 318; 11,  3496, 2871, 258, 253;
    12,  3880, 2867, 175, 178; 13,  4264, 2868, 100, 101;
    14,  4648, 2862,  42,  41; 15,  5032, 2868,  11,  23;
    16,  5416, 2865,  58,  58; 17,  5800, 2865, 127, 126;
    18,  6184, 2867, 194, 193; 19,  6568, 2866, 250, 252;
    20,  6952, 2868, 312, 307; 21,  7336, 2862, 358, 363;
    22,  7720, 2858, 424, 419; 23,  8104, 2857, 477, 478;
    24,  8488, 2851, 534, 533; 25,  8872, 2854, 586, 584;
    26,  9256, 2846, 631, 632; 27,  9640, 2844, 676, 671;
    28, 10024, 2838, 705, 707; 29, 10408, 2824, 746, 745;
    30, 10792, 2829, 779, 779; 31, 11176, 2861, 795, 778;
    32, 11560, 2846, 742, 760; 33, 11944, 2875, 755, 743;
    34, 12328, 2884, 732, 732; 35, 12712, 2860, 695, 691;
    36, 13096, 2868, 627, 624; 37, 13456, 2873, 551, 555;
    38, 13840, 2877, 497, 495; 39, 14224, 2883, 442, 439;
    40, 14616, 2885, 372, 372; 41, 15016, 2892, 301, 301;
    42, 15400, 2890, 239, 243; 43, 15784, 2892, 201, 198;
    44, 16168, 2895, 153, 151; 45, 16552, 2897,  95,  99;
    46, 16936, 2906,  52,  49; 47, 17320, 2903,  12,  14;
    48, 17704, 2908,   4,  13; 49, 18088, 2903,  52,  49;
    50, 18472, 2904, 101, 100; 51, 18856, 2903, 146, 147;
    52, 19240, 2899, 191, 191; 53, 19624, 2903, 236, 233;
    54, 20008, 2898, 271, 274; 55, 20392, 2900, 316, 310;
    56, 20776, 2894, 342, 346; 57, 21160, 2892, 384, 381;
    58, 21544, 2889, 419, 420; 59, 21928, 2885, 458, 455;
    60, 22312, 2887, 486, 486; 61, 22696, 2882, 514, 514;
    62, 23080, 2879, 540, 536; 63, 23464, 2874, 551, 554;
    64, 23848, 2872, 575, 573; 65, 24232, 2869, 596, 595;
    66, 24616, 2864, 614, 614; 67, 25000, 2866, 630, 628;
    68, 25384, 2858, 638, 640; 69, 25768, 2856, 647, 660;
    70,   120, 2850, 638, 625; 71,   504, 2792, 643, 584;
    72,   888, 2839, 670, 920;
];
D2 = [  % 中电压, phase≈87
     0, 25536, 2212, 444, 328;  1, 25920, 2206, 436, 465;
     2,   272, 2206, 443, 445;  3,   656, 2171, 447, 453;
     4,  1040, 2210, 492, 467;  5,  1424, 2206, 430, 440;
     6,  1808, 2183, 388, 385;  7,  2192, 2212, 341, 340;
     8,  2576, 2226, 298, 298;  9,  2960, 2228, 250, 247;
    10,  3344, 2229, 191, 191; 11,  3728, 2219, 135, 140;
    12,  4112, 2234, 100,  94; 13,  4496, 2227,  47,  51;
    14,  4880, 2236,  17,  20; 15,  5264, 2228,  20,  23;
    16,  5648, 2232,  56,  58; 17,  6032, 2229, 109, 105;
    18,  6416, 2228, 150, 153; 19,  6800, 2230, 198, 195;
    20,  7184, 2227, 236, 237; 21,  7568, 2228, 277, 274;
    22,  7952, 2223, 308, 311; 23,  8336, 2222, 351, 348;
    24,  8720, 2223, 385, 386; 25,  9104, 2217, 423, 422;
    26,  9488, 2222, 460, 458; 27,  9872, 2215, 490, 492;
    28, 10256, 2215, 517, 511; 29, 10640, 2202, 532, 526;
    30, 11024, 2188, 550, 581; 31, 11408, 2241, 671, 627;
    32, 11792, 2249, 605, 617; 33, 12176, 2246, 546, 547;
    34, 12560, 2239, 491, 486; 35, 12944, 2244, 436, 439;
    36, 13352, 2240, 398, 397; 37, 13736, 2242, 358, 355;
    38, 14120, 2247, 306, 308; 39, 14528, 2249, 263, 262;
    40, 14888, 2255, 222, 223; 41, 15272, 2254, 186, 185;
    42, 15656, 2260, 148, 146; 43, 16040, 2258, 106, 109;
    44, 16424, 2260,  79,  78; 45, 16808, 2263,  52,  52;
    46, 17192, 2263,  26,  26; 47, 17576, 2270,   6,   7;
    48, 17960, 2266,   7,  13; 49, 18320, 2268,  45,  41;
    50, 18704, 2266,  78,  78; 51, 19088, 2265, 111, 111;
    52, 19472, 2265, 143, 142; 53, 19856, 2263, 174, 174;
    54, 20240, 2266, 206, 204; 55, 20624, 2262, 230, 230;
    56, 21008, 2266, 253, 251; 57, 21392, 2262, 271, 274;
    58, 21776, 2259, 301, 296; 59, 22160, 2259, 319, 322;
    60, 22544, 2256, 349, 346; 61, 22928, 2258, 370, 368;
    62, 23312, 2254, 384, 385; 63, 23696, 2254, 399, 396;
    64, 24080, 2250, 406, 407; 65, 24464, 2248, 421, 419;
    66, 24848, 2247, 433, 434; 67, 25232, 2243, 449, 447;
    68, 25616, 2246, 456, 455; 69, 26000, 2242, 458, 482;
    70,   352, 2174, 468, 444; 71,   736, 2212, 458, 388;
    72,  1120, 2254, 470, 775;
];
D3 = [  % 低电压, phase≈94
     0, 25592, 907, 127,  98;  1, 25968, 908, 132, 132;
     2,   320, 819, 115, 126;  3,   704, 876, 138, 131;
     4,  1072, 896, 148, 147;  5,  1448, 924, 148, 144;
     6,  1832, 924, 127, 129;  7,  2216, 911, 112, 111;
     8,  2632, 935,  95,  92;  9,  2984, 916,  71,  73;
    10,  3400, 922,  58,  55; 11,  3776, 916,  39,  41;
    12,  4136, 920,  29,  26; 13,  4520, 911,  10,  12;
    14,  4936, 916,   5,   5; 15,  5296, 917,   7,   5;
    16,  5672, 915,   7,   7; 17,  6064, 920,  12,  13;
    18,  6440, 918,  26,  25; 19,  6824, 924,  41,  40;
    20,  7208, 916,  55,  55; 21,  7592, 918,  69,  69;
    22,  8008, 917,  85,  83; 23,  8360, 915,  94,  95;
    24,  8776, 920, 108, 106; 25,  9152, 918, 118, 119;
    26,  9536, 917, 132, 129; 27,  9920, 911, 138, 140;
    28, 10280, 913, 152, 150; 29, 10664, 911, 159, 159;
    30, 11048, 903, 166, 165; 31, 11432, 926, 172, 172;
    32, 11816, 930, 174, 173; 33, 12200, 932, 165, 159;
    34, 12584, 926, 135, 140; 35, 12968, 925, 127, 125;
    36, 13384, 925, 118, 116; 37, 13760, 924, 103, 104;
    38, 14144, 927,  90,  89; 39, 14528, 926,  74,  75;
    40, 14920, 932,  61,  57; 41, 15272, 926,  39,  44;
    42, 15656, 930,  39,  36; 43, 16072, 933,  31,  32;
    44, 16448, 930,  27,  23; 45, 16832, 936,  10,  13;
    46, 17216, 933,   7,   6; 47, 17600, 940,   4,   4;
    48, 17984, 934,   6,   3; 49, 18368, 934,   3,   9;
    50, 18752, 935,  24,  14; 51, 19144, 934,  13,  19;
    52, 19520, 938,  26,  23; 53, 19904, 935,  34,  34;
    54, 20288, 936,  44,  43; 55, 20680, 934,  52,  51;
    56, 21056, 934,  59,  60; 57, 21448, 935,  71,  69;
    58, 21824, 933,  77,  78; 59, 22216, 938,  86,  84;
    60, 22592, 935,  90,  91; 61, 22984, 937, 100,  96;
    62, 23360, 932, 100, 102; 63, 23752, 933, 111, 109;
    64, 24128, 933, 118, 117; 65, 24520, 931, 122, 122;
    66, 24896, 934, 127, 125; 67, 25288, 931, 127, 128;
    68, 25664, 934, 133, 131; 69,    24, 932, 135, 163;
    70,   400, 866, 139, 116; 71,   792, 906, 143,  47;
    72,  1168, 923, 149, 539;
];

datasets = {D1, D2, D3};
names = {'High V (~2860 ADC)', 'Mid V (~2240 ADC)', 'Low V (~920 ADC)'};

%% ====== 核心分析 ========================================================
fprintf('\n===== 谐振参数物理量计算 =====\n\n');

all_results = [];

for d = 1:3
    raw = datasets{d};
    idx_arr = raw(:,1);
    hrtim   = raw(:,2);
    v_adc   = raw(:,3);
    i_raw   = raw(:,4);
    i_filt  = raw(:,5);
    n = length(hrtim);

    % ---- HRTIM 展开 (处理计数器溢出) ----
    wrap_idx = find(diff(hrtim) < -20000) + 1;
    hrtim_uw = int64(hrtim);
    cum = int64(0);
    wi = 1;
    for i = 1:n
        if wi <= length(wrap_idx) && i == wrap_idx(wi)
            cum = cum + HW.PERIOD_TICKS;
            wi = wi + 1;
        end
        hrtim_uw(i) = hrtim_uw(i) + cum;
    end
    time_us = double(hrtim_uw - hrtim_uw(1)) / (HW.HRTIM_CLK / 1e6);

    % ---- 物理量转换 ----
    V_bus_V  = double(v_adc)  * HW.V_per_adc;   % 母线电压 (V)
    I_filt_A = double(i_filt) * HW.I_per_adc;    % 滤波电流 (A)
    I_raw_A  = double(i_raw)  * HW.I_per_adc;    % 原始电流 (A)

    % ---- ZC 检测: 深谷 (I < 15% 峰值) ----
    i_thresh = max(I_filt_A) * 0.15;
    zc_candidates = find(I_filt_A < i_thresh);
    zc_deep = [];
    if ~isempty(zc_candidates)
        cluster = zc_candidates(1);
        for k = 2:length(zc_candidates)
            if zc_candidates(k) - zc_candidates(k-1) > 3
                [~, imin] = min(I_filt_A(cluster(1):zc_candidates(k-1)));
                zc_deep = [zc_deep; cluster(1) + imin - 1];
                cluster = zc_candidates(k);
            end
        end
        [~, imin] = min(I_filt_A(cluster(1):zc_candidates(end)));
        zc_deep = [zc_deep; cluster(1) + imin - 1];
    end

    % ---- 峰值检测 ----
    pk_idx = [];
    pk_max = max(I_filt_A);
    for i = 4:(n-3)
        if I_filt_A(i) > I_filt_A(i-1) && I_filt_A(i) > I_filt_A(i-2) && ...
           I_filt_A(i) > I_filt_A(i+1) && I_filt_A(i) > I_filt_A(i+2) && ...
           I_filt_A(i) > pk_max * 0.3
            pk_idx = [pk_idx; i];
        end
    end
    if ~isempty(pk_idx)
        pk_dedup = pk_idx(1);
        for k = 2:length(pk_idx)
            if pk_idx(k) - pk_dedup(end) > 3
                pk_dedup = [pk_dedup; pk_idx(k)];
            end
        end
        pk_idx = pk_dedup;
    end

    % ----- f0: ZC 间距 ----  ----
    f0_Hz = NaN;
    if length(zc_deep) >= 2
        hp_us_all = diff(time_us(zc_deep));
        hp_median = median(hp_us_all);
        hp_good = hp_us_all(hp_us_all > 0.7*hp_median & hp_us_all < 1.3*hp_median);
        if ~isempty(hp_good)
            f0_Hz = 1e6 / (2 * median(hp_good));
        else
            f0_Hz = 1e6 / (2 * hp_median);
        end
    end

    % ----- Q: 隔峰衰减比 ----
    Q_val = NaN;
    if length(pk_idx) >= 3
        pk_vals_A = I_filt_A(pk_idx);
        q_list = [];
        for k = 1:(length(pk_vals_A)-2)
            ratio = pk_vals_A(k+2) / pk_vals_A(k);
            if ratio > 0.5 && ratio < 0.9995
                q_list = [q_list; -2*pi / log(ratio)];
            end
        end
        if ~isempty(q_list)
            Q_val = median(q_list);
        end
    end

    % ----- L: 开关开通段 di/dt -----
    L_uH_estimates = [];
    rise_details = {};

    for z = 1:length(zc_deep)
        zc = zc_deep(z);
        pk_after = pk_idx(pk_idx > zc + 2);
        if isempty(pk_after), break; end
        pk = pk_after(1);
        if pk <= zc + 3, continue; end

        seg_full = zc:pk;
        t_full = time_us(seg_full);
        i_full = I_filt_A(seg_full);

        best_len = 0;  best_L = NaN;  best_r2 = 0;
        best_seg = []; best_slope = NaN; best_vbus = NaN;

        for start = 1:3
            for len = (length(seg_full) - start - 3):-1:5
                sub = start:(start + len);
                t_sub = t_full(sub);
                i_sub = i_full(sub);
                p = polyfit(t_sub, i_sub, 1);
                slope = p(1);  % A/us
                if slope < 0.5, continue; end  % min 0.5 A/us
                i_fit = polyval(p, t_sub);
                ss_res = sum((i_sub - i_fit).^2);
                ss_tot = sum((i_sub - mean(i_sub)).^2);
                r2 = 1 - ss_res / ss_tot;
                if r2 > 0.995 && len > best_len
                    best_len = len;  best_slope = slope;
                    best_r2 = r2;    best_seg = sub + zc - 1;
                    best_vbus = mean(V_bus_V(seg_full(sub)));
                    % L = Vbus / (di/dt)  → Henry → μH
                    best_L = (best_vbus / (best_slope * 1e6)) * 1e6;
                end
            end
        end

        if ~isnan(best_L)
            L_uH_estimates = [L_uH_estimates; best_L];
            rise_details{end+1} = struct('zc', zc, 'pk', pk, ...
                'seg', best_seg, 'L_uH', best_L, 'slope_Aus', best_slope, ...
                'r2', best_r2, 'Vbus_V', best_vbus);
        end
    end

    L_best_uH = NaN;
    if ~isempty(L_uH_estimates)
        L_best_uH = mean(L_uH_estimates);
    end

    % ----- I_peak / I_rms -----
    I_peak_A = max(I_filt_A);
    I_rms_A  = I_peak_A / sqrt(2);

    % ----- V_bus -----
    V_bus_mean = mean(V_bus_V);

    % ----- C_calc: from f0 and L -----
    C_calc_uF = NaN;
    if ~isnan(f0_Hz) && ~isnan(L_best_uH)
        C_calc_F = 1 / ((2*pi*f0_Hz)^2 * L_best_uH * 1e-6);
        C_calc_uF = C_calc_F * 1e6;
    end

    % ----- ESR: from decay + tank impedance -----
    ESR_ohm = NaN;
    if ~isnan(L_best_uH)
        % Z0 = sqrt(L/C) for series resonant tank
        if ~isnan(C_calc_uF)
            Z0 = sqrt(L_best_uH * 1e-6 / (C_calc_uF * 1e-6));
            if ~isnan(Q_val)
                ESR_ohm = Z0 / Q_val;
            end
        end
    end

    % ----- 相位角: 电压-电流相位差 (从 ZC 推断) -----
    phase_deg = NaN;
    % phase ≈ 90° - atan(Q * (f_sw/f0 - f0/f_sw))  (近似)
    if ~isnan(f0_Hz) && ~isnan(Q_val)
        f_ratio = HW.F_SW / f0_Hz;
        phase_rad = atan(Q_val * (f_ratio - 1/f_ratio));
        phase_deg = rad2deg(phase_rad);
    end

    % ----- 有效功率 -----
    P_W = NaN;
    if ~isnan(phase_deg)
        P_W = V_bus_mean * I_rms_A * cosd(phase_deg) * 0.5;  % 半桥因子
    end

    % ---- 输出 ----
    fprintf('%s:\n', names{d});
    fprintf('  f0       = %.0f Hz\n', f0_Hz);
    fprintf('  Q        = %.1f\n', Q_val);
    fprintf('  L        = %.1f uH  (已知=%.0f uH)\n', L_best_uH, HW.L_known_uH);
    fprintf('  C_calc   = %.3f uF  (已知=%.2f uF)\n', C_calc_uF, HW.C_known_uF);
    fprintf('  I_peak   = %.2f A\n', I_peak_A);
    fprintf('  I_rms    = %.2f A\n', I_rms_A);
    fprintf('  V_bus    = %.1f V\n', V_bus_mean);
    fprintf('  ESR      = %.3f ohm\n', ESR_ohm);
    fprintf('  phase    = %.1f deg\n', phase_deg);
    fprintf('  P        = %.0f W\n', P_W);

    % ---- L 各段明细 ----
    fprintf('  Rise segments (%d):\n', length(rise_details));
    for si = 1:length(rise_details)
        rs = rise_details{si};
        dur_us = time_us(rs.seg(end)) - time_us(rs.seg(1));
        fprintf('    [%d:%d] Vbus=%.1fV di/dt=%.2fA/us L=%.1fuH R^2=%.4f dur=%.2fus\n', ...
            rs.seg(1), rs.seg(end), rs.Vbus_V, rs.slope_Aus, rs.L_uH, rs.r2, dur_us);
    end

    fprintf('\n');

    % ---- 累积结果 ----
    all_results = [all_results; struct(...
        'name', names{d}, ...
        'f0_Hz', f0_Hz, 'Q', Q_val, ...
        'L_uH', L_best_uH, 'C_uF', C_calc_uF, ...
        'I_peak_A', I_peak_A, 'I_rms_A', I_rms_A, ...
        'V_bus_V', V_bus_mean, 'ESR_ohm', ESR_ohm, ...
        'phase_deg', phase_deg, 'P_W', P_W)];
end

%% ====== 交叉验证 ======
fprintf('===== 交叉验证 =====\n');

L_vals = [all_results.L_uH]';
f0_vals = [all_results.f0_Hz]';
C_vals = [all_results.C_uF]';

fprintf('L 测量值: ');
fprintf('%.1f ', L_vals);
fprintf('uH\n');
fprintf('  Mean=%.1f  Std=%.1f  CV=%.1f%%\n', mean(L_vals), std(L_vals), 100*std(L_vals)/mean(L_vals));
if ~isnan(HW.L_known_uH)
    fprintf('  vs 已知 L=%.0f uH: 偏差=%.1f%%\n', HW.L_known_uH, 100*abs(mean(L_vals)-HW.L_known_uH)/HW.L_known_uH);
end

fprintf('\nf0 测量值: ');
fprintf('%.0f ', f0_vals);
fprintf('Hz\n');
fprintf('  Mean=%.0f  Std=%.0f  CV=%.1f%%\n', mean(f0_vals), std(f0_vals), 100*std(f0_vals)/mean(f0_vals));

fprintf('\nC 推算值: ');
fprintf('%.3f ', C_vals);
fprintf('uF\n');
if ~isnan(HW.C_known_uF)
    fprintf('  vs 已知 C=%.2f uF: 偏差=%.1f%%\n', HW.C_known_uF, 100*abs(mean(C_vals)-HW.C_known_uF)/HW.C_known_uF);
end

% f0 理论值 vs 实测
f0_theory = 1/(2*pi*sqrt(HW.L_known_uH*1e-6*HW.C_known_uF*1e-6));
fprintf('\nf0_theory(L,C) = %.0f Hz\n', f0_theory);
fprintf('f0_meas  mean  = %.0f Hz\n', mean(f0_vals));
fprintf('偏差 = %.1f%%\n', 100*abs(mean(f0_vals)-f0_theory)/f0_theory);

%% ====== 图表 ======
figure('Position', [100 100 1200 800]);

for d = 1:3
    raw = datasets{d};
    hrtim   = raw(:,2);
    v_adc   = raw(:,3);
    i_filt  = raw(:,5);
    n = length(hrtim);

    wrap_idx = find(diff(hrtim) < -20000) + 1;
    hrtim_uw = int64(hrtim);
    cum = int64(0);  wi = 1;
    for i = 1:n
        if wi <= length(wrap_idx) && i == wrap_idx(wi)
            cum = cum + HW.PERIOD_TICKS;
            wi = wi + 1;
        end
        hrtim_uw(i) = hrtim_uw(i) + cum;
    end
    time_us = double(hrtim_uw - hrtim_uw(1)) / (HW.HRTIM_CLK / 1e6);
    I_A = double(i_filt) * HW.I_per_adc;
    V_V = double(v_adc) * HW.V_per_adc;

    subplot(3,1,d);
    yyaxis left;
    plot(time_us, I_A, 'b-', 'LineWidth', 1.2); hold on;
    ylabel('Current (A)');
    yyaxis right;
    plot(time_us, V_V, 'r-', 'LineWidth', 0.8);
    ylabel('Vbus (V)');
    xlabel('Time (us)');
    title(sprintf('%s  |  f0=%.0fHz  Q=%.0f  L=%.1fuH', ...
        all_results(d).name, all_results(d).f0_Hz, ...
        all_results(d).Q, all_results(d).L_uH));
    grid on;
end

fprintf('\n===== DONE =====\n');
