/**
 * @file    data_switcher.c
 * @brief   Data Switcher — PULL 路由调度器 (v2.2)
 * @layer   core
 *
 * Phase 2 单文件模式: APP_Power (app_power.c) 作为唯一功率模块。
 * PowerBase / PowerCalc 暂时排除 (IncludeInBuild=0)。
 *
 * 数据流（20ms 一周期内）：
 *   Slot 0: AppAdc.DoWork      → g_out (AdcBlock_t*)
 *   Slot 1: APP_Power.DoWork    → InputCallback PULL AdcBlock_t.inputValue
 *                               → ProcessInput: ADC 分发 + PowerTypeFun()
 *
 * APP_Power_InputCallback = 强符号 (本文件提供): 从 AppAdc.g_out PULL 数据
 */

#include "std_module.h"
#include "data_switcher.h"
#include "../include/app_adc_io.h"
#include "../include/calculator_io.h"
#include "../include/elec_params_io.h"

/* ---- APP_Power_GetIO 外部声明 (MODULE_EXPORT 在 app_power.c 生成) ---- */
void APP_Power_GetIO(Para_Grp_t **ppIn,
                     Para_Grp_t **ppOut,
                     void      (**ppDoWork)(void));

/* ---- Calculator + ElecParams 外部声明 ---- */
void Calculator_GetIO(Para_Grp_t **ppIn,
                      Para_Grp_t **ppOut,
                      void      (**ppDoWork)(void));
void ElecParams_GetIO(Para_Grp_t **ppIn,
                      Para_Grp_t **ppOut,
                      void      (**ppDoWork)(void));

/* ===== 模块槽位枚举 ===== */
typedef enum {
    SLOT(APP_Adc)    = 0,
    SLOT(Calculator) = 1,
    SLOT(ElecParams) = 2,
    SLOT(APP_Power)  = 3,
    SLOT(COUNT)
} SwitcherSlot_t;

/* ===== Module Slot 数组 (ModuleSlotDef 类型在 std_module.h 定义) ===== */
static ModuleSlotDef s_slot[SLOT_COUNT];

/* ---- 影子结构: 与 app_power.c:Power_Input_t 布局一致 ---- */
typedef struct {
    uint8_t  status;
    uint8_t  res[3];
    uint32_t inputValue[30];
} PwrInShadow;

/* ================================================================
 * Switcher_Init — 注册全部模块的 GetIO
 * ================================================================ */
void Switcher_Init(void)
{
    SLOT_GETIO(APP_Adc);
    SLOT_GETIO(Calculator);
    SLOT_GETIO(ElecParams);
    SLOT_GETIO(APP_Power);
}

/* ================================================================
 * APP_Power_InputCallback (强符号) — 从 AppAdc PULL 数据
 *
 * 覆盖 MODULE_SKELETON(APP_Power) 生成的 weak 空壳。
 * 从 s_slot[SLOT_APP_Adc].pOut 拉 AdcBlock_t.inputValue[30]
 * 写入 s_slot[SLOT_APP_Power].pIn → g_in.inputValue[30]
 * ================================================================ */
void APP_Power_InputCallback(void)
{
    Adc_Output_t *adc_out = (Adc_Output_t *)s_slot[SLOT_APP_Adc].pOut->para;
    PwrInShadow  *pwr_in  = (PwrInShadow *)s_slot[SLOT_APP_Power].pIn->para;

    if (adc_out && adc_out->pBlock) {
        memcpy(pwr_in->inputValue, adc_out->pBlock->inputValue,
               sizeof(pwr_in->inputValue));
        pwr_in->status |= 0x02;
    }
    s_slot[SLOT_APP_Power].pIn->info.status |= ST_NEW;
}

/* ================================================================
 * ElecParams_InputCallback (强符号) — 从 Calculator PULL 数据到 ElecParams
 *
 * 覆盖 MODULE_SKELETON(ElecParams) 生成的 weak 空壳。
 * 直穿赋值: ElecParams 输入 LINK 指针直接指向 Calculator 输出 LINK 地址。
 * 两结构体布局兼容 (cycles[80] ↔ params[4][20])，零拷贝。
 * ================================================================ */
void ElecParams_InputCallback(void)
{
    Calculator_Output *calc_out = (Calculator_Output *)s_slot[SLOT_Calculator].pOut->para;
    if (!calc_out) return;
    if (!(calc_out->elec_params.status & ST_NEW)) return;

    ElecParams_Input *elec_in = (ElecParams_Input *)s_slot[SLOT_ElecParams].pIn->para;
    if (!elec_in) return;

    // 直穿赋值: ElecParams 输入指针 → Calculator 输出 LINK (别名，零拷贝)
    elec_in->input = &calc_out->elec_params;

    s_slot[SLOT_ElecParams].pIn->info.status |= ST_NEW;
}
/* ================================================================
 * Switcher_Run_Slot1 — 按序调 DoWork
 *
 * DoWork 内部: InputCallback(PULL) → ProcessInput → OutputCallback(按需)
 * ================================================================ */
void Switcher_Run_Slot1(void)
{
    for (uint8_t i = 0; i < SLOT_COUNT; i++) {
        if (s_slot[i].pDoWork) {
            s_slot[i].pDoWork();
        }
    }
}
