/**
 * @file    app_power_hw_param.h
 * @brief   APP_POWER 层硬件参数常量 (从 API_HRTIM.h 提取)
 * @layer   app (local)
 *
 * 本文件包含 APP_POWER 算法所需的频率常量、通道枚举和类型定义。
 * 这些原本来自 API_HRTIM.h，提取后使 APP_POWER 不再依赖 HRTIM API。
 *
 * 设计: 只包含 APP 层算法需要的内容，不包含硬件寄存器操作声明。
 *       DRV 层不需要此文件。
 *
 * Date    : 2026-06-09
 */

#ifndef APP_POWER_HW_PARAM_H
#define APP_POWER_HW_PARAM_H

#include <stdint.h>

/* ==================================================================
 * 频率计算基值
 * ================================================================== */
/* SysPWM_Frequency = 192MHz, PWM_DIV = fPWM_div4
 * PWM_SF = 192MHz * 4 / 2 = 384MHz */
#define PWM_SF              384000000UL

/* ==================================================================
 * PWM 频率常量 (PWM_SF / FREQ_Hz)
 * ================================================================== */
#define START_FRE_PWM       (PWM_SF / 40000)        /* 软启动 40kHz */
#define MIN_FRE_PWM         (PWM_SF / 29500)        /* 钢锅上限 29.5kHz */
#define MAX_FRE_PWM         (PWM_SF / 60000)        /* 最大频率 60kHz */
#define MID_FRE_PWM         (PWM_SF / 22500)        /* 铁锅上限 22.5kHz */
#define LARGE_FRE_PWM       (PWM_SF / 21500)        /* 大功率上限 21.5kHz */
#define PAN_FRE_PWM         (PWM_SF / 60000)        /* 检锅频率 60kHz */
#define POTTYPE2_FRE_PWM    (PWM_SF / 38000)        /* 第二检锅频率 38kHz */

#define FRE_CYCLE_PWM       MIN_FRE_PWM             /* 频率周期值 */
#define OFF_FRE_PWM         0                       /* 关闭 */
#define MAX_FRE_PERIOD      (MAX_FRE_PWM * 2)       /* 最大周期 */

/* 各频率阈值 */
#define FRE_27K_PWM         (PWM_SF / 27000)
#define FRE_30K_PWM         (PWM_SF / 30000)
#define FRE_40K_PWM         (PWM_SF / 40000)
#define FRE_50K_PWM         (PWM_SF / 50000)
#define FRE_55K_PWM         (PWM_SF / 55000)

/* 500ns/1us 精度常量 */
#define FRE_500K_PWM        (PWM_SF / 500000)       /* 768 */
#define FRE_1000K_PWM       (PWM_SF / 1000000)      /* 384 */
#define FRE_2000K_PWM       (PWM_SF / 2000000)      /* 192 */

/* ==================================================================
 * 死区时间常量 (DTG 寄存器值)
 * ================================================================== */
#define DTS1                48
#define DTS1US              DTS1
#define DTS2US              (DTS1 * 2)
#define DTS3US              (DTS1 * 3)
#define DTS4US              (DTS1 * 4)
#define DTS6US              0xFF
#define DTSMAX              0XFF

/* ==================================================================
 * 移锅负荷频率 (用于锅具判断)
 * ================================================================== */
#define LOAD_FRE_IRON       25000               /* 铁锅移锅频率 */
#define LOAD_FRE_STEEL      30000               /* 钢锅移锅频率 */
#define IRON_LOAD_FRE_PWM   (PWM_SF / LOAD_FRE_IRON)
#define STEEL_LOAD_FRE_PWM  (PWM_SF / LOAD_FRE_STEEL)

/* ==================================================================
 * ADC 采样相关
 * ================================================================== */
#define FRE_PER_ADC         FRE_1000K_PWM            /* 0.5us = 384 ticks */

/* ==================================================================
 * 通道枚举
 * ================================================================== */
enum {
    PotCh1      = 0,        /* 炉头 1 */
    PotCh2      = 1,        /* 炉头 2 */
    PotCh3      = 2,        /* 炉头 3 */
    PotCh4      = 3,        /* 炉头 4 */
    PotChTest1  = 4,        /* 测试通道 */
    PotChBase   = 5,        /* 基准通道 */
    PotMax      = 6,
    PotAll      = 0xFF,
    PotNum      = PotChTest1 /* 实际炉头数 */
};

/* ==================================================================
 * PPG 值类型 (与 API_HRTIM.h 保持一致)
 * ================================================================== */
typedef struct {
    uint16_t prioed;        /* PPG 周期 */
    uint16_t duty;          /* PPG 占空比 */
} PPGvalueDef;

typedef struct {
    uint16_t highOn;        /* 死区后高端开通 */
    uint16_t highOff;       /* 高端关闭 (duty) */
    uint16_t lowOn;         /* 死区后低端开通 */
    uint16_t lowOff;        /* 低端关闭 (period) */
} PPGpointDef;

#endif /* APP_POWER_HW_PARAM_H */
