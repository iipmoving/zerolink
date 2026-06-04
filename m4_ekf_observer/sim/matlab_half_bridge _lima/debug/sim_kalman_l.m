% sim_kalman_l — 加权Kalman滤波 + 突变检测仿真
% 用23-26kHz实测L统计量生成模拟序列, 验证噪声抑制和阶跃响应
function sim_kalman_l()
rng(42);

% ---- 参数 (来自实测23-26kHz频段) ----
L_true_mean = 79.8;     % 修正后 L 均值 (μH)
L_true_std  = 0.8;      % 残余 L 波动 (μH)
I_typical   = 17.9;     % 典型 I_rms (A)
I_range     = [8 30];   % I_rms 工作范围 (A)

% 仿真时间
dt     = 0.010;          % 每帧 10ms
T_sim  = 2.0;            % 仿真 2 秒
N      = floor(T_sim / dt);
t      = (0:N-1)' * dt;

% ---- 生成模拟数据 ----
% L 真值: 缓变 (小幅度随机游走 B-H残余)
L_true = L_true_mean + cumsum(randn(N,1) * 0.02);
% 在 t=0.8s 处注入阶跃: +12μH (模拟铁锅→空载)
jump_idx = floor(0.8 / dt);
L_true(jump_idx:end) = L_true(jump_idx:end) + 12;

% I_rms 序列: 随机波动 (模拟不同功率档位)
I_seq = I_typical + randn(N,1) * 5;
I_seq = max(I_range(1), min(I_range(2), I_seq));

% 量测噪声: 与 I_rms 成反比
R0 = 2.5;  % 基准量测噪声 (μH²) @ I=10A
R = R0 ./ (I_seq / 10);  % 电流越大噪声越小
noise = randn(N,1) .* sqrt(R);

% 观测值
L_meas = L_true + noise;

% ---- 加权卡尔曼滤波 ----
% 状态: x = L
% 预测: x_pred = x_est (L 无动态模型)
% 更新: x_est = x_pred + K * (z - x_pred)
% 卡尔曼增益: K = P_pred / (P_pred + R)
% 协方差更新: P_est = (1-K) * P_pred
% 协方差预测: P_pred = P_est + Q

Q = 0.005;               % 过程噪声 (允许缓慢漂移)
P_init = 1.0;            % 初始协方差

x_est = zeros(N,1);      % 状态估计
P_est = zeros(N,1);      % 估计协方差
innov = zeros(N,1);      % 创新序列
K_gain = zeros(N,1);     % 卡尔曼增益

x = L_meas(1);           % 初始状态
P = P_init;

for k = 1:N
    % 预测
    x_pred = x;
    P_pred = P + Q;

    % 创新
    innov(k) = L_meas(k) - x_pred;

    % 卡尔曼增益 (加权: R 来自 I_seq)
    K = P_pred / (P_pred + R(k));
    K_gain(k) = K;

    % 更新
    x = x_pred + K * innov(k);
    P = (1 - K) * P_pred;

    x_est(k) = x;
    P_est(k) = P;
end

% ---- 突变检测 ----
% 稳态创新标准差 (取前 100 帧计算)
n_steady = min(100, jump_idx - 1);
innov_std = std(innov(1:n_steady));
detect_thresh = 3 * innov_std;

% 检测标记
detect_flags = abs(innov) > detect_thresh;
% 连续2帧触发才算 (去抖)
detect_confirmed = zeros(N,1);
for k = 2:N
    if detect_flags(k) && detect_flags(k-1)
        detect_confirmed(k) = 1;
    end
end

% 首次检测时间
first_detect = find(detect_confirmed, 1);
if ~isempty(first_detect)
    detect_time_ms = (first_detect - jump_idx) * dt * 1000;
else
    detect_time_ms = NaN;
end

% ---- 不加权对比 (固定 R) ----
R_fixed = mean(R);
x_fixed = L_meas(1); P_f = P_init;
x_est_fixed = zeros(N,1);
for k = 1:N
    x_pred = x_fixed;
    P_pred = P_f + Q;
    K_f = P_pred / (P_pred + R_fixed);
    x_fixed = x_pred + K_f * (L_meas(k) - x_pred);
    P_f = (1 - K_f) * P_pred;
    x_est_fixed(k) = x_fixed;
end

% ---- 统计 ----
steady_mask = 1:(jump_idx - 1);
after_jump_mask = (jump_idx + 50):N;  % 跳过过渡期

err_raw   = L_meas - L_true;
err_kal   = x_est - L_true;
err_fixed = x_est_fixed - L_true;

fprintf('========== Kalman 仿真结果 ==========\n');
fprintf('量测噪声 std: %.2f μH (min=%.2f @I_max, max=%.2f @I_min)\n', ...
    sqrt(mean(R)), sqrt(min(R)), sqrt(max(R)));
fprintf('\n--- 稳态段 (阶跃前) ---\n');
fprintf('               RMSE   std\n');
fprintf('Raw measurement: %.3f  %.3f μH\n', rms(err_raw(steady_mask)), std(err_raw(steady_mask)));
fprintf('Weighted Kalman: %.3f  %.3f μH\n', rms(err_kal(steady_mask)), std(err_kal(steady_mask)));
fprintf('Fixed Kalman:    %.3f  %.3f μH\n', rms(err_fixed(steady_mask)), std(err_fixed(steady_mask)));
fprintf('Kalman gain: mean=%.3f, P10=%.3f, P90=%.3f\n', ...
    mean(K_gain(steady_mask)), prctile(K_gain(steady_mask),10), prctile(K_gain(steady_mask),90));

fprintf('\n--- 阶跃检测 ---\n');
fprintf('创新 std (稳态): %.3f μH\n', innov_std);
fprintf('检测阈值 (3σ): %.3f μH\n', detect_thresh);
fprintf('首次检测延迟: %.0f ms\n', detect_time_ms);
fprintf('阶跃后10帧内创新值: ');
for k = 0:9
    fprintf('%.1f ', innov(jump_idx + k));
end
fprintf('\n');

fprintf('\n--- 阶跃后稳态段 ---\n');
fprintf('Weighted Kalman RMSE: %.3f μH\n', rms(err_kal(after_jump_mask)));

% ---- 绘图 ----
figure('Position',[50 200 1400 750]);

subplot(3,1,1);
hold on;
h1 = plot(t, L_meas, 'Color', [0.7 0.7 0.7], 'LineWidth',0.5);
h2 = plot(t, L_true, 'b-', 'LineWidth',1.5);
h3 = plot(t, x_est, 'r-', 'LineWidth',1.5);
h4 = plot(t, x_est_fixed, 'g--', 'LineWidth',1);
xline(0.8, 'k--', 'Jump');
ylabel('L (μH)');
legend([h1 h2 h3 h4], {'Measured','True','Weighted KF','Fixed KF'}, 'Location','best');
title('L 估计: 加权Kalman vs 固定Kalman');
grid on;

subplot(3,1,2);
hold on;
plot(t, innov, 'b-', 'LineWidth',0.8);
yline(detect_thresh, 'r--', '+3σ');
yline(-detect_thresh, 'r--', '-3σ');
% 标记检测点
dt_idx = find(detect_confirmed);
if ~isempty(dt_idx)
    plot(t(dt_idx), innov(dt_idx), 'ro', 'MarkerSize',6, 'MarkerFaceColor','r');
end
ylabel('Innovation (μH)');
title(sprintf('创新序列 + 突变检测 (3σ=%.2f μH, 检测延迟=%.0f ms)', detect_thresh, detect_time_ms));
grid on;

subplot(3,1,3);
yyaxis left;
plot(t, K_gain, 'b-', 'LineWidth',0.8); ylabel('Kalman Gain');
yyaxis right;
plot(t, I_seq, 'Color', [0.5 0.5 0.5], 'LineWidth',0.5); ylabel('I_{rms} (A)');
xlabel('Time (s)');
title('卡尔曼增益 vs I_{rms} (权重)');
grid on;

end
