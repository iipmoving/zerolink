function raw = load_data(csv_path, cal)
% load_data  — 导入 CSV 并还原物理量
%   csv_path : CSV 文件路径
%   cal      : calibration struct (含 V_SCALE, I_SCALE 等)
%   返回 raw : struct 含 t, I, V, Vdc, CNT, CMP (4列), meta
%
% CSV 定义格式:
%   第1行:       t_us, I_adc, V_adc, Vdc_adc, CNT, CMP_UON, CMP_UOFF, CMP_LON, CMP_LOFF  (全局表头)
%   每帧第1行:   SIZE, N, N, N, N, N, 1, 1, 1, 1  (列采样数, "SIZE"文本标记帧边界)
%   每帧第2-N行: 数据行, CMP列仅首行有值(稀疏), fillmissing前向填充
%   帧间空行:    分隔 (readmatrix自动跳过)
%
% t_us    : 时间戳 (μs)
% I_adc   : 谐振电流 ADC 原始值
% V_adc   : 母线电压 ADC 原始值
% Vdc_adc : 直流母线电压 ADC 均值 (每帧内)
% CNT     : HRTIM 计数器同步采样值
% CMP_UON / CMP_UOFF / CMP_LON / CMP_LOFF : 四边沿比较值 (帧级参数)
% POWER   : 当前功率参考值 (帧级参数, 仅供参考)

%% 1) 读取 CSV
if ~exist(csv_path, 'file')
    error('[load_data] 文件不存在: %s', csv_path);
end

% 读取数值 (跳过全局表头, SIZE行/空行由 readmatrix 自动处理)
data = readmatrix(csv_path, 'NumHeaderLines', 1);
if isempty(data)
    error('[load_data] CSV 为空或只有表头');
end

% 读取表头
fid = fopen(csv_path, 'r');
header_line = fgetl(fid);
fclose(fid);
fprintf('[load_data] 表头: %s\n', header_line);

[n_rows, n_cols] = size(data);
fprintf('[load_data] 读取 %d 行 × %d 列\n', n_rows, n_cols);

% 检测 SIZE 行 (col 1 = "SIZE" → readmatrix 读为 NaN)
size_idx = find(isnan(data(:, 1)));
n_frames = length(size_idx);
fprintf('[load_data] 检测到 %d 个 SIZE 标记 → %d 帧\n', n_frames, n_frames);
for i = 1:n_frames
    fprintf('[load_data]   帧%d SIZE: %s\n', i-1, ...
            strjoin(string(data(size_idx(i), 2:end)), ','));
end

% 移除 SIZE 行
data(size_idx, :) = [];
[n_rows, n_cols] = size(data);
fprintf('[load_data] 移除 SIZE 行后: %d 行 × %d 列\n', n_rows, n_cols);

%% 2) 列赋值 + 前向填充参数列
% 格式: t_us(1), I_adc(2), V_adc(3), Vdc_adc(4), CNT(5),
%        CMP_UON(6), CMP_UOFF(7), CMP_LON(8), CMP_LOFF(9)

% 前向填充 CMP 列 (每帧仅首行有值, 其余为NaN)
if n_cols >= 6
    data(:, 6:end) = fillmissing(data(:, 6:end), 'previous');
end

t_us    = data(:, 1);          % μs
I_adc   = data(:, 2);
V_adc   = data(:, 3);
Vdc_adc = data(:, 4);          % 直流母线电压 ADC (帧内均值)
CNT     = data(:, 5);
CMP     = data(:, 6:9);        % UON, UOFF, LON, LOFF

% POWER 列 (新格式不含, 填充NaN)
POWER = NaN(n_rows, 1);

%% 3) 物理量还原
t      = t_us * 1e-6;          % μs → s
I      = I_adc * cal.I_SCALE;   % A
V      = V_adc * cal.V_SCALE;   % V
Vdc    = Vdc_adc * cal.VDC_SCALE;  % V (直流母线电压)

%% 4) 自动检测 HRTIM 时钟 & 周期
% 从 CNT 范围推算
CNT_min = min(CNT);
CNT_max = max(CNT);
CNT_range = CNT_max - CNT_min;  % 暂存, 后面若有绕回再修正

% 从 t 向量估计实际时间跨度 → 推算 HRTIM 时钟
T_span = max(t) - min(t);          % 秒

% 用 t 的采样间隔来推算
dt_us = median(diff(t_us));
dt_us(dt_us <= 0) = [];   % 防重复时间戳
avg_dt_us = mean(dt_us);

% 一个 HRTIM 计数的实际时间 = avg_dt_us / mean(diff(CNT))
dCNT = diff(CNT);
dCNT(dCNT <= 0) = [];      % 清除翻转点
if ~isempty(dCNT)
    avg_dcnt = mean(dCNT);
    t_per_cnt_us = avg_dt_us / avg_dcnt;  % μs / count
    hrtim_clk_mhz = round(1 / (t_per_cnt_us * 1e-6) / 1e6);
    % HRTIM 时钟范围: RX32G410 可达 6.17GHz (162ps)
    % 用预分频后有效计数时钟, 上限放宽到 10000
    if hrtim_clk_mhz < 1 || hrtim_clk_mhz > 10000
        % 检测失败，用默认值
        hrtim_clk_mhz = cal.HRTIM_CLK_MHZ;
        t_per_cnt_us = 1 / (hrtim_clk_mhz * 1e6) * 1e6;
    end
else
    % 单周期数据，无法检测
    hrtim_clk_mhz = cal.HRTIM_CLK_MHZ;
    t_per_cnt_us = 1 / (hrtim_clk_mhz * 1e6) * 1e6;
end

% 修正 CNT_range: 检测绕回, 计算真实周期
% CNT_max - CNT_min 在绕回时偏小一整步
wrap_fix = find(diff(CNT) < 0, 1);
if ~isempty(wrap_fix)
    cnt_before = CNT(wrap_fix);
    cnt_after  = CNT(wrap_fix + 1);
    CNT_range = cnt_before + round(avg_dcnt) - cnt_after;
    fprintf('[load_data] CNT绕回: %d→%d, 修正period=%d cnt\n', ...
            cnt_before, cnt_after, CNT_range);
end

% HRTIM 周期（秒）
HRTIM_period_s = CNT_range * t_per_cnt_us * 1e-6;
f_sw = 1 / HRTIM_period_s;

fprintf('[load_data] HRTIM 检测: clk≈%dMHz  f_sw≈%.1fkHz  period≈%.1fμs\n', ...
        hrtim_clk_mhz, f_sw/1e3, HRTIM_period_s*1e6);

%% 5) 打包输出
raw.t        = t;
raw.I        = I;       % A
raw.V        = V;       % V
raw.Vdc      = Vdc;     % V (直流母线电压, 含100Hz纹波)
raw.CNT      = CNT;
raw.CMP      = CMP;     % [UON, UOFF, LON, LOFF]
raw.POWER    = POWER;   % 功率参考值 (仅供参考, 不参与计算)
raw.I_adc    = I_adc;   % 保存原始值备查
raw.V_adc    = V_adc;
raw.Vdc_adc  = Vdc_adc;
raw.t_us     = t_us;

% 元信息
raw.meta.csv_path      = csv_path;
raw.meta.n_rows        = n_rows;
raw.meta.n_cols        = n_cols;
raw.meta.hrtim_clk_mhz = hrtim_clk_mhz;
raw.meta.t_per_cnt_us  = t_per_cnt_us;
raw.meta.HRTIM_period_s = HRTIM_period_s;
raw.meta.f_sw          = f_sw;
raw.meta.CNT_range     = CNT_range;
raw.meta.CNT_min       = CNT_min;
raw.meta.CNT_max       = CNT_max;

% 数据模式推测：如果时间跨度接近 20ms → 模式A (整周期)
T_total_ms = (max(t) - min(t)) * 1000;
if abs(T_total_ms - 20) < 2
    raw.meta.data_mode = 'A';   % 20ms 工频整周期
else
    raw.meta.data_mode = 'B';   % 单周期
end
fprintf('[load_data] 数据模式: %s  (时间跨度 %.2f ms)\n', ...
        raw.meta.data_mode, T_total_ms);

end
