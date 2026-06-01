#ifndef RESONANT_F0_H
#define RESONANT_F0_H

#include <stdint.h>

/* ========== MODBUS 0x4000 寄存器布局 (10 regs, 20 bytes) ========== */
typedef struct {
    uint32_t f0;       /* 0x4000-0x4001 谐振频率 Hz            */
    int32_t  q;        /* 0x4002-0x4003 Q值 Q12               */
    int32_t  l;        /* 0x4004-0x4005 L_adc Q12             */
    uint16_t i_peak;   /* 0x4006 峰值电流 ADC                  */
    uint16_t flags;    /* 0x4007 bit0=f0有效 bit1=Q有效 bit2=L有效 */
    uint16_t f_sw;     /* 0x4008 开关频率 Hz (HRTIM_CLK/period)  */
    uint16_t cycle;    /* 0x4009 更新计数 (每20ms+1)            */
} ResonantResult;

/* 4 通道全局结果 — MODBUS 0x4000 区直接映射 */
extern ResonantResult g_resonant_result[4];

/**
 * Calculate f0, Q, L from per-cycle charge/discharge current waveform.
 *
 * @param current    resonant current ADC array (full cycle, rectified)
 * @param hrtim      HRTIM counter array (same length)
 * @param voltage    bus voltage ADC array (same length, optional)
 * @param count      number of data points
 * @param highOff    upper switch turn-off HRTIM value
 * @param highOn     upper switch turn-on HRTIM value
 * @param period     full PWM period in HRTIM ticks
 * @param f0_out     [out] resonant frequency in Hz
 * @param q_out      [out] Q-factor (fixed Q12 = * 4096)
 * @param l_out      [out] inductance (ADC units, Q12)
 * @return bitmask: bit0=f0 valid, bit1=Q valid, bit2=L valid
 */
int CalcResonantParams(const uint16_t* current, const uint16_t* hrtim,
                       const uint16_t* voltage, uint16_t count,
                       uint16_t highOff, uint16_t highOn, uint16_t period,
                       uint32_t* f0_out, int32_t* q_out, int32_t* l_out);

/**
 * Run full resonant analysis on one cycle: f0, Q, L → g_resonant_result[ch].
 * Called every ~20ms (throttled from ~30kHz per-cycle).
 *
 * @param current   filtered current ADC array
 * @param hrtim     HRTIM counter array
 * @param voltage   bus voltage ADC array
 * @param count     number of data points in the cycle
 * @param highOff   upper switch turn-off HRTIM value
 * @param highOn    upper switch turn-on HRTIM value
 * @param period    full PWM period in HRTIM ticks
 * @param pot_ch    heater channel index 0-3
 */
void ResonantAnalysis_Run(const uint16_t* current, const uint16_t* hrtim,
                          const uint16_t* voltage, uint16_t count,
                          uint16_t highOff, uint16_t highOn, uint16_t period,
                          uint8_t pot_ch);

#endif
