/**
 * @file    appcommmgr.c
 * @brief   AppCommMgr 模块实现 (v2.3 LINK+PARAMS)
 * @layer   app
 *
 * Modbus 通信轮询调度器 — 分发寄存器数据到 Power/Cooking/Protect
 */

// ===== [AI GENERATED] — 每次重生成 ===== 
#include "include/appcommmgr_io.h"
#include "appcommmgr.h"

MODULE_SKELETON(AppCommMgr);

// ===== [USER CODE] =====
// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

void AppCommMgr_Process(MODULE_OUTPUT(AppCommMgr) *out)
{
    /* TODO: 产生输出数据 */
}

// >>> 用户常量
// #define APPCOMMMGR_THRESHOLD  100

// >>> 用户变量
// static uint32_t s_appcommmgr_counter;

// ===== [END USER CODE] =====

// ===== [AI GENERATED] — 每次重生成 ===== 

static void Init(void)
{
    g_input.para  = NULL;  /* 无输入 */
    g_output.para = &s_out;

    // >>> [USER: 添加初始化代码]
}

static void ProcessInput(void)
{
    MODULE_OUTPUT(AppCommMgr) *out = (MODULE_OUTPUT(AppCommMgr)*)g_output.para;

    /* === 输入段: 数据有效检查 === */
    // 无输入依赖，直接处理

    /* === 计算段: 用户业务 === */
    AppCommMgr_Process(out);

    /* === 输出段: 状态管理 === */
    out->AppPower_params.status |= ST_OUT;  /* 输出就绪 */
    out->AppCooking_params.status |= ST_OUT;  /* 输出就绪 */
    out->AppProtect_params.status |= ST_OUT;  /* 输出就绪 */
}

MODULE_EXPORT(AppCommMgr);
