---
name: new-module
description: "Interactive wizard to create a new module following the zero-coupling methodology. Guides through layer selection, header generation, three-phase implementation, struct registration, and weak pair setup. Triggers on: new module, add module, create module, new-module, 新增模块, 新建模块, 添加模块."
user-invocable: true
---

# /new-module — 交互式模块创建向导

Creates a new C module following the zero-coupling methodology SOP: three-phase skeleton, __weak isolation, include guard discipline, and cross-module struct registration.

**Each step confirms with the user before proceeding to the next.**

---

## Step 1: Module Identity

Ask the user:

```
1. Module name? (snake_case, e.g., "pot_detect", "power_ramp")
2. Which layer does it belong to?
```

Available layers depend on the project:

**four_head**: `app` | `drv` | `hal` | `proto` | `core` | `cfg`
**m4_ekf_observer**: `app` | `base_class` | `proto` | `core`

Show the layer rules for the chosen layer:

| Layer | Allowed includes | Forbidden includes | Include guard |
|-------|-----------------|--------------------|---------------|
| `app` | `app/`, `core/`, `proto/`, `cfg/`, `<std>` | `drv/`, `hal/`, `base_class/` | `//#define` (commented) |
| `drv` | `drv/`, `core/`, `hal/`, `cfg/`, `<std>` | `app/`, `proto/` | `//#define` (commented) |
| `hal` | `hal/`, `<vendor>`, `<std>` | `core/`, `app/`, `drv/`, `proto/`, `cfg/` | `#define` (normal) |
| `proto` | `proto/`, `core/`, `<std>` | `app/`, `drv/`, `hal/`, `cfg/` | `//#define` (commented) |
| `base_class` | `base_class/`, `core/`, `<std>` | `app/`, `proto/` | `//#define` (commented) |
| `core` | `core/`, `<std>` | `app/`, `drv/`, `hal/`, `proto/`, `cfg/` | `//#define` (commented) |

Confirm with user: _"Creating module `{name}` in layer `{layer}`. Continue?"_

---

## Step 2: Generate Header (.h)

Create the file at the correct path:

- four_head: `src/{layer}/{module}.h`
- m4_ekf_observer: `{layer}/{module}.h`

### Template

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

**Rules**:
- APP/DRV/PROTO/core/base_class layers: `//#define` MUST be commented out (L0 compiler block)
- HAL layer: use normal `#define` (not commented)
- `@deps` lists ALL includes used by the .c file — this is the module's dependency declaration
- Only declare types here if they are used by OTHER modules (cross-module structs go through structs.json, not here)

**Confirm with user** before proceeding to step 3.

---

## Step 3: Generate Implementation (.c)

Create `{module}.c` using the three-phase skeleton template:

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
 *  Module_Run — three-phase: INPUT → COMPUTE → OUTPUT
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

**Rules**:
- Three-phase structure MUST be preserved: input → compute → output
- `__weak` stubs go at file top (before `_Run()`), not hidden in the middle
- No `#include` of other app/drv module headers — use `__weak` for cross-module calls
- Static state is private to the .c file
- Lazy init via `static uint8_t _init` — no external `Init()` function

**Confirm with user** before proceeding to step 4.

---

## Step 4: Cross-Module Struct Registration

Ask: _"Does this module produce or consume any cross-module structs?"_

If **NO**: Skip to step 5.

If **YES**, choose mode based on project setup:

### Mode A: structs.json 存在 (four_head 等 JSON 驱动项目)

1. Guide the user to edit `cfg/structs.json`:
   - **Producer (owner)**: Add a new struct entry with `owner: "{module}"`, `suffix: "OUT"`, and all fields
   - **Consumer**: Add `{module}` to the existing struct's `consumers` list with the fields it needs

2. Run the struct generator:
   ```bash
   python ../methodology-seed/tools/generate_structs.py . --project {project}
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

Run the four-piece audit:

```
/check
```

All four checks MUST pass before the module is considered complete:

| # | Check | What it validates |
|---|-------|-------------------|
| 1 | `check_deps.py` | No illegal cross-layer includes |
| 2 | `check_weak_pairs.py` | All __weak have strong counterparts |
| 3 | `check_structs.py` | JSON 模式: types.h vs structs.json / .h 模式: consumer vs owner @STRUCT 一致性 |
| 4 | Compile | 0 errors, 0 warnings |

If any check fails, fix the violations and re-run.

**Final confirmation**: All checks green → module is methodology-compliant and ready for integration.

---

## Quick Reference: Communication Selection

When the user asks "how should module A talk to module B?":

```
Same 1ms time-slice, ≤1ms latency needed → __weak direct call
Different time-slice, async notification → Msg_Post
Pure algorithm library, no I/O deps → __weak without #include
```

## Callback Insertion Rules

| Type | Location | Requires confirmation? |
|------|----------|----------------------|
| Input callback | Input section (top) | No — standard |
| Output callback | Output section (bottom) | No — standard |
| Algorithm subset | Compute section (via __weak) | No — standard |
| **Ad-hoc mid-compute callback** | Compute section (middle) | **YES — ask user first** |

**If the user asks to insert a callback mid-computation**: Stop and ask for explicit confirmation before adding it.
