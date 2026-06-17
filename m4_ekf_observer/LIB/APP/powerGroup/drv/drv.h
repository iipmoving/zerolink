/* drv.h — DRV 层公共接口 (过渡)
 *
 * 函数声明是过渡手段，最终全走 Switcher。
 * 各模块自声明 static struct，不 extern。
 */
#ifndef DRV_H
//#define DRV_H

#include <stdint.h>

/* ---- PWM ---- */
void drv_pwm_off(uint8_t ch, int8_t off_num);
void drv_pwm_on(uint8_t ch);

/* ---- 检锅脉冲 ---- */
void drv_StartPPG(uint8_t ch);
void drv_check_pot_pluse(uint8_t ch);
int8_t drv_check_pot_in(uint8_t ch);

/* ---- 脉冲计数 ---- */
void APP_POWER_PanCountInitChX(uint8_t ch);
void APP_POWER_PanCountInitCh1(void);
void APP_POWER_PanCountInitCh2(void);
void APP_POWER_PanCountInitCh3(void);
void APP_POWER_PanCountInitCh4(void);
uint8_t APP_POWER_PanCountGetValue(void);
void APP_POWER_PanCountSetValue(uint8_t onOff);

/* ---- FMAC ---- */
void APP_POWER_FmacSetPan(uint8_t ch);
void API_POWER_PanFmac(void);
void API_POWER_PanCheckPluse(void);
void APP_POWER_PanPluseMessage(uint8_t ch);

/* ---- 保护 ---- */
void APP_POWER_CompSetValue(void);
void getOvpValueAdj(void);
void APP_POWER_SetTxaAwdValue(void);
void APP_POWERR_SetTxaAwdValue(void);
uint8_t APP_ADC_getOverAdcChannel(uint16_t* value, uint16_t ovpValue, uint8_t num);
void getTxaDmaCircleValue(uint16_t* src, uint16_t* dst, uint16_t start, uint16_t size);

/* ---- ISR ---- */
void API_HRTIM1_TEST_CMP1_IRQHandlerCallback(void);
void API_ADC_Current1AWD_IRQHandlerCallBack(void);
void API_ADC_Current2AWD_IRQHandlerCallBack(void);
void APP_ADC_IRQ_PPGstepChangeCallBack(void);
void API_HRTIM_PanOffCallBack(void);
void API_DMA_PAN_IRQHandlerCallBack(uint8_t ch);
void API_FMAC_AppPowerOverCallBack(void* fmacMem);

#endif /* DRV_H */
