#!/usr/bin/env python3
"""
M4 EKF 实时/离线绘图工具
========================
提供两种模式:
  1. LivePlotter  — 实时滚动波形 (配合 MODBUS 监视)
  2. plot_csv()   — 离线 CSV 数据可视化

依赖: matplotlib, numpy
"""

import sys
from collections import deque

try:
    import matplotlib
    matplotlib.use('TkAgg')  # 跨平台兼容, 不依赖 Qt
    import matplotlib.pyplot as plt
    import matplotlib.animation as animation
    from matplotlib.ticker import MaxNLocator
    HAS_MPL = True
except ImportError:
    HAS_MPL = False

import numpy as np

# ============================================================
# 实时绘图器
# ============================================================

class LivePlotter:
    """实时滚动波形显示器

    用法:
        plotter = LivePlotter(window_s=30)
        plotter.open()

        while monitoring:
            data = client.read_telemetry()
            ekf  = client.read_ekf_telemetry()
            plotter.feed(power_w=data['power_w'], freq_hz=ekf['freq_hz'],
                         phase_deg=ekf['phase_deg'])
        plotter.close()
    """

    def __init__(self, window_s: float = 30, sample_interval_s: float = 0.1):
        if not HAS_MPL:
            raise ImportError("请安装 matplotlib: pip install matplotlib")

        self.window_s = window_s
        self.sample_interval_s = sample_interval_s
        self.buf_size = int(window_s / sample_interval_s) + 10

        # 环形缓冲
        self.t_buf  = deque(maxlen=self.buf_size)
        self.p_buf   = deque(maxlen=self.buf_size)  # 功率
        self.f_buf   = deque(maxlen=self.buf_size)  # 频率
        self.ph_buf  = deque(maxlen=self.buf_size)  # 相位
        self.xi_buf  = deque(maxlen=self.buf_size)  # ξ (归一化频率偏移)
        self.dp_buf  = deque(maxlen=self.buf_size)  # ΔPPG (PID增量)

        self.t0 = None
        self.fig = None
        self.axes = None
        self.lines = None
        self.ani = None
        self._running = False

    def open(self):
        """创建图形窗口并启动动画"""
        plt.ion()  # 交互模式
        self.fig, self.axes = plt.subplots(4, 1, figsize=(10, 8), sharex=True)

        (self.ax_p, self.ax_f, self.ax_ph, self.ax_dp) = self.axes

        # 子图1: 功率
        self.ax_p.set_ylabel('Power (W)')
        self.ax_p.set_ylim(-50, 2200)
        self.ax_p.grid(True, alpha=0.3)
        self.line_p, = self.ax_p.plot([], [], 'b-', linewidth=1.2, label='Power')
        self.ax_p.legend(loc='upper right', fontsize=8)

        # 子图2: 频率
        self.ax_f.set_ylabel('Freq (kHz)')
        self.ax_f.set_ylim(15, 60)
        self.ax_f.grid(True, alpha=0.3)
        self.line_f, = self.ax_f.plot([], [], 'r-', linewidth=1.2, label='Freq')
        self.ax_f.legend(loc='upper right', fontsize=8)

        # 子图3: 相位
        self.ax_ph.set_ylabel('Phase (deg)')
        self.ax_ph.set_ylim(-10, 100)
        self.ax_ph.grid(True, alpha=0.3)
        self.line_ph, = self.ax_ph.plot([], [], 'g-', linewidth=1.2, label='Phase')
        self.ax_ph.legend(loc='upper right', fontsize=8)

        # 子图4: PID 增量
        self.ax_dp.set_ylabel('Delta PPG')
        self.ax_dp.set_xlabel('Time (s)')
        self.ax_dp.grid(True, alpha=0.3)
        self.line_dp, = self.ax_dp.plot([], [], 'm-', linewidth=1.2, label='dPPG')
        self.ax_dp.legend(loc='upper right', fontsize=8)

        self.lines = [self.line_p, self.line_f, self.line_ph, self.line_dp]

        self.fig.canvas.manager.set_window_title('M4 EKF — Real-time Monitor')
        plt.tight_layout()
        plt.show(block=False)

        self._running = True

    def feed(self, power_w: float = 0, freq_hz: float = 0,
             phase_deg: float = 0, delta_ppg: float = 0):
        """喂入一帧数据"""
        if not self._running:
            return

        now = self._elapsed()
        self.t_buf.append(now)
        self.p_buf.append(power_w)
        self.f_buf.append(freq_hz / 1000.0 if freq_hz > 0 else 0)  # Hz→kHz
        self.ph_buf.append(phase_deg)
        self.dp_buf.append(delta_ppg)

    def update_plot(self):
        """刷新图形 (在主循环中调用)"""
        if not self._running or not self.t_buf:
            return

        t = list(self.t_buf)

        self.line_p.set_data(t, list(self.p_buf))
        self.line_f.set_data(t, list(self.f_buf))
        self.line_ph.set_data(t, list(self.ph_buf))
        self.line_dp.set_data(t, list(self.dp_buf))

        # 自适应 X 轴
        if len(t) >= 2:
            x_min = max(0, t[-1] - self.window_s)
            x_max = t[-1] + 1
            for ax in self.axes:
                ax.set_xlim(x_min, x_max)

        # 自适应 Y 轴
        if self.p_buf:
            pmax = max(self.p_buf) + 100 if self.p_buf else 2000
            self.ax_p.set_ylim(-50, max(pmax, 500))
        if self.f_buf:
            fvals = [v for v in self.f_buf if v > 0]
            if fvals:
                fmin, fmax = min(fvals), max(fvals)
                margin = max((fmax - fmin) * 0.2, 2)
                self.ax_f.set_ylim(fmin - margin, fmax + margin)
        if self.ph_buf:
            pvals = [v for v in self.ph_buf if abs(v) < 200]  # 过滤异常值
            if pvals:
                pmin, pmax = min(pvals), max(pvals)
                margin = max((pmax - pmin) * 0.3, 5)
                self.ax_ph.set_ylim(pmin - margin, pmax + margin)

        self.fig.canvas.draw_idle()
        self.fig.canvas.flush_events()

    def _elapsed(self) -> float:
        if self.t0 is None:
            self.t0 = __import__('time').time()
        return __import__('time').time() - self.t0

    def close(self):
        self._running = False
        if self.fig:
            plt.close(self.fig)


# ============================================================
# 离线 CSV 绘图
# ============================================================

def plot_csv(csv_path: str, window_s: float | None = None,
             save_png: str | None = None):
    """绘制 CSV 数据文件的时域波形

    用法:
        plot_csv("ekf_power_sweep.csv")
        plot_csv("ekf_power_sweep.csv", window_s=(120, 180))  # 只看120-180s
        plot_csv("ekf_power_sweep.csv", save_png="output.png")  # 保存图片
    """
    if not HAS_MPL:
        print("[错误] 请安装 matplotlib: pip install matplotlib")
        return

    import csv

    # 读取数据
    rows = []
    with open(csv_path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)

    if not rows:
        print(f"[错误] CSV 文件为空: {csv_path}")
        return

    print(f"[OK] 加载 {len(rows)} 条记录: {csv_path}")

    # 提取字段
    elapsed   = [float(r.get('elapsed_s', 0)) for r in rows]
    power_w   = [float(r.get('r_power_w', 0) or 0) for r in rows]
    power_set = [float(r.get('power_set_w', 0) or 0) for r in rows]

    # EKF 字段 (可能有也可能没有)
    def _safe_float(rows_list, key, default=0.0):
        vals = []
        for r in rows_list:
            try:
                v = r.get(key, '')
                vals.append(float(v) if v not in (None, '', 'nan') else default)
            except (ValueError, TypeError):
                vals.append(default)
        return vals

    freq_hz  = _safe_float(rows, 'e_freq_hz')
    phase_deg = _safe_float(rows, 'e_phase_deg')
    delta_ppg = _safe_float(rows, 'e_delta_ppg_signed')
    res_cur   = _safe_float(rows, 'e_res_cur_adc')

    # 按测试标签分段着色
    labels = [r.get('test_label', '') for r in rows]
    unique_labels = list(dict.fromkeys(labels))  # 保持顺序去重
    cmap = plt.cm.tab10

    # 时间窗口过滤
    if window_s is not None:
        w0, w1 = window_s
        indices = [i for i, t in enumerate(elapsed) if w0 <= t <= w1]
    else:
        indices = range(len(elapsed))

    def _filter(arr):
        return [arr[i] for i in indices]

    t_f   = _filter(elapsed)
    p_f   = _filter(power_w)
    ps_f  = _filter(power_set)
    f_f   = _filter(freq_hz)
    ph_f  = _filter(phase_deg)
    dp_f  = _filter(delta_ppg)
    rc_f  = _filter(res_cur)

    # 绘图
    has_ekf = any(v > 0 for v in f_f if v > 0)
    nrows = 5 if has_ekf else 2
    fig, axes = plt.subplots(nrows, 1, figsize=(12, 10), sharex=True)
    if nrows == 2:
        ax_p, ax_rc = axes
    else:
        ax_p, ax_f, ax_ph, ax_dp, ax_rc = axes

    # 功率子图
    ax_p.plot(t_f, p_f, 'b-', linewidth=1.0, alpha=0.8, label='Actual Power')
    ax_p.plot(t_f, ps_f, 'b--', linewidth=0.8, alpha=0.4, label='Set Power')
    # 按测试标签加背景色
    _draw_label_zones(ax_p, elapsed, labels, unique_labels, cmap)
    ax_p.set_ylabel('Power (W)')
    ax_p.legend(loc='upper right', fontsize=8)
    ax_p.grid(True, alpha=0.3)

    row_idx = 1
    if has_ekf:
        # 频率子图
        ax_f.plot(t_f, [v/1000 for v in f_f], 'r-', linewidth=1.0, alpha=0.8)
        _draw_label_zones(ax_f, elapsed, labels, unique_labels, cmap)
        ax_f.set_ylabel('Freq (kHz)')
        ax_f.grid(True, alpha=0.3)
        row_idx += 1

        # 相位于图
        ax_ph.plot(t_f, ph_f, 'g-', linewidth=1.0, alpha=0.8)
        _draw_label_zones(ax_ph, elapsed, labels, unique_labels, cmap)
        ax_ph.set_ylabel('Phase (deg)')
        ax_ph.grid(True, alpha=0.3)
        row_idx += 1

        # PID 增量子图
        ax_dp.plot(t_f, dp_f, 'm-', linewidth=1.0, alpha=0.8)
        _draw_label_zones(ax_dp, elapsed, labels, unique_labels, cmap)
        ax_dp.set_ylabel('Delta PPG')
        ax_dp.grid(True, alpha=0.3)
        row_idx += 1

    # 谐振电流 / 原始相位 AD 子图 (最后一行)
    ax_rc.plot(t_f, rc_f, 'c-', linewidth=1.0, alpha=0.8, label='Res Cur ADC')
    ax_rc.set_ylabel('Res Cur ADC')
    ax_rc.set_xlabel('Time (s)')
    ax_rc.grid(True, alpha=0.3)
    ax_rc.legend(loc='upper right', fontsize=8)

    fig.suptitle(f'CSV: {csv_path.split("/")[-1].split(chr(92))[-1]}', fontsize=10)
    plt.tight_layout()

    if save_png:
        fig.savefig(save_png, dpi=150, bbox_inches='tight')
        print(f"[OK] 图片已保存: {save_png}")

    plt.show(block=True)


def _draw_label_zones(ax, elapsed, labels, unique_labels, cmap):
    """按测试标签绘制背景色带"""
    if not unique_labels or len(unique_labels) <= 1:
        return

    # 找到每个标签的时间范围
    zones = []
    for label in unique_labels:
        starts = [i for i, l in enumerate(labels) if l == label]
        if starts:
            zones.append((elapsed[starts[0]], elapsed[starts[-1]], label))

    for i, (t0, t1, label) in enumerate(zones):
        color = cmap(i % 10)
        ax.axvspan(t0, t1, alpha=0.06, color=color)
        ax.text((t0 + t1) / 2, ax.get_ylim()[1] * 0.95,
                label, ha='center', va='top', fontsize=7,
                color=color, alpha=0.6)


# ============================================================
# 频谱分析
# ============================================================

def plot_spectrum(csv_path: str):
    """绘制功率-频率-相位散点图 (用于标定 P-f-φ 关系)"""
    if not HAS_MPL:
        print("[错误] 请安装 matplotlib: pip install matplotlib")
        return

    import csv

    rows = []
    with open(csv_path, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)

    if not rows:
        return

    power_w   = [float(r.get('r_power_w', 0) or 0) for r in rows]
    power_set = [float(r.get('power_set_w', 0) or 0) for r in rows]
    freq_hz   = [float(r.get('e_freq_hz', 0) or 0) for r in rows]
    phase_deg = [float(r.get('e_phase_deg', 0) or 0) for r in rows]

    # 每段稳态取平均值 (按 power_set 分组)
    from collections import defaultdict
    groups = defaultdict(list)
    for i, ps in enumerate(power_set):
        if ps > 0 and freq_hz[i] > 0:
            # 只取每段后半段 (稳定后)
            groups[ps].append((power_w[i], freq_hz[i], phase_deg[i]))

    if not groups:
        print("[提示] 无有效数据用于频谱分析")
        return

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))

    for ps, pts in sorted(groups.items()):
        # 取后半段
        n = len(pts)
        stable = pts[n//2:]
        if len(stable) < 5:
            continue
        p_avg = np.mean([s[0] for s in stable])
        f_avg = np.mean([s[1] for s in stable]) / 1000
        ph_avg = np.mean([s[2] for s in stable])

        ax1.plot(p_avg, f_avg, 'ro', markersize=8)
        ax1.annotate(f'{int(ps)}W', (p_avg, f_avg),
                     textcoords="offset points", xytext=(5, 5), fontsize=8)

        ax2.plot(p_avg, ph_avg, 'bo', markersize=8)
        ax2.annotate(f'{int(ps)}W', (p_avg, ph_avg),
                     textcoords="offset points", xytext=(5, 5), fontsize=8)

    ax1.set_xlabel('Actual Power (W)')
    ax1.set_ylabel('Frequency (kHz)')
    ax1.set_title('P-f Characteristic')
    ax1.grid(True, alpha=0.3)

    ax2.set_xlabel('Actual Power (W)')
    ax2.set_ylabel('Phase Angle (deg)')
    ax2.set_title('P-φ Characteristic')
    ax2.grid(True, alpha=0.3)

    fig.suptitle(f'Spectrum: {csv_path.split("/")[-1].split(chr(92))[-1]}')
    plt.tight_layout()
    plt.show(block=True)


# ============================================================
# 命令行入口 (离线模式)
# ============================================================

def main():
    """离线绘图命令行"""
    import argparse
    parser = argparse.ArgumentParser(description="M4 EKF 数据可视化")
    parser.add_argument("csv", help="CSV 数据文件路径")
    parser.add_argument("--window", type=float, nargs=2, metavar=('T0', 'T1'),
                        help="时间窗口 (秒)")
    parser.add_argument("--spectrum", action="store_true",
                        help="P-f-φ 频谱散点图")
    parser.add_argument("--save", type=str, metavar="PNG", help="保存为图片")

    args = parser.parse_args()

    if args.spectrum:
        plot_spectrum(args.csv)
    else:
        w = tuple(args.window) if args.window else None
        plot_csv(args.csv, window_s=w, save_png=args.save)

    return 0


if __name__ == "__main__":
    sys.exit(main())
