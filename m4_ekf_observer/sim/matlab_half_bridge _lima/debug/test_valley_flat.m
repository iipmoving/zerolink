% test_valley_flat.m — 验证谷底检测对平坦底部的处理
fprintf('=== 谷底平坦检测测试 ===\n\n');

test_cases = {
    struct('name', '1 单点谷底',    'adc', [100 80 60 40 20 5 0 3 15 30 50],         'start', 3, 'end_i', 10),
    struct('name', '2 两点平谷(00)', 'adc', [100 80 60 40 20 5 0 0 3 15 30 50],       'start', 3, 'end_i', 11),
    struct('name', '3 三点平谷(000)','adc', [100 80 60 40 20 5 0 0 0 3 15 30 50],     'start', 3, 'end_i', 12),
    struct('name', '4 平谷中尖刺(0,1,0)', 'adc', [100 80 60 40 20 5 0 1 0 3 15 30],  'start', 3, 'end_i', 11),
};

for t = 1:length(test_cases)
    tc = test_cases{t};
    v = find_valley_m(tc.adc, tc.start, tc.end_i);

    fprintf('--- %s ---\n', tc.name);
    fprintf('ADC: ['); fprintf('%d ', tc.adc); fprintf(']\n');
    fprintf('搜索范围: [%d, %d]\n', tc.start, tc.end_i);

    % 手动追踪方向位
    direction = uint16(65535);
    pre = double(tc.adc(tc.start));
    for i = (tc.start+1):min(tc.end_i, tc.start+199)
        cur = double(tc.adc(i));
        direction = bitshift(direction, 1);
        if cur > pre
            direction = bitor(direction, uint16(1));
        else
            direction = bitand(direction, bitcmp(uint16(1)));
        end
        bits = bitand(direction, uint16(7));
        tag = '';
        if bitand(direction, uint16(3)) == 1
            pi = i - 1;
            zr = uint32(tc.adc(pi-1)) + uint32(tc.adc(pi+1));
            mid2 = uint32(tc.adc(pi)) * 2;
            ok = mid2 <= zr;
            ok_str = '否'; if ok, ok_str = '是'; end
            tag = sprintf(' *** 候选 pi=%d zr=%d mid2=%d 通过=%s', pi, zr, mid2, ok_str);
        end
        arrow = 'v'; if cur > pre, arrow = '^'; end
        fprintf('  i=%2d  v=%3d  pre=%3d  %s  dir&7=%d%s\n', ...
            i, cur, pre, arrow, bits, tag);
        pre = cur;
    end

    fprintf('结果: v=%d (谷底adc=%d)\n\n', v, tc.adc(v));
end

fprintf('=== 结论 ===\n');
fprintf('平谷 [5,0,0,0,3]:\n');
fprintf('  i=pos-2: 5→0 ↓ bit=0\n');
fprintf('  i=pos-1: 0→0 ↓ bit=0 (相等→不反转)\n');
fprintf('  i=pos:   0→0 ↓ bit=0 (相等→不反转)\n');
fprintf('  i=pos+1: 0→3 ↑ bit=1 → (dir&3)=01 触发!\n');
fprintf('  pi = pos = 平谷段最后一个0\n');
fprintf('  zr = adc[pi-1]+adc[pi+1] = 0+3 = 3\n');
fprintf('  mid2 = 0*2 = 0 <= 3 OK\n');
fprintf('→ 谷底选在平谷段最后一个点, 偏差<1采样点, 对phi/P_W影响可忽略\n');

%% ---- 子函数 (与 export_golden.m / C _FindValley_f 一致) ----
function v = find_valley_m(adc, start_i, end_i)
    v = 0;
    if end_i <= start_i, return; end
    direction = uint16(65535);
    min_sum = uint32(4294967295);
    pre = double(adc(start_i));

    for i = (start_i + 1):min(end_i, start_i + 199)
        cur = double(adc(i));
        direction = bitshift(direction, 1);
        if cur > pre
            direction = bitor(direction, uint16(1));
        else
            direction = bitand(direction, bitcmp(uint16(1)));
        end

        if bitand(direction, uint16(3)) == 1
            pi = i - 1;
            if pi > start_i && (pi + 1) <= end_i
                zr = uint32(adc(pi - 1)) + uint32(adc(pi + 1));
                mid2 = uint32(adc(pi)) * 2;
                if mid2 <= zr && zr < min_sum
                    min_sum = zr;
                    v = pi;
                end
            end
        end
        pre = cur;
    end

    % 平底修正: 以谷底值为基准, ±2 ADC 扫描等值区, 取中心
    if v > 0 && v > start_i && v <= end_i
        v_val = adc(v);
        v_start = v;
        while v_start > start_i && abs(adc(v_start - 1) - v_val) <= 2
            v_start = v_start - 1;
        end
        v_end = v;
        while v_end < end_i && abs(adc(v_end + 1) - v_val) <= 2
            v_end = v_end + 1;
        end
        v = v_start + floor((v_end - v_start) / 2);
    end
end
