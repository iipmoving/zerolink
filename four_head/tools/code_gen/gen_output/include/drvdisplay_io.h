/**
 * drvdisplay_io.h —— DrvDisplay 输入输出接口定义 (v2.3)
 *
 * 数据流:
 *   INPUT: AppHmi → DrvDisplay

 *
 * Include 权限:
 *   - 仅 drvdisplay.c 和 data_switcher.c 可 include
 *   - 使用全路径: #include "include/drvdisplay_io.h"
 *
 * 本文件包含 std_module.h，调用者无需再包含
 */
#ifndef DRVDISPLAY_IO_H
#define DRVDISPLAY_IO_H

#include "../core/std_module.h"
#include <stdint.h>

/* ========== OUTPUT (无) ========== */
/* 本模块没有输出 */


/* ========== INPUT (DrvDisplay 从别的模块得到的数据) ========== */

/* AppHmi_to_DrvDisplay_Input_Params — 从 AppHmi 得到的数据 */
typedef struct {
    // TODO: 添加输入字段（与 AppHmi OUTPUT 布局一致）
    uint8_t  res[4];
} MODULE_INPUT_PARAMS(AppHmi, DrvDisplay);


/* AppHmi_to_DrvDisplay_Input_Link — 输入管道 (与 AppHmi OUTPUT 配对) */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(AppHmi, DrvDisplay) *params;
} MODULE_INPUT_LINK(AppHmi, DrvDisplay);


/* DrvDisplay_Input — 输入聚合 (v2.3 对称命名: 成员名 = {Producer}_params) */
typedef struct {
    MODULE_INPUT_LINK(AppHmi, DrvDisplay) *AppHmi_params;  /* 从 AppHmi 得到 */

} MODULE_INPUT(DrvDisplay);


#endif /* DRVDISPLAY_IO_H */
