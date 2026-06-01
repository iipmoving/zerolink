# RESTART.md — M4 半桥 IH 驱动 · AI 重启入口

> **用途**: 新 AI 会话或接管工程师的第一个文件。读完本文件 ≈ 继承全部项目记忆。
> **最后更新**: 2026-05-30
> **工作目录**: `D:\OBSIDIAN\MOVING IH\低耦合程序架构`

---

## 零、30 秒速览

```
MCU:        RX32G410 (Cortex-M4F, 384MHz, 硬件FPU/CORDIC/FMAC)
KEIL项目:   src/RX32G410_FW_HAL_V1.3N/Projects/Keil/ReTek.uvprojx
控制方式:   PC (Python) → MODBUS RTU (115200 8N1) → M4 从站
从站地址:   5/10/15/20 (四头各一)
控制周期:   20ms (Task_TimeChip1)
开发模式:   增量开发 — 已硬件验证的固件上安全修改
修改边界:   No-Go (原厂库/HRTIM/ADC/MODBUS帧/IGBT保护) vs 安全区 (app/ekf/ tools/)
最终目标:   BaseClass/src/power_calculator.c 按MATLAB物理模型重写
```

---

## 一、项目身份

| 维度 | 说明 |
|------|------|
| **与 four_head 的区别** | 本项目是**实际产品**，在已验证固件上增量开发，不构建零依赖架构 |
| **定位** | 四头半桥 IH 电磁炉驱动 + EKF 负载观测器 + 锅具检测 + 变增益 PID |
| **当前阶段** | EKF 离线验证完成 → 波形采集 MODBUS 化 → C 端移植 |
| **方法论态度** | 消费方法论（用到再取），不强制执行 four_head 六大铁律 |

---

## 二、代码架构

### 2.1 分层结构

```
User/main.c + rx32g4xx_it.c            ← 入口 + 中断
    ↓
APP/sys/                                ← sys_task, s_time_base, S_PID, s_sensor
    ↓                                      系统基础设施 (时基/任务调度/PID队列)
APP/POWER/                              ← app_power, app_work, app_cook, app_task
    ↓                                      业务逻辑 (功率控制/工作状态机/烹饪)
LIB/INTERFACE/ Proto_I2C               ← I2C 从机通讯协议
    ↓
LIB/API/                                ← API_hrtim, API_adc, API_gpio, API_TIM...
    ↓                                      硬件功能封装 (API只调HAL, API不互调 ✅)
LIB/BaseClass/                          ← class_comm, class_phase, class_pluse,
    ↓                                      power_calculator, printMessage, resonant_f0
DRV/                                    ← DRV_GPIO, adc_processing, drv_tim
    ↓
HAL (RX32G410 FW HAL V1.3N)            ← 原厂固件库 (只读不写)
```

### 2.2 与 four_head 方法论的对比

| 维度 | four_head (方法论验证) | m4_ekf_observer (实际产品) |
|------|----------------------|---------------------------|
| 开发模式 | 从零构建零耦合架构 | 已验证固件上增量开发 |
| 架构约束 | L0-L3 全套阻断系统 | 只保护基线稳定 |
| 通信机制 | __weak 回调直调 | MODBUS RTU 主从 |
| 头文件 | //#define 私有化 | 已有代码不适用 |
| 角色 | tech-lead + developer + tester | tech-lead + developer (缺tester) |
| 单功能模块解耦 | ✅ 已完成 | ✅ API 只调 HAL, API 不互调 |
| APP 层耦合 | ❌ 不允许 | ❌ 存在, 待逐步解耦 |

---

## 三、硬件参数 (MATLAB 模型标定值)

### 3.1 采样链路

| 链路 | 参数 | 值 |
|------|------|-----|
| 电压采样 | 3×270K + 6.2K 分压 → ADC | 0.105 V/ADC (分压比 1/131.6) |
| 电流采样 | CT 2000:1 → 2K → 全波整流 → 10K+330 分压 → ADC | 25.2 mA/ADC |
| 校准点 | I_limit_reg = 0x60 → 2300W | 限功率标定点 |
| HRTIM | 有效时钟 | 768 MHz |
| ADC采样间隔 | FRE_PER_ADC = 384 ticks | 0.5 μs/采样 |
| PWM周期 | PERIOD_TICKS = 26112 | f_sw ≈ 29.4 kHz |

### 3.2 谐振参数

| 参数 | 值 | 说明 |
|------|-----|------|
| C_known | 0.90 μF | 0.45μF×2 并联 |
| L_iron @30kHz | 65 μH | 铁锅 |
| L_iron @20kHz | 70 μH | 铁锅低频 |
| L_steel @30kHz | 57 μH | 钢锅 |
| f0_iron | 20-21 kHz | 实测 |
| f0_steel | 23-24 kHz | 实测 |

### 3.3 三相验证数据集

| 数据集 | Vbus ADC | Vbus | 相位 | 用途 |
|--------|---------|------|------|------|
| D1 (高电压) | ~2860 | ~300V | ≈90° | 大功率基准 |
| D2 (中电压) | ~2240 | ~235V | ≈87° | 常规工况 |
| D3 (低电压) | ~920 | ~97V | ≈94° | 低功率验证 |

---

## 四、项目当前状态

### 4.1 已完成

| 里程碑 | 完成日期 | 文档 |
|--------|---------|------|
| EKF 物理模型验证 | 2026-05-30 | `docs/EKF-PHASE-CLOSURE-2026-05-30.md` |
| f0 估计三方法实现 | 2026-05-30 | `tools/ekf_tuner/ekf_q_model.py` + `memory/project_m4-f0-estimation-methods.md` |
| 锅具判别逻辑 (9/9 PASS) | 2026-05-30 | `ekf_q_model.py --self-test` |
| MATLAB 三参数交叉验证 | 2026-05-30 | `tools/ekf_tuner/verify_f0_q_l.m` + 3张验证图 |
| MODBUS EKF 遥测寄存器 0x1020 | 2026-05-27 | `app/ekf/modbus_ekf_regs.c/h` |
| MODBUS 谐振结果 0x4000 区 | 2026-05-30 | `modbus/Modbus_Lib_Init_An_Analysis.c` Area4 |
| 数据采集 14 CSV ~9000行 | 2026-05-29 | `tools/ekf_tuner/m4_ekf_*.csv` |
| PC工具链 (MODBUS/GUI/测试/绘图) | 2026-05-29 | `tools/ekf_tuner/` 全套 |

### 4.2 待推进

| 优先级 | 任务 | 依赖 | 规划文档 |
|--------|------|------|---------|
| **P0** | WaveCapture 模块 — printMessage→MODBUS波形回读 | — | `docs/WAVECAPTURE-PLAN.md` |
| **P1** | EKF C 端移植 (ekf_resonant_track.c) | 需完整源文件 | `docs/ekf-implementation-plan.md` |
| **P1** | EKF 补丁 5 处集成 (FIRMWARE_PATCH.md) | 需完整源文件 | `app/ekf/FIRMWARE_PATCH.md` |
| **P2** | HRTIM MASTER 同步简化 (hrtim-master-sync) | EKF 完成后 | `memory/project_m4-hrtim-master-sync-simplify.md` |
| **P3** | 半桥→全桥迁移 (migrate-full-bridge) | HRTIM完成后 | `memory/project_m4-half-to-full-bridge-migration.md` |
| **P4** | APP 层逐步解耦 (__weak回调化) | 不紧急 | `memory/m4-architecture-analysis.md` |

### 4.3 已知固件问题

| 问题 | MODBUS地址 | 说明 |
|------|-----------|------|
| Phase_Position = 0 | 0x100E | 固件未填充 phaseValue |
| HZ_Cnt 是计数器非频率 | 0x100C | 新增 0x1021-22 频率寄存器 (EKF补丁) |
| 电压/电流 8-bit ADC | 0x1001-0x1002 | 需校准系数换算物理单位 |

---

## 五、WaveCapture 模块 (当前进行中, P0)

### 5.1 整体方案

将 `printMessage` 的 UART printf 输出 → 改为 MCU 6K 环形缓存 → MODBUS FC03 主机回读 → MATLAB 验证。

**最终目标**: `BaseClass/src/power_calculator.c` 按 MATLAB 物理模型重写为全新电参量运算模块。

### 5.2 数据帧结构

```
帧头 (6 words): MAGIC/A5A5, frame_id, head_idx, cycle_cnt, f_sw, reserved
PARA块 (5 words): PPG_VALUE, HIGH_ON, POWER, PERIOD, reserved
周期体 (360 × 3 words): HRTIM_time, Vbus_ADC, I_FMAC_ADC
帧总大小: 1091 words < 6K (6144) ✅
```

### 5.3 MODBUS 0x5000 区

| 地址 | 名称 | R/W | 说明 |
|------|------|-----|------|
| 0x5000 | WAVE_CTRL | R/W | 写1=开始, 写0=停止 |
| 0x5001 | WAVE_HEAD | R/W | 目标炉头 (0-3) |
| 0x5002 | WAVE_FRAME_ID | R | 当前帧序号 |
| 0x5003 | WAVE_CYCLE_CNT | R | 本帧周期数 |
| 0x5004 | WAVE_MAGIC | R | 0xA5A5 (帧同步) |
| 0x5005-09 | 帧头续+PARA | R | 帧描述+运行参数 |
| **0x500A** | **WAVE_DATA** | **R** | **波形数据入口 (1080 words)** |

### 5.4 任务清单 (5 Step)

| 任务 | 文件 | 说明 |
|------|------|------|
| T001 | `BaseClass/inc/wave_capture.h` **(新建)** | 结构体+API声明+6K extern buffer |
| T002 | `BaseClass/src/wave_capture.c` **(新建)** | Push/Freeze/Config实现 |
| T003 | `modbus/src/Modbus_Lib_Init_An_Analysis.c` | AREA_COUNT 5→6, 新增Area5 |
| T004 | `LIB/APP/APP_ADC.C` | 替换PrintMessage→WaveCapture |
| T005 | `tools/ekf_tuner/m4_modbus_tool.py` | 新增 --wave 命令, CSV保存 |

---

## 六、EKF 炉具识别方法 (参考)

### 6.1 三种 f0 估计方法

| 方法 | 文件:行 | 核心算法 |
|------|---------|---------|
| 相位模型法 (PRIMARY) | `ekf_q_model.py:385-456` | 网格搜索 f0, 最小化 Q 的 CV |
| dI/df 斜率法 (REFERENCE) | `ekf_q_model.py:185-232` | 1/\|dI/df\| 线性回归 x截距 |
| 相位修正联合法 | `ekf_q_model.py:235-330` | α·实测 + (1-α)·相位推算 |
| tan(φ) 比值消 Q 法 | `ekf_q_model.py:333-382` | 两频点相除消 Q |

### 6.2 判别逻辑

```
f0 < 19kHz  → IRON (铁锅/不锈钢430)
f0 ≥ 20kHz  → STEEL (钢锅/不锈钢304)
19-20kHz    → 灰色区: f_min_high_pwr < 25kHz → iron, else steel
```

---

## 七、PC 工具链

```bash
cd m4_ekf_observer/tools/ekf_tuner

# 交互模式
python m4_modbus_tool.py COM3

# 单次读取遥测
python m4_modbus_tool.py COM3 --read

# 持续记录
python m4_modbus_tool.py COM3 --log 30

# 设置1000W并启动
python m4_modbus_tool.py COM3 --power 1000 --on

# GUI 调试界面
python m4_gui.py --port COM3

# 自动化测试序列
python run_ekf_tests.py COM3 --test power_sweep

# 离线 f0 分析
python ekf_q_model.py --csv data.csv

# 离线 f0 自检
python ekf_q_model.py --self-test

# 波形采集 (WaveCapture, 待实现)
python m4_modbus_tool.py COM3 --wave --head 0
```

---

## 八、编程规则 (继承自 CLAUDE.md)

0. **{} 不省略** — 所有 if/else/while/for 必须加 {}，单行也不省略
1. **碰前必读** — 改任何 .c/.h 前完整读完该文件, 不读前50行就改
2. **最小化 diff** — 只改任务要求的, 不顺手重构/格式化/重命名
3. **接口兼容** — 已有函数不改签名, 改必须说明影响范围
4. **每次编码后编译** — armcc --cpu Cortex-M4 --c99 → 0 error 0 warning
5. **假设显性化** — 不确定的假设单列一节, 等确认
6. **多解释呈现** — 多种理解时全列出, 有更简单方法主动提出
7. **困惑即停** — 需求不清晰时指出来, 不自己猜
8. **最小化代码** — 不添加未被要求的功能, 不创建单次使用的抽象
9. **简洁性自检** — 代码行数不超过必要行数的1.5倍
10. **自清门户** — 改动导致的死变量/dead include 自行清理
11. **每一行可追溯** — 每个修改的代码行必须直接追溯到用户需求
12. **验证闭环** — 模糊需求转化为可验证的目标后再动手

---

## 九、记忆索引

### 项目记忆 (m4_ekf_observer/memory/)

| 文件 | 内容 |
|------|------|
| `MEMORY.md` | 记忆索引 (本文件入口) |
| `project_m4-overview.md` | M4 项目概述 |
| `m4-architecture-analysis.md` | 源码架构分析, APP耦合评估 |
| `m4-ekf-verification-methodology.md` | ★ EKF 验证方法论完整总结 |
| `m4-matlab-halfbridge-model.md` | ★ 半桥IH MATLAB模型参数 |
| `project_m4-f0-estimation-methods.md` | f0 三种估计方法 + 锅具判别 |
| `project_m4-ekf-patch-status.md` | EKF 遥测补丁状态 |
| `project_m4-modification-boundaries.md` | No-Go / 安全 / 需审批区域 |
| `project_m4-hrtim-master-sync-simplify.md` | HRTIM 同步简化计划 |
| `project_m4-half-to-full-bridge-migration.md` | 半桥→全桥迁移计划 |
| `project_m4-known-issues.md` | 已知固件问题清单 |
| `reference_m4-pc-tools.md` | PC 工具链索引 |

### 规划文档 (docs/)

| 文件 | 内容 |
|------|------|
| `EKF-PHASE-CLOSURE-2026-05-30.md` | ★ EKF 阶段结项报告 |
| **`WAVECAPTURE-PLAN.md`** | **★ 当前任务: WaveCapture 实施计划** |
| `ekf-implementation-plan.md` | EKF 卡尔曼滤波落地计划 |
| `modbus-resonant-output-plan.md` | 谐振参数 MODBUS 0x4000 输出 |
| `resonant-msg-config.md` | resonant_msg 模块配置 |
| `modbus_protocol.md` | MODBUS 协议规格书 |
| `test_conditions.md` | 测试工况清单 |

### 集群记忆 (methodology-seed/)

| 文件 | 内容 |
|------|------|
| `01-architecture.md` ~ `07-struct-generation.md` | ★ 全套方法论文档 |
| `ONBOARDING.md` | 新 AI 启动指南 |
| `templates/` | 项目模板 (CLAUDE.md, interface_map.h, structs.json...) |

---

*本文件是 m4_ekf_observer 项目的唯一重启入口。新 AI 会话从此文件开始。*
*当前进行中: WaveCapture 模块 — T001→T005 逐任务推进*
