/**
 * hal_smg.c —— SMG段码显示库HAL实现
 *
 * 依赖: hal_smg.h + SMG_Disp_General_Lib.h
 * 层级: HAL —— 所有第三方库include在此隔离
 */
#include "hal_smg.h"
#include "../lib/SMG_Disp_General_Lib.h"

static SMG_Disp_General_STATE_ s_smg_states[2];  /* [0]=上, [1]=下 */

uint8_t HAL_SMG_Init(void)
{
    return Disp_General_Init(s_smg_states, 2u);
}

void HAL_SMG_FlashSync(uint8_t flash)
{
    SMG_Disp_General_Lib_Flash_STA(flash);
}

void HAL_SMG_HexHL(uint8_t *buf, uint8_t data_h, uint8_t data_l)
{
    Disp_Hex_H_L(buf, data_h, data_l);
}

void HAL_SMG_FontASCII(uint8_t *buf, const char *font)
{
    Dsp_Font_ASCII(buf, (char*)font);
}
