/**
 * @file    ekf_lkf_io.h
 * @brief   EKF_LKF Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   base_class
 *
 * 扩展卡尔曼滤波观测器 — 从 ElecParams 读取观测值，估计 [R, L, f_res]
 *
 * 输入源:
 *   ElecParams → EKF_LKF  (ElecParams → EKF: 电参数观测值 (共享 ElecParams→AppPower 输出 LINK))
 */

#ifndef EKF_LKF_IO_H
#define EKF_LKF_IO_H

#include <stdint.h>
#include "../src/RX32G410_FW_HAL_V1.3N/Projects/core/std_module.h"

#pragma pack(4)

/* ========== OUTPUT (无) — 本模块没有输出 ========== */

/* EKF_LKF_Output — 无输出 */
typedef struct {
    uint8_t  res[4];
} MODULE_OUTPUT(EKF_LKF);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * ElecParams → EKF_LKF  输入参数  (ElecParams → EKF: 电参数观测值 (共享 ElecParams→AppPower 输出 LINK))
 * ------------------------------------------------------------------ */
typedef struct {
    int32_t I_peak_A;     /* 峰值电流 (0.01A) */
    int32_t Vdc_mean;     /* 母线电压均值 (0.01V) */
    int32_t phi_deg;     /* 相位角 (0.01°) */
    int32_t L_uH;     /* 等效电感 (0.01μH) */
    int32_t f_res_kHz;     /* 谐振频率 (0.01kHz) */
    int32_t R_ohm;     /* 等效电阻 (0.01Ω) */
    uint8_t valid;     /* 数据有效性 */
} MODULE_INPUT_PARAMS(ElecParams, EKF_LKF);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(ElecParams, EKF_LKF) *params;
} MODULE_INPUT_LINK(ElecParams, EKF_LKF);

/* EKF_LKF_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(ElecParams, EKF_LKF) *ElecParams_params;  /* 指向 ElecParams 输出的 LINK 列 */
} MODULE_INPUT(EKF_LKF);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(EKF_LKF);

#endif /* EKF_LKF_IO_H */
