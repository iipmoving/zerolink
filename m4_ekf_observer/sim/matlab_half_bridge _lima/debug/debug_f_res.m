% debug_f_res.m
csv_path = 'r104850.csv';
fid = fopen(csv_path, 'r');
fgetl(fid);
all_I = []; all_CNT = [];
while ~feof(fid)
    line = fgetl(fid);
    if ~ischar(line), break; end
    line = strtrim(line);
    if isempty(line) || startsWith(line, 'SIZE'), continue; end
    parts = strsplit(line, ',', 'CollapseDelimiters', false);
    t_val = str2double(strtrim(parts{1}));
    if isnan(t_val), continue; end
    all_I(end+1)   = str2double(strtrim(parts{2}));
    all_CNT(end+1) = str2double(strtrim(parts{5}));
end
fclose(fid);
n_total = length(all_I);
fprintf('Total: %d  I=[%d,%d]  CNT=[%d,%d]\n', n_total, min(all_I), max(all_I), min(all_CNT), max(all_CNT));
seg = floor(n_total / 4);
for sg = 0:3
    s0 = sg * seg + 1; s1 = min(s0 + seg, n_total);
    vi = find_valley_m(all_I, s0, s1);
    fprintf('Seg%d [%d,%d] I=[%d,%d] v=%d I(v)=%d CNT(v)=%d\n', ...
        sg, s0, s1, min(all_I(s0:s1)), max(all_I(s0:s1)), vi, all_I(vi), all_CNT(vi));
end
fprintf('\nValley CNT diffs:\n');
% manual: find all clear valleys (I < I_peak*0.15 and local minimum)
I_pk = max(all_I(all_I < 60000));  % exclude outlier
valleys = find(all_I(2:end-1) <= all_I(1:end-2) & all_I(2:end-1) <= all_I(3:end) & all_I(2:end-1) < I_pk*0.15) + 1;
fprintf('Simple valley detection (I<Ipk*0.15): %d valleys found\n', length(valleys));
for k = 1:min(length(valleys), 15)
    fprintf('  v%d: idx=%d I=%d CNT=%d\n', k, valleys(k), all_I(valleys(k)), all_CNT(valleys(k)));
end
% compute f_res from simple valleys
if length(valleys) >= 2
    dt_sum = 0; np = 0;
    for k = 1:length(valleys)-1
        dt = double(all_CNT(valleys(k+1))) - double(all_CNT(valleys(k)));
        if dt > 100  % require meaningful spacing
            T_half_us = dt * 162.0e-6;
            dt_sum = dt_sum + T_half_us;
            np = np + 1;
            fprintf('  dt(%d->%d)=%d cnt = %.3f us\n', k, k+1, dt, T_half_us);
        end
    end
    if np > 0
        f_res = 1000 / (2 * dt_sum / np);
        fprintf('f_res = %.2f kHz\n', f_res);
    end
end

function v = find_valley_m(adc, start_i, end_i)
    v = 0;
    if end_i <= start_i, return; end
    direction = uint16(65535);
    min_sum = uint32(4294967295);
    pre = double(adc(start_i));
    n = 0;
    for i = (start_i + 1):min(end_i, start_i + 199)
        cur = double(adc(i));
        direction = bitshift(direction, 1);
        if cur > pre, direction = bitor(direction, uint16(1));
        else, direction = bitand(direction, bitcmp(uint16(1))); end
        if bitand(direction, uint16(3)) == 1
            pi = i - 1; n = n + 1;
            if pi > start_i && (pi + 1) <= end_i
                zr = uint32(adc(pi - 1)) + uint32(adc(pi + 1));
                mid2 = uint32(adc(pi)) * 2;
                if mid2 <= zr && zr < min_sum
                    min_sum = zr; v = pi;
                end
            end
        end
        pre = cur;
    end
    fprintf('  triggers=%d min_zr=%d\n', n, min_sum);
end
