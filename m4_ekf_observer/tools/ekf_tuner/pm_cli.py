#!/usr/bin/env python3
"""
pm_cli.py — PrintMessage CLI 指令工具

纯命令行操作：连接串口 → 下发 MODBUS 指令 → 接收 PrintMessage 数据 → 计算 f_res。
不依赖 GUI，可被 Claude Code (AI) 或其他脚本调用。

用法:
  # 交互模式 (连接后从 stdin 读指令)
  python pm_cli.py COM5

  # 单条指令
  python pm_cli.py COM5 --exec "w 0x200E 16 ; w 0x2010 40"

  # 检锅并等待 f_res
  python pm_cli.py COM5 --exec "w 0x2000 10" --wait-pan 3 --show-freq

  # 加热 + 采集 TXA 数据
  python pm_cli.py COM5 --exec "h" --wait 5 --show-log

指令:
  w <addr> <value>    写寄存器          (如 w 0x2010 40)
  r <addr> <count>    读寄存器          (如 r 0x1000 5)
  h                   启动加热 1000W
  s                   停止加热
  p <power_w>         设目标功率        (如 p 1000)
  pan                 触发检锅          (如 pan)
"""

import sys
import os
import time
import json
import argparse
import threading
import re

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from m4_modbus_tool import M4ModbusClient
from printmessage_decoder import PrintMessageDecoder, extract_pan_adc_data
from pan_analyzer import PanAnalyzer


def modbus_write(client: M4ModbusClient, addr: int, val: int) -> bool:
    """封装写寄存器，带错误处理"""
    ok = client.write_register(addr, val)
    if ok:
        print(f"  ✓ W 0x{addr:04X} = {val}")
    else:
        print(f"  ✗ W 0x{addr:04X} = {val} (失败)")
    return ok


def modbus_read(client: M4ModbusClient, addr: int, count: int) -> list | None:
    """封装读寄存器"""
    result = client.read_registers(addr, count)
    if result:
        for i, v in enumerate(result):
            print(f"    0x{addr+i:04X} = {v}")
    else:
        print(f"  ✗ R 0x{addr:04X} x{count} (无响应)")
    return result


class PMCapture:
    """PrintMessage 数据采集器 (纯 CLI, 无 GUI)"""

    def __init__(self, port: str, baud: int = 115200, slave: int = 5):
        self.port = port
        self.baud = baud
        self.slave = slave
        self.client = None
        self.ser = None
        self.decoder = PrintMessageDecoder()
        self.analyzer = PanAnalyzer()
        self.frames = []       # 收集到的完整帧
        self.raw_lines = []    # 原始行
        self.running = False

    # ── 连接 ──────────────────────────────────────────

    def connect(self) -> bool:
        """连接串口"""
        self.client = M4ModbusClient(
            port=self.port, baudrate=self.baud,
            slave_addr=self.slave, timeout=0.5
        )
        if not self.client.connect():
            print(f"[错误] 连接 {self.port} 失败")
            return False

        # 从 M4ModbusClient 获取 raw serial 对象
        self.ser = self.client.ser
        if self.ser is None:
            print("[错误] 无法获取 serial 对象")
            return False
        print(f"[OK] 已连接 {self.port} @ {self.baud}")
        return True

    def disconnect(self):
        if self.client:
            self.client.disconnect()

    # ── 读取 ──────────────────────────────────────────

    def read_loop(self, duration_s: float = 5.0):
        """持续读取串口指定时长，解析 PrintMessage 帧"""
        self.running = True
        self.frames.clear()
        buf = b""
        deadline = time.monotonic() + duration_s

        while time.monotonic() < deadline and self.running:
            try:
                if self.ser and self.ser.in_waiting:
                    buf += self.ser.read(self.ser.in_waiting)
                    while b"\n" in buf:
                        line, buf = buf.split(b"\n", 1)
                        text = line.decode("utf-8", errors="ignore").strip()
                        if not text:
                            continue
                        self.raw_lines.append(text)
                        result = self.decoder.feed_line(text)
                        if result:
                            self.frames.append(result)
                else:
                    time.sleep(0.01)
            except Exception:
                break

        self.running = False
        return self.frames

    def wait_for_pan(self, timeout_s: float = 5.0):
        """等待 PAN 消息并计算 f_res"""
        self.read_loop(timeout_s)
        for frame in self.frames:
            summary = frame["summary"]
            if summary.get("msg_type") == 0:  # PAN
                adc = extract_pan_adc_data(frame)
                if adc and len(adc) > 10:
                    f_res = self.analyzer.estimate_freq(adc, 1_000_000)
                    summary["f_res"] = round(f_res, 1)
                    return {
                        "pulse": summary.get("pulse"),
                        "rows": summary.get("rows"),
                        "f_res_hz": round(f_res, 1),
                        "frame": frame,
                    }
        return None

    # ── 指令 ──────────────────────────────────────────

    def execute(self, cmd_string: str):
        """执行指令字符串 (; 分隔多条)"""
        for part in cmd_string.split(";"):
            part = part.strip()
            if not part:
                continue
            self._exec_one(part)
            time.sleep(0.05)

    def _exec_one(self, cmd: str):
        parts = cmd.split()
        if not parts:
            return
        op = parts[0].lower()

        if op == "w" and len(parts) >= 3:
            modbus_write(self.client, int(parts[1], 0), int(parts[2], 0))
        elif op == "r" and len(parts) >= 3:
            modbus_read(self.client, int(parts[1], 0), int(parts[2], 0))
        elif op == "h":
            modbus_write(self.client, 0x200E, 0x10)
            modbus_write(self.client, 0x2010, 40)
        elif op == "s":
            modbus_write(self.client, 0x200E, 0x00)
        elif op == "p" and len(parts) >= 2:
            modbus_write(self.client, 0x2010, max(0, int(parts[1]) // 25))
        elif op == "pan":
            modbus_write(self.client, 0x2000, 10)
        else:
            print(f"[未知] {cmd}")


# ── 自动循环 ─────────────────────────────────────────


def _run_auto_loop():
    """解析 argv, 自动循环 N 次: 加热 → 采集 PAN → 计算 f_res → 停止 → 统计"""
    import json

    parser = argparse.ArgumentParser(description="PrintMessage 自动测试循环")
    parser.add_argument("port", nargs="?", default="")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--slave", type=int, default=5)
    parser.add_argument("--exec", default="w 0x200E 16 ; w 0x2010 40", help="加热指令")
    parser.add_argument("--wait-pan", type=float, default=3.0, help="等待 PAN 秒数")
    parser.add_argument("--cool", type=float, default=2.0, help="冷却等待秒数")
    parser.add_argument("--loop", type=int, default=5, help="循环次数")
    parser.add_argument("--csv", default="", help="CSV 结果文件路径")
    parser.add_argument("--show-freq", action="store_true", default=True,
                        help=argparse.SUPPRESS)
    args, _ = parser.parse_known_args()

    port = args.port
    if not port:
        try:
            import serial.tools.list_ports
            for p in serial.tools.list_ports.comports():
                if "USB" in p.description or "UART" in p.description:
                    port = p.device
                    break
            if not port:
                port = serial.tools.list_ports.comports()[0].device
        except Exception:
            pass

    if not port:
        print("请指定串口: python pm_cli.py COM5 --loop 10")
        sys.exit(1)

    cap = PMCapture(port, args.baud, args.slave)
    if not cap.connect():
        sys.exit(1)

    results = []
    print(f"\n▶ 自动测试: {args.loop} 次循环, 指令: {args.exec}")
    print(f"{'#':>3}  {'f_res':>8}  {'pulse':>5}  {'rows':>4}  {'time':>8}")
    print("-" * 45)

    try:
        for cycle in range(1, args.loop + 1):
            # 停止 + 冷却
            cap.execute("s")
            time.sleep(args.cool)
            cap.raw_lines.clear()
            cap.frames.clear()
            cap.decoder.clear()

            # 加热
            cap.execute(args.exec)
            t0 = time.monotonic()
            result = cap.wait_for_pan(args.wait_pan)
            elapsed = time.monotonic() - t0
            cap.execute("s")

            if result:
                f = result["f_res_hz"]
                pulse = result["pulse"]
                rows = result["rows"]
                print(f"{cycle:3d}  {f:>8.1f}  {pulse:>5}  {rows:>4}  {elapsed:>5.1f}s")
            else:
                f = 0
                pulse = 0
                rows = 0
                print(f"{cycle:3d}  {'N/A':>8}  {'-':>5}  {'-':>4}  {elapsed:>5.1f}s")

            results.append({
                "cycle": cycle, "f_res_hz": f, "pulse": pulse,
                "rows": rows, "elapsed_s": round(elapsed, 2)})

    except KeyboardInterrupt:
        print("\n\n▶ 手动中断")
    finally:
        cap.execute("s")
        cap.disconnect()

    # 统计
    freqs = [r["f_res_hz"] for r in results if r["f_res_hz"] > 0]
    print("\n" + "=" * 45)
    print(f"完成 {len(results)} 次循环")
    if freqs:
        avg = sum(freqs) / len(freqs)
        print(f"f_res: 平均={avg:.1f}Hz  最小={min(freqs):.1f}Hz  最大={max(freqs):.1f}Hz  波动={max(freqs)-min(freqs):.1f}Hz")
    else:
        print("未采集到有效 f_res 数据")

    # CSV
    if args.csv:
        with open(args.csv, "w", encoding="utf-8") as f:
            f.write("cycle,f_res_hz,pulse,rows,elapsed_s\n")
            for r in results:
                f.write(f"{r['cycle']},{r['f_res_hz']},{r['pulse']},{r['rows']},{r['elapsed_s']}\n")
        print(f"结果保存: {args.csv}")

    # JSON
    print(json.dumps({"summary": {"avg": sum(freqs)/len(freqs) if freqs else 0,
                                  "count": len(results)}, "results": results}))


# ── CLI ───────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="PrintMessage CLI 工具")
    parser.add_argument("port", nargs="?", default="", help="串口号")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--slave", type=int, default=5)
    parser.add_argument("--exec", default="", help="执行指令 (如 'w 0x200E 16 ; w 0x2010 40')")
    parser.add_argument("--wait", type=float, default=0, help="执行后等待 N 秒读取数据")
    parser.add_argument("--wait-pan", type=float, default=0, help="等待 PAN 消息并输出 f_res")
    parser.add_argument("--show-freq", action="store_true", help="显示 f_res")
    parser.add_argument("--show-log", action="store_true", help="显示原始日志")
    parser.add_argument("--json", action="store_true", help="JSON 输出 (供 AI 解析)")
    parser.add_argument("--interactive", action="store_true", help="交互模式 (从 stdin 读指令)")
    parser.add_argument("--loop", type=int, default=0,
                        help="自动循环 N 次: exec + wait-pan + 停止 + 统计")
    parser.add_argument("--cool", type=float, default=2.0, help="循环间冷却等待秒数")
    args = parser.parse_args()

    # 自动查找串口
    port = args.port
    if not port:
        try:
            import serial.tools.list_ports
            ports = serial.tools.list_ports.comports()
            for p in ports:
                if "USB" in p.description or "UART" in p.description:
                    port = p.device
                    break
            if not port and ports:
                port = ports[0].device
        except ImportError:
            pass

    if not port:
        print("请指定串口: python pm_cli.py COM5")
        sys.exit(1)

    cap = PMCapture(port, args.baud, args.slave)
    if not cap.connect():
        sys.exit(1)

    try:
        # 执行指令
        if args.exec:
            cap.execute(args.exec)

        # 等待 PAN 并计算 f_res
        if args.wait_pan > 0:
            result = cap.wait_for_pan(args.wait_pan)
            if result:
                print(f"\n[PAN] pulse={result['pulse']}  f_res={result['f_res_hz']} Hz  rows={result['rows']}")
                if args.json:
                    print(json.dumps(result, ensure_ascii=False))
            else:
                print(f"\n[PAN] 超时 {args.wait_pan}s，未收到 PAN 消息")
                if args.json:
                    print(json.dumps({"error": "timeout", "raw_lines": cap.raw_lines}))

        # 等待普通数据
        elif args.wait > 0:
            frames = cap.read_loop(args.wait)
            print(f"\n[读取] {len(frames)} 帧, {len(cap.raw_lines)} 行")
            if args.show_log:
                for line in cap.raw_lines:
                    print(f"  | {line}")
            if args.json:
                print(json.dumps({"frames": len(frames), "lines": len(cap.raw_lines)}))

        # 交互模式
        if args.interactive:
            print(f"\n交互模式 (输入指令, Ctrl+C 退出)")
            while True:
                try:
                    cmd = input("> ").strip()
                    if cmd.lower() in ("exit", "quit", "q"):
                        break
                    if cmd:
                        cap.execute(cmd)
                except KeyboardInterrupt:
                    break
                except EOFError:
                    break

    finally:
        cap.disconnect()


if __name__ == "__main__":
    # 自动循环模式（pm_cli.py COM5 --exec "加热指令" --wait-pan 3 --loop 10）
    if len(sys.argv) > 1 and "--loop" in sys.argv:
        _run_auto_loop()
    else:
        main()
