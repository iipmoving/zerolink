# 通用语言 — M4 半桥 EKF 观测器

> 本项目专有术语。所有沟通必须对齐以下定义。

---

## 一、功率控制域

| 术语 | 英文 | 说明 |
|------|------|------|
| PPG | Pulse-Per-Gear | 功率控制单位，PPG 值对应 HRTIM 频率步长 |
| 实际功率 | Practical Power | 由 V_ADC × I_ADC / 校准值计算，单位 25W |
| 目标功率 | Target Power | PC 通过 MODBUS 0x2010 下发，单位 25W |
| 功率校准值 | Power Calibration | 0x3000，范围 36-96 |
| 丢波 | Discard | 功率控制中跳过的半周期，Discard_Cnt 记录 |
| 间断加热 | Intermittent Heat | 0x2011 控制，低功率模式下周期性开关 |

## 二、观测器域 (EKF)

| 术语 | 英文 | 说明 |
|------|------|------|
| EKF | Extended Kalman Filter | 扩展卡尔曼滤波，估计负载等效 R/L |
| 等效电阻 | Equivalent Resistance | 负载 R (寄存器 0x1012, 8-bit) |
| 谐振电流 | Resonate Current | 谐振回路电流 ADC (0x100A) |
| 谐振频率 | Resonant Frequency | 由相位和当前频率推算 |
| 相位角 | Phase Angle | φ，电压电流相位差，0x100E (固件未填充) |
| 反压 | High Voltage | 开关管关断时谐振电容反压 (0x100B) |
| 状态向量 | State Vector | EKF 状态估计: [R, L, f_res, ...] |
| 观测矩阵 | Observation Matrix | H，将状态映射到测量 |
| 过程噪声 | Process Noise | Q 矩阵，模型不确定性 |
| 测量噪声 | Measurement Noise | R 矩阵，传感器噪声 |

## 三、控制参数域

| 术语 | 英文 | 说明 |
|------|------|------|
| 变增益 | Variable Gain | PID 参数随负载 (R) 自适应调整 |
| PID | Proportional-Integral-Derivative | 功率控制回路 |
| 控制周期 | Control Cycle | 20ms (Task_TimeChip1 中断) |
| PPG 步长 | PPG Step | 每次 PID 输出的 PPG 增量/减量 |
| 功率限制 | Power Limit | 0x1008，软限制状态 |

## 四、锅具检测域

| 术语 | 英文 | 说明 |
|------|------|------|
| 检锅 | Pan Detection | 脉冲法检测有无锅具 |
| 检锅脉冲 | Pan Pulse | 短时间加热脉冲 (0x1009 低4位计数) |
| 检锅强度 | Pan Check Level | 0x2000，检锅时 PPG 大小 |
| 移锅 | Pan Removed | 运行中检测到锅具移除 |
| 移锅功率 | Pan Remove Power | 0x2002，移锅后降到的功率 |

## 五、系统保护域

| 术语 | 英文 | 说明 |
|------|------|------|
| IGBT | Insulated Gate Bipolar Transistor | 功率开关管 |
| 浪涌 | Surge | 电压/电流突变 (0x1010 interior_ERR) |
| 故障码 | Error Code | 0x100F 低4位 |
| 短路保护 | Short Circuit | 0x2014 谐振电流短路 |
| 过热保护 | Over Temp | IGBT/炉面温度超限 |

## 六、MODBUS 通信域

| 术语 | 英文 | 说明 |
|------|------|------|
| 主站 | Master | PC (Python MODBUS 客户端) |
| 从站 | Slave | M4 芯片 (站号 5/10/15/20) |
| 寄存器 | Register | 16-bit 数据单元 |
| 功能码 03 | Read Holding Registers | 读取多个寄存器 |
| 功能码 06 | Write Single Register | 写单个寄存器 |
| 功能码 10 | Write Multiple Registers | 写多个寄存器 |
| 广播 | Broadcast | 从站地址 0，所有设备响应 |

## 七、硬件特有术语

| 术语 | 英文 | 说明 |
|------|------|------|
| 半桥 | Half-Bridge | 两个 IGBT 的逆变拓扑 |
| HRTIM | High Resolution Timer | 高分辨率定时器，384MHz 时钟 |
| PER | Period Register | HRTIM 周期寄存器，控制开关频率 |
| CMP | Compare Register | HRTIM 比较寄存器，控制占空比 |
| DT | Dead Time | 死区时间，防止上下管直通 |
| 抖频 | Jitter Frequency | 0x2012，EMC 抖频参数 |
| 风扇 | Fan | 0x200F，散热风扇转速 |

---

*维护规则：任何人在讨论中引入新术语时，须在此文件注册。*
*最后更新: 2026-05-27*
