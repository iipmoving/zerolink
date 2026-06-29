import pytest
from m4_gui import PmAwareSerial

class FakeSerial:
    """模拟 pyserial 对象"""
    def __init__(self):
        self._buf = bytearray()
        self._closed = False
    @property
    def in_waiting(self):
        return len(self._buf)
    @property
    def closed(self):
        return self._closed
    def write(self, data):
        pass
    def read(self, size):
        size = min(size, len(self._buf))
        data = bytes(self._buf[:size])
        self._buf = self._buf[size:]
        return data

def test_scan_extracts_pm_lines():
    ser = FakeSerial()
    pm = PmAwareSerial(ser)
    pm._buf.extend(b"#PM0\nHello\n#PM_END\n")
    pm.scan()
    assert pm._pm_lines == ["#PM0", "Hello", "#PM_END"]
    assert len(pm._buf) == 0  # PM 数据被消费

def test_scan_skips_non_pm():
    ser = FakeSerial()
    pm = PmAwareSerial(ser)
    pm._buf.extend(b"\x05\x03\x0C\xCC\x52CRC#PM1\nData\n")
    pm.scan()
    assert len(pm._pm_lines) == 2  # #PM1 + Data
    assert len(pm._buf) > 0        # MODBUS 数据留下

def test_scan_partial_line_kept():
    ser = FakeSerial()
    pm = PmAwareSerial(ser)
    pm._buf.extend(b"#PM0\nIncomplete")
    pm.scan()
    assert len(pm._pm_lines) == 1    # #PM0 完整
    assert b"Incomplete" in pm._buf  # 无 \n 保留

def test_pm_end_exits_mode():
    ser = FakeSerial()
    pm = PmAwareSerial(ser)
    pm._pm_lines = ["#PM0", "data", "#PM_END"]
    assert pm.check_pm_end() == True
    assert pm._pm_mode == False

def test_read_via_buf():
    ser = FakeSerial()
    ser._buf.extend(b"\x05\x03\x06data!")
    pm = PmAwareSerial(ser, slave_id=5)
    data = pm.read(4)
    assert data == b"\x05\x03\x06d"  # 从 _buf 取前4字节

def test_pm_timeout():
    import time
    ser = FakeSerial()
    pm = PmAwareSerial(ser, pm_timeout=0.1)
    pm._pm_mode = True
    pm._pm_start = time.monotonic() - 0.2
    assert pm.is_pm_active() == False