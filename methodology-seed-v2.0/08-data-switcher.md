# 08 — 数据交换机：周期性结构化数据路由

---

## 一、解决什么问题

`__weak` 回调在模块间传结构化数据时，consumer 模块需要在 `.h` 里声明 owner struct 的副本（`_LINK_t`），AI 负责保持两者布局一致。consumer 越多，维护负担越重，出错概率越大。

数据交换机把这个责任从"AI 编译前维护"移到"中间层运行时搬运"——模块不再需要 consumer struct 副本，数据路由集中在唯一的 Switcher 里完成。

**不是替代 `__weak`。** ISR 回调、同槽同步计算仍然走 `__weak`。交换机接管的是周期性的、模块间的结构化数据路由。

---

## 二、三机制分工

```
__weak 回调    → ISR 驱动、同槽同步计算        （链接器接线，零开销）
Msg_Post      → 跨时间片异步事件通知           （环形队列缓冲）
数据交换机     → 周期性模块间结构化数据路由     （中间层指针搬运，按调度槽执行）
```

选择规则扩展为两层：

```
1. 必须在同一次调度槽内完成？
   → 是: __weak 直调
   → 否: 进入下一问

2. 传递的是事件通知还是结构化数据块？
   → 事件通知: Msg_Post
   → 结构化数据块: 数据交换机
```

---

## 三、文件组织

所有模块的公开 IO 接口放在 `include/` 文件夹，**不加入编译器 `-I` path**。

| 文件 | 内容 | include guard | 谁引用 |
|------|------|--------------|--------|
| `include/module_a_io.h` | Input_t, Output_t, GetIO(), DoWork() 声明 | `#define` **保留** | 本模块 .c + Switcher.c（全路径） |
| `module_a.h`（可选） | 内部配置常量、私有枚举 | `//#define` **注释** | 仅 `module_a.c` 自己 |

**`_io.h` 和模块 `.h` 是两种文件，include guard 规则相反。** 详见 §4.3。

`check_include.py` 扫描全路径 include —— **除 Switcher.c 外任何文件用全路径 include 其他模块的 `_io.h` 即阻断提交。** 模块引用自己的 `_io.h` 也用全路径，工具按文件名区分自己/他人的 `_io.h`。

模块无内部配置项时，不需要 `module_a.h`。只有 `_io.h` 作为对外接口。

---

## 四、模块标准结构

> **v2.1**: 模块使用 `MODULE_SKELETON()` + `MODULE_EXPORT()` 宏消除样板代码, 详见 `09-std-module.md`。以下描述模块在数据交换机中的角色和协议。

### 4.1 组成部分

每个参与交换机的模块由 `MODULE_SKELETON()` 展开以下基础设施:

| 组成部分 | 职责 | 来源 |
|----------|------|------|
| `g_input` (Para_Grp_t) | 外部输入槽 — `info.status` + `void *para` 指向本模块 InData_t | MODULE_SKELETON 展开 |
| `g_output` (Para_Grp_t) | 输出槽 — `info.status` + `void *para` 指向本模块 OutData_t | MODULE_SKELETON 展开 |
| `DoWork()` | 每周期: 懒惰构造 → ProcessInput → 检测 ST_OUT → 调 _onOutput | MODULE_SKELETON 展开 |
| `Init()` | 模块实现: memset InData_t/OutData_t, 绑定 g_input.para/g_output.para | 模块自己写 |
| `ProcessInput()` | 模块实现: 三段式 (输入→计算→输出) | 模块自己写 |
| `GetIO(Para_Grp_t **, Para_Grp_t **, void (**)(void))` | 暴露指针 + DoWork 函数指针给 Switcher | MODULE_EXPORT 展开 |
| `__weak {Module}_OnOutput(Para_Grp_t *pOut)` | 输出桩 — consumer STRONG 实现负责 memcpy | MODULE_EXPORT 展开 |

纯生产者 (如 ADC) 无输入 — `g_input.para` 保持 NULL。纯消费者无下行输出 — `_onOutput` 为空壳。

### 4.2 状态字协议

IN 和 OUT 各自独立的 status 字节, 由 `Info_Header.status` 管理:

```
g_input.info.status   bit0=ST_INIT (Constructor 置)  bit1=ST_NEW (consumer STRONG 回调设 → ProcessInput 消费后自清)
g_output.info.status  bit0=ST_INIT (Constructor 置)  bit2=ST_OUT  (ProcessInput 设 → DoWork 调 _onOutput 后自清)
```

**关键**: 模块 DoWork 进入时**先清自己的 g_output.info.status ST_OUT** (每帧都清)。有产出才在 ProcessInput 末尾置位。

**bit1 生命周期** — 两个方向独立, 均由模块自身管理 (Switcher 不参与):

```
g_output ST_OUT (本模块产出):
  1. ProcessInput 进入 → g_output.info.status &= ~ST_OUT    (每帧先清)
  2. ProcessInput 结束 → 有产出时 g_output.info.status |= ST_OUT

g_input ST_NEW (外部输入):
  1. __weak STRONG 回调被调用 → memcpy(g_input.para, pOut->para, ...) → g_input.info.status |= ST_NEW  (consumer 自设)
  2. ProcessInput 检查 ST_NEW=1 → 消费 → g_input.info.status &= ~ST_NEW                                   (consumer 自清)
```

完整周期:

```
1. Producer ProcessInput: 计算 → 写 g_output.para → g_output.info.status |= ST_OUT
   Producer DoWork 检测 ST_OUT → 调 _onOutput(&g_output)
2. __weak {Producer}_OnOutput → linker 解析到 consumer STRONG 实现
3. Consumer STRONG {Producer}_OnOutput(pOut): memcpy(g_input.para, pOut->para, ...) → g_input.info.status |= ST_NEW
4. Consumer ProcessInput: g_output.info.status &= ~ST_OUT → 检查 ST_NEW → 消费 → g_input.info.status &= ~ST_NEW
```

Switcher 只负责按顺序调用各模块 DoWork, 不读写任何 status 字节。

### 4.3 _io.h 接口文件

**`_io.h` 是公开接口文件, 保留 `#define` include guard。** 它与模块自身 `.h` 的规则相反:

| 文件类型 | include guard | 原因 |
|----------|--------------|------|
| **`_io.h`** (公开 IO 接口) | `#define` **保留** | 公开接口, 恰好两个合法 include 方 (本模块 + Switcher), 需要正常的防重入保护 |
| 模块自身 `.h` (内部配置) | `//#define` **注释** | 私有头文件, 禁止他人 include, 注释 guard = L0 编译阻断 |

`_io.h` 只放本模块的数据结构定义 (InData_t / OutData_t, 挂在 Para_Grp_t.para 后面)。GetIO / DoWork 声明由 `MODULE_EXPORT` 宏生成, 不需要在 `_io.h` 中手写。

```c
// ===== include/module_a_io.h =====
#ifndef MODULE_A_IO_H
#define MODULE_A_IO_H       /* ← 保留: _io.h 是公开接口, 两个合法 include 方 */

#include <stdint.h>

/* 内部数据结构 — 挂在 Para_Grp_t.para 后面 */

typedef struct {
    uint16_t power;         /* 外部输入的功率值 */
    uint8_t  mode;          /* 外部输入的模式 */
    uint8_t  res;           /* 32位对齐 */
} ModuleAInData_t;

typedef struct {
    uint16_t result;
    uint8_t  flag;
    uint8_t  res;           /* 32位对齐 */
} ModuleAOutData_t;

#endif /* MODULE_A_IO_H */
```

### 4.4 模块内部实现 — MODULE_SKELETON 模式

```c
// ===== module_a.c =====
#include "core/std_module.h"
#include "../include/module_a_io.h"            // 自己的 IO 接口

static ModuleAInData_t  s_in;
static ModuleAOutData_t s_out;

/* ---- 骨架: 展开 g_input/g_output/Constructor/DoWork ---- */
MODULE_SKELETON();

/* ---- 初始化: Constructor 自动调用 ---- */
static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

/* ---- 处理逻辑 (DoWork 每帧调) ---- */
static void ProcessInput(void)
{
    ModuleAInData_t  *in  = (ModuleAInData_t *)g_input.para;
    ModuleAOutData_t *out = (ModuleAOutData_t *)g_output.para;

    /* ★ 每帧先清输出标志 */
    g_output.info.status &= ~ST_OUT;

    /* ====== 输入段 ====== */
    if (!(g_input.info.status & ST_NEW)) return;  // 无新输入

    /* ... 消费 in->xxx ... */

    g_input.info.status &= ~ST_NEW;               // 消费完毕

    /* ====== 计算段 ====== */
    out->result = calc(in);
    out->flag   = 1;

    /* ====== 输出段 ====== */
    g_output.info.status |= ST_OUT;               // 本帧有产出
}

/* ---- 导出: GetIO + __weak OnOutput + constructor 注册 ---- */
MODULE_EXPORT(ModuleA);

/* ---- 强符号: 接收 {Producer} 的输出 (每个数据源一个) ---- */
void Producer_OnOutput(Para_Grp_t *pOut)
{
    /* consumer 自己 memcpy — Switcher 不搬运数据 */
    memcpy(g_input.para, pOut->para, sizeof(ModuleAInData_t));
    g_input.info.status |= ST_NEW;
}
```

> **pre-v2.1 手动模式**: 不使用 `MODULE_SKELETON` 时, 需手动写 `Constructor()` / `DoWork()` / `GetIO()` / `__weak OnOutput`。不推荐新项目使用, 详见 `09-std-module.md` §8 迁移路径。

---

## 五、中间层（Switcher）

### 5.1 定位

**全项目唯一有权 include 所有 `_io.h` 的文件。** 模块之间互不知道对方存在。

**Switcher 不做数据搬运。** 数据通过 `__weak` 回调路由——producer 调用 `__weak` 桩，consumer 的 STRONG 实现写入自己的 `g_in`。Switcher 的职责是：
1. 声明所有跨模块 `__weak` 桩（作为"接线板"，集中可见）
2. 按调度次序依次调用各模块 `DoWork()`

### 5.2 初始化：Switcher_Init()

```c
// ===== data_switcher.c =====
#include "../include/module_a_io.h"
#include "../include/module_b_io.h"
// ... 所有模块的 _io.h

void Switcher_Init(void)
{
    /* 所有模块使用懒惰初始化，首次 DoWork 自检 _Constructor() */
    /* 不需要调 GetIO，不需要清零 status */
}
```

Switcher_Init 通常为空。模块的构造由自身 `DoWork()` 首次进入时自检完成。

### 5.3 __weak 桩：接线板

**Switcher 声明所有跨模块 __weak 回调桩**，集中管理数据路由的"函数名 + 签名"。

```c
/* === __weak 桩 — 跨模块数据路由（接线板）=== */

/* app_cooking 产出 → app_power 消费 */
__attribute__((weak)) void AppPower_OnInput_PowerCtrl(uint8_t head_idx, uint8_t onoff, uint16_t target_power)
{ (void)head_idx; (void)onoff; (void)target_power; }

/* app_protect 产出 → app_power 消费 */
__attribute__((weak)) void AppPower_OnInput_SystemError(uint8_t head_idx, uint8_t slave_addr, uint16_t fault)
{ (void)head_idx; (void)slave_addr; (void)fault; }

/* app_comm_mgr 产出 → app_power 消费 */
__attribute__((weak)) void AppPower_OnInput_RegData(uint8_t head_idx, uint8_t online, uint16_t igbt, uint16_t bot, uint16_t vol)
{ (void)head_idx; (void)online; (void)igbt; (void)bot; (void)vol; }
```

**模块不需要 include 其他模块的头文件。** 模块只需知道 `__weak` 函数名和签名。Switcher 声明空壳，consumer 提供 STRONG 实现（写入自己的 `g_in`），producer 直接调用函数名。链接器自动接线。

### 5.4 运行时：Switcher_Run()

按调度次序依次调用各模块 `DoWork()`。数据路由已在模块内部通过 `__weak` 完成，Switcher 不参与。

```c
void Switcher_Run_Slot3(void)
{
    /* 按调度次序执行模块 — 数据通过 __weak 回调自动路由 */
    AppCooking_DoWork();
    AppPower_DoWork();
}
```

**时序保证**：producer 的 DoWork 先执行（内部调 `__weak` 回调 → consumer STRONG 写 `g_in` → 设 `g_in.status bit1`）→ consumer 的 DoWork 后执行（检查 `g_in.status bit1` → 消费）。同一帧内完成，无需延迟一帧。

### 5.5 生产者调用模式

Producer 在 DoWork 的计算段末尾，调用 Switcher 声明的 `__weak` 回调：

```c
/* === app_cooking.c (producer) === */
void AppCooking_DoWork(void)
{
    /* ... 输入段 ... */

    /* === 计算段 === */
    /* ... 计算出功率命令 ... */

    /* 输出段：调用 __weak 回调 → linker 解析到 consumer STRONG */
    AppPower_OnInput_PowerCtrl(head_idx, onoff, target_power);
}

/* === app_power.c (consumer) === */
void AppPower_OnInput_PowerCtrl(uint8_t head_idx, uint8_t onoff, uint16_t target_power)
{
    /* STRONG — 写入自己的 g_in */
    g_in.ctrl_valid  = 1;
    g_in.ctrl_head   = head_idx;
    g_in.ctrl_onoff  = onoff;
    g_in.ctrl_power  = target_power;
}

void AppPower_DoWork(void)
{
    /* ... 检查 g_in.status & 0x02 → 消费 g_in 字段 ... */
}
```

**关键**：producer 不知道 consumer 的存在，只知道函数名 `AppPower_OnInput_PowerCtrl`。如果 consumer 未迁移（无 STRONG），链接器选中 Switcher 的 WEAK 空壳，调用静默丢弃。

---

## 六、结构体使用约束

- **枚举不出模块**：跨模块数据用纯 struct 字段描述，不用枚举值。模块内部遍历用联合体或指针偏移
- **常量不跨模块**：没有公共 `constants.h`。跨模块常量由定义方放入 Output_t，Switcher 填入消费方 Input_t
- **字段变更影响面**：owner 的 Output_t 字段变更 → 只影响 Switcher 接线段。不需要改所有 consumer

---

## 七、与 __weak 的共存

一个模块可以同时有两条入口：

```
模块 X:
  ├── ModuleX_DoWork()  ← Switcher 每周期调用（消费 g_in → 计算 → 更新 g_out）
  └── ModuleX_OnISR()   ← __weak 强符号，ISR 独立入口
```

两条路径互不冲突。ISR 不走 Switcher。

---

## 八、include 权限规则

| 文件 | 权限 | 审计 |
|------|------|------|
| **Switcher.c** | 全路径 `#include "../include/xxx_io.h"` | 允许 |
| **模块 .c** | `#include "../include/xxx_io.h"` | 禁止 — `check_include.py` 阻断 |
| **模块 .c** | `#include "module_a.h"` | 允许 — 只能引自己的头文件 |
| **模块 .c** | `#include "../include/module_a_io.h"` | 允许 — 只能引自己的 `_io.h` |

`check_include.py` 扫描逻辑：
- 非 `Switcher.c` 中检测到全路径 `#include "../include/"` → 阻断
- 模块 .c 引用了非自己的 `_io.h` → 阻断

---

## 九、命名约定

数据交换机模块遵守 `01-architecture.md` §4 的统一命名规范。以下为交换机特有的补充：

```
文件:       include/module_a_io.h   module_a.h   module_a.c
类型:       ModuleA_Input_t         ModuleA_Output_t
函数:       ModuleA_GetIO()         ModuleA_DoWork()
变量:       g_in                    g_out
结构体成员:  snake_case 单词，不加模块前缀 (power, mode, voltage，不是 a_power)
```

`{Module}{Name}_{type}` — 模块前缀区分命名空间，结构体成员不加前缀。

---

## 十、优势

- **消除 `_LINK` 副本**：模块不需要声明 consumer struct
- **AI 维护量下降**：owner 字段变更 → 只改 Switcher 接线，不改 N 个 consumer
- **数据流集中可见**：所有跨模块路由在 `Switcher_Run()` 一目了然
- **头文件物理隔离**：`_io.h` 统一放 `include/`，全路径可被工具审计
- **初始化一致**：Switcher 上电统一清零，不依赖模块记住

---

## 十一、不适用场景

- ISR 回调（延迟敏感 → 仍走 `__weak`）
- 同槽同步计算（调用即执行 → 仍走 `__weak`）
- 跨进程/跨核通信（需消息持久化 → `Msg_Post`）

---

## 十二、与 interface_map.h 的关系

`interface_map.h` 仍然管 __weak 函数配对。数据交换机引入后，新增一块"接线表"——可硬编码在 Switcher，也可抽成 `wiring.json`（后续工具化）。

```
interface_map.h  →  __weak 函数配对 + 签名验证        → check_weak_pairs.py
Switcher 接线段  →  结构化数据路由（Output_t → Input_t） → 暂无工具（后续 generate_switcher.py）
```

---

*方法论版本: v2.1, 2026-06-06*
