---
name: check
description: "Run all methodology compliance checks on the current project. Use after every code change to verify layer dependencies, weak pairs, struct consistency, include permissions, and compilation. Triggers on: check, 检查, 验证, audit, verify, 审计, 合规检查."
user-invocable: true
---

# /check — 一键合规审计

Run all methodology checks against the current project and report pass/fail with violation locations.

**核心**: `core/std_module.h` — 模块的骨架宏就是范式

---

## The Job

Execute up to five checks sequentially. **Do NOT stop on first failure** — run all applicable checks and produce a full report. Each check must report `[PASS]` or `[FAIL]` with file:line locations on failure. Checks that don't apply to the project report `[SKIP]`.

---

## Step 0: Detect Project

Determine which project we are in:

| Signal | Project | Tools subdir |
|--------|---------|-------------|
| `src/app/` `src/drv/` `src/hal/` exist | four_head | `../methodology-seed-v2.0/tools/` |
| `app/` `base_class/` `src/RX32G410_FW_HAL_V1.3N/` exist | m4_ekf_observer | `../methodology-seed-v2.0/tools/` |
| `methodology-seed-v2.0/tools/` exists in cwd | methodology-seed-v2.0 itself | `tools/` |

If unable to detect, ask the user which project and where the tools are.

---

## Step 1: Layer Dependency Audit

```bash
python <tools_dir>/check_deps.py <project_dir> [--project <name>]
```

- four_head: `python ../methodology-seed-v2.0/tools/check_deps.py . --project four_head`
- m4_ekf_observer: `python ../methodology-seed-v2.0/tools/check_deps.py . --project m4-ekf`

**Report format:**
```
--- 1/5 check_deps.py ---
[PASS] 0 layer violations found
```
or on failure:
```
--- 1/5 check_deps.py ---
[FAIL] 3 violations:
  src/app/cooking.c:5: includes forbidden "drv/power.h"
  src/app/display.c:12: includes forbidden "hal/gpio.h"
  src/drv/key.c:3: includes forbidden "app/cooking.h"
```

Parse the script's stdout/stderr. Non-zero exit = `[FAIL]`.

---

## Step 2: Weak Pair Consistency

```bash
python <tools_dir>/check_weak_pairs.py <project_dir> [--project <name>]
```

- four_head: `python ../methodology-seed-v2.0/tools/check_weak_pairs.py . --project four_head`
- m4_ekf_observer: `python ../methodology-seed-v2.0/tools/check_weak_pairs.py . --project m4-ekf`

**Report format:**
```
--- 2/5 check_weak_pairs.py ---
[PASS] 0 orphan pairs found (N pairs registered)
```
or on failure:
```
--- 2/5 check_weak_pairs.py ---
[FAIL] 5 violations:
  app/power.c: orphan WEAK: __weak void Power_OnSet(...)
  core/interface_map.h: orphan strong: void Display_Commit(...)
```

Parse the script's output. Count violations.

---

## Step 3: Struct Consistency

```bash
python <tools_dir>/check_structs.py <project_dir> [--project <name>]
```

If the project has no `cfg/structs.json`, report `[SKIP] No cfg/structs.json found` and continue.

**Report format:**
```
--- 3/5 check_structs.py ---
[PASS] All types.h match structs.json (serial=N)
```
or on failure:
```
--- 3/5 check_structs.py ---
[FAIL] 2 issues:
  [serial] display_module/types.h: JSON serial=2 < file serial=5
  [deprecated] structs["State"].consumers["timer"]: references deprecated field "old_mode"
```

---

## Step 4: Include Permission Audit (Data Switcher)

Detect whether the project uses Data Switcher:

| Signal | Meaning |
|--------|---------|
| `include/` directory exists with `*_io.h` files | Project uses Data Switcher |
| `data_switcher.c` or `Switcher.c` exists in source tree | Project uses Data Switcher |
| Neither exists | Skip this check |

If Switcher detected:
```bash
python <tools_dir>/check_include.py <project_dir>
```

**Report format:**
```
--- 4/5 check_include.py ---
[PASS] 0 unauthorized _io.h includes found
```
or on failure:
```
--- 4/5 check_include.py ---
[FAIL] 2 violations:
  app/power.c:3: unauthorized #include "../include/adc_io.h" (only Switcher.c allowed)
  drv/display.c:7: unauthorized #include "../include/key_io.h"
```

If no Switcher detected:
```
--- 4/5 check_include.py ---
[SKIP] No include/ *_io.h or data_switcher.c found
```

---

## Step 5: Compile Check

Detect compiler and run syntax check on all modified `.c` files.

**For four_head (armcc):**
```bash
cd Project/ && armcc -c --cpu Cortex-M0+ -DSC32L14xx --c99 \
  -I "../src/vendor/FWLib/SC32F1XXX_Lib/inc" -I "../src/vendor/CMSIS" \
  -I "../src/vendor/MCU_Drivers" \
  -I "../src/app" -I "../src" -I "../src/hal" \
  -I "../src/drv" -I "../src/proto" -I "../src/cfg" \
  <source.c> -o <output.o>
```

**For m4_ekf_observer (armcc/gcc):**
Check `CLAUDE.md` for the project-specific compile command.

**If no compiler available, skip:**
```
--- 5/5 compile ---
[SKIP] armcc not found on PATH
```

**Report format:**
```
--- 5/5 compile ---
[PASS] app/cooking.c compiled (0 errors, 0 warnings)
```
or:
```
--- 5/5 compile ---
[FAIL] app/cooking.c:42: error: #20: identifier "PowerCmd_t" is undefined
```

---

## Final Report

After all checks, print a summary:

```
========================================
  CHECK SUMMARY
========================================
  1/5 check_deps.py       [PASS]
  2/5 check_weak_pairs.py [PASS]
  3/5 check_structs.py    [PASS]
  4/5 check_include.py    [SKIP] (no Switcher)
  5/5 compile             [SKIP] (no compiler)
----------------------------------------
  Result: 0 violations — CLEAN
========================================
```

If any check fails:
```
========================================
  CHECK SUMMARY
========================================
  1/5 check_deps.py       [PASS]
  2/5 check_weak_pairs.py [FAIL] 3 orphan pairs
  3/5 check_structs.py    [PASS]
  4/5 check_include.py    [FAIL] 1 violation
  5/5 compile             [FAIL] 2 errors
----------------------------------------
  Result: 6 violations — FIX REQUIRED
========================================
```

**Exit status**: If any check reports `[FAIL]`, exit with error. If only `[PASS]` or `[SKIP]`, success.

---

## Quick Mode

If the user says `--quick` or "快速检查" or "quick check", skip the compile step (5/5) and only run checks 1-4.

---

## 编码基本规范（每次提交前自检）

- [ ] 所有文件为 **UTF-8 without BOM**？
- [ ] 无裸 `__weak` — 全部使用 `__attribute__((weak))`？
- [ ] 4 空格缩进，无 Tab？
- [ ] 每个模块有 `static void _Constructor(void)` 或 `MODULE_SKELETON()`？
- [ ] 跨模块 struct 有 `#pragma pack(4)` + `res[]` 填充到 4 倍数？
- [ ] 函数/变量/常量带模块前缀？（`ModuleName_Func` / `g_` / `MODULE_CONST`）
- [ ] app/drv/proto 的 `.h` 使用 `//#define`（L0 阻断）？
- [ ] `_io.h` 的 `#define` 保留（公开接口）？
- [ ] APP 层不 include DRV/HAL？
- [ ] 在 git reset 前 stash 或建备份分支？
- [ ] 中文注释说明复杂逻辑？

---

## Notes

- The check SKILL does NOT modify any files. It only reports.
- Violations must be fixed by the user/AI before committing.
- If the project has a pre-commit hook, these same checks run automatically on `git commit`.
