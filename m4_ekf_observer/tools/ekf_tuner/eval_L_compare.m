%% eval_L_compare.m
% Compare 3 datasets at different voltages / phase angles
% Verify: L should be consistent across datasets if same coil
%         di/dt should scale with Vbus (di/dt = Vbus / L)
%         f0 should be stable (same resonant tank)
%
% Columns: [Index, HRTIM, Voltage_ADC, Current_ADC, Filtered_Current]

clear; close all; clc;

%% ===== Dataset 1: Normal voltage, phase=90 =====
fprintf('=== Dataset 1 (normal, phase=90) ===\n');

% para0=10130, phase=90, lowOn=15, phaseUpHrtim=684
raw1 = [
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
fprintf('=== Dataset 2 (medium, phase=87) ===\n');

% para0=10069, phase=87, lowOn=14, phaseUpHrtim=7660
raw2 = [
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
fprintf('=== Dataset 3 (low, phase=94) ===\n');

% para0=10059, phase=94, lowOn=15, phaseUpHrtim=791
raw3 = [
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

%% ===== Analysis function =====
% Data columns: [Index, HRTIM, Voltage, Current_Raw, Current_Filtered]
% Returns: [Vbus_mean, di_dt, R2, L_adc, f0_zc, f_sw, Q_mean, rise_start, rise_end]

results = zeros(3, 9);
labels = {'Normal (ph=90,Vb~2850)', 'Medium (ph=87,Vb~2240)', 'Low (ph=94,Vb~920)'};
period_ticks = 26112;
HRTIM_CLK = 384e6;
ADC_PER = 384;

%% ===== FIGURE 1: Three waveforms stacked =====
figure('Name', 'Waveform Comparison', 'Position', [50 50 1400 900]);

for ds = 1:3
    switch ds
        case 1, raw = raw1;
        case 2, raw = raw2;
        case 3, raw = raw3;
    end

    hrtim = raw(:,2);
    v_adc = raw(:,3);
    i_filt = raw(:,5);
    n = length(hrtim);

    % Unwrap HRTIM
    hrtim_uw = int64(hrtim);
    cum_shift = int64(0);
    wraps = [];
    for i = 2:n
        if hrtim(i-1) - hrtim(i) > 20000
            wraps = [wraps; i];
            cum_shift = cum_shift + period_ticks;
        end
        hrtim_uw(i) = hrtim_uw(i) + cum_shift;
    end
    time_us = double(hrtim_uw) / ADC_PER;
    time_us = time_us - time_us(1);
    f_sw = HRTIM_CLK / period_ticks;

    % Find rise segment automatically: current minimum after first wrap → nearby peak
    % The ON period is from HRTIM wrap to highOff
    % We look for the long linear rise (upper switch conducts, V_bus drives L)
    % Strategy: find wrap, then find min current near wrap, then longest rising segment

    % Find the first linear rise: from local minimum to next peak
    % The filtered current has a periodic pattern with DC offset
    % Rise region: where di/dt is consistently positive and large

    % Manual identification based on waveform:
    % The switch-on linear rise is the long segment (12-14 us) with high positive di/dt
    % vs the resonant rise which is shorter (4-8 us)

    % Auto-detect: find segments with sustained di/dt > threshold
    di = diff(i_filt);
    dt = diff(time_us);
    didt = di ./ dt;

    % Find long positive segments
    min_rise_len = 6;
    best_seg = [];
    best_r2 = 0;
    i_start = 0;

    for i = 1:(n - min_rise_len)
        for j = (i + min_rise_len):min(i+30, n)
            seg = i:j;
            t_seg = time_us(seg);
            i_seg = i_filt(seg);

            if length(t_seg) >= min_rise_len
                p = polyfit(t_seg, i_seg, 1);
                y_fit = polyval(p, t_seg);
                r2 = 1 - sum((i_seg - y_fit).^2) / sum((i_seg - mean(i_seg)).^2);
                slope = p(1);

                if r2 > best_r2 && slope > 10 && r2 > 0.95
                    best_r2 = r2;
                    best_seg = seg;
                    i_start = i;
                end
            end
        end
    end

    if isempty(best_seg)
        % Fallback to manual ranges
        switch ds
            case 1
                best_seg = 15:28;
            case 2
                best_seg = 15:28;
            otherwise
                best_seg = 17:31;
        end
    end

    % Fit linear region
    t_rise = time_us(best_seg);
    i_rise = i_filt(best_seg);
    p_rise = polyfit(t_rise, i_rise, 1);
    di_dt = p_rise(1);
    i_fit_rise = polyval(p_rise, t_rise);
    r2 = 1 - sum((i_rise - i_fit_rise).^2) / sum((i_rise - mean(i_rise)).^2);
    v_mean = mean(v_adc(best_seg));
    L_adc = v_mean / di_dt;

    % Find zero-crossings (local minima of filtered current)
    min_times = [];
    for i = 4:(n-3)
        win = i_filt(i-3:i+3);
        if i_filt(i) == min(win) && all(win(1:3) >= win(4)) && all(win(5:7) >= win(4))
            min_times = [min_times; time_us(i)];
        end
    end

    % f0 from half-periods
    f0_zc = NaN;
    if length(min_times) >= 2
        hp = diff(min_times);
        f0_zc = 1/(2 * median(hp) * 1e-6);
    end

    % Q from peak ratios
    % Find peaks
    max_vals = [];
    for i = 4:(n-3)
        win = i_filt(i-3:i+3);
        if i_filt(i) == max(win) && all(win(1:3) <= win(4)) && all(win(5:7) <= win(4))
            max_vals = [max_vals; i_filt(i)];
        end
    end

    q_mean = NaN;
    if length(max_vals) >= 2
        q_all = [];
        for k = 1:(length(max_vals)-1)
            if max_vals(k+1) < max_vals(k) && max_vals(k+1) > 0
                q_all = [q_all; -pi / log(max_vals(k+1)/max_vals(k))];
            end
        end
        if ~isempty(q_all)
            q_mean = mean(q_all);
        end
    end

    % Store results
    results(ds, :) = [v_mean, di_dt, r2, L_adc, f0_zc, f_sw, q_mean, ...
                       best_seg(1), best_seg(end)];

    % Print
    fprintf('\n%s:\n', labels{ds});
    fprintf('  Vbus = %.0f ADC,  di/dt = %.1f ADC/us,  R^2 = %.4f\n', ...
        v_mean, di_dt, r2);
    fprintf('  L (ADC) = %.1f,  f0 = %.0f Hz,  f_sw = %.0f Hz\n', ...
        L_adc, f0_zc, f_sw);
    if ~isnan(q_mean)
        fprintf('  Q = %.1f\n', q_mean);
    end

    % Plot
    subplot(3,2, ds*2-1);
    yyaxis left;
    plot(time_us, i_filt, 'b.-', 'MarkerSize', 6); hold on;
    plot(t_rise, i_rise, 'r.', 'MarkerSize', 8);
    plot(t_rise, i_fit_rise, 'r-', 'LineWidth', 1.5);
    ylabel('I (ADC)');
    yyaxis right;
    plot(time_us, v_adc, 'k.--', 'MarkerSize', 4);
    ylabel('V (ADC)');
    xlabel('Time (\mus)');
    title(sprintf('%s', labels{ds}));
    legend('I filt', 'Rise region', 'Linear fit', 'Vbus', 'Location', 'best');
    grid on;

    subplot(3,2, ds*2);
    plot(t_rise, i_rise, 'b.', 'MarkerSize', 10); hold on;
    plot(t_rise, i_fit_rise, 'r-', 'LineWidth', 1.5);
    res = i_rise - i_fit_rise;
    text(mean(t_rise), max(i_rise), ...
        sprintf('di/dt=%.1f\nR^2=%.4f\nL_{ADC}=%.1f', di_dt, r2, L_adc), ...
        'FontSize', 9, 'BackgroundColor', 'w');
    xlabel('Time (\mus)');
    ylabel('I filt (ADC)');
    title(sprintf('Linear Fit Detail (std res = %.1f ADC)', std(res)));
    grid on;
end

%% ===== FIGURE 2: di/dt vs Vbus (should be linear if L constant) =====
figure('Name', 'di/dt vs Vbus', 'Position', [700 50 550 400]);
plot(results(:,1), results(:,2), 'ro', 'MarkerSize', 15, 'LineWidth', 2); hold on;

% Linear fit through origin: di/dt = Vbus / L
% Fit: slope = 1/L
X = results(:,1);
Y = results(:,2);
slope_fit = X \ Y;  % least squares through origin
L_system = 1 / slope_fit;

x_fit = linspace(0, max(X)*1.1, 100);
plot(x_fit, slope_fit * x_fit, 'b-', 'LineWidth', 1.5);
text(mean(X), mean(Y)*1.5, ...
    sprintf('di/dt = V / %.0f  (L_{sys}=%.0f ADC)\nR across 3 datasets', ...
        1/slope_fit, 1/slope_fit), ...
    'FontSize', 10);

xlabel('Vbus (ADC)');
ylabel('di/dt (ADC/\mus)');
title('di/dt vs Vbus -- L Verification');
grid on;

for ds = 1:3
    text(results(ds,1)+30, results(ds,2), labels{ds}, 'FontSize', 9);
end

%% ===== Summary table =====
fprintf('\n============================================================\n');
fprintf('COMPARISON SUMMARY\n');
fprintf('============================================================\n');
fprintf('%-30s %8s %8s %8s\n', 'Dataset', 'Vbus', 'di/dt', 'R^2');
fprintf('%-30s %8s %8s %8s\n', '--------', '----', '-----', '---');
for ds = 1:3
    fprintf('%-30s %8.0f %8.1f %8.4f\n', labels{ds}, ...
        results(ds,1), results(ds,2), results(ds,3));
end

fprintf('\n%-30s %8s %8s %8s %8s\n', 'Dataset', 'L_ADC', 'f0(Hz)', 'f_sw(Hz)', 'Q');
fprintf('%-30s %8s %8s %8s %8s\n', '--------', '-----', '------', '-------', '--');
for ds = 1:3
    fprintf('%-30s %8.1f %8.0f %8.0f %8.1f\n', labels{ds}, ...
        results(ds,4), results(ds,5), results(ds,6), results(ds,7));
end

fprintf('\nKEY METRIC: L_adc consistency (same coil should have same L)\n');
L_all = results(:,4);
fprintf('  L values: %.1f, %.1f, %.1f\n', L_all);
fprintf('  Mean: %.1f, Std: %.1f, CV: %.2f%%\n', mean(L_all), std(L_all), ...
    100*std(L_all)/mean(L_all));

fprintf('\nIf L is consistent across datasets (CV < 5%%): di/dt method validated.\n');
fprintf('Then f0 = 1/(2*pi*sqrt(L*C)) gives resonant capacitor C.\n');
fprintf('Alternatively: known C -> f0 from L, independent check on ZC-based f0.\n');
