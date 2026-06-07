/**
 * @file    data_switcher.c
 * @brief   Data Switcher — PULL 路由调度器
 * @layer   core
 *
 * 所有模块通过 GetIO 注册函数指针和输出指针。
 * Phase 1: 按序调各模块 DoWork (producer 写 g_output.para)
 * Phase 2: 显式路由 — Switcher 检查输出标志, 调 consumer 回调
 *
 * 数据流: Producer 写 g_output.para → Switcher 路由 → Consumer 回调写入 g_input.para + ST_NEW
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

/* Consumer 回调 — Switcher 在 Producer DoWork 后显式调用 */

/* v2.2 PULL: 接收 Para_Grp_t *pOut (consumer 内部 memcpy + ST_NEW) */
void AppPower_OnCookingData(Para_Grp_t *pOut);
void AppPower_OnProtectData(Para_Grp_t *pOut);
void AppPower_OnCommMgrData(Para_Grp_t *pOut);

/* v1.x __weak 回调: pre-MODULE_SKELETON 模块使用 (uint16_t, void*) */
void AppCooking_OnRegData(uint16_t param, void *data_ptr);
void AppProtect_OnRegData(uint16_t param, void *data_ptr);
void AppHmi_OnKey(uint16_t param, void *data_ptr);
void AppCooking_OnKey(uint16_t param, void *data_ptr);
void AppSegAlign_OnKey(uint16_t param, void *data_ptr);
void DrvDisplay_OnRefresh(uint16_t param, void *data_ptr);
/* @OUTPUT_CALLBACK: buzzer real-time feedback — user confirmed */
void DrvBuzzer_OnCtrl(uint16_t param, void *data_ptr);

#define MAX_MODULES  16

/* 模块槽位索引 */
enum {
    SLOT_COMM_MGR = 0,
    SLOT_POWER,
    SLOT_PROTECT,
    SLOT_COOKING,
    SLOT_HMI,
    SLOT_SEG_ALIGN,
    SLOT_KEY,
    SLOT_COMM_MGR_DRV,
    SLOT_BUZZER,
    SLOT_DISPLAY,
    SLOT_COUNT
};

typedef struct {
    void (*pDoWork)(void);
    Para_Grp_t *pOut;
} ModuleSlot_t;

static ModuleSlot_t s_slots[SLOT_COUNT];

static void _register(uint8_t idx, void (*pDoWork)(void), Para_Grp_t *pOut)
{
    s_slots[idx].pDoWork = pDoWork;
    s_slots[idx].pOut    = pOut;
}

void Switcher_Init(void)
{
    Para_Grp_t *pIn, *pOut;
    void       (*pDoWork)(void);

    AppCommMgr_GetIO(&pIn, &pOut, &pDoWork);
    _register(SLOT_COMM_MGR, pDoWork, pOut);

    AppPower_GetIO(&pIn, &pOut, &pDoWork);
    _register(SLOT_POWER, pDoWork, pOut);

    AppProtect_GetIO(&pIn, &pOut, &pDoWork);
    _register(SLOT_PROTECT, pDoWork, pOut);

    /* AppCooking — not yet migrated to MODULE_SKELETON, skip */

    AppHmi_GetIO(&pIn, &pOut, &pDoWork);
    _register(SLOT_HMI, pDoWork, pOut);

    AppSegAlign_GetIO(&pIn, &pOut, &pDoWork);
    _register(SLOT_SEG_ALIGN, pDoWork, pOut);

    DrvKey_GetIO(&pIn, &pOut, &pDoWork);
    _register(SLOT_KEY, pDoWork, pOut);

    DrvCommMgr_GetIO(&pIn, &pOut, &pDoWork);
    _register(SLOT_COMM_MGR_DRV, pDoWork, pOut);

    DrvBuzzer_GetIO(&pIn, &pOut, &pDoWork);
    _register(SLOT_BUZZER, pDoWork, pOut);

    DrvDisplay_GetIO(&pIn, &pOut, &pDoWork);
    _register(SLOT_DISPLAY, pDoWork, pOut);
}

/* ===== Phase 2: 显式路由 ===== */

static void _route_comm_mgr(void)
{
    Para_Grp_t *pOut = s_slots[SLOT_COMM_MGR].pOut;
    if (!pOut || !pOut->para) return;
    uint8_t *d = (uint8_t *)pOut->para;
    if (!d[0]) return;  /* has_reg */
    AppPower_OnCommMgrData(pOut);          /* v2.2 PULL: Para_Grp_t 直传 */
    uint8_t head = d[1];
    AppCooking_OnRegData((uint16_t)head, d);   /* v1.x compat */
    AppProtect_OnRegData((uint16_t)head, d);   /* v1.x compat */
}

static void _route_protect(void)
{
    Para_Grp_t *pOut = s_slots[SLOT_PROTECT].pOut;
    if (!pOut || !pOut->para) return;
    uint8_t *d = (uint8_t *)pOut->para;
    if (!d[0]) return;  /* has_err */
    AppPower_OnProtectData(pOut);          /* v2.2 PULL: Para_Grp_t 直传 */
}

static void _route_key(void)
{
    Para_Grp_t *pOut = s_slots[SLOT_KEY].pOut;
    if (!pOut || !pOut->para) return;
    uint8_t *d = (uint8_t *)pOut->para;
    if (!d[0]) return;  /* has_key */
    uint16_t param = (uint16_t)d[1] | ((uint16_t)d[2] << 8);
    AppHmi_OnKey(param, NULL);
    AppCooking_OnKey(param, NULL);
    AppSegAlign_OnKey(param, NULL);
}

/* ===== AppHmi _onOutput（ST_OUT 触发，即时路由） ===== */
void AppHmi_OnOutput(Para_Grp_t *pOut)
{
    uint8_t *d = (uint8_t *)pOut->para;
    if (d[1]) {  /* has_buzzer */
        uint8_t sound = d[2];  /* buzzer_on = 枚举值 */
        switch (sound) {
        case 1:  DrvBuzzer_OnCtrl(1, NULL); break;  /* KEY_TAP */
        case 2:  DrvBuzzer_OnCtrl(1, NULL); break;  /* KEY_LONG */
        case 3:  DrvBuzzer_OnCtrl(1, NULL); break;  /* OP_OK */
        case 4:  DrvBuzzer_OnCtrl(0, NULL); break;  /* OP_FAIL */
        case 5:  DrvBuzzer_OnCtrl(1, NULL); break;  /* ALARM */
        default: break;
        }
    }
    /* 提示音已即时路由，显示在 _route_hmi 中处理 */
}

static void _route_hmi(void)
{
    Para_Grp_t *pOut = s_slots[SLOT_HMI].pOut;
    if (!pOut || !pOut->para) return;
    uint8_t *d = (uint8_t *)pOut->para;
    if (d[0]) {
        DrvDisplay_OnRefresh(0, d + 4);  /* d+4 = display data start */
    }
}

void Switcher_Run(void)
{
    /* Phase 1+2 交错: Producer DoWork → 立即路由 → Consumer DoWork */

    /* AppCommMgr: producer of register data */
    if (s_slots[SLOT_COMM_MGR].pDoWork) s_slots[SLOT_COMM_MGR].pDoWork();
    _route_comm_mgr();

    /* AppPower: consumer of reg data + system error, producer of power commands */
    if (s_slots[SLOT_POWER].pDoWork) s_slots[SLOT_POWER].pDoWork();

    /* AppProtect: consumer of reg data, producer of system errors */
    if (s_slots[SLOT_PROTECT].pDoWork) s_slots[SLOT_PROTECT].pDoWork();
    _route_protect();

    /* AppCooking: not yet migrated */

    /* AppHmi: producer of display data (buzzer via @OUTPUT_CALLBACK) */
    if (s_slots[SLOT_HMI].pDoWork) s_slots[SLOT_HMI].pDoWork();
    _route_hmi();

    /* Remaining modules in order */
    if (s_slots[SLOT_SEG_ALIGN].pDoWork) s_slots[SLOT_SEG_ALIGN].pDoWork();

    /* DrvKey: producer of key events */
    if (s_slots[SLOT_KEY].pDoWork) s_slots[SLOT_KEY].pDoWork();
    _route_key();

    if (s_slots[SLOT_COMM_MGR_DRV].pDoWork) s_slots[SLOT_COMM_MGR_DRV].pDoWork();
    if (s_slots[SLOT_BUZZER].pDoWork) s_slots[SLOT_BUZZER].pDoWork();
    if (s_slots[SLOT_DISPLAY].pDoWork) s_slots[SLOT_DISPLAY].pDoWork();
}
