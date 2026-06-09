---
name: modify-module
description: "Convert an existing module from v1.x to v2.2 PULL architecture. 6-step SOP: find entry → trace path → extract I/O → build skeleton → fill logic → wire Switcher. Triggers on: modify module, refactor module, upgrade module, modify-module, 转换模块, 重构模块, 迁移模块."
user-invocable: true
---

# /modify-module — v1.x → v2.2 转换向导

将现有模块改造为符合 v2.2 PULL 数据交换机架构（`std_module.h` 模式）。

**核心**: `core/std_module.h` — **模块的骨架宏就是范式**

---

## 核心原则

- **每个模块独立改造** — 一次改一个，不改调用方
- **多个入口 = 多个 consumer 回调** — 每个外部调用方对应一个 `_On{Producer}Data` 函数
- **向后兼容靠双写** — 过渡期 g_out 与旧 __weak 同时写，最后统一收口
- **先搭骨架后填肉** — 先定好数据结构 + 标准函数，再移业务逻辑
- **输出不推送** — producer 只写 g_output.para，路由由 Switcher 负责

---

## Step 1: 找入口 — 识别模块的所有外部调用点

```bash
# 1a. 找 Init/Run 调用者
grep -rn "Module_Init\|Module_Run" src/main.c src/app/ src/drv/

# 1b. 找 __weak 输入回调（本模块的强符号 = 别人调我）
grep -n "^void Module_On\|^void App.*_On\|^void Drv.*_On" src/{layer}/{module}.c

# 1c. 找 __weak 输出桩（本模块定义的空壳 = 我调别人）
grep -n "__weak\|__attribute__((weak))" src/{layer}/{module}.c
```

**输出**: 一张调用关系表

| 入口类型 | 函数名 | 谁调我 | 传什么数据 | 对应动作 |
|---------|--------|--------|-----------|---------|
| 周期入口 | `Module_Run()` | main.c slot N | 无参数 | → `DoWork()` |
| 输入回调 | `Module_OnXxx()` | other_module.c | param+data_ptr | → consumer 回调 |
| 输出桩 | `Other_OnYyy()` | (本模块调别人) | param+data_ptr | → 改写 `g_out` |

---

## Step 2: 查路径 — 追踪每条 I/O 的数据流

对每个输入回调，追踪：

```
调用方: other_module.c:123
   ↓
传什么: uint16_t param, void *data_ptr
   ↓       param = head_idx (低8位)
   ↓       data_ptr → PowerCtrl_t { onoff, target_power, power_level, ... }
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
接收方: other_module.c — STRONG 强符号
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

## Step 3: 提取输入输出参数 — 设计数据结构

基于 Step 2 的字段清单，定义本模块的输入/输出数据类型。

**设计规则**:

| 规则 | 说明 |
|------|------|
| 每个输入源一个子结构体 | PowerCtrl 一组, SystemError 一组, RegData 一组 |
| 每个子结构体配一个 valid 标志 | `ctrl_valid`, `err_valid`, `reg_valid` — DoWork 靠此判断 |
| `#pragma pack(4)` 包裹 | 32 位对齐 |
| `sizeof()` 为 4 的倍数 | 末尾 `uint8_t resN[N]` 补齐 |

```c
/* 模块内部数据结构 — 挂在 Para_Grp_t.para 后面 */

typedef struct {
    uint8_t  head_idx;
    uint16_t value;
    uint8_t  flag;
} Module_SourceA_Item_t;

typedef struct {
    uint8_t  srcA_valid;
    uint8_t  res[3];
    Module_SourceA_Item_t  srcA;
} Module_InData_t;

typedef struct {
    uint8_t  has_output;
    uint8_t  res[3];
    uint8_t  head_idx;
    uint16_t result;
} Module_OutData_t;
```

---

## Step 4: 搭骨架 — 创建标准函数

在 `.c` 文件中添加以下函数。**先不修改业务逻辑**，只搭框架。

### 4.1 引用 + 数据槽

```c
#include "core/std_module.h"
#include "{module}.h"         /* 可选 — 私有常量 */
#include <string.h>

static Module_InData_t  s_in;
static Module_OutData_t s_out;

MODULE_SKELETON();
```

### 4.2 Init

```c
static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
    /* ... 原有初始化逻辑移到这里 ... */
}
```

### 4.3 Consumer 回调 — 每路一个

```c
/* Switcher 在 Producer DoWork 后显式调用 */
void {Module}_On{Producer}Data(Para_Grp_t *pOut)
{
    Module_InData_t *in = (Module_InData_t *)g_input.para;
    /* 从 pOut->para 读取数据，写入 g_input.para */
    in->srcA_valid = 1;
    in->srcA.head_idx = ((uint8_t *)pOut->para)[0];
    /* ... */
    g_input.info.status |= ST_NEW;
}
```

**关键**: consumer 回调只写 `g_input.para`，不做业务逻辑。业务逻辑统一在 `ProcessInput` 的输入段处理。

### 4.4 ProcessInput — 三段式

```c
static void ProcessInput(void)
{
    Module_InData_t  *in  = (Module_InData_t *)g_input.para;
    Module_OutData_t *out = (Module_OutData_t *)g_output.para;

    /* route 分流: 检查 info.route 选择执行路径 */
    out->has_output = 0;

    /* ====== 输入段：消费 g_input ====== */
    if (g_input.info.status & ST_NEW) {

        if (in->srcA_valid) {
            /* ... 将 in->srcA 应用到内部状态 ... */
            in->srcA_valid = 0;
        }

        g_input.info.status &= ~ST_NEW;     /* 消费完毕 */
    }

    /* ====== 计算段：原业务逻辑 ====== */
    /* ... 原 Module_Run() / Module_OnXxx() 的计算部分移到这里 ... */

    /* ====== 输出段：写 g_output ====== */
    /* 只写 g_output.para — Switcher 负责路由 */
}
```

### 4.5 导出

```c
MODULE_EXPORT({Module});
```

### 4.6 保留旧入口（过渡期双写）

```c
/* 旧周期入口 → 委托 DoWork */
void Module_Run(void) { Module_DoWork(); }

/* 旧输入回调 → 写 g_input.para + 保留旧路径（可选） */
void Module_OnXxx(uint16_t param, void *data_ptr)
{
    /* v2.2: 写 g_input.para */
    /* ... */
}
```

---

## Step 5: 填逻辑 — 移业务代码入 ProcessInput

| 旧代码位置 | 新位置 | 改动 |
|-----------|--------|------|
| `Module_OnXxx()` 的输入处理 | `ProcessInput()` 输入段 | 从 `g_input.para` 读，不用解包 `void*` |
| `Module_Run()` 的计算 | `ProcessInput()` 计算段 | 原样搬入 |
| `OtherModule_OnYyy()` 输出 | `ProcessInput()` 输出段 → 写 `g_output.para` | 替换直接调用为 `out->field = ...` |

---

## Step 6: 验证

```bash
# 1. 本模块通过编译
# 2. 旧调用方不受影响（旧入口仍存在）
# 3. layer deps 不增加
python tools/check_deps.py .
python tools/check_include.py .
python tools/check_output_callback.py .
```

---

## 检查清单

- [ ] Step 1: 所有入口已识别（周期 + 事件 + 输出）
- [ ] Step 2: 所有 I/O 参数字段已提取
- [ ] Step 3: 数据结构已定义（pack(4) + res[] 补齐）
- [ ] Step 4: `MODULE_SKELETON()` + `Init()` + `ProcessInput()` + `MODULE_EXPORT()` 已添加
- [ ] Step 4: 每个旧输入回调对应一个新的 consumer 回调
- [ ] Step 4: 每个生产者对应一个 `{Consumer}_On{Producer}Data(Para_Grp_t *pOut)` 回调
- [ ] Step 5: `ProcessInput()` 三段完整：输入→计算→输出
- [ ] Step 5: 输出只写 g_output.para，不置 ST_OUT，不调 _onOutput
- [ ] Step 6: `check_deps.py` → 0 violations
- [ ] Step 6: `check_output_callback.py` → 0 unapproved callbacks
