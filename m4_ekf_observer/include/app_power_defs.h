/**
 * @file    app_power_defs.h
 * @brief   PowerBase / PowerCalc 内部共享类型定义
 * @layer   app (internal)
 *
 * 本文件定义 PowerBase 和 PowerCalc 模块共享的内部类型。
 * 这些类型原本在 app_power.c 中定义，拆分后两个模块都需要。
 * 不包含 I/O 接口（I/O 接口在各自的 io.h 中）。
 */

#ifndef APP_POWER_DEFS_H
#define APP_POWER_DEFS_H

#include <stdint.h>
#include "data_type.h"
#include "API_HRTIM.h"
#include "phase.h"
#include "S_PID.h"
#include "s_data_stack.h"

/* ================================================================
 * 从 app_power.h 提取的类型定义和常量
 * (原 app_power.h 包含函数声明, 与新模块冲突, 故只取类型)
 * ================================================================ */

/* ---- PowerStatusDef: IH 控制器状态寄存器 ---- */
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
    uint8_t checkSum;
} PowerStatusDef;

/* ---- PowerInitDef: 初始化参数 ---- */
typedef struct {
    uint8_t loadTest;
    uint8_t ovpShort;
    uint8_t loadLeave;
    uint8_t vcLimitMax;
    uint8_t loadLeavePhase;
    uint8_t minPhase;
    uint8_t potPowerM;
    uint8_t maxPowerM;
} PowerInitDef;

/* ---- PowerControlDef: 控制参数 ---- */
typedef struct {
    uint8_t powerControlSet;
    uint8_t powerSwitch;
    uint8_t powerSetm;
    uint8_t fanSpeed;
    uint8_t kValue;
    uint8_t res1;
    uint8_t res2;
    uint8_t res3;
} PowerControlDef;

/* ---- FlashValueDef: FLASH 保存数据 ---- */
typedef struct {
    INT8U protect;
    INT8U len;
    INT8U power25;
    INT8U voltage210;
    INT8U checksum;
} FlashValueDef;

/* ---- 常量 (来自 app_power.h) ---- */
#define MinPowerM               (500/25)
#define POT_TYPE_DELAY1         10
#define POT_TYPE_DELAY2         15
#define APP_POWER_PAN_DELAY     30
#define C_ERR_MAIN              2
#define C_VOLTAGE_210V          0x8D
#define C_P25W                  0x47

/* ---- 关机原因枚举 (来自 app_power.h) ---- */
enum {
    PowerOffSurge     = 0x1,
    PowerOffCommLost  = 0x2,
    PowerOffNoPan     = 0x11,
    PowerOffZero      = 0x12,
    PowerOffCheckPan  = 0x13,
};

/* ---- 检锅/ADC 状态枚举 (来自 app_power.h) ---- */
enum {
    PanCheckRest = 0,
    PanDmaEnd,
    PanFmacEnd,
    PanPluseEnd,
    TxaDmaEnd,
};

/* ---- AWD 选择 (来自 app_power.h) ---- */
enum {
    TxaAwdHigh = 0,
    TxaAwdLow,
};

#pragma pack(4)

/* ---- 锅具材质枚举 ---- */
enum {
    PotSteel = 0,
    PotIron,
};

/* ---- PowerFlagDef: 功率模块标志位 ---- */
typedef struct {
    uint32_t PowerPause      :1;
    uint32_t IcVcAdcOk       :1;
    uint32_t ppgOn           :1;
    uint32_t PowerCycleType  :1;
    uint32_t PowerHoldMax    :1;
    uint32_t PowerHoldMin    :1;
    uint32_t PowerDuty50     :1;
    uint32_t PotType         :1;
    uint32_t CheckPan        :1;
    uint32_t ppgAdd          :1;
    uint32_t DisVoltage      :1;
    uint32_t PowerOff        :1;
    uint32_t ppgWork         :1;
    uint32_t Vcout           :1;
    uint32_t ppgLock         :1;
    uint32_t powerResume     :1;
    uint32_t powerSingle     :1;
    uint32_t LoadCheckPan    :1;
} PowerFlagDef __attribute__((aligned(32)));

/* ---- PowerInputDef: 通讯输入结构体 ---- */
typedef struct {
    PowerStatusDef  status;
    PowerInitDef    init;
    PowerControlDef control;
    FlashValueDef*  flash;
} PowerInputDef __attribute__((aligned(32)));

/* ---- APP_POWER_CYCLE_DEF: 同频倍频切换 ---- */
typedef struct {
    uint16_t doubleCount;
    uint16_t baseCount;
    uint16_t doubleOn;
    uint16_t res1;
    uint16_t baseDuty;
    uint16_t doubleDuty;
    uint16_t basePower;
    uint16_t doublePower;
} APP_POWER_CYCLE_DEF __attribute__((aligned(32)));

/* ---- PowerPotCheckDef: 判锅结构体 ---- */
typedef struct {
    uint16_t duty;
    uint16_t power;
    uint16_t res;
    uint8_t  count;
    uint8_t  channel;
} PowerPotCheckDef __attribute__((aligned(32)));

/* ---- AppPowerStaticDef: 需要上电清零的数据区 ---- */
typedef struct {
    uint8_t         res_2;
    uint8_t         SurgePower;
    uint8_t         LeaveTime;
    uint8_t         DeadCnt;
    uint8_t         VcoutDelay;
    uint8_t         cycleRoll;
    uint8_t         powerHalfCnt;
    uint8_t         limitQ;
    uint8_t         SurgeDelay;
    uint8_t         power_half_adj;
    uint8_t         LimitMaxCount;
    uint8_t         LimitVoltage;
    uint8_t         ppgLimitPower;
    uint8_t         checkPanStep;
    uint8_t         _1;
    uint8_t         DivMaxPowerRes;
    uint8_t         PowerArrive;
    uint8_t         PowerSurge;
    uint8_t         PowerOffFlag;
    uint8_t         PowerLimitFlag;
    uint16_t        res2;
    int16_t         phaseValue;
    uint16_t        current16;
    uint16_t        phaseSumValue;
    uint16_t        ppgLimitMax;
    uint16_t        ppgPowerAdj;
    uint16_t        PowerAdcTrig;
    uint16_t        PowerAdcFact;
    uint16_t        PPGdutyActual;
    uint16_t        PowerDuty;
    uint16_t        PpgLimit;
    uint16_t        OvpValue;
    uint16_t        limitQSum;
    uint16_t        powerCycle;
    uint32_t        PowerTxaFact;
    uint32_t        CompDacValue;
    PPGvalueDef             ppgValue;
    APP_POWER_CYCLE_DEF     cycleChange;
    PowerPotCheckDef        potPowerSave[4];
    PhaseController         potPhase;
    union {
        uint32_t        word;
        PowerFlagDef    bit;
    } flag;
} AppPowerStaticDef __attribute__((aligned(32)));

/* ---- AppPowerKeepDef: 不需要上电清零的数据区 ---- */
typedef struct {
    uint8_t         channel;
    uint8_t         SetMaxPower;
    uint8_t         PancTime;
    uint8_t         _3;
    uint16_t        ppgSave;
    uint16_t        res;
    uint16_t        Power25wAd;
    uint16_t        Valtage210;
    FixedPIDController PowerPIDstr;
} AppPowerKeepDef __attribute__((aligned(32)));

/* ---- AppPowerFunDef: 方法定义 (函数指针表) ---- */
typedef struct {
    void        (*_PPGinit)(void);
    void        (*_PPGdeadTime)(uint8_t downDts, uint8_t upDts);
    void        (*_PPGsetDuty)(uint16_t input);
    PPGvalueDef (*_PPGgetValue)(void);
    void        (*_PPGonOff)(uint8_t onOff);
    void        (*_PPGgetAdcValue)(void);
    void        (*_PanCountInit)(void);
    uint8_t     (*_PanCountGetValue)(void);
    void        (*_PanCountSetValue)(uint8_t onOff);
    uint8_t     (*_TimBkFlag)(void);
} AppPowerFunDef __attribute__((aligned(32)));

/* ---- AppPowerDef: 单炉头功率控制聚合 ---- */
typedef struct {
    void*               commSet;
    PowerInputDef*      input;
    AppPowerStaticDef*  staticReg;
    AppPowerKeepDef*    keepReg;
    AppPowerFunDef*     funAdr;
} AppPowerDef;

/* ---- PowerAllFlagDef: 多炉头统一标志 ---- */
typedef struct {
    uint8_t rest        :1;
    uint8_t potCheckEnd :1;
    uint8_t scrOn       :1;
} PowerAllFlagDef;

/* ---- AppPowerAllDef ---- */
typedef struct {
    union {
        uint8_t             byte;
        PowerAllFlagDef     bit;
    } flag;
    uint8_t         changeStatus;
    uint8_t         PotNum;
    uint8_t         scrCnt;
    uint16_t        minFre;
    uint16_t        pCycle;
    uint32_t        ovp;
    StackStructDef  stack;
} AppPowerAllDef __attribute__((aligned(32)));

#pragma pack()

/* ---- 全局变量声明 (定义在 power_base.c) ---- */
extern AppPowerDef       PowerMem[4];
extern AppPowerStaticDef PowerStaticReg[4];
extern AppPowerKeepDef   PowerKeepReg[4];
extern AppPowerDef      *PowerControl;
extern AppPowerAllDef    PowerAll;

/* ---- 函数声明 ---- */
AppPowerDef * getPowerCHN(uint8_t chn);

#endif /* APP_POWER_DEFS_H */
