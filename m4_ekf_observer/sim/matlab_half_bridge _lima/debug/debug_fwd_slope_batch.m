% 批量验证前向3点斜率递减法: d2>d1 判尖峰
function debug_fwd_slope_batch()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
base = '../../tools/ekf_tuner/captures/captures/';
files = {'capture_20260602_152037.csv', 'capture_20260602_153316.csv', ...
         'capture_20260602_153326.csv', 'capture_20260602_153424.csv'};

fprintf('%-35s %5s %8s %8s %8s %8s\n', 'File','Frm','Ipk','+50spk','d1','d2>d1?','fix_err');
fprintf('%s\n', repmat('-',1,80));

for fi = 1:length(files)
    [frames, fN] = parse_csv_frames([base files{fi}]);
    for f = 1:length(frames)
        fr = frames{f}; N = fN(f);
        I_adc=min(max(fr(:,2),0),4095); CNT=fr(:,5); CMP=fr(1,6:9);
        I=I_adc*cal.I_SCALE; CU=CMP(1); CO=CMP(2);

        se=N;
        while se>1&&I_adc(se)>4090, se=se-1; end
        if se>1, se=se-1; end
        if se<20, continue; end
        I_seg=I(1:se); CNT_seg=CNT(1:se);

        % 谷点找法: 从第一个上升沿起点开始
        [Ipk_raw, pk_i] = max(I_seg);
        if pk_i < 3, continue; end  % 需 pk-2 存在

        % 注入 50 ADC 尖峰
        I_test = I_seg; I_test(pk_i) = I_test(pk_i) + 50 * cal.I_SCALE;
        d1 = I_test(pk_i-1) - I_test(pk_i-2);
        d2 = I_test(pk_i)   - I_test(pk_i-1);

        if d2 > d1
            Ipk_fix = I_test(pk_i-1) + d1;
        else
            Ipk_fix = I_test(pk_i);
        end
        fix_err = Ipk_fix - Ipk_raw;

        [~,fn] = fileparts(files{fi});
        fprintf('%-35s %3d  %7.2f %7.2f %7.3f %7s %7.3f\n', ...
            fn(1:min(35,end)), f, Ipk_raw, max(I_test), d1, ...
            iff(d2>d1,'YES','no'), fix_err);
    end
end

fprintf('\n纯正弦理论: d1>0, d2<d1 (斜率单调递减趋近峰值)\n');
fprintf('判据: d2>d1 → 异常斜率反转 → 尖峰 → 用前推斜率外推\n');
end

function s = iff(flag, t, f)
if flag, s = t; else, s = f; end
end

function [frames, fN] = parse_csv_frames(csv_path)
fid=fopen(csv_path,'r');
fgetl(fid); ncol=length(strsplit(fgetl(fid),',')); frewind(fid); fgetl(fid);
frames={}; fN=[];
while ~feof(fid)
    line=fgetl(fid);
    if ~ischar(line)||isempty(strtrim(line)), continue; end
    if startsWith(line,'SIZE')
        parts=strsplit(line,','); N=str2double(parts{2}); fN(end+1)=N;
        fr=zeros(N,ncol);
        for i=1:N
            dl=fgetl(fid); dl=strtrim(dl);
            if isempty(dl), i=i-1; continue; end
            parts=strsplit(dl,',');
            for j=1:min(length(parts),ncol)
                v=str2double(parts{j}); if ~isnan(v), fr(i,j)=v; end
            end
        end
        frames{end+1}=fr;
    end
end
fclose(fid);
end
