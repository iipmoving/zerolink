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

/* ---- APP_Power_GetIO 外部声明 (MODULE_EXPORT 在 app_power.c 生成) ---- */
void APP_Power_GetIO(Para_Grp_t **ppIn,
                     Para_Grp_t **ppOut,
                     void      (**ppDoWork)(void));

/* ===== 模块槽位枚举 ===== */
typedef enum {
    SLOT_ADC       = 0,
    SLOT_APP_POWER = 1,
    SLOT_COUNT
} SwitcherSlot_t;

/* ===== Module Slot ===== */
typedef struct {
    SwitcherSlot_t  idx;
    unsigned char   res[3];
    Para_Grp_t     *pIn;
    Para_Grp_t     *pOut;
    void          (*pDoWork)(void);
} ModuleSlotDef;

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
    APP_Adc_GetIO(&s_slot[SLOT_ADC].pIn,
                  &s_slot[SLOT_ADC].pOut,
                  &s_slot[SLOT_ADC].pDoWork);

    APP_Power_GetIO(&s_slot[SLOT_APP_POWER].pIn,
                    &s_slot[SLOT_APP_POWER].pOut,
                    &s_slot[SLOT_APP_POWER].pDoWork);
}

/* ================================================================
 * APP_Power_InputCallback (强符号) — 从 AppAdc PULL 数据
 *
 * 覆盖 MODULE_SKELETON(APP_Power) 生成的 weak 空壳。
 * 从 s_slot[SLOT_ADC].pOut 拉 AdcBlock_t.inputValue[30]
 * 写入 s_slot[SLOT_APP_POWER].pIn → g_in.inputValue[30]
 * ================================================================ */
void APP_Power_InputCallback(void)
{
    Adc_Output_t *adc_out = (Adc_Output_t *)s_slot[SLOT_ADC].pOut->para;
    PwrInShadow  *pwr_in  = (PwrInShadow *)s_slot[SLOT_APP_POWER].pIn->para;

    if (adc_out && adc_out->pBlock) {
        memcpy(pwr_in->inputValue, adc_out->pBlock->inputValue,
               sizeof(pwr_in->inputValue));
        pwr_in->status |= 0x02;
    }
    s_slot[SLOT_APP_POWER].pIn->info.status |= ST_NEW;
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
