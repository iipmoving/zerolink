---
name: new-module
description: "Interactive wizard to create a new module following the zero-coupling methodology. Guides through layer selection, header generation, three-phase implementation, struct registration, and weak pair setup. Triggers on: new module, add module, create module, new-module, 新增模块, 新建模块, 添加模块."
user-invocable: true
---

# /new-module — 交互式模块创建向导 (m4_ekf_observer)

Creates a new C module following the zero-coupling methodology SOP.

**Project**: m4_ekf_observer (RX32G410, Cortex-M4F, armcc)
**Layers**: `app` | `base_class` | `proto` | `core`

---

## Step 1: Module Identity

Ask:

```
1. Module name? (snake_case, e.g., "pot_detect", "power_ramp")
2. Layer? (app | base_class | proto | core)
```

Show layer rules:

| Layer | Allowed includes | Forbidden | Include guard |
|-------|-----------------|-----------|---------------|
| `app` | `app/`, `core/`, `proto/`, `<std>` | `base_class/` | `//#define` |
| `base_class` | `base_class/`, `core/`, `<std>` | `app/`, `proto/` | `//#define` |
| `proto` | `proto/`, `core/`, `<std>` | `app/`, `base_class/` | `//#define` |
| `core` | `core/`, `<std>` | all others | `//#define` |

---

## Step 2: Generate .h

Create `{layer}/{module}.h`:

```c
/**
 * @file    {module}.h
 * @brief   {description}
 * @layer   {layer}
 * @deps    {allowed includes}
 */

#ifndef {MODULE}_H
//#define {MODULE}_H       /* L0 compiler block */


/* ---- public types ---- */


/* ---- public interface ---- */

void {Module}_Run(void);


#endif /* {MODULE}_H */
```

---

## Step 3: Generate .c

Create `{layer}/{module}.c` with three-phase skeleton:

```c
/**
 * @file    {module}.c
 * @brief   {description}
 * @layer   {layer}
 */

#include "{module}.h"

/* ====== __weak stubs (what this module NEEDS) ====== */

__weak void Consumer_OnResult(uint16_t param, void *data) {}

/* ====== strong symbols (what this module PROVIDES) ====== */

/* ====== module state ====== */

/* ====== Module_Run ====== */

void {Module}_Run(void)
{
    static uint8_t _init = 0;
    if (!_init) { _init = 1; /* init */ }

    /* --- INPUT --- */

    /* --- COMPUTE --- */

    /* --- OUTPUT --- */
}
```

---

## Step 4: Cross-Module Structs

If the module produces/consumes cross-module structs → edit `cfg/structs.json` → run:

```bash
python ../methodology-seed/tools/generate_structs.py . --project m4-ekf
```

---

## Step 5: Register __weak Pairs

Update `core/interface_map.h` with pair entries and function signatures.

---

## Step 6: Verify

Run `/check`. All 4 checks must pass.

---

## Communication Selection

| Scenario | Mechanism |
|----------|-----------|
| Same time-slice, ≤1ms latency | `__weak` direct call |
| Cross time-slice, async | `Msg_Post` |
| Pure algorithm, no I/O | `__weak` without `#include` |

## Callback Insertion

- Input/output/algorithm-set callbacks: standard, no confirmation needed
- **Mid-compute ad-hoc callback: MUST ask user before adding**
