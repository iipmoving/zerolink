/**
 * @file    app_adc_io.h
 * @brief   APP_ADC Data Switcher IO interface
 * @layer   app (Data Switcher IO)
 *
 * 本文件定义 APP_ADC 模块对外暴露的输出接口.
 * 仅 data_switcher.c 可用全路径 include 本文件.
 * APP_ADC 是纯生产者 — 只有 Output, 没有 Input.
 */

#ifndef APP_ADC_IO_H
#define APP_ADC_IO_H       /* ← 保留: _io.h 是公开接口, 两个合法 include 方 */

#include <stdint.h>

/* === INTERFACE STRUCTS ==========================================
 * APP_ADC 是 Adc_Output_t 的 Owner (生产者).
 *
 * 协议 (详见 08-data-switcher.md):
 *   status bit0=保留, bit1=新数据就绪 (本模块设, Switcher 清)
 *   DoWork 进入时先清 bit1, 有新产出时置位
 *   Switcher 检测 bit1 → 搬运字段到 consumer Input_t → 清 bit1
 * ================================================================ */

#pragma pack(4)

/* AdcGroupMax = 30, 索引见 APP_ADC.H 中的 AdcGroupXxx 枚举 */
typedef struct {
    uint8_t  status;        /* bit0=保留, bit1=新数据就绪 (本模块设, Switcher 清) */
    uint8_t  res[3];        /* 32位对齐 */
    uint32_t inputValue[30]; /* AdcGroupMax — 平均后的 ADC 值 */
} Adc_Output_t;              /* sizeof=124 */

#pragma pack()

/* ---- public interface ---- */

void Adc_GetIO(Adc_Output_t **ppOut);
void Adc_DoWork(void);


#endif /* APP_ADC_IO_H */
