# CalculatePower 原始做法 vs 新版 FPU 对比

> 文件：`BaseClass/src/power_calculator.c`
> 目的：理解原始整数算法，确定哪些步骤需要浮点、哪些保持整数

---

## 一、原始算法（CalculatePower，line 961）

### 整体流程

```
FindZeroCrossing(上半周) → phaseUp         // 纯整数，方向跟踪找谷点
FindZeroCrossing(下半周) → phaseDown       // 纯整数

CalculateAuctalCurrent(phaseUp)  → sumUp   // 上半周积分+峰值
CalculateAuctalCurrent(phaseDown) → sumDown // 下半周积分+峰值（非对称时）

if (对称)  sumAll = sumUp.I × sumUp.V × 2
else       sumAll = sumUp.I × sumUp.V + sumDown.I × sumDown.V

P = sumAll / lowOff                        // 归一化
```

### FindZeroCrossing（line 764）— 纯整数

```
方向跟踪：int16_t delat = newValue - preValue
          direction <<= 1; if(delat>0) direction |= 1
拐点检测：(direction & 0x3) == 0x1
最小值筛选：current[pi-1] + current[pi+1] 最小 且 current[pi] 够小
```

无 float。无 I_zero。只比较 ADC 原始值的方向变化。

### CalculateAuctalCurrent（line 838）— 纯整数，一遍扫描

```
for (num = start; num < end; num++):
    currentSum += adc_i[num]           // 整数累加
    vcSum += adc_v[num]                // 电压累加
    if (adc_i[num] > currentMax)       // 顺手找峰值
        currentMax = adc_i[num]
        currentMaxSum = currentSum     // 峰值处的累加和
        currentMaxNum = num            // 峰值位置
```

返回 `{ voltage: vcSum, current: currentSum }` — 两个 uint32_t 累加值。

**一遍扫描同时完成：积分 + 峰值 + 峰值位置 + 峰值前后判断。**

### 功率归一化（纯整数）

```
sumAll   = sumUp.current × sumUp.voltage × 2    // 对称
         = sumUp.current × sumUp.voltage + sumDown.current × sumDown.voltage  // 非对称
sumAll  /= lowOff                                // 归一化到周期
P_int    = sumAll >> 4                            // 整数定标功率
```

### 相位角（纯整数，半周期 180° 基准，去死区）

```
angle = hrtim[phaseUp] - highOn                   // 过零延迟
angle *= PHASE_DEG_BASE (1800)                    // ×180°×10
angle /= (highOff - highOn)                       // ÷导通时间（去死区）
```

---

## 二、新版 FPU（CalculatePower_FPU，line 1209）

### 新增的不必要的东西

| 新增 | 问题 |
|------|------|
| `_EstimateIzero` | I_zero 是硬件偏置常数，不需要估算 |
| `_FindValley_f` | 算法同 FindZeroCrossing，多了 float 转换 |
| `I_act = (ADC - I_zero) × I_SCALE` 每个采样点 | 用整数累加后一次性转换即可 |
| `weight = dist / dCNT` 插值权重 | 关断点插值有用，但可整数化 |
| `I_rms = I_peak / √2` | 只对纯正弦成立 |
| 全缓冲区扫描 I_max | 积分循环内已有峰值 |
| 兼容字段整数转换 | 按需调用，不每次跑 |

### 比原始版好的地方

| 项目 | 说明 |
|------|------|
| sym 对称判断 | 原始版也有，逻辑相同 |
| 关断点线性插值 | 原始版没有，精度提升 |
| float 输出 | P_W/I_rms/phi 用 float 直接给物理值，下游不用再转 |

---

## 三、修正原则

1. **过零检测**：用原始 `FindZeroCrossing`（纯整数），删 `_FindValley_f`
2. **积分**：继承原始 `CalculateAuctalCurrent` 的一遍扫描模式（整数累加 + 顺手找峰值），最后一次性转浮点。保留关断点插值
3. **I_zero**：硬件常数 `#define`，不估算
4. **sym 对称**：提前判断，`v_down = v_up; s_down = s_up`
5. **P_W 归算**：`(s_up + s_down) / N_cycle`，单行公式
6. **phi**：用原始版半周期 180° + 去死区做法，具体归一化由 L 链条验证
7. **兼容字段**：抽成独立函数 `ConvertToIntegerFields`，按需调
8. **命名**：全拼英文，花括号不省略
