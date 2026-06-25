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
#include "API_HRTIM.h"      /* OFF_FRE_PWM, START_FRE_PWM, PAN_FRE_PWM, PotCh1~4, PotNum */

/* API_HRTIM.h enum 中 PotNum = 4 */
#ifndef POTNUM
#define POTNUM  PotNum
#endif

/* === 外部函数声明 (过渡: 仍在 app_power_drv.c, 暂用 __weak 占位) === */
__attribute__((weak)) void APP_PPG_Action(uint8_t ch, uint16_t duty, uint16_t cycle, uint8_t action)
{ (void)ch; (void)duty; (void)cycle; (void)action; }

INT8U getMaxPowerDiv(void);

void APP_ADC_PanSwChange(uint32_t ch);
void APP_ADC_DMA_RecoverPan(uint8_t ch);
void APP_POWER_PanCountSetValue(uint8_t onOff);

/* === 自声明 struct (同名同类型, 不依赖 PowerMem) === */
#define OFF_FRE_PWM_VAL  0     /* 同 API_HRTIM.h OFF_FRE_PWM */
#define START_FRE_PWM_VAL 1250 /* FRE_40K_PWM 典型值 */

static struct pwm_ctx {
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
    struct pwm_ctx *p = &s_pwm[ch];

    if (off_num < 0x10 && off_num > 0) {
        p->current_offset = off_num;
    }

    if (p->power_pause_flag) {
        p->ppg_save = p->duty;
    } else {
        p->ppg_save = 0;
    }

    if (p->ppg_on) {
        /* 清状态: 等效原 MemSetInt(PowerControl->staticReg, 0, ...) */
        p->ppg_on = 0;
        p->duty_actual = 0;
        p->duty = 0;
        p->surge_delay = 0;
        p->vcout_delay = 0;
    }
    p->ppg_on = 0;
    p->duty_actual = OFF_FRE_PWM_VAL;
    p->power_off_flag = 1;
    p->max_power_set = getMaxPowerDiv();

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
    struct pwm_ctx *p = &s_pwm[ch];

    if (p->ppg_on == 0) {
        if (p->ppg_save) {
            p->duty = p->ppg_save;
        } else {
            p->duty = START_FRE_PWM_VAL;  /* 软启动 */
        }
        p->ppg_on = 1;
        p->dis_voltage_flag = 1;
        p->max_power_set = getMaxPowerDiv();
        p->surge_delay = 0;
        p->vcout_delay = 0;
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
void PPGonOffCh1(uint8_t flag) { API_PPG_OnOff(PotCh1, flag); }
void PPGonOffCh2(uint8_t flag) { API_PPG_OnOff(PotCh2, flag); }
void PPGonOffCh3(uint8_t flag) { API_PPG_OnOff(PotCh3, flag); }
void PPGonOffCh4(uint8_t flag) { API_PPG_OnOff(PotCh4, flag); }


/* ===================================================================
 *  PPGdeadTimeCh1~4 — 死区时间设置
 *  迁出自 app_power_claude.c:6542
 * =================================================================== */
void PPGdeadTimeCh1(uint8_t upDts, uint8_t downDts) { API_PPG_DeadTime(PotCh1, upDts, downDts); }
void PPGdeadTimeCh2(uint8_t upDts, uint8_t downDts) { API_PPG_DeadTime(PotCh2, upDts, downDts); }
void PPGdeadTimeCh3(uint8_t upDts, uint8_t downDts) { API_PPG_DeadTime(PotCh3, upDts, downDts); }
void PPGdeadTimeCh4(uint8_t upDts, uint8_t downDts) { API_PPG_DeadTime(PotCh4, upDts, downDts); }


/* ===================================================================
 *  PPGinit — PPG 初始化
 *  迁出自 app_power_claude.c:6570
 * =================================================================== */
void PPGinit(void) { /* HRTIM_STUB */ }


/* ===================================================================
 *  PPGsetHalf — 输出 50% 占空比 PWM
 *  迁出自 app_power_claude.c:6577
 * =================================================================== */
void PPGsetHalf(uint8_t ch, uint16_t pwm)
{
    PPGvalueDef input;
    input.duty    = pwm;
    input.prioed  = pwm * 2;
    API_PPG_setValue(ch, input);
}


/* ===================================================================
 *  PPGsetDutyChX/1~4 — 设置占空比 (wrapper)
 *  迁出自 app_power_claude.c:5445
 * =================================================================== */
void PPGsetDutyChX(uint8_t ch, uint16_t duty)
{
    API_PPG_setPluse(ch, duty);
}
void PPGsetDutyCh1(uint16_t duty) { PPGsetDutyChX(PotCh1, duty); }
void PPGsetDutyCh2(uint16_t duty) { PPGsetDutyChX(PotCh2, duty); }
void PPGsetDutyCh3(uint16_t duty) { PPGsetDutyChX(PotCh3, duty); }
void PPGsetDutyCh4(uint16_t duty) { PPGsetDutyChX(PotCh4, duty); }


/* ===================================================================
 *  PPGsetValueCh1~4 — 设置周期+占空比 (wrapper)
 *  迁出自 app_power_claude.c:5471
 * =================================================================== */
void PPGsetValueCh1(PPGvalueDef input) { API_PPG_setValue(PotCh1, input); }
void PPGsetValueCh2(PPGvalueDef input) { API_PPG_setValue(PotCh2, input); }
void PPGsetValueCh3(PPGvalueDef input) { API_PPG_setValue(PotCh3, input); }
void PPGsetValueCh4(PPGvalueDef input) { API_PPG_setValue(PotCh4, input); }


/* ===================================================================
 *  PPGgetValueCh1~4 — 读取当前 PPG (wrapper)
 *  迁出自 app_power_claude.c:5490
 * =================================================================== */
PPGvalueDef PPGgetValueCh1(void) { return API_PPG_getValue(PotCh1); }
PPGvalueDef PPGgetValueCh2(void) { return API_PPG_getValue(PotCh2); }
PPGvalueDef PPGgetValueCh3(void) { return API_PPG_getValue(PotCh3); }
PPGvalueDef PPGgetValueCh4(void) { return API_PPG_getValue(PotCh4); }
