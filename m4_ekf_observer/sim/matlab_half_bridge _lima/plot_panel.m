function plot_panel(raw, timing, waveform, power, impedance, ac_cycle)
% plot_panel — 五面板可视化 (仅 Vdc + I 整流)
%
% 布局:
%   [1] I(t) 整流后电流 + 栅极时序
%   [2] Vdc + I_RMS 包络
%   [3] 占空比 + 死区 + 控制模式
%   [4] HRTIM CNT + CMP 四边沿
%   [5] 参数汇总

t = raw.t;
I = raw.I;
Vdc = raw.Vdc;

%% 创建图形窗口
fig = figure('Name', '半桥感应加热分析 (Vdc+I)', 'Position', [200, 200, 1200, 700]);
tiledlayout(3, 2, 'Padding', 'compact', 'TileSpacing', 'compact');

%% ——— 面板1: I(t) 整流后电流 + 栅极标注 ———
ax1 = nexttile;
plot(t*1e3, I, 'b-', 'LineWidth', 1.2); hold(ax1, 'on');
% 标注栅极开关时刻
n_label = min(10, length(timing.CMP_UON));
for k = 1:n_label
    xline(ax1, raw.t_us(k)*1e-3, '--g', 'HandleVisibility','off');
end
hold(ax1, 'off');
xlabel('时间 (ms)'); ylabel('电流 |I| (A)');
title(sprintf('整流后谐振电流  I_{peak}=%.2fA  I_{RMS}=%.2fA', ...
      waveform.I_peak_all, waveform.I_RMS_all));
grid on;

%% ——— 面板2: Vdc + 谷值标记 ———
ax2 = nexttile;
plot(t*1e3, Vdc, 'r-', 'LineWidth', 1); hold(ax2, 'on');
yline(power.Vdc_mean, '--k', sprintf('均值 %.1fV', power.Vdc_mean));
hold(ax2, 'off');
xlabel('时间 (ms)'); ylabel('Vdc (V)');
title(sprintf('直流母线 Vdc  (纹波 %.1fVp-p, %.1f%%)', ...
      2*power.Vdc_ripple_pk, power.Vdc_ripple_pct));
grid on;

%% ——— 面板3: 占空比 + 死区 + 控制模式标注 ———
ax3 = nexttile;
% 占空比
yyaxis left;
plot(t*1e3, timing.D_U_pct, 'g-', 'LineWidth', 1);
ylabel('占空比 (%)', 'Color', 'g');

yyaxis right;
% 死区(整段均值用虚线)
yline(mean(timing.DT1_us), '--m', sprintf('DT1=%.3fμs', mean(timing.DT1_us)));
yline(mean(timing.DT2_us), '--c', sprintf('DT2=%.3fμs', mean(timing.DT2_us)));
ylabel('死区 (μs)', 'Color', 'm');

xlabel('时间 (ms)');
title(sprintf('占空比 %s  f_{sw}=%.2fkHz', timing.ctrl_mode_str, timing.f_sw_kHz));
grid on;

% 控制模式文字在右上
dim = [0.65, 0.55, 0.25, 0.08];
if timing.ctrl_mode == 'F'
    mode_info = sprintf('FM调频 %.2f~%.2fkHz', timing.f_sw_min_kHz, timing.f_sw_max_kHz);
else
    mode_info = sprintf('PWM调占空比 D=%.1f%%~%.1f%%', min(timing.D_U_pct), max(timing.D_U_pct));
end
annotation('textbox', dim, 'String', mode_info, ...
    'FitBoxToText', 'on', 'BackgroundColor', 'w', 'FontSize', 9);

%% ——— 面板4: HRTIM CNT + CMP ———
ax4 = nexttile;
n_show = min(200, length(t));
idx4 = 1:n_show;

plot(idx4, raw.CNT(idx4), 'b-', 'LineWidth', 1.2); hold(ax4, 'on');
plot(idx4, raw.CMP(idx4, 1), 'r--', 'LineWidth', 0.8);  % CMP_UON
plot(idx4, raw.CMP(idx4, 2), 'g--', 'LineWidth', 0.8);  % CMP_UOFF
plot(idx4, raw.CMP(idx4, 3), 'c--', 'LineWidth', 0.8);  % CMP_LON
plot(idx4, raw.CMP(idx4, 4), 'm--', 'LineWidth', 0.8);  % CMP_LOFF
hold(ax4, 'off');
xlabel('采样点序号'); ylabel('HRTIM 计数值');
title(sprintf('HRTIM CNT+CMP  D_U=%.1f%%  DT1=%.3fμs', ...
      mean(timing.D_U_pct), mean(timing.DT1_us)));
legend({'CNT','UON','UOFF','LON','LOFF'}, 'Location','best','FontSize',7);
grid on;

%% ——— 面板5: 参数汇总 ———
ax5 = nexttile;
axis off;

param_str = {
    '═══════════════════════════════════'
    '  半桥IH参数 (Vdc+I)'
    '═══════════════════════════════════'
    ''
    sprintf('  控制: %s', timing.ctrl_mode_str)
    sprintf('  半桥: %s', timing.mode_str)
    sprintf('  f_sw=%.2f kHz  f_res=%.3f kHz', timing.f_sw_kHz, waveform.f_res_mean_khz)
    ''
    sprintf('  I_peak=%.2f A', waveform.I_peak_all)
    sprintf('  I_RMS=%.2f A  I_avg=%.2f A', waveform.I_RMS_all, waveform.I_avg_all)
    sprintf('  CF=%.2f  FF=%.2f', waveform.CF_I(1), waveform.FF_I(1))
    ''
    sprintf('  Vdc_mean=%.1f V', power.Vdc_mean)
    sprintf('  Vdc纹波=%.1f Vp-p (%.1f%%)', 2*power.Vdc_ripple_pk, power.Vdc_ripple_pct)
    ''
    sprintf('  P≈%.1f W (从Vdc+I估)', power.P)
    sprintf('  S≈%.1f VA  PF≈%.3f', power.S, power.PF)
    sprintf('  I_dc(均)≈%.2f A', power.Idc_est)
    ''
    sprintf('  R_eq≈%.3f Ω', power.R_eq)
};

if impedance.L_eq > 0
    param_str{end+1} = sprintf('  L_eq≈%.3f μH', impedance.L_eq * 1e6);
end
if isfinite(impedance.C_eq) && ~isinf(impedance.C_eq)
    param_str{end+1} = sprintf('  C_eq≈%.3f nF', impedance.C_eq * 1e9);
end
param_str{end+1} = sprintf('  Q≈%.2f', impedance.Q);

param_str{end+1} = '';
param_str{end+1} = '  ── 死区 & 时序 ──';
param_str{end+1} = sprintf('  DT1=%.3f μs', mean(timing.DT1_us));
param_str{end+1} = sprintf('  DT2=%.3f μs', mean(timing.DT2_us));
param_str{end+1} = sprintf('  D_U=%.1f%%  D_L=%.1f%%', mean(timing.D_U_pct), mean(timing.D_L_pct));

if timing.ctrl_mode == 'F'
    param_str{end+1} = sprintf('  调频范围: %.2f~%.2f kHz', timing.f_sw_min_kHz, timing.f_sw_max_kHz);
end

if ac_cycle.has_ac_data
    param_str{end+1} = '';
    param_str{end+1} = sprintf('  调制深度: %.2f', ac_cycle.mod_depth);
end

param_str{end+1} = '';
param_str{end+1} = '  ⚠ 功率/阻抗为估算值';
param_str{end+1} = '  (无谐振电压测量)';

text(0, 0.95, param_str, 'FontName', 'Consolas', 'FontSize', 10, ...
     'VerticalAlignment', 'top', 'Parent', ax5);
title(ax5, '参数汇总');

% 面板6: 保留给HRTIM CMP时序放大
ax6 = nexttile;
% 绘制一个开关周期内的栅极时序
T_us = timing.T_hrtim_us;
t_sw = linspace(0, T_us, 100);
% 栅极信号 (理想方波)
gate_U = (t_sw >= mean(timing.t_UON_us)) & (t_sw <= mean(timing.t_UOFF_us));
gate_L = (t_sw >= mean(timing.t_LON_us)) & (t_sw <= mean(timing.t_LOFF_us));
% 处理跨周期
if mean(timing.t_UON_us) > mean(timing.t_LOFF_us)
    gate_L = gate_L | (t_sw >= 0);
end

stairs(t_sw, gate_U*1.1, 'r-', 'LineWidth', 2); hold(ax6, 'on');
stairs(t_sw, gate_L*0.9, 'b-', 'LineWidth', 2);
% 标注死区
dt1_x = mean(timing.t_UOFF_us);
dt2_x = mean(timing.t_LOFF_us);
if dt2_x > T_us, dt2_x = dt2_x - T_us; end
yline(1.05, '--m', sprintf('DT1=%.3fμs', mean(timing.DT1_us)), 'LabelHorizontalAlignment','left');
hold(ax6, 'off');
xlabel('时间 (μs)'); ylabel('栅极信号');
title('一个开关周期栅极时序 (示意)');
ylim([0 1.3]);
legend('上管栅极','下管栅极','Location','best','FontSize',8);
grid on;

%% 保存
saveas(fig, 'IH_analysis_result.png');
fprintf('[plot_panel] 图像已保存: IH_analysis_result.png\n');

end
