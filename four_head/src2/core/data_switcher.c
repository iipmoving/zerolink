/**
 * @file    data_switcher.c
 * @brief   Data Switcher — PULL 路由调度器 (v2.3 LINK+PARAMS)
 * @layer   core
 *
 * ================================================================
 * [AI GENERATED] 此文件由 codeGen 自动生成，请勿手动修改
 * 修改方式: 编辑 project.json → 运行 GUI / code_gen.py 重新生成
 * ================================================================
 */

#include "std_module.h"
#include "data_switcher.h"

/* IO 接口文件 — 全模块接入 */
#include "include/drvkey_io.h"
#include "include/appcommmgr_io.h"
#include "include/appprotect_io.h"
#include "include/appcooking_io.h"
#include "include/apppower_io.h"
#include "include/apphmi_io.h"
#include "include/appsegalign_io.h"
#include "include/drvdisplay_io.h"
#include "include/drvbuzzer_io.h"
#include "include/drvcommmgr_io.h"

/* ===== 模块槽位索引 (使用 SLOT 宏) ===== */
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

static ModuleSlotDef s_slot[SLOT_COUNT];

/* ================================================================
 * Switcher_Init — 注册全部模块的 GetIO
 * ================================================================ */
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

/* ================================================================
 * InputCallback 强符号实现 — 覆盖 MODULE_SKELETON 生成的 weak 空壳
 * 数据流: Producer → Consumer (PULL)
 * ================================================================ */

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


INPUT_CALLBACK(DrvKey, AppSegAlign)
{
    INPUT_GET_SLOT(DrvKey, AppSegAlign);
}


INPUT_CALLBACK(AppHmi, DrvBuzzer)
{
    INPUT_GET_SLOT(AppHmi, DrvBuzzer);
}


INPUT_CALLBACK(AppPower, DrvCommMgr)
{
    INPUT_GET_SLOT(AppPower, DrvCommMgr);
}


INPUT_CALLBACK(AppHmi, DrvDisplay)
{
    INPUT_GET_SLOT(AppHmi, DrvDisplay);
}

/* ================================================================
 * Switcher_Run_Slot[N] — 独立执行链
 *
 * 每个函数对应一个模块的 DoWork，用户自由组合调用顺序：
 *
 *   // 示例: 自定义 1ms 控制流水线
 *   void MyControlLoop(void) {
 *       Switcher_Run_Slot0();  // AppCommMgr
 *       Switcher_Run_Slot1();  // AppPower
 *       Switcher_Run_Slot2();  // AppProtect
 *       Switcher_Run_Slot3();  // AppCooking
 *       Switcher_Run_Slot4();  // AppHmi
 *       Switcher_Run_Slot5();  // AppSegAlign
 *       Switcher_Run_Slot6();  // DrvKey
 *       Switcher_Run_Slot7();  // DrvCommMgr
 *       Switcher_Run_Slot8();  // DrvBuzzer
 *       Switcher_Run_Slot9();  // DrvDisplay
 *   }
 *
 * 或按需只执行部分链路：
 *   if (adc_ready) Switcher_Run_Slot0();
 *   if (power_on)  Switcher_Run_Slot3();
 * ================================================================ */

void Switcher_Run_Slot0(void) { s_slot[SLOT_AppCommMgr].pDoWork(); }
void Switcher_Run_Slot1(void) { s_slot[SLOT_AppPower].pDoWork(); }
void Switcher_Run_Slot2(void) { s_slot[SLOT_AppProtect].pDoWork(); }
void Switcher_Run_Slot3(void) { s_slot[SLOT_AppCooking].pDoWork(); }
void Switcher_Run_Slot4(void) { s_slot[SLOT_AppHmi].pDoWork(); }
void Switcher_Run_Slot5(void) { s_slot[SLOT_AppSegAlign].pDoWork(); }
void Switcher_Run_Slot6(void) { s_slot[SLOT_DrvKey].pDoWork(); }
void Switcher_Run_Slot7(void) { s_slot[SLOT_DrvCommMgr].pDoWork(); }
void Switcher_Run_Slot8(void) { s_slot[SLOT_DrvBuzzer].pDoWork(); }
void Switcher_Run_Slot9(void) { s_slot[SLOT_DrvDisplay].pDoWork(); }

/* ================================================================
 * Switcher_Run_All — 一次执行全部模块 (批量模式)
 *
 * 用于控制周期长的场景 (如 20ms 周期), 一次跑完所有槽。
 * 相当于按序调用全部 Switcher_Run_Slot[N]。
 * ================================================================ */
void Switcher_Run_All(void)
{
    Switcher_Run_Slot0();
    Switcher_Run_Slot1();
    Switcher_Run_Slot2();
    Switcher_Run_Slot3();
    Switcher_Run_Slot4();
    Switcher_Run_Slot5();
    Switcher_Run_Slot6();
    Switcher_Run_Slot7();
    Switcher_Run_Slot8();
    Switcher_Run_Slot9();
}
