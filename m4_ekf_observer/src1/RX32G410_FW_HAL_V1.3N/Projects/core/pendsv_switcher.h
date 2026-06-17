/**
 * @file    pendsv_switcher.h
 * @brief   PendSV Switcher — 实时通路调度器 (v2.3)
 * @layer   core
 *
 * PendSV 通路用于紧急 ISR 事件的实时处理，
 * 所有 ISR 触发 PendSV，在 PendSV_Handler 中按来源路由处理。
 */

#ifndef PENSV_SWITCHER_H
#define PENSV_SWITCHER_H

#include "std_module_v2.3.h"  /* Para_Grp_t, ISR_SOURCE_* */

/* 最大模块数 */
#define PENSV_MAX_MODULES  32

/* PendSV 状态变量声明 */
extern volatile uint8_t g_isr_pending;   /* ISR 请求标志 */
extern volatile uint8_t g_isr_source;    /* ISR 来源标识 */

/* PendSV 中间层初始化 */
void Switcher_PendSV_Init(void);

/* ISR 触发 PendSV */
void Switcher_TriggerPendSV(uint8_t isr_source);

/* 注册模块到 PendSV 注册表 */
void Switcher_RegisterModule(const char *name, 
                             Para_Grp_t *pIn, 
                             Para_Grp_t *pOut, 
                             void (*pDoWork)(void),
                             void (*pOnISR)(void));

/* 查找模块输出 (按名称) */
Para_Grp_t* Switcher_FindModuleOutput(const char *name);

/* 查找模块 DoWork (按名称) */
void* Switcher_FindModule(const char *name);

/* 查找模块 OnISR 回调 (按名称) */
void* Switcher_FindModuleOnISR(const char *name);

#endif /* PENSV_SWITCHER_H */