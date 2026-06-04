% compare_waveform — 铁锅 vs 钢锅 高功率电流波形对比
function compare_waveform()
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
cal.I_SCALE = 3.3/4096/(330/(10e3+330));

% 铁锅: 153424 f4, 30.3kHz, Vdc~300V, Ipk~18A
% 钢锅: 152037 f0, 30.9kHz, Vdc~297V, Ipk~28A
targets = {
    {'../../tools/ekf_tuner/capture_20260602_153424.csv', 4, 'Iron', [0.2, 0.3, 0.8]}
    {'../../tools/ekf_tuner/capture_20260602_152037.csv', 0, 'Steel', [0.2, 0.3, 0.8]}
};

figure('Position', [100 100 1400 500]);

for k = 1:2
    csv_path = targets{k}{1};
    frame_idx = targets{k}{2};
    label = targets{k}{3};

    fid = fopen(csv_path, 'r');
    fgetl(fid);
    frames = {}; fN = [];
    while ~feof(fid)
        line = fgetl(fid); if line == -1, break; end
        line = strtrim(line); if isempty(line), continue; end
        if startsWith(line, 'SIZE')
            parts = strsplit(line, ','); N = str2double(parts{2}); fN(end+1) = N;
            fr = zeros(N, 11);
            for i = 1:N
                dl = fgetl(fid); dl = strtrim(dl);
                if isempty(dl), i = i - 1; continue; end
                parts = strsplit(dl, ',');
                for j = 1:min(length(parts), 11)
                    v = str2double(parts{j}); if ~isnan(v), fr(i,j) = v; end
                end
            end
            frames{end+1} = fr;
        end
    end
    fclose(fid);

    frame = frames{frame_idx + 1};
    N = fN(frame_idx + 1);
    t_us = frame(:, 1);
    I_adc = frame(:, 2);
    I = I_adc * cal.I_SCALE;
    Vdc_adc = frame(:, 4);
    Vdc = Vdc_adc * cal.V_SCALE;
    CNT = frame(:, 5);
    CMP = frame(1, 6:9);

    % 裁剪边缘FMAC垃圾
    edge = 2;
    t_trim = t_us(edge+1:N-edge);
    I_trim = I(edge+1:N-edge);

    % FFT 看谐波
    dt = mean(diff(t_trim)) * 1e-6;
    if dt > 0
        nFFT = 512;
        I_fft = abs(fft(I_trim - mean(I_trim), nFFT));
        f_axis = (0:nFFT/2-1) / (nFFT * dt);
        I_fft = I_fft(1:nFFT/2);
    end

    % 理想正弦波 (同幅值同相位)
    Ipk = max(I_trim);
    f_fit = 1 / (max(t_trim) - min(t_trim)) * 1e6;  % 近似频率
    % 找过零点算频率
    zero_cross = [];
    for i = 2:length(I_trim)
        if I_trim(i-1) <= 0 && I_trim(i) > 0
            zero_cross(end+1) = t_trim(i);
        end
    end
    if length(zero_cross) >= 2
        T_est = (zero_cross(end) - zero_cross(1)) / (length(zero_cross) - 1);
        f_est = 1 / T_est;
    else
        f_est = f_fit;
    end
    I_sine = Ipk * sin(2 * pi * f_est * (t_trim - t_trim(1)) * 1e-6);

    % THD 计算 (前10次谐波)
    if dt > 0
        fundamental_idx = round(f_est * nFFT * dt) + 1;
        if fundamental_idx < 2, fundamental_idx = 2; end
        harm_power = 0;
        for h = 2:10
            hidx = round(h * f_est * nFFT * dt) + 1;
            if hidx <= length(I_fft)
                harm_power = harm_power + I_fft(hidx)^2;
            end
        end
        if I_fft(fundamental_idx) > 0
            THD = sqrt(harm_power) / I_fft(fundamental_idx) * 100;
        else
            THD = NaN;
        end
    end

    subplot(1, 2, k);
    plot(t_trim, I_trim, 'b-', 'LineWidth', 1.5); hold on;
    plot(t_trim, I_sine, 'r--', 'LineWidth', 1.0);
    xlabel('t (us)'); ylabel('I (A)');
    I_rms = rms(I_trim);
    I_rms_sine = Ipk / sqrt(2);
    fprintf('%s: f=%.1fkHz Ipk=%.1fA I_RMS=%.2fA I_RMS_sine=%.2fA ratio=%.2f Vdc=%.0fV THD=%.1f%%\n', ...
        label, f_est, Ipk, I_rms, I_rms_sine, I_rms/I_rms_sine, mean(Vdc), THD);
    title(sprintf('%s: f=%.1fkHz Ipk=%.1fA Vdc=%.0fV THD=%.1f%%', ...
        label, f_est, Ipk, mean(Vdc), THD));
    legend('Measured', 'Ideal sine', 'Location', 'best');
    grid on;
end

saveas(gcf, 'waveform_iron_vs_steel.png');
fprintf('Saved: waveform_iron_vs_steel.png\n');
end
