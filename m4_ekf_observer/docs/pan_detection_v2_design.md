# 锅具检测高速采样与数据分析 — 设计规范 V1.0

> 设计日期: 2026-06-24
> 涉及项目: m4_ekf_observer (RX32G410, src3)
> 关键词: PAN ADC, DMA, wave_capture, HRTIM 单脉冲, 自由振荡分析

---

## 一、问题清单

| #   | 问题            | 现状                                  | 目标                                            |
| --- | ------------- | ----------------------------------- | --------------------------------------------- |
| P1  | 采样精度不足        | 1μs采样，83kHz仅12点/周期，峰值定位误差±1μs(±12%) | 0.25μs等效采样，48点/周期，误差<±3%                      |
| P2  | ADC交替采样干扰     | PAN与VOLTAGE交替，检锅时VC采样点浪费            | 检锅时ADC序列改为PAN→PAN，不采VC                        |
| P3  | FMAC滤波->RAM不足 | FMAC FIR滤波占用额外~8KB及处理延迟             | 去掉FMAC，用原始数据前端插值                              |
| P4  | DMA缓存仅300点    | 300μs时间窗不够长                         | 改用wave_capture.data[4000]做DMA目标，采2000点(500μs) |
| P5  | 检锅脉冲数偏多(3-4个) | HRTIM连续输出多个脉冲后关断去采DMA               | 单脉冲模式：1个激励脉冲→立即采DMA                           |
| P6  | 自由振荡频率离散大     | 同组数据频率波动~10%                        | 用前几个大信号周期的峰-峰间隔计算                             |
| P7  | 有效脉冲计数不准      | 噪声峰被算入递减链                           | 严格递减 + 最小衰减门槛                                 |

---

## 二、硬件方案

### 2.1 ADC 采样 — PAN→PAN 连续模式

```
正常模式:  VC → PAN (TIM7触发, 交替采样)
检锅模式:  PAN → PAN (ADC连续转换, 自触发)
```

| 参数 | 正常 | 检锅 |
|------|------|------|
| ADC序列 | VC, PAN (2ch) | PAN, PAN (2ch同样的PAN) |
| 触发源 | TIM7_UP | ADC自身EOC |
| 等效间隔 | 1μs | ~0.25μs |
| 每次DMA搬运 | 2个值(VC+PAN) | 2个PAN值 |

检锅前调 `API_ADC_Pan_ConfigChannel(1)`，结束后调 `API_ADC_Pan_ConfigChannel(0)`。

### 2.2 DMA — 目标指向 wave_capture

当前: `Pan_ADC_AdcDmaBuff[300]` → `FMAC` → `Pan_ADC_AdcFmacBuff`

改为: `ADC DR` → DMA直写 → `wave_capture.data[4000]`

```c
// wave_capture.h
#define WAVE_MAX_DATA_WORDS  4000    // 8KB缓存，足够2000点

// 检锅前配置DMA
recover.DstAddress = (uint32_t)WaveCapture_GetDataPtr();  // wf->data
recover.DataLength = PAN_DMA_BUF_SIZE;                    // 2000
API_DMA_RECOVER(ChDmaPan, &recover);
```

优势: 零额外RAM开销，wave_capture已有 ~8KB 缓存。

### 2.3 无 FMAC 滤波

去掉 `API_POWER_PanFmac()` 调用链:

| 删除项 | 文件 | 说明 |
|--------|------|------|
| `API_POWER_PanFmac()` 调用 | app_power_drv.c | FMAC 启动 |
| `Pan_ADC_AdcFmacBuff` 分配 | app_power_drv.c | PubicBuffCalloc x2 |
| `PanFmacEnd` 状态判断 | app_power_drv.c | 状态机转移 |
| 省 RAM | — | ~8KB (2 × 2000 × uint16_t) |

原始数据经 DMA 直写 wave_capture，PC 端MATLAB对前几个大信号周期做抛物线插值求频率。

### 2.4 HRTIM 单脉冲模式

当前: HRTIM 连续输出 ~3-4 个脉冲后停止 → DMA 采集

目标: 输出 **1 个激励脉冲** 后立即停止，启动 ADC DMA 采集

实现方案: 利用 HRTIM 的 **单脉冲模式 (Single Shot / One Pulse Mode)**：

```c
// HRTIM 单脉冲配置思路:
// 1. 设置 OPM (One Pulse Mode) 位: 在第一个更新事件后自动停止计数
// 2. 输出 1 个完整脉冲(PER跳变→CMP比较→PER跳变)
// 3. 停止后触发 ADC DMA 开始采集

void drv_hrtim_single_pan_pulse(uint8_t ch)
{
    // 配置 HRTIM Timer 为单脉冲模式
    // HRTIM_TimeBaseConfig.OPM = 1;  // 单脉冲

    // 配置输出: 在 PER 置位、CMP 复位 (1个脉冲)
    // HRTIM_OutputConfig.IdleLevel = HRTIM_OUTPUTIDLELEVEL_INACTIVE;

    // 使能 Timer → 输出 1 个脉冲 → 自动停止
    // 停止后通过中断或标志启动 ADC DMA
}
```

当前 `API_PPG_SET_SINGLE(ch)` 已声明但未实现(`//`注释掉)，正好在此实现。

### 2.5 缓存大小汇总

| 缓存 | 大小 | 位置 | 说明 |
|------|------|------|------|
| wave_capture.data[] | 4000 words (8KB) | wave_capture.c | DMA写入2000个PAN原始值 |
| PubicBuffCalloc ×2 | — | 删除 | FMAC去掉，不再分配 |
| Pan_ADC_AdcDmaBuff[] | — | 删除或保留小尺寸 | DMA已指向wave_capture |
| **净增RAM** | **0KB** | — | 复用wave_capture缓存 |

---

## 三、数据分析方案

### 3.1 采样率变化

| 参数 | 当前 (1μs) | 新 (0.25μs) |
|------|-----------|-------------|
| 采样点数 | 300 | 2000 |
| 时间跨度 | 300μs | 500μs |
| 83kHz 周期内点数 | 12 | **48** |
| 抛物线插值精度 | ~0.1采样点 (0.1μs) | ~0.02采样点 (0.005μs) |
| 10周期平均误差 | ~0.05kHz | ~0.006kHz |

### 3.2 频率计算 — 仅前几个大信号周期

```python
# 分析流程:
# 1. 从原始数据找出激励脉冲峰值 (max)
# 2. 从峰值后找振荡峰值 (min间隔8采样点, 抛物线插值精确位置)
# 3. 取前N个(N=3~5)振荡峰的间隔计算频率:
#    f = 1000 / mean(peak_pos[i+1] - peak_pos[i])  (kHz)
# 4. 不用尾部噪声段 (SNR降低会导致频率误差放大)
```

理由: 振荡刚开始时幅度最大、SNR最高，周期最稳定。尾部接近噪声的振荡峰包含的周期信息不可靠。

### 3.3 有效脉冲计数 — 严格递减 + 最小衰减门槛

```python
# 规则:
# 1. 从第一振荡峰开始
# 2. 每次必须严格递减 (peak[n] < peak[n-1])
# 3. 且衰减量 >= 0.5% 首峰 (防止噪声误判)
# 4. 不满足 → 立即停止

valid_n = 1
min_drop = first_osc_peak * 0.005
for i in range(1, len(osc_peaks)):
    drop = osc_peaks[i-1] - osc_peaks[i]
    if drop >= min_drop:
        valid_n += 1
    else:
        break
```

| 场景 | 当前(1μs, 300点) | 新(0.25μs, 2000点) |
|------|-----------------|-------------------|
| 空载 ~83kHz | 20个递减峰 | ~85个递减峰 (更完整的衰减曲线) |
| 铁锅 ~100kHz | 7个 | ~30个 |
| 钢锅 ~117kHz | 5个 | ~20个 |

### 3.4 MATLAB 脚本更新

唯一改动: 采样率参数

```matlab
% 当前 (1μs):
FS_HZ = 1e6;  DT_US = 1.0;
N_PER_GRP = 300;

% 新 (0.25μs):
FS_HZ = 4e6;  DT_US = 0.25;
N_PER_GRP = 2000;
```

---

## 四、固件改动文件清单

| 文件 | 改动内容 | 优先级 |
|------|---------|--------|
| `API_adc.h` | 改 `Pan_ADC_DMA_BUFF_NUM` 或不用 | P2 |
| `API_ADC.c` | 实现 `Pan_ConfigChannel(1)` 的 PAN→PAN 序列 | P2 |
| `API_DMA.C` | DMA 目标地址指向 wave_capture.data[] | P4 |
| `app_power_drv.c` | 删除 FMAC 链 (`API_POWER_PanFmac` 调用) | P3 |
| `app_power_drv.c` | DMA 完成回调中改用 wave_capture | P4 |
| `drv_hrtim.c` | 实现 `API_PPG_SET_SINGLE(ch)` 单脉冲模式 | P5 |
| `wave_capture.h/c` | 确保缓冲期不被MODBUS并发读取 | P4 |
| `analyze_pot_detection.m` | 更新采样率参数 1μs→0.25μs | — |

---

## 五、实施的先后顺序

```
Step 1: HRTIM 单脉冲 (P5)   → drv_hrtim.c
   → 确认单脉冲模式下检锅时序
   ↓
Step 2: ADC序列切换 (P2)    → API_ADC.c
   → PAN→PAN模式 + DMA指向wave_capture
   ↓
Step 3: 去掉FMAC (P3)       → app_power_drv.c
   → 删除滤波调用 + 释放RAM
   ↓
Step 4: MATLAB脚本更新       → analyze_pot_detection.m
   → 0.25μs采样率 + 2000点
   ↓
Step 5: 联调验证             → 硬件实测对比
```

---

## 六、验证方法

1. **硬件**: 用示波器测量检锅 → 确认只有 1 个激励脉冲
2. **DMA**: 检查 wave_capture.data[] 的 2000 点 PAN 原始数据
3. **频率**: MATLAB 分析前 5 个振荡峰周期，与示波器对比
4. **脉冲数**: 严格递减计数，空载 > 50、带锅 < 20
5. **RAM**: 确认去掉 FMAC 后堆空间充足

---

*设计版本 V1.0, 更新于 2026-06-24*