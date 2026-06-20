---
name: init-project
description: "Initialize a new embedded project with full methodology bindings. Use when creating a new project, setting up a project from scratch, or bootstrapping a new MCU firmware project. Triggers on: init project, create project, new project, bootstrap project, setup project, start project."
user-invocable: true
---

# /init-project — 零耦合方法论新项目初始化

Bootstraps a new embedded C project with the complete methodology constraint system:
folder structure, AGENTS.md, specs, tools, structs.json, interface_map.h, deps_config.json, and pre-commit hooks.

---

## The Job

Guide the user through creating a new embedded project that is **immediately ready** for methodology-compliant coding. After completion, `python ../.Codex/tools/check_deps.py --self-test` passes and the project can accept its first module.

**Do NOT** start implementing modules or business logic. This SKILL only sets up the project scaffold.

---

## Important Paths

The methodology-seed lives at `../methodology-seed-v2.0/` (relative to this project).
Adjust the path based on where the new project is created relative to the cluster root.

| Source | Path |
|--------|------|
| Templates | `../methodology-seed-v2.0/templates/` |
| Tools | `../methodology-seed-v2.0/tools/` |
| Method docs | `../methodology-seed-v2.0/` |

---

## Step 1: Ask Questions (Interactive)

Ask the user these questions with lettered options. Default to option A values if the user skips.

### Q1: Project Name
```
1. What is the project name?
   (kebab-case or snake_case, e.g., "motor_controller", "led_panel")
```

### Q2: Project Directory
```
2. Where should the project be created?
   A. (Recommended) Under the cluster root: ../<project_name>/
   B. Custom absolute path: [please specify]
```

### Q3: MCU Model
```
3. What MCU does this project target?
   A. SC32L14T (Cortex-M0+) — 赛元
   B. RX32G410 (Cortex-M4) — 瑞芯微
   C. STM32F103 (Cortex-M3) — ST
   D. Other: [please specify model + architecture]
```

### Q4: Compiler
```
4. What compiler will the project use?
   A. ARMCC V5.06 (Keil MDK)
   B. GCC (ARM Embedded Toolchain)
   C. Clang
   D. Other: [please specify]
```

### Q5: Layer Directory Names
```
5. The methodology uses these layers by default:
     app/  drv/  hal/  proto/  core/  cfg/
   Customize any layer names? Enter comma-separated overrides, e.g.:
     "drv=driver, hal=hw" — keeps other defaults
   Press Enter for defaults.
```

Record all answers. Derive:
- `PROJECT_NAME`: from Q1
- `PROJECT_DIR`: from Q2 (default: `../<project_name>/`)
- `LAYER_DIRS`: from Q5 (default: `app drv hal proto core cfg`)
- `IS_M4_EKF`: true if Q5 mentions `base_class` instead of `drv`

---

## Step 2: Create Directory Structure

Create the following directories under PROJECT_DIR:

```
PROJECT_DIR/
  app/          # Application layer (business logic)
  drv/          # Driver layer (device wrappers) — or base_class/ for M4 EKF style
  hal/          # Hardware abstraction layer (register ops)
  proto/        # Protocol layer (encode/decode)
  core/         # Core/Message infrastructure (msg_scheduler, data_switcher)
  include/      # Data Switcher IO interfaces (_io.h files) — NOT on compiler -I path
  cfg/          # Configuration (JSON data sources)
  src/          # Entry point (main.c)
  tools/        # Project-local tools
  docs/         # Project documentation
  .Codex/      # Codex-specific config and skills
  .Codex/specs/ # Technical specifications
  .Codex/specs/adr/ # Architecture Decision Records
```

If the user customized layer names, use those instead of defaults.
If `drv/` is renamed to `base_class/`, adjust all subsequent references.

Create an empty `src/main.c` with a minimal skeleton:

```c
/* main.c — {PROJECT_NAME} entry point */

int main(void)
{
    while (1) {
        /* TODO: scheduler tick */
    }
    return 0;
}
```

---

## Step 3: Copy & Fill Templates

From `../methodology-seed-v2.0/templates/`, copy and fill these files:

### 3.1 AGENTS.md → PROJECT_DIR/AGENTS.md

Fill placeholders:
- `{项目名称}` → PROJECT_NAME
- `{一句话描述项目用途}` → Ask user for a one-line project description
- `{占位符}` in hardware/scheduler/compiler sections → leave as-is for user to complete

### 3.2 tech-stack.md → PROJECT_DIR/.Codex/specs/tech-stack.md

Fill:
- Language/Standard → from Q4
- Compiler/IDE → from Q4
- MCU/Architecture/Clock → from Q3
- Peripherals → leave as template (user fills later)
- Libraries → leave as template (user fills later)

### 3.3 code-style.md → PROJECT_DIR/.Codex/specs/code-style.md

No filling needed — the template is already generic. Update the layer list in the "文件结构" section to match the user's LAYER_DIRS.

### 3.4 api-conventions.md → PROJECT_DIR/.Codex/specs/api-conventions.md

No filling needed. The template uses `__weak` callback conventions that work for all projects.

### 3.5 ubiquitous-language.md → PROJECT_DIR/.Codex/specs/ubiquitous-language.md

Only fill the "系统分层术语" section with the user's LAYER_DIRS. Leave domain-specific sections empty (user fills later).

### 3.6 structs.json → PROJECT_DIR/cfg/structs.json

Copy as-is. The template has an `_example_StructKey` that serves as documentation. Set `serial: 0`.

### 3.7 interface_map.h → PROJECT_DIR/cfg/interface_map.h

Copy as-is. The template has documented sections for __weak pair registration and struct pair registration.

### 3.8 std_module.h → PROJECT_DIR/core/std_module.h

Copy from the methodology-seed template or from an existing project (`four_head/src/core/std_module.h`).
This is the **v2.0 module skeleton header** — `MODULE_SKELETON()` + `MODULE_EXPORT()` macros define the module paradigm.
If the template doesn't exist yet, create `core/std_module.h` with the content from `01-architecture.md` §7 appendix.

---

## Step 4: Copy Tools

From `../methodology-seed-v2.0/tools/`, copy these files to `PROJECT_DIR/tools/`:

- `check_deps.py` — Layer dependency auditor
- `check_weak_pairs.py` — __weak callback consistency checker
- `generate_structs.py` — Struct code generator
- `check_structs.py` — Struct consistency verifier
- `check_include.py` — Data Switcher _io.h include permission auditor

Copy as-is — no modifications needed. The tools auto-detect project type from directory structure.

---

## Step 5: Generate deps_config.json

Create `PROJECT_DIR/deps_config.json` with the project's layer rules.

If the user chose `drv/` (four_head pattern):

```json
{
  "project": "{PROJECT_NAME}",
  "layer_dirs": [{comma-quoted layer names}],
  "scan_dirs": [{comma-quoted layer names + "src"}],
  "rules": {
    "app": {
      "allowed": ["app/", "core/", "proto/", "cfg/", "<"],
      "forbidden": ["drv/", "hal/"]
    },
    "drv": {
      "allowed": ["drv/", "core/", "hal/", "cfg/", "<"],
      "forbidden": ["app/", "proto/"]
    },
    "hal": {
      "allowed": ["hal/", "<"],
      "forbidden": ["core/", "app/", "drv/", "proto/", "cfg/"]
    },
    "proto": {
      "allowed": ["proto/", "core/", "<"],
      "forbidden": ["app/", "drv/", "hal/", "cfg/"]
    },
    "core": {
      "allowed": ["core/", "include/", "<"],
      "forbidden": ["app/", "drv/", "hal/", "proto/", "cfg/"]
    },
    "cfg": {
      "allowed": ["cfg/", "app/", "<"],
      "forbidden": ["drv/", "hal/", "proto/", "core/"]
    }
  },
  "weak_pairs": {
    "interface_map": "cfg/interface_map.h",
    "scan_dirs": [{comma-quoted layer names}],
    "public_api_excludes": ["_OnTick", "_OnInit"],
    "callback_name_pattern": "^\\w+_On\\w+$"
  },
  "structs": {
    "structs_source": "cfg/structs.json",
    "module_search_paths": [{comma-quoted layer names with "." as last entry}],
    "type_whitelist": ["uint8_t", "uint16_t", "uint32_t", "int8_t", "int16_t", "int32_t", "char"]
  }
}
```

If the user customized layer names (e.g., `base_class` instead of `drv`), adjust:
- Replace `drv` with the custom name in all rules
- `base_class` rules follow the m4-ekf pattern: allowed `["base_class/", "core/", "<"]`, forbidden `["app/", "proto/"]`
- `core` layer always gets `"include/"` in allowed (for data_switcher.c full-path includes)

**`include/` directory rule**: The `include/` directory holds `_io.h` files. It is NEVER added to the compiler `-I` path. All files use full relative paths: `#include "../include/xxx_io.h"`. Only `data_switcher.c` may include other modules' `_io.h` files.

**Auto-detect the `hal` rules**: if the project has a `hal/` layer, HAL gets `allowed: ["hal/", "<"]` and `forbidden: ["core/", "app/", <drv name>, "proto/", "cfg/"]`.

---

## Step 6: Generate Pre-commit Hook

### 6.1 Windows (.ps1) — `PROJECT_DIR/.git/hooks/pre-commit.ps1`

```powershell
#!/usr/bin/env pwsh
# pre-commit hook — 零耦合方法论检查
# Auto-generated by /init-project for {PROJECT_NAME}

$ErrorActionPreference = "Stop"
$RootDir = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $RootDir

$failures = 0

Write-Host "=== Pre-commit: Methodology Checks ===" -ForegroundColor Cyan

# 1. Layer dependency audit
Write-Host "[1] check_deps.py ... " -NoNewline
$result = & python ../.Codex/tools/check_deps.py ./ 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "FAIL" -ForegroundColor Red
    Write-Host $result
    $failures++
} else {
    Write-Host "PASS" -ForegroundColor Green
}

# 2. __weak pair consistency
if (Test-Path "cfg/interface_map.h") {
    Write-Host "[2] check_weak_pairs.py ... " -NoNewline
    $result = & python ../.Codex/tools/check_weak_pairs.py ./ 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL" -ForegroundColor Red
        Write-Host $result
        $failures++
    } else {
        Write-Host "PASS" -ForegroundColor Green
    }
} else {
    Write-Host "[2] check_weak_pairs.py ... SKIP (no interface_map.h)" -ForegroundColor Yellow
}

# 3. Struct consistency
if (Test-Path "cfg/structs.json") {
    Write-Host "[3] check_structs.py ... " -NoNewline
    $result = & python ../.Codex/tools/check_structs.py ./ 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL" -ForegroundColor Red
        Write-Host $result
        $failures++
    } else {
        Write-Host "PASS" -ForegroundColor Green
    }
} else {
    Write-Host "[3] check_structs.py ... SKIP (no structs.json)" -ForegroundColor Yellow
}

# 4. Data Switcher include permission (conditional)
if ((Test-Path "include/") -and (Get-ChildItem "include/" -Filter "*_io.h" -Recurse)) {
    Write-Host "[4] check_include.py ... " -NoNewline
    $result = & python ../.Codex/tools/check_include.py ./ 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL" -ForegroundColor Red
        Write-Host $result
        $failures++
    } else {
        Write-Host "PASS" -ForegroundColor Green
    }
} else {
    Write-Host "[4] check_include.py ... SKIP (no Data Switcher)" -ForegroundColor Yellow
}

Write-Host "======================================" -ForegroundColor Cyan
if ($failures -gt 0) {
    Write-Host "BLOCKED: $failures check(s) failed. Fix violations before commit." -ForegroundColor Red
    exit 1
} else {
    Write-Host "OK: All methodology checks passed." -ForegroundColor Green
    exit 0
}
```

### 6.2 Unix (.sh) — `PROJECT_DIR/.git/hooks/pre-commit`

```bash
#!/bin/bash
# pre-commit hook — 零耦合方法论检查
# Auto-generated by /init-project for {PROJECT_NAME}

set -euo pipefail
ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

FAILURES=0

echo -e "\033[36m=== Pre-commit: Methodology Checks ===\033[0m"

# 1. Layer dependency audit
echo -n "[1] check_deps.py ... "
if python ../.Codex/tools/check_deps.py ./ 2>&1; then
    echo -e "\033[32mPASS\033[0m"
else
    echo -e "\033[31mFAIL\033[0m"
    ((FAILURES++))
fi

# 2. __weak pair consistency
if [ -f "cfg/interface_map.h" ]; then
    echo -n "[2] check_weak_pairs.py ... "
    if python ../.Codex/tools/check_weak_pairs.py ./ 2>&1; then
        echo -e "\033[32mPASS\033[0m"
    else
        echo -e "\033[31mFAIL\033[0m"
        ((FAILURES++))
    fi
else
    echo -e "[2] check_weak_pairs.py ... \033[33mSKIP (no interface_map.h)\033[0m"
fi

# 3. Struct consistency
if [ -f "cfg/structs.json" ]; then
    echo -n "[3] check_structs.py ... "
    if python ../.Codex/tools/check_structs.py ./ 2>&1; then
        echo -e "\033[32mPASS\033[0m"
    else
        echo -e "\033[31mFAIL\033[0m"
        ((FAILURES++))
    fi
else
    echo -e "[3] check_structs.py ... \033[33mSKIP (no structs.json)\033[0m"
fi

# 4. Data Switcher include permission (conditional)
if [ -d "include/" ] && ls include/*_io.h >/dev/null 2>&1; then
    echo -n "[4] check_include.py ... "
    if python ../.Codex/tools/check_include.py ./ 2>&1; then
        echo -e "\033[32mPASS\033[0m"
    else
        echo -e "\033[31mFAIL\033[0m"
        ((FAILURES++))
    fi
else
    echo -e "[4] check_include.py ... \033[33mSKIP (no Data Switcher)\033[0m"
fi

echo -e "\033[36m======================================\033[0m"
if [ $FAILURES -gt 0 ]; then
    echo -e "\033[31mBLOCKED: $FAILURES check(s) failed. Fix violations before commit.\033[0m"
    exit 1
else
    echo -e "\033[32mOK: All methodology checks passed.\033[0m"
    exit 0
fi
```

**Important**: Make pre-commit hooks executable:
- Unix: `chmod +x .git/hooks/pre-commit`
- Windows: no action needed (pwsh runs .ps1 natively)

**Note**: If the project does not yet have its own `.git/`, inform the user that pre-commit hooks should be moved into the project's git repo when initialized.

---

## Step 7: Verify Toolchain

After all files are in place, run the self-test suite:

```bash
cd PROJECT_DIR
python ../.Codex/tools/check_deps.py --self-test
```

Expected: `SELF_TEST: N/N PASSED` (all tests pass).

If --self-test fails:
1. Verify Python 3 is available: `python --version`
2. Verify deps_config.json is valid JSON
3. Show the error output to the user

If check_deps --self-test passes, also verify generate_structs is functional:

```bash
python ../.Codex/tools/generate_structs.py --check
```

Expected: `OK: ... struct(s) up-to-date with structs.json` (0 diff).

---

## Step 8: Summary Output

Print a summary like this:

```
=== /init-project Complete ===

Project:   {PROJECT_NAME}
Location:  {PROJECT_DIR}
MCU:       {MCU}
Compiler:  {COMPILER}
Layers:    {layer_dirs}

Created:
  Directory structure     ✓  ({count} dirs)
  AGENTS.md               ✓
  .Codex/specs/*.md      ✓  (4 files)
  cfg/structs.json        ✓
  cfg/interface_map.h     ✓
  core/std_module.h       ✓
  deps_config.json        ✓
  tools/check_*.py        ✓  (in `.Codex/tools/`)
  .git/hooks/pre-commit   ✓  (.ps1 + .sh)
  src/main.c (skeleton)   ✓

Toolchain self-test:      PASS

Next steps:
  1. Fill .Codex/specs/tech-stack.md with actual hardware details
  2. Fill .Codex/specs/ubiquitous-language.md with domain terms
  3. Define cfg/structs.json with your first cross-module struct
  4. Create your first module: app/{module_name}.h + .c
  5. Register __weak pairs in cfg/interface_map.h
  6. Run: python ../.Codex/tools/check_deps.py ./ to verify
```

---

## Checklist

Before declaring completion:

- [ ] All 5 questions answered and recorded
- [ ] Directory structure created (9+ dirs)
- [ ] AGENTS.md copied and filled with project name
- [ ] 4 spec files copied to .Codex/specs/
- [ ] structs.json copied to cfg/ (serial: 0)
- [ ] data-contract.md created as docs/data-contract.md (template: methodology-seed/10-data-contract.md)
- [ ] interface_map.h copied to cfg/
- [ ] core/std_module.h copied to core/
- [ ] 5 tools copied to tools/ (check_deps, check_weak_pairs, check_structs, check_include, generate_structs)
- [ ] deps_config.json generated with correct layer rules
- [ ] Pre-commit hooks generated (.ps1 + .sh)
- [ ] `check_deps.py --self-test` passes
- [ ] `generate_structs.py --check` passes (expects up-to-date, which a fresh project is)
- [ ] Summary printed to user
