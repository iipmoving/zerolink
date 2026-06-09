/**
 * @file    power_pid.c
 * @brief   Power PID 功能包 — 功率闭环计算
 * @layer   app
 *
 * 从 app_power_claude.c s_ppg_fun() 迁出。
 * 纯计算：V_ADC × I_ADC → 实际功率 → PID(target, actual) → ppg_delta。
 *
 * 依赖: S_PID.h (FixedPID_Compute), 通过 __weak 解耦
 */
#include "power_pid.h"
#include "app_power_hw_io.h"
#include <string.h>
#include <stdlib.h>

/* ---- PID 控制器实例 (每炉头独立) ---- */
#include "S_PID.h"   /* FixedPIDController, FixedPID_Compute */

#define POWER_PID_CNT  4
static FixedPIDController s_pid[POWER_PID_CNT];
static uint8_t  s_pid_inited;

/* 功率平滑 */
static uint16_t s_prev_actual_power[POWER_PID_CNT];

/* ---- 校准系数 ---- */
static uint16_t s_power_calib = 25;

/* ---- 内部相位限制 (从 APP_POWER_PhaseHalfTypeSet 迁入) ---- */
static void phase_limit(uint8_t ch, int16_t *delta)
{
    (void)ch;
    (void)delta;
    /* TODO: 迁入 APP_POWER_PhaseHalfTypeSet 逻辑 */
}

void PowerPid_SetCalib(uint16_t calib)
{
    s_power_calib = calib;
}

void PowerPid_Init(uint8_t ch, float kp, float ki, float kd, int factor)
{
    if (ch >= POWER_PID_CNT) return;
    FixedPID_Init(&s_pid[ch], kp, ki, kd, factor);
    s_prev_actual_power[ch] = 0;
}

void PowerPid_Compute(PowerBase_Input_t *in, PowerBase_OutHead_t *out, uint8_t ch, uint16_t target_power)
{
    if (ch >= POWER_PID_CNT) return;

    /* ---- 懒惰初始化 (默认 PID 参数) ---- */
    if (!s_pid_inited) {
        s_pid_inited = 1;
        for (uint8_t i = 0; i < POWER_PID_CNT; i++) {
            FixedPID_Init(&s_pid[i], 0.5f, 0.1f, 0.0f, 256);
        }
    }

    /* ---- 读取 ADC ---- */
    uint16_t voltage = in->pAdc[ch].voltage;
    uint16_t current = in->pAdc[ch].current;

    /* ---- 计算实际功率 (V_ADC × I_ADC / calib) ---- */
    uint32_t power_raw   = (uint32_t)voltage * current;
    uint16_t actual_power = (uint16_t)(power_raw / s_power_calib);
    s_prev_actual_power[ch] = actual_power;

    /* ---- 目标功率为零 → 无输出 ---- */
    if (target_power == 0 || voltage == 0 || current == 0) {
        out->ppg_delta   = 0;
        out->delta_valid = 0;
        return;
    }

    /* ---- PID 计算 ---- */
    int pid_return = FixedPID_Compute(&s_pid[ch],
                                      (fixed_point_t)target_power,
                                      (fixed_point_t)power_raw,
                                      (unsigned int)s_power_calib);

    /* ---- 方向编码 (与原 s_ppg_fun 兼容) ---- */
    /* 正数 = 升 PPG, 负数 = 降 PPG (bit7=1) */
    int16_t delta = abs(pid_return);
    if (delta > 127) delta = 127;   /* 限幅到 7-bit */

    if (pid_return < 0) {
        delta |= 0x80;   /* bit7 = 降方向 */
    } else if (pid_return == 0) {
        delta = 0;
    }

    /* ---- 相位限制 ---- */
    phase_limit(ch, &delta);

    out->ppg_delta   = delta;
    out->delta_valid = 1;
    out->power_state = (delta != 0) ? 1 : 0;
}
