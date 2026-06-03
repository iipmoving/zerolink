"""CSV 加载器: 支持 data_simple.csv (9列) 和真实 WaveCapture CSV (含 SIZE 标记 + 电参数摘要)"""
import csv
import numpy as np

CORE_COLS = ["t_us", "I_adc", "V_adc", "Vdc_adc", "CNT",
             "CMP_UON", "CMP_UOFF", "CMP_LON", "CMP_LOFF"]


def _is_float(s):
    try:
        float(s)
        return True
    except (ValueError, TypeError):
        return False


def load_csv(csv_path):
    """
    读取 CSV. 自动处理:
    - SIZE 标记行 (跳过)
    - 电参数摘要行 (非数值 t_us → 跳过)
    - 空白行 (跳过)
    - 稀疏 CMP 值 (前向填充)
    返回 dict of numpy arrays (仅核心 9 列).
    """
    rows_raw = []
    with open(csv_path, "r") as f:
        reader = csv.DictReader(f)
        for row in reader:
            t = (row.get("t_us") or "").strip()
            if not t or t.upper() == "SIZE" or not _is_float(t):
                continue
            rows_raw.append(row)

    if not rows_raw:
        raise ValueError(f"No data rows found in {csv_path}")

    # 检测可用列
    avail = [c for c in CORE_COLS if c in rows_raw[0]]

    n = len(rows_raw)
    data = {}
    for col in avail:
        arr = np.full(n, np.nan, dtype=np.float64)
        for i, row in enumerate(rows_raw):
            v = (row.get(col) or "").strip()
            if v:
                arr[i] = float(v)
        # CMP 列前向填充
        if col.startswith("CMP_"):
            _forward_fill(arr)
            arr = np.nan_to_num(arr, nan=0.0)
        data[col] = arr

    # 移除 I_adc 仍为 NaN 的行 (理论上不会有)
    valid = ~np.isnan(data["I_adc"])
    for col in avail:
        data[col] = data[col][valid]
    return data


def _forward_fill(arr):
    last = np.nan
    for i in range(len(arr)):
        if np.isnan(arr[i]):
            arr[i] = last
        else:
            last = arr[i]


def find_cycles(data):
    """从 CNT 负跳变检测周期边界 → [(start_idx, end_idx), ...]"""
    cnt = data["CNT"]
    wrap_idx = np.where(np.diff(cnt) < 0)[0]
    if len(wrap_idx) == 0:
        return [(0, len(cnt) - 1)]
    boundaries = []
    prev = 0
    for wi in wrap_idx:
        boundaries.append((prev, wi))
        prev = wi + 1
    boundaries.append((prev, len(cnt) - 1))
    return boundaries


def extract_cycle(data, start_idx, end_idx):
    """从一个周期提取 (current[], hrtim[], voltage[], input_dict)"""
    s, e = start_idx, end_idx + 1
    cur = data["I_adc"][s:e].astype(np.uint16)
    hrt = data["CNT"][s:e].astype(np.uint16)
    vlt = data["Vdc_adc"][s:e].astype(np.uint16)

    def _med(key):
        vals = data.get(key, [0])[s:e]
        return int(np.median(vals[vals > 0])) if np.any(vals > 0) else 0

    dcnt = np.diff(hrt.astype(np.int32))
    dcnt = dcnt[dcnt > 0]
    per_adc = int(np.mean(dcnt)) if len(dcnt) > 0 else 384

    return (
        cur.tolist(), hrt.tolist(), vlt.tolist(),
        {"start": 0, "end": len(cur) - 1,
         "highOn": _med("CMP_UON"), "highOff": _med("CMP_UOFF"),
         "lowOn": _med("CMP_LON"), "lowOff": _med("CMP_LOFF"),
         "zero_cross_high": 0, "zero_cross_low": 0,
         "perAdc": per_adc, "lagDuty": 0}
    )


def load_cycles(csv_path, min_len=10):
    """加载 CSV 并返回所有有效周期 [(cur, hrt, vlt, inp, (start, end)), ...]"""
    data = load_csv(csv_path)
    cycles = find_cycles(data)
    result = []
    for s, e in cycles:
        if e - s >= min_len:
            cur, hrt, vlt, inp = extract_cycle(data, s, e)
            result.append((cur, hrt, vlt, inp, s, e))
    return result, data


def load_frame_params(csv_path):
    """
    读取每帧末尾的电参数摘要 (MATLAB 输出的 golden).
    返回 [{param_name: value, ...}, ...] 每帧一个 dict.
    """
    frames = []
    current = {}
    with open(csv_path, "r") as f:
        reader = csv.reader(f)
        next(reader)  # skip header
        in_data = False
        for row in reader:
            if not row or not row[0].strip():
                if current:
                    frames.append(current)
                    current = {}
                in_data = False
                continue
            first = row[0].strip()
            if first.upper() == "SIZE":
                in_data = True
                continue
            if in_data and _is_float(first):
                continue  # data row
            # parameter row: key, value
            if not _is_float(first) and len(row) >= 2:
                try:
                    current[first] = float(row[1])
                except ValueError:
                    current[first] = row[1]
    if current:
        frames.append(current)
    return frames
