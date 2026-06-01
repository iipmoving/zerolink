% calibration.m
% 半桥感应加热 — 硬件标定系数 (修正版 v2)
%
% === 修正内容 ===
% v1 错误: 把 ADC 数字值(0~4095)当成了引脚电压直接乘分压比倒数
% v2 修正: 计入 12bit ADC, Vref=3.3V, 先还原引脚电压再乘分压比
%
% V_actual = ADC_raw × Vref / 4096 × (R_TOP + R_BOT) / R_BOT
% I_actual = ADC_raw × Vref / 4096 / I_DIV_RATIO
% =========================================================

%% ——— ADC 参数 ———
ADC_BITS  = 12;                 % 12 位
ADC_STEPS = 2^ADC_BITS;         % 4096
VREF      = 3.3;                % ADC 参考电压 (V)
ADC_LSB   = VREF / ADC_STEPS;   % 每 LSB = 0.8057 mV

%% ——— 电压分压标定 (母线电压 Vdc) ———
% 3×270K 上拉 + 6.2K 下拉 → ADC 引脚
R_TOP    = 270e3 * 3;           % 810 kΩ
R_BOT    = 6.2e3;               % 6.2 kΩ
V_DIV    = R_BOT / (R_TOP + R_BOT);  % 分压比 ≈ 0.007597

% 综合标定系数: ADC_raw → 实际电压(V)
% V_actual = ADC_raw × Vref / 4096 / V_DIV
%          = ADC_raw × 3.3 / 4096 / 0.007597
%          = ADC_raw × 0.10606
V_SCALE  = VREF / ADC_STEPS / V_DIV;  % ≈ 0.10606 V/count

% 验证: ADC=2850 → V=2850×0.10606 ≈ 302V ← 220V整流母线 ✓
%        ADC=4095 → V=4095×0.10606 ≈ 434V ← 最大量程

%% ——— 电流标定 (谐振电流, 全波整流后) ———
% CT 2000:1, 取样电阻 2kΩ → 全波整流 → 10kΩ+330Ω 分压 → ADC
CT_RATIO     = 2000;            % 2000:1
R_BURDEN     = 2e3;             % 2 kΩ
I_DIV_TOP    = 10e3;            % 10 kΩ
I_DIV_BOT    = 330;             % 330 Ω
I_DIV_RATIO  = I_DIV_BOT / (I_DIV_TOP + I_DIV_BOT);  % ≈ 0.03194

% 电流路径: I_primary → CT(2000:1) → 2kΩ → V_burden = I_primary × 2000/2000
%          → 10k:330分压 → ADC_pin = V_burden × 0.03194
%          → ADC_raw = ADC_pin / Vref × 4096
%
% I_primary = ADC_raw × Vref / 4096 / I_DIV_RATIO × CT_RATIO / R_BURDEN
%           = ADC_raw × 3.3 / 4096 / 0.03194 × 2000 / 2000
%           = ADC_raw × 0.02523
I_SCALE  = VREF / ADC_STEPS / I_DIV_RATIO;  % ≈ 0.02523 A/count
% I_SCALE 中 CT_RATIO/R_BURDEN = 2000/2000 = 1, 已约掉

% 验证: ADC=795 → I=795×0.02523 ≈ 20.0A ← 2kW级IH合理 ✓
%        ADC=4095 → I=4095×0.02523 ≈ 103A ← 最大量程

%% ——— 直流母线电压 (同电压分压网络) ———
VDC_SCALE = V_SCALE;             % Vdc_actual = Vdc_adc × VDC_SCALE

%% ——— 电网参数 ———
F_AC     = 50;                   % Hz
T_AC     = 1 / F_AC;             % 20 ms
V_AC_RMS = 220;                  % V
V_AC_PK  = V_AC_RMS * sqrt(2);  % ≈ 311 V

%% ——— 最小脉宽约束 ———
MIN_PULSE_US = 6;                % 硬件最小脉宽 6 μs

%% ——— 控制模式 ———
CTRL_MODE = 'auto';              % 'FM'=调频, 'PWM'=调占空比, 'auto'=自动判别

%% ——— 测量能力声明 ———
% 硬件只采集了 母线电压(Vdc) + 谐振电流(I,全波整流)
% 无谐振电压测量 → 以下参量需降级/估算:
%   [✓] 可直接算: HRTIM时序, I_RMS/I_peak, f_res, Vdc_mean/纹波, 频率比
%   [≈] 可估算:   功率(从Vdc+I估算), R_eq(从电流包络)
%   [✗] 需电压:   阻抗|Z|, 相位差φ, 视在功率S, L_eq/C_eq精确值

fprintf('[calibration] v2 ✓\n');
fprintf('  V_SCALE=%.5f V/cnt  (ADC=%d→%.1fV)\n', V_SCALE, 2850, 2850*V_SCALE);
fprintf('  I_SCALE=%.5f A/cnt  (ADC=%d→%.1fA)\n', I_SCALE, 795, 795*I_SCALE);
fprintf('  VDC_SCALE=%.5f V/cnt\n', VDC_SCALE);
