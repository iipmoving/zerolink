---
name: modify-module
description: "Convert an existing module from v1.x to v2.0 Data Switcher architecture. 6-step SOP: find entry → trace path → extract I/O → build skeleton → fill logic → wire Switcher. Triggers on: modify module, refactor module, upgrade module, modify-module, 转换模块, 重构模块, 迁移模块."
---

# /modify-module — v1.x → v2.0 转换向导

将现有模块改造为符合 v2.0 数据交换机架构。

**核心**: `core/std_module.h` — **模块的骨架宏就是范式**

**Project**: four_head (SC32L14T, Cortex-M0+, ARMCC V5.06)
**Methodology**: 零耦合嵌入式架构 v2.0

---

## 核心原则

- **每个模块独立改造** — 一次改一个，不改调用方
- **多个入口 = 多个输入回调** — 每个外部调用方对应一个 `_OnInput_` 函数
- **向后兼容靠双写** — 过渡期 g_out 与旧 __weak 同时写，最后统一收口
- **先搭骨架后填肉** — 先定好 _io.h + 标准函数，再移业务逻辑

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
| 输入回调 | `Module_OnXxx()` | other_module.c | param+data_ptr | → `OnInput_Xxx()` |
| 输出桩 | `Other_OnYyy()` | (本模块调别人) | param+data_ptr | → 改写 `g_out` |

---

## Step 1.5: 检残留 — 清除旧 __weak 输出回调

```bash
# 检查本模块是否定义了发给其他 APP 模块的 __weak 回调
grep -n "__attribute__((weak))\|__weak" src/{layer}/{module}.c
```

**规则**: APP→APP 方向不得有 __weak 输出回调。发现则改为写 `g_output` + `ST_OUT`，由 Switcher 路由。

合法例外:
- DRV→APP 方向（如 drv_key → AppHmi_OnKey）— 通过 Switcher 的 `OnOutput` 强符号路由
- APP→DRV 方向（如 AppHmi_OnOutput → DrvDisplay_OnRefresh）— Switcher 直接调 DRV 强符号

**如果确实需要 APP→APP 输出回调，必须先向用户说明理由并等待确认，不得自行添加。**

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

## Step 3: 提取输入输出参数 — 设计 Input_t / Output_t

基于 Step 2 的字段清单，按数据源分组为 `_Input_t`，按目的地分组为 `_Output_t`。

**设计规则**:

| 规则 | 说明 |
|------|------|
| 每个输入源一个子结构体 | PowerCtrl 一组, SystemError 一组, RegData 一组 |
| 每个子结构体配一个 valid 标志 | `ctrl_valid`, `err_valid`, `reg_valid` — DoWork 靠此判断 |
| status 字节嵌在首部 | `uint8_t status` + `uint8_t res[3]` |
| `#pragma pack(4)` 包裹 | 32 位对齐 |
| `sizeof()` 为 4 的倍数 | 末尾 `uint8_t resN[N]` 补齐 |

```c
/* include/{module}_io.h — 模板 */

#ifndef MODULE_IO_H
#define MODULE_IO_H

#include <stdint.h>
#pragma pack(4)

/* ---- 输入槽（本模块消费） ---- */

typedef struct {
    uint8_t  head_idx;
    uint16_t value;
    uint8_t  flag;
} Module_SourceA_InputItem_t;

typedef struct {
    uint8_t  status;         /* bit0=已构造, bit1=新输入到达 (Switcher 设, 本模块清) */
    uint8_t  res[3];

    uint8_t  srcA_valid;     /* 1=本帧有新的 SourceA 数据 */
    uint8_t  res2[3];        /* 对齐 */

    Module_SourceA_InputItem_t  srcA;
} Module_Input_t;

/* ---- 输出槽（本模块产出） ---- */

typedef struct {
    uint8_t  head_idx;
    uint16_t result;
} Module_OutputItem_t;

typedef struct {
    uint8_t  status;         /* bit0=已构造, bit1=新输出就绪 (本模块设, Switcher 清) */
    uint8_t  res[3];

    uint8_t  has_output;     /* 本帧有产出 */
    uint8_t  res2[3];

    Module_OutputItem_t  out;
} Module_Output_t;

#pragma pack()

/* ---- 公开接口 ---- */
void Module_GetIO(Module_Input_t **ppIn, Module_Output_t **ppOut);
void Module_DoWork(void);

/* 输入回调 — 每个数据源一个 */
void Module_OnInput_SourceA(uint8_t head_idx, uint16_t value, uint8_t flag);

#endif /* MODULE_IO_H */
```

---

## Step 4: 搭骨架 — 创建标准函数

在 `.c` 文件中添加以下函数。**先不修改业务逻辑**，只搭框架。

### 4.1 IO 槽 + 构造函数

```c
/* .c 文件顶部新增 */
#include "../include/{module}_io.h"

/* IO 槽 */
static Module_Input_t  g_in;
static Module_Output_t g_out;

/* 构造函数 — 首次 DoWork 自检调用 */
static void _Constructor(void)
{
    memset(&g_in,  0, sizeof(g_in));
    memset(&g_out, 0, sizeof(g_out));
    /* ... 原有初始化逻辑移到这里 ... */
}
```

### 4.2 GetIO

```c
void Module_GetIO(Module_Input_t **ppIn, Module_Output_t **ppOut)
{
    if (ppIn)  *ppIn  = &g_in;
    if (ppOut) *ppOut = &g_out;
}
```

### 4.3 输入回调 — 每路一个

```c
void Module_OnInput_SourceA(uint8_t head_idx, uint16_t value, uint8_t flag)
{
    g_in.srcA_valid = 1;
    g_in.srcA.head_idx = head_idx;
    g_in.srcA.value    = value;
    g_in.srcA.flag     = flag;
}
```

**关键**: 输入回调只写 `g_in`，不做业务逻辑。业务逻辑统一在 `DoWork` 的输入段处理。

### 4.4 DoWork — 三段式骨架

```c
void Module_DoWork(void)
{
    /* --- 构造：懒惰初始化 --- */
    { static uint8_t s_init = 0; if (!s_init) { _Constructor(); s_init = 1; } }

    /* ★ 每帧先清输出标志 */
    g_out.status &= ~0x02;
    g_out.has_output = 0;

    /* ====== 输入段：消费 g_in ====== */
    if (g_in.status & 0x02) {

        if (g_in.srcA_valid) {
            /* ... 将 g_in.srcA 应用到内部状态 ... */
            g_in.srcA_valid = 0;
        }

        g_in.status &= ~0x02;     /* 消费完毕 */
    }

    /* ====== 计算段：原业务逻辑 ====== */
    /* ... 原 Module_Run() / Module_OnXxx() 的计算部分移到这里 ... */

    /* ====== 输出段：写 g_out ====== */
    if (g_out.has_output) {
        g_out.status |= 0x02;     /* 有产出 */
    }
}
```

### 4.5 保留旧入口（过渡期双写）

```c
/* 旧周期入口 → 委托 DoWork */
void Module_Run(void) { Module_DoWork(); }

/* 旧输入回调 → 写 g_in + 保留旧路径（可选） */
void Module_OnXxx(uint16_t param, void *data_ptr)
{
    /* v2.0: 写 g_in */
    Module_OnInput_SourceA(/* 解包参数 */);

    /* v1.0: 保留旧逻辑（可选，过渡期用）*/
    /* ... */
}
```

---

## Step 5: 填逻辑 — 移业务代码入 DoWork

| 旧代码位置 | 新位置 | 改动 |
|-----------|--------|------|
| `Module_OnXxx()` 的输入处理 | `DoWork()` 输入段 | 从 `g_in` 读，不用解包 `void*` |
| `Module_Run()` 的计算 | `DoWork()` 计算段 | 原样搬入 |
| `OtherModule_OnYyy()` 输出 | `DoWork()` 输出段 → 写 `g_out` | 替换直接调用为 `g_out.field = ...` |

---

## Step 6: 验证

```bash
# 1. 本模块通过编译
# 2. 旧调用方不受影响（旧入口仍存在）
# 3. layer deps 不增加
python tools/check_all.py
```

---

## 检查清单

- [ ] Step 1: 所有入口已识别（周期 + 事件 + 输出）
- [ ] Step 2: 所有 I/O 参数字段已提取
- [ ] Step 3: `_io.h` 已创建（pack(4) + status + res[] 补齐）
- [ ] Step 4: `g_in` / `g_out` / `_Constructor()` / `GetIO()` / `DoWork()` 已添加
- [ ] Step 4: 每个旧输入回调对应一个 `_OnInput_` 新函数
- [ ] Step 5: 旧 __weak 输出桩已替换为 `g_out` 写
- [ ] Step 5: `DoWork()` 三段完整：输入→计算→输出
- [ ] Step 6: `check_all.py` → 全部 PASS
