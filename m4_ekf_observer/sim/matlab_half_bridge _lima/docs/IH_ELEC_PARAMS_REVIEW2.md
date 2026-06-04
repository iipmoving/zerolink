# ih_elec_params — 第二次审查报告（FIX 版核查）

**审查对象**：`keil_port/ih_elec_params.c / .h`（修改后版本）  
**参照文档**：`docs/IH_ELEC_PARAMS_REVIEW_FIX.md`（修改摘要）  
**审查日期**：2026-06-04  
**审查方**：李工 + AI 参谋长

---

## 一、FIX 逐条核查

| # | 原问题 | FIX 文档声称 | 代码实际状态 | 结论 |
|---|--------|-------------|-------------|------|
| 1 | I_arr/L_raw 索引错位 | 不修，改为 I_valid 数组同步 | c:74 `I_valid[20]` 声明，c:86-89 同步写入，Step 2 用 `I_valid` 中位数 ✅ | **✅ 已修，且比预期更彻底** |
| 2 | L_ref 用 0.1A 浮窗 | 改为 argmax(I_peak) | c:113-116 `i_max_idx` argmax 循环 ✅ | **✅ 已修** |
| 3 | Python ctypes 结构体不匹配 | 待修 | `.py` 未动，`C_ElecResult` 仍含 `omega_sw/V_C_peak/omega_res` 等中间字段，与 `ih_elec_params.h` 不匹配 | **⏳ 待修（已知，未动）** |
| 4 | φ 还原截 90° 问题 | 整段注释掉 | c:146-152 已注释 ✅ | **✅ 已处理** |
| 5 | calc_one_cycle 死代码/omega_res 混乱 | 改名 calc_one_cycle_L，删 P_out | c:42 `calc_one_cycle_L` 签名，函数只返回 L，无 P_out ✅ | **✅ 已修** |
| 6 | valid 阈值 1μH 过宽 | 改为 IH_L_MIN_uH(10) / IH_FRES_MIN_kHz(5) | h:29-30 常量定义，c:183 使用常量 ✅ | **✅ 已修** |

---

## 二、新增细节审查（FIX 引入的逻辑变化）

### 2.1 ✅ I_valid 同步逻辑——正确

```c
// c:74 声明
float I_valid[20], L_raw[20];
uint8_t valid_cnt = 0;

// c:85-89 同步写入
if (calc_one_cycle_L(I_arr[i], V_arr[i], F_arr[i], &Lv)) {
    I_valid[valid_cnt] = I_arr[i];
    L_raw[valid_cnt]   = Lv;
    valid_cnt++;
}

// c:95 中位数阈值基于有效周期 ✅（原问题#4也顺带修了）
float I_med = median_f(I_valid, valid_cnt);
```

**评估**：FIX 文档说"不修 #1，用 I_valid 替代"，但实际代码里 I_valid 数组既修了 #1（索引同步），也修了 #4（中位数基于有效周期）。两个问题一并解决，逻辑更干净。✅

---

### 2.2 ✅ Step 3 argmax——正确

```c
uint8_t i_max_idx = 0;
for (i = 1; i < kept; i++)
    if (I_kept[i] > I_kept[i_max_idx]) i_max_idx = i;
float L_ref = L_kept[i_max_idx];
float I_max = I_kept[i_max_idx];  // ← 保留 I_max 用于后续权重计算
```

`I_max` 同时被 c:121 权重修正使用，逻辑完整。✅

---

### 2.3 ⚠️ 新发现：φ 采样仍用 I_arr（全部）而非 I_valid

```c
// c:136-140
float I_phi_thr = I_av * IH_I_PEAK_MIN_RATIO;
for (i = 0; i < cnt; i++) {
    if (I_arr[i] >= I_phi_thr) { phi_sum += P_arr[i]; phi_n++; }
}
```

这里 `I_arr[i]` 是全部周期（含 calc_one_cycle_L 失败的），`I_av` 是基于有效周期的 `I_kept` 中位数。  
当有无效周期时，无效周期的 `I_arr[i]` 可能恰好 ≥ `I_phi_thr`，导致把"无效的 φ 值"混入 `phi_sum`。

**影响**：φ 精度问题，不影响 L/f_res 计算，但 Q/R 依赖 φ。  
**建议**：φ 采样也只取有效周期，或加注释说明此处为已知简化。  
**严重度**：⚠️ 中等（量小无效周期时影响很小）

---

### 2.4 ✅ Step 6 f_res/Q/R/Z——无变化，已验证正确

公式链与原来一致，armclang 0 error，通过。

---

### 2.5 ✅ valid 常量化——正确且改善可维护性

```c
// h:29-30
#define IH_L_MIN_uH        10.0f
#define IH_FRES_MIN_kHz    5.0f

// c:183
out->valid = (out->L_uH > IH_L_MIN_uH) && (out->f_res_kHz > IH_FRES_MIN_kHz);
```

10μH 下限比原来 1μH 合理。f_res 5kHz 下限也合理（实际工作 25~40kHz）。✅

---

### 2.6 ⚠️ 遗留：Python ctypes 未更新（已知）

`ih_elec_params.py` 的 `C_ElecInput` 使用 `f_sw_kHz`（float，单位 kHz），但 `IH_ElecInput` 实际是 `uint16_t* f_sw_buf`（单位 Hz，逐周期数组）。  
两个结构体在 API 层面完全不同（标量 vs 缓冲区指针），**不仅字段不匹配，接口语义也不一致**。

DLL 交叉验证功能目前不可用，但 FIX 文档已注明"后续对齐"，属已知待办。

---

## 三、总结

| 维度 | 修改前 | 修改后 | 评级变化 |
|------|--------|--------|---------|
| 索引同步 | ❌ 错位 | ✅ I_valid 数组同步 | ❌→✅ |
| 中位数基准 | ⚠️ 含无效周期 | ✅ 仅有效周期 | ⚠️→✅ |
| L_ref 查找 | ⚠️ 0.1A 浮窗 | ✅ argmax | ⚠️→✅ |
| φ 还原截断 | ⚠️ tan→∞ | ✅ 整段注释 | ⚠️→✅ |
| 死代码清除 | ❌ 混乱 | ✅ 函数重构 | ❌→✅ |
| valid 阈值 | 🔹 过宽 | ✅ 常量 10μH/5kHz | 🔹→✅ |
| φ 无效周期混入 | — | ⚠️ **新发现** | 新增⚠️ |
| Python ctypes | ❌ 不匹配 | ⏳ 待修 | 不变 |

---

## 四、上机前剩余风险

### ⚠️ 唯一需确认的新问题

**φ 采样含无效周期**（c:136-140）：  
建议改为：
```c
for (i = 0; i < valid_cnt; i++) {
    if (I_valid[i] >= I_phi_thr) { phi_sum += P_arr[i]; phi_n++; }
}
```
注意：若改用 `I_valid` 则 `P_arr[]` 的索引需对应，需维护 `P_valid[]` 数组（与 I_valid 同步）；  
或接受当前简化写法，加注释 `/* 注：含无效周期，极端情况φ精度略降 */`。

### ⏳ 遗留（不影响上机）

- Python ctypes `C_ElecResult` / `C_ElecInput` 与 C API 不兼容——DLL 验证路径目前不可用，但纯 Python `self_test()` 路径不受影响。

---

## 五、结论

**修改质量：良好。**  
6 个 FIX 条目全部落地，且 I_valid 一个改动顺带修了原先的两个问题（#1 索引 + #4 中位数基准）。  
发现 1 个新中等问题（φ 采样仍含无效周期），修复简单，可一并处理后上机。

*审查方：李工 + AI 参谋长*  
*2026-06-04*
