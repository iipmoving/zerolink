---
name: modify-module
description: "Modify existing module to comply with v2.0 Data Switcher methodology. Adds constructor pattern, three-phase structure, _io.h interface, and Switcher integration. Triggers on: modify module, upgrade module, update module, modify-module, 修改模块, 升级模块."
user-invocable: true
---

# /modify-module — 模块升级向导 (v2.0)

Modify existing modules to comply with v2.0 Data Switcher methodology.

**Project**: four_head (SC32F1XXX, Cortex-M0+, armcc)
**Methodology**: v2.0 — Data Switcher 中间层架构

---

## Step 1: Select Module

Ask:
```
1. Which module to modify? (Enter module name, e.g., "app_power", "drv_key")
2. Is this module a Producer for Data Switcher? (y/n)
   - Producer: 需要创建/更新 {module}_io.h，添加 _GetIO() 和 _DoWork()
   - Consumer: 添加 _OnInput() 回调接口
```

---

## Step 2: Analyze Current Structure

Check existing file structure:
- `src/{layer}/{module}.h`
- `src/{layer}/{module}.c`
- `include/{module}_io.h` (if exists)

---

## Step 3: Upgrade .h File

**Add `//#define` L0 blocking if missing:**

```c
#ifndef {MODULE}_H
//#define {MODULE}_H       /* L0 compiler block — 禁止跨模块 include */
// ... existing content ...
#endif /* {MODULE}_H */
```

**Add struct registration comments:**

```c
/* === INTERFACE STRUCTS (OWNER) ==================================
 * @STRUCT {Module}_Output  owner={module}  suffix=OUT
 * ================================================================ */
```

---

## Step 4: Upgrade .c File

**Add constructor pattern and three-phase structure:**

```c
/**
 * @file    {module}.c
 * @brief   {description}
 * @layer   {layer}
 */

#include "{module}.h"

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
    #if CONSUMER_MODE
    {Producer}_Output_t input;
    {Module}_OnInput(&input);  /* Switcher 会覆盖此 __weak 回调 */
    #endif

    /* ====== COMPUTE PHASE ====== */
    // ... existing logic ...

    /* ====== OUTPUT PHASE ====== */
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
}
#endif
```

---

## Step 5: Create/Update _io.h (for Producers)

**Create `include/{module}_io.h`:**

```c
/**
 * @file    {module}_io.h
 * @brief   {module} Data Switcher IO interface
 * @layer   {layer}
 */

#ifndef {MODULE}_IO_H
#define {MODULE}_IO_H

#include <stdint.h>

#pragma pack(4)

typedef struct {
    uint8_t  status;        /* bit0=已构造, bit1=新数据就绪 */
    uint8_t  res[3];        /* 32位对齐 */
    // ... existing output fields ...
} {Module}_Output_t;

#pragma pack()

void {module}_GetIO({Module}_Output_t **ppOut);
void {module}_DoWork(void);

#endif /* {MODULE}_IO_H */
```

---

## Step 6: Update Data Switcher

**Update `src/core/data_switcher.c`:**

```c
// For Producers:
#include "../../include/{module}_io.h"
static {Module}_Output_t *p{Module};

// In Switcher_Init():
{module}_GetIO(&p{Module});

// In Switcher_Run_SlotX():
{module}_DoWork();

// For Consumers:
__weak void {Module}_OnInput({Producer}_Output_t *pInput) { (void)pInput; }

// In routing logic:
if (p{Producer}->status & 0x02) {
    {Module}_OnInput(p{Producer});
    p{Producer}->status &= ~0x02;
}
```

---

## Step 7: Update interface_map.h

```c
/* Pair X: {producer} → {module} (v2.0 Switcher PUSH) */
/* 发送方: data_switcher.c  WEAK void {Module}_OnInput({Producer}_Output_t *pData) {} */
/* 接收方: {layer}/{module}.c  void {Module}_OnInput({Producer}_Output_t *pData) */
```

---

## Step 8: Verify

Run `/check` to verify compliance.

---

## v2.0 Upgrade Checklist

| Item | Status |
|------|--------|
| ✅ `.h` 文件使用 `//#define` 屏蔽 | |
| ✅ `.c` 文件添加构造函数 | |
| ✅ `.c` 文件使用三段式结构 | |
| ✅ Producer 创建 `_io.h` 接口 | |
| ✅ Consumer 添加 `_OnInput()` 回调 | |
| ✅ 添加 `_GetIO()` 和 `_DoWork()` 接口 | |
| ✅ 更新 Data Switcher | |
| ✅ 更新 interface_map.h | |
| ✅ 通过 `/check` 验证 | |