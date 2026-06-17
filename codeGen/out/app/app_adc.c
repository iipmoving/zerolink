/**
 * @file    app_adc.c
 * @brief   AppAdc 模块实现 (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * ADC 采样 — 输出谐振电流/电压/HRTIM 时序到 Calculator 和功率参数到 AppPower
 */

// ===== [AI GENERATED] — 每次重生成 ===== 
#include "include/app_adc_io.h"

MODULE_SKELETON(AppAdc);

// ===== [USER CODE] =====
// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

void AppAdc_Process(MODULE_OUTPUT(AppAdc) *out)
{
    /* TODO: 产生输出数据 */
}

// >>> 用户常量
// #define APPADC_THRESHOLD  100

// >>> 用户变量
// static uint32_t s_app_adc_counter;

// ===== [END USER CODE] =====

// ===== [AI GENERATED] — 每次重生成 ===== 

static void Init(void)
{
    g_input.para  = NULL;  /* 无输入 */
    g_output.para = &s_outPara;

    userInit();     // >>> [USER: 统一初始化入口]
}

static void ProcessInput(void)
{
    MODULE_OUTPUT(AppAdc) *out = (MODULE_OUTPUT(AppAdc)*)g_output.para;

    /* === 输入段: 数据有效检查 === */
    // 无输入依赖，直接处理

    /* === 计算段: 用户业务 === */
    AppAdc_Process(out);

    /* === 输出段: 状态管理 === */
    out->AppPower_params.status |= ST_OUT;  /* 输出就绪 */
    out->Calculator_params.status |= ST_OUT;  /* 输出就绪 */
}

MODULE_EXPORT(AppAdc);
