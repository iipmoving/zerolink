//========================================================================
// 描述: MODBUS 协议配置、初始化、数据解析与回调桥接
//       Array-of-structs 消除 4× 重复代码
//       调用 Modbus_Analysis_Lib.c 协议引擎 (CRC/帧解析/响应构建)
//========================================================================
#include <string.h>
#include "Modbus_Analysis_Lib.h"
#include "Modbus_Lib_Init_An_Analysis.h"
#include "API_UART.h"
#include "../../../../../app/ekf/modbus_ekf_regs.h"

/* ========== Capture 模块 __weak 接入点 ================================= */
/*
 * 通过 __weak 回调与 wave_capture.c / raw_capture.c 零耦合。
 * 若 capture 模块未链接 → 弱符号返回 0/NULL → MODBUS 区域无效但不会崩溃。
 * 帧大小常量与 capture 模块 struct sizeof 保持同步 (AI 保证)。
 */
#define WAVE_FRAME_WORDS    (6 + 4000)   /* header(6w) + data(4000w) */
#define RAW_FRAME_WORDS     (6 + 4000)   /* header(6w) + data(4000w) */

__attribute__((weak)) void* WaveCapture_GetFramePtr(void) { return 0; }
__attribute__((weak)) void* RawCapture_GetFramePtr(void)  { return 0; }
__attribute__((weak)) unsigned char WaveCapture_OnAckWrite(void) { return 1; }
__attribute__((weak)) unsigned char RawCapture_OnAckWrite(void)  { return 1; }

/* ========== 常量 ===================================================== */
#define DF_Stove_Quantity       4
#define DF_Versions             1
#define DF_MB_Uart_Rx_LONG      50
#define DF_Modbus_AREA_COUNT    6   /* 区域数: 0x1000,0x2000,0x3000,0x1020,0x5000,0x5100 */

/* MODBUS 从机地址 */
static const unsigned char s_slave_addrs[DF_Stove_Quantity] = {0x05, 10, 15, 20};

/* ========== 从机枚举 (保持与原代码兼容) =============================== */
typedef enum {
    DF_Modbus_Slave_01 = 0,
    DF_Modbus_Slave_02,
    DF_Modbus_Slave_03,
    DF_Modbus_Slave_04,
} _Modbus_Slave_Number;

/* ========== MODBUS 寄存器结构体 (ABI 不变) =========================== */
typedef struct {
    unsigned short  SYS_Sta;                /* 0x1000 系统状态 */
    unsigned short  Vol_AD;                 /* 0x1001 电压AD */
    unsigned short  Current_AD;             /* 0x1002 电流AD */
    unsigned short  IGBT_AD;                /* 0x1003 IGBT温度AD */
    unsigned short  Bot_AD;                 /* 0x1004 炉面温度AD */
    unsigned short  Top_AD;                 /* 0x1005 顶部温度AD */
    unsigned short  Practical_Power;        /* 0x1006 实际功率 */
    unsigned short  target_Power;           /* 0x1007 目标功率(回读) */
    unsigned short  Practical_PPG;          /* 0x1008 实际PPG */
    unsigned short  P_limited_STA;          /* 0x1009 功率限制状态 */
    unsigned short  Pan_pulsating;          /* 0x100A 移锅脉冲+浪涌 */
    unsigned short  HVol_Cnt;               /* 0x100B 反压计数值 */
    unsigned short  PWMValue_L;             /* 0x100C 频率限制值低 */
    unsigned short  PWMValue_H;             /* 0x100D 频率限制值高 */
    unsigned short  PowerAdjust;            /* 0x100E PPG修正值 */
    unsigned short  Version_Number;         /* 0x100F 版本号 */
    unsigned short  Fan_AD;                 /* 0x1010 风扇AD */
    unsigned short  ERROR;                  /* 0x1011 故障码 */
    unsigned short  interior_ERR;           /* 0x1012 内部故障 */
    unsigned short  HZ_Cnt;                 /* 0x1013 频率计数值 */
    unsigned short  Discard_Cnt;            /* 0x1014 丢波计数值 */
} IH_STA_READ;

typedef struct {
    unsigned short  Check_Pan_LV;           /* 0x2000 检锅强度设定 */
    unsigned short  PPG_Max;                /* 0x2001 最大PPG限制 */
    unsigned short  Pan_Power;              /* 0x2002 移锅功率 */
    unsigned short  HVol_Limited;           /* 0x2003 反压限制 */
    unsigned short  Load_Current;           /* 0x2004 负载有效电流 */
    unsigned short  Current_calibration;    /* 0x2005 电流修正系数 */
    unsigned short  Power_MIX;              /* 0x2006 最小连续功率 */
    unsigned short  Power_MAX;              /* 0x2007 最大连续功率 */
    unsigned short  wrong_Pan;              /* 0x2008 恶略锅具保护功率 */
    unsigned short  syntony_Current;        /* 0x2009 谐振电流保护值 */
    unsigned short  phase_Pan;              /* 0x200A 移锅相位 */
    unsigned short  phase_Mix;              /* 0x200B 最小相位 */
    unsigned short  steel_calibration;      /* 0x200C 钢铁锅修正 */
    unsigned short  N_Pan_syntony_C;        /* 0x200D 移锅谐振电流限制 */
    unsigned short  Work_STA;               /* 0x200E 工作状态 */
    unsigned short  FAN_Speed;              /* 0x200F 风扇转速 */
    unsigned short  target_Power;           /* 0x2010 目标功率 */
    unsigned short  intermittent_Heat;      /* 0x2011 间断加热 */
    unsigned short  jitter_frequency;       /* 0x2012 抖频参数 */
    unsigned short  BuzzCof;                /* 0x2013 蜂鸣器控制 */
    unsigned short  syntony_Current_Short;  /* 0x2014 短路保护 */
} IH_STA_READ_WRITE;

typedef struct {
    unsigned short  Power_Calibration;      /* 0x3000 功率校准值 */
    unsigned short  Slave_Addr;             /* 0x3001 从机地址 */
    unsigned short  Baud_rate_SET;          /* 0x3002 波特率 */
    unsigned short  Save_order;             /* 0x3003 保存命令 */
} IH_STA_READ_WRITE_SYS_SET;

/* ========== I2C 桥接结构体 (ABI 不变) ================================ */
typedef struct {
    uint8_t ihStatus;
    uint8_t voltageAd;
    uint8_t currentAd;
    uint8_t igbtAd;
    uint8_t bottomAd;
    uint8_t topAd;
    uint8_t actualPowerDiv25;
    uint8_t targetPowerDiv25;
    uint8_t actualPPG;
    uint8_t powerStatus;
    uint8_t loadValue;
    uint8_t vcountValue;
    uint8_t equivalentResistance;
    uint8_t res2;
    uint8_t powerP25;
} COMM_RUN;

typedef struct {
    uint8_t loadTest;
    uint8_t ovpShort;
    uint8_t loadLeave;
    uint8_t vcLimitMax;
    uint8_t loadLeavePhase;
    uint8_t minPhase;
    uint8_t potPowerM;
    uint8_t maxPowerM;
} IH_RunInit_t;

typedef struct {
    uint8_t powerControlSet;
    uint8_t powerSwitch;
    uint8_t powerSetm;
    uint8_t fanSpeed;
    uint8_t kValue;
} PowerControlDef;

/* ========== 从机上下文 — 每通道独立 ================================== */
typedef struct {
    IH_STA_READ               reg_1000;
    IH_STA_READ_WRITE         reg_2000;       /* RAM 工作副本 */
    IH_STA_READ_WRITE         reg_2000_eep;   /* EEPROM 镜像 */
    IH_STA_READ_WRITE_SYS_SET reg_3000;       /* RAM 工作副本 */
    IH_STA_READ_WRITE_SYS_SET reg_3000_eep;   /* EEPROM 镜像 */
    unsigned char             tx_buf[DF_MB_Uart_Rx_LONG];
} ModbusSlaveCtx;

/* ========== 全局变量 ================================================= */

/* UART 接收缓冲 (ISR 写入) */
unsigned char MB_Uart_Rx_Data[DF_MB_Uart_Rx_LONG] = {0};
unsigned char MB_Uart_Rx_Long;

/* I2C 桥接数据 (保持原名 — 外部可能引用) */
COMM_RUN        Read_IH_STA[DF_Stove_Quantity];
IH_RunInit_t    IH_Work_Init[DF_Stove_Quantity];
PowerControlDef IH_Work_SET[DF_Stove_Quantity];

/* 从机上下文 */
static ModbusSlaveCtx s_slaves[DF_Stove_Quantity];

/* MODBUS 协议引擎配置 (必须连续 — Modbus_Init_Lib 要求) */
static PDUData_TypeDef s_pdu_cfg[DF_Stove_Quantity];
static Register_Area_t s_areas[DF_Stove_Quantity][DF_Modbus_AREA_COUNT];

/* ========== __weak 回调 (APP 层重写) ================================ */

__attribute__((weak)) void API_UART_RxControlCallback(uint8_t ch, int8_t *buff, uint8_t len) {}
__attribute__((weak)) void API_UART_RxInitCallback(uint8_t ch, int8_t *buff, uint8_t len) {}

__attribute__((weak)) uint8_t* API_UART_TxStatusCallback(uint8_t ch, uint8_t len)
{
    return 0;
}

__attribute__((weak)) uint8_t* API_UART_TxInitCallback(uint8_t chn, uint8_t len)
{
    return 0;
}

/* ========== Check_Write_Data 跳板函数 ================================ */
/*
 * 协议引擎调用 unsigned char (*)(void) — 无参数。
 * 每个跳板函数硬编码要校验的从机索引。
 * S1/S4: Power_Calibration [36, 96]; S2/S3: 全部放行。
 * 所有 0x2000 校验: 全部放行。
 */

/* --- S1 --- */
static unsigned char S1_Check_Write_0x2000(void) { return 1; }
static unsigned char S1_Check_Write_0x3000(void) {
    unsigned short v = s_slaves[0].reg_3000.Power_Calibration;
    return (v >= 36 && v <= 96) ? 1 : 0;
}

/* --- S2 --- */
static unsigned char S2_Check_Write_0x2000(void) { return 1; }
static unsigned char S2_Check_Write_0x3000(void) { return 1; }

/* --- S3 --- */
static unsigned char S3_Check_Write_0x2000(void) { return 1; }
static unsigned char S3_Check_Write_0x3000(void) { return 1; }

/* --- S4 --- */
static unsigned char S4_Check_Write_0x2000(void) { return 1; }
static unsigned char S4_Check_Write_0x3000(void) {
    unsigned short v = s_slaves[3].reg_3000.Power_Calibration;
    return (v >= 36 && v <= 96) ? 1 : 0;
}

/* 跳板函数查找表 */
static unsigned char (*const s_check_0x2000[DF_Stove_Quantity])(void) = {
    S1_Check_Write_0x2000, S2_Check_Write_0x2000,
    S3_Check_Write_0x2000, S4_Check_Write_0x2000
};
static unsigned char (*const s_check_0x3000[DF_Stove_Quantity])(void) = {
    S1_Check_Write_0x3000, S2_Check_Write_0x3000,
    S3_Check_Write_0x3000, S4_Check_Write_0x3000
};

/* ========== load_default_init (m4_init_config.json 默认值) =========== */
/*
 * 上电默认值 — 与 m4_init_config.json 严格一致。
 * 在 I2C 回调返回 NULL 时提供合理的初始参数，避免寄存器全零。
 */
static void load_default_init(IH_STA_READ_WRITE *r2k)
{
    r2k->Check_Pan_LV         = 24;   /* 0x2000 */
    r2k->PPG_Max              = 144;  /* 0x2001 */
    r2k->Pan_Power            = 24;   /* 0x2002 */
    r2k->HVol_Limited         = 96;   /* 0x2003 */
    r2k->Load_Current         = 32;   /* 0x2004 */
    r2k->Current_calibration  = 16;   /* 0x2005 */
    r2k->Power_MIX            = 40;   /* 0x2006 */
    r2k->Power_MAX            = 88;   /* 0x2007 */
    r2k->wrong_Pan            = 0;    /* 0x2008 */
    r2k->syntony_Current      = 0;    /* 0x2009 */
    r2k->phase_Pan            = 16;   /* 0x200A */
    r2k->phase_Mix            = 143;  /* 0x200B */
    r2k->steel_calibration    = 0;    /* 0x200C */
    r2k->N_Pan_syntony_C      = 0;    /* 0x200D */
    r2k->Work_STA             = 0;    /* 0x200E */
    r2k->FAN_Speed            = 0xAA; /* 0x200F 风机常开 */
    r2k->target_Power         = 0;    /* 0x2010 由 MODBUS 下发, 不做预设 */
    r2k->intermittent_Heat    = 0;    /* 0x2011 */
    r2k->jitter_frequency     = 0;    /* 0x2012 关=0, 开=2 */
    r2k->BuzzCof              = 0;    /* 0x2013 */
    r2k->syntony_Current_Short = 144; /* 0x2014 */
}

/* ========== Get_IHPower_Main_Init_DATA =============================== */
/*
 * I2C 字段交叉映射 (保留固件原有行为):
 *   I2C potPowerM → MODBUS Power_MAX   (名字互换)
 *   I2C maxPowerM  → MODBUS Power_MIX   (名字互换)
 *
 * 调用 load_default_init() 预填默认值，I2C 回调有值则覆盖。
 */
static void Get_IHPower_Main_Init_DATA(void)
{
    unsigned char i;
    for (i = 0; i < DF_Stove_Quantity; i++) {
        IH_STA_READ_WRITE *r2k = &s_slaves[i].reg_2000_eep;
        IH_RunInit_t st;
        uint8_t *pStatus;

        /* 先加载 JSON 默认值，确保 NULL 回调时也有合理参数 */
        load_default_init(r2k);

        pStatus = API_UART_TxInitCallback(i, sizeof(IH_RunInit_t));
        if (pStatus != NULL) {
            memcpy(&st, pStatus, sizeof(IH_RunInit_t));
            r2k->Check_Pan_LV          = st.loadTest;
            r2k->syntony_Current_Short  = st.ovpShort;
            r2k->Pan_Power             = st.loadLeave;
            r2k->syntony_Current       = st.vcLimitMax;
            r2k->phase_Pan             = st.loadLeavePhase;
            r2k->phase_Mix             = st.minPhase;
            r2k->Power_MAX             = st.potPowerM;   /* cross-wired */
            r2k->Power_MIX             = st.maxPowerM;   /* cross-wired */
        }
    }
}

/* ========== init_SR_RW_DATA ========================================== */
static void init_SR_RW_DATA(void)
{
    static unsigned char Modbus_DATA_Init = 0;
    unsigned char i;
    if (Modbus_DATA_Init) return;
    Modbus_DATA_Init = 0xAA;

    for (i = 0; i < DF_Stove_Quantity; i++) {
        memset(&s_slaves[i].reg_1000,     0, sizeof(IH_STA_READ));
        memset(&s_slaves[i].reg_2000,     0, sizeof(IH_STA_READ_WRITE));
        memset(&s_slaves[i].reg_3000,     0, sizeof(IH_STA_READ_WRITE_SYS_SET));
        memset(&s_slaves[i].reg_3000_eep, 0, sizeof(IH_STA_READ_WRITE_SYS_SET));
        /* reg_2000_eep 先 memset 清零，再在 Get_IHPower_Main_Init_DATA 中
           由 load_default_init() 填默认值，最后 I2C 回调有值则覆盖 */
        memset(&s_slaves[i].reg_2000_eep, 0, sizeof(IH_STA_READ_WRITE));
    }

    Get_IHPower_Main_Init_DATA();
}

/* ========== Modbus_Cofg_Init_SET ===================================== */
static void Modbus_Cofg_Init_SET(void)
{
    static unsigned char Modbus_Init = 0;
    unsigned char i;
    if (Modbus_Init) return;
    Modbus_Init = 0xAA;

    API_UART_DMA_ReadValue(UARTX, MB_Uart_Rx_Data, DF_MB_Uart_Rx_LONG);

    for (i = 0; i < DF_Stove_Quantity; i++) {
        /* Area 0: 0x1000 只读状态 */
        s_areas[i][0].Start_Address   = 0x1000;
        s_areas[i][0].End_Address     = 0x1000 + (sizeof(IH_STA_READ) / sizeof(unsigned short));
        s_areas[i][0].Data_ptr        = (void*)&s_slaves[i].reg_1000;
        s_areas[i][0].Data_ptr_EEPROM = NULL;
        s_areas[i][0].Check_Write_Data = NULL;
        s_areas[i][0].Data_Size       = sizeof(unsigned short);
        s_areas[i][0].Data_Pyte       = 0;

        /* Area 1: 0x2000 可读可写 */
        s_areas[i][1].Start_Address   = 0x2000;
        s_areas[i][1].End_Address     = 0x2000 + (sizeof(IH_STA_READ_WRITE) / sizeof(unsigned short));
        s_areas[i][1].Data_ptr        = (void*)&s_slaves[i].reg_2000;
        s_areas[i][1].Data_ptr_EEPROM = (void*)&s_slaves[i].reg_2000_eep;
        s_areas[i][1].Check_Write_Data = s_check_0x2000[i];
        s_areas[i][1].Data_Size       = sizeof(unsigned short);
        s_areas[i][1].Data_Pyte       = 1;

        /* Area 2: 0x3000 可读可写系统设置 */
        s_areas[i][2].Start_Address   = 0x3000;
        s_areas[i][2].End_Address     = 0x3000 + (sizeof(IH_STA_READ_WRITE_SYS_SET) / sizeof(unsigned short));
        s_areas[i][2].Data_ptr        = (void*)&s_slaves[i].reg_3000;
        s_areas[i][2].Data_ptr_EEPROM = (void*)&s_slaves[i].reg_3000_eep;
        s_areas[i][2].Check_Write_Data = s_check_0x3000[i];
        s_areas[i][2].Data_Size       = sizeof(unsigned short);
        s_areas[i][2].Data_Pyte       = 1;

        /* Area 3: 0x1020 EKF 遥测 (只读) */
        s_areas[i][3].Start_Address   = EKF_REG_BASE;
        s_areas[i][3].End_Address     = EKF_REG_BASE + EKF_REG_COUNT;
        s_areas[i][3].Data_ptr        = EKF_Regs_GetDataPtr(i);
        s_areas[i][3].Data_ptr_EEPROM = NULL;
        s_areas[i][3].Check_Write_Data = NULL;
        s_areas[i][3].Data_Size       = sizeof(unsigned short);
        s_areas[i][3].Data_Pyte       = 0;


        /* Area 4: 0x5000 WaveCapture 波形采集 (R/W: 控制寄存器可写, 帧数据只读) */
        s_areas[i][4].Start_Address   = 0x5000;
        s_areas[i][4].End_Address     = 0x5000 + (WAVE_FRAME_WORDS);
        s_areas[i][4].Data_ptr        = WaveCapture_GetFramePtr();    /* 全局单缓冲, 所有炉头共享 */
        s_areas[i][4].Data_ptr_EEPROM = NULL;
        s_areas[i][4].Check_Write_Data = WaveCapture_OnAckWrite;     /* ACK写入→自动解冻 */
        s_areas[i][4].Data_Size       = sizeof(unsigned short);
        s_areas[i][4].Data_Pyte       = 1;                           /* R/W: 主机写 ACK/ctrl, 读帧数据 */

        /* Area 5: 0x5100 RawCapture 原始9列采集 (R/W: 控制寄存器可写, 数据只读) */
        s_areas[i][5].Start_Address   = 0x5100;
        s_areas[i][5].End_Address     = 0x5100 + (RAW_FRAME_WORDS);
        s_areas[i][5].Data_ptr        = RawCapture_GetFramePtr();
        s_areas[i][5].Data_ptr_EEPROM = NULL;
        s_areas[i][5].Check_Write_Data = RawCapture_OnAckWrite;      /* ACK写入→自动解冻 */
        s_areas[i][5].Data_Size       = sizeof(unsigned short);
        s_areas[i][5].Data_Pyte       = 1;                           /* R/W: 主机写 ACK, 读帧数据 */




        /* PDU 配置 */
        s_pdu_cfg[i].Us_Cof_ARM_Num       = s_areas[i];
        s_pdu_cfg[i].ARM_Count            = DF_Modbus_AREA_COUNT;
        s_pdu_cfg[i].Slave_Hardware_Addr  = s_slave_addrs[i];
        s_pdu_cfg[i].Slave_Hardware_Addr_ERR_RET = 0;
        s_pdu_cfg[i].Tx_Buf               = s_slaves[i].tx_buf;
        s_pdu_cfg[i].Comm_Tx_Len_Max      = DF_MB_Uart_Rx_LONG;
        s_pdu_cfg[i].CRC_Order            = 0;
    }

    Modbus_Init_Lib(s_pdu_cfg, DF_Stove_Quantity);
}

/* ========== Get_Slave_Hardware_Addr ================================== */
unsigned char Get_Slave_Hardware_Addr(unsigned char Slave_ID)
{
    if (Slave_ID >= DF_Stove_Quantity) return 0;
    return s_slave_addrs[Slave_ID];
}

/* ========== Get_0x2000_ADDR_DATA ===================================== */
unsigned short Get_0x2000_ADDR_DATA(IH_0x2000_MEMBER_IDX member_idx,
                                     ARM_DEVICE_IDX arm_idx)
{
    if (arm_idx >= DF_Stove_Quantity) return 0;
    if (member_idx >= MEMBER_COUNT_0x2000) return 0;
    return ((unsigned short*)&s_slaves[arm_idx].reg_2000_eep)[member_idx];
}

/* ========== Get_0x3000_ADDR_DATA ===================================== */
unsigned short Get_0x3000_ADDR_DATA(IH_0x3000_MEMBER_IDX member_idx,
                                     ARM_DEVICE_IDX arm_idx)
{
    if (arm_idx >= DF_Stove_Quantity) return 0;
    if (member_idx >= MEMBER_COUNT_0x3000) return 0;
    return ((unsigned short*)&s_slaves[arm_idx].reg_3000_eep)[member_idx];
}

/* ========== Modbus_I2c_Data_Main ===================================== */
/*
 * I2C 字段交叉映射 (与 Get_IHPower_Main_Init_DATA 方向相反):
 *   MODBUS Power_MIX → I2C potPowerM   (名字互换)
 *   MODBUS Power_MAX → I2C maxPowerM   (名字互换)
 */
void Modbus_I2c_Data_Main(unsigned char Idx)
{
    IH_STA_READ_WRITE         *r2k;
    IH_STA_READ_WRITE_SYS_SET *r3k;

    if (Idx > DF_Modbus_Slave_04) return;

    r2k = &s_slaves[Idx].reg_2000_eep;
    r3k = &s_slaves[Idx].reg_3000_eep;
    (void)r3k;  /* 保留以备将来使用 */

    IH_Work_Init[Idx].loadTest       = r2k->Check_Pan_LV;
    IH_Work_Init[Idx].ovpShort       = r2k->syntony_Current_Short;
    IH_Work_Init[Idx].loadLeave      = r2k->Pan_Power;
    IH_Work_Init[Idx].vcLimitMax     = r2k->HVol_Limited;
    IH_Work_Init[Idx].loadLeavePhase = r2k->phase_Pan;
    IH_Work_Init[Idx].minPhase       = r2k->phase_Mix;
    IH_Work_Init[Idx].potPowerM      = r2k->Power_MIX;    /* cross-wired */
    IH_Work_Init[Idx].maxPowerM      = r2k->Power_MAX;    /* cross-wired */

    IH_Work_SET[Idx].powerControlSet = r2k->Work_STA;
    IH_Work_SET[Idx].powerSwitch     = r2k->jitter_frequency;
    IH_Work_SET[Idx].powerSetm       = r2k->target_Power;
    IH_Work_SET[Idx].fanSpeed        = r2k->FAN_Speed;
    IH_Work_SET[Idx].kValue          = 0x00;

    API_UART_RxInitCallback(Idx, (int8_t*)&IH_Work_Init[Idx], sizeof(IH_RunInit_t));
    API_UART_RxControlCallback(Idx, (int8_t*)&IH_Work_SET[Idx], sizeof(PowerControlDef));
}

/* ========== Update_Static_Register_DATA ============================== */
void Update_Static_Register_DATA(unsigned char Idx)
{
    IH_STA_READ               *r1k;
    IH_STA_READ_WRITE         *r2k;
    IH_STA_READ_WRITE_SYS_SET *r3k;
    IH_RunInit_t               st;
    uint8_t                   *pStatus;

    if (Idx > DF_Modbus_Slave_04) return;

    r1k = &s_slaves[Idx].reg_1000;
    r2k = &s_slaves[Idx].reg_2000;
    r3k = &s_slaves[Idx].reg_3000;
    (void)r3k;  /* 保留以备将来使用 */

    /* 读取 I2C 运行状态 */
    pStatus = API_UART_TxStatusCallback(Idx, sizeof(COMM_RUN));
    if (pStatus != NULL) {
        memcpy(&Read_IH_STA[Idx], pStatus, sizeof(COMM_RUN));
    }

    /* 读取 I2C 初始化参数 (可能已被底层修改) */
    pStatus = API_UART_TxInitCallback(Idx, sizeof(IH_RunInit_t));
    if (pStatus != NULL) {
        memcpy(&st, pStatus, sizeof(IH_RunInit_t));
        r2k->Check_Pan_LV         = st.loadTest;
        r2k->syntony_Current_Short = st.ovpShort;
        r2k->Pan_Power            = st.loadLeave;
        r2k->syntony_Current      = st.vcLimitMax;
        r2k->phase_Pan            = st.loadLeavePhase;
        r2k->phase_Mix            = st.minPhase;
        r2k->Power_MAX            = st.potPowerM;   /* cross-wired */
        r2k->Power_MIX            = st.maxPowerM;   /* cross-wired */
    }
    /* NULL 时保持 MODBUS 寄存器原有值不变 */

    /* I2C → 0x1000 状态寄存器映射 */
    r1k->SYS_Sta         = (Read_IH_STA[Idx].ihStatus & 0xF0) | (Idx + 1);
    r1k->Vol_AD          = Read_IH_STA[Idx].voltageAd;
    r1k->Current_AD      = Read_IH_STA[Idx].currentAd;
    r1k->IGBT_AD         = Read_IH_STA[Idx].igbtAd;
    r1k->Bot_AD          = Read_IH_STA[Idx].bottomAd;
    r1k->Top_AD          = Read_IH_STA[Idx].topAd;
    r1k->Practical_Power = Read_IH_STA[Idx].actualPowerDiv25;
    r1k->target_Power    = Read_IH_STA[Idx].targetPowerDiv25;
    r1k->Practical_PPG   = Read_IH_STA[Idx].actualPPG;
    r1k->P_limited_STA   = Read_IH_STA[Idx].powerStatus;
    r1k->Pan_pulsating   = Read_IH_STA[Idx].loadValue & 0x0F;
    r1k->HVol_Cnt        = Read_IH_STA[Idx].vcountValue;
    r1k->PWMValue_L      = Read_IH_STA[Idx].equivalentResistance;
    r1k->PWMValue_H      = Read_IH_STA[Idx].res2;
    r1k->PowerAdjust     = 0x00;
    r1k->Version_Number  = DF_Versions;
    r1k->Fan_AD          = 0x00;
    r1k->ERROR           = Read_IH_STA[Idx].ihStatus & 0x0F;
    r1k->interior_ERR    = (Read_IH_STA[Idx].loadValue >> 4) & 0x0F;
    r1k->HZ_Cnt          = Read_IH_STA[Idx].res2;
    r1k->Discard_Cnt     = 0x00;

    EKF_Regs_Update(Idx);
}

/* ========== API_UART_RxEventCallback ================================= */
void API_UART_RxEventCallback(uint16_t Size)
{
    MB_Uart_Rx_Long = Size;
    API_UART_DMA_ReadValue(UARTX, MB_Uart_Rx_Data, DF_MB_Uart_Rx_LONG);
}

/* ========== Modbus_Protocol_Analysis_Main ============================ */
void Modbus_Protocol_Analysis_Main(void)
{
    unsigned short RLong;
    unsigned char  i;

    Modbus_Cofg_Init_SET();
    init_SR_RW_DATA();

    RLong = MB_Uart_Rx_Long;
    if (RLong == 0) return;

    /* 遍历从机地址匹配 */
    for (i = 0; i < DF_Stove_Quantity; i++) {
        if (MB_Uart_Rx_Data[0] == s_slave_addrs[i]) {
            Update_Static_Register_DATA(i);
            processModbusRequest(i, MB_Uart_Rx_Data, RLong);
            Get_Mobus_Response_Data_Lib(i, MB_Uart_Rx_Data, &RLong);

            if (RLong) {
                API_UART_DMA_SendValue(UARTX, MB_Uart_Rx_Data, RLong);
            }

            Modbus_I2c_Data_Main(i);
            break;
        }
    }

    MB_Uart_Rx_Long = 0;
}
