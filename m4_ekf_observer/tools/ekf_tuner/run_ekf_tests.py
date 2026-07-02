#!/usr/bin/env python3
"""
M4 半桥电磁炉 EKF 数据采集 — 自动化测试序列
==============================================

按预设工况自动执行: 设定功率 → 等待稳定 → 记录数据 → 切换工况
所有数据保存为 CSV, 供 EKF 离线参数整定。

用法:
  python run_ekf_tests.py COM3 --test power_sweep    # 功率扫描
  python run_ekf_tests.py COM3 --test pot_material    # 锅具材质对比
  python run_ekf_tests.py COM3 --test pot_lift        # 移锅检测
  python run_ekf_tests.py COM3 --test all             # 全部测试序列
"""

import sys
import time
import csv
import argparse
from datetime import datetime
from m4_modbus_tool import M4ModbusClient, READ_REGS, EKF_REGS, EKF_START_ADDR, EKF_COUNT

try:
    from plot_utils import LivePlotter, plot_csv, plot_spectrum
    HAS_PLOT = True
except ImportError:
    HAS_PLOT = False

# ============================================================
# 测试工况定义
# ============================================================

# --- 工况1: 功率扫描 (固定锅具, 扫功率等级) ---
# 目的: 获取 P-f-φ-ξ 关系, 标定 P_max 和 K_f
POWER_SWEEP = [
    # (功率W, 保持秒, 标签)
    (500,  30, "500W"),
    (800,  30, "800W"),
    (1000, 30, "1000W"),
    (1200, 30, "1200W"),
    (1500, 30, "1500W"),
    (1800, 30, "1800W"),
    (2000, 30, "2000W"),
    (1000, 10, "cool_1000W"),  # 回落参考点
    (0,     5, "off"),
]

# --- 工况2: 锅具材质对比 (同尺寸, 不同材质) ---
# 目的: 观察钢锅 Curie 拐点
POT_MATERIAL = [
    (1500, 120, "material_test"),  # 2分钟, 足够过 Curie 点
    (0,     10, "off"),
]

# --- 工况3: 锅具大小对比 (同材质, 不同尺寸) ---
# 目的: 建立 ξ_steady 与锅径的映射
POT_SIZE = [
    (1500, 60, "size_test"),
    (0,    10, "off"),
]

# --- 工况4: 移锅/颠锅检测 ---
# 目的: 标定 dξ/dt 移锅阈值
# 注意: 此工况需要人工配合, 脚本只持续记录, 人在中途抬锅
POT_LIFT = [
    (800, 60, "lift_test"),   # 60秒内人工抬锅2-3次
    (0,    5, "off"),
]

# --- 工况5: 频率步进响应 ---
# 目的: 观察小功率变化时的 Δf→ΔP→Δφ 动态
FREQ_STEP = [
    (500,  20, "step_500W"),
    (600,  20, "step_600W"),
    (700,  20, "step_700W"),
    (800,  20, "step_800W"),
    (900,  20, "step_900W"),
    (1000, 20, "step_1000W"),
    (0,     5, "off"),
]

ALL_TESTS = {
    "power_sweep":  ("功率扫描", POWER_SWEEP, "power_sweep"),
    "pot_material": ("锅具材质", POT_MATERIAL, "pot_material"),
    "pot_size":     ("锅具大小", POT_SIZE,     "pot_size"),
    "pot_lift":     ("移锅检测", POT_LIFT,     "pot_lift"),
    "freq_step":    ("频率步进", FREQ_STEP,    "freq_step"),
}


# ============================================================
# 数据采集核心
# ============================================================

class TestRunner:
    """自动化测试执行器"""

    def __init__(self, client: M4ModbusClient, csv_path: str,
                 sample_interval: float = 0.1, plotter=None):
        self.client = client
        self.csv_path = csv_path
        self.sample_interval = sample_interval
        self.plotter = plotter
        self.csv_file = None
        self.csv_writer = None
        self.total_samples = 0
        self._plot_counter = 0

    def _open_csv(self):
        fieldnames = (
            ["timestamp", "elapsed_s", "test_label", "power_set_w"]
            + [f"r_{name}" for _addr, (name, _u, _s, _d) in READ_REGS.items()]
            + [f"e_{name}" for _addr, (name, _u, _s, _d) in EKF_REGS.items()]
            + ["e_freq_hz", "e_phase_deg", "e_delta_ppg_signed"]
        )
        self.csv_file = open(self.csv_path, 'w', newline='', encoding='utf-8')
        self.csv_writer = csv.DictWriter(self.csv_file, fieldnames=fieldnames)
        self.csv_writer.writeheader()

    def _read_all(self):
        """读取全部数据 (标准遥测 + EKF遥测)"""
        row = {"timestamp": datetime.now().isoformat(timespec='milliseconds')}

        # 标准遥测
        raw = self.client.read_registers(0x1000, len(READ_REGS))
        if raw:
            for i, (_addr, (name, _u, _s, _d)) in enumerate(READ_REGS.items()):
                row[f"r_{name}"] = raw[i]

        # EKF 遥测
        ekf = self.client.read_ekf_telemetry()
        if ekf:
            for _addr, (name, _u, _s, _d) in EKF_REGS.items():
                row[f"e_{name}"] = ekf.get(name, 0)
            row["e_freq_hz"] = ekf.get("freq_hz", 0)
            row["e_phase_deg"] = ekf.get("phase_deg", 0)
            row["e_delta_ppg_signed"] = ekf.get("delta_ppg_signed", 0)

        return row

    def run(self, test_name: str, sequences: list):
        print(f"\n{'='*60}")
        print(f"  测试: {ALL_TESTS[test_name][0]}")
        print(f"  CSV:  {self.csv_path}")
        print(f"{'='*60}")

        self._open_csv()
        t_start = time.time()

        for power_w, hold_s, label in sequences:
            print(f"\n--- [{label}] 设定 {power_w}W, 保持 {hold_s}s ---")

            # 设定功率
            if power_w > 0:
                self.client.set_power(power_w)
                time.sleep(0.2)
                self.client.set_work_sta(True)
                # 启动心跳, 防止测试期间通讯超时自动保护
                self.client.start_heartbeat(power_watt=power_w)
            else:
                self.client.stop_heartbeat()
                self.client.set_work_sta(False)
                time.sleep(0.2)
                self.client.set_power(0)

            # 数据采集
            t_seg_start = time.time()
            while time.time() - t_seg_start < hold_s:
                row = self._read_all()
                row["elapsed_s"] = round(time.time() - t_start, 2)
                row["test_label"] = label
                row["power_set_w"] = power_w
                self.csv_writer.writerow(row)
                self.total_samples += 1

                # 进度显示
                remaining = hold_s - (time.time() - t_seg_start)
                r_power = row.get("r_power_w", 0)
                e_freq = row.get("e_freq_hz", 0)
                e_phase = row.get("e_phase_deg", 0)
                e_delta = row.get("e_delta_ppg_signed", 0)
                sys.stdout.write(
                    f"\r  [{label}] P={r_power}W f={e_freq}Hz "
                    f"φ={e_phase:.1f}° | 剩余 {remaining:.0f}s  "
                )
                sys.stdout.flush()

                # 实时绘图 (每5个采样点更新一次, 降低开销)
                if self.plotter:
                    self.plotter.feed(power_w=r_power, freq_hz=e_freq,
                                     phase_deg=e_phase, delta_ppg=e_delta)
                    self._plot_counter += 1
                    if self._plot_counter % 5 == 0:
                        self.plotter.update_plot()

                time.sleep(self.sample_interval)

            print()  # 换行

        # 关机
        self.client.stop_heartbeat()
        self.client.set_work_sta(False)
        self.client.set_power(0)

        self.csv_file.close()
        t_total = time.time() - t_start
        print(f"\n[完成] {self.total_samples} 条记录, 耗时 {t_total:.0f}s")
        print(f"[文件] {self.csv_path}")


# ============================================================
# 命令行入口
# ============================================================

def main():
    parser = argparse.ArgumentParser(
        description="M4 EKF 数据采集 — 自动化测试序列"
    )
    parser.add_argument("port", nargs='?', default=None,
                        help="串口 (例: COM3). 离线模式 (--plot-csv) 可省略")
    parser.add_argument("-b", "--baudrate", type=int, default=115200)
    parser.add_argument("-s", "--slave", type=int, default=5)
    parser.add_argument("--test", type=str, default="all",
                        choices=["all", "power_sweep", "pot_material",
                                 "pot_size", "pot_lift", "freq_step"],
                        help="测试类型 (默认: all)")
    parser.add_argument("--interval", type=float, default=0.1,
                        help="采样间隔秒 (默认: 0.1 = 100ms)")
    parser.add_argument("--csv", type=str, default=None,
                        help="CSV 输出路径 (默认: 自动生成)")
    parser.add_argument("--plot", action="store_true",
                        help="测试时开启实时波形图 (需 matplotlib)")
    parser.add_argument("--plot-window", type=float, default=30, metavar="SEC",
                        help="波形图时间窗口秒数 (默认: 30)")
    parser.add_argument("--plot-csv", type=str, default=None, metavar="FILE",
                        help="离线绘制 CSV 数据文件 (不连接硬件)")
    parser.add_argument("--spectrum", action="store_true",
                        help="与 --plot-csv 配合, 绘制 P-f-φ 散点图")

    args = parser.parse_args()

    # --- 离线模式: --plot-csv ---
    if args.plot_csv:
        if not HAS_PLOT:
            print("[错误] 请安装 matplotlib: pip install matplotlib")
            return 1
        if args.spectrum:
            plot_spectrum(args.plot_csv)
        else:
            plot_csv(args.plot_csv)
        return 0

    # --- 在线模式: 需要串口 ---
    if not args.port:
        print("[错误] 在线模式需要指定串口, 或使用 --plot-csv 离线绘图")
        return 1

    client = M4ModbusClient(args.port, args.baudrate, args.slave)
    if not client.connect():
        return 1

    # 协议初始化检查
    client.check_and_init()

    # 实时绘图器
    plotter = None
    if args.plot:
        if not HAS_PLOT:
            print("[警告] matplotlib 未安装, 无法绘图。 pip install matplotlib")
        else:
            plotter = LivePlotter(window_s=args.plot_window,
                                 sample_interval_s=args.interval)
            plotter.open()
            print(f"[绘图] 波形窗口已开启 (时间窗={args.plot_window}s)")

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

    try:
        if args.test == "all":
            tests_to_run = ["power_sweep", "pot_material", "pot_size",
                          "pot_lift", "freq_step"]
        else:
            tests_to_run = [args.test]

        all_csv_paths = []

        for i, test_name in enumerate(tests_to_run):
            name_cn, sequences, tag = ALL_TESTS[test_name]

            # 每个测试之间停顿, 提示操作
            if i > 0:
                print(f"\n>>> 准备下一项测试: {name_cn}")
                print(f">>> 请确认硬件就绪后按 Enter...")
                input()

            csv_path = args.csv or f"ekf_{tag}_{timestamp}.csv"
            if len(tests_to_run) > 1 and not args.csv:
                # 多项测试时自动加后缀
                base = f"ekf_{tag}_{timestamp}"
                csv_path = f"{base}_{i+1:02d}.csv"
            all_csv_paths.append(csv_path)

            runner = TestRunner(client, csv_path, args.interval, plotter)
            runner.run(test_name, sequences)

        # 测试完成, 提示可离线绘图
        if all_csv_paths:
            print(f"\n[提示] 离线查看波形:")
            for p in all_csv_paths:
                print(f"  python run_ekf_tests.py --plot-csv {p}")

    except KeyboardInterrupt:
        print("\n[中断] 正在安全关机...")
        client.stop_heartbeat()
        client.set_work_sta(False)
        client.set_power(0)
    finally:
        if plotter:
            plotter.close()
        client.disconnect()

    return 0


if __name__ == "__main__":
    sys.exit(main())
