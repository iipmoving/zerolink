%% verify_L_consistency.m
% Focus: compute equivalent L from 3 datasets via switch-ON di/dt.
% The switch-ON rise starts from a TRUE zero-crossing (deep current valley)
% and follows Vbus = L * di/dt with excellent linearity.
%
% Key corrections from prior runs:
%   1. ZC = deep valley (I < 100 ADC), not any local minimum
%   2. Switch-ON rise = ZC -> next peak (the linear charging segment)
%   3. Q = every-other peak ratio (same switch polarity, full cycle decay)
%
% Constants: HRTIM_CLK = 768 MHz, 0.5 us per ADC sample

clear; close all; clc;

HRTIM_CLK = 768e6;
PERIOD_TICKS = 26112;
F_SW = HRTIM_CLK / PERIOD_TICKS;

%% ===== Dataset 1: High voltage, phase=90 =====
D1 = [ ...
     0, 25304, 2854, 622, 466; 1, 25688, 2856, 633, 670;
     2,    40, 2847, 639, 647; 3,   424, 2779, 646, 628;
     4,   808, 2874, 639, 658; 5,  1192, 2862, 693, 666;
     6,  1576, 2839, 632, 640; 7,  1960, 2822, 567, 561;
     8,  2344, 2861, 472, 469; 9,  2728, 2869, 380, 385;
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

%% ===== Dataset 2: Medium voltage, phase=87 =====
D2 = [ ...
     0, 25536, 2212, 444, 328; 1, 25920, 2206, 436, 465;
     2,   272, 2206, 443, 445; 3,   656, 2171, 447, 453;
     4,  1040, 2210, 492, 467; 5,  1424, 2206, 430, 440;
     6,  1808, 2183, 388, 385; 7,  2192, 2212, 341, 340;
     8,  2576, 2226, 298, 298; 9,  2960, 2228, 250, 247;
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

%% ===== Dataset 3: Low voltage, phase=94 =====
D3 = [ ...
     0, 25592, 907, 127,  98; 1, 25968, 908, 132, 132;
     2,   320, 819, 115, 126; 3,   704, 876, 138, 131;
     4,  1072, 896, 148, 147; 5,  1448, 924, 148, 144;
     6,  1832, 924, 127, 129; 7,  2216, 911, 112, 111;
     8,  2632, 935,  95,  92; 9,  2984, 916,  71,  73;
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

%% ===== Core analysis =====
datasets = {D1, D2, D3};
names = {'High V (2850 ADC)', 'Mid V (2240 ADC)', 'Low V (920 ADC)'};

fprintf('\n============================================================\n');
fprintf('  L CONSISTENCY CHECK: Vbus / (di/dt) across 3 Vbus levels\n');
fprintf('============================================================\n\n');

all_L = [];
all_didt = [];
all_vbus = [];
all_f0 = [];
all_L_means = [];  % one mean L per dataset

for d = 1:3
    raw = datasets{d};
    hrtim = raw(:,2);
    v_adc = raw(:,3);
    i_filt = raw(:,5);
    n = length(hrtim);

    % --- Unwrap HRTIM ---
    wrap_idx = find(diff(hrtim) < -20000) + 1;
    hrtim_uw = int64(hrtim);
    cum = int64(0);
    wi = 1;
    for i = 1:n
        if wi <= length(wrap_idx) && i == wrap_idx(wi)
            cum = cum + PERIOD_TICKS;
            wi = wi + 1;
        end
        hrtim_uw(i) = hrtim_uw(i) + cum;
    end
    time_us = double(hrtim_uw - hrtim_uw(1)) / (HRTIM_CLK / 1e6);

    % --- Find true ZCs: deep valleys (I < threshold) ---
    i_thresh = max(i_filt) * 0.15;  % 15% of peak = near zero
    zc_candidates = find(i_filt < i_thresh);
    % Group into clusters
    zc_deep = [];
    if ~isempty(zc_candidates)
        cluster = zc_candidates(1);
        for k = 2:length(zc_candidates)
            if zc_candidates(k) - zc_candidates(k-1) > 3
                % Find the minimum in this cluster
                [~, imin] = min(i_filt(cluster(1):zc_candidates(k-1)));
                zc_deep = [zc_deep; cluster(1) + imin - 1];
                cluster = zc_candidates(k);
            end
        end
        [~, imin] = min(i_filt(cluster(1):zc_candidates(end)));
        zc_deep = [zc_deep; cluster(1) + imin - 1];
    end

    % --- Find peaks: local maxima after rectification ---
    pk_idx = [];
    for i = 4:(n-3)
        if i_filt(i) > i_filt(i-1) && i_filt(i) > i_filt(i-2) && ...
           i_filt(i) > i_filt(i+1) && i_filt(i) > i_filt(i+2) && ...
           i_filt(i) > max(i_filt) * 0.3
            pk_idx = [pk_idx; i];
        end
    end
    % Deduplicate
    if ~isempty(pk_idx)
        pk_dedup = pk_idx(1);
        for k = 2:length(pk_idx)
            if pk_idx(k) - pk_dedup(end) > 3
                pk_dedup = [pk_dedup; pk_idx(k)];
            end
        end
        pk_idx = pk_dedup;
    end

    % --- f0 from deep ZC spacing (use first 2 ZCs only for robustness) ---
    f0 = NaN;
    if length(zc_deep) >= 2
        % Use median of all half-periods, but filter out outliers
        hp_us_all = diff(time_us(zc_deep));
        hp_median = median(hp_us_all);
        % Keep half-periods within 30% of median
        hp_good = hp_us_all(hp_us_all > 0.7*hp_median & hp_us_all < 1.3*hp_median);
        if ~isempty(hp_good)
            f0 = 1e6 / (2 * median(hp_good));
        else
            f0 = 1e6 / (2 * hp_median);
        end
    end

    % --- Q from every-other peak ratio ---
    % For high-Q systems, decay per cycle is small (ratio near 1).
    % Compare every-other peak (same switch polarity, full cycle apart).
    Q = NaN;
    if length(pk_idx) >= 3
        pk_vals = double(i_filt(pk_idx));
        q_list = [];
        for k = 1:(length(pk_vals)-2)
            ratio = pk_vals(k+2) / pk_vals(k);
            % Accept ratios 0.5~0.9995 (Q from ~4 to ~12000)
            if ratio > 0.5 && ratio < 0.9995
                q_list = [q_list; -2*pi / log(ratio)];
            end
        end
        if ~isempty(q_list)
            Q = median(q_list);
        end
    end

    % --- L from switch-ON rise: deep ZC -> next peak ---
    % The linear charge region is ZC -> peak (or until R^2 degrades)
    L_estimates = [];
    rise_segments = {};

    for z = 1:length(zc_deep)
        zc = zc_deep(z);
        % Find the next peak after this ZC
        pk_after = pk_idx(pk_idx > zc + 2);
        if isempty(pk_after)
            break;
        end
        pk = pk_after(1);
        if pk <= zc + 3
            continue;
        end

        % Full ZC->peak segment
        seg_full = zc:pk;
        t_full = time_us(seg_full);
        i_full = double(i_filt(seg_full));

        % Find the longest subsegment with R^2 > 0.995 (near-perfect linear)
        best_len = 0;
        best_L = NaN;
        best_slope = NaN;
        best_r2 = 0;
        best_seg = [];

        for start = 1:3  % try skipping first 0-2 points (possible ZC noise)
            for len = (length(seg_full) - start - 3):-1:5  % longest first
                sub = start:(start + len);
                t_sub = t_full(sub);
                i_sub = i_full(sub);
                p = polyfit(t_sub, i_sub, 1);
                slope = p(1);
                if slope < 5
                    continue;
                end
                i_fit = polyval(p, t_sub);
                ss_res = sum((i_sub - i_fit).^2);
                ss_tot = sum((i_sub - mean(i_sub)).^2);
                r2 = 1 - ss_res / ss_tot;
                if r2 > 0.995 && len > best_len
                    best_len = len;
                    best_slope = slope;
                    best_r2 = r2;
                    best_seg = sub + zc - 1;  % actual indices
                    v_mean = mean(v_adc(seg_full(sub)));
                    best_L = v_mean / slope;
                end
            end
        end

        if ~isnan(best_L)
            L_estimates = [L_estimates; best_L];
            rise_segments{end+1} = struct('zc', zc, 'pk', pk, ...
                'seg', best_seg, 'L', best_L, 'slope', best_slope, ...
                'r2', best_r2, 'v_mean', v_mean);
        end
    end

    % --- Print results ---
    fprintf('%s:\n', names{d});
    fprintf('  f0 = %.0f Hz  (from deep ZCs at', f0);
    for zi = 1:length(zc_deep)
        fprintf(' I[%d]=%.0f', zc_deep(zi), i_filt(zc_deep(zi)));
    end
    fprintf(')\n');
    fprintf('  Peaks (%d): ', length(pk_idx));
    for pi = 1:length(pk_idx)
        fprintf('I[%d]=%.0f ', pk_idx(pi), i_filt(pk_idx(pi)));
    end
    fprintf('\n');
    fprintf('  Q  = %.1f  (every-other peak decay)\n', Q);

    if ~isnan(f0)
        all_f0 = [all_f0; f0];
    end

    % Collect all valid L estimates from this dataset
    L_this_dataset = [];
    fprintf('  Rise segments found: %d\n', length(rise_segments));
    for si = 1:length(rise_segments)
        rs = rise_segments{si};
        fprintf('    [%d:%d] Vbus=%.0f di/dt=%.1f L=%.1f R^2=%.4f dur=%.1fus\n', ...
            rs.seg(1), rs.seg(end), rs.v_mean, rs.slope, rs.L, rs.r2, ...
            time_us(rs.seg(end)) - time_us(rs.seg(1)));
        L_this_dataset = [L_this_dataset; rs.L];
        all_L = [all_L; rs.L];
        all_didt = [all_didt; rs.slope];
        all_vbus = [all_vbus; rs.v_mean];
    end

    L_best = NaN;
    if ~isempty(L_this_dataset)
        L_best = mean(L_this_dataset);
        all_L_means = [all_L_means; L_best];
        fprintf('  L mean = %.1f ADC*us  (individual: ', L_best);
        fprintf('%.0f ', L_this_dataset);
        fprintf(')\n');
    else
        fprintf('  L = NO VALID RISE FOUND\n');
    end

    fprintf('\n');
end

%% ===== Cross-validation summary =====
fprintf('--- Cross-validation ---\n');
fprintf('L per dataset (mean of all rise segments):\n');
fprintf('  D1=%.1f  D2=%.1f  D3=%.1f ADC*us\n', all_L_means(1), all_L_means(2), all_L_means(3));
fprintf('  Grand mean = %.1f  std = %.1f  CV = %.1f%%\n', ...
    mean(all_L_means), std(all_L_means), 100*std(all_L_means)/mean(all_L_means));

fprintf('\ndi/dt vs Vbus fit:\n');
if length(all_vbus) >= 2
    p = polyfit(all_vbus, all_didt, 1);
    L_sys = 1 / p(1);
    intercept = p(2);
    r2_vdidt = corr(all_vbus(:), all_didt(:))^2;
    fprintf('  L_system = %.1f ADC*us  (from 1/slope)\n', L_sys);
    fprintf('  intercept = %.1f ADC/us\n', intercept);
    fprintf('  R^2 = %.4f\n', r2_vdidt);
end

fprintf('\nf0: mean=%.0f Hz  std=%.0f Hz  CV=%.1f%%\n', ...
    mean(all_f0), std(all_f0), 100*std(all_f0)/mean(all_f0));

fprintf('\n===== DONE =====\n');
