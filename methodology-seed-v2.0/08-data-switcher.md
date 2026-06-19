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
数据交换机     → 周期性模块间结构化数据路由     （Switcher 显式 PULL，按调度槽执行）
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

> **v2.2**: 模块使用 `MODULE_SKELETON()` + `MODULE_EXPORT()` 宏消除样板代码, 详见 `09-std-module.md`。以下描述模块在数据交换机中的角色和协议。

### 4.1 组成部分

每个参与交换机的模块由 `MODULE_SKELETON()` 展开以下基础设施:

| 组成部分 | 职责 | 来源 |
|----------|------|------|
| `g_input` (Para_Grp_t) | 外部输入槽 — `info.status` + `void *para` 指向本模块 InData_t | MODULE_SKELETON 展开 |
| `g_output` (Para_Grp_t) | 输出槽 — `info.status` + `void *para` 指向本模块 OutData_t | MODULE_SKELETON 展开 |
| `DoWork()` | 每周期: 懒惰构造 → ProcessInput | MODULE_SKELETON 展开 |
| `Init()` | 模块实现: memset InData_t/OutData_t, 绑定 g_input.para/g_output.para | 模块自己写 |
| `ProcessInput()` | 模块实现: 三段式 (输入→计算→输出) | 模块自己写 |
| `GetIO(Para_Grp_t **, Para_Grp_t **, void (**)(void))` | 暴露指针 + DoWork 函数指针给 Switcher | MODULE_EXPORT 展开 |

纯生产者 (如 ADC) 无输入 — `g_input.para` 保持 NULL。纯消费者无下行输出。

### 4.2 状态字协议

IN 和 OUT 各自独立的 status 字节, 由 `Info_Header.status` 管理:

```
g_input.info.status   bit0=ST_INIT (Constructor 置)  bit1=ST_NEW (consumer 回调设 → ProcessInput 消费后自清)
g_output.info.status  bit0=ST_INIT (Constructor 置)
```

**bit1 生命周期**:

```
g_input ST_NEW (外部输入):
  1. Consumer 回调被 Switcher 调用 → 指针直穿（消费者 LINK 指针 = 生产者输出地址，零拷贝）→ g_input.info.status |= ST_NEW  (consumer 自设)
  2. ProcessInput 检查 ST_NEW=1 → 消费 → g_input.info.status &= ~ST_NEW                                   (consumer 自清)
```

完整周期 (PULL 模式):

```
1. Producer ProcessInput: 计算 → 写 g_output.para (不置状态位)
2. Switcher 显式路由: 检查 producer g_output.para 标志 → 调 consumer 回调
3. Consumer 回调: 消费者输入 LINK 指针 = 生产者输出地址 (直穿，零拷贝) → g_input.info.status |= ST_NEW
4. Consumer ProcessInput: 检查 ST_NEW → 消费 → g_input.info.status &= ~ST_NEW
```

Switcher 负责按顺序调用各模块 DoWork + 路由检查，不读写任何 status 字节。

**@OUTPUT_CALLBACK 例外** (实时性要求):
```
Producer ProcessInput → 写 g_output.para + 置 info.route → 立即调 consumer DoWork (绕开 Switcher 周期)
Consumer DoWork 检查 g_input.info.route → 选择执行路径 → 立即生效
```
适用场景: 蜂鸣器即时反馈等。须 `@OUTPUT_CALLBACK` 标记 + 用户确认 + interface_map.h 注册。

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
#include "std_module.h"
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

    /* ====== 输入段 ====== */
    if (!(g_input.info.status & ST_NEW)) return;  // 无新输入

    /* ... 消费 in->xxx ... */

    g_input.info.status &= ~ST_NEW;               // 消费完毕

    /* ====== 计算段 ====== */
    out->result = calc(in);
    out->flag   = 1;

    /* ====== 输出段 ====== */
    /* 只写 g_output.para — Switcher 负责路由 */
}

/* ---- 导出: GetIO (不再生成 __weak OnOutput) ---- */
MODULE_EXPORT(ModuleA);

/* ---- Consumer 回调: Switcher 在 Producer DoWork 后显式调用 ---- */
void ModuleA_On{Producer}Data(Para_Grp_t *pOut)
{
    /* v2.3 指针直穿: 生产者 OUTPUT_LINK 地址 → 消费者 INPUT_LINK 指针
     * 用 (void*) 跨类型转换 (两类型布局一致但 C 类型不同) */
    MODULE_INPUT(ModuleA) *in = (MODULE_INPUT(ModuleA) *)g_input.para;
    in->input = (void*)pOut->para;
    g_input.info.status |= ST_NEW;
}
```

> **pre-v2.1 手动模式**: 不使用 `MODULE_SKELETON` 时, 需手动写 `Constructor()` / `DoWork()` / `GetIO()`。不推荐新项目使用, 详见 `09-std-module.md` §8 迁移路径。

---

## 五、中间层（Switcher）

### 5.1 定位

**全项目唯一有权 include 所有模块 `_io.h` 的文件。** Switcher 是所有跨模块数据调用的唯一通道。

模块之间互不知道对方存在，不得直接 include 其他模块的 `.h`（包括 `_io.h` 和普通 `.h`）。

**Switcher 不做数据搬运。** Switcher 的职责是：
1. 按调度次序依次调用各模块 `DoWork()`
2. 在 Producer DoWork 后检查输出标志 → 显式调用 consumer 回调 (PULL)
3. Consumer 回调内部自己 memcpy + 置 ST_NEW

### 5.2 初始化：Switcher_Init()

v2.2 风格（`Switcher_Register` 间接注册）：

```c
// ===== data_switcher.c =====
void Switcher_Init(void)
{
    Para_Grp_t *pIn, *pOut;
    void       (*pDoWork)(void);

    AppCommMgr_GetIO(&pIn, &pOut, &pDoWork);
    Switcher_Register(pDoWork, pOut);   /* 注册 DoWork + 输出指针 */

    AppPower_GetIO(&pIn, &pOut, &pDoWork);
    Switcher_Register(pDoWork, pOut);

    // ... 所有模块
}
```

v2.3 风格（`ModuleSlotDef` 数组 + `SLOT_GETIO` 宏，强制 GetIO 与槽位命名对齐）：

```c
/* data_switcher.h */
#define SLOT(mod)  SLOT_##mod

#define SLOT_GETIO(mod)                                                      \
    mod##_GetIO(&s_slot[SLOT(mod)].pIn,                                      \
                &s_slot[SLOT(mod)].pOut,                                      \
                &s_slot[SLOT(mod)].pDoWork)

/* data_switcher.c — 槽位枚举用 SLOT() 宏 */
typedef enum {
    SLOT(APP_Adc)    = 0,
    SLOT(PowerBase)  = 1,
    SLOT(Calculator) = 2,
    SLOT(COUNT)
} SwitcherSlot_t;

static ModuleSlotDef s_slot[SLOT_COUNT];

void Switcher_Init(void)
{
    SLOT_GETIO(APP_Adc);
    SLOT_GETIO(PowerBase);
    SLOT_GETIO(Calculator);
}
```

`SLOT_GETIO(mod)` 展开为 `mod##_GetIO(&s_slot[SLOT_##mod].pIn, ...)`，强制模块 GetIO 函数名与 SLOT 枚举值使用同一标识符。模块名 = 槽位名，消除命名错位。`SLOT(x)` 宏用于枚举定义，`SLOT_GETIO(mod)` 用于注册。

### 5.3 运行时：Switcher_Run() — PULL 路由

按调度次序依次调用各模块 `DoWork()`。每个 Producer 后立即检查输出并路由到 Consumer。

```c
void Switcher_Run(void)
{
    /* Phase 1+2 交错: Producer DoWork → 路由 → Consumer DoWork */

    /* AppCommMgr: producer of register data */
    s_slots[SLOT_COMM_MGR].pDoWork();
    _route_comm_mgr();  /* 检查 has_reg → AppPower_OnCommMgrData(pOut) */

    /* AppPower: consumer (消费 CommMgr 数据) + producer */
    s_slots[SLOT_POWER].pDoWork();

    /* AppProtect: consumer + producer */
    s_slots[SLOT_PROTECT].pDoWork();
    _route_protect();   /* 检查 has_err → AppPower_OnProtectData(pOut) */

    /* ... 按调度顺序继续 */
}
```

### 5.4 路由函数模板

```c
/* AppCommMgr 输出 → 寄存器数据广播 */
static void _route_comm_mgr(void)
{
    Para_Grp_t *pOut = s_slots[SLOT_COMM_MGR].pOut;
    if (!pOut || !pOut->para) return;
    uint8_t *d = (uint8_t *)pOut->para;
    if (!d[0]) return;  /* has_reg flag */
    AppPower_OnCommMgrData(pOut);          /* v2.2: Para_Grp_t 直传 */
    /* v1.x compat: unpack for pre-MODULE_SKELETON consumers */
    uint8_t head = d[1];
    AppCooking_OnRegData((uint16_t)head, d);
    AppProtect_OnRegData((uint16_t)head, d);
}
```

**关键**：producer 不知道 consumer 的存在，只写 g_output.para + 内部标志。如果 consumer 未注册，Switcher 路由检查 has_* 标志为 0，静默跳过。

### 5.5 @OUTPUT_CALLBACK 例外：立即 DoWork + route 分流

正常 PULL 模式下，consumer 的 DoWork 要等下一轮 Switcher 调度。蜂鸣器等实时操作需要绕开这个延迟。

**模式**:
```c
/* === producer (app_hmi.c) === */
static void post_buzzer(uint8_t valid)
{
    s_out.has_buzzer = 1;
    s_out.buzzer_on  = valid ? 1u : 0u;
    g_output.info.route = 2;  /* route=2 → buzzer 执行路径 */

    /* @OUTPUT_CALLBACK: buzzer real-time feedback — user confirmed */
    DrvBuzzer_OnCtrl(valid ? 1u : 0u, NULL);
}

/* === consumer (drv_buzzer.c) === */
static void ProcessInput(void)
{
    /* route 分流: 检查 info.route 选择执行路径 */
    switch (g_input.info.route) {
    case 1: /* 正常蜂鸣 */ break;
    case 2: /* 即时蜂鸣 — @OUTPUT_CALLBACK 触发 */ break;
    }
}
```

**规则**:
- route 字段由 producer 设置，consumer ProcessInput 内 switch 分流
- @OUTPUT_CALLBACK 绕开 Switcher 周期，直接调 consumer DoWork 或回调
- 须 `@OUTPUT_CALLBACK` 标记 + 用户确认 + interface_map.h 白名单注册
- `check_output_callback.py` 扫描无标记的输出回调 → 阻断提交

---

## 六、结构体使用约束

- **枚举不出模块**：跨模块数据用纯 struct 字段描述，不用枚举值。模块内部遍历用联合体或指针偏移
- **常量不跨模块**：没有公共 `constants.h`。跨模块常量由定义方放入 Output_t，通过 Switcher 路由到达消费方
- **字段变更影响面**：owner 的 Output_t 字段变更 → 只影响 Switcher 路由段 + consumer 回调。不需要改所有 consumer

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

**Switcher 是所有跨模块调用的唯一通道。** 任何模块不得直接引用其他模块的类型定义。

| 文件 | 权限 | 审计 |
|------|------|------|
| **Switcher.c** | 全路径 `#include "../include/xxx_io.h"` | 允许 — 唯一合法跨模块引用点 |
| **模块 .c** | `#include "../include/xxx_io.h"` | 禁止 — `check_include.py` 阻断 |
| **模块 .c** | `#include "module_a.h"` | 允许 — 只能引自己的头文件 |
| **模块 .c** | `#include "../include/module_a_io.h"` | 允许 — 只能引自己的 `_io.h` |
| **任何 `.h`** | `#include "other_module.h"` | **禁止** — 头文件自治规则，任何 `.h` 不得 include 其他模块的 `.h` |

### 头文件自治规则

任何模块的 `.h` 文件（包括 `_io.h` 和普通 `.h`）**不得 `#include` 其他模块的 `.h`**。需要引用其他模块的数据类型时：

1. 在自己 `_io.h` 中自包含定义输入结构体
2. 布局与生产者输出兼容（字段顺序/大小一致）
3. InputCallback 用指针直穿（生产者 LINK 地址赋给消费者输入指针），不依赖对方类型定义

### check_include.py 扫描逻辑

- 非 `Switcher.c` 中检测到全路径 `#include "../include/"` → 阻断
- 模块 .c 引用了非自己的 `_io.h` → 阻断
- 任何 `.h` 文件中检测到 `#include` 其他模块的 `.h` → 阻断

---

## 九、命名约定

数据交换机模块遵守 `01-architecture.md` §4 的统一命名规范。以下为交换机特有的补充：

```
文件:       include/module_a_io.h   module_a.h   module_a.c
类型:       ModuleA_InData_t        ModuleA_OutData_t
函数:       ModuleA_GetIO()         ModuleA_DoWork()
InputCallback 宏:  INPUT_CALLBACK(Consumer)   → 展开 Consumer_InputCallback
InputCallback 槽:  INPUT_GET_SLOT(Producer, Consumer)   → in->Producer_params = &out->Consumer_params
变量:       g_input                 g_output
槽位枚举:   SLOT(ModuleName)         — 宏定义槽位，与模块名对齐
注册入口:   SLOT_GETIO(ModuleName)   — 展开为 ModuleName_GetIO(&s_slot[SLOT_ModuleName]...)
```

### 管道配对命名

```
Producer 输出 LINK 成员名:  {Consumer}_params     (out->ElecParams_params)
Consumer 输入 LINK 成员名:  {Producer}_params     (in->Calculator_params)
```

**双向对称**: 直穿赋值就是 `in->Calculator_params = (void*)out->ElecParams_params`。

当 Producer 有多个 Consumer 时，次管道保留具名（如 `power_direct`），仍遵守 `{Consumer}_params` 优先原则。

### InputCallback 宏速查

| 宏 | 参数 | 检查点 | 展开内容 |
|----|------|--------|---------|
| `INPUT_CALLBACK(consumer)` | 1 | — | `void consumer_InputCallback(void)` |
| `INPUT_GET_SLOT(producer, consumer)` | 2 | ProcessInput | 取 slot 指针 + 直穿赋值 |
| `INPUT_LINK_PULL(producer, consumer, member)` | 3 | 回调内 | 直穿 + null/ST_NEW 检查 |
| `INPUT_EDGE_PULL(producer, consumer, member)` | 3 | 回调内 | 直穿 + ST_OUT 边沿 + 自动清除 |

详见 `09-std-module.md §4` 的完整用法和展开示例。

---

## 十、优势

- **消除 `_LINK` 副本**：模块不需要声明 consumer struct
- **AI 维护量下降**：owner 字段变更 → 只改 Switcher 路由，不改 N 个 consumer
- **数据流集中可见**：所有跨模块路由在 `Switcher_Run()` 一目了然
- **头文件物理隔离**：`_io.h` 统一放 `include/`，全路径可被工具审计
- **PULL 单向调用**：Producer 只写, Switcher 拉, 不产生回调链混乱
- **route 分流**：info.route 字段让 consumer 单入口多分支，不依赖函数名多态

---

## 十一、不适用场景

- ISR 回调（延迟敏感 → 仍走 `__weak`）
- 同槽同步计算（调用即执行 → 仍走 `__weak`）
- 跨进程/跨核通信（需消息持久化 → `Msg_Post`）
- 实时输出（→ `@OUTPUT_CALLBACK` 例外，绕开 Switcher 周期）

---

## 十二、与 interface_map.h 的关系

`interface_map.h` 仍然管 __weak 函数配对 + Switcher 路由文档。数据交换机 PULL 模式引入后：

```
interface_map.h  →  __weak 函数配对 + Switcher 路由表 + @OUTPUT_CALLBACK 白名单
check_weak_pairs.py  →  __weak 签名验证
check_include.py     →  _io.h include 权限审计
check_output_callback.py →  _onOutput/ST_OUT 检测 + @OUTPUT_CALLBACK 白名单验证
```

---

*方法论版本: v2.3, 2026-06-10*
