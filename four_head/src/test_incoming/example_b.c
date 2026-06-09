/**
 * @file    example_b.c
 * @brief   Module B — Consumer (数据消费方)
 * @layer   app (示范)
 */

#include "../include/example_b_io.h"
#include "example_b.h"
#include <string.h>

/* ====== 内部状态 ====== */
static ExampleB_Input_t g_in;        /* 全局输入槽 */
static uint16_t s_result = 0;        /* 消费结果 */

/* ====== 构造函数（标准函数，首次 DoWork 时自检调用） ====== */
static void _Constructor(void)
{
    memset(&g_in, 0, sizeof(g_in));
}

/* ====== 内部函数 ====== */
static uint16_t _ProcessCounter(uint16_t raw)
{
    return raw * EXAMPLE_B_MULTIPLIER;
}

/* ====== 公开接口 ====== */

void ExampleB_GetIO(ExampleB_Input_t **ppIn, void **ppOut)
{
    if (ppIn)  *ppIn  = &g_in;       /* B 是 Consumer，暴露输入槽 */
    if (ppOut) *ppOut = NULL;        /* B 无输出槽 */
}

void ExampleB_DoWork(void)
{
    /* --- 构造：懒惰初始化（首次进入，自检 bit0） --- */
    if (!(g_in.status & 0x01)) {
        _Constructor();
        g_in.status |= 0x01;
    }

    /* ====== 输入段 ====== */
    if (!(g_in.status & 0x02)) {
        return;                      /* 无新数据，跳过 */
    }

    /* ====== 计算段 ====== */
    uint16_t raw = g_in.counter;
    s_result = _ProcessCounter(raw);

    /* ====== 输出段 ====== */
    /* 本模块无输出槽，消费即结束 */

    g_in.status &= ~0x02;            /* 消费完毕，自清 */
}
