---
name: new-module
description: "Interactive wizard to create a new module using std_module.h — data structs + MODULE_SKELETON + ProcessInput + Init + MODULE_EXPORT. Triggers on: new module, add module, create module, new-module, 新增模块, 新建模块, 添加模块."
---

# /new-module — 模块创建向导

创建符合 v2.0 架构的新模块（`std_module.h` 模式）。

**核心**: `core/std_module.h` — **模块的骨架宏就是范式**
**Project**: four_head (SC32L14T, Cortex-M0+, ARMCC V5.06)
**Layers**: `app` | `drv` | `hal` | `proto` | `core`

---

## Step 1: 模块身份

```
1. 模块名称？(snake_case，如 "power_ctrl")
2. 所属层？(app | drv | hal | proto | core)
3. 接收哪些模块的数据？(列出生产者模块名)
```

---

## Step 2: 创建模板

创建 `src/{layer}/{module}.c`：

```c
/**
 * @file    {module}.c
 * @brief   一句话描述
 * @layer   {layer}
 *
 * 输入: XYZ(来自谁)
 * 输出: ABC(发给谁)
 */
#include "core/std_module.h"
#include "{module}.h"     /* 可选 — 私有 #define */
#include <string.h>

/* ---- 数据结构 ---- */
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

/* ---- 骨架 ---- */
MODULE_SKELETON();

/* ---- 处理逻辑（每帧被调）---- */
static void ProcessInput(void)
{
    if (g_input.info.status & 0x02) {
        {Module}InData_t *in = ({Module}InData_t *)g_input.para;
        /* 处理输入 */
        g_input.info.status &= ~0x02;
    }

    {Module}OutData_t *out = ({Module}OutData_t *)g_output.para;
    out->result = calc();
    g_output.info.status |= 0x04;
}

/* ---- 初始化 ---- */
static void Init(void)
{
    memset(&s_in, 0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

/* ---- 导出 ---- */
MODULE_EXPORT({Module});

/* ---- 强符号：接收 {Producer} 的输出 ---- */
void {Producer}_OnOutput(Para_Grp_t *pOut)
{
    memcpy(g_input.para, pOut->para, sizeof({Module}InData_t));
    g_input.info.status |= 0x02;
}
```

---

## Step 3: 注册到 Switcher

在 `src/core/data_switcher.c` 的 `Switcher_Init()` 添加：

```c
{Module}_GetIO(&pIn, &pOut, &pWork);
Switcher_Register(pWork);
```

---

## Step 4: 验证

```bash
python tools/check_deps.py src
```
