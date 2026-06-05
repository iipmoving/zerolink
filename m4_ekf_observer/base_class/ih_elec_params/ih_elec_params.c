/**
 * @file    ih_elec_params.c
 * @brief   IH 电参数计算 — 纯算法集实现
 * @layer   base_class
 *
 * 三段式: 输入提取 → 计算(滤波/权重/聚合) → 输出回填
 * 本模块提供 STRONG 符号 IhElecParams_Calculate.
 * 调用方通过 __weak 空壳调用, 不 include 本模块 .h.
 */

#include "ih_elec_params.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* HRTIM 时钟: 768 MHz, perAdc = 384 cnt/step, 每步 0.5 μs */
#define HRTIM_CLK_HZ  768000000.0f


/* ============================================================
 *  __weak stubs — declare what this module NEEDS from others
 *
 *  本模块为纯算法集, 所有数据通过函数参数传入, 不依赖外部模块.
 *  如未来需要从其他模块获取数据, 在此声明 __weak 空壳.
 * ============================================================ */


/* ============================================================
 *  strong symbols — provide what OTHER modules call
 * ============================================================ */

/* IhElecParams_Calculate — see below (three-phase implementation) */


/* ============================================================
 *  internal helpers (static)
 * ============================================================ */

/* ---- insertion sort, n ≤ 20 ---- */
static void sort_f(float *buf, uint8_t n)
{
    uint8_t i, j;
    for (i = 1; i < n; i++) {
        float key = buf[i];
        j = i;
        while (j > 0 && buf[j - 1] > key) { buf[j] = buf[j - 1]; j--; }
        buf[j] = key;
    }
}

static float median_f(float *buf, uint8_t n)
{
    if (n == 0) return 0.0f;
    float tmp[20];
    uint8_t i;
    for (i = 0; i < n; i++) { tmp[i] = buf[i]; }
    sort_f(tmp, n);
    if (n % 2 == 1) return tmp[n / 2];
    return (tmp[n / 2 - 1] + tmp[n / 2]) * 0.5f;
}

/**
 * @brief 峰值限幅器: 最后一步增量不超过前两增量平均
 *
 * 正常峰值附近最后一步(d1)不应显著大于前两步(d2,d3)的平均.
 * 若 d1 > (d2+d3)/2, 将峰值修正为 I[pk-1] + (d2+d3)/2.
 *
 * @param I      电流数组 (float, A)
 * @param idx    候选峰值索引
 * @param n      数组长度
 * @param corr   [out] 修正后的峰值
 * @return       true=被限幅, false=未修正
 */
static bool peak_limiter(const float *I, uint16_t idx, uint16_t n, float *corr)
{
    if (idx < 3 || idx >= n) {
        *corr = I[idx];
        return false;
    }

    float d3 = I[idx - 2] - I[idx - 3];
    float d2 = I[idx - 1] - I[idx - 2];
    float d1 = I[idx]     - I[idx - 1];

    float avg_d2d3 = (d2 + d3) * 0.5f;

    if (d1 > avg_d2d3 && d1 > 0.0f) {
        *corr = I[idx - 1] + avg_d2d3;
        return true;
    }

    *corr = I[idx];
    return false;
}


/* ============================================================
 *  IhElecParams_Calculate — three-phase: INPUT → COMPUTE → OUTPUT
 * ============================================================ */

void IhElecParams_Calculate(void *in, void *out)
{
    IH_ElecInputDef *input  = (IH_ElecInputDef *)in;
    IH_ElecResult   *result = (IH_ElecResult   *)out;

    uint8_t i;

    /* ---- init output ---- */
    memset(result, 0, sizeof(IH_ElecResult));
    if (input == 0 || input->count == 0 || input->count > 20) {
        result->valid = false;
        return;
    }

    uint8_t cnt = input->count;

    /* ============================================================
     *  INPUT: 逐周期提取 I_pk, Vdc, I_act, f_sw, phi, L_raw
     * ============================================================ */

    float I_pk[20], Vdc_f[20], I_act[20], f_sw[20], phi[20];
    float L_raw[20], I_valid[20], Lv;
    uint8_t valid_cnt = 0;

    for (i = 0; i < cnt; i++) {
        IH_CycleDataDef *c = &input->cycle[i];

        I_pk[i]  = (float)c->peak_current  * IH_I_SCALE;
        Vdc_f[i] = (float)c->voltage       * IH_VDC_SCALE;
        I_act[i] = (float)c->active_current;

        /* f_sw = HRTIM_CLK / lowOff (Hz) */
        float loff = (float)c->hrtim.lowOff;
        f_sw[i] = (loff > 0.0f) ? (HRTIM_CLK_HZ / loff) : 0.0f;

        /* φ = (zero_cross - highOn) / (highOff - highOn) × 180° */
        float cond  = (float)(c->hrtim.highOff - c->hrtim.highOn);
        float delta = (float)c->zero_cross_high - (float)c->hrtim.highOn;
        phi[i] = (cond > 0.0f) ? (delta / cond * 180.0f) : 0.0f;

        /* KVL: L = (Vdc/2 + I/(ωC)) / (I × ω) */
        if (I_pk[i] <= 0.0f || f_sw[i] <= 0.0f) { continue; }
        float omega = 2.0f * (float)M_PI * f_sw[i];
        float Vcp  = I_pk[i] / (omega * IH_C_FARAD);
        float num  = Vdc_f[i] * 0.5f + Vcp;
        float den  = I_pk[i] * omega;
        if (den <= 1e-12f) { continue; }
        Lv = (num / den) * 1e6f;
        if (Lv <= 0.0f) { continue; }

        I_valid[valid_cnt] = I_pk[i];
        L_raw[valid_cnt]   = Lv;
        valid_cnt++;
    }

    /* ============================================================
     *  COMPUTE: 滤波 → 权重修正 → 中位数聚合 → 衍生参数
     * ============================================================ */

    if (valid_cnt < 1) { result->valid = false; return; }

    /* ---- Step 2: I_peak 阈值过滤 ---- */
    float I_med = median_f(I_valid, valid_cnt);
    float I_thr = I_med * IH_I_PEAK_MIN_RATIO;
    uint8_t kept = 0;
    float I_kept[20], L_kept[20];

    for (i = 0; i < valid_cnt; i++) {
        if (I_valid[i] >= I_thr) {
            I_kept[kept] = I_valid[i];
            L_kept[kept] = L_raw[i];
            kept++;
        }
    }
    if (kept < 1) {
        kept = valid_cnt;
        for (i = 0; i < kept; i++) { I_kept[i] = I_valid[i]; L_kept[i] = L_raw[i]; }
    }

    /* ---- Step 3: L_ref = L[argmax(I_peak)], 权重修正 ---- */
    uint8_t i_max = 0;
    for (i = 1; i < kept; i++) { if (I_kept[i] > I_kept[i_max]) { i_max = i; } }
    float L_ref = L_kept[i_max];
    float I_max = I_kept[i_max];

    float L_corr[20];
    for (i = 0; i < kept; i++) {
        float r = I_kept[i] / I_max;
        L_corr[i] = L_ref + (L_kept[i] - L_ref) * (r * r);
    }

    result->L_uH = median_f(L_corr, kept);

    /* ---- Step 4: 回显统计 ---- */
    result->I_peak_A = median_f(I_kept, kept);
    result->Vdc_mean = median_f(Vdc_f, cnt);
    result->f_sw_Hz  = median_f(f_sw, cnt);

    /* φ: 高 I_peak 周期平均 */
    {
        float s = 0;
        uint8_t n = 0;
        float thr = result->I_peak_A * IH_I_PEAK_MIN_RATIO;
        for (i = 0; i < cnt; i++) {
            if (I_pk[i] >= thr) { s += phi[i]; n++; }
        }
        result->phi_deg = (n > 0) ? (s / n) : phi[cnt / 2];
    }

    /* ---- Step 5: P_W = median(I_active × Vdc) ---- */
    {
        float P_arr[20];
        uint8_t pn = 0;
        for (i = 0; i < cnt; i++) {
            float p = (float)input->cycle[i].active_current
                    * (float)input->cycle[i].voltage * IH_VDC_SCALE;
            P_arr[pn++] = p;
        }
        result->P_W = median_f(P_arr, pn);
    }

    /* ---- Step 6: f_res / Q / R / Z (从 L + φ + f_sw 推导) ---- */
    float LC = (result->L_uH * 1e-6f) * IH_C_FARAD;
    float f_res = (LC > 1e-20f)
        ? (1.0f / (2.0f * (float)M_PI * sqrtf(LC)))
        : result->f_sw_Hz;
    result->f_res_kHz = f_res / 1e3f;

    float omega_res = 2.0f * (float)M_PI * f_res;
    float omega_sw  = 2.0f * (float)M_PI * result->f_sw_Hz;
    float phi_rad   = result->phi_deg * (float)M_PI / 180.0f;
    float ratio     = result->f_sw_Hz / f_res;
    float denom_q   = ratio - 1.0f / ratio;

    result->Q_factor = (fabsf(denom_q) > 1e-6f) ? (tanf(phi_rad) / denom_q) : 0.0f;

    if (result->Q_factor > 1e-6f) {
        result->R_ohm = omega_res * (result->L_uH * 1e-6f) / result->Q_factor;
    }

    result->I_rms = result->I_peak_A * 0.70710678f;

    float X_L = omega_sw * (result->L_uH * 1e-6f);
    float X_C = 1.0f / (omega_sw * IH_C_FARAD);
    result->X_ohm = X_L - X_C;
    result->Z_mag_ohm = sqrtf(result->R_ohm * result->R_ohm + result->X_ohm * result->X_ohm);

    /* ============================================================
     *  OUTPUT: 回填结果 + 有效性判定
     * ============================================================ */

    result->L_stable = result->L_uH;    /* Kalman 待实现 */
    result->L_fast   = result->L_uH;
    result->event    = 0;

    result->valid = (result->L_uH > IH_L_MIN_uH) && (result->f_res_kHz > IH_FRES_MIN_kHz);
}
