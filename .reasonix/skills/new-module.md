---
name: new-module
description: "Create a new module using v2.2 std_module.h PULL paradigm — MODULE_SKELETON(name) + ProcessInput + Init + MODULE_EXPORT(name). Triggers on: new module, add module, create module, 新增模块, 新建模块, 添加模块."
---

# /new-module — v2.2 模块创建向导

创建符合 v2.2 PULL 范式的模块（`std_module.h` 模式）。

**核心**: `core/std_module.h`
**范式调用**: `MODULE_SKELETON(name)` → `Init()` → `ProcessInput()` → `MODULE_EXPORT(name)`
**路由**: 中间层覆盖 `{Module}_InputCallback` 强符号注入数据

---

## Step 0: 路径规划 — 先设计 I/O 再编码

先查清所有 I/O 路径：

```bash
# 本模块会被谁调？
grep -rn "Module_Init\|Module_Run\|Module_On" src/main.c src/app/

# 本模块要调谁？(__weak 输出桩)
grep -n "__weak\|__attribute__((weak))" src/{layer}/{module}_v1.c.bak  # 如果是迁移
```

填写 I/O 表：

| 方向 | 来源/去向 | 数据字段 | 类型 |
|------|----------|---------|------|
| 输入 | 生产者A | ... | ... |
| 输出 | 消费者B | ... | ... |

**InData_t / OutData_t 按此表精确设计。**

**单向调用铁律**: 模块只写 `g_output.para` + 置 `ST_OUT`, InputCallback 由中间层调。
模块不定义 `__weak` 输出回调给其他 APP 模块。OutputCallback 保留但一般不用。

---

## Step 1: 模块身份

```
1. 模块名称？(PascalCase, 如 "PowerCalc")
2. 所属层？(app | drv | hal | proto | core)
3. 接收哪些模块的数据？(列出生产者模块名)
```

---

## Step 2: 创建模板

创建 `{layer}/{module}.c`，注意 `MODULE_SKELETON(name)` 的 name 必须与 `MODULE_EXPORT(name)` 一致。

```c
/**
 * @file    {module}.c
 * @brief   一句话描述
 * @layer   {layer}
 *
 * 输入: XYZ (来自谁)
 * 输出: ABC (发给谁)
 *
 * 范式: MODULE_SKELETON({Module})
 *       Init()  → 挂 g_input.para / g_output.para
 *       ProcessInput() → 三段式: 输入→计算→输出
 *       MODULE_EXPORT({Module}) → 生成 {Module}_GetIO
 */
#include "std_module.h"
#include "../include/{module}_io.h"
#include <string.h>

/* ---- 数据结构 ---- */
typedef struct {
    /* 输入字段 */
    uint16_t field1;
    uint8_t  valid;
} {Module}_InData_t;

typedef struct {
    /* 输出字段 */
    uint16_t result;
    uint8_t  ready;
} {Module}_OutData_t;

static {Module}_InData_t  s_in;
static {Module}_OutData_t s_out;

/* ---- 骨架 (name 必须与 MODULE_EXPORT 一致) ---- */
MODULE_SKELETON({Module});

/* ---- Init (首次 DoWork 前自动调用) ---- */
static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

/* ---- ProcessInput (每帧被 DoWork 调) ---- */
static void ProcessInput(void)
{
    /* ====== 输入段: 消费 g_input ====== */
    if (g_input.info.status & ST_NEW) {
        {Module}_InData_t *in = ({Module}_InData_t *)g_input.para;
        /* 处理输入 */
        g_input.info.status &= ~ST_NEW;
    }

    /* ====== 计算段 ====== */
    {Module}_OutData_t *out = ({Module}_OutData_t *)g_output.para;
    out->result = 0;  /* 计算产出 */

    /* ====== 输出段 ====== */
    out->ready = 1;
    g_output.info.status |= ST_OUT;
}

/* ---- 导出 ---- */
MODULE_EXPORT({Module});
```

### 输入回调 (逆序: 先读这个再读上面的模板)

`MODULE_SKELETON({Module})` 自动生成:

```c
/* 弱符号空壳 — 中间层覆盖强符号注入数据 */
__attribute__((weak)) void {Module}_InputCallback(void);

/* 弱符号空壳 — 仅 ProcessInput 中途需要即时输出时覆盖 */
__attribute__((weak)) void {Module}_OutputCallback(Para_Grp_t *pOut);
```

这三个 (SKELETON + InputCallback + OutputCallback) 是在 DoWork 中按序调用的:

```
Module.DoWork:
  ① {Module}_InputCallback()     ← 中间层覆盖强符号, 拉数据入 g_input
  ② ProcessInput()               ← 消费 g_input → 计算 → 写 g_output
  ③ {Module}_OutputCallback()    ← 弱符号, 一般不用, 仅即时场景
```

典型 InputCallback 实现 (在中间层/路由文件中):

```c
void Power_InputCallback(void)
{
    Adc_Output_t  *adc = (Adc_Output_t *)s_slot[SLOT_ADC].pOut->para;
    Power_Input_t *pwr = (Power_Input_t *)s_slot[SLOT_POWER].pIn->para;

    memcpy(pwr->inputValue, adc->inputValue, sizeof(pwr->inputValue));
    s_slot[SLOT_POWER].pIn->info.status |= ST_NEW;
}
```

---

## Step 3: 注册到 Switcher

在 `core/data_switcher.c` 的 `Switcher_Init()` 添加：

```c
#include "../include/{module}_io.h"

{Module}_GetIO(&s_slot[SLOT_{MODULE_UPPER}].pIn,
               &s_slot[SLOT_{MODULE_UPPER}].pOut,
               &s_slot[SLOT_{MODULE_UPPER}].pDoWork);
```

---

## Step 4: 验证

```bash
# 1. 编译通过 (0e0w)
armcc --cpu Cortex-M4 --c99 -I"include" -I"core" -c {layer}/{module}.c

# 2. 层依赖检查
python tools/check_deps.py .

# 3. 范式合规检查 (如果在项目中)
python tools/check_paradigm.py .
```

---

## 命名速查表

| 宏 | name="Power" | name="APP_Adc" |
|----|-------------|----------------|
| `MODULE_SKELETON(name)` | `g_input`, `g_output`, `Power_InputCallback`, `Power_OutputCallback` | `g_input`, `g_output`, `APP_Adc_InputCallback`, `APP_Adc_OutputCallback` |
| `MODULE_EXPORT(name)` | `Power_GetIO(...)` | `APP_Adc_GetIO(...)` |
| `MODULE_IO_H(name)` | `void Power_GetIO(...)` | `void APP_Adc_GetIO(...)` |
