# 半桥IH参数计算模型 — 项目指令

> **定位**: `sim/matlab_half_bridge _lima/` 是生产版本，MATLAB 物理模型在此开发验证，最终移植到 `BaseClass/src/power_calculator.c`
> **与上级项目的区别**: 本目录专注**算法建模与交叉验证**，不参与固件编写

---

## 一、项目身份

| 维度 | 说明 |
|------|------|
| **最终目标** | `BaseClass/src/power_calculator.c` 按 MATLAB 物理模型重写 |
| **当前阶段** | MATLAB 模型已通过 342 周期交叉验证，C 移植就绪 |
| **硬件限制** | 仅 I(谐振电流整流后) + Vdc(母线电压) 两路，V_res 只能推导 |
| **工作流** | MATLAB 建模 → Python 交叉验证 → C 端移植 |

## 二、生产文件

### 运行入口

```matlab
run_per_frame.m       % 逐帧 RLC 计算 (标准流程)
export_golden_batch.m % 批量导出 golden JSON → output/golden/golden_batch.json
```

```bash
python batch_cross_validate.py  # C DLL vs MATLAB 交叉验证 → batch_validate_report.txt
```

### 计算管线和依赖顺序

```
raw → calc_timing → f_sw, D%, 死区
raw, timing → calc_waveform → I_peak, I_rms, f_res
raw, timing → calc_phase → φ
raw, waveform, timing, phase → calc_power → P_W
raw, waveform, timing, phase → calc_rlc → ★ L, Q, R, f_res (核心)
rlc, phase → calc_impedance → Z, R, X
```

## 三、核心公式链 (KVL 封闭解)

```
已知: C = 0.9μF
I_peak, Vdc, φ, f_sw 从采集数据获得

L = (Vdc/2 + I_peak/ωC) / (I_peak × ω)   ← 闭合解，不需要 V_res
f_res = 1 / (2π√LC)
Q = tan(φ) / (f_sw/f_res - f_res/f_sw)    ← 标准串联 RLC Q 公式
R = ω₀L / Q
P = I_RMS² × R
```

## 四、已验证

### 交叉验证 (342 周期 ALL PASS)

| 参数 | 容差 | 最大实测误差 |
|------|------|-------------|
| L_uH | 5% | 0.77% |
| P_W | 2% | 0.71% |
| I_rms | 2% | 0.74% |
| phi_deg | 1° | 0.00° |
| f_sw_kHz | 1% | 0.26% |

详细报告: `docs/TEST_REPORT.md` | `batch_validate_report.txt` | `output/golden/golden_batch.json`

## 五、修改边界

| 区域 | 权限 | 说明 |
|------|------|------|
| `_lima/` 下 .m / .py / .c | ✅ 自由修改 | 算法开发 |
| `tools/ekf_tuner/captures/` | ❌ 只读 | 原始采集数据 |
| `src/RX32G410_FW_HAL_V1.3N/` | ❌ 只读 | 原厂固件库 |
| `BaseClass/src/power_calculator.c` | ⚠️ 需审批 | 最终部署目标 |

## 六、编码约定

- MATLAB: snake_case 变量，PascalCase 函数，4空格缩进
- Python: PEP8
- C: 与固件编码规则一致 (4空格缩进, snake_case)
- 所有 if/for/while 必须加 {}，单行也不省略
- 中文注释说明复杂物理公式

## 七、已知约束

1. 无 V_res 采集，已通过 KVL 封闭解闭环
2. C=0.9μF 是标称值，未考虑容差/温漂
3. I_rms = I_peak/√2 是正弦假设 (不影响 L 计算，L 只用 I_peak)
4. MATLAB double vs C float 有 0.05~0.77% 累积舍入误差
5. 轻载 (<10W) 时 P_W 偏差大

## 八、版本说明

- **★ 生产版本**: `sim/matlab_half_bridge _lima/` (含 PERIOD 修正 + 完整交叉验证)
- **旧版**: `sim/matlab_half_bridge/` (含 PERIOD BUG，不再使用)
