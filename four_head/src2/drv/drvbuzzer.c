/**
 * @file    drvbuzzer.c
 * @brief   DrvBuzzer 模块实现 (v2.3 LINK+PARAMS)
 * @layer   drv
 *
 * 蜂鸣器驱动 — 从 Hmi 接收蜂鸣命令 (@OUTPUT_CALLBACK 即时路由)
 */

// ===== [AI GENERATED] — 每次重生成 ===== 
#include "include/drvbuzzer_io.h"
#include "drvbuzzer.h"

MODULE_SKELETON(DrvBuzzer);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;           /* 全零 = 无数据就绪，跳过本次 */
    struct {
        uint8_t apphmi   : 1;  /* AppHmi 数据就绪 */
    } bits;
} DrvBuzzer_PipeFlags_t;

// ===== [USER CODE] =====
// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

void DrvBuzzer_Process(MODULE_INPUT(DrvBuzzer) *in, DrvBuzzer_PipeFlags_t flags)
{
    if (flags.bits.apphmi) { /* 处理 AppHmi 输入 */ }
}

// >>> 用户常量
// #define DRVBUZZER_THRESHOLD  100

// >>> 用户变量
// static uint32_t s_drvbuzzer_counter;

// ===== [END USER CODE] =====

// ===== [AI GENERATED] — 每次重生成 ===== 

static void Init(void)
{
    g_input.para  = &s_in;
    g_output.para = NULL;  /* 无输出 */

    // >>> [USER: 添加初始化代码]
}

static void ProcessInput(void)
{
    MODULE_INPUT(DrvBuzzer) *in = (MODULE_INPUT(DrvBuzzer)*)g_input.para;

    /* === 输入段: 数据有效检查 === */
    if (!in) return;
    DrvBuzzer_PipeFlags_t flags = {0};
    flags.bits.apphmi = (in->AppHmi_params.status & ST_NEW) ? 1 : 0;
    if (!flags.all) return;

    /* === 计算段: 用户业务 === */
    DrvBuzzer_Process(in, flags);

    /* === 输出段: 状态管理 === */
    in->AppHmi_params.status  &= ~ST_NEW;  /* 消费完成 */
}

MODULE_EXPORT(DrvBuzzer);
