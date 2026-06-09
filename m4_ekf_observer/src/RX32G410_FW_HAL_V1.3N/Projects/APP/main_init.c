/**
 * @file    main_init.c
 * @brief   v2.3 系统初始化示例
 *
 * 初始化两条通路:
 *   1. 主循环通路: Data Switcher (data_switcher.c)
 *   2. PendSV 实时通路: PendSV Switcher (pendsv_switcher.c)
 *
 * 模块双入口:
 *   - DoWork: Switcher 调度入口
 *   - OnISR: ISR 独立入口 (低开销)
 */

#include "core/std_module_v2.3.h"
#include "core/data_switcher.h"
#include "core/pendsv_switcher.h"

/* 外部 GetIO 声明 */
extern void AppAdc_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
extern void PowerBase_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
extern void PowerCalc_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
extern void ISRWORK_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));

/* 外部 OnISR 声明 */
extern void AppAdc_OnISR(void);
extern void ISRWORK_OnISR(void);

/* ========== 系统初始化 ========== */
void System_Init(void)
{
    /* ① PendSV 通路初始化 */
    Switcher_PendSV_Init();
    
    /* ② 注册模块到 PendSV 通路 */
    Para_Grp_t *pIn, *pOut;
    void (*pDoWork)(void);
    
    /* 注册 AppAdc (带 OnISR 回调) */
    AppAdc_GetIO(&pIn, &pOut, &pDoWork);
    Switcher_RegisterModule("AppAdc", pIn, pOut, pDoWork, AppAdc_OnISR);
    
    /* 注册 PowerBase (无 OnISR, 回退到 DoWork) */
    PowerBase_GetIO(&pIn, &pOut, &pDoWork);
    Switcher_RegisterModule("PowerBase", pIn, pOut, pDoWork, NULL);
    
    /* 注册 PowerCalc (无 OnISR, 回退到 DoWork) */
    PowerCalc_GetIO(&pIn, &pOut, &pDoWork);
    Switcher_RegisterModule("PowerCalc", pIn, pOut, pDoWork, NULL);
    
    /* 注册 ISRWORK (带 OnISR 回调) */
    ISRWORK_GetIO(&pIn, &pOut, &pDoWork);
    Switcher_RegisterModule("ISRWORK", pIn, pOut, pDoWork, ISRWORK_OnISR);
    
    /* ③ 主循环 Data Switcher 初始化 (原有) */
    Switcher_Init();
}

/* ========== 主循环 ========== */
void Main_Loop(void)
{
    while (1) {
        /* 主循环通路 */
        Switcher_Run_Slot1();
        
        /* 其他任务... */
    }
}