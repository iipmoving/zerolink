"""
printmessage_ui.py — PrintMessage 串口上报 Tab UI

集成到 m4_gui.py 中作为独立 Toplevel 窗口。
支持实时日志显示、历史记录表格、启动/停止采集。
"""

import tkinter as tk
from tkinter import ttk
import time
import threading
from datetime import datetime
from collections import deque
import sys
import os

# Support both standalone and package import
try:
    from .printmessage_decoder import PrintMessageDecoder
except ImportError:
    from printmessage_decoder import PrintMessageDecoder

# 配色 (与 m4_gui.py 一致)
BG2 = "#1e1e1e"
FG = "#d4d4d4"
ACCENT = "#569cd6"
GREEN = "#4ec9b0"
YELLOW = "#ce9178"
DIM = "#6a6a6a"
BORDER = "#2d2d2d"


class PrintMessageTab:
    """PrintMessage 串口上报 UI — 可作为 Toplevel 或 Frame 嵌入"""

    def __init__(self, parent, sendline_cb=None):
        """
        Args:
            parent: tkinter parent widget (Toplevel 或 Frame)
            sendline_cb: callback to send data to client (for test mode)
        """
        self.parent = parent
        self.sendline_cb = sendline_cb
        self.decoder = PrintMessageDecoder()
        self._running = False
        self._lock = threading.Lock()
        self._log = deque(maxlen=1000)
        self._record_id = 0
        self._build_ui()

    def _build_ui(self):
        tab = tk.Frame(self.parent, bg=BG2)
        tab.pack(fill=tk.BOTH, expand=True, pady=(4, 0))

        # ── 顶部工具栏 ──
        toolbar = tk.Frame(tab, bg=BG2)
        toolbar.pack(fill=tk.X, padx=6, pady=(4, 4))

        tk.Label(toolbar, text="串口上报 (PrintMessage)", bg=BG2, fg=ACCENT,
                 font=("Consolas", 10, "bold")).pack(side=tk.LEFT)

        self.capture_btn = tk.Button(
            toolbar, text="▶ 启动采集", command=self._toggle_capture,
            bg=BORDER, fg=FG, font=("Consolas", 8, "bold"),
            activebackground=BORDER, relief=tk.FLAT, cursor="hand2", width=10
        )
        self.capture_btn.pack(side=tk.RIGHT, padx=(4, 0))

        clear_btn = tk.Button(
            toolbar, text="清空", command=self._clear_log,
            bg=BORDER, fg=FG, font=("Consolas", 8),
            activebackground=BORDER, relief=tk.FLAT, cursor="hand2", width=6
        )
        clear_btn.pack(side=tk.RIGHT, padx=(4, 0))

        self.status_lbl = tk.Label(toolbar, text="停止", bg=BG2, fg=DIM,
                                   font=("Consolas", 8))
        self.status_lbl.pack(side=tk.RIGHT, padx=(8, 0))

        # ── 日志区域 ──
        log_frame = tk.Frame(tab, bg=BG2)
        log_frame.pack(fill=tk.BOTH, expand=True, padx=6, pady=(0, 4))

        tk.Label(log_frame, text="实时日志", bg=BG2, fg=YELLOW,
                 font=("Consolas", 8, "bold")).pack(anchor=tk.W)

        self.log_text = tk.Text(log_frame, height=10, bg=BG2, fg=FG,
                                font=("Consolas", 9), relief=tk.FLAT,
                                borderwidth=0, state=tk.DISABLED)
        log_scroll = ttk.Scrollbar(log_frame, orient=tk.VERTICAL,
                                   command=self.log_text.yview)
        self.log_text.configure(yscrollcommand=log_scroll.set)
        self.log_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        log_scroll.pack(side=tk.RIGHT, fill=tk.Y)

        # ── 历史记录表格 ──
        hist_frame = tk.Frame(tab, bg=BG2)
        hist_frame.pack(fill=tk.BOTH, expand=True, padx=6, pady=(0, 4))

        tk.Label(hist_frame, text="历史记录", bg=BG2, fg=YELLOW,
                 font=("Consolas", 8, "bold")).pack(anchor=tk.W)

        columns = ("#", "time", "type", "pulse", "rows", "summary")
        self.tree = ttk.Treeview(hist_frame, columns=columns,
                                  show="headings", height=8)
        self.tree.heading("#", text="#")
        self.tree.heading("time", text="时间")
        self.tree.heading("type", text="消息类型")
        self.tree.heading("pulse", text="脉冲数")
        self.tree.heading("rows", text="数据行")
        self.tree.heading("summary", text="摘要")

        self.tree.column("#", width=40, anchor=tk.CENTER)
        self.tree.column("time", width=80, anchor=tk.CENTER)
        self.tree.column("type", width=100, anchor=tk.CENTER)
        self.tree.column("pulse", width=60, anchor=tk.CENTER)
        self.tree.column("rows", width=60, anchor=tk.CENTER)
        self.tree.column("summary", width=200)

        tree_scroll = ttk.Scrollbar(hist_frame, orient=tk.VERTICAL,
                                    command=self.tree.yview)
        self.tree.configure(yscrollcommand=tree_scroll.set)
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        tree_scroll.pack(side=tk.RIGHT, fill=tk.Y)

    # ── 控制 ──

    def _toggle_capture(self):
        if self._running:
            self._stop_capture()
        else:
            self._start_capture()

    def _start_capture(self):
        self._running = True
        self.capture_btn.config(text="■ 停止")
        self.status_lbl.config(text="监听中", fg=GREEN)

    def _stop_capture(self):
        self._running = False
        self.capture_btn.config(text="▶ 启动采集")
        self.status_lbl.config(text="停止", fg=DIM)

    def _clear_log(self):
        self._log.clear()
        self.log_text.config(state=tk.NORMAL)
        self.log_text.delete("1.0", tk.END)
        self.log_text.config(state=tk.DISABLED)
        for item in self.tree.get_children():
            self.tree.delete(item)

    # ── 数据输入 ──

    def feed_line(self, line: str):
        """喂一行 UART 数据 (由外部 serial reader 线程调用)"""
        if not self._running:
            return
        result = self.decoder.feed_line(line)
        self._append_log(line)
        if result:
            self._on_frame_complete(result)

    def _append_log(self, line: str):
        """追加一行到日志 (线程安全)"""
        with self._lock:
            self._log.append((time.time(), line))
        # 更新 UI (必须在主线程)
        if self.log_text.winfo_exists():
            self.log_text.after(0, self._update_log_display)

    def _update_log_display(self):
        """在主线程中更新日志显示"""
        with self._lock:
            if lines_to_add == 0:
                return
            self.log_text.config(state=tk.NORMAL)
            for _, line in self._log:
                self.log_text.insert(tk.END, line + "\n")
            self._log.clear()
            self.log_text.see(tk.END)
            self.log_text.config(state=tk.DISABLED)

    def _on_frame_complete(self, result: dict):
        """一帧完整消息到达 — 更新历史表格 (主线程)"""
        summary = result["summary"]
        self._record_id += 1
        msg_type_map = {0: "PAN", 1: "TXA", 2: "CUR"}
        t = summary.get("msg_type", "")
        msg_type_name = msg_type_map.get(t, f"TYPE_{t}")
        pulse = summary.get("pulse", "-")
        rows = summary.get("rows", 0)

        # 摘要文本
        extra = ""
        if summary.get("msg_type") == 0:  # PAN
            extra = f"pulse={pulse}"
        elif "para0" in summary:
            extra = f"para0={summary['para0']}"
        else:
            extra = f"{rows} rows"

        now = datetime.now().strftime("%H:%M:%S")

        def _update():
            self.tree.insert("", 0, values=(
                self._record_id, now, msg_type_name, pulse, rows, extra
            ))

        if self.tree.winfo_exists():
            self.tree.after(0, _update)


# ── 独立测试 ──
if __name__ == "__main__":
    root = tk.Tk()
    root.title("PrintMessage Test")
    root.geometry("800x600")
    root.configure(bg=BG2)

    tab = PrintMessageTab(root)

    # 启动采集 (feed_line 要求 _running=True)
    tab._start_capture()

    # 模拟 PAN 数据
    tab.feed_line("pan pluse is 12.")
    tab.feed_line("Index,para1,para2")
    tab.feed_line("0\t1234\t5678")
    tab.feed_line("1\t1235\t5679")
    tab.feed_line("")

    root.mainloop()