/**
 * @file    ih_elec_params.h
 * @brief   IH 电参数计算 — 纯算法集 (base_class)
 * @layer   base_class
 * @deps    <math.h> <string.h> <stdint.h> <stdbool.h>
 */

#ifndef IH_ELEC_PARAMS_H
//#define IH_ELEC_PARAMS_H       /* L0 compiler block: cross-module include → redefinition error */


#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 *  标定常数
 * ============================================================ */

#define IH_C_FARAD          0.9e-6f     /* 谐振电容 0.9 μF */
#define IH_I_SCALE          0.02523f    /* I_adc → A */
#define IH_VDC_SCALE        0.10606f    /* Vdc_adc → V */
#define IH_PHI_SCALE        0.1f        /* phi 单位 0.1° → ° */

#define IH_I_PEAK_MIN_RATIO 0.3f        /* I_peak < 中位数×0.3 丢弃 */
#define IH_L_MIN_uH        10.0f        /* 有效 L 下限 */
#define IH_FRES_MIN_kHz    5.0f         /* 有效 f_res 下限 */


/* === INTERFACE STRUCTS (OWNER) ==================================
 *
 *  本模块是以下结构体的 Owner (生产者).
 *  消费者需独立声明副本 (不同类型名, 相同内存布局).
 *
 *  格式: @STRUCT struct_name owner suffix
 *  check_structs.py 解析本段, 验证所有 consumer 副本一致性.
 *
 *  消费者 AI 生成流程: /new-module Step 4 Mode B
 *    1. AI 读取本段 → 2. 生成消费者副本 (source= 注解)
 *    → 3. 写入消费者 .h AI-MANAGED 段 → 4. check_structs.py 验证
 *
 *  IMPORTANT: 字段变更后必须通知所有消费者重新生成副本.
 * ============================================================ */

/* @STRUCT IH_HrtimState    owner=ih_elec_params  suffix=OUT */
typedef struct {
    uint16_t highOn;                   /* offset=0, size=2 */
    uint16_t highOff;                  /* offset=2, size=2 */
    uint16_t lowOn;                    /* offset=4, size=2 */
    uint16_t lowOff;                   /* offset=6, size=2 */
} IH_HrtimState;                       /* sizeof=8 */

/* @STRUCT IH_CycleDataDef  owner=ih_elec_params  suffix=OUT */
typedef struct {
    IH_HrtimState hrtim;               /* offset=0,  size=8 — nested */
    uint16_t peak_current;             /* offset=8,  size=2 */
    uint16_t active_current;           /* offset=10, size=2 */
    uint16_t voltage;                  /* offset=12, size=2 */
    uint16_t zero_cross_high;          /* offset=14, size=2 */
} IH_CycleDataDef;                     /* sizeof=16 */

/* @STRUCT IH_ElecInputDef  owner=ih_elec_params  suffix=OUT */
typedef struct {
    IH_CycleDataDef cycle[20];         /* offset=0,  size=320 — nested array */
    uint8_t count;                     /* offset=320, size=1 */
} IH_ElecInputDef;                     /* sizeof=321, pragma_pack=4 → 324 */

/* @STRUCT IH_ElecResult    owner=ih_elec_params  suffix=OUT */
typedef struct {
    float I_peak_A;                    /* offset=0,  size=4 */
    float Vdc_mean;                    /* offset=4,  size=4 */
    float phi_deg;                     /* offset=8,  size=4 */
    float f_sw_Hz;                     /* offset=12, size=4 */
    float L_uH;                        /* offset=16, size=4 */
    float f_res_kHz;                   /* offset=20, size=4 */
    float Q_factor;                    /* offset=24, size=4 */
    float R_ohm;                       /* offset=28, size=4 */
    float I_rms;                       /* offset=32, size=4 */
    float P_W;                         /* offset=36, size=4 */
    float Z_mag_ohm;                   /* offset=40, size=4 */
    float X_ohm;                       /* offset=44, size=4 */
    float L_stable;                    /* offset=48, size=4 */
    float L_fast;                      /* offset=52, size=4 */
    uint8_t event;                     /* offset=56, size=1 */
    bool valid;                        /* offset=57, size=1 */
} IH_ElecResult;                       /* sizeof=58 */

/* === END INTERFACE STRUCTS === */


/* ============================================================
 *  public interface
 * ============================================================ */

/**
 * @brief  IH 电参数汇总计算 (20ms 周期)
 *
 * 本模块提供 STRONG 符号.
 * 调用方声明 __weak void IhElecParams_Calculate(void *in, void *out) {} 空壳,
 * 不 include 本文件.
 *
 * @param in   实际传 IH_ElecInputDef*, 模块内部强制转换
 * @param out  实际传 IH_ElecResult*, 模块内部强制转换
 */
void IhElecParams_Calculate(void *in, void *out);


#endif /* IH_ELEC_PARAMS_H */
