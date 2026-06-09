/**
 * @file    power_protect.c
 * @brief   Power 保护功能包 — 浪涌/OVP/限流/过温判断
 * @layer   app
 *
 * 从 app_power_claude.c surge_Processing / getOvpValueAdj 迁出。
 * 纯业务判断，不访问 HRTIM 硬件。
 * 硬件状态通过 in->hw_status.bk_flag / fault 输入。
 */
#include "power_protect.h"
#include "app_power_hw_io.h"

uint8_t PowerProtect_Run(PowerBase_Input_t *in, uint8_t ch)
{
    if (ch >= POWER_POTMAX) return 0;

    uint8_t triggered = 0;

    /* ---- 浪涌检测 (硬件 BK 标志) ---- */
    if (in->hw_status.bk_flag[ch]) {
        triggered |= 0x01;   /* bit0 = surge */
    }

    /* ---- 硬件故障检测 ---- */
    if (in->hw_status.fault[ch]) {
        triggered |= 0x02;   /* bit1 = fault */
    }

    /* ---- 全局浪涌 ---- */
    if (in->hw_status.surge_flag) {
        triggered |= 0x01;
    }

    /* TODO: 迁入 getOvpValueAdj — 设置 ADC AWD 阀值
     * 使用 in->pAdc[ch]. 读取电压/电流做 OVP 判断 */
    /* TODO: 迁入 igbt_derate — IGBT 高温降功率 */
    /* TODO: 迁入 top_temp_stop — 炉面温度超限停机 */

    return triggered;
}
