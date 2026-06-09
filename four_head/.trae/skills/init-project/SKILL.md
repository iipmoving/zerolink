---
name: init-project
description: "Initialize a new project following the zero-coupling methodology v2.0 with Data Switcher. Creates project structure, core files, and configuration. Triggers on: init project, initialize project, create project, init-project, 初始化项目, 创建项目."
user-invocable: true
---

# /init-project — 项目初始化向导 (four_head v2.0)

Initialize a new project with v2.0 Data Switcher architecture.

**Project**: four_head (SC32F1XXX, Cortex-M0+, armcc)
**Methodology**: v2.0 — Data Switcher 中间层架构

---

## Step 1: Create Directory Structure

```
src/
├── app/           # Application modules
├── drv/           # Driver modules
├── hal/           # Hardware abstraction layer
├── proto/         # Protocol modules
├── core/          # Core infrastructure (Switcher, utils)
├── FWLib/         # Vendor HAL (SC32F1XXX)
└── CMSIS/         # CMSIS headers
include/           # Public IO interfaces (_io.h)
cfg/               # Configuration files
docs/              # Documentation
.trae/
├── skills/        # Trae SKILL files
└── tools/         # Compliance check tools
```

---

## Step 2: Create Core Files

### 2.1 src/core/data_switcher.h

```c
/**
 * @file    data_switcher.h
 * @brief   v2.0 Data Switcher — Structured data routing middleware
 * @layer   core
 *
 * Data Switcher executes in fixed order within scheduling slots:
 *   1. Call producer X_DoWork() → Check status bit1 → Route → Clear flag
 *   2. Call consumer Y_DoWork()
 */

#ifndef DATA_SWITCHER_H
#define DATA_SWITCHER_H

void Switcher_Init(void);
void Switcher_Run_Slot1(void);
void Switcher_Run_Slot2(void);
void Switcher_Run_Slot3(void);

#endif /* DATA_SWITCHER_H */
```

### 2.2 src/core/data_switcher.c

```c
/**
 * @file    data_switcher.c
 * @brief   v2.0 Data Switcher — 结构化数据路由中间层
 * @layer   core
 */

#include "data_switcher.h"
#include "../../include/app_power_io.h"
#include "../../include/app_comm_mgr_io.h"
#include "../../include/drv_key_io.h"
#include "../../include/drv_comm_mgr_io.h"

/* === 内部指针缓存 === */
static AppPower_Output_t *pPower;
static AppCommMgr_Output_t *pCommMgr;
static DrvKey_Output_t *pKey;
static DrvCommMgr_Output_t *pDrvComm;

/* === __weak 回调声明 (Switcher 作为发送方) === */
__weak void AppHmi_OnInput(DrvKey_Output_t *pInput) { (void)pInput; }
__weak void AppCooking_OnInput(AppPower_Output_t *pInput) { (void)pInput; }
__weak void AppCommMgr_OnInput(AppPower_Output_t *pInput) { (void)pInput; }
__weak void AppPower_OnInput(DrvCommMgr_Output_t *pInput) { (void)pInput; }
__weak void DrvDisplay_OnInput(AppHmi_Output_t *pInput) { (void)pInput; }

/* === 初始化 === */
void Switcher_Init(void)
{
    AppPower_GetIO(&pPower);
    AppCommMgr_GetIO(&pCommMgr);
    DrvKey_GetIO(&pKey);
    DrvCommMgr_GetIO(&pDrvComm);
}

/* === Slot1 调度入口 (~10ms) - 快速通道 === */
void Switcher_Run_Slot1(void)
{
    /* Step 1: 执行快速 Producer */
    DrvKey_DoWork();
    
    /* Step 2: 路由 Key 数据到 Consumer */
    if (pKey->status & 0x02) {
        AppHmi_OnInput(pKey);
        pKey->status &= ~0x02;
    }
    
    /* Step 3: 执行快速 Consumer */
    AppHmi_DoWork();
}

/* === Slot2 调度入口 (~50ms) - 通信通道 === */
void Switcher_Run_Slot2(void)
{
    /* Step 1: 执行通信 Producer */
    DrvCommMgr_DoWork();
    
    /* Step 2: 路由通信数据 */
    if (pDrvComm->status & 0x02) {
        AppPower_OnInput(pDrvComm);
        pDrvComm->status &= ~0x02;
    }
    
    /* Step 3: 执行通信相关 Consumer */
    AppCommMgr_DoWork();
}

/* === Slot3 调度入口 (~100ms) - 电源通道 === */
void Switcher_Run_Slot3(void)
{
    /* Step 1: 执行电源 Producer */
    AppPower_DoWork();
    
    /* Step 2: 路由电源数据到多个 Consumer */
    if (pPower->status & 0x02) {
        AppCooking_OnInput(pPower);
        AppCommMgr_OnInput(pPower);
        pPower->status &= ~0x02;
    }
    
    /* Step 3: 执行电源相关 Consumer */
    AppCooking_DoWork();
}

/* === __weak 空壳 (app_task 调用) === */
__weak void Switcher_Run_Slot1(void) {}
__weak void Switcher_Run_Slot2(void) {}
__weak void Switcher_Run_Slot3(void) {}
__weak void Switcher_Init(void) {}
```

### 2.3 src/core/interface_map.h (v2.0)

```c
/**
 * interface_map.h —— v2.0 Data Switcher 回调函数配对映射表
 *
 * ★ 本文件仅作文档参考，任何 .c/.h 不得 #include 本文件 ★
 *
 * v2.0 架构铁律: 
 *   1. 模块间零依赖 —— 所有跨模块通信通过 Data Switcher 路由
 *   2. Producer 定义 {module}_io.h 输出接口
 *   3. Consumer 通过 _OnInput() 回调接收数据
 *   4. Switcher 负责检测 status bit1 → 调用回调 → 清零标志
 *
 * 一致性由 AI 保证 —— 每次修改任一方函数签名时, AI 同步检查并更新配对。
 */

/* === v2.0 Switcher Pairs === */

/* Pair 0: drv_key → app_hmi (Switcher 路由) */
/* 发送方: data_switcher.c  WEAK void AppHmi_OnInput(DrvKey_Output_t *pInput) {} */
/* 接收方: app/app_hmi.c  void AppHmi_OnInput(DrvKey_Output_t *pInput) */
/* v2.0: Switcher 检测 status bit1 → 回调(结构体直接传参) → 清标志 */

/* Pair 1: app_power → app_cooking (Switcher 路由) */
/* 发送方: data_switcher.c  WEAK void AppCooking_OnInput(AppPower_Output_t *pInput) {} */
/* 接收方: app/app_cooking.c  void AppCooking_OnInput(AppPower_Output_t *pInput) */

/* Pair 2: app_power → app_comm_mgr (Switcher 路由) */
/* 发送方: data_switcher.c  WEAK void AppCommMgr_OnInput(AppPower_Output_t *pInput) {} */
/* 接收方: app/app_comm_mgr.c  void AppCommMgr_OnInput(AppPower_Output_t *pInput) */

/* Pair 3: drv_comm_mgr → app_power (Switcher 路由) */
/* 发送方: data_switcher.c  WEAK void AppPower_OnInput(DrvCommMgr_Output_t *pInput) {} */
/* 接收方: app/app_power.c  void AppPower_OnInput(DrvCommMgr_Output_t *pInput) */

/* Pair 4: app_hmi → drv_display (Switcher 路由) */
/* 发送方: data_switcher.c  WEAK void DrvDisplay_OnInput(AppHmi_Output_t *pInput) {} */
/* 接收方: drv/drv_display.c  void DrvDisplay_OnInput(AppHmi_Output_t *pInput) */

/* === v2.0 Status Bit 协议 === */
/* bit0 = 已构造 (模块构造函数设置，永不清除) */
/* bit1 = 新数据就绪 (Producer 设置，Switcher 清除) */

/* === v2.0 构造函数模式 === */
/* static void Module_Construct(void) { ... } */
/* 首次进入 Module_Run() 时自动调用 */

/* 本文件不被任何代码引用 —— 仅供 AI 和人类阅读 */
#ifdef INCLUDE_INTERFACE_MAP
#error "interface_map.h 是文档文件，禁止被 #include"
#endif
```

---

## Step 3: Create Example Producer IO

### 3.1 include/app_power_io.h

```c
/**
 * @file    app_power_io.h
 * @brief   AppPower Data Switcher IO interface
 * @layer   app
 */

#ifndef APP_POWER_IO_H
#define APP_POWER_IO_H

#include <stdint.h>

#pragma pack(4)

typedef struct {
    uint8_t  status;        /* bit0=已构造, bit1=新数据就绪 */
    uint8_t  res[3];        /* 32位对齐 */
    uint8_t  head_idx;      /* 当前选中头索引 */
    uint16_t power_watt;    /* 当前功率值 */
    uint8_t  work_mode;     /* 工作模式 */
} AppPower_Output_t;

#pragma pack()

void AppPower_GetIO(AppPower_Output_t **ppOut);
void AppPower_DoWork(void);

#endif /* APP_POWER_IO_H */
```

---

## Step 4: Update main.c

```c
/**
 * @file    main.c
 * @brief   Main entry point (v2.0)
 */

#include "core/data_switcher.h"

/* === 调度定时器回调 === */
__weak void AppHmi_OnTimer100ms(void) {}
__weak void AppCooking_OnTimer1s(void) {}

int main(void)
{
    /* 初始化 Switcher */
    Switcher_Init();
    
    /* 主循环 */
    while (1) {
        /* 执行调度槽 */
        Switcher_Run_Slot1();  /* ~10ms */
        
        if (timer100ms_flag) {
            timer100ms_flag = 0;
            Switcher_Run_Slot2();  /* ~50ms */
            AppHmi_OnTimer100ms();
        }
        
        if (timer1s_flag) {
            timer1s_flag = 0;
            Switcher_Run_Slot3();  /* ~100ms */
            AppCooking_OnTimer1s();
        }
    }
}
```

---

## Step 5: Create Tool Scripts

Create `.trae/tools/check_deps.py`, `check_weak_pairs.py`, `check_structs.py`

---

## Step 6: Verify Initialization

Run `/check` to verify all checks pass.

---

## v2.0 Project Structure Summary

| Directory | Purpose |
|-----------|---------|
| `src/app/` | Application modules (业务逻辑) |
| `src/drv/` | Driver modules (驱动层) |
| `src/hal/` | Hardware abstraction (硬件抽象) |
| `src/proto/` | Protocol modules (协议层) |
| `src/core/` | Core infrastructure (Switcher, utils) |
| `include/` | Public IO interfaces (`_io.h`) |
| `.trae/skills/` | Trae SKILL files |
| `.trae/tools/` | Compliance check tools |