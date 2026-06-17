/**
 * @file    data_switcher.c
 * @brief   Data Switcher — PULL 路由调度器 (v2.3)
 * @layer   core
 *
 * 自动生成，请勿手动修改！
 */
#include "core/std_module.h"
#include "data_switcher.h"

/* IO 接口文件 */
#include "include/appcommmgr_io.h"
#include "include/apppower_io.h"
#include "include/appprotect_io.h"
#include "include/appcooking_io.h"
#include "include/apphmi_io.h"
#include "include/appsegalign_io.h"
#include "include/drvkey_io.h"
#include "include/drvcommmgr_io.h"
#include "include/drvbuzzer_io.h"
#include "include/drvdisplay_io.h"


/* 模块槽位索引 (使用 SLOT 宏) */
typedef enum {
    SLOT(AppCommMgr) = 0,
    SLOT(AppPower) = 1,
    SLOT(AppProtect) = 2,
    SLOT(AppCooking) = 3,
    SLOT(AppHmi) = 4,
    SLOT(AppSegAlign) = 5,
    SLOT(DrvKey) = 6,
    SLOT(DrvCommMgr) = 7,
    SLOT(DrvBuzzer) = 8,
    SLOT(DrvDisplay) = 9,

    SLOT(COUNT)
} SwitcherSlot_t;

static ModuleSlotDef s_slots[SLOT(COUNT)];

void Switcher_Init(void)
{
    SLOT_GETIO(AppCommMgr);
    SLOT_GETIO(AppPower);
    SLOT_GETIO(AppProtect);
    SLOT_GETIO(AppCooking);
    SLOT_GETIO(AppHmi);
    SLOT_GETIO(AppSegAlign);
    SLOT_GETIO(DrvKey);
    SLOT_GETIO(DrvCommMgr);
    SLOT_GETIO(DrvBuzzer);
    SLOT_GETIO(DrvDisplay);

}

/* ===== Consumer InputCallback 强符号实现 — 指针直穿 (v2.3) ===== */

INPUT_CALLBACK(AppCommMgr, AppPower)
{
    INPUT_GET_SLOT(AppCommMgr, AppPower);
    INPUT_GET_SLOT(AppProtect, AppPower);
    INPUT_GET_SLOT(AppCooking, AppPower);
}

INPUT_CALLBACK(AppCommMgr, AppProtect)
{
    INPUT_GET_SLOT(AppCommMgr, AppProtect);
}

INPUT_CALLBACK(DrvKey, AppCooking)
{
    INPUT_GET_SLOT(DrvKey, AppCooking);
    INPUT_GET_SLOT(AppCommMgr, AppCooking);
}

INPUT_CALLBACK(DrvKey, AppHmi)
{
    INPUT_GET_SLOT(DrvKey, AppHmi);
    INPUT_GET_SLOT(AppPower, AppHmi);
}

INPUT_CALLBACK(DrvKey, AppSegAlign)
{
    INPUT_GET_SLOT(DrvKey, AppSegAlign);
}

INPUT_CALLBACK(AppPower, DrvCommMgr)
{
    INPUT_GET_SLOT(AppPower, DrvCommMgr);
}

INPUT_CALLBACK(AppHmi, DrvDisplay)
{
    INPUT_GET_SLOT(AppHmi, DrvDisplay);
}


/* ===== @OUTPUT_CALLBACK 即时路由 ===== */

void AppHmi_OutputCallback(Para_Grp_t *pOut)
{
    if (!pOut || !pOut->para) return;
    
    AppHmi_Output *out = (AppHmi_Output *)pOut->para;
    if (out->DrvBuzzer_params && (out->DrvBuzzer_params->status & ST_NEW)) {
        // TODO: 蜂鸣器即时路由逻辑
        out->DrvBuzzer_params->status &= ~ST_NEW;
    }
}


void Switcher_Run(void)
{
    if (s_slots[SLOT(AppCommMgr)].pDoWork) s_slots[SLOT(AppCommMgr)].pDoWork();
    if (s_slots[SLOT(AppPower)].pDoWork) s_slots[SLOT(AppPower)].pDoWork();
    if (s_slots[SLOT(AppProtect)].pDoWork) s_slots[SLOT(AppProtect)].pDoWork();
    if (s_slots[SLOT(AppCooking)].pDoWork) s_slots[SLOT(AppCooking)].pDoWork();
    if (s_slots[SLOT(AppHmi)].pDoWork) s_slots[SLOT(AppHmi)].pDoWork();
    if (s_slots[SLOT(AppSegAlign)].pDoWork) s_slots[SLOT(AppSegAlign)].pDoWork();
    if (s_slots[SLOT(DrvKey)].pDoWork) s_slots[SLOT(DrvKey)].pDoWork();
    if (s_slots[SLOT(DrvCommMgr)].pDoWork) s_slots[SLOT(DrvCommMgr)].pDoWork();
    if (s_slots[SLOT(DrvBuzzer)].pDoWork) s_slots[SLOT(DrvBuzzer)].pDoWork();
    if (s_slots[SLOT(DrvDisplay)].pDoWork) s_slots[SLOT(DrvDisplay)].pDoWork();

}
