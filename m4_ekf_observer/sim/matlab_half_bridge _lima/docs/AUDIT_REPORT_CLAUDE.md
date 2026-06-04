# 半桥参数计算 Claude 生成审计报告

**审计对象**：`AUDIT_BRIEF.md`（Claude 生成）  
**审计方**：李工 + AI 参谋长  
**审计日期**：2026-06-03  
**审计类型**：全量代码级交叉验证

---

## 一、总体结论

| 维度 | 评级 | 说明 |
|------|------|------|
| 物理正确性 | ✅ 通过 | KVL 公式链物理推导正确，已验证 |
| 数学一致性 | ✅ 通过 | MATLAB/Python 两方公式完全一致 |
| **架构描述** | ❌ 不实 | L 计算链不在 C 代码中，文档声称的交叉验证对象不存在 |
| 工程完备性 | ⚠️ 需补充 | 4 项缺失：DT定义、校准来源、路径、容差表 |

---

## 二、致命问题（1 项）

### ❌ L/f_res/Q/R 计算链不在 C 代码中

**问题**：`AUDIT_BRIEF.md` 第 16 行声称 `per-file: L(KVL dI/dt) → f_res(1/2π√LC) → Q(tanφ) → R(ωL/Q) → Z,X`，并描述为"C vs MATLAB 交叉验证"。

**事实**：经代码级核查 `power_calculator.c`（1542 行），C 代码只计算：

| C 函数 | 输出参数 |
|--------|---------|
| `CalculatePower()` | P_W, I_peak, phase_angle, ESR (=V_avg/I_avg) |
| `CalculateActualCurrent()` | 电流积分, 电压平均值 |

C 代码中**不存在**以下计算：
- ❌ 电感 L（KVL dI/dt 公式）
- ❌ 谐振频率 f_res
- ❌ 品质因数 Q
- ❌ 等效电阻 R
- ❌ 电抗 X / 阻抗 Z

**真实验证路径**：

```
MATLAB export_golden_batch.m  →  计算出 L, f_res, Q, R, Z, X
                                      ↓ 写入 golden_batch.json
Python cross_validate_all.py  →  读取 C DLL 的 I_peak, f_sw, Vdc, phi
                                      ↓ 用 Python 重新计算 L, f_res, Q, R, Z, X
                                      ↓ 与 MATLAB 的 L, f_res, Q, R 比对
```

**结论**：交叉验证实际是 **"MATLAB 的 L vs Python 用 C 中间值重算的 L"**，不是 "C 的 L vs MATLAB 的 L"。`AUDIT_BRIEF.md` 对交叉验证对象的描述与代码事实不符。

---

## 三、中等问题（4 项）

### ⚠️ 3.1 DT1/DT2 定义缺失

文档提及 `DT2 = (highOn - lowOff + period) × 0.5/perAdc`（第 118 行），但未定义 DT1 算法。对比验证时需要死区参数定义才能判断正确性。

### ⚠️ 3.2 标定常数来源不明

第 162-167 行列出的 V_SCALE、I_SCALE、VDC_SCALE 三个常数，无来源说明：
- 是从硬件校准数据导出？
- 是从某个配置文件读取？
- 是人工设定？

审计方无法验证这些常数的正确性。

### ⚠️ 3.3 路径过于模糊

第 56 行 `src/.../BaseClass/src/power_calculator.c` 使用省略号占位，无法定位实际文件。其他路径（`sim/...`）同样模糊，缺少项目根目录锚点。

### ⚠️ 3.4 交叉验证容差表不在审计文档中

文档声称 16 个参数做交叉验证，但容差定义（MeanErr / MaxErr / Tol）只在 Python 代码 `cross_validate_all.py` 第 16-33 行中，未写入审计文档。审计方需要额外读取 Python 代码才能评估 PASS/FAIL 标准的合理性。

---

## 四、轻微问题（3 项）

### 🔹 4.1 ADC 阈值 60000 无依据

第 40 行 `max(ADC_array[ADC < 60000])` 中 60000 的选值无计算依据或注释说明。

### 🔹 4.2 魔数 +10 无解释

第 139 行对称检测公式 `(highOff×2+10) >= lowOff` 中 `+10` 未说明来源。

### 🔹 4.3 角色分配过度依赖 AI

第 171-178 行所有技术角色（架构、开发、测试）均为 AI，审计方仅为人类。缺少 AI 产出的人工审核环节定义。

---

## 五、核实通过项

以下内容经代码级验证确认正确：

| 项 | 验证方法 | 结果 |
|----|---------|------|
| KVL 公式 `L = (Vdc/2 + V_C_peak)/(I_peak × ω)` | 物理推导 + 代码比对 | ✅ 正确 |
| MATLAB/Python 公式一致性 | 逐行比对两方代码 | ✅ 一致 |
| I_peak 峰值检测法 | 确认两方均用 `max(adc[adc < 60000])` | ✅ 一致 |
| 谷值检测算法 | 确认 MATLAB 翻译自 C 的 `_FindValley_f` | ✅ 一致 |
| 采样数据格式 | 9 列 CSV 定义与实际文件对应 | ✅ 正确 |
| 测试覆盖范围 | 47 文件/342 周期/5W~4814W | ✅ 充分 |

---

## 六、修正建议

### 必须修正

1. **修正交叉验证对象描述**：将文档中"C vs MATLAB"改为"Python 用 C 中间值重算 vs MATLAB"，或把 L/f_res/Q/R 计算链补入 `power_calculator.c`

### 建议补充

2. 补充 DT1 算法定义
3. 标注标定常数的数据来源文件路径
4. 将容差表写入审计文档（或引用 Python 代码具体行号）
5. 将省略号路径替换为实际路径
6. 补充人工审核环节到角色表

---

## 七、公式链物理验证（附）

对核心公式做独立物理验证：

**已知条件**：
- 半桥拓扑，高侧管开通瞬间
- 此时 I ≈ 0（R 压降可忽略）
- V_C ≈ −V_C_peak（前一周期反极性）

**KVL 推导**：
```
V_L = Vdc/2 − V_C = Vdc/2 − (−V_C_peak) = Vdc/2 + V_C_peak
V_L = L × (di/dt)_max = L × I_peak × ω
→ L = (Vdc/2 + V_C_peak) / (I_peak × ω)
```

**验证结论**：公式物理正确，与 MATLAB/Python 代码一致。

---

*审计方：李工 + AI 参谋长*  
*审计范围：代码级全量交叉验证*  
*生成日期：2026-06-03*
