% debug_valley — 打印波形片段
csv_path = '../../tools/ekf_tuner/captures/capture_20260602_104852.csv';
cal.I_SCALE = 3.3/4096/(330/(10e3+330));

fid = fopen(csv_path, 'r');
fgetl(fid);  % skip header
frames_data = {}; frames_N = [];

while ~feof(fid)
    line = fgetl(fid);
    if line == -1, break; end
    line = strtrim(line);
    if isempty(line), continue; end
    if startsWith(line, 'SIZE')
        parts = strsplit(line, ',');
        N = str2double(parts{2});
        frames_N(end+1) = N;
        frame_rows = zeros(N, 9);
        for i = 1:N
            dline = fgetl(fid);
            vals = sscanf(dline, '%f,%f,%f,%f,%f,%f,%f,%f,%f');
            if length(vals) >= 9
                frame_rows(i, :) = vals(1:9)';
            end
        end
        frames_data{end+1} = frame_rows;
    end
end
fclose(fid);

% Frame 2 (stable)
frame = frames_data{3};
N = frames_N(3);
I_adc = frame(:,2);
CNT = frame(:,5);
I = I_adc * cal.I_SCALE;

fprintf('Frame 2: N=%d, Imin=%.3fA Imax=%.3fA\n', N, min(I), max(I));

% Print ALL points to see the waveform
fprintf('\nIdx  I(A)     CNT    diff\n');
for i = 1:N
    diff_i = 0;
    if i > 1, diff_i = I(i) - I(i-1); end
    fprintf('%3d  %7.3f  %5d  %+7.3f\n', i, I(i), CNT(i), diff_i);
end

% Also show what the old algorithm found
fprintf('\n--- Old valley detection (I < Ipeak*0.15) ---\n');
I_peak = max(I);
valley_idx = find(I(2:end-1) < I(1:end-2) & I(2:end-1) < I(3:end) ...
                  & I(2:end-1) < I_peak * 0.15);
valley_idx = valley_idx + 1;
fprintf('I_peak=%.3fA threshold=%.3fA\n', I_peak, I_peak*0.15);
fprintf('Found %d valleys at indices: %s\n', length(valley_idx), mat2str(valley_idx));
