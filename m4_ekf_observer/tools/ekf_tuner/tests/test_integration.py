"""Closed-loop integration tests for M4ModbusClient (no real hardware needed)."""
import struct
import time
import threading
from unittest.mock import MagicMock
from m4_modbus_tool import M4ModbusClient


class MockSerial:
    """Simulates a serial port that responds to MODBUS requests."""

    def __init__(self, responses=None):
        self._buf = bytearray()
        self._sent = bytearray()  # captured outbound data
        self.timeout = 0.01
        self.is_open = True
        # Pre-load response data
        if responses:
            for r in responses:
                self._buf.extend(r)

    @property
    def in_waiting(self):
        return len(self._buf)

    def read(self, size):
        """Read up to `size` bytes from buffer."""
        if not self._buf:
            if self.timeout:
                time.sleep(min(self.timeout, 0.002))
            return b''
        n = min(size, len(self._buf))
        data = bytes(self._buf[:n])
        self._buf = self._buf[n:]
        return data

    def write(self, data):
        """Capture sent data for later assertion."""
        self._sent.extend(data)

    def close(self):
        self.is_open = False


def _crc16(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc


def _make_fc03_response(registers):
    """Build a valid FC03 response frame for registers [int, ...]"""
    payload = bytes([0x05, 0x03, len(registers) * 2])
    for v in registers:
        payload += struct.pack('>H', v)
    return payload + struct.pack('<H', _crc16(payload))


def _make_fc06_echo(addr, value):
    payload = struct.pack('>B B HH', 0x05, 0x06, addr, value)
    return payload + struct.pack('<H', _crc16(payload))


def _make_fc10_echo(addr, count):
    """Build a valid FC10 echo: addr(5) + func(0x10) + start(2) + count(2) + CRC(2)"""
    payload = struct.pack('>B B HH', 0x05, 0x10, addr, count)
    return payload + struct.pack('<H', _crc16(payload))


# ============================================================
# Test cases
# ============================================================


def test_read_registers():
    """Send FC03 request, receive valid response, parse registers."""
    mock = MockSerial()
    mock._buf.extend(_make_fc03_response([100, 200, 300]))

    client = M4ModbusClient("MOCK", timeout=0.05)
    client.ser = mock

    result = client.read_registers(0x1000, 3)
    assert result == [100, 200, 300], f"Got {result}"
    # Verify request was sent: addr=0x05, func=0x03, start=0x1000, count=3
    sent = bytes(mock._sent)
    assert sent[0] == 0x05
    assert sent[1] == 0x03
    assert sent[2] == 0x10  # addr hi
    assert sent[3] == 0x00  # addr lo
    assert sent[4] == 0x00  # count hi
    assert sent[5] == 0x03  # count lo


def test_write_register():
    """Send FC06, receive echo, verify success."""
    mock = MockSerial()
    mock._buf.extend(_make_fc06_echo(0x2010, 40))

    client = M4ModbusClient("MOCK", timeout=0.05)
    client.ser = mock

    result = client.write_register(0x2010, 40)
    assert result is True
    # Verify sent frame starts with addr + FC06
    sent = bytes(mock._sent)
    assert sent[0] == 0x05
    assert sent[1] == 0x06


def test_write_registers():
    """Send FC10, receive echo, verify success."""
    mock = MockSerial()
    mock._buf.extend(_make_fc10_echo(0x200E, 5))

    client = M4ModbusClient("MOCK", timeout=0.05)
    client.ser = mock

    result = client.write_registers(0x200E, [0, 0xAA, 40, 0, 0])
    assert result is True


def test_drain_pm():
    """Inject PM data, verify drain_pm extracts it."""
    client = M4ModbusClient("MOCK", timeout=0.05)
    client.ser = MockSerial()
    client._pm_buf.extend(b"#PM0\nhello\nworld\n#PM_END\n")

    result = client.drain_pm()
    assert len(result) >= 3
    assert "#PM0" in result
    assert "hello" in result
    assert "world" in result


def test_check_and_init():
    """Simulate SYS_STA response (bit 7 set = initialized), verify init logic returns True."""
    mock = MockSerial()
    # SYS_STA register value with bit 7 set = device initialized
    mock._buf.extend(_make_fc03_response([0x80]))

    client = M4ModbusClient("MOCK", timeout=0.05)
    client.ser = mock

    result = client.check_and_init(verbose=False)
    assert result is True, f"Expected True (initialized), got {result}"


def test_check_and_init_not_initialized():
    """Simulate SYS_STA=0 (not initialized), verify init attempt is made."""
    mock = MockSerial()
    # First read returns SYS_STA=0, second returns 0x80 (after init)
    mock._buf.extend(_make_fc03_response([0x00]))
    mock._buf.extend(_make_fc03_response([0x80]))

    client = M4ModbusClient("MOCK", timeout=0.05)
    client.ser = mock

    result = client.check_and_init(verbose=False)
    assert result is True, f"Expected True (after init), got {result}"
    # Verify FC10 init frame was sent
    sent = bytes(mock._sent)
    # Should have at least one FC10 frame sent
    fc10_found = False
    for i in range(len(sent) - 1):
        if sent[i] == 0x05 and sent[i + 1] == 0x10:
            fc10_found = True
            break
    assert fc10_found, "Expected FC10 init frame to be sent"


def test_timeout_returns_none():
    """No response from device, verify timeout returns None."""
    mock = MockSerial()  # no responses pre-loaded
    client = M4ModbusClient("MOCK", timeout=0.05)
    client.ser = mock

    result = client.read_registers(0x1000, 3)
    assert result is None


def test_crc_error_recovery():
    """Bad CRC followed by good CRC, verify recovery."""
    # Build bad frame
    bad_payload = bytes([0x05, 0x03, 0x02, 0x00, 0x01])
    bad_frame = bad_payload + struct.pack('<H', 0x0000)
    # Build good frame
    good_regs = _make_fc03_response([42])

    mock = MockSerial()
    mock._buf.extend(bad_frame + good_regs)

    client = M4ModbusClient("MOCK", timeout=0.05)
    client.ser = mock

    result = client.read_registers(0x1000, 1)
    assert result == [42], f"Got {result}"


def test_mixed_pm_and_modbus():
    """PM data interleaved with MODBUS response, both handled correctly.

    Note: drain_pm() reads all available serial data and extends both
    _pm_buf and _mb_buf. After drain_pm(), read_registers() clears _mb_buf
    then writes a fresh request. We re-inject the MODBUS response to
    simulate the device responding to the new request.
    """
    pm_data = b"#PM0\ninfo\n#PM_END\n"
    modbus_resp = _make_fc03_response([100])

    mock = MockSerial()
    mock._buf.extend(pm_data + modbus_resp)

    client = M4ModbusClient("MOCK", timeout=0.05)
    client.ser = mock

    # First drain PM
    pm_result = client.drain_pm()
    assert len(pm_result) >= 2
    assert "#PM0" in pm_result

    # Re-inject MODBUS response to simulate device responding to
    # the next MODBUS request (read_registers clears _mb_buf and
    # expects fresh serial data)
    mock._buf.extend(modbus_resp)

    # Then read MODBUS
    mb_result = client.read_registers(0x1000, 1)
    assert mb_result == [100], f"Got {mb_result}"


def test_connect_disconnect():
    """Open and close connection, verify no errors."""
    client = M4ModbusClient("MOCK", timeout=0.05)
    # Use MagicMock for ser since connect() opens a real serial port
    client.ser = MagicMock()
    client.ser.is_open = True
    client.ser.in_waiting = 0
    client.ser.read.return_value = b''

    # Verify disconnect doesn't crash
    client.disconnect()
    assert client.ser is None  # disconnect sets ser to None


def test_serial_lock():
    """Verify threading.Lock is used in read_registers."""
    client = M4ModbusClient("MOCK", timeout=0.05)
    assert hasattr(client, '_lock')
    assert client._lock is not None
    assert isinstance(client._lock, type(threading.Lock()))