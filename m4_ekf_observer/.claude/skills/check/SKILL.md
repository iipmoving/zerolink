---
name: check
description: "Run all four methodology compliance checks on the current project. Use after every code change to verify layer dependencies, weak pairs, struct consistency, and compilation. Triggers on: check, 检查, 验证, audit, verify, 审计, 合规检查."
user-invocable: true
---

# /check — 一键四件套合规审计 (m4_ekf_observer)

Run all methodology checks and report pass/fail with violation locations.

**Project**: m4_ekf_observer (RX32G410, Cortex-M4F, armcc)
**Tools**: `../methodology-seed/tools/`

---

## The Job

Execute four checks sequentially. **Do NOT stop on first failure** — run all four and produce a full report.

---

## Step 1: Layer Dependency Audit

```bash
python ../methodology-seed/tools/check_deps.py . --project m4-ekf
```

Parse output. Non-zero exit = `[FAIL]`. Report violations with file:line.

---

## Step 2: Weak Pair Consistency

```bash
python ../methodology-seed/tools/check_weak_pairs.py . --project m4-ekf
```

Report orphan WEAK / orphan strong symbols with file:line.

---

## Step 3: Struct Consistency

If `cfg/structs.json` does not exist, report `[SKIP]` and continue.

```bash
python ../methodology-seed/tools/check_structs.py . --project m4-ekf
```

Report schema errors, serial regressions, deprecated field refs, and diff mismatches.

---

## Step 4: Compile Check

Compile all modified `.c` files. The compile command is in `CLAUDE.md` §编译.

**If no compiler available**, report `[SKIP]` and continue.

---

## Final Report

```
========================================
  CHECK SUMMARY — m4_ekf_observer
========================================
  1/4 check_deps.py       [PASS]
  2/4 check_weak_pairs.py [PASS]
  3/4 check_structs.py    [PASS]
  4/4 compile             [PASS/SKIP]
----------------------------------------
  Result: 0 violations — CLEAN
========================================
```

Report `[PASS]`, `[FAIL]` (with count + locations), or `[SKIP]` (with reason) for each step.

---

## Quick Mode

`--quick` or "快速检查" → skip compile step (4/4), only checks 1-3.
