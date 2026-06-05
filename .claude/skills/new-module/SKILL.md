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


/* ---- public types (only types used by other modules) ---- */


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


/* ============================================================
 *  __weak stubs — declare what this module NEEDS from others
 * ============================================================ */

__weak void Consumer_OnResult(uint16_t param, void *data) {}
/* add more __weak stubs as needed */


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

If **YES**:
1. Guide the user to edit `cfg/structs.json`:
   - **Producer (owner)**: Add a new struct entry with `owner: "{module}"`, `suffix: "OUT"`, and all fields
   - **Consumer**: Add `{module}` to the existing struct's `consumers` list with the fields it needs

2. Run the struct generator:
   ```bash
   python ../methodology-seed/tools/generate_structs.py . --project {project}
   ```

3. The generated `types.h` will appear in the module's directory — include it in the .c if needed.

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

**Confirm with user** before proceeding to step 5.

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
| 3 | `check_structs.py` | types.h match structs.json |
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
