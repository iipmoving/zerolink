/**
 * wasm_bridge.c — WASM 测试接线层 + HAL 桩
 *
 * 编译时与 src/app/app_hmi.c 直接链接，WASM_BUILD 宏启用 getter 导出。
 * 替代旧 modules/main.c + modules/*.c 独立副本模式。
 */
#include "app/app_hmi.h"
#include "cfg/hmi_data.h"
#include <stddef.h>
#include <string.h>

/* app_hmi.c 中定义的强符号 */
void App_Hmi_Init(void);
void AppHmi_OnKey(uint16_t param, void *data_ptr);
void AppHmi_OnTimer100ms(uint16_t param, void *data_ptr);
void AppHmi_OnTimer1s(uint16_t param, void *data_ptr);
void App_Hmi_Run(void);

/* ===== HAL 桩 (WASM 环境无硬件) ===== */
void HAL_GPIO_SetPin(uint8_t pin)            { (void)pin; }
void HAL_GPIO_ResetPin(uint8_t pin)          { (void)pin; }
uint8_t HAL_GPIO_ReadPin(uint8_t pin)        { (void)pin; return 0; }
void HAL_Display_Scan(void)                  {}
void HAL_Display_Init(void)                  {}
void HAL_SMG_Init(void)                      {}
void HAL_SMG_FlashSync(uint8_t t)            { (void)t; }
void HAL_SMG_FontASCII(unsigned char *d, const char *s) { (void)d; (void)s; }
void HAL_SMG_HexHL(unsigned char *d, uint8_t h, uint8_t l) { (void)d; (void)h; (void)l; }
void DrvBuzzer_OnCtrl(uint16_t p, void *d)   { (void)p; (void)d; }
void DrvDisplay_OnRefresh(uint16_t p, void *d) { (void)p; (void)d; }
void Drv_Display_SetRawLEDs(uint8_t a,uint8_t b,uint8_t c) { (void)a;(void)b;(void)c; }
void Drv_Display_ShowRawSMG(const char *u,const char *l) { (void)u;(void)l; }
void Drv_Display_Commit(const void *f)       { (void)f; }
void HAL_Key_Scan(void)                      {}
void HAL_Timer_Init(void)                    {}
void HAL_UART_Debug_Init(void)               {}
void HAL_UART_Debug_Print(const char *s)     { (void)s; }
void HAL_UART_Debug_HexDump(const void *d,uint8_t l) { (void)d;(void)l; }
void HAL_UART_Debug_Flush(void)              {}
uint32_t HAL_Timer_GetUS(void)               { return 0; }
uint8_t HAL_Timer_1msElapsed(void)           { return 0; }
void __aeabi_memcpy(void *d, const void *s, unsigned n) { memcpy(d,s,n); }

/* ===== 按键工具函数 (旧 key_module 导出) ===== */
static const uint8_t s_head_keys[] = {15,14,13,12};  /* HMI_KEY_LEFT_P_SET_UP..RIGHT_P_SET */
int8_t Key_HeadKeyToIndex(uint8_t key) {
    for (int i=0;i<4;i++) if (s_head_keys[i]==key) return i; return -1;
}
uint8_t Key_HeadIndexToKey(uint8_t idx) {
    return (idx<4) ? s_head_keys[idx] : 0;
}
uint8_t Key_DigitKeyToLevel(uint8_t key) {
    return (key>=22&&key<=31) ? (key-22) : 0;
}
uint8_t Key_IsHeadKey(uint8_t key) {
    return Key_HeadKeyToIndex(key)>=0 ? 1 : 0;
}

/* ===== WASM 导出: 引擎接口 ===== */
void engine_init(void)
{
    App_Hmi_Init();
    AppHmi_OnTimer100ms(0, NULL);  /* 触发上电序列 */
    /* 跑完上电序列需要 6s = 60 个 100ms tick, 由测试循环完成 */
}

void engine_post_key(uint8_t key, uint8_t evt)
{
    uint16_t param = (uint16_t)key | ((uint16_t)evt << 8);
    AppHmi_OnKey(param, NULL);
    AppHmi_OnTimer100ms(0, NULL);  /* 触发 100ms 处理 */
}

void engine_tick_100ms(void)
{
    AppHmi_OnTimer100ms(0, NULL);
    extern void App_Hmi_Run(void);
    App_Hmi_Run();
}

void engine_tick_1s(void)
{
    AppHmi_OnTimer1s(0, NULL);
    extern void App_Hmi_Run(void);
    App_Hmi_Run();
}

/* app_hmi.c WASM_BUILD 导出的强制超时函数 */
void wasm_force_select_confirm(uint8_t idx);
void wasm_force_boost_exit(uint8_t idx);
void wasm_force_timer_expire(uint8_t idx);

void engine_force_select_confirm(uint8_t idx)  { wasm_force_select_confirm(idx); }
void engine_force_boost_exit(uint8_t idx)      { wasm_force_boost_exit(idx); }
void engine_force_timer_expire(uint8_t idx)    { wasm_force_timer_expire(idx); }

/* ===== WASM 导出: 状态查询 ===== */
uint8_t engine_get_global_mode(void)      { extern uint8_t wasm_get_global_mode(void); return wasm_get_global_mode(); }
uint8_t engine_is_child_lock(void)        { extern uint8_t wasm_is_child_lock(void); return wasm_is_child_lock(); }
uint8_t engine_is_paused(void)            { extern uint8_t wasm_is_paused(void); return wasm_is_paused(); }
int8_t  engine_get_hot_head(void)         { extern int8_t wasm_get_hot_head(void); return wasm_get_hot_head(); }
uint8_t engine_get_stack_depth(void)      { extern uint8_t wasm_get_stack_depth(void); return wasm_get_stack_depth(); }
int8_t  engine_get_stack_at(uint8_t p)    { extern int8_t wasm_get_stack_at(uint8_t); return wasm_get_stack_at(p); }
uint8_t engine_get_zone_node(uint8_t i)   { extern uint8_t wasm_get_zone_node(uint8_t); return wasm_get_zone_node(i); }
uint8_t engine_get_zone_power(uint8_t i)  { extern uint8_t wasm_get_zone_power(uint8_t); return wasm_get_zone_power(i); }
uint8_t engine_get_zone_boost(uint8_t i)  { extern uint8_t wasm_get_zone_boost(uint8_t); return wasm_get_zone_boost(i); }
uint8_t engine_get_zone_timer_setting(uint8_t i) { extern uint8_t wasm_get_zone_timer_setting(uint8_t); return wasm_get_zone_timer_setting(i); }
uint8_t engine_get_zone_timer_active(uint8_t i)  { extern uint8_t wasm_get_zone_timer_active(uint8_t); return wasm_get_zone_timer_active(i); }
uint8_t engine_get_zone_timer_value(uint8_t i)   { extern uint8_t wasm_get_zone_timer_value(uint8_t); return wasm_get_zone_timer_value(i); }
char    engine_get_seg_char(uint8_t i)    { extern char wasm_get_seg_char(uint8_t); return wasm_get_seg_char(i); }
uint8_t engine_get_seg_blink(uint8_t i)   { extern uint8_t wasm_get_seg_blink(uint8_t); return wasm_get_seg_blink(i); }
uint8_t engine_get_seg_mode(void)         { extern uint8_t wasm_get_seg_mode(void); return wasm_get_seg_mode(); }
uint8_t engine_get_led_power(void)        { extern uint8_t wasm_get_led_power(void); return wasm_get_led_power(); }
uint8_t engine_get_led_timer(void)        { extern uint8_t wasm_get_led_timer(void); return wasm_get_led_timer(); }
uint8_t engine_get_led_pause(void)        { extern uint8_t wasm_get_led_pause(void); return wasm_get_led_pause(); }
uint8_t engine_get_led_child_lock(void)   { extern uint8_t wasm_get_led_child_lock(void); return wasm_get_led_child_lock(); }
uint8_t engine_get_led_head_select(uint8_t i) { extern uint8_t wasm_get_led_head_select(uint8_t); return wasm_get_led_head_select(i); }
uint8_t engine_get_led_power_level(uint8_t i) { extern uint8_t wasm_get_led_power_level(uint8_t); return wasm_get_led_power_level(i); }
