/**
 * drv_display.h —— 显示驱动层接口
 *
 * 依赖: SMG_Disp_General_Lib + hal_display.h
 * 层级: DRV —— 封装SMG库 + IO缓冲管理
 *
 * 显示架构:
 *   Disp_Buffer_Upper[4] → SMG库渲染 → IO[0..3] → 左数码管(炉头0+1)
 *   Disp_Buffer_Lower[4] → SMG库渲染 → IO[4..7] → 右数码管(炉头2+3)
 *   LED_Buffer[3]         → 直接映射 → IO[8..10] → LED组
 *
 * 扫描: 每1ms扫1个COM位, 11ms完成一轮
 * 刷新: 每10ms调用Update, 内部自计数100ms刷新+500ms闪烁
 */
#ifndef DRV_DISPLAY_H
//#define DRV_DISPLAY_H

#include <stdint.h>

/* ========== 可调参数 ========== */

#define DISPLAY_UPDATE_PERIOD_10MS  10u  /* 100ms刷新周期 = 10×10ms */
#define DISPLAY_BLINK_PHASE_MS     300u  /* 闪烁半周期 ms（与 JSON 配置一致）*/

void Drv_Display_Init(void);

/* 每1ms调用 —— COM硬件扫描一个位 */
void Drv_Display_Scan(void);

/* 每10ms调用(槽位) —— 自计时100ms刷新 + 500ms闪烁同步 */
void Drv_Display_Update(void);

/* 显示按键测试信息: key_code=键码, key_state=按键状态 */
void Drv_Display_ShowKey(uint8_t key_code, uint8_t key_state);

/* 直接设置 LED IO 字节 (绕过HMI缓存, 用于硬件验证) */
void Drv_Display_SetRawLEDs(uint8_t io8, uint8_t io9, uint8_t io10);

/* SMG直接更新 (绕过HMI消息, 不动LED, 用于测试模式) */
void Drv_Display_ShowRawSMG(const char *upper, const char *lower);

#endif /* DRV_DISPLAY_H */
