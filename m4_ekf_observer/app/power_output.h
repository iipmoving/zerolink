/**
 * @file    power_output.h
 * @brief   功率输出打包 — 收集各包结果 → 填充 hw_cmd
 * @layer   app
 *
 * 在 power_pid / power_protect / power_pan_if 全部执行后调用。
 * 收集各炉头 delta / 保护标志 / 检锅请求 → PowerHw_Command_t。
 */
#ifndef POWER_OUTPUT_H
//#define POWER_OUTPUT_H

#include <stdint.h>
#include "app_power_io.h"

/**
 * @brief  打包 hw_cmd
 * @param  out  APP 输出（含 head[] + hw_cmd）
 */
void PowerOutput_Pack(PowerBase_Output_t *out);

#endif /* POWER_OUTPUT_H */
