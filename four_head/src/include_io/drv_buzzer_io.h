/**
 * @file    drv_buzzer_io.h
 * @brief   DrvBuzzer Data Switcher IO interface (v2.3 LINK+PARAMS)
 * @layer   drv
 *
 * 蜂鸣器驱动 — 从 Hmi 接收蜂鸣命令 (@OUTPUT_CALLBACK 即时路由)
 *
 * 输入源:
 *   AppHmi → DrvBuzzer  (@OUTPUT_CALLBACK 蜂鸣命令 → DrvBuzzer (即时路由, 不走 Switcher 周期))
 */

#ifndef DRVBUZZER_IO_H
#define DRVBUZZER_IO_H

#include <stdint.h>
#include "std_module.h"

#pragma pack(4)

/* ========== OUTPUT (无) — 本模块没有输出 ========== */

/* DrvBuzzer_Output — 无输出 */
typedef struct {
    uint8_t  res[4];
} MODULE_OUTPUT(DrvBuzzer);

/* ================================================================
 * INPUT — 本模块输入的数据管道
 * ================================================================ */

/* ------------------------------------------------------------------
 * AppHmi → DrvBuzzer  输入参数  (@OUTPUT_CALLBACK 蜂鸣命令 → DrvBuzzer (即时路由, 不走 Switcher 周期))
 * ------------------------------------------------------------------ */
typedef struct {
    uint8_t sound_type;     /* 蜂鸣类型: 1=短按, 2=长按, 3=成功, 4=失败, 5=报警 */
} MODULE_INPUT_PARAMS(AppHmi, DrvBuzzer);

typedef struct {
    uint8_t  status;           /* ST_NEW / ST_OUT */
    uint8_t  max_count;        /* 最大数量 */
    uint8_t  seq;              /* 有效性索引号 — 生产者 seq++, 消费者比对 last_seq */
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppHmi, DrvBuzzer) *params;
} MODULE_INPUT_LINK(AppHmi, DrvBuzzer);

/* DrvBuzzer_Input — 输入聚合 (对称命名: 成员 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppHmi, DrvBuzzer) *AppHmi_params;  /* 指向 AppHmi 输出的 LINK 列 */
} MODULE_INPUT(DrvBuzzer);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(DrvBuzzer);

#endif /* DRVBUZZER_IO_H */
