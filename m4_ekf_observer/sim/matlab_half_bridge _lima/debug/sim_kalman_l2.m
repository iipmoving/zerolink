% sim_kalman_l2 — 多场景仿真: 功率爬坡跟随 + 快速波动 + 阶跃检测
function sim_kalman_l2()
rng(42);

dt     = 0.010;          % 10ms/frame
T_sim  = 3.0;
N      = floor(T_sim / dt);
t      = (0:N-1)' * dt;

% ---- 场景1: 功率爬坡 (0-1s) ----
% I_rms 从 8A 爬到 30A, B-H效应 L_true 从 82 降到 76 μH
I_ramp = linspace(8, 30, floor(1.0/dt))';
L_ramp_true = 82.5 - 0.22 * I_ramp;    % L vs I 实测关系

% ---- 场景2: 稳态 + 实际扰动 (1-2s) ----
% I=18A 稳定, L 有小幅随机游走 + 偶尔的温度漂移
n2 = floor(1.0/dt);
L_steady_base = 80.0 * ones(n2, 1);
% 随机游走 (模拟负载微调)
rw = cumsum(randn(n2, 1) * 0.15);
% 温度漂移 (慢变)
temp_drift = 1.0 * sin(linspace(0, pi, n2))';
L_steady_true = L_steady_base + rw + temp_drift;
I_steady = 18 * ones(n2, 1);

% ---- 场景3: 阶跃 + 恢复 (2-3s) ----
% t=2.0s: 锅具移除, L +12μH
% t=2.5s: 锅具放回, L -12μH
I_final = 15 * ones(floor(1.0/dt), 1);
L_final_true = 79.5 * ones(floor(1.0/dt), 1);
L_final_true(1:floor(0.5/dt)) = 79.5;       % 正常
L_final_true(floor(0.5/dt)+1:end) = 91.5;   % 移锅 +12

% ---- 拼接 ----
I_seq = [I_ramp; I_steady; I_final];
L_true_all = [L_ramp_true; L_steady_true; L_final_true];
N = length(L_true_all);
t = (0:N-1)' * dt;

% 量测噪声 (I_rms 加权)
R0 = 2.0;
R = R0 ./ (I_seq / 10);
R = max(0.5, min(8.0, R));
noise = randn(N,1) .* sqrt(R);
L_meas = L_true_all + noise;

% ---- 加权 Kalman ----
Q = 0.10;  % 过程噪声 — 匹配 0.2s 量级的烹饪扰动
P_init = 1.0;

x_est = zeros(N,1); P_est = zeros(N,1);
innov = zeros(N,1); K_gain = zeros(N,1);
x = L_meas(1); P = P_init;

for k = 1:N
    x_pred = x;
    P_pred = P + Q;

    innov(k) = L_meas(k) - x_pred;

    K = P_pred / (P_pred + R(k));
    K_gain(k) = K;

    x = x_pred + K * innov(k);
    P = (1 - K) * P_pred;

    x_est(k) = x;
    P_est(k) = P;
end

% ---- EMA 对比 (I_rms 加权 alpha) ----
alpha_base = 0.15;
x_ema = zeros(N,1); x_ema(1) = L_meas(1);
for k = 2:N
    alpha = alpha_base * (I_seq(k) / 18);  % 高电流 → 高 alpha
    alpha = max(0.05, min(0.35, alpha));
    x_ema(k) = x_ema(k-1) + alpha * (L_meas(k) - x_ema(k-1));
end

% ---- 突变检测 ----
% 分段计算阈值 (避免阶跃段污染统计)
innov_std_run = zeros(N,1);
win = 50;  % 滑动窗
detect_thresh = 2.8 * ones(N,1);  % 默认阈值
for k = win:N
    win_data = innov(k-win+1:k);
    % 剔除离群点后算std
    win_q = prctile(abs(win_data), 75);
    win_clean = win_data(abs(win_data) < 3 * win_q);
    if length(win_clean) > 20
        innov_std_run(k) = std(win_clean);
        detect_thresh(k) = 3.5 * innov_std_run(k);
    end
end
detect_thresh = max(detect_thresh, 2.5);  % 最小阈值 2.5 μH

detect_flags = abs(innov) > detect_thresh;
detect_confirmed = zeros(N,1);
for k = 3:N
    if sum(detect_flags(k-2:k)) >= 2
        detect_confirmed(k) = 1;
    end
end

% ---- 统计 ----
err_raw = L_meas - L_true_all;
err_kal = x_est - L_true_all;
err_ema = x_ema - L_true_all;

fprintf('========== 多场景 Kalman ==========\n');
fprintf('全段: RMSE raw=%.2f  kal=%.2f  ema=%.2f μH\n', ...
    rms(err_raw), rms(err_kal), rms(err_ema));

% 爬坡段
seg1 = 1:floor(1.0/dt);
fprintf('爬坡段 (0-1s): RMSE kal=%.3f  ema=%.3f μH, 增益 kal=%.3f\n', ...
    rms(err_kal(seg1)), rms(err_ema(seg1)), mean(K_gain(seg1)));

% 稳态段
seg2_start = floor(1.0/dt)+1; seg2_end = floor(2.0/dt);
fprintf('稳态段 (1-2s): RMSE kal=%.3f  ema=%.3f μH, 增益 kal=%.3f\n', ...
    rms(err_kal(seg2_start:seg2_end)), rms(err_ema(seg2_start:seg2_end)), ...
    mean(K_gain(seg2_start:seg2_end)));

% 阶跃检测
jump1 = floor(2.5/dt);  % 移锅时间点
detect_after_jump = find(detect_confirmed(jump1:end) == 1, 1);
if ~isempty(detect_after_jump)
    fprintf('锅具移除检测延迟: %d ms\n', detect_after_jump * 10);
else
    fprintf('锅具移除检测: 未触发\n');
end

% ---- 绘图 ----
figure('Position',[30 80 1500 900]);

% 图1: L 估计全览
subplot(4,1,1);
hold on;
h1 = plot(t, L_meas, 'Color', [0.8 0.8 0.8], 'LineWidth',0.3);
h2 = plot(t, L_true_all, 'b-', 'LineWidth',1.2);
h3 = plot(t, x_est, 'r-', 'LineWidth',1.5);
h5 = plot(t, x_ema, 'm--', 'LineWidth',1.2);
% 标记场景
xline(1.0, 'k--', 'P稳定');
xline(2.0, 'k--');
xline(2.5, 'k-', '移锅', 'LineWidth',2);
ylabel('L (μH)');
legend([h1 h2 h3 h5], {'Measured','True L','Kalman','EMA'}, 'Location','northwest');
title('L 估计: 爬坡(0-1s) → 稳态+波动(1-2s) → 阶跃(2-3s)');
grid on;

% 图2: 创新序列 + 自适应阈值
subplot(4,1,2);
hold on;
plot(t, innov, 'b-', 'LineWidth',0.6);
plot(t, detect_thresh, 'r--', 'LineWidth',1.2);
plot(t, -detect_thresh, 'r--', 'LineWidth',1.2);
dt_idx = find(detect_confirmed);
if ~isempty(dt_idx)
    plot(t(dt_idx), innov(dt_idx), 'ro', 'MarkerSize',5, 'MarkerFaceColor','r');
end
ylabel('Innov (μH)');
title('创新序列 + 自适应检测阈值');
ylim([-20 20]);
grid on;

% 图3: Kalman 增益 + I_rms 权重
subplot(4,1,3);
yyaxis left;
plot(t, K_gain, 'b-', 'LineWidth',0.8);
ylabel('Kalman Gain');
ylim([0 0.5]);
yyaxis right;
plot(t, I_seq, 'Color', [0.4 0.4 0.4], 'LineWidth',0.6);
ylabel('I_{rms} (A)');
xlabel('Time (s)');
title('卡尔曼增益 (蓝) vs I_{rms} (灰)');
grid on;

% 图4: 爬坡段放大
subplot(4,1,4);
seg = 1:floor(1.0/dt);
hold on;
plot(t(seg), L_meas(seg), 'Color', [0.8 0.8 0.8], 'LineWidth',0.5);
plot(t(seg), L_true_all(seg), 'b-', 'LineWidth',1.5);
plot(t(seg), x_est(seg), 'r-', 'LineWidth',1.5);
plot(t(seg), x_ema(seg), 'm--', 'LineWidth',1.2);
ylabel('L (μH)'); xlabel('Time (s)');
title('放大: 功率爬坡段 (I: 8→30A, L: 82→76 μH)');
legend('Measured','True','Kalman','EMA','Location','best');
grid on;

% 保存图形
saveas(gcf, 'kalman_l_response.png');
fprintf('\n图形保存为 kalman_l_response.png\n');
end
