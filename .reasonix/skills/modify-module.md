---
name: modify-module
description: "Migrate an existing module to v2.2 PULL paradigm (std_module.h). Step 0: backup → new-module SKILL template → incremental migration. Triggers on: modify module, refactor module, upgrade module, migrate module, 修改模块, 重构模块, 迁移模块, 转换模块."
---

# /modify-module — v1.x → v2.2 迁移向导

将现有模块改造为符合 v2.2 PULL 数据交换机架构（`std_module.h` 模式）。

**核心**: `core/std_module.h` — **模块的骨架宏就是范式**
**范式**: `MODULE_SKELETON(name)` → `Init()` → `ProcessInput()` → `MODULE_EXPORT(name)`

---

## 铁则 (读三遍)

> ⚠️ **禁止直接修改原文件。禁止在原文件加 MODULE_SKELETON。**
>
> 这是本技能唯一不可违反的规则。已因此翻车: 在原文件插骨架宏导致 GetIO/DoWork 混乱、
> 变量名冲突、多次编辑失败。走标准流程。

## Step 0: 改名备份 → new-module 模板 → 逐段搬功能

```bash
# 0a. 改名备份旧文件 (禁止在原文件修改)
mv src/{layer}/{module}.c src/{layer}/{module}_v1.c.bak

# 0b. 如果有旧 .h, 也改名
mv include/{module}.h include/{module}_v1.h.bak

# 0c. 调 new-module SKILL 生成空模板
#     (给出模块名、层、输入数据来源)
#     执行后得到: src/{layer}/{module}.c  (空白模板)
```

### 为什么不能在原文件改？

| 问题 | 后果 |
|------|------|
| 几千行大文件中间插骨架宏 | 宏展开位置不对导致 `g_input`/`g_output` 未定义 |
| 旧 `g_in`/`g_out` 和范式 `g_input`/`g_output` 同名异义 | 变量名极易混淆, 该读 new 时读了 old |
| 旧 Power_DoWork 和 DoWork 共存 | 不知道哪个是入口 |
| 手动 GetIO 和 MODULE_EXPORT 冲突 | 链接错误, 查半天才找到原因 |

正确流程:

```
旧文件 (几千行)          →   old_module_v1.c.bak   (不动)
                           + new_module.c          (空模板, 从零搭)
                           + 从备份逐段搬功能      (搬一段删一段)
                           + 功能等价后删备份
```

### 其他原则

- **每个模块独立改造** — 一次改一个，不改调用方
- **向后兼容靠双写** — 过渡期 g_output.para 与旧 __weak 同时写
- **先搭骨架后填肉** — 先定好 io.h 结构体 + 骨架宏, 再移业务逻辑
- **单向调用** — 模块通过 InputCallback (强符号) 拉数据, 不定义 __weak 输出给别的 APP 模块

---

## Step 0: 备份 + 生成模板

```bash
# 0a. 改名备份旧文件
mv src/{layer}/{module}.c src/{layer}/{module}_v1.c.bak

# 0b. 如果有旧 .h, 也改名
mv src/{layer}/{module}.h src/{layer}/{module}_v1.h.bak

# 0c. 调 new-module SKILL 生成模板
#     (给出模块名、层、输入数据来源)
```

> 不要想"只加几行骨架宏没事" — 已经因此翻过车。走标准流程。

---

## Step 1: 找入口 — 识别模块的所有外部调用点

```bash
# 1a. 找 Init/Run 调用者 (在旧备份文件里搜)
grep -rn "Module_Init\|Module_Run" src/main.c src/app/

# 1b. 找 __weak 输入回调（本模块的强符号 = 别人调我）
grep -n "^void Module_On\|^void App.*_On\|^void Drv.*_On" src/{layer}/{module}_v1.c.bak

# 1c. 找 __weak 输出桩（本模块定义的空壳 = 我调别人）
grep -n "__weak\|__attribute__((weak))" src/{layer}/{module}_v1.c.bak
```

**输出**: 一张调用关系表

| 入口类型 | 函数名 | 谁调我 | 传什么数据 | 对应动作 |
|---------|--------|--------|-----------|---------|
| 周期入口 | `Module_Run()` | main.c slot N | 无参数 | → `DoWork()` |
| 输入回调 | `Module_OnXxx()` | other_module.c | param+data_ptr | → InputCallback 强符号 |
| 输出桩 | `Other_OnYyy()` | (本模块调别人) | param+data_ptr | → 改写 `g_output.para` |

---

## Step 2: 查路径 — 追踪每条 I/O 的数据流

对每个输入回调，追踪：

```
调用方: other_module.c:123
   ↓
传什么: uint16_t param, void *data_ptr
   ↓       param = head_idx (低8位)
   ↓       data_ptr → PowerCtrl_t { onoff, target_power, ... }
   ↓
本模块: Module_OnXxx()
   ↓
存哪里: s_ctx[head_idx].field = ...
```

对每个输出桩，追踪：

```
本模块: send_output()
   ↓
调谁: OtherModule_OnYyy(param, &data)
   ↓
传什么: PowerOutput_t { head_idx, power_watt }
   ↓
接收方: other_module.c — 改走 InputCallback 强符号
```

**输出**: 每路 I/O 的参数字段清单

| I/O 路 | 字段 | 类型 | 范围 | 来源/去向 |
|--------|------|------|------|----------|
| In: PowerCtrl | head_idx | uint8_t | 0-3 | app_cooking |
| | onoff | uint8_t | 0/1 | app_cooking |
| | target_power | uint16_t | 0-2000W | app_cooking |
| Out: PowerCmd | head_idx | uint8_t | 0-3 | → app_comm_mgr |
| | power_watt | uint16_t | 0-2000W | → app_comm_mgr |

---

## Step 3: 设计 Input_t / Output_t 结构体

基于 Step 2 的字段清单, 设计跨模块 I/O 结构体。

**设计规则**:

| 规则 | 说明 |
|------|------|
| `#pragma pack(4)` 包裹 | 32 位对齐 |
| `status` 字节在首部 | `uint8_t status` + `uint8_t res[3]` (与 Info_Header 兼容) |
| 末尾 `resN[]` 补齐 | `sizeof()` 为 4 的倍数 |
| 每个输入源一个子结构体 | PowerCtrl 一组, SystemError 一组, ... |

```c
/* include/{module}_io.h */
#ifndef MODULE_IO_H
#define MODULE_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

typedef struct {
    uint8_t  status;
    uint8_t  res[3];
    /* ... 输入字段 ... */
} {Module}_Input_t;

typedef struct {
    uint8_t  status;
    uint8_t  res[3];
    /* ... 输出字段 ... */
} {Module}_Output_t;

#pragma pack()

/* GetIO 声明 (由 MODULE_EXPORT 生成) */
MODULE_IO_H({Module});

#endif
```

---

## Step 4: 搭骨架 — 在模板 .c 中写入

在 new-module SKILL 生成的空模板 .c 中填入:

### 4.1 引用 + 数据槽

```c
#include "std_module.h"
#include "../include/{module}_io.h"
#include <string.h>

static {Module}_Input_t  s_in;
static {Module}_Output_t s_out;

MODULE_SKELETON({Module});
```

### 4.2 Init

```c
static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
    /* 原有初始化逻辑 */
}
```

### 4.3 ProcessInput — 三段式

```c
static void ProcessInput(void)
{
    /* ====== 输入段 ====== */
    if (g_input.info.status & ST_NEW) {
        {Module}_Input_t *in = ({Module}_Input_t *)g_input.para;
        /* 消费输入 */
        g_input.info.status &= ~ST_NEW;
    }

    /* ====== 计算段 ====== */
    /* 原业务逻辑 */

    /* ====== 输出段 ====== */
    g_output.info.status |= ST_OUT;
}
```

### 4.4 导出

```c
MODULE_EXPORT({Module});
```

---

## Step 5: 填逻辑 — 从备份文件逐段搬功能

| 旧代码位置 | 新位置 | 改动 |
|-----------|--------|------|
| `Module_OnXxx()` 的输入处理 | `ProcessInput()` 输入段 | 从 `g_input.para` 读 |
| `Module_Run()` 的计算 | `ProcessInput()` 计算段 | 原样搬入 |
| `OtherModule_OnYyy()` 输出 | `ProcessInput()` 输出段 → 写 `g_output.para` | 替换为写结构体字段 |

搬一段测试一段, 不要一次搬完。

---

## Step 6: 注册 Switcher

在 `core/data_switcher.c` 中添加:

```c
#include "../include/{module}_io.h"

/* 在 Switcher_Init 中 */
{Module}_GetIO(&s_slot[SLOT_{MODULE}].pIn,
               &s_slot[SLOT_{MODULE}].pOut,
               &s_slot[SLOT_{MODULE}].pDoWork);
```

---

## Step 7: 清理 + 验证

```bash
# 1. 编译通过 (0e0w)
# 2. 层依赖检查
python tools/check_deps.py src
# 3. 范式合规检查
python tools/check_paradigm.py .
# 4. 功能验证通过后, 删备份
rm src/{layer}/{module}_v1.c.bak
```

---

## 检查清单

- [ ] Step 0: 旧文件已改名备份, 不在原文件空改
- [ ] Step 0: 通过 new-module SKILL 生成空模板
- [ ] Step 1-3: I/O 表 + 结构体已设计
- [ ] Step 4: `MODULE_SKELETON(name)` + `Init()` + `ProcessInput()` + `MODULE_EXPORT(name)`
- [ ] Step 4: `{Module}_InputCallback` 用于输入路由（强符号在中间层覆盖）
- [ ] Step 4: `{Module}_OutputCallback` 保留弱符号（仅即时场景用）
- [ ] Step 5: 功能逐段搬入, 搬一段测一段
- [ ] Step 6: Switcher 注册已添加
- [ ] Step 7: 编译 0e0w + check tool 全部通过
