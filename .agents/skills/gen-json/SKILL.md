---
name: gen-json
description: "AI 机械式扫描项目 → 生成 project.json。读完所有源文件才写，不靠记忆。触发: 生成JSON, 扫描项目, 重构扫描, gen-json"
user-invocable: true
---

# /gen-json — 机械式扫描 → project.json

> **铁律**: 未读完所有 `_io.h` + `MODULE_SKELETON(.c)` + `data_switcher.c` 之前，不得写入任何一行 JSON。

---

## 输出契约

最终 JSON 写入 `codeGen/out/<项目>/project.json`，`output_root` **必须用绝对路径**。

---

## 执行顺序（不可跳跃）

### Step 0: 摸底

```
项目根目录 = ?
项目名称   = ?
KEIL 项目路径 = ?  (用于核对路径一致性)
```

Glob 所有 `**/*.c` `**/*.h`，初步了解目录结构。确定 layer 映射：

| 目录 | layer |
|------|-------|
| `app/` | app |
| `base_class/` | base_class |
| `drv/` | drv |
| `hal/` | hal |
| `proto/` | proto |
| `core/` | core |

**找到 `data_switcher.c` 的实际位置**（可能在 `core/` 或 `Projects/core/`），这是 core_dir 的依据。

**找到 `std_module.h` 的实际位置**，这是 std_module_path 的依据。

### Step 0a: ★ 解析 KEIL 项目文件（铁律 — 必须执行）

用 `codeGen/keil_parser.py` 解析 `.uvprojx`，提取所有 `.c` 的 `rel_path` 和 `abs_path`：

```bash
python codeGen/keil_parser.py path/to/project.uvprojx [app base_class core ...]
```

**输出文件保存在 `codeGen/out/<项目>/`：**
- `ai_scan_data.json` — 结构化扫描数据（模块/文件/函数/结构体/宏）
- `ai_scan_doc.md` — 人类可读的扫描报告

**这两个文件是后续所有步骤的源数据基础**，project.json 的模块和文件列表必须从中提取，不得越过它们直接 Glob/Grep。

---

### Step 1: 读 `_io.h`

只读 `ai_scan_data.json` 中列出的文件对应的 `_io.h`。**不读不在扫描数据中的文件。**

`_io.h` 的目录 = `include_io` 或用扫描数据确定的 io 输出目录。逐个完整读取，记录：

#### 1a. 模块声明

`MODULE_IO_H(Xxx)` → 模块名 `Xxx`。从文件头 `@brief` 或 `@layer` 提取 layer 和 comment。

#### 1b. OUTPUT 管道

`MODULE_OUTPUT_PARAMS(Producer, Consumer)` → 管道 Producer→Consumer。记录其所有 fields（name, type, comment）。

`MODULE_OUTPUT_LINK(Producer, Consumer)` → 确定 out_link.style：
- `*params` → `"style": "pointer", "array_size": 0`
- `params[N]` → `"style": "array", "array_size": N`

#### 1c. INPUT 管道

`MODULE_INPUT_PARAMS(Producer, Consumer)` → 与 OUTPUT 侧对称。**必须与 OUTPUT PARAMS 完全相同**（但方向相反）。

`MODULE_INPUT_LINK(Producer, Consumer)` → 确定 in_link.style（同上规则）。

#### 1d. 嵌套 struct 处理（此处最容易漏）

有两种嵌套结构体，**必须区分处理**：

**类型 A — 独立 typedef，被指针引用：**
```c
typedef struct {
    uint16_t start;
    uint16_t end;
    // ...
} AppAdc_OutputParams_t;

// 管道字段引用它:
AppAdc_OutputParams_t* input;
```
→ 提取为 `modules[].types[]`（**铁律：必须挂在所属模块上，不是全局 `types[]`，否则 `generate_io_h()` 读不到**）：
```json
{
  "name": "AppAdc",
  ...
  "types": [
    {
      "name": "AppAdc_OutputParams_t",
      "comment": "每通道 HRTIM 工作周期参数",
      "fields": [
        {"name": "start", "type": "uint16_t", "comment": "HRTIM 开始点"},
        ...
      ]
    }
  ]
}
```
→ 管道字段 type 写 `"AppAdc_OutputParams_t*"`。

**类型 B — 内联匿名 struct，直接嵌入 PARAMS：**
```c
typedef struct {
    struct {
        uint16_t highOff;
        uint16_t lowOff;
    } hrtim;
    uint16_t peak_current;
} MODULE_OUTPUT_PARAMS(Calculator, ElecParams);
```
→ 管道字段提取为嵌套格式：
```json
{
  "name": "hrtim",
  "type": "struct",
  "comment": "HRTIM 时序参数",
  "fields": [
    {"name": "highOff", "type": "uint16_t", "comment": "高端关闭 HRTIM 值"},
    {"name": "lowOff", "type": "uint16_t", "comment": "低端关闭 HRTIM 值"}
  ]
}
```

**检测方法**：对每个 pipe field，检查其类型是否在同一文件中以 `typedef struct` 定义。如果是 → 类型 A。如果字段本身是匿名 struct → 类型 B。

#### 1e. 填充字段跳过规则

`res` / `reserved` 开头的字段 → **跳过**，不写入 JSON。

---

### Step 2: 读 `data_switcher.c`

找到 Slot 枚举和 `SLOT_GETIO` 调用，确定：
- 哪些模块实际在 switcher 中注册（`slot_order`）
- 数据流执行顺序（Slot 编号顺序）

**`slot_order` 按 Slot 编号排列，不是臆测的"数据流顺序"。**

---

### Step 3: 验证 `MODULE_SKELETON`

在 `ai_scan_data.json` 列出的文件范围内，Grep `MODULE_SKELETON(Xxx)`。**不得扫描扫描数据之外的文件（避免把旧备份/废弃模块带进来）。**

```
MODULE_SKELETON(AppAdc) 在 ../../LIB/APP/adc_sensor.c → source_file = "src/.../LIB/APP/adc_sensor.c"
```

**source_file 硬规则：**

每个模块的 `source_file` 必须是 KEIL `FilePath` 解析后**相对 `output_root` 的路径**。不得自行猜测，不得用 Glob 发现的路径代替。

```python
# 正确推导方式：
keil_abs = os.path.normpath(os.path.join(proj_dir, keil_filepath))
source_file = os.path.relpath(keil_abs, output_root).replace("\\", "/")
```

不在 KEIL 中的文件（如 IncludeInBuild=0）用实际路径，但必须标注 `(暂排除)`。

记录每个模块的 `@brief` 作为 comment（从 `_io.h` 文件头提取）。

---

### Step 4: 核对管道对称性

对每条管道 Producer→Consumer：

| 核对项 | 方法 |
|--------|------|
| OUTPUT_PARAMS 字段 == INPUT_PARAMS 字段 | 逐一比对 name + type，**必须完全一致** |
| 嵌套 struct 两侧一致 | 如果用类型 A 的 typedef，两侧用同一个 typedef 名 |
| 数组大小一致 | out_link.array_size 与 producer 输出数组大小一致 |

**不一致时：**
- 停住，列出差异，问用户
- 不得自行"修正"

---

### Step 5: 输出 JSON

用 `json.dump(indent=2, ensure_ascii=False)` 写入。

**JSON 结构规则：**

| 字段 | 规则 |
|------|------|
| `output_root` | **绝对路径**，指向项目根 |
| `paths.core_dir` | 从 data_switcher.c 实际位置确定 |
| `source_file` | 相对于 output_root 的相对路径 |
| `std_module_path` | 相对于 output_root 的相对路径 |
| `types` | **必须挂载到 `modules[].types[]`**，不存在全局 `types[]` |
| `slot_chains[].modules` | **字段名必须是 `modules`**（不是 `slots`），GUI 读 `ch["modules"]` |

**自检清单（必须逐条确认）：**

- [ ] 每个 `_io.h` 的 `MODULE_IO_H(Xxx)` 都有对应 module
- [ ] 每个 `MODULE_SKELETON(Xxx)` 都有对应 module
- [ ] 每个 module 都有 source_file（且文件实际存在）
- [ ] 每个 source_file 与 KEIL 项目 `FilePath` 解析结果一致（用 `python codeGen/keil_parser.py` 验证）
- [ ] 不在 KEIL 中的模块已标注 `(暂排除)`
- [ ] 每条管道的 Producer 和 Consumer 都在 modules[] 中
- [ ] 没有重名模块
- [ ] 每个 field 都有 comment（非空）
- [ ] 所有 `res`/`reserved` 字段已跳过
- [ ] 所有独立 typedef（类型 A）已提取到 `modules[].types[]`（**铁律：挂在所属模块上，不是全局 `types[]`**）
- [ ] 验证每个 typedef 的所属模块: 哪个模块的 io.h 中定义了它，就挂到哪个模块上
- [ ] 所有内联嵌套 struct（类型 B）已递归展开
- [ ] `slot_chains[].modules` 字段名正确（不是 `slots`，GUI 读 `ch["modules"]`）
- [ ] `slot_order` 包含所有 data_switcher 中注册的模块
- [ ] `output_root` 是绝对路径
- [ ] `core_dir` 与 data_switcher.c 实际位置一致
- [ ] 最后用 `python -c "import json; json.load(open('...'))"` 验证 JSON 语法

---

## 输出后

用 GUI 打开验证：

```bash
python codeGen/gui_editor.py
# 文件 → 导入 project.json → 选择 codeGen/out/<项目>/project.json
```

告知用户：
- 共扫描到 N 个模块，M 条管道
- 嵌套 struct 处理情况（几个类型 A，几个类型 B）
- 自检结果
