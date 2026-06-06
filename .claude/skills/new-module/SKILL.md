---
name: new-module
description: "Interactive wizard to create a new module following the zero-coupling methodology. Guides through layer selection, header generation, three-phase implementation, struct registration, and weak pair setup. Triggers on: new module, add module, create module, new-module, 新增模块, 新建模块, 添加模块."
user-invocable: true
---

# /new-module — 交互式模块创建向导

Creates a new C module following the zero-coupling methodology SOP: three-phase skeleton, __weak isolation, include guard discipline, and cross-module struct registration. Supports both **traditional __weak mode** and **Data Switcher v2.0 mode**.

**Each step confirms with the user before proceeding to the next.**

---

## Step 1: Module Identity

Ask the user:

```
1. Module name? (snake_case, e.g., "pot_detect", "power_ramp")
2. Which layer does it belong to?
3. Data Switcher mode? (y/n)
   - YES: module participates in Data Switcher (Input_t/Output_t, DoWork, GetIO, _io.h)
   - NO:  traditional __weak/Msg_Post mode (Run(), module.h)
```

Available layers depend on the project:

**four_head**: `app` | `drv` | `hal` | `proto` | `core` | `cfg`
**m4_ekf_observer**: `app` | `base_class` | `proto` | `core`

Show the layer rules for the chosen layer:

| Layer | Allowed includes | Forbidden includes | Include guard (non-Switcher) |
|-------|-----------------|--------------------|------------------------------|
| `app` | `app/`, `core/`, `proto/`, `cfg/`, `<std>` | `drv/`, `hal/`, `base_class/` | `//#define` (commented) |
| `drv` | `drv/`, `core/`, `hal/`, `cfg/`, `<std>` | `app/`, `proto/` | `//#define` (commented) |
| `hal` | `hal/`, `<vendor>`, `<std>` | `core/`, `app/`, `drv/`, `proto/`, `cfg/` | `#define` (normal) |
| `proto` | `proto/`, `core/`, `<std>` | `app/`, `drv/`, `hal/`, `cfg/` | `//#define` (commented) |
| `base_class` | `base_class/`, `core/`, `<std>` | `app/`, `proto/` | `//#define` (commented) |
| `core` | `core/`, `<std>` | `app/`, `drv/`, `hal/`, `proto/`, `cfg/` | `//#define` (commented) |

**Data Switcher module 额外规则**:
- `_io.h` 放在 `include/` 目录，**保留 `#define` guard**（公开接口）
- 模块自身 `.h`（如有内部配置常量）仍注释 `//#define`（私有）
- `include/` 不加入编译器 `-I` path — 所有人都用全路径 `#include "../include/xxx_io.h"`
- `_io.h` 放 `Input_t` + `Output_t` + `GetIO()` + `DoWork()` 声明
- 详见 `08-data-switcher.md`

Confirm with user: _"Creating module `{name}` in layer `{layer}`, Switcher mode: {y/n}. Continue?"_

---

## Step 2: Generate Interface File

**If Data Switcher mode: create `include/{module}_io.h`** (project root `include/` directory).

**If non-Switcher mode: create `{layer}/{module}.h`** (traditional module header).

### Template A: Data Switcher `_io.h` (保留 `#define`)

Path: `include/{module}_io.h`

```c
/**
 * @file    {module}_io.h
 * @brief   {module} Data Switcher IO interface
 * @layer   {layer} (Data Switcher IO)
 *
 * 本文件定义 {module} 模块对外暴露的输入/输出接口.
 * 仅 data_switcher.c 可用全路径 include 本文件.
 * 本模块 .c 也通过全路径 include 自己的 _io.h.
 */

#ifndef {MODULE}_IO_H
#define {MODULE}_IO_H       /* ← 保留: _io.h 是公开接口, 两个合法 include 方 */

#include <stdint.h>

/* === INTERFACE STRUCTS ==========================================
 * {module} 的输入/输出槽. status 字节由本模块和 Switcher 共同管理.
 *
 * 协议 (详见 08-data-switcher.md §4.2):
 *   g_in.status  bit0=构造, bit1=新输入到达 (Switcher 设, 本模块消费后清)
 *   g_out.status bit0=保留, bit1=输出就绪  (本模块设, Switcher 取走后清)
 * ================================================================ */

#pragma pack(4)

typedef struct {
    uint8_t  status;        /* bit0=构造, bit1=新输入到达 */
    uint8_t  res[3];        /* 32位对齐 */
    /* TODO: 输入字段 — 由 Switcher 从各 producer Output_t 搬运 */
} {Module}_Input_t;          /* sizeof 须为 4 的倍数 */

typedef struct {
    uint8_t  status;        /* bit0=保留, bit1=输出就绪 */
    uint8_t  res[3];
    /* TODO: 输出字段 — Switcher 搬运给 consumer Input_t */
} {Module}_Output_t;

#pragma pack()

/* ---- public interface ---- */

void {Module}_GetIO({Module}_Input_t **ppIn, {Module}_Output_t **ppOut);
void {Module}_DoWork(void);

#endif /* {MODULE}_IO_H */
```

**Rules for `_io.h`**:
- **`#define` 必须保留** — `_io.h` 是公开接口，本模块 .c + Switcher.c 两个合法 include 方
- `include/` 不加入编译器 `-I` path — 所有人都用全路径 `#include "../include/{module}_io.h"`
- 如果模块是纯生产者 (只有 Output, 没有 Input)，可以省略 `Input_t` 和相关参数
- `check_include.py` 验证: 非 Switcher.c 不能 include 别人的 `_io.h`

### Template B: Traditional module.h (`//#define`)

Path: `{layer}/{module}.h`

```c
/**
 * @file    {module}.h
 * @brief   {one-line description of module responsibility}
 * @layer   {layer}
 * @deps    {list ALL allowed includes this module needs}
 */

#ifndef {MODULE}_H
//#define {MODULE}_H       /* ← commented: L0 compile block on cross-module include */


/* === INTERFACE STRUCTS (OWNER) ==================================
 * 本模块是以下结构体的 Owner (生产者).
 * @STRUCT 标记由 check_structs.py 解析验证.
 * 消费者 AI 生成: 读取本段 → 生成副本 → 写入消费者 AI-MANAGED 段.
 *
 * 格式:
 *   /* @STRUCT StructName  owner={module}  suffix=OUT */
 *   typedef struct {
 *       type field;  /* offset=N, size=N */
 *   } StructName;    /* sizeof=N */
 * ================================================================ */


/* === AI-MANAGED: INTERFACE STRUCTS (CONSUMER) ===================
 *
 *  由 /new-module Step 4 Mode B 自动生成, 不手动编辑.
 *  source= 指向 owner 结构体, check_structs.py 验证一致性.
 *
 *  格式:
 *   /* @STRUCT ConsumerType_IN_t  owner={module}  suffix=IN
 *    * source=OwnerType */
 *   typedef struct { ... } ConsumerType_IN_t;  /* sizeof=N, source=OwnerType */
 * ================================================================ */


/* ---- public interface ---- */

void {Module}_Run(void);


#endif /* {MODULE}_H */
```

**Rules for `module.h`**:
- APP/DRV/PROTO/core/base_class layers: `//#define` MUST be commented out (L0 compiler block)
- HAL layer: use normal `#define` (not commented)
- `@deps` lists ALL includes used by the .c file — this is the module's dependency declaration
- Only declare types here if they are used by OTHER modules (cross-module structs go through structs.json, not here)

**Confirm with user** before proceeding to step 3.

---

## Step 3: Generate Implementation (.c)

Create the implementation file. The template depends on Switcher vs non-Switcher mode.

### Template A: Data Switcher `DoWork()` (status byte protocol)

Path: `{layer}/{module}.c`

```c
/**
 * @file    {module}.c
 * @brief   {one-line description}
 * @layer   {layer} (Data Switcher participant)
 */

#include "../include/{module}_io.h"     /* 全路径 — 自己的 _io.h */
/* 可选: #include "{module}.h"           — 内部配置常量 (如有, 头文件 //#define 注释) */


/* === module state ================================================ */

static {Module}_Input_t  g_in;
static {Module}_Output_t g_out;


/* === internal helpers ============================================ */

static void _Constructor(void)
{
    memset(&g_in,  0, sizeof(g_in));
    memset(&g_out, 0, sizeof(g_out));
    /* TODO: 初始化 g_out 的默认字段值 */
}

/* === public interface ============================================ */

void {Module}_GetIO({Module}_Input_t **ppIn, {Module}_Output_t **ppOut)
{
    *ppIn  = &g_in;
    *ppOut = &g_out;
}

void {Module}_DoWork(void)
{
    /* ---- 构造: lazy init (Switcher 上电清零, 首次触发) ---- */
    if (!(g_in.status & 0x01)) {
        _Constructor();
        g_in.status |= 0x01;
    }

    /* ---- 每帧先清输出标志 ---- */
    g_out.status &= ~0x02;

    /* ---- 输入段: 检查 Switcher 填入的输入 ---- */
    if (!(g_in.status & 0x02)) return;  /* 无新输入, 本帧跳过 */

    /* TODO: 消费 g_in 字段 */

    /* ---- 消费完毕, 清输入标志 ---- */
    g_in.status &= ~0x02;

    /* ---- 计算段 ---- */
    /* TODO: 纯计算逻辑, 不调 I/O */

    /* ---- 输出段 ---- */
    /* TODO: 更新 g_out 字段 */
    g_out.status |= 0x02;              /* "有新输出" */
}
```

**Rules for Switcher module**:
- Three-phase structure MUST be preserved: input (clear out → check in → consume → clear in) → compute → output (set out bit1)
- `g_out.status &= ~0x02` at entry EVERY frame — even if returning early
- `g_out.status |= 0x02` only when output was produced this frame
- Module never touches another module's status byte — only Switcher does
- No `#include` of other modules' `_io.h` — use g_in for all external input
- `_Constructor()` is static — Switcher doesn't call it, module self-detects

### Template B: Traditional `Run()` (__weak/Msg_Post)

Path: `{layer}/{module}.c`

```c
/**
 * @file    {module}.c
 * @brief   {one-line description}
 * @layer   {layer}
 */

#include "{module}.h"
/* allowed layer includes only — see .h @deps */


/* === AI-MANAGED: __weak stubs ====================================
 *
 *  由 /new-module Step 5 自动生成, 与 interface_map.h 配对.
 *  不 include 提供方 .h — 链接器根据 STRONG 符号自动接线.
 *
 *  声明本模块 NEEDS 的外部函数 (空壳).
 *  提供方在 interface_map.h 注册为 STRONG 符号.
 * ================================================================ */

__weak void Consumer_OnResult(uint16_t param, void *data) {}
/* add more __weak stubs as needed */

/* === END AI-MANAGED === */


/* ============================================================
 *  strong symbols — provide what OTHER modules call
 * ============================================================ */


/* ============================================================
 *  module state (static, no external visibility)
 * ============================================================ */


/* ============================================================
 *  internal helpers (static)
 * ============================================================ */


/* ============================================================
 *  {Module}_Run — three-phase: INPUT → COMPUTE → OUTPUT
 * ============================================================ */

void {Module}_Run(void)
{
    /* ---- 构造: lazy init ---- */
    static uint8_t _init = 0;
    if (!_init) {
        _init = 1;
        /* initialize static state here */
    }

    /* ====== 输入段 ====== */
    /* consume __weak callbacks + Msg_Post messages here */


    /* ====== 计算段 ====== */
    /* pure logic — no I/O callbacks, no Msg_Post */


    /* ====== 输出段 ====== */
    /* call consumer weaks + Msg_Post results here */
}
```

**Rules for traditional module**:
- Three-phase structure MUST be preserved: input → compute → output
- `__weak` stubs go at file top (before `_Run()`), not hidden in the middle
- No `#include` of other app/drv module headers — use `__weak` for cross-module calls
- Static state is private to the .c file
- Lazy init via `static uint8_t _init` — no external `Init()` function

**Confirm with user** before proceeding to step 4.

---

## Step 4: Cross-Module Data Flow

The approach depends on Switcher vs non-Switcher mode.

### If Data Switcher Mode

**Does NOT need _LINK struct copies.** 跨模块数据通过 Input_t/Output_t 字段 + Switcher 搬运。

Ask: _"Which modules produce data this module needs?"_

For each producer:
1. Read the producer's `_io.h` → identify `Output_t` fields this module needs
2. Add corresponding fields to this module's `Input_t` in `_io.h`
3. Note the wiring rule: producer `Output_t.field_x` → this module `Input_t.field_y`
4. Update `data_switcher.c` `_Route_*()` — add field copy: `pConsumer_In->field_y = pProducer_Out->field_x`

Ask: _"Which modules consume data this module produces?"_

For each consumer:
1. Inform the user: _"Consumer {X} needs field {Y} from your Output_t. Add to their Input_t."_
2. The consumer's Input_t update and Switcher wiring are handled when that module is created/modified.

**No structs.json entry needed for Switcher-routed data.** The `_io.h` files + Switcher wiring ARE the single source of truth.

### If Non-Switcher Mode (traditional _LINK / structs.json)

### Mode A: structs.json 存在 (four_head 等 JSON 驱动项目)

1. Guide the user to edit `cfg/structs.json`:
   - **Producer (owner)**: Add a new struct entry with `owner: "{module}"`, `suffix: "OUT"`, and all fields
   - **Consumer**: Add `{module}` to the existing struct's `consumers` list with the fields it needs

2. Run the struct generator:
   ```bash
   python ../methodology-seed-v2.0/tools/generate_structs.py . --project {project}
   ```

3. The generated `types.h` will appear in the module's directory.

**Template for structs.json entry** (if this module is the owner):
```json
"{StructKey}": {
  "description": "{purpose}",
  "owner": "{module}",
  "suffix": "OUT",
  "fields": [
    {"name": "field_a", "type": "uint16_t", "note": "{description}"},
    {"name": "field_b", "type": "uint8_t", "note": "{description}"}
  ],
  "consumers": {}
}
```

### Mode B: 无 structs.json — AI 自动生成消费者副本

**适用**: float 类型 / 嵌套结构体 / structs.json V1.0 不覆盖的项目。

**当模块是消费者 (consumer) 时, AI 自动执行**:

1. **问用户**: "需要哪些结构体？从哪个模块获取？"
2. **读取 owner 的 .h** → 定位 `INTERFACE STRUCTS` 段 → 提取目标 `@STRUCT` 定义
3. **生成消费者副本** 写入新模块 .h:
   - 类型名: `{NewModule}_{StructKey}_IN_t`
   - `source=` 指向 owner 结构体名
   - `owner=<新模块>`, `suffix=IN`
   - 每条字段注释 `/* offset=N, size=N */` — 与 owner 完全一致
4. **递归处理嵌套结构体** — owner 结构体嵌套了其他结构体时, 自动生成所有层级的消费者副本
5. **更新 `core/interface_map.h`** — 注册结构体配对

**当模块是生产者 (owner) 时**:
- 在模块 .h 的 `INTERFACE STRUCTS` 段声明 `@STRUCT`, 标注 offset/size
- 消费者由 AI 在后续创建时按上述流程自动生成

**验证**: `check_structs.py` 解析所有 `@STRUCT` 标记, 自动验证 consumer vs owner 一致性 (含嵌套结构体 source 链匹配).

---

## Step 5: Register __weak Pairs

Ask: _"Does this module send data to or receive data from other modules via __weak?"_

If **NO**: Skip to step 6.

If **YES**: Update `core/interface_map.h` (or project-equivalent path) with pair registrations.

**Format**:
```c
/* Pair: {sender} → {receiver} */
/* weak:  __weak void {Receiver}_On{Event}(uint16_t param, void *data) */
/* strong: void {Receiver}_On{Event}(uint16_t param, void *data) in {receiver}.c */
```

**算法集调用模式** (caller → algorithm, 通过 `void*` 解耦):
```
调用方: {caller}.c       WEAK void Algo_Func(void *in, void *out) {}
提供方: {algo}.c                void Algo_Func(void *in, void *out)
```
- 调用方声明 `__weak` 空壳, **不 include** 算法集 .h
- 算法集提供 STRONG 符号, `void*` 内部强制转换

**Rules**:
- One entry per __weak → strong pair
- Include the actual signatures for `check_weak_pairs.py` validation
- Sender declares `__weak`, receiver provides strong symbol
- Both use the same function signature (checked by tooling)

**Confirm with user** before proceeding to step 6.

---

## Step 6: Verify with /check

Run the compliance audit:

```
/check
```

All applicable checks MUST pass before the module is considered complete:

| # | Check | What it validates | Required |
|---|-------|-------------------|----------|
| 1 | `check_deps.py` | No illegal cross-layer includes | Always |
| 2 | `check_weak_pairs.py` | All __weak have strong counterparts | Always |
| 3 | `check_structs.py` | types.h vs structs.json / @STRUCT 一致性 | Always |
| 4 | `check_include.py` | _io.h include permissions (Switcher only) | If include/ exists |
| 5 | Compile | 0 errors, 0 warnings | Always |

If any check fails, fix the violations and re-run.

**Final confirmation**: All checks green → module is methodology-compliant and ready for integration.

---

## Quick Reference: Communication Selection

When the user asks "how should module A talk to module B?":

```
Same time-slice, ≤1ms latency needed → __weak direct call
Different time-slice, async notification   → Msg_Post
Structured data blocks, periodic routing   → Data Switcher (08-data-switcher.md)
Pure algorithm library, no I/O deps       → __weak without #include
```

## Quick Reference: Include Guard Rules

| File type | Guard | Reason |
|-----------|-------|--------|
| `include/*_io.h` | `#define` **保留** | 公开接口, 本模块 .c + Switcher.c 两个合法 include 方 |
| `{layer}/module.h` (APP/DRV/PROTO/core) | `//#define` **注释** | 私有头文件, L0 编译阻断他人 include |
| `hal/*.h` | `#define` 保留 | DRV 层需要引用 HAL 头文件 |

**记忆**: `_io.h` = 故意暴露, `module.h` = 故意隐藏。规则相反, 不要搞反。

## Callback Insertion Rules

| Type | Location | Requires confirmation? |
|------|----------|----------------------|
| Input callback | Input section (top) | No — standard |
| Output callback | Output section (bottom) | No — standard |
| Algorithm subset | Compute section (via __weak) | No — standard |
| **Ad-hoc mid-compute callback** | Compute section (middle) | **YES — ask user first** |

**If the user asks to insert a callback mid-computation**: Stop and ask for explicit confirmation before adding it.

## Data Switcher Module Checklist

When creating a Switcher-mode module, verify:

- [ ] `include/{module}_io.h` created with `#define` guard (NOT commented)
- [ ] `Input_t` and/or `Output_t` defined with status byte (bit0 + bit1)
- [ ] `#pragma pack(4)` wraps all structs, sizeof is 4-byte aligned
- [ ] `{Module}_GetIO()` and `{Module}_DoWork()` declared
- [ ] `{module}.c` includes own `_io.h` via full path: `#include "../include/{module}_io.h"`
- [ ] `_Constructor()` is static, self-triggered by `g_in.status & 0x01` check
- [ ] `DoWork()` clears `g_out.status &= ~0x02` at entry EVERY frame
- [ ] `DoWork()` checks `g_in.status & 0x02` before consuming input
- [ ] `DoWork()` clears `g_in.status &= ~0x02` after consuming
- [ ] `DoWork()` sets `g_out.status |= 0x02` only when output produced
- [ ] Module does NOT include any other module's `_io.h`
- [ ] Switcher wiring added in `data_switcher.c` `_Route_*()`
