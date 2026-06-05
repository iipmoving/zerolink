---
name: check
description: "Run all four methodology compliance checks on the current project. Use after every code change to verify layer dependencies, weak pairs, struct consistency, and compilation. Triggers on: check, 检查, 验证, audit, verify, 审计, 合规检查."
user-invocable: true
---

# /check — 一键四件套合规审计

Run all methodology checks against the current project and report pass/fail with violation locations.

---

## The Job

Execute four checks sequentially. **Do NOT stop on first failure** — run all four and produce a full report. Each check must report `[PASS]` or `[FAIL]` with file:line locations on failure.

---

## Step 0: Detect Project

Determine which project we are in:

| Signal | Project | Tools subdir |
|--------|---------|-------------|
| `src/app/` `src/drv/` `src/hal/` exist | four_head | `../methodology-seed/tools/` |
| `app/` `base_class/` `src/RX32G410_FW_HAL_V1.3N/` exist | m4_ekf_observer | `../methodology-seed/tools/` |
| `methodology-seed/tools/` exists in cwd | methodology-seed itself | `tools/` |

If unable to detect, ask the user which project and where the tools are.

---

## Step 1: Layer Dependency Audit

```bash
python <tools_dir>/check_deps.py <project_dir> [--project <name>]
```

- four_head: `python ../methodology-seed/tools/check_deps.py . --project four_head`
- m4_ekf_observer: `python ../methodology-seed/tools/check_deps.py . --project m4-ekf`

**Report format:**
```
--- 1/4 check_deps.py ---
[PASS] 0 layer violations found
```
or on failure:
```
--- 1/4 check_deps.py ---
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

- four_head: `python ../methodology-seed/tools/check_weak_pairs.py . --project four_head`
- m4_ekf_observer: `python ../methodology-seed/tools/check_weak_pairs.py . --project m4-ekf`

**Report format:**
```
--- 2/4 check_weak_pairs.py ---
[PASS] 0 orphan pairs found (N pairs registered)
```
or on failure:
```
--- 2/4 check_weak_pairs.py ---
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
--- 3/4 check_structs.py ---
[PASS] All types.h match structs.json (serial=N)
```
or on failure:
```
--- 3/4 check_structs.py ---
[FAIL] 2 issues:
  [serial] display_module/types.h: JSON serial=2 < file serial=5
  [deprecated] structs["State"].consumers["timer"]: references deprecated field "old_mode"
```

---

## Step 4: Compile Check

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
--- 4/4 compile ---
[SKIP] armcc not found on PATH
```

**Report format:**
```
--- 4/4 compile ---
[PASS] app/cooking.c compiled (0 errors, 0 warnings)
```
or:
```
--- 4/4 compile ---
[FAIL] app/cooking.c:42: error: #20: identifier "PowerCmd_t" is undefined
```

---

## Final Report

After all four checks, print a summary:

```
========================================
  CHECK SUMMARY
========================================
  1/4 check_deps.py       [PASS]
  2/4 check_weak_pairs.py [PASS]
  3/4 check_structs.py    [PASS]
  4/4 compile             [SKIP] (no compiler)
----------------------------------------
  Result: 0 violations — CLEAN
========================================
```

If any check fails:
```
========================================
  CHECK SUMMARY
========================================
  1/4 check_deps.py       [PASS]
  2/4 check_weak_pairs.py [FAIL] 3 orphan pairs
  3/4 check_structs.py    [PASS]
  4/4 compile             [FAIL] 2 errors
----------------------------------------
  Result: 5 violations — FIX REQUIRED
========================================
```

**Exit status**: If any check reports `[FAIL]`, exit with error. If only `[PASS]` or `[SKIP]`, success.

---

## Quick Mode

If the user says `--quick` or "快速检查" or "quick check", skip the compile step (4/4) and only run checks 1-3.

---

## Notes

- The check SKILL does NOT modify any files. It only reports.
- Violations must be fixed by the user/AI before committing.
- If the project has a pre-commit hook, these same checks run automatically on `git commit`.
