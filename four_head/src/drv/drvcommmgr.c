/**
 * @file    drvcommmgr.c
 * @brief   DrvCommMgr 模块实现 (v2.3 LINK+PARAMS)
 * @layer   drv
 *
 * 通信驱动管理 — 从 Power 接收功率命令，发送 Modbus
 */

// ===== [AI GENERATED] — 每次重生成 ===== 
#include "include/drvcommmgr_io.h"
#include "drvcommmgr.h"

MODULE_SKELETON(DrvCommMgr);

/* 管道就绪标志: 每 BIT 代表一个管道的 ST_NEW 状态 */
typedef union {
    uint8_t all;           /* 全零 = 无数据就绪，跳过本次 */
    struct {
        uint8_t apppower   : 1;  /* AppPower 数据就绪 */
    } bits;
} DrvCommMgr_PipeFlags_t;

// ===== [USER CODE] =====
// >>> 用户常量、变量、业务函数
// 此区域由用户维护，AI 重生成时保留

void DrvCommMgr_Process(MODULE_INPUT(DrvCommMgr) *in, DrvCommMgr_PipeFlags_t flags)
{
    if (flags.bits.apppower) { /* 处理 AppPower 输入 */ }
}

// >>> 用户常量
// #define DRVCOMMMGR_THRESHOLD  100

// >>> 用户变量
// static uint32_t s_drvcommmgr_counter;

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
    MODULE_INPUT(DrvCommMgr) *in = (MODULE_INPUT(DrvCommMgr)*)g_input.para;

    /* === 输入段: 数据有效检查 === */
    if (!in) return;
    DrvCommMgr_PipeFlags_t flags = {0};
    flags.bits.apppower = (in->AppPower_params.status & ST_NEW) ? 1 : 0;
    if (!flags.all) return;

    /* === 计算段: 用户业务 === */
    DrvCommMgr_Process(in, flags);

    /* === 输出段: 状态管理 === */
    in->AppPower_params.status  &= ~ST_NEW;  /* 消费完成 */
}

MODULE_EXPORT(DrvCommMgr);
