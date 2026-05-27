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
typedef struct {
    uint16_t Phase_Angle;       /* 0x1020 相位角 (0.1°单位, int16)     */
    uint16_t Freq_Hz_Hi;        /* 0x1021 频率高字 (uint32 Hz)         */
    uint16_t Freq_Hz_Lo;        /* 0x1022 频率低字                     */
    uint16_t Resonant_Curr;     /* 0x1023 谐振电流 ADC (uint16)       */
    uint16_t PPG_Period;        /* 0x1024 HRTIM 周期 prioed            */
    uint16_t PPG_Duty;          /* 0x1025 HRTIM 占空比                  */
    uint16_t Delta_PPG;         /* 0x1026 PID 输出增量 (int16)         */
    uint16_t Res1;              /* 0x1027 保留                         */
    uint16_t Res2;              /* 0x1028 保留                         */
    uint16_t Res3;              /* 0x1029 保留                         */
} EKF_Telemetry_t;

/* ========== API ========== */

/**
 * @brief 刷新 EKF 遥测寄存器
 * @param slave_idx 从机索引 0-3
 * @note 在主循环的 MODBUS 协议解析前调用
 *       内部调用 API_POWER_EKF_GetTelemetry() 获取实时数据
 */
void EKF_Regs_Update(uint8_t slave_idx);

/**
 * @brief 获取 EKF 遥测数据指针 (用于 MODBUS 内存区域注册)
 * @param slave_idx 从机索引 0-3
 * @return EKF_Telemetry_t* 或 NULL
 */
void* EKF_Regs_GetDataPtr(uint8_t slave_idx);

#endif /* MODBUS_EKF_REGS_H */
