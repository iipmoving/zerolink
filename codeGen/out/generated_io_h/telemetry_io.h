/**
 * @file    telemetry_io.h
 * @brief   Telemetry Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   base_class
 *
 * Telemetry I/O (v2.3 LINK+PARAMS)
 *
 * 输入源:
 *   ElecParams → Telemetry  ()
 */

#ifndef TELEMETRY_IO_H
#define TELEMETRY_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

// ===== [AI GENERATED] 范式接入+骨架, 可被PY替换 =====

/* ========== OUTPUT (无) — 本模块没有输出 ========== */

/* Telemetry_Output — 无输出 */
typedef struct {
    uint8_t  res[4];
} MODULE_OUTPUT(Telemetry);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * ElecParams → Telemetry  输入参数  ()
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t  res[4];       /* TODO: 填写数据字段 */
} MODULE_INPUT_PARAMS(ElecParams, Telemetry);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  count;            /* 当前周期索引 */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(ElecParams, Telemetry) *params;
} MODULE_INPUT_LINK(ElecParams, Telemetry);

/* Telemetry_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(ElecParams, Telemetry) *ElecParams_params;  /* 指向 ElecParams 输出的 LINK 列 */
} MODULE_INPUT(Telemetry);
// ===== [END AI GENERATED] =====

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(Telemetry);

#endif /* TELEMETRY_IO_H */
