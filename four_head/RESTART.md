# RESTART.md — 四头电磁炉固件 · AI 重启入口

> **用途**: 新 AI 会话或接管工程师的第一个文件。读完本文件 ≈ 继承全部项目记忆。
> **最后更新**: 2026-06-21 (v2.3 指针直穿范式合并完成)

---

## 零、30 秒速览

```
MCU:        SC32L14TR8 (Cortex-M0+, 48MHz, 256K Flash, 16K SRAM)
编译器:      Keil MDK ARMCC V5.06 (C89, --c99)
项目文件:    Project/32L14Tdmoe.uvprojx
架构:       五层零耦合 — HAL → DRV → PROTO → APP + core
通信机制:    Switcher PULL 路由 + 指针直穿 (零拷贝, 编译器保证一致性)
约束系统:    L0(编译器) + L1(pre-commit check_deps) + L2(codeGen) + L3(文档)
头文件规则:  DRV/APP/PROTO 的 .h 禁止外部引用 — include guard 故意失效
公共接口:    include_io/*_io.h (codeGen 生成) + json/project.json (唯一数据源)
```

---

## 一、架构全景图

### 1.1 五层结构

```
┌────────────────────────────────────────────┐
│  APP  业务逻辑层 (app/)                        │
│  - AppHmi, AppCooking, AppPower,              │
│    AppProtect, AppCommMgr, AppSegAlign        │
│  - 禁止: include drv/ hal/ proto/ 其他app/     │
├────────────────────────────────────────────┤
│  PROTO 协议层 (proto/)                         │
│  - ProtoModbus                                │
│  - MODBUS RTU 编解码, AI块+纯函数+user_Process │
├─────────────────────────────────────────────┤
│  DRV  设备驱动层 (drv/)                         │
│  - DrvKey, DrvDisplay, DrvBuzzer,             │
│    DrvCommMgr                                 │
│  - 只 include hal/ + include_io/              │
├─────────────────────────────────────────────┤
│  HAL  硬件抽象层 (hal/)                         │
│  - hal_gpio, hal_timer, hal_comm, hal_uart,   │
│    hal_buzzer, hal_display, hal_smg, hal_key  │
│  - 零依赖: 不知道上层存在                        │
│  - 只 include vendor/ 寄存器头文件              │
├────────────────────────────────────────────┤
│  VENDOR 芯片厂商库 (vendor/)                    │
│  - CMSIS, FWLib, MCU_Drivers                  │
│  - 只读: 不修改 vendor 文件                     │
└─────────────────────────────────────────────┘
```

### 1.2 唯一合法的调用链

```
APP → Switcher PULL 路由 → DRV → HAL → VENDOR
APP → Switcher PULL 路由 → PROTO
```

模块间数据通过 `data_switcher.c` 的 PULL 路由（指针直穿），不直接调用。

---

## 二、v2.3 范式机制

### 2.1 模块文件结构 (前置范式段模式)

**每个模块的 .c 文件统一为：**

```c
// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====
#include "../include_io/xxx_io.h"

MODULE_SKELETON(Xxx);
/* PipeFlags_t / s_inPara / s_outPara / last_seq / OUTPUT_LINK */
static void Init(void) { ... }
static void user_Process(MODULE_INPUT(Xxx) *in, MODULE_OUTPUT(Xxx) *out, Xxx_PipeFlags_t flags);
static void ProcessInput(void) { ... seq比对 ... user_Process(in, out, flags); }
MODULE_EXPORT(Xxx);

// ===== [END AI GENERATED] =====

/* === 用户代码区 (codeGen 不触碰) === */
static void user_Process(MODULE_INPUT(Xxx) *in, MODULE_OUTPUT(Xxx) *out, Xxx_PipeFlags_t flags)
{
    /* 用户业务实现 — 直接访问 in/out 指针, 调用本文件内的 static 辅助函数 */
}
```

**关键原则**:
- AI 块始终在文件最前端，由 codeGen 生成/替换
- 用户代码在 AI 块之后，codeGen 只保留不动
- `user_Process` 前向声明在 AI 块内，实现在用户代码区
- codeGen 幂等：重复 `--only-modules` 输出一致

### 2.2 数据流

```
生产者 DoWork() → 写 output LINK.params + seq++ + ST_OUT
Switcher InputCallback (强符号) → 指针直穿: __in->Producer_params = __out->Consumer_params
消费者 ProcessInput() → seq 比对 → flags 置位 → user_Process(in, out, flags)
```

### 2.3 数据源: project.json

**位置**: `src/json/project.json` (schema_version 2.3)
**性质**: 模块/管道/字段的唯一真相源。codeGen 从此文件生成所有 IO 接口。

### 2.4 当前管道清单 (16 条)

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

### 2.5 ProtoModbus 合并模式 (范例)

ProtoModbus 是"纯函数库+范式"合并的典型范例：
- `proto_modbus.c`: AI块(1-61) + `static` 纯函数(CRC/Build/Parse) + 非static `Proto_*` wrapper(供 `app_comm_mgr` `__weak` 链接) + `user_Process`
- 不需要单独的壳文件 (`proto_modbus_module.c` 已删除)
- 不需要 .h 暴露内部函数签名，纯函数全 `static`

---

## 三、约束系统：违规即阻断

### 3.1 四个阻断层级

| 层级 | 机制 | 效果 | 本项目的实例 |
|------|------|------|-------------|
| **L0** | 编译器报错 | 绝对无法通过编译 | 头文件 `//#define GUARD` 失效 → 重复包含 → 重定义 error |
| **L1** | pre-commit 非零退出 | 能编译但不能提交 | `check_deps.py` 扫描跨层 #include |
| **L2** | 生成器保证一致性 | 手动改动被检出 | codeGen 从 project.json 生成 io.h/switcher/module |
| **L3** | 文档约定 | 最后手段 | 命名规范 |

**设计原则**: L3 是最后手段，不是默认手段。能升级到 L2/L1/L0 的规则必须升级。

### 3.2 L0 约束详解：头文件私有化

**DRV/APP/PROTO 层的每个 .h 文件，include guard 的 `#define` 必须注释掉：**

```c
#ifndef MODULE_NAME_H
//#define MODULE_NAME_H   // ← 故意注释！include guard 失效
...
#endif /* MODULE_NAME_H */
```

| 层 | 头文件私有化 | 原因 |
|----|------------|------|
| APP (app/*.h) | **是** | 只允许自身 .c + main.c 引用 |
| DRV (drv/*.h) | **是** | 只允许自身 .c + main.c 引用 |
| PROTO (proto/*.h) | **是** | 只允许自身 .c 引用 |
| HAL (hal/*.h) | **否** | DRV 层需要包含 HAL 头文件 |

### 3.3 L1 约束详解：check_deps.py

```bash
cd four_head
python tools/check_deps.py src/
```

检查规则:
- `app/*.c` 不得包含 `drv/` `hal/` `proto/`
- `drv/*.c` 不得包含 `app/` `proto/`
- `hal/*.c` 不得包含 `app/` `drv/` `proto/`
- `proto/*.c` 不得包含 `app/` `drv/` `hal/`
- `core/*.c` 不得包含 `app/` `drv/` `hal/` `proto/` (仅允许 `include_io/`)
- `main.c` 可 include hal/drv (它是硬件初始化合法入口)

**不通过 → 不得提交。**

---

## 四、codeGen 工具链

### 4.1 CLI 命令

```bash
# 全量生成 (io.h + switcher + module .c/.h)
python code_gen.py gen -c project.json -o <src路径>

# 按需生成
python code_gen.py gen -c project.json -o <src路径> --only-io       # 仅 io.h
python code_gen.py gen -c project.json -o <src路径> --only-switcher  # 仅 data_switcher.c
python code_gen.py gen -c project.json -o <src路径> --only-modules   # 仅模块 .c/.h
python code_gen.py gen -c project.json -o <src路径> --only-pipes     # io.h + switcher (管道相关)

# GUI 编辑
python gui_editor.py
```

### 4.2 生成原则

- **有文件+有AI块**: 替换 AI 块，保留用户代码
- **有文件+无AI块**: 在文件最前插入 AI 块，保留原内容（去掉旧文件头注释避免重复）
- **无文件**: 生成完整骨架 (AI块 + 占位注释)
- **已有 .c 只替换 AI 块，绝不覆盖用户代码**
- 每次生成前自动 .bak 备份

### 4.3 跨项目共享

codeGen 路径: `D:\OBSIDIAN\MOVING IH\ZEROLINK\codeGen\`

---

## 五、文件清单

### 5.1 源码文件 (src/)

#### APP 层 — 业务逻辑 (6 模块)

| 文件 | 职责 |
|------|------|
| `app/app_hmi.c/.h` | HMI 状态机、按键路由、显示逻辑 |
| `app/app_cooking.c/.h` | 烹饪控制、定时器、档位管理 |
| `app/app_power.c/.h` | 功率计算、功率下发指令生成 |
| `app/app_protect.c/.h` | 过温/过流保护、故障检测 |
| `app/app_comm_mgr.c/.h` | MODBUS 轮询调度、通讯数据缓存 |
| `app/app_seg_align.c/.h` | 段码对齐 |

#### DRV 层 — 设备驱动 (4 模块)

| 文件 | 职责 |
|------|------|
| `drv/drv_key.c/.h` | 触摸按键去抖、键码映射 |
| `drv/drv_display.c/.h` | 显示帧接收、COM/SEG IO 映射 |
| `drv/drv_buzzer.c/.h` | 蜂鸣器 PWM 控制、音效模式 |
| `drv/drv_comm_mgr.c/.h` | 通讯调度、TX/RX 缓冲管理 |

#### PROTO 层 — 协议 (1 模块)

| 文件 | 职责 |
|------|------|
| `proto/proto_modbus.c/.h` | MODBUS RTU 编解码 (AI块+纯函数+user_Process) |

#### 基础设施

| 文件 | 职责 |
|------|------|
| `main.c` | 系统入口、槽调度、hal/drv 初始化 |
| `core/std_module.h` | v2.3 统一模块骨架 (MODULE_SKELETON/MODULE_EXPORT/LINK/seq/route) |
| `core/data_switcher.c/.h` | v2.3 PULL 路由调度器 (INPUT_CALLBACK/INPUT_GET_SLOT/INPUT_LINK_PULL/INPUT_EDGE_PULL) |
| `include_io/*_io.h` | v2.3 模块 IO 接口 (由 codeGen 生成, LINK+PARAMS 指针模式) |
| `json/project.json` | codeGen 数据源 — 模块/管道/字段的唯一真相 (schema_version 2.3) |
| `cfg/hmi_data.c/.h` | HMI 配置数据 (由 gen_hmi.js 从 JSON 生成) |

### 5.2 项目本地工具 (tools/)

| 文件 | 用途 | 何时运行 |
|------|------|---------|
| `tools/check_deps.py` | 层依赖审计 | 每次编码后 |

---

## 六、调度系统

### 6.1 Slot 调度

```
11 槽 × 1ms = 11ms 完整周期

slot_order: [DrvKey, DrvCommMgr, AppCommMgr, ProtoModbus, AppPower, AppProtect, AppCooking, AppHmi, AppSegAlign, DrvBuzzer, DrvDisplay]
```

---

## 七、编译与验证

### 7.1 每次编码后的强制检查

```bash
# 1. 层依赖审计 (L1)
cd four_head
python tools/check_deps.py src/
# → 必须 0 violations

# 2. 独立编译验证 (L0 + 完整性)
cd Project
armcc -c ... ../src/app/app_xxx.c -o app_xxx.o
# → 必须 0 error, 0 warning

# 3. 全量编译
# 在 KEIL uVision 中: Project → Build Target
# → 必须 0 error, 0 warning
```

---

## 八、Git 提交规范

```
[milestone]         — 架构变更、方法论更新、重大里程碑
[project:four_head] — 此项目的日常提交
[common]            — 跨项目公共资源变更
[methodology]       — 方法论种子更新
```

**每次编码完成后必须提交。不积累未提交变更。**

---

## 九、当前状态与待办

### 已完成 (v2.3 指针直穿范式迁移)

- [x] codeGen 统一模块生成逻辑: 有文件保留用户代码+替换/插入AI块, 无文件生成骨架
- [x] `--only-pipes` CLI 命令 (io.h + switcher, 不动模块)
- [x] `cmd_gen` 使用 `source_file` 字段定位模块路径
- [x] ProtoModbus 合并: 删除壳文件 `proto_modbus_module.c`, 统一为前置范式段模式
- [x] check_deps: 0 violations (56 文件扫描)
- [x] codeGen 幂等验证: 重复 `--only-modules` 输出一致 (delta 0B)

### 待完成

- [ ] armcc 编译验证 (需要 Keil 编译器路径)
- [ ] 验证所有模块 user_Process 实现是否与旧代码逻辑一致
- [ ] AppSegAlign 模块 user_Process 实现 (新增模块)
- [ ] 清理 .bak 文件 (项目完成后再删)

---

## 十、快速参考卡

```
┌─────────────────────────────────────────────────────────┐
│                    FOUR_HEAD 快速参考 (v2.3)                 │
├──────────────────────────────────────────────────────────┤
│ 层依赖:     APP → DRV → HAL    (HAL 零依赖)                   │
│            APP → PROTO (Switcher PULL 路由)                    │
│ 通信:      PULL 路由, Switcher 指针直穿 (零拷贝)              │
│ 模块结构:  [AI块(范式)] + [用户代码] — 前置范式段模式           │
│ AI块内容:  SKELETON + PipeFlags + seq + LINK + Init +        │
│           ProcessInput(含seq比对) + MODULE_EXPORT              │
│ 用户代码:  user_Process 实现 + static 辅助函数                  │
│ 头文件:    HAL 正常 include guard, 其余 //#define              │
│ IO接口:    include_io/*_io.h (codeGen 生成, 指针模式)          │
│ 数据源:    json/project.json (codeGen 唯一真相)               │
│ Switcher:  core/data_switcher.c (codeGen 生成)               │
│ 骨架:      core/std_module.h (MODULE_SKELETON + MODULE_EXPORT)│
│ 调度:      11 槽 × 1ms, 每 11ms 完整周期                       │
│ 编译器:    ARMCC V5.06, C99, Cortex-M0+                      │
│ 项目文件:   Project/32L14Tdmoe.uvprojx                        │
│ 验证:      check_deps.py → armcc → 提交                       │
│ codeGen:   ZEROLINK/codeGen/code_gen.py                       │
│   --only-io / --only-switcher / --only-modules / --only-pipes │
│ 铁律:      违规应被阻断, 不被提醒                               │
│            L0(编译器) > L1(check_deps) > L2(codeGen) > L3(文档) │
└─────────────────────────────────────────────────────────┘
```

---

*本文件是 four_head 项目的唯一重启入口。任何新 AI 会话或接管工程师应从此文件开始。*
