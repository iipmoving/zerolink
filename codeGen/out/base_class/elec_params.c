/**
 * @file    elec_params.c
 * @brief   ElecParams 模块实现 (v2.3 LINK+PARAMS)
 * @layer   base_class
 *
 * 电参数计算 — 20ms 周期计算 I_peak/L/f_res/Q/R/P 等，输出到 AppPower 和 EKF_LKF
 */

// ===== [AI GENERATED] — 每次重生成 ===== 
#include "include/elec_params_io.h"

MODULE_SKELETON(ElecParams);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;           /* 全零 = 无数据就绪，跳过本次 */
    struct {
        uint8_t calculator   : 1;  /* Calculator 数据就绪 */
    } bits;
} ElecParams_PipeFlags_t;

// ===== [USER CODE] =====
// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

// 用户业务函数 — by ProcessInput 调用
void ElecParams_Process(MODULE_INPUT(ElecParams) *in, MODULE_OUTPUT(ElecParams) *out, ElecParams_PipeFlags_t flags)
{
    if (flags.bits.calculator) { /* 处理 Calculator 输入 */ }
    /* TODO: 写入 EKF_LKF 输出 */
}

// >>> 用户常量
// #define ELECPARAMS_THRESHOLD  100

// >>> 用户变量
// static uint32_t s_elec_params_counter;

// ===== [END USER CODE] =====

// ===== [AI GENERATED] — 每次重生成 ===== 

static void Init(void)
{
    g_input.para  = &s_inPara;
    g_output.para = &s_outPara;

    userInit();     // >>> [USER: 统一初始化入口]
}

static void ProcessInput(void)
{
    MODULE_INPUT(ElecParams) *in = (MODULE_INPUT(ElecParams)*)g_input.para;
    MODULE_OUTPUT(ElecParams) *out = (MODULE_OUTPUT(ElecParams)*)g_output.para;

    /* === 输入段: 数据有效检查 === */
    if (!in) return;
    ElecParams_PipeFlags_t flags = {0};
    flags.bits.calculator = (in->Calculator_params->status & ST_NEW) ? 1 : 0;
    if (!flags.all) return;

    /* === 计算段: 用户业务 === */
    ElecParams_Process(in, out, flags);

    /* === 输出段: 状态管理 === */
    in->Calculator_params->status  &= ~ST_NEW;  /* 消费完成 */
    out->EKF_LKF_params.status |= ST_OUT;  /* 输出就绪 */
}

MODULE_EXPORT(ElecParams);
