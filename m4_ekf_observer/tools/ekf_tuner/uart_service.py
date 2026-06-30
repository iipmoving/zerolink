"""
uart_service.py — UART 数据分发服务

从串口读取全量数据，按协议头分发给 MODBUS 和 PM。
替换 pymodbus 的 socket，PmAwareSerial 的最终正确版本。
"""

import time


class UartService:
    """UART 数据分发服务

    从串口读全量数据，按协议分发给 MODBUS 和 PM：
    - fill() 从真实串口读数据
    - 0x05 开头 → MODBUS 环形队列 → pymodbus read()
    - #PM 开头 → PM 解码 → drain_pm()
    - write() 直写真实串口
    """

    def __init__(self, real_ser, slave_id=5):
        self._ser = real_ser          # 真实 pyserial 对象
        self._modbus_buf = bytearray()  # MODBUS 环形队列
        self._slave_id = slave_id
        self._pm_buf = bytearray()     # PM 字节缓存（跨周期积累）
        self._pm_lines = []            # PM 数据行
        self._pm_active = False        # 是否在收 PM 帧
        self._pm_start = 0.0           # PM 开始时间
        self._pm_timeout = 5.0         # PM 超时

    # ── pymodbus 接口 ──────────────────────────────

    @property
    def in_waiting(self):
        return len(self._modbus_buf) + (self._ser.in_waiting if self._ser else 0)

    def write(self, data):
        return self._ser.write(data)

    def read(self, size):
        """供 pymodbus 调用，从 MODBUS 环形队列取数据。
        不够时直接从真实串口补充。
        """
        deadline = time.monotonic() + getattr(self._ser, 'timeout', 0.05)
        while len(self._modbus_buf) < size:
            if time.monotonic() > deadline:
                break
            if self._ser.in_waiting:
                raw = self._ser.read(self._ser.in_waiting)
                self._modbus_buf.extend(raw)
                self._pm_buf.extend(raw)
            else:
                time.sleep(0.001)
        n = min(size, len(self._modbus_buf))
        data = bytes(self._modbus_buf[:n])
        self._modbus_buf = self._modbus_buf[n:]
        return data

    # ── 数据填充与分发 ──────────────────────────────

    def fill(self):
        """从真实串口读所有数据，分发给 MODBUS 和 PM"""
        if not self._ser.in_waiting:
            return
        raw = self._ser.read(self._ser.in_waiting)
        if not raw:
            return
        # 全部给 MODBUS（pymodbus framer 自行过滤非 MODBUS 数据）
        self._modbus_buf.extend(raw)
        # 全部给 PM（drain_pm 自行过滤非 PM 数据）
        self._pm_buf.extend(raw)

    # ── PM 提取 ──────────────────────────────

    def _feed_pm(self, data: bytes):
        """喂 PM 数据到 _pm_buf，由 drain_pm 消费"""
        self._pm_buf.extend(data)

    def drain_pm(self):
        """从 _pm_buf 提取 PM 帧。
        返回完整帧的行列表，或 []（帧未完成）。
        """
        if not self._pm_active:
            # IDLE: 找 #PM 头
            idx = self._pm_buf.find(b'#PM')
            if idx < 0:
                self._pm_buf.clear()
                return []
            nl = self._pm_buf.find(b'\n', idx)
            if nl < 0:
                return []
            line = self._pm_buf[idx:nl].decode('utf-8', errors='replace').strip()
            self._pm_buf = self._pm_buf[nl+1:]
            self._pm_lines.append(line)
            self._pm_active = True
            self._pm_start = time.monotonic()

        if self._pm_active:
            parts = self._pm_buf.split(b'\n')
            for i, raw in enumerate(parts[:-1]):
                if not raw:
                    continue
                line = raw.decode('utf-8', errors='replace').strip()
                if line == '#PM_END':
                    result = list(self._pm_lines)
                    self._pm_lines.clear()
                    self._pm_active = False
                    consumed = sum(len(p) + 1 for p in parts[:i+1])
                    self._pm_buf = self._pm_buf[consumed:]
                    return result if result else []
                if line.isprintable() or "\t" in line:
                    self._pm_lines.append(line)
            if parts:
                consumed = len(self._pm_buf) - len(parts[-1])
                self._pm_buf = self._pm_buf[consumed:]

            if time.monotonic() - self._pm_start > self._pm_timeout:
                self._pm_active = False
                result = list(self._pm_lines)
                self._pm_lines.clear()
                self._pm_buf.clear()
                return result if result else []

        return []