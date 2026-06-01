% main.m  — 半桥感应加热电路工作模型 主控流程
%
% ========== 硬件测量能力 ==========
%   两路 ADC: Vdc (母线电压) + I (谐振电流, 全波整流后)
%   三路 HRTIM: CNT + CMP 四边沿
%   无谐振电压测量 → 功率/阻抗为估算值
% =================================
%
% CSV 格式 (9列带表头):
%   t_us,I_adc,V_adc,Vdc_adc,CNT,CMP_UON,CMP_UOFF,CMP_LON,CMP_LOFF
%   (V_adc 列为 Vdc, 与 Vdc_adc 重复, 兼容用)
%
% 使用前:
%   1. 将 CSV 数据文件放在本目录下，或修改 DATA_PATH
%   2. 运行 main
%   3. 无数据时自动生成演示数据，支持 FM/PWM 两种模式
%
% 数据采集模式:
%   MODE A: 慢扫 (20周期/1s) — 完整工频包络 (20ms), ~20kB/帧
%   MODE B: 快扫 (1周期/50ms) — 快速参量跟踪, ~1kB/帧
%
% 控制模式 (自动判别):
%   FM 模式 — 固定占空比(50%互补), 变频率调功
%   PWM 模式 — 固定频率, 变占空比(10%~50%)调功, 最小脉宽 6μs
%
% 数据流:
%   CSV → load_data → raw (含 Vdc)
%   raw → calc_timing → timing (控制模式/CMP四边沿/死区)
%   raw → calc_waveform → waveform (I_RMS/I_peak/f_res, 谷值法)
%   raw, waveform, timing → calc_power → power (Vdc+I估算)
%   waveform, power → calc_impedance → impedance (基波法估算)
%   raw, waveform, power → calc_ac_cycle → ac_cycle (50Hz包络)
%   全部 → plot_panel → 可视化 (5面板)

clear; clc; close all;
fprintf('========== 半桥感应加热电路模型 ==========\n\n');

%% 1) 加载标定系数
calibration;   % 运行 calibration.m 定义各系数

%% 2) 配置
% ========== 用户配置区 ==========
DATA_PATH    = 'data.csv';       % CSV 数据文件路径
DATA_MODE    = 'A';              % 'A' = 20周期/1s, 'B' = 1周期/50ms
DEMO_MODE    = 'FM';             % 演示模式: 'FM'=调频 或 'PWM'=调占空比
SAVE_RESULT  = true;             % 是否保存 result.mat
% ================================

cal.DATA_MODE = DATA_MODE;

%% 3) 数据加载
fprintf('\n--- 第1步: 数据加载 ---\n');
if ~exist(DATA_PATH, 'file')
    warning('[main] 数据文件 %s 不存在！生成演示数据用于调试验证。', DATA_PATH);
    generate_demo_data(DATA_PATH, cal, DEMO_MODE);
end

raw = load_data(DATA_PATH, cal);

%% 4) HRTIM 时序分析
fprintf('\n--- 第2步: HRTIM 时序分析 ---\n');
timing = calc_timing(raw, cal);

%% 5) 时域参量
fprintf('\n--- 第3步: 时域参量 ---\n');
waveform = calc_waveform(raw, timing);

%% 6) 功率计算
fprintf('\n--- 第4步: 功率计算 ---\n');
power = calc_power(raw, waveform, timing);

%% 7) 阻抗建模
fprintf('\n--- 第5步: 负载参数估算 (无谐振电压) ---\n');
impedance = calc_impedance(waveform, power);

%% 8) 工频包络 (仅模式A)
fprintf('\n--- 第6步: 工频包络分析 ---\n');
ac_cycle = calc_ac_cycle(raw, waveform, power);

%% 9) 可视化
fprintf('\n--- 第7步: 可视化 ---\n');
plot_panel(raw, timing, waveform, power, impedance, ac_cycle);

%% 打印无谐振电压提示
fprintf('\n⚠ 注意: 当前仅采集 Vdc(母线) + I(整流后) 两路\n');
fprintf('       功率和阻抗值为估算, 需V_resonant精确计算\n');

%% 10) 保存结果
result = struct();
result.raw       = raw;
result.timing    = timing;
result.waveform  = waveform;
result.power     = power;
result.impedance = impedance;
result.ac_cycle  = ac_cycle;
result.cal       = cal;

if SAVE_RESULT
    save('result.mat', '-struct', 'result');
    fprintf('[main] 结果已保存: result.mat\n');
end

fprintf('\n========== 完成 ==========\n');

%% ——— 演示数据生成器（无真实 CSV 时使用） ———
function generate_demo_data(csv_path, cal, ctrl_mode)
% 生成模拟的半桥谐振周期数据
%   ctrl_mode = 'FM' (调频) 或 'PWM' (调占空比)

fprintf('[demo] 生成演示数据(%s) → %s\n', ctrl_mode, csv_path);

% ===== 基础参数 =====
Fs     = 1e6;                % 采样率 1MHz (1μs/点)
f_ac   = 50;                 % 工频 50Hz
HRTIM_CLK = 144e6;           % HRTIM 时钟 144MHz

% 模式A: 20ms 完整工频周期 (约500个谐振周期@25kHz)
T_total = 20e-3;             % 20ms
n_total = round(T_total * Fs);
t = (0:n_total-1)' / Fs;     % 时间向量 (s)

% 直流母线: 220V整流后 311V 带100Hz纹波
Vdc_raw = 311 * abs(sin(pi * f_ac * t));  % 100Hz纹波
Vdc_noise = Vdc_raw + 2 * randn(n_total, 1);

% 谐振电流包络 (跟随母线电压)
I_env = 15 * (Vdc_raw / 311);
phi = 15 * pi / 180;         % 感性负载 15° 相位超前

% ===== 根据控制模式生成不同数据 =====
switch ctrl_mode
    case 'FM'
        % ========== 调频模式 (FM) ==========
        % 固定占空比 50%, 频率从 20~30kHz 扫频
        f_sw_min = 20000;  f_sw_max = 30000;
        f_sw_t = f_sw_min + (f_sw_max - f_sw_min) * (t / T_total);
        
        % 瞬时相位 (积分频率)
        phase_sw = 2*pi * cumtrapz(t, f_sw_t);
        
        % 谐振电压/电流
        I = I_env .* sin(phase_sw) + 0.5 * randn(n_total, 1);
        V = 1.2 * 311 * (Vdc_raw/311) .* sin(phase_sw + phi) + 1.0 * randn(n_total, 1);
        
        % HRTIM CNT (变频率 → 变周期)
        cnt_period_inst = round(HRTIM_CLK ./ f_sw_t);
        CNT = mod(round(t * HRTIM_CLK), cnt_period_inst);
        
        % CMP: 50% 互补固定
        D_fixed = 0.50;
        DT_cnt = 50;   % 死区 ~0.35μs
        CMP_UON  = zeros(n_total, 1);
        CMP_UOFF = round(D_fixed * cnt_period_inst);
        CMP_LON  = min(CMP_UOFF + DT_cnt, cnt_period_inst - DT_cnt);
        CMP_LOFF = cnt_period_inst - DT_cnt;
        
        fprintf('[demo] FM模式: f_sw=%d~%dHz  D=%.0f%%(固定)  死区=%dcnt\n', ...
                f_sw_min, f_sw_max, D_fixed*100, DT_cnt);
        
    case 'PWM'
        % ========== 调占空比模式 (PWM) ==========
        % 固定频率 25kHz, 占空比从 10%~50% 扫描
        f_sw_fixed = 25000;
        cnt_period = round(HRTIM_CLK / f_sw_fixed);
        CNT = mod(round(t * HRTIM_CLK), cnt_period);
        
        % 占空比随时间从 10% → 50% 变化
        D_min = 0.10;  D_max = 0.50;
        D_t = D_min + (D_max - D_min) * (t / T_total);
        % 确保最小脉宽 ≥ 6μs
        min_pulse_cnt = max(round(6e-6 * HRTIM_CLK), 1);
        D_t = max(D_t, min_pulse_cnt / cnt_period);
        
        % 谐振频率固定
        phase_sw = 2*pi * f_sw_fixed * t;
        
        % 电压/电流 (占空比影响输出功率)
        power_factor = (D_t - D_min) / (D_max - D_min + eps);
        I_pk_mod = I_env .* (0.3 + 0.7 * power_factor);
        I = I_pk_mod .* sin(phase_sw) + 0.5 * randn(n_total, 1);
        V = 1.2 * 311 * (Vdc_raw/311) .* (0.3 + 0.7 * power_factor) ...
            .* sin(phase_sw + phi) + 1.0 * randn(n_total, 1);
        
        % CMP: 变占空比 (上短下长)
        DT_cnt = 50;
        CMP_UON  = zeros(n_total, 1);
        CMP_UOFF = round(D_t * cnt_period);
        CMP_LON  = min(CMP_UOFF + DT_cnt, cnt_period - DT_cnt);
        CMP_LOFF = cnt_period - DT_cnt;
        
        fprintf('[demo] PWM模式: f_sw=%dHz(固定)  D=%.0f%%~%.0f%%  死区=%dcnt\n', ...
                f_sw_fixed, D_min*100, D_max*100, DT_cnt);
end

% ===== 写入 CSV (9列) =====
fid = fopen(csv_path, 'w');
fprintf(fid, 't_us,I_adc,V_adc,Vdc_adc,CNT,CMP_UON,CMP_UOFF,CMP_LON,CMP_LOFF\n');

step = 10;  % 每10μs输出一个点 = 100kHz 输出率
for i = 1:step:n_total
    t_us   = t(i) * 1e6;
    I_adc  = I(i) / cal.I_SCALE;
    V_adc  = V(i) / cal.V_SCALE;
    Vdc_a  = Vdc_noise(i) / cal.VDC_SCALE;
    fprintf(fid, '%.3f,%.4f,%.4f,%.4f,%d,%d,%d,%d,%d\n', ...
        t_us, I_adc, V_adc, Vdc_a, round(CNT(i)), ...
        round(CMP_UON(i)), round(CMP_UOFF(i)), ...
        round(CMP_LON(i)), round(CMP_LOFF(i)));
end
fclose(fid);

fprintf('[demo] 生成完成: %d 行数据 (9列)\n', round(n_total/step));
end
