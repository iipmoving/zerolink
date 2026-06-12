/**
 * @file    app_power_io.h
 * @brief   PowerBase Data Switcher IO interface
 * @layer   app (Data Switcher IO)
 *
 * 多输入源: APP_Adc (ADC 数据), Comm (命令), Drv (硬件反馈)
 * 单一输出: Drv (功率控制命令)
 *
 * APP_Adc → PowerBase: field-by-field copy (布局不同, InputCallback 搬运)
 * Comm    → PowerBase: 命令解析后写入
 * Drv     → PowerBase: 10ms 硬件状态反馈
 * PowerBase → Drv:    每炉头 ppg_delta + power_state
 */

#ifndef APP_POWER_IO_H
#define APP_POWER_IO_H

#include <stdint.h>
#include "std_module.h"
//#include "app_power_hw_io.h"

#pragma pack(4)

#define POWER_POTMAX  4

/* ===================================================================
 * 参数定义 (数据列) — 定义模块间传输的数据结构
 * =================================================================== */

/* APP_Adc → PowerBase: 每炉头 ADC 数据
 * (布局与 ADC 端 MODULE_OUTPUT_PARAMS(APP_Adc, Power) 不同,
 *  InputCallback 中逐字段搬运) */
typedef struct {
    uint16_t current;
    uint16_t voltage;
    uint16_t igbt;
    uint16_t bottom;
    uint16_t phase;
    uint16_t phaseDown;
    uint16_t ceilQ;
    uint16_t power;
} MODULE_INPUT_PARAMS(APP_Adc, PowerBase);

/* Comm → PowerBase: 每炉头 MODBUS 命令 */
typedef struct {
    uint16_t target_power;
    uint8_t  power_on;
    uint8_t  res;
} MODULE_INPUT_PARAMS(Comm, PowerBase);

/* PowerBase → Drv: 每炉头功率控制输出 */
typedef struct {
    int16_t  ppg_delta;
    uint8_t  delta_valid;
    uint8_t  power_state;
    uint8_t  res[2];
} MODULE_OUTPUT_PARAMS(PowerBase, Drv);

/* Calculator → PowerBase: 每炉头直接参数 (Calculator 累加平均) */
typedef struct {
    int32_t resonant_current;
    int32_t phase_angle;
    int32_t voltage;
    uint8_t valid;
    uint8_t res[3];
} MODULE_INPUT_PARAMS(Calculator, PowerBase);

/* ===================================================================
 * LINK 连接器 (4 字节 INFO + params 指针)
 * =================================================================== */

/* APP_Adc → PowerBase: 输入 LINK */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(APP_Adc, PowerBase)* params[POWER_POTMAX];
} MODULE_INPUT_LINK(APP_Adc, PowerBase);

/* Comm → PowerBase: 输入 LINK */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_INPUT_PARAMS(Comm, PowerBase) params[POWER_POTMAX];
} MODULE_INPUT_LINK(Comm, PowerBase);    //这里声明的是实例 

/* Calculator → PowerBase: 输入 LINK */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  res[2];
    MODULE_INPUT_PARAMS(Calculator, PowerBase) params[POWER_POTMAX];
} MODULE_INPUT_LINK(Calculator, PowerBase);    //这里声明的是实例

/* PowerBase → Drv: 输出 LINK */
typedef struct {
    uint8_t  status;
    uint8_t  max_count;
    uint8_t  count;
    uint8_t  res[1];
    MODULE_OUTPUT_PARAMS(PowerBase, Drv) params[POWER_POTMAX];
} MODULE_OUTPUT_LINK(PowerBase, Drv);    //这里声明的是实例 

/* ===================================================================
 * 输入引用结构体 — 汇集所有输入源
 * =================================================================== */
typedef struct {
    MODULE_INPUT_LINK(APP_Adc, PowerBase)  adc;   /* 从 APP_Adc 来 */
    MODULE_INPUT_LINK(Comm, PowerBase)     comm;  /* 从 Comm 来 */
    MODULE_INPUT_LINK(Calculator, PowerBase) calc; /* 从 Calculator 来 */
//    PowerHw_Status_t                       hw_status;           /* 从 Drv 来 (10ms 反馈) */
} MODULE_INPUT(PowerBase);      //这里声明的是实例 

/* ===================================================================
 * 输出引用结构体
 * =================================================================== */
typedef struct {
    MODULE_OUTPUT_LINK(PowerBase, Drv)  head;                /* 每炉头输出 → Drv */
//    PowerHw_Command_t                   hw_cmd;                 /* → DRV 命令 (10ms) */
} MODULE_OUTPUT(PowerBase);

#pragma pack()

/* ---- v2.3 统一接口 ---- */
MODULE_IO_H(PowerBase);

#endif /* APP_POWER_IO_H */
