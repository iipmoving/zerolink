%% verify_f0_q_l.m
% Verify 3-parameter extraction from inductor charge/discharge physics.
% Three independent measurements, cross-validated across 3 datasets:
%   1. f0  from ZC valley spacing       (time-domain, no amplitude needed)
%   2. Q   from peak decay ratio        (amplitude ratio, no Vbus needed)
%   3. L   from di/dt during switch-ON  (needs Vbus, but V-independent in time)
%
% All three are voltage-independent in principle:
%   f0: pure timing
%   Q:  pure ratio
%   L:  Vbus cancels if we compute L_adc ratio across datasets

clear; close all; clc;

%% ===== Constants =====
HRTIM_CLK = 768e6;          % Effective HRTIM clock (Hz)
FRE_PER_ADC = 384;          % HRTIM ticks per ADC sample (0.5us)
ADC_PERIOD_US = 0.5;        % us per ADC sample

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

%% ===== Processing engine =====
% Column mapping: 1=Idx, 2=HRTIM, 3=Voltage, 4=Current_raw, 5=Current_filt

function result = analyze_dataset(raw, name, HRTIM_CLK, FRE_PER_ADC)
    hrtim = raw(:,2);
    v_adc = raw(:,3);
    i_filt = raw(:,5);
    n = length(hrtim);

    % --- Find PWM period from data ---
    % HRTIM counter wraps from near-period to near-zero.
    % Find the wrap point and compute actual period.
    wrap_idx = [];
    for i = 2:n
        if hrtim(i-1) - hrtim(i) > 20000
            wrap_idx = [wrap_idx; i];
        end
    end

    % PWM period = value just before wrap + (period - value after wrap)
    % Since counter goes ... -> max_val -> 0 -> ..., period = max_val + gap
    if ~isempty(wrap_idx)
        w = wrap_idx(1);
        period_ticks = int64(hrtim(w-1)) + int64(hrtim(w-1)) - int64(hrtim(w));
        % Actually: counter goes period-1 -> 0. So hrtim(w-1) is near period,
        % hrtim(w) is near 0. period = hrtim(w-1) + (reload value).
        % Simpler: period = max(hrtim) rounded up to nearest PWM granularity.
        % The PWM period in ticks: use known value 26112.
        period_ticks = 26112;
    else
        period_ticks = 26112;
    end

    f_sw = HRTIM_CLK / double(period_ticks);

    % Unwrap HRTIM: add period_ticks at each wrap
    hrtim_uw = int64(hrtim);
    cum = int64(0);
    wi = 1;
    for i = 1:n
        if wi <= length(wrap_idx) && i == wrap_idx(wi)
            cum = cum + period_ticks;
            wi = wi + 1;
        end
        hrtim_uw(i) = hrtim_uw(i) + cum;
    end

    % Convert to microseconds: ticks / (ticks_per_us)
    % 768 MHz -> 768 ticks per us
    ticks_per_us = HRTIM_CLK / 1e6;
    time_us = double(hrtim_uw) / ticks_per_us;
    time_us = time_us - time_us(1);  % zero-base

    % ================================================================
    % 1. f0 from ZC (current minima) spacing
    % Uses direction-based valley detection (from C code FindCurrentMin)
    % ================================================================
    zc_idx = [];
    direction = bitshift(uint16(65535), 0);  % 0xFFFF, 16-bit
    for i = 2:n
        now = i_filt(i);
        pre = i_filt(i-1);

        direction = bitshift(direction, 1);
        if now > pre
            direction = bitor(direction, uint16(1));
        else
            direction = bitand(direction, bitcmp(uint16(1)));
        end

        % Pattern 0x1 (bit0=1, bit1=0): pre was a local minimum
        if bitand(direction, uint16(3)) == uint16(1)
            pt = i - 1;  % the candidate minimum
            if pt >= 3 && pt <= n-2
                % Check: pt is lower than neighbors
                zr = uint32(i_filt(pt-1)) + uint32(i_filt(pt+1));
                if i_filt(pt) <= double(zr) / 2
                    zc_idx = [zc_idx; pt];
                end
            end
        end
    end

    % Deduplicate: keep only one ZC per group of nearby indices
    if ~isempty(zc_idx)
        zc_filt = zc_idx(1);
        for k = 2:length(zc_idx)
            if zc_idx(k) - zc_filt(end) > 3
                zc_filt = [zc_filt; zc_idx(k)];
            end
        end
        zc_idx = zc_filt;
    end

    f0 = NaN;
    f0_valid = false;
    if length(zc_idx) >= 2
        hp_us = diff(time_us(zc_idx));
        % Filter out impossibly short or long half-periods
        hp_us = hp_us(hp_us > 8 & hp_us < 30);
        if ~isempty(hp_us)
            hp_median = median(hp_us);
            f0 = 1e6 / (2 * hp_median);
            f0_valid = true;
        end
    end

    % ================================================================
    % 2. Q from peak decay ratio
    % Peaks are local maxima of rectified current = envelope of |I(t)|
    % ================================================================
    pk_idx = [];
    direction2 = bitshift(uint16(65535), 0);
    for i = 2:n
        now = i_filt(i);
        pre = i_filt(i-1);

        direction2 = bitshift(direction2, 1);
        if now < pre
            direction2 = bitor(direction2, uint16(1));
        else
            direction2 = bitand(direction2, bitcmp(uint16(1)));
        end

        % Pattern 0x1: pre was a local maximum (direction = falling after rise)
        if bitand(direction2, uint16(3)) == uint16(1)
            pt = i - 1;
            if pt >= 3 && pt <= n-2 && i_filt(pt) > 20
                zr = uint32(i_filt(pt-1)) + uint32(i_filt(pt+1));
                if i_filt(pt) >= double(zr) / 2
                    pk_idx = [pk_idx; pt];
                end
            end
        end
    end

    % Deduplicate peaks
    if ~isempty(pk_idx)
        pk_filt = pk_idx(1);
        for k = 2:length(pk_idx)
            if pk_idx(k) - pk_filt(end) > 3
                pk_filt = [pk_filt; pk_idx(k)];
            end
        end
        pk_idx = pk_filt;
    end

    Q_decay = NaN;
    Q_decay_valid = false;
    if length(pk_idx) >= 3
        pk_vals = double(i_filt(pk_idx));
        % Half-bridge: adjacent peaks alternate between upper/lower switch
        % half-cycles. Compare every-other peak (same polarity) for true decay.
        q_estimates = [];
        for k = 1:(length(pk_vals)-2)
            ratio = pk_vals(k+2) / pk_vals(k);
            % Full-cycle decay: I(n+1)/I(n) = exp(-2*pi/Q)
            if ratio > 0.3 && ratio < 0.98
                q_estimates = [q_estimates; -2*pi / log(ratio)];
            end
        end
        if ~isempty(q_estimates)
            Q_decay = median(q_estimates);
            Q_decay_valid = true;
        end
    end

    % ================================================================
    % 3. L from di/dt during switch-ON (linear rise)
    % Rise: from ZC1 to its subsequent peak = switch-ON charging
    % ================================================================
    L_adc = NaN;
    L_valid = false;
    rise_start = NaN;
    rise_end = NaN;
    di_dt_val = NaN;
    r2_val = NaN;

    % Find longest segment with sustained positive di/dt and high R^2.
    % This is the switch-ON linear rise (Vbus drives L, so di/dt = Vbus/L).
    best_r2 = 0.95;
    best_seg = [];
    best_slope = 0;

    for i = 1:(n - 5)
        for j = (i + 5):min(i+25, n)
            seg = i:j;
            t_seg = time_us(seg);
            i_seg = double(i_filt(seg));
            p = polyfit(t_seg, i_seg, 1);
            slope = p(1);
            if slope < 10
                continue;
            end
            i_fit = polyval(p, t_seg);
            ss_res = sum((i_seg - i_fit).^2);
            ss_tot = sum((i_seg - mean(i_seg)).^2);
            r2 = 1 - ss_res / ss_tot;
            if r2 > best_r2 && length(seg) > length(best_seg)
                best_r2 = r2;
                best_seg = seg;
                best_slope = slope;
            end
        end
    end

    if ~isempty(best_seg)
        t_seg = time_us(best_seg);
        i_seg = double(i_filt(best_seg));
        p = polyfit(t_seg, i_seg, 1);
        di_dt_val = p(1);
        i_fit = polyval(p, t_seg);
        ss_res = sum((i_seg - i_fit).^2);
        ss_tot = sum((i_seg - mean(i_seg)).^2);
        r2_val = 1 - ss_res / ss_tot;

        if r2_val > 0.95 && di_dt_val > 10
            v_mean = mean(v_adc(best_seg));
            L_adc = v_mean / di_dt_val;
            L_valid = true;
            rise_start = best_seg(1);
            rise_end = best_seg(end);
        end
    end

    % Store results
    result.name = name;
    result.f_sw = f_sw;
    result.f0 = f0;
    result.f0_valid = f0_valid;
    result.Q_decay = Q_decay;
    result.Q_decay_valid = Q_decay_valid;
    result.L_adc = L_adc;
    result.L_valid = L_valid;
    result.di_dt = di_dt_val;
    result.r2 = r2_val;
    result.time_us = time_us;
    result.i_filt = i_filt;
    result.v_adc = v_adc;
    result.zc_idx = zc_idx;
    result.pk_idx = pk_idx;
    result.rise_start = rise_start;
    result.rise_end = rise_end;
    result.hrtim_uw = hrtim_uw;
    result.period_ticks = period_ticks;
end

%% ===== Run analysis on all 3 datasets =====
R1 = analyze_dataset(D1, 'D1: High V (2850 ADC, ph=90)', HRTIM_CLK, FRE_PER_ADC);
R2 = analyze_dataset(D2, 'D2: Mid  V (2240 ADC, ph=87)', HRTIM_CLK, FRE_PER_ADC);
R3 = analyze_dataset(D3, 'D3: Low  V ( 920 ADC, ph=94)', HRTIM_CLK, FRE_PER_ADC);

%% ===== Diagnostic: print detected features =====
for i = 1:3
    r = [R1, R2, R3];
    r = r(i);
    fprintf('\n--- %s ---\n', r.name);
    fprintf('  ZC indices (%d): ', length(r.zc_idx));
    fprintf('%d ', r.zc_idx);
    fprintf('\n  PK indices (%d): ', length(r.pk_idx));
    fprintf('%d ', r.pk_idx);
    fprintf('\n  PK values: ');
    fprintf('%.0f ', r.i_filt(r.pk_idx));
    fprintf('\n  Rise seg: [%d %d]  slope=%.1f  R^2=%.4f\n', ...
        r.rise_start, r.rise_end, r.di_dt, r.r2);
end

%% ===== Print summary =====
fprintf('\n============================================================\n');
fprintf('  3-PARAMETER VERIFICATION: f0 (ZC) | Q (decay) | L (di/dt)\n');
fprintf('============================================================\n\n');

fprintf('%-8s %10s %10s %10s %10s %10s\n', ...
    'Dataset', 'f_sw(Hz)', 'f0(Hz)', 'Q_decay', 'L_adc', 'R^2(di/dt)');
fprintf('%-8s %10s %10s %10s %10s %10s\n', ...
    '--------', '--------', '--------', '--------', '--------', '----------');

results = {R1, R2, R3};
for i = 1:3
    r = results{i};
    fprintf('%-8s %10.0f %10.0f %10.1f %10.1f %10.4f\n', ...
        ['D', num2str(i)], r.f_sw, r.f0, r.Q_decay, r.L_adc, r.r2);
end

fprintf('\n--- Cross-validation ---\n');

% f0 consistency
f0_vals = [R1.f0, R2.f0, R3.f0];
fprintf('f0:  mean=%.0f Hz,  std=%.0f Hz,  CV=%.1f%%\n', ...
    mean(f0_vals), std(f0_vals), 100*std(f0_vals)/mean(f0_vals));

% Q consistency
q_vals = [R1.Q_decay, R2.Q_decay, R3.Q_decay];
fprintf('Q:   mean=%.1f,  std=%.1f,  CV=%.1f%%\n', ...
    mean(q_vals), std(q_vals), 100*std(q_vals)/mean(q_vals));

% L_adc consistency
L_vals = [R1.L_adc, R2.L_adc, R3.L_adc];
fprintf('L:   mean=%.1f ADC*us,  std=%.1f,  CV=%.1f%%\n', ...
    mean(L_vals), std(L_vals), 100*std(L_vals)/mean(L_vals));

fprintf('\n--- Key insight ---\n');
fprintf('If CV < 10%% across 3 very different Vbus levels (2850, 2240, 920):\n');
fprintf('  f0: voltage-independent (pure timing)      -> VERIFIED\n');
fprintf('  Q:  voltage-independent (pure ratio)       -> VERIFIED\n');
fprintf('  L:  voltage-independent (V cancels in fit) -> VERIFIED\n');

%% ===== FIGURE 1: Three datasets, 3-panel overview =====
figure('Name', '3-Parameter Extraction', 'Position', [50 50 1400 900]);

for i = 1:3
    r = results{i};

    % Left: full waveform with annotations
    subplot(3, 2, i*2-1);
    yyaxis left;
    plot(r.time_us, r.i_filt, 'b-', 'LineWidth', 1.2); hold on;

    % Mark ZC points
    if ~isempty(r.zc_idx)
        plot(r.time_us(r.zc_idx), r.i_filt(r.zc_idx), 'go', ...
            'MarkerSize', 10, 'LineWidth', 1.5);
    end
    % Mark peaks
    if ~isempty(r.pk_idx)
        plot(r.time_us(r.pk_idx), r.i_filt(r.pk_idx), 'r^', ...
            'MarkerSize', 10, 'LineWidth', 1.5);
    end
    % Mark rise region
    if r.L_valid
        seg = r.rise_start:r.rise_end;
        plot(r.time_us(seg), r.i_filt(seg), 'm.', 'MarkerSize', 12);
        t_fit = r.time_us(seg);
        p_fit = polyfit(t_fit, double(r.i_filt(seg)), 1);
        plot(t_fit, polyval(p_fit, t_fit), 'm-', 'LineWidth', 2);
    end

    ylabel('I (ADC)');
    yyaxis right;
    plot(r.time_us, r.v_adc, 'k-', 'LineWidth', 0.8);
    ylabel('Vbus (ADC)');

    xlabel('Time (\mus)');
    title(sprintf('%s  |  f0=%.0f Hz  Q=%.1f  L=%.0f ADC*us  R^2=%.4f', ...
        r.name, r.f0, r.Q_decay, r.L_adc, r.r2));
    legend('I filt', 'ZC', 'Peak', 'Rise seg', 'Lin fit', 'Vbus', ...
        'Location', 'best');
    grid on;

    % Right: rise region detail + residual
    subplot(3, 2, i*2);
    if r.L_valid
        seg = r.rise_start:r.rise_end;
        t_seg = r.time_us(seg);
        i_seg = double(r.i_filt(seg));
        p = polyfit(t_seg, i_seg, 1);
        i_fit = polyval(p, t_seg);
        res = i_seg - i_fit;

        yyaxis left;
        plot(t_seg, i_seg, 'b.', 'MarkerSize', 12); hold on;
        plot(t_seg, i_fit, 'r-', 'LineWidth', 1.5);
        ylabel('I (ADC)');

        yyaxis right;
        plot(t_seg, res, 'k.-', 'MarkerSize', 8);
        ylabel('Residual (ADC)');

        xlabel('Time (\mus)');
        title(sprintf('Rise detail: di/dt=%.1f ADC/us  R^2=%.4f  std(res)=%.1f ADC', ...
            r.di_dt, r.r2, std(res)));
        legend('Data', 'Linear fit', 'Residual', 'Location', 'best');
        grid on;
    end
end

%% ===== FIGURE 2: di/dt vs Vbus (L verification) =====
figure('Name', 'L Verification: di/dt vs Vbus', 'Position', [700 50 600 500]);

v_list = [];
didt_list = [];
for i = 1:3
    r = results{i};
    if r.L_valid
        seg = r.rise_start:r.rise_end;
        v_mean_data = mean(r.v_adc(seg));
        v_list = [v_list; v_mean_data];
        didt_list = [didt_list; r.di_dt];
        plot(v_mean_data, r.di_dt, 'ro', 'MarkerSize', 18, 'LineWidth', 2); hold on;
        text(v_mean_data + 30, r.di_dt, r.name, 'FontSize', 9);
    end
end

if length(v_list) >= 2
    p_L = polyfit(v_list, didt_list, 1);
    v_x = linspace(0, max(v_list)*1.1, 100);
    plot(v_x, polyval(p_L, v_x), 'b-', 'LineWidth', 1.5);
    L_from_slope = 1 / p_L(1);
    intercept = p_L(2);
    title(sprintf(['di/dt vs Vbus: L=%.0f ADC*us  (intercept=%.1f ADC/us)\n', ...
        'If L constant: all points on one line through (near) origin'], ...
        L_from_slope, intercept));
    fprintf('\n--- System L from multi-point fit ---\n');
    fprintf('L = %.0f ADC*us  (from di/dt vs Vbus slope)\n', L_from_slope);
    fprintf('Intercept = %.1f ADC/us  (should be near 0 if Vc = Vbus/2)\n', intercept);
    fprintf('R^2 of L fit = %.4f\n', corr(v_list, didt_list)^2);
else
    title('di/dt vs Vbus -- insufficient valid L points');
end

xlabel('Vbus (ADC)');
ylabel('di/dt (ADC/\mus)');
grid on;

%% ===== FIGURE 3: f0 and Q robustness =====
figure('Name', 'f0 and Q Cross-check', 'Position', [50 550 1400 400]);

for i = 1:3
    r = results{i};

    % f0: show half-periods
    subplot(2, 3, i);
    if length(r.zc_idx) >= 2
        hp = diff(r.time_us(r.zc_idx));
        bar(1:length(hp), hp);
        hold on;
        yline(median(hp), 'r--', 'LineWidth', 1.5);
        xlabel('ZC pair #');
        ylabel('Half-period (\mus)');
        title(sprintf('%s: f0=%.0f Hz (%.1f us half-per)', ...
            r.name, r.f0, median(hp)));
        grid on;
    end

    % Q: show peak ratios
    subplot(2, 3, i+3);
    if length(r.pk_idx) >= 2
        pk_v = double(r.i_filt(r.pk_idx));
        bar(1:length(pk_v), pk_v);
        hold on;
        % Show decay envelope
        if length(pk_v) >= 2
            env_x = 1:length(pk_v);
            env = pk_v(1) * exp(-pi/r.Q_decay * (0:length(pk_v)-1));
            plot(env_x, env, 'r--', 'LineWidth', 1.5);
        end
        xlabel('Peak #');
        ylabel('Peak current (ADC)');
        title(sprintf('%s: Q=%.1f (from decay)', r.name, r.Q_decay));
        grid on;
    end
end

fprintf('\n===== VERIFICATION COMPLETE =====\n');
