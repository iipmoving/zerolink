import pytest
from m4_modbus_tool import _pm_feed


def test_idle_finds_pm_in_middle():
    pm = {"state": "IDLE", "accum": bytearray(),
          "lines": [], "start": 0, "buf": bytearray()}
    _pm_feed(pm, b"garbage\xffdata#PM0\nhello\n")
    assert pm["state"] == "COLLECT"
    assert "#PM0" in pm["lines"]


def test_idle_no_pm():
    pm = {"state": "IDLE", "accum": bytearray(),
          "lines": [], "start": 0, "buf": bytearray()}
    _pm_feed(pm, b"\x05\x03\x0CdataCRC")
    assert pm["state"] == "IDLE"
    assert pm["lines"] == []


def test_collect_lines():
    pm = {"state": "COLLECT", "accum": bytearray(),
          "lines": ["#PM0"], "start": 0, "buf": bytearray()}
    _pm_feed(pm, b"Index,p1,p2\n0\t100\t200\n")
    assert pm["lines"] == ["#PM0", "Index,p1,p2", "0\t100\t200"]
    assert b"0\t100\t200" not in pm["buf"]


def test_collect_incomplete_line():
    pm = {"state": "COLLECT", "accum": bytearray(),
          "lines": ["#PM0"], "start": 0, "buf": bytearray()}
    _pm_feed(pm, b"Index,p1,")
    assert len(pm["lines"]) == 1  # 没有 \n，不形成行
    assert b"Index,p1," in pm["buf"]  # 留在 buf


def test_collect_partial_then_complete():
    pm = {"state": "COLLECT", "accum": bytearray(),
          "lines": ["#PM0"], "start": 0, "buf": bytearray()}
    _pm_feed(pm, b"Index,p1,")
    _pm_feed(pm, b"p2\n0\t1\t2\n")
    assert pm["lines"] == ["#PM0", "Index,p1,p2", "0\t1\t2"]
    assert len(pm["buf"]) == 0


def test_pm_end():
    pm = {"state": "COLLECT", "accum": bytearray(),
          "lines": ["#PM0", "data"], "start": 0, "buf": bytearray()}
    result = _pm_feed(pm, b"#PM_END\n")
    assert result == {"msg_type": 0, "rows": ["#PM0", "data"], "lines": 2}
    assert pm["state"] == "IDLE"