#!/usr/bin/env python3
"""
M4 半桥电磁炉 — MODBUS 调试界面
================================
基于 tkinter 的图形化调试工具，无需额外依赖。

用法:
  python m4_gui.py                      # 启动GUI
  python m4_gui.py --port COM3          # 启动并自动连接

依赖: pymodbus, (可选 matplotlib for --plot)
"""

import sys
import os
import csv
import json
import time
import tkinter as tk
from tkinter import ttk, messagebox, filedialog
from datetime import datetime
from collections import deque, OrderedDict

# 确保能找到同目录模块
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from m4_modbus_tool import (
    M4ModbusClient, DataLogger, READ_REGS, EKF_REGS,
    READ_START_ADDR, READ_COUNT, EKF_START_ADDR, EKF_COUNT
)

try:
    from plot_utils import LivePlotter
    HAS_PLOT = True
except ImportError:
    HAS_PLOT = False

# ============================================================
# 初始化 / 控制寄存器 (0x2000-0x2014)
# 与 M4 固件 IH_STA_READ_WRITE 结构体对齐
# ============================================================

INIT_REGS = OrderedDict([
    # (addr, name,         label_cn,           category)
    (0x2000, ("check_pan_lv",    "检锅强度",        "init")),
    (0x2001, ("ppg_max",         "最大PPG限制",     "init")),
    (0x2002, ("pan_power",       "移锅功率",        "init")),
    (0x2003, ("hvol_limited",    "反压限制",        "init")),
    (0x2004, ("load_current",    "负载有效电流",    "init")),
    (0x2005, ("cur_calibration", "电流修正系数",    "init")),
    (0x2006, ("power_mix",       "最小连续功率",    "init")),
    (0x2007, ("power_max",       "最大连续功率",    "init")),
    (0x2008, ("wrong_pan",       "恶略锅具保护功率","init")),
    (0x2009, ("syntony_current", "谐振电流保护值",  "init")),
    (0x200A, ("phase_pan",       "移锅相位",        "init")),
    (0x200B, ("phase_mix",       "最小相位",        "init")),
    (0x200C, ("steel_cal",       "钢铁锅修正",      "init")),
    (0x200D, ("n_pan_syntony_c", "移锅谐振电流限制","init")),
    (0x200E, ("work_sta",        "工作状态",        "ctrl")),
    (0x200F, ("fan_speed",       "风扇转速",        "ctrl")),
    (0x2010, ("target_power",    "目标功率(×25W)",  "ctrl")),
    (0x2011, ("intermittent",    "间断加热",        "ctrl")),
    (0x2012, ("jitter_freq",     "抖频参数",        "ctrl")),
    (0x2013, ("buzz_cof",        "蜂鸣器控制",      "ctrl")),
    (0x2014, ("syntony_short",   "短路保护谐振电流","init")),
])

INIT_START_ADDR = 0x2000
INIT_COUNT = len(INIT_REGS)  # 21 registers

# ============================================================
# 主题颜色
# ============================================================

BG       = "#1e1e2e"
BG2      = "#2a2a3a"
FG       = "#cdd6f4"
ACCENT   = "#89b4fa"
GREEN    = "#a6e3a1"
RED      = "#f38ba8"
YELLOW   = "#f9e2af"
DIM      = "#6c7086"
BORDER   = "#45475a"

# ============================================================
# 串口自动检测
# ============================================================

def scan_serial_ports():
    """扫描可用串口，返回 [(device, label), ...]

    USB转串口设备会显示芯片型号 (CH340/CP2102/FT232等)，
    便于在多个串口中识别目标。
    """
    result = []
    try:
        import serial.tools.list_ports
        for p in serial.tools.list_ports.comports():
            desc = p.description or ""
            # 去掉冗余前缀
            for prefix in ["USB Serial Port", "USB-SERIAL", "USB Serial Device",
                           "Communications Port", "串行端口", "USB Serial",
                           "USB 串行设备"]:
                if desc.lower().startswith(prefix.lower()):
                    desc = desc[len(prefix):].strip()
                    break
            # 补充硬件ID (VID:PID)
            if p.hwid and "VID" in p.hwid.upper():
                vid_pid = _extract_vid_pid(p.hwid)
                if vid_pid and vid_pid not in desc:
                    desc = f"{desc} ({vid_pid})" if desc else vid_pid

            label = f"{p.device}" if not desc else f"{p.device}  -  {desc}"
            result.append((p.device, label))
    except ImportError:
        pass

    # 回退: 尝试 Windows 注册表
    if not result:
        result = _scan_windows_registry()
    # 最后回退: 暴力探测
    if not result:
        result = _scan_by_probe()

    # 去重 + 过滤非物理串口
    seen = set()
    filtered = []
    for dev, label in result:
        if dev in seen:
            continue
        # 跳过明显不是 USB 串口的蓝牙设备
        if "bthmodem" in label.lower():
            continue
        seen.add(dev)
        filtered.append((dev, label))

    return filtered


def _extract_vid_pid(hwid: str) -> str:
    """从硬件ID字符串提取 VID:PID"""
    import re
    m = re.search(r'VID[:_]?([0-9A-Fa-f]{4}).*PID[:_]?([0-9A-Fa-f]{4})', hwid)
    if m:
        return f"USB\\{m.group(1)}:{m.group(2)}"
    return ""


def _scan_windows_registry():
    """从 Windows 注册表读取串口列表 (无需 serial 库)"""
    result = []
    seen = set()
    try:
        import winreg
        with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE,
                           r"HARDWARE\DEVICEMAP\SERIALCOMM") as key:
            i = 0
            while True:
                try:
                    name, value, _ = winreg.EnumValue(key, i)
                    if value in seen:
                        i += 1
                        continue
                    seen.add(value)
                    # 过滤蓝牙设备
                    if "bthmodem" in name.lower():
                        i += 1
                        continue
                    # 简化设备名
                    short = name.split("\\")[-1] if "\\" in name else name
                    label = f"{value}  -  {short}"
                    result.append((value, label))
                    i += 1
                except OSError:
                    break
    except (ImportError, OSError):
        pass
    return result


def _scan_by_probe():
    """暴力探测: 尝试打开 COM1-COM16"""
    result = []
    import serial
    for i in range(1, 17):
        name = f"COM{i}"
        try:
            s = serial.Serial(name)
            s.close()
            result.append((name, name))
        except (OSError, serial.SerialException):
            pass
    return result


# ============================================================
# 数值显示组件
# ============================================================

class RegisterRow(ttk.Frame):
    """单行寄存器显示: 地址 | 名称 | 原始值 | 换算值 | 单位"""

    def __init__(self, parent, addr, name, unit, scale, desc):
        super().__init__(parent)
        self.name = name
        self.scale = scale
        self.unit = unit

        self.configure(style="Row.TFrame")

        # 地址
        addr_lbl = tk.Label(self, text=f"0x{addr:04X}", width=6,
                           bg=BG, fg=DIM, font=("Consolas", 9), anchor="w")
        addr_lbl.pack(side=tk.LEFT, padx=(0, 4))

        # 名称
        name_lbl = tk.Label(self, text=name, width=14,
                           bg=BG, fg=ACCENT, font=("Consolas", 9), anchor="w")
        name_lbl.pack(side=tk.LEFT, padx=(0, 4))

        # 原始值
        self.raw_var = tk.StringVar(value="--")
        raw_lbl = tk.Label(self, textvariable=self.raw_var, width=5,
                          bg=BG, fg=FG, font=("Consolas", 9, "bold"), anchor="e")
        raw_lbl.pack(side=tk.LEFT, padx=(0, 4))

        # 换算值
        self.val_var = tk.StringVar(value="--")
        val_lbl = tk.Label(self, textvariable=self.val_var, width=10,
                          bg=BG, fg=GREEN, font=("Consolas", 9), anchor="e")
        val_lbl.pack(side=tk.LEFT, padx=(0, 2))

        # 单位
        unit_lbl = tk.Label(self, text=unit, width=4,
                           bg=BG, fg=DIM, font=("Consolas", 9), anchor="w")
        unit_lbl.pack(side=tk.LEFT)

    def update(self, raw_val, disp_val):
        self.raw_var.set(str(raw_val))
        if isinstance(disp_val, float):
            # 智能小数位
            if abs(disp_val) < 10:
                self.val_var.set(f"{disp_val:.2f}")
            elif abs(disp_val) < 1000:
                self.val_var.set(f"{disp_val:.1f}")
            else:
                self.val_var.set(f"{disp_val:.0f}")
        else:
            self.val_var.set(str(disp_val))


# ============================================================
# 主应用
# ============================================================

class M4DebugApp:
    """M4 MODBUS 调试主窗口"""

    def __init__(self, root: tk.Tk, auto_port: str = None):
        self.root = root
        self.root.title("M4 半桥电磁炉 — MODBUS 调试工具")
        self.root.geometry("960x780")
        self.root.configure(bg=BG)
        self.root.minsize(800, 600)

        self.client: M4ModbusClient | None = None
        self.plotter: LivePlotter | None = None
        self.monitoring = False
        self.monitor_job = None
        self._sample_interval_ms = 50   # 默认 20Hz 刷新

        # 遥测选择
        self._mon_std = tk.BooleanVar(value=False)  # 标准遥测 (默认关, 节省带宽)
        self._mon_ekf = tk.BooleanVar(value=True)   # EKF 遥测

        # 数据记录
        self._recording = False
        self._records = []   # 内存记录, 停止时批量写 CSV
        self._csv_fields = ["timestamp", "freq_hz", "phase_deg",
                           "delta_ppg_signed", "res_cur_adc",
                           "power_actual", "power_target"]

        # WaveCapture 波形批量回读
        self._mon_wave = tk.BooleanVar(value=False)
        self._wave_interval_ms = 2000
        self._wave_job = None
        self._wave_csv = None
        self._wave_file = None
        self._wave_batch_id = 0
        self._wave_csv_path = ""
        self._wave_csv_fields = [
            "batch_id", "frame_idx", "sample_idx", "value",
            "para0", "para1", "para2", "para3", "para4"
        ]
        self._mon_wave.trace_add('write', self._on_wave_toggle)

        # RawCapture 原始9列数据采集
        self._mon_raw = tk.BooleanVar(value=False)
        self._raw_interval_ms = 2000
        self._raw_job = None
        self._raw_csv = None
        self._raw_file = None
        self._raw_batch_id = 0
        self._raw_csv_path = ""
        self._raw_csv_fields = [
            "t_us", "I_adc", "V_adc", "Vdc_adc",
            "CNT", "CMP_UON", "CMP_UOFF", "CMP_LON", "CMP_LOFF", "POWER"
        ]
        self._mon_raw.trace_add('write', self._on_raw_toggle)

        # 遥测行组件引用
        self.reg_rows = {}      # name → RegisterRow (标准)
        self.ekf_rows = {}      # name → RegisterRow (EKF)
        self._ekf_computed = {} # 解析值标签

        # 初始化寄存器面板
        self._init_entries = {}  # addr → {"entry": tk.Entry, "var": StringVar, "name": str}
        self._hex_mode = tk.BooleanVar(value=False)  # Hex/Dec 切换
        self._init_config = {}   # addr → int (loaded from JSON)

        self._build_ui()

        # 启动时自动扫描串口
        self.root.after(100, self._scan_ports)

        # 自动连接
        if auto_port:
            self.port_var.set(auto_port)
            self.root.after(500, self._connect)

    # ---- 构建UI ------------------------------------------------

    def _build_ui(self):
        # 顶部连接栏
        self._build_toolbar()

        # 主内容区 (左右两栏 + 底部控制)
        main = tk.Frame(self.root, bg=BG)
        main.pack(fill=tk.BOTH, expand=True, padx=8, pady=(4, 8))

        # 上半: 左右两栏寄存器 (固定高度, 不挤占底部控制栏)
        regs_frame = tk.Frame(main, bg=BG, height=260)
        regs_frame.pack(fill=tk.X, expand=False)
        regs_frame.pack_propagate(False)

        left = self._build_reg_panel(regs_frame, "标准遥测 0x1000-0x1014",
                                     READ_REGS, "reg", tk.LEFT)
        right = self._build_reg_panel(regs_frame, "EKF 遥测 0x1020-0x1029",
                                      EKF_REGS, "ekf", tk.RIGHT)

        # 初始化寄存器面板 (可编辑)
        self._build_init_panel(main)

        # 底部控制栏
        self._build_control_bar(main)

        # 状态栏
        self._build_statusbar()

    def _build_toolbar(self):
        bar = tk.Frame(self.root, bg=BG2, height=44)
        bar.pack(fill=tk.X, padx=8, pady=(8, 0))
        bar.pack_propagate(False)

        # 标题
        title = tk.Label(bar, text="M4 MODBUS", bg=BG2, fg=ACCENT,
                        font=("Consolas", 12, "bold"))
        title.pack(side=tk.LEFT, padx=(10, 20))

        # 串口 (Combobox + 扫描按钮)
        tk.Label(bar, text="Port", bg=BG2, fg=DIM,
                font=("Consolas", 8)).pack(side=tk.LEFT)
        self.port_var = tk.StringVar(value="")
        self._port_map = {}  # label → device
        self.port_cb = ttk.Combobox(bar, textvariable=self.port_var,
                                    width=22, font=("Consolas", 9))
        self.port_cb.pack(side=tk.LEFT, padx=(2, 2))

        scan_btn = tk.Button(bar, text="🔍", command=self._scan_ports,
                            bg=BORDER, fg=FG, font=("Consolas", 8),
                            activebackground=BORDER, relief=tk.FLAT,
                            cursor="hand2", width=3)
        scan_btn.pack(side=tk.LEFT, padx=(0, 8))

        # 波特率
        tk.Label(bar, text="Baud", bg=BG2, fg=DIM,
                font=("Consolas", 8)).pack(side=tk.LEFT)
        self.baud_var = tk.StringVar(value="115200")
        baud_cb = ttk.Combobox(bar, textvariable=self.baud_var,
                               values=["9600", "19200", "38400", "57600", "115200"],
                               width=6, font=("Consolas", 9))
        baud_cb.pack(side=tk.LEFT, padx=(2, 8))

        # 从站
        tk.Label(bar, text="Slave", bg=BG2, fg=DIM,
                font=("Consolas", 8)).pack(side=tk.LEFT)
        self.slave_var = tk.StringVar(value="5")
        slave_entry = tk.Entry(bar, textvariable=self.slave_var, width=3,
                              bg=BG, fg=FG, font=("Consolas", 9),
                              insertbackground=FG)
        slave_entry.pack(side=tk.LEFT, padx=(2, 12))

        # 连接按钮
        self.connect_btn = tk.Button(bar, text="● 连接", command=self._connect,
                                     bg=BORDER, fg=GREEN, font=("Consolas", 9, "bold"),
                                     activebackground=BORDER, activeforeground=GREEN,
                                     relief=tk.FLAT, cursor="hand2", width=8)
        self.connect_btn.pack(side=tk.LEFT, padx=(0, 8))

        # 连接状态灯
        self.conn_led = tk.Canvas(bar, width=14, height=14, bg=BG2, highlightthickness=0)
        self.conn_led.pack(side=tk.LEFT)
        self._led_circle = self.conn_led.create_oval(2, 2, 12, 12, fill=DIM, outline="")

        # 右侧: 监视和绘图按钮
        self.plot_btn = tk.Button(bar, text="▣ 波形", command=self._toggle_plot,
                                  bg=BORDER, fg=FG, font=("Consolas", 9),
                                  activebackground=BORDER, relief=tk.FLAT,
                                  cursor="hand2", state=tk.DISABLED, width=7)
        self.plot_btn.pack(side=tk.RIGHT, padx=(4, 0))

        self.mon_btn = tk.Button(bar, text="▶ 监视", command=self._toggle_monitor,
                                 bg=BORDER, fg=FG, font=("Consolas", 9),
                                 activebackground=BORDER, relief=tk.FLAT,
                                 cursor="hand2", state=tk.DISABLED, width=7)
        self.mon_btn.pack(side=tk.RIGHT, padx=(4, 0))

        self.refresh_btn = tk.Button(bar, text="↻ 刷新", command=self._read_once,
                                     bg=BORDER, fg=FG, font=("Consolas", 9),
                                     activebackground=BORDER, relief=tk.FLAT,
                                     cursor="hand2", state=tk.DISABLED, width=7)
        self.refresh_btn.pack(side=tk.RIGHT, padx=(4, 0))

        self.rec_btn = tk.Button(bar, text="● 记录", command=self._toggle_record,
                                  bg=BORDER, fg=FG, font=("Consolas", 9),
                                  activebackground=BORDER, relief=tk.FLAT,
                                  cursor="hand2", state=tk.DISABLED, width=7)
        self.rec_btn.pack(side=tk.RIGHT, padx=(4, 0))

        # 遥测选择 checkboxes
        sep2 = tk.Frame(bar, width=2, bg=BORDER)
        sep2.pack(side=tk.RIGHT, padx=6, fill=tk.Y)
        cb1 = tk.Checkbutton(bar, text="标准", variable=self._mon_std,
                            bg=BG2, fg=FG, font=("Consolas", 8),
                            selectcolor=BG, activebackground=BG2,
                            activeforeground=FG)
        cb1.pack(side=tk.RIGHT, padx=(0, 2))
        cb2 = tk.Checkbutton(bar, text="EKF", variable=self._mon_ekf,
                            bg=BG2, fg=FG, font=("Consolas", 8),
                            selectcolor=BG, activebackground=BG2,
                            activeforeground=FG)
        cb2.pack(side=tk.RIGHT, padx=(0, 2))
        cb3 = tk.Checkbutton(bar, text="Wave", variable=self._mon_wave,
                            bg=BG2, fg=FG, font=("Consolas", 8),
                            selectcolor=BG, activebackground=BG2,
                            activeforeground=FG)
        cb3.pack(side=tk.RIGHT, padx=(0, 2))
        cb4 = tk.Checkbutton(bar, text="Raw", variable=self._mon_raw,
                            bg=BG2, fg=FG, font=("Consolas", 8),
                            selectcolor=BG, activebackground=BG2,
                            activeforeground=FG)
        cb4.pack(side=tk.RIGHT, padx=(0, 2))

    def _build_reg_panel(self, parent, title, reg_dict, prefix, side):
        """构建寄存器面板"""
        panel = tk.Frame(parent, bg=BG2)
        panel.pack(side=side, fill=tk.BOTH, expand=True,
                   padx=(0, 4) if side == tk.LEFT else (4, 0))

        # 标题
        hdr = tk.Frame(panel, bg=BG2)
        hdr.pack(fill=tk.X, padx=6, pady=(6, 2))
        tk.Label(hdr, text=title, bg=BG2, fg=ACCENT,
                font=("Consolas", 10, "bold")).pack(side=tk.LEFT)

        # 表头
        col_hdr = tk.Frame(panel, bg=BG2)
        col_hdr.pack(fill=tk.X, padx=6)
        for text, width, align in [
            ("Addr", 6, "w"), ("Name", 14, "w"),
            ("Raw", 5, "e"), ("Value", 10, "e"), ("Unit", 4, "w")
        ]:
            lbl = tk.Label(col_hdr, text=text, width=width, bg=BG2, fg=DIM,
                          font=("Consolas", 7), anchor=align)
            lbl.pack(side=tk.LEFT, padx=(0, 4) if align == "w" else (0, 2))

        # 分隔线
        sep = tk.Frame(panel, height=1, bg=BORDER)
        sep.pack(fill=tk.X, padx=6, pady=(1, 2))

        # 可滚动寄存器列表
        canvas = tk.Canvas(panel, bg=BG2, highlightthickness=0, height=220)
        scrollbar = ttk.Scrollbar(panel, orient=tk.VERTICAL, command=canvas.yview)
        scroll_frame = tk.Frame(canvas, bg=BG2)

        scroll_frame.bind("<Configure>",
                         lambda e: canvas.configure(scrollregion=canvas.bbox("all")))
        canvas.create_window((0, 0), window=scroll_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)

        canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(4, 0))
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        # 鼠标滚轮
        def _on_mousewheel(event):
            canvas.yview_scroll(int(-1 * (event.delta / 120)), "units")
        canvas.bind_all("<MouseWheel>", _on_mousewheel)

        # 寄存器行
        target_dict = self.reg_rows if prefix == "reg" else self.ekf_rows
        for addr, (name, unit, scale, desc) in reg_dict.items():
            row = RegisterRow(scroll_frame, addr, name, unit, scale, desc)
            row.pack(fill=tk.X, padx=6, pady=1)
            target_dict[name] = row

        # EKF 额外显示解析值
        if prefix == "ekf":
            sep2 = tk.Frame(scroll_frame, height=1, bg=BORDER)
            sep2.pack(fill=tk.X, padx=6, pady=(6, 4))

            computed = [
                ("power_actual", "实际功率",     "W"),
                ("freq_hz",     "频率 (Hz)",     "Hz"),
                ("phase_deg",   "相位角 (deg)",  "°"),
                ("delta_ppg_signed", "PID增量",  ""),
            ]
            self._ekf_computed = {}
            for key, label, unit in computed:
                fr = tk.Frame(scroll_frame, bg=BG2)
                fr.pack(fill=tk.X, padx=8, pady=1)
                tk.Label(fr, text=label, width=14, bg=BG2, fg=YELLOW,
                        font=("Consolas", 9), anchor="w").pack(side=tk.LEFT)
                val_lbl = tk.Label(fr, text="--", width=14, bg=BG2, fg=GREEN,
                                  font=("Consolas", 9, "bold"), anchor="e")
                val_lbl.pack(side=tk.LEFT, padx=(4, 0))
                tk.Label(fr, text=unit, width=4, bg=BG2, fg=DIM,
                        font=("Consolas", 9), anchor="w").pack(side=tk.LEFT)
                self._ekf_computed[key] = val_lbl

        return panel

    def _build_control_bar(self, parent):
        """功率控制栏"""
        bar = tk.Frame(parent, bg=BG2)
        bar.pack(fill=tk.X, pady=(6, 0))
        bar.pack_propagate(False)
        bar.configure(height=70)

        inner = tk.Frame(bar, bg=BG2)
        inner.pack(expand=True)

        # 功率设定
        tk.Label(inner, text="功率", bg=BG2, fg=DIM,
                font=("Consolas", 9)).pack(side=tk.LEFT, padx=(12, 4))

        self.power_var = tk.IntVar(value=0)
        power_spin = tk.Spinbox(inner, textvariable=self.power_var,
                                from_=0, to=3000, increment=100,
                                width=6, bg=BG, fg=FG, font=("Consolas", 11, "bold"),
                                buttonbackground=BG2, relief=tk.FLAT,
                                state=tk.DISABLED)
        power_spin.pack(side=tk.LEFT, padx=(0, 2))

        tk.Label(inner, text="W", bg=BG2, fg=DIM,
                font=("Consolas", 9)).pack(side=tk.LEFT, padx=(0, 8))

        set_btn = tk.Button(inner, text="设定", command=self._set_power,
                           bg=ACCENT, fg=BG, font=("Consolas", 9, "bold"),
                           activebackground=ACCENT, relief=tk.FLAT,
                           cursor="hand2", state=tk.DISABLED, width=5)
        set_btn.pack(side=tk.LEFT, padx=(0, 16))
        self.power_btn = set_btn

        # 功率滑块
        self.power_slider = tk.Scale(inner, from_=0, to=3000, orient=tk.HORIZONTAL,
                                     variable=self.power_var, length=200,
                                     bg=BG2, fg=FG, troughcolor=BG,
                                     highlightthickness=0, state=tk.DISABLED,
                                     command=lambda _: self.power_var.set(
                                         round(self.power_var.get() / 100) * 100))
        self.power_slider.pack(side=tk.LEFT, padx=(0, 12))

        # 快捷功率按钮
        for w in [1000, 1500, 2000, 2200, 2400, 2600, 2800, 3000]:
            btn = tk.Button(inner, text=str(w), command=lambda v=w: self._quick_power(v),
                           bg=BORDER, fg=FG, font=("Consolas", 8),
                           activebackground=ACCENT, relief=tk.FLAT,
                           cursor="hand2", state=tk.DISABLED, width=4)
            btn.pack(side=tk.LEFT, padx=1)
            if not hasattr(self, '_quick_btns'):
                self._quick_btns = []
            self._quick_btns.append(btn)

        # 分隔
        tk.Frame(inner, width=2, bg=BORDER).pack(side=tk.LEFT, padx=12, fill=tk.Y)

        # ON/OFF 按钮
        self.on_btn = tk.Button(inner, text="● 启动加热", command=self._turn_on,
                                bg=RED, fg=BG, font=("Consolas", 10, "bold"),
                                activebackground=RED, relief=tk.FLAT,
                                cursor="hand2", state=tk.DISABLED, width=10)
        self.on_btn.pack(side=tk.LEFT, padx=4)

        self.off_btn = tk.Button(inner, text="○ 停止加热", command=self._turn_off,
                                 bg=BORDER, fg=FG, font=("Consolas", 10, "bold"),
                                 activebackground=BORDER, relief=tk.FLAT,
                                 cursor="hand2", state=tk.DISABLED, width=10)
        self.off_btn.pack(side=tk.LEFT, padx=4)

        # 加热状态指示灯
        self.heat_led = tk.Canvas(inner, width=14, height=14, bg=BG2, highlightthickness=0)
        self.heat_led.pack(side=tk.LEFT, padx=(4, 0))
        self._heat_circle = self.heat_led.create_oval(2, 2, 12, 12, fill=DIM, outline="")

    def _build_init_panel(self, parent):
        """控制寄存器面板 (0x2000-0x2014) — 可编辑, 固定高度可滚动"""
        # 外层容器 — 不扩展, 固定高度
        outer = tk.Frame(parent, bg=BG2, height=170)
        outer.pack(fill=tk.X, expand=False, pady=(6, 0))
        outer.pack_propagate(False)

        # 标题行
        hdr = tk.Frame(outer, bg=BG2)
        hdr.pack(fill=tk.X, padx=6, pady=(3, 1))
        tk.Label(hdr, text="控制寄存器 0x2000-0x2014 (可编辑, Enter/离焦即写, 自动保存)",
                bg=BG2, fg=ACCENT, font=("Consolas", 9, "bold")).pack(side=tk.LEFT)

        # Hex/Dec 切换
        self._hex_toggle_btn = tk.Button(hdr, text="DEC", bg=BORDER, fg=FG,
            font=("Consolas", 8, "bold"), activebackground=BORDER,
            relief=tk.FLAT, cursor="hand2", width=4,
            command=self._toggle_hex_mode)
        self._hex_toggle_btn.pack(side=tk.RIGHT, padx=(6, 0))
        tk.Label(hdr, text="显示:", bg=BG2, fg=DIM,
                font=("Consolas", 8)).pack(side=tk.RIGHT)
        read_btn = tk.Button(hdr, text="↻读取", command=self._read_init_regs,
                           bg=BORDER, fg=FG, font=("Consolas", 8),
                           activebackground=BORDER, relief=tk.FLAT, cursor="hand2")
        read_btn.pack(side=tk.RIGHT, padx=(8, 0))
        write_btn = tk.Button(hdr, text="⬆写入", command=self._write_all_init,
                            bg=ACCENT, fg=BG, font=("Consolas", 8, "bold"),
                            activebackground=ACCENT, relief=tk.FLAT, cursor="hand2")
        write_btn.pack(side=tk.RIGHT, padx=(6, 0))
        force_btn = tk.Button(hdr, text="⬇强制下发", command=self._force_write_all_init,
                            bg=RED, fg=BG, font=("Consolas", 8, "bold"),
                            activebackground=RED, relief=tk.FLAT, cursor="hand2")
        force_btn.pack(side=tk.RIGHT, padx=(6, 0))

        # 可滚动 canvas
        canvas = tk.Canvas(outer, bg=BG2, highlightthickness=0, height=108)
        scrollbar = ttk.Scrollbar(outer, orient=tk.VERTICAL, command=canvas.yview)
        grid_frame = tk.Frame(canvas, bg=BG2)
        grid_frame.bind("<Configure>",
                        lambda e: canvas.configure(scrollregion=canvas.bbox("all")))
        canvas.create_window((0, 0), window=grid_frame, anchor="nw", width=900)
        canvas.configure(yscrollcommand=scrollbar.set)

        canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(4, 0))
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y, padx=(0, 2))

        # 绑定鼠标滚轮
        def _on_mousewheel(event):
            canvas.yview_scroll(int(-1 * (event.delta / 120)), "units")
        canvas.bind("<MouseWheel>", _on_mousewheel)
        canvas.bind("<Enter>", lambda e: canvas.focus_set())

        # 表头
        for col, (text, w) in enumerate([
            ("Addr", 6), ("名称", 12), ("值", 12),
            ("", 2), ("Addr", 6), ("名称", 12), ("值", 12)
        ]):
            lbl = tk.Label(grid_frame, text=text, width=w, bg=BG2, fg=DIM,
                          font=("Consolas", 7),
                          anchor="w" if col in (1, 2, 4, 5) else "e")
            lbl.grid(row=0, column=col, padx=(0, 3), sticky="w")

        sep = tk.Frame(grid_frame, height=1, bg=BORDER)
        sep.grid(row=1, column=0, columnspan=7, sticky="ew", pady=(1, 2))

        # 分列: init (左) / ctrl (右)
        init_items = [(a, n, l) for a, (n, l, c) in INIT_REGS.items() if c == "init"]
        ctrl_items = [(a, n, l) for a, (n, l, c) in INIT_REGS.items() if c == "ctrl"]
        all_rows = max(len(init_items), len(ctrl_items))

        # 空行占位 (左列比右列多时)
        for row_idx in range(all_rows):
            if row_idx < len(init_items):
                addr, name, label = init_items[row_idx]
                self._make_init_row(grid_frame, row_idx + 2, 0, addr, name, label)
            if row_idx < len(ctrl_items):
                addr, name, label = ctrl_items[row_idx]
                # 功率控制行用特殊颜色
                is_ctrl = (addr in (0x200E, 0x2010, 0x200F, 0x2012))
                self._make_init_row(grid_frame, row_idx + 2, 3, addr, name, label,
                                   fg_color=YELLOW if is_ctrl else GREEN)

        self._load_config()

    def _make_init_row(self, parent, row, col, addr, name, label,
                      fg_color=GREEN):
        """创建一个可编辑的寄存器行"""
        addr_lbl = tk.Label(parent, text=f"0x{addr:04X}", width=6,
                           bg=BG2, fg=DIM, font=("Consolas", 8), anchor="e")
        addr_lbl.grid(row=row, column=col, padx=(0, 4), sticky="e")

        name_lbl = tk.Label(parent, text=label, width=12,
                           bg=BG2, fg=ACCENT, font=("Consolas", 8), anchor="w")
        name_lbl.grid(row=row, column=col+1, padx=(0, 4), sticky="w")

        var = tk.StringVar(value="--")
        entry = tk.Entry(parent, textvariable=var, width=12,
                        bg=BG, fg=fg_color, font=("Consolas", 9, "bold"),
                        insertbackground=FG, relief=tk.FLAT,
                        disabledbackground=BG, disabledforeground=DIM)
        entry.grid(row=row, column=col+2, padx=(0, 2), sticky="w")

        entry.bind("<Return>", lambda e, a=addr: self._on_init_edit(a))
        entry.bind("<FocusOut>", lambda e, a=addr: self._on_init_edit(a))

        self._init_entries[addr] = {"entry": entry, "var": var, "name": name}

    # ---- 初始化寄存器操作 ---------------------------------------

    def _toggle_hex_mode(self):
        """切换 Hex/Dec 显示模式"""
        new = not self._hex_mode.get()
        self._hex_mode.set(new)
        self._hex_toggle_btn.configure(text="HEX" if new else "DEC")
        self._refresh_init_display()

    def _refresh_init_display(self):
        """按当前 hex/dec 模式刷新所有 init entry 显示"""
        for addr, info in self._init_entries.items():
            val = self._init_config.get(addr)
            if val is not None:
                if self._hex_mode.get():
                    info["var"].set(f"0x{val:04X}")
                else:
                    info["var"].set(str(val))
            else:
                info["var"].set("--")

    def _read_init_regs(self):
        """读取全部 0x2000-0x2014 寄存器"""
        if not self.client:
            return
        self._set_status("读取控制寄存器 0x2000-0x2014...")
        raw = self.client.read_registers(INIT_START_ADDR, INIT_COUNT)
        if raw is None:
            self._set_status("读取控制寄存器失败")
            return
        for i, (addr, (_name, _label, _cat)) in enumerate(INIT_REGS.items()):
            self._init_config[addr] = raw[i]
        self._refresh_init_display()
        self._set_status(f"已读取 {len(raw)} 个控制寄存器")
        self._save_config()

    def _on_init_edit(self, addr):
        """用户编辑寄存器值 → 解析 → 写入设备 → 保存"""
        if not self.client:
            return
        info = self._init_entries.get(addr)
        if not info:
            return
        text = info["var"].get().strip()
        if text == "--" or not text:
            # 恢复显示
            self._refresh_init_display()
            return

        # 解析 (支持 hex: 0xNNNN 或 dec: NNNN)
        try:
            if text.lower().startswith("0x"):
                val = int(text, 16)
            elif self._hex_mode.get():
                val = int(text, 16)
            else:
                val = int(text)
        except ValueError:
            self._refresh_init_display()
            return

        if val < 0 or val > 0xFFFF:
            self._refresh_init_display()
            return

        # 写入设备
        ok = self.client.write_register(addr, val)
        if ok:
            self._init_config[addr] = val
            self._set_status(f"写 0x{addr:04X} = {val} (0x{val:04X})  已保存")
            self._refresh_init_display()
            self._save_config()
        else:
            self._refresh_init_display()

    def _write_all_init(self):
        """批量写入全部已编辑的 init 寄存器"""
        if not self.client:
            return
        # 先按地址排序写入
        for addr in sorted(self._init_config.keys()):
            val = self._init_config[addr]
            self.client.write_register(addr, val)
            time.sleep(0.02)  # 2ms delay between writes
        self._set_status(f"已写入 {len(self._init_config)} 个控制寄存器")

    def _force_write_all_init(self):
        """强制下发: 先读设备再写, 确保配置生效 (用于初始化场景)"""
        if not self.client:
            return
        # 1. 先读一次设备当前值
        raw = self.client.read_registers(INIT_START_ADDR, INIT_COUNT)
        if raw is None:
            self._set_status("强制下发失败: 读取设备超时")
            return
        # 2. 合并: 设备值做底, 本地配置覆盖
        merged = {}
        for i, (addr, (_name, _label, _cat)) in enumerate(INIT_REGS.items()):
            merged[addr] = raw[i]
        for addr, val in self._init_config.items():
            merged[addr] = val
        # 3. 全部写入设备
        for addr in sorted(merged.keys()):
            self.client.write_register(addr, merged[addr])
            time.sleep(0.02)
        # 4. 更新本地配置
        self._init_config = merged
        self._refresh_init_display()
        self._save_config()
        self._set_status(f"强制下发完成: {len(merged)} 个寄存器 (先读后写)")

    # ---- JSON 配置持久化 ---------------------------------------

    def _config_path(self):
        """配置文件路径 (与 GUI 同目录)"""
        return os.path.join(os.path.dirname(os.path.abspath(__file__)),
                           "m4_init_config.json")

    def _save_config(self):
        """保存 init 配置到 JSON 文件"""
        if not self._init_config:
            return
        try:
            data = {"_comment": "M4 init/ctrl registers — auto-saved by m4_gui.py",
                    "_updated": datetime.now().isoformat(),
                    "registers": {f"0x{a:04X}": v for a, v in self._init_config.items()}}
            with open(self._config_path(), 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=2, ensure_ascii=False)
        except Exception:
            pass  # 静默失败, 不影响操作

    def _load_config(self):
        """从 JSON 文件加载上次保存的配置"""
        path = self._config_path()
        if not os.path.exists(path):
            return
        try:
            with open(path, 'r', encoding='utf-8') as f:
                data = json.load(f)
            regs = data.get("registers", {})
            for key, val in regs.items():
                addr = int(key, 16)
                if addr in self._init_entries:
                    self._init_config[addr] = int(val)
            self._refresh_init_display()
            if regs:
                updated = data.get("_updated", "?")
                self._set_status(f"已加载保存的配置 ({len(regs)} 项, {updated})")
        except Exception:
            pass

    def _build_statusbar(self):
        self.status_var = tk.StringVar(value="就绪 — 请先连接串口")
        status = tk.Label(self.root, textvariable=self.status_var,
                         bg=BG2, fg=DIM, font=("Consolas", 8), anchor="w",
                         padx=10, pady=2)
        status.pack(fill=tk.X, side=tk.BOTTOM, padx=8, pady=(0, 8))

    # ---- 连接管理 ----------------------------------------------

    def _scan_ports(self):
        """扫描串口并更新下拉列表"""
        self._set_status("正在扫描串口...")
        self.port_cb.configure(state=tk.DISABLED)
        ports = scan_serial_ports()

        if ports:
            labels = [label for _dev, label in ports]
            self._port_map = {label: dev for dev, label in ports}
            self.port_cb["values"] = labels
            # 自动选中第一个
            self.port_var.set(labels[0])
            self._set_status(f"发现 {len(ports)} 个串口: {', '.join(dev for dev, _ in ports)}")
        else:
            self._port_map = {}
            self.port_cb["values"] = []
            self.port_var.set("")
            self._set_status("未发现串口 — 请检查 USB 连接后重试")

        self.port_cb.configure(state=tk.NORMAL if not self.client else tk.DISABLED)

    def _get_port_device(self) -> str:
        """从界面选中的 label 提取实际设备名"""
        selected = self.port_var.get().strip()
        return self._port_map.get(selected, selected.split()[0] if selected else "")

    def _connect(self):
        if self.client:
            self._disconnect()

        port = self._get_port_device()
        if not port:
            messagebox.showerror("参数错误", "请先扫描选择串口")
            return
        try:
            baud = int(self.baud_var.get())
            slave = int(self.slave_var.get())
        except ValueError:
            messagebox.showerror("参数错误", "波特率/从站地址必须为数字")
            return

        self._set_status(f"正在连接 {port}...")
        self.connect_btn.configure(text="...", state=tk.DISABLED)
        self.root.update()  # 刷新UI

        try:
            c = M4ModbusClient(port, baud, slave, timeout=1.0)
            ok = c.connect()
        except Exception as e:
            import traceback
            traceback.print_exc()
            messagebox.showerror("连接异常",
                               f"创建 MODBUS 客户端时发生异常:\n\n{e}\n\n"
                               f"请检查是否已安装:\n"
                               f"  python -m pip install pymodbus pyserial")
            self.connect_btn.configure(text="● 连接", state=tk.NORMAL)
            self._set_status(f"连接失败: {e}")
            return

        if ok:
            self.client = c
            self._set_connected_state(True)
            self._set_status(f"已连接 {c.port} @ {c.baudrate} baud, 从站={c.slave_addr}")
            # 协议初始化: 检查 SYS_STA bit 7, 按需触发初始化
            self.client.check_and_init()
            # 连接后先关功率, 避免沿用设备旧状态
            self.power_var.set(0)
            self.client.set_power(0)
            self.client.set_work_sta(False, fan_on=True)
            self.client.start_heartbeat(power_watt=0, fan_on=True)
            self._read_once()
            # 自动加载 init regs (优先JSON本地配置, 再读设备)
            if not self._init_config:
                self._read_init_regs()
            else:
                self._refresh_init_display()
            # 自动启动后台轮询 (50ms间隔刷新遥测)
            self._start_monitor()
        else:
            self.connect_btn.configure(text="● 连接", state=tk.NORMAL)
            self._set_status(f"连接失败: {port}")

    def _disconnect(self):
        self._stop_monitor()
        self._stop_recording()
        self._mon_wave.set(False)  # triggers _wave_stop via trace
        self._mon_raw.set(False)   # triggers _raw_stop via trace
        if self.plotter:
            self.plotter.close()
            self.plotter = None
            self.plot_btn.configure(text="▣ 波形", fg=FG)

        if self.client:
            self._save_config()
            try:
                self.client.disconnect()
            except Exception:
                pass
            self.client = None

        self._set_connected_state(False)
        self._clear_all_values()
        self._set_status("已断开")

    def _set_connected_state(self, connected: bool):
        state = tk.NORMAL if connected else tk.DISABLED
        color = GREEN if connected else DIM

        self.connect_btn.configure(
            text="● 断开" if connected else "● 连接",
            fg=RED if connected else GREEN,
            state=tk.NORMAL)
        self.conn_led.itemconfig(self._led_circle, fill=color)

        self.refresh_btn.configure(state=state)
        self.mon_btn.configure(state=state)
        self.rec_btn.configure(state=state)
        self.power_btn.configure(state=state)
        self.on_btn.configure(state=state)
        self.off_btn.configure(state=state)
        self.power_slider.configure(state=state)
        for btn in getattr(self, '_quick_btns', []):
            btn.configure(state=state)

        if HAS_PLOT:
            self.plot_btn.configure(state=state if connected else tk.DISABLED)

        # Init 面板条目
        entry_state = tk.NORMAL if connected else tk.DISABLED
        for info in self._init_entries.values():
            info["entry"].configure(state=entry_state)

    # ---- 数据读取 ----------------------------------------------

    def _read_once(self):
        if not self.client:
            return

        data = self.client.read_telemetry()
        if data:
            self._update_reg_display(data, self.reg_rows)

        ekf = self.client.read_ekf_telemetry()
        if ekf:
            self._update_reg_display(ekf, self.ekf_rows)
            self._update_ekf_computed(ekf)

        self._set_status(f"刷新完成 @ {datetime.now().strftime('%H:%M:%S')}")

    def _update_reg_display(self, data, row_dict):
        for name, row in row_dict.items():
            raw = data.get(f"{name}_raw", data.get(name, 0))
            val = data.get(name, 0)
            row.update(raw, val)

    def _update_ekf_computed(self, ekf):
        for key, lbl in self._ekf_computed.items():
            val = ekf.get(key, 0)
            if isinstance(val, float):
                if abs(val) < 10:
                    lbl.configure(text=f"{val:.3f}")
                elif abs(val) < 100:
                    lbl.configure(text=f"{val:.2f}")
                else:
                    lbl.configure(text=f"{val:.1f}")
            else:
                lbl.configure(text=str(val))

    def _clear_all_values(self):
        for row in self.reg_rows.values():
            row.update(0, 0)
        for row in self.ekf_rows.values():
            row.update(0, 0)
        for lbl in self._ekf_computed.values():
            lbl.configure(text="--")
        self.heat_led.itemconfig(self._heat_circle, fill=DIM)
        # 清除 init 条目
        for info in self._init_entries.values():
            info["var"].set("--")

    # ---- 监视模式 ----------------------------------------------

    def _toggle_monitor(self):
        if self.monitoring:
            self._stop_monitor()
        else:
            self._start_monitor()

    def _start_monitor(self):
        if not self.client:
            return
        self.monitoring = True
        self.mon_btn.configure(text="■ 停止", fg=YELLOW)
        self._monitor_poll()

    def _stop_monitor(self):
        self.monitoring = False
        if self.monitor_job:
            self.root.after_cancel(self.monitor_job)
            self.monitor_job = None
        self.mon_btn.configure(text="▶ 监视", fg=FG)

    def _monitor_poll(self):
        if not self.monitoring or not self.client:
            return

        pwr = 0

        if self._mon_std.get():
            data = self.client.read_telemetry()
            if data:
                self._update_reg_display(data, self.reg_rows)
                pwr = data.get("power_w", 0)
                work_sta = data.get("sys_sta", 0)
                heat_color = RED if (work_sta & 0x10) else DIM
                self.heat_led.itemconfig(self._heat_circle, fill=heat_color)
        else:
            # 不从标准遥测读功率时, 用心跳状态推算
            pwr = self.client._heartbeat_power_w * 25
            heat_on = bool(self.client._heartbeat_work_sta & 0x10)
            self.heat_led.itemconfig(self._heat_circle,
                                    fill=RED if heat_on else DIM)

        ekf = None
        if self._mon_ekf.get():
            ekf = self.client.read_ekf_telemetry()
            if ekf:
                # 补充实际功率 (0x1006 = Practical_Power, 寄存器值需 *25 = W)
                pwr_raw = self.client.read_registers(0x1006, 1)
                if pwr_raw is not None:
                    ekf["power_actual"] = pwr_raw[0] * 25
                self._update_reg_display(ekf, self.ekf_rows)
                self._update_ekf_computed(ekf)

        # 数据记录 (内存攒, 停止时批量写 CSV)
        if self._recording and ekf:
            self._records.append({
                "timestamp": datetime.now().isoformat(timespec="milliseconds"),
                "freq_hz": ekf.get("freq_hz", 0),
                "phase_deg": ekf.get("phase_deg", 0),
                "delta_ppg_signed": ekf.get("delta_ppg_signed", 0),
                "res_cur_adc": ekf.get("res_cur_adc", 0),
                "power_actual": ekf.get("power_actual", 0),
                "power_target": self.client._heartbeat_power_w * 25,
            })

        # 喂绘图器
        if self.plotter and ekf:
            self.plotter.feed(power_w=pwr,
                             freq_hz=ekf.get("freq_hz", 0),
                             phase_deg=ekf.get("phase_deg", 0),
                             delta_ppg=ekf.get("delta_ppg_signed", 0))
            self.plotter.update_plot()

        status = f"监视中 @ {datetime.now().strftime('%H:%M:%S')}  |  {self._sample_interval_ms}ms"
        if self._recording:
            status += f"  |  已记录 {len(self._records)} 条"
        self._set_status(status)

        self.monitor_job = self.root.after(self._sample_interval_ms, self._monitor_poll)

    # ---- 绘图 --------------------------------------------------

    def _toggle_plot(self):
        if self.plotter:
            self.plotter.close()
            self.plotter = None
            self.plot_btn.configure(text="▣ 波形", fg=FG)
            self._set_status("波形已关闭")
            return

        if not HAS_PLOT:
            messagebox.showinfo("提示", "需要 matplotlib: pip install matplotlib")
            return

        try:
            self.plotter = LivePlotter(window_s=30, sample_interval_s=0.2)
            self.plotter.open()
            self.plot_btn.configure(text="▣ 波形", fg=YELLOW)
            self._set_status("波形窗口已开启")
        except Exception as e:
            messagebox.showerror("绘图错误", str(e))

    # ---- 功率控制 ----------------------------------------------

    def _set_power(self):
        if not self.client:
            return
        watts = self.power_var.get()
        self.client.set_power(watts)  # FC10 批下发, 已包含全部控制寄存器
        self._set_status(f"设定功率: {watts}W (FC10 批下发)")

    def _quick_power(self, watts):
        self.power_var.set(watts)
        self._set_power()

    def _turn_on(self):
        if not self.client:
            return
        self.client.check_and_init()
        if self.power_var.get() == 0:
            self.power_var.set(1000)
        self.client.set_power(self.power_var.get())
        self.client.set_work_sta(True, fan_on=True)
        self.client.start_heartbeat(power_watt=self.power_var.get(), fan_on=True)
        self.heat_led.itemconfig(self._heat_circle, fill=RED)
        self._set_status(f"加热已启动 — {self.power_var.get()}W")

    def _turn_off(self):
        if not self.client:
            return
        self.power_var.set(0)
        self.client.set_power(0)
        self.client.set_work_sta(False, fan_on=True)
        self.heat_led.itemconfig(self._heat_circle, fill=DIM)
        self._set_status("加热已停止 — power=0, jitter=0")

    # ---- 数据记录 ----------------------------------------------

    def _toggle_record(self):
        if self._recording:
            self._stop_recording()
        else:
            self._start_recording()

    def _start_recording(self):
        self._records.clear()
        self._recording = True
        self.rec_btn.configure(text="■ 停止", fg=RED)
        self._set_status("EKF 数据记录中...")

    def _stop_recording(self):
        self._recording = False
        self.rec_btn.configure(text="● 记录", fg=FG)
        if self._records:
            ts = datetime.now().strftime("%Y%m%d_%H%M%S")
            filepath = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                   f"m4_ekf_{ts}.csv")
            logger = DataLogger(filepath)
            logger.open(self._csv_fields)
            for row in self._records:
                logger.write(row)
            logger.close()
            self._set_status(f"已保存 {len(self._records)} 条 → {os.path.basename(filepath)}")
        else:
            self._set_status("记录已停止 (无数据)")

    # ---- WaveCapture 波形批量回读 -------------------------------

    def _on_wave_toggle(self, *_):
        if self._mon_wave.get():
            self._wave_start()
        else:
            self._wave_stop()

    def _wave_start(self):
        if not self.client:
            return
        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        self._wave_csv_path = os.path.join(
            os.path.dirname(os.path.abspath(__file__)),
            f"wave_{ts}.csv")
        self._wave_file = open(self._wave_csv_path, 'w', newline='',
                               encoding='utf-8')
        self._wave_csv = csv.DictWriter(self._wave_file,
                                        fieldnames=self._wave_csv_fields)
        self._wave_csv.writeheader()
        self._wave_batch_id = 0
        self._set_status(f"Wave 记录开始 → {os.path.basename(self._wave_csv_path)}")
        self._wave_poll()

    def _wave_stop(self):
        if self._wave_job:
            self.root.after_cancel(self._wave_job)
            self._wave_job = None
        if self._wave_file:
            self._wave_file.close()
            self._wave_file = None
            self._wave_csv = None
            self._set_status(f"Wave 记录已停止 → "
                           f"{os.path.basename(self._wave_csv_path)}")

    def _wave_poll(self):
        if not self._mon_wave.get() or not self.client:
            return

        w = self.client.read_wave_capture()
        if w and w['count'] > 0:
            bid = self._wave_batch_id
            self._wave_batch_id += 1

            for fi, frame in enumerate(w['frames']):
                data = frame['data']
                para = frame['para']

                for si, val in enumerate(data):
                    row = {
                        'batch_id': bid,
                        'frame_idx': fi,
                        'sample_idx': si,
                        'value': val,
                        'para0': para[0] if len(para) > 0 else 0,
                        'para1': para[1] if len(para) > 1 else 0,
                        'para2': para[2] if len(para) > 2 else 0,
                        'para3': para[3] if len(para) > 3 else 0,
                        'para4': para[4] if len(para) > 4 else 0,
                    }
                    self._wave_csv.writerow(row)

            self._wave_file.flush()
            self._set_status(
                f"Wave #{bid} saved: {w['count']} frames × "
                f"~{w.get('data_words', 0)}w "
                f" → {os.path.basename(self._wave_csv_path)}")

        self._wave_job = self.root.after(self._wave_interval_ms,
                                         self._wave_poll)

    # ---- RawCapture 原始9列数据采集 ------------------------------

    def _on_raw_toggle(self, *_):
        if self._mon_raw.get():
            self._raw_start()
        else:
            self._raw_stop()

    def _raw_start(self):
        if not self.client:
            return
        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        self._raw_csv_path = os.path.join(
            os.path.dirname(os.path.abspath(__file__)),
            f"raw_{ts}.csv")
        self._raw_file = open(self._raw_csv_path, 'w', newline='',
                              encoding='utf-8')
        self._raw_csv = csv.DictWriter(self._raw_file,
                                       fieldnames=self._raw_csv_fields)
        self._raw_csv.writeheader()
        self._raw_batch_id = 0
        self._set_status(f"Raw 记录开始 → {os.path.basename(self._raw_csv_path)}")
        self._raw_poll()

    def _raw_stop(self):
        if self._raw_job:
            self.root.after_cancel(self._raw_job)
            self._raw_job = None
        if self._raw_file:
            self._raw_file.close()
            self._raw_file = None
            self._raw_csv = None
            self._set_status(f"Raw 记录已停止 → "
                           f"{os.path.basename(self._raw_csv_path)}")

    def _raw_poll(self):
        if not self._mon_raw.get() or not self.client:
            return

        r = self.client.read_raw_capture()
        if r and r['count'] > 0:
            bid = self._raw_batch_id
            self._raw_batch_id += 1

            for fi, frame in enumerate(r['frames']):
                i_adc = frame['i_adc']
                v_adc = frame['v_adc']
                cnt   = frame['cnt']
                cmp_  = frame['cmp']
                n     = frame['n']

                # Vdc_adc = 母线电压均值 (此帧内)
                vdc_mean = round(sum(v_adc) / len(v_adc)) if v_adc else 0

                # t_us 从 CNT 推算 (处理 16-bit 绕回)
                hrtim_clk_mhz = 768.0
                dt_per_cnt = 1.0 / hrtim_clk_mhz
                t_us = [0.0]
                for i in range(1, n):
                    delta = cnt[i] - cnt[i-1]
                    if delta < 0:
                        delta += 65536
                    t_us.append(t_us[-1] + delta * dt_per_cnt)

                for si in range(n):
                    row = {
                        't_us':     t_us[si],
                        'I_adc':    i_adc[si],
                        'V_adc':    v_adc[si],
                        'Vdc_adc':  vdc_mean,
                        'CNT':      cnt[si],
                        'CMP_UON':  cmp_[0] if len(cmp_) > 0 else 0,
                        'CMP_UOFF': cmp_[1] if len(cmp_) > 1 else 0,
                        'CMP_LON':  cmp_[2] if len(cmp_) > 2 else 0,
                        'CMP_LOFF': cmp_[3] if len(cmp_) > 3 else 0,
                        'POWER':    cmp_[4] if len(cmp_) > 4 else 0,
                    }
                    self._raw_csv.writerow(row)

            self._raw_file.flush()
            self._set_status(
                f"Raw #{bid} saved: {r['count']} frames × ~{n} samples "
                f" → {os.path.basename(self._raw_csv_path)}")

        self._raw_job = self.root.after(self._raw_interval_ms,
                                        self._raw_poll)

    # ---- 工具方法 ----------------------------------------------

    def _set_status(self, msg):
        self.status_var.set(msg)

    def on_close(self):
        self._stop_monitor()
        self._stop_recording()
        self._mon_wave.set(False)  # triggers _wave_stop via trace
        self._mon_raw.set(False)   # triggers _raw_stop via trace
        if self.plotter:
            self.plotter.close()
        if self.client:
            try:
                self.client.stop_heartbeat()
                self.client.set_work_sta(False)
                self.client.set_power(0)
                self.client.disconnect()
            except Exception:
                pass
        self.root.destroy()


# ============================================================
# 入口
# ============================================================

def main():
    import argparse
    parser = argparse.ArgumentParser(description="M4 MODBUS 调试 GUI")
    parser.add_argument("--port", type=str, default=None, help="自动连接串口")
    args = parser.parse_args()

    root = tk.Tk()
    app = M4DebugApp(root, auto_port=args.port)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()
    return 0


if __name__ == "__main__":
    sys.exit(main())
