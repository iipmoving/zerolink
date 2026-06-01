#!/usr/bin/env python3
"""
M4 半桥电磁炉 MODBUS RTU 通信工具
====================================
功能: PC ↔ M4 半桥控制器 MODBUS 通信
  - 读取遥测寄存器 (0x1000-0x1014: 电压/电流/功率/相位/频率等)
  - 写入控制寄存器 (0x2010: 目标功率, 0x200E: 工作状态)
  - CSV 数据记录
  - 实时数据监视

M4 MODBUS 从站参数:
  - 物理层: UART2, 115200 baud, 8N1
  - 从站地址: 5 (Slave1, 可通过 0x3001 修改)
  - CRC: MODBUS CRC-16, 低字节优先 (标准)

用法:
  python m4_modbus_tool.py  COM3          # 交互模式
  python m4_modbus_tool.py  COM3 --read   # 单次读取
  python m4_modbus_tool.py  COM3 --log    # 持续记录到CSV
  python m4_modbus_tool.py  COM3 --power 1000  # 设定1000W
  python m4_modbus_tool.py  COM3 --on     # 启动加热
  python m4_modbus_tool.py  COM3 --off    # 停止加热
"""

import sys
import time
import csv
import argparse
import threading
from datetime import datetime
from collections import OrderedDict

# ============================================================
# MODBUS 寄存器映射 (与 M4 固件 Modbus_Lib_Init_An_Analysis.c 一致)
# ============================================================

# --- 只读遥测寄存器: 0x1000-0x1014 ---
# 顺序严格对应 I2C 读状态 0x10-0x1F + MODBUS 扩展 0x1010-0x1014
# 常用轮询区 (0x1000-0x1008): 每次心跳 FC03 必读 (I2C 0x10-0x18)
# 调试区 (0x1009-0x1014): 按需读取 (I2C 0x19-0x1F + MODBUS 扩展)
READ_REGS = OrderedDict([
    # I2C 映射区 (0x1000-0x100F): 严格按 I2C 读状态顺序
    (0x1000, ("sys_sta",        "raw",  1,    "IHStatus: BIT7=初始化, BIT3-0=炉头号")),
    (0x1001, ("vol_ad",         "raw",  1,    "VoltageValue: 电压 ADC")),
    (0x1002, ("cur_ad",         "raw",  1,    "CurrentValue: 电流 ADC")),
    (0x1003, ("igbt_ad",        "raw",  1,    "Sensor1Value: IGBT 温度 ADC")),
    (0x1004, ("bot_ad",         "raw",  1,    "Sensor2Value: 炉面温度 ADC")),
    (0x1005, ("top_ad",         "raw",  1,    "Sensor3Value: 顶部温度 ADC (预留)")),
    (0x1006, ("power_w",        "W",    1,    "ActualPower: 实际功率 (=raw×25W)")),
    (0x1007, ("target_power_rd","x25W", 25,   "TargetPower: 目标功率回读")),
    (0x1008, ("ppg",            "raw",  1,    "ActualPPG: 实际加热 PPG")),
    (0x1009, ("power_limit",    "raw",  1,    "PowerStatus: 功率限制状态 (低4位)")),
    (0x100A, ("pan_pulse",      "raw",  1,    "LoadValue: 检锅脉冲(低4位)+浪涌(高4位)")),
    (0x100B, ("hv_cnt",         "raw",  1,    "VCNTValue: 反压计数器")),
    (0x100C, ("pwm_l",          "raw",  1,    "PWMValue_L: 频率限制值低位 (预留)")),
    (0x100D, ("pwm_h",          "raw",  1,    "PWMValue_H: 频率限制值高位 (预留)")),
    (0x100E, ("power_adj",      "raw",  1,    "PowerAdjust: PPG修正值 (预留)")),
    (0x100F, ("version",        "raw",  1,    "Version: 主板版本号")),
    # MODBUS 扩展区 (0x1010-0x1014)
    (0x1010, ("fan_ad",         "raw",  1,    "风扇 ADC")),
    (0x1011, ("fault_code",     "raw",  1,    "故障码 (0=正常)")),
    (0x1012, ("internal_err",   "raw",  1,    "内部故障 (高4位浪涌)")),
    (0x1013, ("hz_cnt",         "raw",  1,    "频率计数器")),
    (0x1014, ("discard_cnt",    "raw",  1,    "丢波计数器")),
])

READ_START_ADDR = 0x1000
READ_COUNT = len(READ_REGS)  # 21 registers
READ_POLL_COUNT = 9  # 常用轮询区: 0x1000-0x1008 共9个寄存器

# --- EKF 遥测寄存器: 0x1020-0x1029 (独立节点, 新增) ---
EKF_REGS = OrderedDict([
    (0x1020, ("phase_01deg",  "0.1°",  1,    "相位角 (0.1°单位, int16)")),
    (0x1021, ("freq_hz_hi",   "raw",   1,    "频率高字 (uint32 Hz)")),
    (0x1022, ("freq_hz_lo",   "raw",   1,    "频率低字")),
    (0x1023, ("res_cur_adc",  "raw",   1,    "谐振电流 ADC")),
    (0x1024, ("ppg_period",   "raw",   1,    "HRTIM 周期 prioed")),
    (0x1025, ("ppg_duty",     "raw",   1,    "HRTIM 占空比")),
    (0x1026, ("delta_ppg",    "raw",   1,    "PID 增量 (int16)")),
    (0x1027, ("ekf_res1",     "raw",   1,    "保留")),
    (0x1028, ("ekf_res2",     "raw",   1,    "保留")),
    (0x1029, ("ekf_res3",     "raw",   1,    "保留")),
])
EKF_START_ADDR = 0x1020
EKF_COUNT = len(EKF_REGS)

# --- WaveCapture 0x5000 区 ---
CAPTURE_START_ADDR = 0x5000
CAPTURE_HEADER_WORDS = 6
CAPTURE_MAX_DATA_WORDS = 4000
CAPTURE_FRAME_WORDS = CAPTURE_HEADER_WORDS + CAPTURE_MAX_DATA_WORDS

# --- 可读写控制寄存器: 0x2000-0x2014 ---
WRITE_REGS = {
    "work_sta":     0x200E,  # 工作状态 (powerControlSet)
    "target_power": 0x2010,  # 目标功率 (powerSetm, 单位: 25W, 即 40 = 1000W)
    "fan_speed":    0x200F,  # 风扇转速 (fanSpeed)
    "jitter_freq":  0x2012,  # 抖频参数 (powerSwitch)
}

# --- 可读写系统设置: 0x3000-0x3003 ---
SYS_REGS = {
    "power_cal":  0x3000,  # 功率校准值 (36-96)
    "slave_addr": 0x3001,  # 从机地址
    "baud_rate":  0x3002,  # 波特率
    "save_order": 0x3003,  # 保存命令
}

# 工作状态位定义
WORK_STA_OFF = 0x0000   # 停止加热
WORK_STA_ON  = 0x0010   # 启动加热 (对应 powerSwitch=0x10)

# 风扇控制
FAN_SPEED_FULL = 0xAA   # 风扇全速值 (参考 work_power_out.c:1077)
FAN_CTRL_BIT    = 0x04  # 风机控制位 = bit 2 (参考 CTRL_SET 结构体)


def nibble_invert(low_nibble: int) -> int:
    """高低4位求反编码 (参考 s_comm.c:377-386)

    high_nibble = ~low_nibble & 0x0F
    返回完整字节: (high_nibble << 4) | low_nibble

    例: low_nibble=0x4 → high=0xB → 返回 0xB4
    """
    lo = low_nibble & 0x0F
    hi = (~lo) & 0x0F
    return (hi << 4) | lo


def make_work_sta_byte(heat_on: bool, fan_on: bool) -> int:
    """构造 Work_STA 控制字节 (带高低4位反码验证)

    低4位: Beep(2bit) | FAN(1bit=bit2) | freqJit(1bit)
    高4位: ~低4位

    heat_on: 置 bit4 (加热使能)
    fan_on:  置 bit2 (风机使能) + nibble 反码
    """
    lo_nibble = 0
    if fan_on:
        lo_nibble |= FAN_CTRL_BIT   # bit 2 = 风机
    ctrl_byte = nibble_invert(lo_nibble)
    if heat_on:
        ctrl_byte |= 0x10           # bit 4 = 加热使能 (独立于反码机制)
    return ctrl_byte


# ============================================================
# MODBUS 通信类
# ============================================================

class M4ModbusClient:
    """M4 半桥电磁炉 MODBUS RTU 客户端

    MODBUS 协议要点 (从 I2C 协议转换而来):
      - 每次写 0x2000 区域都会触发 Modbus_I2c_Data_Main()
        → API_UART_RxInitCallback() + API_UART_RxControlCallback()
      - 功率控制层内部有通讯超时看门狗, 超时未收到数据则触发
        PowerOffCommLost (0x2) 关机
      - SYS_STA (0x1000) bit 7: 0=未初始化, 1=工作中
      - 心跳间隔建议 ≤ 500ms (超时周期在 .lib 内部, 不可读)
    """

    HEARTBEAT_INTERVAL = 0.5  # 心跳间隔秒

    def __init__(self, port: str, baudrate: int = 115200,
                 slave_addr: int = 5, timeout: float = 0.5):
        self.port = port
        self.baudrate = baudrate
        self.slave_addr = slave_addr
        self.timeout = timeout
        self.client = None
        self._heartbeat_thread = None
        self._heartbeat_stop = None
        self._heartbeat_power_w = 0        # 0x2010 功率寄存器值 (W/25)
        self._heartbeat_work_sta = 0x0000  # 0x200E
        self._heartbeat_fan_speed = 0      # 0x200F
        self._heartbeat_fan_on = False     # 风机使能标志
        self._heartbeat_jitter = 0x0000    # 0x2012 powerSwitch (0=关机, 2=关功率检锅)

    def connect(self) -> bool:
        """建立 MODBUS RTU 连接"""
        try:
            from pymodbus.client import ModbusSerialClient
            from pymodbus.framer import FramerRTU
        except ImportError as e:
            import traceback
            traceback.print_exc()
            print(f"[错误] MODBUS 库导入失败: {e}")
            print("  请执行: python -m pip install pymodbus pyserial")
            return False

        # pymodbus 3.13 FramerRTU 硬编码 CRC 大端, 但 M4 设备用标准
        # MODBUS CRC 小端。Patch 编解码方法。
        self._patch_framer_crc(FramerRTU)

        try:
            self.client = ModbusSerialClient(
                port=self.port,
                baudrate=self.baudrate,
                bytesize=8,
                parity='N',
                stopbits=1,
                timeout=self.timeout,
            )
        except RuntimeError as e:
            print(f"[错误] {e}")
            return False
        except Exception as e:
            import traceback
            traceback.print_exc()
            print(f"[错误] 创建 MODBUS 客户端失败: {e}")
            return False

        if self.client.connect():
            print(f"[OK] 已连接 {self.port} @ {self.baudrate} baud, 从站={self.slave_addr}")
            return True
        else:
            print(f"[失败] 无法打开串口 {self.port}")
            return False

    @staticmethod
    def _patch_framer_crc(framer_cls):
        """修正 FramerRTU CRC 字节序: big-endian → little-endian"""
        if getattr(framer_cls, '_crc_patched', False):
            return

        _encode_orig = framer_cls.encode
        _decode_orig = framer_cls.decode

        def encode_le(self, payload, device_id, _tid):
            frame = device_id.to_bytes(1, 'big') + payload
            crc = framer_cls.compute_CRC(frame)
            return frame + crc.to_bytes(2, 'little')

        def decode_le(self, data):
            from pymodbus.logging import Log
            data_len = len(data)
            for used_len in range(data_len):
                if data_len - used_len < self.MIN_SIZE:
                    Log.debug("Short frame: {} wait for more data", data, ":hex")
                    return 0, 0, 0, self.EMPTY
                dev_id = int(data[used_len])
                if self.device_ids and dev_id not in self.device_ids:
                    return data_len, 0, 0, self.EMPTY
                if not (pdu_class := self.decoder.lookupPduClass(
                        data[used_len:])):
                    continue
                if not (size := pdu_class.calculateRtuFrameSize(
                        data[used_len:])):
                    Log.debug("Frame - rtu_byte_count_pos wrong")
                    return 0, dev_id, 0, self.EMPTY
                if data_len < used_len + size:
                    Log.debug("Frame - not ready")
                    return 0, dev_id, 0, self.EMPTY
                for test_len in range(data_len, used_len + size - 1, -1):
                    start_crc = test_len - 2
                    crc = data[start_crc : start_crc + 2]
                    crc_val = (int(crc[1]) << 8) + int(crc[0])
                    if not framer_cls.check_CRC(
                            data[used_len : start_crc], crc_val):
                        Log.debug("Frame check failed...")
                        continue
                    return (data_len, dev_id, 0,
                            data[used_len + 1 : start_crc])
            return 0, 0, 0, self.EMPTY

        framer_cls.encode = encode_le
        framer_cls.decode = decode_le
        framer_cls._crc_patched = True

    def disconnect(self):
        self.stop_heartbeat()
        if self.client:
            self.client.close()
            print("[OK] 已断开连接")

    def read_registers(self, start_addr: int, count: int) -> list | None:
        """读取保持寄存器 (功能码 0x03)"""
        if not self.client:
            print("[错误] 未连接")
            return None
        try:
            rr = self.client.read_holding_registers(
                start_addr, count=count, device_id=self.slave_addr
            )
            if rr.isError():
                print(f"[MODBUS错误] 读寄存器 0x{start_addr:04X}: {rr}")
                return None
            return list(rr.registers)
        except Exception as e:
            print(f"[通信错误] 读寄存器: {e}")
            return None

    def write_register(self, addr: int, value: int) -> bool:
        """写单个保持寄存器 (功能码 0x06)"""
        if not self.client:
            print("[错误] 未连接")
            return False
        try:
            rr = self.client.write_register(addr, value, device_id=self.slave_addr)
            if rr.isError():
                print(f"[MODBUS错误] 写寄存器 0x{addr:04X}={value}: {rr}")
                return False
            return True
        except Exception as e:
            print(f"[通信错误] 写寄存器: {e}")
            return False

    def write_registers(self, addr: int, values: list) -> bool:
        """写多个保持寄存器 (功能码 0x10).
        Args:
            addr: 起始寄存器地址
            values: 寄存器值列表 (每个元素为 16-bit)
        """
        if not self.client:
            print("[错误] 未连接")
            return False
        try:
            rr = self.client.write_registers(addr, values, device_id=self.slave_addr)
            if rr.isError():
                print(f"[MODBUS错误] 批量写 0x{addr:04X}: {rr}")
                return False
            return True
        except Exception as e:
            print(f"[通信错误] 批量写寄存器: {e}")
            return False

    def read_telemetry(self) -> dict | None:
        """读取全部遥测数据, 返回命名字典"""
        raw = self.read_registers(READ_START_ADDR, READ_COUNT)
        if raw is None:
            return None
        result = {"timestamp": datetime.now().isoformat(timespec='milliseconds')}
        for i, (addr, (name, unit, scale, _desc)) in enumerate(READ_REGS.items()):
            raw_val = raw[i]
            result[name] = raw_val * scale
            result[f"{name}_raw"] = raw_val
        return result

    def read_ekf_telemetry(self) -> dict | None:
        """读取 EKF 遥测寄存器块 (0x1020-0x1029), 返回命名字典"""
        raw = self.read_registers(EKF_START_ADDR, EKF_COUNT)
        if raw is None:
            return None
        result = {"timestamp": datetime.now().isoformat(timespec='milliseconds')}
        for i, (addr, (name, unit, scale, _desc)) in enumerate(EKF_REGS.items()):
            raw_val = raw[i]
            result[name] = raw_val * scale
        # 解析频率: uint32 = hi<<16 | lo
        freq_hi = result.get("freq_hz_hi", 0)
        freq_lo = result.get("freq_hz_lo", 0)
        result["freq_hz"] = (freq_hi << 16) | freq_lo
        # 解析相位 (int16)
        phase_raw = result.get("phase_01deg", 0)
        if phase_raw > 32767:
            result["phase_deg"] = (phase_raw - 65536) / 10.0
        else:
            result["phase_deg"] = phase_raw / 10.0
        # 解析 PID 增量 (int16)
        delta_raw = result.get("delta_ppg", 0)
        if delta_raw > 32767:
            result["delta_ppg_signed"] = delta_raw - 65536
        else:
            result["delta_ppg_signed"] = delta_raw
        return result

    # ---- WaveCapture 0x5000 读取 ----
    MAX_READ_WORDS = 120  # 响应 ≤ 245B (MCU RX buf = 256)

    def read_capture(self, max_data_words: int = 2000) -> dict | None:
        """读取 WaveCapture 0x5000 区: 先读帧头6字, 再按需读数据体."""
        # Step 1: 读帧头
        raw = self.read_registers(CAPTURE_START_ADDR, CAPTURE_HEADER_WORDS)
        if raw is None:
            return None
        header = {
            "ack":        raw[0],
            "status":     raw[1],
            "frame_id":   raw[2],
            "count":      raw[3],
            "data_words": raw[4],
            "max_frames": raw[5],
        }

        # Step 2: 读数据体 (分块)
        data_words = min(header["data_words"], max_data_words,
                         CAPTURE_MAX_DATA_WORDS)
        if data_words == 0:
            return {"header": header, "data": []}

        data = []
        data_start = CAPTURE_START_ADDR + CAPTURE_HEADER_WORDS
        remaining = data_words
        offset = 0
        while remaining > 0:
            chunk = min(remaining, self.MAX_READ_WORDS)
            raw = self.read_registers(data_start + offset, chunk)
            if raw is None:
                break
            data.extend(raw)
            offset += chunk
            remaining -= chunk

        return {"header": header, "data": data}

    # ---- 控制批下发 (FC10, 模拟 I2C 一次性传输) ----

    def _send_control_batch(self) -> bool:
        """FC10 批量写 5 个连续寄存器 (0x200E-0x2012)
        对应固件 PowerControlDef 结构, 必须一次下发完整结构体,
        单寄存器 FC06 会被最小长度检查拦截 (I2C 遗留)。

        结构: {powerControlSet, fanSpeed, powerSetm, intermittentHeat, powerSwitch}
              0x200E            0x200F     0x2010      0x2011             0x2012
        """
        fan_val = FAN_SPEED_FULL if self._heartbeat_fan_on else self._heartbeat_fan_speed
        return self.write_registers(0x200E, [
            self._heartbeat_work_sta,   # 0x200E Work_STA (保持原值)
            fan_val,                    # 0x200F FAN_Speed (0xAA=全速)
            self._heartbeat_power_w,    # 0x2010 target_Power (powerSetm)
            0x0000,                     # 0x2011 IntermittentHeat
            self._heartbeat_jitter,     # 0x2012 jitter_freq (powerSwitch)
        ])

    def set_power(self, power_watt: int) -> bool:
        """设定目标功率 (单位: W, 自动转为 25W 单位) — 更新内部状态, 心跳下发"""
        if power_watt < 0 or power_watt > 3000:
            print(f"[错误] 功率超出范围: {power_watt}W (0-3000W)")
            return False
        val_25w = max(power_watt // 25, 1) if power_watt > 0 else 0
        print(f"  设定功率: {power_watt}W -> 寄存器值={val_25w} (x25W), 心跳下发")
        self._heartbeat_power_w = val_25w
        return True

    def set_work_sta(self, on: bool, fan_on: bool = False) -> bool:
        """启动/停止加热, 可选开启风机 — 立即 FC10 批下发"""
        fan = fan_on or (on and self._heartbeat_fan_on)
        action = "启动加热" if on else "停止加热"
        if fan:
            action += " + 风机"
            self._heartbeat_fan_speed = FAN_SPEED_FULL
        self._heartbeat_work_sta = WORK_STA_ON if on else WORK_STA_OFF
        self._heartbeat_fan_on = fan
        self._heartbeat_jitter = 0x0002 if on else 0x0000  # powerSwitch: 2=检锅, 0=关机
        fs = FAN_SPEED_FULL if fan else self._heartbeat_fan_speed
        print(f"  {action}: FC10 批下发 5regs "
              f"(Work_STA=0x{self._heartbeat_work_sta:04X}, "
              f"pwr={self._heartbeat_power_w}, fan=0x{fs:02X}, "
              f"jitter={self._heartbeat_jitter})")
        return self._send_control_batch()

    # ---- 心跳 / 协议状态机 ----

    def get_sys_sta(self) -> int | None:
        """读取 SYS_STA (0x1000) 原始值, 返回 None 表示通信失败"""
        raw = self.read_registers(0x1000, count=1)
        if raw is None:
            return None
        return raw[0]

    def is_initialized(self) -> bool | None:
        """检查设备是否已初始化 (SYS_STA bit 7 = 1).
        返回 None 表示通信失败."""
        sta = self.get_sys_sta()
        if sta is None:
            return None
        return bool(sta & 0x80)

    def read_polling_block(self) -> dict | None:
        """快速轮询: FC03 读 0x1000 起 9 个寄存器 (常用轮询区)
        返回命名字典, 包含 sys_sta/vol/cur/igbt/bot/top/power/target_power_rd/ppg
        """
        raw = self.read_registers(READ_START_ADDR, READ_POLL_COUNT)
        if raw is None:
            return None
        result = {}
        i = 0
        for addr, (name, unit, scale, _desc) in READ_REGS.items():
            if i >= READ_POLL_COUNT:
                break
            result[name] = raw[i] * scale
            i += 1
        return result

    def check_and_init(self, verbose: bool = True,
                       retry_limit: int = 20,
                       retry_interval: float = 0.3) -> bool:
        """读取 SYS_STA, 按协议状态机执行:
        - bit 7 = 0: 设备未初始化, 发送空写触发初始化, 轮询直到 bit 7=1
        - bit 7 = 1: 设备已就绪, 无需操作
        - 通信失败: 返回 False
        返回: 设备是否就绪 (可接受功率指令和EKF回读)
        """
        for attempt in range(retry_limit):
            sys_sta = self.get_sys_sta()
            if sys_sta is None:
                if verbose:
                    print(f"  [初始化] 通信失败 (尝试 {attempt+1}/{retry_limit})")
                time.sleep(retry_interval)
                continue

            head = sys_sta & 0x0F
            is_init = bool(sys_sta & 0x80)

            if is_init:
                if verbose:
                    print(f"  SYS_STA=0x{sys_sta:02X} (已初始化, 炉头={head})")
                return True

            if verbose:
                print(f"  SYS_STA=0x{sys_sta:02X} (未初始化, 炉头={head}), "
                      f"发送初始化帧...")

            # FC10 批写全零触发 Modbus_I2c_Data_Main() → init callback
            self.write_registers(0x200E, [0x0000, 0x0000, 0x0000, 0x0000, 0x0000])
            time.sleep(retry_interval)

        if verbose:
            print("  [初始化] 超时: SYS_STA bit 7 未置位, 继续操作")
        return True  # 超时也继续, 允许手动操作

    def send_heartbeat(self) -> bool:
        """发送心跳帧: 与 set_power/set_work_sta 使用同一 FC10 批下发路径"""
        return self._send_control_batch()

    def start_heartbeat(self, power_watt: int = 0,
                        fan_on: bool = True,
                        interval_s: float | None = None) -> bool:
        """启动后台心跳线程.

        Args:
            power_watt: 当前目标功率 (W), 用于周期性重写
            fan_on: 是否开启风机 (默认开, 0xAA 全速)
            interval_s: 心跳间隔, 默认 HEARTBEAT_INTERVAL (0.5s)

        Returns: True if started, False if already running.
        """
        if self._heartbeat_thread and self._heartbeat_thread.is_alive():
            return False

        if interval_s is None:
            interval_s = self.HEARTBEAT_INTERVAL

        self._heartbeat_power_w = max(power_watt // 25, 0) if power_watt > 0 else 0
        self._heartbeat_fan_on = fan_on
        self._heartbeat_fan_speed = FAN_SPEED_FULL if fan_on else 0
        self._heartbeat_stop = threading.Event()

        def _worker():
            while not self._heartbeat_stop.wait(interval_s):
                try:
                    self.send_heartbeat()
                except Exception:
                    pass  # 心跳丢失一次不致命

        self._heartbeat_thread = threading.Thread(target=_worker, daemon=True)
        self._heartbeat_thread.start()
        return True

    def stop_heartbeat(self):
        """停止后台心跳线程"""
        if self._heartbeat_stop:
            self._heartbeat_stop.set()
        if self._heartbeat_thread:
            self._heartbeat_thread.join(timeout=2)
            self._heartbeat_thread = None
            self._heartbeat_stop = None

    def update_heartbeat_power(self, power_watt: int):
        """更新心跳中的功率值 (不重启心跳线程)"""
        self._heartbeat_power_w = max(power_watt // 25, 0) if power_watt > 0 else 0

    def update_heartbeat_fan(self, fan_val: int):
        """更新心跳中的风扇转速"""
        self._heartbeat_fan_speed = fan_val & 0xFFFF

    def update_heartbeat_jitter(self, jitter_val: int):
        """更新心跳中的抖频参数"""
        self._heartbeat_jitter = jitter_val & 0xFFFF


# ============================================================
# 数据记录器
# ============================================================

class DataLogger:
    """CSV 数据记录器"""

    def __init__(self, filepath: str):
        self.filepath = filepath
        self.file = None
        self.writer = None
        self.record_count = 0

    def open(self, fieldnames: list):
        self.file = open(self.filepath, 'w', newline='', encoding='utf-8')
        self.writer = csv.DictWriter(self.file, fieldnames=fieldnames)
        self.writer.writeheader()
        print(f"[记录] CSV 文件: {self.filepath}")

    def write(self, row: dict):
        if self.writer:
            self.writer.writerow(row)
            self.record_count += 1

    def close(self):
        if self.file:
            self.file.close()
            print(f"[记录] 已保存 {self.record_count} 条记录到 {self.filepath}")


# ============================================================
# 实时绘图 (可选依赖)
# ============================================================

try:
    from plot_utils import LivePlotter
    HAS_PLOT = True
except ImportError:
    HAS_PLOT = False


# ============================================================
# 交互式控制台
# ============================================================

def interactive_mode(client: M4ModbusClient):
    """交互式命令菜单"""
    menu = """
┌──────────────────────────────────────────────┐
│  M4 半桥电磁炉 MODBUS 通信工具               │
├──────────────────────────────────────────────┤
│  r     读取全部遥测寄存器                     │
│  ekf   读取 EKF 遥测寄存器 (0x1020)           │
│  p N   设定功率为 N 瓦 (例: p 1000)           │
│  on    启动加热                                │
│  on fan 启动加热 + 风机全速 (0xAA)            │
│  off   停止加热                                │
│  fan   切换风机 ON/OFF                         │
│  log   开始 CSV 记录 (每100ms一条)             │
│  stop  停止 CSV 记录                           │
│  mon   实时监视 (50ms刷新, s/e切换遥测)        │
│  mon plot  实时监视 + 波形图                   │
│  q     退出                                    │
└──────────────────────────────────────────────┘
"""
    print(menu)
    logger = None
    log_thread = None
    stop_log = threading.Event()

    def _log_worker():
        """后台记录线程"""
        while not stop_log.is_set():
            data = client.read_telemetry()
            if data and logger:
                logger.write(data)
                sys.stdout.write(f"\r  已记录: {logger.record_count} 条  ")
                sys.stdout.flush()
            time.sleep(0.1)  # 100ms 间隔

    while True:
        try:
            cmd = input("\n> ").strip().lower()
        except (EOFError, KeyboardInterrupt):
            break

        if not cmd:
            continue

        parts = cmd.split()

        if parts[0] == 'r':
            # 读取遥测
            data = client.read_telemetry()
            if data:
                print(f"\n  === M4 遥测数据 @ {data['timestamp']} ===")
                for addr, (name, unit, scale, desc) in READ_REGS.items():
                    raw_val = data.get(f"{name}_raw", 0)
                    disp_val = data.get(name, 0)
                    print(f"  0x{addr:04X} {name:16s} = {raw_val:5d} ({disp_val:6.1f} {unit:4s}) | {desc}")

        elif parts[0] == 'ekf':
            # 读取 EKF 遥测
            ekf = client.read_ekf_telemetry()
            if ekf:
                print(f"\n  === EKF 遥测 @ {ekf['timestamp']} ===")
                for addr, (name, unit, _scale, desc) in EKF_REGS.items():
                    val = ekf.get(name, 0)
                    print(f"  0x{addr:04X} {name:16s} = {val:5d} ({unit:4s}) | {desc}")
                print(f"  --- 解析值 ---")
                print(f"  freq_hz          = {ekf.get('freq_hz', 0)} Hz")
                print(f"  phase_deg        = {ekf.get('phase_deg', 0):.1f} °")
                print(f"  delta_ppg_signed = {ekf.get('delta_ppg_signed', 0)}")

        elif parts[0] == 'p' and len(parts) >= 2:
            # 设定功率
            try:
                watts = int(parts[1])
                client.set_power(watts)
                client.update_heartbeat_power(watts)
            except ValueError:
                print("  用法: p <瓦特数>  例: p 1000")

        elif parts[0] == 'on':
            # 先检查设备初始化状态
            client.check_and_init()
            fan = len(parts) >= 2 and parts[1] == 'fan'
            client.set_work_sta(True, fan_on=fan)
            # 启动心跳, 防止通讯超时关机
            if not client._heartbeat_thread or \
               not client._heartbeat_thread.is_alive():
                client.start_heartbeat(power_watt=0, fan_on=fan)

        elif parts[0] == 'fan':
            # 切换风机状态
            current = client._heartbeat_fan_on
            new_state = not current
            client._heartbeat_fan_on = new_state
            client._heartbeat_fan_speed = FAN_SPEED_FULL if new_state else 0
            print(f"  风机: {'ON (0xAA 全速)' if new_state else 'OFF'}")
            # 如果心跳在跑, 下一跳自动更新

        elif parts[0] == 'off':
            client.stop_heartbeat()
            client.set_work_sta(False, fan_on=False)

        elif parts[0] == 'log':
            # 开始记录
            if logger:
                print("  已在记录中, 先执行 stop")
                continue
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filepath = f"m4_data_{timestamp}.csv"
            fieldnames = ["timestamp"] + [name for _addr, (name, _u, _s, _d) in READ_REGS.items()]
            logger = DataLogger(filepath)
            logger.open(fieldnames)
            stop_log.clear()
            log_thread = threading.Thread(target=_log_worker, daemon=True)
            log_thread.start()
            print("  [记录] 开始后台记录 (100ms间隔)")

        elif parts[0] == 'stop':
            # 停止记录
            if log_thread:
                stop_log.set()
                log_thread.join(timeout=2)
                log_thread = None
            if logger:
                logger.close()
                logger = None

        elif parts[0] == 'mon':
            # 实时监视
            plotter = None
            if len(parts) >= 2 and parts[1] == 'plot':
                if not HAS_PLOT:
                    print("  [警告] matplotlib 未安装")
                else:
                    plotter = LivePlotter(window_s=30, sample_interval_s=0.1)
                    plotter.open()
                    print("  [绘图] 波形窗口已开启")

            # 遥测选择状态
            mon_std = True   # 标准遥测
            mon_ekf = True   # EKF 遥测

            sample_interval = 0.1 if plotter else 0.05  # 默认50ms刷新
            print(f"  实时监视 (Ctrl+C 停止, 采样={sample_interval}s)")
            print(f"  遥测: 标准={'ON' if mon_std else 'OFF'}  EKF={'ON' if mon_ekf else 'OFF'}")
            print(f"  [s]切换标准  [e]切换EKF  其他键停止")
            try:
                import select
                while True:
                    data = client.read_telemetry() if mon_std else None
                    ekf = client.read_ekf_telemetry() if mon_ekf else None
                    if data:
                        power = data.get("power_w", 0)
                        vol = data.get("vol_ad", 0)
                        cur = data.get("cur_ad", 0)
                        fault = data.get("fault_code", 0)
                        freq_hz = ekf.get("freq_hz", 0) if ekf else 0
                        phase_deg = ekf.get("phase_deg", 0) if ekf else 0
                        delta = ekf.get("delta_ppg_signed", 0) if ekf else 0

                        sys.stdout.write(
                            f"\r  P={power:5.0f}W | V_ad={vol:3d} I_ad={cur:3d} | "
                            f"f={freq_hz:5.0f}Hz ph={phase_deg:5.1f}° "
                            f"dPPG={delta:+4d} | fault={fault:#04X}  "
                        )
                        sys.stdout.flush()

                        if plotter:
                            plotter.feed(power_w=power, freq_hz=freq_hz,
                                        phase_deg=phase_deg, delta_ppg=delta)
                            plotter.update_plot()

                    # 检查键盘输入 (非阻塞)
                    if sys.platform == 'win32':
                        import msvcrt
                        if msvcrt.kbhit():
                            key = msvcrt.getch().decode('utf-8', errors='ignore').lower()
                            if key == 's':
                                mon_std = not mon_std
                                print(f"\n  标准遥测: {'ON' if mon_std else 'OFF'}")
                            elif key == 'e':
                                mon_ekf = not mon_ekf
                                print(f"\n  EKF遥测: {'ON' if mon_ekf else 'OFF'}")
                            else:
                                break
                    else:
                        import termios, tty
                        fd = sys.stdin.fileno()
                        old = termios.tcgetattr(fd)
                        try:
                            tty.setraw(fd)
                            r, _, _ = select.select([sys.stdin], [], [], 0)
                            if r:
                                key = sys.stdin.read(1).lower()
                                if key == 's':
                                    mon_std = not mon_std
                                    print(f"\n  标准遥测: {'ON' if mon_std else 'OFF'}")
                                elif key == 'e':
                                    mon_ekf = not mon_ekf
                                    print(f"\n  EKF遥测: {'ON' if mon_ekf else 'OFF'}")
                                else:
                                    break
                        finally:
                            termios.tcsetattr(fd, termios.TCSADRAIN, old)
                    time.sleep(sample_interval)
            except KeyboardInterrupt:
                print("\n  监视停止")
            finally:
                if plotter:
                    plotter.close()

        elif parts[0] == 'q':
            break

    # 清理
    if log_thread:
        stop_log.set()
        log_thread.join(timeout=2)
    if logger:
        logger.close()
    print("退出.")


# ============================================================
# 命令行入口
# ============================================================

def main():
    parser = argparse.ArgumentParser(
        description="M4 半桥电磁炉 MODBUS RTU 通信工具",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  python m4_modbus_tool.py COM3              # 交互模式
  python m4_modbus_tool.py COM3 --read        # 单次读取遥测
  python m4_modbus_tool.py COM3 --power 1000  # 设定1000W
  python m4_modbus_tool.py COM3 --on          # 启动加热
  python m4_modbus_tool.py COM3 --off         # 停止加热
  python m4_modbus_tool.py COM3 --log 30      # 记录30秒数据
        """
    )
    parser.add_argument("port", help="串口 (例: COM3 或 /dev/ttyUSB0)")
    parser.add_argument("-b", "--baudrate", type=int, default=115200,
                        help="波特率 (默认: 115200)")
    parser.add_argument("-s", "--slave", type=int, default=5,
                        help="MODBUS 从站地址 (默认: 5)")
    parser.add_argument("-t", "--timeout", type=float, default=0.5,
                        help="通信超时秒数 (默认: 0.5)")

    # 操作模式
    parser.add_argument("--read", action="store_true", help="单次读取遥测并退出")
    parser.add_argument("--power", type=int, metavar="W", help="设定目标功率(W)")
    parser.add_argument("--on", action="store_true", help="启动加热")
    parser.add_argument("--off", action="store_true", help="停止加热")
    parser.add_argument("--fan", action="store_true", help="启动加热时开启风机 (0xAA 全速)")
    parser.add_argument("--log", type=int, metavar="SEC", const=60, nargs='?',
                        help="记录数据N秒 (默认60秒)")
    parser.add_argument("--csv", type=str, metavar="FILE", help="CSV输出文件路径")
    parser.add_argument("--monitor", action="store_true", help="实时监视模式")
    parser.add_argument("--plot", action="store_true",
                        help="监视时开启实时波形图 (需 matplotlib)")
    parser.add_argument("--plot-window", type=float, default=30, metavar="SEC",
                        help="波形图时间窗口秒数 (默认: 30)")
    parser.add_argument("--read-ekf", action="store_true",
                        help="单次读取 EKF 遥测寄存器 (0x1020)")

    args = parser.parse_args()

    # 连接
    client = M4ModbusClient(args.port, args.baudrate, args.slave, args.timeout)
    if not client.connect():
        return 1

    try:
        # 检查设备初始化状态
        client.check_and_init()

        # 处理写操作 (先写,因为--read和--power可同时使用)
        if args.power is not None:
            client.set_power(args.power)
            time.sleep(0.1)

        if args.on:
            fan = args.fan
            client.set_work_sta(True, fan_on=fan)
            client.start_heartbeat(power_watt=args.power or 0, fan_on=fan)
            time.sleep(0.1)

        if args.off:
            client.stop_heartbeat()
            client.set_work_sta(False)
            time.sleep(0.1)

        # 处理读操作
        if args.read or args.read_ekf or (args.power is None and not args.on and not args.off
                         and not args.log and not args.monitor):
            # 默认交互模式, 或显式 --read / --read-ekf
            if args.read:
                data = client.read_telemetry()
                if data:
                    print(f"\n=== M4 遥测 @ {data['timestamp']} ===")
                    for addr, (name, unit, _scale, desc) in READ_REGS.items():
                        raw = data.get(f"{name}_raw", data.get(name, 0))
                        val = data.get(name, 0)
                        print(f"  0x{addr:04X} {name:16s} = {raw:5d} ({val:6.1f} {unit:4s}) | {desc}")
            elif args.read_ekf:
                ekf = client.read_ekf_telemetry()
                if ekf:
                    print(f"\n=== EKF 遥测 @ {ekf['timestamp']} ===")
                    for addr, (name, unit, _scale, desc) in EKF_REGS.items():
                        val = ekf.get(name, 0)
                        print(f"  0x{addr:04X} {name:16s} = {val:5d} ({unit:4s}) | {desc}")
                    print(f"  --- 解析值 ---")
                    print(f"  freq_hz         = {ekf.get('freq_hz', 0)} Hz")
                    print(f"  phase_deg       = {ekf.get('phase_deg', 0):.1f} °")
                    print(f"  delta_ppg_signed= {ekf.get('delta_ppg_signed', 0)}")
            else:
                interactive_mode(client)

        elif args.log is not None:
            # 记录模式
            duration = args.log if args.log > 0 else 60
            csv_file = args.csv or f"m4_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
            fieldnames = ["timestamp"] + [name for _addr, (name, _u, _s, _d) in READ_REGS.items()]
            logger = DataLogger(csv_file)
            logger.open(fieldnames)
            print(f"记录 {duration} 秒, 每100ms采样...")
            t_start = time.time()
            try:
                while time.time() - t_start < duration:
                    data = client.read_telemetry()
                    if data:
                        logger.write(data)
                    time.sleep(0.1)
            except KeyboardInterrupt:
                print("\n用户中断")
            logger.close()

        elif args.monitor:
            # 实时监视 (可选 --plot 开启波形图)
            plotter = None
            if args.plot:
                if not HAS_PLOT:
                    print("[警告] matplotlib 未安装, 无法绘图。 pip install matplotlib")
                else:
                    plotter = LivePlotter(window_s=args.plot_window,
                                         sample_interval_s=0.1)
                    plotter.open()
                    print(f"[绘图] 波形窗口已开启 (时间窗={args.plot_window}s)")

            sample_interval = 0.1 if plotter else 1.0
            print(f"实时监视 (Ctrl+C 停止) [采样={sample_interval}s]...")
            try:
                while True:
                    data = client.read_telemetry()
                    ekf = client.read_ekf_telemetry()

                    if data:
                        power = data.get("power_w", 0)
                        vol = data.get("vol_ad", 0)
                        cur = data.get("cur_ad", 0)
                        phase = data.get("phase", 0)
                        hz = data.get("hz_cnt", 0)
                        fault = data.get("fault_code", 0)
                        freq_hz = ekf.get("freq_hz", 0) if ekf else 0
                        phase_deg = ekf.get("phase_deg", 0) if ekf else 0
                        delta = ekf.get("delta_ppg_signed", 0) if ekf else 0

                        sys.stdout.write(
                            f"\r  P={power:5.0f}W | V_ad={vol:3d} I_ad={cur:3d} | "
                            f"f={freq_hz:5.0f}Hz ph={phase_deg:5.1f}° "
                            f"dPPG={delta:+4d} | fault={fault:#04X}  "
                        )
                        sys.stdout.flush()

                        if plotter:
                            plotter.feed(power_w=power, freq_hz=freq_hz,
                                        phase_deg=phase_deg, delta_ppg=delta)
                            plotter.update_plot()

                    time.sleep(sample_interval)
            except KeyboardInterrupt:
                print("\n监视停止")
            finally:
                if plotter:
                    plotter.close()

    finally:
        client.disconnect()

    return 0


if __name__ == "__main__":
    sys.exit(main())
