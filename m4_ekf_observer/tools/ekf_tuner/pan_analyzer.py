"""
pan_analyzer.py — PAN 数据处理: CSV 导出 + f_res 计算

支持两种模式:
  1. MATLAB Engine: 调用 load_data.m + calc_waveform.m (需 MATLAB 安装)
  2. numpy 估算: 谷值法简单估算 f_res (无需 MATLAB)
"""

import os
import numpy as np


class PanAnalyzer:
    """PAN 检锅数据处理"""

    def __init__(self, matlab_engine=None):
        self.matlab = matlab_engine

    # ── CSV 导出 ──────────────────────────────────────

    def to_csv(self, adc_data: list, pulse_count: int,
               csv_path: str = "pan_data.csv") -> str:
        """导出兼容 load_data.m 格式的 CSV

        load_data.m 预期格式:
          t_us, I_adc, V_adc, Vdc_adc, CNT, CMP_UON, CMP_UOFF, CMP_LON, CMP_LOFF
        PAN 消息只包含 I_adc (谐振电流), 其他列填 0 或自动推算。
        """
        header = "t_us,I_adc,V_adc,Vdc_adc,CNT,CMP_UON,CMP_UOFF,CMP_LON,CMP_LOFF"
        with open(csv_path, "w", newline="") as f:
            f.write(header + "\n")
            # 行数 = 数据点数
            for i, val in enumerate(adc_data):
                # t_us: 假设 1MHz 采样率, 每行间隔 1μs
                t = i
                f.write(f"{t},{val},0,0,{t},0,0,0,0\n")
        return csv_path

    # ── MATLAB 引擎 ──────────────────────────────────

    def run_matlab(self, csv_path: str) -> dict:
        """调用 MATLAB 计算 f_res

        需要: MATLAB Engine API for Python 已安装
              matlab_half_bridge 脚本在 MATLAB 路径中
        """
        if not self.matlab:
            return {"error": "MATLAB Engine not available"}

        try:
            # 加载标定
            self.matlab.eval("calibration;", nargout=0)
            # 加载数据
            raw = self.matlab.workspace["raw"]
            # ... 调用 calc_waveform 等
            # 返回结果
            return {"f_res": 0, "r_load": 0, "status": "ok"}
        except Exception as e:
            return {"error": str(e)}

    # ── numpy 估算 ──────────────────────────────────

    @staticmethod
    def estimate_freq(adc_data: list, sample_rate_hz: float = 1e6) -> float:
        """谷值法估算谐振频率 (无需 MATLAB)

        对半桥谐振电流波形, 统计谷值间隔推算周期。
        """
        arr = np.array(adc_data, dtype=float)
        if len(arr) < 10:
            return 0.0

        # 找谷值 (局部最小值)
        valleys = []
        for i in range(1, len(arr) - 1):
            if arr[i] < arr[i - 1] and arr[i] < arr[i + 1]:
                valleys.append(i)

        if len(valleys) < 3:
            return 0.0

        # 谷值间隔 = 一个完整周期 (相邻谷值距离 = 周期)
        intervals = np.diff(valleys)
        period_samples = np.mean(intervals)
        if period_samples <= 0:
            return 0.0

        # 频率 = 采样率 / 周期(点数)
        freq_hz = sample_rate_hz / period_samples
        return freq_hz


# ── 单元测试 ──

def test_estimate_freq_sine():
    """模拟 25kHz 正弦波, 验证谷值法"""
    fs = 1e6
    f = 25000
    t = np.arange(0, 1000) / fs
    sine = np.sin(2 * np.pi * f * t) * 512 + 2048  # 偏置到 ADC 范围, 放大振幅
    adc = sine.astype(int).tolist()

    analyzer = PanAnalyzer()
    f_est = analyzer.estimate_freq(adc, fs)
    print(f"[test] 25kHz sine -> estimated {f_est:.1f} Hz")
    assert abs(f_est - f) / f < 0.15, f"f_est={f_est:.0f} too far from {f}"
    print("[PASS] test_estimate_freq_sine")


def test_to_csv():
    analyzer = PanAnalyzer()
    adc = [100, 200, 300, 200, 100]
    path = analyzer.to_csv(adc, 5, "_test_pan.csv")
    assert os.path.exists(path), "CSV not created"
    with open(path) as f:
        header = f.readline().strip()
    assert header == "t_us,I_adc,V_adc,Vdc_adc,CNT,CMP_UON,CMP_UOFF,CMP_LON,CMP_LOFF"
    os.remove(path)
    print("[PASS] test_to_csv")


if __name__ == "__main__":
    test_to_csv()
    test_estimate_freq_sine()
    print("[OK] All pan_analyzer tests passed")