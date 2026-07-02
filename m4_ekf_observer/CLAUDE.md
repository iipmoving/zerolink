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
- `base_class/src/` — 基础类层（新增模块）
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

## 四、每次编码后强制：五件套

**以下 5 步必须全部 PASS 才能声明"完成"。任一步失败 = 不得提交。**

```bash
# 工作目录: m4_ekf_observer/
# 工具位于 ../methodology-seed-v2.0/tools/，自动检测项目类型

# 1. 层依赖审计 → 必须 0 violations
python ../methodology-seed-v2.0/tools/check_deps.py . --project m4-ekf

# 2. __weak 配对一致性 → 必须 0 violations
python ../methodology-seed-v2.0/tools/check_weak_pairs.py . --project m4-ekf

# 3. 结构体一致性 → 必须 PASS
python ../methodology-seed-v2.0/tools/check_structs.py . --project m4-ekf

# 4. 输出回调审计 (v2.2 PULL 范式) → 必须 0 violations
python tools/check_output_callback.py .

# 5. 编译验证 → 必须 0 error, 0 warning
armcc -c --cpu Cortex-M4 --c99 -I... <modified.c>
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

- snake_case 变量, PascalCase 函数, 4空格缩进
- 中文注释说明复杂逻辑
- EKF 矩阵运算显式展开
- 浮点分母加 1e-9f 防除零
- 协方差对角线非负保护
- M4 FPU 硬件指令可用

---

## 十二、方法论文档：分层规则

### 12.1 五层架构

本项目遵循 methodology-seed 零耦合分层架构。层间隔离是绝对约束，不得妥协。

```
┌──────────────────────────────────────────────┐
│  APP  应用层  (app/)                           │
│  允许: include core/ proto/                    │
│  禁止: include base_class/ src/               │
├──────────────────────────────────────────────┤
│  PROTO  协议层  (proto/)                       │
│  纯函数库 — 编解码，无状态，无副作用              │
│  禁止: include app/ base_class/ src/           │
├──────────────────────────────────────────────┤
│  BASE_CLASS  基础类层  (base_class/)            │
│  允许: include src/ (vendor HAL)               │
│  禁止: include app/                            │
├──────────────────────────────────────────────┤
│  CORE  核心基础设施  (core/)                     │
│  消息调度器、时间基、基础设施                     │
│  禁止: include app/ base_class/ proto/         │
├──────────────────────────────────────────────┤
│  VENDOR  原厂固件库  (src/RX32G410_FW_HAL_V1.3N/) │
│  No-Go: 只读不写，禁止修改                       │
└──────────────────────────────────────────────┘
```

**唯一合法的跨层调用链**: APP → __weak 直调 → BASE_CLASS → VENDOR HAL

### 12.2 层依赖白名单

| 层 | 目录 | 可 include | 不可 include |
|-----|------|-----------|-------------|
| APP | `app/` | `core/`, `proto/` | `base_class/`, `src/`, 其他 `app/` |
| BASE_CLASS | `base_class/` | `src/` (HAL) | `app/` |
| PROTO | `proto/` | — | `app/`, `base_class/`, `src/` |
| CORE | `core/` | — | `app/`, `base_class/`, `proto/` |

**验证机制**: `check_deps.py` 扫描所有 `#include` 语句，对照上表。违规 → 非零退出码 → §四四件套阻断提交。

### 12.3 通信选择规则

两种通信机制按调用时机选择，**不得互相替代**。

| 机制 | 适用场景 | 调用时机 | 中间层 | 发送方代码 |
|------|---------|---------|--------|-----------|
| `__weak` 直调 | **同时间片**连续执行 | 同步，调用即执行 | 无（链接器接线） | `Receiver_OnXxx(param, &data)` |
| `Msg_Post` 消息 | **跨时间片** / 异步通知 | 异步，队列缓冲 | msg_scheduler | `Msg_Post(MSG_ID, param, &data)` |

**选择规则 (MUST)**: "这个操作必须在同一次控制周期内完成？" → 是: `__weak` / 否: `Msg_Post`。

### 12.4 __weak 回调机制

```c
/* 发送方定义 __weak 空壳（不被覆盖时静默丢弃）*/
__weak void Receiver_OnEvent(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

/* 发送方调用 — 立即执行，无队列，无注册 */
Receiver_OnEvent(param, &data);

/* 接收方强符号实现 — 链接器自动覆盖 __weak 空壳 */
void Receiver_OnEvent(uint16_t param, void *data_ptr)
{
    /* 业务处理 */
}
```

**配对管理**: 所有 __weak 配对记录在 `interface_map.h`，由 `check_weak_pairs.py` 验证一致性。

**PULL 路由模式 (v2.2)**: 对于使用 `MODULE_SKELETON()` 的模块，推荐 Switcher 显式路由
(producer 写 g_output.para → Switcher 调 consumer 回调)，__weak 直调作为简易通道保留。
`_onOutput`/`ST_OUT` 仅限 `@OUTPUT_CALLBACK` 白名单场景，由 `check_output_callback.py` 审计。

### 12.5 头文件私有化

**APP/PROTO 层的每个 .h 文件，include guard 的 `#define` 必须注释掉。**

```c
#ifndef MODULE_NAME_H
//#define MODULE_NAME_H   // ← 注释掉, 禁用 include guard — L0 编译器阻断跨模块引用
#endif
```

这是 L0 物理阻断 — 同一 .c 内重复包含直接编译报错。

---

## 十三、新增模块 SOP — 7 步可执行清单

新增模块时严格按以下步骤执行，**每步确认后才进入下一步**。

### Step 1: 确定模块层级

问三个问题：
- 这个模块做业务决策（状态机、逻辑路由）？→ `app/`
- 这个模块封装硬件操作（寄存器、外设）？→ `base_class/`
- 这个模块做协议编解码（无状态纯函数）？→ `proto/`
- 这个模块是系统基础设施（调度、时基）？→ `core/`

**MUST**: 确认层级后查 §12.2 依赖白名单，确认允许的 include 范围。

### Step 2: 创建 module.h + module.c

```bash
touch app/<module>/module.h app/<module>/module.c
```

### Step 3: 编写 .h — 注释 include guard + 公共接口

```c
/* module.h — 模块职责一句话描述
 * 层级: app | base_class | proto | core
 * 依赖: 列出本模块依赖的其他模块（仅白名单内）
 */
#ifndef MODULE_NAME_H
//#define MODULE_NAME_H   // ← APP/PROTO 层必须注释

#include <stdint.h>       // 仅系统头文件用 <> angle brackets

/* 公共类型定义 */
typedef struct {
    // ...
} Module_Config_t;

/* 公共接口 */
void Module_Run(void);

#endif /* MODULE_NAME_H */
```

### Step 4: 编写 .c — 三段式骨架

按 §十四 三段式范式编写 `Module_Run()`。详见下节。

### Step 5: 注册跨模块结构体 (如有)

如果模块输出/输入复杂数据（非基本类型），MUST 在 `cfg/structs.json` 中注册。

```bash
# 编辑 cfg/structs.json → 添加 owner/consumer 定义
# 生成 types.h
python ../methodology-seed-v2.0/tools/generate_structs.py . --project m4-ekf
# 验证一致性
python ../methodology-seed-v2.0/tools/check_structs.py . --project m4-ekf
```

### Step 6: 注册 __weak 配对到 interface_map.h

如果有跨模块 __weak 回调，MUST 在 `interface_map.h` 中注册配对：

```c
/* 格式: {线ID, "方向", "发送模块", "__weak声明函数", ..., "接收模块", "接收函数", ...} */
```

### Step 7: 运行五件套验证

```bash
python ../methodology-seed-v2.0/tools/check_deps.py . --project m4-ekf
python ../methodology-seed-v2.0/tools/check_weak_pairs.py . --project m4-ekf
python ../methodology-seed-v2.0/tools/check_structs.py . --project m4-ekf
python tools/check_output_callback.py .
armcc -c --cpu Cortex-M4 --c99 -I... <module.c>
```

**五件套全部 PASS → 才能声明模块完成。任一步失败 → 修复后重跑全部。**

---

## 十四、模块三段式范式

### 14.1 总览

所有模块（无论大小）统一为三段式结构：**输入 → 计算 → 输出**。

```c
void Module_Run(void)
{
    /* ====== 输入段 — 所有外部数据入口集中在此 ====== */
    // __weak 直调: 同时间片同步数据
    // Msg_Post 消费: 跨时间片异步数据

    /* ====== 计算段 — 纯计算，不调输入/输出通道 ====== */
    // 核心算法、状态机、数据变换

    /* ====== 输出段 — 所有结果出口集中在此 ====== */
    // __weak 直调: 同步回调下游
    // Msg_Post 投递: 异步通知其他模块
}
```

### 14.2 构造：懒惰初始化

```c
void Module_Run(void)
{
    static uint8_t initialized = 0;
    if (!initialized) {
        initialized = 1;
        /* 初始化自己的状态变量 */
    }
    /* ... 三段式主体 ... */
}
```

**约束**: `main()` 不得调用 `Module_Init()`。每个模块首次进 `_Run()` 时自检 `static` 标志。

### 14.3 完整模板

```c
/* === module.c === */
#include "module.h"

/* === __weak 输入回调声明 (代替 #include "other_module.h") === */
__weak void Module_OnParam(uint16_t param, void *data_ptr)
{ (void)param; (void)data_ptr; }

/* === 内部状态 === */
typedef struct {
    uint16_t value;
} Module_State_t;
static Module_State_t s_self;

/* === 公共入口 === */
void Module_Run(void)
{
    static uint8_t init_done = 0;
    if (!init_done) { init_done = 1; /* 初始化 */ }

    /* ====== 输入段 ====== */
    /* 同时间片同步输入 — __weak 被覆盖时非空调用 */
    Module_OnParam(0, NULL);

    /* ====== 计算段 ====== */
    s_self.value += 1;

    /* ====== 输出段 ====== */
    /* 同步回调下游 — __weak 直调 */
    Consumer_OnResult(s_self.value, NULL);
}
```

---

## 十五、回调插入规则表

**MUST**: 回调只在模块的输入段或输出段插入。计算段中途插入回调必须先经用户确认。

| 回调类型 | 位置 | 是否需要用户确认 | 说明 |
|---------|------|----------------|------|
| **输入回调** | 输入段（Module_Run 前部） | **否** — 标准做法 | 所有外部数据入口集中顶部 |
| **输出回调** | 输出段（Module_Run 后部） | **否** — 标准做法 | 所有结果出口集中底部 |
| **算法子集调用** (__weak) | 计算段 | **否** — 必须的依赖 | 不 include .h，链接器接线 |
| **即时回调** | 计算段中途 | **是 — MUST 先问用户** | 表明输入规划不完整 |

**即时回调管控规则**:
- 计算段中间发现需要一个外部数据 → **MUST 先停下来问用户**，不是先加上再说
- 如果规划到位，输入段已覆盖所有外部参数，不需中途插入
- 如果确实需要中途回调 → MUST 先确认是否可重构到输入段

---

## 十六、通信选择速查表

| 场景 | 机制 | 示例 |
|------|------|------|
| 同控制周期连续调用（上下级） | `__weak` 直调 | APP → BASE_CLASS 功率下发 |
| 同控制周期平行模块 | `__weak` 直调 | EKF → PID 参数传递 |
| 跨控制周期异步通知 | `Msg_Post` | 按键事件 → 状态变更 |
| 跨控制周期延迟消费 | `Msg_Post` | EKF 结果 → MODBUS 下一帧发送 |
| 纯算法集调用 | `__weak` 空壳覆盖 | 主模块 → 算法集 (不 include .h) |

**原则**: 同步走 `__weak`，异步走 `Msg_Post`。两者不互相替代，组合使用。

---

*最后更新: 2026-06-05 — 方法论绑定*
