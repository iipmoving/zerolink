/**
 * hal_display.h —— 显示硬件抽象层
 *
 * 依赖: sc32f1xxx_gpio.h
 * 层级: HAL —— 纯GPIO操作，不include任何业务头文件
 *
 * 硬件: 8 SEG + 11 COM 共阴数码管直推
 * 扫描: 每COM依次输出段码→选通→延时→关断
 */
#ifndef HAL_DISPLAY_H
#define HAL_DISPLAY_H

#include <stdint.h>

#define DISP_COM_COUNT  11u
#define DISP_SM8_COUNT   8u   /* 8个数码管位(IO[0..7]) */

void HAL_Display_Init(void);

/* 扫描一个COM位: com_idx 0..10, seg_data=段码值, prev_com=上一COM(0xFF=首次) */
void HAL_Display_Scan(uint8_t com_idx, uint8_t seg_data, uint8_t prev_com);

#endif /* HAL_DISPLAY_H */
