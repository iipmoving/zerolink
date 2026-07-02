# PendSV 实时处理通路架构规划

> 版本: v1.0  
> 日期: 2026-06-08  
> 状态: 待审查

---

## 一、设计目标

| 目标 | 说明 |
|------|------|
| **实时性** | ISR 退出后立即执行，比主循环更快 |
| **无重入** | 加锁机制确保处理完整后才响应新请求 |
| **统一入口** | ISRWORK 模块作为 ISR 处理总入口 |
| **灵活路由** | PendSV 中间层可访问所有模块 IO |

---

## 二、架构总览

```
┌─────────────────────────────────────────────────────────────┐
│                    PendSV 通路架构                            │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────┐                                            │
│  │ ISR 触发层  │                                            │
│  │ ISR_Handler │ → g_isr_pending = 1                       │
│  │             │ → SCB->ICSR |= PENDSVSET                   │
│  └─────────────┘                                            │
│         │                                                   │
│         ▼                                                   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │              PendSV 中间层 (Switcher)                 │   │
│  │                                                     │   │
│  │  ┌─────────────────────────────────────────────┐   │   │
│  │  │           模块注册表 (所有 IO 接口)           │   │   │
│  │  │                                             │   │   │
│  │  │  slots[] = {                                │   │   │
│  │  │    {A, g_A_input, g_A_output, A_DoWork},    │   │   │
│  │  │    {B, g_B_input, g_B_output, B_DoWork},    │   │   │
│  │  │    {ISRWORK, g_isr_in, g_isr_out, ISRWORK}, │   │   │
│  │  │    ...                                      │   │   │
│  │  │  }                                          │   │   │
│  │  └─────────────────────────────────────────────┘   │   │
│  │                                                     │   │
│  │  PendSV_Handler() {                                 │   │
│  │    if (!g_pendsv_busy && g_isr_pending) {           │   │
│  │      g_pendsv_busy = 1;                             │   │
│  │      Switcher_Run_ISR_Slot();                       │   │
│  │      g_pendsv_busy = 0;                             │   │
│  │    }                                                │   │
│  │  }                                                  │   │
│  │                                                     │   │
│  └─────────────────────────────────────────────────────┘   │
│         │                                                   │
│         ▼                                                   │
│  ┌─────────────┐                                            │
│  │ ISRWORK 模块│                                            │
│  │             │ ← 收集 A+B+C 数据                          │
│  │             │ ← 统一处理                                 │
│  └─────────────┘                                            │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 三、PendSV 中间层实现

### 3.1 状态变量

```c
/* PendSV 状态变量 */
volatile uint8_t g_pendsv_busy = 0;    /* 处理锁 */
volatile uint8_t g_isr_pending = 0;    /* ISR 请求标志 */
volatile uint8_t g_isr_source = 0;     /* ISR 来源标识 */
```

### 3.2 模块注册表

```c
/* 模块注册表结构 */
typedef struct {
    const char    *name;
    Para_Grp_t    *pInput;
    Para_Grp_t    *pOutput;
    void          (*pDoWork)(void);
} ModuleSlot_t;

static ModuleSlot_t g_slots[32];       /* 最大 32 个模块 */
static uint8_t      g_slot_count = 0;

/* 注册模块 */
void Switcher_RegisterModule(const char *name, 
                             Para_Grp_t *pIn, 
                             Para_Grp_t *pOut, 
                             void (*pDoWork)(void))
{
    if (g_slot_count < 32) {
        g_slots[g_slot_count].name    = name;
        g_slots[g_slot_count].pInput  = pIn;
        g_slots[g_slot_count].pOutput = pOut;
        g_slots[g_slot_count].pDoWork = pDoWork;
        g_slot_count++;
    }
}

/* 查找模块 */
ModuleSlot_t* Switcher_FindModule(const char *name)
{
    for (uint8_t i = 0; i < g_slot_count; i++) {
        if (strcmp(g_slots[i].name, name) == 0) {
            return &g_slots[i];
        }
    }
    return NULL;
}
```

### 3.3 ISR 触发函数

```c
/* ISR 触发 PendSV */
void Switcher_TriggerPendSV(uint8_t isr_source)
{
    g_isr_source = isr_source;         /* 记录 ISR 来源 */
    g_isr_pending = 1;                 /* 设置请求标志 */
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;  /* 触发 PendSV */
}
```

### 3.4 PendSV Handler

```c
/* PendSV Handler — 加锁处理 */
void PendSV_Handler(void)
{
    /* 加锁检查 */
    if (g_pendsv_busy) {
        return;  /* 已在处理，退出 */
    }
    
    if (!g_isr_pending) {
        return;  /* 无请求，退出 */
    }
    
    /* 加锁 */
    g_pendsv_busy = 1;
    g_isr_pending = 0;
    
    /* 执行 ISR 处理槽 */
    Switcher_Run_ISR_Slot(g_isr_source);
    
    /* 解锁 */
    g_pendsv_busy = 0;
    
    /* 检查累积请求 */
    if (g_isr_pending) {
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    }
}
```

### 3.5 ISR 处理槽路由

```c
/* ISR 处理槽 — 根据 ISR 来源路由 */
void Switcher_Run_ISR_Slot(uint8_t isr_source)
{
    switch (isr_source) {
        case ISR_SOURCE_ADC:
            Switcher_Route_ADC_ISR();
            break;
            
        case ISR_SOURCE_TIMER:
            Switcher_Route_Timer_ISR();
            break;
            
        case ISR_SOURCE_FAULT:
            Switcher_Route_Fault_ISR();
            break;
            
        default:
            Switcher_Route_Generic_ISR();
            break;
    }
}

/* ADC ISR 路由 */
void Switcher_Route_ADC_ISR(void)
{
    /* 1. 先让 ADC 模块更新输出 */
    ModuleSlot_t *adc = Switcher_FindModule("ADC");
    if (adc && adc->pDoWork) {
        adc->pDoWork();  /* ADC_DoWork() */
    }
    
    /* 2. 路由 → ISRWORK */
    ModuleSlot_t *isrwork = Switcher_FindModule("ISRWORK");
    if (isrwork && isrwork->pDoWork) {
        isrwork->pDoWork();  /* ISRWORK_DoWork() */
    }
}

/* Timer ISR 路由 */
void Switcher_Route_Timer_ISR(void)
{
    /* 1. 先让各模块更新输出 */
    ModuleSlot_t *a = Switcher_FindModule("A");
    ModuleSlot_t *b = Switcher_FindModule("B");
    ModuleSlot_t *c = Switcher_FindModule("C");
    
    if (a && a->pDoWork) a->pDoWork();
    if (b && b->pDoWork) b->pDoWork();
    if (c && c->pDoWork) c->pDoWork();
    
    /* 2. 路由 → ISRWORK */
    ModuleSlot_t *isrwork = Switcher_FindModule("ISRWORK");
    if (isrwork && isrwork->pDoWork) {
        isrwork->pDoWork();
    }
}
```

---

## 四、ISRWORK 模块设计

### 4.1 数据结构

```c
/* 输入数据结构 */
#pragma pack(4)
typedef struct {
    uint8_t  a_valid;
    uint8_t  b_valid;
    uint8_t  c_valid;
    uint8_t  isr_source;    /* ISR 来源标识 */
    uint8_t  res[4];        /* 对齐填充 */
    
    A_Data_t a_data;        /* 来自 Module A */
    B_Data_t b_data;        /* 来自 Module B */
    C_Data_t c_data;        /* 来自 Module C */
} ISRWORK_InData_t;
#pragma pack()

/* 输出数据结构 */
#pragma pack(4)
typedef struct {
    uint8_t  result_valid;
    uint8_t  res[3];
    uint16_t processed_value;
    uint16_t fault_code;
} ISRWORK_OutData_t;
#pragma pack()
```

### 4.2 模块实现

```c
/**
 * @file    isrwork.c
 * @brief   ISRWORK 模块 — ISR 实时处理入口
 * @layer   app
 */

#include "core/std_module.h"
#include <string.h>

static ISRWORK_InData_t  s_in;
static ISRWORK_OutData_t s_out;

MODULE_SKELETON();

/* 输入段: 从各模块 g_output 收集数据 */
static void CollectInput(void)
{
    extern Para_Grp_t g_A_output, g_B_output, g_C_output;
    
    ISRWORK_InData_t *in = (ISRWORK_InData_t *)g_input.para;
    
    /* 收集 A 数据 */
    if (g_A_output.para != NULL) {
        memcpy(&in->a_data, g_A_output.para, sizeof(A_Data_t));
        in->a_valid = 1;
    }
    
    /* 收集 B 数据 */
    if (g_B_output.para != NULL) {
        memcpy(&in->b_data, g_B_output.para, sizeof(B_Data_t));
        in->b_valid = 1;
    }
    
    /* 收集 C 数据 */
    if (g_C_output.para != NULL) {
        memcpy(&in->c_data, g_C_output.para, sizeof(C_Data_t));
        in->c_valid = 1;
    }
    
    in->isr_source = g_isr_source;
    g_input.info.status |= ST_NEW;
}

/* 计算段: 统一处理 */
static void ProcessInput(void)
{
    ISRWORK_InData_t  *in  = (ISRWORK_InData_t *)g_input.para;
    ISRWORK_OutData_t *out = (ISRWORK_OutData_t *)g_output.para;
    
    CollectInput();
    
    if (g_input.info.status & ST_NEW) {
        
        switch (in->isr_source) {
            case ISR_SOURCE_ADC:
                if (in->a_valid && in->b_valid) {
                    out->processed_value = 
                        fuse_adc_data(in->a_data.value, in->b_data.value);
                }
                break;
                
            case ISR_SOURCE_TIMER:
                if (in->a_valid && in->b_valid && in->c_valid) {
                    out->processed_value = 
                        calculate_periodic(&in->a_data, &in->b_data, &in->c_data);
                }
                break;
                
            case ISR_SOURCE_FAULT:
                out->fault_code = detect_fault(in);
                break;
        }
        
        out->result_valid = 1;
        g_input.info.status &= ~ST_NEW;
    }
}

static void Init(void)
{
    memset(&s_in,  0, sizeof(s_in));
    memset(&s_out, 0, sizeof(s_out));
    g_input.para  = &s_in;
    g_output.para = &s_out;
}

MODULE_EXPORT(ISRWORK);
```

---

## 五、ISR 触发入口

### 5.1 ISR 来源定义

```c
/* ISR 来源定义 */
#define ISR_SOURCE_ADC     0x01
#define ISR_SOURCE_TIMER   0x02
#define ISR_SOURCE_FAULT   0x03
```

### 5.2 ISR Handler 实现

```c
/**
 * @file    isr_triggers.c
 * @brief   ISR 触发 PendSV 入口
 */

#include "pendsv_switcher.h"

/* ADC ISR */
void ADC_IRQHandler(void)
{
    ADC->SR &= ~ADC_SR_EOC;                 /* 清除 ADC 标志 */
    Switcher_TriggerPendSV(ISR_SOURCE_ADC); /* 触发 PendSV */
}

/* Timer ISR */
void TIM_IRQHandler(void)
{
    TIM->SR &= ~TIM_SR_UIF;                 /* 清除 Timer 标志 */
    Switcher_TriggerPendSV(ISR_SOURCE_TIMER);
}

/* Fault ISR */
void Fault_IRQHandler(void)
{
    Switcher_TriggerPendSV(ISR_SOURCE_FAULT);
}
```

---

## 六、加锁机制流程

```
┌─────────────────────────────────────────────────────────────┐
│                    PendSV 加锁处理流程                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ISR_Handler()                                              │
│    ├─ g_isr_pending = 1                                     │
│    └─ 触发 PendSV                                           │
│                                                             │
│  PendSV_Handler()                                           │
│    ├─ if (g_pendsv_busy) return  ← 已在处理，退出           │
│    ├─ if (!g_isr_pending) return  ← 无请求，退出            │
│    ├─ g_pendsv_busy = 1  ← 加锁                             │
│    ├─ g_isr_pending = 0                                     │
│    ├─ Switcher_Run_ISR_Slot()  ← 处理                       │
│    │   │                                                    │
│    │   │  ← 此时新 ISR 触发，g_isr_pending = 1              │
│    │   │    但 g_pendsv_busy = 1，不执行                     │
│    │   │                                                    │
│    ├─ g_pendsv_busy = 0  ← 解锁                             │
│    ├─ if (g_isr_pending) 触发 PendSV  ← 处理累积请求        │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 七、完整数据流

```
┌─────────────────────────────────────────────────────────────┐
│                    PendSV 通路完整数据流                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ADC_IRQHandler()                                           │
│    └ Switcher_TriggerPendSV(ISR_SOURCE_ADC)                │
│    └ g_isr_pending = 1                                      │
│    └ SCB->ICSR |= PENDSVSET                                 │
│                                                             │
│  PendSV_Handler()                                           │
│    ├─ g_pendsv_busy = 1 (加锁)                              │
│    ├─ Switcher_Run_ISR_Slot(ISR_SOURCE_ADC)                 │
│    │   │                                                    │
│    │   ├─ Switcher_Route_ADC_ISR()                          │
│    │   │   ├─ ADC_DoWork()                                  │
│    │   │   │   └─ 更新 g_ADC_output.para                    │
│    │   │   │                                                │
│    │   │   ├─ ISRWORK_DoWork()                              │
│    │   │   │   ├─ CollectInput()                            │
│    │   │   │   │   ├─ memcpy g_A_output → s_in.a_data       │
│    │   │   │   │   ├─ memcpy g_B_output → s_in.b_data       │
│    │   │   │   │   └─ memcpy g_C_output → s_in.c_data       │
│    │   │   │   │                                            │
│    │   │   │   ├─ ProcessInput()                            │
│    │   │   │   │   └─ fuse_adc_data()                       │
│    │   │   │   │   └─ 更新 g_ISRWORK_output.para            │
│    │   │   │   │                                            │
│    │   │   │   └─ MODULE_EXPORT(ISRWORK)                    │
│    │   │   │                                                │
│    │   │   └─ Switcher_FindModule("ISRWORK")                │
│    │   │                                                    │
│    │   └─                                                   │
│    ├─ g_pendsv_busy = 0 (解锁)                              │
│    ├─ if (g_isr_pending) 再次触发 PendSV                    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 八、关键设计总结

| 层级 | 组件 | 职责 |
|------|------|------|
| **ISR 触发层** | ISR_Handler | 只触发 PendSV，标记来源 |
| **PendSV 中间层** | PendSV_Handler | 加锁 + 路由 + 解锁 |
| **模块注册表** | g_slots[] | 持有所有模块 IO 接口 |
| **ISRWORK 模块** | ISRWORK_DoWork | 收集数据 + 统一处理 |

---

## 九、与方法论对比

| 方法论规定 | PendSV 方案 | 符合性 |
|------------|-------------|--------|
| ISR 只设标志 | ISR 只触发 PendSV | ✅ |
| 不走 Switcher | PendSV 是独立中断 | ✅ |
| 实时处理 | ISR 退出后立即执行 | ✅ |
| 无重入 | 加锁机制保证 | ✅ |
| 单入口 | ISRWORK_DoWork 统一入口 | ✅ |

---

## 十、优势总结

| 特性 | 说明 |
|------|------|
| **最低优先级** | 所有 ISR 完成后才执行，不打断其他中断 |
| **软件触发** | ISR 中只需写寄存器，无函数调用开销 |
| **实时性** | ISR 退出后立即执行，比主循环更快 |
| **无重入** | 加锁机制确保处理完整后才响应新请求 |
| **灵活路由** | PendSV 中间层可访问所有模块 IO |
| **统一入口** | ISRWORK 模块作为 ISR 处理总入口 |

---

## 十一、待审查项

| 序号 | 审查项 | 状态 |
|------|--------|------|
| 1 | PendSV Handler 是否需要关中断保护 | 待定 |
| 2 | 模块注册表是否需要动态扩容 | 待定 |
| 3 | ISRWORK 是否需要输出回调 | 待定 |
| 4 | 是否需要支持 ISR 优先级分级 | 待定 |
| 5 | 是否需要与主循环 Switcher 协调 | 待定 |

---

## 十二、附录

### A. ARM Cortex-M PendSV 特性

| 特性 | 说明 |
|------|------|
| **中断号** | 14 |
| **优先级** | 可配置，通常设为最低 |
| **触发方式** | 写 SCB->ICSR PENDSVSET 位 |
| **用途** | RTOS 上下文切换、延迟处理 |

### B. 相关文件

| 文件 | 职责 |
|------|------|
| `pendsv_switcher.c` | PendSV 中间层实现 |
| `pendsv_switcher.h` | PendSV 接口声明 |
| `isrwork.c` | ISRWORK 模块实现 |
| `isr_triggers.c` | ISR 触发入口 |

---

> **审查说明**: 请审查以上架构规划，确认是否符合方法论规范，并提出修改建议。