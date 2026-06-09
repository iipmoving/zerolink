---
name: check
description: "Run all four methodology compliance checks on the current project (v2.0 Data Switcher). Use after every code change to verify layer dependencies, weak pairs, struct consistency, and compilation. Triggers on: check, 检查, 验证, audit, verify, 审计, 合规检查."
user-invocable: true
---

# /check — 一键四件套合规审计 (four_head v2.0)

Run all methodology checks and report pass/fail with violation locations.

**Project**: four_head (SC32F1XXX, Cortex-M0+, armcc)
**Methodology**: v2.0 — Data Switcher 中间层架构
**Tools**: `.trae/tools/`

---

## The Job

Execute four checks sequentially. **Do NOT stop on first failure** — run all four and produce a full report.

---

## Step 1: Layer Dependency Audit

```bash
python .trae/tools/check_deps.py . --project four-head --v2
```

**v2.0 新增检查项**:
- 验证 `_io.h` 文件仅被 `data_switcher.c` 全路径 include
- 验证其他模块的 `.h` 文件使用 `//#define` 屏蔽（L0 阻断）
- 验证 `data_switcher.c` 正确 include 所有 `_io.h` 接口

Parse output. Non-zero exit = `[FAIL]`. Report violations with file:line.

---

## Step 2: Weak Pair Consistency

```bash
python .trae/tools/check_weak_pairs.py . --project four-head --v2
```

**v2.0 新增检查项**:
- 验证 Switcher 模式的 `_OnInput` 回调配对
- 验证 `data_switcher.c` 中的 WEAK 声明与接收方 STRONG 符号匹配
- 验证 status bit 协议一致性

Report orphan WEAK / orphan strong symbols with file:line.

---

## Step 3: Struct Consistency

If `cfg/structs.json` does not exist, report `[SKIP]` and continue.

```bash
python .trae/tools/check_structs.py . --project four-head --v2
```

**v2.0 新增检查项**:
- 验证 `_io.h` 中输出结构体的 status 字段位置
- 验证 Producer/Consumer 结构体的 sizeof/offsetof 一致性
- 验证 Switcher 路由的结构体传递类型安全

Report schema errors, serial regressions, deprecated field refs, and diff mismatches.

---

## Step 4: Compile Check

Compile all modified `.c` files.

```bash
armcc -c --cpu Cortex-M0plus --c99 \
  -I"src/FWLib/SC32F1XXX_Lib/inc" \
  -I"src/CMSIS" \
  -I"include" \
  -I"src/app" \
  -I"src/drv" \
  -I"src/hal" \
  -I"src/proto" \
  -I"src/core" \
  <modified.c> -o <output.o>
```

**If no compiler available**, report `[SKIP]` and continue.

---

## Final Report

```
========================================
  CHECK SUMMARY — four_head v2.0
========================================
  1/4 check_deps.py       [PASS]
    - Layer rules: OK
    - _io.h restrictions: OK
    - L0 blocking: OK
  2/4 check_weak_pairs.py [PASS]
    - __weak pairs: OK
    - Switcher callbacks: OK
  3/4 check_structs.py    [PASS]
    - Struct consistency: OK
    - _io.h structs: OK
  4/4 compile             [PASS/SKIP]
----------------------------------------
  Result: 0 violations — CLEAN
  Methodology: v2.0 Data Switcher
========================================
```

Report `[PASS]`, `[FAIL]` (with count + locations), or `[SKIP]` (with reason) for each step.

---

## Quick Mode

`--quick` or "快速检查" → skip compile step (4/4), only checks 1-3.

---

## v2.0 Specific Checks

| Check | Description |
|-------|-------------|
| **_io.h Validation** | 确认 `_io.h` 文件保留 `#define`，其他 `.h` 使用 `//#define` |
| **Switcher Routing** | 验证 `data_switcher.c` 正确 include 所有 producer 的 `_io.h` |
| **Status Bit Protocol** | 验证 Producer 正确设置 bit1，Consumer 通过 `_OnInput` 接收 |
| **Struct Direct Pass** | 验证回调使用结构体直接传参，而非 `void*` |
| **Constructor Pattern** | 验证构造函数标准范式 `Module_Construct()` |