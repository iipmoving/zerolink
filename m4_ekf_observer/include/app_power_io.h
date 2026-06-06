/**
 * @file    app_power_io.h
 * @brief   app_power Data Switcher IO interface
 * @layer   app (Data Switcher IO)
 *
 * 本文件定义 app_power 模块对外暴露的输入/输出接口.
 * 仅 data_switcher.c 可用全路径 include 本文件.
 */

#ifndef APP_POWER_IO_H
#define APP_POWER_IO_H       /* ← 保留: _io.h 是公开接口, 两个合法 include 方 */

#include <stdint.h>

/* === INTERFACE STRUCTS ==========================================
 * app_power 是 Power_Output_t 的 Owner (生产者),
 * 同时是 Power_Input_t 的 Consumer (消费者).
 *
 * 输入来源: Adc_Output_t.inputValue[] + 其他模块 output → Switcher 搬运
 * 输出去向: 后续 consumer 模块
 *
 * 协议 (详见 08-data-switcher.md):
 *   g_in.status   bit0=构造, bit1=新输入到达 (Switcher 设, 本模块消费后清)
 *   g_out.status  bit0=保留, bit1=输出就绪   (本模块设, Switcher 取走后清)
 * ================================================================ */

#pragma pack(4)

/* AdcGroupMax = 30, 由 Switcher 从 Adc_Output_t.inputValue[] 搬运 */
typedef struct {
    uint8_t  status;        /* bit0=构造, bit1=新输入到达 (Switcher 设, 本模块消费后清) */
    uint8_t  res[3];        /* 32位对齐 */
    uint32_t inputValue[30]; /* AdcGroupMax — 平均后的 ADC 值 */
} Power_Input_t;             /* sizeof=124 */

typedef struct {
    uint8_t  status;        /* bit0=保留, bit1=输出就绪 (本模块设, Switcher 清) */
    uint8_t  res[3];
    uint8_t  overcurrent;   /* 任意通道过流 */
    uint8_t  pot_detected;  /* 锅具检测完成 */
    uint16_t reserved;
} Power_Output_t;            /* sizeof=8 */

#pragma pack()

/* ---- public interface ---- */

void Power_GetIO(Power_Input_t **ppIn, Power_Output_t **ppOut);
void Power_DoWork(void);


#endif /* APP_POWER_IO_H */
