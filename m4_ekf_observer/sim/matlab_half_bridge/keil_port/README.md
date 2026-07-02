# RX32G410 移植指南 — 半桥感应加热参数计算

## 芯片信息

| 参数 | 值 |
|------|-----|
| **型号** | 中山翰林电器 RX32G410 |
| **内核** | Cortex-M4F（FPUv4 + DSP 指令集） |
| **主频** | 192MHz（Flash 零等待） |
| **HRTIM** | 6 × 16位计数器, 162ps 分辨率, 12路PWM(6对互补) |
| **硬件加速** | CORDIC（三角函数）+ FMAC（FIR/IIR 滤波器） |
| **ADC** | 3 × 12位, 4Msps, 双分组(注入组), DMA |
| **Flash/RAM** | 512KB(无ECC) / 256KB(ECC) + 64KB SRAM |
| **特色** | 4×比较器 + 4×PGA(4~32倍) + 6×DAC + 高级电机定时器 |

## 文件清单

| 文件 | 用途 | 需修改? |
|------|------|---------|
| `ih_params.h` | 数据结构和硬件标定 | ✅ 修改 V_SCALE / I_SCALE / VDC_SCALE / CORDIC 基地址 |
| `ih_calculate.h` | 函数声明 | ❌ 无需修改 |
| `ih_calculate.c` | 全部算法 (CORDIC + M4 FPU) | ⚠️ 确认 CORDIC 寄存器地址匹配 HAL 头文件 |
| `main.c` | 主流程 + 外设模板 | ✅ 替换为 RX32G410 CubeMX 生成的 MX_ 函数 |

## Keil MDK 项目配置

### Target
- **Device**: RX32G410（需要安装厂商 Keil Pack）
- **FPU**: **Single Precision**（VFPv4）— **必须勾选**
- **AC6 flags**:
  ```
  -O2 -ffast-math -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -DUSE_CORDIC
  ```

### CORDIC 硬件加速

`ih_calculate.c` 使用 CORDIC 协处理器加速三角函数。寄存器地址：

| 寄存器 | 地址 (ih_params.h 默认) |
|--------|------------------------|
| CSR    | `0x40023000` |
| WDATA  | `0x40023004` |
| RDATA  | `0x40023008` |

**⚠️ 这些地址来自芯片通用假设。** 请在芯片数据手册中确认 `CORDIC_BASE` 的实际值，并在 `ih_params.h` 中修改。

若 HAL 库已有 `cordic_sin()` 等函数，替换为 HAL 调用即可。

### 内存需求

| 区域 | 大小 | 说明 |
|------|------|------|
| `.text` | ~8KB | 代码 (无 CMSIS-DSP, 更小) |
| `.data` | ~2KB | 全局变量 |
| `.bss` | ~32KB | RawPoint_t[2048] |
| 栈 | 2KB | 局部变量 |
| **总计** | **~8KB Flash + ~36KB RAM** | 对 512KB/64KB 芯片绰绰有余 |

## 从 MATLAB 到 C 的数据流

```
MATLAB 模型 (离线验证)
      │
      │  CSV 9列: t, I_adc, V_adc, Vdc_adc, CNT, CMP×4
      │
      ▼
RX32G410 实时 (在线计算)
  ┌─────────────┐
  │ HRTIM CNT   │──┐
  │ ADC1_IN0(I) │──┤  注入组同步采样
  │ ADC1_IN2(V) │──┤  @4Msps
  │ ADC1_IN3(Vdc) │─┤
  └─────────────┘  │
                    ▼
              DMA → RawPoint_t[2048]
                    │
              IH_Calculate()
                    │
              IH_Result_t
                    │
         ┌──────────┼──────────┐
         ▼          ▼          ▼
       UART      CORDIC      LCD/OLED
     (115200)    (trig加速)   (参数显示)
```

## HRTIM + ADC 同步关键配置

```
HRTIM 周期 = 20μs (50kHz)
  └─ CMP_UON  = 0        (上管开)
  └─ CMP_UOFF = 450      (上管关, D=45%)
  └─ 死区 = 50 cnts (~0.35μs)
  └─ CMP_LON  = CMP_UOFF + 50
  └─ CMP_LOFF = PERIOD - 50

ADC 注入组:
  └─ 触发源: HRTIM 周期更新事件
  └─ 采样率: 每 HRTIM 周期采样 N 点 (N=20 → 1Msps)
  └─ 每次注入: 3 通道 (I, V, Vdc)
  └─ DMA: 循环模式, 半满中断通知帧完成

时序:
  HRTIM CNT ─┬───────────────────────────┐
  ADC注入    │ I₁ V₁ Vdc₁  I₂ V₂ Vdc₂ ...
             ↓
  DMA buf    [I,V,Vdc, I,V,Vdc, ...]
```

## CORDIC 精度说明

| 函数 | 迭代次数 | 精度 | 对比 math.h |
|------|---------|------|------------|
| `sin_f` | 24 | ±1e-6 | 相同, 快 5~8x |
| `cos_f` | 24 | ±1e-6 | 相同, 快 5~8x |
| `atan2_f` | 24 | ±1e-5 | 足够 IH 计算 |

## 性能预估 (@192MHz)

| 计算 | 周期 | 时间 |
|------|------|------|
| 时序分析 | ~2K | ~10μs |
| 时域 (2000点) | ~15K | ~78μs |
| 功率 (2000点) | ~15K | ~78μs |
| 阻抗+RLC | ~3K | ~16μs |
| **总计** | **~35K** | **~182μs** |

模式A(2000点): ~182μs << 1s 帧间隔  
模式B(100点): ~15μs << 50ms 帧间隔  
**RX32G410 @192MHz 完全满足, 还有大量余量做控温/PID/通讯**
