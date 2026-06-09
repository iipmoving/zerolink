---
name: check
description: "Run all methodology compliance checks — layer deps, message channels, paradigm audit, coding conventions, compile. Use after every code change. Triggers on: check, 检查, 验证, audit, verify, 审计, 合规检查."
---

# /check — 一键合规审计

Run all methodology checks and report pass/fail with violation locations.

**核心**: `core/std_module.h` — PULL 范式

---

## 工作目录约定

在项目根目录执行。

---

## 检查序列

执行 6 项检查。**不要在第一项失败时停止** — 全部跑完再出报告。

### Step 1: 层依赖审计

```bash
python tools/check_deps.py .
```

检查所有 `#include`，对照层白名单。

| 违规 | 检测 |
|------|------|
| `app/*.c` 包含 `drv/` 或 `hal/` | FAIL |
| `drv/*.c` 包含 `app/` | FAIL |
| `hal/*.c` 包含 `core/` `app/` `drv/` `proto/` | FAIL |
| `proto/*.c` 包含 `app/` `drv/` `hal/` | FAIL |

### Step 2: 范式合规审计

```bash
python tools/check_paradigm.py .
```

检查每个模块是否正确使用 v2.2 PULL 范式：

| 检查项 | 违规 |
|--------|------|
| MODULE_SKELETON 带 name 参数 | 空 `MODULE_SKELETON()` |
| MODULE_EXPORT 与 SKELETON 配对 | 有 SKELETON 无 EXPORT, 或反过来 |
| 手动 GetIO 实现 | 自己写 `XXX_GetIO()` 而不是 `MODULE_EXPORT(XXX)` |
| .h 手动声明 GetIO | 手写 `void Xxx_GetIO(...)` 而不是 `MODULE_IO_H(Xxx)` |

### Step 3: 输出回调审计

```bash
python tools/check_output_callback.py .
```

检查 v1.x 遗留模式：

| 检查项 | 违规 |
|--------|------|
| `_onOutput` 标识符 | 需 `@OUTPUT_CALLBACK` 白名单 |
| `__weak OnOutput` 定义 | 不应存在 |
| `void*` 回调 (在 MODULE_SKELETON 文件) | 需 `@V1_VOIDPTR` 白名单 |
| APP→DRV `__weak` 传结构体指针 | 禁止 |
| `__weak` 函数名不以 Callback 结尾 | 需重命名或 `@ALLOW_NON_CALLBACK_WEAK` |

### Step 4: 消息通道一致性（如有消息系统）

```bash
python tools/check_msgs.py .
```

验证每条 Msg_Post/Send/Register 成对存在，ID 不冲突。无消息系统的项目跳过。

### Step 5: 结构体一致性（如 cfg/structs.json 存在）

```bash
python tools/check_structs.py .
```

如 `cfg/structs.json` 不存在则报告 `[SKIP]`。

### Step 6: 编译验证

```bash
# 用项目对应的编译器命令
armcc --cpu Cortex-M4 --c99 -I"include" -I"core" -c <modified.c>
```

如无编译器则 `[SKIP]`。

---

## 输出格式

```
========================================
  CHECK SUMMARY — m4_ekf_observer
========================================
  1/6 check_deps.py          [PASS]      0 violations
  2/6 check_paradigm.py      [PASS]      0 violations
  3/6 check_output_callback  [PASS]      0 violations
  4/6 check_msgs.py          [SKIP]      No msg system
  5/6 check_structs.py       [PASS]      0 violations
  6/6 compile                [PASS]      0 errors, 0 warnings
----------------------------------------
  Result: 0 violations — CLEAN
========================================
```

---

## 编码基本规范

- [ ] 所有文件为 **UTF-8 without BOM**？
- [ ] 无裸 `__weak` — 全部使用 `__attribute__((weak))`？
- [ ] 每个模块有 `MODULE_SKELETON(name)` + `MODULE_EXPORT(name)`？
- [ ] 跨模块 struct 有 `#pragma pack(4)` + `res[]` 填充到 4 倍数？
- [ ] APP 层不 include DRV/HAL？
- [ ] 模块通过 InputCallback 拉数据，不用 __weak 输出给别的 APP 模块？
- [ ] 所有 `__attribute__((weak))` 函数以 `Callback` 后缀结尾？

## Quick Mode

`--quick` 或 "快速检查" → 跳过 Step 6 编译，只跑 1-5。
