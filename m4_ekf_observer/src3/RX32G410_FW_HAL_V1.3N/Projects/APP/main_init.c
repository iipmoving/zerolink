/**
 * @file    main_init.c
 * @brief   v2.3 系统初始化示例
 *
 * 初始化两条通路:
 *   1. 主循环通路: Data Switcher (data_switcher.c)
 *   2. PendSV 实时通路: PendSV Switcher (pendsv_switcher.c)
 */

#define STD_MODULE_ENABLE_ISR  1
#include "std_module.h"
#include "data_switcher.h"
#include "pendsv_switcher.h"

/* ========== 主循环通路: 模块 GetIO 声明 ========== */
extern void AppAdc_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
extern void PowerBase_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
extern void PowerCalc_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));

/* ========== ISR 通路: 模块 GetISR_IO 声明 ========== */
extern void AppAdc_GetISR_IO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
extern void ISRWORK_GetISR_IO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));

/* ========== 系统初始化 ========== */
void System_Init(void)
{
    /* ① PendSV 通路初始化 */
    Switcher_PendSV_Init();

    /* ② 注册 ISR 模块到 PendSV 通路 (使用 GetISR_IO) */
    Para_Grp_t *pIn, *pOut;
    void (*pISR_DoWork)(void);

    AppAdc_GetISR_IO(&pIn, &pOut, &pISR_DoWork);
    if (pISR_DoWork) Switcher_RegisterISRModule(pISR_DoWork);

    ISRWORK_GetISR_IO(&pIn, &pOut, &pISR_DoWork);
    if (pISR_DoWork) Switcher_RegisterISRModule(pISR_DoWork);

    /* ③ 主循环 Data Switcher 初始化 (原有 v2.2 方式) */
    Switcher_Init();
}

/* ========== 主循环 ========== */
void Main_Loop(void)
{
    while (1) {
        Switcher_Run_Slot1();
    }
}
