---
name: new-module
description: "Interactive wizard to create a new module following the zero-coupling methodology v2.0 with Data Switcher. Guides through layer selection, header generation (including _io.h for Switcher), three-phase implementation with constructor, struct registration, and weak pair setup. Triggers on: new module, add module, create module, new-module, 新增模块, 新建模块, 添加模块."
user-invocable: true
---

# /new-module — 交互式模块创建向导 (four_head v2.0)

Creates a new C module following the zero-coupling methodology SOP with Data Switcher support.

**Project**: four_head (SC32F1XXX, Cortex-M0+, armcc)
**Methodology**: v2.0 — Data Switcher 中间层架构
**Layers**: `app` | `drv` | `hal` | `proto` | `core`

---

## Step 1: Module Identity

Ask:

```
1. Module name? (snake_case, e.g., "power_ctrl", "key_scan")
2. Layer? (app | drv | hal | proto | core)
3. Is this module a Producer for Data Switcher? (y/n)
   - Producer: 输出数据供其他模块消费，需要创建 {module}_io.h
   - Consumer: 消费其他模块数据，通过 Switcher 获取输入
```

Show layer rules:

| Layer | Allowed includes | Forbidden | Include guard |
|-------|-----------------|-----------|---------------|
| `app` | `app/`, `drv/`, `proto/`, `core/`, `<std>` | `hal/` | `//#define` |
| `drv` | `drv/`, `hal/`, `core/`, `<std>` | `app/`, `proto/` | `//#define` |
| `hal` | `hal/`, `core/`, `<std>` | `app/`, `drv/`, `proto/` | `//#define` |
| `proto` | `proto/`, `core/`, `<std>` | `app/`, `drv/`, `hal/` | `//#define` |
| `core` | `core/`, `<std>` | all others | `//#define` |

---

## Step 2: Generate Module Header ({layer}/{module}.h)

Create `src/{layer}/{module}.h`:

```c
/**
 * @file    {module}.h
 * @brief   {description}
 * @layer   {layer}
 * @deps    {allowed includes}
 */

#ifndef {MODULE}_H
//#define {MODULE}_H       /* L0 compiler block — 禁止跨模块 include */

/* === INTERFACE STRUCTS (OWNER) ==================================
 * 本模块是以下结构体的 Owner (生产者).
 * @STRUCT 标记由 check_structs.py 解析验证.
 *
 * 格式:
 *   /* @STRUCT StructName  owner={module}  suffix=OUT */
 *   typedef struct {
 *       type field;  /* offset=N, size=N */
 *   } StructName;    /* sizeof=N */
 * ================================================================ */


/* === AI-MANAGED: INTERFACE STRUCTS (CONSUMER) ===================
 *
 *  由 /new-module Step 4 Mode B 自动生成, 不手动编辑.
 *  source= 指向 owner 结构体, check_structs.py 验证一致性.
 * ================================================================ */


/* ---- public interface ---- */

void {Module}_Run(void);


#endif /* {MODULE}_H */
```

---

## Step 3: Generate _io.h for Producers (v2.0 Switcher Interface)

**Only if user answered "y" to Step 1 Q3 (Is this a Producer):**

Create `include/{module}_io.h`:

```c
/**
 * @file    {module}_io.h
 * @brief   {module} Data Switcher IO interface
 * @layer   {layer} (Data Switcher IO)
 *
 * 本文件定义 {module} 模块对外暴露的输出接口.
 * 仅 data_switcher.c 可用全路径 include 本文件.
 *
 * 协议 (详见 v2.0 methodology):
 *   status bit0=已构造, bit1=新数据就绪 (本模块设, Switcher 清)
 *   DoWork 进入时先清 bit1, 有新产出时置位
 *   Switcher 检测 bit1 → 调用消费者 _OnInput → 清 bit1
 */

#ifndef {MODULE}_IO_H
#define {MODULE}_IO_H       /* ← 保留: _io.h 是公开接口, 两个合法 include 方 */

#include <stdint.h>

#pragma pack(4)

/* === OUTPUT STRUCT (OWNER) ====================================== */
typedef struct {
    uint8_t  status;        /* bit0=已构造, bit1=新数据就绪 (本模块设, Switcher 清) */
    uint8_t  res[3];        /* 32位对齐 */
    /* TODO: Add your output fields here */
} {Module}_Output_t;

#pragma pack()

/* ---- public interface ---- */

void {module}_GetIO({Module}_Output_t **ppOut);
void {module}_DoWork(void);


#endif /* {MODULE}_IO_H */
```

---

## Step 4: Generate .c with Three-Phase Skeleton (v2.0 Standard)

Create `src/{layer}/{module}.c`:

```c
/**
 * @file    {module}.c
 * @brief   {description}
 * @layer   {layer}
 */

#include "{module}.h"

/* === LOCAL STATE ================================================= */
static {Module}_State_t s_self;

/* === OUTPUT BUFFER (for Producers) =============================== */
#if PRODUCER_MODE
#include "../../include/{module}_io.h"
static {Module}_Output_t s_out;
#endif

/* === INPUT CALLBACK (for Consumers) ============================== */
#if CONSUMER_MODE
#include "../../include/{producer}_io.h"
__weak void {Module}_OnInput({Producer}_Output_t *pInput) 
{ (void)pInput; }
#endif

/* === 构造函数 (标准范式) === */
static void {Module}_Construct(void)
{
    #if PRODUCER_MODE
    s_out.status = 0x01;     /* bit0=已构造 */
    #endif
    // 初始化内部状态变量
}

/* ====== Module_Run (v2.0 Three-Phase Standard) ====== */

void {Module}_Run(void)
{
    static uint8_t _constructed = 0;
    if (!_constructed) { 
        _constructed = 1; 
        {Module}_Construct();  /* 首次进入时调用构造函数 */
    }

    /* ====== INPUT PHASE ====== */
    /* 所有外部数据入口集中在此 */
    #if CONSUMER_MODE
    {Producer}_Output_t input;
    {Module}_OnInput(&input);  /* Switcher 会覆盖此 __weak 回调 */
    #endif

    /* ====== COMPUTE PHASE ====== */
    /* 纯计算，不调输入/输出通道 */
    // TODO: 核心算法、状态机、数据变换

    /* ====== OUTPUT PHASE ====== */
    /* 所有结果出口集中在此 */
    #if PRODUCER_MODE
    s_out.status |= 0x02;     /* bit1=新数据就绪 */
    #endif
}

/* === PRODUCER IO Interface === */
#if PRODUCER_MODE
void {module}_GetIO({Module}_Output_t **ppOut)
{
    *ppOut = &s_out;
}

void {module}_DoWork(void)
{
    s_out.status &= ~0x02;    /* 每帧先清就绪标志 */
    {Module}_Run();           /* 执行模块逻辑 */
    /* 有新产出时由 _Run() 内部置位 bit1 */
}
#endif
```

---

## Step 5: Update Data Switcher

If this module is a **Producer** or **Consumer**, update `src/core/data_switcher.c`:

### For Producers:
```c
// In data_switcher.c
#include "../../include/{module}_io.h"

static {Module}_Output_t *p{Module};

// In Switcher_Init()
{module}_GetIO(&p{Module});
```

### For Consumers:
```c
// In data_switcher.c - Add __weak callback
__weak void {Module}_OnInput({Producer}_Output_t *pInput) { (void)pInput; }

// In Switcher_Run_SlotX()
void Switcher_Run_SlotX(void)
{
    // ... other producers ...
    
    if (p{Producer}->status & 0x02) {
        {Module}_OnInput(p{Producer});  /* 结构体直接传参 */
        p{Producer}->status &= ~0x02;     /* Switcher 清标志 */
    }
    
    {module}_DoWork();
}
```

---

## Step 6: Register __weak Pairs

Update `src/core/interface_map.h`:

```c
/* Pair X: {producer} → {consumer} (v2.0 Switcher PUSH) */
/* 发送方: data_switcher.c  WEAK void {Consumer}_OnInput({Producer}_Output_t *pData) {} */
/* 接收方: {layer}/{consumer}.c  void {Consumer}_OnInput({Producer}_Output_t *pData) */
/* v2.0: Switcher 检测 status bit1 → 回调(结构体直接传参) → 清标志 */
```

---

## Step 7: Verify

Run `/check`. All 4 checks must pass:

```bash
python .trae/tools/check_deps.py . --project four-head
python .trae/tools/check_weak_pairs.py . --project four-head
python .trae/tools/check_structs.py . --project four-head
armcc -c --cpu Cortex-M0plus --c99 -I... {module}.c
```

---

## v2.0 Communication Selection

| Scenario | Mechanism | Implementation |
|----------|-----------|---------------|
| **Switcher 路由** (周期性数据) | Struct direct param | `Consumer_OnInput(Producer_Output_t*)` |
| **ISR → 业务** (延迟敏感) | `__weak` direct call | 零中间层，链接器接线 |
| **跨时间片异步** | `Msg_Post` | 队列缓冲 |
| **纯算法集调用** | `__weak` + `void*` | 不 include .h，链接器接线 |

---

## v2.0 Key Rules Summary

1. **_io.h 文件**: 保留 `#define`，仅允许 `data_switcher.c` 全路径 include
2. **普通 .h 文件**: 屏蔽 `//#define`，禁止跨模块 include
3. **Status Bit 协议**: bit0=已构造, bit1=新数据就绪
4. **构造函数**: 标准范式 `Module_Construct()`，首次执行自动调用
5. **输入回调**: 所有外部参数通过 `_OnInput()` 统一获取
6. **Switcher 职责**: 检测 bit1 → 调用输入回调 → 清零 bit1