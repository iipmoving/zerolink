---
name: scan-legacy
description: "Scan legacy/non-standard project → output project.json. AI reads .c/.h, extracts modules/pipes/fields mechanically, writes JSON. Trigger: 扫描接口, 导出JSON, scan legacy, 重构扫描, 接口分析."
user-invocable: true
---

# /scan-legacy — 遗留项目扫描 → JSON

对**未采用 v2.3 规范**的遗留项目，AI 读取源码提取模块边界和管道关系，输出 `project.json`。

**输出契约**: 最终 JSON 必须符合下方 **§输出标准**。GUI 依赖此格式解析。

---

## Step 1: 项目摸底

收集基本信息，填入 `project` 段：

```
1. 项目根目录？
2. 项目名称？
3. 代码目录结构？
   ├─ app/         → layer: "app"
   ├─ drv/         → layer: "drv"
   ├─ hal/         → layer: "hal"
   ├─ core/        → layer: "core"
   ├─ proto/       → layer: "proto"
   ├─ base_class/  → layer: "base_class"
4. 输出路径配置？(io.h 放哪里, switcher 放哪里)
```

执行: Glob 查找 `**/*.c` `**/*.h`，Grep 查找 `MODULE_SKELETON`、`MODULE_EXPORT`、`*_io.h`。

---

## Step 2: 识别模块

| 特征 | 判断 |
|------|------|
| 含 `MODULE_SKELETON` / `MODULE_EXPORT` | 模块 |
| 有 `*_io.h` 文件 | 模块 |
| 有独立 `.h` + `.c`，暴露 Init/Process 类函数 | 很可能 |
| 仅有 `.h` 的结构体/常量 | 非模块 |
| `main.c` / `startup_*.c` / `interrupt*.c` | 排除 |

记录每个模块: `name(PascalCase)`, `layer`, `source_file`, `comment(从文件头 @brief 提取)`。

---

## Step 3: 推断管道

管道 = Producer → Consumer 的数据流。判断依据：

| 特征 | 含义 |
|------|------|
| ModuleA.h 被 ModuleB.c `#include` | B 依赖 A |
| ModuleB 调用 ModuleA 的函数 | B 消费 A |
| `data_switcher.c` 中 `INPUT_CALLBACK(X, Y)` | 管道 X → Y |
| `*_io.h` 中 `MODULE_OUTPUT_PARAMS(A, B)` | 管道 A → B |

---

## Step 4: 提取字段

对每条管道，找到 Producer→Consumer 传递的结构体定义，提取字段名、类型、注释。类型规则：

- 原生类型: `uint16_t`, `int32_t` 等直接写入
- 数组: `uint16_t[4]` → type 写 `uint16_t[4]`
- 指针: `uint16_t*` 直接写入
- 嵌套结构体: 展开为扁平字段，或标注为 `struct Xxx`
- `res` / `reserved` 开头的填充字段: **跳过**

每个字段必须有 comment。

---

## Step 5: 输出 JSON

### §输出标准 — project.json

**`codeGen/config_schema.json`** 是规格的权威来源。此处仅列出核心约束速查，所有字段的精确定义以 Schema 为准。

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
    "comment_guard": true,
    "output_root": "../目标目录"
  },

  "modules": [
    {
      "id": "模块名_lowercase",
      "name": "PascalCase",
      "layer": "app | drv | base_class | hal | proto | core",
      "source_file": "layer/文件名.c",
      "comment": "模块职责"
    }
  ],

  "pipes": [
    {
      "id": "pipe_from_to",
      "from": "Producer",
      "to": "Consumer",
      "callback_type": "pull",
      "comment": "管道描述",
      "out_link": { "style": "pointer", "array_size": 0 },
      "in_link":  { "style": "pointer", "array_size": 0 },
      "fields": [
        { "name": "field_name", "type": "uint16_t", "comment": "字段作用" }
      ]
    }
  ],

  "slot_order": ["模块A", "模块B"]
}
```

### 核心约束速查

| 字段 | 约束 |
|------|------|
| `modules[].id` | 全小写 snake_case |
| `modules[].name` | PascalCase |
| `modules[].comment` | **不可为空** |
| `pipes[].id` | `pipe_{from}_{to}` 全小写唯一 |
| `pipes[].callback_type` | 默认 `pull`，除非有特殊边沿/搬运需求 |
| `pipes[].out_link` / `in_link` | 遗留项目不确定时统一 `pointer`/0 |
| `pipes[].fields[].comment` | **不可为空** |
| `slot_order` | 包含所有模块 |

> **完整标准见 `codeGen/config_schema.json`**。任何 AI 生成 JSON 后应以此文件校验。

写入 `codeGen/project.json`，然后用 GUI 打开确认：

```bash
python codeGen/gui_editor.py --load codeGen/project.json
```