/**
 * @file    elec_params.h
 * @brief   ElecParams 模块公共接口
 * @layer   base_class
 *
 * 电参数计算 — 20ms 周期计算 I_peak/L/f_res/Q/R/P 等，输出到 AppPower 和 EKF_LKF
 */

#ifndef ELECPARAMS_H
#define ELECPARAMS_H

#include <stdint.h>

/* ---- v2.3 统一接口声明 ---- */
MODULE_IO_H(ElecParams);

#endif /* guard */
