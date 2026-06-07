---
name: new-module
description: "Interactive wizard to create a new module using std_module.h — data structs + MODULE_SKELETON + ProcessInput + Init + MODULE_EXPORT. Triggers on: new module, add module, create module, new-module, 新增模块, 新建模块, 添加模块."
user-invocable: true
---

# /new-module — 模块创建向导

创建符合 v2.2 架构的新模块（`std_module.h` PULL 模式）。

**核心**: `core/std_module.h` — **模块的骨架宏就是范式**

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
    if (g_input.info.status & ST_NEW) {
        {Module}InData_t *in = ({Module}InData_t *)g_input.para;
        /* 处理输入 */
        g_input.info.status &= ~ST_NEW;
    }

    {Module}OutData_t *out = ({Module}OutData_t *)g_output.para;
    out->result = calc();
    /* 输出: 只写 g_output.para — Switcher 负责路由 */
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

/* ---- Consumer 回调: Switcher 在 Producer DoWork 后显式调用 ---- */
void {Module}_On{Producer}Data(Para_Grp_t *pOut)
{
    memcpy(g_input.para, pOut->para, sizeof({Module}InData_t));
    g_input.info.status |= ST_NEW;
}
```

### 关键规则

| 元素 | 规则 |
|------|------|
| `MODULE_SKELETON()` | 文件顶部调用，展开 `g_input`/`g_output`/`Constructor`/`DoWork` |
| `Init()` | 初始化 `s_in`/`s_out`，绑定 `g_input.para`/`g_output.para` |
| `ProcessInput()` | 三段式：检查 ST_NEW → 消费输入 → 计算 → 写输出 |
| `MODULE_EXPORT({Module})` | 文件底部调用，生成 `GetIO` (不生成 __weak OnOutput) |
| `{Consumer}_On{Producer}Data()` | 每个数据源一个 consumer 回调，memcpy 写入自己的 `g_input.para` |

### 状态位 (`core/std_module.h`)

| 位 | 常量 | 含义 |
|----|------|------|
| bit0 | `ST_INIT (0x01)` | 已初始化（Constructor 置位） |
| bit1 | `ST_NEW (0x02)` | 新输入到达（consumer 回调置位，ProcessInput 消费后自清） |

### route 分流 (info.route)

consumer 的 ProcessInput 可通过 `g_input.info.route` 选择不同的执行路径：

```c
static void ProcessInput(void)
{
    switch (g_input.info.route) {
    case 1: /* 路径A */ break;
    case 2: /* 路径B — @OUTPUT_CALLBACK 即时触发 */ break;
    }
}
```

正常 PULL 路由无需 route，producer 不设 route = 0 即可。`@OUTPUT_CALLBACK` 例外场景下，producer 设 route 值指定 consumer 执行路径。

---

## Step 3: 注册到 Switcher

### 3.1 Switcher 注册

在 `src/core/data_switcher.c` 的 `Switcher_Init()` 添加：

```c
{Module}_GetIO(&pIn, &pOut, &pDoWork);
Switcher_Register(pDoWork, pOut);
```

### 3.2 Switcher 路由

如果本模块是 **producer**，在 `Switcher_Run()` 中添加路由：

```c
/* {Module} DoWork 之后立即路由 */
s_slots[SLOT_{MODULE}].pDoWork();
_route_{module}();  /* 检查 has_* 标志 → 调 consumer 回调 */
```

如果本模块是 **consumer**，添加 consumer 回调并在路由函数中调用。

---

## Step 4: 验证

```bash
python tools/check_deps.py .
python tools/check_include.py .
python tools/check_output_callback.py .
```

## @OUTPUT_CALLBACK 例外

如需绕开 Switcher 周期立即触发 consumer (如蜂鸣器实时反馈)：

1. Producer 写 `g_output.para` + 设 `g_output.info.route`
2. Producer 立即调 consumer 的 DoWork 或回调
3. 添加 `/* @OUTPUT_CALLBACK: <reason> — user confirmed */` 标记
4. 在 `interface_map.h` 白名单注册
5. `check_output_callback.py` 验证通过

**无标记的输出回调 → check_output_callback.py 阻断提交。**
