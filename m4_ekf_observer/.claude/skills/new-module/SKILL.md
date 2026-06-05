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
 * ================================================================ */


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

/* === AI-MANAGED: __weak stubs (what this module NEEDS) ===========
 *
 *  由 /new-module Step 5 自动生成, 与 interface_map.h 配对.
 *  不 include 提供方 .h — 链接器根据 STRONG 符号自动接线.
 * ================================================================ */

__weak void Consumer_OnResult(uint16_t param, void *data) {}

/* === END AI-MANAGED === */

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

### Mode A: structs.json 存在 → JSON 驱动生成

Edit `cfg/structs.json` → run:

```bash
python ../methodology-seed/tools/generate_structs.py . --project m4-ekf
```

### Mode B: 无 structs.json → AI 自动生成消费者副本

**本模式适用于 m4_ekf_observer (float 类型 + 嵌套结构体, structs.json V1.0 不覆盖).**

当新模块需要消费其他模块的结构体时, AI 自动完成:

1. **读取 owner 的 .h 文件** → 找到 `INTERFACE STRUCTS` 段 → 提取目标 `@STRUCT` 定义
2. **生成消费者副本** → 写入新模块 .h 的 `INTERFACE STRUCTS` 段:
   - 类型名: `{NewModule}_{StructKey}_IN_t` (如 `AppAdc_IH_ElecResult_IN_t`)
   - `source=` 指向 owner 结构体名 (如 `source=IH_ElecResult`)
   - `owner=<新模块名>`, `suffix=IN`
   - 每条字段注释 `/* offset=N, size=N */` — 与 owner 完全一致
3. **自动计算嵌套结构体链** — 如 `IH_ElecInputDef` 嵌套了 `IH_CycleDataDef`, `IH_CycleDataDef` 嵌套了 `IH_HrtimState`, AI 递归生成所有层级的消费者副本
4. **更新 `core/interface_map.h`** — 注册结构体配对 (owner → consumer)

**约束**: `check_structs.py` 解析所有 `@STRUCT` 标记, 自动验证 consumer 的 offset/type/sizeof 与 owner 一致 (含嵌套结构体的 source 链匹配).

**AI 自检**: 生成后立即运行 `/check` — `check_structs.py` 必须 PASS.

---

## Step 5: Register __weak Pairs

Update `core/interface_map.h` with pair entries and function signatures.

### 算法集调用模式 (caller → algorithm)

```
调用方: {caller}.c       WEAK void Algo_Func(void *in, void *out) {}
提供方: {algo}.c                void Algo_Func(void *in, void *out)
```

- 调用方声明 `__weak` 空壳, **不 include** 算法集 .h
- 算法集提供 STRONG 符号, `void*` 内部强制转换
- interface_map.h 注册配对, check_weak_pairs.py 验证

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
