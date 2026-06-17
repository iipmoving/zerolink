// ============================================================================
// ih_elec_params.c — IH 电参数计算
// 输入 IH_CycleDataDef[20], 逐周期 KVL → 权重修正 → 中位数聚合
// 与 MATLAB export_golden.m 公式链逐行对齐
// ============================================================================

#include "ih_elec_params.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// HRTIM 时钟: 768 MHz, perAdc = 384 cnt/step, 每步 0.5 μs
#define HRTIM_CLK_HZ        768000000.0f

// 内部: 排序 + 中位数 (n ≤ 20)
static void sort_f(float* buf, uint8_t n) {
    uint8_t i, j;
    for (i = 1; i < n; i++) {
        float key = buf[i];
        j = i;
        while (j > 0 && buf[j - 1] > key) { buf[j] = buf[j - 1]; j--; }
        buf[j] = key;
    }
}

static float median_f(float* buf, uint8_t n) {
    if (n == 0) return 0.0f;
    float tmp[20];
    for (uint8_t i = 0; i < n; i++) { tmp[i] = buf[i]; }
    sort_f(tmp, n);
    if (n % 2 == 1) return tmp[n / 2];
    return (tmp[n / 2 - 1] + tmp[n / 2]) * 0.5f;
}

// ========== 主计算 ====================================================
void IH_CalculateParams(IH_ElecInputDef* in, IH_ElecResult* out) {
    uint8_t i;

    memset(out, 0, sizeof(IH_ElecResult));
    if (in == 0 || in->count == 0 || in->count > 20) {
        out->valid = false;
        return;
    }

    uint8_t cnt = in->count;

    // ---- 逐周期提取数据 ------------------------------------------------
    float I_pk[20], Vdc_f[20], I_act[20], f_sw[20], phi[20];
    float L_raw[20], I_valid[20], Lv;
    uint8_t valid_cnt = 0;

    for (i = 0; i < cnt; i++) {
        IH_CycleDataDef* c = &in->cycle[i];

        I_pk[i]  = (float)c->peak_current  * IH_I_SCALE;
        Vdc_f[i] = (float)c->voltage       * IH_VDC_SCALE;
        I_act[i] = (float)c->active_current;

        // f_sw = HRTIM_CLK / lowOff (Hz)
        float loff = (float)c->hrtim.lowOff;
        f_sw[i] = (loff > 0.0f) ? (HRTIM_CLK_HZ / loff) : 0.0f;

        // φ = (zero_cross - highOn) / (highOff - highOn) × 180°
        float cond  = (float)(c->hrtim.highOff - c->hrtim.highOn);
        float delta = (float)c->zero_cross_high - (float)c->hrtim.highOn;
        phi[i] = (cond > 0.0f) ? (delta / cond * 180.0f) : 0.0f;

        // KVL: L = (Vdc/2 + I/(ωC)) / (I × ω)
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

    if (valid_cnt < 1) { out->valid = false; return; }

    // ---- Step 2: I_peak 阈值过滤 --------------------------------------
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

    // ---- Step 3: L_ref = L[argmax(I_peak)], 权重修正 ------------------
    uint8_t i_max = 0;
    for (i = 1; i < kept; i++) { if (I_kept[i] > I_kept[i_max]) { i_max = i; } }
    float L_ref = L_kept[i_max];
    float I_max = I_kept[i_max];

    float L_corr[20];
    for (i = 0; i < kept; i++) {
        float r = I_kept[i] / I_max;
        L_corr[i] = L_ref + (L_kept[i] - L_ref) * (r * r);
    }

    out->L_uH = median_f(L_corr, kept);

    // ---- Step 4: 回显 -------------------------------------------------
    out->I_peak_A = median_f(I_kept, kept);
    out->Vdc_mean = median_f(Vdc_f, cnt);
    out->f_sw_Hz  = median_f(f_sw, cnt);

    // φ: 高 I_peak 周期平均
    {
        float s = 0;
        uint8_t n = 0;
        float thr = out->I_peak_A * IH_I_PEAK_MIN_RATIO;
        for (i = 0; i < cnt; i++) {
            if (I_pk[i] >= thr) { s += phi[i]; n++; }
        }
        out->phi_deg = (n > 0) ? (s / n) : phi[cnt / 2];
    }

    // ---- Step 5: P_W = median(I_active × Vdc) (直接积分路径) ----------
    {
        float P_arr[20];
        uint8_t pn = 0;
        for (i = 0; i < cnt; i++) {
            float p = (float)in->cycle[i].active_current
                    * (float)in->cycle[i].voltage * IH_VDC_SCALE;
            P_arr[pn++] = p;
        }
        out->P_W = median_f(P_arr, pn);
    }

    // ---- Step 6: f_res / Q / R / Z (从 L + φ + f_sw 推) ---------------
    float LC = (out->L_uH * 1e-6f) * IH_C_FARAD;
    float f_res = (LC > 1e-20f)
        ? (1.0f / (2.0f * (float)M_PI * sqrtf(LC)))
        : out->f_sw_Hz;
    out->f_res_kHz = f_res / 1e3f;

    float omega_res = 2.0f * (float)M_PI * f_res;
    float omega_sw  = 2.0f * (float)M_PI * out->f_sw_Hz;
    float phi_rad   = out->phi_deg * (float)M_PI / 180.0f;
    float ratio     = out->f_sw_Hz / f_res;
    float denom_q   = ratio - 1.0f / ratio;

    out->Q_factor = (fabsf(denom_q) > 1e-6f) ? (tanf(phi_rad) / denom_q) : 0.0f;

    if (out->Q_factor > 1e-6f) {
        out->R_ohm = omega_res * (out->L_uH * 1e-6f) / out->Q_factor;
    }

    out->I_rms = out->I_peak_A * 0.70710678f;

    float X_L = omega_sw * (out->L_uH * 1e-6f);
    float X_C = 1.0f / (omega_sw * IH_C_FARAD);
    out->X_ohm = X_L - X_C;
    out->Z_mag_ohm = sqrtf(out->R_ohm * out->R_ohm + out->X_ohm * out->X_ohm);

    out->L_stable = out->L_uH;    // Kalman 待实现
    out->L_fast   = out->L_uH;
    out->event    = 0;

    out->valid = (out->L_uH > IH_L_MIN_uH) && (out->f_res_kHz > IH_FRES_MIN_kHz);
}
