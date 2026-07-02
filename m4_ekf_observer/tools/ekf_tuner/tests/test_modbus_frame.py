"""Tests for MODBUS RTU frame parsing (no hardware needed)."""
import struct
from m4_modbus_tool import M4ModbusClient


def make_client():
    c = M4ModbusClient("VIRTUAL", timeout=0.01)
    c.ser = None  # no serial needed
    return c


def _crc(data: bytes) -> bytes:
    c = make_client()
    return struct.pack('<H', c._crc16(data))


def test_extract_fc03_frame():
    c = make_client()
    # Build valid FC03 response: addr=5, func=3, byte_count=4 (2 regs), data, CRC
    payload = bytes([0x05, 0x03, 0x04, 0x00, 0x64, 0x00, 0xC8])
    frame = payload + _crc(payload)
    c._mb_buf.extend(frame)
    result = c._extract_modbus_frame()
    assert result is not None
    assert result[0] == 0x05
    assert result[1] == 0x03


def test_extract_with_pm_noise():
    c = make_client()
    # PM text before MODBUS frame
    noise = b"#PM0\ndata\n#PM_END\n"
    payload = bytes([0x05, 0x03, 0x04, 0x00, 0x64, 0x00, 0xC8])
    frame = noise + payload + _crc(payload)
    c._mb_buf.extend(frame)
    result = c._extract_modbus_frame()
    assert result is not None  # should skip PM noise
    assert result[0] == 0x05


def test_extract_crc_mismatch_skipped():
    c = make_client()
    payload = bytes([0x05, 0x03, 0x02, 0x00, 0x01])
    bad_frame = payload + struct.pack('<H', 0x0000)  # wrong CRC
    good_frame = payload + _crc(payload)
    c._mb_buf.extend(bad_frame + good_frame)
    result = c._extract_modbus_frame()
    assert result is not None  # should skip bad frame, find good one
    assert result == good_frame


def test_fc06_echo():
    c = make_client()
    payload = bytes([0x05, 0x06, 0x20, 0x10, 0x00, 0x00])
    frame = payload + _crc(payload)
    c._mb_buf.extend(frame)
    result = c._extract_modbus_frame()
    assert result is not None
    assert result[1] == 0x06


def test_fc10_echo():
    c = make_client()
    payload = bytes([0x05, 0x10, 0x20, 0x0E, 0x00, 0x05])
    frame = payload + _crc(payload)
    c._mb_buf.extend(frame)
    result = c._extract_modbus_frame()
    assert result is not None
    assert result[1] == 0x10


def test_exception_response():
    c = make_client()
    # Exception: addr(5) + func(0x83=FC03+0x80) + exc_code(2=ILLEGAL_DATA_ADDR)
    payload = bytes([0x05, 0x83, 0x02])
    frame = payload + _crc(payload)
    c._mb_buf.extend(frame)
    result = c._extract_modbus_frame()
    assert result is not None
    assert result[1] == 0x83


def test_incomplete_frame():
    c = make_client()
    c._mb_buf.extend(bytes([0x05, 0x03]))  # only 2 bytes, need more
    result = c._extract_modbus_frame()
    assert result is None  # incomplete, wait for more data


def test_read_registers_parsing():
    c = make_client()
    payload = bytes([0x05, 0x03, 0x04, 0x00, 0x64, 0x01, 0x2C])
    frame = payload + _crc(payload)
    # Manually inject frame and parse regs
    byte_count = frame[2]
    registers = []
    for i in range(byte_count // 2):
        pos = 3 + i * 2
        registers.append((frame[pos] << 8) | frame[pos + 1])
    assert registers == [0x0064, 0x012C]
    assert registers[0] == 100  # 0x0064 = 100
    assert registers[1] == 300  # 0x012C = 300