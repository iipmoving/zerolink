"""
printmessage_decoder.py — PrintMessage UART 主动上报解码器

UART 输出格式 (来自 PrintMessageOut + PrintMessageFun):
  PAN_MESSAGE:
    "pan pluse is 12.\r\n"                          ← paraBuff.buff[0] = pulse_count
    "Index,para1,para2\r\n"                           ← CSV header
    "0\t1234\t5678\r\n"                                ← data rows
    "1\t1235\t5679\r\n"
    ...

  TXA/CURRENT_MESSAGE:
    "para0 is 123.phase Angle is 45,lowOn is 1,phaseUpHrtim is 100\r\n"
    "Index,para1,para2,para3,para4\r\n"
    "0\tval0\tval0\tval0\tval0\r\n"
    ...

解码器状态机:
  IDLE → HEADER_LINE → CSV_HEADER → DATA_ROWS → FINISHED
"""

import re
from typing import Optional


class PrintMessageDecoder:
    """PrintMessage 帧解码器 — 状态机解析"""

    # 消息类型
    MSG_PAN = 0
    MSG_TXA = 1
    MSG_CURRENT = 2

    def __init__(self):
        self.clear()

    def clear(self):
        self._state = "IDLE"
        self._msg_type = None
        self._summary = {}       # 标量参数: pulse_count, para0..para3
        self._csv_header = []    # ["Index", "para1", "para2"]
        self._csv_rows = []      # [{"Index": 0, "para1": 1234, ...}, ...]
        self._raw_lines = []     # 原始行缓存

    # ── 状态机 ──────────────────────────────────────────

    def feed_line(self, line: str) -> Optional[dict]:
        """喂一行 UART 数据。返回解码结果或 None"""
        line = line.strip()
        if not line:
            return None

        self._raw_lines.append(line)

        if self._state == "IDLE":
            return self._on_idle(line)
        elif self._state == "HEADER_LINE":
            return self._on_header(line)
        elif self._state == "CSV_HEADER":
            return self._on_csv_header(line)
        elif self._state == "DATA_ROWS":
            return self._on_data_row(line)
        return None

    def _on_idle(self, line: str) -> Optional[dict]:
        """检测消息头: #PM0=PAN, #PM1=TXA, #PM2=CURRENT"""
        m = re.match(r'^#PM(\d+)', line)
        if m:
            self._msg_type = int(m.group(1))
            self._state = "CSV_HEADER"
            return None

        # 旧格式兼容 (无报头时用内容判断)
        m = re.match(r'pan pluse is (\d+)', line)
        if m:
            self._msg_type = self.MSG_PAN
            self._summary["pulse"] = int(m.group(1))
            self._state = "CSV_HEADER"
            return None

        # 其他消息: "para0 is N.phase Angle is N..."
        m = re.match(r'para0 is ([\d.-]+).*phase Angle is ([\d.-]+)', line)
        if m:
            self._msg_type = self.MSG_TXA  # 或 CURRENT, 由后续数据区分
            self._summary["para0"] = float(m.group(1))
            self._summary["phase"] = float(m.group(2))
            self._state = "CSV_HEADER"
            return None

        return None

    def _on_header(self, line: str) -> Optional[dict]:
        """首行信息 (部分消息类型可能有额外首行, 目前未用到)"""
        self._state = "CSV_HEADER"
        return self._on_csv_header(line)

    def _on_csv_header(self, line: str) -> Optional[dict]:
        """解析 CSV 标题行: "Index,para1,para2,..." """
        if line.startswith("Index") or line.startswith("Index\t"):
            parts = re.split(r'[,\t]', line)
            self._csv_header = [p.strip() for p in parts if p.strip()]
            self._state = "DATA_ROWS"
        else:
            # 不是 CSV 标题，可能是另一类行，回到 IDLE
            self._state = "IDLE"
        return None

    def _on_data_row(self, line: str) -> Optional[dict]:
        """解析数据行: tab 分隔的数字"""
        parts = line.split("\t")
        row = {}
        for i, val in enumerate(parts):
            val = val.strip()
            if i < len(self._csv_header):
                try:
                    row[self._csv_header[i]] = int(val)
                except ValueError:
                    try:
                        row[self._csv_header[i]] = float(val)
                    except ValueError:
                        row[self._csv_header[i]] = val
            else:
                row[f"col_{i}"] = val
        self._csv_rows.append(row)
        # 持续解析直到遇到空行或非数字行
        return None

    # ── 结果提取 ────────────────────────────────────────

    def get_table(self) -> list:
        """返回解析后的 CSV 表格"""
        return list(self._csv_rows)

    def get_summary(self) -> dict:
        """返回消息摘要"""
        result = dict(self._summary)
        result["msg_type"] = self._msg_type
        result["rows"] = len(self._csv_rows)
        return result

    def is_frame_complete(self) -> bool:
        """判断当前帧是否已结束"""
        return (self._state in ("IDLE", "FINISHED", "DATA_ROWS")
                and bool(self._csv_rows)
                and len(self._csv_header) > 0)

    def get_latest_result(self) -> Optional[dict]:
        """返回完整帧并清空状态"""
        if not self.is_frame_complete():
            return None
        result = {
            "summary": self.get_summary(),
            "table": self.get_table(),
            "header": list(self._csv_header),
        }
        self.clear()
        return result


def extract_pan_adc_data(result: dict) -> Optional[list]:
    """从 PAN 帧提取 ADC 数据列 (para1)"""
    if result is None:
        return None
    if result["summary"].get("msg_type") != PrintMessageDecoder.MSG_PAN:
        return None
    table = result["table"]
    if not table:
        return None
    # para1 是 ADC 数据列
    adc_values = []
    for row in table:
        val = row.get("para1")
        if val is not None:
            adc_values.append(int(val))
    return adc_values


# ── 单元测试 ──────────────────────────────────────────

def test_decode_pan_message():
    dec = PrintMessageDecoder()
    lines = [
        "pan pluse is 12.",
        "Index,para1,para2",
        "0\t1234\t5678",
        "1\t1235\t5679",
        "",
    ]
    for line in lines:
        dec.feed_line(line)
    assert dec._summary["pulse"] == 12, f"pulse should be 12, got {dec._summary}"
    assert len(dec._csv_rows) == 2, f"rows should be 2, got {len(dec._csv_rows)}"
    print("[PASS] test_decode_pan_message")


def test_decode_empty():
    dec = PrintMessageDecoder()
    assert dec.feed_line("") is None
    assert dec.feed_line("some random text") is None
    assert dec._state == "IDLE"
    print("[PASS] test_decode_empty")


def test_extract_pan_adc():
    dec = PrintMessageDecoder()
    lines = [
        "pan pluse is 12.",
        "Index,para1,para2",
        "0\t1234\t5678",
        "1\t1235\t5679",
        "",
    ]
    for line in lines:
        dec.feed_line(line)
    result = dec.get_latest_result()
    adc = extract_pan_adc_data(result)
    assert adc == [1234, 1235], f"ADC should be [1234, 1235], got {adc}"
    print("[PASS] test_extract_pan_adc")


if __name__ == "__main__":
    test_decode_empty()
    test_decode_pan_message()
    test_extract_pan_adc()
    print("[OK] All tests passed")