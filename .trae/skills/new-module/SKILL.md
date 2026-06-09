---
name: new-module
description: "Interactive wizard to create a new module using std_module.h — data structs + MODULE_SKELETON + ProcessInput + Init + MODULE_EXPORT. Triggers on: new module, add module, create module, new-module, 新增模块, 新建模块, 添加模块."
user-invocable: true
---

# /new-module — 模块创建向导

创建符合 v2.2 架构的新模块（`std_module.h` PULL 模式）。

**核心**: `core/std_module.h` — **模块的骨架宏就是范式**

**数据流向管理**: `data_flow_table.json` — 统一维护模块间数据流向

---

## 前置步骤：数据流向表规划

### 0.1 数据流向表格式

项目根目录下的 `data_flow_table.json` 用于维护所有模块的输入输出关系：

```json
{
  "project": "four_head",
  "version": "2.2",
  "modules": [
    {
      "name": "app_power",
      "layer": "app",
      "inputs": [
        {"source": "app_adc", "slot": "SLOT_APP_ADC", "fields": ["current", "voltage"]},
        {"source": "app_comm", "slot": "SLOT_APP_COMM", "fields": ["target_power"]}
      ],
      "outputs": [
        {"target": "app_display", "slot": "SLOT_APP_DISPLAY", "fields": ["ppg_delta"]}
      ]
    }
  ]
}
```

### 0.2 数据流向管理工具

```bash
# 初始化数据流向表
python tools/data_flow_manager.py init <project_name>

# 添加模块及其输入输出
python tools/data_flow_manager.py add app_power --layer app \
    --input app_adc --input app_comm \
    --output app_display

# 验证数据流向一致性（层依赖、循环依赖等）
python tools/data_flow_manager.py validate

# 生成代码
python tools/data_flow_manager.py generate link      # 生成 Link 函数
python tools/data_flow_manager.py generate switcher  # 生成 Switcher 注册
python tools/data_flow_manager.py generate io --module app_power  # 生成 IO 头文件

# 查看数据流向表
python tools/data_flow_manager.py print
```

### 0.3 验证规则

| 规则 | 说明 |
|------|------|
| 输入来源必须存在 | 禁止引用未定义的模块 |
| 输出目标必须存在 | 禁止输出到未定义的模块 |
| 层依赖顺序 | core → proto → hal → drv → app（上层可依赖下层，反之禁止） |
| 无循环依赖 | A→B→C→A 是非法的 |

---

## Step 1: 模块身份

```
1. 模块名称？(snake_case，如 "power_ctrl")
2. 所属层？(app | drv | hal | proto | core)
3. 接收哪些模块的数据？(列出生产者模块名)
4. 输出给哪些模块？(列出消费者模块名)
```

---

## Step 2: I/O 接口设计（xxx_io.h）

### 2.1 三层分离原则

```
┌─────────────────────────────────────────────────────────┐
│  框架层 (std_module.h)                                  │
│  Para_Grp_t { Info_Header info; void *para; }        │
│  → g_input / g_output 容器（status/route 在这层）     │
└─────────────────────────────────────────────────────────┘
                         ↑ para 指针绑定
┌─────────────────────────────────────────────────────────┐
│  接口层 (xxx_io.h) — 纯业务数据，不含框架字段          │
│  {Module}_Input_t  { 子结构体1; 子结构体2; }         │
│  {Module}_Output_t { ... }                             │
└─────────────────────────────────────────────────────────┘
                         ↑ Link 函数搬运
┌─────────────────────────────────────────────────────────┐
│  实现层 (xxx.c)                                         │
│  static {Module}_Input_t  s_in;                        │
│  static {Module}_Output_t s_out;                       │
│  g_input.para  = &s_in;                                │
│  g_output.para = &s_out;                              │
└─────────────────────────────────────────────────────────┘
```

### 2.2 命名规范（宏模板）

| 类型 | 宏模板 | 示例 |
|------|--------|------|
| 输入类型 | `{M}_Input_t` | `AppPower_Input_t` |
| 输出类型 | `{M}_Output_t` | `AppPower_Output_t` |
| 头文件宏 | `{M}_IO_H` | `APP_POWER_IO_H` |
| Link 函数 | `{Src}_to_{Dst}_Link` | `Adc_to_Power_Link` |
| Consumer 回调 | `{Dst}_On{Src}Data` | `Power_OnAdcData` |
| 槽位枚举 | `SLOT_{M}` | `SLOT_APP_POWER` |

### 2.3 xxx_io.h 标准模板

```c
/**
 * @file    {module}_io.h
 * @brief   {Module} Data Switcher IO interface
 * @layer   {layer} (Data Switcher IO)
 *
 * 本文件定义 {Module} 模块对外暴露的输入/输出接口。
 * 每个输入源独立子结构体 — 通过 Link 函数注入。
 */
#ifndef {MODULE}_IO_H
#define {MODULE}_IO_H

#include <stdint.h>
#include "std_module.h"    /* 仅用于 Para_Grp_t 前置声明 */

/* ---- 输入: 每个数据源一个子结构体 ---- */
typedef struct {
    /* 数据源A字段 */
    uint16_t valueA;
    uint32_t valueB;
} {Module}_FromSourceA_t;

typedef struct {
    /* 数据源B字段 */
    uint8_t flag;
} {Module}_FromSourceB_t;

typedef struct {
    {Module}_FromSourceA_t fromA;   /* → SourceA 模块 */
    {Module}_FromSourceB_t fromB;   /* → SourceB 模块 */
} {Module}_Input_t;

/* ---- 输出 ---- */
typedef struct {
    uint8_t  has_output;
    uint8_t  res[3];
    uint16_t result;
} {Module}_Output_t;

/* ---- v2.2 统一接口宏 ---- */
MODULE_IO_H({Module});

#endif
```

**⚠️ 禁止在 xxx_io.h 中声明 status/res 字段！框架字段在 Para_Grp_t 中。**

---

## Step 3: 创建模板

### 3.1 创建 xxx_io.h

```c
/**
 * @file    {module}_io.h
 * @brief   {Module} Data Switcher IO interface
 * @layer   {layer} (Data Switcher IO)
 *
 * 本文件定义 {Module} 模块对外暴露的输入/输出接口。
 * 每个输入源独立子结构体 — 通过 Link 函数注入。
 */
#ifndef {MODULE}_IO_H
#define {MODULE}_IO_H

#include <stdint.h>
#include "std_module.h"

/* ---- 输入: 每个数据源一个子结构体 ---- */
typedef struct {
    /* TODO: 添加 {SourceA} 字段 */
    uint16_t valueA;
} {Module}_From{SourceA}_t;

typedef struct {
    /* TODO: 添加 {SourceB} 字段 */
    uint8_t flag;
} {Module}_From{SourceB}_t;

typedef struct {
    {Module}_From{SourceA}_t from{SourceA};   /* → {SourceA} 模块 */
    {Module}_From{SourceB}_t from{SourceB};   /* → {SourceB} 模块 */
} {Module}_Input_t;

/* ---- 输出 ---- */
typedef struct {
    uint8_t  has_output;
    uint8_t  res[3];
    uint16_t result;
} {Module}_Output_t;

/* ---- v2.2 统一接口宏 ---- */
MODULE_IO_H({Module});

#endif
```

### 3.2 创建 xxx.c

```c
/**
 * @file    {module}.c
 * @brief   一句话描述
 * @layer   {layer}
 *
 * 输入: {SourceA}(来自谁) + {SourceB}(来自谁)
 * 输出: ABC(发给谁)
 */
#include "std_module.h"
#include "{module}_io.h"
#include "{module}.h"     /* 可选 — 私有 #define */
#include <string.h>

/* ---- 数据实体（模块私有）---- */
static {Module}_Input_t   s_in;
static {Module}_Output_t  s_out;

/* ---- 骨架 ---- */
MODULE_SKELETON({Module});

/* ---- 处理逻辑（每帧被调）---- */
static void ProcessInput(void)
{
    if (g_input.info.status & ST_NEW) {
        {Module}_Input_t *in = ({Module}_Input_t *)g_input.para;
        /* 访问子结构体: in->from{SourceA}.valueA */
        g_input.info.status &= ~ST_NEW;
    }

    {Module}_Output_t *out = ({Module}_Output_t *)g_output.para;
    /* 写输出: out->result = ... */
}

/* ---- 初始化 ---- */
static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para   = &s_in;    /* 容器指向实体 */
    g_output.para  = &s_out;
}

/* ---- 导出 ---- */
MODULE_EXPORT({Module});

/* ---- Consumer 回调: Link 函数调用此函数 ---- */
void {Module}_On{SourceA}Data(Para_Grp_t *pOut)
{
    {Module}_Input_t *in = ({Module}_Input_t *)g_input.para;
    {Module}_From{SourceA}_t *src = ({Module}_From{SourceA}_t *)pOut->para;
    
    in->from{SourceA} = *src;
    g_input.info.status |= ST_NEW;
}

void {Module}_On{SourceB}Data(Para_Grp_t *pOut)
{
    {Module}_Input_t *in = ({Module}_Input_t *)g_input.para;
    {Module}_From{SourceB}_t *src = ({Module}_From{SourceB}_t *)pOut->para;
    
    in->from{SourceB} = *src;
    g_input.info.status |= ST_NEW;
}
```

---

## Step 4: Link 函数规范

### 4.1 标准模板

```c
/* Link 函数: {SourceA} → {Module} */
void {SourceA}_to_{Module}_Link(void)
{
    {SourceA}_Output_t *src = ({SourceA}_Output_t *)s_slot[SLOT_{SOURCEA}].pOut->para;
    {Module}_Input_t   *dst = ({Module}_Input_t   *)s_slot[SLOT_{MODULE}].pIn->para;

    dst->from{SourceA}.valueA = src->valueA;
    dst->from{SourceA}.valueB = src->valueB;
}
```

### 4.2 注册到 Switcher

在 `src/core/data_switcher.c` 的 `Switcher_Init()` 添加：

```c
/* 模块注册 */
{Module}_GetIO(&pIn, &pOut, &pDoWork);
Switcher_Register(SLOT_{MODULE}, pDoWork, pOut);

/* Link 注册 */
Switcher_RegisterLink(SLOT_{SOURCEA}, SLOT_{MODULE}, {SourceA}_to_{Module}_Link);
Switcher_RegisterLink(SLOT_{SOURCEB}, SLOT_{MODULE}, {SourceB}_to_{Module}_Link);
```

---

## Step 5: 验证

```bash
python tools/check_io_convention.py {module}      # I/O 命名规范检查
python tools/check_deps.py .                        # 层依赖检查
python tools/check_include.py .                     # include 权限检查
```

---

## 关键规则汇总

| 规则 | 说明 |
|------|------|
| xxx_io.h 只含业务数据 | 禁止声明 status/res — 框架字段在 Para_Grp_t |
| 每个输入源独立子结构体 | `from{SourceA}_t`, `from{SourceB}_t` |
| Link 函数显式搬运 | 字段一一对应，不做隐式转换 |
| Consumer 回调写 g_input.para | 写完置位 ST_NEW |
| g_input.para 类型转换 | `(Module_Input_t *)g_input.para` |
| g_output.para 类型转换 | `(Module_Output_t *)g_output.para` |

---

## @OUTPUT_CALLBACK 例外

如需绕开 Switcher 周期立即触发 consumer (如蜂鸣器实时反馈)：

1. Producer 写 `g_output.para` + 设 `g_output.info.route`
2. Producer 立即调 consumer 的 DoWork 或回调
3. 添加 `/* @OUTPUT_CALLBACK: <reason> — user confirmed */` 标记
4. 在 `interface_map.h` 白名单注册
5. `check_output_callback.py` 验证通过

**无标记的输出回调 → check_output_callback.py 阻断提交。**
