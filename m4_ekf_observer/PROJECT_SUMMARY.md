# m4_ekf_observer — 项目总结

**日期**: 2026-06-03  
**MCU**: RX32G410 (Cortex-M4F, 192MHz)  
**用途**: 半桥/全桥电磁炉加热控制 — EKF 观测器 + 变增益 PID  
**状态**: 活跃开发

---

## 一、核心产出

### 1.1 MCU 固件 (`src/RX32G410_FW_HAL_V1.3N/`)

| 模块 | 路径 | 职责 |
|------|------|------|
| 功率计算 | `BaseClass/src/power_calculator.c` | `CalculatePower_FPU` — P_W/I_rms/I_peak/Vdc/phi |
| 电参数 | 同上 | `CalculateElecParams_20ms` — 完整 KVL 公式链: L→f_res→Q→R→X→Z (4头) |
| HRTIM 主同步 | `LIB/API/API_hrtim.c` | Master PREEN + UPD 中断源切换 (Slave→Master) |
| WaveCapture | `BaseClass/src/wave_capture.c` | 0x5000 MODBUS 区 — 9 列 CSV 波形采集 |
| ADC 处理 | `LIB/APP/APP_ADC.C` | ADC DMA + EOC 回调 + UPD 中断链 |
| 功率控制 | `LIB/APP/app_power.c` | POWER_CHANGE_CYCLE 状态机 + PPG 同步更新 |
| MODBUS | `modbus/` | 协议解析 + 寄存器映射 (0x5000/0x5100/0x1030) |

### 1.2 MATLAB 半桥模型 (`sim/matlab_half_bridge/`)

| 脚本 | 职责 |
|------|------|
| `load_data.m` | CSV 加载 + CNT 绕回检测 → 周期分割 |
| `calc_waveform.m` | I_zero 检测 + P_W 积分 + I_peak 峰值 |
| `calc_phase.m` | 谷值检测 → φ 相位差 |
| `calc_impedance.m` | KVL L 计算 + f_res/Q/R/X/Z 推导 |
| `export_golden_batch.m` | 批量导出 47 文件 golden → `golden_batch.json` |
| `export_golden.m` | 3 文件快速 golden → `golden.json` |
| `run_ih.m` | 主流程编排 |

### 1.3 C DLL 验证工具 (`sim/dll_test/`)

| 文件 | 职责 |
|------|------|
| `ctypes_bridge.py` | ctypes 封装 — `calculate_power_fpu()`, `calculate_elec_params_20ms()` |
| `csv_loader.py` | CSV 解析 + 周期检测 |
| `test_runner.py` | 单文件验证 |
| `*.dll` | C 侧编译产物 |

### 1.4 Python 验证脚本

| 脚本 | 职责 | 公式逻辑 |
|------|------|----------|
| `batch_cross_validate.py` | 47 文件全量验证 | **零** — 纯调用 C DLL |
| `cross_validate_all.py` | 3 文件快速验证 | **零** — 纯调用 C DLL |

### 1.5 调试工具箱 (`tools/ekf_tuner/`)

- `m4_gui.py` — MODBUS GUI (PyQt)
- `m4_modbus_tool.py` — MODBUS 通讯栈
- `ekf_q_model.py` — EKF Q 值模型
- 47 个 CSV 数据文件 (capture_*.csv)

---

## 二、问题记录

### 问题 1: Python 重复实现 C 公式链 ⚠️ 反复 2 次

**现象**: `cross_validate_all.py` 和 `batch_cross_validate.py` 中各写了 ~35 行 Python 实现 KVL L→f_res→Q→R→Z→X 公式链。

**实际情况**: C DLL 已有 `CalculateElecParams_20ms` 完整实现同样公式。

**后果**:
- 验证不是真正的 "C vs MATLAB"，而是 "Python(C中间值) vs Python(MATLAB中间值)"
- AUDIT_BRIEF.md 描述与代码事实不符，被审计报告指出

**修复** (2026-06-03):
- 删除 Python 脚本中所有公式逻辑
- 改为纯调用 `calculate_elec_params_20ms` 取 C 侧结果
- MATLAB golden 新增 per-cycle KVL 字段 (L_uH/Q/R/Z/X)
- 验证变为真正的 "C KVL vs MATLAB KVL"

**根因**: "快速出结果" 心态 — 写 Python 公式比研究 C 函数签名快

**避免方法**:
- 铁律: **Python 禁止出现任何物理公式** (`math.pi`, `math.sqrt`, `math.tan` 等)
- 写任何验证脚本前，必须先确认 C DLL 是否已有同类函数
- 如果 C DLL 缺函数 → 先在 C 侧实现，再写 Python 调用
- 已写入 `memory/feedback_python-caller-only.md`

---

### 问题 2: HRTIM 中断源切换反复追查 ⚠️ 反复 3+ 轮

**现象**: 为找到 "Timer F UPD 运行时重使能" 位置，追查了 `API_hrtim.c`、`API_hrtim_master_sync.c`、`APP_ADC.C`、`app_power.c` 四个文件。

**实际情况**: 
- `API_HRTIM_BASE_ENABLE_IT_UPD()` 从未被调用 — 它只在复制的 `master_sync.c` 中定义
- 运行时实际使用 **CMP4** 中断 (非 UPD)，通过 `API_HRTIM_BASE_ENABLE_IT_CMP()` → `API_T12_EOC_IRQHandlerCallBack()` 链
- UPD 路径仅用于初始化一次性触发

**根因**: 中断架构 (UPD vs CMP4, Slave vs Master, one-shot vs recurring) 未文档化

**避免方法**:
- 涉及中断的模块必须在头文件注释中写明: 触发源、触发时机、one-shot/continuous、重使能位置
- 新增中断相关代码时附带 ASCII 流程图 (见 §三 中断流程)

---

### 问题 3: 数据通道字段版本不一致

**现象**: 
- `golden_batch.json` 最初只有 11 字段 (无 L/Q/R/Z/X)
- Python 脚本用 `compute_MATLAB_derived()` 补偿
- golden 更新后 Python 不知情，继续冗余计算

**根因**: 数据通道 (MATLAB→JSON→Python) 无字段版本检查

**避免方法**:
- golden JSON 文件增加 `"fields": ["P_W", "L_uH", ...]` 元数据数组
- Python 脚本启动时检查 golden 字段 > C DLL 字段，不匹配则报错而非静默重算

---

### 问题 4: 重复代码文件未清理

**现象**: `API_hrtim_master_sync.c` 是 `API_hrtim.c` 的近似副本，但项目只编译后者。存在两份 `API_HRTIM_BASE_ENABLE_IT_UPD()`、两份 `API_HRTIM1_TEST2_IRQHandler()`。

**风险**: 后续维护者可能在错误文件中修改，导致编译后不生效。

**避免方法**: 不编译的备份文件应立即删除或移到 `archive/`。

---

## 三、关键架构图

### 中断流程 (HRTIM Master 同步模式)

```
ADC T12 EOC
    ↓ API_T12_EOC_IRQHandlerCallBack()
    ↓ API_HRTIM_Master_ENABLE_IT_UPD()
    ↓
Master 周期结束 → UPD event → shadow→active (Master+所有Slave同时)
    ↓ 中断触发 (IRQ 67)
HRTIM1_Master_IRQHandler()
    ↓ API_HRTIM1_Master_IRQHandler()
    ↓ clear MUPD flag, disable MUPD IT (one-shot)
    ↓ API_HRTIM1_TEST_UPD_IRQHandlerCallback(1)
    ↓
APP_ADC_IRQ_PPGstepChangeCallBack()
    ↓ POWER_CHANGE_CYCLE_Line:
    ↓   API_HRTIM_MasterSync_SetPeriod()  → 写 MPER shadow
    ↓   API_PPG_setValueChx() ×6          → 写 Slave shadow
    ↓
下一个 ADC T12 EOC → re-enable Master UPD → 循环
```

### 数据验证管道

```
MCU HRTIM ISR → RawSample_t[2048] → MODBUS 0x5100
    ↓ GUI 2s poll
CSV (9列) → MATLAB export_golden_batch.m → golden_batch.json (16字段含KVL)
    ↓                                         ↓
CSV → csv_loader.py → calculate_elec_params_20ms (C DLL) → ElecParamsDef
                                                              ↓
                                    batch_cross_validate.py: 逐参数比对 → 报告
```

---

## 四、未完成项

| 项 | 状态 | 说明 |
|----|------|------|
| `keil_port/` CORDIC 移植 | 待做 | Phase 3: CORDIC 地址需用户提供官方例程确认 |
| `app/` 模块 | 骨架 | ekf/pot_detect/var_gain_pid 仅有空目录 |
| `API_hrtim_master_sync.c` | 待删除 | 不编译的重复文件 |
| `APP_ADC - 副本.C` | 待删除 | 备份副本 |

---

## 五、验证状态

| 验证项 | 范围 | 结果 |
|--------|------|------|
| C DLL vs MATLAB KVL 全参数 | 47文件/342周期/16参数 | ✅ ALL PASS |
| L_uH 最大误差 | 342周期 | 0.77% (float32 vs float64) |
| P_W 最大误差 | 342周期 | 0.71% |
| phi_deg 误差 | 342周期 | 0.00 (完全一致) |

---

*最后更新: 2026-06-03*
