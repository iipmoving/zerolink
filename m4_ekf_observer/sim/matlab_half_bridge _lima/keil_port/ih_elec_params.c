/********************************************************************************
    FileName    :  ih_elec_params.c
    Brief       :  半桥IH电参数计算
                  输入 4 组 uint16_t[20] + 导通占比,
                  逐周期 KVL → I_peak 权重修正 → 中位数聚合 → Kalman × 2
                  与 MATLAB export_golden.m 公式链逐行对齐.
    Date        :  2026-06-04
********************************************************************************/

#include "ih_elec_params.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* ========== 内部: 插入排序 (≤20) ====================================== */
static void sort_f(float* buf, uint8_t n)
{
    uint8_t i, j;
    for (i = 1; i < n; i++) {
        float key = buf[i];
        j = i;
        while (j > 0 && buf[j - 1] > key) { buf[j] = buf[j - 1]; j--; }
        buf[j] = key;
    }
}

static float median_f(float* buf, uint8_t n)
{
    if (n == 0) return 0.0f;
    float tmp[20];
    uint8_t i;
    for (i = 0; i < n; i++) tmp[i] = buf[i];
    sort_f(tmp, n);
    if (n % 2 == 1) return tmp[n / 2];
    return (tmp[n / 2 - 1] + tmp[n / 2]) * 0.5f;
}

/* ========== 单周期 L 计算 (KVL 封闭解, 与 MATLAB 逐行对齐) ============ */
static bool calc_one_cycle_L(float I_peak, float Vdc, float f_sw_hz, float* L_out)
{
    if (I_peak <= 0.0f || f_sw_hz <= 0.0f) return false;

    float omega_sw = 2.0f * (float)M_PI * f_sw_hz;
    float V_C_peak = I_peak / (omega_sw * IH_C_FARAD);
    float num = Vdc * 0.5f + V_C_peak;
    float den = I_peak * omega_sw;

    if (den <= 1e-12f) return false;
    float L_val = (num / den) * 1e6f;
    if (L_val <= 0.0f) return false;

    *L_out = L_val;
    return true;
}

/* ========== 主计算 ==================================================== */
void IH_CalculateParams(IH_ElecInput* in, IH_ElecResult* out)
{
    uint8_t i;

    memset(out, 0, sizeof(IH_ElecResult));
    if (in == 0 || in->I_peak_buf == 0 || in->count == 0) {
        out->valid = false;
        return;
    }

    uint8_t cnt = (in->count > 20) ? 20 : in->count;

    /* ---- 临时数组 ---- */
    float I_arr[20], V_arr[20], P_arr[20], F_arr[20];
    float I_valid[20], P_valid[20], L_raw[20];
    uint8_t valid_cnt = 0;

    /* ---- Step 1: 逐周期 KVL 算 L[i], I_valid 与 L_raw 同步 ---- */
    for (i = 0; i < cnt; i++) {
        I_arr[i] = (float)in->I_peak_buf[i] * IH_I_SCALE;
        V_arr[i] = (float)in->Vdc_buf[i]    * IH_VDC_SCALE;
        P_arr[i] = (float)in->phi_buf[i]    * IH_PHI_SCALE;
        F_arr[i] = (float)in->f_sw_buf[i]   * IH_FSW_SCALE;

        float Lv;
        if (calc_one_cycle_L(I_arr[i], V_arr[i], F_arr[i], &Lv)) {
            I_valid[valid_cnt] = I_arr[i];
            P_valid[valid_cnt] = P_arr[i];
            L_raw[valid_cnt] = Lv;
            valid_cnt++;
        }
    }

    if (valid_cnt < 1) { out->valid = false; return; }

    /* ---- Step 2: I_peak 阈值过滤 (仅有效周期) ---- */
    float I_med = median_f(I_valid, valid_cnt);
    float I_thr = I_med * IH_I_PEAK_MIN_RATIO;
    uint8_t kept = 0;
    float I_kept[20], P_kept[20], L_kept[20];

    for (i = 0; i < valid_cnt; i++) {
        if (I_valid[i] >= I_thr) {
            I_kept[kept] = I_valid[i];
            P_kept[kept] = P_valid[i];
            L_kept[kept] = L_raw[i];
            kept++;
        }
    }
    if (kept < 1) {
        kept = valid_cnt;
        for (i = 0; i < kept; i++) { I_kept[i] = I_valid[i]; P_kept[i] = P_valid[i]; L_kept[i] = L_raw[i]; }
    }

    /* ---- Step 3: L_ref = L[argmax(I_peak)] ---- */
    uint8_t i_max_idx = 0;
    for (i = 1; i < kept; i++)
        if (I_kept[i] > I_kept[i_max_idx]) i_max_idx = i;
    float L_ref = L_kept[i_max_idx];
    float I_max = I_kept[i_max_idx];

    float L_corr[20];
    for (i = 0; i < kept; i++) {
        float ratio = I_kept[i] / I_max;
        L_corr[i] = L_ref + (L_kept[i] - L_ref) * (ratio * ratio);
    }

    out->L_uH = median_f(L_corr, kept);

    /* ---- Step 4: 回显输入 ---- */
    out->I_peak_A = median_f(I_kept, kept);
    out->Vdc_mean = median_f(V_arr, cnt);

    /* φ: 高 I_peak 对应保留周期取平均 (与 I_kept 同步) */
    {
        float phi_sum = 0;
        uint8_t phi_n = 0;
        float I_phi_thr = out->I_peak_A * IH_I_PEAK_MIN_RATIO;
        for (i = 0; i < kept; i++) {
            if (I_kept[i] >= I_phi_thr) { phi_sum += P_kept[i]; phi_n++; }
        }
        out->phi_deg = (phi_n > 0) ? (phi_sum / phi_n) : P_kept[kept / 2];
    }

    /* f_sw 中位数 */
    out->f_sw_Hz = median_f(F_arr, cnt);

    /* ---- Step 5: φ 还原 (暂注 — 1ms 端已归一化到导通宽度) ---- */
    /* float duty_pu = (float)in->duty_ratio / 10000.0f;
    if (duty_pu > 0.01f) {
        float phi_raw = out->phi_deg;
        out->phi_deg = phi_raw / duty_pu;
        if (out->phi_deg > 90.0f) out->phi_deg = 90.0f;
    } */

    /* ---- Step 6: f_res / Q / R / P / Z ---- */
    float LC = (out->L_uH * 1e-6f) * IH_C_FARAD;
    float f_res_val = (LC > 1e-20f)
        ? (1.0f / (2.0f * (float)M_PI * sqrtf(LC)))
        : out->f_sw_Hz;
    out->f_res_kHz = f_res_val / 1e3f;

    float omega_res = 2.0f * (float)M_PI * f_res_val;
    float omega_sw  = 2.0f * (float)M_PI * out->f_sw_Hz;
    float phi_rad   = out->phi_deg * (float)M_PI / 180.0f;
    float ratio     = out->f_sw_Hz / f_res_val;
    float denom_q   = ratio - 1.0f / ratio;

    out->Q_factor = (fabsf(denom_q) > 1e-6f) ? (tanf(phi_rad) / denom_q) : 0.0f;

    if (out->Q_factor > 1e-6f)
        out->R_ohm = omega_res * (out->L_uH * 1e-6f) / out->Q_factor;

    out->I_rms = out->I_peak_A * 0.70710678f;
    out->P_W   = out->I_rms * out->I_rms * out->R_ohm;

    float X_L = omega_sw * (out->L_uH * 1e-6f);
    float X_C = 1.0f / (omega_sw * IH_C_FARAD);
    out->X_ohm = X_L - X_C;
    out->Z_mag_ohm = sqrtf(out->R_ohm * out->R_ohm + out->X_ohm * out->X_ohm);

    out->L_stable = out->L_uH;   /* Kalman 待实现 */
    out->L_fast   = out->L_uH;
    out->event    = 0;
    out->valid    = (out->L_uH > IH_L_MIN_uH) && (out->f_res_kHz > IH_FRES_MIN_kHz);
}
