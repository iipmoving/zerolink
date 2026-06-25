/********************************************************************************
    FileName    :  resonant_msg.c
    Brief       :  谐振分析 MESSAGE — printMessage 框架 + MODBUS 寄存器输出
                   原 printf CSV → CalcResonantParams → g_resonant_result
    Copyright (c) Foshan XinSun Electronic Technology CO.,Ltd
********************************************************************************/

/********************************************Head Files*/
#include    "resonant_msg.h"
#include    "resonant_f0.h"
#include    <string.h>

uint16_t 		resonant_buff[2046];			// 预分配空间

#ifdef	ResonantMsg

#define			ResonantMsgBuffSize		4*180

#else
#define			ResonantMsgBuffSize		4*2
#endif

uint16_t 		resonant_outBuff[ResonantMsgBuffSize];
uint16_t*       resonant_memPoint = resonant_outBuff;	// 信息区首址

ResonantMsgDef       resonant_message;
ResonantMsgBuffDef   resonant_MessageBuff[10];
ResonantMsgBuffDef   resonant_paraBuff;

static uint8_t resonant_lock = 0;

/* ---- 内部辅助 ---------------------------------------------------------- */

static uint8_t resonant_msg_pop(ResonantMsgBuffDef* msg_src,
                                 ResonantMsgBuffDef* msg_dst)
{
    uint16_t size = msg_src->size;
    msg_dst->size = size;
    msg_dst->buff = resonant_memPoint;

    if (resonant_memPoint + size >
        resonant_outBuff + ResonantMsgBuffSize * sizeof(uint16_t)) {
        return 0;   // 缓存溢出
    }

    if (msg_src->buff != 0) {
        for (uint16_t j = 0; j < size; j++) {
            msg_dst->buff[j] = ((uint16_t*)msg_src->buff)[j];
        }
        resonant_memPoint += size;
    }
    return size;
}

/* ---- 数据压入 (与 PrintMessagePush 完全一致) --------------------------- */
uint8_t ResonantMsg_Push(ResonantMsgDef messageIn)
{
    uint8_t ret = 0;
#ifdef	ResonantMsg
    uint16_t size = messageIn.array[0].size;

    if (resonant_lock) {
        return ret;
    }
    resonant_lock = 1;
    resonant_message = messageIn;

    uint8_t messageIndex = 0;

    resonant_memPoint = resonant_outBuff;		// 信息区首址
    for (int i = 0; i < RESONANT_MSG_BUFF_SIZE; i++) {
        ret = resonant_msg_pop(&resonant_message.array[i],
                                &resonant_MessageBuff[i]);
        if (ret == 0) {
            return ret;
        }
    }

    ret = resonant_msg_pop(&resonant_message.paraArray,
                            &resonant_paraBuff);
    if (ret == 0) {
        return ret;
    }
#endif
    return ret;
}

/* ---- 谐振分析输出 (替代 printf) ---------------------------------------- */
/*
 * 数据来源 (与原 APP_ADC_TxaMessageOut 的 array 分配一致):
 *   array[0] = TxaHrtimBuff[PotChWork]     → HRTIM 时间戳
 *   array[1] = TxaVcBuff[PotChWork]        → 母线电压 ADC
 *   array[2] = TxaAdcBuff[PotChWork]       → 谐振电流 ADC (原始)
 *   array[3] = TxaFmacBuff[PotChWork]      → FMAC 滤波后电流
 *
 * paraArray (4 个 uint16_t):
 *   para[0] = highOff  (上管关断 HRTIM 值 → PPG 占空比)
 *   para[1] = phase    (相位角度)
 *   para[2] = zc_high  (过零点)
 *   para[3] = power    (功率值)
 *   para[4] = highOn   (上管开通 HRTIM 值 — 待 APP_ADC 配置)
 *   para[5] = period   (PWM 周期 HRTIM ticks — 待 APP_ADC 配置)
 *
 * 输出: g_resonant_result[0] → MODBUS 0x4000 区可见
 */
static void ResonantMsg_Run(void)
{
    /* 从 FMAC 滤波电流 (array[3]) 计算谐振参数 */
    const uint16_t* current  = resonant_MessageBuff[3].buff;
    const uint16_t* hrtim    = resonant_MessageBuff[0].buff;
    const uint16_t* voltage  = resonant_MessageBuff[1].buff;
    uint16_t        count    = resonant_MessageBuff[0].size;
    uint16_t        highOff  = resonant_paraBuff.buff[0];
    uint16_t        highOn   = 0;
    uint16_t        period   = 0;

    if (current == NULL || hrtim == NULL || count == 0) return;

    /* highOn / period: 从 para[4]/para[5] 读取 (待 APP_ADC 配置);
       未配置时从 HRTIM 数据自动推算 period */
    if (resonant_paraBuff.size > 4) {
        highOn = resonant_paraBuff.buff[4];
    }
    if (resonant_paraBuff.size > 5) {
        period = resonant_paraBuff.buff[5];
    }
    if (period == 0) {
        /* 自动推算: period = max(HRTIM) + 1 */
        for (uint16_t i = 0; i < count; i++) {
            if (hrtim[i] > period) period = hrtim[i];
        }
        period++;
    }

    uint32_t f0 = 0;
    int32_t  q  = 0;
    int32_t  l  = 0;
    uint16_t i_peak = 0;
    uint16_t i;

    for (i = 0; i < count; i++) {
        if (current[i] > i_peak) i_peak = current[i];
    }

    int flags = CalcResonantParams(current, hrtim, voltage, count,
                                    highOff, highOn, period, &f0, &q, &l);

    g_resonant_result[0].f0     = f0;
    g_resonant_result[0].q      = q;
    g_resonant_result[0].l      = l;
    g_resonant_result[0].i_peak = i_peak;
    g_resonant_result[0].flags  = (uint16_t)flags;
    g_resonant_result[0].f_sw   = (uint16_t)(768000000UL / period);
    g_resonant_result[0].cycle++;
}

/* ---- 非阻塞输出 (原 PrintMessageOut → 现写 MODBUS 寄存器) ------------ */
uint8_t ResonantMsg_Out(void)
{
#ifdef	ResonantMsg
    if (resonant_lock == 0) {
        return 0;
    }

    if (resonant_MessageBuff[0].size) {
        ResonantMsg_Run();
    }

    resonant_memPoint = resonant_outBuff;	// 清空空间
    resonant_lock = 0;
#endif
    return 1;
}
