/**
 * @file    app_power.c
 * @brief   AppPower 模块实现 (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * 功率控制 PID — 从 AppAdc 拉取数据，输出功率增量到 DrvHrtim
 */

// ===== [AI GENERATED] — 每次重生成 ===== 
#include "include/app_power_io.h"

MODULE_SKELETON(AppPower);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;           /* 全零 = 无数据就绪，跳过本次 */
    struct {
        uint8_t appadc   : 1;  /* AppAdc 数据就绪 */
        uint8_t calculator   : 1;  /* Calculator 数据就绪 */
    } bits;
} AppPower_PipeFlags_t;

// ===== [USER CODE] =====
// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

void AppPower_Process(MODULE_INPUT(AppPower) *in, AppPower_PipeFlags_t flags)
{
    if (flags.bits.appadc) { /* 处理 AppAdc 输入 */ }
    if (flags.bits.calculator) { /* 处理 Calculator 输入 */ }
}

// >>> 用户常量
// #define APPPOWER_THRESHOLD  100

// >>> 用户变量
// static uint32_t s_app_power_counter;

// ===== [END USER CODE] =====

// ===== [AI GENERATED] — 每次重生成 ===== 

static void Init(void)
{
    g_input.para  = &s_inPara;
    g_output.para = NULL;  /* 无输出 */

    userInit();     // >>> [USER: 统一初始化入口]
}

static void ProcessInput(void)
{
    MODULE_INPUT(AppPower) *in = (MODULE_INPUT(AppPower)*)g_input.para;

    /* === 输入段: 数据有效检查 === */
    if (!in) return;
    AppPower_PipeFlags_t flags = {0};
    flags.bits.appadc = (in->AppAdc_params->status & ST_NEW) ? 1 : 0;
    flags.bits.calculator = (in->Calculator_params->status & ST_NEW) ? 1 : 0;
    if (!flags.all) return;

    /* === 计算段: 用户业务 === */
    AppPower_Process(in, flags);

    /* === 输出段: 状态管理 === */
    in->AppAdc_params->status  &= ~ST_NEW;  /* 消费完成 */
    in->Calculator_params->status  &= ~ST_NEW;  /* 消费完成 */
}

MODULE_EXPORT(AppPower);
