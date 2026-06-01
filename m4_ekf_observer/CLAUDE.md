# M4 半桥 EKF 观测器 — 项目指令

> **定位**: 在已验证的 M4 半桥固件上增量开发 EKF 观测器 + 锅具检测 + 变增益 PID。
> **与 four_head 的区别**: 本项目是**修改现有固件**，不是构建零依赖架构。

---

## 一、项目身份

| 维度 | 说明 |
|------|------|
| **MCU** | RX32G410 (Cortex-M4F, 384MHz, 硬件 FPU) |
| **控制方式** | PC (Python) → MODBUS RTU → M4 从站 |
| **现有基线** | 半桥功率控制 + MODBUS 从站 (已验证) |
| **增量目标** | EKF 负载观测器 + 锅具检测 + 变增益 PID |
| **开发模式** | PC 端算法整定 → M4 固件集成 |

---

## 二、最优先规则：保护已验证基线

### 规则 1：不可修改区域 (No-Go)

以下文件/模块**只读不写**，修改必须有用户明确指令：

- `src/RX32G410_FW_HAL_V1.3N/` — 原厂固件库全部
- HRTIM 半桥驱动 (PER/CMP/DT 寄存器操作)
- ADC 采样逻辑 (测量链路)
- MODBUS 帧解析 (通讯链路)
- IGBT/硬件保护

**安全修改区域** (自由修改):
- `app/ekf/` — EKF 观测器
- `app/pot_detect/` — 锅具检测
- `app/var_gain_pid/` — 变增益 PID
- `app/data_logger/` — 数据记录
- `tools/ekf_tuner/` — PC 工具

详细规则见 @.claude/specs/modification-rules.md

### 规则 2：先 PC 后 M4

- EKF/PID 算法先在 Python 整定验证
- PC 工具跑通后，再移植 C 到 M4
- M4 上只跑已验证的逻辑

### 规则 3：最小化改动

- 只改任务要求的代码
- 不顺手重构无关代码
- 不改现有 MODBUS 寄存器含义 (会破坏 PC 工具兼容性)
- 新增寄存器在 0x1015+ 区域

### 规则 4：新模块零耦合 (自 2026-06-01 起执行)

新增或重构的单功能模块必须遵循：

1. **模块内部状态 `static`** — 禁止全局变量，只通过 API 函数暴露
2. **跨模块通信用 `__weak` 回调** — 调用方定义 `__attribute__((weak))` 默认空实现，实现方提供强覆盖。不在 `.h` 中声明回调函数
3. **值传递优先** — 回调参数用值类型，不传指针跨越模块边界
4. **一个 `.h` 只暴露 API** — 头文件只放结构体定义 + 公开函数声明，不放回调声明

```c
// 正确: modbus 层定义 weak 默认 (无操作)
__attribute__((weak)) unsigned char WaveCapture_OnAckWrite(void) { return 1; }

// 正确: wave_capture.c 提供强实现覆盖 (链接器自动选择)
unsigned char WaveCapture_OnAckWrite(void) { ... }

// 错误: 在 .h 中声明回调, 让调用方 include
uint8_t WaveCapture_OnAckWrite(void);  // ← 不要这样做
```

---

## 三、项目规格

| 文件 | 说明 |
|------|------|
| @.claude/specs/tech-stack.md | 技术栈 |
| @.claude/specs/ubiquitous-language.md | DDD 通用语言 (所有术语唯一定义) |
| @.claude/specs/modification-rules.md | 修改约束与边界 |
| @.claude/agents/tech-lead.md | 技术负责人 |
| @.claude/agents/developer.md | 程序员 |

---

## 四、每次编码后检查

```bash
# 编译验证 → 0 error, 0 warning
armcc -c --cpu Cortex-M4 --c99 -I... <modified.c>

# 如果改了 MODBUS 寄存器或 EKF/PID:
python tools/ekf_tuner/m4_modbus_tool.py COM3 --read
python tools/ekf_tuner/run_ekf_tests.py
```

### 自检清单

- [ ] 修改在安全区域内？(不在 No-Go 列表)
- [ ] 没有顺手重构无关代码？
- [ ] 编译 0e0w？
- [ ] 如果改了寄存器 → CLAUDE.md 寄存器表已同步更新？
- [ ] 如果改了接口 → PC 工具已同步更新？

---

## 五、硬件平台

| 参数 | 值 |
|------|-----|
| MCU | RX32G410 (Cortex-M4F) |
| 主频 | 384 MHz |
| HRTIM | 半桥 PWM 生成 |
| ADC | 电压/电流/相位/谐振 多通道 |
| UART2 | MODBUS RTU, DMA 收发 |
| 控制周期 | 20ms (Task_TimeChip1) |

## 六、MODBUS 物理层

| 参数 | 值 |
|------|-----|
| 波特率 | **115200** |
| 数据位/校验/停止位 | 8 / N / 1 |
| CRC | MODBUS CRC-16, 低字节优先 |
| 从站地址 | 5 (Slave1), 10 (Slave2), 15 (Slave3), 20 (Slave4) |
| RX/TX | DMA |

## 七、MODBUS 寄存器表

### 只读遥测: 0x1000-0x1014 (21寄存器)

| 地址 | 字段 | 说明 |
|------|------|------|
| 0x1000 | Vol_AD | 电压 ADC (8-bit有效) |
| 0x1001 | Current_AD | 电流 ADC (8-bit有效) |
| 0x1002 | IGBT_AD | IGBT 温度 ADC (8-bit有效) |
| 0x1003 | Bot_AD | 炉面温度 ADC (8-bit有效) |
| 0x1004 | Fan_AD | 风扇 ADC |
| 0x1005 | Top_AD | 顶部温度 ADC (8-bit有效) |
| 0x1006 | Practical_Power | 实际功率 (W) = val × 25 |
| 0x1007 | Practical_PPG | 实际 PPG 值 |
| 0x1008 | P_limited_STA | 功率限制状态 |
| 0x1009 | Pan_pulsating | 检锅脉冲计数 |
| 0x100A | Resonate_Curr | 谐振电流 ADC |
| 0x100B | HVol_Cnt | 反压计数器 (8-bit) |
| 0x100C | HZ_Cnt | 频率计数器 (8-bit) |
| 0x100D | Discard_Cnt | 丢波计数器 |
| 0x100E | Phase_Position | **相位值 (⚠️ 固件未填充, =0)** |
| 0x100F | ERROR | 故障码 (低4位) |
| 0x1010 | interior_ERR | 内部故障 (浪涌标志) |
| 0x1011 | Version_Number | 主板版本号 |
| 0x1012 | Equivalent_Resistance | 等效电阻 (8-bit有效) |
| 0x1013 | _25W_Power | 25W 修正值 (8-bit有效) |
| 0x1014 | SYS_Sta | 系统状态 |

### 可读写控制: 0x2000-0x2014 (21寄存器)

| 地址 | 字段 | 说明 |
|------|------|------|
| 0x2000 | Check_Pan_LV | 检锅强度 |
| 0x2001 | PPG_Max | 最大 PPG 限制 |
| 0x2002 | Pan_Power | 移锅功率 |
| 0x200E | Work_STA | 工作状态 (0x10=启动, 0=停止) |
| 0x200F | FAN_Speed | 风扇转速 |
| 0x2010 | target_Power | **目标功率 (单位: 25W)** |
| 0x2011 | intermittent_Heat | 间断加热 |
| 0x2012 | jitter_frequency | 抖频参数 |
| 0x2013 | BuzzCof | 蜂鸣器 |
| 0x2014 | syntony_Current_Short | 短路保护 |

### 系统设置: 0x3000-0x3003 (4寄存器)

| 地址 | 字段 | 说明 |
|------|------|------|
| 0x3000 | Power_Calibration | 功率校准 (36-96) |
| 0x3001 | Slave_Addr | 从机地址 |
| 0x3002 | Baud_rate_SET | 波特率设置 |
| 0x3003 | Save_order | 保存命令 (写1保存) |

---

## 八、数据流 (20ms 控制周期)

```
Task_TimeChip1 → PowerTypeFun → PowerControlFun:
  1. PPGgetAdcValue()      读取 V/I/Phase/Resonate ADC
  2. s_ppg_fun()           实际功率 = V_ADC × I_ADC / g_p25_ad
                           调用 FixedPID_Compute(target, actual) → delta_ppg
  3. i_ppg_control()       PPG 步长应用到 HRTIM PER/CMP
                           f = 384MHz / prioed

MODBUS 同步:
  Update_Static_Register_DATA() → 从 COMM_RUN 填充 0x1000-0x1014
```

---

## 九、PC 工具

```bash
pip install pymodbus pyserial

# 交互模式
python m4_modbus_tool.py COM3

# 单次读取
python m4_modbus_tool.py COM3 --read

# 记录30秒
python m4_modbus_tool.py COM3 --log 30

# 设定1000W并启动
python m4_modbus_tool.py COM3 --power 1000 --on
```

## 十、已知 M4 固件待修复项

1. **Phase_Position (0x100E) = 0**: 固件未填充，需从 phaseValue 或 phase_angle 读取
2. **频率未上报**: HZ_Cnt (0x100C) 是计数器非实际频率，建议新增 0x1015-0x1016
3. **电压/电流为8-bit ADC**: 需校准系数才能换算物理单位

---

## 十一、编码约定

- 所有 if/else/while/for 必须加 {}，单行也不省略
- snake_case 变量, PascalCase 函数, 4空格缩进
- 中文注释说明复杂逻辑
- EKF 矩阵运算显式展开
- 浮点分母加 1e-9f 防除零
- 协方差对角线非负保护
- M4 FPU 硬件指令可用

---

*最后更新: 2026-05-27*
