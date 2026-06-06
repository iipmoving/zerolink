/**
 * @file    data_switcher.c
 * @brief   v2.0 Data Switcher — 结构化数据路由中间层
 * @layer   core
 *
 * 职责: 全项目唯一有权 include 所有 _io.h 的文件.
 * 检测 producer 的 status bit1 → 搬运字段到 consumer Input_t →
 * 清 producer bit1 → 设 consumer bit1 → 调各模块 DoWork.
 *
 * 模块不操作别人的 status — 只有 Switcher 有权读写所有模块的 status.
 *
 * Slot1 (每 ~10ms): Adc_DoWork → 检测 ADC flag → 搬运字段 → Power_DoWork
 */

#include "data_switcher.h"
#include "../include/app_adc_io.h"      /* 特权: 全路径 include 所有 _io.h */
#include "../include/app_power_io.h"
#include	<string.h>

/* === 内部指针缓存 (Init 时绑定) ================================== */
static Adc_Output_t   *pAdc_Out;
static Power_Input_t  *pPower_In;
static Power_Output_t *pPower_Out;

/* === 初始化 ======================================================= */
void Switcher_Init(void)
{
    Adc_GetIO(&pAdc_Out);
    Power_GetIO(&pPower_In, &pPower_Out);

    /* 上电清所有模块 input/output status → 模块首次 DoWork 自检 bit0=0 → _Constructor() */
    pAdc_Out->status   = 0;
    pPower_In->status  = 0;
    pPower_Out->status = 0;

    pAdc_Out->status   |= 0x01;          /* bit0=已构造 (ADC 无 Input_t, 直接标记) */
}

/* === Slot1 调度入口 (约每 10ms) ================================ */
void Switcher_Run_Slot1(void)
{
    /* 1. producer: 检测 20ms 数据 → 平均 → 置 g_out bit1 */
    Adc_DoWork();

    /* 2. 路由: 检测 producer bit1 → 搬运字段 → 清 producer bit1 → 设 consumer bit1 */
    if (pAdc_Out->status & 0x02) {               /* producer 有新数据? */
        /* 按接线规则搬运: Adc_Output_t.inputValue[] → Power_Input_t.inputValue[] */
        memcpy(pPower_In->inputValue, pAdc_Out->inputValue,
               sizeof(pPower_In->inputValue));
        pAdc_Out->status  &= ~0x02;              /* 清 producer — 数据已取走 */
        pPower_In->status |=  0x02;              /* 通知 consumer — 新输入到达 */
    }

    /* 3. consumer: 检查 g_in bit1 → 消费 → 计算 → 置 g_out bit1 */
    Power_DoWork();
}
