# 09 — std_module.h：统一模块骨架宏 (v2.3 route+Switcher_RunNow)

> **`res[1]` → `route`**: LINK 结构体的保留字段改为 `route`, 与 PARAMS union 配合实现多入口分流。
> **`Switcher_RunNow(slot)`**: 输出段写管道后立即调 consumer DoWork, 见 `08-data-switcher.md §5.6`。

---

## 一、解决什么问题

v2.1 之前，每个模块手动编写：
- `g_in` / `g_out` 自定义 struct（Input_t/Output_t）
- `_Constructor()` 懒惰初始化
- `GetIO()` 暴露指针
- `DoWork()` 三段式结构
- `__weak` 输出桩 + 函数指针绑定

这些样板代码每个模块都要重复，容易出错。`std_module.h` 用两个宏消除重复——**骨架宏就是范式**。

---

## 二、三要素

`#include "std_module.h"` 提供一个模块所需的全部基础设施：

> v2.3 新增: `STD_MODULE_ENABLE_ISR=1` 时 MODULE_SKELETON/MODULE_EXPORT 额外生成 ISR 通路（独立 g_isr_input/g_isr_output + ISR_DoWork）。ISR 默认关闭，零开销。

### 2.1 统一参数组 `Para_Grp_t`

```c
enum {
    ST_INIT  = 0x01,   /* bit0: 已初始化 */
    ST_NEW   = 0x02,   /* bit1: 新输入到达 (中间层在 InputCallback 中置位) */
    ST_OUT   = 0x04,   /* bit2: 输出就绪 (触发 OutputCallback, 一般不用) */
};

typedef struct {
    uint8_t status;   /* 组合 ST_INIT | ST_NEW | ST_OUT */
    uint8_t inMax;    /* 输入 LINK 数量 (MODULE_INPUT 的 LINK 列数) */
    uint8_t route;    /* 路由标识 — 多炉头/多模式时区分数据路径 */
    uint8_t outMax;   /* 输出 LINK 数量 (MODULE_OUTPUT 的 LINK 列数) */
} Info_Header;

typedef struct {
    Info_Header info;
    void       *para;    /* 指向模块自己的数据结构 */
} Para_Grp_t;
```

每个模块不再自定义 `Input_t`/`Output_t`，而是统一用 `Para_Grp_t` 包装。实际数据挂在 `void *para` 后面。

### 2.2 `MODULE_SKELETON(name)` — 声明骨架

展开为：
- `g_input` / `g_output` (Para_Grp_t)
- `g_init_done` 标志
- `Init()` / `ProcessInput()` 前向声明
- `{Module}_InputCallback()` — **弱符号空壳**，中间层覆盖强符号注入数据（Primary）
- `{Module}_OutputCallback(Para_Grp_t *pOut)` — **弱符号**，一般不用，仅即时场景（Exception）
- `Constructor()` — memset + 调 `Init()` + 置 `ST_INIT`
- `DoWork()` — 懒惰构造 → `{Module}_InputCallback()` → `ProcessInput()` → `{Module}_OutputCallback(&g_output)`

### 2.3 `MODULE_EXPORT(module_name)` — 导出接口

展开为：
- `{Module}_GetIO(pIn, pOut, pDoWork)` — 暴露主循环指针 + DoWork 函数指针
- `{Module}_GetISR_IO(pIn, pOut, pDoWork)` — 仅 STD_MODULE_ENABLE_ISR=1 时生成，暴露 ISR 通路指针（独立 g_isr_input/g_isr_output + ISR_DoWork）

---

## 三、模块标准结构

> **编码前先做数据契约**: 在 `docs/data-contract.md` 中定义模块 I/O 结构体对齐，见 `10-data-contract.md`。确保上下游模块的输入输出结构体布局一致，可实现指针直穿（零拷贝）。

> **🚫 头文件自治规则**: 任何模块的 `.h`（包括 `_io.h` 和普通 `.h`）**不得 `#include` 其他模块的 `.h`**。需要引用其他模块的数据类型时，在 `_io.h` 中自包含定义自己的输入结构体，布局与生产者输出兼容。InputCallback 通过指针直穿访问数据，不依赖对方类型定义。

### 3.1 I/O 结构体 — 三层定义 (v2.3 LINK+PARAMS + route)

**管道配对模式**: Consumer 定义自己的 INPUT 类型 (自包含, 不引用 Producer), 布局与 Producer OUTPUT 一致。中间层用 `(void*)` 连接。

**v2.3 route 机制**: LINK 的 `res[1]` 字段改为 `route`, 用于区分同一管道的不同子功能入口。与 `Info_Header.route` 配合, producer 设 route, consumer 在 ProcessInput 中 switch 分流。PARAMS 用 union 容纳多个 route 的 req/resp 结构, 实现"一次 DoWork + route 分流"的即时调用模式。

```c
/* ===== {module}_io.h ===== */

/* --- 输入: 从 Producer 接收 (自包含, 布局兼容) --- */

/* 数据参数: 字段顺序/类型/大小与 Producer OUTPUT_PARAMS 一致 */
typedef struct {
    uint16_t field1;
    uint16_t field2;
    uint8_t  valid;
    uint8_t  res[3];
} MODULE_INPUT_PARAMS(Producer, {Module});

/* 输入管道: 与 MODULE_OUTPUT_LINK(Producer, {Module}) 配对 */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  seq;            /* 更新有效性 — 消费者比对 last_seq 判断新数据 */
    uint8_t  route;          /* 路由标识 — 区分同一 LINK 的不同子功能入口 */
    MODULE_INPUT_PARAMS(Producer, {Module}) params[POT_MAX];
} MODULE_INPUT_LINK(Producer, {Module});

/* 输入聚合: InputCallback 直穿赋值 input 指针 */
typedef struct {
    MODULE_INPUT_LINK(Producer, {Module})* input;   // ← 指针
} MODULE_INPUT({Module});

/* --- 输出: 发给 Consumer --- */

/* 数据参数 — 多 route 用 union 区分 req/resp 结构 */
typedef struct {
    uint8_t  route;           /* 0=功能A, 1=功能B ... 与 LINK.route 一致 */
    union {
        struct {              /* route=0: req+resp 成对定义 */
            uint16_t param;   /* → 请求参数 */
            uint8_t  result;  /* ← 回传结果 (Switcher_RunNow 后 producer 读) */
        } func_a;
        struct {              /* route=1 */
            uint8_t  cmd;
            uint8_t  ack;
        } func_b;
    };
} MODULE_OUTPUT_PARAMS({Module}, Consumer);

/* 输出管道 */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  seq;
    uint8_t  route;           /* 替代 res[1], 语义明确 — AI 看见即理解 */
    MODULE_OUTPUT_PARAMS({Module}, Consumer) *params;  /* 指针指向 PARAMS */
} MODULE_OUTPUT_LINK({Module}, Consumer);

/* 输出聚合 */
typedef struct {
    MODULE_OUTPUT_LINK({Module}, Consumer) *{Consumer}_params;   // ← 以消费者命名 (指针)
} MODULE_OUTPUT({Module});
```

**AI 自理解要点**:

| 命名 | 含义 | AI 推理 |
|------|------|---------|
| `route` | 路由选择 | AI 见到 `route` → 在 consumer 端找 switch(route) |
| `seq` | 更新序列号 | AI 见到 `seq++` → 在 consumer 端找 `cur_seq != last_seq` |
| `union { func_a, func_b }` | 多入口 | AI 见到 union → 每种 route 有自己的参数字段 |

### 3.2 源文件骨架
#include "std_module.h"
#include "{module}.h"     /* 可选 — 私有常量 */
#include <string.h>

/* ---- 本模块的数据结构 (挂在 Para_Grp_t.para 后面) ---- */
typedef struct {
    uint8_t  field;
    uint16_t value;
} MODULE_INPUT({Module});

typedef struct {
    uint8_t  head;
    uint16_t result;
} MODULE_OUTPUT({Module});

static {Module}_Input  s_in;
static {Module}_Output s_out;

/* ---- 骨架: name 必须与 MODULE_EXPORT 一致 ---- */
MODULE_SKELETON({Module});

/* ---- 初始化: 绑定数据指针 (Constructor 自动调用) ---- */
static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

/* ---- 处理逻辑 (DoWork 每帧调) — v2.3 使用数据层 status ---- */
static void ProcessInput(void)
{
    {Module}_Input  *in  = ({Module}_Input *)g_input.para;
    {Module}_Output *out = ({Module}_Output *)g_output.para;

    /* ====== 输入段 ====== */
    /* v2.3: status 在数据层 LINK 结构体首字节, 不在 g_input.info.status */
    /* 单输入 LINK: 直接检查 LINK 成员首字节的 status */
    if (in->input->status & ST_NEW) {
        in->input->status &= ~ST_NEW;
        /* 消费 in->xxx */
    }

    /* 多输入 LINK: 每个 LINK 独立检查 */
    // if (in->adc.status  & ST_NEW) { ... in->adc.status  &= ~ST_NEW; }
    // if (in->comm.status & ST_NEW) { ... in->comm.status &= ~ST_NEW; }

    /* ====== 计算段 ====== */
    out->result = calc(in);

    /* ====== 输出段 ====== */
    /* v2.3: 置输出 LINK 的 status, 由 Switcher 或下游模块读取 */
    out->{Consumer}_params->status |= ST_NEW;

    /* v2.3 route + Switcher_RunNow: 写管道, 设 route, 即刻调消费者 */
    out->{Consumer}_params->seq++;
    out->{Consumer}_params->route = 0;  /* 标识子功能入口 */
    g_output.info.status |= ST_OUT;
    Switcher_RunNow(SLOT_Consumer);     /* 即刻执行消费者 DoWork */
    uint8_t result = out->{Consumer}_params->params->result;  /* 读回传 */
}

/* ---- 导出: GetIO ---- */
MODULE_EXPORT({Module});
```

---

## 四、数据流 (v2.3 PULL — InputCallback 宏 + 命名约定)

### 4.1 管道配对命名约定 (v2.3)

```
Producer 输出 LINK 成员名:  {Consumer}_params     (out->ElecParams_params)
Consumer 输入 LINK 成员名:  {Producer}_params     (in->Calculator_params)
```

```
typedef struct {
    MODULE_OUTPUT_LINK(Calculator, ElecParams) *ElecParams_params;   // ← 以消费者命名 (指针)
    MODULE_OUTPUT_LINK(Calculator, PowerBase)  power_direct;        // ← 次管道保留具名
} MODULE_OUTPUT(Calculator);

typedef struct {
    MODULE_INPUT_LINK(Calculator, ElecParams)* Calculator_params;   // ← 以生产者命名
} MODULE_INPUT(ElecParams);
```

Consumer 的输入指针命名 = Producer 名，Producer 的输出 LINK 命名 = Consumer 名。双向对称，直穿赋值就是 `in->Calculator_params = (void*)out->ElecParams_params`。

### 4.2 InputCallback 宏

`data_switcher.h` 提供以下宏，覆盖所有 InputCallback 模式：

```c
/* 函数壳: INPUT_CALLBACK(Telemetry) { ... }
 * 展开: void Telemetry_InputCallback(void) { ... }
 * 参数: 仅 consumer 名 — 一个 consumer 一个回调函数 */
#define INPUT_CALLBACK(consumer) \
    void consumer##_InputCallback(void)

/* 取 slot 指针 + 直穿: 检查放 ProcessInput
 * 参数: producer, consumer */
#define INPUT_GET_SLOT(producer, consumer) \
    MODULE_OUTPUT(producer) *__out = \
        (MODULE_OUTPUT(producer) *)s_slot[SLOT(producer)].pOut->para; \
    MODULE_INPUT(consumer)   *__in  = \
        (MODULE_INPUT(consumer)   *)s_slot[SLOT(consumer)].pIn->para; \
    __in->producer##_params = (void*)__out->consumer##_params

/* 单管道直穿+回调内检查 (ST_NEW 触发) */
#define INPUT_LINK_PULL(producer, consumer, link_member) \
    do { \
        MODULE_OUTPUT(producer) *__p_out = (MODULE_OUTPUT(producer) *)s_slot[SLOT(producer)].pOut->para; \
        if (__p_out && (__p_out->link_member.status & ST_NEW)) { \
            MODULE_INPUT(consumer) *__p_in = (MODULE_INPUT(consumer) *)s_slot[SLOT(consumer)].pIn->para; \
            if (__p_in) { \
                __p_in->producer##_params = (void*)__p_out->link_member; \
                s_slot[SLOT(consumer)].pIn->info.status |= ST_NEW; \
            } \
        } \
    } while (0)

/* 单管道边沿触发 (ST_OUT, 自动清除实现 0→1 边沿检测) */
#define INPUT_EDGE_PULL(producer, consumer, link_member) \
    do { \
        MODULE_OUTPUT(producer) *__p_out = (MODULE_OUTPUT(producer) *)s_slot[SLOT(producer)].pOut->para; \
        if (__p_out && (__p_out->link_member.status & ST_OUT)) { \
            __p_out->link_member.status &= ~ST_OUT; \
            MODULE_INPUT(consumer) *__p_in = (MODULE_INPUT(consumer) *)s_slot[SLOT(consumer)].pIn->para; \
            if (__p_in) { \
                __p_in->producer##_params = (void*)__p_out->link_member; \
                s_slot[SLOT(consumer)].pIn->info.status |= ST_NEW; \
            } \
        } \
    } while (0)
```

### 4.3 InputCallback 写法

**写法 A — 单管道直穿 (推荐, 检查放 ProcessInput):**

```c
INPUT_CALLBACK(ElecParams)
{
    INPUT_GET_SLOT(Calculator, ElecParams);
}
```

展开:

```c
void ElecParams_InputCallback(void) {
    Calculator_Output *out = (Calculator_Output *)...;
    ElecParams_Input  *in  = (ElecParams_Input  *)...;
    in->Calculator_params = (void*)&out->ElecParams_params;
}
```

**写法 B — 多管道组合 (一个回调接收多个数据源):**

```c
INPUT_CALLBACK(Telemetry)
{
    /* 从 Calculator 直穿拉数据 */
    INPUT_GET_SLOT(Calculator, Telemetry);

    /* 从 ElecParams 直穿拉数据 */
    INPUT_GET_SLOT(ElecParams, Telemetry);
}
```

**写法 C — 管道直穿+回调内检查 (INPUT_LINK_PULL):**

```c
INPUT_CALLBACK(ElecParams)
{
    INPUT_LINK_PULL(Calculator, ElecParams, ElecParams_params);
}
```

### 4.4 数据流图 (v2.3)

```
模块名 = "ElecParams" 的实例:

Module.DoWork:
  ① ElecParams_InputCallback()    ← INPUT_CALLBACK(ElecParams) 宏展开
      └─ INPUT_GET_SLOT(Calculator, ElecParams)
         └─ in->Calculator_params = (void*)out->ElecParams_params
            └─ 指针直穿, 零拷贝, 别名 = Calculator 的 ElecParams_params LINK

  ② ProcessInput()           ← 消费 g_input → 计算 → 写 g_output
     ├─ 输入段: 检查 in->Calculator_params->status & ST_NEW
     ├─ 计算段: 业务逻辑
     └─ 输出段: 写 out->PowerBase_params / {Consumer}_params.status |= ST_OUT
```

**关键:**
- **单向调用原则**: 模块不定义 `__weak` 输出给其他 APP 模块, 只写 `g_output.para`
- **InputCallback 是数据入口单点**: INPUT_GET_SLOT 直穿赋值, 零拷贝
- **INPUT_CALLBACK(consumer)**: 一个 consumer 一个回调函数, 内部放多个 INPUT_GET_SLOT
- **OutputCallback 是例外**: 仅即时场景（蜂鸣器反馈等）使用, 一般不用

---

## 五、Switcher 集成

```c
// data_switcher.c
typedef struct {
    void (*pDoWork)(void);
    Para_Grp_t *pOut;           /* 模块的 g_output 指针 */
} ModuleSlot_t;

static ModuleSlot_t s_slots[16];
static uint8_t      s_count = 0;

void Switcher_Register(void (*pDoWork)(void), Para_Grp_t *pOut)
{
    if (s_count < 16) {
        s_slots[s_count].pDoWork = pDoWork;
        s_slots[s_count].pOut    = pOut;
        s_count++;
    }
}

void Switcher_Init(void)
{
    Para_Grp_t *pIn, *pOut;
    void (*pDoWork)(void);

    AppCooking_GetIO(&pIn, &pOut, &pDoWork);
    Switcher_Register(pDoWork, pOut);

    AppPower_GetIO(&pIn, &pOut, &pDoWork);
    Switcher_Register(pDoWork, pOut);

    // ... 所有模块
}

void Switcher_Run(void)
{
    /* Phase 1+2 交错: Producer DoWork → 路由 → Consumer DoWork */

    /* AppCommMgr: producer */
    s_slots[SLOT_COMM_MGR].pDoWork();
    _route_comm_mgr();  /* 检查 has_reg → AppPower_OnCommMgrData(pOut) */

    /* AppPower: consumer + producer */
    s_slots[SLOT_POWER].pDoWork();

    /* ... 按调度顺序继续 */
}
```

**路由函数模板**:
```c
static void _route_comm_mgr(void)
{
    Para_Grp_t *pOut = s_slots[SLOT_COMM_MGR].pOut;
    if (!pOut || !pOut->para) return;
    uint8_t *d = (uint8_t *)pOut->para;
    if (!d[0]) return;  /* has_reg flag */
    AppPower_OnCommMgrData(pOut);
}
```

---

## 六、状态位速查

| 位 | 常量 | 谁置位 | 谁清零 | 含义 |
|----|------|--------|--------|------|
| bit0 | `ST_INIT 0x01` | Constructor | — | 模块已构造 |
| bit1 | `ST_NEW 0x02` | InputCallback (中间层) | ProcessInput 消费后 | 新输入到达 |
| bit2 | `ST_OUT 0x04` | ProcessInput 输出段 | Switcher 路由后 | 有输出待消费 |

---

## 七、与 __weak 的共存

一个模块可以同时有两条入口：

```
模块 X:
  ├── DoWork() ← Switcher 每周期调用 (ProcessInput 消费 g_input.para)
  └── ModuleX_OnISR() ← __weak 强符号，ISR 独立入口
```

两条路径互不冲突。ISR 不走 Switcher，直接操作寄存器。

---

## 八、迁移路径 (v1.x → v2.2)

### 8.1 铁则: 先改名备份, 不在原文件空改

**禁止直接修改原模块文件。禁止在原文件加 MODULE_SKELETON。**

原因是:
| 问题 | 后果 |
|------|------|
| 几千行大文件中间插骨架宏 | 宏展开位置不对导致 `g_input`/`g_output` 未定义 |
| 旧 `g_in`/`g_out` 和范式 `g_input`/`g_output` 同名异义 | 变量名极易混淆, 该读 new 时读了 old |
| 旧 `Module_DoWork` 和 `DoWork` 共存 | 不知道哪个是入口 |
| 手动 GetIO 和 MODULE_EXPORT 冲突 | 链接错误 |

正确流程:

```bash
# Step 0: 改名备份旧文件 (禁止在原文件修改)
mv src/{layer}/{module}.c src/{layer}/{module}_v1.c.bak

# Step 0b: 如果有旧 .h, 也改名
mv include/{module}.h include/{module}_v1.h.bak

# Step 0c: 用 new-module SKILL 生成空范模板
#     得到: src/{layer}/{module}.c (空白模板, 含 MODULE_SKELETON)

# Step 0d: 从备份逐段搬功能到模板 (搬一段测一段)

# Step 0e: 功能等价后删备份
rm src/{layer}/{module}_v1.c.bak
```

> **详见 `/modify-module` SKILL — 这是唯一合法的迁移流程。**

### 8.2 迁移步骤 (搬入功能后)

1. `#include "std_module.h"` 替换手动声明的 `g_in`/`g_out`
2. `MODULE_SKELETON(name)` 替换手动 `Constructor()`/`DoWork()` 骨架
3. `Init()` 绑定 `g_input.para` / `g_output.para`
4. `ProcessInput()` 搬入原 `DoWork()` 的三段式逻辑
5. `MODULE_EXPORT(Module)` 替换手动 `GetIO()`
6. 删除旧的 `__weak OnOutput` 桩 + constructor 注册
7. Switcher 注册 + InputCallback 强符号实现 (中间层)

### 8.3 验证

```bash
python ../.claude/tools/check_paradigm.py .   # MODULE_SKELETON + MODULE_EXPORT 配对
python ../.claude/tools/check_output_callback.py .  # 无未批准的 _onOutput/ST_OUT
python ../.claude/tools/check_deps.py .       # 层依赖
```

---

## 九、v2.3 新增: ISR 实时通路 (PendSV)

### 9.1 编译开关

```c
#define STD_MODULE_ENABLE_ISR  1     // .c 文件顶部，包含 std_module.h 之前
#include "std_module.h"
```

默认 `STD_MODULE_ENABLE_ISR=0`，宏展开与 v2.2 完全一致，零开销。

### 9.2 ISR 模块骨架变化

ISR=1 时 `MODULE_SKELETON` 额外生成：
- `g_isr_input` / `g_isr_output` — 独立槽位，不与主循环冲突
- `ISR_ProcessInput()` — 模块实现
- `{Module}_ISR_InputCallback()` / `{Module}_ISR_OutputCallback()` — weak 空壳
- `ISR_DoWork()` — PendSV 入口（`!g_init_done` 时跳过，不负责初始化）
- `g_isr_busy` — 重入保护

### 9.3 ISR 模块标准结构

```c
#define STD_MODULE_ENABLE_ISR  1
#include "std_module.h"
#include "pendsv_switcher.h"      /* g_isr_source, ISR_SOURCE_* */
#include <string.h>

/* ---- 数据结构 ---- */
typedef struct { /* ... */ } MODULE_INPUT(MyISR);
typedef struct { /* ... */ } MODULE_OUTPUT(MyISR);
static MyISR_Input  s_in;
static MyISR_Output s_out;

MODULE_SKELETON(MyISR);

static void Init(void) {
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para       = &s_in;     /* 主循环通路 */
    g_output.para      = &s_out;
    g_isr_input.para   = &s_in;     /* ISR 通路 */
    g_isr_output.para  = &s_out;
}

/* ---- ISR 三段式 (PendSV 中执行) ---- */
static void ISR_ProcessInput(void) {
    MyISR_Input  *in  = (MyISR_Input  *)g_isr_input.para;
    MyISR_Output *out = (MyISR_Output *)g_isr_output.para;
    /* 输入段: 收集数据 */
    /* 计算段: 根据 g_isr_source 区分处理 */
    /* 输出段: 置 ST_OUT */
}

/* ---- 主循环三段式 (可选, 无 ISR 需求可留空) ---- */
static void ProcessInput(void) { }

/* ---- 导出 (同时生成 GetIO + GetISR_IO) ---- */
MODULE_EXPORT(MyISR);
```

### 9.4 PendSV 路由

**data_switcher.c**（主循环调度）不变，只注册 `GetIO`。

**pendsv_switcher.c**（ISR 通路调度）单独注册 `GetISR_IO`：

```c
#if STD_MODULE_ENABLE_ISR
#include "pendsv_switcher.h"

void Switcher_Init_ISR(void) {
    Switcher_PendSV_Init();

    Para_Grp_t *pIn, *pOut;
    void (*pISR_DoWork)(void);
    MyISR_GetISR_IO(&pIn, &pOut, &pISR_DoWork);
    Switcher_RegisterISRModule(pISR_DoWork);
    /* ... 更多 ISR 模块 */
}
#endif
```

### 9.5 ISR 触发 (isr_triggers.c)

```c
void ADC_IRQHandler(void) {
    /* 清标志 */
    Switcher_TriggerPendSV(ISR_SOURCE_ADC);
}

void TIM1_IRQHandler(void) {
    /* 清标志 */
    Switcher_TriggerPendSV(ISR_SOURCE_TIMER);
}
```

ISR 只做两件事: 清标志 + `Switcher_TriggerPendSV`。所有业务逻辑移到 PendSV_Handler 中。

### 9.6 设计约束

| 规则 | 原因 |
|------|------|
| ISR 通路不负责初始化 | `ISR_DoWork` 在 `!g_init_done` 时返回，避免与主循环竞态 |
| 主循环和 ISR 数据独立 | `g_input`/`g_output` 与 `g_isr_input`/`g_isr_output` 分开，无需锁 |
| PendSV 优先级最低 | 在所有 ISR 完成后才执行，天然不嵌套 |
| 一个模块的 ISR 和主循环可共享同一结构体 | Init 中 `g_input.para = g_isr_input.para` 自选 |

---

*方法论版本: v2.3, 2026-06-10*
