% analyze_K_factor.m — 统计 I_rms/I_peak (K) 分布, 铁锅/钢锅分文件
function analyze_K_factor()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
cal.V_SCALE = 3.3/4096/(6.2e3/(270e3*3+6.2e3));
base = '../../tools/ekf_tuner/captures/captures/';

% 钢锅 (steel)
steel_files = {'capture_20260602_152037.csv'};
% 铁锅 (iron) — 选几个代表性文件覆盖功率/频率范围
iron_files = {
    'capture_20260602_153316.csv', ...
    'capture_20260602_153326.csv', ...
    'capture_20260602_153338.csv', ...
    'capture_20260602_153347.csv', ...
    'capture_20260602_153424.csv'};

fprintf('========== K = I_rms / I_peak 分布 ==========\n');

for pot = 1:2
    if pot == 1
        flist = steel_files; label = 'STEEL';
    else
        flist = iron_files; label = 'IRON';
    end
    fprintf('\n--- %s ---\n', label);
    fprintf('%-40s %6s %7s %7s %7s %7s\n', 'File','N','K_mean','K_std','K_min','K_max');

    all_K = [];
    for fi = 1:length(flist)
        csv_path = [base flist{fi}];
        [frames, fN] = parse_csv_frames(csv_path);
        Ks = [];
        for f = 1:length(frames)
            fr = frames{f};
            I_adc = fr(:,2); CNT = fr(:,5); CMP = fr(1,6:9);
            I_adc = min(max(I_adc,0),4095);
            I = I_adc * cal.I_SCALE;

            % 去尾部饱和
            se = size(fr,1);
            while se>1 && I_adc(se)>4090, se=se-1; end
            if se>1, se=se-1; end
            if se<20, continue; end
            I_clean = I(1:se);

            % 找 CU/highOff 截止点: 用 CNT 找到上管关断位置
            dCNT = diff(CNT(1:se));
            dCNT_pos = dCNT(dCNT>0);
            if isempty(dCNT_pos), continue; end
            avg_dcnt = mean(dCNT_pos);
            CU = CMP(1); CO = CMP(2);
            % 找 CNT 最接近 CU 的点
            CNT_seg = CNT(1:se);
            [~, uoff_i] = min(abs(CNT_seg - CU));

            % I_peak: 导通区间内 max (方案修正后)
            if uoff_i > 5
                I_peak_on = max(I_clean(1:uoff_i));
            else
                I_peak_on = max(I_clean);
            end

            % I_rms: 导通区间 RMS
            if uoff_i > 10
                I_rms_on = rms(I_clean(1:uoff_i));
            else
                I_rms_on = rms(I_clean);
            end

            if I_peak_on > 1.0 && I_rms_on > 0.5
                K = I_rms_on / I_peak_on;
                Ks(end+1) = K;
            end
        end
        if ~isempty(Ks)
            fprintf('%-40s %6d %7.4f %7.4f %7.4f %7.4f\n', ...
                flist{fi}(1:min(40,end)), length(Ks), ...
                mean(Ks), std(Ks), min(Ks), max(Ks));
            all_K = [all_K, Ks];
        end
    end
    if ~isempty(all_K)
        fprintf('  TOTAL: N=%d  mean=%.4f  std=%.4f  CV=%.2f%%\n', ...
            length(all_K), mean(all_K), std(all_K), std(all_K)/mean(all_K)*100);
    end
end
fprintf('\n纯正弦理论值: K = 1/sqrt(2) = %.4f\n', 1/sqrt(2));
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
