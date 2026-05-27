/**
 * hal_smg.h —— SMG段码显示库HAL包装
 *
 * 依赖: SMG_Disp_General_Lib (预编译.lib)
 * 层级: HAL —— 封装第三方库，DRV不直接include库头文件
 *
 * 功能: 段码转换、显示特效(闪烁/跑马/翻滚)、数字/字符串/16进制渲染
 */
#ifndef HAL_SMG_H
#define HAL_SMG_H

#include <stdint.h>

/* ========== 公共接口 ========== */

/* 初始化SMG库(内部管理2组: 上/下显示) */
uint8_t HAL_SMG_Init(void);

/* 闪烁同步(每500ms调用, flash=0/1交替) */
void HAL_SMG_FlashSync(uint8_t flash);

/* 16进制显示: 两个byte分别显示 (如 0x55,0x12 → "55 12") */
void HAL_SMG_HexHL(uint8_t *buf, uint8_t data_h, uint8_t data_l);

/* 字符串显示: 仅支持0-9 A-Z(不分大小写) 空格 - */
void HAL_SMG_FontASCII(uint8_t *buf, const char *font);

#endif /* HAL_SMG_H */
