/**
 * modbus_ekf_regs.h — EKF 遥测寄存器节点定义
 *
 * 独立的 MODBUS 寄存器块 0x1020-0x1029, 专门为 EKF/数据分析提供
 * 相位、频率、PPG 等数据。不修改现有的 COMM_RUN 结构。
 *
 * 依赖: <stdint.h>
 * 层级: PROTO — MODBUS 寄存器扩展
 */

#ifndef MODBUS_EKF_REGS_H
#define MODBUS_EKF_REGS_H

#include <stdint.h>

/* ========== EKF 遥测寄存器地址 ========== */
#define EKF_REG_BASE         0x1020u
#define EKF_REG_COUNT        10u

/* ========== EKF 遥测结构体 (只读, 0x1020-0x1029) ========== */
/* 频率: uint16 直接存 Hz 整数 (最大60kHz)
 * 其他: int32_t×100 源值 ÷10 后存 uint16 (最大6553) */
typedef struct {
    uint16_t phi_deg_x10;     /* 0x1020 相位角 (0.1°单位, 源÷10) */
    uint16_t f_sw_hz;         /* 0x1021 开关频率 (Hz 整数) */
    uint16_t i_peak_ax10;     /* 0x1022 峰值电流 (0.1A, 源÷10) */
    uint16_t l_uh_x10;        /* 0x1023 等效电感 (0.1μH, 源÷10) */
    uint16_t r_ohm_x10;       /* 0x1024 等效电阻 (0.1Ω, 源÷10) */
    uint16_t f_res_hz;        /* 0x1025 谐振频率 (Hz 整数) */
    uint16_t vdc_mean_v_x10;  /* 0x1026 母线电压 (0.1V, 源÷10) */
    uint16_t p_w_x10;         /* 0x1027 有功功率 (0.1W, 源÷10) */
    uint16_t i_rms_ax10;      /* 0x1028 电流有效值 (0.1A, 源÷10) */
    uint16_t z_ohm_x10;       /* 0x1029 阻抗模 (0.1Ω, 源÷10) */
} EKF_Telemetry_t;

/* ========== API ========== */

/**
 * @brief 刷新 EKF 遥测寄存器
 * @param slave_idx 从机索引 0-3
 * @note 在主循环的 MODBUS 协议解析前调用
 *       从 EKF_LKF_GetOutput() 获取滤波后的电参数 (int32_t ×100 定标)
 *       频率直接取整, 其他 ÷10 转 uint16 写入
 */
void EKF_Regs_Update(uint8_t slave_idx);

/**
 * @brief 获取 EKF 遥测数据指针 (用于 MODBUS 内存区域注册)
 * @param slave_idx 从机索引 0-3
 * @return EKF_Telemetry_t* 或 NULL
 */
void* EKF_Regs_GetDataPtr(uint8_t slave_idx);

#endif /* MODBUS_EKF_REGS_H */
