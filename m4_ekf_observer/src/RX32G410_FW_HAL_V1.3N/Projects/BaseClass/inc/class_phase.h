
/**
 * @file    phase_controller.h
 * @brief  电磁炉相位角异常检测与保护模块接口
 * @note   - 全部使用定点整数运算，相位角×10
 *         - 所有布尔标志集成于联合体状态寄存器，支持整体清零
 *         - 仅管理相位异常状态，不包含功率、PWM 等外部变量
 *         - 结构体 8 字节，32 位对齐
 */

#ifndef PHASE_CONTROLLER_H
#define PHASE_CONTROLLER_H

#include <stdint.h>

/* ===================== 定点缩放系数 ===================== */
#define PHASE_SCALE         10      /**< 相位角放大倍数 (66.5° → 665) */
#define BASELINE_ALPHA_Q12  1229    /**< 基线滤波系数 α 的 Q12 表示 (0.3*4096) */

/* ===================== 阈值定义 (已含 PHASE_SCALE) ===================== */
#define PHASE_SUDDEN_RISE_THRESHOLD (15 * PHASE_SCALE)   /**< 突升阈值 15.0° → 150 */
#define PHASE_TOO_LOW_THRESHOLD     (10 * PHASE_SCALE)   /**< 过低阈值 10.0° → 100 */
#define PHASE_OVER_LIMIT            (100 * PHASE_SCALE)  /**< 移锅预警 100.0° → 1000 */

/* ===================== 控制参数 ===================== */
#define OVER_LIMIT_CONFIRM_COUNT    5   /**< 移锅确认连续超限次数 */
#define RISE_INHIBIT_CYCLES         5   /**< 异常后禁止增加保持周期数 */

/* ===================== 状态标志寄存器（联合体） ===================== */
/**
 * @brief 所有布尔标志的集合，可通过 .all 整体清零
 */
typedef union {
    struct {
        uint8_t baseline_initialized : 1; /**< 基线是否已初始化 (1:已初始化) */
        uint8_t baseline_locked      : 1; /**< 基线是否因突升锁定 (1:锁定) */
        uint8_t sudden_rise          : 1; /**< 检测到突升 (1:突升) */
        uint8_t too_low              : 1; /**< 相位角过低 (1:过低) */
        uint8_t over_limit_warn      : 1; /**< 移锅预警中 (1:预警) */
        uint8_t pot_removed          : 1; /**< 确认移锅 (1:移锅保护) */
        uint8_t inhibit_increase     : 1; /**< 禁止 PWM 增加 (1:禁止) */
        uint8_t reserved             : 1; /**< 保留 */
    } bits;
    uint8_t all;                           /**< 整体操作寄存器 */
} PhaseStatusRegister;

/* ===================== 控制器结构体 (32位对齐) ===================== */
/**
 * @brief 相位控制器实例，仅维护相位异常检测所需状态
 * @note  大小 8 字节，4 字节对齐
 */
typedef struct {
    int16_t  phase_angle;          /**< 当前相位角 (×10) [输入] */
    int16_t  phase_baseline;       /**< 相位角基线 (×10) [内部] */
    uint8_t  inhibit_counter;      /**< 禁止增加剩余周期计数 */
    uint8_t  over_limit_counter;   /**< 移锅连续超限计数 */
    PhaseStatusRegister status;    /**< 状态标志寄存器 */
    uint8_t  padding[1];           /**< 填充，保证 8 字节 4 对齐 */
} PhaseController;

/* 编译期检查 4 字节对齐 (可置于源文件) */
// extern int _align_check[sizeof(PhaseController) % 4 == 0 ? 1 : -1];

/* ===================== 接口函数 ===================== */

/**
 * @brief 初始化控制器，所有状态清零，基线待初始化
 * @param ctrl 控制器指针
 */
void PhaseController_Init(PhaseController *ctrl);

/**
 * @brief 外部手动重置基线（如功率调整或换锅后调用）
 * @param ctrl      控制器指针
 * @param cur_phase 当前相位角 (×10)，将作为新的基线起点
 */
void PhaseController_ResetBaseline(PhaseController *ctrl, int16_t cur_phase);

/**
 * @brief 每个控制周期调用一次，更新相位异常状态标志
 * @param ctrl 控制器指针
 * @note  调用前需将最新的 phase_angle (×10) 填入结构体；
 *        本函数只更新 status 标志和内部变量，不操作 PWM。
 */
void PhaseController_Update(PhaseController *ctrl);

#endif /* PHASE_CONTROLLER_H */
