/**
 * @file    data_telemetry.h
 * @brief   Telemetry — Calculator→ElecParams 数据流 MODBUS 遥测
 * @note    内嵌缓冲区，零指针跟随，MODBUS 直接映射
 */

#ifndef DATA_TELEMETRY_H
#define DATA_TELEMETRY_H

#include <stdint.h>

#define TELE_CALC_PERIODS  20
#define TELE_CALC_WORDS    18   /* 每周期 18 uint16 (含 uint32 split) */
#define TELE_ELEC_WORDS    26   /* 12 float(2words) + valid(1word) */
#define TELE_TOTAL_WORDS   (4 + TELE_CALC_PERIODS * TELE_CALC_WORDS + TELE_ELEC_WORDS)
/* 4 + 360 + 26 = 390 */

/* ---- 控制寄存器 ---- */
typedef struct {
    uint16_t ctrl_start;      /* 写1=开始监测, 写0=停止 */
    uint16_t reserved;
    uint16_t status;          /* bit0=running, bit1=calc_ready, bit2=elec_ready */
    uint16_t reserved2;
} TelemCtrl_t;

/* ---- 单周期数据 (18 uint16) ---- */
typedef struct {
    uint16_t hrtim_highOff;
    uint16_t hrtim_lowOff;
    uint16_t hrtim_highOn;
    uint16_t hrtim_lowOn;
    uint16_t peak_current;
    uint16_t act_curr_high_lo;
    uint16_t act_curr_high_hi;
    uint16_t act_curr_low_lo;
    uint16_t act_curr_low_hi;
    uint16_t volt_sum_lo;
    uint16_t volt_sum_hi;
    uint16_t voltage_count;
    uint16_t zero_cross_high;
    uint16_t zero_cross_low;
    uint16_t peak_point;
    uint16_t res[3];          /* 补齐到 18 */
} TelemCalcPeriod_t;

/* ---- 输出聚合 ---- */
typedef struct {
    TelemCtrl_t              ctrl;
    TelemCalcPeriod_t        calc[TELE_CALC_PERIODS];
    float                    elec[12];  /* 12 float = 24 uint16 */
    uint16_t                 elec_valid;
} TelemetryData_t;

/* API */
void Telemetry_Init(void);
TelemetryData_t* Telemetry_GetData(void);

#endif /* DATA_TELEMETRY_H */
