---
name: modify-module
description: "Convert an existing module from v1.x to v2.2/v2.3 PULL architecture. 6-step SOP: find entry → trace path → extract I/O → build skeleton → fill logic → wire Switcher. Triggers on: modify module, refactor module, upgrade module, modify-module, 转换模块, 重构模块, 迁移模块."
user-invocable: true
---

# /modify-module — v1.x → v2.2 转换向导

将现有模块改造为符合 v2.2/v2.3 PULL 架构（`std_module.h` 模式）。

**核心**: `std_module.h` — **模块的骨架宏就是范式**

> v2.3: 如果需要 ISR 支持，在 .c 文件顶部加 `#define STD_MODULE_ENABLE_ISR 1` 后 `#include "std_module.h"`。

---

## 核心原则

- **每个模块独立改造** — 一次改一个，不改调用方
- **多个入口 = 多个 consumer 回调** — 每个外部调用方对应一个 `_On{Producer}Data` 函数
- **向后兼容靠双写** — 过渡期 g_out 与旧 __weak 同时写，最后统一收口
- **先搭骨架后填肉** — 先定好数据结构 + 标准函数，再移业务逻辑
- **输出不推送** — producer 只写 g_output.para，路由由 Switcher 负责

---

## Step 0: 数据契约检查

修改前，先检查数据交互说明书：

```
1. `docs/data-contract.md` 是否存在？
   ├─ 是 → 跳到第 2 步
   └─ 否 → 本模块是否涉及 I/O 变更？
       ├─ 是 → 创建 `docs/data-contract.md` (参照 methodology-seed/10-data-contract.md)
       └─ 否 → 跳过 (纯内部重构不影响数据契约)

2. 本模块的输出结构体是否有消费者？
   ├─ 有 → 确认消费者的 Input_t 与 Output_t 布局一致 (可 memcpy)
   └─ 无 → OK

3. 本模块的输入结构体是否来自生产者？
   ├─ 有 → 确认生产者的 Output_t 与 Input_t 布局一致 (可 memcpy)
   └─ 无 → OK

4. 如 I/O 变更，同步更新 `docs/data-contract.md`
```

**先更新数据契约，再改代码。**

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

**🚫 核心概念：INPUT_LINK 与 OUTPUT_LINK 是配对管道**

```
┌─────────────────────────────────────────────────────────────────┐
│  数据管道配对                                                    │
│                                                                 │
│  Producer (DrvKey):                                             │
│    MODULE_OUTPUT_PARAMS(Key, Hmi)   — 输出数据参数              │
│    MODULE_OUTPUT_LINK(Key, Hmi)     — 输出管道                  │
│    MODULE_OUTPUT(DrvKey)            — 输出聚合                  │
│                                                                 │
│  Consumer (AppHmi):                                             │
│    MODULE_INPUT_PARAMS(Key, Hmi)    — 输入数据参数 (布局一致)   │
│    MODULE_INPUT_LINK(Key, Hmi)      — 输入管道 (与OUTPUT配对)   │
│    MODULE_INPUT(AppHmi)             — 输入聚合                  │
│                                                                 │
│  关键规则:                                                       │
│    1. INPUT = 你想从别的模块得到的数据                           │
│    2. OUTPUT = 你给别的模块提供的数据                            │
│    3. INPUT_LINK 与 OUTPUT_LINK 布局完全一致 (配对管道)         │
│    4. INPUT_LINK 里用 MODULE_INPUT_PARAMS，不是 OUTPUT_PARAMS   │
│    5. 模块可以只有 INPUT 或只有 OUTPUT，或两者都有               │
└─────────────────────────────────────────────────────────────────┘
```

### 核心概念：管道配对（必须理解，不可跳过）

**这是 PULL 范式的核心。另一个 AI 搞错了，说明文档不够清楚，所以请务必读这一节。**

```
消费者向生产者注册一个管道，双方用同一套结构体布局，
MODULE_OUTPUT_LINK / MODULE_INPUT_LINK 宏保证命名一致性，
两个结构在模块双方独立声明 (自包含)，中间层 (data_switcher.c) 用 void* 连接。
```

#### 管道怎么配对

| 角色 | 在 io.h 中的声明 | 是否自包含 | 连接方式 |
|------|-----------------|-----------|---------|
| Producer | `MODULE_OUTPUT_LINK(Producer, Consumer)` | ✅ 引用自己的 OUTPUT_PARAMS | — |
| Consumer | `MODULE_INPUT_LINK(Producer, Consumer)` | ✅ 定义自己的 INPUT_PARAMS (不引用 Producer) | InputCallback 用 `(void*)` 赋值 |

#### 实例 vs 指针 (容易搞错)

```c
// === Producer 的 io.h ===
typedef struct {
    MODULE_OUTPUT_LINK(Producer, Consumer)  {Consumer}_params;    // ← 实例 (以消费者命名)
} MODULE_OUTPUT(Producer);

// === Consumer 的 io.h ===
typedef struct {
    MODULE_INPUT_LINK(Producer, Consumer)* {Producer}_params;    // ← 指针 (以生产者命名)
} MODULE_INPUT(Consumer);
```

Producer 持有 OUTPUT_LINK 的**实例** ({Consumer}_params)，Consumer 持有 INPUT_LINK 的**指针** (*{Producer}_params)。InputCallback 用 INPUT_GET_SLOT 宏将 producer 的 {Consumer}_params 地址直穿赋值给 consumer 的 {Producer}_params 指针，零拷贝。

#### 对称命名

```c
// in = consumer, out = producer
in->{Producer}_params = (void*)&out->{Consumer}_params;
// 例: in->Calculator_params = (void*)&out->ElecParams_params
```

命名对称: consumer 的输入成员以 producer 命名，producer 的输出成员以 consumer 命名。赋值语句一目了然表明数据流向。

#### void* 跨类型转换

```c
// data_switcher.c — Consumer 的 InputCallback 强符号
INPUT_CALLBACK(Producer, Consumer)
{
    /* 对称命名直穿: in->{Producer}_params = (void*)&out->{Consumer}_params
     * OUTPUT_LINK 和 INPUT_LINK 类型不同但布局一致, void* 跨类型转换 */
    INPUT_GET_SLOT(Producer, Consumer);
}
```

`&out->{Consumer}_params` 是 `MODULE_OUTPUT_LINK(Producer, Consumer)*`（producer 侧实例），`in->{Producer}_params` 是 `MODULE_INPUT_LINK(Producer, Consumer)*`（consumer 侧指针）。两者是**不同的 C 类型**，但 INPUT_GET_SLOT 内部用 `(void*)` 跨类型转换，布局相同确保字段偏移正确。这是`(void*)` 跨类型转换是必须的，不是"偷懒"。

#### 自包含规则

Consumer 的 `_io.h` **不得 `#include` Producer 的任何头文件**。Consumer 定义自己的输入类型:

```c
// consumer_io.h — 自包含输入类型
typedef struct {
    uint16_t field1;       // 布局与 Producer OUTPUT_PARAMS 一致
    uint16_t field2;       // 但用的是自己的类型名
    uint8_t  valid;
} MODULE_INPUT_PARAMS(Producer, Consumer);  // ← 自包含，不是引用
```

**两个模块独立声明、布局一致、宏保证一致性、中间层连接。这就是管道配对的核心。**

---

**示例：DrvKey 只有 OUTPUT**

```c
/* drv_key_io.h */

/* ========== OUTPUT (DrvKey 给别的模块提供的数据) ========== */

typedef struct {
    uint8_t  key_code;
    uint8_t  key_state;
    uint8_t  head_index;
    uint8_t  res[1];
} MODULE_OUTPUT_PARAMS(Key, Hmi);

typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(Key, Hmi) *params;  /* 指向自己的输出数据 */
} MODULE_OUTPUT_LINK(Key, Hmi);

typedef struct {
    MODULE_OUTPUT_LINK(Key, Hmi) *to_hmi;
} MODULE_OUTPUT(DrvKey);

/* ========== INPUT (无) ========== */
/* DrvKey 是底层驱动，没有输入 */
```

**示例：AppHmi 有 INPUT 和 OUTPUT**

```c
/* app_hmi_io.h */

/* ========== INPUT (AppHmi 从别的模块得到的数据) ========== */

typedef struct {
    uint8_t  key_code;
    uint8_t  key_state;
    uint8_t  head_index;
    uint8_t  res[1];
} MODULE_INPUT_PARAMS(Key, Hmi);  /* 布局与 DrvKey OUTPUT_PARAMS 一致 */

typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(Key, Hmi) *params;  /* 指向自己的输入数据 */
} MODULE_INPUT_LINK(Key, Hmi);  /* 与 DrvKey OUTPUT_LINK 配对 */

typedef struct {
    MODULE_INPUT_LINK(Key, Hmi)   *key;
    MODULE_INPUT_LINK(Power, Hmi) *power;
} MODULE_INPUT(AppHmi);

/* ========== OUTPUT (AppHmi 给别的模块提供的数据) ========== */

typedef struct {
    char     seg_chars[8];
    uint8_t  seg_mode;
    /* ... */
} MODULE_OUTPUT_PARAMS(Hmi, Display);

typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(Hmi, Display) *params;
} MODULE_OUTPUT_LINK(Hmi, Display);

typedef struct {
    MODULE_OUTPUT_LINK(Hmi, Display) *to_display;
} MODULE_OUTPUT(AppHmi);
```

**设计规则**:

| 规则 | 说明 |
|------|------|
| 每个输入源一个子结构体 | PowerCtrl 一组, SystemError 一组, RegData 一组 |
| 每个子结构体配一个 valid 标志 | `ctrl_valid`, `err_valid`, `reg_valid` — DoWork 靠此判断 |
| `#pragma pack(4)` 包裹 | 32 位对齐 |
| `sizeof()` 为 4 的倍数 | 末尾 `uint8_t resN[N]` 补齐 |
| INPUT_LINK 用 INPUT_PARAMS | 不引用 OUTPUT_PARAMS，定义自己的 INPUT_PARAMS |
| INPUT_PARAMS 布局与 OUTPUT_PARAMS 一致 | 确保指针直穿后字段偏移正确 |

---

## Step 4: 搭骨架 — 创建标准函数

在 `.c` 文件中添加以下函数。**先不修改业务逻辑**，只搭框架。

### 4.1 引用 + 数据槽

```c
#include "std_module.h"
#include "{module}.h"         /* 可选 — 私有常量 */
#include <string.h>

static Module_InData_t  s_in;
static Module_OutData_t s_out;

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
    /* ... 原有初始化逻辑移到这里 ... */
}
```

### 4.3 Consumer 回调 — 每路一个

**v2.3 LINK+PARAMS 模式 (INPUT_CALLBACK 宏 + 指针直穿, 零拷贝):**

```c
/* data_switcher.c 中覆盖强符号 — Switcher 在 Producer DoWork 后调用 */
INPUT_CALLBACK(Producer, Consumer)
{
    /* 指针直穿: 生产者 {Consumer}_params 地址 → 消费者 {Producer}_params 指针
     * 两类型 (MODULE_OUTPUT_LINK / MODULE_INPUT_LINK) 布局一致但 C 类型不同,
     * 用 (void*) 跨类型转换, 零拷贝
     * 展开为: in->{Producer}_params = (void*)&out->{Consumer}_params */
    INPUT_GET_SLOT(Producer, Consumer);
}
```

**多管道组合**: 一个 INPUT_CALLBACK 内可多次调用 INPUT_GET_SLOT，连接多个数据源:

```c
INPUT_CALLBACK(Calculator, PowerBase)
{
    /* 从 Calculator 直穿拉数据 */
    INPUT_GET_SLOT(Calculator, PowerBase);

    /* 从 ElecParams 直穿拉数据 (自定义成员名) */
    ElecParams_Output *ep_out = (ElecParams_Output *)s_slot[SLOT_ElecParams].pOut->para;
    PowerBase_Input *pwr_in = (PowerBase_Input *)s_slot[SLOT_PowerBase].pIn->para;
    pwr_in->Calculator_params = (void*)&ep_out->{Consumer}_params;
}
```

**关键**: consumer 回调只做指针赋值，不做业务逻辑。业务逻辑统一在 `ProcessInput` 的输入段处理。

### 4.4 ProcessInput — 三段式

```c
static void ProcessInput(void)
{
    Module_InData_t  *in  = (Module_InData_t *)g_input.para;
    Module_OutData_t *out = (Module_OutData_t *)g_output.para;

    /* ====== 输入段：消费 g_input ====== */
    /* v2.3: 检查数据层 LINK struct 首字节的 status, 非 g_input.info.status */
    if (!in || !in->{Producer}_params) return;
    if (!(in->{Producer}_params->status & ST_NEW)) return;

    /* 消费输入后清除 ST_NEW */
    in->{Producer}_params->status &= ~ST_NEW;

    /* 多输入 LINK: 每个 LINK 独立检查 */
    // if (in->adc.status  & ST_NEW) { in->adc.status  &= ~ST_NEW; }
    // if (in->comm.status & ST_NEW) { in->comm.status &= ~ST_NEW; }

    /* ====== 计算段：原业务逻辑 ====== */
    /* ... 原 Module_Run() / Module_OnXxx() 的计算部分移到这里 ... */

    /* ====== 输出段：写 g_output ====== */
    /* v2.3: 置输出 LINK status, 由 Switcher 或下游模块读取 */
    out->{Consumer}_params.status |= ST_OUT;
}
```

**🚫 强制约束：头文件自治规则**

模块的任何 `.h` 文件（包括 `_io.h` 和普通 `.h`）**不得 `#include` 任何其他模块的 `.h`**。

```c
// ❌ 错误：elec_params_io.h 不能 include calculator_io.h
#include "calculator_io.h"      // calculator_io.h 是其他模块的 IO 头

// ❌ 错误：普通 .h 也不能 include 其他模块的 .h
#include "power_calculator.h"   // power_calculator.h 是其他模块的头文件

// ✅ 正确：elec_params_io.h 自包含所有输入类型定义
typedef struct {
    ElecParams_HrtimState hrtim;
    uint16_t peak_current;
    uint16_t active_current;
    uint16_t voltage;
    uint16_t zero_cross_high;
    uint16_t zero_cross_low;
    // padding 字段匹配 Calculator 输出布局以便指针直穿
} ElecParams_CycleInput;
```

每个模块的 IO 头定义自己的输入类型，**布局与生产者输出类型兼容**（字段顺序/大小一致），但不 include 对方的头文件。

| 规则 | 说明 |
|------|------|
| 自包含 | IO 头只 `#include <stdint.h>` `"std_module.h"` 等系统/框架头 |
| 布局兼容 | 自己的输入 struct 字段顺序/大小 = 生产者输出 struct |
| 指针直穿 | 布局兼容才能使 InputCallback 用指针赋值（非 memcpy） |

**🚫 强制约束：ProcessInput 不得包含内联计算**

`ProcessInput` 只能做 I/O 边界工作：

| 段 | 允许的操作 | 禁止的操作 |
|----|-----------|-----------|
| 输入段 | 从 `g_input.para` 读取，提取数据 | 不得做业务计算、滤波、数学运算 |
| 计算段 | **调用纯计算函数**（一行调用） | 不得内联上百行数学逻辑 |
| 输出段 | 将计算结果写入 `g_output.para` | 不得掺杂业务逻辑 |

纯计算函数（如 `ElecParams_Calc`）必须遵守：
- 函数签名不引用本模块的 `MODULE_INPUT` / `MODULE_OUTPUT` 类型
- 输入用生产者类型（如 `Calculator_Output_Link*`）或基本类型
- 输出用内部浮点结构体（非 I/O 结构体）
- 修改 I/O 接口只需改 `ProcessInput` 的输入/输出段，不动计算函数

```c
/* ✅ 正确：ProcessInput 三段清晰 — v2.3 数据层 status */
static void ProcessInput(void) {
    /* ====== 输入段 ====== */
    if (!in || !in->{Producer}_params) return;
    if (!(in->{Producer}_params->status & ST_NEW)) return;

    /* ====== 计算段（一行调用） ====== */
    CalcResult r;
    Calc(&r, link, link->count);

    /* ====== 输出边界 ====== */
    out->{Consumer}_params[0].params->value = (int32_t)(r.value * 100.0f);
    out->{Consumer}_params[0].status |= ST_OUT;
    in->{Producer}_params->status &= ~ST_NEW;
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
python ../.claude/tools/check_deps.py .
python ../.claude/tools/check_include.py .
python ../.claude/tools/check_output_callback.py .
```

---

## 检查清单

- [ ] Step 1: 所有入口已识别（周期 + 事件 + 输出）
- [ ] Step 2: 所有 I/O 参数字段已提取
- [ ] Step 3: 数据结构已定义（pack(4) + res[] 补齐）
- [ ] Step 4: `MODULE_SKELETON()` + `Init()` + `ProcessInput()` + `MODULE_EXPORT()` 已添加
- [ ] Step 4: 每个旧输入回调对应一个新的 INPUT_CALLBACK + INPUT_GET_SLOT 回调
- [ ] Step 4: 每个消费者回调使用对称命名: in->{Producer}_params = (void*)&out->{Consumer}_params
- [ ] Step 5: `ProcessInput()` 三段完整：输入→计算→输出
- [ ] Step 5: 输出只写 g_output.para，不置 ST_OUT，不调 _onOutput
- [ ] Step 6: `check_deps.py` → 0 violations
- [ ] Step 6: `check_output_callback.py` → 0 unapproved callbacks

---

## Step 7 (可选): 多 route 分流 + Switcher_RunNow

如果改的模块有"多个子功能走同一管道"的需求（如既写段码又阻塞 HMI），旧模式是多个 `__weak` 条目，新模式合并为一个 LINK + route 分流。

### LINK 结构变化

```c
// 旧: res[3] 无语义
typedef struct {
    uint8_t  status;
    uint8_t  res[3];
    MODULE_OUTPUT_PARAMS(...) params[POT_MAX];
} MODULE_OUTPUT_LINK(...);

// 新: route + seq 具名, params 改指针
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  seq;       /* 更新有效性 */
    uint8_t  route;     /* 路由标识 — AI 看见即理解 */
    MODULE_OUTPUT_PARAMS(...) *params;  /* 指针 */
} MODULE_OUTPUT_LINK(...);
```

### PARAMS 用 union 分 route

```c
typedef struct { uint8_t com; uint8_t seg_mask; uint8_t result; } Write_Params;  /* route=0 */
typedef struct { uint8_t block; uint8_t ack; }       Block_Params;                 /* route=1 */

typedef struct {
    uint8_t route;
    union { Write_Params write; Block_Params block; };
} MODULE_OUTPUT_PARAMS({Module}, Consumer);
```

### producer 输出段末尾调 Switcher_RunNow

```c
s_link->params->write.com = com;
s_link->params->write.seg_mask = mask;
s_link->seq++;
s_link->route = 0;
g_output.info.status |= ST_OUT;
Switcher_RunNow(SLOT_Consumer);    /* 即刻执行 */
uint8_t r = s_link->params->write.result;  /* 读回传 */
```

> ⚠️ **Switcher_RunNow 不支持重入，仅单向固定调用**。consumer 不得反向调回 producer，调用链是编译期确定的直链。详见 `methodology-seed/08-data-switcher.md §5.6.3`。

详见 `methodology-seed/08-data-switcher.md §5.6`。
