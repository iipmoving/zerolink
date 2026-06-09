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
} {Module}_InData_t;

typedef struct {
    uint8_t  head;
    uint16_t result;
} {Module}_OutData_t;

static {Module}_InData_t  s_in;
static {Module}_OutData_t s_out;

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

/* ---- 处理逻辑 (DoWork 每帧调) ---- */
static void ProcessInput(void)
{
    {Module}_InData_t  *in  = ({Module}_InData_t *)g_input.para;
    {Module}_OutData_t *out = ({Module}_OutData_t *)g_output.para;

    /* ====== 输入段 ====== */
    if (g_input.info.status & ST_NEW) {
        /* 消费 in->xxx */
        g_input.info.status &= ~ST_NEW;
    }

    /* ====== 计算段 ====== */
    out->result = calc(in);

    /* ====== 输出段 ====== */
    /* 只写 g_output.para + 置 ST_OUT — Switcher 负责路由 */
    g_output.info.status |= ST_OUT;
}

/* ---- 导出: GetIO ---- */
MODULE_EXPORT({Module});
```

---

## 四、数据流 (v2.2 PULL — InputCallback 主路由)

```
模块名 = "Power" 的实例:

Module.DoWork:
  ① Power_InputCallback()    ← 弱符号空壳, 中间层覆盖强符号注入数据
     ├─ (weak) 默认空函数 — 无输入时不做事
     └─ (strong) 中间层覆盖: 读 s_slot[SLOT_ADC].pOut->para,
                     写入 s_slot[SLOT_POWER].pIn->para, 置 ST_NEW

  ② ProcessInput()           ← 消费 g_input → 计算 → 写 g_output
     ├─ 输入段: 检查 ST_NEW, 读 g_input.para
     ├─ 计算段: 业务逻辑
     └─ 输出段: 写 g_output.para + 置 ST_OUT

  ③ Power_OutputCallback(&g_output)  ← 弱符号, 一般不用
     └─ (weak) 默认空函数 — 仅 ProcessInput 中途需要即时输出时覆盖
```

**关键**:
- **单向调用原则**: 模块不定义 `__weak` 输出给其他 APP 模块, 只写 `g_output.para`
- **InputCallback 是主路由**: 中间层覆盖强符号拉数据, 模块本身不知道数据来源
- **OutputCallback 是例外**: 仅即时场景（蜂鸣器反馈等）使用, 且一般不用
- **无 `{Module}_On{Producer}Data` 模式**: v2.2 废弃了 Switcher 显式调 consumer 回调的模式, 统一为 InputCallback 弱符号

典型 InputCallback 实现 (在中间层/路由文件中):

```c
void Power_InputCallback(void)
{
    Adc_Output_t  *adc = (Adc_Output_t *)s_slot[SLOT_ADC].pOut->para;
    Power_Input_t *pwr = (Power_Input_t *)s_slot[SLOT_POWER].pIn->para;

    memcpy(pwr, adc->measured_data, sizeof(pwr->measured_data)); /* 或逐字段赋值 */
    s_slot[SLOT_POWER].pIn->info.status |= ST_NEW;
}
```

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

1. `#include "core/std_module.h"` 替换手动声明的 `g_in`/`g_out`
2. `MODULE_SKELETON(name)` 替换手动 `Constructor()`/`DoWork()` 骨架
3. `Init()` 绑定 `g_input.para` / `g_output.para`
4. `ProcessInput()` 搬入原 `DoWork()` 的三段式逻辑
5. `MODULE_EXPORT(Module)` 替换手动 `GetIO()`
6. 删除旧的 `__weak OnOutput` 桩 + constructor 注册
7. Switcher 注册 + InputCallback 强符号实现 (中间层)

### 8.3 验证

```bash
python tools/check_paradigm.py .   # MODULE_SKELETON + MODULE_EXPORT 配对
python tools/check_output_callback.py .  # 无未批准的 _onOutput/ST_OUT
python tools/check_deps.py .       # 层依赖
```

---

*方法论版本: v2.2, 2026-06-06*
