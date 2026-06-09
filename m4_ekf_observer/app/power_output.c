/**
 * @file    power_output.c
 * @brief   功率输出打包 — 收集各包结果 → 填充 hw_cmd
 * @layer   app
 *
 * 在 power_pid / power_protect / power_pan_if 全部执行后调用。
 * 收集各炉头 delta / 保护标志 / 检锅请求 → PowerHw_Command_t。
 */
#include "power_output.h"
#include "app_power_hw_io.h"
#include <string.h>

void PowerOutput_Pack(PowerBase_Output_t *out)
{
    PowerHw_Command_t *cmd = &out->hw_cmd;

    /* 每帧重置 */
    memset(cmd, 0, sizeof(*cmd));

    for (uint8_t i = 0; i < POWER_POTMAX; i++) {
        /* PID 结果 */
        cmd->ppg_delta[i]   = out->head[i].ppg_delta;
        cmd->delta_valid[i] = out->head[i].delta_valid;

        /* 功率开关 (来自 power_state) */
        cmd->power_on[i]    = (out->head[i].power_state != 0) ? 1 : 0;
    }

    /* TODO: 收集保护标志 → cmd 对应位 */
    /* TODO: 收集检锅请求 → cmd->pan_request / pan_request_ch */
    /* TODO: 收集同步请求 → cmd->sync_request */
}
