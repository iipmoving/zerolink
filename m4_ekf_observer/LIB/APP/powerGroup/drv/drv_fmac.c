/* drv_fmac.c — FMAC 检锅
 * layer: DRV (base class)
 *
 * 职责: FMAC 硬件滤波器配置、检锅脉冲检测、调试消息输出。
 *       自声明局部状态，不 extern。
 *
 * 迁出自: app_power_claude.c
 *    APP_POWER_FmacSetPan       (line 6166)
 *    API_POWER_PanFmac          (line 6206)
 *    API_POWER_PanCheckPluse    (line 6292)
 *    APP_POWER_PanPluseMessage  (line 6248)
 *    API_FMAC_AppPowerOverCallBack (line 6196)
 */

#include "data_type.h"
#include "API_FMAC.h"
#include "pluse.h"

/* === __weak getter: 系统 DMA/FMAC 缓冲 (由 app_power_claude.c 强符号覆盖) === */
__attribute__((weak)) uint16_t* drv_fmac_get_dma_buff(void) { return NULL; }
__attribute__((weak)) uint16_t* drv_fmac_get_fmac_buff(void) { return NULL; }

void PublicBuffFreeAll(void);
void* PubicBuffCalloc(uint32_t size);
int16_t* pluse_getIntervalsAddress(uint8_t idx);
uint16_t pluse_getIntervalsSize(uint8_t idx);
uint8_t pulse_check_process(int16_t* data, uint16_t len);
int PrintMessagePush(void* msg);

#ifndef Pan_ADC_DMA_BUFF_NUM
#define Pan_ADC_DMA_BUFF_NUM  128
#endif

#ifndef FmacLeve
#define FmacLeve  4
#endif

/* === 检锅状态 (与 drv_pan_count.c 共享类型但各自实例)
 *  过渡期各自声明, 后续通过 Switcher 路由数据
 */
enum {
    PanPluseEnd = 0,
    PanCheckRest = 1,
    PanDmaEnd = 2,
    PanFmacEnd = 3
};

static struct {
    uint8_t  ch;
    uint8_t  res;
    uint16_t pulse_count;
    uint16_t res16;
} s_pan;

/* ===================================================================
 *  APP_POWER_FmacSetPan — FMAC 检锅配置
 * =================================================================== */
void APP_POWER_FmacSetPan(uint8_t ch)
{
    API_FMAC_MEMDEF* fmacMem = API_FMAC_GetMemAddress();
    if (fmacMem->type == FmacStop) {
        fmacMem->type = FmacPan;
        PublicBuffFreeAll();
        uint16_t* fmac_buff = drv_fmac_get_fmac_buff();
        if (!fmac_buff) {
            /* __weak 未被覆盖: 无法获取 FMAC 缓冲，跳过 */
            return;
        }

#ifdef DEBUG_POWER_OUT_CONST
        fmacMem->X1 = (int16_t*)pluse_getIntervalsAddress(0);
        fmacMem->Y = (int16_t*)fmac_buff;
        fmacMem->outSize = pluse_getIntervalsSize(0);
#else
        fmacMem->X1 = (int16_t*)drv_fmac_get_dma_buff();
        fmacMem->Y = (int16_t*)fmac_buff;
        fmacMem->outSize = Pan_ADC_DMA_BUFF_NUM;
#endif
        fmacMem->ch = ch;
        /* HRTIM_STUB */
    }
}


/* ===================================================================
 *  API_FMAC_AppPowerOverCallBack — FMAC 完成回调
 * =================================================================== */
void API_FMAC_AppPowerOverCallBack(API_FMAC_MEMDEF* fmacMem)
{
    (void)fmacMem;
    s_pan.res = PanFmacEnd;
}


/* ===================================================================
 *  API_POWER_PanFmac — FMAC 检锅主入口
 * =================================================================== */
void API_POWER_PanFmac(void)
{
    if (s_pan.res == PanDmaEnd) {
        APP_POWER_FmacSetPan(s_pan.ch);
        API_POWER_PanCheckPluse();
    }
}


/* ===================================================================
 *  APP_POWER_PanPluseMessage — 检锅结果调试输出
 * =================================================================== */
void APP_POWER_PanPluseMessage(uint8_t ch)
{
#include "printMessage.h"
    MessageDef message;

#ifdef DEBUG_POWER_OUT_CONST
    message.array[0].buff = (uint16_t*)pluse_getIntervalsAddress(0);
#else
    message.array[0].buff = Pan_ADC_AdcDmaBuff;
#endif
    message.array[1].buff = Pan_ADC_AdcFmacBuff + 4;
    message.array[2].buff = 0;
    message.array[3].buff = 0;

    message.array[0].size = Pan_ADC_DMA_BUFF_NUM;
    message.array[1].size = Pan_ADC_DMA_BUFF_NUM;
    message.array[2].size = 0;
    message.array[3].size = 0;
    message.array[4].size = 0;
    message.array[5].size = 0;
    message.array[6].size = 0;
    message.array[7].size = 0;

    message.array[4].buff = 0;
    message.array[5].buff = 0;
    message.array[6].buff = 0;
    message.array[7].buff = 0;

    uint16_t para[2];
    para[0] = s_pan.pulse_count;
    message.paraArray.buff = para;
    message.paraArray.size = 2;
    message.num = PAN_MESSAGE;

    if (PrintMessagePush(message) == 0) {
        printf(" Message pan Printf Fail");
    }
    (void)ch;
}


/* ===================================================================
 *  API_POWER_PanCheckPluse — 脉冲检测
 * =================================================================== */
void API_POWER_PanCheckPluse(void)
{
    int16_t newValue, oldValue;
    int16_t div;
    int32_t avgValue = 0;

    {
        while (s_pan.res == PanDmaEnd);

        s_pan.res = PanPluseEnd;
        s_pan.pulse_count = pulse_check_process(
            Pan_ADC_AdcFmacBuff + FmacLeve / 2,
            Pan_ADC_DMA_BUFF_NUM - FmacLeve);

        APP_POWER_PanPluseMessage(s_pan.ch);
    }
    (void)newValue; (void)oldValue; (void)div; (void)avgValue;
}
