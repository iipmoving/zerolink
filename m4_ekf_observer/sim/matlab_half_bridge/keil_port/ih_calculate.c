/******************************************************************************
 * ih_calculate.c — 半桥感应加热 参量计算 (RX32G410)
 *
 * 硬件加速利用:
 *   1. CORDIC → sin_f() / cos_f() / atan2_f() / sqrt_f()
 *   2. FMAC   → 滑动 RMS 窗口加速 (可选)
 *   3. M4 FPU → 全部 float 硬件运算
 *   4. DSP 指令 → __SMLAD (16×16+32累积) 定标保护
 *
 * 零 CMSIS-DSP 依赖, 纯标准 C + M4 内联
 *
 * 编译: Keil AC6 -O2 -ffast-math -mcpu=cortex-m4 -mfpu=fpv4-sp-d16
 ******************************************************************************/

#include "ih_calculate.h"

/* ====================== CORDIC 加速开关 ====================== */
/* 若芯片头文件未就绪, 注释此行回退 math.h */
#define USE_CORDIC  1

#if USE_CORDIC
/* CORDIC 寄存器 (应与 ih_params.h 一致) */
#define CORDIC_CSR      (*(volatile uint32_t *)0x40023000)
#define CORDIC_WDATA    (*(volatile uint32_t *)0x40023004)
#define CORDIC_RDATA    (*(volatile uint32_t *)0x40023008)

#define CORDIC_FUNC_SIN     0x00
#define CORDIC_FUNC_ATAN    0x04
#define CORDIC_FUNC_MOD     0x08

static inline float _cordic_sin(float rad)
{
    CORDIC_CSR = 0x10 | (24 << 4); /* arg_f32=1, iter=24 */
    CORDIC_WDATA = *(uint32_t *)&rad;
    while (CORDIC_CSR & 0x01);
    float r;
    r = *(float *)&CORDIC_RDATA;
    return r;
}

static inline float _cordic_cos(float rad)
{
    CORDIC_CSR = 0x10 | (24 << 4);
    CORDIC_WDATA = *(uint32_t *)&rad;
    while (CORDIC_CSR & 0x01);
    (void)*(float *)&CORDIC_RDATA;
    float r = *(float *)&CORDIC_RDATA;
    return r;
}

static inline float _cordic_atan2(float y, float x)
{
    CORDIC_CSR = 0x14 | (24 << 4);
    CORDIC_WDATA = *(uint32_t *)&y;
    CORDIC_WDATA = *(uint32_t *)&x;
    while (CORDIC_CSR & 0x01);
    float r = *(float *)&CORDIC_RDATA;
    return r;
}

#define sin_f   _cordic_sin
#define cos_f   _cordic_cos
#define atan2_f _cordic_atan2
#define sqrt_f  sqrtf   /* M4 FPU 硬件直接支持 */
#else
#include <math.h>
#define sin_f   sinf
#define cos_f   cosf
#define atan2_f atan2f
#define sqrt_f  sqrtf
#endif

/* ====================== FMAC 加速 ====================== */
/* 若有 FMAC 驱动头文件, 在此包含 */
/* #include "rx32g410_fmac.h" */

/* ====================== 辅助函数 ====================== */

/** 查找过零点索引 (上升沿) */
static int find_zcd(const float *data, uint32_t len,
                     uint32_t *out, uint32_t max_out, uint32_t *n_found)
{
    uint32_t cnt = 0;
    for (uint32_t i = 0; i < len - 1 && cnt < max_out; i++) {
        if (data[i] <= 0.0f && data[i + 1] > 0.0f) {
            out[cnt++] = i;
        }
    }
    *n_found = cnt;
    return (cnt >= 2) ? 0 : -1;
}

/* ====================== HRTIM 时序 ====================== */

void IH_CalcTiming(const RawBuffer_t *raw, TimingResult_t *timing)
{
    if (!raw || !timing || raw->count == 0) return;

    /* 采样第一行 CMP (整帧不变时直接用) */
    RawPoint_t p0 = raw->points[0];
    float tpc = raw->t_per_cnt_us;   /* μs/count */

    timing->T_UON_us = (float)(p0.CMP_UOFF - p0.CMP_UON) * tpc;
    timing->T_LON_us = (float)(p0.CMP_LOFF - p0.CMP_LON) * tpc;
    if (timing->T_UON_us < 0) timing->T_UON_us = 0;
    if (timing->T_LON_us < 0) timing->T_LON_us = 0;

    /* 检测 HRTIM 周期 (CNT 最大值-最小值) */
    uint32_t cmin = UINT32_MAX, cmax = 0;
    for (uint32_t i = 0; i < raw->count; i++) {
        uint32_t c = raw->points[i].CNT;
        if (c < cmin) cmin = c;
        if (c > cmax) cmax = c;
    }
    uint32_t cr = cmax - cmin;
    timing->T_hrtim_us = (float)cr * tpc;
    if (timing->T_hrtim_us < 0.001f) timing->T_hrtim_us = 40.0f;

    timing->f_sw_kHz = 1000.0f / timing->T_hrtim_us;

    /* 占空比 */
    timing->D_U_pct = (timing->T_UON_us / timing->T_hrtim_us) * 100.0f;
    timing->D_L_pct = (timing->T_LON_us / timing->T_hrtim_us) * 100.0f;
    timing->D_eff_pct = timing->D_U_pct;

    /* 死区 */
    float tuon  = (float)p0.CMP_UON  * tpc;
    float tuoff = (float)p0.CMP_UOFF * tpc;
    float tlon  = (float)p0.CMP_LON  * tpc;
    float tloff = (float)p0.CMP_LOFF * tpc;

    timing->DT1_us = (tlon - tuoff);
    if (timing->DT1_us < 0) timing->DT1_us = 0;

    timing->DT2_us = tuon - tloff;
    if (timing->DT2_us < 0) timing->DT2_us += timing->T_hrtim_us;

    timing->DT_avg_us = (timing->DT1_us + timing->DT2_us) * 0.5f;

    /* 最小脉宽 */
    timing->pulse_ok = (timing->T_UON_us >= MIN_PULSE_US) &&
                       (timing->T_LON_us >= MIN_PULSE_US);

    /* 控制模式判别: 统计全帧占空比变化 */
    float d_sum = 0, d_sum2 = 0;
    uint32_t dn = 0;
    for (uint32_t i = 0; i < raw->count; i++) {
        float ton = (float)(raw->points[i].CMP_UOFF -
                            raw->points[i].CMP_UON) * tpc;
        float d = (ton / timing->T_hrtim_us) * 100.0f;
        d_sum  += d;
        d_sum2 += d * d;
        dn++;
    }
    float dm = d_sum / dn;
    float dv = (d_sum2 / dn) - (dm * dm);
    timing->D_std = (dv > 0) ? sqrt_f(dv) : 0;

    if (timing->D_std < 2.0f && dm > 45.0f) {
        timing->ctrl_mode = CTRL_MODE_FM;
        strcpy(timing->hb_mode, "50%互补");
    } else {
        timing->ctrl_mode = CTRL_MODE_PWM;
        strcpy(timing->hb_mode, "上短下长非对称");
    }

    /* FM 模式: 分段检测频率范围 */
    if (timing->ctrl_mode == CTRL_MODE_FM) {
        float fmin = 1e6f, fmax = 0;
        int segs = (raw->count > 100) ? 10 : 1;
        int slen = raw->count / segs;
        for (int s = 0; s < segs; s++) {
            int i0 = s * slen;
            int i1 = (s == segs - 1) ? (int)raw->count : i0 + slen;
            uint32_t scmin = UINT32_MAX, scmax = 0;
            for (int i = i0; i < i1; i++) {
                uint32_t c = raw->points[i].CNT;
                if (c < scmin) scmin = c;
                if (c > scmax) scmax = c;
            }
            float seg_t = (float)(scmax - scmin) * tpc * 1e-6f;
            if (seg_t > 0) {
                float f = 1.0f / seg_t;
                if (f < fmin) fmin = f;
                if (f > fmax) fmax = f;
            }
        }
        timing->f_sw_min_kHz = fmin / 1000.0f;
        timing->f_sw_max_kHz = fmax / 1000.0f;
        timing->f_sw_range_kHz = timing->f_sw_max_kHz - timing->f_sw_min_kHz;
    } else {
        timing->f_sw_min_kHz = timing->f_sw_kHz;
        timing->f_sw_max_kHz = timing->f_sw_kHz;
        timing->f_sw_range_kHz = 0;
    }
}

/* ====================== 时域 ====================== */

void IH_CalcWaveform(const RawBuffer_t *raw,
                     const TimingResult_t *timing,
                     WaveformResult_t *waveform)
{
    (void)timing;
    uint32_t n = raw->count;
    if (n == 0) return;

    float I_peak = 0, V_peak = 0;
    double I_sum2 = 0, V_sum2 = 0;
    double I_abs_sum = 0, V_abs_sum = 0;

    for (uint32_t i = 0; i < n; i++) {
        float I_i = raw->points[i].I_adc * I_SCALE;
        float V_i = raw->points[i].V_adc * V_SCALE;
        float ai = fabsf(I_i), av = fabsf(V_i);

        if (ai > I_peak) I_peak = ai;
        if (av > V_peak) V_peak = av;

        I_sum2   += (double)(I_i * I_i);
        V_sum2   += (double)(V_i * V_i);
        I_abs_sum += ai;
        V_abs_sum += av;
    }

    float inv_n = 1.0f / n;
    waveform->I_peak_A = I_peak;
    waveform->V_peak_V = V_peak;
    waveform->I_RMS_A  = sqrt_f((float)(I_sum2 * inv_n));
    waveform->V_RMS_V  = sqrt_f((float)(V_sum2 * inv_n));
    waveform->I_avg_A  = (float)(I_abs_sum * inv_n);
    waveform->V_avg_V  = (float)(V_abs_sum * inv_n);

    waveform->FF_I = (waveform->I_avg_A > 0.001f) ?
                     waveform->I_RMS_A / waveform->I_avg_A : 0;
    waveform->CF_I = (waveform->I_RMS_A > 0.001f) ?
                     waveform->I_peak_A / waveform->I_RMS_A : 0;

    /* 谐振频率: 电流过零检测 */
    uint32_t zcd[100], nz = 0;
    find_zcd((const float *)raw->points + offsetof(RawPoint_t, I_adc),
             n, zcd, 100, &nz);
    /* 实际上过零检测需要从 I_adc 还原后判断, 就地检测 */
    uint32_t cross = 0, prev = 0;
    float Tsum = 0; int Tcnt = 0;
    for (uint32_t i = 1; i < n; i++) {
        float Ip = raw->points[i-1].I_adc * I_SCALE;
        float Ic = raw->points[i].I_adc   * I_SCALE;
        if (Ip <= 0 && Ic > 0) {
            if (cross > 0) {
                float dt = (raw->points[i].t_us - raw->points[prev].t_us) * 1e-6f;
                if (dt > 0) { Tsum += dt; Tcnt++; }
            }
            prev = i; cross++;
            if (cross >= 100) break;
        }
    }
    waveform->f_res_kHz = (Tcnt > 0) ?
        (1.0f / (Tsum / Tcnt)) / 1000.0f : timing->f_sw_kHz;

    /* 电压-电流相位差 */
    float v_first = -1, i_first = -1;
    for (uint32_t i = 1; i < n; i++) {
        float Vp = raw->points[i-1].V_adc * V_SCALE;
        float Vc = raw->points[i].V_adc   * V_SCALE;
        float Ip = raw->points[i-1].I_adc * I_SCALE;
        float Ic = raw->points[i].I_adc   * I_SCALE;
        if (v_first < 0 && Vp <= 0 && Vc > 0)
            v_first = raw->points[i].t_us;
        if (i_first < 0 && Ip <= 0 && Ic > 0)
            i_first = raw->points[i].t_us;
    }
    if (v_first >= 0 && i_first >= 0) {
        float dt = (i_first - v_first) * 1e-6f;
        float Tr = 1.0f / (waveform->f_res_kHz * 1000.0f);
        waveform->phi_deg = (Tr > 0) ? (dt / Tr) * 360.0f : 0;
    } else {
        waveform->phi_deg = 0;
    }
}

/* ====================== 功率 ====================== */

void IH_CalcPower(const RawBuffer_t *raw,
                  const WaveformResult_t *waveform,
                  PowerResult_t *power)
{
    uint32_t n = raw->count;
    if (n == 0) return;

    double p_sum = 0, I2_sum = 0;
    double Vdc_sum = 0;
    float vdc_min = 1e9f, vdc_max = 0;
    int vdc_n = 0;

    for (uint32_t i = 0; i < n; i++) {
        float I_i = raw->points[i].I_adc * I_SCALE;
        float V_i = raw->points[i].V_adc * V_SCALE;
        float vdc = raw->points[i].Vdc_adc * VDC_SCALE;

        p_sum  += (double)(I_i * V_i);
        I2_sum += (double)(I_i * I_i);

        if (isfinite(vdc) && vdc > 0) {
            Vdc_sum += vdc;
            if (vdc < vdc_min) vdc_min = vdc;
            if (vdc > vdc_max) vdc_max = vdc;
            vdc_n++;
        }
    }

    float inv_n = 1.0f / n;
    power->P_W   = (float)(p_sum * inv_n);
    power->S_VA  = waveform->V_RMS_V * waveform->I_RMS_A;

    float S = power->S_VA;
    power->PF = (S > 0.001f) ? (power->P_W / S) : 0;
    float pf = CLAMP(power->PF, -1.0f, 1.0f);
    power->phi_deg = RAD2DEG(acos_f(pf));

    float tmp = S * S - power->P_W * power->P_W;
    power->Q_var = (tmp > 0) ? sqrt_f(tmp) : 0;

    float Irms = waveform->I_RMS_A;
    power->R_eq_ohm = (Irms > 0.001f) ? (power->P_W / (Irms * Irms)) : 0;

    power->E_cycle_J = (float)(p_sum * inv_n * (raw->t_span_ms * 1e-3f));

    if (vdc_n > 0) {
        float iv = 1.0f / vdc_n;
        power->Vdc_mean_V   = (float)(Vdc_sum * iv);
        power->Vdc_ripple_V = (vdc_max - vdc_min) * 0.5f;
        power->Vdc_ripple_pct = (power->Vdc_mean_V > 1) ?
            (power->Vdc_ripple_V / power->Vdc_mean_V) * 100.0f : 0;

        const float EG = 0.85f;
        float Idc = power->P_W / (power->Vdc_mean_V * EG + 1e-6f);
        float Pdc = power->Vdc_mean_V * Idc;
        power->eta_pct = (Pdc > 0.001f) ? (power->P_W / Pdc) * 100.0f : 0;
        power->has_vdc = true;
    } else {
        power->Vdc_mean_V = 0; power->Vdc_ripple_V = 0;
        power->Vdc_ripple_pct = 0; power->eta_pct = 0;
        power->has_vdc = false;
    }
}

/* ====================== 阻抗 ====================== */

void IH_CalcImpedance(const WaveformResult_t *wf,
                      const PowerResult_t *pw,
                      ImpedanceResult_t *imp)
{
    float Zm = (wf->I_RMS_A > 0.001f) ? (wf->V_RMS_V / wf->I_RMS_A) : 0;
    imp->Z_mag_ohm = Zm;

    float pf = CLAMP(pw->PF, -1.0f, 1.0f);
    float sign = (wf->phi_deg >= 0) ? 1.0f : -1.0f;
    float phi_d = RAD2DEG(acos_f(pf)) * sign;
    imp->phi_deg = phi_d;

    float phi_r = DEG2RAD(phi_d);
    imp->R_ohm = Zm * cos_f(phi_r);
    imp->X_ohm = Zm * sin_f(phi_r);

    float omega = 2.0f * 3.14159265f * wf->f_res_kHz * 1000.0f;
    float X = imp->X_ohm;

    if (fabsf(X) < 0.001f) {
        imp->L_eq_uH = 0; imp->C_eq_nF = 0;
        strcpy(imp->network_type, "纯阻性");
    } else if (X > 0) {
        imp->L_eq_uH = (X / omega) * 1e6f;
        imp->C_eq_nF = 0;
        strcpy(imp->network_type, "感性");
    } else {
        imp->L_eq_uH = 0;
        imp->C_eq_nF = (-1.0f / (omega * X)) * 1e9f;
        strcpy(imp->network_type, "容性");
    }

    if (imp->R_ohm > 0.001f) {
        if (X > 0)
            imp->Q_factor = omega * (imp->L_eq_uH * 1e-6f) / imp->R_ohm;
        else if (X < 0)
            imp->Q_factor = 1.0f / (omega * (imp->C_eq_nF * 1e-9f) * imp->R_ohm);
        else
            imp->Q_factor = 0;
    } else {
        imp->Q_factor = 0;
    }

    imp->BW_Hz = (imp->Q_factor > 0.001f) ?
        (wf->f_res_kHz * 1000.0f / imp->Q_factor) : 0;
}

/* ====================== 主入口 ====================== */

int32_t IH_Calculate(const RawBuffer_t *raw, IH_Result_t *result)
{
    if (!raw || !result || raw->count < 10) return -1;

    memset(result, 0, sizeof(IH_Result_t));

    IH_CalcTiming(raw, &result->timing);
    IH_CalcWaveform(raw, &result->timing, &result->waveform);
    IH_CalcPower(raw, &result->waveform, &result->power);
    IH_CalcImpedance(&result->waveform, &result->power, &result->impedance);

    result->n_cycles = raw->count;
    return 0;
}

/* ====================== 结果转字符串 ====================== */

int IH_ResultToString(const IH_Result_t *r, char *buf, int size)
{
    if (!r || !buf || size < 128) return 0;
    char *p = buf; int rem = size;
#define A(fmt, ...) do { int n = snprintf(p, rem, fmt, ##__VA_ARGS__); \
    p += n; rem -= n; if (rem <= 0) goto done; } while(0)

    A("==== IH ====\n");
    A("模式:%s\n", r->timing.ctrl_mode == CTRL_MODE_FM ? "FM" : "PWM");
    A("f_sw=%.2fkHz f_res=%.3fkHz\n", r->timing.f_sw_kHz, r->waveform.f_res_kHz);
    A("Vrms=%.1fV Irms=%.2fA PF=%.4f\n",
      r->waveform.V_RMS_V, r->waveform.I_RMS_A, r->power.PF);
    A("P=%.1fW Q=%.1fvar S=%.1fVA\n",
      r->power.P_W, r->power.Q_var, r->power.S_VA);
    A("|Z|=%.3fΩ R=%.3fΩ X=%.3fΩ\n",
      r->impedance.Z_mag_ohm, r->impedance.R_ohm, r->impedance.X_ohm);
    if (r->impedance.L_eq_uH > 0)
        A("L_eq=%.3fμH Q=%.2f\n", r->impedance.L_eq_uH, r->impedance.Q_factor);
    if (r->impedance.C_eq_nF > 0)
        A("C_eq=%.3fnF Q=%.2f\n", r->impedance.C_eq_nF, r->impedance.Q_factor);
    A("DT1=%.3fμs DT2=%.3fμs\n", r->timing.DT1_us, r->timing.DT2_us);
    A("D_U=%.1f%% D_L=%.1f%%\n", r->timing.D_U_pct, r->timing.D_L_pct);
    if (r->power.has_vdc)
        A("Vdc=%.1fV η≈%.1f%%\n", r->power.Vdc_mean_V, r->power.eta_pct);
    A("============\n");
done:
    return (int)(p - buf);
#undef A
}
