/**
 * @file    phase_controller.c
 * @brief  电磁炉相位角异常检测与保护模块（精简版）
 * @note   - 全部定点整数，相位角×10
 *         - 所有布尔标志集于联合体状态寄存器，可通过 .all 整体清零
 *         - 仅管理相位异常状态，不包含功率、PWM 等外部变量
 *         - 结构体 8 字节，32 位对齐
 */
#include "phase.h"
/* ===================== 接口函数 ===================== */

/**
 * @brief 初始化控制器，所有状态清零，基线待初始化
 */
void PhaseController_Init(PhaseController *ctrl) {
    ctrl->phase_angle = 0;
    ctrl->phase_baseline = 0;
    ctrl->inhibit_counter = 0;
    ctrl->over_limit_counter = 0;
    ctrl->status.all = 0;                /* 整体清零 */
    ctrl->padding[0] = 0;
}

/**
 * @brief 外部手动重置基线（如功率调整或换锅后调用）
 * @param ctrl      控制器指针
 * @param cur_phase 当前相位角 (×10)，将作为新的基线起点
 */
void PhaseController_ResetBaseline(PhaseController *ctrl, int16_t cur_phase) {
    ctrl->phase_baseline = cur_phase;
    ctrl->inhibit_counter = 0;
    ctrl->status.all = 0;
    ctrl->status.bits.baseline_initialized = 1;   /* 标记已初始化 */
}

/**
 * @brief 每个控制周期调用一次，更新相位异常状态标志
 * @note  调用前需将最新的 phase_angle (×10) 填入结构体；
 *        本函数只更新 status 标志和内部变量，不操作 PWM。
 */
void PhaseController_Update(PhaseController *ctrl) {
    /* 1. 移锅保护检测 (相位角 ≥100° 连续确认) */
    if (ctrl->phase_angle >= PHASE_OVER_LIMIT) {
        ctrl->over_limit_counter++;
        if (ctrl->over_limit_counter >= OVER_LIMIT_CONFIRM_COUNT) {
            ctrl->status.bits.pot_removed = 1;    /* 确认移锅 */
        }
    } else {
        ctrl->over_limit_counter = 0;
        ctrl->status.bits.pot_removed = 0;
    }

    /* 2. 基线初始化 (仅执行一次) */
    if (!ctrl->status.bits.baseline_initialized) {
        ctrl->phase_baseline = ctrl->phase_angle;
        ctrl->status.bits.baseline_initialized = 1;
    }

    /* 3. 突升检测 (相对基线 > 阈值) */
    int16_t delta_from_baseline = ctrl->phase_angle - ctrl->phase_baseline;
    if (!ctrl->status.bits.baseline_locked &&
        (delta_from_baseline > PHASE_SUDDEN_RISE_THRESHOLD)) {
        ctrl->status.bits.baseline_locked = 1;
        ctrl->status.bits.sudden_rise = 1;
        ctrl->status.bits.inhibit_increase = 1;
        ctrl->inhibit_counter = RISE_INHIBIT_CYCLES;
    }

    /* 基线未锁定时，一阶低通滤波跟踪 (Q12 定点) */
    if (!ctrl->status.bits.baseline_locked) {
        int32_t temp = (int32_t)delta_from_baseline * BASELINE_ALPHA_Q12;
        ctrl->phase_baseline += (int16_t)(temp >> 12);
    }

    /* 4. 相位角过低检测 */
    if (ctrl->phase_angle < PHASE_TOO_LOW_THRESHOLD) {
        ctrl->status.bits.too_low = 1;
        ctrl->status.bits.inhibit_increase = 1;
        ctrl->inhibit_counter = RISE_INHIBIT_CYCLES;
    } else {
        ctrl->status.bits.too_low = 0;
    }

    /* 5. 禁止增加保持周期管理 */
    if (ctrl->inhibit_counter > 0) {
        ctrl->inhibit_counter--;
        if (ctrl->inhibit_counter == 0) {
            ctrl->status.bits.inhibit_increase = 0;
            if (ctrl->status.bits.baseline_locked) {
                int16_t check_delta = ctrl->phase_angle - ctrl->phase_baseline;
                if (check_delta <= PHASE_SUDDEN_RISE_THRESHOLD) {
                    /* 异常消失：解锁基线，用当前值重置 */
                    ctrl->status.bits.baseline_locked = 0;
                    ctrl->status.bits.sudden_rise = 0;
                    ctrl->phase_baseline = ctrl->phase_angle;
                } else {
                    /* 异常仍存在，续期禁止 */
                    ctrl->status.bits.inhibit_increase = 1;
                    ctrl->inhibit_counter = RISE_INHIBIT_CYCLES;
                }
            }
        }
    }

    /* 6. 更新预警标志 (相位角 ≥100° 但未确认移锅) */
    if (ctrl->over_limit_counter > 0 && !ctrl->status.bits.pot_removed) {
        ctrl->status.bits.over_limit_warn = 1;
    } else {
        ctrl->status.bits.over_limit_warn = 0;
    }
}