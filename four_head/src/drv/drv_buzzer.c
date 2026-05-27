/**
 * drv_buzzer.c —— 蜂鸣器驱动层实现
 *
 * 依赖: hal_buzzer.h + drv_buzzer.h
 * 层级: DRV —— 声音模式/音符序列/时序调度
 *
 * 端口自 Buzz_Drive.c + Buzz_Drive_MY.c (参考程序 2026.4.27)
 * 适配: APB0=48MHz, ARMCC V5, C89
 *
 * 普通模式: 固定~1kHz方波, 次数×时间+间隔循环
 *   参数: (count, on_time×10ms, interval×10ms)
 *   尾音: 到时后PE1切高阻输入, 电容RC放电衰减
 *
 * 美声模式: 可变频率音符序列
 *   每音符: [频率索引, 开启ms, 关闭ms]
 *   包络: PE4电源引脚控制音符开关
 */
#include "drv_buzzer.h"
#include "../hal/hal_buzzer.h"
#include <stddef.h>

/* ========== 音阶频率表（PWM中断频率 = 2×输出音频频率） ========== */
/* 这些PWM值与APB0时钟频率无关，因为TIM_Preload中的分子分母同时缩放 */
static const uint16_t s_pwm_freq_table[] = {
    /* 低音区 */
    1047u, 1109u, 1175u, 1245u, 1319u, 1397u, 1480u,   /* DO~UFA */
    1568u, 1661u, 1760u, 1865u, 1976u,                   /* SO~SI   */
    /* 中音区 */
    2093u, 2217u, 2349u, 2489u, 2637u, 2794u, 2960u,   /* MDO~UMFA */
    3136u, 3322u, 3520u, 3729u, 3951u,                   /* MSO~MSI */
    /* 自定义音色 */
    1000000u/(217u*2u),   /* NEW_SD_Yin  */
    1000000u/(185u*2u),   /* NEW_SD_Yin2 */
    1000000u/(170u*2u),   /* NEW_SD_Yin3 */
    1000000u/(278u*2u),   /* _527_SD1    */
    1000000u/(310u*2u),   /* _527_SD2    */
    1000000u/(292u*2u),   /* _527_SD3    */
    1000000u/(216u*2u),   /* _527_SD4    */
    1000000u/(169u*2u),   /* _527_ONOFF1 */
    1000000u/(183u*2u),   /* _527_ONOFF2 */
    1000000u/(215u*2u),   /* _527_ONOFF3 */
    1000000u/(243u*2u),   /* _527_ADDSUB1*/
    1000000u/(229u*2u),   /* _527_ADDSUB2*/
    1000000u/(215u*2u),   /* _527_ADDSUB3*/
    1000000u/(215u*2u),   /* _527_OFFWork1*/
    1000000u/(183u*2u),   /* _527_OFFWork2*/
    1000000u/(168u*2u),   /* _527_OFFWork3*/
    1000000u/(280u*2u),   /* _527_WEnd1  */
    1000000u/(246u*2u),   /* _527_WEnd2  */
    1000000u/(214u*2u),   /* _527_WEnd3  */
    1000000u/(180u*2u)    /* _527_WEnd4  */
};

/* 音阶索引 */
enum {
    TONE_DO = 0, TONE_UDO, TONE_RE, TONE_URE, TONE_MI, TONE_FA, TONE_UFA,
    TONE_SO, TONE_USO, TONE_LA, TONE_ULA, TONE_SI,
    TONE_MDO, TONE_MUDO, TONE_MRE, TONE_MURE, TONE_MMI, TONE_MFA, TONE_MUFA,
    TONE_MSO, TONE_MUSO, TONE_MLA, TONE_MULA, TONE_MSI,
    IDX_NEW_SD1, IDX_NEW_SD2, IDX_NEW_SD3,
    IDX_527_SD1, IDX_527_SD2, IDX_527_SD3, IDX_527_SD4,
    IDX_527_ONOFF1, IDX_527_ONOFF2, IDX_527_ONOFF3,
    IDX_527_ADDSUB1, IDX_527_ADDSUB2, IDX_527_ADDSUB3,
    IDX_527_OFFWORK1, IDX_527_OFFWORK2, IDX_527_OFFWORK3,
    IDX_527_WEND1, IDX_527_WEND2, IDX_527_WEND3, IDX_527_WEND4
};

/* ========== 美声/和弦音符序列定义 ========== */
/* 格式: [音阶索引, 开启时间ms, 关闭时间ms] */

static const uint16_t TONE_NEW_POWER_ON[3][3] = {
    { IDX_NEW_SD1,  20u,  90u },
    { IDX_NEW_SD2,  20u,  90u },
    { IDX_NEW_SD3,  60u, 800u }
};

static const uint16_t TONE_POWER_ON[3][3] = {
    { TONE_MUDO, 50u,  40u },
    { TONE_MSI,  50u,  50u },
    { TONE_MLA, 100u, 300u }
};

static const uint16_t TONE_KEY[3][3] = {
    { TONE_MUFA, 35u, 130u },
    { TONE_MSO,  35u, 130u },
    { TONE_MLA,  47u, 250u }
};

static const uint16_t TONE_ERR[3][3] = {
    { TONE_UFA, 30u, 120u },
    { TONE_SO,  30u, 120u },
    { TONE_LA, 470u, 300u }
};

static const uint16_t TONE_527_SD[4][3] = {
    { IDX_527_SD1, 60u, 400u },
    { IDX_527_SD2, 40u, 190u },
    { IDX_527_SD3, 40u, 190u },
    { IDX_527_SD4, 80u, 300u }
};

static const uint16_t TONE_527_ON[3][3] = {
    { IDX_527_ONOFF1, 35u,  67u },
    { IDX_527_ONOFF2, 35u,  67u },
    { IDX_527_ONOFF3, 47u, 300u }
};

static const uint16_t TONE_527_OFF[3][3] = {
    { IDX_527_ONOFF3, 20u,  88u },
    { IDX_527_ONOFF2, 20u,  88u },
    { IDX_527_ONOFF1, 60u, 300u }
};

static const uint16_t TONE_527_ADD[3][3] = {
    { IDX_527_ADDSUB1, 35u, 120u },
    { IDX_527_ADDSUB2, 35u, 120u },
    { IDX_527_ADDSUB3, 35u, 300u }
};

static const uint16_t TONE_527_SUB[3][3] = {
    { IDX_527_ADDSUB3, 35u, 120u },
    { IDX_527_ADDSUB2, 35u, 120u },
    { IDX_527_ADDSUB1, 35u, 300u }
};

static const uint16_t TONE_527_WORK[6][3] = {
    { IDX_527_ADDSUB3, 48u, 108u },
    { IDX_527_ADDSUB3, 48u, 408u },
    { IDX_527_ADDSUB3, 48u, 108u },
    { IDX_527_ADDSUB3, 48u, 408u },
    { IDX_527_ADDSUB3, 48u, 108u },
    { IDX_527_ADDSUB3, 48u, 408u }
};

static const uint16_t TONE_527_DING[1][3] = {
    { IDX_527_ADDSUB3, 35u, 300u }
};

static const uint16_t TONE_527_DONG[1][3] = {
    { IDX_527_ADDSUB1, 35u, 300u }
};

static const uint16_t TONE_527_WEND[4][3] = {
    { IDX_527_WEND1, 247u, 28u },
    { IDX_527_WEND2, 124u, 28u },
    { IDX_527_WEND3, 124u, 28u },
    { IDX_527_WEND4, 124u, 28u }
};

static const uint16_t TONE_START[4][3] = {
    { TONE_MSO, 100u, 200u },
    { TONE_MFA, 100u, 200u },
    { TONE_LA,  100u, 200u },
    { TONE_SI,  100u, 300u }
};

static const uint16_t TONE_SMALLSTAR[39][3] = {
    { TONE_MMI, 100u, 200u }, { TONE_MFA, 100u, 200u }, { TONE_MSO, 100u, 200u },
    { TONE_MSO, 100u, 300u }, { TONE_MDO, 100u, 200u }, { TONE_MDO, 100u, 200u },
    { TONE_MMI, 100u, 200u }, { TONE_MFA, 100u, 200u }, { TONE_MSO, 100u, 200u },
    { TONE_MSO, 100u, 300u }, { TONE_MFA, 100u, 200u }, { TONE_MFA, 100u, 200u },
    { TONE_MRE, 100u, 200u }, { TONE_MMI, 100u, 200u }, { TONE_MFA, 100u, 200u },
    { TONE_MFA, 100u, 300u }, { TONE_SI,  100u, 200u }, { TONE_SI,  100u, 200u },
    { TONE_MRE, 100u, 200u }, { TONE_MMI, 100u, 200u }, { TONE_MFA, 100u, 200u },
    { TONE_MMI, 100u, 600u }, { TONE_MMI, 100u, 200u }, { TONE_MFA, 100u, 200u },
    { TONE_MSO, 100u, 200u }, { TONE_MSO, 100u, 300u }, { TONE_MDO, 100u, 200u },
    { TONE_MDO, 100u, 200u }, { TONE_MMI, 100u, 200u }, { TONE_MFA, 100u, 200u },
    { TONE_MSO, 100u, 200u }, { TONE_MSO, 100u, 300u }, { TONE_MFA, 100u, 200u },
    { TONE_MLA, 100u, 200u }, { TONE_MFA, 100u, 200u }, { TONE_MMI, 100u, 200u },
    { TONE_MRE, 100u, 200u }, { TONE_MDO, 100u, 300u }, { TONE_MRE, 100u, 500u }
};

/* ========== DRV层私有状态（原HAL全局变量，纯DRV使用） ========== */
static uint16_t s_my_time_on;     /* 音符开启剩余(ms) */
static uint16_t s_my_time_off;    /* 音符关闭剩余(ms) */
static uint8_t  s_my_step;        /* 剩余音符步数 */
static uint8_t  s_my_step_init;   /* 初始步数(备份) */
static uint8_t  s_my_mode;        /* 当前美声模式编号 */
static uint8_t  s_count;          /* 鸣叫次数 */
static uint8_t  s_jiange;         /* 鸣叫间隔(×10ms) */
static uint8_t  s_hz_timer_hc;    /* 鸣叫时间备份 */
static uint8_t  s_jiange_hc;      /* 鸣叫间隔备份 */
static uint8_t  s_on_delay;       /* 连续调用消隐(ms) */
static uint8_t  s_off_flag;       /* 消隐窗口内不响应 */
static uint8_t  s_tick_10ms;      /* 10ms分频计数器 */

/* ========== 美声模式: 查表设置一个音符 ========== */
static void Buzz_MY_SetupNote(void)
{
    const uint16_t (*table)[3] = NULL;
    uint8_t  row;
    uint16_t pwm_val;
    uint8_t  row_count;

    row = s_my_step_init - s_my_step;

    switch (s_my_mode) {
    case DRV_BUZZ_MY_P_ON:      table = TONE_POWER_ON;     row_count = 3u; break;
    case DRV_BUZZ_MY_KEY:       table = TONE_KEY;          row_count = 3u; break;
    case DRV_BUZZ_MY_EER:       table = TONE_ERR;          row_count = 3u; break;
    case DRV_BUZZ_MY_NEW_P_ON:  table = TONE_NEW_POWER_ON; row_count = 3u; break;
    case DRV_BUZZ_MY_SD:        table = TONE_527_SD;       row_count = 4u; break;
    case DRV_BUZZ_MY_ON:        table = TONE_527_ON;       row_count = 3u; break;
    case DRV_BUZZ_MY_OFF:       table = TONE_527_OFF;      row_count = 3u; break;
    case DRV_BUZZ_MY_ADD:       table = TONE_527_ADD;      row_count = 3u; break;
    case DRV_BUZZ_MY_SUB:       table = TONE_527_SUB;      row_count = 3u; break;
    case DRV_BUZZ_MY_WORK:      table = TONE_527_WORK;     row_count = 6u; break;
    case DRV_BUZZ_MY_DING:      table = TONE_527_DING;     row_count = 1u; break;
    case DRV_BUZZ_MY_DONG:      table = TONE_527_DONG;     row_count = 1u; break;
    case DRV_BUZZ_MY_END:       table = TONE_527_WEND;     row_count = 4u; break;
    case DRV_BUZZ_MY_START:     table = TONE_START;        row_count = 4u; break;
    case DRV_BUZZ_MY_SMALLSTAR: table = TONE_SMALLSTAR;    row_count = 39u; break;
    default: return;
    }

    if (row >= row_count) return;

    pwm_val = s_pwm_freq_table[table[row][0]];
    s_my_time_on  = table[row][1];
    s_my_time_off = table[row][2];

    HAL_BUZZ_PWM_SET(pwm_val);
    HAL_Buzzer_SetActive(1u);
    HAL_BUZZ_PWR(1);
}

/* ========== 美声模式: 选择音效并启动 ========== */
static void Buzz_Mode_MY(uint8_t mode)
{
    /* 先停普通蜂鸣器 */
    s_count = 0u;
    HAL_Buzzer_SetTimer(0u);
    s_jiange = 0u;

    if (mode == 0u) {
        /* 关闭美声 */
        s_my_step_init = 0u;
        s_my_step      = 0u;
        s_my_time_on   = 0u;
        s_my_time_off  = 0u;
        HAL_Buzzer_SetActive(0u);
        HAL_BUZZ_PWM_DEFAULT();
        return;
    }

    s_my_mode = mode;

    switch (mode) {
    case DRV_BUZZ_MY_SMALLSTAR: s_my_step_init = 39u; break;
    case DRV_BUZZ_MY_START:
    case DRV_BUZZ_MY_SD:
    case DRV_BUZZ_MY_END:       s_my_step_init = 4u;  break;
    case DRV_BUZZ_MY_WORK:      s_my_step_init = 6u;  break;
    case DRV_BUZZ_MY_DING:
    case DRV_BUZZ_MY_DONG:      s_my_step_init = 1u;  break;
    default:                     s_my_step_init = 3u;  break;
    }

    s_my_step = s_my_step_init;
    Buzz_MY_SetupNote();
}

/* ========== 美声模式: 1ms时序处理 ========== */
static void Buzz_Dispose_MY(void)
{
    HAL_BUZZ_PWR_OUT;
    HAL_Buzzer_IntSync();

    if (s_my_time_on != 0u) {
        s_my_time_on--;
        HAL_BUZZ_PWR(1);
        return;
    }

    if (s_my_time_off != 0u) {
        s_my_time_off--;
        HAL_BUZZ_PWR(0);
        return;
    }

    if (s_my_step != 0u) {
        HAL_Buzzer_SetActive(1u);
    } else {
        HAL_Buzzer_SetActive(0u);
    }

    if (s_my_step != 0u) {
        s_my_step--;
        if (s_my_step != 0u) {
            Buzz_MY_SetupNote();
        }
    }
}

/* ========== 普通蜂鸣器: 设置参数 ========== */
static void Buzz_Dispose_Set(uint8_t count, uint8_t hz_timer, uint8_t jiange)
{
    s_count         = count;
    s_hz_timer_hc   = hz_timer;
    HAL_Buzzer_SetTimer(hz_timer);
    s_jiange_hc     = jiange;
    s_jiange        = jiange;
}

/* ========== 普通蜂鸣器: 模式选择 ========== */
static void Buzz_Mode(uint8_t mode)
{
    Buzz_Mode_MY(0);  /* 关闭美声 */

    switch (mode) {
    case DRV_BUZZ_KEY:        Buzz_Dispose_Set(1u,  20u,   0u); break;
    case DRV_BUZZ_SD:         Buzz_Dispose_Set(1u,  30u,  10u); break;
    case DRV_BUZZ_END_B5:     Buzz_Dispose_Set(5u,  20u, 100u); break;
    case DRV_BUZZ_END_B10:    Buzz_Dispose_Set(10u, 20u, 100u); break;
    case DRV_BUZZ_LONG1S_120: Buzz_Dispose_Set(120u,100u,250u); break;
    case DRV_BUZZ_DBUG:       Buzz_Dispose_Set(5u,   6u,  10u); break;
    case DRV_BUZZ_LONG05:     Buzz_Dispose_Set(1u,  50u,   0u); break;
    case DRV_BUZZ_06:         Buzz_Dispose_Set(6u,  20u, 100u); break;
    case DRV_BUZZ_06_TIPS:    Buzz_Dispose_Set(6u,  20u,  50u); break;
    case DRV_BUZZ_03_COOKTIPS:Buzz_Dispose_Set(3u,  20u,  50u); break;
    case DRV_BUZZ_ERR:        Buzz_Dispose_Set(10u, 20u, 100u); break;
    case DRV_BUZZ_LONG1S_5:   Buzz_Dispose_Set(5u, 100u, 250u); break;
    case DRV_BUZZ_TIPS:       Buzz_Dispose_Set(2u,  15u,  10u); break;
    case DRV_BUZZ_TIPS_1:     Buzz_Dispose_Set(1u,  20u,   0u); break;
    case DRV_BUZZ_PAN_ON:     Buzz_Dispose_Set(5u,  20u,  20u); break;
    default:
        s_count = 0u;
        HAL_Buzzer_SetTimer(0u);
        s_jiange = 0u;
        break;
    }
}

/* ========== 普通蜂鸣器: 10ms处理 ========== */
static void Buzz_Dispose(void)
{
    uint8_t t;

    if (s_count == 0u) return;

    t = HAL_Buzzer_GetTimer();
    if (t != 0u) {
        HAL_Buzzer_SetTimer(t - 1u);
    } else {
        if (s_jiange != 0u) {
            s_jiange--;
        } else {
            s_count--;
            if (s_count != 0u) {
                HAL_Buzzer_SetTimer(s_hz_timer_hc);
                s_jiange = s_jiange_hc;
            }
        }
    }
}

/* ========== 公共接口 ========== */

/* ========== __weak 接收: 由 app_hmi 直调 ========== */
void DrvBuzzer_OnCtrl(uint16_t param, void *data_ptr)
{
    (void)data_ptr;
    if (param != 0u) {
        Drv_Buzzer_Select(DRV_BUZZ_OUT_MY, DRV_BUZZ_MY_KEY);
    } else {
        Drv_Buzzer_Select(DRV_BUZZ_OUT_MY, DRV_BUZZ_MY_EER);
    }
}

void Drv_Buzzer_Init(void)
{
    s_tick_10ms    = 0u;
    s_my_time_on   = 0u;
    s_my_time_off  = 0u;
    s_my_step      = 0u;
    s_my_step_init = 0u;
    s_my_mode      = 0u;
    s_count        = 0u;
    s_jiange       = 0u;
    s_hz_timer_hc  = 0u;
    s_jiange_hc    = 0u;
    s_on_delay     = 0u;
    s_off_flag     = 0u;
}

void Drv_Buzzer_Select(uint8_t out_sel, uint8_t mode)
{
    /* 消隐: 150ms内连续调用不响应 */
    s_on_delay = BUZZ_BLANK_MS;
    if (s_off_flag != 0u) return;
    s_off_flag = 1u;

    if (out_sel == DRV_BUZZ_OUT_MY) {
        Buzz_Mode_MY(mode);
    } else if (out_sel == DRV_BUZZ_OUT_NORM) {
        Buzz_Mode(mode);
    }
}

void Drv_Buzzer_Timer_1ms(void)
{
    /* 消隐计时 */
    if (s_on_delay != 0u) {
        s_on_delay--;
    } else {
        s_off_flag = 0u;
    }

    /* 普通蜂鸣器: 每10ms处理一次 */
    s_tick_10ms++;
    if (s_tick_10ms >= BUZZ_TICK_DIV_10MS) {
        s_tick_10ms = 0u;
        Buzz_Dispose();
    }

    /* 美声蜂鸣器: 每1ms处理 */
    Buzz_Dispose_MY();
}
