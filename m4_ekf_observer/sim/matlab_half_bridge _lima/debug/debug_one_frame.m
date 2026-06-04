% debug_one_frame.m — 快速诊断
function debug_one_frame()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
base = '../../tools/ekf_tuner/captures/captures/';
csv_path = [base 'capture_20260602_153316.csv'];
[frames, fN] = parse_csv_frames(csv_path);
fprintf('Loaded %d frames\n', length(frames));

fr = frames{1};
N = fN(1);
t_us=fr(:,1); I_adc=fr(:,2); Vdc_adc=fr(:,4); CNT=fr(:,5);
CMP=fr(1,6:9);
fprintf('N=%d  CMP=[%d %d %d %d]\n', N, CMP(1), CMP(2), CMP(3), CMP(4));
fprintf('CNT range: %d..%d\n', min(CNT), max(CNT));
fprintf('I_adc range: %d..%d\n', min(I_adc), max(I_adc));
fprintf('I(A) range: %.2f..%.2f\n', min(I_adc)*cal.I_SCALE, max(I_adc)*cal.I_SCALE);

% Check if CU is in CNT range
CU = CMP(1);
fprintf('CU=%d, min(CNT)=%d, max(CNT)=%d\n', CU, min(CNT), max(CNT));

% Find uoff_i
CNT_clean = CNT;
[~, uoff_i] = min(abs(CNT_clean - CU));
fprintf('uoff_i=%d, n_on=%d\n', uoff_i, uoff_i);
end

function [frames, fN] = parse_csv_frames(csv_path)
fid = fopen(csv_path,'r');
header = fgetl(fid);
ncol = length(strsplit(header,','));
frames = {}; fN = [];
while ~feof(fid)
    line = fgetl(fid);
    if ~ischar(line)||isempty(strtrim(line)), continue; end
    if startsWith(line,'SIZE')
        parts = strsplit(line,','); N = str2double(parts{2}); fN(end+1)=N;
        fr = zeros(N,ncol);
        for i=1:N
            dl = fgetl(fid); dl = strtrim(dl);
            if isempty(dl), i=i-1; continue; end
            parts = strsplit(dl,',');
            for j=1:min(length(parts),ncol)
                v = str2double(parts{j}); if ~isnan(v), fr(i,j)=v; end
            end
        end
        frames{end+1} = fr;
    end
end
fclose(fid);
end
