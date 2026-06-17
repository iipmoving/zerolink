# RX32G410 HRTIM MASTER 统一时钟源配置

> 来源：RX32G410_Reference_Manual_v0p7 — 第22章 高分辨率定时器（HRTIM）
> 提取目标：移相全桥项目中，所有HRTIM定时器以MASTER为统一时钟源的配置方法

---

## 1. 背景与目的

- **原系统（半桥）**：HRTIM各通道独立时钟、IO口同步，存在相位偏移
- **新系统（移相全桥）**：需要MASTER统一时钟同步所有通道，消除相位偏移
- **目标**：TimerA（超前臂Q1/Q2互补）和TimerC（滞后臂Q3/Q4互补）共用MASTER时钟，TimerC相对TimerA可调移相角

---

## 2. HRTIM时钟架构

### 2.1 时钟层次

```
fHRTIM (hrtim_ker_ck)  ── 主HRTIM时钟（APB2时钟）
    │
    ├── 定时器A 时钟预分频器 (CKPSCA[2:0])  →  fCOUNTER_A
    ├── 定时器B 时钟预分频器 (CKPSCB[2:0])
    ├── 定时器C 时钟预分频器 (CKPSCC[2:0])  →  fCOUNTER_C
    ├── 定时器D 时钟预分频器 (CKPSCD[2:0])
    ├── 定时器E 时钟预分频器 (CKPSCE[2:0])
    ├── 定时器F 时钟预分频器 (CKPSCF[2:0])
    ├── 主定时器 时钟预分频器 (CKPSC[2:0])  →  fCOUNTER_M
    │
    ├── 死区发生器时钟 (fDTG)
    ├── 斩波级时钟 (fCHPFRQ)
    ├── 突发模式时钟 (fBRST)
    └── 滤波采样时钟 (fSAMPLING / fFLTS / fEEVS)
```

所有时钟均源自 `fHRTIM`（hrtim_ker_ck），通过边缘定位逻辑将 `tHRTIM` 时钟周期均匀分为最多32个中间步骤，实现 **163 ps 高分辨率**。

### 2.2 预分频器配置表

| CKPSC[2:0] | 预分频比 | fHRCK等效频率（fHRTIM=192MHz） | 分辨率 | 最小PWM频率 |
|-----------|---------|-------------------------------|-------|------------|
| 000 | 1 | 6.14 GHz | 163 ps | 93.74 kHz |
| 001 | 2 | 3.07 GHz | 326 ps | 46.87 kHz |
| 010 | 4 | 1.54 GHz | 651 ps | 23.49 kHz |
| 011 | 8 | 768 MHz | 1.30 ns | 11.75 kHz |
| 100 | 16 | 384 MHz | 2.60 ns | 5.86 kHz |
| 101 | 32 | 192 MHz | 5.21 ns | 2.93 kHz |
| 110 | 64 | 96 MHz | 10.41 ns | 1.47 kHz |
| 111 | 128 | 48 MHz | 20.83 ns | 0.73 kHz |

### 2.3 关键约束（重要！）

> **警告**：仅当计数器与输出行为与其他定时器的信息和信号无关时，主定时器和 TIMA..E 定时器才能使用不同的预分频比。
>
> 如果以下某个事件从一个定时单元（或主定时器）传输到另一个定时单元，则**务必要在这些定时器中配置相同的预分频比**：
> - 输出置位/复位事件
> - 计数器复位事件
> - 更新事件
> - 外部事件过滤
> - 捕获触发
>
> **预分频系数不相等会导致结果不可预测。**

**对移相全桥的影响**：TimerA 和 TimerC 之间有计数器复位和输出置位/复位的事件传播，因此 **必须配置相同的 CKPSC 值**。

---

## 3. 主定时器（MASTER）架构

### 3.1 主定时器作用

主定时器的**主要用途**是向6个定时单元提供公用信号，以便进行同步或置位/复位输出。

主定时器本身**不直接控制任何输出**，但可通过置位/复位纵横开关间接影响输出。

### 3.2 主定时器特点

| 特性 | 说明 |
|------|------|
| 计数器 | 16位递增计数器 |
| 比较单元 | 4个（MCMP1R ~ MCMP4R） |
| 输出控制 | 无直接输出，无纵横开关 |
| 死区/推挽 | 不支持 |
| 捕获单元 | 无 |
| 外部事件消隐/窗口 | 无 |
| 中断/DMA | 有限：比较1~4、重复、更新、外部同步 |
| 外部同步 | 支持同步输入/输出，可级联多个HRTIM实例 |

### 3.3 主定时器控制寄存器（HRTIM_MCR）

HRTIM_MCR 包含了**主定时器和A~F定时单元的所有定时器使能位**，可通过单次写访问同时启动所有定时器。

关键位域：
- **CKPSC[2:0]**（Bit 2:0）：主定时器时钟预分频器
- **CONT**（Bit 3）：连续模式
- **HALF**（Bit 5）：半占空比模式
- **INTLVD[1:0]**（Bit 7:6）：交错模式（自动生成相移）
- **MCEN**（Bit 16）：主定时器计数器使能
- **TxCEN**（Bit 17~22）：定时器A~F计数器使能
- **SYNCIN[1:0]**（Bit 9:8）：同步输入源选择
- **SYNCRSTM**（Bit 10）：同步复位主定时器
- **SYNCSTRTM**（Bit 11）：同步启动主定时器
- **SYNCOUT[1:0]**（Bit 13:12）：同步输出路由
- **SYNCSRC[1:0]**（Bit 15:14）：同步输出源选择

---

## 4. 以MASTER为统一时钟源的配置方法

### 4.1 时钟预分频配置

所有参与事件传播的定时器（MASTER、TimerA、TimerC）必须配置**相同的 CKPSC 值**。

```c
// 示例：fHRTIM = 192MHz，CKPSC = 4分频
// fCOUNTER = 192MHz / 4 = 48MHz
// 高分辨率 = 651ps

HRTIM->MCR   |= (2 << 0);  // CKPSC[2:0] = 010 (4分频)
HRTIM->TIMACR |= (2 << 0); // CKPSCA[2:0] = 010 (与MASTER相同)
HRTIM->TIMCCR |= (2 << 0); // CKPSCC[2:0] = 010 (与MASTER相同)
```

### 4.2 计数器复位配置——实现移相

移相全桥的相位关系通过**主定时器比较事件复位定时器计数器**实现：

| 定时器 | 复位源 | 效果 |
|--------|--------|------|
| TimerA | MSTPER（主定时器周期） | TimerA 在主周期开始时复位 |
| TimerC | MSTCMP1（主定时器比较1） | TimerC 在主周期内偏移 MCMP1 处复位 |

```c
// TimerA 由主定时器周期复位
HRTIM->RSTAR |= (1 << 4);  // MSTPER = 1

// TimerC 由主定时器比较1复位（移相角 = MCMP1 / MPER * 360°）
HRTIM->RSTCR |= (1 << 5);  // MSTCMP1 = 1
```

**主定时器周期和比较值设置**：
```c
HRTIM->MPER  = PWM_PERIOD;      // 主定时器周期（决定开关频率）
HRTIM->MCMP1R = PHASE_SHIFT_VAL; // 比较1值（决定移相角）
```

> **注**：主定时器也可以使用 `INTLVD[1:0]`（交错模式）自动生成相移值：
> - `01`：三重交错 — MCMP1 = MPER/3，MCMP2 = 2×MPER/3
> - `10`：四重交错 — MCMP1 = MPER/4，MCMP2 = MPER/2，MCMP3 = 3×MPER/4

### 4.3 输出置位/复位配置

Q1/Q2（TimerA 输出1/输出2）和 Q3/Q4（TimerC 输出1/输出2）的波形通过 SETx1R/SETx2R 和 RSTx1R/RSTx2R 寄存器配置。

**配置举例（对称互补PWM）**：

```c
// === TimerA 输出（Q1: TA1, Q2: TA2）===

// Q1 置位源：TA周期事件（周期开始时置位）
HRTIM->SETA1R |= (1 << 0);  // 定时器A周期 → 置位TA1

// Q1 复位源：TA比较1（占空比控制）
HRTIM->RSTA1R |= (1 << 0);  // 定时器A比较1 → 复位TA1

// Q2 置位源：TA比较1（与Q1互补）
HRTIM->SETA2R |= (1 << 2);  // 定时器A比较1 → 置位TA2

// Q2 复位源：TA周期
HRTIM->RSTA2R |= (1 << 0);  // 定时器A周期 → 复位TA2


// === TimerC 输出（Q3: TC1, Q4: TC2）===

// Q3 置位源：TC周期（被MSTCMP1复位后开始计数，周期到来时置位）
HRTIM->SETC1R |= (1 << 0);  // 定时器C周期 → 置位TC1

// Q3 复位源：TC比较1
HRTIM->RSTC1R |= (1 << 0);  // 定时器C比较1 → 复位TC1

// Q4 置位源：TC比较1
HRTIM->SETC2R |= (1 << 2);  // 定时器C比较1 → 置位TC2

// Q4 复位源：TC周期
HRTIM->RSTC2R |= (1 << 0);  // 定时器C周期 → 复位TC2
```

> **说明**：以上为不使用死区插入（DTEN）时的软件置位/复位方式。实际推荐使用硬件死区插入（见4.4节）。

### 4.4 死区配置

如果使能死区插入，只需配置一对置位/复位事件，互补输出由硬件自动生成。

```c
// 使能死区插入
HRTIM->TIMACR |= (1 << 16); // DTAEN = 1 (TimerA死区使能)
HRTIM->TIMCCR |= (1 << 16); // DTCEN = 1 (TimerC死区使能)

// 死区时间配置
HRTIM->DTAR |= (50 << 0);   // 死区时间 = 50 × tDTG
HRTIM->DTCR |= (50 << 0);   // (实际值根据应用计算)
```

### 4.5 初始化流程

按照规格书 22.3.24 节建议的HRTIM初始化顺序：

```
步骤1：RCC时钟使能
  → 使能HRTIM时钟源，满足DLL锁定所需的fHRTIM范围

步骤2：DLL校准
  → 设置 HRTIM_DLLCR.CAL = 1，启动DLL校准
  → 轮询 HRTIM_DLLCR.DLLRDY，等待校准完成
  → 设置 HRTIM_DLLCR.CALEN = 1，使能定期校准补偿

步骤3：预分频器配置（必须先于比较/周期寄存器写入！）
  → 配置 HRTIM_MCR.CKPSC[2:0]（主定时器）
  → 配置 HRTIM_TIMACR.CKPSCA[2:0]（TimerA）
  → 配置 HRTIM_TIMCCR.CKPSCC[2:0]（TimerC）
  → 【警告】三者必须相同！

步骤4：周期和比较寄存器
  → HRTIM_MPER, HRTIM_MCMP1R（主定时器）
  → HRTIM_PERAR, HRTIM_CMP1AR（TimerA）
  → HRTIM_PERCR, HRTIM_CMP1CR（TimerC）

步骤5：计数器复位源
  → HRTIM_RSTAR.MSTPER = 1（TimerA由主周期复位）
  → HRTIM_RSTCR.MSTCMP1 = 1（TimerC由主比较1复位）

步骤6：输出置位/复位配置
  → HRTIM_SETA1R, HRTIM_RSTA1R（Q1）
  → HRTIM_SETA2R, HRTIM_RSTA2R（Q2）
  → HRTIM_SETC1R, HRTIM_RSTC1R（Q3）
  → HRTIM_SETC2R, HRTIM_RSTC2R（Q4）

步骤7：输出极性/故障/空闲状态
  → HRTIM_OUTAR.POLA, HRTIM_OUTAR.FAULTA, HRTIM_OUTAR.IDLESA
  → HRTIM_OUTCR.POLC, HRTIM_OUTCR.FAULTC, HRTIM_OUTCR.IDLESC

步骤8：GPIO复用功能配置
  → 按数据手册复用功能映射表配置HRTIM I/O引脚

步骤9：使能HRTIM输出（进入RUN模式）
  → HRTIM_OENR.TA1OEN = 1, HRTIM_OENR.TA2OEN = 1
  → HRTIM_OENR.TC1OEN = 1, HRTIM_OENR.TC2OEN = 1

步骤10：启动计数器
  → HRTIM_MCR.MCEN = 1（启动主定时器）
  → HRTIM_MCR.TACEN = 1（启动TimerA）
  → HRTIM_MCR.TCCEN = 1（启动TimerC）
  → 注：可以同时写MCR一次性启动所有定时器
```

---

## 5. 寄存器参考

### 5.1 HRTIM_MCR（主定时器控制寄存器）

偏移地址：0x000 | 复位值：0x0000 0000

| 位域 | 位 | 描述 | 移相全桥配置 |
|------|---|------|-------------|
| CKPSC[2:0] | 2:0 | 时钟预分频器 | 与TIMxCR相同 |
| CONT | 3 | 连续模式 | 1（连续运行） |
| RETRIG | 4 | 可再触发模式 | 0（不可再触发） |
| HALF | 5 | 半占空比模式 | 0 |
| INTLVD[1:0] | 7:6 | 交错模式 | 00（手动控制） |
| SYNCIN[1:0] | 9:8 | 同步输入源 | 00（独立运行） |
| SYNCRSTM | 10 | 同步复位主定时器 | 0 |
| SYNCSTRTM | 11 | 同步启动主定时器 | 0 |
| SYNCOUT[1:0] | 13:12 | 同步输出路由 | 00（禁用） |
| SYNCSRC[1:0] | 15:14 | 同步输出源 | 00（主启动） |
| MCEN | 16 | 主定时器使能 | 1 |
| TACEN | 17 | TimerA使能 | 1 |
| TCCEN | 19 | TimerC使能 | 1 |
| PREEN | 27 | 预加载使能 | 0（直写）或1 |

### 5.2 HRTIM_TIMxCR（定时器x控制寄存器）

偏移地址：TimerA=0x080, TimerB=0x100, TimerC=0x180 | 复位值：0x0000 0000

| 位域 | 位 | 描述 | 移相全桥配置 |
|------|---|------|-------------|
| CKPSCx[2:0] | 2:0 | 时钟预分频器 | 与MCR.CKPSC相同 |
| CONT | 3 | 连续模式 | 1 |
| RETRIG | 4 | 可再触发模式 | 0 |
| HALF | 5 | 半占空比模式 | 0 |
| PSHPLL | 6 | 推挽模式 | 0 |
| INTLVD[1:0] | 8:7 | 交错模式 | 00 |
| MSTU | 24 | 主定时器更新触发 | 1（可选，同步更新） |
| PREEN | 27 | 预加载使能 | 0或1 |
| UPDGAT[3:0] | 31:28 | 更新门控 | 0000 |

### 5.3 HRTIM_RSTAR / HRTIM_RSTCR（定时器复位寄存器）

**HRTIM_RSTAR**（偏移：0x0D4）

| 位 | 名称 | 描述 | 配置 |
|----|------|------|------|
| 4 | MSTPER | 主定时器周期复位TA | **1**（超前臂同步） |
| 5 | MSTCMP1 | 主定时器比较1复位TA | 0 |
| 3 | CMP4 | TA比较4复位TA | 0 |
| 2 | CMP2 | TA比较2复位TA | 0 |
| 1 | UPDT | 更新复位TA | 0 |

**HRTIM_RSTCR**（偏移：0x2D4）

| 位 | 名称 | 描述 | 配置 |
|----|------|------|------|
| 4 | MSTPER | 主定时器周期复位TC | 0 |
| 5 | MSTCMP1 | 主定时器比较1复位TC | **1**（滞后臂移相） |
| 6 | MSTCMP2 | 主定时器比较2复位TC | 0 |
| 8 | MSTCMP4 | 主定时器比较4复位TC | 0 |
| 3 | CMP4 | TC比较4复位TC | 0 |
| 2 | CMP2 | TC比较2复位TC | 0 |
| 1 | UPDT | 更新复位TC | 0 |

### 5.4 HRTIM_DTxR（死区时间寄存器）

| 位域 | 位 | 描述 |
|------|---|------|
| DTPRSC[2:0] | 31:29 | 死区时钟预分频器 |
| SDTFx | 26 | 下降沿符号位 |
| SDTRx | 25 | 上升沿符号位 |
| DTF[8:0] | 24:16 | 下降沿死区时间 |
| DTR[8:0] | 8:0 | 上升沿死区时间 |

死区时间计算：`tDTG = 1 / ((fHRTIM × 8) / 2^DTPRSC)`

### 5.5 HRTIM_OUTxR（输出寄存器）

| 位域 | 位 | 描述 |
|------|---|------|
| POLx | 各输出位 | 输出极性（0=高有效，1=低有效） |
| FAULTx[1:0] | 各输出位 | 故障状态电平 |
| IDLESx | 各输出位 | 空闲状态电平 |

---

## 6. 案例参考：多相转换器（移相架构）

规格书 22.4.3 节描述了多相交错降压转换器，其**主定时器负责相位管理**，与移相全桥的核心机制一致：

> 主定时器负责进行相位管理：它通过定期复位定时器来定义转换器之间的相位关系。相移为360°除以相位数。

输出定义逻辑：
- `HRTIM_CHA1`：在主定时器周期开始时置位，在TACMP1时复位
- `HRTIM_CHB1`：在主定时器MCMP1时置位，在TBCMP1时复位
- `HRTIM_CHC1`：在主定时器MCMP2时置位，在TCCMP1时复位

**移相全桥的映射关系**：

| 信号 | 定时器 | 置位事件 | 复位事件 | 说明 |
|------|--------|---------|---------|------|
| Q1（超前臂上管） | TA输出1 | TA周期 | TACMP1 | 先开后关 |
| Q2（超前臂下管） | TA输出2 | TACMP1 | TA周期 | Q1互补 |
| Q3（滞后臂上管） | TC输出1 | TC周期 | TCCMP1 | 先开后关 |
| Q4（滞后臂下管） | TC输出2 | TCCMP1 | TC周期 | Q3互补 |

**注**：如果使用死区插入（DTEN=1），只需配置TA/TC输出1的置位/复位事件，输出2的互补波形由硬件自动插入死区。

---

## 7. 同步机制补充

如果需要与外部定时器或另一HRTIM实例同步，配置HRTIM_MCR：

```c
// 外部同步输入（如TIM1_TRGO）
HRTIM->MCR |= (2 << 8);  // SYNCIN[1:0] = 10（内部事件hrtim_in_sync2）
HRTIM->MCR |= (1 << 11); // SYNCSTRTM = 1（同步启动主定时器）

// 同步输出（级联HRTIM实例）
HRTIM->MCR |= (1 << 15) | (1 << 14); // SYNCSRC[1:0] = 00（主启动）
HRTIM->MCR |= (2 << 12);              // SYNCOUT[1:0] = 10（正脉冲输出到SCOUT引脚）
```

---

## 8. 配置检查清单

- [ ] **CKPSC[2:0]** — MCR、TIMACR、TIMCCR 三者是否相同？
- [ ] **MCEN / TACEN / TCCEN** — 是否所有使能位已置1？
- [ ] **RSTAR.MSTPER** — TimerA 是否由主定时器周期复位？
- [ ] **RSTCR.MSTCMP1** — TimerC 是否由主定时器比较1复位？
- [ ] **SETx1R / RSTx1R** 或 **死区使能** — 输出波形是否正确？
- [ ] **DLL校准** — DLLRDY 标志是否检查？CALEN 是否使能？
- [ ] **预分频器写入顺序** — 是否在配置比较/周期寄存器**之前**写入了CKPSC？
- [ ] **PRELOAD** — 是否需要预加载同步更新？
- [ ] **GPIO复用** — HRTIM输出引脚是否正确配置？
