# M4 固件补丁说明 — EKF 遥测寄存器节点

## 概述

在 MODBUS 寄存器表中新增独立的 EKF 遥测节点 (0x1020-0x1029)，
不修改现有 COMM_RUN / IH_STA_READ 结构。

## 新增文件

| 文件 | 位置 |
|------|------|
| `modbus_ekf_regs.h` | `app/ekf/` |
| `modbus_ekf_regs.c` | `app/ekf/` |

## 需要修改的现有文件

### 1. `Modbus_Lib_Init_An_Analysis.c`

#### 1.1 顶部添加 include

```c
#include "modbus_ekf_regs.h"
```

#### 1.2 修改 ARM 区域数量: 3 → 4

找到四行宏定义，全部改 3 为 4:

```c
#define DF_Modbus_ARM1_Num  4  // 原来是 3
#define DF_Modbus_ARM2_Num  4
#define DF_Modbus_ARM3_Num  4
#define DF_Modbus_ARM4_Num  4
```

#### 1.3 在每个从机的 Modbus_Cofg_Init_SET() 末尾加第4区域

在每个从机的 `// 设置从机 N 的基本信息` 之前，插入:

```c
// 配置内存区域 4 (EKF 遥测: 地址 0x1020 开始的只读区域)
EKF_Regs_Init(0, Modbus_TX_BUFF, DF_Modbus_S1_TX_Len,
              Modbus_Cofg[0].Slave_Hardware_Addr);
Modbus_ARM1_Addr_SET[3] = EKF_Area_S1[0];  // 注意: 需要 extern EKF_Area_S1
```

**简化方案**: 直接在 `Modbus_Cofg_Init_SET()` 末尾统一追加:

```c
// --- EKF 遥测区域 (追加在每个从机已配置完成后) ---
// 从机1 加第4区域
Modbus_ARM1_Addr_SET[3].Start_Address   = 0x1020;
Modbus_ARM1_Addr_SET[3].End_Address     = 0x102A;
Modbus_ARM1_Addr_SET[3].Data_ptr        = (void*)&EKF_Data_S1;
Modbus_ARM1_Addr_SET[3].Data_ptr_EEPROM = NULL;
Modbus_ARM1_Addr_SET[3].Check_Write_Data = NULL;
Modbus_ARM1_Addr_SET[3].Data_Size       = sizeof(unsigned short);
Modbus_ARM1_Addr_SET[3].Data_Pyte       = 0; // 只读

// 从机2
Modbus_ARM2_Addr_SET[3].Start_Address   = 0x1020;
Modbus_ARM2_Addr_SET[3].End_Address     = 0x102A;
Modbus_ARM2_Addr_SET[3].Data_ptr        = (void*)&EKF_Data_S2;
Modbus_ARM2_Addr_SET[3].Data_ptr_EEPROM = NULL;
Modbus_ARM2_Addr_SET[3].Check_Write_Data = NULL;
Modbus_ARM2_Addr_SET[3].Data_Size       = sizeof(unsigned short);
Modbus_ARM2_Addr_SET[3].Data_Pyte       = 0;

// 从机3
Modbus_ARM3_Addr_SET[3].Start_Address   = 0x1020;
Modbus_ARM3_Addr_SET[3].End_Address     = 0x102A;
Modbus_ARM3_Addr_SET[3].Data_ptr        = (void*)&EKF_Data_S3;
Modbus_ARM3_Addr_SET[3].Data_ptr_EEPROM = NULL;
Modbus_ARM3_Addr_SET[3].Check_Write_Data = NULL;
Modbus_ARM3_Addr_SET[3].Data_Size       = sizeof(unsigned short);
Modbus_ARM3_Addr_SET[3].Data_Pyte       = 0;

// 从机4
Modbus_ARM4_Addr_SET[3].Start_Address   = 0x1020;
Modbus_ARM4_Addr_SET[3].End_Address     = 0x102A;
Modbus_ARM4_Addr_SET[3].Data_ptr        = (void*)&EKF_Data_S4;
Modbus_ARM4_Addr_SET[3].Data_ptr_EEPROM = NULL;
Modbus_ARM4_Addr_SET[3].Check_Write_Data = NULL;
Modbus_ARM4_Addr_SET[3].Data_Size       = sizeof(unsigned short);
Modbus_ARM4_Addr_SET[3].Data_Pyte       = 0;
```

#### 1.4 在 Modbus_Protocol_Analysis_Main() 中刷新 EKF 数据

在每个 `Update_Static_Register_DATA(DF_Modbus_Slave_0N)` 调用之后、
`processModbusRequest()` 之前，插入:

```c
EKF_Regs_Update(DF_Modbus_Slave_01);  // 刷新 EKF 遥测
```

(四个从机各加一行)

#### 1.5 添加 extern 声明 (文件顶部, include 之后)

```c
extern EKF_Telemetry_t  EKF_Data_S1;
extern EKF_Telemetry_t  EKF_Data_S2;
extern EKF_Telemetry_t  EKF_Data_S3;
extern EKF_Telemetry_t  EKF_Data_S4;
```

---

### 2. `app_power.c` — 覆盖弱回调

在文件末尾 (约 5000 行附近, API_POWER_TxStatusCallback 之后) 添加:

```c
#include "modbus_ekf_regs.h"

void API_POWER_EKF_GetTelemetry(uint8_t chn, EKF_Telemetry_t *ekf)
{
    AppPowerDef *saved;
    PPGvalueDef  ppg;
    uint32_t     freq_hz;

    if (!ekf) return;
    if (chn >= 4) return;

    saved = PowerControl;

    /* 切换到目标炉头 */
    if      (chn == 0) PowerControl = &PowerMem[0];
    else if (chn == 1) PowerControl = &PowerMem[1];
    else if (chn == 2) PowerControl = &PowerMem[2];
    else if (chn == 3) PowerControl = &PowerMem[3];

    /* 相位值 (原始 8-bit ADC 值, 或 PhaseController.phase_angle 的 0.1° 值) */
    ekf->Phase_Angle = (uint16_t)PowerControl->staticReg->phaseValue;

    /* 频率: f_hz = 384000000 / prioed */
    ppg = FunPPGgetValue();
    if (ppg.prioed > 0) {
        freq_hz = 384000000UL / (uint32_t)ppg.prioed;
    } else {
        freq_hz = 0;
    }
    ekf->Freq_Hz_Hi = (uint16_t)(freq_hz >> 16);
    ekf->Freq_Hz_Lo = (uint16_t)(freq_hz & 0xFFFF);

    /* PPG 周期和占空比 */
    ekf->PPG_Period = ppg.prioed;
    ekf->PPG_Duty   = ppg.duty;

    /* PID 增量 (TODO: 从 PID 结构体读取最后一次的 delta_output) */
    ekf->Delta_PPG = 0;

    /* 谐振电流 (TODO: 从 TXA ADC 读取) */
    ekf->Resonant_Curr = 0;

    /* 恢复上下文 */
    PowerControl = saved;
}
```

如果 `FunPPGgetValue()` 或 `PowerMem[]` 在 app_power.c 中是 static 的，
需要在文件头部确认它们的可见性。如果是 static，改为非 static 或添加 getter 函数。

---

## 验证步骤

1. 编译 M4 固件，确认无编译错误
2. 上电后用 PC 工具测试:
   ```
   python m4_modbus_tool.py COM3 --read-ekf
   ```
3. 检查 0x1021-0x1022 的频率值是否合理 (应在 20000-60000 Hz 范围)
4. 检查 0x1020 的相位值是否随功率变化
