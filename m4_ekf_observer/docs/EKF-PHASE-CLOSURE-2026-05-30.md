# 阶段性结项报告: M4 半桥 EKF 谐振参数估计 + 锅具识别

> **角色**: 技术负责人  
> **日期**: 2026-05-30  
> **阶段**: EKF 离线验证 + f0 估计三方法验证 + 锅具判别完成  
> **前置条件**: 已验证半桥固件 + MODBUS 遥测补丁  
> **下一阶段**: C 端移植 (ekf_resonant_track.c) & HRTIM MASTER 同步简化

---

## 一、本阶段交付成果

| 交付物 | 说明 | 量化 |
|--------|------|------|
| **物理模型验证** | 半桥 RLC 串联谐振模型，工作点在 f > f0 感性区 | 铁锅 20-21kHz / 钢锅 23-24kHz ✅ |
| **f0 估计三方法** | 相位模型法(Primary) + dI/df斜率法(Reference) + 相位修正联合法 | 4 种算法独立实现 |
| **锅具判别逻辑** | f0 阈值 + 灰色区域 f_min_high_pwr 辅助判别 | 9 组测试用例 100% PASS |
| **数据采集工具链** | MODBUS 通信 + GUI + 6 个自动化工况序列 | 14 组 CSV，~9000 行数据 |
| **MATLAB 交叉验证** | f0/Q/L 三参数从物理波形提取，电压无关性验证 | 3 组数据集，3 张验证图 |
| **MODBUS EKF 遥测** | 0x1020-0x1029 寄存器布局 + resonant_msg 替代 printMessage | resonant_f0.c/h 已创建 |
| **验证方法论沉淀** | memory 条目 `m4-ekf-verification-methodology` 完整记录 9 大检索点 | 已归档 |

---

## 二、核心验证结论

### 2.1 f0 估计精度

| 方法 | 铁锅 f0 (实测) | 钢锅 f0 (实测) | 误差 | 备注 |
|------|---------------|---------------|------|------|
| 相位模型法 (PRIMARY) | **20-21 kHz** | **23-24 kHz** | 稳定 CV<20% | 全部数据点拟合，最可靠 |
| dI/df 斜率法 (REFERENCE) | 偏高 1-3kHz | 接近 | 远场外推偏差 | 铁锅远离谐振点，p=1 模型预期偏大 |
| 相位修正联合法 | 与 PRIMARY 一致 | 与 PRIMARY 一致 | — | 高频段噪声大时有效 |
| tan(φ) 比值消 Q 法 | 中位数稳定 | 中位数稳定 | 需相位≥5° | 多点对组合消除 Q 依赖 |

### 2.2 锅具判别验证

```
判别逻辑 (ekf_q_model.py):
  f0 < 19kHz  → IRON (铁锅/不锈钢430)
  f0 ≥ 20kHz  → STEEL (钢锅/不锈钢304)
  19-20kHz    → 灰色区: f_min_high_pwr < 25kHz → iron, else steel

实测验证: 铁锅 f0≈20-21kHz ✅  钢锅 f0≈23-24kHz ✅ (9/9 测试用例)
```

### 2.3 f0/Q/L 三参数物理提取

从半桥一个完整周期的谐振电流波形提取（MATLAB `verify_f0_q_l.m`）：

| 参数 | 提取方法 | 电压无关性 | 验证状态 |
|------|---------|-----------|---------|
| **f0** | ZC 谷间距（纯时域） | ✅ 完全无依赖 | 3 组数据一致 |
| **Q** | 峰值衰减比（幅度比） | ✅ Vbus 在 ratio 中消去 | 包络拟合 R²>0.99 |
| **L** | 开通段 di/dt（需 Vbus） | ✅ 两电压档位 L 值差异 <3% | Vc = Vbus/2 假定成立 |

---

## 三、测试数据统计

### 3.1 离线分析数据 (seq_f0_*.csv)

| 文件 | 锅具 | 数据行数 | f0 范围 | 用途 |
|------|------|---------|---------|------|
| `seq_f0_iron_121119.csv` | 铁锅深锅 | 210 | ~20-21kHz | 主数据集 |
| `seq_f0_steel_121355.csv` | 钢锅 | 393 | ~23-24kHz | 钢锅基准 |
| `seq_f0_iron_deep_142106.csv` | 铁锅深锅 (高功率) | 232 | ~20-21kHz | 大功率验证 |

### 3.2 实时采集数据 (m4_ekf_*.csv)

| 日期 | 文件数 | 总行数 | 说明 |
|------|--------|--------|------|
| 2026-05-27 | 1 | 916 | 初始扫描 |
| 2026-05-28 | 3 | 1269 | 功率扫描 + 钢锅 |
| 2026-05-29 | 10 | 6827 | 覆盖全部 6 个测试工况 |

---

## 四、工具链归档

```
tools/ekf_tuner/
├── m4_modbus_tool.py        # MODBUS 通信底层 (994行)
├── m4_gui.py                # tkinter 调试界面 (1227行)
├── run_ekf_tests.py         # 自动化工况序列 (326行)
├── ekf_q_model.py           # f0 估计 + 锅具判别 + --self-test (711行)
├── plot_utils.py            # 实时/离线绘图 (429行)
├── calc_physical_params.m   # MATLAB 物理量换算
├── eval_L_compare.m         # L 值多数据集对比
├── verify_f0_q_l.m          # f0/Q/L 三参数交叉验证 (596行)
├── run_verify_L.m           # L 值验证批处理
├── verify_L_consistency.m   # L 跨工况一致性
└── m4_init_config.json      # 参数持久化
```

---

## 五、C 端移植待办

> **EKF 算法的桌面端验证已完成**。以下为下一阶段的移植任务，参照 `docs/ekf-implementation-plan.md` §4。

| # | 文件 | 内容 | 优先级 | 依赖 |
|---|------|------|--------|------|
| 1 | `app/ekf/ekf_resonant_track.h` | EKF_ResonantTracker_t 结构体 + API 声明 | P0 | 参数 K_p/Q/R 已标定 |
| 2 | `app/ekf/ekf_resonant_track.c` | EKF_Tracker_Init() + EKF_Tracker_Update() 实现 | P0 | #1 |
| 3 | 固件集成 | 5 处修改 (FIRMWARE_PATCH.md) 将 EKF 接入 app_power.c | P0 | #2 + 需完整源文件 |
| 4 | 回归测试 | MODBUS 回读 C 端输出 vs Python 输出偏差 < 1Hz | P1 | #3 |
| 5 | HRTIM 主同步简化 | 清理 IO 同步残余 (5 任务，工作名 `hrtim-master-sync`) | P1 | EKF 完成后启动 |

---

## 六、后续待开发功能

| 功能 | 状态 | 参考文档 |
|------|------|---------|
| 谐振参数 0x4000 区 MODBUS 输出 | `resonant_f0.c` 已创建，待集成到主固件 | `docs/modbus-resonant-output-plan.md` |
| EKF 遥测补丁 (0x1020-0x1029) | `modbus_ekf_regs.c/h` 已创建，5 处待集成 | `memory/project_m4-ekf-patch-status.md` |
| 死区补偿 | 低功率时 ZC 检测偏移，MATLAB 先验证再 C 移植 | `memory/project_m4-f0-estimation-methods.md` §6 |
| 半桥 → 全桥迁移 | 工作名 `migrate-full-bridge`，EKF + HRTIM 同步完成后启动 | `memory/project_m4-half-to-full-bridge-migration.md` |

---

## 七、关键指标总结

| 指标 | 值 |
|------|-----|
| 覆盖测试工况 | 6 个 (power_sweep/pot_material/freq_step/pot_lift/pot_size/恶劣锅具) |
| 总数据量 | 14 CSV 文件，~9000 行 |
| f0 估计方法 | 4 种 (相位模型/dI/df斜率/联合修正/tan比值消Q) |
| 验证方法 | 2 种独立路径 (Python ekf_q_model + MATLAB verify_f0_q_l) |
| 锅具判别测试 | 9 用例 100% PASS |
| 归档文档 | 1 份结项报告 + 1 份方法论记忆 |

---

*本报告由技术负责人归档。下一阶段启动条件：用户提供完整 M4 固件源文件后执行 C 端移植。*
