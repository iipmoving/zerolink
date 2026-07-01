# codeGen 生成器 — 文件分类与维护指南

> 生成时间: 2026-06-16
> 用途: 新建项目时知道哪些文件需要复制/生成，哪些是模板需要手动维护

---

## 一、文件分类总览

| 类别 | 说明 | 新建项目时怎么做 |
|------|------|----------------|
| **生成物** | 由生成器从 `project.json` 创建 | 生成器自动创建 |
| **模板** | 框架运行时文件，生成器不创建 | 从 codeGen 副本复制到项目 |
| **工具** | 仅生成器自身使用 | 不需要 |

---

## 二、生成物 (Generated) — 生成器自动创建

这些文件由 `code_gen.py` / `gui_editor.py` 的"全部生成"创建，写入目标项目的 `output_root` 下。

| 文件 | 生成器 | 说明 |
|------|--------|------|
| `include/*_io.h` | `gen_io_h.py` | 每个模块一个 io.h，定义 LINK/PARAMS 结构体 |
| `core/data_switcher.c` | `gen_switcher.py` | 模块注册 + InputCallback 实现 |
| `{layer}/{module_id}.c` | `gen_module_c.py` | 模块骨架 (AI GENERATED 段 + 用户代码) |
| `{layer}/{module_id}.h` | `gen_module_c.py::generate_module_h()` | 模块头文件 (仅声明 GetIO) |
| `project.json` | `code_gen.py::cmd_init()` | 项目配置 (仅在 `--init` 模式下创建) |

**特点**: 这些文件可以被生成器覆盖，不需要手动维护。

---

## 三、模板 (Templates) — 生成器不创建，需要手动复制

这些是 v2.3 框架的运行时文件，生成器只写 `#include` 引用它们，**不生成内容本身**。

### 3.1 核心模板文件

| 文件 | 存放位置 (codeGen) | 目标位置 (项目) | 说明 |
|------|-------------------|----------------|------|
| `std_module.h` | `codeGen/templates/std_module.h` | `core/std_module.h` | 模块骨架宏: `MODULE_SKELETON`, `MODULE_EXPORT`, `MODULE_IO_H`, `MODULE_INPUT`, `MODULE_OUTPUT`, `ST_NEW/ST_OUT/ST_INIT`, `Para_Grp_t`, `Info_Header`, `ModuleSlotDef` |
| `data_switcher.h` | `codeGen/templates/data_switcher.h` | `core/data_switcher.h` | 调度器头文件: `SLOT`, `SLOT_GETIO`, `INPUT_CALLBACK`, `INPUT_GET_SLOT`, `INPUT_LINK_PULL`, `INPUT_EDGE_PULL`, 函数声明 |

### 3.2 模板文件内容说明

#### `std_module.h` — 模块框架核心

生成器引用它的位置:
- `gen_io_h.py` line 116: 读取 `project.paths.std_module_path` 确认宏定义
- `gen_module_c.py` line 65: 生成 `MODULE_SKELETON({mod_name});`
- `gen_module_c.py` line 109: 生成 `MODULE_EXPORT({mod_name});`
- `gen_module_h.py` line 263: 生成 `MODULE_IO_H({mod_name});`

**修改规则**: 改了这个文件 → 必须同步更新生成器和所有项目的模板副本。

#### `data_switcher.h` — 调度器宏定义

生成器引用它的位置:
- `gen_switcher.py` line 60: 生成 `#include "data_switcher.h"`
- `data_switcher.c` 生成后 `#include "data_switcher.h"`

**修改规则**: 改了这个文件 → 必须同步更新生成器和所有项目的模板副本。

### 3.3 模板副本存放位置

```
codeGen/
├── templates/              ← 新建此目录
│   ├── std_module.h        ← 框架模板
│   └── data_switcher.h     ← 框架模板
├── gen_io_h.py
├── gen_switcher.py
├── gen_module_c.py
├── gui_editor.py
└── code_gen.py
```

每次更新 `std_module.h` 或 `data_switcher.h` 时，同步更新 `codeGen/templates/` 下的副本。

---

## 四、工具 (Tool-only) — 不需要复制到项目

这些文件只在 `codeGen/` 目录下使用，不参与项目生成。

| 文件 | 用途 |
|------|------|
| `config_schema.json` | JSON Schema 定义，用于校验 project.json |
| `keil_parser.py` | 解析 KEIL `.uvprojx` 文件 |
| `generate_project_json.py` | 通过 Claude CLI 自动生成 project.json |
| `check_json.py` | 校验 project.json 合规性 |
| `check_null_ptrs.py` | NULL 指针风险分析 |
| `test_data_flow.py` | 数据流模拟测试 |
| `codeGen-gui.spec` | PyInstaller 打包配置 |
| `dist/codeGen-gui.exe` | 打包后的 GUI 可执行文件 |
| `example_flow.json` | `--init` 模式的示例数据 |
| `four_head.json` / `m4_ekf.json` | 项目配置文件 |
| `out/` | 生成输出暂存目录 |

---

## 五、新建项目流程

```
1. 复制 codeGen/ 到项目工具目录
2. 复制 codeGen/templates/ 中的 std_module.h → 项目 core/ 目录
3. 复制 codeGen/templates/ 中的 data_switcher.h → 项目 core/ 目录
4. 用 GUI 或 CLI 解析 KEIL 项目 → 生成 project.json
5. 用 GUI 或 CLI 生成全部文件 → 覆盖 *_io.h, data_switcher.c, module .c/.h
6. 在 .c 文件中编写用户业务代码 (AI GENERATED 段之后)
```

---

## 六、维护规则

### 什么时候需要更新模板副本？

| 变更 | 需要更新模板？ | 需要重新生成？ |
|------|--------------|--------------|
| 改 `std_module.h` 宏定义 | **是** — 更新 `codeGen/templates/std_module.h` | 是 — 所有项目重新生成 |
| 改 `data_switcher.h` 宏定义 | **是** — 更新 `codeGen/templates/data_switcher.h` | 是 — 所有项目重新生成 |
| 改 `gen_io_h.py` 生成逻辑 | 否 — 生成器自身 | 是 — 重新生成 io.h |
| 改 `gen_switcher.py` 生成逻辑 | 否 — 生成器自身 | 是 — 重新生成 data_switcher.c |
| 改 `gen_module_c.py` 生成逻辑 | 否 — 生成器自身 | 是 — 重新生成 module .c |

### 同步更新检查清单

修改模板文件后:
- [ ] 更新 `codeGen/templates/` 下的副本
- [ ] 更新所有已有项目的 `core/` 下的副本
- [ ] 更新生成器中的相关引用 (如果有)
- [ ] 运行 `python codeGen/check_json.py` 验证
- [ ] Git 提交

---

## 七、常见问题

### Q: 为什么生成器不生成 std_module.h 和 data_switcher.h？

因为它们是整个项目的**框架基础设施**，所有模块共享同一份。生成器的职责是根据 `project.json` 生成**模块级别的代码** (io.h, module .c)，框架层面的宏定义属于"运行时"，应该由开发者维护。

### Q: 如果我想升级 std_module.h 怎么办？

1. 修改 `codeGen/templates/std_module.h`
2. 复制到所有项目的 `core/` 目录
3. 重新生成所有模块代码 (因为宏变了，生成的代码可能也需要适配)

### Q: 模板文件和项目文件的关系？

```
codeGen/templates/std_module.h   ← 权威版本 (生成器维护)
        ↓ 复制
项目/core/std_module.h           ← 运行时版本 (项目使用)
        ↓ 生成器引用
gen_io_h.py / gen_module_c.py    ← 写入 #include 指令
```

---

*维护者: 范式生成器团队*
*最后更新: 2026-06-16*
