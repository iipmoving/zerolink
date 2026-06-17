/**
 * @file    ekf_lkf.c
 * @brief   EKF_LKF 模块实现 (v2.3 LINK+PARAMS)
 * @layer   base_class
 *
 * 扩展卡尔曼滤波观测器 — 从 ElecParams 读取观测值，估计 [R, L, f_res]
 */

// ===== [AI GENERATED] — 每次重生成 ===== 
#include "include/ekf_lkf_io.h"

MODULE_SKELETON(EKF_LKF);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;           /* 全零 = 无数据就绪，跳过本次 */
    struct {
        uint8_t elecparams   : 1;  /* ElecParams 数据就绪 */
    } bits;
} EKF_LKF_PipeFlags_t;

// ===== [USER CODE] =====
// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

void EKF_LKF_Process(MODULE_INPUT(EKF_LKF) *in, EKF_LKF_PipeFlags_t flags)
{
    if (flags.bits.elecparams) { /* 处理 ElecParams 输入 */ }
}

// >>> 用户常量
// #define EKF_LKF_THRESHOLD  100

// >>> 用户变量
// static uint32_t s_ekf_lkf_counter;

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
    MODULE_INPUT(EKF_LKF) *in = (MODULE_INPUT(EKF_LKF)*)g_input.para;

    /* === 输入段: 数据有效检查 === */
    if (!in) return;
    EKF_LKF_PipeFlags_t flags = {0};
    flags.bits.elecparams = (in->ElecParams_params->status & ST_NEW) ? 1 : 0;
    if (!flags.all) return;

    /* === 计算段: 用户业务 === */
    EKF_LKF_Process(in, flags);

    /* === 输出段: 状态管理 === */
    in->ElecParams_params->status  &= ~ST_NEW;  /* 消费完成 */
}

MODULE_EXPORT(EKF_LKF);
