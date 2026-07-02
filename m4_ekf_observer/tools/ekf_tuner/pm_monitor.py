#!/usr/bin/env python3
"""
pm_monitor.py — PrintMessage 独立监控 + CLI 指令

接收 PrintMessage 主动上报，同时支持命令行下发 MODBUS 指令。
MODBUS 协议复用 m4_modbus_tool.py 的 M4ModbusClient。

用法:
  python pm_monitor.py COM5
  python pm_monitor.py COM5 --baud 115200 --slave 5

指令:
  w <addr> <value>    — 写寄存器
  r <addr> <count>    — 读寄存器
  h                   — 启动加热
  s                   — 停止
  p <power_w>         — 设目标功率
"""

import sys
import os
import tkinter as tk
from tkinter import ttk
import threading
import time
from datetime import datetime

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from m4_modbus_tool import M4ModbusClient
from printmessage_ui import PrintMessageTab

# 配色
BG2 = "#1e1e1e"
FG = "#d4d4d4"
ACCENT = "#569cd6"
GREEN = "#4ec9b0"
YELLOW = "#ce9178"
DIM = "#6a6a6a"
BORDER = "#2d2d2d"


class PMMonitorApp:
    def __init__(self, port: str, baud: int = 115200, slave: int = 5):
        self.port = port
        self.baud = baud
        self.slave = slave
        self.client = None
        self.ser = None  # 从 M4ModbusClient 取出的原始 serial 对象
        self.running = True

        # ── 窗口 ──
        self.root = tk.Tk()
        self.root.title(f"PrintMessage - {port}")
        self.root.geometry("950x750")
        self.root.configure(bg=BG2)

        # ── 指令行 ──
        ctrl = tk.Frame(self.root, bg=BG2)
        ctrl.pack(fill=tk.X, padx=6, pady=(6, 2))

        tk.Label(ctrl, text="▶", bg=BG2, fg=ACCENT,
                 font=("Consolas", 10, "bold")).pack(side=tk.LEFT)

        self.cmd_var = tk.StringVar()
        self.cmd_entry = tk.Entry(ctrl, textvariable=self.cmd_var,
                                  bg=BORDER, fg=FG, font=("Consolas", 10),
                                  relief=tk.FLAT, insertbackground=FG)
        self.cmd_entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(4, 4))
        self.cmd_entry.bind("<Return>", lambda e: self.send_cmd())

        self.send_btn = tk.Button(ctrl, text="发送", command=self.send_cmd,
                                  bg=ACCENT, fg="#fff", font=("Consolas", 9, "bold"),
                                  activebackground="#3a8fd4", relief=tk.FLAT,
                                  cursor="hand2", width=6)
        self.send_btn.pack(side=tk.LEFT)

        # ── 快速按钮 ──
        btn_frame = tk.Frame(self.root, bg=BG2)
        btn_frame.pack(fill=tk.X, padx=6, pady=(0, 4))

        # 按钮配置: (文本, 命令列表, 颜色)
        buttons = [
            ("🔥 加热", "w 0x200E 16 ; w 0x2010 40", "#e06c75"),
            ("⏹ 停止", "w 0x200E 0", "#d19a66"),
            ("📡 检锅", "w 0x2000 10", "#61afef"),
            ("📊 读遥测", "r 0x1000 21", "#98c379"),
            ("📊 读EKF", "r 0x1020 21", "#c678dd"),
        ]
        for label, cmds, color in buttons:
            btn = tk.Button(btn_frame, text=label,
                            command=lambda c=cmds: self._enqueue(c),
                            bg=BORDER, fg=color,
                            font=("Consolas", 8, "bold"),
                            activebackground=BORDER, relief=tk.FLAT,
                            cursor="hand2")
            btn.pack(side=tk.LEFT, padx=(0, 4))

        # ── PrintMessage Tab ──
        self.pm_tab = PrintMessageTab(self.root, **{})
        self.pm_tab._start_capture()

        # ── 状态栏 ──
        self.status_lbl = tk.Label(self.root, text="连接中...",
                                   bg=BG2, fg=DIM, font=("Consolas", 9),
                                   anchor=tk.W)
        self.status_lbl.pack(fill=tk.X, padx=6, pady=(0, 4))

        # ── 连接 ──
        self._connect()
        if self.ser:
            threading.Thread(target=self._reader, daemon=True).start()

        self.root.protocol("WM_DELETE_WINDOW", self._on_close)

    # ── 连接 ──────────────────────────────────────────

    def _connect(self):
        """打开 MODBUS 连接 + 获取 raw serial 对象"""
        self.client = M4ModbusClient(
            port=self.port, baudrate=self.baud,
            slave_addr=self.slave, timeout=0.5
        )
        if not self.client.connect():
            self.status_lbl.config(text=f"连接 {self.port} 失败", fg=YELLOW)
            return

        # 从 M4ModbusClient 获取 raw serial 对象用于读 PM 数据
        self.ser = self.client.ser
        if self.ser is None:
            self.status_lbl.config(text="连接成功，但无法获取 serial 对象", fg=YELLOW)
        else:
            self.status_lbl.config(text=f"已连接 {self.port} @ {self.baud}", fg=GREEN)

    # ── PM 数据读取 ──────────────────────────────────

    def _reader(self):
        """从 serial 读取 PrintMessage 文本行"""
        buf = b""
        while self.running and self.ser:
            try:
                if self.ser.in_waiting:
                    buf += self.ser.read(self.ser.in_waiting)
                    while b"\n" in buf:
                        line, buf = buf.split(b"\n", 1)
                        text = line.decode("utf-8", errors="ignore").strip()
                        if text:
                            self.pm_tab.feed_line(text)
                else:
                    time.sleep(0.01)
            except Exception:
                break
        self.running = False

    # ── MODBUS 指令 ──────────────────────────────────

    def _execute(self, cmd: str):
        """执行指令 (在独立线程中运行, 不阻塞 UI)"""
        for part in cmd.split(";"):
            part = part.strip()
            if not part:
                continue
            parts = part.split()
            if not parts:
                continue
            op = parts[0].lower()

            if op == "w" and len(parts) >= 3:
                addr = int(parts[1], 0)
                val = int(parts[2], 0)
                ok = self.client.write_register(addr, val)
                self.status_lbl.config(
                    text=f"W 0x{addr:04X}={val} {'✓' if ok else '✗'}", fg=FG)

            elif op == "r" and len(parts) >= 3:
                addr = int(parts[1], 0)
                cnt = int(parts[2], 0)
                result = self.client.read_registers(addr, cnt)
                if result:
                    line = "  ".join(f"0x{addr+i:04X}={v}" for i, v in enumerate(result))
                    self.status_lbl.config(text=f"R {line}", fg=FG)
                else:
                    self.status_lbl.config(text="R 无响应", fg=YELLOW)

            elif op == "h":
                self.client.write_register(0x200E, 0x10)
                self.client.write_register(0x2010, 40)
                self.status_lbl.config(text="加热 1000W", fg=FG)

            elif op == "s":
                self.client.write_register(0x200E, 0x00)
                self.status_lbl.config(text="停止", fg=FG)

            elif op == "p" and len(parts) >= 2:
                power_w = int(parts[1])
                reg_val = max(0, power_w // 25)
                self.client.write_register(0x2010, reg_val)
                self.status_lbl.config(text=f"目标功率 {power_w}W (reg={reg_val})", fg=FG)

            else:
                self.status_lbl.config(text=f"未知: {cmd}", fg=YELLOW)

            time.sleep(0.05)  # 连续指令间间隔

    def _enqueue(self, cmds: str):
        """在后台线程执行"""
        threading.Thread(target=self._execute, args=(cmds,), daemon=True).start()

    def send_cmd(self):
        cmd = self.cmd_var.get().strip()
        if cmd:
            self.cmd_var.set("")
            self._enqueue(cmd)

    def _on_close(self):
        self.running = False
        if self.client:
            self.client.disconnect()
        self.root.destroy()

    def run(self):
        self.root.mainloop()


def main():
    import argparse
    parser = argparse.ArgumentParser(description="PrintMessage 独立监控")
    parser.add_argument("port", nargs="?", default="", help="串口号")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--slave", type=int, default=5)
    args = parser.parse_args()

    if not args.port:
        try:
            import serial.tools.list_ports
            ports = serial.tools.list_ports.comports()
            for p in ports:
                if "USB" in p.description or "UART" in p.description:
                    args.port = p.device
                    break
            if not args.port and ports:
                args.port = ports[0].device
        except ImportError:
            pass

    if not args.port:
        print("用法: python pm_monitor.py COM5")
        sys.exit(1)

    app = PMMonitorApp(args.port, args.baud, args.slave)
    app.run()


if __name__ == "__main__":
    main()