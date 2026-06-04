# 半桥电磁炉参数计算 — 第三方审计说明

> **目标**: 供独立审计方验证 MATLAB 与 C 的算法一致性、测试覆盖率和公式链正确性。
> **审计焦点**: L 计算准确性 → 其余参数是否从 L 正确反推。

---

## 一、计算链路总览

```
原始数据 (9列 CSV)
    │
    ├─→ MATLAB export_golden_batch.m  ──→ golden_batch.json
    │         │
    │         └── per-cycle: I_zero → valley detect → P_W(积分) → I_peak(峰值) → phi(谷值-up vs highOn)
    │         └── per-file: L(KVL dI/dt) → f_res(1/2π√LC) → Q(tanφ) → R(ωL/Q) → Z,X
    │
    └─→ Python batch_cross_validate.py
              │
              ├── 加载 golden_batch.json (MATLAB 输出)
              ├── 通过 ctypes 调用 power_calculator.dll (C 编译产物)
              └── 逐周期逐参数比对 → batch_validate_report.txt
```

**核心公式链 (KVL dI/dt 封闭解)**:

```
已知: C = 0.9μF (谐振电容), 从 CSV 直接获取: I_peak, Vdc, φ, f_sw

步骤1: ω_sw = 2π × f_sw × 1000
步骤2: V_C_peak = I_peak / (ω_sw × C)           — 谐振电容峰值电压
步骤3: L = (Vdc/2 + V_C_peak) / (I_peak × ω_sw)  — KVL 电感 (dI/dt 验证)
步骤4: f_res = 1 / (2π × √(L × C))               — 谐振频率
步骤5: Q = tan(φ) / (f_sw/f_res - f_res/f_sw)    — 品质因数
步骤6: R = ω_res × L / Q                          — 等效电阻
步骤7: X = ω_sw × L - 1/(ω_sw × C)               — 工作频率电抗
步骤8: |Z| = √(R² + X²)                          — 阻抗模
```

**I_peak 获取方式**: 峰值检测法 — `max(ADC_array[ADC < 60000])`, 不采用 4-point 斜率法 (死区 dI/dt 极低, 斜率法低估 5-8×)。

**谷值检测**: 16-bit 方向跟踪移位寄存器, 捕获 falling→rising 跳变点, 算法与 C `_FindValley_f` 逐行一致。

**P_W**: I_act × Vdc 加权积分 (对称波形 ×2/N, 非对称波形分上下管), 加权处理 CNT 跨边界部分样本。

---

## 二、文件清单与调用方式

### 2.1 源代码 (算法实现)

| 文件 | 语言 | 职责 |
|------|------|------|
| `sim/matlab_half_bridge/export_golden.m` | MATLAB | 单文件 golden 导出 (3 个 r104*.csv) |
| `sim/matlab_half_bridge/export_golden_batch.m` | MATLAB | 批量 golden 导出 (全部 47 个 CSV) |
| `src/.../BaseClass/src/power_calculator.c` | C (KEIL) | MCU 端实时计算 (DLL_CalculatePower_FPU + CalculateElecParams_20ms) |
| `sim/dll_test/test/ctypes_bridge.py` | Python | ctypes 封装, 调用 power_calculator.dll |
| `sim/dll_test/test/csv_loader.py` | Python | CSV → numpy → ctypes 数据桥接 |
| `sim/matlab_half_bridge/cross_validate_all.py` | Python | 3 文件交叉验证脚本 |
| `sim/matlab_half_bridge/batch_cross_validate.py` | Python | 47 文件批量交叉验证脚本 |

### 2.2 调用命令

**生成 MATLAB golden**:
```bash
cd sim/matlab_half_bridge
matlab -batch "run('export_golden_batch.m')"
# 输出: golden_batch.json
```

**运行交叉验证**:
```bash
cd sim/matlab_half_bridge
python batch_cross_validate.py
# 输出: batch_validate_report.txt
```

**重新编译 C DLL (如有算法变更)**:
```bash
cd sim/dll_test
# 使用 MinGW/GCC 编译 power_calculator.c → power_calculator.dll
```

### 2.3 数据源

| 位置 | 内容 |
|------|------|
| `tools/ekf_tuner/capture_20260602_*.csv` | 47 个原始采集文件 (排除 `*_result.csv`) |
| CSV 格式 | `t_us, I_adc, V_adc, Vdc_adc, CNT, CMP_UON, CMP_UOFF, CMP_LON, CMP_LOFF` + 额外列 |
| 采样方式 | MCU HRTIM ISR 同步快照, 每采样点 9 寄存器值 |
| 功率覆盖 | 5W ~ 4814W (轻载到满载) |
| 周期数 | 342 个独立开关周期 |

---

## 三、测试结果位置

| 文件 | 内容 |
|------|------|
| `sim/matlab_half_bridge/golden_batch.json` | MATLAB 全量 golden — 47 文件, 每周期 11 个原始参数 + input 定义 |
| `sim/matlab_half_bridge/batch_validate_report.txt` | **主审计报告** — 47 文件, 342 周期, 16 参数, 逐帧 M vs C 偏差 |

### 报告结构

每文件包含:
1. **逐周期明细**: 每行列出 C 计算值 (P, I_rms, I_pk, phi, f_sw, L, f_res, Q, R, DT1, DT2) 及最差参数偏差
2. **逐参数汇总表**: MeanErr / MaxErr / Tol / PASS/FAIL
3. **全局汇总**: 全部 342 周期合并统计

---

## 四、审计要点

### 4.1 算法一致性验证

- **MATLAB 与 C 是否使用同一公式链?** 检查 `export_golden_batch.m` 的 §3 (KVL 公式链) 与 `cross_validate_all.py` 的 `run_c_cycle()` 中的 Python 重算是否一致。
- **I_peak 计算?** 确认 MATLAB 和 C 都使用 `max(adc[adc < 60000])`, 而非 4-point 斜率。
- **DT2 公式?** 确认两方都用 `(highOn - lowOff + period) × 0.5/perAdc`, 而非旧版错误公式。
- **谷值检测?** MATLAB `find_valley_m` 逐行翻译自 C `_FindValley_f`, 确认位运算、边界条件一致。

### 4.2 测试覆盖率

| 维度 | 覆盖情况 |
|------|----------|
| 文件数 | 47 个独立采集, 覆盖多个运行日/工况 |
| 周期数 | 342 个独立开关周期 |
| 功率范围 | 5W ~ 4814W (轻载/中载/满载/过载) |
| L 范围 | 37 ~ 139 μH (含 B-H 饱和效应) |
| f_sw 范围 | 24 ~ 38 kHz (含不同控制模式) |
| φ 范围 | 22° ~ 93° (含超前/滞后) |
| Q 范围 | 0.19 ~ 9.3 (低 Q 重载 → 高 Q 轻载) |
| 参数维度 | 16 个参数全覆盖 |

### 4.3 已知局限 (审计时应关注)

1. **I_rms = I_peak / √2**: 这是正弦假设。实际波形 form factor 可能偏离 0.707, 但不影响 L 计算 (L 只用 I_peak)。
2. **P_W 低功率偏差**: 轻载 (<10W) 时, I_zero 估计误差放大, 积分 P_W 与 I²R 推算值偏差较大。用户已明确 "P_W 先不管"。
3. **float vs double**: C 使用 float (32-bit), MATLAB 使用 double (64-bit), 公式链参数 (L→f_res→Q→R) 存在 0.05~0.77% 累积舍入误差。当前容差覆盖。
4. **不对称波形**: 对称检测 `(highOff×2+10) >= lowOff` 可能在某些死区配置下误判。
5. **单一 C 值**: 谐振电容固定 0.9μF, 未考虑容差/温漂。

### 4.4 可重现性验证步骤

```bash
# 1. 重跑 MATLAB golden
cd sim/matlab_half_bridge
matlab -batch "run('export_golden_batch.m')"

# 2. 重跑交叉验证
python batch_cross_validate.py

# 3. 检查报告
# 打开 batch_validate_report.txt
# 确认 GLOBAL REPORT 段显示 "ALL FILES PASSED"
# 确认 "Total cycles: 342, failed: 0"
```

---

## 五、关键标定常数

| 常数 | 值 | 用途 |
|------|-----|------|
| V_SCALE | 0.10606 V/cnt | ADC→电压 (ADC=2850 → 302.3V) |
| I_SCALE | 0.02522 A/cnt | ADC→电流 (ADC=795 → 20.0A) |
| VDC_SCALE | 0.10606 V/cnt | ADC→母线电压 |
| C_RES | 0.9 μF | 谐振电容标称值 |

---

## 六、联系人/角色

| 角色 | 职责 |
|------|------|
| 技术负责人 (AI) | 架构设计、公式链审核、容差定义 |
| 程序员 (AI) | MATLAB export / C DLL / Python 验证脚本实现 |
| 测试员 (AI) | 交叉验证执行、报告生成 |
| **审计方 (人类)** | 独立检查算法正确性、测试充分性、生产就绪度 |

---

*生成日期: 2026-06-03*
*关联 ADR: 待技术负责人决定是否写入 .claude/specs/adr/*
