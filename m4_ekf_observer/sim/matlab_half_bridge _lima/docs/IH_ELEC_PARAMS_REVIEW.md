# ih_elec_params.c 代码审查报告

**审查对象**：`keil_port/ih_elec_params.c / .h`  
**设计依据**：`docs/IH_ELEC_PARAMS_DESIGN.md`  
**审查日期**：2026-06-04  
**审查方**：李工 + AI 参谋长  
**方法**：逐行对比设计文档 × C 实现 × Python 参考

---

## 一、总评

| 维度 | 评级 | 说明 |
|------|------|------|
| KVL 公式链 | ✅ 通过 | 与设计/Python/MATLAB 一致，但 `calc_one_cycle` 有一处冗余计算 |
| 输入处理流程 | ⚠️ 3 处偏差 | Step 1 先算后过滤、I_kept 索引错位、φ 还原阈值偏宽 |
| 边界条件 | ⚠️ 2 处问题 | f_sw=0 NULL 指针未防、L_ref 查找逻辑不健壮 |
| 数值稳定性 | ✅ 基本合格 | 主要零除均有保护，有一处冗余 omega_res 计算 |
| MCU 适配 | ✅ 可用 | 无动态分配，栈消耗合理 |

---

## 二、逐项详查

### ✅ 2.1 KVL 公式链

**设计**（DESIGN.md §3.1）：
```
ω_sw  = 2π × f_sw
V_C_pk = I_peak / (ω_sw × C)
L     = (Vdc/2 + V_C_pk) / (I_peak × ω_sw) × 1e6
```

**C 代码**（c:47~53）：
```c
float omega_sw = 2.0f * M_PI * f_sw_hz;
float V_C_peak = I_peak / (omega_sw * IH_C_FARAD);
float L_val = (Vdc * 0.5f + V_C_peak) / (I_peak * omega_sw) * 1e6f;
```

**Python 参考**（py:38~44）：`f_sw` 单位为 kHz，用 `f_sw * 1e3`；C 代码输入单位 Hz，直接用 `f_sw_hz`。**两者等价，公式链正确。**

---

### ❌ 2.2 严重：`calc_one_cycle` 中 omega_res 计算方式错误（但结果等价，只是写法混乱）

**C 代码**（c:59~62）：
```c
float omega_res = 2.0f * (float)M_PI
    / (2.0f * (float)M_PI * sqrtf((L_val * 1e-6f) * IH_C_FARAD));
float ratio = f_sw_hz / (omega_res / 6.2831853f);
```

**问题**：
1. `omega_res = 2π / (2π × √LC) = 1/√LC`，实际上算的是 `ω_res` 但写法像手误，多乘了一个 `2π` 又除回去。
2. 然后 `omega_res / 6.2831853f` 把 `ω_res` 除以 `2π` 得到 `f_res_Hz`，再求 `ratio`。整个计算**结果正确**，但是写法极度混乱，维护者难以理解。

**应有写法**（与 Step 6 一致）：
```c
float f_res = 1.0f / (2.0f * M_PI * sqrtf((L_val * 1e-6f) * IH_C_FARAD));
float ratio = f_sw_hz / f_res;
```

另外 `calc_one_cycle` 里的 P_out 路径不被主路径调用（Step 1 传 `P_out=0`），这段代码是死代码，可以删除或重构。

---

### ❌ 2.3 严重：Step 2 过滤逻辑——索引错位

**设计**（DESIGN.md §2.2）：先计算全部 L[i]，再对 I_peak 低于阈值的周期**连同 L[i] 一起**丢弃。

**C 代码**（c:91~116）：

```c
/* Step 1: 逐周期 KVL 算 L[i] */
for (i = 0; i < cnt; i++) {
    I_arr[i] = ...;
    ...
    if (calc_one_cycle(I_arr[i], V_arr[i], P_arr[i], F_arr[i], &Lv, 0))
        L_raw[valid_cnt++] = Lv;  // ← valid_cnt 与 i 不再同步！
}

/* Step 2: I_peak 阈值过滤 */
for (i = 0; i < valid_cnt; i++) {
    if (I_arr[i] >= I_thr) {     // ← 用 I_arr[i]，但 L_raw[i] 对应的是第 i 个"有效"周期
        I_kept[kept] = I_arr[i]; // 若某周期 calc_one_cycle 返回 false，I_arr[i] 和 L_raw[i] 错位！
        L_kept[kept] = L_raw[i];
    }
}
```

**问题**：当某些周期 `calc_one_cycle` 失败时，`L_raw[k]` 对应的是第 k 个成功的周期，但 `I_arr[k]` 对应的是原始第 k 个周期（包含失败的），两者索引错位，导致 `I_arr[i]` 和 `L_raw[i]` 配对混乱。

**修正**：用同一有效索引同步记录 I_arr 和 L_raw：
```c
float I_valid[20];  // 与 L_raw 同步
if (calc_one_cycle(I_arr[i], V_arr[i], P_arr[i], F_arr[i], &Lv, 0)) {
    I_valid[valid_cnt] = I_arr[i];
    L_raw[valid_cnt] = Lv;
    valid_cnt++;
}
// Step 2 改用 I_valid 而非 I_arr
```

---

### ⚠️ 2.4 中等：Step 2 阈值计算基准有误

**设计**：`I_peak < 中位数×0.3 丢弃`

**C 代码**（c:105~106）：
```c
float I_med = median_f(I_arr, cnt);   // 全部 cnt 个，含无效周期
float I_thr = I_med * IH_I_PEAK_MIN_RATIO;
```

`I_arr` 包含所有周期（含 `calc_one_cycle` 失败的），应只对有效周期计算中位数。在正常情况下两者差异不大，但极端情况（大量无效周期）会导致阈值偏低，无效过滤。

---

### ⚠️ 2.5 中等：Step 3 L_ref 查找逻辑不健壮

**C 代码**（c:123~124）：
```c
float L_ref = L_kept[0];
for (i = 0; i < kept; i++) if (I_kept[i] >= I_max - 0.1f) { L_ref = L_kept[i]; break; }
```

**问题**：
1. `I_max - 0.1f` 是硬编码 0.1A 浮窗，与量程无关。I_max 若为 40A 则窗口占 0.25%，合理；若为 5A 则占 2%，偏宽。
2. 多个周期可能满足条件，只取第一个，可能不是真正的最高值周期。

**设计文档**要求：找 `argmax(I_peak)` 对应的 L 作为 L_ref，直接用：
```c
uint8_t i_max_idx = 0;
for (i = 1; i < kept; i++) if (I_kept[i] > I_kept[i_max_idx]) i_max_idx = i;
float L_ref = L_kept[i_max_idx];
```

---

### ⚠️ 2.6 中等：φ 还原上限 90° 硬截断

**C 代码**（c:157~158）：
```c
out->phi_deg = phi_raw / duty_pu;
if (out->phi_deg > 90.0f) out->phi_deg = 90.0f;  // 硬截
```

`tan(90°) = ∞`，后续 `Q = tan(φ_rad) / denom`（c:174）会得到 INF。设计文档未规定截断，且直接截 90° 会让边界情况产生非物理的大 Q 值。

建议改为：若 `phi_deg >= 89.0f`，设 valid=false 或输出警告。

---

### ⚠️ 2.7 中等：ctypes 结构体与 C 结构体字段不匹配

**Python** `C_ElecResult`（py:101~121）包含 18 个 float 字段（`omega_sw`、`V_C_peak`、`X_L_ohm`、`X_C_ohm`、`omega_res`…），

**C** `IH_ElecResult`（h:41~61）输出结构体**没有**这些中间值字段（无 `omega_sw`/`V_C_peak`/`omega_res`/`X_L_ohm`/`X_C_ohm`）。

**C 结构体确实有** `f_sw_Hz`、`L_stable`、`L_fast`、`event` 等字段，但 Python 的 `C_ElecResult` 完全没有映射这些字段。

**结论**：Python ctypes 封装的 `C_ElecResult` 是给一个**不同版本**的 C 接口写的，当前 `ih_elec_params.h` 的实际结构体与 Python ctypes 映射**不匹配**。如果直接用 `ih_elec_params.py --dll` 来测当前代码，结果会是错的（内存错位）。

---

### 🔹 2.8 轻微：f_sw=0 防御

**C 代码**（c:95）：
```c
F_arr[i] = (float)in->f_sw_buf[i] * IH_FSW_SCALE;
```

若 `f_sw_buf == NULL`（h:35 定义但不是必传）会 NULL 解引用。设计文档第 2.1 节标注 `f_sw_buf` 为"暂缓"，实际代码没有判空保护。

---

### 🔹 2.9 轻微：valid 条件过宽

**C 代码**（c:190）：
```c
out->valid = (out->L_uH > 1.0f) && (out->f_res_kHz > 1.0f);
```

L_uH=1.01μH 即通过，实际工作范围应为 37~139μH。建议下限改为至少 10μH，或增加 Q>0.1 && R>0 条件。

---

## 三、问题汇总

| # | 严重度 | 位置 | 问题 |
|---|--------|------|------|
| 1 | ❌ 严重 | c:59-62 | `omega_res` 计算方式极度混乱（结果正确但不可读） |
| 2 | ❌ 严重 | c:110-115 | I_arr 与 L_raw 索引错位（含无效周期时数据配对错误） |
| 3 | ❌ 严重 | py:93-121 | Python ctypes C_ElecResult 结构体与当前 C 头文件不匹配 |
| 4 | ⚠️ 中等 | c:105 | 中位数阈值基于全部周期而非有效周期 |
| 5 | ⚠️ 中等 | c:123-124 | L_ref 查找用 0.1A 浮窗非 argmax，不健壮 |
| 6 | ⚠️ 中等 | c:157-158 | φ 截 90° 后 tan→∞，未保护 Q/R 计算 |
| 7 | 🔹 轻微 | c:95 | f_sw_buf 无判空（文档标注"暂缓"） |
| 8 | 🔹 轻微 | c:190 | valid 下限 1μH 过宽，工作范围应 37~139μH |

---

## 四、核实通过项

| 项 | 结论 |
|----|------|
| KVL L 公式 | ✅ 与设计/Python/MATLAB 完全一致 |
| f_res = 1/(2π√LC) | ✅ 正确（Step 6 部分） |
| Q = tan(φ)/(ratio − 1/ratio) | ✅ 正确 |
| R = ω₀L/Q | ✅ 正确 |
| P = I_rms²R | ✅ 正确 |
| Z = √(R²+X²) | ✅ 正确 |
| 插入排序中位数 | ✅ 正确，n≤20 合适 |
| Vdc 取全部均值 | ✅ 与设计一致 |
| Kalman 占位 | ✅ 标注"待实现"，占位合理 |
| 编译验证 | ✅ armclang+gcc 0 error |

---

## 五、必须修复的 3 项（上机前）

1. **I_arr/L_raw 索引错位**（#2）：同步维护 I_valid 数组
2. **L_ref 改 argmax**（#5）：删掉 0.1A 浮窗，用最大值索引
3. **Python ctypes 对齐**（#3）：`C_ElecResult` 字段重新按 `ih_elec_params.h` 映射

其余可后续迭代处理。

---

*审查方：李工 + AI 参谋长*  
*生成日期：2026-06-04*
