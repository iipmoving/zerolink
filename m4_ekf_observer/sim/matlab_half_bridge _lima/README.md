# 半桥加热 MATLAB 仿真与交叉验证

> 用途: 半桥谐振变换器 RLC 参数提取 + C DLL 交叉验证
> 硬件: RX32G410 M4 + HRTIM 768MHz
> 最后更新: 2026-06-04

---

## 目录结构

```
matlab_half_bridge/
├── *.m, *.py              # 生产脚本 (21个)
├── debug/                  # 调试/过程文件 (42个, 归档)
├── output/
│   ├── golden/             # Golden 参考值 JSON
│   ├── reports/            # 逐帧 L 数据 CSV
│   └── figures/            # 图表 PNG
├── data/                   # 小型测试 CSV
├── docs/                   # 审查报告
├── keil_port/              # C 参考实现
└── README.md
```

CSV 采集数据: `../../tools/ekf_tuner/captures/` (50 个 capture 文件)

---

## 核心生产脚本

| 文件 | 功能 |
|------|------|
| `main.m` | 主入口, 加载+计算+输出 |
| `run_per_frame.m` | 逐帧 RLC 完整流程 (标准流程) |
| `batch_iron2.m` | 铁锅批量处理 |
| `batch_steel.m` | 钢锅批量处理 |
| `export_golden.m` | 单文件 golden 导出 |
| `export_golden_batch.m` | 批量 golden 导出 |
| `batch_cross_validate.py` | C DLL vs MATLAB golden 交叉验证 |
| `cross_validate_all.py` | 全量交叉验证 |
| `load_data.m` | CSV 数据加载 |
| `list_all_frames.m` | 列出所有帧的参数 |

### 计算模块 (calc_*.m)

| 文件 | 功能 | 依赖 |
|------|------|------|
| `calc_ac_cycle.m` | 交流周期检测 | raw |
| `calc_timing.m` | HRTIM 时序提取 (f_sw, D_U, DT) | raw |
| `calc_waveform.m` | 波形参量 (I_peak, I_rms, 峰值因数) | raw, timing |
| `calc_phase.m` | 相位角 φ | raw, timing |
| `calc_power.m` | 有功功率 P_W | raw, waveform, timing, phase |
| `calc_rlc.m` | RLC 参数 (L, Q, R, f_res) | raw, waveform, timing, phase |
| `calc_impedance.m` | 阻抗三角 (Z, R, X) | rlc, phase |
| `calibration.m` | 标定常数定义 | 无 |

### KVL 核心公式链

```
I_peak, ω, Vdc, C → V_C_peak = I_peak / (ωC)
                  → L = (Vdc/2 + V_C_peak) / (I_peak × ω)
                  → f_res = 1 / (2π√(LC))
                  → Q = tan(φ) / (f_sw/f_res - f_res/f_sw)
                  → R = ωL / Q
```

---

## 关键技术决策

1. **dI/dt 法已排除** (TEST_REPORT.md T05): 时域 dI/dt 法与频域 I_peak×ω 法 CV 一致 (6.5% vs 6.6%), 不提供精度提升
2. **I_peak 检测**: MATLAB 用 98% 分位数 (robust_peak), 需限制导通区间; C 端待改为前向斜率递减法抗尖峰
3. **I_zero**: 待从 _EstimateIzero 改为静态标定
4. **L 噪声瓶颈**: B-H 非线性主导, I_rms 是主要噪声源 (r=-0.975)

---

## 交叉验证状态

- MATLAB → golden_batch.json: 47 文件 342 周期全 PASS
- C DLL vs MATLAB: batch_cross_validate.py, 47 文件 342/342 PASS
- 误差容忍: P_W 2%, I_rms 2%, phi 1°, L 5%

---

## 参考

- 审查报告: `docs/`
- 调试记录: `debug/`
- C 实现: `keil_port/` + `../../src/.../power_calculator.c`
