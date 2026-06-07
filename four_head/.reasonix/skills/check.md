---
name: check
description: "Run all methodology compliance checks on four_head project — layer deps, message channels, coding conventions, compile. Use after every code change. Triggers on: check, 检查, 验证, audit, verify, 审计, 合规检查."
---

# /check — 一键合规审计 (four_head)

Run all methodology checks and report pass/fail with violation locations.

**核心**: `core/std_module.h`

**Project**: four_head (SC32L14T, Cortex-M0+, ARMCC V5.06)
**Methodology**: 零耦合嵌入式架构 v2.0

---

## 工作目录约定

所有命令在 `four_head/` 根目录执行。

---

## 检查序列

执行 5 项检查。**不要在第一项失败时停止** — 全部跑完再出报告。

### Step 1: 层依赖审计

```bash
python tools/check_deps.py src
```

检查 `src/` 下所有 `#include`，对照层白名单。

| 违规 | 检测 |
|------|------|
| `app/*.c` 包含 `drv/` 或 `hal/` | FAIL |
| `drv/*.c` 包含 `app/` | FAIL |
| `hal/*.c` 包含 `core/` `app/` `drv/` `proto/` | FAIL |
| `proto/*.c` 包含 `app/` `drv/` `hal/` | FAIL |

### Step 2: 消息通道一致性

```bash
python tools/check_msgs.py src
```

验证每条 Msg_Post/Send/Register 成对存在，ID 不冲突。

### Step 3: 编码规范审计

逐文件检查以下项，报告 `[WARN]` 位置：

| 检查项 | 方法 | 违规示例 |
|--------|------|---------|
| 裸 `__weak` 关键字 | `grep -n "__weak "` | `__weak void Foo(void)` → 应 `__attribute__((weak))` |
| __weak 输出回调审计 | `python tools/check_deps.py src` （自动审计） | APP→APP 违规 / 无强符号 / _OnOutput 空壳 |
| 缺 _Constructor | `grep "_Constructor" {module}.c` | 每个模块必须有一处 |
| include guard 未注释 | 检查 app/drv/proto .h | `#define` 应改为 `//#define` |
| 跨模块 struct 无 pack(4) | grep `typedef struct` 前 pack(4) | struct 未对齐 |
| 文件编码 | `file --mime-encoding {file}` | 非 UTF-8 → WARN |

APP→APP 方向合法例外：无。所有 APP 间通信必须通过 Switcher 路由（g_output → ST_OUT → Switcher → g_input）。DRV→APP 和 APP→DRV 方向可通过 Switcher 的 OnOutput 强符号路由。

报告格式：
```
--- 3/5 编码规范审计 ---
[PASS] 所有文件使用 __attribute__((weak))
[WARN] src/app/app_hmi.h: 12 — 改用 //#define 注释 include guard
[PASS] 所有模块有 _Constructor()
[PASS] 跨模块 struct 有 #pragma pack(4)
```

### Step 4: 结构体一致性（如 cfg/structs.json 存在）

```bash
python ../methodology-seed-v2.0/tools/check_structs.py .
```

如 `cfg/structs.json` 不存在则报告 `[SKIP]`。

### Step 5: 编译验证

从 `Project/` 目录执行：

```bash
cd Project
armcc -c --cpu Cortex-M0+ -DSC32L14xx --c99 \
  -I "../src/vendor/FWLib/SC32F1XXX_Lib/inc" \
  -I "../src/vendor/CMSIS" \
  -I "../src/vendor/MCU_Drivers" \
  -I "../src/app" -I "../src" -I "../src/hal" \
  -I "../src/drv" -I "../src/proto" -I "../src/cfg" \
  <modified.c> -o <output.o>
```

如无编译器则 `[SKIP]`。

---

## 输出格式

```
========================================
  CHECK SUMMARY — four_head
========================================
  1/5 check_deps.py          [PASS]      0 violations
  2/5 check_msgs.py          [PASS]      0 orphan msgs
  3/5 编码规范审计             [PASS]      0 warnings
  4/5 check_structs.py       [SKIP]      No cfg/structs.json
  5/5 compile                [PASS]      0 errors, 0 warnings
----------------------------------------
  Result: 0 violations — CLEAN
========================================
```

---

## 编码基本规范（每次提交前自检）

- [ ] 所有文件为 **UTF-8 without BOM**？
- [ ] 无裸 `__weak` — 全部使用 `__attribute__((weak))`？
- [ ] 4 空格缩进，无 Tab？
- [ ] 每个模块有 `static void _Constructor(void)`？
- [ ] 跨模块 struct 有 `#pragma pack(4)` + `res[]` 填充到 4 倍数？
- [ ] 函数/变量/常量带模块前缀？
- [ ] app/drv/proto 的 `.h` 使用 `//#define`（L0 阻断）？
- [ ] APP 层不 include DRV/HAL？
- [ ] 在 git reset 前 stash 或建备份分支？
- [ ] 中文注释说明复杂逻辑？

---

## Quick Mode

`--quick` 或 "快速检查" → 跳过 Step 5 编译，只跑 1-4。
