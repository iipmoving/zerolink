/**
 * @file    example_a.c
 * @brief   Module A — Producer (数据产生方)
 * @layer   app (示范)
 */

#include "../include/example_a_io.h"
#include "example_a.h"
#include <string.h>

/* ====== 内部状态 ====== */
static ExampleA_Output_t g_out;      /* 全局输出槽 */
static uint16_t s_counter = 0;       /* 内部计数器 */

/* ====== 构造函数（标准函数，首次 DoWork 时自检调用） ====== */
static void _Constructor(void)
{
    memset(&g_out, 0, sizeof(g_out));
    s_counter = EXAMPLE_A_INIT_VAL;
}

/* ====== 公开接口 ====== */

void ExampleA_GetIO(void **ppIn, ExampleA_Output_t **ppOut)
{
    if (ppIn)  *ppIn  = NULL;        /* A 是纯 Producer，无输入槽 */
    if (ppOut) *ppOut = &g_out;
}

void ExampleA_DoWork(void)
{
    /* --- 构造：懒惰初始化（首次进入，自检 bit0） --- */
    if (!(g_out.status & 0x01)) {
        _Constructor();
        g_out.status |= 0x01;        /* 标记已构造 */
    }

    /* ★ 每帧先清输出就绪标志 */
    g_out.status &= ~0x02;

    /* ====== 计算段 ====== */
    s_counter++;
    g_out.counter = s_counter;
    g_out.mode    = EXAMPLE_A_DEFAULT_MODE;

    /* ====== 输出段 ====== */
    g_out.status |= 0x02;            /* 本帧有产出，通知 Switcher */
}
