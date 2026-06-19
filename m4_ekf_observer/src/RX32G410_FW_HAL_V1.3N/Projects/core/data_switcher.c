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
#include "../include_io/app_adc_io.h"
#include "../include_io/app_power_io.h"
#include "../include_io/calculator_io.h"
#include "../include_io/elec_params_io.h"
//#include "../include_io/drv_hrtim_io.h"
#include "../include_io/ekf_lkf_io.h"
#include "../include_io/telemetry_io.h"

/* ===== 模块槽位索引 (使用 SLOT 宏) ===== */
typedef enum {
    SLOT(AppAdc) = 0,
    SLOT(AppPower) = 1,
    SLOT(Calculator) = 2,
    SLOT(ElecParams) = 3,
    SLOT(EKF_LKF) = 4,
    SLOT(Telemetry) = 5,
    SLOT(COUNT)
} SwitcherSlot_t;

static ModuleSlotDef s_slot[SLOT_COUNT];

/* ================================================================
 * Switcher_Init — 注册全部模块的 GetIO
 * ================================================================ */
void Switcher_Init(void)
{
    SLOT_GETIO(AppAdc);
    SLOT_GETIO(AppPower);
    SLOT_GETIO(Calculator);

    SLOT_GETIO(ElecParams);
    SLOT_GETIO(EKF_LKF);
    SLOT_GETIO(Telemetry);
}

/* ================================================================
 * InputCallback 强符号实现 — 覆盖 MODULE_SKELETON 生成的 weak 空壳
 * 数据流: Producer → Consumer (PULL)
 * ================================================================ */

INPUT_CALLBACK(AppAdc)
{
    INPUT_GET_SLOT(Calculator, AppAdc);
//    do { \
//        MODULE_OUTPUT(Calculator) *__out = \
//            (MODULE_OUTPUT(Calculator) *)s_slot[SLOT(Calculator)].pOut->para; \
//        MODULE_INPUT(AppAdc)   *__in  = \
//            (MODULE_INPUT(AppAdc)   *)s_slot[SLOT(AppAdc)].pIn->para; \
//        __in->Calculator_params = (void*)&__out->AppAdc_params; \
//    } while (0);		
	
	
}
INPUT_CALLBACK(AppPower)
{
	
//    do { \
//        MODULE_OUTPUT(AppAdc) *__out = \
//            (MODULE_OUTPUT(AppAdc) *)s_slot[SLOT(AppAdc)].pOut->para; \
//        MODULE_INPUT(AppPower)   *__in  = \
//            (MODULE_INPUT(AppPower)   *)s_slot[SLOT(AppPower)].pIn->para; \
//        __in->AppAdc_params = (void*)&__out->AppPower_params; \
//    } while (0);	
		
	
    INPUT_GET_SLOT(AppAdc, AppPower);

    INPUT_GET_SLOT(EKF_LKF, AppPower);


	
}


INPUT_CALLBACK(Calculator)
{
    INPUT_GET_SLOT(AppAdc, Calculator);
}


INPUT_CALLBACK(EKF_LKF)
{
    INPUT_GET_SLOT(ElecParams, EKF_LKF);
}


INPUT_CALLBACK(ElecParams)
{
    INPUT_GET_SLOT(Calculator, ElecParams);
}

INPUT_CALLBACK(Telemetry)
{
//    INPUT_GET_SLOT(Calculator, Telemetry);
//    INPUT_GET_SLOT(ElecParams, Telemetry);
	
	    do { \
        MODULE_OUTPUT(Calculator) *__out = \
            (MODULE_OUTPUT(Calculator) *)s_slot[SLOT(Calculator)].pOut->para; \
        MODULE_INPUT(Telemetry)   *__in  = \
            (MODULE_INPUT(Telemetry)   *)s_slot[SLOT(Telemetry)].pIn->para; \
        __in->Calculator_params = (void*)__out->Telemetry_params; \
    } while (0);		
	
	
	
    do { \
        MODULE_OUTPUT(ElecParams) *__out = \
            (MODULE_OUTPUT(ElecParams) *)s_slot[SLOT(ElecParams)].pOut->para; \
        MODULE_INPUT(Telemetry)   *__in  = \
            (MODULE_INPUT(Telemetry)   *)s_slot[SLOT(Telemetry)].pIn->para; \
        __in->ElecParams_params = (void*)__out->Telemetry_params; \
    } while (0);		
	
}

/* ================================================================
 * Switcher_Slot_{Module} — 单模块独立执行
 * ================================================================ */

void Switcher_Slot_AppAdc(void) { s_slot[SLOT_AppAdc].pDoWork(); }
void Switcher_Slot_AppPower(void) { s_slot[SLOT_AppPower].pDoWork(); }
void Switcher_Slot_Calculator(void) { s_slot[SLOT_Calculator].pDoWork(); }
void Switcher_Slot_ElecParams(void) { s_slot[SLOT_ElecParams].pDoWork(); }
void Switcher_Slot_EKF_LKF(void) { s_slot[SLOT_EKF_LKF].pDoWork(); }
void Switcher_Slot_Telemetry(void) { s_slot[SLOT_Telemetry].pDoWork(); }

/* ================================================================
 * Switcher_Run_All — 一次执行全部模块 (批量模式)
 * ================================================================ */
void Switcher_Run_All(void)
{
//    Switcher_Slot_AppAdc();
//    Switcher_Slot_AppPower();
//    Switcher_Slot_Calculator();
//    Switcher_Slot_ElecParams();
//    Switcher_Slot_EKF_LKF();
}

/* ================================================================
 * Switcher_Run_{name} — SLOT 调用链条 (由 project.json slot_chains 定义)
 * ================================================================ */

/* Slot1 — 1ms 实时控制: AppAdc → AppPower */
void Switcher_Run_Slot1(void)
{
    Switcher_Slot_AppAdc();
    Switcher_Slot_AppPower();
}

/* TK — 20ms 周期: Calculator → ElecParams → EKF_LKF */
void Switcher_Run_TK(void)
{
    Switcher_Slot_Calculator();
    Switcher_Slot_ElecParams();
    Switcher_Slot_EKF_LKF();
    Switcher_Slot_Telemetry();
}

