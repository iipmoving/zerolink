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
import re
import time
import threading
import tkinter as tk
from tkinter import ttk, messagebox, filedialog
from datetime import datetime
from collections import deque, OrderedDict

# 确保能找到同目录模块
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from m4_modbus_tool import (
    M4ModbusClient, DataLogger, READ_REGS, EKF_REGS,
    READ_START_ADDR, READ_COUNT, EKF_START_ADDR, EKF_COUNT,
    TELEM_START_ADDR, TELEM_TOTAL_WORDS, _pm_feed
)

try:
    from plot_utils import LivePlotter
    HAS_PLOT = True
except ImportError:
    HAS_PLOT = False

try:
    from printmessage_ui import PrintMessageTab
    HAS_PM = True
except ImportError:
    HAS_PM = False

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
# WaveCapture 数据布局 (与 wave_capture.h CaptureSrcId 对齐)
# 每帧数据结构: [CNT×N][V_AD×N][I_AD×N][CMP1][CMP2][CMP3][CMP4][POWER]
# ============================================================

# C 端数据解析布局 (-1 = 动态, 从帧size推算)
CAPTURE_LAYOUT = [
    # (内部键, 每帧字数, -1=等分剩余)
    ("CNT",   -1),  # CAPTURE_SRC_CNT    = 0
    ("V_AD",  -1),  # CAPTURE_SRC_V_AD   = 1
    ("I_AD",  -1),  # CAPTURE_SRC_I_AD   = 2
    ("CMP1",   1),  # CAPTURE_SRC_CMP1   = 10
    ("CMP2",   1),  # CAPTURE_SRC_CMP2   = 11
    ("CMP3",   1),  # CAPTURE_SRC_CMP3   = 12
    ("CMP4",   1),  # CAPTURE_SRC_CMP4   = 13
    ("POWER",  1),  # CAPTURE_SRC_POWER  = 20  (解析用, 不输出CSV)
]

# 动态列数 (count == -1)
_CAPTURE_DYNAMIC_COUNT = sum(1 for _, c in CAPTURE_LAYOUT if c == -1)
# 固定列总字数 (count > 0)
_CAPTURE_FIXED_SUM = sum(c for _, c in CAPTURE_LAYOUT if c > 0)

# CSV 输出列定义: (MATLAB列名, 来源键)
# 来源键 "t_us" / "Vdc_adc" 为 GUI 合成, 其余对应 CAPTURE_LAYOUT 内部键
CSV_COLUMNS = [
    ("t_us",       "t_us"),       # sample × 0.5μs
    ("I_adc",      "I_AD"),       # C端: 谐振电流 ADC
    ("V_adc",      "V_AD"),       # C端: 谐振电压 ADC
    ("Vdc_adc",    "Vdc_adc"),    # GUI计算: mean(V_AD) 帧内均值
    ("CNT",        "CNT"),        # C端: HRTIM 计数器
    ("CMP_UON",    "CMP1"),       # C端: 上管开通比较值
    ("CMP_UOFF",   "CMP2"),       # C端: 上管关断比较值
    ("CMP_LON",    "CMP3"),       # C端: 下管开通比较值
    ("CMP_LOFF",   "CMP4"),       # C端: 下管关断比较值
    ("POWER",      "POWER"),      # C端: 功率参考值
    ("sample",     "sample"),     # GUI合成: 帧内序号 0,1,2,...
]
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
        self.root.state("zoomed")  # Windows 最大化
        self.root.configure(bg=BG)
        self.root.minsize(800, 600)

        self.client: M4ModbusClient | None = None
        self.plotter: LivePlotter | None = None
        self.monitoring = False
        self.monitor_job = None
        self._sample_interval_ms = 40   # 25Hz 刷新

        # 遥测选择
        self._mon_std = tk.BooleanVar(value=False)  # 标准遥测 (默认关, 节省带宽)
        self._mon_ekf = tk.BooleanVar(value=True)   # EKF 遥测
        self._mon_telem = tk.BooleanVar(value=False)  # 0x7000 遥测 (默认关)

        # 数据记录
        self._recording = False
        self._records = []   # 内存记录, 停止时批量写 CSV

        # PrintMessage 后台读取
        self._pm_thread = None
        self._pm_running = False
        self._csv_fields = ["timestamp", "freq_hz", "phase_deg",
                           "delta_ppg_signed", "power_actual", "power_target"]

        # 遥测数据流验证
        self._telem_data = None

        # 遥测行组件引用
        self.reg_rows = {}      # name → RegisterRow (标准)
        self.ekf_rows = {}      # name → RegisterRow (EKF)
        self._ekf_computed = {} # 解析值标签

        # 初始化寄存器面板
        self._init_entries = {}  # addr → {"entry": tk.Entry, "var": StringVar, "name": str}
        self._hex_mode = tk.BooleanVar(value=False)  # Hex/Dec 切换
        self._init_config = {}   # addr → int (loaded from JSON)

        # 文件保存路径 (默认脚本所在目录)
        self.save_path_var = tk.StringVar(
            value=os.path.dirname(os.path.abspath(__file__)))

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

        # 保存路径设置行
        self._build_savepath_row()

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

        # 数据流验证 tab
        self._build_telem_tab(main)

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

        self.pm_btn = tk.Button(bar, text="📡 PM", command=self._toggle_pm_window,
                                bg=BORDER, fg=FG, font=("Consolas", 9),
                                activebackground=BORDER, relief=tk.FLAT,
                                cursor="hand2", state=tk.DISABLED, width=7)
        self.pm_btn.pack(side=tk.RIGHT, padx=(4, 0))

        self.auto_btn = tk.Button(bar, text="▶ 自动", command=self._toggle_auto_test,
                                  bg=BORDER, fg="#c678dd", font=("Consolas", 9),
                                  activebackground=BORDER, relief=tk.FLAT,
                                  cursor="hand2", state=tk.DISABLED, width=7)
        self.auto_btn.pack(side=tk.RIGHT, padx=(4, 0))

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

        cb3 = tk.Checkbutton(bar, text="遥测", variable=self._mon_telem,
                            bg=BG2, fg=FG, font=("Consolas", 8),
                            selectcolor=BG, activebackground=BG2,
                            activeforeground=FG)
        cb3.pack(side=tk.RIGHT, padx=(0, 2))
        self.cap_btn = tk.Button(bar, text="■ Capt", command=self._read_capture,
                                  bg=BORDER, fg=FG, font=("Consolas", 9),
                                  activebackground=BORDER, relief=tk.FLAT,
                                  cursor="hand2", state=tk.DISABLED, width=7)
        self.cap_btn.pack(side=tk.RIGHT, padx=(4, 0))

        self.telem_btn = tk.Button(bar, text="📡 Telem",
                                   command=self._read_telemetry,
                                   bg=BORDER, fg=FG, font=("Consolas", 9),
                                   activebackground=BORDER, relief=tk.FLAT,
                                   cursor="hand2", state=tk.DISABLED, width=8)
        self.telem_btn.pack(side=tk.RIGHT, padx=(4, 0))

    def _build_savepath_row(self):
        """文件保存路径设置行"""
        row = tk.Frame(self.root, bg=BG2, height=32)
        row.pack(fill=tk.X, padx=8, pady=(2, 0))
        row.pack_propagate(False)

        tk.Label(row, text="保存路径", bg=BG2, fg=DIM,
                font=("Consolas", 8)).pack(side=tk.LEFT, padx=(10, 4))

        path_entry = tk.Entry(row, textvariable=self.save_path_var,
                              bg=BG, fg=FG, font=("Consolas", 8),
                              insertbackground=FG, relief=tk.FLAT)
        path_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 4))

        def _browse():
            d = filedialog.askdirectory(
                title="选择 CSV 保存目录",
                initialdir=self.save_path_var.get())
            if d:
                self.save_path_var.set(d)
                self._save_config()

        browse_btn = tk.Button(row, text="浏览...", command=_browse,
                               bg=BORDER, fg=FG, font=("Consolas", 8),
                               activebackground=BORDER, relief=tk.FLAT,
                               cursor="hand2", width=6)
        browse_btn.pack(side=tk.LEFT, padx=(0, 10))

    def _get_save_path(self, filename):
        """返回完整保存路径, 若配置目录不存在则回退到脚本目录"""
        d = self.save_path_var.get()
        if not os.path.isdir(d):
            d = os.path.dirname(os.path.abspath(__file__))
        return os.path.join(d, filename)

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
                ("power_actual", "实际功率",      "W"),
                ("freq_khz",     "频率",          "Hz"),
                ("phase_deg",    "相位角",        "°"),
                ("vdc_mean",     "母线电压",      "V"),
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

    def _build_telem_tab(self, parent):
        """数据流验证面板 — Calculator 20周期 + ElecParams float + CSV保存"""
        tab = tk.Frame(parent, bg=BG2)
        tab.pack(fill=tk.BOTH, expand=True, pady=(6, 0))
        tab.pack_propagate(False)
        tab.configure(height=340)

        inner = tk.Frame(tab, bg=BG2)
        inner.pack(fill=tk.BOTH, expand=True, padx=6, pady=4)

        # ---- 顶部工具栏 ----
        toolbar = tk.Frame(inner, bg=BG2)
        toolbar.pack(fill=tk.X, pady=(0, 4))

        tk.Label(toolbar, text="数据流验证 0x7000", bg=BG2, fg=ACCENT,
                font=("Consolas", 9, "bold")).pack(side=tk.LEFT)

        self.telem_read_btn = tk.Button(toolbar, text="📡 读取遥测",
                                        command=self._read_telemetry,
                                        bg=BORDER, fg=FG, font=("Consolas", 8, "bold"),
                                        activebackground=BORDER, relief=tk.FLAT,
                                        cursor="hand2", state=tk.DISABLED, width=12)
        self.telem_read_btn.pack(side=tk.RIGHT, padx=(4, 0))

        self.telem_save_btn = tk.Button(toolbar, text="💾 保存CSV",
                                        command=self._save_telem_csv,
                                        bg=BORDER, fg=FG, font=("Consolas", 8, "bold"),
                                        activebackground=BORDER, relief=tk.FLAT,
                                        cursor="hand2", state=tk.DISABLED, width=10)
        self.telem_save_btn.pack(side=tk.RIGHT, padx=(4, 0))

        self.telem_status_lbl = tk.Label(toolbar, text="就绪", bg=BG2, fg=DIM,
                                         font=("Consolas", 8))
        self.telem_status_lbl.pack(side=tk.RIGHT, padx=(8, 0))

        # ---- 分隔 ----
        sep = tk.Frame(inner, height=1, bg=BORDER)
        sep.pack(fill=tk.X, pady=(2, 4))

        # ---- 左侧: Calculator 20周期表格 ----
        left = tk.Frame(inner, bg=BG2)
        left.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 4))

        tk.Label(left, text="Calculator 输入 (20周期)", bg=BG2, fg=YELLOW,
                font=("Consolas", 8, "bold")).pack(fill=tk.X, pady=(0, 2))

        calc_canvas = tk.Canvas(left, bg=BG2, highlightthickness=0,
                                yscrollcommand=lambda s, e: self.telem_calc_scroll.config(command=s))
        calc_scroll = ttk.Scrollbar(left, orient=tk.VERTICAL, command=calc_canvas.yview)
        self.telem_calc_frame = tk.Frame(calc_canvas, bg=BG2)

        self.telem_calc_frame.bind("<Configure>",
            lambda e: calc_canvas.configure(scrollregion=calc_canvas.bbox("all")))
        calc_canvas.create_window((0, 0), window=self.telem_calc_frame, anchor="nw")
        calc_canvas.configure(yscrollcommand=lambda s, e: calc_scroll.config(command=s))

        calc_canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        calc_scroll.pack(side=tk.RIGHT, fill=tk.Y)

        def _on_mousewheel(event):
            calc_canvas.yview_scroll(int(-1 * (event.delta / 120)), "units")
        calc_canvas.bind_all("<MouseWheel>", _on_mousewheel)

        # 表头
        calc_hdr = tk.Frame(self.telem_calc_frame, bg=BG2)
        calc_hdr.pack(fill=tk.X, padx=2, pady=(2, 0))
        for text, w in [("Idx", 3), ("hOff", 5), ("lOff", 5), ("hOn", 5), ("lOn", 5),
                         ("Ipk", 5), ("vc", 4), ("zcH", 5), ("zcL", 5), ("pp", 4),
                         ("actH", 5), ("actL", 5), ("vSum", 5)]:
            tk.Label(calc_hdr, text=text, width=w, bg=BG2, fg=DIM,
                    font=("Consolas", 7)).pack(side=tk.LEFT, padx=(0, 1))

        # 20行占位
        self.telem_calc_rows = []
        for i in range(20):
            row = tk.Frame(calc_hdr, bg=BG2)
            row.pack(fill=tk.X, pady=0)
            vals = []
            for _j in range(13):
                v = tk.Label(row, text="--", width=5, bg=BG2, fg=FG,
                            font=("Consolas", 7))
                v.pack(side=tk.LEFT, padx=(0, 1))
                vals.append(v)
            self.telem_calc_rows.append(vals)

        # ---- 右侧: ElecParams 浮点值 ----
        right = tk.Frame(inner, bg=BG2)
        right.pack(side=tk.RIGHT, fill=tk.Y, padx=(4, 0))

        tk.Label(right, text="ElecParams 输出 (float)", bg=BG2, fg=YELLOW,
                font=("Consolas", 8, "bold")).pack(fill=tk.X, pady=(0, 2))

        ep_canvas = tk.Canvas(right, bg=BG2, highlightthickness=0, height=240, width=200)
        ep_scroll = ttk.Scrollbar(right, orient=tk.VERTICAL, command=ep_canvas.yview)
        ep_frame = tk.Frame(ep_canvas, bg=BG2)

        ep_frame.bind("<Configure>",
            lambda e: ep_canvas.configure(scrollregion=ep_canvas.bbox("all")))
        ep_canvas.create_window((0, 0), window=ep_frame, anchor="nw")
        ep_canvas.configure(yscrollcommand=ep_scroll.set)

        ep_canvas.pack(side=tk.LEFT, fill=tk.Y, expand=True)
        ep_scroll.pack(side=tk.RIGHT, fill=tk.Y)

        float_names = [
            ("I_peak_A", "A"), ("Vdc_mean", "V"), ("phi_deg", "°"),
            ("f_sw_Hz", "Hz"), ("L_uH", "μH"), ("f_res_kHz", "kHz"),
            ("Q_factor", ""), ("R_ohm", "Ω"), ("I_rms", "A"),
            ("P_W", "W"), ("Z_mag_ohm", "Ω"), ("X_ohm", "Ω"),
        ]
        self.telem_ep_labels = []
        for name, unit in float_names:
            fr = tk.Frame(ep_frame, bg=BG2)
            fr.pack(fill=tk.X, pady=1)
            tk.Label(fr, text=name, width=11, bg=BG2, fg=ACCENT,
                    font=("Consolas", 7), anchor="w").pack(side=tk.LEFT)
            vl = tk.Label(fr, text="--", width=9, bg=BG2, fg=GREEN,
                         font=("Consolas", 7, "bold"), anchor="e")
            vl.pack(side=tk.LEFT)
            tk.Label(fr, text=unit, width=4, bg=BG2, fg=DIM,
                    font=("Consolas", 7), anchor="w").pack(side=tk.LEFT)
            self.telem_ep_labels.append(vl)

        # valid 行
        fr = tk.Frame(ep_frame, bg=BG2)
        fr.pack(fill=tk.X, pady=1)
        tk.Label(fr, text="valid", width=11, bg=BG2, fg=ACCENT,
                font=("Consolas", 7), anchor="w").pack(side=tk.LEFT)
        vl = tk.Label(fr, text="--", width=9, bg=BG2, fg=YELLOW,
                     font=("Consolas", 7, "bold"), anchor="e")
        vl.pack(side=tk.LEFT)
        self.telem_ep_labels.append(vl)

        # status 行
        fr = tk.Frame(ep_frame, bg=BG2)
        fr.pack(fill=tk.X, pady=1)
        tk.Label(fr, text="status", width=11, bg=BG2, fg=ACCENT,
                font=("Consolas", 7), anchor="w").pack(side=tk.LEFT)
        vl = tk.Label(fr, text="--", width=9, bg=BG2, fg=YELLOW,
                     font=("Consolas", 7, "bold"), anchor="e")
        vl.pack(side=tk.LEFT)
        self.telem_ep_labels.append(vl)

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
        for w in [500, 1000, 1500, 2000]:
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
                    "save_path": self.save_path_var.get(),
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
            # 恢复保存路径
            saved_path = data.get("save_path", "")
            if saved_path and os.path.isdir(saved_path):
                self.save_path_var.set(saved_path)
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
            return  # 断开后保持断开

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
            c = M4ModbusClient(port, baud, slave, timeout=0.05)
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
            # 用 UartService 替换 socket，统一管理串口数据
            from uart_service import UartService
            if hasattr(c.client, 'socket') and c.client.socket:
                self._uart = UartService(c.client.socket, slave_id=c.slave_addr)
                c.client.socket = self._uart
            else:
                self._uart = None
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
        self.cap_btn.configure(state=state)
        self.telem_btn.configure(state=state)
        self.telem_read_btn.configure(state=state)
        self.pm_btn.configure(state=state)
        self.auto_btn.configure(state=state)
        for btn in getattr(self, '_quick_btns', []):
            btn.configure(state=state)

        if HAS_PLOT:
            self.plot_btn.configure(state=state if connected else tk.DISABLED)

        # Init 面板条目
        entry_state = tk.NORMAL if connected else tk.DISABLED
        for info in self._init_entries.values():
            info["entry"].configure(state=entry_state)

    def _toggle_pm_window(self):
        """打开/关闭 PrintMessage 监控窗口"""
        if hasattr(self, '_pm_window') and self._pm_window and self._pm_window.winfo_exists():
            self._pm_window.destroy()
            self._pm_window = None
            self._pm_ui = None
            return

        if not HAS_PM:
            messagebox.showinfo("提示", "printmessage_ui.py 未找到 — 无法打开 PrintMessage 窗口")
            return

        self._pm_window = tk.Toplevel(self.root)
        self._pm_window.title("PrintMessage 串口上报")
        self._pm_window.geometry("800x600")
        self._pm_window.configure(bg=BG)

        self._pm_ui = PrintMessageTab(self._pm_window)
        self._pm_ui._start_capture()

    # ---- 自动测试 ------------------------------------------------

    def _toggle_auto_test(self):
        if hasattr(self, '_auto_win') and self._auto_win and self._auto_win.winfo_exists():
            self._auto_win.destroy()
            self._auto_win = None
            self._auto_active = False
            return
        self._auto_active = False
        self._auto_results = []
        self._auto_cycle = 0
        self._auto_state = "IDLE"
        self._auto_last_frame = None
        self._auto_win = tk.Toplevel(self.root)
        self._auto_win.title("自动测试")
        self._auto_win.geometry("520x320")
        self._auto_win.configure(bg=BG2)
        ctrl = tk.Frame(self._auto_win, bg=BG2)
        ctrl.pack(fill=tk.X, padx=6, pady=6)
        tk.Label(ctrl, text="循环:", bg=BG2, fg=FG,
                 font=("Consolas", 9)).pack(side=tk.LEFT)
        self._auto_spin = tk.Spinbox(ctrl, from_=1, to=100, width=4,
                                      bg=BORDER, fg=FG, font=("Consolas", 9),
                                      relief=tk.FLAT, buttonbackground=BORDER)
        self._auto_spin.pack(side=tk.LEFT, padx=(4, 6))
        self._auto_spin.delete(0, tk.END)
        self._auto_spin.insert(0, "10")
        self._auto_start_btn = tk.Button(ctrl, text="▶ 开始",
                                          command=self._auto_cmd_start,
                                          bg="#c678dd", fg="#fff",
                                          font=("Consolas", 9, "bold"),
                                          relief=tk.FLAT, cursor="hand2")
        self._auto_start_btn.pack(side=tk.LEFT)
        self._auto_save_btn = tk.Button(ctrl, text="💾 CSV",
                                         command=self._auto_save_csv,
                                         bg=BORDER, fg=FG, font=("Consolas", 9),
                                         relief=tk.FLAT, cursor="hand2",
                                         state=tk.DISABLED)
        self._auto_save_btn.pack(side=tk.LEFT, padx=(6, 0))

        cols = ("#", "f_res", "pulse", "rows", "time")
        self._auto_tree = ttk.Treeview(self._auto_win, columns=cols,
                                        show="headings", height=10)
        for c in cols:
            self._auto_tree.heading(c, text=c)
        self._auto_tree.column("#", width=40, anchor=tk.CENTER)
        self._auto_tree.column("f_res", width=100, anchor=tk.CENTER)
        self._auto_tree.column("pulse", width=60, anchor=tk.CENTER)
        self._auto_tree.column("rows", width=60, anchor=tk.CENTER)
        self._auto_tree.column("time", width=80, anchor=tk.CENTER)
        self._auto_tree.pack(fill=tk.BOTH, expand=True, padx=6)

        stat = tk.Frame(self._auto_win, bg=BG2)
        stat.pack(fill=tk.X, padx=6, pady=4)
        self._auto_stat = tk.Label(stat, text="", bg=BG2, fg=DIM,
                                    font=("Consolas", 9))
        self._auto_stat.pack(side=tk.LEFT)

    def _auto_cmd_start(self):
        self._auto_active = not getattr(self, '_auto_active', False)
        if not self._auto_active:
            self._auto_stop()
            return

        try:
            self._auto_total = int(self._auto_spin.get())
        except ValueError:
            return

        if not self.client:
            return

        # 保存原始 0x2000-0x2014
        self._auto_saved_regs = self.client.read_registers(0x2000, 21)
        if not self._auto_saved_regs:
            self._set_status("自动测试: 读取原始寄存器失败")
            return

        # 设置 0x2000 = 0x5F (A=5 检锅间隔, B=F 只检锅不加热)
        # 先尝试 FC06 单写，失败则走 FC10 批写
        ok = self.client.write_register(0x2000, 0x5F)
        if not ok:
            # FC06 可能被 MCU 拦截，走 FC10 批写全部 21 寄存器
            vals = [0x5F] + list(self._auto_saved_regs[1:])
            ok = self.client.write_registers(0x2000, vals)
        if not ok:
            self._set_status("自动测试: 设置 0x2000 失败")
            return

        # 初始化状态
        self._auto_cycle = 0
        self._auto_results = []
        self._auto_last_frame = None
        for i in self._auto_tree.get_children():
            self._auto_tree.delete(i)
        self._auto_start_btn.configure(text="■ 停止")
        self._auto_save_btn.configure(state=tk.DISABLED)
        self._auto_stat.configure(text="等待 PM 数据...", fg=YELLOW)

        # 开加热（心跳自动维持）
        self.client.set_power(1000)
        self.client.set_work_sta(True, fan_on=True)
        self._set_status(f"自动测试 {self._auto_total} 次 — 只检锅模式")

    def _auto_tick(self):
        """在 _poll_once 末尾执行。PM 持续收帧，不收 MODBUS 回读。"""
        if not self._auto_active:
            return

        # 读取 PM 帧（UartService 已填充）
        frame = getattr(self, '_auto_last_frame', None)
        if not frame:
            return
        self._auto_last_frame = None

        rows = frame.get("rows", [])
        if frame.get("msg_type", -1) != 0:  # 非 PAN
            return

        self._auto_cycle += 1
        f = pulse = 0
        adc = []
        in_data = False
        for r in rows:
            if "pluse" in r:
                try: pulse = int(r.split("is")[-1].strip().rstrip("."))
                except: pass
            if r.startswith("Index"):
                in_data = True
                continue
            if in_data and r.count("\t") >= 1:
                try: adc.append(int(r.split("\t")[1].strip()))
                except: pass

        from pan_analyzer import PanAnalyzer
        analyzer = PanAnalyzer()
        cnt = len(adc)
        f_res = round(analyzer.estimate_freq(adc, 1_000_000), 1) if cnt > 10 else 0

        rec = {"cycle": self._auto_cycle, "f_res_hz": f_res,
               "pulse": pulse, "rows": cnt, "adc": adc}
        self._auto_results.append(rec)

        self._auto_tree.insert("", "end", values=(
            self._auto_cycle, f"{f_res:.1f}" if f_res else "N/A",
            pulse or "-", cnt or "-", datetime.now().strftime("%H:%M:%S")))
        self._auto_tree.see(self._auto_tree.get_children()[-1])
        self._auto_stat.configure(
            text=f"已收 {self._auto_cycle}/{self._auto_total} 帧", fg=GREEN)

        # 完成
        if self._auto_cycle >= self._auto_total:
            self._auto_stop()

    def _auto_stop(self):
        """停止自动测试：关加热 → 恢复寄存器 → 保存 CSV"""
        self._auto_active = False
        self._auto_start_btn.configure(text="▶ 开始")
        self._auto_save_btn.configure(state=tk.NORMAL)

        # 关加热
        if self.client:
            self.client.set_power(0)
            self.client.set_work_sta(False, fan_on=True)

        # 恢复原始 0x2000-0x2014
        if self.client and hasattr(self, '_auto_saved_regs') and self._auto_saved_regs:
            try:
                self.client.write_register(0x2000, self._auto_saved_regs[0])
                self.client.write_registers(0x2000, self._auto_saved_regs)
            except Exception:
                pass

        # 统计
        freqs = [r["f_res_hz"] for r in self._auto_results if r["f_res_hz"] > 0]
        if freqs:
            avg = sum(freqs) / len(freqs)
            mn, mx = min(freqs), max(freqs)
            self._auto_stat.configure(
                text=f"完成 {len(self._auto_results)} 次  "
                     f"f_res: 平均={avg:.1f}  最小={mn:.1f}  最大={mx:.1f}  波动={mx-mn:.1f}Hz",
                fg=GREEN)
        else:
            self._auto_stat.configure(text="无有效数据", fg=YELLOW)

        # 自动保存 CSV
        self._auto_save_csv()
        self._auto_save_btn.configure(text="✓ 已保存")

    def _auto_save_csv(self):
        if not self._auto_results:
            return
        fn = f"auto_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
        path = self._get_save_path(fn)
        with open(path, "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            # 原始寄存器值
            if hasattr(self, '_auto_saved_regs') and self._auto_saved_regs:
                w.writerow(["# 原始 0x2000-0x2014"])
                w.writerow(["addr"] + [f"0x{0x2000+i:04X}" for i in range(21)])
                w.writerow(["value"] + list(self._auto_saved_regs))
                w.writerow([])
            # 结果表头
            w.writerow(["cycle", "f_res_hz", "pulse", "rows", "adc_para1"])
            for r in self._auto_results:
                adc_str = " ".join(str(v) for v in r.get("adc", []))
                w.writerow([r["cycle"], r["f_res_hz"], r["pulse"],
                           r["rows"], adc_str])
            # 统计
            freqs = [r["f_res_hz"] for r in self._auto_results if r["f_res_hz"] > 0]
            if freqs:
                w.writerow([])
                w.writerow(["avg", f"{sum(freqs)/len(freqs):.1f}"])
                w.writerow(["min", f"{min(freqs):.1f}"])
                w.writerow(["max", f"{max(freqs):.1f}"])
                w.writerow(["波动", f"{max(freqs)-min(freqs):.1f}"])
        print(f"[自动测试] CSV: {path}")

    # ---- PrintMessage 数据转发 ----------------------------------

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

    def _read_capture(self):
        """按 Capture 按钮: 读取 0x5000 区 + 保存 CSV"""
        if not self.client:
            self._set_status("未连接")
            return

        self._set_status("读取 0x5000 WaveCapture...")
        cap = self.client.read_capture()
        if cap is None:
            self._set_status("读取 0x5000 失败")
            return

        hdr = cap["header"]
        data = cap["data"]

        # 控制台打印原始数据
        print(f"\n=== 0x5000 Capture @ {datetime.now().strftime('%H:%M:%S')} ===")
        print(f"  ack=0x{hdr['ack']:04X} status=0x{hdr['status']:04X}"
              f" {'READY' if hdr['status'] & 1 else ''}{'COLLECTING' if hdr['status'] & 2 else ''}")
        print(f"  frame_id={hdr['frame_id']} count={hdr['count']}"
              f"  data_words={hdr['data_words']} max_frames={hdr['max_frames']}")
        print(f"  received data len = {len(data)}")
        if data:
            for k in range(0, min(len(data), 64), 16):
                chunk = " ".join(f"{w:04X}" for w in data[k:k+16])
                print(f"  [{k:4d}] {chunk}")
        print(f"=== end ===\n")

        self._set_status(
            f"0x5000: status=0x{hdr['status']:04X} "
            f"帧数={hdr['count']} 数据字数={hdr['data_words']} "
            f"最大帧数={hdr['max_frames']} frame_id={hdr['frame_id']}"
        )

        if hdr['count'] == 0 or len(data) == 0:
            self._set_status("0x5000: 无数据 (count=0)")
            return

        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"capture_{ts}.csv"
        filepath = self._get_save_path(filename)

        csv_header = [name for name, _ in CSV_COLUMNS]

        with open(filepath, 'w', newline='') as f:
            writer = csv.writer(f)

            # 全局表头 (仅一次)
            writer.writerow(csv_header)

            offset = 0
            frame_idx = 0
            dwords = min(hdr["data_words"], len(data))
            total_rows = 0

            while offset < dwords and frame_idx < hdr["count"]:
                sz = data[offset]  # size prefix
                offset += 1
                if offset + sz > dwords:
                    break

                # 动态计算每列采样数: N = (sz - 固定总字数) / 动态列数
                N = (sz - _CAPTURE_FIXED_SUM) // _CAPTURE_DYNAMIC_COUNT

                # 每帧第一行: SIZE 行 (文本标记 "SIZE", MATLAB 读为 NaN 可检测)
                size_row = ["SIZE"]
                for csv_name, src_key in CSV_COLUMNS[1:]:  # 跳过 t_us
                    if src_key in ("Vdc_adc", "sample"):
                        size_row.append(N)
                    else:
                        c = next((cnt for k, cnt in CAPTURE_LAYOUT if k == src_key), 1)
                        size_row.append(N if c == -1 else c)
                writer.writerow(size_row)

                # 按动态 N 切分本帧数据
                pos = offset
                columns = {}
                for key, count in CAPTURE_LAYOUT:
                    n = N if count == -1 else count
                    if pos + n > offset + sz:
                        columns[key] = []
                        break
                    columns[key] = list(data[pos:pos + n])
                    pos += n

                # 计算帧级派生值
                v_adc_vals = columns.get("V_AD", [])
                vdc_adc = round(sum(v_adc_vals) / len(v_adc_vals)) if v_adc_vals else 0

                # 写入采样行
                for s in range(N):
                    row = []
                    for csv_name, src_key in CSV_COLUMNS:
                        if src_key == "t_us":
                            row.append(s * 0.5)
                        elif src_key == "Vdc_adc":
                            row.append(vdc_adc)
                        elif src_key == "sample":
                            row.append(s)
                        else:
                            vals = columns.get(src_key, [])
                            if not vals:
                                row.append("")
                            elif s < len(vals):
                                row.append(vals[s])
                            else:
                                row.append("")  # 帧级参数 (CMP), 仅第0行
                    writer.writerow(row)
                    total_rows += 1

                # 帧间空行分隔
                writer.writerow([])

                offset += sz
                frame_idx += 1

        # 写 ACK → MCU 解锁 WaveCapture 缓冲区
        self.client.write_register(0x2015, 1)

        self._set_status(
            f"保存 {filename} — {frame_idx}帧, {total_rows}行, "
            f"列: {','.join(csv_header)}  |  ACK已发送 → 缓冲区解锁"
        )

    def _update_reg_display(self, data, row_dict):
        for name, row in row_dict.items():
            raw = data.get(f"{name}_raw", data.get(name, 0))
            val = data.get(name, 0)
            row.update(raw, val)

    def _update_ekf_computed(self, ekf):
        for key, lbl in self._ekf_computed.items():
            if key == "freq_khz":
                val = ekf.get("f_sw_hz", 0) * 1.0
            elif key == "phase_deg":
                val = ekf.get("phi_deg_x10", 0) * 0.1
            elif key == "vdc_mean":
                val = ekf.get("vdc_mean_v_x10", 0) * 0.1
            elif key == "q_factor":
                val = ekf.get("q_factor_x100", 0) * 0.01
            elif key == "power_actual":
                val = ekf.get("power_actual", 0)
            else:
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
        # 清除 Telemetry 显示
        self._telem_data = None
        self.telem_save_btn.configure(state=tk.DISABLED)
        self.telem_status_lbl.configure(text="就绪", fg=DIM)
        for row in self.telem_calc_rows:
            for lbl in row:
                lbl.configure(text="--")
        for lbl in self.telem_ep_labels:
            lbl.configure(text="--")

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
        self._telem_poll_skip = 0
        self.mon_btn.configure(text="■ 停止", fg=YELLOW)
        self._latest_std = None
        self._latest_ekf = None
        self._latest_telem = None
        threading.Thread(target=self._poll_loop, daemon=True).start()

    def _stop_monitor(self):
        self.monitoring = False
        if self.monitor_job:
            self.root.after_cancel(self.monitor_job)
            self.monitor_job = None
        self.mon_btn.configure(text="▶ 监视", fg=FG)

    def _poll_loop(self):
        """后台线程循环：固定 40ms 间隔读取 MODBUS"""
        while self.monitoring:
            t0 = time.perf_counter()
            self._poll_once()
            elapsed = (time.perf_counter() - t0) * 1000
            sleep = max(0.005, (self._sample_interval_ms - elapsed) / 1000)
            time.sleep(sleep)

    def _poll_once(self):
        """单次 MODBUS 读取 + 提交 UI 更新"""
        try:
            # ====== PM 数据读取（从 UartService 提取）======
            pm_frame = []
            if self._uart:
                self._uart.fill()
                pm_frame = self._uart.drain_pm()
            if pm_frame and hasattr(self, '_pm_ui') and self._pm_ui:
                for line in pm_frame:
                    self._pm_ui.feed_line(line)
                self._auto_last_frame = {"rows": pm_frame, "msg_type": 0}
            # ====== MODBUS 回读（自动模式下跳过，只发功率控制）======
            if not self.monitoring or not self.client:
                return
            if not getattr(self, '_auto_active', False):
                pwr = 0
                ekf = None
                if self._mon_std.get():
                    data = self.client.read_telemetry()
                    if data:
                        self._latest_std = data
                        pwr = data.get("power_w", 0)
                        work_sta = data.get("sys_sta", 0)
                        heat_color = RED if (work_sta & 0x10) else DIM
                        self.root.after(0, lambda c=heat_color: self.heat_led.itemconfig(
                            self._heat_circle, fill=c))
                else:
                    pwr = self.client._heartbeat_power_w * 25
                    heat_on = bool(self.client._heartbeat_work_sta & 0x10)
                    self.root.after(0, lambda: self.heat_led.itemconfig(
                        self._heat_circle, fill=RED if heat_on else DIM))

                if self._mon_ekf.get():
                    ekf = self.client.read_ekf_telemetry()
                    if ekf:
                        pwr_raw = self.client.read_registers(0x1006, 1)
                        if pwr_raw is not None:
                            ekf["power_actual"] = pwr_raw[0]
                        self._latest_ekf = ekf

                # Telemetry 0x7000 — 约 100ms 读取一次 (每3次轮询 ≈ 120ms)
                self._telem_poll_skip += 1
                if self._mon_telem.get() and self._telem_poll_skip >= 3:
                    self._telem_poll_skip = 0
                    telem = self.client.read_telemetry_pipe()
                    if telem:
                        self._latest_telem = telem
                        self.root.after(0, self._update_telem_panel)

                # 数据记录 (内存攒, 停止时批量写 CSV)
                if self._recording and ekf:
                    self._records.append({
                        "timestamp": datetime.now().isoformat(timespec="milliseconds"),
                        "freq_hz": ekf.get("f_sw_hz", 0) * 1.0,
                        "phase_deg": ekf.get("phi_deg_x10", 0) * 0.1,
                        "vdc_mean": ekf.get("vdc_mean_v_x10", 0) * 0.1,
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

                # 切回主线程更新 UI
                self.root.after(0, self._update_ui_from_worker)
        except Exception:
            pass  # 断线时静默容错

        # ====== 自动测试状态机（在 _poll_once 末尾执行）======
        if getattr(self, '_auto_active', False):
            self._auto_tick()

    def _update_ui_from_worker(self):
        """主线程：更新 UI 控件"""
        if self._latest_std:
            self._update_reg_display(self._latest_std, self.reg_rows)
            self._latest_std = None
        if self._latest_ekf:
            self._update_reg_display(self._latest_ekf, self.ekf_rows)
            self._update_ekf_computed(self._latest_ekf)
            status = f"监视中 @ {datetime.now().strftime('%H:%M:%S')}  |  {self._sample_interval_ms}ms"
            if self._recording:
                status += f"  |  已记录 {len(self._records)} 条"
            self._set_status(status)
            self._latest_ekf = None

    def _update_telem_panel(self):
        """主线程：刷新 Telemetry 0x7000 面板"""
        telem = self._latest_telem
        if not telem:
            return
        # 更新 Calculator 20周期表格
        for p, cp in enumerate(telem.get("calc_periods", [])):
            if p >= len(self.telem_calc_rows):
                break
            row = self.telem_calc_rows[p]
            row[0].configure(text=str(p))
            row[1].configure(text=str(cp.get("hrtim_highOff", 0)))
            row[2].configure(text=str(cp.get("hrtim_lowOff", 0)))
            row[3].configure(text=str(cp.get("hrtim_highOn", 0)))
            row[4].configure(text=str(cp.get("hrtim_lowOn", 0)))
            row[5].configure(text=str(cp.get("peak_current", 0)))
            row[6].configure(text=str(cp.get("voltage_count", 0)))
            row[7].configure(text=str(cp.get("zero_cross_high", 0)))
            row[8].configure(text=str(cp.get("zero_cross_low", 0)))
            row[9].configure(text=str(cp.get("peak_point", 0)))
            row[10].configure(text=str(cp.get("act_curr_high", 0)))
            row[11].configure(text=str(cp.get("act_curr_low", 0)))
            row[12].configure(text=str(cp.get("volt_sum", 0)))
        # 更新 ElecParams 浮点值
        elec = telem.get("elec", {})
        ep_keys = ["I_peak_A", "Vdc_mean", "phi_deg", "f_sw_Hz",
                    "L_uH", "f_res_kHz", "Q_factor", "R_ohm",
                    "I_rms", "P_W", "Z_mag_ohm", "X_ohm"]
        for i, key in enumerate(ep_keys):
            val = elec.get(key, 0)
            if isinstance(val, float):
                if abs(val) < 10:
                    self.telem_ep_labels[i].configure(text=f"{val:.4f}")
                elif abs(val) < 1000:
                    self.telem_ep_labels[i].configure(text=f"{val:.2f}")
                else:
                    self.telem_ep_labels[i].configure(text=f"{val:.1f}")
            else:
                self.telem_ep_labels[i].configure(text=str(val))
        self.telem_ep_labels[12].configure(text=str(elec.get("valid", 0)))
        st = telem.get("status", 0)
        flags = []
        if st & 0x01: flags.append("RUN")
        if st & 0x02: flags.append("CALC")
        if st & 0x04: flags.append("ELEC")
        self.telem_ep_labels[13].configure(text=f"0x{st:02X} {' '.join(flags)}")
        self.telem_status_lbl.configure(text="OK", fg=GREEN)

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
            self.power_var.set(500)
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
            filepath = self._get_save_path(f"m4_ekf_{ts}.csv")
            logger = DataLogger(filepath)
            logger.open(self._csv_fields)
            for row in self._records:
                logger.write(row)
            logger.close()
            self._set_status(f"已保存 {len(self._records)} 条 → {os.path.basename(filepath)}")
        else:
            self._set_status("记录已停止 (无数据)")

    # ---- Telemetry 数据流验证 -------------------------------------------

    def _read_telemetry(self):
        """读取 0x7000 Telemetry 数据并刷新显示"""
        if not self.client:
            return
        self._set_status("读取 Telemetry 0x7000...")
        self.root.update()

        telem = self.client.read_telemetry_pipe()
        if telem is None:
            self._set_status("读取 Telemetry 失败")
            return

        self._telem_data = telem

        # 更新 Calculator 20周期表格
        for p, cp in enumerate(telem["calc_periods"]):
            row = self.telem_calc_rows[p]
            row[0].configure(text=str(p))
            row[1].configure(text=str(cp.get("hrtim_highOff", 0)))
            row[2].configure(text=str(cp.get("hrtim_lowOff", 0)))
            row[3].configure(text=str(cp.get("hrtim_highOn", 0)))
            row[4].configure(text=str(cp.get("hrtim_lowOn", 0)))
            row[5].configure(text=str(cp.get("peak_current", 0)))
            row[6].configure(text=str(cp.get("voltage_count", 0)))
            row[7].configure(text=str(cp.get("zero_cross_high", 0)))
            row[8].configure(text=str(cp.get("zero_cross_low", 0)))
            row[9].configure(text=str(cp.get("peak_point", 0)))
            row[10].configure(text=str(cp.get("act_curr_high", 0)))
            row[11].configure(text=str(cp.get("act_curr_low", 0)))
            row[12].configure(text=str(cp.get("volt_sum", 0)))

        # 更新 ElecParams 浮点值
        elec = telem.get("elec", {})
        ep_keys = ["I_peak_A", "Vdc_mean", "phi_deg", "f_sw_Hz",
                    "L_uH", "f_res_kHz", "Q_factor", "R_ohm",
                    "I_rms", "P_W", "Z_mag_ohm", "X_ohm"]
        for i, key in enumerate(ep_keys):
            val = elec.get(key, 0)
            if isinstance(val, float):
                if abs(val) < 10:
                    self.telem_ep_labels[i].configure(text=f"{val:.4f}")
                elif abs(val) < 1000:
                    self.telem_ep_labels[i].configure(text=f"{val:.2f}")
                else:
                    self.telem_ep_labels[i].configure(text=f"{val:.1f}")
            else:
                self.telem_ep_labels[i].configure(text=str(val))

        # valid
        self.telem_ep_labels[12].configure(text=str(elec.get("valid", 0)))
        # status
        st = telem.get("status", 0)
        flags = []
        if st & 0x01: flags.append("RUN")
        if st & 0x02: flags.append("CALC")
        if st & 0x04: flags.append("ELEC")
        self.telem_ep_labels[13].configure(text=f"0x{st:02X} {' '.join(flags)}")

        self.telem_save_btn.configure(state=tk.NORMAL)
        self.telem_status_lbl.configure(text="OK", fg=GREEN)
        self._set_status(
            f"Telemetry 读取完成 @ {telem.get('timestamp', '?')}")

    def _save_telem_csv(self):
        """保存 Telemetry 数据到 CSV"""
        if not self._telem_data:
            messagebox.showwarning("提示", "暂无数据，请先读取遥测")
            return

        filename = f"telem_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
        filepath = self._get_save_path(filename)

        calc_periods = self._telem_data.get("calc_periods", [])
        elec = self._telem_data.get("elec", {})

        with open(filepath, 'w', newline='', encoding='utf-8') as f:
            writer = csv.writer(f)

            # 头部信息
            writer.writerow(["Telemetry 数据流验证"])
            writer.writerow(["时间戳", self._telem_data.get("timestamp", "")])
            writer.writerow(["控制", f"start={self._telem_data.get('ctrl_start', 0)} "
                                     f"status=0x{self._telem_data.get('status', 0):02X}"])
            writer.writerow([])

            # Calculator 20周期表头
            calc_headers = ["period", "hrtim_highOff", "hrtim_lowOff",
                           "hrtim_highOn", "hrtim_lowOn", "peak_current",
                           "act_curr_high", "act_curr_low", "volt_sum",
                           "voltage_count", "zero_cross_high", "zero_cross_low",
                           "peak_point"]
            writer.writerow(["=== Calculator 输入副本 (20周期) ==="])
            writer.writerow(calc_headers)
            for cp in calc_periods:
                writer.writerow([cp.get(k, 0) for k in calc_headers])

            writer.writerow([])

            # ElecParams 输出
            writer.writerow(["=== ElecParams 输出 (float) ==="])
            ep_items = list(elec.items())
            writer.writerow([name for name, _ in ep_items])
            writer.writerow([val for _, val in ep_items])

        self._set_status(f"已保存 → {os.path.basename(filepath)}")
        messagebox.showinfo("保存成功", f"数据已保存到:\n{filepath}")

    # ---- 工具方法 ----------------------------------------------

    def _set_status(self, msg):
        self.status_var.set(msg)

    def on_close(self):
        self._stop_monitor()
        self._stop_recording()
        if self.plotter:
            self.plotter.close()
        if self.client:
            try:
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
