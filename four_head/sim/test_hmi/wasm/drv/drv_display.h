/* drv_display.h — WASM stub: 显示驱动空实现 */
#ifndef DRV_DISPLAY_H
#define DRV_DISPLAY_H

#include <stdint.h>

/* WASM: 不需要硬件显示, 但 app_hmi.c 引用了某些函数声明 */
void Drv_Display_Commit(const void *frame);
void Drv_Display_SetRawLEDs(uint8_t io8, uint8_t io9, uint8_t io10);
void Drv_Display_ShowRawSMG(const char *upper, const char *lower);

#endif
