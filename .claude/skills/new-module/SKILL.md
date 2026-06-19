---
name: new-module
description: "Interactive wizard to create a new module using std_module.h — data structs + MODULE_SKELETON + ProcessInput + Init + MODULE_EXPORT. Also handles AI interface maintenance workflow (scan → JSON → GUI → generate). Triggers on: new module, add module, create module, new-module, 新增模块, 新建模块, 添加模块, update-json, 更新JSON, 接口维护."
user-invocable: true
---

# /new-module — 模块创建向导

创建符合 v2.3 架构的新模块（`std_module.h` PULL 模式）。支持可选的 ISR 实时通路。

**核心**: `std_module.h` — **模块的骨架宏就是范式**

> v2.3 新增: ISR 模块可选。在文件顶部加 `#define STD_MODULE_ENABLE_ISR 1` 启用 ISR 通路。

---

## Step 1: 模块身份

```
1. 模块名称？(snake_case，如 "power_ctrl")
2. 所属层？(app | drv | hal | proto | core)
3. 接收哪些模块的数据？(列出生产者模块名)
```

---

## Step 1.5: 数据契约 — I/O 对齐检查

在生成代码前，先检查数据交互说明书：

```
1. 项目根目录是否有 `docs/data-contract.md`？
   ├─ 有 → 检查新模块的 I/O 结构体是否已在其中定义
   └─ 无 → 创建 `docs/data-contract.md`，参照 `methodology-seed/10-data-contract.md`

2. 本模块的输入结构体与上游生产者输出结构体是否布局一致？
   ├─ 是 → InputCallback 用指针直穿（生产者 LINK 地址赋给消费者输入指针，零拷贝）
   └─ 否 → 与生产者对齐字段顺序/类型/大小，或说明理由

3. 本模块的输出结构体与下游消费者输入结构体是否布局一致？
   ├─ 是 → 消费者 InputCallback 可用指针直穿
   └─ 否 → 与消费者对齐

4. 本模块是否属于某模块组？
   ├─ 是 → 组内数据流必须全部指针直传，不外溢
   └─ 否 → 跳过
```

**🚫 强制约束：头文件自治规则**

任何模块的 `.h` 文件（包括 `_io.h` 和普通 `.h`）**不得 `#include` 其他模块的 `.h`**。

| 文件类型 | 允许 include | 禁止 include |
|---------|-------------|-------------|
| `_io.h` | `<stdint.h>` `"std_module.h"` 等系统/框架头 | 任何其他模块的 `_io.h` 或 `.h` |
| 普通 `.h` | `<stdint.h>` `"std_module.h"` 等系统/框架头 | 任何其他模块的 `.h` |

需要引用其他模块的数据类型时：在自己 `_io.h` 中自包含定义输入结构体，布局与生产者输出兼容。InputCallback 用指针直穿，不依赖对方的类型定义。

数据契约定稿后，再继续生成代码模板。

---

## Step 1.6: 核心概念 — 管道配对 (PIPE PAIRING)

**这是整个 PULL 范式最核心的模式。**

```
消费者向生产者注册一个管道，双方用同一套结构体布局，
但用 MODULE_OUTPUT_LINK / MODULE_INPUT_LINK 宏分别声明，
两个结构在模块双方独立声明，通过中间层 (data_switcher.c) 连接。
```

### 管道配对规则

```
Producer (输出方)                                Consumer (输入方)
─────────────────────────────                    ─────────────────────────────
MODULE_OUTPUT_PARAMS(Producer, Consumer)           MODULE_INPUT_PARAMS(Producer, Consumer)
  └── 字段布局: { field1, field2, ... }              └── 字段布局: 完全一致
                                                    (自包含, 不引用 producer 类型)

MODULE_OUTPUT_LINK(Producer, Consumer)              MODULE_INPUT_LINK(Producer, Consumer)
  └── status + params[] 实例                         └── status + params[] 实例
                                                    (自包含, 布局与 OUTPUT_LINK 一致)

MODULE_OUTPUT(Producer)                             MODULE_INPUT(Consumer)
  └── MODULE_OUTPUT_LINK *{Consumer}_params (指针)    └── MODULE_INPUT_LINK* {Producer}_params (指针)
                                                          ↑ InputCallback 直穿赋值
```

### 关键约束

| 规则 | 说明 |
|------|------|
| 自包含 | Consumer 的 `_io.h` 定义自己的 INPUT_PARAMS / INPUT_LINK，不 `#include` Producer 的任何头文件 |
| 布局一致 | INPUT_PARAMS 的字段顺序/类型/大小与 OUTPUT_PARAMS 完全相同 |
| 宏不展开 | 类型定义必须用 `MODULE_INPUT_PARAMS(...)` / `MODULE_OUTPUT_LINK(...)` 宏，不得写展开后的名字 |
| void* 直穿 | InputCallback 用 `(void*)` 将 producer OUTPUT_LINK 地址赋给 consumer INPUT_LINK 指针 |
| 实例 vs 指针 | OUTPUT_LINK 在 producer 端是指针 (*{Consumer}_params)，INPUT_LINK 在 consumer 端是指针 (*{Producer}_params)。Producer 模块 .c 中声明内部 MODULE_OUTPUT_LINK 实例，Init() 中绑定指针 |

---

## Step 2: 创建模板

### 2.1 IO 头文件 `include/{module}_io.h`

```c
/**
 * @file    {module}_io.h
 * @brief   {Module} I/O (v2.3 LINK+PARAMS)
 * @layer   {layer}
 *
 * 输入: ProducerA (来自谁)
 * 输出: ConsumerB (发给谁)
 *
 * 管道配对命名:
 *   MODULE_OUTPUT_LINK(Producer, {Module})   → out->{Module}_params    (Producer 的 io.h)
 *   MODULE_INPUT_LINK(Producer, {Module})    → in->{Producer}_params   (本文件声明)
 */
#ifndef {MODULE}_IO_H
#define {MODULE}_IO_H

#include <stdint.h>
#include "std_module.h"

#define {MODULE}_POTMAX  4

/* ========== 输入 — 从 ProducerA 接收的数据 (自包含) ========== */

/* 数据参数: 布局与 MODULE_OUTPUT_PARAMS(ProducerA, {Module}) 一致 */
typedef struct {
    uint16_t field1;
    uint16_t field2;
    uint8_t  valid;
    uint8_t  res[3];
} MODULE_INPUT_PARAMS(ProducerA, {Module});

/* 输入管道: 与 MODULE_OUTPUT_LINK(ProducerA, {Module}) 配对 */
typedef struct {
    uint8_t  status;
    uint8_t  res[3];
    MODULE_INPUT_PARAMS(ProducerA, {Module}) params[{MODULE}_POTMAX];
} MODULE_INPUT_LINK(ProducerA, {Module});

/* 输入聚合: InputCallback 直穿赋值 {Producer}_params 指针 */
typedef struct {
    MODULE_INPUT_LINK(ProducerA, {Module})* {ProducerA}_params;  /* ← 以生产者命名 */
} MODULE_INPUT({Module});

/* ========== 输出 — 发给 ConsumerB 的数据 ========== */

/* 数据参数 */
typedef struct {
    uint16_t result;
    uint8_t  valid;
    uint8_t  res[5];
} MODULE_OUTPUT_PARAMS({Module}, ConsumerB);

/* 输出管道: 与 MODULE_INPUT_LINK({Module}, ConsumerB) 配对 */
typedef struct {
    uint8_t  status;
    uint8_t  res[3];
    MODULE_OUTPUT_PARAMS({Module}, ConsumerB) params[{MODULE}_POTMAX];
} MODULE_OUTPUT_LINK({Module}, ConsumerB);

/* 输出聚合 */
typedef struct {
    MODULE_OUTPUT_LINK({Module}, ConsumerB) *{ConsumerB}_params;  /* ← 以消费者命名 (指针) */
} MODULE_OUTPUT({Module});

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H({Module});

#endif /* {MODULE}_IO_H */
```

### 2.2 源文件 `src/{layer}/{module}.c`

```c
/**
 * @file    {module}.c
 * @brief   一句话描述
 * @layer   {layer}
 *
 * 输入: ProducerA (管道直穿)
 * 输出: ConsumerB (管道直读)
 */
#include "../include/{module}_io.h"
#include <string.h>

/* ---- 数据结构实例 ---- */
static {Module}_Input  s_in;    // MODULE_INPUT({Module})  展开
static {Module}_Output s_out;   // MODULE_OUTPUT({Module}) 展开

/* ---- 内部 OUTPUT_LINK 实例 (指针直穿目标) ---- */
static MODULE_OUTPUT_LINK({Module}, ConsumerB)  s_{Module}To{ConsumerB}Link;

/* ---- 骨架 ---- */
MODULE_SKELETON({Module});

/* ---- 处理逻辑 (每帧被调) ---- */
static void ProcessInput(void)
{
    {Module}_Input  *in  = ({Module}_Input *)g_input.para;
    {Module}_Output *out = ({Module}_Output *)g_output.para;

    if (!in->{ProducerA}_params) return;

    /* ====== 输入段: 检查数据 LINK 的 status ====== */
    if (!(in->{ProducerA}_params->status & ST_NEW)) return;
    in->{ProducerA}_params->status &= ~ST_NEW;

    /* 消费 in->{ProducerA}_params->params[h].field1 */

    /* ====== 计算段 ====== */
    out->{ConsumerB}_params->params[0].result = calc();

    /* ====== 输出段: 置输出 LINK status ====== */
    out->{ConsumerB}_params->status |= ST_OUT;
}

/* ---- 初始化 ---- */
static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));

    /* 绑定 OUTPUT_LINK 指钺 */
    s_out.{ConsumerB}_params = &s_{Module}To{ConsumerB}Link;

    g_input.para  = &s_in;
    g_output.para = &s_out;
}

/* ---- 导出 ---- */
MODULE_EXPORT({Module});

/* ---- Consumer 回调: data_switcher.c 中覆盖强符号 (INPUT_CALLBACK 宏) ---- */
/* ⚠️ 注意: INPUT_CALLBACK 只在 data_switcher.c 中写, 不在模块 .c 中写 */
/* 模块 .c 中只需写 MODULE_SKELETON + ProcessInput + MODULE_EXPORT */
```

### 关键规则

| 元素 | 规则 |
|------|------|
| `MODULE_SKELETON({Module})` | 文件顶部调用，展开 `g_input`/`g_output`/`Constructor`/`DoWork` |
| `Init()` | 初始化 `s_in`/`s_out`，绑定 `g_input.para`/`g_output.para` |
| `ProcessInput()` | 三段式：检查 ST_NEW → 消费输入 → 计算 → 写输出 |
| `MODULE_EXPORT({Module})` | 文件底部调用，生成 `GetIO` (不生成 __weak OnOutput) |
| `INPUT_CALLBACK({Module})` + `INPUT_GET_SLOT(ProducerA, {Module})` | **仅在 data_switcher.c 中写**。一个 consumer 一个回调，内部放多个 INPUT_GET_SLOT。INPUT_GET_SLOT 指针直穿（将生产者 OUTPUT_LINK 地址赋给消费者 INPUT_LINK 指针，零拷贝）|

### 状态位 (`core/std_module.h`)

| 位 | 常量 | 含义 |
|----|------|------|
| bit0 | `ST_INIT (0x01)` | 已初始化（Constructor 置位） |
| bit1 | `ST_NEW (0x02)` | 新输入到达（InputCallback 置位，ProcessInput 消费后自清） |
| bit2 | `ST_OUT (0x04)` | 输出就绪（ProcessInput 输出段置位，Switcher 路由后消费） |

### 数据层 status 规则 (v2.3)

**status 位于数据 struct LINK 首字节**，不在 `Para_Grp_t` 包装层：

| 层级 | status 位置 | 谁读写 | 用途 |
|------|------------|--------|------|
| 框架层 | `g_input.info.status` | Constructor/DoWork | ST_INIT 管理 |
| 数据层 | `in->{Producer}_params->status` (单LINK) / `in->adc.status` (多LINK) | InputCallback/ProcessInput | ST_NEW/ST_OUT 数据流控制 |

**ProcessInput 只读写数据层 status**，不碰 `g_input.info.status`。

### route 分流 (info.route)

consumer 的 ProcessInput 可通过 `g_input.info.route` 选择不同的执行路径：

```c
static void ProcessInput(void)
{
    switch (g_input.info.route) {
    case 1: /* 路径A */ break;
    case 2: /* 路径B — @OUTPUT_CALLBACK 即时触发 */ break;
    }
}
```

正常 PULL 路由无需 route，producer 不设 route = 0 即可。`@OUTPUT_CALLBACK` 例外场景下，producer 设 route 值指定 consumer 执行路径。

---

## Step 3: 注册到 Switcher

### 3.1 Switcher 注册

在 `src/core/data_switcher.h` 的槽位枚举中添加：
```c
    SLOT({Module}) = N,       // 在 SLOT(COUNT) 之前
```

在 `src/core/data_switcher.c` 的 `Switcher_Init()` 添加：
```c
    SLOT_GETIO({Module});
```

### 3.2 Switcher 路由

如果本模块是 **producer**，在 `Switcher_Run()` 中添加路由：

```c
/* {Module} DoWork 之后立即路由 */
s_slot[SLOT({Module})].pDoWork();
_route_{module}();  /* 检查 has_* 标志 → 调 consumer 回调 */
```

如果本模块是 **consumer**，添加 consumer 回调并在路由函数中调用。

---

## Step 4: 验证

```bash
python ../.claude/tools/check_deps.py .
python ../.claude/tools/check_include.py .
python ../.claude/tools/check_output_callback.py .
```

## @OUTPUT_CALLBACK 例外

如需绕开 Switcher 周期立即触发 consumer (如蜂鸣器实时反馈)：

1. Producer 写 `g_output.para` + 设 `g_output.info.route`
2. Producer 立即调 consumer 的 DoWork 或回调
3. 添加 `/* @OUTPUT_CALLBACK: <reason> — user confirmed */` 标记
4. 在 `interface_map.h` 白名单注册
5. `check_output_callback.py` 验证通过

**无标记的输出回调 → check_output_callback.py 阻断提交。**

---

## Step 5: AI 接口维护工作流 (JSON 驱动)

**双向维护数据源**:
```
src/JSON/project.json    ← 接口权威数据源 (AI 修改这里)
src/include_io/*_io.h    ← 模块接口定义 (自动生成)
```

**核心原则**: `JSON/project.json` 是唯一数据源。AI 修改接口 → 改 JSON → GUI 确认 → 生成代码覆盖 include_io/。

### 5.1 扫描确认

当用户说"新增/修改接口"或"程序已改 JSON 没更新"时：

```bash
# 扫描 include_io/ → 写入 JSON/project.json
python codeGen/scan_project.py <项目路径> [--name <项目名>]
```

输出:
```
[OK] project.json 已生成: <项目>/JSON/project.json
     模块数: 6
     管道数: 9
```

### 5.2 GUI 确认

打开 GUI 让用户确认扫描结果：
```bash
python codeGen/gui_editor.py
```

GUI 中：
- 左侧模块列表 → 查看各模块管道
- 中间字段详情 → 核对 fields/in_fields/link dims
- 右侧 diff → 对比生成 io.h 与实际 io.h

### 5.3 AI 修改 JSON

用户在 GUI 中确认后，AI 直接编辑 `JSON/project.json`：
- 添加/删除管道
- 修改 fields 顺序/类型
- 调整 link style/dims

**不需要生成代码**，只维护 JSON。

### 5.4 重新生成

JSON 修改完成后：
```bash
# 从 JSON 生成所有代码 (覆盖 include_io/ 和 src/)
python codeGen/code_gen.py gen --config <项目>/JSON/project.json --output <项目>/src
```

### 5.5 临时提交 vs 正式提交

| 类型 | 命令 | 说明 |
|------|------|------|
| 临时保存 | `git commit --no-verify -m "TEMP: 扫描确认 JSON 结构"` | 跳过检查，仅保存进度 |
| 正式提交 | `git commit -m "feat: xxx"` | 运行 pre-commit 检查 |

**标记规范**: 提交信息开头标注版本类型
- `TEMP:` — 临时保存，未完成的工作
- `WIP:` — 工作进行中
- 无前缀 — 正式提交，通过检查

---

## /update-json — 更新 JSON 工作流

触发条件: 用户说"更新 JSON"、"JSON 没同步"、"程序改了 JSON 没改"。

```
1. python codeGen/scan_project.py <项目路径>
2. python codeGen/gui_editor.py (用户确认)
3. AI 编辑 JSON/project.json
4. python codeGen/code_gen.py gen --config <项目>/JSON/project.json --output <项目>/src
5. 验证: 对比生成文件与实际文件
6. 正式提交
```
