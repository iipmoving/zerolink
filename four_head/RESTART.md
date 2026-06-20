# RESTART.md — 四头电磁炉固件 · AI 重启入口

> **用途**: 新 AI 会话或接管工程师的第一个文件。读完本文件 ≈ 继承全部项目记忆。
> **最后更新**: 2026-06-19 (v2.3 指针直穿)

---

## 零、30 秒速览

```
MCU:        SC32L14TR8 (Cortex-M0+, 48MHz, 256K Flash, 16K SRAM)
编译器:      Keil MDK ARMCC V5.06 (C89, --c99)
项目文件:    Project/32L14Tdmoe.uvprojx
架构:       五层零耦合 — HAL → DRV → PROTO → APP
通信机制:    __weak 回调直调 (零中间层, 零队列, 链接器接线)
约束系统:    L0(编译器) + L1(pre-commit) + L2(生成器) + L3(文档)
头文件规则:  DRV/APP/PROTO 的 .h 禁止外部引用 — include guard 故意失效
公共接口表:  src/core/interface_map.h — 所有 __weak 通道的唯一注册表
```

---

## 一、架构全景图

### 1.1 五层结构

```
┌──────────────────────────────────────────────┐
│  APP  业务逻辑层 (app/)                        │
│  - app_hmi, app_cooking, app_power,           │
│    app_protect, app_comm_mgr                  │
│  - 禁止: include drv/ hal/ proto/ 其他app/     │
│  - 对外: __weak 空壳 + 强符号实现               │
├──────────────────────────────────────────────┤
│  PROTO 协议层 (proto/)                         │
│  - proto_modbus                               │
│  - 纯函数库, 无状态, 无副作用                    │
│  - 通过 __weak 返回值函数与 APP 抽象对接         │
├──────────────────────────────────────────────┤
│  DRV  设备驱动层 (drv/)                         │
│  - drv_key, drv_display, drv_buzzer,          │
│    drv_comm, drv_comm_mgr                     │
│  - 只 include hal/                             │
│  - 定义 __weak 回调给 APP                       │
├──────────────────────────────────────────────┤
│  HAL  硬件抽象层 (hal/)                         │
│  - hal_gpio, hal_timer, hal_comm, hal_uart,   │
│    hal_buzzer, hal_display, hal_smg, hal_key  │
│  - 零依赖: 不知道上层存在                        │
│  - 只 include vendor/ 寄存器头文件              │
├──────────────────────────────────────────────┤
│  VENDOR 芯片厂商库 (vendor/)                    │
│  - CMSIS, FWLib, MCU_Drivers                  │
│  - 只读: 不修改 vendor 文件                     │
└──────────────────────────────────────────────┘
```

### 1.2 唯一合法的调用链

```
APP → __weak 直调 → DRV → HAL → VENDOR
APP → __weak 返回值函数 → PROTO (协议抽象)
```

---

## 二、约束系统：违规即阻断

### 2.1 四个阻断层级

| 层级 | 机制 | 效果 | 本项目的实例 |
|------|------|------|-------------|
| **L0** | 编译器报错 | 绝对无法通过编译 | 头文件 `//#define GUARD` 失效 → 重复包含 → 重定义 error |
| **L1** | pre-commit 非零退出 | 能编译但不能提交 | `check_deps.py` 扫描跨层 #include |
| **L2** | 生成器保证一致性 | 手动改动被检出 | (预留) `generate_structs.py` + `check_structs.py` |
| **L3** | 文档约定 | 最后手段 | `interface_map.h` 格式约定, 命名规范 |

**设计原则**: L3 是最后手段，不是默认手段。能升级到 L2/L1/L0 的规则必须升级。

### 2.2 L0 约束详解：头文件私有化

**DRV/APP/PROTO 层的每个 .h 文件，include guard 的 `#define` 必须注释掉：**

```c
#ifndef MODULE_NAME_H
//#define MODULE_NAME_H   // ← 故意注释！include guard 失效
...
#endif /* MODULE_NAME_H */
```

**原理**:
- `typedef`/`struct`/`enum`/`#define` 不能在同一编译单元内重复出现
- 注释掉 `#define` 后，每次 `#include` 都会重新展开全部内容
- 如果同一个 .h 在同一 .c 中被包含两次 → 重定义 → **编译器直接报错 (L0)**
- 函数声明可以重复，所以 main.c 包含每个模块 .h 一次不受影响

**哪些层适用**:

| 层 | 头文件私有化 | 原因 |
|----|------------|------|
| APP (app/*.h) | **是** | 只允许自身 .c + main.c 引用 |
| DRV (drv/*.h) | **是** | 只允许自身 .c + main.c 引用 |
| PROTO (proto/*.h) | **是** | 只允许自身 .c 引用（通过 __weak 协议函数对接） |
| HAL (hal/*.h) | **否** | DRV 层需要包含 HAL 头文件 |

**当前状态**: DRV 5 个 + APP 3 个 + PROTO 1 个 = 9 个头文件已实施。`app_hmi.h` 待拆分后处理（与 `cfg/hmi_data.h` 有循环引用需先拆解）。

### 2.3 L1 约束详解：check_deps.py

```bash
cd four_head
python tools/check_deps.py src/
```

检查规则:
- `app/*.c` 不得包含 `drv/` 或 `hal/` 或 `proto/`
- `drv/*.c` 不得包含 `app/` 或 `proto/`
- `hal/*.c` 不得包含 `app/` `drv/` `proto/`
- `proto/*.c` 不得包含 `app/` `drv/` `hal/`

**不通过 → 不得提交。**

### 2.4 L0 + L1 双重保险

| 场景 | L0 (编译器) | L1 (check_deps) |
|------|-----------|-----------------|
| APP 直接 `#include "drv_xxx.h"` | check_deps 先阻断 (L1) | — |
| 未来新模块的 .h 里 `#include "app_xxx.h"` → 被 app_xxx.c 间接包含 | 编译器报重定义错 (L0) | — |
| 在 app_xxx.c 中 `#include "../drv/drv_xxx.h"` | — | check_deps 阻断 (L1) |

两者互补，覆盖所有攻击角度。

---

## 三、接口管理系统：Switcher PULL 路由 + project.json

### 3.1 核心机制 (v2.3)

**模块间通信通过 Switcher PULL 路由，零拷贝指针直穿。** 数据流由 `json/project.json` 定义，codeGen 生成 `include_io/*_io.h` + `data_switcher.c`。

```
生产者 DoWork() → 写 output LINK.params + seq++ + ST_OUT
Switcher InputCallback (强符号) → 指针直穿: __in->Producer_params = __out->Consumer_params
消费者 DoWork() → ProcessInput() → seq 比对 → 消费 input LINK.params
```

**与 1.0 的区别**: 无 __weak 回调、无 interface_map.h、无消息队列。编译器保证字段一致性。

### 3.2 数据源: project.json

**位置**: `src/json/project.json`
**性质**: 模块/管道/字段的唯一真相源。codeGen 从此文件生成所有 IO 接口。

**修改流程**:
```
1. 编辑 project.json (或 GUI 编辑)
2. python code_gen.py gen --config project.json -o <src路径>
3. 生成文件覆盖 include_io/ + data_switcher.c
4. armcc 编译验证 → 提交
```

### 3.3 当前管道清单 (16 条)

| # | 方向 | 用途 |
|---|------|------|
| 0 | DrvKey → AppHmi | 按键事件 |
| 1 | DrvKey → AppCooking | 按键事件 |
| 2 | DrvKey → AppSegAlign | 按键事件 |
| 3 | AppCommMgr → AppPower | 寄存器数据 |
| 4 | AppCommMgr → AppCooking | 寄存器数据 |
| 5 | AppCommMgr → AppProtect | 寄存器数据 |
| 6 | AppCooking → AppPower | 烹饪状态 |
| 7 | AppHmi → DrvDisplay | 显示数据 |
| 8 | AppHmi → DrvBuzzer | 蜂鸣命令 (edge) |
| 9 | AppPower → AppHmi | 功率状态 |
| 10 | AppPower → DrvCommMgr | 功率命令 |
| 11 | AppProtect → AppPower | 故障状态 |
| 12 | AppCommMgr → ProtoModbus | 协议请求 |
| 13 | ProtoModbus → AppCommMgr | 协议响应 |
| 14 | AppCommMgr → DrvCommMgr | TX发送请求 |
| 15 | DrvCommMgr → AppCommMgr | TX/RX事件 |
| 16 | AppSegAlign → DrvDisplay | 段码控制 |

---

## 四、文件清单

### 4.1 源码文件 (src/) — 你需要编辑的

#### APP 层 — 业务逻辑 (5 模块)

| 文件 | 职责 | 关键函数 |
|------|------|---------|
| `app/app_hmi.c/.h` | HMI 状态机、按键路由、显示逻辑 | App_Hmi_Init, App_Hmi_Run |
| `app/app_cooking.c/.h` | 烹饪控制、定时器、档位管理 | App_Cooking_Init, App_Cooking_Run |
| `app/app_power.c/.h` | 功率计算、功率下发指令生成 | App_Power_Init, App_Power_Run |
| `app/app_protect.c/.h` | 过温/过流保护、故障检测 | App_Protect_Init, App_Protect_Run |
| `app/app_comm_mgr.c/.h` | MODBUS 轮询调度、通讯数据缓存 | App_CommMgr_Init, App_CommMgr_Run |

#### DRV 层 — 设备驱动 (5 模块)

| 文件 | 职责 |
|------|------|
| `drv/drv_key.c/.h` | 触摸按键去抖、键码映射 |
| `drv/drv_display.c/.h` | 显示帧接收、COM/SEG IO 映射 |
| `drv/drv_buzzer.c/.h` | 蜂鸣器 PWM 控制、音效模式 |
| `drv/drv_comm.c/.h` | UART 收发、DMA 管理、帧检测 |
| `drv/drv_comm_mgr.c/.h` | 通讯调度、TX/RX 缓冲管理 |

#### HAL 层 — 硬件抽象 (8 模块)

| 文件 | 职责 |
|------|------|
| `hal/hal_gpio.c/.h` | GPIO 初始化、读写 |
| `hal/hal_timer.c/.h` | TIM0 125us 时基 |
| `hal/hal_comm.c/.h` | UART0 MODBUS 初始化 |
| `hal/hal_uart.c/.h` | UART3 调试串口 |
| `hal/hal_buzzer.c/.h` | TIM1 蜂鸣器 PWM |
| `hal/hal_display.c/.h` | 数码管 COM 扫描 |
| `hal/hal_smg.c/.h` | 段码转换库封装 |
| `hal/hal_key.c/.h` | 触摸芯片初始化 |

#### PROTO 层 — 协议 (1 模块)

| 文件 | 职责 |
|------|------|
| `proto/proto_modbus.c/.h` | MODBUS RTU 编解码、CRC 校验 |

#### 基础设施

| 文件 | 职责 |
|------|------|
| `main.c` | 系统入口、10 槽调度、各模块初始化 |
| `cfg/hmi_data.c/.h` | HMI 配置数据 (由 gen_hmi.js 从 JSON 生成) |
| `cfg/gen_hmi.js` | JSON→C 代码生成器 |
| `cfg/灯板逻辑_head_template.json` | HMI 规则 JSON 数据源 |
| `core/interface_map.h` | **公共声明链接表** — __weak 通道注册表 (文档) |
| `core/msg_def.h` | 消息 ID 定义 (已废弃, 保留参考) |
| `core/msg_scheduler.c/.h` | 消息调度器 (已废弃, v2.0 用 __weak 替代) |
| `core/std_module.h` | v2.3 统一模块骨架 (MODULE_SKELETON / MODULE_EXPORT / LINK 宏) |
| `core/data_switcher.c/.h` | v2.3 PULL 路由调度器 (INPUT_CALLBACK / INPUT_GET_SLOT) |
| `include_io/*_io.h` | v2.3 模块 IO 接口 (由 codeGen 生成, 指针模式) |
| `json/project.json` | codeGen 数据源 — 模块/管道/字段的唯一真相 |

### 4.2 外部工具链 (codeGen/)

> codeGen 是跨项目共享的代码生成工具，独立于 four_head 仓库。

| 路径 | 用途 | 命令 |
|------|------|------|
| `ZEROLINK/codeGen/scan_project.py` | 扫描 include_io/ → project.json | `python scan_project.py <src路径> --name four_head` |
| `ZEROLINK/codeGen/code_gen.py` | project.json → io.h + switcher + module | `python code_gen.py gen --config project.json -o <src路径>` |
| `ZEROLINK/codeGen/gui_editor.py` | GUI 编辑 project.json + 预览 + 生成 | `python gui_editor.py` |
| `ZEROLINK/codeGen/gen_io_h.py` | io.h 生成器 (被 code_gen.py 调用) | — |
| `ZEROLINK/codeGen/gen_switcher.py` | data_switcher.c 生成器 | — |
| `ZEROLINK/codeGen/gen_module_c.py` | module .c/.h 生成器 | — |
| `ZEROLINK/codeGen/keil_parser.py` | Keil uvprojx 解析器 | — |

**典型工作流**:
```
1. 修改 src/json/project.json (或 GUI 编辑)
2. python code_gen.py gen --config project.json -o <src路径> [--only-io | --only-switcher | --only-modules]
3. 生成文件覆盖 src/include_io/ 和 src/core/
```

### 4.3 项目本地工具 (tools/)

| 文件 | 用途 | 何时运行 |
|------|------|---------|
| `tools/check_deps.py` | 层依赖审计 | 每次编码后 |
| `tools/check_msgs.py` | 消息通道一致性 | 每次编码后 |
| `tools/v4json_to_c.py` | JSON→C 数据转换 | JSON 规则变更后 |

### 4.4 项目文件 (Project/)

| 文件 | 用途 |
|------|------|
| `Project/32L14Tdmoe.uvprojx` | KEIL 项目主文件 |
| `Project/32L14Tdmoe.uvoptx` | KEIL 调试选项 |
| `Project/startup_sc32L14xx.s` | MCU 启动汇编 |

### 4.4 关键文档 (docs/)

| 文件 | 何时阅读 |
|------|---------|
| `docs/architecture/嵌入式灯板控制系统架构说明.md` | 理解架构设计原意 |
| `docs/specs/4头电磁炉Modbus通信协议规格书.md` | MODBUS 寄存器定义 |
| `docs/specs/四头电磁炉完整规格书 V1.0.md` | 产品功能规格 |
| `docs/architecture/json-architecture-phase-report.md` | JSON 驱动架构实施状态 |

### 4.5 AI 配置 (.claude/)

| 文件 | 用途 |
|------|------|
| `.claude/specs/tech-stack.md` | 技术栈详情 |
| `.claude/specs/code-style.md` | 代码规范 (含头文件保护规则) |
| `.claude/specs/ubiquitous-language.md` | DDD 通用语言 (术语标准) |
| `.claude/specs/api-conventions.md` | 接口约定 (回调 vs 消息选择规则) |
| `.claude/agents/tech-lead.md` | 技术负责人角色 |
| `.claude/agents/developer.md` | 程序员角色 |
| `.claude/agents/tester.md` | 测试员角色 |

---

## 五、编译与验证

### 5.1 单文件编译 (armcc)

```bash
cd four_head/Project
"C:/Keil_v5/ARM/ARMCC/bin/armcc.exe" -c --cpu Cortex-M0+ -DSC32L14xx --c99 \
  -I "../src/vendor/FWLib/SC32F1XXX_Lib/inc" \
  -I "../src/vendor/CMSIS" \
  -I "../src/vendor/MCU_Drivers" \
  -I "../src/app" -I "../src" -I "../src/hal" \
  -I "../src/drv" -I "../src/proto" -I "../src/cfg" \
  "../src/app/app_xxx.c" -o "app_xxx.o"
```

**要求**: 0 error, 0 warning

### 5.2 每次编码后的强制检查

```bash
# 1. 层依赖审计 (L1)
cd four_head
python tools/check_deps.py src/
# → 必须 0 violations

# 2. 消息通道一致性 (L1)
python tools/check_msgs.py src/
# → 必须 0 violations

# 3. 独立编译验证 (L0 + 完整性)
cd Project
armcc -c ... ../src/app/app_xxx.c -o app_xxx.o
# → 必须 0 error, 0 warning

# 4. 全量编译 (IDE 或命令行)
# 在 KEIL uVision 中: Project → Build Target
# → 必须 0 error, 0 warning
```

### 5.3 KEIL 项目编译

打开 `Project/32L14Tdmoe.uvprojx` → F7 编译。或者命令行:
```bash
cd four_head/Project
"C:/Keil_v5/UV4/UV4.exe" -r 32L14Tdmoe.uvprojx -o build.log
```

---

## 六、标准工作流

### 6.1 新增 APP 模块

```
1. 确定模块职责和所属层 (APP)
2. 创建 app/app_xxx.h:
   - #ifndef APP_XXX_H / //#define APP_XXX_H  ← 注意: define 注释掉
   - 模块类型定义
   - 公共函数声明 (Init/Run)
3. 创建 app/app_xxx.c:
   - #include "app_xxx.h"
   - 声明依赖的 __weak 空壳 (其他模块会发给我的事件)
   - 定义本模块的 __weak 空壳 (我要发给其他模块的事件)
   - 实现强符号函数
4. 在 main.c 中:
   - #include "app/app_xxx.h"
   - 在 engine_init() 中调用 App_Xxx_Init()
   - 在对应调度槽位调用 App_Xxx_Run()
5. 在 interface_map.h 注册所有新 __weak 通道
6. check_deps.py → armcc → 提交
```

### 6.2 新增跨模块通道

```
1. 确定: 谁发 → 谁收 → 传什么数据
2. 发送方 .c:
   - 添加 __weak void Receiver_OnXxx(uint16_t p, void *d) { (void)p; (void)d; }
   - 在适当位置调用 Receiver_OnXxx(param, &data)
3. 接收方 .c:
   - 添加 void Receiver_OnXxx(uint16_t p, void *d) { /* 实际逻辑 */ }
4. 如需新结构体: 两端各独立声明 (不同名, 同 sizeof, 同字段布局)
5. 在 interface_map.h 注册通道
6. check_deps.py → armcc → 提交
```

### 6.3 修改现有通道

```
1. 改发送方的 __weak 空壳签名
2. 改接收方的强符号签名 (必须完全一致)
3. 如果改了 data_ptr 指向的结构体 → 在 interface_map.h 的结构体配对表中同步
4. 在 interface_map.h 通道表中更新 Data Type 列
5. armcc 编译验证
```

---

## 七、常见禁区 (遇到即停止)

### 7.1 绝对不允许的操作

| 禁区 | 后果 | 阻断层 |
|------|------|--------|
| `app/*.c` 中 `#include "drv/xxx.h"` | check_deps 阻断 | L1 |
| `app/*.c` 中 `#include "hal/xxx.h"` | check_deps 阻断 | L1 |
| `app/*.c` 中 `#include "../app/other.h"` | 重定义报错 (define 已注释) | L0 |
| 新建模块 .h 用 `#define GUARD_H` (未注释) | 失去 L0 保护 | — |
| 在 HAL 层头文件中注释 `#define` | DRV 无法包含 HAL | L0 |
| 修改 `vendor/` 下的文件 | 芯片厂商代码 | 约定 |
| 绕过 JSON 在 C 中硬编码 HMI 规则 | 双引擎不一致 | L3 |
| 在 ISR 中做业务处理 | 时序破坏 | 架构 |

### 7.2 interface_map.h 操作铁律

```
1. 本文件只能手工编辑 — 它是文档, 不是生成目标
2. 任何 .c/.h include 它 → #error 炸掉
3. 新增通道 → 先在本文档注册, 再写代码
4. 修改签名 → 同步更新本文档
5. 删除模块 → 同步清理本文档中的相关通道
```

---

## 八、调度系统

### 8.1 10 槽轮转

```
TIM0 125us ISR → 8 次 = 1ms 标志
10 个槽位, 每槽 1ms, 完整周期 10ms:

槽 0: drv_key        — 触摸按键扫描 + 去抖
槽 1: app_hmi        — HMI 状态机
槽 2: app_cooking    — 烹饪控制
槽 3: app_power      — 功率管理
槽 4: app_comm_mgr   — MODBUS 轮询
槽 5: app_protect    — 安全保护
槽 6: drv_display    — 显示帧下发 (COM 扫描)
槽 7: (预留)
槽 8: (预留)
槽 9: (预留)
```

### 8.2 模块自计数

每个模块通过自计数实现更长周期:
```c
void App_Xxx_Run(void) {
    s_tick++;
    if (s_tick >= N) { s_tick = 0; /* 每 N×10ms 执行的逻辑 */ }
}
```

---

## 九、Git 提交规范

```
[milestone]         — 架构变更、方法论更新、重大里程碑
[project:four_head] — 此项目的日常提交
[common]            — 跨项目公共资源变更
[methodology]       — 方法论种子更新
```

**每次编码完成后必须提交。不积累未提交变更。**

---

## 十、快速参考卡

```
┌───────────────────────────────────────────────────────────┐
│                    FOUR_HEAD 快速参考 (v2.3)                 │
├────────────────────────────────────────────────────────────┤
│ 层依赖:     APP → DRV → HAL    (HAL 零依赖)                   │
│            APP → PROTO (仅 __weak 返回值函数)                  │
│ 通信:      PULL 路由, Switcher 指针直穿 (零拷贝)              │
│ OUTPUT:    *{Consumer}_params 指针, .c 中 static 实例绑定     │
│ INPUT:     *{Producer}_params 指针, InputCallback 直穿赋值    │
│ 头文件:    HAL 正常 include guard, 其余 //#define              │
│ IO接口:    include_io/*_io.h (codeGen 生成, 指针模式)          │
│ 数据源:    json/project.json (codeGen 唯一真相)               │
│ Switcher:  core/data_switcher.c (codeGen 生成)               │
│ 骨架:      core/std_module.h (MODULE_SKELETON + MODULE_EXPORT)│
│ 调度:      10 槽 × 1ms, 每 10ms 完整周期                      │
│ 编译器:    ARMCC V5.06, C99, Cortex-M0+                      │
│ 项目文件:   Project/32L14Tdmoe.uvprojx                        │
│ 验证:      check_deps.py → armcc → 提交                       │
│ codeGen:   ZEROLINK/codeGen/ (scan_project → code_gen)        │
│ 铁律:      违规应被阻断, 不被提醒                               │
│            L0(编译器) > L1(check_deps) > L2(codeGen) > L3(文档) │
└───────────────────────────────────────────────────────────┘
```

---

*本文件是 four_head 项目的唯一重启入口。任何新 AI 会话或接管工程师应从此文件开始。*
*与之配套的方法论体系见: `../methodology-seed-v2.0/ONBOARDING.md`*
