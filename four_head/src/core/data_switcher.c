/**
 * @file    data_switcher.c
 * @brief   Data Switcher — 统一调度
 * @layer   core
 *
 * 所有模块通过 GetIO 注册函数指针，Switcher 只调 pDoWork。
 * 数据通过 __weak 回调自动路由，Switcher 不碰 struct 字段。
 */
#include "core/std_module.h"
#include "data_switcher.h"

/* 模块 GetIO 声明（由 MODULE_EXPORT 生成，在各自 .c 中定义）*/
void AppPower_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
void AppHmi_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
void AppCommMgr_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
void AppProtect_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
void AppSegAlign_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
void DrvKey_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
void DrvCommMgr_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
void DrvBuzzer_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));
void DrvDisplay_GetIO(Para_Grp_t **ppIn, Para_Grp_t **ppOut, void (**ppDoWork)(void));

/* APP / DRV 强符号 */
void AppHmi_OnKey(uint16_t param, void *data_ptr);
void AppCooking_OnKey(uint16_t param, void *data_ptr);
void AppSegAlign_OnKey(uint16_t param, void *data_ptr);
void AppPower_OnRegData(uint16_t param, void *data_ptr);
void AppCooking_OnRegData(uint16_t param, void *data_ptr);
void AppProtect_OnRegData(uint16_t param, void *data_ptr);
void AppPower_OnSystemError(uint16_t param, void *data_ptr);

/* DRV 层强符号（v1.0 桥接，migrate 后移除）*/
void DrvDisplay_OnRefresh(uint16_t param, void *data_ptr);
void DrvBuzzer_OnCtrl(uint16_t param, void *data_ptr);

#define MAX_MODULES  16

typedef struct {
    void (*pDoWork)(void);
} ModuleSlot_t;

static ModuleSlot_t s_slots[MAX_MODULES];
static uint8_t      s_count = 0;

void Switcher_Register(void (*pDoWork)(void))
{
    if (s_count < MAX_MODULES)
        s_slots[s_count++].pDoWork = pDoWork;
}

void Switcher_Init(void)
{
    Para_Grp_t *pIn, *pOut;
    void       (*pWork)(void);

    /* 已迁移到 std_module.h 的模块 */
    AppPower_GetIO(&pIn, &pOut, &pWork);
    /* INFO 是模块私有的，Switcher 不碰 */
    Switcher_Register(pWork);

    AppHmi_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork);

    AppCommMgr_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork);

    AppProtect_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork);

    AppSegAlign_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork);

    DrvKey_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork);

    DrvCommMgr_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork);

    DrvBuzzer_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork);

    DrvDisplay_GetIO(&pIn, &pOut, &pWork);
    Switcher_Register(pWork);
}

void Switcher_Run(void)
{
    for (uint8_t i = 0; i < s_count; i++)
        if (s_slots[i].pDoWork)
            s_slots[i].pDoWork();
}

/* ===== v2.0 输出路由 ===== */

/* AppCommMgr 输出 → 寄存器数据广播到三个 APP 模块 */
void AppCommMgr_OnOutput(Para_Grp_t *pOut)
{
    uint8_t *d = (uint8_t *)pOut->para;
    if (!d[0]) return;
    uint8_t head = d[1];
    AppPower_OnRegData((uint16_t)head, d);
    AppCooking_OnRegData((uint16_t)head, d);
    AppProtect_OnRegData((uint16_t)head, d);
}

/* AppProtect 输出 → 系统错误到 app_power */
void AppProtect_OnOutput(Para_Grp_t *pOut)
{
    uint8_t *d = (uint8_t *)pOut->para;
    if (!d[0]) return;
    AppPower_OnSystemError((uint16_t)d[1], d);
}

/* DrvKey 输出 → 广播到三个 APP 模块 */
void DrvKey_OnOutput(Para_Grp_t *pOut)
{
    uint8_t *d = (uint8_t *)pOut->para;
    if (!d[0]) return;  /* has_key */
    uint16_t param = (uint16_t)d[1] | ((uint16_t)d[2] << 8);
    AppHmi_OnKey(param, NULL);
    AppCooking_OnKey(param, NULL);
    AppSegAlign_OnKey(param, NULL);
}

/* AppHmi 输出 → 分发到 drv_display / drv_buzzer */
void AppHmi_OnOutput(Para_Grp_t *pOut)
{
    /* OutData_t layout: has_display(1) has_buzzer(1) buzzer_on(1) res(1) [display_data...] */
    uint8_t *d = (uint8_t *)pOut->para;
    uint8_t has_display = d[0];
    uint8_t has_buzzer  = d[1];
    uint8_t buzzer_on   = d[2];

    if (has_display) {
        DrvDisplay_OnRefresh(0, d + 4);  /* d+4 = &hot_head_idx = display data start */
    }
    if (has_buzzer) {
        DrvBuzzer_OnCtrl(buzzer_on ? 1u : 0u, NULL);
    }
}
