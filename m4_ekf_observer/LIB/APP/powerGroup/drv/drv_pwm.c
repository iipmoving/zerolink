/* drv_pwm.c — PWM 通断/死区/初始化
 * layer: DRV (base class)
 *
 * 职责: 管理 PWM 通断、死区时间、初始化。
 *       内部维护 per-channel 状态，不依赖 PowerMem，不 extern。
 *
 * 迁出自: app_power_claude.c (LIB/APP/)
 *    s_pwm_off    (line 4086)
 *    s_pwm_on     (line 4118)
 *    PPGonOffCh1~4  (line 5882)
 *    PPGdeadTimeCh1~4 (line 5904)
 *    PPGinit       (line 5923)
 *    PPGsetHalf    (line 6025)
 *    API_HRTIM_PanOffCallBack (line 3649)
 */

#include "data_type.h"
#include "API_HRTIM.h"      /* OFF_FRE_PWM, START_FRE_PWM, PAN_FRE_PWM */
#include "app_power_hw_param.h"
#include "app_power_constants.h"  /* PotCh1~4, POTNUM */

/* === 外部函数声明 (过渡: 仍在 app_power_claude.c) === */
void APP_PPG_Action(uint8_t ch, uint16_t duty, uint16_t cycle, uint8_t action);
INT8U getMaxPowerDiv(void);

void APP_ADC_PanSwChange(uint32_t ch);
void APP_ADC_DMA_RecoverPan(uint8_t ch);
void APP_POWER_PanCountSetValue(uint8_t onOff);

/* === 自声明 struct (同名同类型, 不依赖 PowerMem) === */
#define OFF_FRE_PWM_VAL  0     /* 同 API_HRTIM.h OFF_FRE_PWM */
#define START_FRE_PWM_VAL 1250 /* FRE_40K_PWM 典型值 */

static struct {
    /* —— 通断状态 —— */
    uint8_t  ppg_on;             /* was m_ppg_on */
    uint8_t  power_on;           /* 功率输出允许 */

    /* —— 频率/占空比 —— */
    uint16_t duty;               /* was g_power_duty (PowerDuty) */
    uint16_t duty_actual;        /* was g_duty_actual (PPGdutyActual) */

    /* —— 功率标志 —— */
    uint8_t  power_pause_flag;   /* was m_power_pause_flag */
    uint8_t  power_off_flag;     /* was m_power_off_flag */
    uint8_t  dis_voltage_flag;   /* was m_dis_voltage_flag */

    /* —— 保护/延时 —— */
    uint16_t surge_delay;        /* was g_surge_delay */
    uint8_t  vcout_delay;        /* was vcout_delay */

    /* —— 功率保存/限制 —— */
    uint16_t ppg_save;           /* was PowerPpgSave */
    uint16_t max_power_set;      /* was MaxPowerSet */

    /* —— 当前偏置 —— */
    uint8_t  current_offset;     /* was CURRENT_OFFSET */

} s_pwm[POTNUM];


/* ===================================================================
 *  s_pwm_off — 关闭 PWM 输出
 *  原: s_pwm_off(INT8U off_num)  无显式通道
 *  新: drv_pwm_off(ch, off_num)  通道显式化
 * =================================================================== */
void drv_pwm_off(uint8_t ch, INT8U off_num)
{
    if (ch >= POTNUM) return;

    if (off_num < 0x10 && off_num > 0) {
        s_pwm[ch].current_offset = off_num;
    }

    if (s_pwm[ch].power_pause_flag) {
        s_pwm[ch].ppg_save = s_pwm[ch].duty;
    } else {
        s_pwm[ch].ppg_save = 0;
    }

    if (s_pwm[ch].ppg_on) {
        /* 清状态: 等效原 MemSetInt(PowerControl->staticReg, 0, ...) */
        s_pwm[ch].ppg_on = 0;
        s_pwm[ch].duty_actual = 0;
        s_pwm[ch].duty = 0;
        s_pwm[ch].surge_delay = 0;
        s_pwm[ch].vcout_delay = 0;
    }
    s_pwm[ch].ppg_on = 0;
    s_pwm[ch].duty_actual = OFF_FRE_PWM_VAL;
    s_pwm[ch].power_off_flag = 1;
    s_pwm[ch].max_power_set = getMaxPowerDiv();

    /* HRTIM_STUB: 写 HRTIM 寄存器停止输出 */
}


/* ===================================================================
 *  s_pwm_on — 开启 PWM 输出
 *  原: s_pwm_on(void)  无显式通道
 *  新: drv_pwm_on(ch)  通道显式化
 * =================================================================== */
void drv_pwm_on(uint8_t ch)
{
    if (ch >= POTNUM) return;

    if (s_pwm[ch].ppg_on == 0) {
        if (s_pwm[ch].ppg_save) {
            s_pwm[ch].duty = s_pwm[ch].ppg_save;
        } else {
            s_pwm[ch].duty = START_FRE_PWM_VAL;  /* 软启动 */
        }
        s_pwm[ch].ppg_on = 1;
        s_pwm[ch].dis_voltage_flag = 1;
        s_pwm[ch].max_power_set = getMaxPowerDiv();
        s_pwm[ch].surge_delay = 0;
        s_pwm[ch].vcout_delay = 0;
    }
    /* HRTIM_STUB: 写 HRTIM 寄存器开启输出 */
}


/* ===================================================================
 *  API_HRTIM_PanOffCallBack — 移锅 HRTIM 回调
 *  转发到 drv_pan_count: 复位检锅脉冲计数 + 恢复 DMA
 * =================================================================== */
void API_HRTIM_PanOffCallBack(void)
{
    APP_POWER_PanCountSetValue(1);
}


/* ===================================================================
 *  PPGonOffCh1~4 — PPG 开关 (空壳)
 *  原样迁移, 当前为 HRTIM_STUB
 * =================================================================== */
void PPGonOffCh1(uint8_t flag) { /* HRTIM_STUB */ (void)flag; }
void PPGonOffCh2(uint8_t flag) { /* HRTIM_STUB */ (void)flag; }
void PPGonOffCh3(uint8_t flag) { /* HRTIM_STUB */ (void)flag; }
void PPGonOffCh4(uint8_t flag) { /* HRTIM_STUB */ (void)flag; }


/* ===================================================================
 *  PPGdeadTimeCh1~4 — 死区时间设置 (空壳)
 *  原样迁移, 当前为 HRTIM_STUB
 * =================================================================== */
void PPGdeadTimeCh1(uint8_t upDts, uint8_t downDts) { /* HRTIM_STUB */ (void)upDts; (void)downDts; }
void PPGdeadTimeCh2(uint8_t upDts, uint8_t downDts) { /* HRTIM_STUB */ (void)upDts; (void)downDts; }
void PPGdeadTimeCh3(uint8_t upDts, uint8_t downDts) { /* HRTIM_STUB */ (void)upDts; (void)downDts; }
void PPGdeadTimeCh4(uint8_t upDts, uint8_t downDts) { /* HRTIM_STUB */ (void)upDts; (void)downDts; }


/* ===================================================================
 *  PPGinit — PPG 初始化 (空壳)
 *  原样迁移, 当前为空函数
 * =================================================================== */
void PPGinit(void) { /* HRTIM_STUB */ }


/* ===================================================================
 *  PPGsetHalf — 输出 50% 占空比 PWM
 *  原样迁移, 调用 APP_PPG_Action
 * =================================================================== */
void PPGsetHalf(uint8_t ch, uint16_t pwm)
{
    APP_PPG_Action(ch, pwm, pwm * 2, 0);
}
