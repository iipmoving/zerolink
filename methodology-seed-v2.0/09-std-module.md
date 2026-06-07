# 09 — std_module.h：统一模块骨架宏

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

`#include "core/std_module.h"` 提供一个模块所需的全部基础设施：

### 2.1 统一参数组 `Para_Grp_t`

```c
enum {
    ST_INIT  = 0x01,   /* bit0: 已初始化 (Constructor 置位) */
    ST_NEW   = 0x02,   /* bit1: 新输入到达 (consumer 回调置，ProcessInput 消费后自清) */
};

typedef struct {
    uint8_t status;   /* ST_INIT, ST_NEW */
    uint8_t res1;
    uint8_t route;    /* 路由标识 */
    uint8_t res2;
} Info_Header;

typedef struct {
    Info_Header info;
    void       *para;    /* 指向模块自己的数据结构 */
} Para_Grp_t;
```

每个模块不再自定义 `Input_t`/`Output_t`，而是统一用 `Para_Grp_t` 包装。实际数据挂在 `void *para` 后面。

### 2.2 `MODULE_SKELETON()` — 声明骨架

展开为：
- `g_input` / `g_output` (Para_Grp_t)
- `g_init_done` 标志
- `Init()` / `ProcessInput()` 前向声明
- `Constructor()` — memset + 调 `Init()` + 置 `ST_INIT`
- `DoWork()` — 懒惰构造 → `ProcessInput()`

### 2.3 `MODULE_EXPORT(module_name)` — 导出接口

展开为：
- `{Module}_GetIO(pIn, pOut, pDoWork)` — 暴露指针 + DoWork 函数指针

---

## 三、模块标准结构

```c
#include "core/std_module.h"
#include "{module}.h"     /* 可选 — 私有常量 */
#include <string.h>

/* ---- 本模块的数据结构 (挂在 Para_Grp_t.para 后面) ---- */
typedef struct {
    uint8_t  field;
    uint16_t value;
} {Module}InData_t;

typedef struct {
    uint8_t  head;
    uint16_t result;
} {Module}OutData_t;

static {Module}InData_t  s_in;
static {Module}OutData_t s_out;

/* ---- 骨架: 展开 g_input/g_output/Constructor/DoWork ---- */
MODULE_SKELETON();

/* ---- 初始化: 绑定数据指针 (Constructor 自动调用) ---- */
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
    {Module}InData_t  *in  = ({Module}InData_t *)g_input.para;
    {Module}OutData_t *out = ({Module}OutData_t *)g_output.para;

    /* ====== 输入段 ====== */
    if (g_input.info.status & ST_NEW) {
        /* 消费 in->xxx */
        g_input.info.status &= ~ST_NEW;
    }

    /* ====== 计算段 ====== */
    out->result = calc(in);

    /* ====== 输出段 ====== */
    /* 只写 g_output.para — Switcher 负责路由 */
}

/* ---- 导出: GetIO ---- */
MODULE_EXPORT({Module});

/* ---- Consumer 回调: Switcher 在 Producer DoWork 后显式调用 ---- */
void {Module}_On{Producer}Data(Para_Grp_t *pOut)
{
    memcpy(g_input.para, pOut->para, sizeof({Module}InData_t));
    g_input.info.status |= ST_NEW;
}
```

---

## 四、数据流 (v2.2 PULL)

```
Producer DoWork():
  1. ProcessInput() 消费 g_input.para
  2. 计算 → 写 g_output.para (不置任何状态位)

Switcher 显式路由:
  检查 producer g_output.para 的 has_* 标志
  → 调 consumer 回调: {Consumer}_On{Producer}Data(producer_pOut)

Consumer 回调 {Consumer}_On{Producer}Data(pOut):
  1. memcpy(g_input.para, pOut->para, sizeof(InData_t))  ← consumer 自拷贝
  2. g_input.info.status |= ST_NEW                       ← 通知自己的 DoWork

Consumer DoWork():
  1. ProcessInput 检查 g_input.info.status & ST_NEW
  2. 消费 → g_input.info.status &= ~ST_NEW
  3. 计算 → 写 g_output.para
```

**关键**: 输出不推，Switcher 拉。Producer 只写 g_output.para，不设 ST_OUT，不调 _onOutput。Switcher 在 DoWork 调用之间显式检查标志并路由。

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
| bit1 | `ST_NEW 0x02` | consumer 回调 | ProcessInput 消费后 | 新输入到达 |

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

1. `#include "core/std_module.h"` 替换手动声明的 `g_in`/`g_out`
2. `MODULE_SKELETON()` 替换手动 `Constructor()`/`DoWork()` 骨架
3. `Init()` 绑定 `g_input.para` / `g_output.para`
4. `ProcessInput()` 搬入原 `DoWork()` 的三段式逻辑
5. `MODULE_EXPORT(Module)` 替换手动 `GetIO()`
6. 删除旧的 `__weak OnOutput` 桩 + constructor 注册
7. Switcher 添加显式路由调用 consumer 回调

详见 `/modify-module` SKILL。

---

*方法论版本: v2.2, 2026-06-06*
