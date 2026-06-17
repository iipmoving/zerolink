/**
 * @file    drvkey.c
 * @brief   DrvKey 模块实现 (v2.3 LINK+PARAMS)
 * @layer   drv
 *
 * 按键扫描与事件分发 — 输出按键事件到 Hmi/Cooking/SegAlign
 */

// ===== [AI GENERATED] — 每次重生成 ===== 
#include "include/drvkey_io.h"
#include "drvkey.h"

MODULE_SKELETON(DrvKey);

// ===== [USER CODE] =====
// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

void DrvKey_Process(MODULE_OUTPUT(DrvKey) *out)
{
    /* TODO: 产生输出数据 */
}

// >>> 用户常量
// #define DRVKEY_THRESHOLD  100

// >>> 用户变量
// static uint32_t s_drvkey_counter;

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
    MODULE_OUTPUT(DrvKey) *out = (MODULE_OUTPUT(DrvKey)*)g_output.para;

    /* === 输入段: 数据有效检查 === */
    // 无输入依赖，直接处理

    /* === 计算段: 用户业务 === */
    DrvKey_Process(out);

    /* === 输出段: 状态管理 === */
    out->AppHmi_params.status |= ST_OUT;  /* 输出就绪 */
    out->AppCooking_params.status |= ST_OUT;  /* 输出就绪 */
    out->AppSegAlign_params.status |= ST_OUT;  /* 输出就绪 */
}

MODULE_EXPORT(DrvKey);
