# codeGen — v2.3 LINK+PARAMS 框架代码生成器

## 概述

codeGen 是一个代码生成工具，根据 **单一 JSON 配置** 自动生成嵌入式 C 项目的框架代码（接口头文件、数据交换机、模块骨架）。

**核心思想**: JSON 是唯一数据源。模块连接关系、数据结构、链路样式全部在 JSON 中定义。生成器保证代码与 JSON 完全一致，消除手写接口文件带来的不一致风险。

---

## 目录结构

```
codeGen/
├── code_gen.py          # CLI 主入口 (init + gen 双模式)
├── gen_io_h.py          # io.h 生成器 — 三层 LINK+PARAMS 结构体
├── gen_switcher.py      # data_switcher.c 生成器 — SLOT + Switcher_Run_SlotN
├── gen_module_c.py      # 模块 .c/.h 骨架生成器 (含用户区保留)
├── gui_editor.py        # Tkinter GUI 配置编辑器
├── example_flow.json    # 4 模块 5 管道示例配置
├── four_head.json       # four_head 项目扫描结果 (10 模块 12 管道)
└── project.json         # 当前工作配置 (由 GUI/CLI 产生)
```

---

## 安装

**依赖**: Python 3.8+（Tkinter 随 Python 自带，零额外依赖）

```bash
# 验证 Tkinter 可用
python -c "import tkinter; print('OK:', tkinter.TkVersion)"
```

---

## 使用方式

### 1. CLI 生成

```bash
# 从完整 JSON 生成框架代码
python code_gen.py gen --config project.json --output ../目标项目目录

# 部分生成
python code_gen.py gen --config project.json --output ../目标项目 --only-io
python code_gen.py gen --config project.json --output ../目标项目 --only-switcher
python code_gen.py gen --config project.json --output ../目标项目 --only-modules

# 初始化简化流向表 → 完整 JSON
python code_gen.py init --config flow.json --output project.json
```

### 2. GUI 编辑器

```bash
python gui_editor.py                    # 启动后手动加载 JSON
python gui_editor.py --load project.json # 启动时自动加载
```

GUI 功能:
- 打开/保存 JSON 配置文件
- 模块树浏览（模块 → 入参/出参 → 管道）
- 管道属性编辑（callback_type, LINK style, array_size）
- 字段 Table 编辑（双击内联修改名称/类型/注释）
- 代码实时预览（选中管道显示 io.h 片段）
- 一键生成（全部/io.h/switcher/modules）

### 3. AI Skills

| Skill | 用途 | 触发 |
|-------|------|------|
| `/scan-legacy` | AI 扫描遗留项目 → 输出 JSON | `扫描接口`, `导出JSON` |
| `/scan-v23` | 扫描 v2.3 规范项目 → 输出 JSON | `扫描v2.3接口` |

---

## 完整工作流

```
┌──────────────────────────────────────────────────────────────┐
│  Phase 1: 扫描（AI / 手动）                                  │
│                                                              │
│  AI 用 /scan-legacy 读取源码 → 输出 project.json             │
│  或 GUI 手动创建模块和管道 → 保存 JSON                       │
└──────────────────────────┬───────────────────────────────────┘
                           ↓
┌──────────────────────────┴───────────────────────────────────┐
│  Phase 2: 确认（GUI）                                         │
│                                                              │
│  python gui_editor.py --load project.json                    │
│  → 检查管道连接、字段类型、注释                              │
│  → 修改后保存 JSON                                           │
└──────────────────────────┬───────────────────────────────────┘
                           ↓
┌──────────────────────────┴───────────────────────────────────┐
│  Phase 3: 生成（CLI / GUI）                                   │
│                                                              │
│  python code_gen.py gen --config project.json --output <dir> │
│  → 生成 include/*_io.h                                       │
│  → 生成 core/data_switcher.c                                 │
│  → 生成 {layer}/*.c 骨架 + .h                                │
└──────────────────────────┬───────────────────────────────────┘
                           ↓
┌──────────────────────────┴───────────────────────────────────┐
│  Phase 4: 迁移（AI / 手动）                                   │
│                                                              │
│  AI 从旧代码中提取业务逻辑 → 填入新骨架的 [USER CODE] 区     │
│  确认 // ==== [USER CODE] ==== 之间的代码完整迁移             │
└──────────────────────────────────────────────────────────────┘
```

---

## 输出文件清单

### include/*_io.h

每个模块一个 io.h，包含:

```
OUTPUT 段:                          INPUT 段:
  PARAMS(Producer, Consumer)          PARAMS(Producer, Consumer)
    └── fields (业务数据)               └── fields (与 OUTPUT 对称)
  LINK(Producer, Consumer)           LINK(Producer, Consumer)
    └── status + *params               └── status + *params
  OUTPUT(Module)                     INPUT(Module)
    └── {Consumer}_params (实例)        └── {Producer}_params (实例)
```

关键规则:
- **对称命名**: OUTPUT 成员 = `{Consumer}_params`，INPUT 成员 = `{Producer}_params`
- **自包含**: Consumer 的 `_io.h` 不 `#include` Producer 的头文件
- **L0 阻断**: APP/PROTO 层 `//#define` 注释掉 include guard

### core/data_switcher.c

```
SwitcherSlot_t   ← SLOT 宏枚举
Switcher_Init()   ← SLOT_GETIO 注册全部模块
InputCallback()  ← 每 Consumer 一个回调，指针直穿
Switcher_Run_SlotN() ← 每个模块一个独立执行链
Switcher_Run_All()   ← 批量执行全部模块
```

用户在外部的 main loop / ISR 中自由组合调用:
```c
void MyControlLoop(void) {
    Switcher_Run_Slot0();  // ADC
    Switcher_Run_Slot1();  // 计算
    Switcher_Run_Slot2();  // 参数提取
    // ...
}
```

### {layer}/*.c 骨架

每个模块一个 .c，结构:

```
[AI GENERATED] 区:
  #include, MODULE_SKELETON
  PipeFlags_t (位域, 每 BIT 一个上游管道)

[USER CODE] 区 ← 重生成时保留:
  Module_Process(in, out, flags)  — 用户业务函数
  用户常量/变量

[AI GENERATED] 区:
  Init()          — g_input/g_output 绑定
  ProcessInput()  — 输入检查 → 用户函数 → 状态管理
  MODULE_EXPORT()
```

---

## JSON 配置格式

```json
{
  "schema_version": "1.0",

  "project": {
    "name": "项目名",
    "paths": {
      "io_dir": "include",
      "core_dir": "core",
      "app_dir": "app",
      "base_class_dir": "base_class",
      "proto_dir": "proto"
    },
    "std_module_path": "core/std_module.h",
    "pack": 4,
    "output_root": "../target_dir"
  },

  "modules": [
    {
      "id": "模块名_lowercase",
      "name": "PascalCase模块名",
      "layer": "app | drv | base_class | proto | core",
      "source_file": "layer/文件名.c",
      "comment": "模块职责描述"
    }
  ],

  "pipes": [
    {
      "id": "pipe_唯一标识",
      "from": "Producer模块名",
      "to": "Consumer模块名",
      "callback_type": "pull | edge | field_copy",
      "comment": "管道描述",

      "out_link": {
        "style": "array | pointer | pointer_array",
        "array_size": 4
      },
      "in_link": {
        "style": "array | pointer | pointer_array",
        "array_size": 4
      },

      "fields": [
        { "name": "字段名",     "type": "uint16_t",     "comment": "字段说明" },
        { "name": "数组字段",   "type": "uint16_t[4]",  "comment": "批量数据" },
        { "name": "指针字段",   "type": "uint16_t*",    "comment": "缓冲区指针" }
      ]
    }
  ],

  "slot_order": ["模块A", "模块B", "模块C"]
}
```

### 字段说明

| 字段 | 说明 |
|------|------|
| `module.id` | 全小写标识符 |
| `module.name` | PascalCase，对应 `MODULE_SKELETON(name)` |
| `module.layer` | 决定输出目录映射 |
| `pipe.callback_type` | `pull`=指针直穿, `edge`=边沿触发, `field_copy`=逐字段搬运 |
| `pipe.out_link.style` | `array`=定长数组, `pointer`=单指针, `pointer_array`=指针数组 |
| `pipe.fields[].type` | 类型表达式，数组用 `type[n]` |

---

## v2.3 架构原理

### 三层结构

```
PARAMS 层: 纯业务数据字段
    typedef struct { uint16_t current; uint8_t valid; } xxx_Params;

LINK 层: status + params
    typedef struct { uint8_t status; xxx_Params *params; } xxx_Link;

OUTPUT/INPUT 层: 模块级聚合
    typedef struct { LINK_A A_params; LINK_B B_params; } Module_Output;
```

### 管道配对

```
Producer 侧:                          Consumer 侧:
  MODULE_OUTPUT_PARAMS(A, B)             MODULE_INPUT_PARAMS(A, B)
    └── fields (同)                        └── fields (同, 自包含)
  MODULE_OUTPUT(A)                       MODULE_INPUT(B)
    └── B_params (实例)                     └── A_params (实例)
```

InputCallback 通过指针直穿打通，零拷贝:
```c
in->A_params = (void*)&out->B_params;
```

### 状态位

| 位 | 常量 | 含义 |
|----|------|------|
| bit0 | ST_INIT (0x01) | 已初始化 |
| bit1 | ST_NEW (0x02) | 新输入到达 |
| bit2 | ST_OUT (0x04) | 输出就绪 |

### 管道就绪标志 (PipeFlags_t)

对多输入的 consumer，生成位域结构体:

```c
typedef union {
    uint8_t all;
    struct {
        uint8_t producer_a : 1;
        uint8_t producer_b : 1;
    } bits;
} Module_PipeFlags_t;
```

ProcessInput 中:
```c
flags.bits.producer_a = (in->A_params.status & ST_NEW) ? 1 : 0;
flags.bits.producer_b = (in->B_params.status & ST_NEW) ? 1 : 0;
if (!flags.all) return;  // 全零则跳过
Module_Process(in, out, flags);
```

---

## 用户区保护机制

`.c` 文件中 `// ==== [USER CODE] ====` 和 `// ==== [END USER CODE] ====` 之间的内容在重生成时自动保留。

```
┌─ [AI GENERATED]  include, MODULE_SKELETON, PipeFlags_t
├─ ===== [USER CODE] =====        ← 边界标记
├─ [用户] Module_Process()          ← 重生成保留
├─ [用户] 常量/变量                ← 重生成保留
├─ ===== [END USER CODE] =====    ← 边界标记
├─ [AI GENERATED]  Init, ProcessInput, MODULE_EXPORT
└──────────────────────────────────
```

---

## 项目迁移指南

### 从遗留项目迁移

```bash
# Step 1: AI 扫描
/scan-legacy                    # 读取 .c/.h → project.json

# Step 2: GUI 确认
python gui_editor.py --load project.json

# Step 3: 生成骨架到新目录
python code_gen.py gen --config project.json --output ../项目/src2

# Step 4: AI 迁移业务逻辑
# 从原 .c 提取业务代码 → 填入 src2/*.c 的 [USER CODE] 区
```

### 从 v2.3 项目更新

```bash
# Step 1: 扫描现有框架
/scan-v23                       # 读取 _io.h + data_switcher.c → project.json

# Step 2: GUI 修改
python gui_editor.py --load project.json

# Step 3: 重新生成
python code_gen.py gen --config project.json --output ../项目/src2
```
