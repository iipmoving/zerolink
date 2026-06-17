/* mw.h — 中间层 API (过渡)
 *
 * MW 层封装多步操作，APP 通过这层调 DRV。
 * 函数声明是过渡手段，最终全走 Switcher。
 */
#ifndef MW_H
//#define MW_H

#include <stdint.h>

/* 关机原因 (供 mw_Shutdown 选用) */
enum {
    MW_OFF_NOPAN = 0x11,     /* 无锅关机 */
    MW_OFF_ZERO  = 0x12,     /* 零功率关机 */
    MW_OFF_SURGE = 0x01,     /* 浪涌关机 */
    MW_OFF_CHECK = 0x13,     /* 检锅中间关 */
};

/* === 关机 === */
void mw_Shutdown(uint8_t ch, uint8_t reason);

/* === 起振 === */
void mw_Startup(uint8_t ch);
void mw_StartupAll(void);

/* === 保护检测 === */
uint8_t mw_Protect_CheckBK(uint8_t ch);    /* 返回 0=正常, 非0=浪涌 */
uint8_t mw_Protect_CheckOvercurrent(uint8_t ch);
void mw_Protect_SetBK(uint8_t ch, uint8_t flag);
void mw_Protect_SetOvercurrent(uint8_t ch, uint8_t flag);
void mw_Protect_Init(uint8_t pot_num);

/* === 限流 === */
int16_t mw_CurrentLimit_Clip(uint8_t ch, int16_t delta_ppg);
void mw_CurrentLimit_SetMaxDelta(uint8_t ch, int16_t inc, int16_t dec);
void mw_CurrentLimit_Init(uint8_t pot_num);

/* === 系统状态 === */
void mw_SetState(uint8_t ch, uint8_t state);
uint8_t mw_GetState(uint8_t ch);

#endif /* MW_H */
