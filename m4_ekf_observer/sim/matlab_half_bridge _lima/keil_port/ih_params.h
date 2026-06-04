/******************************************************************************
 * ih_params.h — 半桥感应加热 参数数据结构 (RX32G410 / Keil MDK)
 *
 * MCU: 中山翰林电器 RX32G410
 *   内核: Cortex-M4F (FPUv4 + DSP指令)
 *   主频: 192MHz (Flash零等待)
 *   HRTIM: 1×6计数器, 16位, 162ps分辨率 (≈6.17GHz等效)
 *   硬件加速: CORDIC(三角函数) + FMAC(FIR/IIR)
 *   ADC: 3×12位, 4Msps, 注入组同步采样, DMA
 *   编译器: Keil MDK (AC6), ARMCC / GCC
 *
 * 数据采集格式 (9列 / 串口DMA / SPI):
 *   t_us, I_adc, V_adc, Vdc_adc, CNT,
 *   CMP_UON, CMP_UOFF, CMP_LON, CMP_LOFF
 ******************************************************************************/

#ifndef IH_PARAMS_H
#define IH_PARAMS_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================== 硬件标定常数 ========================== */

/* --- 电压分压: 3×270K + 6.2K --- */
#define V_SCALE         131.65f     /* V_actual = V_adc_raw × V_SCALE */

/* --- 电流: CT 2000:1 + 2K负载 + 10K:330分压 --- */
#define I_SCALE         31.31f      /* I_actual = I_adc_raw × I_SCALE */

/* --- 直流母线电压 (同分压网络) --- */
#define VDC_SCALE       131.65f

/* --- HRTIM 硬件参数 --- */
#define HRTIM_PSEC      162.0f      /* 分辨率 162ps */
#define T_PER_CNT_US    (162.0e-12f * 1e6f)  /* ≈ 1.62e-4 μs/count */
#define HRTIM_MAX_CNT   65535       /* 16位计数器 */

/* --- 最小脉宽 --- */
#define MIN_PULSE_US    6.0f        /* 硬件保护 */

/* --- 工频 --- */
#define F_AC            50.0f       /* Hz */
#define T_AC_MS         20.0f       /* ms */

/* ========================== 硬件加速定义 ========================== */

/* RX32G410 CORDIC 协处理器寄存器 (需确认芯片头文件中的地址) */
#define CORDIC_BASE     0x40023000UL
#define CORDIC_CSR      (*(volatile uint32_t *)(CORDIC_BASE + 0x00))
#define CORDIC_WDATA    (*(volatile uint32_t *)(CORDIC_BASE + 0x04))
#define CORDIC_RDATA    (*(volatile uint32_t *)(CORDIC_BASE + 0x08))

/* CORDIC 功能选择 */
#define CORDIC_FUNC_SIN     0x00    /* SIN: 输出 sin(θ), cos(θ) */
#define CORDIC_FUNC_ATAN    0x04    /* ATAN: 输出 arctan(y/x) */
#define CORDIC_FUNC_MOD     0x08    /* MOD: 输出 sqrt(x²+y²) */

/* --- CORDIC 加速宏 (内联) --- */
__STATIC_INLINE float cordic_sin(float rad)
{
    /* RX32G410: 将 rad 写入 WDATA, 读取 RDATA 得到 sin/cos */
    CORDIC_CSR = CORDIC_FUNC_SIN | (24 << 4);  /* 24次迭代 */
    CORDIC_WDATA = *(uint32_t *)&rad;
    /* 等待完成 (轮询或中断) */
    while (CORDIC_CSR & (1 << 0));  /* BUSY位 */
    float result;
    result = *(float *)&CORDIC_RDATA;
    return result;
}

__STATIC_INLINE float cordic_cos(float rad)
{
    CORDIC_CSR = CORDIC_FUNC_SIN | (24 << 4);
    CORDIC_WDATA = *(uint32_t *)&rad;
    while (CORDIC_CSR & (1 << 0));
    /* 第二次读得到 cos */
    (void)*(float *)&CORDIC_RDATA;  /* 丢弃 sin */
    float result;
    result = *(float *)&CORDIC_RDATA;  /* 读 cos */
    return result;
}

__STATIC_INLINE float cordic_atan2(float y, float x)
{
    CORDIC_CSR = CORDIC_FUNC_ATAN | (24 << 4);
    CORDIC_WDATA = *(uint32_t *)&y;
    CORDIC_WDATA = *(uint32_t *)&x;
    while (CORDIC_CSR & (1 << 0));
    float result;
    result = *(float *)&CORDIC_RDATA;
    return result;
}

__STATIC_INLINE float cordic_magnitude(float x, float y)
{
    CORDIC_CSR = CORDIC_FUNC_MOD | (24 << 4);
    CORDIC_WDATA = *(uint32_t *)&x;
    CORDIC_WDATA = *(uint32_t *)&y;
    while (CORDIC_CSR & (1 << 0));
    float result;
    result = *(float *)&CORDIC_RDATA;
    return result;
}

/* 回退: 若无 CORDIC 头文件, 用 math.h */
#ifndef USE_CORDIC
#define sin_f   sinf
#define cos_f   cosf
#define atan2_f atan2f
#define sqrt_f  sqrtf
#else
#define sin_f   cordic_sin
#define cos_f   cordic_cos
#define atan2_f cordic_atan2
/* sqrt: 用 M4 FPU 硬件指令, 不需要 CORDIC */
#define sqrt_f  sqrtf
#endif

/* FMAC 滤波器加速 (用于 RMS 滑动计算) */
/* RX32G410 FMAC 基地址 (需确认) */
#define FMAC_BASE       0x40024000UL

/* ========================== 原始数据结构 ========================== */

/** 单采样点 (从 DMA 接收) */
typedef struct {
    float   t_us;           /* 时间戳 (μs) */
    float   I_adc;          /* 电流 ADC 原始值 */
    float   V_adc;          /* 谐振电压 ADC 原始值 */
    float   Vdc_adc;        /* 直流母线 ADC 原始值 */
    uint32_t CNT;           /* HRTIM 计数器 */
    uint32_t CMP_UON;       /* 上管开通 */
    uint32_t CMP_UOFF;      /* 上管关断 */
    uint32_t CMP_LON;       /* 下管开通 */
    uint32_t CMP_LOFF;      /* 下管关断 */
} RawPoint_t;

/** 一帧原始数据 */
typedef struct {
    RawPoint_t *points;
    uint32_t    count;
    float       t_span_ms;
    uint32_t    n_cols;         /* 8 或 9 */
    float       hrtim_clk_mhz;  /* 自动检测 */
    float       t_per_cnt_us;   /* μs/count (auto-detect) */
    char        data_mode;      /* 'A'=20ms, 'B'=单周期 */
} RawBuffer_t;

/* ========================== 计算结果 ========================== */

typedef struct {
    float t_UON_us;     float t_UOFF_us;
    float t_LON_us;     float t_LOFF_us;
    float T_UON_us;     float T_LON_us;
    float T_hrtim_us;
    float f_sw_kHz;     float D_U_pct;  float D_L_pct;
    float D_eff_pct;
    float DT1_us;       float DT2_us;   float DT_avg_us;
    bool  pulse_ok;
    char  ctrl_mode;    /* 'F'=FM, 'P'=PWM */
    float D_std;
    float f_sw_min_kHz; float f_sw_max_kHz; float f_sw_range_kHz;
    char  hb_mode[32];
} TimingResult_t;

typedef struct {
    float f_res_kHz;
    float I_peak_A;     float V_peak_V;
    float I_RMS_A;      float V_RMS_V;
    float I_avg_A;      float V_avg_V;
    float CF_I;         float FF_I;
    float phi_deg;
} WaveformResult_t;

typedef struct {
    float P_W;          float S_VA;     float Q_var;
    float PF;           float phi_deg;
    float E_cycle_J;    float R_eq_ohm;
    float Vdc_mean_V;   float Vdc_ripple_V; float Vdc_ripple_pct;
    float eta_pct;
    bool  has_vdc;
} PowerResult_t;

typedef struct {
    float Z_mag_ohm;    float phi_deg;
    float R_ohm;        float X_ohm;
    float L_eq_uH;      float C_eq_nF;
    float Q_factor;     float BW_Hz;
    char  network_type[32];
} ImpedanceResult_t;

typedef struct {
    TimingResult_t      timing;
    WaveformResult_t    waveform;
    PowerResult_t       power;
    ImpedanceResult_t   impedance;
    uint32_t            n_cycles;
} IH_Result_t;

/* 控制模式枚举 */
typedef enum {
    CTRL_MODE_FM  = 'F',
    CTRL_MODE_PWM = 'P'
} CtrlMode_t;

/* ========================== 辅助宏 ========================== */

#define DEG2RAD(d)  ((d) * 0.01745329252f)
#define RAD2DEG(r)  ((r) * 57.295779513f)
#define CLAMP(x,lo,hi) (((x)<(lo))?(lo):(((x)>(hi))?(hi):(x)))
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))

#ifdef __cplusplus
}
#endif

#endif /* IH_PARAMS_H */
