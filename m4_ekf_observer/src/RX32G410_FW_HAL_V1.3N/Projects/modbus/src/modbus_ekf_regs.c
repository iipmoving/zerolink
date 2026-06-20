/**
 * modbus_ekf_regs.c — EKF 遥测寄存器节点实现
 *
 * 独立的 MODBUS 寄存器块 0x1020-0x1029 (10 regs)
 * 频率: uint16 直接存 Hz 整数 (最大60kHz)
 * 其他: int32_t×100 源值 ÷10 → uint16 (0.1单位)
 *
 * 数据流: EKF_LKF_GetOutput() → ÷10/取整 → EKF_Telemetry_t
 */

#include "modbus_ekf_regs.h"
#include "Modbus_Analysis_Lib.h"    /* Register_Area_t */
#include <string.h>

/* EKF_LKF 输出访问器 — 从 BaseClass 层获取 EKF 滤波后的电参数 */
extern void* EKF_LKF_GetOutput(uint8_t chn);

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

__attribute__((weak))void API_POWER_EKF_GetTelemetryCallback(uint8_t chn, EKF_Telemetry_t *ekf)
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

void EKF_Regs_Update(unsigned char slave_idx)
{
    EKF_Telemetry_t *ekf = NULL;

    switch (slave_idx) {
        case 0: ekf = &EKF_Data_S1; break;
        case 1: ekf = &EKF_Data_S2; break;
        case 2: ekf = &EKF_Data_S3; break;
        case 3: ekf = &EKF_Data_S4; break;
        default: return;
    }

    /* EKF 输出: int32_t ×100 定标
     * 频率: 取整直接存 (0~60000)
     * 其他: ÷10 转 0.1 单位 */
    void *raw = EKF_LKF_GetOutput(slave_idx);
    if (!raw) { memset(ekf, 0, sizeof(*ekf)); return; }

    /* 布局: I_peak_A(0) Vdc_mean(1) phi_deg(2) f_sw_Hz(3)
       L_uH(4) f_res_Hz(5) Q_factor(6) R_ohm(7)
       I_rms(8) P_W(9) Z_mag_ohm(10) X_ohm(11) */
    int32_t *p = (int32_t*)raw;

    /* 0x1020: 相位角 — phi_deg ÷10 → 0.1° */
    ekf->phi_deg_x10 = (uint16_t)((int32_t)p[2] / 10);

    /* 0x1021: 开关频率 — f_sw_Hz 直接存 uint16 (Hz×100 → ÷100 得 Hz) */
    ekf->f_sw_hz = (uint16_t)(p[3]  );

    /* 0x1022: 峰值电流 — I_peak_A ÷10 → 0.1A */
    ekf->i_peak_ax10 = (uint16_t)((int32_t)p[0] / 10);

    /* 0x1023: 等效电感 — L_uH ÷10 → 0.1μH */
    ekf->l_uh_x10 = (uint16_t)((int32_t)p[4] / 10);

    /* 0x1024: 等效电阻 — R_ohm ÷10 → 0.1Ω */
    ekf->r_ohm_x10 = (uint16_t)((int32_t)p[7] / 10);

    /* 0x1025: 谐振频率 — f_res_Hz 直接存 uint16 (Hz×100 → ÷100 得 Hz) */
    ekf->f_res_hz = (uint16_t)(p[5] );

    /* 0x1026: 母线电压 — Vdc_mean ÷10 → 0.1V */
    ekf->vdc_mean_v_x10 = (uint16_t)((int32_t)p[1] / 10);

    /* 0x1027: 有功功率 — P_W ÷10 → 0.1W */
    ekf->p_w_x10 = (uint16_t)((int32_t)p[9] / 10);

    /* 0x1028: 电流有效值 — I_rms ÷10 → 0.1A */
    ekf->i_rms_ax10 = (uint16_t)((int32_t)p[8] / 10);

    /* 0x1029: 阻抗模 — Z_mag_ohm ÷10 → 0.1Ω */
    ekf->z_ohm_x10 =(uint16_t)((int32_t)p[10] / 10);
}
