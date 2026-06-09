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
#include "../include/app_power_io.h"

/* ---- APP_Power_GetIO 外部声明 (MODULE_EXPORT 在 app_power.c 生成) ---- */
void APP_Power_GetIO(Para_Grp_t **ppIn,
                     Para_Grp_t **ppOut,
                     void      (**ppDoWork)(void));

/* ---- DrvHrtim_GetIO (base_class/drv_hrtim_consumer.c MODULE_EXPORT) ---- */
void DrvHrtim_GetIO(Para_Grp_t **ppIn,
                    Para_Grp_t **ppOut,
                    void      (**ppDoWork)(void));

/* ===== 模块槽位枚举 ===== */
typedef enum {
    SLOT_ADC       = 0,
    SLOT_APP_POWER = 1,
    SLOT_DRV       = 2,
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

    DrvHrtim_GetIO(&s_slot[SLOT_DRV].pIn,
                   &s_slot[SLOT_DRV].pOut,
                   &s_slot[SLOT_DRV].pDoWork);
}

void 		ADC_to_Power_Link(void)
{
				APP_Adc_Output_t *adc_out = s_slot[SLOT_ADC].pOut->para;
			  PowerBase_Input_t  *pwr_in  =s_slot[SLOT_APP_POWER].pIn->para;
	
				for(unsigned char i=0;i<ADC_POTMAX;i++)
				{
					pwr_in->pAdc[i].current=adc_out->txa[i];
					pwr_in->pAdc[i].voltage=adc_out->voltage;
					pwr_in->pAdc[i].bottom=adc_out->bottom[i];
					pwr_in->pAdc[i].igbt=adc_out->igbt[i];
					pwr_in->pAdc[i].phase=adc_out->phase[i];
					pwr_in->pAdc[i].phaseDown=adc_out->phaseDown[i];
					pwr_in->pAdc[i].ceilQ=adc_out->ceilQ[i];
				}

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



		if(s_slot[SLOT_ADC].pOut->info.status&=ST_NEW)
		{
		
				//中间层从各个模块对数据进行搬运
//ADC 转换			
				APP_Adc_Output_t *adc_out = s_slot[SLOT_ADC].pOut->para;
			
				for(unsigned char i=0;i<ADC_POTMAX;i++)
				{
					 		ADC_to_Power_Link();
					
					
					
				}
	
//				
				
				
				s_slot[SLOT_APP_POWER].pIn->info.status |= ST_NEW;
		}	
		

}

/* ================================================================
 * Switcher_Run_Slot1 — 按序调 DoWork
 *
 * DoWork 内部: InputCallback(PULL) → ProcessInput → OutputCallback(按需)
 * ================================================================ */
void Switcher_Run_Slot1(void)
{
    /* Slot 0: ADC */
    s_slot[SLOT_ADC].pDoWork();

    /* Slot 1: APP_Power (InputCallback pulls ADC data from Slot 0) */
    s_slot[SLOT_APP_POWER].pDoWork();

    /* Middleware: APP_Power → DrvHrtim (hw_cmd) */
    {
        PowerBase_Output_t *app_out = (PowerBase_Output_t *)s_slot[SLOT_APP_POWER].pOut->para;
        memcpy(s_slot[SLOT_DRV].pIn->para, &app_out->hw_cmd, sizeof(PowerHw_Command_t));
        s_slot[SLOT_DRV].pIn->info.status |= ST_NEW;
    }

    /* Slot 2: DrvHrtim */
    s_slot[SLOT_DRV].pDoWork();

    /* Middleware: DrvHrtim → APP_Power (hw_status feedback) */
    {
        PowerBase_Input_t *app_in = (PowerBase_Input_t *)s_slot[SLOT_APP_POWER].pIn->para;
        memcpy(&app_in->hw_status, s_slot[SLOT_DRV].pOut->para, sizeof(PowerHw_Status_t));
    }
}
