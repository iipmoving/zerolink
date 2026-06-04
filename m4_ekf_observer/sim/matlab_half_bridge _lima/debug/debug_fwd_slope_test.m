% debug_fwd_slope_test.m — 前向斜率递减法: d1 > d2 判尖峰
function debug_fwd_slope_test()
cal.I_SCALE = 3.3/4096/(330/(10e3+330));
base = '../../tools/ekf_tuner/captures/captures/';

[frames, fN] = parse_csv_frames([base 'capture_20260602_153316.csv']);
fr = frames{4}; N = fN(4);

I_adc=fr(:,2); CNT=fr(:,5); CMP=fr(1,6:9); t_us=fr(:,1);
I_adc=min(max(I_adc,0),4095);
I = I_adc * cal.I_SCALE;
CU=CMP(1); CO=CMP(2);

seg_end=N;
while seg_end>1 && I_adc(seg_end)>4090, seg_end=seg_end-1; end
if seg_end>1, seg_end=seg_end-1; end
CNT_seg=CNT(1:seg_end); I_seg=I(1:seg_end); t_seg=t_us(1:seg_end);

% 频率
dCNT=diff(CNT_seg); dCNT_pos=dCNT(dCNT>0);
avg_dcnt=mean(dCNT_pos);
dtv=diff(t_seg); dtv(dtv<=0)=[];
avg_dt_us=mean(dtv);
wi=find(diff(CNT(1:seg_end))<0,1);
if ~isempty(wi)
    CNT_range=CNT_seg(wi)+round(avg_dcnt)-CNT_seg(wi+1);
else
    CNT_range=max(CNT_seg)-min(CNT_seg);
end
f_sw=1/(CNT_range*avg_dt_us/avg_dcnt*1e-6);
fprintf('f_sw=%.2fkHz  dt=%.2fus\n', f_sw/1e3, avg_dt_us);

% 导通区间
cond_idx=[];
for i=1:seg_end
    cnt=CNT_seg(i);
    if CU<CO, ok=(cnt>=CU&&cnt<=CO); else, ok=(cnt>=CU||cnt<=CO); end
    if ok, cond_idx(end+1)=i; end
end
I_cond=I_seg(cond_idx);
t_cond=t_seg(cond_idx);
n_cond=length(I_cond);
[Ipk_raw, pk_rel]=max(I_cond);
Ipk_true=Ipk_raw;

fprintf('True peak: %.2fA (ADC=%d) at idx=%d/%d\n', ...
    Ipk_raw, round(Ipk_raw/cal.I_SCALE), pk_rel, n_cond);

%% 前向斜率检测
% 需要 pk-2, pk-1, pk 三个点
fprintf('\n=== 前向3点斜率递减检测 ===\n');
fprintf('Spike  d1(pk-2->pk-1)  d2(pk-1->pk)  d2>d1?  Action\n');
fprintf('%s\n', repmat('-',1,60));

spike_levels = [0, 5, 10, 15, 20, 30, 50];

for si = 1:length(spike_levels)
    spike_adc = spike_levels(si);
    I_test = I_cond;
    I_test(pk_rel) = I_test(pk_rel) + spike_adc * cal.I_SCALE;

    % 前向3点: pk-2, pk-1, pk
    if pk_rel >= 3
        d1 = I_test(pk_rel-1) - I_test(pk_rel-2);  % 前推第2差分
        d2 = I_test(pk_rel)   - I_test(pk_rel-1);  % 前推第1差分

        if d2 > d1
            % 斜率递增 → 尖峰 → 用 pk-1 + d1 推算峰值
            Ipk_fixed = I_test(pk_rel-1) + d1;
            action = sprintf('FIX: %.2f -> %.2f', I_test(pk_rel), Ipk_fixed);
        else
            % 斜率递减 → 正常正弦
            Ipk_fixed = I_test(pk_rel);
            action = 'OK (slope decr)';
        end
    else
        d1=NaN; d2=NaN; Ipk_fixed=I_test(pk_rel);
        action = '(no pk-2, skip)';
    end

    err = Ipk_fixed - Ipk_true;
    fprintf('%4d   %8.3f        %8.3f        %4s    %s\n', ...
        spike_adc, d1, d2, iff(~isnan(d1)&&d2>d1,'YES','no'), action);
    if abs(err) > 0.01
        fprintf('       -> fixed err=%+.3fA (%.1f%%)\n', err, err/Ipk_true*100);
    end
end

%% 多帧批量验证
fprintf('\n=== 批量验证 (iron 153316 全部8帧) ===\n');
fprintf('Frame  True_Ipk  Spike50  raw_err  fixed_err  OK?\n');
for f = 1:length(frames)
    fr2 = frames{f};
    I_adc2=min(max(fr2(:,2),0),4095); CNT2=fr2(:,5); CMP2=fr2(1,6:9);
    CU2=CMP2(1); CO2=CMP2(2);
    se=size(fr2,1);
    while se>1 && I_adc2(se)>4090, se=se-1; end
    if se>1, se=se-1; end
    if se<20, continue; end
    I2=I_adc2(1:se)*cal.I_SCALE; CNT2=CNT2(1:se);

    % 导通区间
    cidx=[];
    for i=1:se
        cnt=CNT2(i);
        if CU2<CO2, ok=(cnt>=CU2&&cnt<=CO2); else, ok=(cnt>=CU2||cnt<=CO2); end
        if ok, cidx(end+1)=i; end
    end
    if length(cidx)<6, continue; end
    I_c=I2(cidx); n_c=length(I_c);
    [Ipk_t, pk_r]=max(I_c);
    if pk_r<3, continue; end

    % 注入50 ADC尖峰
    I_test=I_c; I_test(pk_r)=I_test(pk_r)+50*cal.I_SCALE;
    Ipk_raw50=max(I_test);
    d1=I_test(pk_r-1)-I_test(pk_r-2);
    d2=I_test(pk_r)-I_test(pk_r-1);
    if d2>d1
        Ipk_fix50=I_test(pk_r-1)+d1;
    else
        Ipk_fix50=I_test(pk_r);
    end
    ok_mark = iff(abs(Ipk_fix50-Ipk_t)<0.2, 'OK', 'FAIL');
    fprintf('%3d     %6.2f     %6.2f   %6.2f    %6.2f    %s\n', ...
        f, Ipk_t, Ipk_raw50, Ipk_raw50-Ipk_t, Ipk_fix50-Ipk_t, ok_mark);
end
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
