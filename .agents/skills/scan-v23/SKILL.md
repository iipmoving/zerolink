---
name: scan-v23
description: "Scan v2.3 framework project → project.json. Reads *_io.h + data_switcher.c mechanically, no inference. Trigger: 扫描v2.3接口, scan v2.3, 框架扫描."
user-invocable: true
---

# /scan-v23 — v2.3 规范项目扫描 → JSON

对**已采用 v2.3 LINK+PARAMS 框架**的项目，从 `*_io.h` 和 `data_switcher.c` 中机械提取信息，输出 `project.json`。

**与 `/scan-legacy` 的区别**: 不做推断，只做提取。`*_io.h` 和 `data_switcher.c` 是 codeGen 生成的，结构固定，直接解析。

---

## §输出契约

与 `/scan-legacy` 完全相同的 JSON 格式 — 见下方 **§输出标准**。

---

## Step 1: 读取项目结构

Glob 查找:
- `include/*_io.h` — 每个模块一个
- `core/data_switcher.c` — SLOT 枚举 + INPUT_CALLBACK
- `core/data_switcher.h`

从 `data_switcher.c` 提取 **slot_order**（按 SLOT 枚举顺序）:
```c
SLOT(AppAdc) = 0,   → "AppAdc"
SLOT(Calculator) = 1, → "Calculator"
```

---

## Step 2: 提取 modules

对每个 `*_io.h`，从文件头提取 `@layer` 和 `@brief` → modules[]:
```json
{
  "id": "appadc",
  "name": "AppAdc",
  "layer": "app",
  "source_file": "app/adc_sensor.c",
  "comment": "ADC 采样与预处理"
}
```

---

## Step 3: 提取 pipes

从 `data_switcher.c`:
- `INPUT_CALLBACK(Producer, Consumer) { ... }` → 管道 `from: Producer, to: Consumer`
- callback_type 统一填 `pull`（v2.3 默认）

从 `*_io.h`:
- `MODULE_OUTPUT_PARAMS(Producer, Consumer)` 结构体中的 `params` 声明决定 LINK style:
  - `params[N]` → `array`, `array_size: N`
  - `*params`   → `pointer`, `array_size: 0`
  - `*params[N]` → `pointer_array`, `array_size: N`

---

## Step 4: 提取 fields

在 `{producer}_io.h` 的 `MODULE_OUTPUT_PARAMS(Producer, Consumer)` 前面找到 PARAMS 结构体定义，提取每个成员：

- `uint16_t resonant_current;` → `{ "name": "resonant_current", "type": "uint16_t", "comment": "..." }`
- `uint16_t hrtim_values[3];`  → `{ "name": "hrtim_values", "type": "uint16_t[3]", "comment": "..." }`
- 跳过 `res` / `reserved` 字段

对比 OUTPUT_PARAMS 和 INPUT_PARAMS 结构体。如果字段不一致，标记 `callback_type: "field_copy"`。

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
| `modules[].name` | PascalCase，与 `MODULE_SKELETON` 一致 |
| `modules[].comment` | **不可为空** |
| `pipes[].id` | `pipe_{from}_{to}` 全小写唯一 |
| `pipes[].callback_type` | v2.3 统一 `pull` |
| `pipes[].out_link` / `in_link` | 从 `*_io.h` 的 params 声明提取 |
| `pipes[].fields[].comment` | 从代码注释提取，**不可为空** |
| `slot_order` | 从 SLOT 枚举顺序，保持原始次序 |

> **完整标准见 `codeGen/config_schema.json`**。任何 AI 生成 JSON 后应以此文件校验。

写入 `codeGen/project.json`，再用 GUI 打开确认:

```bash
python codeGen/gui_editor.py --load codeGen/project.json
```