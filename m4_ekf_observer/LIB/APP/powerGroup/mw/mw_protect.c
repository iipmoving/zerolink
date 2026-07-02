/* mw_protect.c — 保护检测封装
 * layer: MW (中间层)
 *
 * 职责: 封装保护检测接口，APP 通过这层读取故障状态。
 *       目前为直通占位，待 DRV 保护模块完善后接入实际逻辑。
 */
#include "mw.h"
#include "../drv/drv.h"


uint8_t mw_Protect_CheckBK(uint8_t ch)
{
    /* TODO: 接入 drv_protect 实际 BK 检测 */
    (void)ch;
    return 0;  /* 0 = 正常, 非0 = 浪涌 */
}


uint8_t mw_Protect_CheckOvercurrent(uint8_t ch)
{
    /* TODO: 接入实际过流检测 */
    (void)ch;
    return 0;  /* 0 = 正常 */
}
