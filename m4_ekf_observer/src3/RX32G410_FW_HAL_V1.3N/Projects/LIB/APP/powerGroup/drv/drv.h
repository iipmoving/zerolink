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
void PPGsetHalf(uint8_t ch, uint16_t pwm);
void PPGsetDutyChX(uint8_t ch, uint16_t duty);
void PPGsetDutyCh1(uint16_t duty);
void PPGsetDutyCh2(uint16_t duty);
void PPGsetDutyCh3(uint16_t duty);
void PPGsetDutyCh4(uint16_t duty);
void PPGonOffCh1(uint8_t flag);
void PPGonOffCh2(uint8_t flag);
void PPGonOffCh3(uint8_t flag);
void PPGonOffCh4(uint8_t flag);
void PPGdeadTimeCh1(uint8_t upDts, uint8_t downDts);
void PPGdeadTimeCh2(uint8_t upDts, uint8_t downDts);
void PPGdeadTimeCh3(uint8_t upDts, uint8_t downDts);
void PPGdeadTimeCh4(uint8_t upDts, uint8_t downDts);

/* ---- PPG value wrappers ---- */
typedef struct { uint16_t duty; uint16_t prioed; } PPGvalueDef;
void PPGsetValueCh1(PPGvalueDef input);
void PPGsetValueCh2(PPGvalueDef input);
void PPGsetValueCh3(PPGvalueDef input);
void PPGsetValueCh4(PPGvalueDef input);
PPGvalueDef PPGgetValueCh1(void);
PPGvalueDef PPGgetValueCh2(void);
PPGvalueDef PPGgetValueCh3(void);
PPGvalueDef PPGgetValueCh4(void);

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
