%% eval_L_from_adc.m
% Evaluate L (inductance) from per-cycle raw ADC data
% Physics: L = V_bus * dt / dI (linear region during switch-on)
%          f0 = 1 / (2 * pi * sqrt(L * C))
%          Q from decay envelope

clear; close all; clc;

%% --- Raw data from data.md ---
% Columns: [Index, HRTIM, Voltage_ADC, Current_ADC, Filtered_Current]
raw = [
     0, 25304, 2854, 622, 466;
     1, 25688, 2856, 633, 670;
     2,    40, 2847, 639, 647;
     3,   424, 2779, 646, 628;
     4,   808, 2874, 639, 658;
     5,  1192, 2862, 693, 666;
     6,  1576, 2839, 632, 640;
     7,  1960, 2822, 567, 561;
     8,  2344, 2861, 472, 469;
     9,  2728, 2869, 380, 385;
    10,  3112, 2867, 319, 318;
    11,  3496, 2871, 258, 253;
    12,  3880, 2867, 175, 178;
    13,  4264, 2868, 100, 101;
    14,  4648, 2862,  42,  41;
    15,  5032, 2868,  11,  23;
    16,  5416, 2865,  58,  58;
    17,  5800, 2865, 127, 126;
    18,  6184, 2867, 194, 193;
    19,  6568, 2866, 250, 252;
    20,  6952, 2868, 312, 307;
    21,  7336, 2862, 358, 363;
    22,  7720, 2858, 424, 419;
    23,  8104, 2857, 477, 478;
    24,  8488, 2851, 534, 533;
    25,  8872, 2854, 586, 584;
    26,  9256, 2846, 631, 632;
    27,  9640, 2844, 676, 671;
    28, 10024, 2838, 705, 707;
    29, 10408, 2824, 746, 745;
    30, 10792, 2829, 779, 779;
    31, 11176, 2861, 795, 778;
    32, 11560, 2846, 742, 760;
    33, 11944, 2875, 755, 743;
    34, 12328, 2884, 732, 732;
    35, 12712, 2860, 695, 691;
    36, 13096, 2868, 627, 624;
    37, 13456, 2873, 551, 555;
    38, 13840, 2877, 497, 495;
    39, 14224, 2883, 442, 439;
    40, 14616, 2885, 372, 372;
    41, 15016, 2892, 301, 301;
    42, 15400, 2890, 239, 243;
    43, 15784, 2892, 201, 198;
    44, 16168, 2895, 153, 151;
    45, 16552, 2897,  95,  99;
    46, 16936, 2906,  52,  49;
    47, 17320, 2903,  12,  14;
    48, 17704, 2908,   4,  13;
    49, 18088, 2903,  52,  49;
    50, 18472, 2904, 101, 100;
    51, 18856, 2903, 146, 147;
    52, 19240, 2899, 191, 191;
    53, 19624, 2903, 236, 233;
    54, 20008, 2898, 271, 274;
    55, 20392, 2900, 316, 310;
    56, 20776, 2894, 342, 346;
    57, 21160, 2892, 384, 381;
    58, 21544, 2889, 419, 420;
    59, 21928, 2885, 458, 455;
    60, 22312, 2887, 486, 486;
    61, 22696, 2882, 514, 514;
    62, 23080, 2879, 540, 536;
    63, 23464, 2874, 551, 554;
    64, 23848, 2872, 575, 573;
    65, 24232, 2869, 596, 595;
    66, 24616, 2864, 614, 614;
    67, 25000, 2866, 630, 628;
    68, 25384, 2858, 638, 640;
    69, 25768, 2856, 647, 660;
    70,   120, 2850, 638, 625;
    71,   504, 2792, 643, 584;
    72,   888, 2839, 670, 920;
];

idx    = raw(:,1);
hrtim  = raw(:,2);
v_adc  = raw(:,3);
i_raw  = raw(:,4);
i_filt = raw(:,5);

n = length(idx);

%% --- Parameters ---
highOff      = 10130;        % upper switch turn-off HRTIM time
lowOn        = 15;           % deadtime
phaseUpHrtim = 684;          % zero-crossing HRTIM offset
HRTIM_CLK    = 384e6;        % 384 MHz
ADC_PER      = 384;          % 1 ADC sample = 384 HRTIM ticks = 1 us
period_ticks = 26112;        % PWM period in HRTIM ticks

%% --- HRTIM unwrapping ---
hrtim_unwrapped = int64(hrtim);
wraps = [];
cum_shift = int64(0);
for i = 2:n
    if hrtim(i-1) - hrtim(i) > 20000
        wraps = [wraps; i];
        cum_shift = cum_shift + period_ticks;
    end
    hrtim_unwrapped(i) = hrtim_unwrapped(i) + cum_shift;
end

time_us = double(hrtim_unwrapped) / ADC_PER;
time_us = time_us - time_us(1);

f_sw = HRTIM_CLK / period_ticks;
fprintf('=== HRTIM unwrapping ===\n');
fprintf('Wraps at indices: %s\n', mat2str(wraps));
fprintf('PWM period: %d ticks = %.1f us\n', period_ticks, period_ticks/HRTIM_CLK*1e6);
fprintf('f_sw = %.1f Hz\n', f_sw);

%% ====== FIGURE 1: Full waveform ======
figure('Name', 'Full Cycle Waveform', 'Position', [100 500 900 400]);

yyaxis left;
plot(time_us, i_raw, 'b.-', 'MarkerSize', 8, 'DisplayName', 'Raw Current (ADC)');
hold on;
plot(time_us, i_filt, 'r.-', 'MarkerSize', 8, 'DisplayName', 'Filtered Current (ADC)');
ylabel('Current (ADC)');

yyaxis right;
plot(time_us, v_adc, 'k.--', 'MarkerSize', 6, 'DisplayName', 'Vbus (ADC)');
ylabel('Voltage (ADC)');
xlabel('Time (\mus)');
title(sprintf('Half-Bridge One PWM Cycle (f_{sw}=%.0f Hz)', f_sw));
legend('Location', 'best');
grid on;

% Mark wraps
for w = 1:length(wraps)
    xline(time_us(wraps(w)), 'g--', sprintf('Wrap %d', w));
end
% Mark highOff point
xline(highOff/ADC_PER, 'm--', 'highOff');

%% ====== FIGURE 2: ON-period linear rise ======
% The switch-ON period is after each wrap (HRTIM near 0)
% First ON-period rise: idx 15->28 (current rising linearly under V_bus)
rise_start = 15;
rise_end   = 28;

figure('Name', 'ON-Period Linear Rise (di/dt)', 'Position', [100 100 900 400]);

subplot(2,2,[1 2]);
plot(time_us, i_filt, 'r.-', 'MarkerSize', 6, 'DisplayName', 'Filtered I');
hold on;
plot(time_us(rise_start:rise_end), i_filt(rise_start:rise_end), 'b.-', ...
    'MarkerSize', 12, 'LineWidth', 2, 'DisplayName', 'Linear Rise Region');
xlabel('Time (\mus)');
ylabel('I_{filt} (ADC)');
title('Current Waveform - Rise Region Highlighted');
legend('Location', 'best');
grid on;

% Linear fit
t_rise = time_us(rise_start:rise_end);
i_rise = i_filt(rise_start:rise_end);
p = polyfit(t_rise, i_rise, 1);
di_dt = p(1);
i_fit = polyval(p, t_rise);

R2 = 1 - sum((i_rise - i_fit).^2) / sum((i_rise - mean(i_rise)).^2);

subplot(2,2,3);
plot(t_rise, i_rise, 'b.', 'MarkerSize', 12);
hold on;
plot(t_rise, i_fit, 'r-', 'LineWidth', 2);
xlabel('Time (\mus)');
ylabel('I_{filt} (ADC)');
title(sprintf('Linear Fit: di/dt = %.1f ADC/\\mus, R^2 = %.4f', di_dt, R2));
legend('Data', 'Fit');
grid on;

subplot(2,2,4);
plot(t_rise, i_rise - i_fit, 'k.', 'MarkerSize', 12);
yline(0, 'r--');
xlabel('Time (\mus)');
ylabel('Residual (ADC)');
title(sprintf('Residuals (std = %.1f ADC)', std(i_rise - i_fit)));
grid on;

v_mean = mean(v_adc(rise_start:rise_end));
L_adc = v_mean / di_dt;

fprintf('\n=== ON-PERIOD LINEAR RISE (idx %d->%d) ===\n', rise_start, rise_end);
fprintf('di/dt = %.2f ADC/us\n', di_dt);
fprintf('R^2 = %.4f\n', R2);
fprintf('Delta t = %.2f us\n', t_rise(end) - t_rise(1));
fprintf('Delta I = %.0f ADC\n', i_rise(end) - i_rise(1));
fprintf('Mean Vbus = %.0f ADC\n', v_mean);
fprintf('L (ADC units) = %.2f\n', L_adc);

%% ====== FIGURE 3: Zero-crossing detection & f0 ======
% Find minima in filtered current (these are the "zero-crossings"
% since the ADC has a DC offset around 0-20 ADC)
% The resonant current crosses zero relative to its AC baseline

% Find local minima
min_vals = [];
min_times = [];
for i = 3:n-2
    if i_filt(i) < i_filt(i-1) && i_filt(i) < i_filt(i-2) && ...
       i_filt(i) <= i_filt(i+1) && i_filt(i) <= i_filt(i+2)
        min_vals = [min_vals; i_filt(i)];
        min_times = [min_times; time_us(i)];
    end
end

% Find local maxima
max_vals = [];
max_times = [];
max_idx = [];
for i = 3:n-2
    if i_filt(i) > i_filt(i-1) && i_filt(i) > i_filt(i-2) && ...
       i_filt(i) >= i_filt(i+1) && i_filt(i) >= i_filt(i+2)
        max_vals = [max_vals; i_filt(i)];
        max_times = [max_times; time_us(i)];
        max_idx = [max_idx; i];
    end
end

fprintf('\n=== RESONANT FREQUENCY (f0) FROM ZERO-CROSSINGS ===\n');
fprintf('Minima at: %s us\n', mat2str(round(min_times, 1)));
fprintf('Maxima at: %s us\n', mat2str(round(max_times, 1)));

% Resonant half-period = time between consecutive minima
if length(min_times) >= 2
    half_periods = diff(min_times);
    hp_mean = mean(half_periods);
    f0_zc = 1/(2 * hp_mean * 1e-6);
    fprintf('Half-periods: %s us\n', mat2str(round(half_periods, 1)));
    fprintf('Mean half-period: %.1f us\n', hp_mean);
    fprintf('f0 (from ZC): %.0f Hz\n', f0_zc);
end

figure('Name', 'Zero-Crossing Detection', 'Position', [550 100 500 350]);
plot(time_us, i_filt, 'b.-', 'MarkerSize', 6); hold on;
plot(min_times, min_vals, 'ro', 'MarkerSize', 10, 'LineWidth', 2, 'DisplayName', 'ZC (min)');
plot(max_times, max_vals, 'g^', 'MarkerSize', 10, 'LineWidth', 2, 'DisplayName', 'Peak (max)');

% Annotate half-periods
for i = 1:length(min_times)-1
    x = (min_times(i) + min_times(i+1))/2;
    y = 100;
    text(x, y, sprintf('%.1f us', half_periods(i)), ...
        'HorizontalAlignment', 'center', 'FontSize', 9, 'Color', 'r');
end

xlabel('Time (\mus)');
ylabel('I_{filt} (ADC)');
title(sprintf('Resonant Current - f_0 \\approx %.0f Hz', f0_zc));
legend('Location', 'best');
grid on;

%% ====== FIGURE 4: Q from envelope decay ======
% For a driven series resonant circuit at steady state, the envelope approach
% is approximate. Use the ln(A1/A2) method on successive half-cycles.
% Q = pi / ln(|I_peak(n)| / |I_peak(n+1)|) per half-cycle

if length(max_vals) >= 2
    q_estimates = [];
    for i = 1:length(max_vals)-1
        ratio = max_vals(i+1) / max_vals(i);
        if ratio > 0 && ratio < 1
            q_estimates = [q_estimates; -pi / log(ratio)];
        end
    end

    if ~isempty(q_estimates)
        q_mean = mean(q_estimates);
        fprintf('\n=== Q-FACTOR FROM ENVELOPE ===\n');
        fprintf('Q estimates (half-cycle): %s\n', mat2str(round(q_estimates, 1)));
        fprintf('Mean Q: %.1f\n', q_mean);
    end

    figure('Name', 'Q-Factor Analysis', 'Position', [1050 100 500 350]);
    semilogy(time_us, i_filt, 'b.-'); hold on;
    semilogy(max_times, max_vals, 'r^', 'MarkerSize', 10, 'LineWidth', 2);
    semilogy(min_times, min_vals, 'go', 'MarkerSize', 8);
    xlabel('Time (\mus)');
    ylabel('|I_{filt}| (ADC) - log scale');
    title(sprintf('Q \\approx %.1f (from envelope decay)', q_mean));
    legend('I_{filt}', 'Peaks', 'Valleys', 'Location', 'best');
    grid on;
end

%% ====== FIGURE 5: V-I phase relationship ======
figure('Name', 'V-I Phase Relationship', 'Position', [550 500 500 300]);
yyaxis left;
plot(time_us, i_filt, 'r-', 'LineWidth', 1.5, 'DisplayName', 'I filter');
ylabel('Current (ADC)');
yyaxis right;
plot(time_us, v_adc, 'k-', 'LineWidth', 1, 'DisplayName', 'Vbus');
ylabel('Voltage (ADC)');
xlabel('Time (\mus)');
title('Vbus and Resonant Current Phase Relationship');
legend('Location', 'best');
grid on;

%% ====== Summary ======
fprintf('\n============================================================\n');
fprintf('SUMMARY\n');
fprintf('============================================================\n');
fprintf('f_sw (PWM):          %.0f Hz\n', f_sw);
fprintf('di/dt linearity:     R^2 = %.4f\n', R2);
fprintf('Rise time:           %.1f us\n', t_rise(end)-t_rise(1));
fprintf('Delta I:             %.0f ADC\n', i_rise(end)-i_rise(1));
fprintf('Mean Vbus:           %.0f ADC\n', v_mean);
fprintf('L (ADC units):       %.1f V*us/A_eq\n', L_adc);
fprintf('f0 (zero-crossing):  %.0f Hz\n', f0_zc);
if exist('q_mean', 'var')
    fprintf('Q (envelope):        %.1f\n', q_mean);
end
fprintf('\nConclusion: di/dt is highly linear (R^2=%.4f) -> L can be \n', R2);
fprintf('measured per-cycle from the switch-on current slope.\n');
fprintf('Need V_scale and I_scale calibration for physical uH.\n');
