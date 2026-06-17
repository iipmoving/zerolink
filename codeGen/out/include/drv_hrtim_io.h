/**
 * @file    drv_hrtim_io.h
 * @brief   DrvHrtim Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   base_class
 *
 * HRTIM 驱动 — 功率命令下发到硬件，反馈硬件状态
 */

#ifndef DRVHRTIM_IO_H
#define DRVHRTIM_IO_H

#include <stdint.h>
#include "../src/RX32G410_FW_HAL_V1.3N/Projects/core/std_module.h"

#pragma pack(4)

/* ========== OUTPUT (无) — 本模块没有输出 ========== */

/* DrvHrtim_Output — 无输出 */
typedef struct {
    uint8_t  res[4];
} MODULE_OUTPUT(DrvHrtim);

/* ========== INPUT (无) — 本模块没有输入 ========== */

/* DrvHrtim_Input — 无输入 */
typedef struct {
    uint8_t  res[4];
} MODULE_INPUT(DrvHrtim);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(DrvHrtim);

#endif /* DRVHRTIM_IO_H */
