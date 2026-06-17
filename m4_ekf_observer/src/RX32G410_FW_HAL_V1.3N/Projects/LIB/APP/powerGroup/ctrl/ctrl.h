/* ctrl.h — CTRL 层公共接口 (APP 逻辑层)
 * layer: CTRL (APP logic)
 *
 * 职责: 功率控制、检锅状态机、锅类型判断、周期/频率策略、过零策略。
 *       APP 调这些接口替代直接操作 PowerMem。
 *
 * 每个模块自行声明内部 struct，不 extern。
 */
#ifndef CTRL_H
//#define CTRL_H

#include <stdint.h>

/* ===== 功率控制输出状态 (ctrl_GetState / ctrl_PowerRun 输出) ===== */
typedef struct {
    uint16_t power_duty;
    uint16_t duty_actual;
    uint16_t power_adc_fact;
    uint8_t  actual_power;
    uint8_t  actual_ppg;
    uint8_t  power_status;
    uint8_t  ih_status;
    uint8_t  target_power;
    uint8_t  off_flag;
    uint8_t  power_limit_flag;
    uint8_t  power_dead_cnt;
    uint16_t ppg_limit;
    uint16_t ppg_limit_max;
    uint16_t ppg_limit_power;
    uint8_t  limit_max_count;
    uint8_t  limit_voltage;
    uint8_t  limit_Qvalue;
    uint8_t  ppg_lock_flag;
    uint8_t  vcout_flag;
    uint8_t  vcout_delay;
    uint8_t  ppg_add_flag;
    uint8_t  power_half_adj;
    uint8_t  half_cnt;
    uint8_t  resume_flag;
    uint8_t  ic_vc_adc_ok_flag;
    uint16_t bottom_value;
    uint16_t top_value;
    uint16_t power_min_fre;
    int      pid_return;
} CtrlPowerState_t;

/* ===== 功率控制 ===== */
uint16_t ctrl_PowerRun(uint8_t ch);      /* 功率控制主循环, 返回 power_duty */
void ctrl_SetInputs(uint8_t ch, uint8_t current_ad, uint8_t voltage_ad);
void ctrl_SyncHoldFlags(uint8_t ch, uint8_t hold_max, uint8_t hold_min);
void ctrl_GetState(uint8_t ch, CtrlPowerState_t *state);

/* 单字段查询 */
uint16_t ctrl_GetPowerDuty(uint8_t ch);
uint8_t  ctrl_GetActualPower(uint8_t ch);
uint8_t  ctrl_GetActualPPG(uint8_t ch);
uint8_t  ctrl_GetPowerStatus(uint8_t ch);
uint8_t  ctrl_GetIHStatus(uint8_t ch);
int      ctrl_GetPidReturn(uint8_t ch);
void ctrl_PowerType(void);               /* 功率模式选择 (原 PowerTypeFun) */
void ctrl_PowerOff(uint8_t ch);          /* 关闭通道功率 (原 powerOffPot) */
void ctrl_PowerOnMin(uint8_t ch);        /* 最小功率启动 (原 powerOnSetMIN) */
uint8_t ctrl_PowerZeroAdjust(uint8_t ch); /* 过零功率调整 (原 power_zero_adjust) */

/* ===== 检锅状态机 ===== */
void ctrl_PanStateRun(uint8_t ch);       /* 检锅状态机 (原 PowerPanCheckFun) */
void ctrl_PanStateCheckLeave(void);      /* 移锅检测 */
uint8_t ctrl_PanErrAdj(uint8_t ch, uint8_t lens);
void ctrl_PanInit(uint8_t pot_num);
void ctrl_PanSetState(uint8_t ch, uint8_t state);
uint8_t ctrl_PanGetState(uint8_t ch);

/* ===== 锅类型判断 ===== */
void ctrl_PotTypeCheck(void);            /* 锅类型判断 (原 APP_POWER_PotTypeCheck) */
void ctrl_PotTypeCheckFRE(uint8_t ch);   /* 特定频率下判断 */
void ctrl_PotTypeConfirm(void);
void ctrl_PotMaxPpgSet(uint8_t ch);
void ctrl_PotGetMinMaxPPG(uint8_t ch);
void ctrl_PotInit(uint8_t pot_num);
uint8_t ctrl_PotGetType(uint8_t ch);
void ctrl_PotSetType(uint8_t ch, uint8_t type);

/* ===== 周期/频率策略 ===== */
uint8_t ctrl_GetCycleType(uint8_t change);
void ctrl_CycleChange(void);
void ctrl_CycleReset(void);
void ctrl_SetPowerCycleType(void);
void ctrl_ResetPowerCycleType(void);
void ctrl_DutyLess50(void);
void ctrl_CycleInit(uint8_t pot_num);
void ctrl_CycleUpdateDuty(uint8_t ch, uint16_t duty);
uint16_t ctrl_CycleGetPowerCycle(void);
uint16_t ctrl_CycleGetChPowerCycle(uint8_t ch);
uint16_t ctrl_CycleGetChDuty(uint8_t ch);
uint8_t ctrl_CycleGetChangeStatus(void);

/* ===== 过零策略 ===== */
void ctrl_ZeroChange(void);
uint8_t ctrl_ZeroSync(void);
void ctrl_ZeroInit(uint8_t pot_num);
void ctrl_ZeroUpdate(uint8_t ch, uint16_t power_cycle, uint16_t duty,
                     uint8_t cycle_roll, uint8_t cycle_type);

#endif /* CTRL_H */
