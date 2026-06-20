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
#include "../include_io/drv_key_io.h"
#include "../include_io/app_comm_mgr_io.h"
#include "../include_io/app_cooking_io.h"
#include "../include_io/app_hmi_io.h"
#include "../include_io/app_power_io.h"
#include "../include_io/app_protect_io.h"
#include "../include_io/app_seg_align_io.h"
#include "../include_io/drv_buzzer_io.h"
#include "../include_io/drv_comm_mgr_io.h"
#include "../include_io/drv_display_io.h"
#include "../include_io/proto_modbus_io.h"

/* ===== 模块槽位索引 ===== */
typedef enum {
    SLOT_AppCommMgr = 0,
    SLOT_ProtoModbus = 1,
    SLOT_AppPower = 2,
    SLOT_AppProtect = 3,
    SLOT_AppCooking = 4,
    SLOT_AppHmi = 5,
    SLOT_AppSegAlign = 6,
    SLOT_DrvKey = 7,
    SLOT_DrvCommMgr = 8,
    SLOT_DrvBuzzer = 9,
    SLOT_DrvDisplay = 10,
    SLOT_COUNT,
} SwitcherSlot_t;

static ModuleSlotDef s_slot[SLOT_COUNT];

/* ================================================================
 * Switcher_Init — 注册全部模块的 GetIO
 * ================================================================ */
void Switcher_Init(void)
{
    SLOT_GETIO(AppCommMgr);
    SLOT_GETIO(ProtoModbus);
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

INPUT_CALLBACK(AppCommMgr)
{
    INPUT_GET_SLOT(ProtoModbus, AppCommMgr);
    INPUT_GET_SLOT(DrvCommMgr, AppCommMgr);
}


INPUT_CALLBACK(AppCooking)
{
    INPUT_GET_SLOT(DrvKey, AppCooking);
    INPUT_GET_SLOT(AppCommMgr, AppCooking);
}


INPUT_CALLBACK(AppHmi)
{
    INPUT_GET_SLOT(DrvKey, AppHmi);
    INPUT_GET_SLOT(AppPower, AppHmi);
}


INPUT_CALLBACK(AppPower)
{
    INPUT_GET_SLOT(AppCommMgr, AppPower);
    INPUT_GET_SLOT(AppCooking, AppPower);
    INPUT_GET_SLOT(AppProtect, AppPower);
}


INPUT_CALLBACK(AppProtect)
{
    INPUT_GET_SLOT(AppCommMgr, AppProtect);
}


INPUT_CALLBACK(AppSegAlign)
{
    INPUT_GET_SLOT(DrvKey, AppSegAlign);
}


INPUT_CALLBACK(DrvBuzzer)
{
    INPUT_GET_SLOT(AppHmi, DrvBuzzer);
}

OUTPUT_CALLBACK(DrvBuzzer)
{
    s_slot[SLOT_DrvBuzzer].pDoWork();
}


INPUT_CALLBACK(DrvCommMgr)
{
    INPUT_GET_SLOT(AppPower, DrvCommMgr);
    INPUT_GET_SLOT(AppCommMgr, DrvCommMgr);
}


INPUT_CALLBACK(DrvDisplay)
{
    INPUT_GET_SLOT(AppHmi, DrvDisplay);
    INPUT_GET_SLOT(AppSegAlign, DrvDisplay);
}


INPUT_CALLBACK(ProtoModbus)
{
    INPUT_GET_SLOT(AppCommMgr, ProtoModbus);
}

/* ================================================================
 * Switcher_Slot_{Module} — 单模块独立执行
 * ================================================================ */

void Switcher_Slot_AppCommMgr(void) { s_slot[SLOT_AppCommMgr].pDoWork(); }
void Switcher_Slot_ProtoModbus(void) { s_slot[SLOT_ProtoModbus].pDoWork(); }
void Switcher_Slot_AppPower(void) { s_slot[SLOT_AppPower].pDoWork(); }
void Switcher_Slot_AppProtect(void) { s_slot[SLOT_AppProtect].pDoWork(); }
void Switcher_Slot_AppCooking(void) { s_slot[SLOT_AppCooking].pDoWork(); }
void Switcher_Slot_AppHmi(void) { s_slot[SLOT_AppHmi].pDoWork(); }
void Switcher_Slot_AppSegAlign(void) { s_slot[SLOT_AppSegAlign].pDoWork(); }
void Switcher_Slot_DrvKey(void) { s_slot[SLOT_DrvKey].pDoWork(); }
void Switcher_Slot_DrvCommMgr(void) { s_slot[SLOT_DrvCommMgr].pDoWork(); }
void Switcher_Slot_DrvBuzzer(void) { s_slot[SLOT_DrvBuzzer].pDoWork(); }
void Switcher_Slot_DrvDisplay(void) { s_slot[SLOT_DrvDisplay].pDoWork(); }

/* ================================================================
 * Switcher_Run_All — 一次执行全部模块 (批量初始化模式)
 * ================================================================ */
void Switcher_Run_All(void)
{
    Switcher_Slot_AppCommMgr();
    Switcher_Slot_ProtoModbus();
    Switcher_Slot_AppPower();
    Switcher_Slot_AppProtect();
    Switcher_Slot_AppCooking();
    Switcher_Slot_AppHmi();
    Switcher_Slot_AppSegAlign();
    Switcher_Slot_DrvKey();
    Switcher_Slot_DrvCommMgr();
    Switcher_Slot_DrvBuzzer();
    Switcher_Slot_DrvDisplay();
}

/* ================================================================
 * main __weak 强符号实现
 * ================================================================ */

void Slot_Init(void)
{
    Switcher_Init();
    Switcher_Run_All();
}

void Slot_every1ms(void) { }

void Slot_loop1ms(void)  { }

/* Slot0 — 定时消息生成(100ms/1s) */
void Switcher_Run_Slot0(void)
{
    Slot_TimerTick();
}

/* Slot1 — ProtoModbus 协议处理 */
void Switcher_Run_Slot1(void)
{
    Switcher_Slot_ProtoModbus();
}

/* Slot2 — AppPower 功率控制 */
void Switcher_Run_Slot2(void)
{
    Switcher_Slot_AppPower();
}

/* Slot3 — DrvKey 按键扫描 */
void Switcher_Run_Slot3(void)
{
    Switcher_Slot_DrvKey();
}

/* Slot4 — DrvCommMgr 通讯 + AppCommMgr */
void Switcher_Run_Slot4(void)
{
    Switcher_Slot_DrvCommMgr();
    Switcher_Slot_AppCommMgr();
}

/* Slot5 — AppProtect 保护检测 */
void Switcher_Run_Slot5(void)
{
    Switcher_Slot_AppProtect();
}

/* Slot6 — AppCooking 烹饪状态机 */
void Switcher_Run_Slot6(void)
{
    Switcher_Slot_AppCooking();
}

/* Slot7 — AppHmi 人机交互 */
void Switcher_Run_Slot7(void)
{
    Switcher_Slot_AppHmi();
}

/* Slot8 — AppSegAlign 对齐调校 */
void Switcher_Run_Slot8(void)
{
    Switcher_Slot_AppSegAlign();
}

/* Slot9 — DrvBuzzer 蜂鸣器 */
void Switcher_Run_Slot9(void)
{
    Switcher_Slot_DrvBuzzer();
}

/* Slot10 — DrvDisplay 显示更新 */
void Switcher_Run_Slot10(void)
{
    Switcher_Slot_DrvDisplay();
}
