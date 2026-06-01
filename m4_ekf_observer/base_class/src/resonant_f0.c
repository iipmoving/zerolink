#include <stdint.h>
#include <string.h>
#include "resonant_f0.h"

/* FRE_PER_ADC = 384 counts per 0.5us  =>  effective clock = 384/0.5us = 768 MHz */
#define HRTIM_CLK_EFF   768000000UL
#define HALF_MIN_TICKS  2500    /* ~10us @768MHz -> f0 < 50kHz */
#define HALF_MAX_TICKS  20000   /* ~26us @768MHz -> f0 > 19kHz */
#define Q12_SCALE       4096

/* 4通道全局结果 — MODBUS 0x4000 区直接映射 */
ResonantResult g_resonant_result[4];

/**
 * Find a current minimum ("zero-crossing" of AC component) within [start, end).
 * Returns array index of the valley, or 0 if not found.
 */
static uint16_t FindCurrentMin(const uint16_t* current, uint16_t start, uint16_t end)
{
    if (current == NULL || start >= end) return 0;

    uint16_t direction = 0xFFFF;
    uint16_t pre = current[start];
    uint32_t minZ = 0xFFFFFFFF;
    uint16_t candidate = 0;

    for (uint16_t i = start + 1; i < end; i++) {
        uint16_t now = current[i];

        direction <<= 1;
        if (now > pre) direction |= 1;
        else           direction &= ~1;

        if ((direction & 0x3) == 0x1) {
            uint16_t pt = i - 1;
            uint32_t zr = (uint32_t)current[pt - 1] + current[pt + 1];

            if (minZ > zr) {
                minZ = zr;
                if (current[pt] <= (zr >> 1)) {
                    candidate = pt;
                }
            }
        }
        pre = now;
    }
    return candidate;
}

/**
 * Calculate f0, Q, and L from per-cycle charge/discharge current waveform.
 *
 * Method:
 *   1. f0 from zero-crossing timing (current valley spacing)
 *   2. Q  from phase angle:  tan(phi) = Q * (f/f0 - f0/f)
 *   3. L  from switch-on di/dt:  L = Vbus * dt / dI
 *
 * @param current    resonant current ADC array (full cycle)
 * @param hrtim      HRTIM counter array (same length)
 * @param voltage    bus voltage ADC array (same length)
 * @param count      number of data points
 * @param highOff    upper switch turn-off HRTIM value
 * @param highOn     upper switch turn-on HRTIM value
 * @param period     full PWM period in HRTIM ticks
 * @param f0_out     [out] resonant frequency in Hz
 * @param q_out      [out] Q-factor (fixed Q12)
 * @param l_out      [out] inductance in ADC units (Vbus*us / di)
 * @return 1 if valid, 0 if measurement failed
 */
int CalcResonantParams(const uint16_t* current, const uint16_t* hrtim,
                       const uint16_t* voltage, uint16_t count,
                       uint16_t highOff, uint16_t highOn, uint16_t period,
                       uint32_t* f0_out, int32_t* q_out, int32_t* l_out)
{
    int ret = 0;

    /* --- 1. f0 from zero-crossings --- */
    uint16_t half = count / 2;
    uint16_t zc1 = FindCurrentMin(current, 4, half);
    uint16_t zc2 = FindCurrentMin(current, half, count - 1);

    if (zc1 < 4 || zc2 <= zc1) goto done;

    uint32_t t1 = hrtim[zc1];
    uint32_t t2 = hrtim[zc2];
    if (t2 < t1) t2 += period;

    uint32_t dh = t2 - t1;
    if (dh < HALF_MIN_TICKS || dh > HALF_MAX_TICKS) goto done;

    uint32_t f0 = HRTIM_CLK_EFF / (2 * dh);
    if (f0_out) *f0_out = f0;
    ret |= 1;

    /* --- 2. Q from phase angle --- */
    /* phase angle = time from switch-on to ZC, converted to degrees */
    if (hrtim[zc1] > highOn) {
        uint32_t angle_deg = (uint32_t)(hrtim[zc1] - highOn) * 180;
        angle_deg /= (highOff - highOn);

        /* f_sw = HRTIM_CLK_EFF / period */
        uint32_t f_sw = HRTIM_CLK_EFF / period;

        /* tan(phi) = Q * (f_sw/f0 - f0/f_sw) */
        /* Q = tan(phi) * f0 * f_sw / (f_sw*f_sw - f0*f0) */
        /* Use Q12 fixed point for tan(phi) lookup */

        /* Simplified: for most IH operating points, f_sw < f0,
           Q = |tan(phi)| * f0 * f_sw / (f0*f0 - f_sw*f_sw) */

        int32_t df2 = (int32_t)f0*f0 - (int32_t)f_sw*f_sw;
        if (df2 > 0 && angle_deg < 90) {
            /* tan approximation for integer math: tan(x) ~ x*Q12_PI/180 for small angles
               for larger angles use a small lookup or linear approx */
            /* tan(angle) * 4096 */
            int32_t tan_q12;
            if (angle_deg <= 45) {
                /* tan(x) ~ x * 71.51 (Q12) for x in degrees */
                tan_q12 = (int32_t)angle_deg * 71;  /* 71.51 * deg ~ tan*4096 */
            } else if (angle_deg <= 75) {
                tan_q12 = (int32_t)angle_deg * 150;  /* coarser approx */
            } else {
                tan_q12 = 16384;  /* tan(76°) ≈ 4.0 */
            }
            int32_t q_q12 = (tan_q12 * (int32_t)f0 * (int32_t)f_sw) / df2;
            if (q_out) *q_out = q_q12;
            ret |= 2;
        }
    }

    /* --- 3. L from switch-on di/dt --- */
    {
        /* Find the linear current rise during upper-switch ON period.
           The rise region is from near-zc1 (min current) to the peak
           before the next zero-crossing. */
        uint16_t rise_start = zc1;
        uint16_t rise_end   = zc1;
        uint16_t peak = 0;

        /* search forward from zc1 for the current peak */
        for (uint16_t i = zc1 + 1; i < half && i < count - 2; i++) {
            if (current[i] > peak) {
                peak = current[i];
                rise_end = i;
            }
            /* stop when current starts falling */
            if (i > zc1 + 3 && current[i] < current[i - 1] && current[i] < current[i - 2]) {
                break;
            }
        }

        if (rise_end > rise_start + 3) {
            uint32_t dI = current[rise_end] - current[rise_start];
            uint32_t dt_ticks = 0;
            if (hrtim[rise_end] > hrtim[rise_start]) {
                dt_ticks = hrtim[rise_end] - hrtim[rise_start];
            }

            if (dI > 10 && dt_ticks > 0) {
                /* Vbus average over rise region */
                uint32_t vsum = 0;
                for (uint16_t i = rise_start; i <= rise_end; i++) {
                    vsum += voltage[i];
                }
                uint32_t n_pts = rise_end - rise_start + 1;
                uint32_t vbus = vsum / n_pts;

                /* L_adc = Vbus * dt / dI  (dt in ticks -> convert to us: dt/768)
                   L_adc = Vbus * dt_ticks / (dI * 768)
                   For Q12 output: L_adc * 4096 */
                int32_t L = (int32_t)vbus * (int32_t)dt_ticks * 4096 /
                            ((int32_t)dI * 768);
                if (l_out) *l_out = L;
                ret |= 4;
            }
        }
    }

done:
    return ret;
}

/* ========== ResonantAnalysis_Run ====================================== */
void ResonantAnalysis_Run(const uint16_t* current, const uint16_t* hrtim,
                          const uint16_t* voltage, uint16_t count,
                          uint16_t highOff, uint16_t highOn, uint16_t period,
                          uint8_t pot_ch)
{
    uint32_t f0 = 0;
    int32_t  q  = 0;
    int32_t  l  = 0;
    uint16_t i_peak = 0;
    uint16_t i;

    if (pot_ch >= 4) return;

    /* find peak current in this cycle */
    for (i = 0; i < count; i++) {
        if (current[i] > i_peak) i_peak = current[i];
    }

    int flags = CalcResonantParams(current, hrtim, voltage, count,
                                    highOff, highOn, period, &f0, &q, &l);

    g_resonant_result[pot_ch].f0     = f0;
    g_resonant_result[pot_ch].q      = q;
    g_resonant_result[pot_ch].l      = l;
    g_resonant_result[pot_ch].i_peak = i_peak;
    g_resonant_result[pot_ch].flags  = (uint16_t)flags;
    g_resonant_result[pot_ch].f_sw   = (uint16_t)(HRTIM_CLK_EFF / period);
    g_resonant_result[pot_ch].cycle++;
}
