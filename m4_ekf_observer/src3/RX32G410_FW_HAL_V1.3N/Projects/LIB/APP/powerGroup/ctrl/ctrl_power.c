/* ctrl_power.c — 功率控制主循环
 * layer: CTRL (APP logic)
 *
 * 职责: 功率指令解析、PPG 增减计算、PPG 步进控制。
 *       自声明 per-channel 状态结构体，不 extern PowerMem。
 *
 * 迁出自: app_power_claude.c
 *    power_con_fun     (line 2271)  → ctrl_PowerCmd (内部)
 *    s_ppg_fun         (line 2701)  → ctrl_PpgCalc (内部)
 *    i_ppg_control     (line 3783)  → ctrl_PpgCtrl (内部)
 *    i_ppg_set_limit   (line 4010)  → 内部辅助
 *    reset_ppg_limit   (line 4127)  → 内部辅助
 *    s_power_convert_bass (line 2454) → 内部辅助
 *    getMemSetValue    (line 1292)  → 内部辅助 (空壳)
 *    get_MAXMIN_PPG    (line 2444)  → 内部辅助 (空壳)
 *
 * 对外接口: ctrl_PowerRun(), ctrl_SetInputs(), ctrl_Get*()
 */
#include "ctrl.h"
#include "API_HRTIM.h"
#include "S_PID.h"
#include "s_data_stack.h"
#include "s_time_base.h"
#include "data_type.h"
#include "app_power.h"
#include <stdlib.h>

#define CTRL_POWER_CH_MAX  4
#define MinPowerM           500/25
#define PPG_MAX_RANGE       200
#define C_KEEP_TIME         5

#define F_POWER_FAST_UP     1
#define F_POWER_FAST_DOWN   2
#define F_POWER_OVER        3
#define F_SURGE_OVER        4
#define F_POT_ERR           5

#define B_POW_STB_FLAG      _BIT6
#define B_POW_ARRIVE_FLAG   _BIT5
#define B_PAN_ADJ_FLAG      _BIT4

#define B_PPGMAX  _BIT0
#define B_VCOUT   _BIT1
#define B_LOWV    _BIT2
#define B_PWDEAD  _BIT3

/* ===== 自声明 per-channel 状态结构体 =====
 * 包含 power_con_fun / s_ppg_fun / i_ppg_control 全部所需字段
 * 函数体原样迁入，#define 短名 → 本结构体字段 */
struct ctrl_power_ctx {
    /* --- keepReg --- */
    uint8_t              channel;
    uint8_t              max_power_set;
    uint16_t             p25_ad;
    FixedPIDController   pid;
    uint8_t              panc_time;

    /* --- staticReg --- */
    uint16_t             adc_trig;
    uint16_t             power_adc_fact;
    uint8_t              half_adj;
    uint8_t              half_cnt;
    uint8_t              resume_flag;
    uint16_t             power_min_fre;
    uint16_t             power_cycle;
    uint8_t              surge_delay;
    uint8_t              off_flag;
    uint8_t              power_limit_flag;
    uint8_t              power_dead_cnt;
    uint16_t             duty_actual;
    uint16_t             power_duty;
    uint8_t              vcout_flag;
    uint8_t              vcout_delay;
    uint16_t             ppg_limit;
    uint16_t             ppg_limit_max;
    uint16_t             ppg_limit_power;
    uint8_t              limit_max_count;
    uint8_t              limit_voltage;
    uint8_t              limit_Qvalue;
    uint8_t              ppg_lock_flag;
    uint16_t             bottom_value;
    uint16_t             top_value;

    /* --- flag bits (staticReg) --- */
    uint8_t              ppg_on;
    uint8_t              ic_vc_adc_ok_flag;
    uint8_t              ppg_add_flag;

    /* --- input->status (由 ctrl_SetInputs 填入) --- */
    uint8_t              ih_status;
    uint8_t              target_power;
    uint8_t              actual_power;
    uint8_t              current_value;
    uint8_t              voltage_value;
    uint8_t              actual_ppg;
    uint8_t              power_status;
    uint8_t              max_power_m;

    /* --- input->control (由外部控制写入) --- */
    uint8_t              power_setm;
    uint8_t              power_switch;

    /* --- 外部函数同步缓存 --- */
    uint8_t              hold_max;
    uint8_t              hold_min;

    /* --- PidReturn 临时缓存 (per-channel, 兼容原 PidReturn[4] 下标) --- */
    int                  pid_return[CTRL_POWER_CH_MAX];
};

static struct ctrl_power_ctx s_ctx[CTRL_POWER_CH_MAX];
static struct ctrl_power_ctx *s_p;

/* ===== 公开接口: 必须在 #define 映射前 (避免宏展开冲突) ===== */

void ctrl_GetState(uint8_t ch, CtrlPowerState_t *state)
{
    if(ch >= CTRL_POWER_CH_MAX || !state) return;
    const struct ctrl_power_ctx *p = &s_ctx[ch];
    state->power_duty       = p->power_duty;
    state->duty_actual      = p->duty_actual;
    state->power_adc_fact   = p->power_adc_fact;
    state->actual_power     = p->actual_power;
    state->actual_ppg       = p->actual_ppg;
    state->power_status     = p->power_status;
    state->ih_status        = p->ih_status;
    state->target_power     = p->target_power;
    state->off_flag         = p->off_flag;
    state->power_limit_flag = p->power_limit_flag;
    state->power_dead_cnt   = p->power_dead_cnt;
    state->ppg_limit        = p->ppg_limit;
    state->ppg_limit_max    = p->ppg_limit_max;
    state->ppg_limit_power  = p->ppg_limit_power;
    state->limit_max_count  = p->limit_max_count;
    state->limit_voltage    = p->limit_voltage;
    state->limit_Qvalue     = p->limit_Qvalue;
    state->ppg_lock_flag    = p->ppg_lock_flag;
    state->vcout_flag       = p->vcout_flag;
    state->vcout_delay      = p->vcout_delay;
    state->ppg_add_flag     = p->ppg_add_flag;
    state->power_half_adj   = p->half_adj;
    state->half_cnt         = p->half_cnt;
    state->resume_flag      = p->resume_flag;
    state->ic_vc_adc_ok_flag = p->ic_vc_adc_ok_flag;
    state->bottom_value     = p->bottom_value;
    state->top_value        = p->top_value;
    state->power_min_fre    = p->power_min_fre;
    state->pid_return       = p->pid_return[ch];
}

/* ===================================================================
 *  #define 映射: 短名 → 本地结构体字段
 *  函数体原样迁入，不改逻辑。只需改这里的映射。
 * =================================================================== */

/* keepReg */
#define Power_channel       s_p->channel
#define MaxPowerSet         s_p->max_power_set
#define g_p25_ad            s_p->p25_ad
#define PowerPid            s_p->pid
#define g_panc_time         s_p->panc_time

/* staticReg */
#define g_power_adc_trig    s_p->adc_trig
#define g_power_adc_fact    s_p->power_adc_fact
#define power_half_adj      s_p->half_adj
#define PowerHalfCnt        s_p->half_cnt
#define m_power_resume_flag s_p->resume_flag
#define PowerMinFre         s_p->power_min_fre
#define PowerCycle          s_p->power_cycle
#define g_surge_delay       s_p->surge_delay
#define g_off_flag          s_p->off_flag
#define g_power_limit_flag  s_p->power_limit_flag
#define PowerDeadCnt        s_p->power_dead_cnt
#define g_duty_actual       s_p->duty_actual
#define g_power_duty        s_p->power_duty
#define m_vcout_flag        s_p->vcout_flag
#define vcout_delay         s_p->vcout_delay
#define s_ppg_limit         s_p->ppg_limit
#define s_ppg_limit_max     s_p->ppg_limit_max
#define s_ppg_limit_power   s_p->ppg_limit_power
#define s_limit_max_count   s_p->limit_max_count
#define s_limit_voltage     s_p->limit_voltage
#define s_limit_Qvalue      s_p->limit_Qvalue
#define m_ppg_lock_flag     s_p->ppg_lock_flag
#define BOTTOMValue         s_p->bottom_value
#define TOPValue            s_p->top_value

/* flag bits */
#define m_ppg_on            s_p->ppg_on
#define m_ic_vc_adc_ok_flag s_p->ic_vc_adc_ok_flag
#define m_ppg_add_flag      s_p->ppg_add_flag
#define m_power_hold_max    s_p->hold_max
#define m_power_hold_min    s_p->hold_min

/* input->status (当前控制周期的快照) */
#define IHStatus            s_p->ih_status
#define TargetPower         s_p->target_power
#define ActualPower         s_p->actual_power
#define CurrentValue        s_p->current_value
#define VoltageValue        s_p->voltage_value
#define ActualPPG           s_p->actual_ppg
#define PowerStatus         s_p->power_status
#define MaxPowerM           s_p->max_power_m

/* input->control */
#define power_setm          s_p->power_setm
#define power_switch        s_p->power_switch

/* PidReturn (局部缓存，外部通过 ctrl_GetPidReturn 读取) */
#define PidReturn           (s_p->pid_return)

/* PowerStack 通过外部函数访问 */
extern void ctrl_StackDelet(uint8_t ch);
extern void ctrl_StackPush(uint8_t ch);


/* ===== 外部函数 (留在 app_power_claude.c) ===== */
extern void APP_POWER_PhaseHalfTypeSet(void);
extern void APP_POWER_PpgHalfTypeSet(int t_ppgchange);


/* ===================================================================
 *  内部辅助函数 (原样迁入)
 * =================================================================== */

static INT16U s_power_convert_bass(INT8U triger_powerm)
{
    INT16U t_convert_p;
    if(triger_powerm==0) {
        triger_powerm=MaxPowerM;
    }
    t_convert_p = g_p25_ad;
    t_convert_p *= triger_powerm;
    return t_convert_p;
}

static void getMemSetValue(void) { }
static void get_MAXMIN_PPG(void) { }

static void reset_ppg_limit(void)
{
    s_ppg_limit_power=TargetPower;
    s_limit_max_count=0;
    PowerMinFre=MIN_FRE_PWM;
    s_ppg_limit=PowerMinFre;
    s_ppg_limit_max=PowerMinFre;
    s_limit_voltage=VoltageValue;
    s_limit_Qvalue=0;
    g_power_limit_flag=0;
    PowerDeadCnt=0;
}

static void i_ppg_set_limit(INT16U t_corrent_ppg)
{
    if((g_power_limit_flag & B_PWDEAD)) {
        if(TargetPower==s_ppg_limit_power&&power_half_adj==0) {
            if(g_surge_delay>C_KEEP_TIME+POT_TYPE_DELAY2) {
                s_limit_max_count++;
                if(s_limit_max_count>10) {
                    s_limit_max_count=0;
                    s_ppg_limit_max=t_corrent_ppg+PPG_MAX_RANGE;
                    if(s_ppg_limit_max<FRE_30K_PWM)
                        s_ppg_limit_max=FRE_30K_PWM;
                }
                if((t_corrent_ppg<s_ppg_limit)) {
                    IHStatus|=B_POW_ARRIVE_FLAG;
                    s_ppg_limit=t_corrent_ppg+(PPG_MAX_RANGE/5);
                    if(s_ppg_limit<FRE_40K_PWM)
                        s_ppg_limit=FRE_40K_PWM;
                    uint16_t cycle = PowerCycle/4+100;
                    if(s_ppg_limit<cycle)
                        s_ppg_limit=cycle;
                    m_ppg_lock_flag=1;
                    s_limit_voltage=VoltageValue;
                }
            } else {
                s_limit_max_count=0;
            }
        }
    }
    if(s_ppg_limit>s_ppg_limit_max)
        s_ppg_limit=s_ppg_limit_max;
}


/* ===================================================================
 *  ctrl_PowerCmd — 功率指令解析
 *  原: power_con_fun (app_power_claude.c:2271)
 * =================================================================== */
static INT16U ctrl_PowerCmd(uint8_t ch)
{
    INT8U	temp;
    INT8U	t_power_switch,t_power_setm;
    uint8_t switchTemp;

    getMemSetValue();
    get_MAXMIN_PPG();

    if(Power_channel==1) { }

    if(power_setm>MaxPowerSet)
        power_setm=MaxPowerSet;

    t_power_setm=power_setm;
    t_power_switch=power_switch;

    if(power_setm) { }

    switchTemp=(t_power_switch&0x0f);

    if(t_power_setm) {
        g_power_adc_trig = s_power_convert_bass(t_power_setm);
        if(t_power_switch&0xf0)
            switchTemp=0x2;
    } else {
        g_power_adc_trig = 0;
        if(t_power_switch==0)
            IHStatus &= (~B_PAN_ADJ_FLAG);
    }

    temp=t_power_setm;

    if(TargetPower!=temp) {
        IHStatus &= (~B_POW_STB_FLAG);
        power_half_adj=0;
        FixedPIDclearIntegral(&PowerPid);
        if(TargetPower<temp&&temp>1500/25) {
            power_half_adj=TargetPower+(temp-TargetPower)/2;
            PowerHalfCnt=0;
        }
        m_power_resume_flag=1;
        TargetPower=temp;
        reset_ppg_limit();

        if(TargetPower==0)
            ctrl_StackDelet(Power_channel);
        else
            ctrl_StackPush(Power_channel);
    }

    if(power_half_adj)
        g_power_adc_trig = s_power_convert_bass(power_half_adj);

    g_power_adc_trig&=0xfff0;
    g_power_adc_trig +=  switchTemp;

    return g_power_adc_trig;
}


/* ===================================================================
 *  ctrl_PpgCalc — PPG 增减值计算
 *  原: s_ppg_fun (app_power_claude.c:2701)
 * =================================================================== */
static INT8U ctrl_PpgCalc(uint8_t ch)
{
    static uint8_t  s_power_dalta;
    INT16U          t_power_adc_trig;
    INT16U          t_tem_power_new;
    INT16U          t_tem_power_dalta;
    INT8U           t_tem_ppg_add=0;
    INT8U           t_ic_value,t_vc_value;
    int             t_pidReturn;

    if(m_ic_vc_adc_ok_flag==0)
        return 0xff;

    t_power_adc_trig = g_power_adc_trig;

#if 1
    t_ic_value=CurrentValue;
    t_vc_value=VoltageValue;
    t_tem_power_new = t_ic_value;
    t_tem_power_new *= t_vc_value;
#else
    t_tem_power_new=powerAdcFactTxa;
#endif

    t_tem_power_dalta=t_tem_power_new/g_p25_ad;
    s_power_dalta=0;

    if(t_tem_power_dalta>ActualPower) {
        if(ActualPower>MinPowerM)
            s_power_dalta=t_tem_power_dalta-ActualPower;
    }

    ActualPower=t_tem_power_dalta;
    g_power_adc_fact = t_tem_power_new;

    if(!m_ppg_on)
        return 0xff;

    if(ActualPower>MaxPowerM+8) {
        g_off_flag=F_POWER_OVER;
        return 0x96;
    } else {
        t_pidReturn=FixedPID_Compute(&PowerPid, t_power_adc_trig, g_power_adc_fact, g_p25_ad);
        *PidReturn = t_pidReturn;
        t_tem_ppg_add=abs(t_pidReturn);

        APP_POWER_PhaseHalfTypeSet();

        uint16_t t_powerDead=abs(TargetPower-ActualPower);

        if(t_powerDead<=1&&t_tem_ppg_add<1) {
            if(power_half_adj==0) {
                PowerDeadCnt++;
                if(PowerDeadCnt>5) {
                    PowerDeadCnt=5;
                    g_power_limit_flag |= B_PWDEAD;
                }
            }
        } else {
            if(PowerDeadCnt>=5)
                PowerDeadCnt=3;
            else
                PowerDeadCnt=0;
            g_power_limit_flag &= (~B_PWDEAD);
            APP_POWER_PpgHalfTypeSet(t_pidReturn);
        }

        if(t_pidReturn<2) {
            if(power_half_adj) {
                PowerHalfCnt++;
                if(PowerHalfCnt>50) {
                    PowerHalfCnt=0;
                    power_half_adj=0;
                    g_surge_delay=APP_POWER_PAN_DELAY-10;
                }
            }
        }

        if(t_pidReturn<0) {
            t_tem_ppg_add|=0x80;
            if(m_power_hold_min)
                t_tem_ppg_add=0;
        } else {
            if(m_power_hold_max)
                t_tem_ppg_add=0;
        }

        return t_tem_ppg_add;
    }
}


/* ===================================================================
 *  ctrl_PpgCtrl — PPG 步进控制
 *  原: i_ppg_control (app_power_claude.c:3783)
 * =================================================================== */
static INT16U ctrl_PpgCtrl(uint8_t ch, INT8U t_pan_cur_change)
{
    INT16U t_corrent_ppg;
    INT8U  t_ppg_dict;
    INT8U  t_ppg_change_value;
    INT16U t_ppg_limit;

    t_corrent_ppg=g_duty_actual;

    if(g_surge_delay<POT_TYPE_DELAY2+2) {
        m_vcout_flag=0;
        vcout_delay=0;
        return g_power_duty;
    }

    if(m_vcout_flag) {
        vcout_delay=50;
        m_vcout_flag=0;
    }

    if(vcout_delay) {
        if(Time_GetMs100Flg())
            vcout_delay--;
    }

    if(t_pan_cur_change!=0xff) {
        g_power_limit_flag&=~B_PPGMAX;

        if(t_pan_cur_change) {
            t_ppg_dict=t_pan_cur_change&0x80;
            t_ppg_change_value=t_pan_cur_change&0x7f;

            if(t_ppg_dict) {
                m_ppg_add_flag = 0;
                if(t_corrent_ppg>t_ppg_change_value)
                    t_corrent_ppg -= t_ppg_change_value;
            } else {
                if(vcout_delay) {
                    t_ppg_change_value=1;
                    if(vcout_delay>40)
                        t_ppg_change_value=0;
                }
                BOTTOMValue=t_ppg_change_value;
                t_corrent_ppg += t_ppg_change_value;
                m_ppg_add_flag = 1;
            }
        }

        i_ppg_set_limit(t_corrent_ppg);

#if 1
        t_ppg_limit=s_ppg_limit;
        if(t_corrent_ppg >= t_ppg_limit) {
            if(vcout_delay==0) {
                if(s_ppg_limit<s_ppg_limit_max)
                    s_ppg_limit++;
            }
            t_corrent_ppg=t_ppg_limit;
            g_power_limit_flag|=B_PPGMAX;
            IHStatus|=B_POW_ARRIVE_FLAG;
        } else {
            g_power_limit_flag&=~B_PPGMAX;
        }
#endif
    }

    PowerStatus = ((g_off_flag&0x0f)<<4) + (g_power_limit_flag&0x0f);

    if(m_ppg_on) {
        if(t_corrent_ppg < MAX_FRE_PWM)
            t_corrent_ppg = MAX_FRE_PWM;
    }
    if(t_corrent_ppg > PowerMinFre)
        t_corrent_ppg = PowerMinFre;

    g_power_duty=t_corrent_ppg;

    TOPValue=s_ppg_limit>>8;
    ActualPPG= g_power_duty>>8;

    return t_corrent_ppg;
}


/* ===================================================================
 *  对外接口
 * =================================================================== */


void ctrl_SetInputs(uint8_t ch, uint8_t current_ad, uint8_t voltage_ad)
{
    if(ch >= CTRL_POWER_CH_MAX) return;
    s_ctx[ch].current_value = current_ad;
    s_ctx[ch].voltage_value = voltage_ad;
}

void ctrl_SyncHoldFlags(uint8_t ch, uint8_t hold_max, uint8_t hold_min)
{
    if(ch >= CTRL_POWER_CH_MAX) return;
    s_ctx[ch].hold_max = hold_max;
    s_ctx[ch].hold_min = hold_min;
}

uint16_t ctrl_PowerRun(uint8_t ch)
{
    if(ch >= CTRL_POWER_CH_MAX) return 0;
    s_p = &s_ctx[ch];
    s_p->channel = ch;                  /* init per-channel index */

    /* Step 1: 功率指令解析 */
    ctrl_PowerCmd(ch);

    /* Step 2: PPG 增减计算 */
    uint16_t return_value = ctrl_PpgCalc(ch);

    /* Step 3: PPG 步进控制 */
    return_value = ctrl_PpgCtrl(ch, (INT8U)return_value);

    /* 清 ADC 采集标志 */
    m_ic_vc_adc_ok_flag = 0;

    return s_p->power_duty;
}

uint8_t ctrl_GetActualPower(uint8_t ch)
{
    if(ch >= CTRL_POWER_CH_MAX) return 0;
    return s_ctx[ch].actual_power;
}

uint8_t ctrl_GetActualPPG(uint8_t ch)
{
    if(ch >= CTRL_POWER_CH_MAX) return 0;
    return s_ctx[ch].actual_ppg;
}

uint8_t ctrl_GetPowerStatus(uint8_t ch)
{
    if(ch >= CTRL_POWER_CH_MAX) return 0;
    return s_ctx[ch].power_status;
}

uint8_t ctrl_GetIHStatus(uint8_t ch)
{
    if(ch >= CTRL_POWER_CH_MAX) return 0;
    return s_ctx[ch].ih_status;
}

int ctrl_GetPidReturn(uint8_t ch)
{
    if(ch >= CTRL_POWER_CH_MAX) return 0;
    return s_ctx[ch].pid_return[ch];
}

uint16_t ctrl_GetPowerDuty(uint8_t ch)
{
    if(ch >= CTRL_POWER_CH_MAX) return 0;
    return s_ctx[ch].power_duty;
}
