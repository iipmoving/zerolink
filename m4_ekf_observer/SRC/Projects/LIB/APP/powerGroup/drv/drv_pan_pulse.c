/* drv_pan_pulse.c — 检锅脉冲/起振
 * layer: DRV (base class)
 *
 * 职责: 检锅脉冲序列产生、起振控制。
 *       内部维护 per-channel 状态，不依赖 PowerMem，不 extern。
 *
 * 注意: 当前为占位骨架。完整实现在 app_power_claude.c 以旧名活跃。
 *       待 Switcher 就绪后填入完整函数体，移除 app_power_claude.c 副本。
 *
 * 迁出自: app_power_claude.c
 *    StartPPG        (line 3620)
 *    check_pot_pluse (line 3655)
 *    check_pot_in    (line 3759)
 */

#include "data_type.h"
#include "API_HRTIM.h"


/* ===================================================================
 *  drv_StartPPG — 起振脉冲 (future)
 *  TODO: 填入起振脉冲序列，使用 s_pulse[ch] 替代 PowerMem 宏
 * =================================================================== */
void drv_StartPPG(uint8_t ch)
{
    (void)ch;
    /* HRTIM_STUB: 待 Switcher 就绪后接入 */
}


/* ===================================================================
 *  drv_check_pot_pluse — 检锅脉冲产生 (future)
 *  TODO: 填入检锅状态机，使用 s_pulse[ch] 替代 PowerMem 宏
 * =================================================================== */
void drv_check_pot_pluse(uint8_t ch)
{
    (void)ch;
    /* HRTIM_STUB: 待 Switcher 就绪后接入 */
}


/* ===================================================================
 *  drv_check_pot_in — 读取检锅脉冲数，判锅 (future)
 *  TODO: 填入判锅逻辑，使用 s_pulse[ch] 替代 PowerMem 宏
 * =================================================================== */
int8_t drv_check_pot_in(uint8_t ch)
{
    (void)ch;
    return 0;  /* C_POT_MAY */
}
