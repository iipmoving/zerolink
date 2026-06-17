/**
 * @file    telemetry_io.h
 * @brief   Telemetry I/O (v2.3 LINK+PARAMS)
 * @layer   base_class
 *
 * 数据流遥测 — 从 ElecParams → Telemetry 输出管道捕获
 *   Calculator 输入副本 (20周期) + ElecParams 输出 (float)
 *
 * 输入: ElecParams (通过 ElecParams → Telemetry 输出管道)
 * 输出: 无 (纯捕获，MODBUS Area 5 映射缓冲区)
 */

#ifndef TELEMETRY_IO_H
#define TELEMETRY_IO_H

#include <stdint.h>
#include "std_module.h"

#define TELEMETRY_POTMAX  4

/* ==========================================================================
 * 输入 — 从 ElecParams 接收的数据 (自包含)
 * ========================================================================== */

/* 输入管道: 与 MODULE_OUTPUT_LINK(ElecParams, Telemetry) 配对 */
typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(ElecParams, Telemetry) params[TELEMETRY_POTMAX];
} MODULE_INPUT_LINK(ElecParams, Telemetry);

/* 输入聚合: InputCallback 直穿赋值 {ElecParams}_params 指针 */
typedef struct {
    MODULE_INPUT_LINK(ElecParams, Telemetry) *ElecParams_params;
} MODULE_INPUT(Telemetry);

/* ==========================================================================
 * 输出 — 本模块没有输出
 * ========================================================================== */
typedef struct {
    uint8_t  res[4];
} MODULE_OUTPUT(Telemetry);

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(Telemetry);

#endif /* TELEMETRY_IO_H */
