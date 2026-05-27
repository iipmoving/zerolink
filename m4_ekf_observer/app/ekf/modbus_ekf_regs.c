/**
 * modbus_ekf_regs.c — EKF 遥测寄存器节点实现
 *
 * 独立的 MODBUS 寄存器块 0x1020-0x1029, 为 EKF 提供:
 *   相位角 / HRTIM 频率 / PPG 周期占空比 / PID 增量 / 谐振电流
 *
 * 数据流: PowerMem[ch] → API_POWER_EKF_GetTelemetry() → 本模块
 */

#include "modbus_ekf_regs.h"
#include "Modbus_Analysis_Lib.h"    /* Register_Area_t */
#include <string.h>

/* ================================================================
 * 4 从机的数据实例和寄存器区域配置
 * ================================================================ */

static EKF_Telemetry_t  EKF_Data_S1;
static EKF_Telemetry_t  EKF_Data_S2;
static EKF_Telemetry_t  EKF_Data_S3;
static EKF_Telemetry_t  EKF_Data_S4;

#define EKF_ARM_COUNT  1u  /* 每个从机 1 个 EKF 寄存器区域 */

static Register_Area_t EKF_Area_S1[EKF_ARM_COUNT];
static Register_Area_t EKF_Area_S2[EKF_ARM_COUNT];
static Register_Area_t EKF_Area_S3[EKF_ARM_COUNT];
static Register_Area_t EKF_Area_S4[EKF_ARM_COUNT];

/* ================================================================
 * 弱回调: 由 app_power.c 覆盖实现
 * ================================================================ */

__attribute__((weak))
void API_POWER_EKF_GetTelemetry(uint8_t chn, EKF_Telemetry_t *ekf)
{
    (void)chn;
    if (ekf) memset(ekf, 0, sizeof(EKF_Telemetry_t));
}

/* ================================================================
 * EKF_Regs_Init — 注册 EKF 寄存器区域到 MODBUS 配置
 * ================================================================ */

void EKF_Regs_Init(uint8_t slave_idx, uint8_t *tx_buf,
                   uint16_t tx_len, uint8_t slave_addr)
{
    EKF_Telemetry_t  *data_ptr = NULL;
    Register_Area_t  *area     = NULL;

    switch (slave_idx) {
        case 0:
            data_ptr = &EKF_Data_S1;
            area     = EKF_Area_S1;
            break;
        case 1:
            data_ptr = &EKF_Data_S2;
            area     = EKF_Area_S2;
            break;
        case 2:
            data_ptr = &EKF_Data_S3;
            area     = EKF_Area_S3;
            break;
        case 3:
            data_ptr = &EKF_Data_S4;
            area     = EKF_Area_S4;
            break;
        default:
            return;
    }

    /* 初始化为零 */
    memset(data_ptr, 0, sizeof(EKF_Telemetry_t));

    /* 配置 Register_Area_t */
    area[0].Start_Address   = EKF_REG_BASE;
    area[0].End_Address     = EKF_REG_BASE + EKF_REG_COUNT;
    area[0].Data_ptr        = (void*)data_ptr;
    area[0].Data_ptr_EEPROM = NULL;          /* 只读 */
    area[0].Check_Write_Data = NULL;         /* 只读, 无写检查 */
    area[0].Data_Size       = sizeof(uint16_t);
    area[0].Data_Pyte       = 0;             /* 只读 */

    /*
     * 注意: 调用方需要把 area 加入 Modbus_Cofg[] 的 ARM 列表。
     * 由于每个从机原有的 3 个 ARM 区域需要扩展为 4 个来容纳 EKF 区域，
     * 这一步在 Modbus_Lib_Init_An_Analysis.c 中完成。
     */
    (void)tx_buf;
    (void)tx_len;
    (void)slave_addr;
}

/* ================================================================
 * EKF_Regs_GetDataPtr — 返回 EKF 数据实例指针
 * ================================================================ */

void* EKF_Regs_GetDataPtr(uint8_t slave_idx)
{
    switch (slave_idx) {
        case 0: return &EKF_Data_S1;
        case 1: return &EKF_Data_S2;
        case 2: return &EKF_Data_S3;
        case 3: return &EKF_Data_S4;
        default: return NULL;
    }
}

/* ================================================================
 * EKF_Regs_Update — 刷新 EKF 遥测数据
 * ================================================================ */

void EKF_Regs_Update(uint8_t slave_idx)
{
    EKF_Telemetry_t *ekf = NULL;

    switch (slave_idx) {
        case 0: ekf = &EKF_Data_S1; break;
        case 1: ekf = &EKF_Data_S2; break;
        case 2: ekf = &EKF_Data_S3; break;
        case 3: ekf = &EKF_Data_S4; break;
        default: return;
    }

    /* 调用 power 层获取实时数据 (弱函数, app_power.c 覆盖) */
    API_POWER_EKF_GetTelemetry(slave_idx, ekf);
}
