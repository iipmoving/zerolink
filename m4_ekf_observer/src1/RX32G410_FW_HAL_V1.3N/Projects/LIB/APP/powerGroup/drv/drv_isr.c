/* drv_isr.c — ISR 回调原子模块
 * layer: DRV (base class)
 *
 * 职责: 硬件中断回调，原样迁入，不重写逻辑。
 *       自声明状态结构体，不 extern。
 *
 * 迁出自: app_power_claude.c
 *    API_HRTIM1_TEST_CMP1_IRQHandlerCallback  (line 108)
 *    API_ADC_Current1AWD_IRQHandlerCallBack    (line 125)
 *    API_ADC_Current2AWD_IRQHandlerCallBack    (line 137)
 *    (后续: AWD ISR, FMAC 回调)
 */

#include "data_type.h"
#include "API_DMA.h"

/* === AWD 限流 LINK 结构 (原 app_power_claude.c:75-99) === */
typedef struct {
    uint32_t num;
    uint32_t ch;
    uint32_t res;
    uint32_t dntr[5];
} Power_AwdDntr_LINK_t;

#define DNTR_BUFF_MAX  5
enum { DNTR_T12A = 0, DNTR_T34A = 1 };

static Power_AwdDntr_LINK_t Power_Dntr[2] = {{0,0,0,{0}}, {0,2,0,{0}}};
#define DNTR(id)  (&Power_Dntr[(id)])


/* === 外部函数/变量声明 (过渡: 仍在 app_power_claude.c/API 层) === */
void APP_ADC_IRQ_PPGstepDecTxA(Power_AwdDntr_LINK_t* txaDntr, uint16_t* txaBuff);
__attribute__((weak)) uint16_t* Adc_GetCurrentAdc2Ptr(void);
__attribute__((weak)) uint16_t* Adc_GetCurrentAdc3Ptr(void);

/* DMA 通道常量 (来自 API_DMA.h 或 API_DMA.C) */
#define ChDmaCurrentAdc2  6   /* 暂定, 需与 API 层对齐 */
#define ChDmaCurrentAdc3  7


/* ===================================================================
 *  API_HRTIM1_TEST_CMP1_IRQHandlerCallback
 *  原: app_power_claude.c:108
 *  CMP1 过流检测 ISR 回调 — 原样迁入
 * =================================================================== */
void API_HRTIM1_TEST_CMP1_IRQHandlerCallback(void)
{
    if (DNTR(DNTR_T12A)->num) {
        APP_ADC_IRQ_PPGstepDecTxA(DNTR(DNTR_T12A),
            Adc_GetCurrentAdc2Ptr());
    }

    if (DNTR(DNTR_T34A)->num) {
        APP_ADC_IRQ_PPGstepDecTxA(DNTR(DNTR_T34A),
            Adc_GetCurrentAdc3Ptr());
    }
    DNTR(DNTR_T12A)->num = 0;
    DNTR(DNTR_T34A)->num = 0;
}


/* ===================================================================
 *  API_ADC_Current1AWD_IRQHandlerCallBack
 *  原: app_power_claude.c:125
 *  T12A AWD 过流中断回调 — 原样迁入
 * =================================================================== */
void API_ADC_Current1AWD_IRQHandlerCallBack(void)
{
    volatile uint32_t value;
    value = API_DMA_GetDmaCndtr(ChDmaCurrentAdc2);
    /* HRTIM_STUB */
    DNTR(DNTR_T12A)->dntr[DNTR(DNTR_T12A)->num] = value;
    if (DNTR(DNTR_T12A)->num < DNTR_BUFF_MAX - 1) {
        DNTR(DNTR_T12A)->num++;
    }
}


/* ===================================================================
 *  API_ADC_Current2AWD_IRQHandlerCallBack
 *  原: app_power_claude.c:137
 *  T34A AWD 过流中断回调 — 原样迁入
 * =================================================================== */
void API_ADC_Current2AWD_IRQHandlerCallBack(void)
{
    volatile uint32_t value;
    value = API_DMA_GetDmaCndtr(ChDmaCurrentAdc3);
    /* HRTIM_STUB */
    DNTR(DNTR_T34A)->dntr[DNTR(DNTR_T34A)->num] = value;
    if (DNTR(DNTR_T34A)->num < DNTR_BUFF_MAX - 1) {
        DNTR(DNTR_T34A)->num++;
    }
}
