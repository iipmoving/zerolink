/* drv_pan_count.c — 检锅脉冲计数
 * layer: DRV (base class)
 *
 * 职责: 检锅脉冲计数初始化/读取/设置，DMA IRQ 回调。
 *       自声明 PanPluse 状态结构体，不 extern。
 *
 * 迁出自: app_power_claude.c
 *    APP_POWER_PanCountInitChX    (line 5938)
 *    APP_POWER_PanCountInitCh1~4  (line 5953)
 *    APP_POWER_PanCountGetValue   (line 5972)
 *    APP_POWER_PanCountSetValue   (line 5983)
 *    API_DMA_PAN_IRQHandlerCallBack (line 6000)
 */

#include "data_type.h"

/* === 外部函数声明 (过渡) === */
void APP_ADC_PanSwChange(uint32_t ch);
void APP_ADC_DMA_RecoverPan(uint8_t ch);

/* === 检锅通道常量 === */
#ifndef PotCh1
#define PotCh1  0
#endif
#ifndef PotCh2
#define PotCh2  1
#endif
#ifndef PotCh3
#define PotCh3  2
#endif
#ifndef PotCh4
#define PotCh4  3
#endif

/* === 检锅状态常量 (原全局 enum/define) === */
enum {
    PanPluseEnd = 0,
    PanCheckRest = 1,

};

/* === 自声明 PanPluse 结构 === */
static struct pan_pluse_ctx {
    uint8_t  ch;            /* 当前通道 */
    uint8_t  res;           /* 状态 */
    uint16_t pulse_count;   /* 脉冲计数值 */
} s_pan_pluse;


/* ===================================================================
 *  APP_POWER_PanCountInitChX — 检锅脉冲计数初始化
 * =================================================================== */
void APP_POWER_PanCountInitChX(uint8_t ch)
{
    /* HRTIM_STUB */              /* 关PPG */
    /* HRTIM_STUB */              /* 开启TIM 1.3us检锅触发源 */
    s_pan_pluse.ch = ch;
}


/* ===================================================================
 *  APP_POWER_PanCountInitCh1~4 — 各通道初始化包装
 * =================================================================== */
void APP_POWER_PanCountInitCh1(void) { APP_POWER_PanCountInitChX(PotCh1); }
void APP_POWER_PanCountInitCh2(void) { APP_POWER_PanCountInitChX(PotCh2); }
void APP_POWER_PanCountInitCh3(void) { APP_POWER_PanCountInitChX(PotCh3); }
void APP_POWER_PanCountInitCh4(void) { APP_POWER_PanCountInitChX(PotCh4); }


/* ===================================================================
 *  APP_POWER_PanCountGetValue — 读取检锅脉冲数
 * =================================================================== */
uint8_t APP_POWER_PanCountGetValue(void)
{
    uint8_t xReturn;
    s_pan_pluse.res = PanCheckRest;
    xReturn = s_pan_pluse.pulse_count;
    return xReturn;
}


/* ===================================================================
 *  APP_POWER_PanCountSetValue — 开启/恢复计数器
 * =================================================================== */
void APP_POWER_PanCountSetValue(uint8_t onOff)
{
    (void)onOff;
    APP_ADC_PanSwChange(s_pan_pluse.ch);
    APP_ADC_DMA_RecoverPan(s_pan_pluse.ch);
}


/* ===================================================================
 *  API_DMA_PAN_IRQHandlerCallBack — DMA 中断回调
 * =================================================================== */
uint8_t API_DMA_PAN_IRQHandlerCallBack(uint8_t ch)
{
    /* HRTIM_STUB */
    APP_ADC_PanSwChange(0x20);       /* 关所有检锅通道 */
    s_pan_pluse.res = PanDmaEnd;     /* DMA读取完成 */
    return ch;
}
