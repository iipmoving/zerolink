/* drv_protect.c — 保护硬件设定
 * layer: DRV (base class)
 *
 * 职责: OVP 值设定、AWD 保护值设定、ADC 过流检测、DMA 环形缓冲拷贝。
 *       自声明局部状态，不 extern。
 *
 * 迁出自: app_power_claude.c
 *    APP_POWER_CompSetValue       (line 1119)
 *    getOvpValueAdj              (line 1302)
 *    APP_POWER_SetTxaAwdValue    (line 1377)
 *    APP_POWERR_SetTxaAwdValue   (line 6029)
 *    APP_ADC_getOverAdcChannel   (line 6059)
 *    getTxaDmaCircleValue        (line 6040)
 */

#include "data_type.h"
#include "API_ADC.h"
#include "API_DAC.h"

/* === 外部函数声明 === */
void API_ADC_TxaAwdValue(uint32_t value);
void API_DAC_COMP_SetValue(uint8_t ch, uint32_t value);

/* === 自声明 struct === */
static struct {
    uint16_t ovp_value;      /* was PowerOvpValue */
    uint32_t ovp_value_all;  /* was PowerOvpValueAll */
} s_prot;


/* ===================================================================
 *  APP_POWER_CompSetValue — 设置比较器保护值 (OVP + 限流)
 *  注: 原函数遍历 PowerMem[0..POTNUM] 取各通道 ovpShort/CompDacValue/OvpValue
 *      当前简化: 设默认值, 待 Switcher 传入通道数据
 * =================================================================== */
void APP_POWER_CompSetValue(void)
{
    /* HRTIM_STUB: 待接收各通道 OVP 值 */
}


/* ===================================================================
 *  getOvpValueAdj — 设置谐振电流过流 CMP 保护值
 * =================================================================== */
void getOvpValueAdj(void)
{
    /* 原函数从 VC_LIMIT_MAX/PowerOvpValue 计算
     * 当前简化: 设默认值
     */
    s_prot.ovp_value = 0x96;       /* VC_LIMIT_MAX 典型值 */
}


/* ===================================================================
 *  APP_POWER_SetTxaAwdValue — 更新 AWD 保护值
 * =================================================================== */
void APP_POWER_SetTxaAwdValue(void)
{
    API_ADC_TxaAwdValue(s_prot.ovp_value_all);
}


/* ===================================================================
 *  APP_POWERR_SetTxaAwdValue — 设置 AWD (谐振电流)
 *  注: 原函数名含 typo, 保持兼容
 * =================================================================== */
void APP_POWERR_SetTxaAwdValue(void)
{
    uint32_t ovpvalue = s_prot.ovp_value_all;
    API_ADC_TxaAwdValue(ovpvalue);
}


/* ===================================================================
 *  APP_ADC_getOverAdcChannel — 检测 ADC 过流通道
 * =================================================================== */
uint8_t APP_ADC_getOverAdcChannel(uint16_t* value, uint16_t ovpValue, uint8_t num)
{
    uint8_t over = 0;
    for (uint8_t i = 0; i < num; i++) {
        if (value[i] > ovpValue) {
            over++;
        }
    }
    return over;
}


/* ===================================================================
 *  getTxaDmaCircleValue — 从 DMA 循环队列拷贝数据
 * =================================================================== */
void getTxaDmaCircleValue(uint16_t* src, uint16_t* dst, uint16_t start, uint16_t size)
{
#ifndef Current_ADC_AdcDma_BUFF_NUM
#define Current_ADC_AdcDma_BUFF_NUM  128
#endif

    for (uint8_t i = 0; i < size; i++) {
        dst[i] = src[start];
        if (start == 0) {
            start = Current_ADC_AdcDma_BUFF_NUM;
        } else {
            start--;
        }
    }
}
